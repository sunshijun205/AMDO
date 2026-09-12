#ifndef DECISIONPAGE_H
#define DECISIONPAGE_H

#include "model/evaluationresult.h"

#include <QVector>
#include <QWidget>
#include <memory>

class QLabel;
class QLineEdit;
class QComboBox;
class AircraftStore;
class SrdStore;
class AnalysisStore;
class AnalysisComputeService;
class EvaluationService;
class DecisionPresenter;

class DecisionPage : public QWidget
{
    Q_OBJECT
public:
    explicit DecisionPage(QWidget *parent = nullptr);
    ~DecisionPage() override;

signals:
    void evaluateRequested(const QString &objectId, double tolerancePercent);
    void refreshComparisonRequested();

public slots:
    void showEvaluation(const SchemeEvaluationResult &result);
    void showComparison(const QVector<SchemeEvaluationResult> &results);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);

private:
    QWidget *buildSinglePage();
    QWidget *buildComparePage();
    void reloadObjectOptions();
    void requestEvaluate();

    QWidget *m_kpiHost = nullptr;
    QWidget *m_tableHost = nullptr;
    QComboBox *m_objectBox = nullptr;
    QLabel *m_schemeInfo = nullptr;
    QLineEdit *m_tolerance = nullptr;
    QLabel *m_status = nullptr;
    bool m_updating = false;

    QWidget *m_compareKpiHost = nullptr;
    QWidget *m_compareTableHost = nullptr;
    QWidget *m_shortlistHost = nullptr;

    std::unique_ptr<AircraftStore> m_aircraftStore;
    std::unique_ptr<SrdStore> m_srdStore;
    std::unique_ptr<AnalysisStore> m_analysisStore;
    std::unique_ptr<AnalysisComputeService> m_compute;
    std::unique_ptr<EvaluationService> m_evaluation;
    DecisionPresenter *m_presenter = nullptr;
};

#endif
