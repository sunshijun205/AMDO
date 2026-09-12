#ifndef ANALYSISTYPES_H
#define ANALYSISTYPES_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

// 学科分析领域 POD：分析集（多学科分析配置）。无业务判断。

// 单个可编辑字段的静态定义（结构与默认值，来自目录）。
struct AnalysisField {
    QString label;         // 字段标签，如“求解方法”
    QString type;          // select / number / text / check
    QString defaultValue;  // 默认值（check 用“启用”/“关闭”）
    QString unit;          // 单位（可空）
    QStringList options;   // select 备选项（可空则用通用项）
};

struct AnalysisSection {
    QString title;
    QVector<AnalysisField> fields;
};

struct AnalysisSummaryItem {
    QString key;
    QString value;
};

// 一个学科（气动/结构/…）的静态配置结构。
struct AnalysisDomain {
    QString id;            // 稳定标识：aero/structure/…
    QString name;          // 侧栏名
    QString title;         // 页标题
    QString subtitle;      // 页副标题
    QStringList templates; // 配置模板名
    QVector<AnalysisSection> sections;
    QVector<AnalysisSummaryItem> summary;
    QStringList checks;    // 运行前检查项（静态提示）
};

// 分析集文档：仅保存引用与用户字段取值（结构在目录中）。
struct AnalysisDocument {
    QString schemaVersion;   // amdo.analysis.v1
    QString id;              // 草稿为 analysis-draft；基线为 analysis_vN
    QString title;           // 分析集名称
    QString status;          // draft / published
    int version = 0;         // 已发布版本号；草稿为 0
    QString publishedAt;     // 发布时间戳（ISO）
    QString sourceRevision;  // 引用的飞机修订，如 R002（不复制方案数据）
    QString sourceCase;      // 引用的用例快照，如 case_1（可空）
    QHash<QString, QString> values; // 键 = domainId/sectionTitle/fieldLabel
    bool loadedKnown = false;
};

struct AnalysisBaselineInfo {
    QString id;
    QString title;
    int version = 0;
    QString publishedAt;
    QString filePath;
};

// 字段取值的稳定键。
inline QString analysisFieldKey(const QString &domainId, const QString &sectionTitle,
                                const QString &fieldLabel)
{
    return domainId + QLatin1Char('/') + sectionTitle + QLatin1Char('/') + fieldLabel;
}

#endif
