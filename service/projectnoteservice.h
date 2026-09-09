#ifndef PROJECTNOTESERVICE_H
#define PROJECTNOTESERVICE_H

#include "model/projectnote.h"
#include "model/projectnotestore.h"

#include <QString>

// 用例：加载 / 保存方案备注
class ProjectNoteService
{
public:
    explicit ProjectNoteService(ProjectNoteStore *store);

    bool loadNote(ProjectNote *out, QString *errorMessage = nullptr);
    bool saveNote(const QString &text, QString *errorMessage = nullptr);
    QString storagePath() const;

private:
    ProjectNoteStore *m_store;
};

#endif
