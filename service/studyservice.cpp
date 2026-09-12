#include "service/studyservice.h"

#include "model/analysisstore.h"
#include "service/analysiscomputeservice.h"
#include "service/evaluationservice.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static const int kMaxPoints = 500; // 采样点上限，防止组合爆炸

// 网格搜索取最优：可行(通过)优先，其次按满足率评分；均无则取评分最高。
// TODO：NSGA-II/梯度/MDO 等按 IStudyOptimizer 接口后续实现（需外部专业库）。
class GridBestOptimizer : public IStudyOptimizer
{
public:
    QString name() const override { return QStringLiteral("GridBest"); }
    int selectBest(const StudyResult &result) const override
    {
        int best = -1;
        double bestScore = -1.0;
        bool bestFeasible = false;
        for (int i = 0; i < result.points.size(); ++i) {
            const StudyPointResult &p = result.points[i];
            const bool feasible = (p.feasibility == QString::fromUtf8("通过"));
            const double score = p.scoreKnown ? p.score : -1.0;
            // 可行优先；同类按评分。
            if (best < 0
                || (feasible && !bestFeasible)
                || (feasible == bestFeasible && score > bestScore)) {
                best = i;
                bestScore = score;
                bestFeasible = feasible;
            }
        }
        return best;
    }
};

StudyService::StudyService(AnalysisComputeService *compute, EvaluationService *evaluation,
                           AnalysisStore *analysisStore)
    : m_compute(compute)
    , m_evaluation(evaluation)
    , m_analysisStore(analysisStore)
{
}

static void extractArLd(const AnalysisRunResult &run, StudyPointResult *pt)
{
    for (int i = 0; i < run.disciplines.size(); ++i) {
        const AnalysisDisciplineResult &d = run.disciplines[i];
        for (int j = 0; j < d.items.size(); ++j) {
            const AnalysisResultItem &it = d.items[j];
            if (it.key == QLatin1String("aero.AR") && it.valueKnown) {
                pt->ar = it.value;
                pt->arKnown = true;
            } else if (it.key == QLatin1String("aero.LD") && it.valueKnown) {
                pt->ld = it.value;
                pt->ldKnown = true;
            }
        }
    }
}

StudyResult StudyService::run(const AnalysisDocument &base, const StudyDefinition &definition,
                              QString *errorMessage) const
{
    StudyResult result;
    result.runAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    result.baseObjectId = definition.baseObjectId;

    // 只取启用且合法(steps>=1，max>=min)的变量。
    QVector<StudyVariable> vars;
    for (int i = 0; i < definition.variables.size(); ++i) {
        const StudyVariable &v = definition.variables[i];
        if (v.enabled && !v.symbol.isEmpty() && v.maxValue >= v.minValue && v.steps >= 1)
            vars.append(v);
    }
    result.variables = vars;
    if (vars.isEmpty()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("没有有效的设计变量");
        return result;
    }

    // 计算每个变量的取值序列（网格）与总组合数。
    QVector<QVector<double>> axis;
    long long total = 1;
    for (int i = 0; i < vars.size(); ++i) {
        const StudyVariable &v = vars[i];
        QVector<double> vals;
        const int n = qMax(1, v.steps);
        if (n == 1) {
            vals.append(v.minValue);
        } else {
            for (int k = 0; k < n; ++k)
                vals.append(v.minValue + (v.maxValue - v.minValue) * k / (n - 1));
        }
        axis.append(vals);
        total *= vals.size();
    }
    const int pointCount = static_cast<int>(qMin<long long>(total, kMaxPoints));

    bool srdResolvedAny = false;
    for (int idx = 0; idx < pointCount; ++idx) {
        // 解码第 idx 个组合为各变量下标。
        QHash<QString, double> overrides;
        StudyPointResult pt;
        pt.index = idx;
        long long rem = idx;
        for (int i = 0; i < vars.size(); ++i) {
            const int cnt = axis[i].size();
            const int k = static_cast<int>(rem % cnt);
            rem /= cnt;
            const double value = axis[i][k];
            overrides.insert(vars[i].symbol, value);
            pt.variables.insert(vars[i].symbol, value);
        }

        const AnalysisRunResult runRes = m_compute->run(base, overrides);
        extractArLd(runRes, &pt);

        const SchemeEvaluationResult eval = m_evaluation->evaluateWith(
            runRes, base, QString::fromUtf8("study#%1").arg(idx), definition.tolerancePercent);
        srdResolvedAny = srdResolvedAny || eval.srdResolved;
        pt.score = eval.score;
        pt.scoreKnown = eval.scoreKnown;
        pt.feasibility = eval.feasibility;
        pt.satisfied = eval.satisfied;
        pt.violated = eval.violated;
        pt.critical = eval.critical;
        pt.pending = eval.pending;

        result.points.append(pt);
    }
    result.srdResolved = srdResolvedAny;

    GridBestOptimizer optimizer;
    result.bestIndex = optimizer.selectBest(result);
    return result;
}

bool StudyService::saveResult(const StudyResult &result, QString *outPath, QString *errorMessage) const
{
    if (!m_analysisStore) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    const QString dir = m_analysisStore->rootDir() + QLatin1String("/studies");
    QDir().mkpath(dir);
    const QString path = dir + QLatin1String("/study_result.json");

    QJsonObject root;
    root.insert(QStringLiteral("schema"), QStringLiteral("amdo.study-result.v1"));
    root.insert(QStringLiteral("runAt"), result.runAt);
    root.insert(QStringLiteral("baseObjectId"), result.baseObjectId);
    root.insert(QStringLiteral("bestIndex"), result.bestIndex);

    QJsonArray vars;
    for (int i = 0; i < result.variables.size(); ++i) {
        const StudyVariable &v = result.variables[i];
        QJsonObject o;
        o.insert(QStringLiteral("symbol"), v.symbol);
        o.insert(QStringLiteral("name"), v.name);
        o.insert(QStringLiteral("min"), v.minValue);
        o.insert(QStringLiteral("max"), v.maxValue);
        o.insert(QStringLiteral("steps"), v.steps);
        vars.append(o);
    }
    root.insert(QStringLiteral("variables"), vars);

    QJsonArray points;
    for (int i = 0; i < result.points.size(); ++i) {
        const StudyPointResult &p = result.points[i];
        QJsonObject o;
        o.insert(QStringLiteral("index"), p.index);
        QJsonObject vals;
        for (auto it = p.variables.constBegin(); it != p.variables.constEnd(); ++it)
            vals.insert(it.key(), it.value());
        o.insert(QStringLiteral("variables"), vals);
        if (p.arKnown)
            o.insert(QStringLiteral("AR"), p.ar);
        if (p.ldKnown)
            o.insert(QStringLiteral("LD"), p.ld);
        if (p.scoreKnown)
            o.insert(QStringLiteral("score"), p.score);
        o.insert(QStringLiteral("feasibility"), p.feasibility);
        o.insert(QStringLiteral("satisfied"), p.satisfied);
        o.insert(QStringLiteral("violated"), p.violated);
        o.insert(QStringLiteral("critical"), p.critical);
        o.insert(QStringLiteral("pending"), p.pending);
        points.append(o);
    }
    root.insert(QStringLiteral("points"), points);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无法写入探索结果：%1").arg(file.errorString());
        return false;
    }
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("探索结果写入不完整：%1").arg(file.errorString());
        return false;
    }
    if (outPath)
        *outPath = path;
    return true;
}
