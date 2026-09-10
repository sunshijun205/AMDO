#include "model/srdstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static const char kSchema[] = "amdo.srd.v1";

static void setError(QString *errorMessage, const QString &text)
{
    if (errorMessage)
        *errorMessage = text;
}

static QJsonObject knownDouble(double value, bool known)
{
    QJsonObject o;
    o.insert(QStringLiteral("value"), value);
    o.insert(QStringLiteral("known"), known);
    return o;
}

static void readKnownDouble(const QJsonObject &parent, const QString &key, double *value, bool *known)
{
    if (!parent.contains(key)) {
        *known = false;
        return;
    }
    const QJsonValue v = parent.value(key);
    if (v.isObject()) {
        const QJsonObject o = v.toObject();
        *value = o.value(QStringLiteral("value")).toDouble();
        *known = o.value(QStringLiteral("known")).toBool(true);
        return;
    }
    if (v.isDouble()) {
        *value = v.toDouble();
        *known = true;
    }
}

static QJsonObject airportToJson(const SrdAirportOps &a)
{
    QJsonObject o;
    o.insert(QStringLiteral("elevationM"), knownDouble(a.elevationM, a.elevationKnown));
    o.insert(QStringLiteral("runwayLengthM"), knownDouble(a.runwayLengthM, a.runwayLengthKnown));
    o.insert(QStringLiteral("takeoffFieldLimitM"), knownDouble(a.takeoffFieldLimitM, a.takeoffFieldLimitKnown));
    o.insert(QStringLiteral("landingFieldLimitM"), knownDouble(a.landingFieldLimitM, a.landingFieldLimitKnown));
    o.insert(QStringLiteral("climbGradientPercent"), knownDouble(a.climbGradientPercent, a.climbGradientKnown));
    return o;
}

static SrdAirportOps airportFromJson(const QJsonObject &o)
{
    SrdAirportOps a;
    readKnownDouble(o, QStringLiteral("elevationM"), &a.elevationM, &a.elevationKnown);
    readKnownDouble(o, QStringLiteral("runwayLengthM"), &a.runwayLengthM, &a.runwayLengthKnown);
    readKnownDouble(o, QStringLiteral("takeoffFieldLimitM"), &a.takeoffFieldLimitM, &a.takeoffFieldLimitKnown);
    readKnownDouble(o, QStringLiteral("landingFieldLimitM"), &a.landingFieldLimitM, &a.landingFieldLimitKnown);
    readKnownDouble(o, QStringLiteral("climbGradientPercent"), &a.climbGradientPercent, &a.climbGradientKnown);
    return a;
}

static QJsonObject segmentToJson(const SrdMissionSegment &s)
{
    QJsonObject o;
    o.insert(QStringLiteral("id"), s.id);
    o.insert(QStringLiteral("name"), s.name);
    o.insert(QStringLiteral("endCondition"), s.endCondition);
    o.insert(QStringLiteral("altitudeM"), knownDouble(s.altitudeM, s.altitudeKnown));
    o.insert(QStringLiteral("mach"), knownDouble(s.mach, s.machKnown));
    return o;
}

static SrdMissionSegment segmentFromJson(const QJsonObject &o)
{
    SrdMissionSegment s;
    s.id = o.value(QStringLiteral("id")).toString();
    s.name = o.value(QStringLiteral("name")).toString();
    s.endCondition = o.value(QStringLiteral("endCondition")).toString();
    readKnownDouble(o, QStringLiteral("altitudeM"), &s.altitudeM, &s.altitudeKnown);
    readKnownDouble(o, QStringLiteral("mach"), &s.mach, &s.machKnown);
    return s;
}

