#include "service/workflowservice.h"

#include "model/aircraftstore.h"
#include "model/aircrafttypes.h"
#include "model/analysisresult.h"
#include "model/analysisstore.h"
#include "model/analysistypes.h"
#include "model/evaluationresult.h"
#include "model/studytypes.h"
#include "service/aircraftcpacsservice.h"
#include "service/aircraftdocumentservice.h"
#include "service/analysiscomputeservice.h"
#include "service/evaluationservice.h"
#include "service/studyservice.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QStandardPaths>

namespace {
QString nowClock() { return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")); }
QString kPass() { return QString::fromUtf8("通过"); }

// 工作流默认设计变量（与「设计空间探索」一致：S_ref、b）。
StudyDefinition defaultStudyDef(const QString &baseObjectId)
{
    StudyDefinition def;
    def.baseObjectId = baseObjectId;
    def.sampling = QStringLiteral("grid");
    def.tolerancePercent = 0.5;

    StudyVariable s;
    s.symbol = QStringLiteral("S_ref");
    s.name = QString::fromUtf8("机翼面积 S");
    s.unit = QString::fromUtf8("m²");
    s.minValue = 110.0;
    s.maxValue = 140.0;
    s.steps = 4;
    s.enabled = true;

    StudyVariable b;
    b.symbol = QStringLiteral("b");
    b.name = QString::fromUtf8("机翼展长 b");
    b.unit = QStringLiteral("m");
    b.minValue = 30.0;
    b.maxValue = 37.0;
    b.steps = 4;
    b.enabled = true;

    def.variables << s << b;
    return def;
}
} // namespace

WorkflowService::WorkflowService(AnalysisStore *analysisStore, AnalysisComputeService *compute,
                                 EvaluationService *evaluation, StudyService *study,
                                 AircraftStore *aircraftStore, AircraftDocumentService *aircraftDoc)
    : m_analysisStore(analysisStore)
    , m_compute(compute)
    , m_evaluation(evaluation)
    , m_study(study)
    , m_aircraftStore(aircraftStore)
    , m_aircraftDoc(aircraftDoc)
{
}

QString WorkflowService::workflowDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath();
    dir += QLatin1String("/workflow");
    QDir().mkpath(dir);
    return dir;
}

QVector<WorkflowTemplate> WorkflowService::templates()
{
    QVector<WorkflowTemplate> list;

    WorkflowTemplate explore;
    explore.id = QStringLiteral("explore");
    explore.name = QString::fromUtf8("设计空间探索与方案比选");
    explore.description = QString::fromUtf8(
        "载入基准 → 设计空间探索(网格采样) → 结果汇总/比选 → (可选)提升最优为飞机方案版本。");
    explore.supportsPromote = true;
    explore.nodes = {
        {QStringLiteral("N1"), QString::fromUtf8("载入基准分析集"), QString::fromUtf8("数据"),
         QString::fromUtf8("学科分析(AnalysisStore)"), QString::fromUtf8("分析集文档"),
         WorkflowFidelity::real(), false},
        {QStringLiteral("N2"), QString::fromUtf8("设计空间探索"), QString::fromUtf8("计算"),
         QString::fromUtf8("方案优化(StudyService)"), QStringLiteral("study_result.json"),
         WorkflowFidelity::real(), false},
        {QStringLiteral("N3"), QString::fromUtf8("结果汇总/方案比选"), QString::fromUtf8("汇总"),
         QString::fromUtf8("方案决策(评价/比选)"), QString::fromUtf8("最优点摘要"),
         WorkflowFidelity::real(), false},
        {QStringLiteral("N4"), QString::fromUtf8("提升最优为飞机方案版本"), QString::fromUtf8("更新"),
         QString::fromUtf8("飞机方案定义(发布基线+CPACS)"), QString::fromUtf8("aircraft_vN + CPACS"),
         WorkflowFidelity::real(), true},
    };
    list << explore;

    WorkflowTemplate single;
    single.id = QStringLiteral("single");
    single.name = QString::fromUtf8("单方案分析与评价");
    single.description = QString::fromUtf8(
        "载入基准 → 学科分析计算(六学科) → 单方案评价(⊗ SRD 需求)。气动真实、其余学科为 MOCK。");
    single.supportsPromote = false;
    single.nodes = {
        {QStringLiteral("N1"), QString::fromUtf8("载入基准分析集"), QString::fromUtf8("数据"),
         QString::fromUtf8("学科分析(AnalysisStore)"), QString::fromUtf8("分析集文档"),
         WorkflowFidelity::real(), false},
        {QStringLiteral("N2"), QString::fromUtf8("学科分析计算"), QString::fromUtf8("计算"),
         QString::fromUtf8("学科运行计算(AnalysisComputeService)"), QString::fromUtf8("analysis 结果 JSON"),
         WorkflowFidelity::partial(), false},
        {QStringLiteral("N3"), QString::fromUtf8("单方案评价"), QString::fromUtf8("评价"),
         QString::fromUtf8("方案决策(EvaluationService)"), QString::fromUtf8("<对象>.evaluation.json"),
         WorkflowFidelity::real(), false},
    };
    list << single;

    return list;
}

