#include "service/evaluationservice.h"

#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "model/srdtypes.h"
#include "service/analysiscomputeservice.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>

EvaluationService::EvaluationService(AnalysisComputeService *compute, SrdStore *srdStore,
                                     AnalysisStore *analysisStore)
    : m_compute(compute)
    , m_srdStore(srdStore)
    , m_analysisStore(analysisStore)
{
}

// SRD 指标的 analysisResponseId → 学科分析结果键 的映射。
// TODO：后期统一命名或做成配置；未在表中的指标 → 该需求标“待分析”。
static QString mapResponseToAnalysisKey(const QString &responseId)
{
    static const QHash<QString, QString> kMap = {
        { QStringLiteral("aero.cruise_ld"),                QStringLiteral("aero.LD") },
        { QStringLiteral("weight.mtow_kg"),                QStringLiteral("mass.MTOW") },
        { QStringLiteral("mission.range_km"),              QStringLiteral("mission.range") },
        { QStringLiteral("mission.fuel_kg"),               QStringLiteral("mission.fuel") },
        { QStringLiteral("mission.takeoff_field_m"),       QStringLiteral("mission.toField") },
        { QStringLiteral("mission.landing_field_m"),       QStringLiteral("mission.ldField") },
        { QStringLiteral("dyn.static_margin_percent_mac"), QStringLiteral("dyn.staticMargin") },
        { QStringLiteral("struct.buckling_margin"),        QStringLiteral("struct.marginMin") },
    };
    return kMap.value(responseId);
}

