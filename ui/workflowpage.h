#ifndef WORKFLOWPAGE_H
#define WORKFLOWPAGE_H

#include "model/workflowtypes.h"

#include <QVector>
#include <QWidget>
#include <memory>

class QLabel;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QPlainTextEdit;
class AircraftStore;
class AircraftCpacsService;
class AircraftDocumentService;
class SrdStore;
class AnalysisStore;
class AnalysisComputeService;
class EvaluationService;
class StudyService;
class WorkflowService;
class WorkflowPresenter;

class WorkflowPage : public QWidget
{
    Q_OBJECT
public:
    explicit WorkflowPage(QWidget *parent = nullptr);
    ~WorkflowPage() override;

    // 执行设置（供 Presenter 读取）。
    QString templateId() const;    // explore / single
    QString baseObjectId() const;  // draft / analysis_vN
    bool promoteBest() const;
    int retryLimit() const;

signals:
    void runWorkflowRequested();

public slots:
    void showRunResult(const WorkflowRunResult &result);
    void setStatus(const QString &text, bool isError = false);
    void reloadHistory();

private:
    QWidget *buildDefinitionPage();
    QWidget *buildExecutionPage();
    QWidget *buildMonitorPage();
    void reloadBaseOptions();
    void reloadTemplateView();
    void onReplaySelected();

    QComboBox *m_templateBox = nullptr;
    QComboBox *m_baseBox = nullptr;
    QCheckBox *m_promoteCheck = nullptr;
    QSpinBox *m_retrySpin = nullptr;
    QComboBox *m_historyBox = nullptr;
    QLabel *m_defDesc = nullptr;
    QWidget *m_defFlowHost = nullptr;
    QWidget *m_defNodeHost = nullptr;
    QWidget *m_nodeHost = nullptr;
    QWidget *m_summaryHost = nullptr;
    QPlainTextEdit *m_logView = nullptr;
    QLabel *m_status = nullptr;
    bool m_updating = false;
    QVector<WorkflowRunResult> m_runs;

    std::unique_ptr<AircraftStore> m_aircraftStore;
    std::unique_ptr<AircraftCpacsService> m_aircraftCpacs;
    std::unique_ptr<AircraftDocumentService> m_aircraftDoc;
    std::unique_ptr<SrdStore> m_srdStore;
    std::unique_ptr<AnalysisStore> m_analysisStore;
    std::unique_ptr<AnalysisComputeService> m_compute;
    std::unique_ptr<EvaluationService> m_evaluation;
    std::unique_ptr<StudyService> m_study;
    std::unique_ptr<WorkflowService> m_workflow;
    WorkflowPresenter *m_presenter = nullptr;
};

#endif
