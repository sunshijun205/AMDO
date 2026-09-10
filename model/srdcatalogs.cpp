#include "model/srdcatalogs.h"

static SrdCatalogMetric metricDef(const char *id, const char *nameUtf8, const char *unit,
                                  const char *domainUtf8, const char *responseId)
{
    SrdCatalogMetric m;
    m.id = QString::fromUtf8(id);
    m.name = QString::fromUtf8(nameUtf8);
    m.unit = QString::fromUtf8(unit);
    m.domain = QString::fromUtf8(domainUtf8);
    m.responseId = QString::fromUtf8(responseId);
    return m;
}

QVector<SrdCatalogMetric> SrdCatalogs::metrics()
{
    QVector<SrdCatalogMetric> list;
    list.append(metricDef("designRange_km", "设计航程", "km", "任务与性能", "mission.range_km"));
    list.append(metricDef("designPayload_kg", "设计商载", "kg", "任务与性能", "mission.payload_kg"));
    list.append(metricDef("cruiseMach", "巡航速度", "Ma", "任务与性能", "mission.cruise_mach"));
    list.append(metricDef("missionFuel_kg", "任务燃油", "kg", "任务与性能", "mission.fuel_kg"));
    list.append(metricDef("docDelta_percent", "直接运营成本", "%", "总体", "cost.doc_delta_percent"));
    list.append(metricDef("mtow_kg", "最大起飞重量", "kg", "重量与质量特性", "weight.mtow_kg"));
    list.append(metricDef("cruiseLD", "巡航升阻比", "—", "气动", "aero.cruise_ld"));
    list.append(metricDef("bucklingMargin", "屈曲裕度", "—", "结构", "struct.buckling_margin"));
    list.append(metricDef("shortPeriodDamping", "短周期阻尼比", "—", "操稳与飞行动力学", "dyn.short_period_zeta"));
    list.append(metricDef("takeoffFieldLength_m", "起飞场长", "m", "任务与性能", "mission.takeoff_field_m"));
    list.append(metricDef("landingFieldLength_m", "着陆场长", "m", "任务与性能", "mission.landing_field_m"));
    list.append(metricDef("climbGradient_percent", "起飞爬升梯度", "%", "任务与性能", "mission.climb_gradient_percent"));
    list.append(metricDef("staticMargin_percentMac", "静稳定裕度", "%MAC", "操稳与飞行动力学", "dyn.static_margin_percent_mac"));
    list.append(metricDef("crosswindLimit_kt", "侧风上限", "kt", "任务与性能", "mission.crosswind_kt"));
    return list;
}

bool SrdCatalogs::findMetric(const QString &id, SrdCatalogMetric *out)
{
    const QVector<SrdCatalogMetric> list = metrics();
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].id == id) {
            if (out)
                *out = list[i];
            return true;
        }
    }
    return false;
}

static SrdCatalogClause clauseDef(const char *id, const char *topicUtf8, const char *domainUtf8,
                                  bool electric, const char *metricId, const char *relation,
                                  double value, bool valueKnown, const char *unit, const char *gradeUtf8)
{
    SrdCatalogClause c;
    c.clauseId = QString::fromUtf8(id);
    c.topic = QString::fromUtf8(topicUtf8);
    c.domain = QString::fromUtf8(domainUtf8);
    c.electricPropulsionRelated = electric;
    c.suggestedMetricId = QString::fromUtf8(metricId);
    c.suggestedRelation = QString::fromUtf8(relation);
    c.suggestedValue = value;
    c.suggestedValueKnown = valueKnown;
    c.suggestedUnit = QString::fromUtf8(unit);
    c.suggestedGrade = QString::fromUtf8(gradeUtf8);
    return c;
}

