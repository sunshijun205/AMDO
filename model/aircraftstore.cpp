#include "model/aircraftstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QStandardPaths>

static const char kSchema[] = "amdo.aircraft.v1";

static void setError(QString *errorMessage, const QString &text)
{
    if (errorMessage)
        *errorMessage = text;
}

static QJsonObject semanticsToJson(const AcSemantics &s)
{
    QJsonObject o;
    o.insert(QStringLiteral("schema"), s.schema);
    o.insert(QStringLiteral("namespace"), s.namespaceStr);
    o.insert(QStringLiteral("referenceStrategy"), s.referenceStrategy);
    o.insert(QStringLiteral("exchangeFormat"), s.exchangeFormat);
    o.insert(QStringLiteral("lengthUnit"), s.lengthUnit);
    o.insert(QStringLiteral("massUnit"), s.massUnit);
    o.insert(QStringLiteral("coordinateSystem"), s.coordinateSystem);
    o.insert(QStringLiteral("origin"), s.origin);
    return o;
}

static AcSemantics semanticsFromJson(const QJsonObject &o)
{
    AcSemantics s;
    s.schema = o.value(QStringLiteral("schema")).toString();
    s.namespaceStr = o.value(QStringLiteral("namespace")).toString();
    s.referenceStrategy = o.value(QStringLiteral("referenceStrategy")).toString();
    s.exchangeFormat = o.value(QStringLiteral("exchangeFormat")).toString();
    s.lengthUnit = o.value(QStringLiteral("lengthUnit")).toString();
    s.massUnit = o.value(QStringLiteral("massUnit")).toString();
    s.coordinateSystem = o.value(QStringLiteral("coordinateSystem")).toString();
    s.origin = o.value(QStringLiteral("origin")).toString();
    return s;
}

static QJsonObject configToJson(const AcConfiguration &c)
{
    QJsonObject o;
    o.insert(QStringLiteral("category"), c.category);
    o.insert(QStringLiteral("wingLayout"), c.wingLayout);
    o.insert(QStringLiteral("tailConfig"), c.tailConfig);
    o.insert(QStringLiteral("engineArrangement"), c.engineArrangement);
    o.insert(QStringLiteral("gearType"), c.gearType);
    o.insert(QStringLiteral("cabinLayout"), c.cabinLayout);
    o.insert(QStringLiteral("primaryMaterial"), c.primaryMaterial);
    o.insert(QStringLiteral("connectionStrategy"), c.connectionStrategy);
    o.insert(QStringLiteral("symmetry"), c.symmetry);
    return o;
}

static AcConfiguration configFromJson(const QJsonObject &o)
{
    AcConfiguration c;
    c.category = o.value(QStringLiteral("category")).toString();
    c.wingLayout = o.value(QStringLiteral("wingLayout")).toString();
    c.tailConfig = o.value(QStringLiteral("tailConfig")).toString();
    c.engineArrangement = o.value(QStringLiteral("engineArrangement")).toString();
    c.gearType = o.value(QStringLiteral("gearType")).toString();
    c.cabinLayout = o.value(QStringLiteral("cabinLayout")).toString();
    c.primaryMaterial = o.value(QStringLiteral("primaryMaterial")).toString();
    c.connectionStrategy = o.value(QStringLiteral("connectionStrategy")).toString();
    c.symmetry = o.value(QStringLiteral("symmetry")).toString();
    return c;
}

