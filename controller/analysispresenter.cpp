#include "controller/analysispresenter.h"

#include "analysispage.h"
#include "service/analysiscomputeservice.h"
#include "service/analysisdocumentservice.h"

AnalysisPresenter::AnalysisPresenter(AnalysisPage *view, AnalysisDocumentService *document,
                                     AnalysisComputeService *compute, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_document(document)
    , m_compute(compute)
{
    connect(m_view, &AnalysisPage::saveRequested, this, &AnalysisPresenter::onSaveRequested);
    connect(m_view, &AnalysisPage::validateRequested, this, &AnalysisPresenter::onValidateRequested);
    connect(m_view, &AnalysisPage::publishRequested, this, &AnalysisPresenter::onPublishRequested);
    connect(m_view, &AnalysisPage::switchBaselineRequested,
            this, &AnalysisPresenter::onSwitchBaselineRequested);
    connect(m_view, &AnalysisPage::copyToDraftRequested,
            this, &AnalysisPresenter::onCopyToDraftRequested);
    connect(m_view, &AnalysisPage::referencesChanged,
            this, &AnalysisPresenter::onReferencesChanged);
    connect(m_view, &AnalysisPage::runRequested, this, &AnalysisPresenter::onRunRequested);

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

void AnalysisPresenter::onReferencesChanged(const QString &sourceRevision, const QString &sourceCase,
                                            const QString &sourceSrd, const QString &sourceCondition)
{
    if (m_document->viewingReadOnly())
        return;
    QString error;
    if (!m_document->setReferences(sourceRevision, sourceCase, sourceSrd, sourceCondition, &error)) {
        m_view->showError(error);
        m_view->setStatus(error, true);
        return;
    }
    QStringList parts;
    parts.append(sourceRevision.isEmpty() ? QString::fromUtf8("飞机修订：未关联")
                                          : QString::fromUtf8("飞机修订 %1").arg(sourceRevision));
    if (!sourceSrd.isEmpty()) {
        QString srdPart = QString::fromUtf8("设计需求 %1").arg(sourceSrd);
        if (!sourceCondition.isEmpty())
            srdPart += QString::fromUtf8("(工况#%1)").arg(sourceCondition.toInt() + 1);
        parts.append(srdPart);
    }
    if (!sourceCase.isEmpty())
        parts.append(QString::fromUtf8("用例 %1").arg(sourceCase));
    m_view->setStatus(QString::fromUtf8("已更新关联：") + parts.join(QString::fromUtf8(" · ")),
                      sourceRevision.isEmpty());
}

void AnalysisPresenter::onRunRequested()
{
    if (!m_compute) {
        m_view->setStatus(QString::fromUtf8("计算服务未初始化"), true);
        return;
    }
    // 可编辑草稿：先落盘最新编辑，确保计算基于当前配置。
    if (!m_document->viewingReadOnly()) {
        QHash<QString, QString> values;
        m_view->snapshot(&values);
        QString saveError;
        if (!m_document->saveValues(values, &saveError)) {
            m_view->showError(saveError);
            m_view->setStatus(saveError, true);
            return;
        }
    }

    QString error;
    const AnalysisRunResult result = m_compute->run(m_document->current(), &error);

    QString outPath;
    QString saveError;
    if (!m_compute->saveResult(result, &outPath, &saveError)) {
        // 结果展示不受落盘失败影响，仅提示。
        m_view->setStatus(QString::fromUtf8("计算完成，但结果保存失败：%1").arg(saveError), true);
    } else {
        m_view->setStatus(QString::fromUtf8("学科计算完成，结果已保存：%1").arg(outPath));
    }
    m_view->showRunResult(result);
}