WorkflowTemplate WorkflowService::templateById(const QString &id)
{
    const QVector<WorkflowTemplate> all = templates();
    for (int i = 0; i < all.size(); ++i)
        if (all[i].id == id)
            return all[i];
    return all.isEmpty() ? WorkflowTemplate() : all.first();
}

WorkflowRunResult WorkflowService::run(const QString &templateId, const QString &baseObjectId,
                                       bool promoteBest, int retryLimit, QString *errorMessage)
{
    WorkflowRunResult r;
    r.runAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    r.runId = QStringLiteral("run_") + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const WorkflowTemplate tpl = templateById(templateId);
    r.templateId = tpl.id;
    r.templateName = tpl.name;
    r.baseObjectId = baseObjectId;
    r.retryLimit = qMax(0, retryLimit);
    r.resultDir = workflowDir();

    for (int i = 0; i < tpl.nodes.size(); ++i) {
        const WorkflowNodeSpec &spec = tpl.nodes[i];
        WorkflowNodeResult n;
        n.id = spec.id;
        n.name = spec.name;
        n.type = spec.type;
        n.fidelity = spec.fidelity;
        n.status = QString::fromUtf8("待运行");
        r.nodes.append(n);
    }
    if (r.nodes.isEmpty()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无效的工作流模板");
        return r;
    }

    auto logLine = [&](const QString &s) { r.log.append(nowClock() + QLatin1Char(' ') + s); };

    QElapsedTimer timer;
    auto step = [&](int idx, const std::function<bool(WorkflowNodeResult &)> &body) -> bool {
        WorkflowNodeResult &n = r.nodes[idx];
        n.status = QString::fromUtf8("运行中");
        bool ok = false;
        timer.restart();
        const int maxAttempts = r.retryLimit + 1;
        for (int a = 1; a <= maxAttempts; ++a) {
            n.attempts = a;
            ok = body(n);
            if (ok)
                break;
            if (a < maxAttempts)
                logLine(QString::fromUtf8("节点 %1 第 %2 次失败，重试…").arg(n.name).arg(a));
        }
        n.elapsedMs = timer.elapsed();
        n.status = ok ? QString::fromUtf8("完成") : QString::fromUtf8("失败");
        logLine(QString::fromUtf8("%1 %2 · %3ms%4")
                    .arg(n.name, n.status)
                    .arg(n.elapsedMs)
                    .arg(n.detail.isEmpty() ? QString() : QString::fromUtf8(" · ") + n.detail));
        return ok;
    };
    auto skip = [&](int idx, const QString &reason) {
        WorkflowNodeResult &n = r.nodes[idx];
        n.status = QString::fromUtf8("跳过");
        n.detail = reason;
        logLine(QString::fromUtf8("%1 跳过 · %2").arg(n.name, reason));
    };

    // ---- N1 载入基准分析集（所有模板共享）----
    AnalysisDocument base;
    const bool baseOk = step(0, [&](WorkflowNodeResult &n) -> bool {
        QString d;
        const bool okl = (baseObjectId.isEmpty() || baseObjectId == QLatin1String("draft"))
                             ? (m_analysisStore && m_analysisStore->loadDraft(&base, &d))
                             : (m_analysisStore && m_analysisStore->loadBaseline(baseObjectId, &base, &d));
        if (!okl || base.id.isEmpty()) {
            n.detail = d.isEmpty() ? QString::fromUtf8("无分析集") : d;
            return false;
        }
        n.detail = QString::fromUtf8("基准=%1 · 飞机=%2 · 需求=%3")
                       .arg(baseObjectId,
                            base.sourceRevision.isEmpty() ? QString::fromUtf8("未关联") : base.sourceRevision,
                            base.sourceSrd.isEmpty() ? QString::fromUtf8("未关联") : base.sourceSrd);
        return true;
    });
    if (!baseOk) {
        for (int i = 1; i < r.nodes.size(); ++i)
            skip(i, QString::fromUtf8("上游失败"));
        r.summary << WorkflowSummaryItem{QString::fromUtf8("状态"), QString::fromUtf8("失败"), QString()};
        if (errorMessage)
            *errorMessage = QString::fromUtf8("载入基准分析集失败");
        writeRunRecord(r);
        return r;
    }

    if (tpl.id == QLatin1String("single")) {
        // ---- N2 学科分析计算 ----
        AnalysisRunResult arr;
        const bool okAnalyze = step(1, [&](WorkflowNodeResult &n) -> bool {
            QString e;
            arr = m_compute ? m_compute->run(base, &e) : AnalysisRunResult();
            if (arr.disciplines.isEmpty()) {
                n.detail = e.isEmpty() ? QString::fromUtf8("学科计算无结果") : e;
                return false;
            }
            QString sp;
            if (m_compute)
                m_compute->saveResult(arr, &sp, nullptr);
            n.artifact = sp;
            n.detail = QString::fromUtf8("%1 个学科 · 气动真实/其余 MOCK").arg(arr.disciplines.size());
            return true;
        });

        SchemeEvaluationResult eval;
        if (!okAnalyze) {
            skip(2, QString::fromUtf8("上游失败"));
        } else {
            step(2, [&](WorkflowNodeResult &n) -> bool {
                QString e;
                eval = m_evaluation
                           ? m_evaluation->evaluateWith(arr, base, baseObjectId, 0.5, &e)
                           : SchemeEvaluationResult();
                if (!m_evaluation) {
                    n.detail = QString::fromUtf8("评价服务未初始化");
                    return false;
                }
                QString sp;
                m_evaluation->saveResult(eval, &sp, nullptr);
                n.artifact = sp;
                if (eval.total == 0) {
                    r.headline = QString::fromUtf8("未关联设计需求或无可评价指标");
                    n.detail = r.headline;
                    return true;
                }
                r.headline = QString::fromUtf8("综合评分 %1% · 可行性 %2 · 满足 %3/违反 %4/临界 %5/待分析 %6")
                                 .arg(eval.scoreKnown ? QString::number(eval.score, 'f', 1) : QString::fromUtf8("—"))
                                 .arg(eval.feasibility)
                                 .arg(eval.satisfied)
                                 .arg(eval.violated)
                                 .arg(eval.critical)
                                 .arg(eval.pending);
                n.detail = r.headline;
                return true;
            });
        }

        r.summary << WorkflowSummaryItem{QString::fromUtf8("评价指标"), QString::number(eval.total), QString::fromUtf8("项")}
                   << WorkflowSummaryItem{QString::fromUtf8("满足"), QString::number(eval.satisfied), QString::fromUtf8("项")}
                   << WorkflowSummaryItem{QString::fromUtf8("综合评分"),
                                          eval.scoreKnown ? QString::number(eval.score, 'f', 1) : QString::fromUtf8("—"),
                                          eval.scoreKnown ? QStringLiteral("%") : QString()}
                   << WorkflowSummaryItem{QString::fromUtf8("可行性"),
                                          eval.feasibility.isEmpty() ? QString::fromUtf8("—") : eval.feasibility, QString()};
        r.ok = okAnalyze;
    } else {
        // ---- explore ----
        StudyResult study;
        int feasible = 0;
        const StudyDefinition def = defaultStudyDef(baseObjectId);
        const bool okExplore = step(1, [&](WorkflowNodeResult &n) -> bool {
            QString e;
            study = m_study ? m_study->run(base, def, &e) : StudyResult();
            if (study.points.isEmpty()) {
                n.detail = e.isEmpty() ? QString::fromUtf8("未生成设计点") : e;
                return false;
            }
            QString sp;
            if (m_study)
                m_study->saveResult(study, &sp, nullptr);
            n.artifact = sp;
            feasible = 0;
            for (int i = 0; i < study.points.size(); ++i)
                if (study.points[i].feasibility == kPass())
                    ++feasible;
            n.detail = QString::fromUtf8("采样 %1 点 · 可行 %2 点").arg(study.points.size()).arg(feasible);
            return true;
        });

        if (!okExplore) {
            skip(2, QString::fromUtf8("上游失败"));
            skip(3, QString::fromUtf8("上游失败"));
        } else {
            // N3 结果汇总/方案比选
            step(2, [&](WorkflowNodeResult &n) -> bool {
                if (study.bestIndex >= 0 && study.bestIndex < study.points.size()) {
                    const StudyPointResult &bp = study.points[study.bestIndex];
                    QStringList parts;
                    for (int i = 0; i < study.variables.size(); ++i) {
                        const QString sym = study.variables[i].symbol;
                        parts << QString::fromUtf8("%1=%2").arg(sym).arg(bp.variables.value(sym), 0, 'g', 6);
                    }
                    r.headline = QString::fromUtf8("点#%1 · %2 · L/D=%3 · 评分=%4% · %5")
                                     .arg(bp.index)
                                     .arg(parts.join(QLatin1Char(' ')))
                                     .arg(bp.ldKnown ? QString::number(bp.ld, 'g', 4) : QString::fromUtf8("—"))
                                     .arg(bp.scoreKnown ? QString::number(bp.score, 'f', 1) : QString::fromUtf8("—"))
                                     .arg(bp.feasibility);
                } else {
                    r.headline = QString::fromUtf8("无最优点");
                }
                n.detail = r.headline;
                return true;
            });

            // N4 提升最优为飞机方案版本（可选）
            if (promoteBest && study.bestIndex >= 0 && m_aircraftStore && m_aircraftDoc) {
                step(3, [&](WorkflowNodeResult &n) -> bool {
                    QRegExp re(QStringLiteral("^R0*([0-9]+)$"));
                    if (!re.exactMatch(base.sourceRevision)) {
                        n.detail = QString::fromUtf8("基准未关联飞机修订");
                        return false;
                    }
                    const int version = re.cap(1).toInt();
                    AircraftDocument aircraft;
                    QString d;
                    if (!m_aircraftStore->loadBaseline(QStringLiteral("aircraft_v%1").arg(version), &aircraft, &d)) {
                        n.detail = QString::fromUtf8("加载基准飞机失败：%1").arg(d);
                        return false;
                    }
                    const StudyPointResult &bp = study.points[study.bestIndex];
                    for (int i = 0; i < aircraft.parameters.size(); ++i) {
                        AcParameter &p = aircraft.parameters[i];
                        if (bp.variables.contains(p.symbol)) {
                            p.value = bp.variables.value(p.symbol);
                            p.valueKnown = true;
                        }
                    }
                    aircraft.title = QString::fromUtf8("工作流优化候选（自 %1）").arg(base.sourceRevision);
                    AircraftDocument published;
                    QString pubErr;
                    if (!m_aircraftDoc->publishExternalBaseline(aircraft, &published, &pubErr)) {
                        n.detail = pubErr;
                        return false;
                    }
                    r.promotedId = published.id;
                    n.detail = QString::fromUtf8("已发布 %1（CPACS %2）")
                                   .arg(published.id, AircraftCpacsService::revisionId(published.version));
                    return true;
                });
            } else {
                skip(3, promoteBest ? (study.bestIndex < 0 ? QString::fromUtf8("无最优点")
                                                            : QString::fromUtf8("飞机方案服务未初始化"))
                                    : QString::fromUtf8("未选择提升"));
            }
        }

        r.summary << WorkflowSummaryItem{QString::fromUtf8("采样点"), QString::number(study.points.size()), QString::fromUtf8("个")}
                   << WorkflowSummaryItem{QString::fromUtf8("可行点"), QString::number(feasible), QString::fromUtf8("个")}
                   << WorkflowSummaryItem{QString::fromUtf8("是否提升"),
                                          r.promotedId.isEmpty() ? QString::fromUtf8("否") : QString::fromUtf8("是"), QString()}
                   << WorkflowSummaryItem{QString::fromUtf8("状态"),
                                          okExplore ? QString::fromUtf8("完成") : QString::fromUtf8("失败"), QString()};
        r.ok = okExplore;
    }

    writeRunRecord(r);
    return r;
}

