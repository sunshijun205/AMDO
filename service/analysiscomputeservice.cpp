#include "service/analysiscomputeservice.h"

#include "model/aircraftstore.h"
#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "model/srdtypes.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QtGlobal>
#include <QtMath>

#include <cmath>

// ---- 气动估算的假设系数（低保真占位）------------------------------------
// TODO：后期应改为从气动分析配置/极曲线求解得到，而非写死常数。
static const double kAssumedCd0 = 0.020;      // 零升阻力系数（假设）
static const double kAssumedOswaldE = 0.80;   // Oswald 效率因子（假设）

// ---- ISA 1976 标准大气（0–20km，供气动真实计算）--------------------------
struct IsaPoint {
    double T;    // 温度 K
    double p;    // 压强 Pa
    double rho;  // 密度 kg/m³
    double a;    // 声速 m/s
};

static IsaPoint isaAt(double h)
{
    const double g = 9.80665;
    const double R = 287.05287;
    const double gamma = 1.4;
    const double T0 = 288.15;
    const double p0 = 101325.0;
    const double L = -0.0065; // 对流层温度梯度 K/m

    double T = T0;
    double p = p0;
    if (h <= 11000.0) {
        T = T0 + L * h;
        p = p0 * std::pow(T / T0, -g / (L * R));
    } else {
        const double T11 = T0 + L * 11000.0; // 216.65 K
        const double p11 = p0 * std::pow(T11 / T0, -g / (L * R));
        const double hh = qMin(h, 20000.0);  // 平流层下段等温
        T = T11;
        p = p11 * std::exp(-g * (hh - 11000.0) / (R * T11));
    }
    IsaPoint pt;
    pt.T = T;
    pt.p = p;
    pt.rho = p / (R * T);
    pt.a = std::sqrt(gamma * R * T);
    return pt;
}

static AnalysisResultItem makeItem(const QString &key, const QString &label, double value,
                                   const QString &unit, const QString &fidelity,
                                   const QString &note = QString())
{
    AnalysisResultItem it;
    it.key = key;
    it.label = label;
    it.value = value;
    it.valueKnown = true;
    it.unit = unit;
    it.fidelity = fidelity;
    it.note = note;
    return it;
}

// 缺模型/缺数据时的占位项（明确标 MOCK）。
static AnalysisResultItem mockItem(const QString &key, const QString &label, double value,
                                   const QString &unit, const QString &note)
{
    return makeItem(key, label, value, unit, AnalysisFidelity::mock(), note);
}

// 从分析集 values 读取某数值字段（domainId/sectionTitle/fieldLabel）。
static double configNum(const AnalysisComputeInputs &in, const QString &domain,
                        const QString &section, const QString &field, bool *ok)
{
    const QString key = analysisFieldKey(domain, section, field);
    if (in.analysis.values.contains(key))
        return in.analysis.values.value(key).toDouble(ok);
    if (ok)
        *ok = false;
    return 0.0;
}

// Sutherland 公式：空气动力粘性 μ(T)（Pa·s）。
static double sutherlandMu(double T)
{
    const double mu0 = 1.716e-5;
    const double T0 = 273.15;
    const double S = 110.4;
    return mu0 * std::pow(T / T0, 1.5) * (T0 + S) / (T + S);
}

// ============================================================================
double AnalysisComputeInputs::aircraftParam(const QString &symbol, bool *known) const
{
    for (int i = 0; i < aircraft.parameters.size(); ++i) {
        const AcParameter &p = aircraft.parameters[i];
        if (p.symbol == symbol && p.valueKnown) {
            if (known)
                *known = true;
            return p.value;
        }
    }
    if (known)
        *known = false;
    return 0.0;
}