static QJsonObject scenarioToJson(const SrdMissionScenario &s)
{
    QJsonObject o;
    o.insert(QStringLiteral("id"), s.id);
    o.insert(QStringLiteral("name"), s.name);
    o.insert(QStringLiteral("missionType"), s.missionType);
    o.insert(QStringLiteral("boundary"), s.boundary);
    o.insert(QStringLiteral("environment"), s.environment);
    o.insert(QStringLiteral("status"), s.status);
    o.insert(QStringLiteral("crew"), s.crew);
    o.insert(QStringLiteral("utilization"), s.utilization);
    o.insert(QStringLiteral("designLife"), s.designLife);
    o.insert(QStringLiteral("airport"), airportToJson(s.airport));
    QJsonArray segs;
    for (int i = 0; i < s.segments.size(); ++i)
        segs.append(segmentToJson(s.segments[i]));
    o.insert(QStringLiteral("segments"), segs);
    return o;
}

static SrdMissionScenario scenarioFromJson(const QJsonObject &o)
{
    SrdMissionScenario s;
    s.id = o.value(QStringLiteral("id")).toString();
    s.name = o.value(QStringLiteral("name")).toString();
    s.missionType = o.value(QStringLiteral("missionType")).toString();
    s.boundary = o.value(QStringLiteral("boundary")).toString();
    s.environment = o.value(QStringLiteral("environment")).toString();
    s.status = o.value(QStringLiteral("status")).toString();
    s.crew = o.value(QStringLiteral("crew")).toString();
    s.utilization = o.value(QStringLiteral("utilization")).toString();
    s.designLife = o.value(QStringLiteral("designLife")).toString();
    s.airport = airportFromJson(o.value(QStringLiteral("airport")).toObject());
    const QJsonArray segs = o.value(QStringLiteral("segments")).toArray();
    for (int i = 0; i < segs.size(); ++i)
        s.segments.append(segmentFromJson(segs.at(i).toObject()));
    return s;
}

static QJsonObject needToJson(const SrdPerformanceNeed &n)
{
    QJsonObject o;
    o.insert(QStringLiteral("id"), n.id);
    o.insert(QStringLiteral("name"), n.name);
    o.insert(QStringLiteral("metricId"), n.metricId);
    o.insert(QStringLiteral("relation"), n.relation);
    o.insert(QStringLiteral("value"), knownDouble(n.value, n.valueKnown));
    o.insert(QStringLiteral("unit"), n.unit);
    o.insert(QStringLiteral("grade"), n.grade);
    o.insert(QStringLiteral("sourceKind"), n.sourceKind);
    return o;
}

static SrdPerformanceNeed needFromJson(const QJsonObject &o)
{
    SrdPerformanceNeed n;
    n.id = o.value(QStringLiteral("id")).toString();
    n.name = o.value(QStringLiteral("name")).toString();
    n.metricId = o.value(QStringLiteral("metricId")).toString();
    n.relation = o.value(QStringLiteral("relation")).toString();
    readKnownDouble(o, QStringLiteral("value"), &n.value, &n.valueKnown);
    n.unit = o.value(QStringLiteral("unit")).toString();
    n.grade = o.value(QStringLiteral("grade")).toString();
    n.sourceKind = o.value(QStringLiteral("sourceKind")).toString();
    return n;
}

static QJsonObject envToJson(const SrdEnvironment &e)
{
    QJsonObject o;
    o.insert(QStringLiteral("atmosphereModel"), e.atmosphereModel);
    o.insert(QStringLiteral("temperatureOffset"), e.temperatureOffset);
    o.insert(QStringLiteral("crosswindLimit"), e.crosswindLimit);
    o.insert(QStringLiteral("runwaySlope"), e.runwaySlope);
    o.insert(QStringLiteral("gustModel"), e.gustModel);
    o.insert(QStringLiteral("icingCondition"), e.icingCondition);
    o.insert(QStringLiteral("nzMin"), e.nzMin);
    o.insert(QStringLiteral("nzMax"), e.nzMax);
    o.insert(QStringLiteral("nzKnown"), e.nzKnown);
    return o;
}

