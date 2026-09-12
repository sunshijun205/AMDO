#ifndef STUDYSERVICE_H
#define STUDYSERVICE_H

#include "model/analysistypes.h"
#include "model/studytypes.h"

#include <QString>

class AnalysisComputeService;
class EvaluationService;
class AnalysisStore;

// 优化器接口：从设计空间结果中选出最优点。
// 现有实现 GridBestOptimizer（可行优先、按满足率评分）——即"网格搜索取最优"。
// TODO：NSGA-II / 差分进化 / 梯度 / MDO(OpenMDAO) 等高级优化作为本接口的后续实现，
//       需外部专业库/数值后端，当前不实现（可按此接口替换接入）。
class IStudyOptimizer
{
public:
    virtual ~IStudyOptimizer() {}
    virtual QString name() const = 0;
    virtual int selectBest(const StudyResult &result) const = 0;
};

// 设计空间探索服务：按定义生成采样点，逐点覆盖飞机参数跑分析+评价，汇总并选最优。
class StudyService
{
public:
    StudyService(AnalysisComputeService *compute, EvaluationService *evaluation,
                 AnalysisStore *analysisStore);

    StudyResult run(const AnalysisDocument &base, const StudyDefinition &definition,
                    QString *errorMessage = nullptr) const;

    bool saveResult(const StudyResult &result, QString *outPath = nullptr,
                    QString *errorMessage = nullptr) const;

private:
    AnalysisComputeService *m_compute;
    EvaluationService *m_evaluation;
    AnalysisStore *m_analysisStore;
};

#endif
