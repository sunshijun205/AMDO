#include "controller/requirementspresenter.h"

#include "model/srdtypes.h"
#include "requirementspage.h"
#include "service/srdcompletenessservice.h"
#include "service/srdderivationservice.h"
#include "service/srddocumentservice.h"
#include "service/srdimportexportservice.h"

#include <QStringList>

RequirementsPresenter::RequirementsPresenter(RequirementsPage *view,
                                             SrdDocumentService *document,
                                             SrdCompletenessService *completeness,
                                             SrdDerivationService *derivation,
                                             SrdImportExportService *io,
                                             QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_document(document)
    , m_completeness(completeness)
    , m_derivation(derivation)
    , m_io(io)
{
    connect(m_view, &RequirementsPage::loadRequested, this, &RequirementsPresenter::onLoadRequested);
    connect(m_view, &RequirementsPage::switchBaselineRequested,
            this, &RequirementsPresenter::onSwitchBaselineRequested);
    connect(m_view, &RequirementsPage::copyBaselineRequested,
            this, &RequirementsPresenter::onCopyBaselineRequested);
    connect(m_view, &RequirementsPage::saveMissionRequested,
            this, &RequirementsPresenter::onSaveMissionRequested);
    connect(m_view, &RequirementsPage::addScenarioRequested,
            this, &RequirementsPresenter::onAddScenarioRequested);
    connect(m_view, &RequirementsPage::removeScenarioRequested,
            this, &RequirementsPresenter::onRemoveScenarioRequested);
    connect(m_view, &RequirementsPage::addNeedRequested,
            this, &RequirementsPresenter::onAddNeedRequested);
    connect(m_view, &RequirementsPage::removeNeedRequested,
            this, &RequirementsPresenter::onRemoveNeedRequested);
    connect(m_view, &RequirementsPage::generateConditionsRequested,
            this, &RequirementsPresenter::onGenerateConditionsRequested);
    connect(m_view, &RequirementsPage::saveEnvelopeRequested,
            this, &RequirementsPresenter::onSaveEnvelopeRequested);
    connect(m_view, &RequirementsPage::addConditionRequested,
            this, &RequirementsPresenter::onAddConditionRequested);
    connect(m_view, &RequirementsPage::removeConditionRequested,
            this, &RequirementsPresenter::onRemoveConditionRequested);
    connect(m_view, &RequirementsPage::saveStandardsRequested,
            this, &RequirementsPresenter::onSaveStandardsRequested);
    connect(m_view, &RequirementsPage::freezeStandardsRequested,
            this, &RequirementsPresenter::onFreezeStandardsRequested);
    connect(m_view, &RequirementsPage::unfreezeStandardsRequested,
            this, &RequirementsPresenter::onUnfreezeStandardsRequested);
    connect(m_view, &RequirementsPage::applyApplicabilityRequested,
            this, &RequirementsPresenter::onApplyApplicabilityRequested);
    connect(m_view, &RequirementsPage::generateRequirementsRequested,
            this, &RequirementsPresenter::onGenerateRequirementsRequested);
    connect(m_view, &RequirementsPage::saveMetricsRequested,
            this, &RequirementsPresenter::onSaveMetricsRequested);
    connect(m_view, &RequirementsPage::addRequirementRequested,
            this, &RequirementsPresenter::onAddRequirementRequested);
    connect(m_view, &RequirementsPage::removeRequirementRequested,
            this, &RequirementsPresenter::onRemoveRequirementRequested);
    connect(m_view, &RequirementsPage::publishRequested,
            this, &RequirementsPresenter::onPublishRequested);
    connect(m_view, &RequirementsPage::importPathRequested,
            this, &RequirementsPresenter::onImportPathRequested);
    connect(m_view, &RequirementsPage::exportSpecRequested,
            this, &RequirementsPresenter::onExportSpecRequested);

    onLoadRequested();
}

void RequirementsPresenter::fail(const QString &error)
{
    m_view->setBusy(false);
    m_view->setStatus(error, true);
    m_view->showError(error);
}

void RequirementsPresenter::refreshView()
{
    QVector<SrdBaselineInfo> baselines;
    QString listError;
    m_document->listBaselines(&baselines, &listError);
    const SrdCompletenessReport report = m_completeness->evaluate(m_document->current());
    m_view->setDocument(m_document->current(), m_document->viewingReadOnly());
    m_view->setCompleteness(report);
    m_view->setBaselines(baselines, m_document->viewingId());
}

bool RequirementsPresenter::persistAll(QString *errorMessage)
{
    QVector<SrdMissionScenario> missions;
    QVector<SrdPerformanceNeed> needs;
    m_view->snapshotMissionChapter(&missions, &needs);
    if (!m_document->setMissionChapter(missions, needs, errorMessage))
        return false;

    SrdEnvironment env;
    QVector<SrdEnvelopePoint> points;
    QVector<SrdFlightCondition> conditions;
    m_view->snapshotEnvelopeChapter(&env, &points, &conditions);
    if (!m_document->setEnvelopeChapter(env, points, conditions, errorMessage))
        return false;

    SrdCertification cert;
    QVector<SrdClauseRow> clauses;
    m_view->snapshotStandardsChapter(&cert, &clauses);
    if (!m_document->current().chapters.standardsFrozen) {
        if (!m_document->setStandardsChapter(cert, clauses, errorMessage))
            return false;
    }

    QVector<SrdRequirement> reqs;
    m_view->snapshotMetricsChapter(&reqs);
    if (!m_document->setMetricsChapter(reqs, errorMessage))
        return false;
    return true;
}

void RequirementsPresenter::onLoadRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->loadDraft(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已加载草稿：%1").arg(m_document->draftPath()));
}

void RequirementsPresenter::onSwitchBaselineRequested(const QString &id)
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
        m_view->setStatus(QString::fromUtf8("只读基线：%1").arg(m_document->current().id));
    else
        m_view->setStatus(QString::fromUtf8("已切换到草稿：%1").arg(m_document->draftPath()));
}

