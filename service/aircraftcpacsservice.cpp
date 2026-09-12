#include "service/aircraftcpacsservice.h"

#include "model/aircraftstore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>

static const char kCpacsVersion[] = "3.3";
static const char kAmdoNamespace[] = "urn:amdo:aircraft:v1";

AircraftCpacsService::AircraftCpacsService(AircraftStore *store)
    : m_store(store)
{
}

QString AircraftCpacsService::revisionId(int version)
{
    return QStringLiteral("R%1").arg(version, 3, 10, QLatin1Char('0'));
}

QString AircraftCpacsService::revisionPath(int version) const
{
    return m_store ? m_store->cpacsPathForVersion(version) : QString();
}

static void writeExtElement(QXmlStreamWriter &writer, const QString &name, const QString &value)
{
    writer.writeTextElement(QString::fromUtf8(kAmdoNamespace), name, value);
}

void AircraftCpacsService::writeDocument(QXmlStreamWriter &writer, const AircraftDocument &doc,
                                         const CaseInfo &caseInfo) const
{
    const QString ns = QString::fromUtf8(kAmdoNamespace);
    const QString stamp = doc.publishedAt.isEmpty()
                              ? QDateTime::currentDateTime().toString(Qt::ISODate)
                              : doc.publishedAt;

    writer.writeStartDocument(QStringLiteral("1.0"));
    writer.writeStartElement(QStringLiteral("cpacs"));
    writer.writeNamespace(ns, QStringLiteral("amdo"));

    // CPACS header：标识、修订与时间戳。
    writer.writeStartElement(QStringLiteral("header"));
    writer.writeTextElement(QStringLiteral("name"),
                            doc.title.isEmpty() ? QStringLiteral("aircraft") : doc.title);
    writer.writeTextElement(QStringLiteral("description"),
                            QString::fromUtf8("AMDO 飞机语义主数据（简化 CPACS 子集）"));
    writer.writeTextElement(QStringLiteral("creator"), QStringLiteral("AMDO"));
    writer.writeTextElement(QStringLiteral("timestamp"), stamp);
    writer.writeTextElement(QStringLiteral("version"), revisionId(doc.version));
    writer.writeTextElement(QStringLiteral("cpacsVersion"), QString::fromUtf8(kCpacsVersion));

    // AMDO 扩展：标识、单位与坐标语义。
    writer.writeStartElement(ns, QStringLiteral("semantics"));
    writeExtElement(writer, QStringLiteral("schemaVersion"),
                    doc.schemaVersion.isEmpty() ? QStringLiteral("amdo.aircraft.v1") : doc.schemaVersion);
    writeExtElement(writer, QStringLiteral("baselineId"), doc.id);
    writeExtElement(writer, QStringLiteral("status"), doc.status);
    writeExtElement(writer, QStringLiteral("schema"), doc.semantics.schema);
    writeExtElement(writer, QStringLiteral("namespace"), doc.semantics.namespaceStr);
    writeExtElement(writer, QStringLiteral("referenceStrategy"), doc.semantics.referenceStrategy);
    writeExtElement(writer, QStringLiteral("exchangeFormat"), doc.semantics.exchangeFormat);
    writeExtElement(writer, QStringLiteral("lengthUnit"), doc.semantics.lengthUnit);
    writeExtElement(writer, QStringLiteral("massUnit"), doc.semantics.massUnit);
    writeExtElement(writer, QStringLiteral("coordinateSystem"), doc.semantics.coordinateSystem);
    writeExtElement(writer, QStringLiteral("origin"), doc.semantics.origin);
    writer.writeEndElement(); // amdo:semantics

    // AMDO 扩展：用例登记（冻结为不可变分析用例输入时写入）。
    if (caseInfo.number > 0) {
        writer.writeStartElement(ns, QStringLiteral("case"));
        writeExtElement(writer, QStringLiteral("caseId"),
                        QStringLiteral("case_%1").arg(caseInfo.number));
        writeExtElement(writer, QStringLiteral("sourceBaseline"), caseInfo.sourceBaseline);
        writeExtElement(writer, QStringLiteral("sourceRevision"), caseInfo.sourceRevision);
        writeExtElement(writer, QStringLiteral("frozen"), QStringLiteral("true"));
        writeExtElement(writer, QStringLiteral("createdAt"),
                        QDateTime::currentDateTime().toString(Qt::ISODate));
        writer.writeEndElement(); // amdo:case
    }
    writer.writeEndElement(); // header

    writer.writeStartElement(QStringLiteral("vehicles"));
    writer.writeStartElement(QStringLiteral("aircraft"));
    writer.writeStartElement(QStringLiteral("model"));
    writer.writeAttribute(QStringLiteral("uID"),
                          doc.id.isEmpty() ? QStringLiteral("aircraft") : doc.id);
    writer.writeTextElement(QStringLiteral("name"),
                            doc.title.isEmpty() ? QStringLiteral("aircraft") : doc.title);
    writer.writeTextElement(QStringLiteral("description"),
                            QString::fromUtf8("由 AMDO 方案定义导出的概念方案语义"));

    // AMDO 扩展：总体方案配置。
    writer.writeStartElement(ns, QStringLiteral("configuration"));
    writeExtElement(writer, QStringLiteral("category"), doc.configuration.category);
    writeExtElement(writer, QStringLiteral("wingLayout"), doc.configuration.wingLayout);
    writeExtElement(writer, QStringLiteral("tailConfig"), doc.configuration.tailConfig);
    writeExtElement(writer, QStringLiteral("engineArrangement"), doc.configuration.engineArrangement);
    writeExtElement(writer, QStringLiteral("gearType"), doc.configuration.gearType);
    writeExtElement(writer, QStringLiteral("cabinLayout"), doc.configuration.cabinLayout);
    writeExtElement(writer, QStringLiteral("primaryMaterial"), doc.configuration.primaryMaterial);
    writeExtElement(writer, QStringLiteral("connectionStrategy"), doc.configuration.connectionStrategy);
    writeExtElement(writer, QStringLiteral("symmetry"), doc.configuration.symmetry);
    writer.writeEndElement(); // amdo:configuration

    // AMDO 扩展：系统/设备与安装定位。
    writer.writeStartElement(ns, QStringLiteral("systems"));
    for (int i = 0; i < doc.systems.size(); ++i) {
        const AcSystemItem &s = doc.systems[i];
        writer.writeStartElement(ns, QStringLiteral("system"));
        writer.writeAttribute(QStringLiteral("uID"), s.id);
        writeExtElement(writer, QStringLiteral("object"), s.object);
        writeExtElement(writer, QStringLiteral("scheme"), s.scheme);
        writeExtElement(writer, QStringLiteral("mounting"), s.mounting);
        writeExtElement(writer, QStringLiteral("material"), s.material);
        writeExtElement(writer, QStringLiteral("status"), s.status);
        writer.writeEndElement(); // amdo:system
    }
    writer.writeEndElement(); // amdo:systems

    // AMDO 扩展：参数化几何（尺寸与外形参数体系）。
    writer.writeStartElement(ns, QStringLiteral("parameters"));
    for (int i = 0; i < doc.parameters.size(); ++i) {
        const AcParameter &p = doc.parameters[i];
        writer.writeStartElement(ns, QStringLiteral("parameter"));
        writer.writeAttribute(QStringLiteral("uID"), p.id);
        writeExtElement(writer, QStringLiteral("name"), p.name);
        writeExtElement(writer, QStringLiteral("symbol"), p.symbol);
        writeExtElement(writer, QStringLiteral("value"),
                        p.valueKnown ? acFormatNumber(p.value) : QString());
        writeExtElement(writer, QStringLiteral("valueKnown"),
                        p.valueKnown ? QStringLiteral("true") : QStringLiteral("false"));
        writeExtElement(writer, QStringLiteral("unit"), p.unit);
        writeExtElement(writer, QStringLiteral("driveType"), p.driveType);
        writeExtElement(writer, QStringLiteral("note"), p.note);
        writer.writeEndElement(); // amdo:parameter
    }
    writer.writeEndElement(); // amdo:parameters

    writer.writeEndElement(); // model
    writer.writeEndElement(); // aircraft
    writer.writeEndElement(); // vehicles
    writer.writeEndElement(); // cpacs
    writer.writeEndDocument();
}

