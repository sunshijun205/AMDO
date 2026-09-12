#include "controller/workflowpresenter.h"

#include "service/workflowservice.h"
#include "ui/workflowpage.h"

WorkflowPresenter::WorkflowPresenter(WorkflowPage *view, WorkflowService *workflow, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_workflow(workflow)
{
    connect(m_view, &WorkflowPage::runWorkflowRequested, this, &WorkflowPresenter::onRunRequested);
}

void WorkflowPresenter::onRunRequested()
{
    if (!m_workflow) {
        m_view->setStatus(QString::fromUtf8("工作流服务未初始化"), true);
        return;
    }

    const QString templateId = m_view->templateId();
    const QString base = m_view->baseObjectId();
    const bool promote = m_view->promoteBest();
    const int retry = m_view->retryLimit();

    QString error;
    const WorkflowRunResult result = m_workflow->run(templateId, base, promote, retry, &error);
    m_view->showRunResult(result);
    m_view->reloadHistory();

    if (!result.ok) {
        m_view->setStatus(error.isEmpty() ? QString::fromUtf8("工作流执行失败，详见运行监控") : error, true);
        return;
    }

    QString msg = QString::fromUtf8("工作流「%1」完成").arg(result.templateName);
    if (!result.headline.isEmpty())
        msg += QString::fromUtf8("：%1").arg(result.headline);
    if (!result.promotedId.isEmpty())
        msg += QString::fromUtf8("；已提升为 %1").arg(result.promotedId);
    m_view->setStatus(msg);
}
