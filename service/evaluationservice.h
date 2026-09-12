#ifndef EVALUATIONSERVICE_H
#define EVALUATIONSERVICE_H

#include "model/analysistypes.h"
#include "model/evaluationresult.h"

#include <QString>
#include <QVector>

class AnalysisComputeService;
class AnalysisStore;
class SrdStore;

// 单方案评价：把学科分析结果值与关联 SRD 需求的限值逐条比对，得到可行性/裕度。
// 对齐 PDF 的 MetricEvaluator（CPACS + 评价规则 → 评价结果）。
//
// TODO（后期细化）：
//   - 综合评分/多目标权衡属「方案比较与权衡」，本服务只做单方案约束/指标判定。
//   - responseId ↔ 分析结果键 现为内置映射表；后期宜统一命名或做成可配置。
//   - 支持选择历史运行结果（case_N.evaluation.json）而非仅当前分析草稿。
class EvaluationService
{
public:
    EvaluationService(AnalysisComputeService *compute, SrdStore *srdStore,
                      AnalysisStore *analysisStore);

    // 评价给定分析集：内部先跑学科计算得到方案值，再对关联 SRD 需求逐条判定。
    // objectId：评价对象标识（draft / analysis_vN），写入结果并用于持久化文件名。
    // tolerancePercent：判定“临界”的容差（百分比，如 0.5 表示 0.5%）。
    SchemeEvaluationResult evaluate(const AnalysisDocument &analysis,
                                    const QString &objectId,
                                    double tolerancePercent,
                                    QString *errorMessage = nullptr) const;

    // 持久化评价结果到 analysis/evaluations/<objectId>.evaluation.json。
    bool saveResult(const SchemeEvaluationResult &result, QString *outPath = nullptr,
                    QString *errorMessage = nullptr) const;
    // 读取全部已保存评价结果（仅汇总字段，供比较与权衡）。
    bool listResults(QVector<SchemeEvaluationResult> *out, QString *errorMessage = nullptr) const;

    QString evaluationsDir() const;

private:
    AnalysisComputeService *m_compute;
    SrdStore *m_srdStore;
    AnalysisStore *m_analysisStore;
};

#endif