static SrdEnvironment envFromJson(const QJsonObject &o)
{
    SrdEnvironment e;
    e.atmosphereModel = o.value(QStringLiteral("atmosphereModel")).toString();
    e.temperatureOffset = o.value(QStringLiteral("temperatureOffset")).toString();
    e.crosswindLimit = o.value(QStringLiteral("crosswindLimit")).toString();
    e.runwaySlope = o.value(QStringLiteral("runwaySlope")).toString();
    e.gustModel = o.value(QStringLiteral("gustModel")).toString();
    e.icingCondition = o.value(QStringLiteral("icingCondition")).toString();
    if (o.contains(QStringLiteral("nzMin")))
        e.nzMin = o.value(QStringLiteral("nzMin")).toDouble(-1.0);
    if (o.contains(QStringLiteral("nzMax")))
        e.nzMax = o.value(QStringLiteral("nzMax")).toDouble(2.5);
    e.nzKnown = o.value(QStringLiteral("nzKnown")).toBool(true);
    return e;
}

static QJsonObject documentToJson(const SrdDocument &d)
{
    QJsonObject o;
    o.insert(QStringLiteral("schemaVersion"), d.schemaVersion.isEmpty()
             ? QString::fromUtf8(kSchema) : d.schemaVersion);
    o.insert(QStringLiteral("id"), d.id);
    o.insert(QStringLiteral("title"), d.title);
    o.insert(QStringLiteral("status"), d.status);
    o.insert(QStringLiteral("version"), d.version);
    o.insert(QStringLiteral("publishedAt"), d.publishedAt);

    QJsonObject chapters;
    chapters.insert(QStringLiteral("missionFrozen"), d.chapters.missionFrozen);
    chapters.insert(QStringLiteral("envelopeFrozen"), d.chapters.envelopeFrozen);
    chapters.insert(QStringLiteral("standardsFrozen"), d.chapters.standardsFrozen);
    chapters.insert(QStringLiteral("metricsFrozen"), d.chapters.metricsFrozen);
    o.insert(QStringLiteral("chapters"), chapters);

    QJsonObject cert;
    cert.insert(QStringLiteral("airworthinessClass"), d.certification.airworthinessClass);
    cert.insert(QStringLiteral("opsRules"), d.certification.opsRules);
    cert.insert(QStringLiteral("amendment"), d.certification.amendment);
    cert.insert(QStringLiteral("noiseStandard"), d.certification.noiseStandard);
    cert.insert(QStringLiteral("emissionStandard"), d.certification.emissionStandard);
    cert.insert(QStringLiteral("specialCondition"), d.certification.specialCondition);
    o.insert(QStringLiteral("certification"), cert);

    QJsonArray missions;
    for (int i = 0; i < d.missions.size(); ++i)
        missions.append(scenarioToJson(d.missions[i]));
    o.insert(QStringLiteral("missions"), missions);

    QJsonArray needs;
    for (int i = 0; i < d.needs.size(); ++i)
        needs.append(needToJson(d.needs[i]));
    o.insert(QStringLiteral("needs"), needs);

    QJsonArray points;
    for (int i = 0; i < d.envelopePoints.size(); ++i) {
        QJsonObject p;
        p.insert(QStringLiteral("mach"), d.envelopePoints[i].mach);
        p.insert(QStringLiteral("altitudeKm"), d.envelopePoints[i].altitudeKm);
        points.append(p);
    }
    o.insert(QStringLiteral("envelopePoints"), points);
    o.insert(QStringLiteral("environment"), envToJson(d.environment));

    QJsonArray conditions;
    for (int i = 0; i < d.conditions.size(); ++i) {
        QJsonObject c;
        c.insert(QStringLiteral("id"), d.conditions[i].id);
        c.insert(QStringLiteral("phase"), d.conditions[i].phase);
        c.insert(QStringLiteral("altitude"), d.conditions[i].altitude);
        c.insert(QStringLiteral("speed"), d.conditions[i].speed);
        c.insert(QStringLiteral("configuration"), d.conditions[i].configuration);
        c.insert(QStringLiteral("atmosphere"), d.conditions[i].atmosphere);
        c.insert(QStringLiteral("sourceScenarioId"), d.conditions[i].sourceScenarioId);
        conditions.append(c);
    }
    o.insert(QStringLiteral("conditions"), conditions);

    QJsonArray clauses;
    for (int i = 0; i < d.clauses.size(); ++i) {
        QJsonObject c;
        c.insert(QStringLiteral("clauseId"), d.clauses[i].clauseId);
        c.insert(QStringLiteral("topic"), d.clauses[i].topic);
        c.insert(QStringLiteral("domain"), d.clauses[i].domain);
        c.insert(QStringLiteral("applicability"), d.clauses[i].applicability);
        c.insert(QStringLiteral("mappingStatus"), d.clauses[i].mappingStatus);
        c.insert(QStringLiteral("mappedReqId"), d.clauses[i].mappedReqId);
        clauses.append(c);
    }
    o.insert(QStringLiteral("clauses"), clauses);

    QJsonArray reqs;
    for (int i = 0; i < d.requirements.size(); ++i) {
        const SrdRequirement &r = d.requirements[i];
        QJsonObject c;
        c.insert(QStringLiteral("id"), r.id);
        c.insert(QStringLiteral("metricId"), r.metricId);
        c.insert(QStringLiteral("metricName"), r.metricName);
        c.insert(QStringLiteral("domain"), r.domain);
        c.insert(QStringLiteral("relation"), r.relation);
        c.insert(QStringLiteral("boundValue"), knownDouble(r.boundValue, r.boundKnown));
        c.insert(QStringLiteral("unit"), r.unit);
        c.insert(QStringLiteral("grade"), r.grade);
        c.insert(QStringLiteral("constraintKind"), r.constraintKind);
        c.insert(QStringLiteral("source"), r.source);
        c.insert(QStringLiteral("status"), r.status);
        c.insert(QStringLiteral("priority"), r.priority);
        c.insert(QStringLiteral("verificationMethod"), r.verificationMethod);
        c.insert(QStringLiteral("upstream"), r.upstream);
        c.insert(QStringLiteral("downstreamVars"), r.downstreamVars);
        c.insert(QStringLiteral("analysisResponseId"), r.analysisResponseId);
        c.insert(QStringLiteral("evidenceRun"), r.evidenceRun);
        reqs.append(c);
    }
    o.insert(QStringLiteral("requirements"), reqs);
    return o;
}

