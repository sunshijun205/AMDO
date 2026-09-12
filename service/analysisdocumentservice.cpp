#include "service/analysisdocumentservice.h"

#include "model/analysiscatalogs.h"

#include <QDateTime>

static const char kDraftId[] = "analysis-draft";

AnalysisDocumentService::AnalysisDocumentService(AnalysisStore *store)
    : m_store(store)
{
}

bool AnalysisDocumentService::ensureEditable(QString *errorMessage) const
{
    if (m_readOnly) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("当前为只读分析集版本，请先切回草稿或另存为新草稿");
        return false;
    }
    return true;
}

bool AnalysisDocumentService::persist(QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->saveDraft(m_current, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存分析集失败：%1").arg(detail);
        return false;
    }
    return true;
}

bool AnalysisDocumentService::loadDraft(QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载失败：存储未初始化");
        return false;
    }

    AnalysisDocument doc;
    QString detail;
    if (!m_store->loadDraft(&doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载分析集失败：%1").arg(detail);
        return false;
    }

    if (doc.id.isEmpty()) {
        doc = AnalysisCatalogs::seedDocument();
        if (!m_store->saveDraft(doc, &detail)) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8("写入种子分析集失败：%1").arg(detail);
            return false;
        }
    }

    m_current = doc;
    m_readOnly = false;
    return true;
}

int AnalysisDocumentService::maxBaselineVersion(QString *errorMessage) const
{
    QVector<AnalysisBaselineInfo> list;
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

bool AnalysisDocumentService::saveValues(const QHash<QString, QString> &values, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.values = values;
    if (m_current.schemaVersion.isEmpty())
        m_current.schemaVersion = QStringLiteral("amdo.analysis.v1");
    if (m_current.id.isEmpty())
        m_current.id = QString::fromUtf8(kDraftId);
    if (m_current.status.isEmpty())
        m_current.status = QStringLiteral("draft");
    m_current.loadedKnown = true;
    return persist(errorMessage);
}

bool AnalysisDocumentService::setReferences(const QString &sourceRevision, const QString &sourceCase,
                                            QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.sourceRevision = sourceRevision;
    m_current.sourceCase = sourceCase;
    return persist(errorMessage);
}

bool AnalysisDocumentService::publishBaseline(QString *errorMessage, AnalysisDocument *publishedOut)
{
    if (!ensureEditable(errorMessage))
        return false;

    const int maxVersion = maxBaselineVersion(errorMessage);
    if (maxVersion < 0)
        return false;

    AnalysisDocument published = m_current;
    published.version = maxVersion + 1;
    published.id = QStringLiteral("analysis_v%1").arg(published.version);
    published.status = QStringLiteral("published");
    published.publishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString detail;
    if (!m_store->saveBaseline(published, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("发布分析集版本失败：%1").arg(detail);
        return false;
    }
    if (!persist(errorMessage))
        return false;
    if (publishedOut)
        *publishedOut = published;
    return true;
}

bool AnalysisDocumentService::openBaseline(const QString &id, QString *errorMessage)
{
    if (id.isEmpty() || id == QLatin1String("draft"))
        return loadDraft(errorMessage);

    AnalysisDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("打开分析集版本失败：%1").arg(detail);
        return false;
    }
    m_current = doc;
    m_readOnly = true;
    return true;
}

bool AnalysisDocumentService::copyBaselineToDraft(const QString &id, QString *errorMessage)
{
    AnalysisDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("复制分析集版本失败：%1").arg(detail);
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

bool AnalysisDocumentService::listBaselines(QVector<AnalysisBaselineInfo> *out, QString *errorMessage) const
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->listBaselines(out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("列出分析集版本失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString AnalysisDocumentService::viewingId() const
{
    if (m_readOnly)
        return m_current.id;
    return QStringLiteral("draft");
}

QString AnalysisDocumentService::draftPath() const
{
    return m_store ? m_store->draftPath() : QString();
}

QStringList AnalysisDocumentService::validate() const
{
    QStringList issues;
    if (m_current.sourceRevision.isEmpty())
        issues.append(QString::fromUtf8("未关联飞机方案修订（sourceRevision 为空）"));

    const QVector<AnalysisDomain> domainList = AnalysisCatalogs::domains();
    for (int di = 0; di < domainList.size(); ++di) {
        const AnalysisDomain &dm = domainList[di];
        for (int si = 0; si < dm.sections.size(); ++si) {
            const AnalysisSection &sec = dm.sections[si];
            for (int fi = 0; fi < sec.fields.size(); ++fi) {
                const AnalysisField &f = sec.fields[fi];
                if (f.type == QLatin1String("check"))
                    continue;
                const QString key = analysisFieldKey(dm.id, sec.title, f.label);
                const QString value = m_current.values.value(key, f.defaultValue);
                if (value.trimmed().isEmpty()) {
                    issues.append(QString::fromUtf8("%1 · %2：取值为空").arg(dm.name, f.label));
                    continue;
                }
                if (f.type == QLatin1String("number")) {
                    bool ok = false;
                    value.toDouble(&ok);
                    if (!ok) {
                        issues.append(QString::fromUtf8("%1 · %2：数值非法（%3）")
                                          .arg(dm.name, f.label, value));
                    }
                }
            }
        }
    }
    return issues;
}
