#ifndef PROJECTNOTEPRESENTER_H
#define PROJECTNOTEPRESENTER_H

#include <QObject>

class ProjectNotePanel;
class ProjectNoteService;

// Presenter：唯一协调者；不依赖具体布局细节，不直接做文件 IO
class ProjectNotePresenter : public QObject
{
    Q_OBJECT
public:
    ProjectNotePresenter(ProjectNotePanel *view,
                         ProjectNoteService *service,
                         QObject *parent = nullptr);

private slots:
    void onLoadRequested();
    void onSaveRequested(const QString &text);

private:
    ProjectNotePanel *m_view;
    ProjectNoteService *m_service;
};

#endif
