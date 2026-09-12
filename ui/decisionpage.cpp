#include "decisionpage.h"

#include "chartwidgets.h"
#include "controller/decisionpresenter.h"
#include "model/aircraftstore.h"
#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "service/analysiscomputeservice.h"
#include "service/evaluationservice.h"
#include "uihelpers.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <algorithm>

// 用一个容器承载可替换内容（KPI / 表格），刷新时清空再填。
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

static QString numText(double v)
{
    return QString::number(v, 'g', 6);
}

QWidget *DecisionPage::buildComparePage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    m_compareKpiHost = makeHostWidget();
    lay->addWidget(m_compareKpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);
    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("候选方案比较"),
                                            QString::fromUtf8("汇总已评价的分析集版本 · 按满足率排序")));
    m_compareTableHost = makeHostWidget();
    tbl->layout()->addWidget(m_compareTableHost);
    vl->addWidget(tbl);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *sl = makePanel();
    sl->layout()->addWidget(makePanelTitle(QString::fromUtf8("推荐短名单"),
                                           QString::fromUtf8("可行优先；无可行时列最接近候选")));
    m_shortlistHost = makeHostWidget();
    sl->layout()->addWidget(m_shortlistHost);
    al->addWidget(sl);
    auto *refresh = makeButton(QString::fromUtf8("刷新比较"), true);
    connect(refresh, &QPushButton::clicked, this, &DecisionPage::refreshComparisonRequested);
    al->addWidget(refresh);
    auto *note = new QLabel(QString::fromUtf8(
        "汇总来自各方案的单方案评价结果（analysis/evaluations/*.json）。"
        "满足率含 MOCK 方案值的判定仅示意；分析真实化后自动生效。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    al->addWidget(note);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);

    // 初始占位。
    setHostContent(m_compareKpiHost, makeKpis({
        {QString::fromUtf8("已评价方案"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("可行方案"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("最高评分"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("待分析项合计"), QString::fromUtf8("—"), QString()}
    }));
    setHostContent(m_compareTableHost, makeTable(
        {QString::fromUtf8("方案"), QString::fromUtf8("飞机修订"), QString::fromUtf8("设计需求"),
         QString::fromUtf8("满足率"), QString::fromUtf8("可行性"),
         QString::fromUtf8("满足/违反/临界/待分析")}, {}, TableOptions{}));
    return root;
}

void DecisionPage::showComparison(const QVector<SchemeEvaluationResult> &results)
{
    // 按满足率（评分）降序，未知评分排后。
    QVector<SchemeEvaluationResult> sorted = results;
    std::sort(sorted.begin(), sorted.end(),
              [](const SchemeEvaluationResult &a, const SchemeEvaluationResult &b) {
                  if (a.scoreKnown != b.scoreKnown)
                      return a.scoreKnown;
                  return a.score > b.score;
              });

    int feasibleCount = 0;
    int pendingSum = 0;
    double topScore = 0.0;
    bool topKnown = false;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i].feasibility == QString::fromUtf8("通过"))
            ++feasibleCount;
        pendingSum += sorted[i].pending;
        if (sorted[i].scoreKnown && (!topKnown || sorted[i].score > topScore)) {
            topScore = sorted[i].score;
            topKnown = true;
        }
    }

    if (m_compareKpiHost) {
        setHostContent(m_compareKpiHost, makeKpis({
            {QString::fromUtf8("已评价方案"), QString::number(sorted.size()), QString::fromUtf8("个")},
            {QString::fromUtf8("可行方案"), QString::number(feasibleCount), QString::fromUtf8("个")},
            {QString::fromUtf8("最高评分"),
             topKnown ? QString::number(topScore, 'f', 1) : QString::fromUtf8("—"),
             topKnown ? QStringLiteral("%") : QString()},
            {QString::fromUtf8("待分析项合计"), QString::number(pendingSum), QString::fromUtf8("项")}
        }));
    }

    if (m_compareTableHost) {
        QVector<QStringList> rows;
        TableOptions opt;
        for (int i = 0; i < sorted.size(); ++i) {
            const SchemeEvaluationResult &r = sorted[i];
            const QString score = r.scoreKnown ? (QString::number(r.score, 'f', 1) + QStringLiteral("%"))
                                               : QString::fromUtf8("—");
            const QString counts = QString::fromUtf8("%1 / %2 / %3 / %4")
                                       .arg(r.satisfied).arg(r.violated).arg(r.critical).arg(r.pending);
            rows.append({
                r.objectId,
                r.sourceRevision.isEmpty() ? QString::fromUtf8("—") : r.sourceRevision,
                r.sourceSrd.isEmpty() ? QString::fromUtf8("—") : r.sourceSrd,
                score,
                r.feasibility,
                counts
            });
            if (r.feasibility == QString::fromUtf8("违反"))
                opt.warnRows.append(i);
        }
        setHostContent(m_compareTableHost, makeTable(
            {QString::fromUtf8("方案"), QString::fromUtf8("飞机修订"), QString::fromUtf8("设计需求"),
             QString::fromUtf8("满足率"), QString::fromUtf8("可行性"),
             QString::fromUtf8("满足/违反/临界/待分析")}, rows, opt));
    }

    if (m_shortlistHost) {
        auto *host = new QWidget;
        auto *hl = new QVBoxLayout(host);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);

        bool anyFeasible = false;
        for (int i = 0; i < sorted.size(); ++i) {
            if (sorted[i].feasibility == QString::fromUtf8("通过")) {
                anyFeasible = true;
                break;
            }
        }
        // 无严格可行方案时，仍按评分列出最接近的候选（标注违反/待分析）。
        if (!anyFeasible && !sorted.isEmpty()) {
            auto *hint = new QLabel(QString::fromUtf8("暂无严格可行方案，按评分列出最接近的候选："));
            hint->setObjectName(QStringLiteral("NoteLabel"));
            hint->setWordWrap(true);
            hl->addWidget(hint);
        }

        int rank = 0;
        for (int i = 0; i < sorted.size() && rank < 3; ++i) {
            const SchemeEvaluationResult &r = sorted[i];
            if (anyFeasible && r.feasibility != QString::fromUtf8("通过"))
                continue; // 有可行方案时，短名单只收可行方案
            ++rank;
            const QString rev = r.sourceRevision.isEmpty() ? QString::fromUtf8("未关联修订")
                                                           : r.sourceRevision;
            QString desc;
            if (r.feasibility == QString::fromUtf8("通过"))
                desc = QString::fromUtf8("%1 · 待分析 %2 项").arg(rev).arg(r.pending);
            else
                desc = QString::fromUtf8("%1 · %2 · 违反 %3 / 待分析 %4")
                           .arg(rev).arg(r.feasibility).arg(r.violated).arg(r.pending);
            const QString score = r.scoreKnown ? QString::number(r.score, 'f', 1)
                                               : QString::fromUtf8("—");
            hl->addWidget(new CandidateRow(QString::number(rank), r.objectId, desc, score));
        }
        if (sorted.isEmpty())
            hl->addWidget(new QLabel(QString::fromUtf8("暂无评价结果，请先在「单方案评价」运行评价。")));
        setHostContent(m_shortlistHost, host);
    }
}

