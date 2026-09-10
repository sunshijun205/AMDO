#include "service/srdcompletenessservice.h"

#include "model/srdcatalogs.h"

static bool isMandatoryGrade(const QString &grade)
{
    return grade == QString::fromUtf8("强制");
}

static bool isDefinedStatus(const QString &status)
{
    return status == QString::fromUtf8("已定义");
}

static bool isPendingStatus(const QString &status)
{
    return status.contains(QString::fromUtf8("草案"))
        || status.contains(QString::fromUtf8("待"));
}

static bool isApplicable(const QString &applicability)
{
    return applicability == QString::fromUtf8("直接适用")
        || applicability == QString::fromUtf8("条件适用");
}

static QString normalizePhase(const QString &name)
{
    if (name.contains(QString::fromUtf8("起飞")))
        return QString::fromUtf8("起飞");
    if (name.contains(QString::fromUtf8("爬升")))
        return QString::fromUtf8("爬升");
    if (name.contains(QString::fromUtf8("巡航")))
        return QString::fromUtf8("巡航");
    if (name.contains(QString::fromUtf8("下降")))
        return QString::fromUtf8("下降");
    if (name.contains(QString::fromUtf8("备降")))
        return QString::fromUtf8("备降");
    if (name.contains(QString::fromUtf8("盘旋")))
        return QString::fromUtf8("盘旋");
    if (name.contains(QString::fromUtf8("着陆")) || name.contains(QString::fromUtf8("进近")))
        return QString::fromUtf8("着陆");
    return name;
}

static bool conditionCovers(const SrdFlightCondition &fc, const QString &phaseKey)
{
    const QString p = normalizePhase(fc.phase);
    return p == phaseKey || fc.phase.contains(phaseKey);
}

static const SrdMissionScenario *primaryMission(const SrdDocument &doc)
{
    for (int i = 0; i < doc.missions.size(); ++i) {
        if (doc.missions[i].id == QLatin1String("M-001"))
            return &doc.missions[i];
    }
    for (int i = 0; i < doc.missions.size(); ++i) {
        if (isDefinedStatus(doc.missions[i].status) && !doc.missions[i].segments.isEmpty())
            return &doc.missions[i];
    }
    if (doc.missions.isEmpty())
        return nullptr;
    return &doc.missions.first();
}

