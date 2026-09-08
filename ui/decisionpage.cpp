#include "decisionpage.h"
#include "chartwidgets.h"
#include "uihelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *singlePage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("当前方案"), QStringLiteral("C-027"), QString()},
        {QString::fromUtf8("可行性"), QString::fromUtf8("通过"), QString()},
        {QString::fromUtf8("综合评分"), QStringLiteral("86.4"), QStringLiteral("/100")},
        {QString::fromUtf8("约束裕度"), QStringLiteral("1"), QString::fromUtf8("项临界")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);
    auto *radar = makePanel();
    radar->layout()->addWidget(makePanelTitle(QString::fromUtf8("指标综合评估"), QString::fromUtf8("方案 C-027")));
    radar->layout()->addWidget(new RadarChart);
    vl->addWidget(radar);
    auto *cons = makePanel();
    cons->layout()->addWidget(makePanelTitle(QString::fromUtf8("约束违反与裕度"), QString::fromUtf8("5 项关键约束")));
    cons->layout()->addWidget(makeTable(
        {QString::fromUtf8("指标"), QString::fromUtf8("方案值"), QString::fromUtf8("约束"), QString::fromUtf8("裕度"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("最大起飞重量"), QStringLiteral("68,420 kg"), QString::fromUtf8("≤ 72,000"), QStringLiteral("+4.97%"), QString::fromUtf8("满足")},
            {QString::fromUtf8("任务燃油"), QStringLiteral("14,860 kg"), QString::fromUtf8("≤ 15,500"), QStringLiteral("+4.13%"), QString::fromUtf8("满足")},
            {QString::fromUtf8("起飞场长"), QStringLiteral("2,312 m"), QString::fromUtf8("≤ 2,500"), QStringLiteral("+7.52%"), QString::fromUtf8("满足")},
            {QString::fromUtf8("屈曲裕度"), QStringLiteral("0.18"), QString::fromUtf8("≥ 0.15"), QStringLiteral("+0.03"), QString::fromUtf8("临界")},
            {QString::fromUtf8("静稳定裕度"), QStringLiteral("7.4 %MAC"), QString::fromUtf8("≥ 5.0"), QStringLiteral("+2.4"), QString::fromUtf8("满足")}
        }));
    vl->addWidget(cons);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *set = makePanel();
    set->layout()->addWidget(makePanelTitle(QString::fromUtf8("评估设置")));
    set->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("基线方案"), QStringLiteral("Baseline v12")),
        makeSelectField(QString::fromUtf8("评分规则"), QString::fromUtf8("归一化加权"), {QString::fromUtf8("理想点距离"), QString::fromUtf8("效用函数")}),
        makeField(QString::fromUtf8("约束容差"), QStringLiteral("0.5 %")),
        makeField(QString::fromUtf8("数据版本"), QStringLiteral("Run 2026-09-04"))
    }));
    al->addWidget(set);
    auto *conv = makePanel();
    conv->layout()->addWidget(makePanelTitle(QString::fromUtf8("优化收敛")));
    conv->layout()->addWidget(makeProgressRow(QString::fromUtf8("超体积改善"), QStringLiteral("+0.3%"), 78));
    conv->layout()->addWidget(makeProgressRow(QString::fromUtf8("可行方案占比"), QStringLiteral("64%"), 64));
    al->addWidget(conv);
    auto *btn = makeButton(QString::fromUtf8("加入方案短名单"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *comparePage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("候选方案"), QStringLiteral("32"), QString::fromUtf8("个")},
        {QString::fromUtf8("可行方案"), QStringLiteral("21"), QString::fromUtf8("个")},
        {QString::fromUtf8("Pareto 方案"), QStringLiteral("8"), QString::fromUtf8("个")},
        {QString::fromUtf8("已选对比"), QStringLiteral("4"), QString::fromUtf8("个")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);
    auto *par = makePanel();
    par->layout()->addWidget(makePanelTitle(QString::fromUtf8("多指标权衡"), QString::fromUtf8("已选 4 个方案 · 选中 C-027")));
    par->layout()->addWidget(new ParallelChart);
    vl->addWidget(par);

    TableOptions opt;
    opt.firstColumnCheck = true;
    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("候选方案比较"), QString::fromUtf8("按综合评分排序")));
    tbl->layout()->addWidget(makeTable(
        {QString::fromUtf8("选择"), QString::fromUtf8("方案"), QString::fromUtf8("综合评分"), QString::fromUtf8("任务燃油 kg"),
         QStringLiteral("MTOW kg"), QStringLiteral("L/D"), QString::fromUtf8("违反约束")},
        {
            {QString(), QStringLiteral("C-027"), QStringLiteral("86.4"), QStringLiteral("14,860"), QStringLiteral("68,420"), QStringLiteral("18.7"), QStringLiteral("0")},
            {QString(), QStringLiteral("C-014"), QStringLiteral("84.9"), QStringLiteral("14,520"), QStringLiteral("69,180"), QStringLiteral("19.1"), QStringLiteral("1")},
            {QString(), QStringLiteral("C-031"), QStringLiteral("83.7"), QStringLiteral("15,040"), QStringLiteral("67,930"), QStringLiteral("18.3"), QStringLiteral("0")},
            {QString(), QStringLiteral("Baseline"), QStringLiteral("76.2"), QStringLiteral("15,880"), QStringLiteral("70,610"), QStringLiteral("17.6"), QStringLiteral("0")}
        }, opt));
    vl->addWidget(tbl);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *w = makePanel();
    w->layout()->addWidget(makePanelTitle(QString::fromUtf8("权重与基线")));
    w->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("归一化"), QString::fromUtf8("相对基线"), {QStringLiteral("Min-Max"), QStringLiteral("Z-score")}),
        makeSelectField(QString::fromUtf8("权衡方法"), QStringLiteral("TOPSIS"), {QString::fromUtf8("加权和"), QStringLiteral("AHP")}),
        makeField(QString::fromUtf8("基线方案"), QStringLiteral("Baseline v12")),
        makeField(QString::fromUtf8("可行性优先"), QString::fromUtf8("是"))
    }));
    al->addWidget(w);
    auto *shortlist = makePanel();
    shortlist->layout()->addWidget(makePanelTitle(QString::fromUtf8("推荐短名单")));
    shortlist->layout()->addWidget(new CandidateRow(QStringLiteral("1"), QStringLiteral("C-027"),
                                                    QString::fromUtf8("均衡型 · 无约束违反"), QStringLiteral("86.4")));
    shortlist->layout()->addWidget(new CandidateRow(QStringLiteral("2"), QStringLiteral("C-031"),
                                                    QString::fromUtf8("轻量型 · 性能略低"), QStringLiteral("83.7")));
    shortlist->layout()->addWidget(new CandidateRow(QStringLiteral("3"), QStringLiteral("C-014"),
                                                    QString::fromUtf8("效率型 · 1项临界"), QStringLiteral("84.9")));
    al->addWidget(shortlist);
    auto *btn = makeButton(QString::fromUtf8("确定推荐方案"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

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
    structP->layout()->addWidget(makePanelTitle(QString::fromUtf8("报告结构与内容"), QString::fromUtf8("设计评审报告 v2")));
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

DecisionPage::DecisionPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("方案决策与结果评估"),
        QString::fromUtf8("当前视图：跨学科总览 · 从可行性、指标表现与权衡关系形成决策证据"),
        QString::fromUtf8("评估批次"),
        {QString::fromUtf8("优化任务 Run-240904"), QString::fromUtf8("基准方案对比")}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("single"), QString::fromUtf8("单方案评价")},
        {QStringLiteral("compare"), QString::fromUtf8("方案比较与权衡")},
        {QStringLiteral("report"), QString::fromUtf8("报告生成与数据输出")}
    }, QStringLiteral("single"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(singlePage(this));
    stack->addWidget(comparePage(this));
    stack->addWidget(reportPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("compare")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("report")) stack->setCurrentIndex(2);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));
}