static bool documentFromJson(const QJsonObject &o, SrdDocument *out, QString *errorMessage)
{
    const QString schema = o.value(QStringLiteral("schemaVersion")).toString();
    if (!schema.isEmpty() && schema != QLatin1String(kSchema)) {
        setError(errorMessage, QString::fromUtf8("不支持的 SRD schema：%1").arg(schema));
        return false;
    }

    out->schemaVersion = schema.isEmpty() ? QString::fromUtf8(kSchema) : schema;
    out->id = o.value(QStringLiteral("id")).toString();
    out->title = o.value(QStringLiteral("title")).toString();
    out->status = o.value(QStringLiteral("status")).toString();
    if (out->status.isEmpty())
        out->status = QStringLiteral("draft");
    out->version = o.value(QStringLiteral("version")).toInt(0);
    out->publishedAt = o.value(QStringLiteral("publishedAt")).toString();

    const QJsonObject chapters = o.value(QStringLiteral("chapters")).toObject();
    out->chapters.missionFrozen = chapters.value(QStringLiteral("missionFrozen")).toBool();
    out->chapters.envelopeFrozen = chapters.value(QStringLiteral("envelopeFrozen")).toBool();
    out->chapters.standardsFrozen = chapters.value(QStringLiteral("standardsFrozen")).toBool();
    out->chapters.metricsFrozen = chapters.value(QStringLiteral("metricsFrozen")).toBool();

    const QJsonObject cert = o.value(QStringLiteral("certification")).toObject();
    out->certification.airworthinessClass = cert.value(QStringLiteral("airworthinessClass")).toString();
    out->certification.opsRules = cert.value(QStringLiteral("opsRules")).toString();
    out->certification.amendment = cert.value(QStringLiteral("amendment")).toString();
    out->certification.noiseStandard = cert.value(QStringLiteral("noiseStandard")).toString();
    out->certification.emissionStandard = cert.value(QStringLiteral("emissionStandard")).toString();
    out->certification.specialCondition = cert.value(QStringLiteral("specialCondition")).toString();

    out->missions.clear();
    const QJsonArray missions = o.value(QStringLiteral("missions")).toArray();
    for (int i = 0; i < missions.size(); ++i)
        out->missions.append(scenarioFromJson(missions.at(i).toObject()));

    out->needs.clear();
    const QJsonArray needs = o.value(QStringLiteral("needs")).toArray();
    for (int i = 0; i < needs.size(); ++i)
        out->needs.append(needFromJson(needs.at(i).toObject()));

    out->envelopePoints.clear();
    const QJsonArray points = o.value(QStringLiteral("envelopePoints")).toArray();
    for (int i = 0; i < points.size(); ++i) {
        const QJsonObject p = points.at(i).toObject();
        SrdEnvelopePoint pt;
        pt.mach = p.value(QStringLiteral("mach")).toDouble();
        pt.altitudeKm = p.value(QStringLiteral("altitudeKm")).toDouble();
        out->envelopePoints.append(pt);
    }

    out->environment = envFromJson(o.value(QStringLiteral("environment")).toObject());

    out->conditions.clear();
    const QJsonArray conditions = o.value(QStringLiteral("conditions")).toArray();
    for (int i = 0; i < conditions.size(); ++i) {
        const QJsonObject c = conditions.at(i).toObject();
        SrdFlightCondition fc;
        fc.id = c.value(QStringLiteral("id")).toString();
        fc.phase = c.value(QStringLiteral("phase")).toString();
        fc.altitude = c.value(QStringLiteral("altitude")).toString();
        fc.speed = c.value(QStringLiteral("speed")).toString();
        fc.configuration = c.value(QStringLiteral("configuration")).toString();
        fc.atmosphere = c.value(QStringLiteral("atmosphere")).toString();
        fc.sourceScenarioId = c.value(QStringLiteral("sourceScenarioId")).toString();
        out->conditions.append(fc);
    }

    out->clauses.clear();
    const QJsonArray clauses = o.value(QStringLiteral("clauses")).toArray();
    for (int i = 0; i < clauses.size(); ++i) {
        const QJsonObject c = clauses.at(i).toObject();
        SrdClauseRow row;
        row.clauseId = c.value(QStringLiteral("clauseId")).toString();
        row.topic = c.value(QStringLiteral("topic")).toString();
        row.domain = c.value(QStringLiteral("domain")).toString();
        row.applicability = c.value(QStringLiteral("applicability")).toString();
        row.mappingStatus = c.value(QStringLiteral("mappingStatus")).toString();
        row.mappedReqId = c.value(QStringLiteral("mappedReqId")).toString();
        out->clauses.append(row);
    }

    out->requirements.clear();
    const QJsonArray reqs = o.value(QStringLiteral("requirements")).toArray();
    for (int i = 0; i < reqs.size(); ++i) {
        const QJsonObject c = reqs.at(i).toObject();
        SrdRequirement r;
        r.id = c.value(QStringLiteral("id")).toString();
        r.metricId = c.value(QStringLiteral("metricId")).toString();
        r.metricName = c.value(QStringLiteral("metricName")).toString();
        r.domain = c.value(QStringLiteral("domain")).toString();
        r.relation = c.value(QStringLiteral("relation")).toString();
        readKnownDouble(c, QStringLiteral("boundValue"), &r.boundValue, &r.boundKnown);
        r.unit = c.value(QStringLiteral("unit")).toString();
        r.grade = c.value(QStringLiteral("grade")).toString();
        r.constraintKind = c.value(QStringLiteral("constraintKind")).toString();
        if (r.constraintKind.isEmpty())
            r.constraintKind = srdGradeToKind(r.grade);
        r.source = c.value(QStringLiteral("source")).toString();
        r.status = c.value(QStringLiteral("status")).toString();
        r.priority = c.value(QStringLiteral("priority")).toString();
        r.verificationMethod = c.value(QStringLiteral("verificationMethod")).toString();
        r.upstream = c.value(QStringLiteral("upstream")).toString();
        r.downstreamVars = c.value(QStringLiteral("downstreamVars")).toString();
        r.analysisResponseId = c.value(QStringLiteral("analysisResponseId")).toString();
        r.evidenceRun = c.value(QStringLiteral("evidenceRun")).toString();
        out->requirements.append(r);
    }

    out->loadedKnown = true;
    return true;
}

