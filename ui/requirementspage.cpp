#include "requirementspage.h"

#include "chartwidgets.h"
#include "controller/projectnotepresenter.h"
#include "controller/requirementspresenter.h"
#include "model/projectnotestore.h"
#include "model/srdcatalogs.h"
#include "model/srdstore.h"
#include "projectnotepanel.h"
#include "service/projectnoteservice.h"
#include "service/srdcompletenessservice.h"
#include "service/srdderivationservice.h"
#include "service/srddocumentservice.h"
#include "service/srdimportexportservice.h"
#include "uihelpers.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>

static void replaceHost(QWidget *host, QWidget *child)
{
    QVBoxLayout *lay = qobject_cast<QVBoxLayout *>(host->layout());
    if (!lay) {
        lay = new QVBoxLayout(host);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
    }
    while (lay->count() > 0) {
        QLayoutItem *item = lay->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    lay->addWidget(child);
}

static QWidget *makeKpiHost()
{
    auto *host = new QWidget;
    auto *lay = new QVBoxLayout(host);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    return host;
}

static int selectedRow(const QTableWidget *table)
{
    if (!table || table->selectionModel() == nullptr)
        return -1;
    const QModelIndexList rows = table->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return table->currentRow();
    return rows.first().row();
}

static QString selectedId(const QTableWidget *table)
{
    return tableRowId(table, selectedRow(table)).toString();
}

static double parseNumberText(QString text, bool *ok)
{
    text.remove(QLatin1Char(','));
    text = text.trimmed();
    return text.toDouble(ok);
}

static const SrdMissionScenario *findScenario(const SrdDocument &doc, const QString &id)
{
    for (int i = 0; i < doc.missions.size(); ++i) {
        if (doc.missions[i].id == id)
            return &doc.missions[i];
    }
    return nullptr;
}

static const SrdRequirement *findReq(const SrdDocument &doc, const QString &id)
{
    for (int i = 0; i < doc.requirements.size(); ++i) {
        if (doc.requirements[i].id == id)
            return &doc.requirements[i];
    }
    return nullptr;
}

RequirementsPage::RequirementsPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();

    m_noteStore.reset(new ProjectNoteStore);
    m_noteService.reset(new ProjectNoteService(m_noteStore.get()));
    m_notePresenter = new ProjectNotePresenter(m_notePanel, m_noteService.get(), this);

    m_srdStore.reset(new SrdStore);
    m_srdDocument.reset(new SrdDocumentService(m_srdStore.get()));
    m_srdCompleteness.reset(new SrdCompletenessService);
    m_srdDerivation.reset(new SrdDerivationService);
    m_srdIo.reset(new SrdImportExportService(m_srdStore.get()));
    m_srdPresenter = new RequirementsPresenter(this, m_srdDocument.get(), m_srdCompleteness.get(),
                                               m_srdDerivation.get(), m_srdIo.get(), this);
}

RequirementsPage::~RequirementsPage() = default;

void RequirementsPage::requestImport()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QString::fromUtf8("导入需求"), QString(),
        QString::fromUtf8("需求文件 (*.json *.csv);;JSON (*.json);;CSV (*.csv)"));
    if (path.isEmpty())
        return;
    emit importPathRequested(path);
}

void RequirementsPage::requestPublish()
{
    emit publishRequested();
}

void RequirementsPage::buildUi()
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);

    auto *head = new QWidget;
    auto *hl = new QHBoxLayout(head);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(18);
    auto *left = new QWidget;
    auto *vl = new QVBoxLayout(left);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(4);
    auto *title = new QLabel(QString::fromUtf8("设计需求与评价定义"));
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *sub = new QLabel(QString::fromUtf8("从任务场景、适航边界到可追溯的指标与约束"));
    sub->setObjectName(QStringLiteral("PageSubtitle"));
    sub->setWordWrap(true);
    vl->addWidget(title);
    vl->addWidget(sub);
    m_pageStatus = new QLabel;
    m_pageStatus->setObjectName(QStringLiteral("MutedLabel"));
    m_pageStatus->setWordWrap(true);
    vl->addWidget(m_pageStatus);
    hl->addWidget(left, 1);

    auto *right = new QWidget;
    auto *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setSpacing(6);
    m_baselineBox = new QComboBox;
    m_baselineBox->setMinimumWidth(220);
    m_copyDraftBtn = makeButton(QString::fromUtf8("另存为新草稿"));
    rl->addWidget(makeLabeled(QString::fromUtf8("需求基线"), m_baselineBox));
    rl->addWidget(m_copyDraftBtn);
    hl->addWidget(right, 0, Qt::AlignTop);
    lay->addWidget(head);

    connect(m_baselineBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &RequirementsPage::onBaselineChanged);
    connect(m_copyDraftBtn, &QPushButton::clicked, this, &RequirementsPage::copyBaselineRequested);

    m_notePanel = new ProjectNotePanel;
    lay->addWidget(m_notePanel);

    auto *tabs = new SubTabBar({
        {QStringLiteral("mission"), QString::fromUtf8("任务与使用场景")},
        {QStringLiteral("envelope"), QString::fromUtf8("工况与飞行包线")},
        {QStringLiteral("standards"), QString::fromUtf8("规范与适用性")},
        {QStringLiteral("metrics"), QString::fromUtf8("评价指标与需求约束")}
    }, QStringLiteral("mission"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(buildMissionPage());
    stack->addWidget(buildEnvelopePage());
    stack->addWidget(buildStandardsPage());
    stack->addWidget(buildMetricsPage());
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("envelope"))
            stack->setCurrentIndex(1);
        else if (id == QLatin1String("standards"))
            stack->setCurrentIndex(2);
        else if (id == QLatin1String("metrics"))
            stack->setCurrentIndex(3);
        else
            stack->setCurrentIndex(0);
    });

    outer->addWidget(wrapScroll(body));
}

