#include "workflowpage.h"

#include "controller/workflowpresenter.h"
#include "model/aircraftstore.h"
#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "service/aircraftcpacsservice.h"
#include "service/aircraftdocumentservice.h"
#include "service/analysiscomputeservice.h"
#include "service/evaluationservice.h"
#include "service/studyservice.h"
#include "service/workflowservice.h"
#include "uihelpers.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *makeHostWidget()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    return w;
}

static void setHostContent(QWidget *host, QWidget *content)
{
    if (!host || !host->layout())
        return;
    QLayout *l = host->layout();
    QLayoutItem *item = nullptr;
    while ((item = l->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    l->addWidget(content);
}

static QWidget *flowRow(const QList<QFrame *> &nodes)
{
    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(9);
    hl->addStretch();
    for (int i = 0; i < nodes.size(); ++i) {
        hl->addWidget(nodes[i]);
        if (i + 1 < nodes.size())
            hl->addWidget(makeFlowArrow());
    }
    hl->addStretch();
    return row;
}

// ---- 流程定义与编排（可编排项只读展示；随所选模板刷新）------------------------
// TODO：可视化拖拽编排、分支/循环/迭代、节点级编辑（阶段二起）。
QWidget *WorkflowPage::buildDefinitionPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *panel = makePanel();
    panel->layout()->addWidget(makePanelTitle(
        QString::fromUtf8("流程定义与编排"),
        QString::fromUtf8("预置模板 · 顺序执行"),
        QString::fromUtf8("复用现有模块串成自动闭环；节点链随「执行」页所选模板刷新")));
    m_defDesc = new QLabel;
    m_defDesc->setObjectName(QStringLiteral("NoteLabel"));
    m_defDesc->setWordWrap(true);
    panel->layout()->addWidget(m_defDesc);
    m_defFlowHost = makeHostWidget();
    panel->layout()->addWidget(m_defFlowHost);
    lay->addWidget(panel);

    auto *nodePanel = makePanel();
    nodePanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("节点清单与连接"),
                                                  QString::fromUtf8("复用模块 · 保真度")));
    m_defNodeHost = makeHostWidget();
    nodePanel->layout()->addWidget(m_defNodeHost);
    lay->addWidget(nodePanel);

    auto *note = new QLabel(QString::fromUtf8(
        "说明：本工作流按所选模板一键顺序串跑并记录节点状态/耗时/日志。物理保真沿用各模块现状"
        "（气动真实估算，其余学科 MOCK）；并行/HPC/许可证/真 MDO 循环暂不在范围内。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    lay->addWidget(note);
    lay->addStretch();
    return root;
}

// ---- 执行 -------------------------------------------------------------------
QWidget *WorkflowPage::buildExecutionPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *setPanel = makePanel();
    setPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("执行设置"),
                                                 QString::fromUtf8("同步运行 · 单次")));

    auto *tplBar = new QWidget;
    auto *tl = new QHBoxLayout(tplBar);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(8);
    auto *tplLabel = new QLabel(QString::fromUtf8("工作流模板"));
    tplLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_templateBox = new QComboBox;
    m_templateBox->setMinimumWidth(240);
    const QVector<WorkflowTemplate> tpls = WorkflowService::templates();
    for (int i = 0; i < tpls.size(); ++i)
        m_templateBox->addItem(tpls[i].name, tpls[i].id);
    tl->addWidget(tplLabel);
    tl->addWidget(m_templateBox);
    tl->addStretch();
    setPanel->layout()->addWidget(tplBar);
    connect(m_templateBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int) { if (!m_updating) reloadTemplateView(); });

    auto *baseBar = new QWidget;
    auto *bl = new QHBoxLayout(baseBar);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);
    auto *baseLabel = new QLabel(QString::fromUtf8("基准分析集"));
    baseLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_baseBox = new QComboBox;
    m_baseBox->setMinimumWidth(240);
    bl->addWidget(baseLabel);
    bl->addWidget(m_baseBox);
    bl->addStretch();
    setPanel->layout()->addWidget(baseBar);

    auto *retryBar = new QWidget;
    auto *rtl = new QHBoxLayout(retryBar);
    rtl->setContentsMargins(0, 0, 0, 0);
    rtl->setSpacing(8);
    auto *retryLabel = new QLabel(QString::fromUtf8("失败重试"));
    retryLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_retrySpin = new QSpinBox;
    m_retrySpin->setRange(0, 3);
    m_retrySpin->setValue(0);
    m_retrySpin->setSuffix(QString::fromUtf8(" 次"));
    rtl->addWidget(retryLabel);
    rtl->addWidget(m_retrySpin);
    rtl->addStretch();
    setPanel->layout()->addWidget(retryBar);

    m_promoteCheck = makeCheck(QString::fromUtf8("完成后提升最优为新的飞机方案版本（仅探索模板）"), false);
    setPanel->layout()->addWidget(m_promoteCheck);

    auto *runBar = new QWidget;
    auto *rl = new QHBoxLayout(runBar);
    rl->setContentsMargins(0, 0, 0, 0);
    auto *hint = new QLabel(QString::fromUtf8(
        "运行模式：单次（同步）。运行后到「运行监控」查看逐节点状态/耗时与汇总。"));
    hint->setObjectName(QStringLiteral("NoteLabel"));
    hint->setWordWrap(true);
    auto *runBtn = makeButton(QString::fromUtf8("运行工作流"), true);
    connect(runBtn, &QPushButton::clicked, this, &WorkflowPage::runWorkflowRequested);
    rl->addWidget(hint, 1);
    rl->addWidget(runBtn);
    setPanel->layout()->addWidget(runBar);
    lay->addWidget(setPanel);

    auto *histPanel = makePanel();
    histPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("历史运行"),
                                                  QString::fromUtf8("运行记录 · 可复现")));
    auto *histBar = new QWidget;
    auto *hl = new QHBoxLayout(histBar);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(8);
    m_historyBox = new QComboBox;
    m_historyBox->setMinimumWidth(300);
    auto *replayBtn = makeButton(QString::fromUtf8("复现选中运行"));
    connect(replayBtn, &QPushButton::clicked, this, &WorkflowPage::onReplaySelected);
    hl->addWidget(m_historyBox, 1);
    hl->addWidget(replayBtn);
    histPanel->layout()->addWidget(histBar);
    lay->addWidget(histPanel);

    m_status = new QLabel(QString::fromUtf8("就绪：选择模板与基准分析集后点「运行工作流」。"));
    m_status->setObjectName(QStringLiteral("PageStatus"));
    lay->addWidget(m_status);
    lay->addStretch();
    return root;
}