bool WorkflowService::writeRunRecord(const WorkflowRunResult &result) const
{
    const QString path = workflowDir() + QLatin1Char('/') + result.runId + QLatin1String(".json");
    QJsonObject root;
    root.insert(QStringLiteral("schema"), QStringLiteral("amdo.workflow-run.v2"));
    root.insert(QStringLiteral("runId"), result.runId);
    root.insert(QStringLiteral("runAt"), result.runAt);
    root.insert(QStringLiteral("templateId"), result.templateId);
    root.insert(QStringLiteral("templateName"), result.templateName);
    root.insert(QStringLiteral("baseObjectId"), result.baseObjectId);
    root.insert(QStringLiteral("retryLimit"), result.retryLimit);
    root.insert(QStringLiteral("resultDir"), result.resultDir);
    root.insert(QStringLiteral("headline"), result.headline);
    root.insert(QStringLiteral("promotedId"), result.promotedId);
    root.insert(QStringLiteral("ok"), result.ok);

    QJsonArray nodes;
    for (int i = 0; i < result.nodes.size(); ++i) {
        const WorkflowNodeResult &n = result.nodes[i];
        QJsonObject o;
        o.insert(QStringLiteral("id"), n.id);
        o.insert(QStringLiteral("name"), n.name);
        o.insert(QStringLiteral("type"), n.type);
        o.insert(QStringLiteral("fidelity"), n.fidelity);
        o.insert(QStringLiteral("status"), n.status);
        o.insert(QStringLiteral("detail"), n.detail);
        o.insert(QStringLiteral("artifact"), n.artifact);
        o.insert(QStringLiteral("attempts"), n.attempts);
        o.insert(QStringLiteral("elapsedMs"), static_cast<double>(n.elapsedMs));
        nodes.append(o);
    }
    root.insert(QStringLiteral("nodes"), nodes);

    QJsonArray summary;
    for (int i = 0; i < result.summary.size(); ++i) {
        const WorkflowSummaryItem &s = result.summary[i];
        QJsonObject o;
        o.insert(QStringLiteral("label"), s.label);
        o.insert(QStringLiteral("value"), s.value);
        o.insert(QStringLiteral("unit"), s.unit);
        summary.append(o);
    }
    root.insert(QStringLiteral("summary"), summary);

    QJsonArray log;
    for (int i = 0; i < result.log.size(); ++i)
        log.append(result.log[i]);
    root.insert(QStringLiteral("log"), log);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    return file.write(bytes) == bytes.size();
}

