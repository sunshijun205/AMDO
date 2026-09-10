#include "service/srdderivationservice.h"

#include "model/srdcatalogs.h"

#include <QSet>

static QString formatAlt(double meters)
{
    if (meters >= 1000.0)
        return srdFormatNumber(meters) + QStringLiteral(" m");
    return srdFormatNumber(meters) + QStringLiteral(" m");
}

static QString formatMach(double mach)
{
    return srdFormatNumber(mach) + QStringLiteral(" Ma");
}

static SrdFlightCondition makeFc(const QString &phase, const QString &alt, const QString &speed,
                                 const QString &config, const QString &atm, const QString &src)
{
    SrdFlightCondition c;
    c.phase = phase;
    c.altitude = alt;
    c.speed = speed;
    c.configuration = config;
    c.atmosphere = atm;
    c.sourceScenarioId = src;
    return c;
}

static bool hasSame(const QVector<SrdFlightCondition> &list, const SrdFlightCondition &c)
{
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].phase == c.phase && list[i].altitude == c.altitude
            && list[i].speed == c.speed && list[i].configuration == c.configuration)
            return true;
    }
    return false;
}

static void addUnique(QVector<SrdFlightCondition> *out, const SrdFlightCondition &c)
{
    if (!hasSame(*out, c))
        out->append(c);
}

QVector<SrdFlightCondition> SrdDerivationService::suggestConditions(const SrdDocument &doc) const
{
    QVector<SrdFlightCondition> fresh;
    for (int i = 0; i < doc.missions.size(); ++i) {
        const SrdMissionScenario &s = doc.missions[i];
        const bool primary = (s.id == QLatin1String("M-001"))
            || s.name.contains(QString::fromUtf8("主设计"));
        const bool plateau = s.name.contains(QString::fromUtf8("高原"))
            || (s.airport.elevationKnown && s.airport.elevationM >= 1000.0);
        const bool alternate = s.name.contains(QString::fromUtf8("备降"));

        QString takeoffAtm = QStringLiteral("ISA+15");
        if (s.environment.contains(QStringLiteral("ISA+25")))
            takeoffAtm = QStringLiteral("ISA+25");

        if (primary) {
            double cruiseAlt = 11000;
            double cruiseMach = 0.78;
            for (int k = 0; k < s.segments.size(); ++k) {
                if (s.segments[k].name.contains(QString::fromUtf8("巡航"))) {
                    if (s.segments[k].altitudeKnown)
                        cruiseAlt = s.segments[k].altitudeM;
                    if (s.segments[k].machKnown)
                        cruiseMach = s.segments[k].mach;
                }
            }
            addUnique(&fresh, makeFc(QString::fromUtf8("起飞"), QStringLiteral("0 m"),
                                     QStringLiteral("0.20 Ma"), QString::fromUtf8("襟翼 15°"),
                                     takeoffAtm, s.id));
            addUnique(&fresh, makeFc(QString::fromUtf8("初始爬升"), QStringLiteral("1,500 m"),
                                     QStringLiteral("0.35 Ma"), QString::fromUtf8("清洁"),
                                     takeoffAtm, s.id));
            addUnique(&fresh, makeFc(QString::fromUtf8("巡航"), formatAlt(cruiseAlt),
                                     formatMach(cruiseMach), QString::fromUtf8("清洁"),
                                     QStringLiteral("ISA"), s.id));
            addUnique(&fresh, makeFc(QString::fromUtf8("着陆进近"), QStringLiteral("0 m"),
                                     QStringLiteral("0.18 Ma"), QString::fromUtf8("全襟翼"),
                                     takeoffAtm, s.id));
        }

        if (plateau) {
            const double elev = s.airport.elevationKnown ? s.airport.elevationM : 2100;
            addUnique(&fresh, makeFc(QString::fromUtf8("起飞"), formatAlt(elev),
                                     QStringLiteral("0.20 Ma"), QString::fromUtf8("襟翼 15°"),
                                     QStringLiteral("ISA+25"), s.id));
        }

        if (alternate) {
            addUnique(&fresh, makeFc(QString::fromUtf8("盘旋"), QStringLiteral("1,500 m"),
                                     QStringLiteral("0.25 Ma"), QString::fromUtf8("清洁"),
                                     QStringLiteral("ISA"), s.id));
            addUnique(&fresh, makeFc(QString::fromUtf8("着陆进近"), QStringLiteral("0 m"),
                                     QStringLiteral("0.18 Ma"), QString::fromUtf8("全襟翼"),
                                     QStringLiteral("ISA"), s.id));
        }
    }

    QVector<SrdFlightCondition> result;
    for (int i = 0; i < fresh.size(); ++i) {
        if (!hasSame(doc.conditions, fresh[i]))
            result.append(fresh[i]);
    }
    return result;
}