// ---- 运行监控 ---------------------------------------------------------------
QWidget *WorkflowPage::buildMonitorPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *sumPanel = makePanel();
    sumPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行汇总")));
    m_summaryHost = makeHostWidget();
    sumPanel->layout()->addWidget(m_summaryHost);
    lay->addWidget(sumPanel);

    auto *nodePanel = makePanel();
    nodePanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("节点状态"),
                                                  QString::fromUtf8("待运行 → 运行中 → 完成/失败/跳过")));
    m_nodeHost = makeHostWidget();
    nodePanel->layout()->addWidget(m_nodeHost);
    lay->addWidget(nodePanel);

    auto *logPanel = makePanel();
    logPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行日志")));
    m_logView = new QPlainTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setMinimumHeight(140);
    m_logView->setPlaceholderText(QString::fromUtf8("尚无运行日志。"));
    logPanel->layout()->addWidget(m_logView);
    lay->addWidget(logPanel);

    setHostContent(m_summaryHost, makeKpis({
        {QString::fromUtf8("汇总"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("汇总"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("汇总"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("状态"), QString::fromUtf8("—"), QString()}
    }));
    setHostContent(m_nodeHost, makeTable(
        {QString::fromUtf8("节点"), QString::fromUtf8("类型"), QString::fromUtf8("保真"),
         QString::fromUtf8("状态"), QString::fromUtf8("耗时"), QString::fromUtf8("说明")},
        {}, TableOptions{}));
    lay->addStretch();
    return root;
}

QString WorkflowPage::templateId() const
{
    return m_templateBox ? m_templateBox->currentData().toString() : QStringLiteral("explore");
}

QString WorkflowPage::baseObjectId() const
{
    return m_baseBox ? m_baseBox->currentData().toString() : QStringLiteral("draft");
}

bool WorkflowPage::promoteBest() const
{
    return m_promoteCheck && m_promoteCheck->isEnabled() && m_promoteCheck->isChecked();
}

int WorkflowPage::retryLimit() const
{
    return m_retrySpin ? m_retrySpin->value() : 0;
}

void WorkflowPage::reloadTemplateView()
{
    const QString id = templateId();
    const WorkflowTemplate tpl = WorkflowService::templateById(id);

    if (m_defDesc)
        m_defDesc->setText(tpl.description);

    QList<QFrame *> frames;
    for (int i = 0; i < tpl.nodes.size(); ++i) {
        const WorkflowNodeSpec &s = tpl.nodes[i];
        frames << makeFlowNode(s.id + QLatin1Char(' ') + s.name,
                               s.type + QString::fromUtf8(" · ") + s.fidelity,
                               s.optional ? QString() : QStringLiteral("done"));
    }
    if (m_defFlowHost)
        setHostContent(m_defFlowHost, flowRow(frames));

    QVector<QStringList> rows;
    for (int i = 0; i < tpl.nodes.size(); ++i) {
        const WorkflowNodeSpec &s = tpl.nodes[i];
        rows.append({
            s.id + QLatin1Char(' ') + s.name,
            s.type,
            s.module + (s.optional ? QString::fromUtf8("（可选）") : QString()),
            s.output,
            s.fidelity
        });
    }
    if (m_defNodeHost)
        setHostContent(m_defNodeHost, makeTable(
            {QString::fromUtf8("节点"), QString::fromUtf8("类型"), QString::fromUtf8("复用模块"),
             QString::fromUtf8("产物"), QString::fromUtf8("保真")},
            rows, TableOptions{}));

    if (m_promoteCheck) {
        m_promoteCheck->setEnabled(tpl.supportsPromote);
        if (!tpl.supportsPromote)
            m_promoteCheck->setChecked(false);
    }
}

void WorkflowPage::reloadBaseOptions()
{
    if (!m_baseBox || !m_analysisStore)
        return;
    m_updating = true;
    m_baseBox->clear();
    m_baseBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    QVector<AnalysisBaselineInfo> baselines;
    QString detail;
    if (m_analysisStore->listBaselines(&baselines, &detail)) {
        for (int i = 0; i < baselines.size(); ++i) {
            QString label = baselines[i].id;
            if (baselines[i].version > 0)
                label += QString::fromUtf8("  v%1").arg(baselines[i].version);
            m_baseBox->addItem(label, baselines[i].id);
        }
    }
    m_updating = false;
}

void WorkflowPage::reloadHistory()
{
    if (!m_historyBox || !m_workflow)
        return;
    m_updating = true;
    m_historyBox->clear();
    m_runs.clear();
    QString detail;
    if (m_workflow->listRuns(&m_runs, &detail)) {
        for (int i = 0; i < m_runs.size(); ++i) {
            const WorkflowRunResult &r = m_runs[i];
            QString label = QString::fromUtf8("%1 · %2 · 基准 %3%4")
                                .arg(r.runId,
                                     r.templateName.isEmpty() ? r.templateId : r.templateName,
                                     r.baseObjectId,
                                     r.promotedId.isEmpty() ? QString() : QString::fromUtf8(" · 已提升"));
            m_historyBox->addItem(label, i);
        }
    }
    if (m_runs.isEmpty())
        m_historyBox->addItem(QString::fromUtf8("（暂无历史运行）"), -1);
    m_updating = false;
}

void WorkflowPage::onReplaySelected()
{
    if (m_updating || !m_historyBox)
        return;
    const int idx = m_historyBox->currentData().toInt();
    if (idx < 0 || idx >= m_runs.size()) {
        setStatus(QString::fromUtf8("请选择一条历史运行记录。"), true);
        return;
    }
    showRunResult(m_runs[idx]);
    setStatus(QString::fromUtf8("已复现运行记录：%1").arg(m_runs[idx].runId));
}

void WorkflowPage::showRunResult(const WorkflowRunResult &result)
{
    // 汇总：KPI(来自 result.summary) + 头条摘要。
    auto *sum = new QWidget;
    auto *sl = new QVBoxLayout(sum);
    sl->setContentsMargins(0, 0, 0, 0);
    sl->setSpacing(10);

    QVector<KpiItem> kpis;
    for (int i = 0; i < result.summary.size(); ++i)
        kpis.append({result.summary[i].label, result.summary[i].value, result.summary[i].unit});
    while (kpis.size() < 4)
        kpis.append({QString::fromUtf8("—"), QString::fromUtf8("—"), QString()});
    sl->addWidget(makeKpis(kpis));

    auto *headline = new QLabel(QString::fromUtf8("结果摘要：%1")
                                    .arg(result.headline.isEmpty() ? QString::fromUtf8("—") : result.headline));
    headline->setWordWrap(true);
    sl->addWidget(headline);
    if (!result.promotedId.isEmpty()) {
        auto *promoted = new QLabel(QString::fromUtf8("已提升为飞机方案版本：%1").arg(result.promotedId));
        promoted->setWordWrap(true);
        promoted->setObjectName(QStringLiteral("NoteLabel"));
        sl->addWidget(promoted);
    }
    auto *meta = new QLabel(QString::fromUtf8("模板 %1 · 基准 %2 · 重试上限 %3 次")
                                .arg(result.templateName.isEmpty() ? result.templateId : result.templateName,
                                     result.baseObjectId)
                                .arg(result.retryLimit));
    meta->setObjectName(QStringLiteral("NoteLabel"));
    meta->setWordWrap(true);
    sl->addWidget(meta);
    setHostContent(m_summaryHost, sum);

    // 节点状态表：节点/类型/保真/状态/耗时/说明。
    QVector<QStringList> rows;
    TableOptions opt;
    for (int i = 0; i < result.nodes.size(); ++i) {
        const WorkflowNodeResult &n = result.nodes[i];
        const QString elapsed = (n.status == QString::fromUtf8("跳过"))
                                    ? QString::fromUtf8("—")
                                    : QString::number(n.elapsedMs) + QStringLiteral("ms");
        rows.append({n.id + QLatin1Char(' ') + n.name, n.type, n.fidelity, n.status, elapsed, n.detail});
        if (n.status == QString::fromUtf8("失败"))
            opt.warnRows.append(i);
    }
    setHostContent(m_nodeHost, makeTable(
        {QString::fromUtf8("节点"), QString::fromUtf8("类型"), QString::fromUtf8("保真"),
         QString::fromUtf8("状态"), QString::fromUtf8("耗时"), QString::fromUtf8("说明")},
        rows, opt));

    if (m_logView)
        m_logView->setPlainText(result.log.join(QLatin1Char('\n')));
}

void WorkflowPage::setStatus(const QString &text, bool isError)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

WorkflowPage::WorkflowPage(QWidget *parent)
    : QWidget(parent)
{
    m_aircraftStore.reset(new AircraftStore);
    m_aircraftCpacs.reset(new AircraftCpacsService(m_aircraftStore.get()));
    m_aircraftDoc.reset(new AircraftDocumentService(m_aircraftStore.get(), m_aircraftCpacs.get()));
    m_srdStore.reset(new SrdStore);
    m_analysisStore.reset(new AnalysisStore);
    m_compute.reset(new AnalysisComputeService(m_aircraftStore.get(), m_analysisStore.get(),
                                               m_srdStore.get()));
    m_evaluation.reset(new EvaluationService(m_compute.get(), m_srdStore.get(), m_analysisStore.get()));
    m_study.reset(new StudyService(m_compute.get(), m_evaluation.get(), m_analysisStore.get()));
    m_workflow.reset(new WorkflowService(m_analysisStore.get(), m_compute.get(), m_evaluation.get(),
                                         m_study.get(), m_aircraftStore.get(), m_aircraftDoc.get()));

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("工作流"),
        QString::fromUtf8("按所选模板一键串跑：载入基准 → 计算(探索/学科分析) → 汇总/评价 → (可选)提升方案版本"),
        QString(), {}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("definition"), QString::fromUtf8("流程定义与编排")},
        {QStringLiteral("execution"), QString::fromUtf8("执行")},
        {QStringLiteral("monitor"), QString::fromUtf8("运行监控")}
    }, QStringLiteral("execution"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(buildDefinitionPage());
    stack->addWidget(buildExecutionPage());
    stack->addWidget(buildMonitorPage());
    stack->setCurrentIndex(1);
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("definition")) stack->setCurrentIndex(0);
        else if (id == QLatin1String("monitor")) stack->setCurrentIndex(2);
        else stack->setCurrentIndex(1);
    });
    outer->addWidget(wrapScroll(body));

    reloadBaseOptions();
    reloadTemplateView();
    reloadHistory();
    m_presenter = new WorkflowPresenter(this, m_workflow.get(), this);
}

WorkflowPage::~WorkflowPage() = default;