QWidget *RequirementsPage::buildMissionPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_missionKpiHost = makeKpiHost();
    lay->addWidget(m_missionKpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *railPanel = makePanel();
    railPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("主设计任务剖面"), QString::fromUtf8("主场景航段")));
    m_rail = new MissionRail(QStringList());
    railPanel->layout()->addWidget(m_rail);
    vl->addWidget(railPanel);

    auto *scenPanel = makePanel();
    scenPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("任务与使用场景")));
    m_scenarioTable = makeEditableTable({
        QString::fromUtf8("场景"), QString::fromUtf8("任务边界"),
        QString::fromUtf8("环境/运营条件"), QString::fromUtf8("状态")
    });
    scenPanel->layout()->addWidget(m_scenarioTable);
    auto *scenBtns = new QWidget;
    auto *sbl = new QHBoxLayout(scenBtns);
    sbl->setContentsMargins(0, 0, 0, 0);
    auto *addS = makeButton(QString::fromUtf8("添加场景"));
    auto *delS = makeButton(QString::fromUtf8("删除场景"));
    sbl->addWidget(addS);
    sbl->addWidget(delS);
    sbl->addStretch();
    scenPanel->layout()->addWidget(scenBtns);
    vl->addWidget(scenPanel);
    connect(addS, &QPushButton::clicked, this, &RequirementsPage::addScenarioRequested);
    connect(delS, &QPushButton::clicked, this, &RequirementsPage::removeScenarioRequested);
    connect(m_scenarioTable, &QTableWidget::itemSelectionChanged, this, &RequirementsPage::onScenarioSelected);

    auto *needPanel = makePanel();
    needPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("任务与性能需求")));
    m_needTable = makeEditableTable({
        QString::fromUtf8("需求"), QString::fromUtf8("关系"), QString::fromUtf8("数值"),
        QString::fromUtf8("单位"), QString::fromUtf8("等级")
    });
    needPanel->layout()->addWidget(m_needTable);
    auto *needBtns = new QWidget;
    auto *nbl = new QHBoxLayout(needBtns);
    nbl->setContentsMargins(0, 0, 0, 0);
    m_needMetricBox = new QComboBox;
    const QVector<SrdCatalogMetric> metrics = SrdCatalogs::metrics();
    for (int i = 0; i < metrics.size(); ++i)
        m_needMetricBox->addItem(metrics[i].name, metrics[i].id);
    auto *addN = makeButton(QString::fromUtf8("添加需求"));
    auto *delN = makeButton(QString::fromUtf8("删除需求"));
    nbl->addWidget(m_needMetricBox, 1);
    nbl->addWidget(addN);
    nbl->addWidget(delN);
    needPanel->layout()->addWidget(needBtns);
    vl->addWidget(needPanel);
    connect(addN, &QPushButton::clicked, this, [this]() { emit addNeedRequested(currentNeedMetricId()); });
    connect(delN, &QPushButton::clicked, this, &RequirementsPage::removeNeedRequested);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);

    auto *def = makePanel();
    def->layout()->addWidget(makePanelTitle(QString::fromUtf8("场景定义")));
    m_missionType = new QComboBox;
    m_missionType->addItems({QString::fromUtf8("客运运输"), QString::fromUtf8("货运"), QString::fromUtf8("支线运输")});
    m_crew = makeInput(QString());
    m_utilization = makeInput(QString());
    m_designLife = makeInput(QString());
    m_elevation = makeInput(QString());
    m_runway = makeInput(QString());
    def->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("任务类型"), m_missionType),
        makeLabeled(QString::fromUtf8("典型乘员"), m_crew),
        makeLabeled(QString::fromUtf8("年利用率"), m_utilization),
        makeLabeled(QString::fromUtf8("设计寿命"), m_designLife),
        makeLabeled(QString::fromUtf8("机场标高 m"), m_elevation),
        makeLabeled(QString::fromUtf8("跑道长度 m"), m_runway)
    }));
    al->addWidget(def);

    auto *src = makePanel();
    src->layout()->addWidget(makePanelTitle(QString::fromUtf8("需求来源")));
    m_sourceHost = makeKpiHost();
    src->layout()->addWidget(m_sourceHost);
    al->addWidget(src);

    m_saveMissionBtn = makeButton(QString::fromUtf8("保存任务需求"), true);
    m_genFcBtn = makeButton(QString::fromUtf8("从任务生成建议工况"));
    connect(m_saveMissionBtn, &QPushButton::clicked, this, &RequirementsPage::saveMissionRequested);
    connect(m_genFcBtn, &QPushButton::clicked, this, &RequirementsPage::generateConditionsRequested);
    al->addWidget(m_saveMissionBtn);
    al->addWidget(m_genFcBtn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QWidget *RequirementsPage::buildEnvelopePage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_envelopeKpiHost = makeKpiHost();
    lay->addWidget(m_envelopeKpiHost);

    auto *two = new QWidget;
    auto *hl = new QHBoxLayout(two);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *chartP = makePanel();
    chartP->layout()->addWidget(makePanelTitle(QString::fromUtf8("高度—速度飞行包线"), QString::fromUtf8("由包线顶点绘制")));
    m_envelopeChart = new EnvelopeChart;
    chartP->layout()->addWidget(m_envelopeChart);
    auto *env = makePanel();
    env->layout()->addWidget(makePanelTitle(QString::fromUtf8("环境与边界条件")));
    m_atmModel = new QComboBox;
    m_atmModel->addItems({QStringLiteral("US Standard 1976"), QString::fromUtf8("自定义大气")});
    m_tempOffset = makeInput(QString());
    m_crosswind = makeInput(QString());
    m_slope = makeInput(QString());
    m_gust = new QComboBox;
    m_gust->addItems({QStringLiteral("1-cos"), QStringLiteral("Von Kármán")});
    m_icing = makeInput(QString());
    m_nzMin = makeInput(QString());
    m_nzMax = makeInput(QString());
    env->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("大气模型"), m_atmModel),
        makeLabeled(QString::fromUtf8("温度偏差"), m_tempOffset),
        makeLabeled(QString::fromUtf8("侧风上限"), m_crosswind),
        makeLabeled(QString::fromUtf8("跑道坡度"), m_slope),
        makeLabeled(QString::fromUtf8("阵风模型"), m_gust),
        makeLabeled(QString::fromUtf8("结冰条件"), m_icing),
        makeLabeled(QString::fromUtf8("载荷因数下限 g"), m_nzMin),
        makeLabeled(QString::fromUtf8("载荷因数上限 g"), m_nzMax)
    }));
    auto *cov = new QWidget;
    auto *cvl = new QVBoxLayout(cov);
    cvl->setContentsMargins(0, 6, 0, 0);
    cvl->setSpacing(4);
    auto *crow = new QWidget;
    auto *chl = new QHBoxLayout(crow);
    chl->setContentsMargins(0, 0, 0, 0);
    auto *cl = new QLabel(QString::fromUtf8("工况覆盖完整度"));
    cl->setObjectName(QStringLiteral("MutedLabel"));
    m_coverageLabel = new QLabel(QStringLiteral("0%"));
    chl->addWidget(cl);
    chl->addStretch();
    chl->addWidget(m_coverageLabel);
    m_coverageBar = new QProgressBar;
    m_coverageBar->setRange(0, 100);
    m_coverageBar->setValue(0);
    m_coverageBar->setTextVisible(false);
    cvl->addWidget(crow);
    cvl->addWidget(m_coverageBar);
    env->layout()->addWidget(cov);
    hl->addWidget(chartP, 1);
    hl->addWidget(env, 1);
    lay->addWidget(two);

    auto *pts = makePanel();
    pts->layout()->addWidget(makePanelTitle(QString::fromUtf8("包线顶点"), QString::fromUtf8("马赫 / 高度 km")));
    m_pointTable = makeEditableTable({QString::fromUtf8("马赫 Ma"), QString::fromUtf8("高度 km")});
    pts->layout()->addWidget(m_pointTable);
    lay->addWidget(pts);

    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计工况集")));
    m_fcTable = makeEditableTable({
        QString::fromUtf8("编号"), QString::fromUtf8("阶段"), QString::fromUtf8("高度"),
        QString::fromUtf8("速度"), QString::fromUtf8("构型"), QString::fromUtf8("大气")
    });
    tbl->layout()->addWidget(m_fcTable);
    auto *fcBtns = new QWidget;
    auto *fl = new QHBoxLayout(fcBtns);
    fl->setContentsMargins(0, 0, 0, 0);
    auto *addC = makeButton(QString::fromUtf8("添加工况"));
    auto *delC = makeButton(QString::fromUtf8("删除工况"));
    m_saveEnvBtn = makeButton(QString::fromUtf8("保存工况与包线"), true);
    fl->addWidget(addC);
    fl->addWidget(delC);
    fl->addStretch();
    fl->addWidget(m_saveEnvBtn);
    tbl->layout()->addWidget(fcBtns);
    lay->addWidget(tbl);
    connect(addC, &QPushButton::clicked, this, &RequirementsPage::addConditionRequested);
    connect(delC, &QPushButton::clicked, this, &RequirementsPage::removeConditionRequested);
    connect(m_saveEnvBtn, &QPushButton::clicked, this, &RequirementsPage::saveEnvelopeRequested);
    return root;
}

