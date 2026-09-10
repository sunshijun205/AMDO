#include "service/srddocumentservice.h"

#include "model/srdcatalogs.h"

#include <QDateTime>
#include <QRegExp>
#include <QSet>

SrdDocumentService::SrdDocumentService(SrdStore *store)
    : m_store(store)
{
}

bool SrdDocumentService::ensureEditable(QString *errorMessage) const
{
    if (m_readOnly) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("当前为只读基线，请先切回草稿或另存为新草稿");
        return false;
    }
    return true;
}

bool SrdDocumentService::persist(QString *errorMessage)
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

QString SrdDocumentService::nextId(const QString &prefix, int width) const
{
    int maxNum = 0;
    QRegExp re(QLatin1Char('^') + QRegExp::escape(prefix) + QStringLiteral("(\\d+)$"));

    auto consider = [&](const QString &id) {
        if (re.exactMatch(id))
            maxNum = qMax(maxNum, re.cap(1).toInt());
    };

    for (int i = 0; i < m_current.missions.size(); ++i)
        consider(m_current.missions[i].id);
    for (int i = 0; i < m_current.needs.size(); ++i)
        consider(m_current.needs[i].id);
    for (int i = 0; i < m_current.conditions.size(); ++i)
        consider(m_current.conditions[i].id);
    for (int i = 0; i < m_current.requirements.size(); ++i)
        consider(m_current.requirements[i].id);

    return prefix + QStringLiteral("%1").arg(maxNum + 1, width, 10, QLatin1Char('0'));
}

QString SrdDocumentService::nextRequirementId(const QString &metricId) const
{
    QString domainCode = QStringLiteral("GEN");
    SrdCatalogMetric metric;
    if (SrdCatalogs::findMetric(metricId, &metric)) {
        if (metric.domain.contains(QString::fromUtf8("任务")))
            domainCode = QStringLiteral("PERF");
        else if (metric.domain.contains(QString::fromUtf8("重量")))
            domainCode = QStringLiteral("WGT");
        else if (metric.domain.contains(QString::fromUtf8("气动")))
            domainCode = QStringLiteral("AERO");
        else if (metric.domain.contains(QString::fromUtf8("结构")))
            domainCode = QStringLiteral("STR");
        else if (metric.domain.contains(QString::fromUtf8("操稳")))
            domainCode = QStringLiteral("DYN");
        else if (metric.domain.contains(QString::fromUtf8("总体")))
            domainCode = QStringLiteral("COST");
    }
    return nextId(QStringLiteral("REQ-") + domainCode + QLatin1Char('-'), 3);
}

int SrdDocumentService::maxBaselineVersion(QString *errorMessage) const
{
    QVector<SrdBaselineInfo> list;
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

bool SrdDocumentService::loadDraft(QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载失败：存储未初始化");
        return false;
    }

    SrdDocument doc;
    QString detail;
    if (!m_store->loadDraft(&doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载草稿失败：%1").arg(detail);
        return false;
    }

    if (doc.id.isEmpty()) {
        doc = SrdCatalogs::seedDocument();
        if (!m_store->saveDraft(doc, &detail)) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8("写入种子草稿失败：%1").arg(detail);
            return false;
        }
    }

    m_current = doc;
    m_readOnly = false;
    return true;
}

bool SrdDocumentService::saveDraft(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    return persist(errorMessage);
}

bool SrdDocumentService::replaceDocument(const SrdDocument &doc, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current = doc;
    m_current.status = QStringLiteral("draft");
    m_current.id = QStringLiteral("srd-draft");
    m_current.publishedAt.clear();
    m_current.loadedKnown = true;
    if (m_current.schemaVersion.isEmpty())
        m_current.schemaVersion = QStringLiteral("amdo.srd.v1");
    return persist(errorMessage);
}

bool SrdDocumentService::openBaseline(const QString &id, QString *errorMessage)
{
    if (id.isEmpty() || id == QLatin1String("draft"))
        return loadDraft(errorMessage);

    SrdDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("打开基线失败：%1").arg(detail);
        return false;
    }
    m_current = doc;
    m_readOnly = true;
    return true;
}

