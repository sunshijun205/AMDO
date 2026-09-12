#ifndef WORKFLOWPRESENTER_H
#define WORKFLOWPRESENTER_H

#include <QObject>

class WorkflowPage;
class WorkflowService;

class WorkflowPresenter : public QObject
{
    Q_OBJECT
public:
    WorkflowPresenter(WorkflowPage *view, WorkflowService *workflow, QObject *parent = nullptr);

public slots:
    void onRunRequested();

private:
    WorkflowPage *m_view;
    WorkflowService *m_workflow;
};

#endif