QWidget *RequirementsPage::buildStandardsPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_stdKpiHost = makeKpiHost();
    lay->addWidget(m_stdKpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *base = makePanel();
    base->layout()->addWidget(makePanelTitle(QString::fromUtf8("认证基础与适用范围")));
    m_awClass = new QComboBox;
    m_awClass->addItems({QString::fromUtf8("CS-25 大型飞机"), QStringLiteral("FAR Part 25")});
    m_opsRules = new QComboBox;
    m_opsRules->addItems({QString::fromUtf8("CAT 商业运输"), QString::fromUtf8("非商业运行")});
    m_amendment = makeInput(QString());
    m_noise = makeInput(QString());
    m_emission = makeInput(QString());
    m_special = makeInput(QString());
    base->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("适航类别"), m_awClass),
        makeLabeled(QString::fromUtf8("运行规则"), m_opsRules),
        makeLabeled(QString::fromUtf8("审定修订版"), m_amendment),
        makeLabeled(QString::fromUtf8("噪声标准"), m_noise),
        makeLabeled(QString::fromUtf8("排放标准"), m_emission),
        makeLabeled(QString::fromUtf8("特殊条件"), m_special)
    }, 3));
    vl->addWidget(base);

    auto *mat = makePanel();
    mat->layout()->addWidget(makePanelTitle(QString::fromUtf8("条款适用性矩阵")));
    m_clauseTable = makeEditableTable({
        QString::fromUtf8("条款"), QString::fromUtf8("主题"), QString::fromUtf8("责任域"),
        QString::fromUtf8("适用性"), QString::fromUtf8("需求映射")
    });
    mat->layout()->addWidget(m_clauseTable);
    vl->addWidget(mat);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *judge = makePanel();
    judge->layout()->addWidget(makePanelTitle(QString::fromUtf8("适用性判定")));
    m_applyHost = makeKpiHost();
    judge->layout()->addWidget(m_applyHost);
    al->addWidget(judge);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("完整性检查")));
    m_stdCheckHost = makeKpiHost();
    chk->layout()->addWidget(m_stdCheckHost);
    al->addWidget(chk);
    m_applyAppBtn = makeButton(QString::fromUtf8("按认证基础填默认适用性"));
    m_saveStdBtn = makeButton(QString::fromUtf8("保存规范章"));
    m_freezeBtn = makeButton(QString::fromUtf8("发布适用性基线"), true);
    m_unfreezeBtn = makeButton(QString::fromUtf8("解冻规范章"));
    connect(m_applyAppBtn, &QPushButton::clicked, this, &RequirementsPage::applyApplicabilityRequested);
    connect(m_saveStdBtn, &QPushButton::clicked, this, &RequirementsPage::saveStandardsRequested);
    connect(m_freezeBtn, &QPushButton::clicked, this, &RequirementsPage::freezeStandardsRequested);
    connect(m_unfreezeBtn, &QPushButton::clicked, this, &RequirementsPage::unfreezeStandardsRequested);
    al->addWidget(m_applyAppBtn);
    al->addWidget(m_saveStdBtn);
    al->addWidget(m_freezeBtn);
    al->addWidget(m_unfreezeBtn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QWidget *RequirementsPage::buildMetricsPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_metricKpiHost = makeKpiHost();
    lay->addWidget(m_metricKpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("评价指标与需求约束")));
    m_reqTable = makeEditableTable({
        QString::fromUtf8("需求编号"), QString::fromUtf8("指标"), QString::fromUtf8("责任域"),
        QString::fromUtf8("关系"), QString::fromUtf8("数值"), QString::fromUtf8("单位"),
        QString::fromUtf8("等级"), QString::fromUtf8("来源"), QString::fromUtf8("状态")
    });
    tbl->layout()->addWidget(m_reqTable);
    auto *reqBtns = new QWidget;
    auto *rbl = new QHBoxLayout(reqBtns);
    rbl->setContentsMargins(0, 0, 0, 0);
    m_reqMetricBox = new QComboBox;
    const QVector<SrdCatalogMetric> metrics = SrdCatalogs::metrics();
    for (int i = 0; i < metrics.size(); ++i)
        m_reqMetricBox->addItem(metrics[i].name, metrics[i].id);
    auto *addR = makeButton(QString::fromUtf8("添加指标需求"));
    auto *delR = makeButton(QString::fromUtf8("删除需求"));
    rbl->addWidget(m_reqMetricBox, 1);
    rbl->addWidget(addR);
    rbl->addWidget(delR);
    tbl->layout()->addWidget(reqBtns);
    connect(addR, &QPushButton::clicked, this, [this]() { emit addRequirementRequested(currentReqMetricId()); });
    connect(delR, &QPushButton::clicked, this, &RequirementsPage::removeRequirementRequested);
    connect(m_reqTable, &QTableWidget::itemSelectionChanged, this, &RequirementsPage::onRequirementSelected);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *attr = makePanel();
    attr->layout()->addWidget(makePanelTitle(QString::fromUtf8("需求属性")));
    m_constraintKind = new QComboBox;
    m_constraintKind->addItems({QString::fromUtf8("强制性约束"), QString::fromUtf8("指标性约束"), QString::fromUtf8("期望项")});
    m_priority = makeInput(QString());
    m_verify = new QComboBox;
    m_verify->addItems({QString::fromUtf8("分析"), QString::fromUtf8("试验"), QString::fromUtf8("检查"), QString()});
    m_reqStatus = makeInput(QString());
    attr->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("约束类型"), m_constraintKind),
        makeLabeled(QString::fromUtf8("优先级"), m_priority),
        makeLabeled(QString::fromUtf8("验证方法"), m_verify),
        makeLabeled(QString::fromUtf8("成熟度"), m_reqStatus)
    }));
    al->addWidget(attr);
    auto *trace = makePanel();
    trace->layout()->addWidget(makePanelTitle(QString::fromUtf8("追溯关系")));
    m_traceHost = makeKpiHost();
    trace->layout()->addWidget(m_traceHost);
    al->addWidget(trace);
    m_genReqBtn = makeButton(QString::fromUtf8("从任务与条款建议需求"));
    m_saveReqBtn = makeButton(QString::fromUtf8("保存需求章"));
    m_exportBtn = makeButton(QString::fromUtf8("导出评价规格"));
    m_publishBtn = makeButton(QString::fromUtf8("创建需求基线"), true);
    connect(m_genReqBtn, &QPushButton::clicked, this, &RequirementsPage::generateRequirementsRequested);
    connect(m_saveReqBtn, &QPushButton::clicked, this, &RequirementsPage::saveMetricsRequested);
    connect(m_exportBtn, &QPushButton::clicked, this, &RequirementsPage::exportSpecRequested);
    connect(m_publishBtn, &QPushButton::clicked, this, &RequirementsPage::publishRequested);
    al->addWidget(m_genReqBtn);
    al->addWidget(m_saveReqBtn);
    al->addWidget(m_exportBtn);
    al->addWidget(m_publishBtn);
    al->addStretch();

    hl->addWidget(tbl, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QString RequirementsPage::currentNeedMetricId() const
{
    return m_needMetricBox ? m_needMetricBox->currentData().toString() : QString();
}

QString RequirementsPage::currentReqMetricId() const
{
    return m_reqMetricBox ? m_reqMetricBox->currentData().toString() : QString();
}

QString RequirementsPage::selectedScenarioId() const
{
    return selectedId(m_scenarioTable);
}

QString RequirementsPage::selectedNeedId() const
{
    return selectedId(m_needTable);
}

QString RequirementsPage::selectedConditionId() const
{
    return selectedId(m_fcTable);
}

QString RequirementsPage::selectedRequirementId() const
{
    return selectedId(m_reqTable);
}

void RequirementsPage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    const QList<QWidget *> editors = findChildren<QWidget *>();
    for (int i = 0; i < editors.size(); ++i) {
        QWidget *w = editors[i];
        if (w->objectName() == QLatin1String("SubTabButton")
            || w->objectName() == QLatin1String("ModeButton")
            || w == m_baselineBox)
            continue;
        const bool isEditor = qobject_cast<QLineEdit *>(w) || qobject_cast<QComboBox *>(w)
            || qobject_cast<QTableWidget *>(w) || qobject_cast<QPushButton *>(w);
        if (!isEditor)
            continue;
        if (w == m_copyDraftBtn)
            continue;
        w->setEnabled(!readOnly);
    }
    if (m_copyDraftBtn)
        m_copyDraftBtn->setEnabled(readOnly);
    if (m_baselineBox)
        m_baselineBox->setEnabled(true);
}

