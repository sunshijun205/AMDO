#include "model/analysisstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static const char kSchema[] = "amdo.analysis.v1";

static void setError(QString *errorMessage, const QString &text)
{
    if (errorMessage)
        *errorMessage = text;
}

static QJsonObject documentToJson(const AnalysisDocument &d)
{
    QJsonObject o;
    o.insert(QStringLiteral("schemaVersion"),
             d.schemaVersion.isEmpty() ? QString::fromUtf8(kSchema) : d.schemaVersion);
    o.insert(QStringLiteral("id"), d.id);
    o.insert(QStringLiteral("title"), d.title);
    o.insert(QStringLiteral("status"), d.status);
    o.insert(QStringLiteral("version"), d.version);
    o.insert(QStringLiteral("publishedAt"), d.publishedAt);
    o.insert(QStringLiteral("sourceRevision"), d.sourceRevision);
    o.insert(QStringLiteral("sourceCase"), d.sourceCase);
    o.insert(QStringLiteral("sourceSrd"), d.sourceSrd);
    o.insert(QStringLiteral("sourceCondition"), d.sourceCondition);

    QJsonObject values;
    for (auto it = d.values.constBegin(); it != d.values.constEnd(); ++it)
        values.insert(it.key(), it.value());
    o.insert(QStringLiteral("values"), values);
    return o;
}

static bool documentFromJson(const QJsonObject &o, AnalysisDocument *out, QString *errorMessage)
{
    const QString schema = o.value(QStringLiteral("schemaVersion")).toString();
    if (!schema.isEmpty() && schema != QLatin1String(kSchema)) {
        setError(errorMessage, QString::fromUtf8("不支持的分析集 schema：%1").arg(schema));
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
    out->sourceRevision = o.value(QStringLiteral("sourceRevision")).toString();
    out->sourceCase = o.value(QStringLiteral("sourceCase")).toString();
    out->sourceSrd = o.value(QStringLiteral("sourceSrd")).toString();
    out->sourceCondition = o.value(QStringLiteral("sourceCondition")).toString();

    out->values.clear();
    const QJsonObject values = o.value(QStringLiteral("values")).toObject();
    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        out->values.insert(it.key(), it.value().toString());

    out->loadedKnown = true;
    return true;
}

QString AnalysisStore::rootDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath();
    dir += QLatin1String("/analysis");
    QDir().mkpath(dir);
    return dir;
}

QString AnalysisStore::draftPath() const
{
    return rootDir() + QLatin1String("/analysis_draft.json");
}

QString AnalysisStore::baselinesDir() const
{
    const QString dir = rootDir() + QLatin1String("/baselines");
    QDir().mkpath(dir);
    return dir;
}

bool AnalysisStore::readDocumentFile(const QString &path, AnalysisDocument *out, QString *errorMessage) const
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

bool AnalysisStore::writeDocumentFile(const QString &path, const AnalysisDocument &doc, QString *errorMessage) const
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

bool AnalysisStore::loadDraft(AnalysisDocument *out, QString *errorMessage) const
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

bool AnalysisStore::saveDraft(const AnalysisDocument &doc, QString *errorMessage) const
{
    return writeDocumentFile(draftPath(), doc, errorMessage);
}

bool AnalysisStore::saveBaseline(const AnalysisDocument &doc, QString *errorMessage) const
{
    if (doc.id.isEmpty()) {
        setError(errorMessage, QString::fromUtf8("基线 id 为空"));
        return false;
    }
    const QString path = baselinesDir() + QLatin1Char('/') + doc.id + QLatin1String(".json");
    return writeDocumentFile(path, doc, errorMessage);
}

bool AnalysisStore::loadBaseline(const QString &id, AnalysisDocument *out, QString *errorMessage) const
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

bool AnalysisStore::listBaselines(QVector<AnalysisBaselineInfo> *out, QString *errorMessage) const
{
    if (!out) {
        setError(errorMessage, QString::fromUtf8("内部错误：输出参数为空"));
        return false;
    }
    out->clear();

    QDir dir(baselinesDir());
    const QStringList files = dir.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
    for (int i = 0; i < files.size(); ++i) {
        AnalysisDocument doc;
        QString detail;
        const QString path = dir.absoluteFilePath(files[i]);
        if (!readDocumentFile(path, &doc, &detail)) {
            setError(errorMessage, QString::fromUtf8("读取基线失败：%1").arg(detail));
            return false;
        }
        AnalysisBaselineInfo info;
        info.id = doc.id.isEmpty() ? QFileInfo(files[i]).completeBaseName() : doc.id;
        info.title = doc.title;
        info.version = doc.version;
        info.publishedAt = doc.publishedAt;
        info.filePath = path;
        out->append(info);
    }
    return true;
}