QString SrdStore::rootDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath();
    dir += QLatin1String("/srd");
    QDir().mkpath(dir);
    return dir;
}

QString SrdStore::draftPath() const
{
    return rootDir() + QLatin1String("/srd_draft.json");
}

QString SrdStore::baselinesDir() const
{
    const QString dir = rootDir() + QLatin1String("/baselines");
    QDir().mkpath(dir);
    return dir;
}

bool SrdStore::readDocumentFile(const QString &path, SrdDocument *out, QString *errorMessage) const
{
    if (!out) {
        setError(errorMessage, QString::fromUtf8("内部错误：输出参数为空"));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(errorMessage, QString::fromUtf8("无法打开文件：%1").arg(file.errorString()));
        out->loadedKnown = false;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        setError(errorMessage, QString::fromUtf8("JSON 解析失败：%1").arg(parseError.errorString()));
        out->loadedKnown = false;
        return false;
    }

    return documentFromJson(doc.object(), out, errorMessage);
}

bool SrdStore::writeDocumentFile(const QString &path, const SrdDocument &doc, QString *errorMessage) const
{
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(errorMessage, QString::fromUtf8("无法写入文件：%1").arg(file.errorString()));
        return false;
    }

    const QJsonDocument json(documentToJson(doc));
    const QByteArray bytes = json.toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size()) {
        setError(errorMessage, QString::fromUtf8("写入不完整：%1").arg(file.errorString()));
        return false;
    }
    return true;
}

