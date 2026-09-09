#include "projectnotepanel.h"
#include "uihelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

ProjectNotePanel::ProjectNotePanel(QWidget *parent)
    : QFrame(parent)
{
    setObjectName(QStringLiteral("Panel"));
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(14, 12, 14, 12);
    lay->setSpacing(10);

    lay->addWidget(makePanelTitle(
        QString::fromUtf8("方案备注（MVP 示例）"),
        QString::fromUtf8("View → Presenter → Service → 本地文件")));

    auto *hint = new QLabel(QString::fromUtf8(
        "演示分层闭环：加载真实本地文件 → 编辑 → 保存落盘。非演示伪数据。"));
    hint->setObjectName(QStringLiteral("MutedLabel"));
    hint->setWordWrap(true);
    lay->addWidget(hint);

    m_editor = new QPlainTextEdit;
    m_editor->setObjectName(QStringLiteral("NoteEditor"));
    m_editor->setPlaceholderText(QString::fromUtf8("在此填写方案备注…"));
    m_editor->setMinimumHeight(72);
    m_editor->setMaximumHeight(120);
    lay->addWidget(m_editor);

    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(8);

    m_status = new QLabel;
    m_status->setObjectName(QStringLiteral("MutedLabel"));
    m_status->setWordWrap(true);

    m_reload = makeButton(QString::fromUtf8("重新加载"));
    m_save = makeButton(QString::fromUtf8("保存备注"), true);

    hl->addWidget(m_status, 1);
    hl->addWidget(m_reload);
    hl->addWidget(m_save);
    lay->addWidget(row);

    connect(m_reload, &QPushButton::clicked, this, &ProjectNotePanel::loadRequested);
    connect(m_save, &QPushButton::clicked, this, [this]() {
        emit saveRequested(m_editor->toPlainText());
    });
}

void ProjectNotePanel::setNote(const QString &text)
{
    m_editor->setPlainText(text);
}

void ProjectNotePanel::setBusy(bool busy)
{
    m_editor->setEnabled(!busy);
    m_reload->setEnabled(!busy);
    m_save->setEnabled(!busy);
}

void ProjectNotePanel::setStatus(const QString &text, bool isError)
{
    m_status->setText(text);
    if (isError)
        m_status->setStyleSheet(QStringLiteral("color: #b46b22;"));
    else
        m_status->setStyleSheet(QString());
}

void ProjectNotePanel::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("方案备注"), message);
}
