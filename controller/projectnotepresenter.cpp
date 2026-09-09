#include "controller/projectnotepresenter.h"

#include "model/projectnote.h"
#include "service/projectnoteservice.h"
#include "projectnotepanel.h"

ProjectNotePresenter::ProjectNotePresenter(ProjectNotePanel *view,
                                           ProjectNoteService *service,
                                           QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_service(service)
{
    connect(m_view, &ProjectNotePanel::loadRequested,
            this, &ProjectNotePresenter::onLoadRequested);
    connect(m_view, &ProjectNotePanel::saveRequested,
            this, &ProjectNotePresenter::onSaveRequested);
    onLoadRequested();
}

void ProjectNotePresenter::onLoadRequested()
{
    if (!m_view || !m_service)
        return;

    m_view->setBusy(true);
    ProjectNote note;
    QString error;
    if (!m_service->loadNote(&note, &error)) {
        m_view->setBusy(false);
        m_view->setStatus(error, true);
        m_view->showError(error);
        return;
    }

    if (note.textKnown)
        m_view->setNote(note.text);

    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已加载：%1").arg(m_service->storagePath()));
}

void ProjectNotePresenter::onSaveRequested(const QString &text)
{
    if (!m_view || !m_service)
        return;

    m_view->setBusy(true);
    QString error;
    if (!m_service->saveNote(text, &error)) {
        m_view->setBusy(false);
        m_view->setStatus(error, true);
        m_view->showError(error);
        return;
    }

    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存：%1").arg(m_service->storagePath()));
}
