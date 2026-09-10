#ifndef SRDDOCUMENTSERVICE_H
#define SRDDOCUMENTSERVICE_H

#include "model/srdstore.h"
#include "model/srdtypes.h"

class SrdDocumentService
{
public:
    explicit SrdDocumentService(SrdStore *store);

    bool loadDraft(QString *errorMessage = nullptr);
    bool saveDraft(QString *errorMessage = nullptr);
    bool replaceDocument(const SrdDocument &doc, QString *errorMessage = nullptr);
    bool openBaseline(const QString &id, QString *errorMessage = nullptr);
    bool copyBaselineToDraft(const QString &id, QString *errorMessage = nullptr);
    bool publishBaseline(QString *errorMessage = nullptr);
    bool freezeStandards(QString *errorMessage = nullptr);
    bool unfreezeStandards(QString *errorMessage = nullptr);

    bool setMissionChapter(const QVector<SrdMissionScenario> &missions,
                           const QVector<SrdPerformanceNeed> &needs,
                           QString *errorMessage = nullptr);
    bool setEnvelopeChapter(const SrdEnvironment &environment,
                            const QVector<SrdEnvelopePoint> &points,
                            const QVector<SrdFlightCondition> &conditions,
                            QString *errorMessage = nullptr);
    bool setStandardsChapter(const SrdCertification &certification,
                             const QVector<SrdClauseRow> &clauses,
                             QString *errorMessage = nullptr);
    bool setMetricsChapter(const QVector<SrdRequirement> &requirements,
                           QString *errorMessage = nullptr);

    bool addScenario(QString *errorMessage = nullptr);
    bool removeScenario(const QString &id, QString *errorMessage = nullptr);
    bool addNeedFromMetric(const QString &metricId, QString *errorMessage = nullptr);
    bool removeNeed(const QString &id, QString *errorMessage = nullptr);
    bool addCondition(QString *errorMessage = nullptr);
    bool removeCondition(const QString &id, QString *errorMessage = nullptr);
    bool addRequirementFromMetric(const QString &metricId, QString *errorMessage = nullptr);
    bool removeRequirement(const QString &id, QString *errorMessage = nullptr);

    bool mergeSuggestedConditions(const QVector<SrdFlightCondition> &suggested, int *added,
                                  QString *errorMessage = nullptr);
    bool mergeEnvelopePoints(const QVector<SrdEnvelopePoint> &points, int *added,
                             QString *errorMessage = nullptr);
    bool mergeSuggestedRequirements(const QVector<SrdRequirement> &suggested, int *added,
                                    QString *errorMessage = nullptr);
    bool appendNeeds(const QVector<SrdPerformanceNeed> &needs, int *added,
                     QString *errorMessage = nullptr);
    bool applyDefaultApplicability(QString *errorMessage = nullptr);

    const SrdDocument &current() const { return m_current; }
    bool viewingReadOnly() const { return m_readOnly; }
    QString draftPath() const;
    bool listBaselines(QVector<SrdBaselineInfo> *out, QString *errorMessage = nullptr) const;
    QString viewingId() const;

private:
    bool ensureEditable(QString *errorMessage) const;
    bool persist(QString *errorMessage);
    QString nextId(const QString &prefix, int width) const;
    QString nextRequirementId(const QString &metricId) const;
    int maxBaselineVersion(QString *errorMessage) const;

    SrdStore *m_store;
    SrdDocument m_current;
    bool m_readOnly = false;
};

#endif