SchemeEvaluationResult EvaluationService::evaluate(const AnalysisDocument &analysis,
                                                   const QString &objectId,
                                                   double tolerancePercent,
                                                   QString *errorMessage) const
{
    SchemeEvaluationResult res;
    res.objectId = objectId;
    res.evaluatedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    res.sourceRevision = analysis.sourceRevision;
    res.sourceSrd = analysis.sourceSrd;
    res.sourceCase = analysis.sourceCase;
    res.sourceCondition = analysis.sourceCondition;

    // 1) 跑学科计算，得到方案值（键 → 结果项）。
    QHash<QString, AnalysisResultItem> byKey;
    if (m_compute) {
        const AnalysisRunResult run = m_compute->run(analysis);
        res.aircraftResolved = run.aircraftResolved;
        for (int i = 0; i < run.disciplines.size(); ++i) {
            const AnalysisDisciplineResult &d = run.disciplines[i];
            for (int j = 0; j < d.items.size(); ++j)
                byKey.insert(d.items[j].key, d.items[j]);
        }
    }

    // 2) 读关联 SRD 基线的需求。
    if (analysis.sourceSrd.isEmpty() || !m_srdStore) {
        res.srdResolved = false;
        if (errorMessage)
            *errorMessage = QString::fromUtf8("未关联设计需求基线，无法评价");
        return res;
    }
    SrdDocument srd;
    QString detail;
    if (!m_srdStore->loadBaseline(analysis.sourceSrd, &srd, &detail)) {
        res.srdResolved = false;
        if (errorMessage)
            *errorMessage = QString::fromUtf8("读取设计需求基线失败：%1").arg(detail);
        return res;
    }
    res.srdResolved = true;

    const double tol = tolerancePercent / 100.0;

    // 3) 逐条需求判定。
    for (int i = 0; i < srd.requirements.size(); ++i) {
        const SrdRequirement &rq = srd.requirements[i];
        EvaluationItem it;
        it.requirementId = rq.id;
        it.metricName = rq.metricName.isEmpty() ? rq.metricId : rq.metricName;
        it.domain = rq.domain;
        it.relation = rq.relation;
        it.boundValue = rq.boundValue;
        it.boundKnown = rq.boundKnown;
        it.unit = rq.unit;
        it.grade = rq.grade;

        const bool mandatory = rq.grade.contains(QString::fromUtf8("强制"));
        if (mandatory)
            ++res.mandatoryTotal;

        const QString key = mapResponseToAnalysisKey(rq.analysisResponseId);
        const bool hasValue = !key.isEmpty() && byKey.contains(key)
                              && byKey.value(key).valueKnown;

        if (!hasValue || !rq.boundKnown) {
            it.status = QString::fromUtf8("待分析");
            ++res.pending;
            if (mandatory)
                ++res.mandatoryPending;
            res.items.append(it);
            ++res.total;
            continue;
        }

        const AnalysisResultItem &av = byKey.value(key);
        it.actualValue = av.value;
        it.actualKnown = true;
        it.actualFidelity = av.fidelity;

        const double v = av.value;
        const double b = rq.boundValue;
        const double denom = qFuzzyIsNull(b) ? 1.0 : qAbs(b);
        bool satisfied = false;
        if (rq.relation.contains(QLatin1Char('<')) || rq.relation.contains(QString::fromUtf8("≤"))) {
            satisfied = v <= b * (1.0 + tol);
            it.marginPercent = (b - v) / denom * 100.0;   // 正=有余量
        } else if (rq.relation.contains(QLatin1Char('>')) || rq.relation.contains(QString::fromUtf8("≥"))) {
            satisfied = v >= b * (1.0 - tol);
            it.marginPercent = (v - b) / denom * 100.0;
        } else { // 等式
            satisfied = qAbs(v - b) <= qAbs(b) * tol;
            it.marginPercent = -qAbs(v - b) / denom * 100.0;
        }
        it.marginKnown = true;

        const bool critical = qAbs(it.marginPercent) <= tolerancePercent;
        if (!satisfied) {
            it.status = QString::fromUtf8("违反");
            ++res.violated;
            if (mandatory)
                ++res.mandatoryViolated;
        } else if (critical) {
            it.status = QString::fromUtf8("临界");
            ++res.critical;
        } else {
            it.status = QString::fromUtf8("满足");
            ++res.satisfied;
        }

        res.items.append(it);
        ++res.total;
    }

    // 4) 可行性：强制项全部满足且无待分析强制项。
    if (res.mandatoryViolated > 0)
        res.feasibility = QString::fromUtf8("违反");
    else if (res.mandatoryPending > 0)
        res.feasibility = QString::fromUtf8("待定");
    else
        res.feasibility = QString::fromUtf8("通过");

    // 5) 综合评分（满足率 %）：(满足+临界) / 有值项。仅作可比较标量，非多目标权衡。
    // TODO：接入归一化加权 / TOPSIS 等多目标权衡（方案比较与权衡）。
    const int evaluated = res.total - res.pending;
    res.scoreKnown = evaluated > 0;
    res.score = res.scoreKnown ? (100.0 * (res.satisfied + res.critical) / evaluated) : 0.0;

    return res;
}

QString EvaluationService::evaluationsDir() const
{
    if (!m_analysisStore)
        return QString();
    const QString dir = m_analysisStore->rootDir() + QLatin1String("/evaluations");
    QDir().mkpath(dir);
    return dir;
}

static QString sanitizeName(const QString &s)
{
    QString t = s;
    t.replace(QRegExp(QStringLiteral("[^A-Za-z0-9_.-]")), QStringLiteral("_"));
    return t;
}