// ============================================================================
// 气动：真实（大气/V/q/AR）+ 估算（L/D）+ MOCK（CL/CD/导数）
AnalysisDisciplineResult AeroComputer::compute(const AnalysisComputeInputs &in) const
{
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();

    bool anyReal = false;

    if (in.altitudeKnown) {
        const IsaPoint isa = isaAt(in.altitudeM);
        const QString atmNote = in.conditionNote.isEmpty()
                                    ? QString::fromUtf8("ISA 1976")
                                    : QString::fromUtf8("ISA 1976；%1").arg(in.conditionNote);
        r.items.append(makeItem(QStringLiteral("aero.T"), QString::fromUtf8("大气温度"),
                                 isa.T, QStringLiteral("K"), AnalysisFidelity::exact(), atmNote));
        r.items.append(makeItem(QStringLiteral("aero.p"), QString::fromUtf8("大气压强"),
                                 isa.p, QStringLiteral("Pa"), AnalysisFidelity::exact(),
                                 QString::fromUtf8("ISA 1976")));
        r.items.append(makeItem(QStringLiteral("aero.rho"), QString::fromUtf8("空气密度"),
                                 isa.rho, QString::fromUtf8("kg/m³"), AnalysisFidelity::exact(),
                                 QString::fromUtf8("ISA 1976")));
        r.items.append(makeItem(QStringLiteral("aero.a"), QString::fromUtf8("声速"),
                                 isa.a, QStringLiteral("m/s"), AnalysisFidelity::exact(),
                                 QString::fromUtf8("ISA 1976")));
        anyReal = true;

        if (in.machKnown) {
            const double V = in.mach * isa.a;
            const double q = 0.5 * isa.rho * V * V;
            r.items.append(makeItem(QStringLiteral("aero.V"), QString::fromUtf8("真空速"),
                                     V, QStringLiteral("m/s"), AnalysisFidelity::exact(),
                                     QString::fromUtf8("V = Ma · a")));
            r.items.append(makeItem(QStringLiteral("aero.q"), QString::fromUtf8("动压"),
                                     q, QStringLiteral("Pa"), AnalysisFidelity::exact(),
                                     QString::fromUtf8("q = ½ρV²")));

            // 雷诺数 Re = ρVL/μ（L 取平均气动弦 MAC）；μ 由 Sutherland 公式。
            bool macK = false;
            const double mac = in.aircraftParam(QStringLiteral("MAC"), &macK);
            if (macK && mac > 0.0) {
                const double mu = sutherlandMu(isa.T);
                const double re = isa.rho * V * mac / mu;
                r.items.append(makeItem(QStringLiteral("aero.Re"), QString::fromUtf8("雷诺数(基于MAC)"),
                                         re, QString(), AnalysisFidelity::exact(),
                                         QString::fromUtf8("Re = ρVL/μ，L=MAC，μ 用 Sutherland")));
            }
        }
    } else {
        r.items.append(mockItem(QStringLiteral("aero.rho"), QString::fromUtf8("空气密度"),
                                 0.3639, QString::fromUtf8("kg/m³"),
                                 QString::fromUtf8("缺工况高度，占位；请在分析集填写高度")));
    }

    // 展弦比 AR = b²/S（几何真实）
    bool sK = false, bK = false;
    const double S = in.aircraftParam(QStringLiteral("S_ref"), &sK);
    const double b = in.aircraftParam(QStringLiteral("b"), &bK);
    double ar = 0.0;
    bool arKnown = false;
    if (sK && bK && S > 0.0) {
        ar = (b * b) / S;
        arKnown = true;
        r.items.append(makeItem(QStringLiteral("aero.AR"), QString::fromUtf8("展弦比"),
                                 ar, QString(), AnalysisFidelity::exact(),
                                 QString::fromUtf8("AR = b²/S_ref（来自关联飞机修订）")));
        anyReal = true;
    } else {
        ar = 9.0;
        r.items.append(mockItem(QStringLiteral("aero.AR"), QString::fromUtf8("展弦比"),
                                 ar, QString(),
                                 QString::fromUtf8("缺 S_ref/b（未关联飞机修订或参数缺失），占位")));
    }

    // 巡航 L/D 估算：L/D_max = 0.5·√(π·AR·e/Cd0)。假设 Cd0、e。
    // TODO：改为从气动配置/极曲线求解，去除写死假设。
    const double ldMax = 0.5 * std::sqrt(M_PI * ar * kAssumedOswaldE / kAssumedCd0);
    r.items.append(makeItem(QStringLiteral("aero.LD"), QString::fromUtf8("巡航升阻比"),
                             ldMax, QString(),
                             arKnown ? AnalysisFidelity::estimate() : AnalysisFidelity::mock(),
                             QString::fromUtf8("假设 Cd0=%1, e=%2；TODO：接极曲线")
                                 .arg(kAssumedCd0).arg(kAssumedOswaldE)));

    // CL/CD/导数：当前 MOCK。TODO：接极曲线/涡格法。
    r.items.append(mockItem(QStringLiteral("aero.CL"), QString::fromUtf8("巡航升力系数"),
                             0.50, QString(), QString::fromUtf8("MOCK，需配平重量/涡格法")));
    r.items.append(mockItem(QStringLiteral("aero.CD"), QString::fromUtf8("巡航阻力系数"),
                             0.028, QString(), QString::fromUtf8("MOCK，需极曲线")));
    r.items.append(mockItem(QStringLiteral("aero.Cma"), QString::fromUtf8("纵向静稳定导数 Cmα"),
                             -0.8, QString::fromUtf8("1/rad"), QString::fromUtf8("MOCK，需气动导数")));

    r.status = anyReal ? QStringLiteral("partial") : QStringLiteral("mock");
    return r;
}

