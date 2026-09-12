#ifndef DESIGNPAGE_H
#define DESIGNPAGE_H

#include "model/studytypes.h"

#include <QWidget>
#include <memory>

class QLabel;
class QLineEdit;
class QComboBox;
class AircraftStore;
class AircraftCpacsService;
class AircraftDocumentService;
class SrdStore;
class AnalysisStore;
class AnalysisComputeService;
class EvaluationService;
class StudyService;
class DesignPresenter;

class DesignPage : public QWidget
{
    Q_OBJECT
public:
    explicit DesignPage(QWidget *parent = nullptr);
    ~DesignPage() override;

    // 从设计变量输入构建研究定义（供 Presenter 读取）。
    StudyDefinition studyDefinition() const;
    // 当前优化基准对象：draft / analysis_vN。
    QString baseObjectId() const;

signals:
    void exploreRequested();
    void promoteRequested();

public slots:
    void showStudyResult(const StudyResult &result);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);

private:
    QWidget *buildExplorationPage();
    void reloadConstraints();
    void reloadObjectOptions();

    QComboBox *m_baseBox = nullptr;
    QLineEdit *m_sMin = nullptr;
    QLineEdit *m_sMax = nullptr;
    QLineEdit *m_sSteps = nullptr;
    QLineEdit *m_bMin = nullptr;
    QLineEdit *m_bMax = nullptr;
    QLineEdit *m_bSteps = nullptr;
    QWidget *m_constraintHost = nullptr;
    QWidget *m_studyKpiHost = nullptr;
    QWidget *m_studyTableHost = nullptr;
    QLabel *m_bestLabel = nullptr;
    QLabel *m_status = nullptr;
    bool m_updating = false;

    std::unique_ptr<AircraftStore> m_aircraftStore;
    std::unique_ptr<AircraftCpacsService> m_aircraftCpacs;
    std::unique_ptr<AircraftDocumentService> m_aircraftDoc;
    std::unique_ptr<SrdStore> m_srdStore;
    std::unique_ptr<AnalysisStore> m_analysisStore;
    std::unique_ptr<AnalysisComputeService> m_compute;
    std::unique_ptr<EvaluationService> m_evaluation;
    std::unique_ptr<StudyService> m_study;
    DesignPresenter *m_presenter = nullptr;
};

#endif