void RequirementsPage::setBusy(bool busy)
{
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (int i = 0; i < buttons.size(); ++i) {
        if (buttons[i]->objectName() == QLatin1String("SubTabButton"))
            continue;
        buttons[i]->setEnabled(!busy && (!m_readOnly || buttons[i] == m_copyDraftBtn));
    }
    if (m_baselineBox)
        m_baselineBox->setEnabled(!busy);
}

void RequirementsPage::setStatus(const QString &text, bool isError)
{
    m_pageStatus->setText(text);
    m_pageStatus->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void RequirementsPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("设计需求"), message);
}

bool RequirementsPage::confirmList(const QString &title, const QStringList &lines)
{
    const QString text = QString::fromUtf8("将写入下列建议，是否继续？\n\n") + lines.join(QLatin1Char('\n'));
    return QMessageBox::question(this, title, text) == QMessageBox::Yes;
}

void RequirementsPage::onBaselineChanged(int)
{
    if (m_updating)
        return;
    emit switchBaselineRequested(m_baselineBox->currentData().toString());
}

void RequirementsPage::fillScenarioSide(const SrdMissionScenario &s)
{
    m_updating = true;
    const int t = m_missionType->findText(s.missionType);
    m_missionType->setCurrentIndex(t >= 0 ? t : 0);
    m_crew->setText(s.crew);
    m_utilization->setText(s.utilization);
    m_designLife->setText(s.designLife);
    m_elevation->setText(s.airport.elevationKnown ? srdFormatNumber(s.airport.elevationM) : QString());
    m_runway->setText(s.airport.runwayLengthKnown ? srdFormatNumber(s.airport.runwayLengthM) : QString());
    m_updating = false;
}