// ============================================================================
// 结构：全 MOCK。TODO：梁壳 FEM、载荷映射、模态、裕度（需几何网格/求解器）。
AnalysisDisciplineResult StructureComputer::compute(const AnalysisComputeInputs &in) const
{
    Q_UNUSED(in);
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();
    r.status = QStringLiteral("mock");
    const QString todo = QString::fromUtf8("MOCK，待接梁壳 FEM/载荷映射");
    r.items.append(mockItem(QStringLiteral("struct.sigmaMax"), QString::fromUtf8("最大应力"),
                             280.0, QStringLiteral("MPa"), todo));
    r.items.append(mockItem(QStringLiteral("struct.tipDefl"), QString::fromUtf8("翼尖位移"),
                             1.2, QStringLiteral("m"), todo));
    r.items.append(mockItem(QStringLiteral("struct.mode1"), QString::fromUtf8("一阶弯曲频率"),
                             3.5, QStringLiteral("Hz"), todo));
    r.items.append(mockItem(QStringLiteral("struct.marginMin"), QString::fromUtf8("最小强度裕度 MS"),
                             0.15, QString(), todo));
    return r;
}

// ============================================================================
// 重量与质量特性：全 MOCK。TODO：Class-II 统计估重 + 重量闭环 + CG/惯量。
AnalysisDisciplineResult MassComputer::compute(const AnalysisComputeInputs &in) const
{
    Q_UNUSED(in);
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();
    r.status = QStringLiteral("mock");
    const QString todo = QString::fromUtf8("MOCK，待接统计估重/重量闭环");
    r.items.append(mockItem(QStringLiteral("mass.MTOW"), QString::fromUtf8("最大起飞重量 MTOW"),
                             78000.0, QStringLiteral("kg"), todo));
    r.items.append(mockItem(QStringLiteral("mass.OEW"), QString::fromUtf8("使用空重 OEW"),
                             42000.0, QStringLiteral("kg"), todo));
    r.items.append(mockItem(QStringLiteral("mass.fuel"), QString::fromUtf8("任务燃油"),
                             18000.0, QStringLiteral("kg"), todo));
    r.items.append(mockItem(QStringLiteral("mass.cg"), QString::fromUtf8("重心位置"),
                             25.0, QStringLiteral("%MAC"), todo));
    r.items.append(mockItem(QStringLiteral("mass.Ixx"), QString::fromUtf8("滚转惯量 Ixx"),
                             1.8e6, QString::fromUtf8("kg·m²"), todo));
    return r;
}

// ============================================================================
// 推进与能源：安装后总推力=真实（来自配置）；SFC/流量/裕度仍 MOCK。
// TODO：接发动机图谱/安装修正/离设计点得到随高度马赫变化的可用推力与 SFC。
AnalysisDisciplineResult PropulsionComputer::compute(const AnalysisComputeInputs &in) const
{
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();

    bool anyReal = false;
    // 安装后总推力 = 发动机数 × 额定净推力 × (1 − 安装损失)。取自分析集配置。
    bool nK = false, tK = false, lK = false;
    const double n = configNum(in, QStringLiteral("propulsion"),
                               QString::fromUtf8("动力架构与模型"),
                               QString::fromUtf8("发动机数量"), &nK);
    const double rated = configNum(in, QStringLiteral("propulsion"),
                                   QString::fromUtf8("设计点与工作网格"),
                                   QString::fromUtf8("额定净推力"), &tK);
    const double lossPct = configNum(in, QStringLiteral("propulsion"),
                                     QString::fromUtf8("进排气与推进器"),
                                     QString::fromUtf8("安装损失"), &lK);
    if (nK && tK && n > 0.0 && rated > 0.0) {
        const double loss = lK ? lossPct / 100.0 : 0.0;
        const double total = n * rated * (1.0 - loss);
        r.items.append(makeItem(QStringLiteral("prop.thrustTotal"), QString::fromUtf8("安装后总推力"),
                                 total, QStringLiteral("kN"), AnalysisFidelity::estimate(),
                                 QString::fromUtf8("n×额定净推力×(1−安装损失)，来自分析集配置")));
        anyReal = true;
    }

    const QString todo = QString::fromUtf8("MOCK，待接发动机图谱/安装修正");
    r.items.append(mockItem(QStringLiteral("prop.sfc"), QString::fromUtf8("巡航耗油率 SFC"),
                             16.5, QString::fromUtf8("g/(kN·s)"), todo));
    r.items.append(mockItem(QStringLiteral("prop.massflow"), QString::fromUtf8("空气流量"),
                             120.0, QStringLiteral("kg/s"), todo));
    r.items.append(mockItem(QStringLiteral("prop.margin"), QString::fromUtf8("推力裕度"),
                             0.08, QString(), todo));

    r.status = anyReal ? QStringLiteral("partial") : QStringLiteral("mock");
    return r;
}

