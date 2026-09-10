#ifndef REQUIREMENTSPRESENTER_H
#define REQUIREMENTSPRESENTER_H

#include <QObject>
#include <QString>

class RequirementsPage;
class SrdCompletenessService;
class SrdDerivationService;
class SrdDocumentService;
class SrdImportExportService;

class RequirementsPresenter : public QObject
{
    Q_OBJECT
public:
    RequirementsPresenter(RequirementsPage *view,
                          SrdDocumentService *document,
                          SrdCompletenessService *completeness,
                          SrdDerivationService *derivation,
                          SrdImportExportService *io,
                          QObject *parent = nullptr);

public slots:
    void onLoadRequested();
    void onSwitchBaselineRequested(const QString &id);
    void onCopyBaselineRequested();
    void onSaveMissionRequested();
    void onAddScenarioRequested();
    void onRemoveScenarioRequested();
    void onAddNeedRequested(const QString &metricId);
    void onRemoveNeedRequested();
    void onGenerateConditionsRequested();
    void onSaveEnvelopeRequested();
    void onAddConditionRequested();
    void onRemoveConditionRequested();
    void onSaveStandardsRequested();
    void onFreezeStandardsRequested();
    void onUnfreezeStandardsRequested();
    void onApplyApplicabilityRequested();
    void onGenerateRequirementsRequested();
    void onSaveMetricsRequested();
    void onAddRequirementRequested(const QString &metricId);
    void onRemoveRequirementRequested();
    void onPublishRequested();
    void onImportPathRequested(const QString &path);
    void onExportSpecRequested();

private:
    bool persistAll(QString *errorMessage);
    void refreshView();
    void fail(const QString &error);

    RequirementsPage *m_view;
    SrdDocumentService *m_document;
    SrdCompletenessService *m_completeness;
    SrdDerivationService *m_derivation;
    SrdImportExportService *m_io;
};

#endif
