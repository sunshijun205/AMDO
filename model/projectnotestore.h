#ifndef PROJECTNOTESTORE_H
#define PROJECTNOTESTORE_H

#include "model/projectnote.h"

#include <QString>

// 本地持久：读写方案备注文件（无业务判断）
class ProjectNoteStore
{
public:
    QString filePath() const;

    bool load(ProjectNote *out, QString *errorMessage = nullptr) const;
    bool save(const ProjectNote &note, QString *errorMessage = nullptr) const;
};

#endif