void RequirementsPage::fillRequirementSide(const SrdRequirement &r)
{
    m_updating = true;
    if (r.constraintKind == QLatin1String("mandatory") || r.grade == QString::fromUtf8("强制"))
        m_constraintKind->setCurrentIndex(0);
    else if (r.constraintKind == QLatin1String("wish") || r.grade == QString::fromUtf8("期望"))
        m_constraintKind->setCurrentIndex(2);
    else
        m_constraintKind->setCurrentIndex(1);
    m_priority->setText(r.priority);
    const int v = m_verify->findText(r.verificationMethod);
    m_verify->setCurrentIndex(v >= 0 ? v : 0);
    m_reqStatus->setText(r.status);
    replaceHost(m_traceHost, makeSummary({
        {QString::fromUtf8("上游来源"), r.upstream.isEmpty() ? r.source : r.upstream},
        {QString::fromUtf8("下游变量"), r.downstreamVars.isEmpty() ? QStringLiteral("—") : r.downstreamVars},
        {QString::fromUtf8("分析响应"), r.analysisResponseId.isEmpty() ? QStringLiteral("—") : r.analysisResponseId},
        {QString::fromUtf8("验证证据"), r.evidenceRun.isEmpty() ? QStringLiteral("—") : r.evidenceRun}
    }));
    m_updating = false;
}

void RequirementsPage::onScenarioSelected()
{
    if (m_updating)
        return;
    const SrdMissionScenario *s = findScenario(m_shown, selectedScenarioId());
    if (s)
        fillScenarioSide(*s);
}

void RequirementsPage::onRequirementSelected()
{
    if (m_updating)
        return;
    const SrdRequirement *r = findReq(m_shown, selectedRequirementId());
    if (r)
        fillRequirementSide(*r);
}

void RequirementsPage::applyScenarioSide(SrdMissionScenario *s) const
{
    s->missionType = m_missionType->currentText();
    s->crew = m_crew->text();
    s->utilization = m_utilization->text();
    s->designLife = m_designLife->text();
    bool ok = false;
    const double elev = parseNumberText(m_elevation->text(), &ok);
    s->airport.elevationKnown = ok;
    if (ok)
        s->airport.elevationM = elev;
    const double rw = parseNumberText(m_runway->text(), &ok);
    s->airport.runwayLengthKnown = ok;
    if (ok)
        s->airport.runwayLengthM = rw;
}