bool AircraftCpacsService::exportRevision(const AircraftDocument &doc, QString *outPath,
                                          QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    if (doc.version <= 0) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("方案版本号无效，无法生成 CPACS 修订");
        return false;
    }

    const QString path = m_store->cpacsPathForVersion(doc.version);
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无法写入 CPACS 文件：%1").arg(file.errorString());
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);
    writeDocument(writer, doc, CaseInfo());

    if (file.error() != QFile::NoError) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("写入 CPACS 不完整：%1").arg(file.errorString());
        return false;
    }
    if (outPath)
        *outPath = path;
    return true;
}

bool AircraftCpacsService::exportCaseSnapshot(const AircraftDocument &doc, int caseNumber,
                                              QString *outPath, QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    if (caseNumber <= 0) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("用例编号无效，无法冻结分析用例");
        return false;
    }

    const QString path = m_store->caseInputPath(caseNumber);
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("无法写入用例快照：%1").arg(file.errorString());
        return false;
    }

    CaseInfo caseInfo;
    caseInfo.number = caseNumber;
    caseInfo.sourceBaseline = doc.id;
    caseInfo.sourceRevision = doc.version > 0 ? revisionId(doc.version) : QString();

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);
    writeDocument(writer, doc, caseInfo);

    if (file.error() != QFile::NoError) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("写入用例快照不完整：%1").arg(file.errorString());
        return false;
    }
    if (outPath)
        *outPath = path;
    return true;
}
