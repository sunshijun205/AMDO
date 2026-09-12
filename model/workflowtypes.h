#ifndef WORKFLOWTYPES_H
#define WORKFLOWTYPES_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

// 工作流编排与执行 POD。
//   - 模板(WorkflowTemplate)描述“可编排项”：一条有序节点链，每节点复用某个已实现模块。
//   - 运行结果(WorkflowRunResult)记录逐节点状态/耗时/保真度/日志，写运行记录支持复现。
//
// 保真度约定：节点复用的底层计算能真算的标“真实”，缺模型/数据只能占位的标“MOCK”，
// 部分真实部分占位标“部分真实”（如学科分析：气动真实、其余 MOCK）。
//
// TODO（后续阶段）：可视化拖拽画布、真 DAG 并行/循环/迭代收敛、依赖图调度、
//   HPC/许可证、后台线程实时进度、真 MDO 循环 —— 多属外部专业库/后端，按接口预留。

namespace WorkflowFidelity {
inline QString real() { return QString::fromUtf8("真实"); }
inline QString partial() { return QString::fromUtf8("部分真实"); }
inline QString mock() { return QStringLiteral("MOCK"); }
}

// 模板中的节点定义（静态描述，供“流程定义”只读展示）。
struct WorkflowNodeSpec {
    QString id;        // N1/N2/…
    QString name;      // 节点名
    QString type;      // 数据/计算/汇总/评价/更新
    QString module;    // 复用模块（展示）
    QString output;    // 产物（展示）
    QString fidelity;  // 真实 / 部分真实 / MOCK
    bool optional = false; // 是否可选（如“提升”受开关控制）
};

// 工作流模板：可编排项的组织（当前为固定几套预置模板，顺序执行）。
struct WorkflowTemplate {
    QString id;
    QString name;
    QString description;
    QVector<WorkflowNodeSpec> nodes;
    bool supportsPromote = false; // 是否含“提升为飞机方案版本”节点
};

// 单节点运行结果。
struct WorkflowNodeResult {
    QString id;
    QString name;
    QString type;
    QString fidelity;
    QString status;    // 待运行 / 运行中 / 完成 / 失败 / 跳过
    QString detail;    // 摘要
    QString artifact;  // 产物路径（可空）
    int attempts = 0;  // 实际尝试次数（含重试）
    qint64 elapsedMs = 0;
};

// 监控 KPI（模板无关的键值项）。
struct WorkflowSummaryItem {
    QString label;
    QString value;
    QString unit;
};

struct WorkflowRunResult {
    QString runId;         // run_<时间戳>
    QString runAt;         // ISO 时间
    QString templateId;
    QString templateName;
    QString baseObjectId;  // 基准分析集：draft / analysis_vN
    int retryLimit = 0;    // 失败重试上限
    QString resultDir;     // 结果目录

    QVector<WorkflowNodeResult> nodes;
    QStringList log;
    QVector<WorkflowSummaryItem> summary; // 运行汇总 KPI

    QString headline;      // 头条摘要（最优点 / 评价结论）
    QString promotedId;    // 若提升，新的飞机方案版本 id
    bool ok = false;       // 是否成功跑完主流程
};

#endif