void RequirementsPage::applyRequirementSide(SrdRequirement *r) const
{
    if (m_constraintKind->currentIndex() == 0) {
        r->constraintKind = QStringLiteral("mandatory");
        if (r->grade.isEmpty())
            r->grade = QString::fromUtf8("强制");
    } else if (m_constraintKind->currentIndex() == 2) {
        r->constraintKind = QStringLiteral("wish");
        if (r->grade != QString::fromUtf8("期望"))
            r->grade = QString::fromUtf8("期望");
    } else {
        r->constraintKind = QStringLiteral("objective");
    }
    r->priority = m_priority->text();
    r->verificationMethod = m_verify->currentText();
    if (!m_reqStatus->text().isEmpty())
        r->status = m_reqStatus->text();
}

void RequirementsPage::setBaselines(const QVector<SrdBaselineInfo> &baselines, const QString &currentId)
{
    m_updating = true;
    m_baselineBox->clear();
    m_baselineBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    int current = 0;
    for (int i = 0; i < baselines.size(); ++i) {
        QString label = baselines[i].title;
        if (label.isEmpty())
            label = baselines[i].id;
        if (!baselines[i].publishedAt.isEmpty())
            label += QStringLiteral("  ") + baselines[i].publishedAt;
        m_baselineBox->addItem(label, baselines[i].id);
        if (baselines[i].id == currentId)
            current = i + 1;
    }
    if (currentId == QLatin1String("draft"))
        current = 0;
    m_baselineBox->setCurrentIndex(current);
    m_updating = false;
}

void RequirementsPage::setCompleteness(const SrdCompletenessReport &report)
{
    const SrdKpis &k = report.kpis;
    replaceHost(m_missionKpiHost, makeKpis({
        {QString::fromUtf8("任务场景"), QString::number(k.scenarioCount), QString::fromUtf8("个")},
        {QString::fromUtf8("需求条目"), QString::number(k.needCount), QString::fromUtf8("项")},
        {QString::fromUtf8("强制需求"), QString::number(k.mandatoryNeedCount), QString::fromUtf8("项")},
        {QString::fromUtf8("待澄清"), QString::number(k.pendingCount), QString::fromUtf8("项")}
    }));
    replaceHost(m_sourceHost, makeSummary({
        {QString::fromUtf8("市场需求"), QString::fromUtf8("%1 项").arg(k.sourceMarket)},
        {QString::fromUtf8("运营方输入"), QString::fromUtf8("%1 项").arg(k.sourceOperator)},
        {QString::fromUtf8("法规派生"), QString::fromUtf8("%1 项").arg(k.sourceReg)},
        {QString::fromUtf8("设计目标"), QString::fromUtf8("%1 项").arg(k.sourceGoal)}
    }));
    replaceHost(m_envelopeKpiHost, makeKpis({
        {QString::fromUtf8("设计工况"), QString::number(k.conditionCount), QString::fromUtf8("个")},
        {QString::fromUtf8("包线维度"), QString::number(k.envelopeDimCount), QString::fromUtf8("维")},
        {QString::fromUtf8("大气模型"), k.atmosphereModel, QString()},
        {QString::fromUtf8("载荷因数"), k.loadFactorRange, QStringLiteral("g")}
    }));
    m_coverageLabel->setText(QString::number(k.conditionCoveragePercent) + QLatin1Char('%'));
    m_coverageBar->setValue(k.conditionCoveragePercent);
    replaceHost(m_stdKpiHost, makeKpis({
        {QString::fromUtf8("适用规范"), QString::number(k.standardSetCount), QString::fromUtf8("套")},
        {QString::fromUtf8("适用条款"), QString::number(k.clauseCount), QString::fromUtf8("项")},
        {QString::fromUtf8("已完成映射"), QString::number(k.mappedClauseCount), QString::fromUtf8("项")},
        {QString::fromUtf8("待确认"), QString::number(k.pendingClauseCount), QString::fromUtf8("项")}
    }));
    replaceHost(m_applyHost, makeSummary({
        {QString::fromUtf8("直接适用"), QString::number(k.applyDirect)},
        {QString::fromUtf8("条件适用"), QString::number(k.applyConditional)},
        {QString::fromUtf8("不适用"), QString::number(k.applyNa)},
        {QString::fromUtf8("需等效安全"), QString::number(k.applyEquivalent)}
    }));
    QStringList bullets;
    if (report.items.isEmpty())
        bullets.append(QString::fromUtf8("当前检查项全部通过"));
    for (int i = 0; i < report.items.size(); ++i)
        bullets.append(report.items[i].message);
    replaceHost(m_stdCheckHost, makeBulletList(bullets));
    replaceHost(m_metricKpiHost, makeKpis({
        {QString::fromUtf8("评价指标"), QString::number(k.metricCount), QString::fromUtf8("项")},
        {QString::fromUtf8("强制约束"), QString::number(k.mandatoryConstraintCount), QString::fromUtf8("项")},
        {QString::fromUtf8("目标指标"), QString::number(k.objectiveCount), QString::fromUtf8("项")},
        {QString::fromUtf8("需求覆盖率"), QString::number(k.coveragePercent), QStringLiteral("%")}
    }));
}