static bool parseAltitudeKm(const QString &text, double *km)
{
    QString t = text;
    t.remove(QLatin1Char(','));
    t.replace(QString::fromUtf8("千米"), QString());
    bool isKm = t.contains(QStringLiteral("km"), Qt::CaseInsensitive);
    t.replace(QStringLiteral("km"), QString(), Qt::CaseInsensitive);
    t.replace(QStringLiteral("m"), QString(), Qt::CaseInsensitive);
    t = t.trimmed();
    bool ok = false;
    const double v = t.toDouble(&ok);
    if (!ok)
        return false;
    *km = isKm ? v : v / 1000.0;
    return true;
}

static bool parseMach(const QString &text, double *mach)
{
    QString t = text;
    t.replace(QStringLiteral("Ma"), QString(), Qt::CaseInsensitive);
    t = t.trimmed();
    bool ok = false;
    *mach = t.toDouble(&ok);
    return ok;
}

QVector<SrdEnvelopePoint> SrdDerivationService::suggestEnvelopePoints(const SrdDocument &doc) const
{
    QVector<SrdEnvelopePoint> pts = doc.envelopePoints;
    if (pts.size() >= 3)
        return QVector<SrdEnvelopePoint>();

    QVector<SrdEnvelopePoint> derived;
    for (int i = 0; i < doc.conditions.size(); ++i) {
        SrdEnvelopePoint p;
        if (!parseMach(doc.conditions[i].speed, &p.mach))
            continue;
        if (!parseAltitudeKm(doc.conditions[i].altitude, &p.altitudeKm))
            continue;
        derived.append(p);
    }
    return derived;
}

QVector<SrdRequirement> SrdDerivationService::suggestRequirements(const SrdDocument &doc) const
{
    QVector<SrdRequirement> result;
    QSet<QString> have;
    for (int i = 0; i < doc.requirements.size(); ++i)
        have.insert(doc.requirements[i].metricId);

    auto appendFrom = [&](const QString &metricId, const QString &relation, double value, bool known,
                          const QString &grade, const QString &source, const QString &status) {
        if (metricId.isEmpty() || have.contains(metricId))
            return;
        SrdCatalogMetric metric;
        if (!SrdCatalogs::findMetric(metricId, &metric))
            return;
        SrdRequirement r;
        r.metricId = metric.id;
        r.metricName = metric.name;
        r.domain = metric.domain;
        r.unit = metric.unit;
        r.analysisResponseId = metric.responseId;
        r.relation = relation;
        r.boundValue = value;
        r.boundKnown = known;
        r.grade = grade;
        r.constraintKind = srdGradeToKind(grade);
        r.source = source;
        r.status = status;
        r.priority = QStringLiteral("P1");
        r.verificationMethod = QString::fromUtf8("分析");
        r.upstream = source;
        result.append(r);
        have.insert(metricId);
    };

    for (int i = 0; i < doc.needs.size(); ++i) {
        const SrdPerformanceNeed &n = doc.needs[i];
        appendFrom(n.metricId, n.relation, n.value, n.valueKnown, n.grade, n.name,
                   n.grade == QString::fromUtf8("强制") ? QString::fromUtf8("已分配")
                                                        : QString::fromUtf8("草案"));
    }

    for (int i = 0; i < doc.clauses.size(); ++i) {
        const SrdClauseRow &row = doc.clauses[i];
        if (row.applicability != QString::fromUtf8("直接适用")
            && row.applicability != QString::fromUtf8("条件适用"))
            continue;
        SrdCatalogClause cat;
        if (!SrdCatalogs::findClause(row.clauseId, &cat))
            continue;
        if (!cat.suggestedMetricId.isEmpty() && cat.suggestedValueKnown) {
            appendFrom(cat.suggestedMetricId, cat.suggestedRelation, cat.suggestedValue, true,
                       cat.suggestedGrade, row.clauseId, QString::fromUtf8("已分配"));
        }
    }

    return result;
}