bool EvaluationService::saveResult(const SchemeEvaluationResult &result, QString *outPath,
                                   QString *errorMessage) const
{
    const QString dir = evaluationsDir();
    if (dir.isEmpty()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    const QString id = result.objectId.isEmpty() ? QStringLiteral("draft") : result.objectId;
    const QString path = dir + QLatin1Char('/') + sanitizeName(id) + QLatin1String(".evaluation.json");

    QJsonObject root;
    root.insert(QStringLiteral("schema"), QStringLiteral("amdo.scheme-evaluation.v1"));
    root.insert(QStringLiteral("objectId"), result.objectId);
    root.insert(QStringLiteral("evaluatedAt"), result.evaluatedAt);
    root.insert(QStringLiteral("sourceRevision"), result.sourceRevision);
    root.insert(QStringLiteral("sourceSrd"), result.sourceSrd);
    root.insert(QStringLiteral("sourceCase"), result.sourceCase);
    root.insert(QStringLiteral("sourceCondition"), result.sourceCondition);
    root.insert(QStringLiteral("feasibility"), result.feasibility);
    if (result.scoreKnown)
        root.insert(QStringLiteral("score"), result.score);
    root.insert(QStringLiteral("total"), result.total);
    root.insert(QStringLiteral("satisfied"), result.satisfied);
    root.insert(QStringLiteral("violated"), result.violated);
    root.insert(QStringLiteral("critical"), result.critical);
    root.insert(QStringLiteral("pending"), result.pending);

    QJsonArray items;
    for (int i = 0; i < result.items.size(); ++i) {
        const EvaluationItem &it = result.items[i];
        QJsonObject o;
        o.insert(QStringLiteral("requirementId"), it.requirementId);
        o.insert(QStringLiteral("metricName"), it.metricName);
        o.insert(QStringLiteral("relation"), it.relation);
        if (it.boundKnown)
            o.insert(QStringLiteral("boundValue"), it.boundValue);
        o.insert(QStringLiteral("unit"), it.unit);
        o.insert(QStringLiteral("grade"), it.grade);
        if (it.actualKnown)
            o.insert(QStringLiteral("actualValue"), it.actualValue);
        o.insert(QStringLiteral("actualFidelity"), it.actualFidelity);
        if (it.marginKnown)
            o.insert(QStringLiteral("marginPercent"), it.marginPercent);
        o.insert(QStringLiteral("status"), it.status);
        items.append(o);
    }
    root.insert(QStringLiteral("items"), items);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无法写入评价结果：%1").arg(file.errorString());
        return false;
    }
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("评价结果写入不完整：%1").arg(file.errorString());
        return false;
    }
    if (outPath)
        *outPath = path;
    return true;
}

bool EvaluationService::listResults(QVector<SchemeEvaluationResult> *out, QString *errorMessage) const
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("内部错误：输出参数为空");
        return false;
    }
    out->clear();
    const QString dir = evaluationsDir();
    if (dir.isEmpty())
        return true;

    QDir d(dir);
    const QStringList files = d.entryList(QStringList() << QStringLiteral("*.evaluation.json"),
                                          QDir::Files, QDir::Name);
    for (int i = 0; i < files.size(); ++i) {
        QFile file(d.absoluteFilePath(files[i]));
        if (!file.open(QIODevice::ReadOnly))
            continue;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject())
            continue;
        const QJsonObject o = doc.object();
        SchemeEvaluationResult r;
        r.objectId = o.value(QStringLiteral("objectId")).toString();
        r.evaluatedAt = o.value(QStringLiteral("evaluatedAt")).toString();
        r.sourceRevision = o.value(QStringLiteral("sourceRevision")).toString();
        r.sourceSrd = o.value(QStringLiteral("sourceSrd")).toString();
        r.sourceCondition = o.value(QStringLiteral("sourceCondition")).toString();
        r.feasibility = o.value(QStringLiteral("feasibility")).toString();
        r.scoreKnown = o.contains(QStringLiteral("score"));
        r.score = o.value(QStringLiteral("score")).toDouble();
        r.total = o.value(QStringLiteral("total")).toInt();
        r.satisfied = o.value(QStringLiteral("satisfied")).toInt();
        r.violated = o.value(QStringLiteral("violated")).toInt();
        r.critical = o.value(QStringLiteral("critical")).toInt();
        r.pending = o.value(QStringLiteral("pending")).toInt();
        r.srdResolved = true;
        out->append(r);
    }
    return true;
}