void RequirementsPage::setDocument(const SrdDocument &doc, bool readOnly)
{
    m_shown = doc;
    m_updating = true;

    QStringList segments;
    const SrdMissionScenario *primary = findScenario(doc, QStringLiteral("M-001"));
    if (!primary && !doc.missions.isEmpty())
        primary = &doc.missions.first();
    if (primary) {
        for (int i = 0; i < primary->segments.size(); ++i)
            segments.append(primary->segments[i].name);
        fillScenarioSide(*primary);
    }
    m_rail->setSegments(segments);

    QVector<QStringList> scenRows;
    QVector<QVariant> scenIds;
    for (int i = 0; i < doc.missions.size(); ++i) {
        scenRows.append({doc.missions[i].name, doc.missions[i].boundary,
                         doc.missions[i].environment, doc.missions[i].status});
        scenIds.append(doc.missions[i].id);
    }
    setTableContents(m_scenarioTable, scenRows, scenIds);

    QVector<QStringList> needRows;
    QVector<QVariant> needIds;
    for (int i = 0; i < doc.needs.size(); ++i) {
        needRows.append({doc.needs[i].name, doc.needs[i].relation,
                         doc.needs[i].valueKnown ? srdFormatNumber(doc.needs[i].value) : QString(),
                         doc.needs[i].unit, doc.needs[i].grade});
        needIds.append(doc.needs[i].id);
    }
    setTableContents(m_needTable, needRows, needIds);

    const int atm = m_atmModel->findText(doc.environment.atmosphereModel);
    if (atm >= 0)
        m_atmModel->setCurrentIndex(atm);
    else if (!doc.environment.atmosphereModel.isEmpty()) {
        m_atmModel->insertItem(0, doc.environment.atmosphereModel);
        m_atmModel->setCurrentIndex(0);
    }
    m_tempOffset->setText(doc.environment.temperatureOffset);
    m_crosswind->setText(doc.environment.crosswindLimit);
    m_slope->setText(doc.environment.runwaySlope);
    const int gust = m_gust->findText(doc.environment.gustModel);
    if (gust >= 0)
        m_gust->setCurrentIndex(gust);
    m_icing->setText(doc.environment.icingCondition);
    m_nzMin->setText(doc.environment.nzKnown ? srdFormatNumber(doc.environment.nzMin) : QString());
    m_nzMax->setText(doc.environment.nzKnown ? srdFormatNumber(doc.environment.nzMax) : QString());

    QVector<QPointF> chartPts;
    QVector<QStringList> ptRows;
    QVector<QVariant> ptIds;
    for (int i = 0; i < doc.envelopePoints.size(); ++i) {
        chartPts.append(QPointF(doc.envelopePoints[i].mach, doc.envelopePoints[i].altitudeKm));
        ptRows.append({srdFormatNumber(doc.envelopePoints[i].mach),
                       srdFormatNumber(doc.envelopePoints[i].altitudeKm)});
        ptIds.append(i);
    }
    m_envelopeChart->setPoints(chartPts);
    setTableContents(m_pointTable, ptRows, ptIds);

    QVector<QStringList> fcRows;
    QVector<QVariant> fcIds;
    for (int i = 0; i < doc.conditions.size(); ++i) {
        fcRows.append({doc.conditions[i].id, doc.conditions[i].phase, doc.conditions[i].altitude,
                       doc.conditions[i].speed, doc.conditions[i].configuration, doc.conditions[i].atmosphere});
        fcIds.append(doc.conditions[i].id);
    }
    setTableContents(m_fcTable, fcRows, fcIds);

    const int aw = m_awClass->findText(doc.certification.airworthinessClass);
    if (aw >= 0)
        m_awClass->setCurrentIndex(aw);
    const int ops = m_opsRules->findText(doc.certification.opsRules);
    if (ops >= 0)
        m_opsRules->setCurrentIndex(ops);
    m_amendment->setText(doc.certification.amendment);
    m_noise->setText(doc.certification.noiseStandard);
    m_emission->setText(doc.certification.emissionStandard);
    m_special->setText(doc.certification.specialCondition);

    QVector<QStringList> clauseRows;
    QVector<QVariant> clauseIds;
    for (int i = 0; i < doc.clauses.size(); ++i) {
        clauseRows.append({doc.clauses[i].clauseId, doc.clauses[i].topic, doc.clauses[i].domain,
                           doc.clauses[i].applicability, doc.clauses[i].mappingStatus});
        clauseIds.append(doc.clauses[i].clauseId);
    }
    setTableContents(m_clauseTable, clauseRows, clauseIds);

    QVector<QStringList> reqRows;
    QVector<QVariant> reqIds;
    for (int i = 0; i < doc.requirements.size(); ++i) {
        reqRows.append({doc.requirements[i].id, doc.requirements[i].metricName, doc.requirements[i].domain,
                        doc.requirements[i].relation,
                        doc.requirements[i].boundKnown ? srdFormatNumber(doc.requirements[i].boundValue) : QString(),
                        doc.requirements[i].unit, doc.requirements[i].grade, doc.requirements[i].source,
                        doc.requirements[i].status});
        reqIds.append(doc.requirements[i].id);
    }
    setTableContents(m_reqTable, reqRows, reqIds);
    if (!doc.requirements.isEmpty())
        fillRequirementSide(doc.requirements.first());

    m_updating = false;
    setReadOnly(readOnly);
    if (m_scenarioTable->rowCount() > 0)
        m_scenarioTable->selectRow(0);
    if (m_reqTable->rowCount() > 0)
        m_reqTable->selectRow(0);
}

