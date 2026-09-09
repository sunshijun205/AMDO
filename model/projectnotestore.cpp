#include "model/projectnotestore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

QString ProjectNoteStore::filePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty())
        dir = QCoreApplication::applicationDirPath();
    QDir().mkpath(dir);
    return dir + QLatin1String("/mvp_project_note.txt");
}

bool ProjectNoteStore::load(ProjectNote *out, QString *errorMessage) const
{
    if (!out) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("内部错误：输出参数为空");
        return false;
    }

    const QString path = filePath();
    QFile file(path);
    if (!file.exists()) {
        out->text.clear();
        out->textKnown = true;
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QString::fromUtf8("无法打开备注文件：%1").arg(file.errorString());
        }
        out->textKnown = false;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    out->text = stream.readAll();
    out->textKnown = true;
    return true;
}

bool ProjectNoteStore::save(const ProjectNote &note, QString *errorMessage) const
{
    const QString path = filePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString::fromUtf8("无法写入备注文件：%1").arg(file.errorString());
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << note.text;
    if (stream.status() != QTextStream::Ok) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("写入备注时发生流错误");
        return false;
    }
    return true;
}