static QJsonObject documentToJson(const AircraftDocument &d)
{
    QJsonObject o;
    o.insert(QStringLiteral("schemaVersion"), d.schemaVersion.isEmpty()
             ? QString::fromUtf8(kSchema) : d.schemaVersion);
    o.insert(QStringLiteral("id"), d.id);
    o.insert(QStringLiteral("title"), d.title);
    o.insert(QStringLiteral("status"), d.status);
    o.insert(QStringLiteral("version"), d.version);
    o.insert(QStringLiteral("publishedAt"), d.publishedAt);
    o.insert(QStringLiteral("semantics"), semanticsToJson(d.semantics));
    o.insert(QStringLiteral("configuration"), configToJson(d.configuration));

    QJsonArray systems;
    for (int i = 0; i < d.systems.size(); ++i) {
        QJsonObject s;
        s.insert(QStringLiteral("id"), d.systems[i].id);
        s.insert(QStringLiteral("object"), d.systems[i].object);
        s.insert(QStringLiteral("scheme"), d.systems[i].scheme);
        s.insert(QStringLiteral("mounting"), d.systems[i].mounting);
        s.insert(QStringLiteral("material"), d.systems[i].material);
        s.insert(QStringLiteral("status"), d.systems[i].status);
        systems.append(s);
    }
    o.insert(QStringLiteral("systems"), systems);

    QJsonArray params;
    for (int i = 0; i < d.parameters.size(); ++i) {
        const AcParameter &p = d.parameters[i];
        QJsonObject po;
        po.insert(QStringLiteral("id"), p.id);
        po.insert(QStringLiteral("name"), p.name);
        po.insert(QStringLiteral("symbol"), p.symbol);
        po.insert(QStringLiteral("value"), p.value);
        po.insert(QStringLiteral("valueKnown"), p.valueKnown);
        po.insert(QStringLiteral("unit"), p.unit);
        po.insert(QStringLiteral("driveType"), p.driveType);
        po.insert(QStringLiteral("note"), p.note);
        params.append(po);
    }
    o.insert(QStringLiteral("parameters"), params);
    return o;
}

static bool documentFromJson(const QJsonObject &o, AircraftDocument *out, QString *errorMessage)
{
    const QString schema = o.value(QStringLiteral("schemaVersion")).toString();
    if (!schema.isEmpty() && schema != QLatin1String(kSchema)) {
        setError(errorMessage, QString::fromUtf8("不支持的方案 schema：%1").arg(schema));
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
    out->semantics = semanticsFromJson(o.value(QStringLiteral("semantics")).toObject());
    out->configuration = configFromJson(o.value(QStringLiteral("configuration")).toObject());

    out->systems.clear();
    const QJsonArray systems = o.value(QStringLiteral("systems")).toArray();
    for (int i = 0; i < systems.size(); ++i) {
        const QJsonObject s = systems.at(i).toObject();
        AcSystemItem item;
        item.id = s.value(QStringLiteral("id")).toString();
        item.object = s.value(QStringLiteral("object")).toString();
        item.scheme = s.value(QStringLiteral("scheme")).toString();
        item.mounting = s.value(QStringLiteral("mounting")).toString();
        item.material = s.value(QStringLiteral("material")).toString();
        item.status = s.value(QStringLiteral("status")).toString();
        out->systems.append(item);
    }

    out->parameters.clear();
    const QJsonArray params = o.value(QStringLiteral("parameters")).toArray();
    for (int i = 0; i < params.size(); ++i) {
        const QJsonObject po = params.at(i).toObject();
        AcParameter p;
        p.id = po.value(QStringLiteral("id")).toString();
        p.name = po.value(QStringLiteral("name")).toString();
        p.symbol = po.value(QStringLiteral("symbol")).toString();
        if (po.contains(QStringLiteral("value"))) {
            p.value = po.value(QStringLiteral("value")).toDouble();
            p.valueKnown = po.value(QStringLiteral("valueKnown")).toBool(true);
        }
        p.unit = po.value(QStringLiteral("unit")).toString();
        p.driveType = po.value(QStringLiteral("driveType")).toString();
        p.note = po.value(QStringLiteral("note")).toString();
        out->parameters.append(p);
    }

    out->loadedKnown = true;
    return true;
}

QString AircraftStore::rootDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath();
    dir += QLatin1String("/aircraft");
    QDir().mkpath(dir);
    return dir;
}

QString AircraftStore::draftPath() const
{
    return rootDir() + QLatin1String("/aircraft_draft.json");
}

QString AircraftStore::baselinesDir() const
{
    const QString dir = rootDir() + QLatin1String("/baselines");
    QDir().mkpath(dir);
    return dir;
}

