#ifndef REQUIREMENTSPAGE_H
#define REQUIREMENTSPAGE_H

#include "model/srdtypes.h"

#include <QWidget>
#include <memory>

class EnvelopeChart;
class MissionRail;
class ProjectNotePanel;
class ProjectNotePresenter;
class ProjectNoteService;
class ProjectNoteStore;
class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QTableWidget;
class RequirementsPresenter;
class SrdCompletenessService;
class SrdDerivationService;
class SrdDocumentService;
class SrdImportExportService;
class SrdStore;

class RequirementsPage : public QWidget
{
    Q_OBJECT
public:
    explicit RequirementsPage(QWidget *parent = nullptr);
    ~RequirementsPage() override;

    void requestImport();
    void requestPublish();

    void snapshotMissionChapter(QVector<SrdMissionScenario> *missions,
                                QVector<SrdPerformanceNeed> *needs) const;
    void snapshotEnvelopeChapter(SrdEnvironment *environment,
                                 QVector<SrdEnvelopePoint> *points,
                                 QVector<SrdFlightCondition> *conditions) const;
    void snapshotStandardsChapter(SrdCertification *certification,
                                  QVector<SrdClauseRow> *clauses) const;
    void snapshotMetricsChapter(QVector<SrdRequirement> *requirements) const;

    QString selectedScenarioId() const;
    QString selectedNeedId() const;
    QString selectedConditionId() const;
    QString selectedRequirementId() const;

signals:
    void loadRequested();
    void switchBaselineRequested(const QString &id);
    void copyBaselineRequested();
    void saveMissionRequested();
    void addScenarioRequested();
    void removeScenarioRequested();
    void addNeedRequested(const QString &metricId);
    void removeNeedRequested();
    void generateConditionsRequested();
    void saveEnvelopeRequested();
    void addConditionRequested();
    void removeConditionRequested();
    void saveStandardsRequested();
    void freezeStandardsRequested();
    void unfreezeStandardsRequested();
    void applyApplicabilityRequested();
    void generateRequirementsRequested();
    void saveMetricsRequested();
    void addRequirementRequested(const QString &metricId);
    void removeRequirementRequested();
    void publishRequested();
    void importPathRequested(const QString &path);
    void exportSpecRequested();

public slots:
    void setDocument(const SrdDocument &doc, bool readOnly);
    void setCompleteness(const SrdCompletenessReport &report);
    void setBaselines(const QVector<SrdBaselineInfo> &baselines, const QString &currentId);
    void setBusy(bool busy);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);
    bool confirmList(const QString &title, const QStringList &lines);

private slots:
    void onScenarioSelected();
    void onRequirementSelected();
    void onBaselineChanged(int index);

private:
    void buildUi();
    QWidget *buildMissionPage();
    QWidget *buildEnvelopePage();
    QWidget *buildStandardsPage();
    QWidget *buildMetricsPage();
    void fillScenarioSide(const SrdMissionScenario &s);
    void fillRequirementSide(const SrdRequirement &r);
    void applyScenarioSide(SrdMissionScenario *s) const;
    void applyRequirementSide(SrdRequirement *r) const;
    QString currentNeedMetricId() const;
    QString currentReqMetricId() const;
    void setReadOnly(bool readOnly);

    SrdDocument m_shown;
    bool m_readOnly = false;
    bool m_updating = false;

    ProjectNotePanel *m_notePanel = nullptr;
    std::unique_ptr<ProjectNoteStore> m_noteStore;
    std::unique_ptr<ProjectNoteService> m_noteService;
    ProjectNotePresenter *m_notePresenter = nullptr;

    std::unique_ptr<SrdStore> m_srdStore;
    std::unique_ptr<SrdDocumentService> m_srdDocument;
    std::unique_ptr<SrdCompletenessService> m_srdCompleteness;
    std::unique_ptr<SrdDerivationService> m_srdDerivation;
    std::unique_ptr<SrdImportExportService> m_srdIo;
    RequirementsPresenter *m_srdPresenter = nullptr;

    QComboBox *m_baselineBox = nullptr;
    QPushButton *m_copyDraftBtn = nullptr;
    QLabel *m_pageStatus = nullptr;

    QWidget *m_missionKpiHost = nullptr;
    MissionRail *m_rail = nullptr;
    QTableWidget *m_scenarioTable = nullptr;
    QTableWidget *m_needTable = nullptr;
    QComboBox *m_missionType = nullptr;
    QLineEdit *m_crew = nullptr;
    QLineEdit *m_utilization = nullptr;
    QLineEdit *m_designLife = nullptr;
    QLineEdit *m_elevation = nullptr;
    QLineEdit *m_runway = nullptr;
    QWidget *m_sourceHost = nullptr;
    QComboBox *m_needMetricBox = nullptr;

    QWidget *m_envelopeKpiHost = nullptr;
    EnvelopeChart *m_envelopeChart = nullptr;
    QComboBox *m_atmModel = nullptr;
    QLineEdit *m_tempOffset = nullptr;
    QLineEdit *m_crosswind = nullptr;
    QLineEdit *m_slope = nullptr;
    QComboBox *m_gust = nullptr;
    QLineEdit *m_icing = nullptr;
    QLineEdit *m_nzMin = nullptr;
    QLineEdit *m_nzMax = nullptr;
    QLabel *m_coverageLabel = nullptr;
    QProgressBar *m_coverageBar = nullptr;
    QTableWidget *m_pointTable = nullptr;
    QTableWidget *m_fcTable = nullptr;

    QWidget *m_stdKpiHost = nullptr;
    QComboBox *m_awClass = nullptr;
    QComboBox *m_opsRules = nullptr;
    QLineEdit *m_amendment = nullptr;
    QLineEdit *m_noise = nullptr;
    QLineEdit *m_emission = nullptr;
    QLineEdit *m_special = nullptr;
    QTableWidget *m_clauseTable = nullptr;
    QWidget *m_applyHost = nullptr;
    QWidget *m_stdCheckHost = nullptr;

    QWidget *m_metricKpiHost = nullptr;
    QTableWidget *m_reqTable = nullptr;
    QComboBox *m_reqMetricBox = nullptr;
    QComboBox *m_constraintKind = nullptr;
    QLineEdit *m_priority = nullptr;
    QComboBox *m_verify = nullptr;
    QLineEdit *m_reqStatus = nullptr;
    QWidget *m_traceHost = nullptr;

    QPushButton *m_saveMissionBtn = nullptr;
    QPushButton *m_genFcBtn = nullptr;
    QPushButton *m_saveEnvBtn = nullptr;
    QPushButton *m_freezeBtn = nullptr;
    QPushButton *m_unfreezeBtn = nullptr;
    QPushButton *m_applyAppBtn = nullptr;
    QPushButton *m_saveStdBtn = nullptr;
    QPushButton *m_saveReqBtn = nullptr;
    QPushButton *m_genReqBtn = nullptr;
    QPushButton *m_exportBtn = nullptr;
    QPushButton *m_publishBtn = nullptr;
};

#endif