SrdCompletenessReport SrdCompletenessService::evaluate(const SrdDocument &doc) const
{
    SrdCompletenessReport report;
    SrdKpis &k = report.kpis;

    k.scenarioCount = doc.missions.size();
    k.needCount = doc.needs.size();
    for (int i = 0; i < doc.needs.size(); ++i) {
        if (isMandatoryGrade(doc.needs[i].grade))
            ++k.mandatoryNeedCount;
        if (!doc.needs[i].valueKnown)
            ++k.pendingCount;
        if (doc.needs[i].sourceKind.contains(QString::fromUtf8("市场")))
            ++k.sourceMarket;
        else if (doc.needs[i].sourceKind.contains(QString::fromUtf8("运营")))
            ++k.sourceOperator;
        else if (doc.needs[i].sourceKind.contains(QString::fromUtf8("法规")))
            ++k.sourceReg;
        else
            ++k.sourceGoal;
    }
    for (int i = 0; i < doc.missions.size(); ++i) {
        if (isPendingStatus(doc.missions[i].status))
            ++k.pendingCount;
    }

    k.conditionCount = doc.conditions.size();
    k.atmosphereModel = doc.environment.atmosphereModel;
    if (k.atmosphereModel.contains(QStringLiteral("1976")) && !k.atmosphereModel.contains(QStringLiteral("ISA")))
        k.atmosphereModel = QStringLiteral("ISA 1976");
    if (doc.environment.nzKnown)
        k.loadFactorRange = srdFormatNumber(doc.environment.nzMin) + QString::fromUtf8("～")
            + srdFormatNumber(doc.environment.nzMax);
    else
        k.loadFactorRange = QString::fromUtf8("未填");

    k.clauseCount = doc.clauses.size();
    for (int i = 0; i < doc.clauses.size(); ++i) {
        const SrdClauseRow &row = doc.clauses[i];
        if (row.applicability == QString::fromUtf8("直接适用"))
            ++k.applyDirect;
        else if (row.applicability == QString::fromUtf8("条件适用"))
            ++k.applyConditional;
        else if (row.applicability == QString::fromUtf8("需等效安全"))
            ++k.applyEquivalent;
        else
            ++k.applyNa;

        if (isApplicable(row.applicability)) {
            if (row.mappingStatus == QString::fromUtf8("已映射"))
                ++k.mappedClauseCount;
            if (isPendingStatus(row.mappingStatus))
                ++k.pendingClauseCount;
        }
    }
    k.standardSetCount = 0;
    if (!doc.certification.airworthinessClass.isEmpty())
        ++k.standardSetCount;
    if (!doc.certification.noiseStandard.isEmpty())
        ++k.standardSetCount;
    if (!doc.certification.emissionStandard.isEmpty())
        ++k.standardSetCount;

    k.metricCount = doc.requirements.size();
    for (int i = 0; i < doc.requirements.size(); ++i) {
        if (doc.requirements[i].constraintKind == QLatin1String("mandatory")
            || isMandatoryGrade(doc.requirements[i].grade))
            ++k.mandatoryConstraintCount;
        if (doc.requirements[i].constraintKind == QLatin1String("objective")
            || doc.requirements[i].grade == QString::fromUtf8("目标"))
            ++k.objectiveCount;
    }

    int completeReq = 0;
    for (int i = 0; i < doc.requirements.size(); ++i) {
        const SrdRequirement &r = doc.requirements[i];
        if (!r.metricId.isEmpty() && !r.relation.isEmpty() && r.boundKnown && !r.verificationMethod.isEmpty())
            ++completeReq;
    }
    k.coveragePercent = doc.requirements.isEmpty() ? 0 : qRound(100.0 * completeReq / doc.requirements.size());

    const SrdMissionScenario *primary = primaryMission(doc);
    int coveredSeg = 0;
    int totalSeg = 0;
    if (primary) {
        totalSeg = primary->segments.size();
        for (int i = 0; i < primary->segments.size(); ++i) {
            const QString key = normalizePhase(primary->segments[i].name);
            bool ok = false;
            for (int j = 0; j < doc.conditions.size(); ++j) {
                if (conditionCovers(doc.conditions[j], key)) {
                    ok = true;
                    break;
                }
            }
            if (ok)
                ++coveredSeg;
        }
    }
    k.conditionCoveragePercent = (totalSeg == 0) ? 0 : qRound(100.0 * coveredSeg / totalSeg);

    auto add = [&](const QString &code, const QString &message, bool blocking) {
        SrdCheckItem item;
        item.code = code;
        item.message = message;
        item.blocking = blocking;
        report.items.append(item);
    };

    bool hasDefined = false;
    for (int i = 0; i < doc.missions.size(); ++i) {
        if (isDefinedStatus(doc.missions[i].status))
            hasDefined = true;
    }
    if (!hasDefined)
        add(QStringLiteral("M1"), QString::fromUtf8("至少需要 1 个状态为「已定义」的任务场景"), true);

    bool hasMandatoryNeed = false;
    for (int i = 0; i < doc.needs.size(); ++i) {
        if (isMandatoryGrade(doc.needs[i].grade) && doc.needs[i].valueKnown)
            hasMandatoryNeed = true;
    }
    if (!hasMandatoryNeed)
        add(QStringLiteral("M2"), QString::fromUtf8("至少需要 1 条已填数值的强制性能需求"), true);

    if (primary) {
        bool hasTo = false, hasCr = false, hasLd = false;
        for (int i = 0; i < primary->segments.size(); ++i) {
            const QString key = normalizePhase(primary->segments[i].name);
            if (key == QString::fromUtf8("起飞"))
                hasTo = true;
            if (key == QString::fromUtf8("巡航"))
                hasCr = true;
            if (key == QString::fromUtf8("着陆"))
                hasLd = true;
        }
        if (!hasTo || !hasCr || !hasLd)
            add(QStringLiteral("M3"), QString::fromUtf8("主剖面须包含起飞、巡航、着陆航段"), true);
    } else {
        add(QStringLiteral("M3"), QString::fromUtf8("缺少主任务剖面"), true);
    }

    if (doc.environment.atmosphereModel.isEmpty())
        add(QStringLiteral("E1"), QString::fromUtf8("未选择大气模型"), true);
    if (!doc.environment.nzKnown)
        add(QStringLiteral("E2"), QString::fromUtf8("未定义载荷因数范围"), true);
    if (doc.envelopePoints.size() < 3)
        add(QStringLiteral("E3"), QString::fromUtf8("飞行包线至少需要 3 个高度—马赫点"), true);
    if (k.conditionCoveragePercent < 100)
        add(QStringLiteral("E4"),
            QString::fromUtf8("工况未覆盖全部主剖面航段（当前 %1%）").arg(k.conditionCoveragePercent),
            false);

    if (doc.certification.airworthinessClass.isEmpty())
        add(QStringLiteral("S1"), QString::fromUtf8("未选择适航类别"), true);

    int missingDomain = 0;
    int missingMap = 0;
    int missingOwner = 0;
    for (int i = 0; i < doc.clauses.size(); ++i) {
        const SrdClauseRow &row = doc.clauses[i];
        if (!isApplicable(row.applicability))
            continue;
        if (row.domain.isEmpty()) {
            ++missingDomain;
            ++missingOwner;
        }
        SrdCatalogClause cat;
        const bool hasSuggested = SrdCatalogs::findClause(row.clauseId, &cat)
            && !cat.suggestedMetricId.isEmpty() && cat.suggestedValueKnown;
        bool mapped = row.mappingStatus == QString::fromUtf8("已映射");
        if (!mapped && hasSuggested) {
            for (int r = 0; r < doc.requirements.size(); ++r) {
                if (doc.requirements[r].metricId == cat.suggestedMetricId) {
                    mapped = true;
                    break;
                }
            }
        }
        if (row.applicability == QString::fromUtf8("直接适用") && !mapped && hasSuggested)
            ++missingMap;
    }
    if (missingDomain > 0)
        add(QStringLiteral("S2"), QString::fromUtf8("%1 项适用条款缺少责任域").arg(missingDomain), true);
    if (missingOwner > 0)
        add(QStringLiteral("S2b"), QString::fromUtf8("%1 项条款缺少责任人/责任域").arg(missingOwner), false);
    if (missingMap > 0)
        add(QStringLiteral("S3"), QString::fromUtf8("%1 项直接适用条款尚未映射到需求").arg(missingMap), true);

    int missingVerify = 0;
    for (int i = 0; i < doc.requirements.size(); ++i) {
        const SrdRequirement &r = doc.requirements[i];
        const bool mandatory = r.constraintKind == QLatin1String("mandatory") || isMandatoryGrade(r.grade);
        if (mandatory && r.verificationMethod.isEmpty())
            ++missingVerify;
    }
    if (missingVerify > 0)
        add(QStringLiteral("Q1"), QString::fromUtf8("%1 项强制需求尚未建立验证方法").arg(missingVerify), true);

    bool hasBlockingStd = false;
    bool hasBlockingPub = false;
    for (int i = 0; i < report.items.size(); ++i) {
        if (!report.items[i].blocking)
            continue;
        hasBlockingPub = true;
        if (report.items[i].code.startsWith(QLatin1Char('S')) || report.items[i].code == QLatin1String("S1"))
            hasBlockingStd = true;
        if (report.items[i].code == QLatin1String("S1") || report.items[i].code == QLatin1String("S2"))
            hasBlockingStd = true;
    }
    // 冻结规范章不要求条款已全部映射（映射在冻结后派生）
    report.canFreezeStandards = true;
    for (int i = 0; i < report.items.size(); ++i) {
        if (report.items[i].blocking && (report.items[i].code == QLatin1String("S1")
                                        || report.items[i].code == QLatin1String("S2")))
            report.canFreezeStandards = false;
    }
    report.canPublish = !hasBlockingPub;
    Q_UNUSED(hasBlockingStd);
    return report;
}