// ============================================================================
// 操稳与飞行动力学：全 MOCK。TODO：配平/线化导数（需 VLM）。
AnalysisDisciplineResult DynamicsComputer::compute(const AnalysisComputeInputs &in) const
{
    Q_UNUSED(in);
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();
    r.status = QStringLiteral("mock");
    const QString todo = QString::fromUtf8("MOCK，待接配平/线化导数");
    r.items.append(mockItem(QStringLiteral("dyn.staticMargin"), QString::fromUtf8("纵向静稳定裕度"),
                             12.0, QStringLiteral("%MAC"), todo));
    r.items.append(mockItem(QStringLiteral("dyn.spFreq"), QString::fromUtf8("短周期频率"),
                             2.4, QStringLiteral("rad/s"), todo));
    r.items.append(mockItem(QStringLiteral("dyn.dutchRoll"), QString::fromUtf8("荷兰滚阻尼比"),
                             0.12, QString(), todo));
    return r;
}

// ============================================================================
// 任务与性能：场长余量=真实（配置相减）；航程/燃油/实际场长仍 MOCK。
// TODO：接 Breguet 航程与真实场长模型（需 SFC/重量/L-D 闭环、推重比-翼载）。
AnalysisDisciplineResult MissionComputer::compute(const AnalysisComputeInputs &in) const
{
    AnalysisDisciplineResult r;
    r.domainId = domainId();
    r.domainName = domainName();

    bool anyReal = false;
    bool rwK = false, toK = false, ldK = false;
    const double runway = configNum(in, QStringLiteral("mission"),
                                    QString::fromUtf8("机场与性能约束"),
                                    QString::fromUtf8("跑道长度"), &rwK);
    const double toLimit = configNum(in, QStringLiteral("mission"),
                                     QString::fromUtf8("机场与性能约束"),
                                     QString::fromUtf8("起飞场长限制"), &toK);
    const double ldLimit = configNum(in, QStringLiteral("mission"),
                                     QString::fromUtf8("机场与性能约束"),
                                     QString::fromUtf8("着陆场长限制"), &ldK);
    if (rwK && toK) {
        r.items.append(makeItem(QStringLiteral("mission.toMargin"), QString::fromUtf8("起飞场长余量"),
                                 runway - toLimit, QStringLiteral("m"), AnalysisFidelity::exact(),
                                 QString::fromUtf8("可用跑道 − 起飞场长限制（配置）")));
        anyReal = true;
    }
    if (rwK && ldK) {
        r.items.append(makeItem(QStringLiteral("mission.ldMargin"), QString::fromUtf8("着陆场长余量"),
                                 runway - ldLimit, QStringLiteral("m"), AnalysisFidelity::exact(),
                                 QString::fromUtf8("可用跑道 − 着陆场长限制（配置）")));
        anyReal = true;
    }

    const QString todo = QString::fromUtf8("MOCK，待接 Breguet/场长（需 SFC/重量）");
    r.items.append(mockItem(QStringLiteral("mission.range"), QString::fromUtf8("设计航程"),
                             5200.0, QStringLiteral("km"), todo));
    r.items.append(mockItem(QStringLiteral("mission.fuel"), QString::fromUtf8("任务燃油"),
                             18000.0, QStringLiteral("kg"), todo));
    r.items.append(mockItem(QStringLiteral("mission.toField"), QString::fromUtf8("起飞场长(实际)"),
                             2300.0, QStringLiteral("m"), todo));
    r.items.append(mockItem(QStringLiteral("mission.ldField"), QString::fromUtf8("着陆场长(实际)"),
                             1600.0, QStringLiteral("m"), todo));

    r.status = anyReal ? QStringLiteral("partial") : QStringLiteral("mock");
    return r;
}

