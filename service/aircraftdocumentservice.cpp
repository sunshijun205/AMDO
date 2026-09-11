#include "service/aircraftdocumentservice.h"

#include "model/aircraftcatalogs.h"

#include <QDateTime>
#include <QRegExp>

static const char kDraftId[] = "aircraft-draft";

AircraftDocumentService::AircraftDocumentService(AircraftStore *store)
    : m_store(store)
{
}

bool AircraftDocumentService::ensureEditable(QString *errorMessage) const
{
    if (m_readOnly) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("当前为只读方案版本，请先切回草稿或另存为新草稿");
        return false;
    }
    return true;
}

bool AircraftDocumentService::persist(QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->saveDraft(m_current, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存草稿失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString AircraftDocumentService::nextSystemId() const
{
    int maxNum = 0;
    QRegExp re(QStringLiteral("^SYS-(\\d+)$"));
    for (int i = 0; i < m_current.systems.size(); ++i) {
        if (re.exactMatch(m_current.systems[i].id))
            maxNum = qMax(maxNum, re.cap(1).toInt());
    }
    return QStringLiteral("SYS-%1").arg(maxNum + 1, 3, 10, QLatin1Char('0'));
}

QString AircraftDocumentService::nextParameterId() const
{
    int maxNum = 0;
    QRegExp re(QStringLiteral("^PARAM-(\\d+)$"));
    for (int i = 0; i < m_current.parameters.size(); ++i) {
        if (re.exactMatch(m_current.parameters[i].id))
            maxNum = qMax(maxNum, re.cap(1).toInt());
    }
    return QStringLiteral("PARAM-%1").arg(maxNum + 1, 3, 10, QLatin1Char('0'));
}

int AircraftDocumentService::maxBaselineVersion(QString *errorMessage) const
{
    QVector<AircraftBaselineInfo> list;
    QString detail;
    if (!m_store->listBaselines(&list, &detail)) {
        if (errorMessage)
            *errorMessage = detail;
        return -1;
    }
    int maxVersion = 0;
    for (int i = 0; i < list.size(); ++i)
        maxVersion = qMax(maxVersion, list[i].version);
    return maxVersion;
}

bool AircraftDocumentService::loadDraft(QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载失败：存储未初始化");
        return false;
    }

    AircraftDocument doc;
    QString detail;
    if (!m_store->loadDraft(&doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载草稿失败：%1").arg(detail);
        return false;
    }

    if (doc.id.isEmpty()) {
        doc = AircraftCatalogs::seedDocument();
        if (!m_store->saveDraft(doc, &detail)) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8("写入种子草稿失败：%1").arg(detail);
            return false;
        }
    }

    // baselines 为空时，用原型样板数据默认生成一份方案版本，供“选择已有基线”。
    if (!ensureDefaultBaseline(errorMessage))
        return false;

    m_current = doc;
    m_readOnly = false;
    return true;
}

bool AircraftDocumentService::ensureDefaultBaseline(QString *errorMessage)
{
    QVector<AircraftBaselineInfo> list;
    QString detail;
    if (!m_store->listBaselines(&list, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("列出方案版本失败：%1").arg(detail);
        return false;
    }
    if (!list.isEmpty())
        return true;

    AircraftDocument base = AircraftCatalogs::seedDocument();
    base.version = 1;
    base.id = QStringLiteral("aircraft_v1");
    base.status = QStringLiteral("published");
    base.publishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    if (!m_store->saveBaseline(base, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("生成默认方案版本失败：%1").arg(detail);
        return false;
    }
    return true;
}

bool AircraftDocumentService::saveDraft(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    return persist(errorMessage);
}

bool AircraftDocumentService::replaceDocument(const AircraftDocument &doc, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current = doc;
    m_current.status = QStringLiteral("draft");
    m_current.id = QString::fromUtf8(kDraftId);
    m_current.version = 0;
    m_current.publishedAt.clear();
    m_current.loadedKnown = true;
    if (m_current.schemaVersion.isEmpty())
        m_current.schemaVersion = QStringLiteral("amdo.aircraft.v1");
    return persist(errorMessage);
}

bool AircraftDocumentService::newBlank(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current = AircraftCatalogs::blankDocument();
    m_readOnly = false;
    return persist(errorMessage);
}

bool AircraftDocumentService::newFromTemplate(const QString &templateName, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current = AircraftCatalogs::templateDocument(templateName);
    m_current.id = QString::fromUtf8(kDraftId);
    m_current.status = QStringLiteral("draft");
    m_current.version = 0;
    m_current.publishedAt.clear();
    m_readOnly = false;
    return persist(errorMessage);
}

bool AircraftDocumentService::openBaseline(const QString &id, QString *errorMessage)
{
    if (id.isEmpty() || id == QLatin1String("draft"))
        return loadDraft(errorMessage);

    AircraftDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("打开方案版本失败：%1").arg(detail);
        return false;
    }
    m_current = doc;
    m_readOnly = true;
    return true;
}

bool AircraftDocumentService::copyBaselineToDraft(const QString &id, QString *errorMessage)
{
    AircraftDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("复制方案版本失败：%1").arg(detail);
        return false;
    }
    doc.status = QStringLiteral("draft");
    doc.id = QString::fromUtf8(kDraftId);
    doc.version = 0;
    doc.publishedAt.clear();
    m_current = doc;
    m_readOnly = false;
    return persist(errorMessage);
}

bool AircraftDocumentService::publishBaseline(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;

    const int maxVersion = maxBaselineVersion(errorMessage);
    if (maxVersion < 0)
        return false;

    AircraftDocument published = m_current;
    published.version = maxVersion + 1;
    published.id = QStringLiteral("aircraft_v%1").arg(published.version);
    published.status = QStringLiteral("published");
    published.publishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString detail;
    if (!m_store->saveBaseline(published, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("发布方案版本失败：%1").arg(detail);
        return false;
    }
    return persist(errorMessage);
}

bool AircraftDocumentService::setSemantics(const AcSemantics &semantics, const QString &title,
                                           QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.semantics = semantics;
    if (!title.isEmpty())
        m_current.title = title;
    return persist(errorMessage);
}

bool AircraftDocumentService::setConfiguration(const AcConfiguration &configuration,
                                               const QVector<AcSystemItem> &systems,
                                               QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.configuration = configuration;
    m_current.systems = systems;
    return persist(errorMessage);
}

bool AircraftDocumentService::setParameters(const QVector<AcParameter> &parameters, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.parameters = parameters;
    return persist(errorMessage);
}

bool AircraftDocumentService::addSystem(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    AcSystemItem s;
    s.id = nextSystemId();
    s.object = QString::fromUtf8("新对象");
    s.status = QString::fromUtf8("待确认");
    m_current.systems.append(s);
    return persist(errorMessage);
}

bool AircraftDocumentService::removeSystem(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.systems.size(); ++i) {
        if (m_current.systems[i].id == id) {
            m_current.systems.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到系统对象：%1").arg(id);
    return false;
}

bool AircraftDocumentService::addParameter(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    AcParameter p;
    p.id = nextParameterId();
    p.name = QString::fromUtf8("新参数");
    p.driveType = QString::fromUtf8("设计变量");
    p.valueKnown = false;
    m_current.parameters.append(p);
    return persist(errorMessage);
}

bool AircraftDocumentService::removeParameter(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.parameters.size(); ++i) {
        if (m_current.parameters[i].id == id) {
            m_current.parameters.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到几何参数：%1").arg(id);
    return false;
}

QString AircraftDocumentService::draftPath() const
{
    return m_store ? m_store->draftPath() : QString();
}

bool AircraftDocumentService::listBaselines(QVector<AircraftBaselineInfo> *out, QString *errorMessage) const
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->listBaselines(out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("列出方案版本失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString AircraftDocumentService::viewingId() const
{
    if (m_readOnly)
        return m_current.id;
    return QStringLiteral("draft");
}