void RequirementsPage::snapshotMissionChapter(QVector<SrdMissionScenario> *missions,
                                              QVector<SrdPerformanceNeed> *needs) const
{
    *missions = m_shown.missions;
    const QVector<QStringList> rows = tableAllRows(m_scenarioTable);
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_scenarioTable, r).toString();
        SrdMissionScenario *target = nullptr;
        for (int i = 0; i < missions->size(); ++i) {
            if ((*missions)[i].id == id)
                target = &(*missions)[i];
        }
        if (!target)
            continue;
        if (rows[r].size() > 0)
            target->name = rows[r][0];
        if (rows[r].size() > 1)
            target->boundary = rows[r][1];
        if (rows[r].size() > 2)
            target->environment = rows[r][2];
        if (rows[r].size() > 3)
            target->status = rows[r][3];
        if (id == selectedScenarioId())
            applyScenarioSide(target);
    }

    *needs = m_shown.needs;
    const QVector<QStringList> needRows = tableAllRows(m_needTable);
    for (int r = 0; r < needRows.size(); ++r) {
        const QString id = tableRowId(m_needTable, r).toString();
        SrdPerformanceNeed *target = nullptr;
        for (int i = 0; i < needs->size(); ++i) {
            if ((*needs)[i].id == id)
                target = &(*needs)[i];
        }
        if (!target)
            continue;
        if (needRows[r].size() > 0)
            target->name = needRows[r][0];
        if (needRows[r].size() > 1)
            target->relation = needRows[r][1];
        if (needRows[r].size() > 2) {
            bool ok = false;
            target->value = parseNumberText(needRows[r][2], &ok);
            target->valueKnown = ok;
        }
        if (needRows[r].size() > 3)
            target->unit = needRows[r][3];
        if (needRows[r].size() > 4)
            target->grade = needRows[r][4];
    }
}

void RequirementsPage::snapshotEnvelopeChapter(SrdEnvironment *environment,
                                               QVector<SrdEnvelopePoint> *points,
                                               QVector<SrdFlightCondition> *conditions) const
{
    *environment = m_shown.environment;
    environment->atmosphereModel = m_atmModel->currentText();
    environment->temperatureOffset = m_tempOffset->text();
    environment->crosswindLimit = m_crosswind->text();
    environment->runwaySlope = m_slope->text();
    environment->gustModel = m_gust->currentText();
    environment->icingCondition = m_icing->text();
    bool ok = false;
    environment->nzMin = parseNumberText(m_nzMin->text(), &ok);
    bool ok2 = false;
    environment->nzMax = parseNumberText(m_nzMax->text(), &ok2);
    environment->nzKnown = ok && ok2;

    points->clear();
    const QVector<QStringList> ptRows = tableAllRows(m_pointTable);
    for (int r = 0; r < ptRows.size(); ++r) {
        SrdEnvelopePoint p;
        bool okM = false, okH = false;
        p.mach = parseNumberText(ptRows[r].value(0), &okM);
        p.altitudeKm = parseNumberText(ptRows[r].value(1), &okH);
        if (okM && okH)
            points->append(p);
    }

    *conditions = m_shown.conditions;
    const QVector<QStringList> rows = tableAllRows(m_fcTable);
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_fcTable, r).toString();
        SrdFlightCondition *target = nullptr;
        for (int i = 0; i < conditions->size(); ++i) {
            if ((*conditions)[i].id == id)
                target = &(*conditions)[i];
        }
        SrdFlightCondition local;
        if (!target) {
            local.id = rows[r].value(0);
            conditions->append(local);
            target = &conditions->last();
        }
        target->id = rows[r].value(0);
        target->phase = rows[r].value(1);
        target->altitude = rows[r].value(2);
        target->speed = rows[r].value(3);
        target->configuration = rows[r].value(4);
        target->atmosphere = rows[r].value(5);
    }
}

void RequirementsPage::snapshotStandardsChapter(SrdCertification *certification,
                                                QVector<SrdClauseRow> *clauses) const
{
    *certification = m_shown.certification;
    certification->airworthinessClass = m_awClass->currentText();
    certification->opsRules = m_opsRules->currentText();
    certification->amendment = m_amendment->text();
    certification->noiseStandard = m_noise->text();
    certification->emissionStandard = m_emission->text();
    certification->specialCondition = m_special->text();

    *clauses = m_shown.clauses;
    const QVector<QStringList> rows = tableAllRows(m_clauseTable);
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_clauseTable, r).toString();
        SrdClauseRow *target = nullptr;
        for (int i = 0; i < clauses->size(); ++i) {
            if ((*clauses)[i].clauseId == id)
                target = &(*clauses)[i];
        }
        if (!target)
            continue;
        target->clauseId = rows[r].value(0);
        target->topic = rows[r].value(1);
        target->domain = rows[r].value(2);
        target->applicability = rows[r].value(3);
        target->mappingStatus = rows[r].value(4);
    }
}

void RequirementsPage::snapshotMetricsChapter(QVector<SrdRequirement> *requirements) const
{
    *requirements = m_shown.requirements;
    const QVector<QStringList> rows = tableAllRows(m_reqTable);
    const QString selected = selectedRequirementId();
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_reqTable, r).toString();
        SrdRequirement *target = nullptr;
        for (int i = 0; i < requirements->size(); ++i) {
            if ((*requirements)[i].id == id)
                target = &(*requirements)[i];
        }
        if (!target)
            continue;
        target->id = rows[r].value(0);
        target->metricName = rows[r].value(1);
        target->domain = rows[r].value(2);
        target->relation = rows[r].value(3);
        bool ok = false;
        target->boundValue = parseNumberText(rows[r].value(4), &ok);
        target->boundKnown = ok;
        target->unit = rows[r].value(5);
        target->grade = rows[r].value(6);
        target->source = rows[r].value(7);
        target->status = rows[r].value(8);
        target->constraintKind = srdGradeToKind(target->grade);
        if (id == selected)
            applyRequirementSide(target);
    }
}