bool SrdStore::loadDraft(SrdDocument *out, QString *errorMessage) const
{
    if (!out) {
        setError(errorMessage, QString::fromUtf8("内部错误：输出参数为空"));
        return false;
    }

    const QString path = draftPath();
    if (!QFile::exists(path)) {
        out->loadedKnown = true;
        out->id.clear();
        return true;
    }
    return readDocumentFile(path, out, errorMessage);
}

bool SrdStore::saveDraft(const SrdDocument &doc, QString *errorMessage) const
{
    return writeDocumentFile(draftPath(), doc, errorMessage);
}

bool SrdStore::saveBaseline(const SrdDocument &doc, QString *errorMessage) const
{
    if (doc.id.isEmpty()) {
        setError(errorMessage, QString::fromUtf8("基线 id 为空"));
        return false;
    }
    const QString path = baselinesDir() + QLatin1Char('/') + doc.id + QLatin1String(".json");
    return writeDocumentFile(path, doc, errorMessage);
}

bool SrdStore::loadBaseline(const QString &id, SrdDocument *out, QString *errorMessage) const
{
    if (id.isEmpty()) {
        setError(errorMessage, QString::fromUtf8("基线 id 为空"));
        return false;
    }
    const QString path = baselinesDir() + QLatin1Char('/') + id + QLatin1String(".json");
    if (!QFile::exists(path)) {
        setError(errorMessage, QString::fromUtf8("找不到基线文件：%1").arg(path));
        return false;
    }
    return readDocumentFile(path, out, errorMessage);
}

bool SrdStore::listBaselines(QVector<SrdBaselineInfo> *out, QString *errorMessage) const
{
    if (!out) {
        setError(errorMessage, QString::fromUtf8("内部错误：输出参数为空"));
        return false;
    }
    out->clear();

    QDir dir(baselinesDir());
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
    for (int i = 0; i < files.size(); ++i) {
        SrdDocument doc;
        QString detail;
        const QString path = dir.absoluteFilePath(files[i]);
        if (!readDocumentFile(path, &doc, &detail)) {
            setError(errorMessage, QString::fromUtf8("读取基线失败：%1").arg(detail));
            return false;
        }
        SrdBaselineInfo info;
        info.id = doc.id.isEmpty() ? QFileInfo(files[i]).completeBaseName() : doc.id;
        info.title = doc.title;
        info.version = doc.version;
        info.publishedAt = doc.publishedAt;
        info.filePath = path;
        out->append(info);
    }
    return true;
}
