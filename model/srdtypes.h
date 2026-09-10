#ifndef SRDTYPES_H
#define SRDTYPES_H

#include <QString>
#include <QVector>
#include <QtGlobal>

// 设计需求（SRD）领域 POD。无业务判断。

struct SrdAirportOps {
    double elevationM = 0;
    bool elevationKnown = false;
    double runwayLengthM = 0;
    bool runwayLengthKnown = false;
    double takeoffFieldLimitM = 0;
    bool takeoffFieldLimitKnown = false;
    double landingFieldLimitM = 0;
    bool landingFieldLimitKnown = false;
    double climbGradientPercent = 0;
    bool climbGradientKnown = false;
};

struct SrdMissionSegment {
    QString id;
    QString name;
    QString endCondition;
    double altitudeM = 0;
    bool altitudeKnown = false;
    double mach = 0;
    bool machKnown = false;
};

struct SrdMissionScenario {
    QString id;
    QString name;
    QString missionType;
    QString boundary;
    QString environment;
    QString status;
    QString crew;
    QString utilization;
    QString designLife;
    SrdAirportOps airport;
    QVector<SrdMissionSegment> segments;
};

struct SrdPerformanceNeed {
    QString id;
    QString name;
    QString metricId;
    QString relation;
    double value = 0;
    bool valueKnown = false;
    QString unit;
    QString grade;
    QString sourceKind;
};

struct SrdEnvelopePoint {
    double mach = 0;
    double altitudeKm = 0;
};

struct SrdEnvironment {
    QString atmosphereModel;
    QString temperatureOffset;
    QString crosswindLimit;
    QString runwaySlope;
    QString gustModel;
    QString icingCondition;
    double nzMin = -1.0;
    double nzMax = 2.5;
    bool nzKnown = true;
};

struct SrdFlightCondition {
    QString id;
    QString phase;
    QString altitude;
    QString speed;
    QString configuration;
    QString atmosphere;
    QString sourceScenarioId;
};

struct SrdCertification {
    QString airworthinessClass;
    QString opsRules;
    QString amendment;
    QString noiseStandard;
    QString emissionStandard;
    QString specialCondition;
};

struct SrdClauseRow {
    QString clauseId;
    QString topic;
    QString domain;
    QString applicability;
    QString mappingStatus;
    QString mappedReqId;
};

struct SrdRequirement {
    QString id;
    QString metricId;
    QString metricName;
    QString domain;
    QString relation;
    double boundValue = 0;
    bool boundKnown = false;
    QString unit;
    QString grade;
    QString constraintKind;
    QString source;
    QString status;
    QString priority;
    QString verificationMethod;
    QString upstream;
    QString downstreamVars;
    QString analysisResponseId;
    QString evidenceRun;
};

struct SrdChapterFlags {
    bool missionFrozen = false;
    bool envelopeFrozen = false;
    bool standardsFrozen = false;
    bool metricsFrozen = false;
};

struct SrdDocument {
    QString schemaVersion;
    QString id;
    QString title;
    QString status;
    int version = 0;
    QString publishedAt;
    SrdChapterFlags chapters;
    SrdCertification certification;
    QVector<SrdMissionScenario> missions;
    QVector<SrdPerformanceNeed> needs;
    QVector<SrdEnvelopePoint> envelopePoints;
    SrdEnvironment environment;
    QVector<SrdFlightCondition> conditions;
    QVector<SrdClauseRow> clauses;
    QVector<SrdRequirement> requirements;
    bool loadedKnown = false;
};

struct SrdCatalogMetric {
    QString id;
    QString name;
    QString unit;
    QString domain;
    QString responseId;
};

struct SrdCatalogClause {
    QString clauseId;
    QString topic;
    QString domain;
    bool electricPropulsionRelated = false;
    QString suggestedMetricId;
    QString suggestedRelation;
    double suggestedValue = 0;
    bool suggestedValueKnown = false;
    QString suggestedUnit;
    QString suggestedGrade;
};

struct SrdCheckItem {
    QString code;
    QString message;
    bool blocking = true;
};

struct SrdKpis {
    int scenarioCount = 0;
    int needCount = 0;
    int mandatoryNeedCount = 0;
    int pendingCount = 0;
    int conditionCount = 0;
    int envelopeDimCount = 5;
    QString atmosphereModel;
    QString loadFactorRange;
    int standardSetCount = 0;
    int clauseCount = 0;
    int mappedClauseCount = 0;
    int pendingClauseCount = 0;
    int metricCount = 0;
    int mandatoryConstraintCount = 0;
    int objectiveCount = 0;
    int coveragePercent = 0;
    int conditionCoveragePercent = 0;
    int sourceMarket = 0;
    int sourceOperator = 0;
    int sourceReg = 0;
    int sourceGoal = 0;
    int applyDirect = 0;
    int applyConditional = 0;
    int applyNa = 0;
    int applyEquivalent = 0;
};

struct SrdCompletenessReport {
    SrdKpis kpis;
    QVector<SrdCheckItem> items;
    bool canFreezeStandards = false;
    bool canPublish = false;
};

struct SrdBaselineInfo {
    QString id;
    QString title;
    int version = 0;
    QString publishedAt;
    QString filePath;
};

inline QString srdGradeToKind(const QString &grade)
{
    if (grade == QString::fromUtf8("强制"))
        return QStringLiteral("mandatory");
    if (grade == QString::fromUtf8("目标"))
        return QStringLiteral("objective");
    return QStringLiteral("wish");
}

inline QString srdFormatNumber(double value)
{
    const qint64 rounded = qRound64(value);
    if (qAbs(value - static_cast<double>(rounded)) < 1e-9)
        return QString::number(rounded);
    return QString::number(value, 'g', 6);
}

inline QString srdFormatBound(const QString &relation, double value, bool known, const QString &unit)
{
    if (!known)
        return relation + QString::fromUtf8(" （未填）");
    QString text = relation + QLatin1Char(' ') + srdFormatNumber(value);
    if (!unit.isEmpty())
        text += QLatin1Char(' ') + unit;
    return text;
}

#endif
