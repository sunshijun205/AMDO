#ifndef WORKFLOWSERVICE_H
#define WORKFLOWSERVICE_H

#include "model/workflowtypes.h"

#include <functional>

#include <QString>
#include <QVector>

class AnalysisStore;
class AnalysisComputeService;
class EvaluationService;
class StudyService;
class AircraftStore;
class AircraftDocumentService;

// 工作流编排器（阶段一 + 模板数据模型）：
//   - 提供多套预置模板（可编排项的组织），每套是一条有序节点链，各节点复用已实现模块；
//   - 按模板顺序同步执行，逐节点记录状态/耗时/保真度/日志，支持失败重试；
//   - 写可复现的运行记录，支持历史复现。
//
// 现有真实模板：
//   explore「设计空间探索与方案比选」：载入基准 → 设计空间探索(StudyService)
//           → 结果汇总/比选 → (可选)提升最优为飞机方案版本(AircraftDocumentService)。
//   single 「单方案分析与评价」：载入基准 → 学科分析计算(AnalysisComputeService)
//           → 单方案评价(EvaluationService)。
//
// TODO：可视化编排、并行/循环、依赖图调度、HPC/许可证、真 MDO 循环（外部专业库/后端）。
class WorkflowService
{
public:
    WorkflowService(AnalysisStore *analysisStore, AnalysisComputeService *compute,
                    EvaluationService *evaluation, StudyService *study,
                    AircraftStore *aircraftStore, AircraftDocumentService *aircraftDoc);

    // 可用模板（可编排项）。
    static QVector<WorkflowTemplate> templates();
    static WorkflowTemplate templateById(const QString &id);

    // 运行工作流。
    //   templateId：模板标识（explore / single）。
    //   baseObjectId：draft / analysis_vN。
    //   promoteBest：是否提升最优为新飞机版本（仅 explore 有效）。
    //   retryLimit：单节点失败重试上限（0 = 不重试）。
    WorkflowRunResult run(const QString &templateId, const QString &baseObjectId,
                          bool promoteBest, int retryLimit, QString *errorMessage = nullptr);

    // 历史运行（用于复现）。
    bool listRuns(QVector<WorkflowRunResult> *out, QString *errorMessage = nullptr) const;

    QString workflowDir() const;

private:
    bool writeRunRecord(const WorkflowRunResult &result) const;

    AnalysisStore *m_analysisStore;
    AnalysisComputeService *m_compute;
    EvaluationService *m_evaluation;
    StudyService *m_study;
    AircraftStore *m_aircraftStore;
    AircraftDocumentService *m_aircraftDoc;
};

#endif
