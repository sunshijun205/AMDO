#ifndef ANALYSISCOMPUTESERVICE_H
#define ANALYSISCOMPUTESERVICE_H

#include "model/aircrafttypes.h"
#include "model/analysisresult.h"
#include "model/analysistypes.h"

#include <QString>

class AircraftStore;
class AnalysisStore;
class SrdStore;

// ============================================================================
// 学科运行计算模块（独立于配置持久化）。
//
// 现状（低保真原型，能真算的已真算）：
//   - 气动：ISA 大气、真空速 V、动压 q、展弦比 AR、雷诺数 Re = 真实；L/D = 估算；CL/CD/导数 = MOCK。
//   - 推进：安装后总推力 = 真实（来自配置）；SFC/流量/裕度 = MOCK。
//   - 任务性能：起飞/着陆场长余量 = 真实（配置相减）；航程/燃油/实际场长 = MOCK。
//   - 结构/重量/操稳：全 MOCK（缺求解器/关键输入，无法诚实计算）。
//
// TODO（后期细化各学科计算，逐项替换 MOCK）：
//   - 气动：CL/CD、极曲线、气动导数（当前为 MOCK）。
//   - 重量：Class-II 统计估重、重量闭环、CG/惯量（需 MTOW/部件质量）。
//   - 任务性能：Breguet 航程、真实场长、爬升（需 SFC/重量分数/L-D 闭环）。
//   - 推进：随高度马赫变化的可用推力与 SFC（需发动机图谱）。
//   - 结构：梁壳 FEM/载荷映射（需几何网格，涉及 OCCT/求解器）。
//   - 操稳：配平/线化导数（需 VLM 或气动导数）。
//   - 输入：设计工况应来自关联的 sourceSrd（当前仅取分析集内手填的高度/马赫）。
//   - 输出：关联用例时对齐 PDF 的 case_N.evaluation.json（当前统一写 analysis_result.json）。
// ============================================================================

// 学科计算输入上下文。
struct AnalysisComputeInputs {
    AircraftDocument aircraft;      // 由 sourceRevision 解析（未解析时 aircraftResolved=false）
    bool aircraftResolved = false;
    AnalysisDocument analysis;      // 分析集配置（values + 引用）

    double altitudeM = 0.0;         // 工况高度（m）
    bool altitudeKnown = false;
    double mach = 0.0;              // 工况马赫数
    bool machKnown = false;
    // 工况来源：优先取关联 SRD 的飞行包线点，否则回退分析集手填。
    bool conditionFromSrd = false;
    QString conditionNote;          // 工况来源说明（用于结果 note）

    // 按 symbol 取飞机几何参数（如 "S_ref"、"b"）；找不到时 *known=false。
    double aircraftParam(const QString &symbol, bool *known) const;
};

// 学科计算器统一接口：每个学科产出其“预期输出”的全部项
// （能算的给真实/估算值，缺模型或数据的给 MOCK 占位并在 note 注明 TODO）。
class IDisciplineComputer
{
public:
    virtual ~IDisciplineComputer() {}
    virtual QString domainId() const = 0;
    virtual QString domainName() const = 0;
    virtual AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const = 0;
};

// —— 气动：部分真实 + 部分 MOCK ——
// 真实：ISA 大气(T/p/ρ/a)、真空速 V、动压 q、展弦比 AR、雷诺数 Re。
// 估算：巡航 L/D（假设 Cd0、Oswald e）。
// MOCK：CL、CD、俯仰力矩系数、气动导数（TODO：接极曲线/涡格法）。
class AeroComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("aero"); }
    QString domainName() const override { return QString::fromUtf8("气动"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// —— 结构：全 MOCK（TODO：梁壳 FEM/载荷映射，需几何网格）——
class StructureComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("structure"); }
    QString domainName() const override { return QString::fromUtf8("结构"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// —— 重量与质量特性：全 MOCK（TODO：统计估重 + 重量闭环 + CG/惯量）——
class MassComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("mass"); }
    QString domainName() const override { return QString::fromUtf8("重量与质量特性"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// —— 推进与能源：安装后总推力真实（来自配置）；SFC/流量/裕度 MOCK ——
class PropulsionComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("propulsion"); }
    QString domainName() const override { return QString::fromUtf8("推进与能源"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// —— 操稳与飞行动力学：全 MOCK（TODO：配平/线化导数，需 VLM）——
class DynamicsComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("dynamics"); }
    QString domainName() const override { return QString::fromUtf8("操稳与飞行动力学"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// —— 任务与性能：场长余量真实（配置相减）；航程/燃油/实际场长 MOCK ——
class MissionComputer : public IDisciplineComputer
{
public:
    QString domainId() const override { return QStringLiteral("mission"); }
    QString domainName() const override { return QString::fromUtf8("任务与性能"); }
    AnalysisDisciplineResult compute(const AnalysisComputeInputs &in) const override;
};

// 编排：解析输入 → 逐学科计算 → 汇总 → 可持久化结果 JSON。
class AnalysisComputeService
{
public:
    AnalysisComputeService(AircraftStore *aircraftStore, AnalysisStore *analysisStore,
                           SrdStore *srdStore);

    AnalysisRunResult run(const AnalysisDocument &analysis, QString *errorMessage = nullptr) const;
    bool saveResult(const AnalysisRunResult &result, QString *outPath = nullptr,
                    QString *errorMessage = nullptr) const;
    QString resultPath() const;

private:
    AnalysisComputeInputs resolveInputs(const AnalysisDocument &analysis) const;

    AircraftStore *m_aircraftStore;
    AnalysisStore *m_analysisStore;
    SrdStore *m_srdStore;
};

#endif
