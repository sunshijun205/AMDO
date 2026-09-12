#include "controller/analysispresenter.h"

#include "analysispage.h"
#include "service/analysisdocumentservice.h"

AnalysisPresenter::AnalysisPresenter(AnalysisPage *view, AnalysisDocumentService *document,
                                     QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_document(document)
{
    connect(m_view, &AnalysisPage::saveRequested, this, &AnalysisPresenter::onSaveRequested);
    connect(m_view, &AnalysisPage::validateRequested, this, &AnalysisPresenter::onValidateRequested);
    connect(m_view, &AnalysisPage::publishRequested, this, &AnalysisPresenter::onPublishRequested);
    connect(m_view, &AnalysisPage::switchBaselineRequested,
            this, &AnalysisPresenter::onSwitchBaselineRequested);
    connect(m_view, &AnalysisPage::copyToDraftRequested,
            this, &AnalysisPresenter::onCopyToDraftRequested);

    onLoadRequested();
}

void AnalysisPresenter::refreshView()
{
    QVector<AnalysisBaselineInfo> baselines;
    QString listError;
    m_document->listBaselines(&baselines, &listError);
    m_view->setDocument(m_document->current());
    m_view->setReadOnly(m_document->viewingReadOnly());
    m_view->setBaselines(baselines, m_document->viewingId());
}

void AnalysisPresenter::onLoadRequested()
{
    QString error;
    if (!m_document->loadDraft(&error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    refreshView();
    m_view->setStatus(QString::fromUtf8("已加载分析集：%1").arg(m_document->draftPath()));
}

void AnalysisPresenter::onSaveRequested()
{
    if (m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("只读分析集版本，请先“另存为新草稿”再编辑"), true);
        return;
    }
    QHash<QString, QString> values;
    m_view->snapshot(&values);
    QString error;
    if (!m_document->saveValues(values, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    m_view->setStatus(QString::fromUtf8("已保存分析集：%1").arg(m_document->draftPath()));
}

void AnalysisPresenter::onValidateRequested()
{
    if (!m_document->viewingReadOnly()) {
        QHash<QString, QString> values;
        m_view->snapshot(&values);
        QString error;
        if (!m_document->saveValues(values, &error)) {
            m_view->showError(error);
            m_view->setStatus(error, true);
            return;
        }
    }
    const QStringList issues = m_document->validate();
    m_view->reportValidation(issues);
    m_view->setStatus(issues.isEmpty()
                          ? QString::fromUtf8("配置校验通过")
                          : QString::fromUtf8("配置校验发现 %1 项待处理").arg(issues.size()),
                      !issues.isEmpty());
}

void AnalysisPresenter::onPublishRequested()
{
    if (m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("只读版本无法发布，请先切回草稿"), true);
        return;
    }
    QHash<QString, QString> values;
    m_view->snapshot(&values);
    QString error;
    if (!m_document->saveValues(values, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    AnalysisDocument published;
    if (!m_document->publishBaseline(&error, &published)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    refreshView();
    m_view->setStatus(QString::fromUtf8("已发布分析集版本：%1").arg(published.id));
}

void AnalysisPresenter::onSwitchBaselineRequested(const QString &id)
{
    QString error;
    if (!m_document->viewingReadOnly() && id != QLatin1String("draft")) {
        QHash<QString, QString> values;
        m_view->snapshot(&values);
        if (!m_document->saveValues(values, &error)) {
            m_view->showError(error);
            m_view->setStatus(error, true);
            return;
        }
    }
    if (!m_document->openBaseline(id, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    refreshView();
    if (m_document->viewingReadOnly())
        m_view->setStatus(QString::fromUtf8("只读分析集版本：%1").arg(m_document->current().id));
    else
        m_view->setStatus(QString::fromUtf8("已切换到分析集草稿"));
}

void AnalysisPresenter::onCopyToDraftRequested()
{
    if (!m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("当前已是可编辑草稿"), false);
        return;
    }
    QString error;
    if (!m_document->copyBaselineToDraft(m_document->current().id, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    refreshView();
    m_view->setStatus(QString::fromUtf8("已复制为新草稿：%1").arg(m_document->draftPath()));
}
