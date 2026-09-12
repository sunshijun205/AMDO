#ifndef DECISIONPRESENTER_H
#define DECISIONPRESENTER_H

#include <QObject>

class DecisionPage;
class AnalysisStore;
class EvaluationService;

class DecisionPresenter : public QObject
{
    Q_OBJECT
public:
    DecisionPresenter(DecisionPage *view, AnalysisStore *analysisStore,
                      EvaluationService *evaluation, QObject *parent = nullptr);

public slots:
    void onEvaluateRequested(const QString &objectId, double tolerancePercent);
    void onRefreshComparisonRequested();

private:
    void refreshComparison();

    DecisionPage *m_view;
    AnalysisStore *m_analysisStore;
    EvaluationService *m_evaluation;
};

#endif