bool WorkflowService::listRuns(QVector<WorkflowRunResult> *out, QString *errorMessage) const
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("内部错误：输出参数为空");
        return false;
    }
    out->clear();
    QDir dir(workflowDir());
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("run_*.json"),
                                            QDir::Files, QDir::Name | QDir::Reversed);
    for (int i = 0; i < files.size(); ++i) {
        QFile file(dir.absoluteFilePath(files[i]));
        if (!file.open(QIODevice::ReadOnly))
            continue;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (!doc.isObject())
            continue;
        const QJsonObject o = doc.object();
        WorkflowRunResult r;
        r.runId = o.value(QStringLiteral("runId")).toString();
        r.runAt = o.value(QStringLiteral("runAt")).toString();
        r.templateId = o.value(QStringLiteral("templateId")).toString();
        r.templateName = o.value(QStringLiteral("templateName")).toString();
        r.baseObjectId = o.value(QStringLiteral("baseObjectId")).toString();
        r.retryLimit = o.value(QStringLiteral("retryLimit")).toInt();
        r.resultDir = o.value(QStringLiteral("resultDir")).toString();
        r.headline = o.value(QStringLiteral("headline")).toString();
        r.promotedId = o.value(QStringLiteral("promotedId")).toString();
        r.ok = o.value(QStringLiteral("ok")).toBool();

        const QJsonArray nodes = o.value(QStringLiteral("nodes")).toArray();
        for (int j = 0; j < nodes.size(); ++j) {
            const QJsonObject no = nodes.at(j).toObject();
            WorkflowNodeResult n;
            n.id = no.value(QStringLiteral("id")).toString();
            n.name = no.value(QStringLiteral("name")).toString();
            n.type = no.value(QStringLiteral("type")).toString();
            n.fidelity = no.value(QStringLiteral("fidelity")).toString();
            n.status = no.value(QStringLiteral("status")).toString();
            n.detail = no.value(QStringLiteral("detail")).toString();
            n.artifact = no.value(QStringLiteral("artifact")).toString();
            n.attempts = no.value(QStringLiteral("attempts")).toInt();
            n.elapsedMs = static_cast<qint64>(no.value(QStringLiteral("elapsedMs")).toDouble());
            r.nodes.append(n);
        }
        const QJsonArray summary = o.value(QStringLiteral("summary")).toArray();
        for (int j = 0; j < summary.size(); ++j) {
            const QJsonObject so = summary.at(j).toObject();
            WorkflowSummaryItem s;
            s.label = so.value(QStringLiteral("label")).toString();
            s.value = so.value(QStringLiteral("value")).toString();
            s.unit = so.value(QStringLiteral("unit")).toString();
            r.summary.append(s);
        }
        const QJsonArray log = o.value(QStringLiteral("log")).toArray();
        for (int j = 0; j < log.size(); ++j)
            r.log.append(log.at(j).toString());
        out->append(r);
    }
    return true;
}