// ============================================================================
AnalysisComputeService::AnalysisComputeService(AircraftStore *aircraftStore, AnalysisStore *analysisStore,
                                               SrdStore *srdStore)
    : m_aircraftStore(aircraftStore)
    , m_analysisStore(analysisStore)
    , m_srdStore(srdStore)
{
}

AnalysisComputeInputs AnalysisComputeService::resolveInputs(const AnalysisDocument &analysis,
                                                            const QHash<QString, double> &paramOverrides) const
{
    AnalysisComputeInputs in;
    in.analysis = analysis;

    // 工况优先级：关联 SRD 的设计工况（飞行包线点）> 分析集手填高度/马赫。
    // 对齐 PDF「设计工况/包线来自设计需求 → 学科分析」。
    if (!analysis.sourceSrd.isEmpty() && !analysis.sourceCondition.isEmpty() && m_srdStore) {
        SrdDocument srd;
        QString detail;
        if (m_srdStore->loadBaseline(analysis.sourceSrd, &srd, &detail)) {
            bool idxOk = false;
            const int idx = analysis.sourceCondition.toInt(&idxOk);
            if (idxOk && idx >= 0 && idx < srd.envelopePoints.size()) {
                const SrdEnvelopePoint &ep = srd.envelopePoints[idx];
                in.altitudeM = ep.altitudeKm * 1000.0;
                in.altitudeKnown = true;
                in.mach = ep.mach;
                in.machKnown = true;
                in.conditionFromSrd = true;
                in.conditionNote = QString::fromUtf8("工况来自 %1 包线点#%2（%3）")
                                       .arg(analysis.sourceSrd)
                                       .arg(idx + 1)
                                       .arg(srd.environment.atmosphereModel.isEmpty()
                                                ? QString::fromUtf8("ISA")
                                                : srd.environment.atmosphereModel);
            }
        }
    }

    // 回退：未关联 SRD 工况时，用分析集手填的高度/马赫（气动 · 飞行与流动条件）。
    if (!in.conditionFromSrd) {
        const QString altKey = analysisFieldKey(QStringLiteral("aero"),
                                                QString::fromUtf8("飞行与流动条件"),
                                                QString::fromUtf8("高度"));
        const QString machKey = analysisFieldKey(QStringLiteral("aero"),
                                                 QString::fromUtf8("飞行与流动条件"),
                                                 QString::fromUtf8("马赫数"));
        if (analysis.values.contains(altKey)) {
            bool ok = false;
            const double v = analysis.values.value(altKey).toDouble(&ok);
            if (ok) { in.altitudeM = v; in.altitudeKnown = true; }
        }
        if (analysis.values.contains(machKey)) {
            bool ok = false;
            const double v = analysis.values.value(machKey).toDouble(&ok);
            if (ok) { in.mach = v; in.machKnown = true; }
        }
        in.conditionNote = QString::fromUtf8("工况来自分析集手填（未关联 SRD 设计工况）");
    }

    // 飞机方案：由 sourceRevision（R00N）解析到 aircraft_vN 基线。
    if (!analysis.sourceRevision.isEmpty() && m_aircraftStore) {
        QRegExp re(QStringLiteral("^R0*([0-9]+)$"));
        if (re.exactMatch(analysis.sourceRevision)) {
            const int version = re.cap(1).toInt();
            const QString baselineId = QStringLiteral("aircraft_v%1").arg(version);
            AircraftDocument doc;
            QString detail;
            if (m_aircraftStore->loadBaseline(baselineId, &doc, &detail)) {
                in.aircraft = doc;
                in.aircraftResolved = true;
            }
        }
    }

    // 设计空间探索：按 symbol 覆盖飞机几何参数（S_ref/b 等）。
    if (!paramOverrides.isEmpty()) {
        for (int i = 0; i < in.aircraft.parameters.size(); ++i) {
            AcParameter &p = in.aircraft.parameters[i];
            if (paramOverrides.contains(p.symbol)) {
                p.value = paramOverrides.value(p.symbol);
                p.valueKnown = true;
            }
        }
    }
    return in;
}

AnalysisRunResult AnalysisComputeService::run(const AnalysisDocument &analysis, QString *errorMessage) const
{
    return run(analysis, QHash<QString, double>(), errorMessage);
}