bool SrdDocumentService::copyBaselineToDraft(const QString &id, QString *errorMessage)
{
    SrdDocument doc;
    QString detail;
    if (!m_store->loadBaseline(id, &doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("复制基线失败：%1").arg(detail);
        return false;
    }
    doc.status = QStringLiteral("draft");
    doc.id = QStringLiteral("srd-draft");
    doc.publishedAt.clear();
    doc.chapters.standardsFrozen = false;
    doc.chapters.missionFrozen = false;
    doc.chapters.envelopeFrozen = false;
    doc.chapters.metricsFrozen = false;
    m_current = doc;
    m_readOnly = false;
    return persist(errorMessage);
}

bool SrdDocumentService::publishBaseline(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;

    const int maxVersion = maxBaselineVersion(errorMessage);
    if (maxVersion < 0)
        return false;

    SrdDocument published = m_current;
    published.version = maxVersion + 1;
    published.id = QStringLiteral("srd_v%1").arg(published.version);
    published.status = QStringLiteral("published");
    published.publishedAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    published.title = QString::fromUtf8("SRD 基线 v%1").arg(published.version);

    QString detail;
    if (!m_store->saveBaseline(published, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("发布基线失败：%1").arg(detail);
        return false;
    }
    return persist(errorMessage);
}

bool SrdDocumentService::freezeStandards(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.chapters.standardsFrozen = true;
    return persist(errorMessage);
}

bool SrdDocumentService::unfreezeStandards(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.chapters.standardsFrozen = false;
    return persist(errorMessage);
}

bool SrdDocumentService::setMissionChapter(const QVector<SrdMissionScenario> &missions,
                                           const QVector<SrdPerformanceNeed> &needs,
                                           QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.missions = missions;
    m_current.needs = needs;
    return persist(errorMessage);
}

bool SrdDocumentService::setEnvelopeChapter(const SrdEnvironment &environment,
                                            const QVector<SrdEnvelopePoint> &points,
                                            const QVector<SrdFlightCondition> &conditions,
                                            QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.environment = environment;
    m_current.envelopePoints = points;
    m_current.conditions = conditions;
    return persist(errorMessage);
}

bool SrdDocumentService::setStandardsChapter(const SrdCertification &certification,
                                             const QVector<SrdClauseRow> &clauses,
                                             QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    if (m_current.chapters.standardsFrozen) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("规范章已冻结，请先解冻再修改");
        return false;
    }
    m_current.certification = certification;
    m_current.clauses = clauses;
    return persist(errorMessage);
}

bool SrdDocumentService::setMetricsChapter(const QVector<SrdRequirement> &requirements,
                                           QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    m_current.requirements = requirements;
    return persist(errorMessage);
}

bool SrdDocumentService::addScenario(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    SrdMissionScenario s;
    s.id = nextId(QStringLiteral("M-"), 3);
    s.name = QString::fromUtf8("新场景");
    s.missionType = QString::fromUtf8("客运运输");
    s.status = QString::fromUtf8("草案");
    m_current.missions.append(s);
    return persist(errorMessage);
}

bool SrdDocumentService::removeScenario(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.missions.size(); ++i) {
        if (m_current.missions[i].id == id) {
            m_current.missions.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到场景：%1").arg(id);
    return false;
}

bool SrdDocumentService::addNeedFromMetric(const QString &metricId, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    SrdCatalogMetric metric;
    if (!SrdCatalogs::findMetric(metricId, &metric)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("未知指标：%1").arg(metricId);
        return false;
    }
    SrdPerformanceNeed n;
    n.id = nextId(QStringLiteral("NEED-"), 3);
    n.name = metric.name;
    n.metricId = metric.id;
    n.relation = QStringLiteral(">=");
    n.unit = metric.unit;
    n.grade = QString::fromUtf8("目标");
    n.sourceKind = QString::fromUtf8("设计目标");
    m_current.needs.append(n);
    return persist(errorMessage);
}

bool SrdDocumentService::removeNeed(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.needs.size(); ++i) {
        if (m_current.needs[i].id == id) {
            m_current.needs.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到性能需求：%1").arg(id);
    return false;
}

bool SrdDocumentService::addCondition(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    SrdFlightCondition c;
    c.id = nextId(QStringLiteral("FC-"), 2);
    c.phase = QString::fromUtf8("巡航");
    c.altitude = QStringLiteral("11,000 m");
    c.speed = QStringLiteral("0.78 Ma");
    c.configuration = QString::fromUtf8("清洁");
    c.atmosphere = QStringLiteral("ISA");
    m_current.conditions.append(c);
    return persist(errorMessage);
}

bool SrdDocumentService::removeCondition(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.conditions.size(); ++i) {
        if (m_current.conditions[i].id == id) {
            m_current.conditions.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到工况：%1").arg(id);
    return false;
}

bool SrdDocumentService::addRequirementFromMetric(const QString &metricId, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    SrdCatalogMetric metric;
    if (!SrdCatalogs::findMetric(metricId, &metric)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("未知指标：%1").arg(metricId);
        return false;
    }
    for (int i = 0; i < m_current.requirements.size(); ++i) {
        if (m_current.requirements[i].metricId == metricId) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8("该指标已存在需求：%1").arg(m_current.requirements[i].id);
            return false;
        }
    }
    SrdRequirement r;
    r.id = nextRequirementId(metricId);
    r.metricId = metric.id;
    r.metricName = metric.name;
    r.domain = metric.domain;
    r.unit = metric.unit;
    r.analysisResponseId = metric.responseId;
    r.relation = QStringLiteral(">=");
    r.grade = QString::fromUtf8("目标");
    r.constraintKind = srdGradeToKind(r.grade);
    r.status = QString::fromUtf8("草案");
    r.priority = QStringLiteral("P2");
    r.verificationMethod = QString::fromUtf8("分析");
    m_current.requirements.append(r);
    return persist(errorMessage);
}

bool SrdDocumentService::removeRequirement(const QString &id, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    for (int i = 0; i < m_current.requirements.size(); ++i) {
        if (m_current.requirements[i].id == id) {
            m_current.requirements.remove(i);
            return persist(errorMessage);
        }
    }
    if (errorMessage)
        *errorMessage = QString::fromUtf8("未找到需求：%1").arg(id);
    return false;
}

static bool sameCondition(const SrdFlightCondition &a, const SrdFlightCondition &b)
{
    return a.phase == b.phase && a.altitude == b.altitude
        && a.speed == b.speed && a.configuration == b.configuration;
}

bool SrdDocumentService::mergeSuggestedConditions(const QVector<SrdFlightCondition> &suggested,
                                                  int *added, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    int count = 0;
    for (int i = 0; i < suggested.size(); ++i) {
        bool exists = false;
        for (int j = 0; j < m_current.conditions.size(); ++j) {
            if (sameCondition(m_current.conditions[j], suggested[i])) {
                exists = true;
                break;
            }
        }
        if (exists)
            continue;
        SrdFlightCondition c = suggested[i];
        if (c.id.isEmpty())
            c.id = nextId(QStringLiteral("FC-"), 2);
        m_current.conditions.append(c);
        ++count;
    }
    if (added)
        *added = count;
    return persist(errorMessage);
}

bool SrdDocumentService::mergeEnvelopePoints(const QVector<SrdEnvelopePoint> &points, int *added,
                                             QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    int count = 0;
    for (int i = 0; i < points.size(); ++i) {
        bool exists = false;
        for (int j = 0; j < m_current.envelopePoints.size(); ++j) {
            if (qAbs(m_current.envelopePoints[j].mach - points[i].mach) < 1e-6
                && qAbs(m_current.envelopePoints[j].altitudeKm - points[i].altitudeKm) < 1e-6) {
                exists = true;
                break;
            }
        }
        if (exists)
            continue;
        m_current.envelopePoints.append(points[i]);
        ++count;
    }
    if (added)
        *added = count;
    return persist(errorMessage);
}

bool SrdDocumentService::mergeSuggestedRequirements(const QVector<SrdRequirement> &suggested,
                                                    int *added, QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    int count = 0;
    QSet<QString> metricIds;
    for (int i = 0; i < m_current.requirements.size(); ++i)
        metricIds.insert(m_current.requirements[i].metricId);

    for (int i = 0; i < suggested.size(); ++i) {
        if (suggested[i].metricId.isEmpty() || metricIds.contains(suggested[i].metricId))
            continue;
        SrdRequirement r = suggested[i];
        if (r.id.isEmpty())
            r.id = nextRequirementId(r.metricId);
        if (r.constraintKind.isEmpty())
            r.constraintKind = srdGradeToKind(r.grade);
        m_current.requirements.append(r);
        metricIds.insert(r.metricId);
        ++count;

        for (int c = 0; c < m_current.clauses.size(); ++c) {
            if (m_current.clauses[c].clauseId == r.source
                || r.source.contains(m_current.clauses[c].clauseId)) {
                m_current.clauses[c].mappingStatus = QString::fromUtf8("已映射");
                m_current.clauses[c].mappedReqId = r.id;
            }
        }
    }
    if (added)
        *added = count;
    return persist(errorMessage);
}

bool SrdDocumentService::appendNeeds(const QVector<SrdPerformanceNeed> &needs, int *added,
                                     QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    int count = 0;
    QSet<QString> metricIds;
    for (int i = 0; i < m_current.needs.size(); ++i)
        metricIds.insert(m_current.needs[i].metricId);
    for (int i = 0; i < needs.size(); ++i) {
        SrdPerformanceNeed n = needs[i];
        if (n.metricId.isEmpty() || metricIds.contains(n.metricId))
            continue;
        n.id = nextId(QStringLiteral("NEED-"), 3);
        m_current.needs.append(n);
        metricIds.insert(n.metricId);
        ++count;
    }
    if (added)
        *added = count;
    return persist(errorMessage);
}

bool SrdDocumentService::applyDefaultApplicability(QString *errorMessage)
{
    if (!ensureEditable(errorMessage))
        return false;
    if (m_current.chapters.standardsFrozen) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("规范章已冻结，请先解冻再修改");
        return false;
    }

    const bool cs25 = m_current.certification.airworthinessClass.contains(QStringLiteral("CS-25"));
    const bool noElectric = m_current.certification.specialCondition.contains(QString::fromUtf8("电推进：不适用"))
        || m_current.certification.specialCondition.contains(QString::fromUtf8("电推进不适用"));

    QSet<QString> existing;
    for (int i = 0; i < m_current.clauses.size(); ++i)
        existing.insert(m_current.clauses[i].clauseId);

    const QVector<SrdCatalogClause> catalog = SrdCatalogs::clauses();
    for (int i = 0; i < catalog.size(); ++i) {
        if (!existing.contains(catalog[i].clauseId)) {
            SrdClauseRow row;
            row.clauseId = catalog[i].clauseId;
            row.topic = catalog[i].topic;
            row.domain = catalog[i].domain;
            row.applicability = QString::fromUtf8("待确认");
            row.mappingStatus = QString::fromUtf8("草案");
            m_current.clauses.append(row);
        }
    }

    if (cs25) {
        for (int i = 0; i < m_current.clauses.size(); ++i) {
            SrdCatalogClause cat;
            const bool found = SrdCatalogs::findClause(m_current.clauses[i].clauseId, &cat);
            if (found && cat.electricPropulsionRelated && noElectric) {
                m_current.clauses[i].applicability = QString::fromUtf8("不适用");
                m_current.clauses[i].mappingStatus = QString::fromUtf8("不适用");
            } else if (m_current.clauses[i].applicability.isEmpty()
                       || m_current.clauses[i].applicability == QString::fromUtf8("待确认")) {
                m_current.clauses[i].applicability = QString::fromUtf8("直接适用");
                if (m_current.clauses[i].mappingStatus.isEmpty())
                    m_current.clauses[i].mappingStatus = QString::fromUtf8("待确认");
            }
        }
    }
    return persist(errorMessage);
}

QString SrdDocumentService::draftPath() const
{
    return m_store ? m_store->draftPath() : QString();
}

bool SrdDocumentService::listBaselines(QVector<SrdBaselineInfo> *out, QString *errorMessage) const
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->listBaselines(out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("列出基线失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString SrdDocumentService::viewingId() const
{
    if (m_readOnly)
        return m_current.id;
    return QStringLiteral("draft");
}
