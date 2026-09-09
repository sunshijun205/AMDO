#include "service/projectnoteservice.h"

ProjectNoteService::ProjectNoteService(ProjectNoteStore *store)
    : m_store(store)
{
}

bool ProjectNoteService::loadNote(ProjectNote *out, QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载失败：存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->load(out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("加载备注失败：%1").arg(detail);
        return false;
    }
    return true;
}

bool ProjectNoteService::saveNote(const QString &text, QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存失败：存储未初始化");
        return false;
    }

    ProjectNote note;
    note.text = text;
    note.textKnown = true;

    QString detail;
    if (!m_store->save(note, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("保存备注失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString ProjectNoteService::storagePath() const
{
    if (!m_store)
        return QString();
    return m_store->filePath();
}