QVector<SrdCatalogClause> SrdCatalogs::clauses()
{
    QVector<SrdCatalogClause> list;
    list.append(clauseDef("CS-25.101", "性能总则", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.105", "起飞", "任务与性能", false, "takeoffFieldLength_m", "<=", 2500, true, "m", "强制"));
    list.append(clauseDef("CS-25.107", "起飞速度", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.109", "加速-停止距离", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.111", "起飞航迹", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.113", "起飞距离与滑跑", "任务与性能", false, "takeoffFieldLength_m", "<=", 2500, true, "m", "强制"));
    list.append(clauseDef("CS-25.115", "起飞飞行航迹", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.119", "着陆爬升", "任务与性能", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.121", "起飞/爬升性能", "任务与性能", false, "climbGradient_percent", ">=", 2.4, true, "%", "强制"));
    list.append(clauseDef("CS-25.125", "着陆", "任务与性能", false, "landingFieldLength_m", "<=", 1800, true, "m", "强制"));
    list.append(clauseDef("CS-25.143", "操纵性一般要求", "操稳与飞行动力学", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.145", "纵向操纵", "操稳与飞行动力学", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.147", "航向与横向操纵", "操稳与飞行动力学", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.161", "配平", "操稳与飞行动力学", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.171", "静稳定性总则", "操稳与飞行动力学", false, "staticMargin_percentMac", ">=", 5.0, true, "%MAC", "强制"));
    list.append(clauseDef("CS-25.173", "纵向静稳定性", "操稳与飞行动力学", false, "staticMargin_percentMac", ">=", 5.0, true, "%MAC", "强制"));
    list.append(clauseDef("CS-25.181", "动稳定性", "操稳与飞行动力学", false, "shortPeriodDamping", ">=", 0.30, true, "—", "强制"));
    list.append(clauseDef("CS-25.237", "侧风", "任务与性能", false, "crosswindLimit_kt", ">=", 20, true, "kt", "强制"));
    list.append(clauseDef("CS-25.301", "载荷与强度", "结构", false, "bucklingMargin", ">=", 0.15, true, "—", "强制"));
    list.append(clauseDef("CS-25.303", "安全系数", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.321", "飞行载荷总则", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.331", "对称机动条件", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.341", "阵风与连续湍流", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.351", "偏航机动条件", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.603", "材料", "结构", false, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.1309", "设备与系统安全性", "系统", true, "", "", 0, false, "", "强制"));
    list.append(clauseDef("CS-25.1703", "电气系统安装", "系统", true, "", "", 0, false, "", "强制"));
    return list;
}

bool SrdCatalogs::findClause(const QString &id, SrdCatalogClause *out)
{
    const QVector<SrdCatalogClause> list = clauses();
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].clauseId == id) {
            if (out)
                *out = list[i];
            return true;
        }
    }
    return false;
}

static SrdMissionSegment seg(const char *id, const char *nameUtf8, const char *endUtf8,
                             double alt, bool altKnown, double mach, bool machKnown)
{
    SrdMissionSegment s;
    s.id = QString::fromUtf8(id);
    s.name = QString::fromUtf8(nameUtf8);
    s.endCondition = QString::fromUtf8(endUtf8);
    s.altitudeM = alt;
    s.altitudeKnown = altKnown;
    s.mach = mach;
    s.machKnown = machKnown;
    return s;
}

static SrdPerformanceNeed need(const char *id, const char *nameUtf8, const char *metricId,
                               const char *rel, double value, const char *unit,
                               const char *gradeUtf8, const char *sourceUtf8)
{
    SrdPerformanceNeed n;
    n.id = QString::fromUtf8(id);
    n.name = QString::fromUtf8(nameUtf8);
    n.metricId = QString::fromUtf8(metricId);
    n.relation = QString::fromUtf8(rel);
    n.value = value;
    n.valueKnown = true;
    n.unit = QString::fromUtf8(unit);
    n.grade = QString::fromUtf8(gradeUtf8);
    n.sourceKind = QString::fromUtf8(sourceUtf8);
    return n;
}

static SrdFlightCondition fc(const char *id, const char *phaseUtf8, const char *alt,
                             const char *speed, const char *configUtf8, const char *atm,
                             const char *src)
{
    SrdFlightCondition c;
    c.id = QString::fromUtf8(id);
    c.phase = QString::fromUtf8(phaseUtf8);
    c.altitude = QString::fromUtf8(alt);
    c.speed = QString::fromUtf8(speed);
    c.configuration = QString::fromUtf8(configUtf8);
    c.atmosphere = QString::fromUtf8(atm);
    c.sourceScenarioId = QString::fromUtf8(src);
    return c;
}

static SrdRequirement req(const char *id, const char *metricId, const char *rel, double value,
                          const char *gradeUtf8, const char *sourceUtf8, const char *statusUtf8,
                          const char *verifyUtf8, const char *upstreamUtf8, const char *downUtf8,
                          const char *evidenceUtf8)
{
    SrdRequirement r;
    SrdCatalogMetric m;
    r.id = QString::fromUtf8(id);
    r.metricId = QString::fromUtf8(metricId);
    if (SrdCatalogs::findMetric(r.metricId, &m)) {
        r.metricName = m.name;
        r.domain = m.domain;
        r.unit = m.unit;
        r.analysisResponseId = m.responseId;
    }
    r.relation = QString::fromUtf8(rel);
    r.boundValue = value;
    r.boundKnown = true;
    r.grade = QString::fromUtf8(gradeUtf8);
    r.constraintKind = srdGradeToKind(r.grade);
    r.source = QString::fromUtf8(sourceUtf8);
    r.status = QString::fromUtf8(statusUtf8);
    r.priority = QStringLiteral("P1");
    r.verificationMethod = QString::fromUtf8(verifyUtf8);
    r.upstream = QString::fromUtf8(upstreamUtf8);
    r.downstreamVars = QString::fromUtf8(downUtf8);
    r.evidenceRun = QString::fromUtf8(evidenceUtf8);
    return r;
}

SrdDocument SrdCatalogs::seedDocument()
{
    SrdDocument d;
    d.schemaVersion = QStringLiteral("amdo.srd.v1");
    d.id = QStringLiteral("srd-draft");
    d.title = QString::fromUtf8("HX-01 需求草稿");
    d.status = QStringLiteral("draft");
    d.version = 0;
    d.loadedKnown = true;

    d.certification.airworthinessClass = QString::fromUtf8("CS-25 大型飞机");
    d.certification.opsRules = QString::fromUtf8("CAT 商业运输");
    d.certification.amendment = QStringLiteral("Amendment 28");
    d.certification.noiseStandard = QStringLiteral("ICAO Annex 16 Ch.14");
    d.certification.emissionStandard = QStringLiteral("CAEP/8");
    d.certification.specialCondition = QString::fromUtf8("电推进：不适用");

    SrdMissionScenario main;
    main.id = QStringLiteral("M-001");
    main.name = QString::fromUtf8("主设计任务");
    main.missionType = QString::fromUtf8("客运运输");
    main.boundary = QString::fromUtf8("180 座 / 5,500 km");
    main.environment = QString::fromUtf8("标准日");
    main.status = QString::fromUtf8("已定义");
    main.crew = QStringLiteral("2 + 4");
    main.utilization = QStringLiteral("3,400 h");
    main.designLife = QStringLiteral("60,000 FC");
    main.airport.elevationM = 0;
    main.airport.elevationKnown = true;
    main.airport.runwayLengthM = 2800;
    main.airport.runwayLengthKnown = true;
    main.airport.takeoffFieldLimitM = 2500;
    main.airport.takeoffFieldLimitKnown = true;
    main.airport.landingFieldLimitM = 1800;
    main.airport.landingFieldLimitKnown = true;
    main.airport.climbGradientPercent = 2.4;
    main.airport.climbGradientKnown = true;
    main.segments.append(seg("SEG-01", "滑行/起飞", "离地", 0, true, 0.20, true));
    main.segments.append(seg("SEG-02", "爬升", "达到巡航高度", 1500, true, 0.35, true));
    main.segments.append(seg("SEG-03", "巡航", "达到设计航程", 11000, true, 0.78, true));
    main.segments.append(seg("SEG-04", "下降", "进近高度", 3000, true, 0.45, true));
    main.segments.append(seg("SEG-05", "备降", "200 nmi 改航", 3000, true, 0.40, true));
    main.segments.append(seg("SEG-06", "盘旋", "30 min 等待", 1500, true, 0.25, true));
    main.segments.append(seg("SEG-07", "着陆", "接地", 0, true, 0.18, true));
    d.missions.append(main);

    SrdMissionScenario hot;
    hot.id = QStringLiteral("M-002");
    hot.name = QString::fromUtf8("高温高原起飞");
    hot.missionType = QString::fromUtf8("客运运输");
    hot.boundary = QString::fromUtf8("满载 / 2,100 m机场");
    hot.environment = QStringLiteral("ISA+25°C");
    hot.status = QString::fromUtf8("已定义");
    hot.crew = QStringLiteral("2 + 4");
    hot.utilization = QStringLiteral("3,400 h");
    hot.designLife = QStringLiteral("60,000 FC");
    hot.airport.elevationM = 2100;
    hot.airport.elevationKnown = true;
    hot.airport.runwayLengthM = 3200;
    hot.airport.runwayLengthKnown = true;
    hot.airport.takeoffFieldLimitM = 2500;
    hot.airport.takeoffFieldLimitKnown = true;
    d.missions.append(hot);

    SrdMissionScenario altn;
    altn.id = QStringLiteral("M-003");
    altn.name = QString::fromUtf8("备降任务");
    altn.missionType = QString::fromUtf8("客运运输");
    altn.boundary = QString::fromUtf8("200 nmi + 30 min盘旋");
    altn.environment = QString::fromUtf8("标准日");
    altn.status = QString::fromUtf8("已定义");
    altn.crew = QStringLiteral("2 + 4");
    d.missions.append(altn);

    SrdMissionScenario shortHaul;
    shortHaul.id = QStringLiteral("M-004");
    shortHaul.name = QString::fromUtf8("短程高频运营");
    shortHaul.missionType = QString::fromUtf8("客运运输");
    shortHaul.boundary = QString::fromUtf8("180 座 / 900 km");
    shortHaul.environment = QString::fromUtf8("年利用率 3,400 h");
    shortHaul.status = QString::fromUtf8("草案");
    shortHaul.utilization = QStringLiteral("3,400 h");
    d.missions.append(shortHaul);

    d.needs.append(need("NEED-001", "最大设计航程", "designRange_km", ">=", 5500, "km", "强制", "市场需求"));
    d.needs.append(need("NEED-002", "设计商载", "designPayload_kg", ">=", 18000, "kg", "强制", "市场需求"));
    d.needs.append(need("NEED-003", "巡航速度", "cruiseMach", ">=", 0.78, "Ma", "目标", "设计目标"));
    d.needs.append(need("NEED-004", "任务燃油", "missionFuel_kg", "<=", 15500, "kg", "目标", "运营方输入"));
    d.needs.append(need("NEED-005", "直接运营成本", "docDelta_percent", "<=", -8, "%", "期望", "设计目标"));

    d.environment.atmosphereModel = QStringLiteral("US Standard 1976");
    d.environment.temperatureOffset = QStringLiteral("ISA +15°C");
    d.environment.crosswindLimit = QStringLiteral("20 kt");
    d.environment.runwaySlope = QString::fromUtf8("±2.0 %");
    d.environment.gustModel = QStringLiteral("1-cos");
    d.environment.icingCondition = QString::fromUtf8("附录 C");
    d.environment.nzMin = -1.0;
    d.environment.nzMax = 2.5;
    d.environment.nzKnown = true;

    d.envelopePoints.append({0.18, 0.0});
    d.envelopePoints.append({0.20, 0.0});
    d.envelopePoints.append({0.35, 1.5});
    d.envelopePoints.append({0.72, 7.5});
    d.envelopePoints.append({0.78, 11.0});
    d.envelopePoints.append({0.82, 11.0});
    d.envelopePoints.append({0.70, 4.0});

    d.conditions.append(fc("FC-01", "起飞", "0 m", "0.20 Ma", "襟翼 15°", "ISA+15", "M-001"));
    d.conditions.append(fc("FC-02", "初始爬升", "1,500 m", "0.35 Ma", "清洁", "ISA+15", "M-001"));
    d.conditions.append(fc("FC-03", "巡航", "11,000 m", "0.78 Ma", "清洁", "ISA", "M-001"));
    d.conditions.append(fc("FC-04", "最大机动", "7,500 m", "0.72 Ma", "清洁", "ISA", "M-001"));
    d.conditions.append(fc("FC-05", "着陆进近", "0 m", "0.18 Ma", "全襟翼", "ISA+15", "M-001"));
    d.conditions.append(fc("FC-06", "高原起飞", "2,100 m", "0.20 Ma", "襟翼 15°", "ISA+25", "M-002"));
    d.conditions.append(fc("FC-07", "盘旋", "1,500 m", "0.25 Ma", "清洁", "ISA", "M-003"));

    const QVector<SrdCatalogClause> catalog = clauses();
    for (int i = 0; i < catalog.size(); ++i) {
        SrdClauseRow row;
        row.clauseId = catalog[i].clauseId;
        row.topic = catalog[i].topic;
        row.domain = catalog[i].domain;
        if (catalog[i].electricPropulsionRelated) {
            row.applicability = QString::fromUtf8("不适用");
            row.mappingStatus = QString::fromUtf8("不适用");
        } else {
            row.applicability = QString::fromUtf8("直接适用");
            row.mappingStatus = QString::fromUtf8("待确认");
        }
        d.clauses.append(row);
    }

    d.requirements.append(req("REQ-PERF-001", "designRange_km", ">=", 5500, "强制", "任务 M-001", "已验证", "分析", "任务 M-001", "4 个", "Run-240904"));
    d.requirements.append(req("REQ-PERF-105", "takeoffFieldLength_m", "<=", 2500, "强制", "CS-25.105", "已分配", "分析", "CS-25.105", "2 个", ""));
    d.requirements.append(req("REQ-PERF-125", "landingFieldLength_m", "<=", 1800, "强制", "CS-25.125", "已分配", "分析", "CS-25.125", "1 个", ""));
    d.requirements.append(req("REQ-PERF-121", "climbGradient_percent", ">=", 2.4, "强制", "CS-25.121", "已分配", "分析", "CS-25.121", "2 个", ""));
    d.requirements.append(req("REQ-WGT-004", "mtow_kg", "<=", 72000, "强制", "市场约束", "已分配", "分析", "市场约束", "2 个", ""));
    d.requirements.append(req("REQ-AERO-007", "cruiseLD", ">=", 18.5, "目标", "效率目标", "已分配", "分析", "效率目标", "3 个", ""));
    d.requirements.append(req("REQ-STR-011", "bucklingMargin", ">=", 0.15, "强制", "CS-25.301", "已分配", "分析", "CS-25.301", "2 个", ""));
    d.requirements.append(req("REQ-DYN-006", "shortPeriodDamping", ">=", 0.30, "强制", "CS-25.181", "待验证", "分析", "CS-25.181", "1 个", ""));
    d.requirements.append(req("REQ-DYN-171", "staticMargin_percentMac", ">=", 5.0, "强制", "CS-25.171", "已分配", "分析", "CS-25.171", "1 个", ""));
    d.requirements.append(req("REQ-PERF-237", "crosswindLimit_kt", ">=", 20, "强制", "CS-25.237", "已分配", "分析", "CS-25.237", "1 个", ""));
    d.requirements.append(req("REQ-COST-003", "docDelta_percent", "<=", -8, "期望", "商业目标", "草案", "分析", "商业目标", "1 个", ""));

    for (int i = 0; i < d.clauses.size(); ++i) {
        const QString id = d.clauses[i].clauseId;
        if (id == QLatin1String("CS-25.105") || id == QLatin1String("CS-25.113")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-PERF-105");
        } else if (id == QLatin1String("CS-25.125")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-PERF-125");
        } else if (id == QLatin1String("CS-25.121")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-PERF-121");
        } else if (id == QLatin1String("CS-25.143")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
        } else if (id == QLatin1String("CS-25.171") || id == QLatin1String("CS-25.173")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-DYN-171");
        } else if (id == QLatin1String("CS-25.181")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-DYN-006");
        } else if (id == QLatin1String("CS-25.237")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-PERF-237");
        } else if (id == QLatin1String("CS-25.301")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("已映射");
            d.clauses[i].mappedReqId = QStringLiteral("REQ-STR-011");
        } else if (id == QLatin1String("CS-25.331")) {
            d.clauses[i].mappingStatus = QString::fromUtf8("待确认");
        }
    }

    return d;
}
