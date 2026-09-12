#ifndef EVALUATIONRESULT_H
#define EVALUATIONRESULT_H

#include <QString>
#include <QVector>

// 单方案评价结果 POD：分析结果值 ⊗ SRD 需求限值 → 逐指标可行性/裕度。
//
// 注意：actualFidelity 透传自学科分析（精确/低保真估算/MOCK）。
//   基于 MOCK 值的判定仅为示意，界面会显式标注；后期分析真实化后判定自动变真。

struct EvaluationItem {
    QString requirementId;   // 如 REQ-WGT-004
    QString metricName;      // 如 最大起飞重量
    QString domain;
    QString relation;        // <= / >= / ==
    double boundValue = 0.0; // 需求限值
    bool boundKnown = false;
    QString unit;
    QString grade;           // 强制 / 目标 / 期望

    double actualValue = 0.0;// 来自分析结果的方案值
    bool actualKnown = false;
    QString actualFidelity;  // 精确 / 低保真估算 / MOCK（空=无分析值）

    double marginPercent = 0.0; // 相对限值的裕度百分比（带号：正=有余量）
    bool marginKnown = false;
    QString status;          // 满足 / 违反 / 临界 / 待分析
};

struct SchemeEvaluationResult {
    QString objectId;              // 评价对象：draft / analysis_vN（用于持久化文件名与比较标识）
    QString evaluatedAt;
    QString sourceRevision;
    QString sourceSrd;
    QString sourceCase;
    QString sourceCondition;
    bool aircraftResolved = false;
    bool srdResolved = false;      // 是否成功关联并读到 SRD 需求

    int total = 0;
    int satisfied = 0;
    int violated = 0;
    int critical = 0;
    int pending = 0;               // 待分析（无分析值）
    int mandatoryTotal = 0;
    int mandatoryViolated = 0;
    int mandatoryPending = 0;
    QString feasibility;           // 通过 / 违反 / 待定

    double score = 0.0;            // 综合评分（满足率 %：(满足+临界)/有值项）
    bool scoreKnown = false;

    QVector<EvaluationItem> items;
};

#endif
