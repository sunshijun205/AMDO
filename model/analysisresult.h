#ifndef ANALYSISRESULT_H
#define ANALYSISRESULT_H

#include <QString>
#include <QVector>

// 学科运行计算的结果 POD。无业务判断，仅承载数值与元信息。
//
// 保真度约定（fidelity）：
//   "精确"       —— 由输入直接精确计算（如 ISA 大气、AR=b²/S）。
//   "低保真估算" —— 经验/半经验公式 + 明确假设系数（如 L/D）。
//   "MOCK"       —— 占位假值：缺模型或缺数据，先给合理量级占位，note 注明 TODO。
//
// !!! MOCK 项不是真实结果，仅为打通链路的占位；后期细化各学科计算时须替换。!!!

namespace AnalysisFidelity {
inline QString exact() { return QString::fromUtf8("精确"); }
inline QString estimate() { return QString::fromUtf8("低保真估算"); }
inline QString mock() { return QStringLiteral("MOCK"); }
}

struct AnalysisResultItem {
    QString key;            // 稳定键，如 "aero.LD"
    QString label;          // 展示名，如 "巡航升阻比"
    double value = 0.0;     // 数值
    bool valueKnown = false;// 是否有值（含 MOCK 占位；仅在完全无法给出时为 false）
    QString unit;           // 单位（可空）
    QString fidelity;       // 见上：精确 / 低保真估算 / MOCK
    QString note;           // 假设、来源或 TODO 说明
};

struct AnalysisDisciplineResult {
    QString domainId;       // aero / structure / mass / propulsion / dynamics / mission
    QString domainName;     // 展示名
    QString status;         // "computed"（全真实）/ "partial"（部分真实）/ "mock"（全占位）
    QVector<AnalysisResultItem> items;
};

struct AnalysisRunResult {
    QString runAt;          // 运行时间（ISO）
    QString sourceRevision; // 关联的飞机修订（R00N，可空）
    QString sourceSrd;      // 关联的设计需求基线（srd_vN，可空）
    QString sourceCase;     // 关联的用例（case_N，可空）
    QString sourceCondition;// 选定的设计工况（SRD 包线点索引，可空）
    bool aircraftResolved = false; // 是否成功解析到飞机方案参数
    QVector<AnalysisDisciplineResult> disciplines;
};

#endif
