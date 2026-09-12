#include "controller/decisionpresenter.h"

#include "decisionpage.h"
#include "model/analysisstore.h"
#include "model/analysistypes.h"
#include "service/evaluationservice.h"

#include <QVector>

DecisionPresenter::DecisionPresenter(DecisionPage *view, AnalysisStore *analysisStore,
                                     EvaluationService *evaluation, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_analysisStore(analysisStore)
    , m_evaluation(evaluation)
{
    connect(m_view, &DecisionPage::evaluateRequested, this, &DecisionPresenter::onEvaluateRequested);
    connect(m_view, &DecisionPage::refreshComparisonRequested,
            this, &DecisionPresenter::onRefreshComparisonRequested);

    // 初始评价（默认评价草稿 + 默认容差），让页面加载即显示真实数据。
    onEvaluateRequested(QStringLiteral("draft"), 0.5);
    refreshComparison();
}

void DecisionPresenter::refreshComparison()
{
    if (!m_evaluation)
        return;
    QVector<SchemeEvaluationResult> list;
    QString detail;
    m_evaluation->listResults(&list, &detail);
    m_view->showComparison(list);
}

void DecisionPresenter::onRefreshComparisonRequested()
{
    refreshComparison();
}

void DecisionPresenter::onEvaluateRequested(const QString &objectId, double tolerancePercent)
{
    if (!m_analysisStore || !m_evaluation) {
        m_view->setStatus(QString::fromUtf8("评价服务未初始化"), true);
        return;
    }

    // 评价对象：草稿 或 某个已发布分析集版本 analysis_vN。
    AnalysisDocument doc;
    QString detail;
    bool loaded = false;
    if (objectId.isEmpty() || objectId == QLatin1String("draft"))
        loaded = m_analysisStore->loadDraft(&doc, &detail);
    else
        loaded = m_analysisStore->loadBaseline(objectId, &doc, &detail);
    if (!loaded) {
        m_view->showError(QString::fromUtf8("加载分析集失败：%1").arg(detail));
        m_view->setStatus(QString::fromUtf8("加载分析集失败"), true);
        return;
    }
    if (doc.id.isEmpty()) {
        m_view->setStatus(QString::fromUtf8("尚无分析集，请先在「学科分析」页配置并关联设计需求"), true);
        return;
    }

    const QString id = (objectId.isEmpty() ? QStringLiteral("draft") : objectId);
    QString error;
    const SchemeEvaluationResult result = m_evaluation->evaluate(doc, id, tolerancePercent, &error);
    m_view->showEvaluation(result);

    if (!result.srdResolved) {
        m_view->setStatus(error.isEmpty()
                              ? QString::fromUtf8("未关联设计需求，无法评价")
                              : error, true);
        return;
    }

    // 持久化评价结果，供「方案比较与权衡」汇总。
    QString saveError;
    if (!m_evaluation->saveResult(result, nullptr, &saveError)) {
        m_view->setStatus(QString::fromUtf8("评价完成，但结果保存失败：%1").arg(saveError), true);
    } else {
        m_view->setStatus(QString::fromUtf8("评价完成：可行性 %1，满足率 %2，满足 %3 / 违反 %4 / 临界 %5 / 待分析 %6")
                              .arg(result.feasibility)
                              .arg(result.scoreKnown ? QString::number(result.score, 'f', 1) + QStringLiteral("%")
                                                     : QString::fromUtf8("—"))
                              .arg(result.satisfied).arg(result.violated)
                              .arg(result.critical).arg(result.pending));
    }
    refreshComparison();
}
