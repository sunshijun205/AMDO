#ifndef PROJECTNOTEPANEL_H
#define PROJECTNOTEPANEL_H

#include <QFrame>

class QLabel;
class QPlainTextEdit;
class QPushButton;

// View：只负责展示与意图信号，不调 Service / 不写业务
class ProjectNotePanel : public QFrame
{
    Q_OBJECT
public:
    explicit ProjectNotePanel(QWidget *parent = nullptr);

signals:
    void loadRequested();
    void saveRequested(const QString &text);

public slots:
    void setNote(const QString &text);
    void setBusy(bool busy);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);

private:
    QPlainTextEdit *m_editor = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_reload = nullptr;
    QPushButton *m_save = nullptr;
};

#endif