AnalysisRunResult AnalysisComputeService::run(const AnalysisDocument &analysis,
                                              const QHash<QString, double> &paramOverrides,
                                              QString *errorMessage) const
{
    Q_UNUSED(errorMessage);
    const AnalysisComputeInputs in = resolveInputs(analysis, paramOverrides);

    AnalysisRunResult result;
    result.runAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    result.sourceRevision = analysis.sourceRevision;
    result.sourceSrd = analysis.sourceSrd;
    result.sourceCase = analysis.sourceCase;
    result.sourceCondition = analysis.sourceCondition;
    result.aircraftResolved = in.aircraftResolved;

    // 逐学科计算（计算器无状态，就地构造）。
    AeroComputer aero;
    StructureComputer structure;
    MassComputer mass;
    PropulsionComputer propulsion;
    DynamicsComputer dynamics;
    MissionComputer mission;
    const IDisciplineComputer *computers[] = {
        &aero, &structure, &mass, &propulsion, &dynamics, &mission
    };
    for (int i = 0; i < 6; ++i)
        result.disciplines.append(computers[i]->compute(in));

    return result;
}

// 文件名安全化：非字母数字与 _.- 一律替换为 _。
static QString sanitizeName(const QString &s)
{
    QString t = s;
    t.replace(QRegExp(QStringLiteral("[^A-Za-z0-9_.-]")), QStringLiteral("_"));
    return t;
}

QString AnalysisComputeService::resultPath(const AnalysisRunResult &result) const
{
    if (!m_analysisStore)
        return QString();
    const QString dir = m_analysisStore->rootDir() + QLatin1String("/results");

    QString name;
    if (!result.sourceCase.isEmpty()) {
        // A：贴 PDF —— 单设计点/用例一份：case_N.evaluation.json
        name = result.sourceCase + QLatin1String(".evaluation.json");
    } else {
        // 无用例：用「修订+需求+工况」组合键，确保不同关联互不覆盖。
        QStringList parts;
        parts << (result.sourceRevision.isEmpty() ? QStringLiteral("noRev") : result.sourceRevision);
        if (!result.sourceSrd.isEmpty())
            parts << result.sourceSrd;
        if (!result.sourceCondition.isEmpty())
            parts << (QStringLiteral("cond") + result.sourceCondition);
        name = QStringLiteral("analysis_") + parts.join(QLatin1Char('_')) + QLatin1String(".json");
    }
    return dir + QLatin1Char('/') + sanitizeName(name);
}

bool AnalysisComputeService::saveResult(const AnalysisRunResult &result, QString *outPath,
                                        QString *errorMessage) const
{
    const QString path = resultPath(result);
    if (path.isEmpty()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QJsonObject root;
    root.insert(QStringLiteral("schema"), QStringLiteral("amdo.analysis-result.v1"));
    root.insert(QStringLiteral("runAt"), result.runAt);
    root.insert(QStringLiteral("sourceRevision"), result.sourceRevision);
    root.insert(QStringLiteral("sourceSrd"), result.sourceSrd);
    root.insert(QStringLiteral("sourceCase"), result.sourceCase);
    root.insert(QStringLiteral("sourceCondition"), result.sourceCondition);
    root.insert(QStringLiteral("aircraftResolved"), result.aircraftResolved);

    QJsonArray domains;
    for (int i = 0; i < result.disciplines.size(); ++i) {
        const AnalysisDisciplineResult &d = result.disciplines[i];
        QJsonObject dObj;
        dObj.insert(QStringLiteral("domainId"), d.domainId);
        dObj.insert(QStringLiteral("domainName"), d.domainName);
        dObj.insert(QStringLiteral("status"), d.status);
        QJsonArray items;
        for (int j = 0; j < d.items.size(); ++j) {
            const AnalysisResultItem &it = d.items[j];
            QJsonObject iObj;
            iObj.insert(QStringLiteral("key"), it.key);
            iObj.insert(QStringLiteral("label"), it.label);
            if (it.valueKnown)
                iObj.insert(QStringLiteral("value"), it.value);
            iObj.insert(QStringLiteral("unit"), it.unit);
            iObj.insert(QStringLiteral("fidelity"), it.fidelity);
            iObj.insert(QStringLiteral("note"), it.note);
            items.append(iObj);
        }
        dObj.insert(QStringLiteral("items"), items);
        domains.append(dObj);
    }
    root.insert(QStringLiteral("disciplines"), domains);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无法写入结果文件：%1").arg(file.errorString());
        return false;
    }
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("结果写入不完整：%1").arg(file.errorString());
        return false;
    }
    if (outPath)
        *outPath = path;
    return true;
}
