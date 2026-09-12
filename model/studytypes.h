#ifndef STUDYTYPES_H
#define STUDYTYPES_H

#include <QHash>
#include <QString>
#include <QVector>

// 方案优化（设计空间探索）领域 POD。
//
// 现状：首版为设计空间探索(DOE 网格采样) + 从样本中选最优的闭环。
// 设计变量取已真实计算的几何量（S_ref、b → AR → L/D），采样会真实影响真实指标；
// 真正的优化算法(NSGA/梯度/MDO)按 IStudyOptimizer 接口留待后续实现（见 studyservice.h）。

// 设计变量：按飞机参数 symbol 覆盖（如 "S_ref"、"b"）。
struct StudyVariable {
    QString symbol;      // 飞机参数符号（覆盖键）
    QString name;        // 展示名
    QString unit;
    double minValue = 0.0;
    double maxValue = 0.0;
    int steps = 3;       // 网格步数（>=2）
    bool enabled = true;
};

struct StudyDefinition {
    QString baseObjectId;            // 基准分析集：draft / analysis_vN
    QString sampling = QStringLiteral("grid"); // 采样法（当前仅 grid）
    double tolerancePercent = 0.5;   // 评价容差
    QVector<StudyVariable> variables;
};

// 单个设计点的分析+评价结果。
struct StudyPointResult {
    int index = 0;
    QHash<QString, double> variables; // symbol → 取值
    double ar = 0.0; bool arKnown = false;
    double ld = 0.0; bool ldKnown = false;
    double score = 0.0; bool scoreKnown = false; // 满足率 %
    QString feasibility;              // 通过 / 违反 / 待定
    int satisfied = 0;
    int violated = 0;
    int critical = 0;
    int pending = 0;
};

struct StudyResult {
    QString runAt;
    QString baseObjectId;
    QVector<StudyVariable> variables; // 参与的变量（表头/顺序）
    QVector<StudyPointResult> points;
    int bestIndex = -1;               // 最优点（可行优先，按评分）；-1 表示无
    bool srdResolved = false;
};

#endif