// ---- 报告生成与数据输出（暂为原型静态展示）--------------------------------
static QWidget *reportPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("报告方案"), QStringLiteral("4"), QString::fromUtf8("个")},
        {QString::fromUtf8("图表"), QStringLiteral("12"), QString::fromUtf8("幅")},
        {QString::fromUtf8("数据表"), QStringLiteral("9"), QString::fromUtf8("张")},
        {QString::fromUtf8("附件"), QStringLiteral("3"), QString::fromUtf8("项")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *structP = makePanel();
    structP->layout()->addWidget(makePanelTitle(QString::fromUtf8("报告结构与内容"), QString::fromUtf8("原型示意")));
    auto *two = new QWidget;
    auto *th = new QHBoxLayout(two);
    th->setContentsMargins(0, 0, 0, 0);
    auto *c1 = new QWidget;
    auto *l1 = new QVBoxLayout(c1);
    l1->setContentsMargins(0, 0, 0, 0);
    l1->addWidget(makeCheck(QString::fromUtf8("执行摘要与推荐结论"), true));
    l1->addWidget(makeCheck(QString::fromUtf8("设计空间与变量范围"), true));
    l1->addWidget(makeCheck(QString::fromUtf8("单方案约束与指标评估"), true));
    l1->addWidget(makeCheck(QString::fromUtf8("多方案比较与权衡"), true));
    auto *c2 = new QWidget;
    auto *l2 = new QVBoxLayout(c2);
    l2->setContentsMargins(0, 0, 0, 0);
    l2->addWidget(makeCheck(QString::fromUtf8("优化收敛与迭代历史"), true));
    l2->addWidget(makeCheck(QString::fromUtf8("六学科关键结果"), true));
    l2->addWidget(makeCheck(QString::fromUtf8("完整求解日志"), false));
    l2->addWidget(makeCheck(QString::fromUtf8("数据版本与追溯信息"), true));
    th->addWidget(c1);
    th->addWidget(c2);
    structP->layout()->addWidget(two);
    vl->addWidget(structP);

    TableOptions opt;
    opt.firstColumnCheck = false;
    auto *exp = makePanel();
    exp->layout()->addWidget(makePanelTitle(QString::fromUtf8("结果数据导出")));
    auto *table = makeTable(
        {QString::fromUtf8("数据集"), QString::fromUtf8("范围"), QString::fromUtf8("格式"), QString::fromUtf8("预计大小"), QString::fromUtf8("包含")},
        {
            {QString::fromUtf8("候选方案总表"), QString::fromUtf8("32 个方案"), QStringLiteral("CSV / XLSX"), QStringLiteral("1.8 MB"), QString()},
            {QString::fromUtf8("设计变量与响应"), QString::fromUtf8("全部迭代"), QStringLiteral("Parquet"), QStringLiteral("26 MB"), QString()},
            {QString::fromUtf8("学科结果摘要"), QString::fromUtf8("短名单方案"), QStringLiteral("JSON"), QStringLiteral("4.2 MB"), QString()},
            {QString::fromUtf8("运行日志"), QString::fromUtf8("失败与告警"), QStringLiteral("TXT"), QStringLiteral("0.6 MB"), QString()}
        }, opt);
    for (int r = 0; r < 4; ++r) {
        auto *cb = makeCheck(QString(), r < 3);
        table->setCellWidget(r, 4, cb);
    }
    exp->layout()->addWidget(table);
    vl->addWidget(exp);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *gen = makePanel();
    gen->layout()->addWidget(makePanelTitle(QString::fromUtf8("生成设置")));
    gen->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("报告模板"), QString::fromUtf8("方案评审｜详细版"), {QString::fromUtf8("管理摘要版")}),
        makeSelectField(QString::fromUtf8("输出格式"), QStringLiteral("PDF + PPTX"), {QString::fromUtf8("仅 PDF"), QStringLiteral("DOCX")}),
        makeField(QString::fromUtf8("语言"), QString::fromUtf8("简体中文")),
        makeField(QString::fromUtf8("数据精度"), QString::fromUtf8("工程显示"))
    }));
    al->addWidget(gen);
    auto *ver = makePanel();
    ver->layout()->addWidget(makePanelTitle(QString::fromUtf8("版本与追溯")));
    ver->layout()->addWidget(makeSummary({
        {QString::fromUtf8("项目基线"), QStringLiteral("v12")},
        {QString::fromUtf8("分析集"), QStringLiteral("Run-240904")},
        {QString::fromUtf8("决策版本"), QStringLiteral("Review-03")},
        {QString::fromUtf8("生成后锁定"), QString::fromUtf8("开启")}
    }));
    al->addWidget(ver);
    auto *btn = makeButton(QString::fromUtf8("生成报告与数据包"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QWidget *DecisionPage::buildSinglePage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    m_kpiHost = makeHostWidget();
    lay->addWidget(m_kpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);
    auto *cons = makePanel();
    cons->layout()->addWidget(makePanelTitle(QString::fromUtf8("约束违反与裕度"),
                                             QString::fromUtf8("方案值来自学科分析 · 约束来自设计需求")));
    m_tableHost = makeHostWidget();
    cons->layout()->addWidget(m_tableHost);
    vl->addWidget(cons);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *set = makePanel();
    set->layout()->addWidget(makePanelTitle(QString::fromUtf8("评估设置")));
    m_objectBox = new QComboBox;
    set->layout()->addWidget(makeLabeled(QString::fromUtf8("评价对象"), m_objectBox));
    connect(m_objectBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int) { if (!m_updating) requestEvaluate(); });
    m_schemeInfo = new QLabel(QString::fromUtf8("—"));
    m_schemeInfo->setWordWrap(true);
    set->layout()->addWidget(m_schemeInfo);
    m_tolerance = makeInput(QStringLiteral("0.5"));
    set->layout()->addWidget(makeLabeled(QString::fromUtf8("约束容差（%）"), m_tolerance));
    auto *runBtn = makeButton(QString::fromUtf8("运行评价"), true);
    connect(runBtn, &QPushButton::clicked, this, &DecisionPage::requestEvaluate);
    set->layout()->addWidget(runBtn);
    al->addWidget(set);
    auto *note = new QLabel(QString::fromUtf8(
        "说明：方案值取自学科分析结果、约束取自关联的设计需求。保真度为 MOCK 的方案值为占位，"
        "对应判定仅示意；分析真实化后判定自动生效。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    al->addWidget(note);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);

    m_status = new QLabel(QString::fromUtf8("就绪"));
    m_status->setObjectName(QStringLiteral("PageStatus"));
    m_status->setContentsMargins(0, 4, 0, 0);
    lay->addWidget(m_status);

    // 初始占位（等 Presenter 初次评价后填真实数据）。
    setHostContent(m_kpiHost, makeKpis({
        {QString::fromUtf8("评价方案"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("可行性"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("满足/总数"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("待分析"), QString::fromUtf8("—"), QString()}
    }));
    setHostContent(m_tableHost, makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("方案值"), QString::fromUtf8("约束"),
         QString::fromUtf8("裕度"), QString::fromUtf8("保真度"), QString::fromUtf8("状态")},
        {}, TableOptions{}));
    return root;
}

void DecisionPage::reloadObjectOptions()
{
    if (!m_objectBox || !m_analysisStore)
        return;
    m_updating = true;
    m_objectBox->clear();
    m_objectBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    QVector<AnalysisBaselineInfo> baselines;
    QString detail;
    if (m_analysisStore->listBaselines(&baselines, &detail)) {
        for (int i = 0; i < baselines.size(); ++i) {
            QString label = baselines[i].id;
            if (baselines[i].version > 0)
                label += QString::fromUtf8("  v%1").arg(baselines[i].version);
            m_objectBox->addItem(label, baselines[i].id);
        }
    }
    m_updating = false;
}

void DecisionPage::requestEvaluate()
{
    bool ok = false;
    double tol = m_tolerance ? m_tolerance->text().toDouble(&ok) : 0.5;
    if (!ok || tol < 0)
        tol = 0.5;
    const QString objectId = m_objectBox ? m_objectBox->currentData().toString()
                                         : QStringLiteral("draft");
    emit evaluateRequested(objectId, tol);
}

void DecisionPage::showEvaluation(const SchemeEvaluationResult &result)
{
    // 方案信息
    if (m_schemeInfo) {
        QString info = QString::fromUtf8("飞机修订：%1")
                           .arg(result.sourceRevision.isEmpty() ? QString::fromUtf8("未关联")
                                                                : result.sourceRevision);
        info += QString::fromUtf8("\n设计需求：%1")
                    .arg(result.sourceSrd.isEmpty() ? QString::fromUtf8("未关联") : result.sourceSrd);
        if (!result.sourceCondition.isEmpty())
            info += QString::fromUtf8("（工况#%1）").arg(result.sourceCondition.toInt() + 1);
        m_schemeInfo->setText(info);
    }

    // KPI
    const QString feas = result.srdResolved ? result.feasibility : QString::fromUtf8("—");
    setHostContent(m_kpiHost, makeKpis({
        {QString::fromUtf8("评价方案"),
         result.sourceRevision.isEmpty() ? QString::fromUtf8("草稿") : result.sourceRevision, QString()},
        {QString::fromUtf8("可行性"), feas, QString()},
        {QString::fromUtf8("满足/总数"),
         QString::fromUtf8("%1/%2").arg(result.satisfied).arg(result.total), QString()},
        {QString::fromUtf8("待分析"), QString::number(result.pending), QString::fromUtf8("项")}
    }));

    // 约束表
    QVector<QStringList> rows;
    TableOptions opt;
    for (int i = 0; i < result.items.size(); ++i) {
        const EvaluationItem &it = result.items[i];
        const QString actual = it.actualKnown
            ? (numText(it.actualValue) + (it.unit.isEmpty() ? QString() : QLatin1Char(' ') + it.unit))
            : QString::fromUtf8("—");
        QString constraint = it.relation;
        if (it.boundKnown)
            constraint += QLatin1Char(' ') + numText(it.boundValue);
        if (!it.unit.isEmpty())
            constraint += QLatin1Char(' ') + it.unit;
        const QString margin = it.marginKnown
            ? ((it.marginPercent >= 0 ? QStringLiteral("+") : QString())
               + QString::number(it.marginPercent, 'f', 2) + QStringLiteral("%"))
            : QString::fromUtf8("—");
        const QString fidelity = it.actualFidelity.isEmpty() ? QString::fromUtf8("—") : it.actualFidelity;
        rows.append({it.metricName, actual, constraint, margin, fidelity, it.status});
        if (it.status == QString::fromUtf8("违反") || it.status == QString::fromUtf8("临界"))
            opt.warnRows.append(i);
    }
    setHostContent(m_tableHost, makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("方案值"), QString::fromUtf8("约束"),
         QString::fromUtf8("裕度"), QString::fromUtf8("保真度"), QString::fromUtf8("状态")},
        rows, opt));
}

void DecisionPage::setStatus(const QString &text, bool isError)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void DecisionPage::showError(const QString &message)
{
    // 决策页以状态行提示为主，避免频繁弹窗打断查看。
    setStatus(message, true);
}

DecisionPage::DecisionPage(QWidget *parent)
    : QWidget(parent)
{
    m_aircraftStore.reset(new AircraftStore);
    m_srdStore.reset(new SrdStore);
    m_analysisStore.reset(new AnalysisStore);
    m_compute.reset(new AnalysisComputeService(m_aircraftStore.get(), m_analysisStore.get(),
                                               m_srdStore.get()));
    m_evaluation.reset(new EvaluationService(m_compute.get(), m_srdStore.get(), m_analysisStore.get()));

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("方案决策与结果评估"),
        QString::fromUtf8("当前视图：单方案评价接入学科分析结果与设计需求限值，形成可行性/裕度证据"),
        QString(), {}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("single"), QString::fromUtf8("单方案评价")},
        {QStringLiteral("compare"), QString::fromUtf8("方案比较与权衡")},
        {QStringLiteral("report"), QString::fromUtf8("报告生成与数据输出")}
    }, QStringLiteral("single"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(buildSinglePage());
    stack->addWidget(buildComparePage());
    stack->addWidget(reportPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("compare")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("report")) stack->setCurrentIndex(2);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));

    reloadObjectOptions();
    m_presenter = new DecisionPresenter(this, m_analysisStore.get(), m_evaluation.get(), this);
}

DecisionPage::~DecisionPage() = default;
