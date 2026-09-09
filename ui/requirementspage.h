#ifndef REQUIREMENTSPAGE_H
#define REQUIREMENTSPAGE_H

#include <QWidget>
#include <memory>

class ProjectNotePanel;
class ProjectNotePresenter;
class ProjectNoteService;
class ProjectNoteStore;

class RequirementsPage : public QWidget
{
    Q_OBJECT
public:
    explicit RequirementsPage(QWidget *parent = nullptr);
    ~RequirementsPage() override;

private:
    ProjectNotePanel *m_notePanel = nullptr;
    std::unique_ptr<ProjectNoteStore> m_noteStore;
    std::unique_ptr<ProjectNoteService> m_noteService;
    ProjectNotePresenter *m_notePresenter = nullptr;
};

#endif
