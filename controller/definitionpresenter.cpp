#include "controller/definitionpresenter.h"

#include "definitionpage.h"
#include "model/aircrafttypes.h"
#include "service/aircraftdocumentservice.h"
#include "service/aircraftimportexportservice.h"

#include <QStringList>

DefinitionPresenter::DefinitionPresenter(DefinitionPage *view,
                                         AircraftDocumentService *document,
                                         AircraftImportExportService *io,
                                         QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_document(document)
    , m_io(io)
{
    connect(m_view, &DefinitionPage::loadRequested, this, &DefinitionPresenter::onLoadRequested);
    connect(m_view, &DefinitionPage::switchBaselineRequested,
            this, &DefinitionPresenter::onSwitchBaselineRequested);
    connect(m_view, &DefinitionPage::copyBaselineRequested,
            this, &DefinitionPresenter::onCopyBaselineRequested);
    connect(m_view, &DefinitionPage::newBlankRequested,
            this, &DefinitionPresenter::onNewBlankRequested);
    connect(m_view, &DefinitionPage::newFromTemplateRequested,
            this, &DefinitionPresenter::onNewFromTemplateRequested);
    connect(m_view, &DefinitionPage::saveAllRequested,
            this, &DefinitionPresenter::onSaveAllRequested);
    connect(m_view, &DefinitionPage::saveSemanticsRequested,
            this, &DefinitionPresenter::onSaveSemanticsRequested);
    connect(m_view, &DefinitionPage::saveConfigurationRequested,
            this, &DefinitionPresenter::onSaveConfigurationRequested);
    connect(m_view, &DefinitionPage::saveParametersRequested,
            this, &DefinitionPresenter::onSaveParametersRequested);
    connect(m_view, &DefinitionPage::addSystemRequested,
            this, &DefinitionPresenter::onAddSystemRequested);
    connect(m_view, &DefinitionPage::removeSystemRequested,
            this, &DefinitionPresenter::onRemoveSystemRequested);
    connect(m_view, &DefinitionPage::addParameterRequested,
            this, &DefinitionPresenter::onAddParameterRequested);
    connect(m_view, &DefinitionPage::removeParameterRequested,
            this, &DefinitionPresenter::onRemoveParameterRequested);
    connect(m_view, &DefinitionPage::publishRequested,
            this, &DefinitionPresenter::onPublishRequested);
    connect(m_view, &DefinitionPage::caseSnapshotRequested,
            this, &DefinitionPresenter::onCaseSnapshotRequested);
    connect(m_view, &DefinitionPage::importPathRequested,
            this, &DefinitionPresenter::onImportPathRequested);
    connect(m_view, &DefinitionPage::exportRequested,
            this, &DefinitionPresenter::onExportRequested);
    connect(m_view, &DefinitionPage::integrityCheckRequested,
            this, &DefinitionPresenter::onIntegrityCheckRequested);

    onLoadRequested();
}

void DefinitionPresenter::fail(const QString &error)
{
    m_view->setBusy(false);
    m_view->setStatus(error, true);
    m_view->showError(error);
}

void DefinitionPresenter::refreshView()
{
    QVector<AircraftBaselineInfo> baselines;
    QString listError;
    m_document->listBaselines(&baselines, &listError);
    m_view->setDocument(m_document->current(), m_document->viewingReadOnly());
    m_view->setBaselines(baselines, m_document->viewingId());
}

bool DefinitionPresenter::persistAll(QString *errorMessage)
{
    AcSemantics semantics;
    QString title;
    m_view->snapshotSemantics(&semantics, &title);
    if (!m_document->setSemantics(semantics, title, errorMessage))
        return false;

    AcConfiguration configuration;
    QVector<AcSystemItem> systems;
    m_view->snapshotConfiguration(&configuration, &systems);
    if (!m_document->setConfiguration(configuration, systems, errorMessage))
        return false;

    QVector<AcParameter> parameters;
    m_view->snapshotParameters(&parameters);
    if (!m_document->setParameters(parameters, errorMessage))
        return false;
    return true;
}

void DefinitionPresenter::onLoadRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->loadDraft(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已加载方案草稿：%1").arg(m_document->draftPath()));
}

void DefinitionPresenter::onSwitchBaselineRequested(const QString &id)
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->viewingReadOnly() && id != QLatin1String("draft")) {
        if (!persistAll(&error)) {
            fail(error);
            return;
        }
    }
    if (!m_document->openBaseline(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    if (m_document->viewingReadOnly())
        m_view->setStatus(QString::fromUtf8("只读方案版本：%1").arg(m_document->current().id));
    else
        m_view->setStatus(QString::fromUtf8("已切换到方案草稿：%1").arg(m_document->draftPath()));
}

void DefinitionPresenter::onCopyBaselineRequested()
{
    if (!m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("当前已是可编辑草稿"), false);
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!m_document->copyBaselineToDraft(m_document->current().id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已复制为新草稿：%1").arg(m_document->draftPath()));
}

void DefinitionPresenter::onNewBlankRequested()
{
    if (m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("请先切回草稿再新建"), true);
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!m_document->newBlank(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已新建空白方案草稿"));
}

void DefinitionPresenter::onNewFromTemplateRequested(const QString &templateName)
{
    if (m_document->viewingReadOnly()) {
        m_view->setStatus(QString::fromUtf8("请先切回草稿再新建"), true);
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!m_document->newFromTemplate(templateName, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已按模板新建方案草稿：%1").arg(templateName));
}

void DefinitionPresenter::onSaveAllRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存方案定义：%1").arg(m_document->draftPath()));
}

void DefinitionPresenter::onSaveSemanticsRequested()
{
    m_view->setBusy(true);
    QString error;
    AcSemantics semantics;
    QString title;
    m_view->snapshotSemantics(&semantics, &title);
    if (!m_document->setSemantics(semantics, title, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存语义数据模型"));
}

void DefinitionPresenter::onSaveConfigurationRequested()
{
    m_view->setBusy(true);
    QString error;
    AcConfiguration configuration;
    QVector<AcSystemItem> systems;
    m_view->snapshotConfiguration(&configuration, &systems);
    if (!m_document->setConfiguration(configuration, systems, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存总体方案配置"));
}

void DefinitionPresenter::onSaveParametersRequested()
{
    m_view->setBusy(true);
    QString error;
    QVector<AcParameter> parameters;
    m_view->snapshotParameters(&parameters);
    if (!m_document->setParameters(parameters, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存参数化几何"));
}

void DefinitionPresenter::onAddSystemRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addSystem(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已添加系统对象"));
}

void DefinitionPresenter::onRemoveSystemRequested()
{
    const QString id = m_view->selectedSystemId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的系统对象"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeSystem(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除系统对象 %1").arg(id));
}

void DefinitionPresenter::onAddParameterRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addParameter(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已添加几何参数"));
}

void DefinitionPresenter::onRemoveParameterRequested()
{
    const QString id = m_view->selectedParameterId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的几何参数"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeParameter(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除几何参数 %1").arg(id));
}

void DefinitionPresenter::onPublishRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    AircraftDocument published;
    if (!m_document->publishBaseline(&error, &published)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已创建方案版本 %1，CPACS 主数据：%2")
                          .arg(published.id, m_document->cpacsPathForVersion(published.version)));
}

void DefinitionPresenter::onCaseSnapshotRequested()
{
    m_view->setBusy(true);
    QString error;
    // 可编辑草稿：先落盘最新编辑，确保快照反映当前界面内容。
    if (!m_document->viewingReadOnly() && !persistAll(&error)) {
        fail(error);
        return;
    }
    QString outPath;
    if (!m_document->createCaseSnapshot(&outPath, &error)) {
        fail(error);
        return;
    }
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已冻结不可变分析用例快照：%1").arg(outPath));
}

void DefinitionPresenter::onImportPathRequested(const QString &path)
{
    m_view->setBusy(true);
    QString error;
    AircraftDocument doc;
    if (!m_io->importAircraftJson(path, &doc, &error)) {
        fail(error);
        return;
    }
    if (!m_document->replaceDocument(doc, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已导入方案文件：%1").arg(path));
}

void DefinitionPresenter::onExportRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->viewingReadOnly() && !persistAll(&error)) {
        fail(error);
        return;
    }
    const QString path = m_io->defaultExportPath(m_document->current());
    if (!m_io->exportAircraftJson(path, m_document->current(), &error)) {
        fail(error);
        return;
    }
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已导出方案：%1").arg(path));
}

void DefinitionPresenter::onIntegrityCheckRequested()
{
    if (!m_document->viewingReadOnly()) {
        QString error;
        if (!persistAll(&error)) {
            fail(error);
            return;
        }
    }
    const AircraftDocument &d = m_document->current();
    QStringList issues;
    if (d.configuration.category.isEmpty())
        issues.append(QString::fromUtf8("构型类别未填写"));
    if (d.semantics.lengthUnit.isEmpty() || d.semantics.massUnit.isEmpty())
        issues.append(QString::fromUtf8("单位（长度/质量）未完整定义"));
    if (d.semantics.coordinateSystem.isEmpty())
        issues.append(QString::fromUtf8("坐标系语义未定义"));
    if (d.systems.isEmpty())
        issues.append(QString::fromUtf8("尚无系统/设备对象"));
    if (d.parameters.isEmpty())
        issues.append(QString::fromUtf8("尚无几何参数"));
    int pending = 0;
    for (int i = 0; i < d.systems.size(); ++i) {
        if (d.systems[i].status.contains(QString::fromUtf8("待")))
            ++pending;
    }
    if (pending > 0)
        issues.append(QString::fromUtf8("%1 个系统对象状态为“待确认”").arg(pending));
    int unknown = 0;
    for (int i = 0; i < d.parameters.size(); ++i) {
        if (!d.parameters[i].valueKnown)
            ++unknown;
    }
    if (unknown > 0)
        issues.append(QString::fromUtf8("%1 个几何参数数值未填写").arg(unknown));

    m_view->reportIntegrity(issues);
    m_view->setStatus(issues.isEmpty()
                          ? QString::fromUtf8("完整性检查通过")
                          : QString::fromUtf8("完整性检查发现 %1 项待处理").arg(issues.size()));
}