QString AircraftStore::cpacsDir() const
{
    const QString dir = rootDir() + QLatin1String("/cpacs");
    QDir().mkpath(dir);
    return dir;
}

QString AircraftStore::cpacsPathForVersion(int version) const
{
    const QString revision = QStringLiteral("aircraft_R%1").arg(version, 3, 10, QLatin1Char('0'));
    return cpacsDir() + QLatin1Char('/') + revision + QLatin1String(".cpacs.xml");
}

QString AircraftStore::casesDir() const
{
    const QString dir = rootDir() + QLatin1String("/cases");
    QDir().mkpath(dir);
    return dir;
}

QString AircraftStore::caseInputPath(int caseNumber) const
{
    return casesDir() + QLatin1String("/case_") + QString::number(caseNumber)
           + QLatin1String(".input.cpacs.xml");
}

int AircraftStore::nextCaseNumber() const
{
    QDir dir(casesDir());
    const QStringList files = dir.entryList(
        QStringList() << QStringLiteral("case_*.input.cpacs.xml"), QDir::Files);
    int maxNum = 0;
    QRegExp re(QStringLiteral("^case_(\\d+)\\.input\\.cpacs\\.xml$"));
    for (int i = 0; i < files.size(); ++i) {
        if (re.exactMatch(files[i]))
            maxNum = qMax(maxNum, re.cap(1).toInt());
    }
    return maxNum + 1;
}

QStringList AircraftStore::listCaseIds() const
{
    QDir dir(casesDir());
    const QStringList files = dir.entryList(
        QStringList() << QStringLiteral("case_*.input.cpacs.xml"), QDir::Files, QDir::Name);
    QStringList ids;
    QRegExp re(QStringLiteral("^(case_\\d+)\\.input\\.cpacs\\.xml$"));
    for (int i = 0; i < files.size(); ++i) {
        if (re.exactMatch(files[i]))
            ids.append(re.cap(1));
    }
    return ids;
}

bool AircraftStore::readDocumentFile(const QString &path, AircraftDocument *out, QString *errorMessage) const
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

bool AircraftStore::writeDocumentFile(const QString &path, const AircraftDocument &doc, QString *errorMessage) const
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

bool AircraftStore::loadDraft(AircraftDocument *out, QString *errorMessage) const
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

bool AircraftStore::saveDraft(const AircraftDocument &doc, QString *errorMessage) const
{
    return writeDocumentFile(draftPath(), doc, errorMessage);
}

bool AircraftStore::saveBaseline(const AircraftDocument &doc, QString *errorMessage) const
{
    if (doc.id.isEmpty()) {
        setError(errorMessage, QString::fromUtf8("基线 id 为空"));
        return false;
    }
    const QString path = baselinesDir() + QLatin1Char('/') + doc.id + QLatin1String(".json");
    return writeDocumentFile(path, doc, errorMessage);
}

bool AircraftStore::loadBaseline(const QString &id, AircraftDocument *out, QString *errorMessage) const
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

bool AircraftStore::listBaselines(QVector<AircraftBaselineInfo> *out, QString *errorMessage) const
{
    if (!out) {
        setError(errorMessage, QString::fromUtf8("内部错误：输出参数为空"));
        return false;
    }
    out->clear();

    QDir dir(baselinesDir());
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
    for (int i = 0; i < files.size(); ++i) {
        AircraftDocument doc;
        QString detail;
        const QString path = dir.absoluteFilePath(files[i]);
        if (!readDocumentFile(path, &doc, &detail)) {
            setError(errorMessage, QString::fromUtf8("读取基线失败：%1").arg(detail));
            return false;
        }
        AircraftBaselineInfo info;
        info.id = doc.id.isEmpty() ? QFileInfo(files[i]).completeBaseName() : doc.id;
        info.title = doc.title;
        info.version = doc.version;
        info.publishedAt = doc.publishedAt;
        info.filePath = path;
        out->append(info);
    }
    return true;
}