void RequirementsPresenter::onCopyBaselineRequested()
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

void RequirementsPresenter::onSaveMissionRequested()
{
    m_view->setBusy(true);
    QString error;
    QVector<SrdMissionScenario> missions;
    QVector<SrdPerformanceNeed> needs;
    m_view->snapshotMissionChapter(&missions, &needs);
    if (!m_document->setMissionChapter(missions, needs, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存任务需求：%1").arg(m_document->draftPath()));
}

void RequirementsPresenter::onAddScenarioRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addScenario(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已添加场景"));
}

void RequirementsPresenter::onRemoveScenarioRequested()
{
    const QString id = m_view->selectedScenarioId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的场景"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeScenario(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除场景 %1").arg(id));
}

void RequirementsPresenter::onAddNeedRequested(const QString &metricId)
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addNeedFromMetric(metricId, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已添加性能需求"));
}

void RequirementsPresenter::onRemoveNeedRequested()
{
    const QString id = m_view->selectedNeedId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的性能需求"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeNeed(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除性能需求 %1").arg(id));
}

void RequirementsPresenter::onGenerateConditionsRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    const QVector<SrdFlightCondition> suggested = m_derivation->suggestConditions(m_document->current());
    const QVector<SrdEnvelopePoint> points = m_derivation->suggestEnvelopePoints(m_document->current());
    if (suggested.isEmpty() && points.isEmpty()) {
        m_view->setBusy(false);
        m_view->setStatus(QString::fromUtf8("没有新的建议工况或包线点"));
        return;
    }
    QStringList lines;
    for (int i = 0; i < suggested.size(); ++i) {
        lines.append(suggested[i].phase + QStringLiteral(" / ") + suggested[i].altitude
                     + QStringLiteral(" / ") + suggested[i].speed);
    }
    for (int i = 0; i < points.size(); ++i) {
        lines.append(QString::fromUtf8("包线点 Ma %1 / %2 km")
                         .arg(srdFormatNumber(points[i].mach))
                         .arg(srdFormatNumber(points[i].altitudeKm)));
    }
    m_view->setBusy(false);
    if (!m_view->confirmList(QString::fromUtf8("从任务生成建议工况"), lines))
        return;

    m_view->setBusy(true);
    int addedFc = 0;
    int addedPt = 0;
    if (!suggested.isEmpty() && !m_document->mergeSuggestedConditions(suggested, &addedFc, &error)) {
        fail(error);
        return;
    }
    if (!points.isEmpty() && !m_document->mergeEnvelopePoints(points, &addedPt, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已写入建议：工况 %1 条，包线点 %2 个").arg(addedFc).arg(addedPt));
}

void RequirementsPresenter::onSaveEnvelopeRequested()
{
    m_view->setBusy(true);
    QString error;
    SrdEnvironment env;
    QVector<SrdEnvelopePoint> points;
    QVector<SrdFlightCondition> conditions;
    m_view->snapshotEnvelopeChapter(&env, &points, &conditions);
    if (!m_document->setEnvelopeChapter(env, points, conditions, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存工况与飞行包线"));
}

void RequirementsPresenter::onAddConditionRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addCondition(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已添加工况"));
}

void RequirementsPresenter::onRemoveConditionRequested()
{
    const QString id = m_view->selectedConditionId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的工况"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeCondition(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除工况 %1").arg(id));
}

void RequirementsPresenter::onSaveStandardsRequested()
{
    m_view->setBusy(true);
    QString error;
    SrdCertification cert;
    QVector<SrdClauseRow> clauses;
    m_view->snapshotStandardsChapter(&cert, &clauses);
    if (!m_document->setStandardsChapter(cert, clauses, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存规范与适用性"));
}

void RequirementsPresenter::onFreezeStandardsRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    const SrdCompletenessReport report = m_completeness->evaluate(m_document->current());
    if (!report.canFreezeStandards) {
        QStringList lines;
        for (int i = 0; i < report.items.size(); ++i) {
            if (report.items[i].blocking && report.items[i].code.startsWith(QLatin1Char('S')))
                lines.append(report.items[i].message);
        }
        fail(QString::fromUtf8("规范章未通过检查：\n") + lines.join(QLatin1Char('\n')));
        return;
    }
    if (!m_document->freezeStandards(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已发布适用性基线（规范章已冻结）"));
}

void RequirementsPresenter::onUnfreezeStandardsRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->unfreezeStandards(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已解冻规范章"));
}

void RequirementsPresenter::onApplyApplicabilityRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->applyDefaultApplicability(&error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已按认证基础填写默认适用性"));
}

void RequirementsPresenter::onGenerateRequirementsRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    const QVector<SrdRequirement> suggested = m_derivation->suggestRequirements(m_document->current());
    if (suggested.isEmpty()) {
        m_view->setBusy(false);
        m_view->setStatus(QString::fromUtf8("没有新的建议需求"));
        return;
    }
    QStringList lines;
    for (int i = 0; i < suggested.size(); ++i) {
        lines.append(suggested[i].metricName + QStringLiteral("  ")
                     + srdFormatBound(suggested[i].relation, suggested[i].boundValue,
                                      suggested[i].boundKnown, suggested[i].unit)
                     + QStringLiteral("  ← ") + suggested[i].source);
    }
    m_view->setBusy(false);
    if (!m_view->confirmList(QString::fromUtf8("从任务与条款建议需求"), lines))
        return;
    m_view->setBusy(true);
    int added = 0;
    if (!m_document->mergeSuggestedRequirements(suggested, &added, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已写入 %1 条建议需求").arg(added));
}

void RequirementsPresenter::onSaveMetricsRequested()
{
    m_view->setBusy(true);
    QString error;
    QVector<SrdRequirement> reqs;
    m_view->snapshotMetricsChapter(&reqs);
    if (!m_document->setMetricsChapter(reqs, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已保存评价指标与需求约束"));
}

void RequirementsPresenter::onAddRequirementRequested(const QString &metricId)
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->addRequirementFromMetric(metricId, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已从指标目录添加需求"));
}

void RequirementsPresenter::onRemoveRequirementRequested()
{
    const QString id = m_view->selectedRequirementId();
    if (id.isEmpty()) {
        m_view->showError(QString::fromUtf8("请先选择要删除的需求"));
        return;
    }
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error) || !m_document->removeRequirement(id, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已删除需求 %1").arg(id));
}

void RequirementsPresenter::onPublishRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!persistAll(&error)) {
        fail(error);
        return;
    }
    const SrdCompletenessReport report = m_completeness->evaluate(m_document->current());
    if (!report.canPublish) {
        QStringList lines;
        for (int i = 0; i < report.items.size(); ++i) {
            if (report.items[i].blocking)
                lines.append(report.items[i].message);
        }
        fail(QString::fromUtf8("不能发布需求基线：\n") + lines.join(QLatin1Char('\n')));
        return;
    }
    if (!m_document->publishBaseline(&error)) {
        fail(error);
        return;
    }
    const QString exportPath = m_io->defaultExportPath(m_document->current());
    QString exportError;
    m_io->exportEvaluationSpec(exportPath, m_document->current(), &exportError);
    refreshView();
    m_view->setBusy(false);
    if (!exportError.isEmpty())
        m_view->setStatus(QString::fromUtf8("已发布基线，但导出评价规格失败：%1").arg(exportError), true);
    else
        m_view->setStatus(QString::fromUtf8("已发布需求基线，评价规格：%1").arg(exportPath));
}

void RequirementsPresenter::onImportPathRequested(const QString &path)
{
    m_view->setBusy(true);
    QString error;
    if (path.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive)) {
        if (!persistAll(&error)) {
            fail(error);
            return;
        }
        QVector<SrdPerformanceNeed> needs;
        if (!m_io->importNeedsCsv(path, &needs, &error)) {
            fail(error);
            return;
        }
        int added = 0;
        if (!m_document->appendNeeds(needs, &added, &error)) {
            fail(error);
            return;
        }
        refreshView();
        m_view->setBusy(false);
        m_view->setStatus(QString::fromUtf8("已从 CSV 导入 %1 条性能需求").arg(added));
        return;
    }

    SrdDocument doc;
    if (!m_io->importSrdJson(path, &doc, &error)) {
        fail(error);
        return;
    }
    if (!m_document->replaceDocument(doc, &error)) {
        fail(error);
        return;
    }
    refreshView();
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已导入需求文件：%1").arg(path));
}

void RequirementsPresenter::onExportSpecRequested()
{
    m_view->setBusy(true);
    QString error;
    if (!m_document->viewingReadOnly() && !persistAll(&error)) {
        fail(error);
        return;
    }
    const QString path = m_io->defaultExportPath(m_document->current());
    if (!m_io->exportEvaluationSpec(path, m_document->current(), &error)) {
        fail(error);
        return;
    }
    m_view->setBusy(false);
    m_view->setStatus(QString::fromUtf8("已导出评价规格：%1").arg(path));
}
