#include "designpage.h"
#include "chartwidgets.h"
#include "uihelpers.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *variablesPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("设计变量"), QStringLiteral("18"), QString::fromUtf8("个")},
        {QString::fromUtf8("约束"), QStringLiteral("12"), QString::fromUtf8("项")},
        {QString::fromUtf8("分析工况"), QStringLiteral("7"), QString::fromUtf8("组")},
        {QString::fromUtf8("预计组合规模"), QStringLiteral("3.2×10⁸"), QString()}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    TableOptions varOpt;
    varOpt.firstColumnCheck = true;
    varOpt.chipColumns = {2};
    auto *vars = makePanel();
    vars->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计变量与范围"), QString::fromUtf8("显示 6 / 18"),
                                             QString::fromUtf8("在同一张表中管理变量所属学科、边界与采样类型")));
    vars->layout()->addWidget(makeTable(
        {QString::fromUtf8("启用"), QString::fromUtf8("变量"), QString::fromUtf8("学科"), QString::fromUtf8("基准值"),
         QString::fromUtf8("下界"), QString::fromUtf8("上界"), QString::fromUtf8("单位"), QString::fromUtf8("类型")},
        {
            {QString(), QString::fromUtf8("翼面积 S"), QString::fromUtf8("气动"), QStringLiteral("124.0"), QStringLiteral("110"), QStringLiteral("145"), QString::fromUtf8("m²"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("展弦比 AR"), QString::fromUtf8("气动"), QStringLiteral("9.4"), QStringLiteral("8.0"), QStringLiteral("11.5"), QString::fromUtf8("—"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("后掠角 Λ25"), QString::fromUtf8("气动"), QStringLiteral("25.0"), QStringLiteral("18"), QStringLiteral("32"), QStringLiteral("deg"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("翼盒厚度系数"), QString::fromUtf8("结构"), QStringLiteral("1.00"), QStringLiteral("0.85"), QStringLiteral("1.20"), QString::fromUtf8("—"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("额定推力"), QString::fromUtf8("推进与能源"), QStringLiteral("118"), QStringLiteral("102"), QStringLiteral("135"), QStringLiteral("kN"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("巡航马赫数"), QString::fromUtf8("任务与性能"), QStringLiteral("0.78"), QStringLiteral("0.72"), QStringLiteral("0.82"), QStringLiteral("Ma"), QString::fromUtf8("离散")}
        }, varOpt));
    vl->addWidget(vars);

    auto *cons = makePanel();
    cons->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计约束"), QString::fromUtf8("4 项重点约束")));
    cons->layout()->addWidget(makeTable(
        {QString::fromUtf8("约束指标"), QString::fromUtf8("关系"), QString::fromUtf8("限制值"), QString::fromUtf8("基准方案"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("最大起飞重量"), QString::fromUtf8("≤"), QStringLiteral("72,000 kg"), QStringLiteral("68,420 kg"), QString::fromUtf8("满足")},
            {QString::fromUtf8("起飞场长"), QString::fromUtf8("≤"), QStringLiteral("2,500 m"), QStringLiteral("2,312 m"), QString::fromUtf8("满足")},
            {QString::fromUtf8("二阶屈曲裕度"), QString::fromUtf8("≥"), QStringLiteral("0.15"), QStringLiteral("0.18"), QString::fromUtf8("临界")},
            {QString::fromUtf8("静稳定裕度"), QString::fromUtf8("≥"), QStringLiteral("5 %MAC"), QStringLiteral("7.4 %MAC"), QString::fromUtf8("满足")}
        }));
    vl->addWidget(cons);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *type = makePanel();
    type->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计类型")));
    type->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("模式"), QString::fromUtf8("探索与优化"), {QString::fromUtf8("仅探索"), QString::fromUtf8("仅优化")}),
        makeSelectField(QString::fromUtf8("变量集"), QString::fromUtf8("MDO 基准变量集"), {QString::fromUtf8("气动专项"), QString::fromUtf8("推进专项")}),
        makeSelectField(QString::fromUtf8("约束集"), QString::fromUtf8("适航 + 任务约束"), {QString::fromUtf8("仅任务约束")}),
        makeSelectField(QString::fromUtf8("工况集"), QString::fromUtf8("全任务关键工况"), {QString::fromUtf8("巡航工况")})
    }));
    al->addWidget(type);
    auto *flt = makePanel();
    flt->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前筛选")));
    flt->layout()->addWidget(makeSummary({
        {QString::fromUtf8("学科"), QString::fromUtf8("跨学科总览")},
        {QString::fromUtf8("耦合变量"), QString::fromUtf8("6 个")},
        {QString::fromUtf8("共享响应"), QString::fromUtf8("9 个")}
    }));
    al->addWidget(flt);
    auto *btn = makeButton(QString::fromUtf8("校验设计空间"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *explorationPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("采样方法"), QString::fromUtf8("优化拉丁超立方"), QString()},
        {QString::fromUtf8("计划样本"), QStringLiteral("240"), QString::fromUtf8("点")},
        {QString::fromUtf8("可并行任务"), QStringLiteral("12"), QString::fromUtf8("个")},
        {QString::fromUtf8("预计完成"), QStringLiteral("3.6"), QStringLiteral("h")}
    }));

    auto *toolbar = new QWidget;
    auto *tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(9);
    tl->addWidget(makeSelectField(QStringLiteral("DOE 方法"), QString::fromUtf8("优化拉丁超立方"),
                                  {QStringLiteral("Sobol 序列"), QString::fromUtf8("正交试验"), QString::fromUtf8("全因子")}));
    tl->addWidget(makeField(QString::fromUtf8("样本数量"), QStringLiteral("240")));
    tl->addWidget(makeField(QString::fromUtf8("随机种子"), QStringLiteral("20260904")));
    tl->addWidget(makeField(QString::fromUtf8("并行数"), QStringLiteral("12")));
    tl->addStretch();
    auto *preview = makeButton(QString::fromUtf8("预览采样"));
    auto *gen = makeButton(QString::fromUtf8("生成候选方案"), true);
    wireDummyAction(preview, parent);
    wireDummyAction(gen, parent);
    tl->addWidget(preview);
    tl->addWidget(gen);
    lay->addWidget(toolbar);

    auto *two = new QWidget;
    auto *hl = new QHBoxLayout(two);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *cover = makePanel();
    cover->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计空间覆盖"), QString::fromUtf8("18 个变量 · 投影：翼载荷 × 推重比")));
    cover->layout()->addWidget(new ScatterChart);
    auto *diag = makePanel();
    diag->layout()->addWidget(makePanelTitle(QString::fromUtf8("采样质量诊断"), QString::fromUtf8("自动更新")));
    diag->layout()->addWidget(makeSummary({
        {QString::fromUtf8("最小点间距"), QStringLiteral("0.074")},
        {QString::fromUtf8("最大相关系数"), QStringLiteral("0.061")},
        {QString::fromUtf8("空间填充度"), QStringLiteral("94%"), false, true},
        {QString::fromUtf8("重复设计点"), QStringLiteral("0")}
    }));
    diag->layout()->addWidget(makeProgressRow(QString::fromUtf8("候选生成准备度"), QStringLiteral("94%"), 94));
    hl->addWidget(cover, 1);
    hl->addWidget(diag, 1);
    lay->addWidget(two);

    auto *three = new QWidget;
    auto *th = new QHBoxLayout(three);
    th->setContentsMargins(0, 0, 0, 0);
    th->setSpacing(12);
    auto addList = [&](const QString &title, const QStringList &items) {
        auto *p = makePanel();
        p->layout()->addWidget(makePanelTitle(title));
        p->layout()->addWidget(makeBulletList(items));
        th->addWidget(p);
    };
    addList(QString::fromUtf8("采样范围"), {QString::fromUtf8("连续变量：14"), QString::fromUtf8("离散变量：3"), QString::fromUtf8("构型变量：1")});
    addList(QString::fromUtf8("分析策略"), {QString::fromUtf8("低保真全量计算"), QString::fromUtf8("关键点高保真校核"), QString::fromUtf8("失败点自动重试 1 次")});
    addList(QString::fromUtf8("候选方案集"), {QString::fromUtf8("保留全部可行点"), QString::fromUtf8("聚类代表点：24"), QString::fromUtf8("Pareto 候选：预计 12—20")});
    lay->addWidget(three);
    return root;
}

static QWidget *optimizationPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("优化目标"), QStringLiteral("3"), QString::fromUtf8("项")},
        {QString::fromUtf8("算法"), QStringLiteral("NSGA-II"), QString()},
        {QString::fromUtf8("种群规模"), QStringLiteral("80"), QString()},
        {QString::fromUtf8("最大评估"), QStringLiteral("4,800"), QString::fromUtf8("次")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    TableOptions opt;
    opt.firstColumnCheck = true;
    auto *obj = makePanel();
    obj->layout()->addWidget(makePanelTitle(QString::fromUtf8("优化目标设置"), QString::fromUtf8("多目标")));
    obj->layout()->addWidget(makeTable(
        {QString::fromUtf8("启用"), QString::fromUtf8("目标响应"), QString::fromUtf8("方向"), QString::fromUtf8("单位"), QString::fromUtf8("优先权重")},
        {
            {QString(), QString::fromUtf8("任务燃油"), QString::fromUtf8("最小化"), QStringLiteral("kg"), QStringLiteral("1.00")},
            {QString(), QString::fromUtf8("最大起飞重量"), QString::fromUtf8("最小化"), QStringLiteral("kg"), QStringLiteral("0.75")},
            {QString(), QString::fromUtf8("巡航升阻比"), QString::fromUtf8("最大化"), QString::fromUtf8("—"), QStringLiteral("0.55")}
        }, opt));
    vl->addWidget(obj);

    auto *sol = makePanel();
    sol->layout()->addWidget(makePanelTitle(QString::fromUtf8("求解设置")));
    sol->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("优化算法"), QStringLiteral("NSGA-II"), {QStringLiteral("MOEA/D"), QString::fromUtf8("差分进化"), QString::fromUtf8("贝叶斯优化")}),
        makeField(QString::fromUtf8("初始样本"), QStringLiteral("DOE-240")),
        makeField(QString::fromUtf8("种群规模"), QStringLiteral("80")),
        makeField(QString::fromUtf8("迭代代数"), QStringLiteral("60")),
        makeField(QString::fromUtf8("交叉概率"), QStringLiteral("0.90")),
        makeField(QString::fromUtf8("变异概率"), QStringLiteral("自动 1/n")),
        makeField(QString::fromUtf8("并行评估数"), QStringLiteral("12")),
        makeSelectField(QString::fromUtf8("失败策略"), QString::fromUtf8("罚函数 + 重试"), {QString::fromUtf8("跳过并记录")})
    }));
    vl->addWidget(sol);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *stop = makePanel();
    stop->layout()->addWidget(makePanelTitle(QString::fromUtf8("终止条件")));
    stop->layout()->addWidget(makeCheck(QString::fromUtf8("达到最大评估次数"), true));
    stop->layout()->addWidget(makeCheck(QString::fromUtf8("超体积连续 8 代无改善"), true));
    stop->layout()->addWidget(makeCheck(QString::fromUtf8("达到目标阈值"), false));
    al->addWidget(stop);
    auto *run = makePanel();
    run->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行策略")));
    run->layout()->addWidget(makeSummary({
        {QString::fromUtf8("缓存复用"), QString::fromUtf8("开启")},
        {QString::fromUtf8("断点续算"), QString::fromUtf8("每代保存")},
        {QString::fromUtf8("高保真校核"), QString::fromUtf8("Pareto 前 10")}
    }));
    al->addWidget(run);
    auto *btn = makeButton(QString::fromUtf8("创建优化任务"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *mdoPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QStringLiteral("MDO 架构"), QStringLiteral("MDF"), QString()},
        {QString::fromUtf8("耦合学科"), QStringLiteral("6"), QString::fromUtf8("个")},
        {QString::fromUtf8("耦合变量"), QStringLiteral("9"), QString::fromUtf8("个")},
        {QString::fromUtf8("总导数方式"), QString::fromUtf8("混合"), QString()}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *prob = makePanel();
    prob->layout()->addWidget(makePanelTitle(QString::fromUtf8("优化问题自动构建"),
                                             QString::fromUtf8("由设计空间、分析响应和约束映射生成")));
    prob->layout()->addWidget(makeMiniFields({
        makeSelectField(QStringLiteral("MDO 架构"), QStringLiteral("MDF"), {QStringLiteral("IDF"), QString::fromUtf8("协同优化 CO")}),
        makeSelectField(QString::fromUtf8("优化驱动器"), QStringLiteral("SLSQP"), {QStringLiteral("IPOPT"), QStringLiteral("NSGA-II")}),
        makeField(QString::fromUtf8("设计变量集"), QString::fromUtf8("MDO 基准变量集")),
        makeField(QString::fromUtf8("响应映射"), QString::fromUtf8("自动发现 21 项")),
        makeField(QString::fromUtf8("缩放方式"), QString::fromUtf8("基准值归一化")),
        makeField(QString::fromUtf8("单位一致性"), QString::fromUtf8("严格检查"))
    }, 3));
    vl->addWidget(prob);

    auto *coup = makePanel();
    coup->layout()->addWidget(makePanelTitle(QString::fromUtf8("学科耦合关系"), QString::fromUtf8("行：输出学科 · 列：输入学科")));
    const QStringList names = {
        QString::fromUtf8("气动"), QString::fromUtf8("结构"), QString::fromUtf8("重量"),
        QString::fromUtf8("推进"), QString::fromUtf8("操稳"), QString::fromUtf8("任务")
    };
    const int matrix[6][6] = {
        {0,1,1,1,1,1}, {1,0,1,0,1,0}, {0,1,0,1,1,1},
        {1,0,1,0,0,1}, {1,0,1,1,0,0}, {1,1,1,1,0,0}
    };
    QStringList headers = {QString()};
    headers += names;
    QVector<QStringList> rows;
    for (int i = 0; i < 6; ++i) {
        QStringList row;
        row << names[i];
        for (int j = 0; j < 6; ++j)
            row << (matrix[i][j] ? QString::fromUtf8("耦合") : QString::fromUtf8("—"));
        rows << row;
    }
    coup->layout()->addWidget(makeTable(headers, rows));
    vl->addWidget(coup);

    auto *grad = makePanel();
    grad->layout()->addWidget(makePanelTitle(QString::fromUtf8("导数与梯度计算")));
    grad->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("总导数方法"), QString::fromUtf8("解析/伴随优先"), {QString::fromUtf8("全有限差分")}),
        makeField(QString::fromUtf8("缺失导数"), QString::fromUtf8("中心差分")),
        makeField(QString::fromUtf8("相对步长"), QStringLiteral("1e-4")),
        makeField(QString::fromUtf8("并行导数组"), QStringLiteral("12")),
        makeField(QString::fromUtf8("导数检查"), QString::fromUtf8("每次基线更新")),
        makeField(QString::fromUtf8("一致性容差"), QStringLiteral("1e-3"))
    }));
    vl->addWidget(grad);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *iter = makePanel();
    iter->layout()->addWidget(makePanelTitle(QString::fromUtf8("迭代与收敛控制")));
    iter->layout()->addWidget(makeMiniFields({
        makeSelectField(QStringLiteral("MDA 求解器"), QStringLiteral("Block Gauss-Seidel"), {QStringLiteral("Newton-Krylov")}),
        makeField(QString::fromUtf8("最大内迭代"), QStringLiteral("40")),
        makeField(QString::fromUtf8("耦合残差"), QStringLiteral("1e-5")),
        makeField(QString::fromUtf8("松弛因子"), QStringLiteral("0.65"))
    }));
    iter->layout()->addWidget(makeProgressRow(QString::fromUtf8("耦合闭合准备度"), QStringLiteral("91%"), 91));
    al->addWidget(iter);
    auto *upd = makePanel();
    upd->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计点与方案更新")));
    upd->layout()->addWidget(makeSummary({
        {QString::fromUtf8("更新策略"), QString::fromUtf8("信赖域")},
        {QString::fromUtf8("边界处理"), QString::fromUtf8("投影")},
        {QString::fromUtf8("分析缓存"), QString::fromUtf8("复用")},
        {QString::fromUtf8("方案版本"), QString::fromUtf8("每 5 代")}
    }));
    al->addWidget(upd);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("问题检查")));
    chk->layout()->addWidget(makeBulletList({
        QString::fromUtf8("18 个设计变量已缩放"),
        QString::fromUtf8("3 个导数缺失，使用差分"),
        QString::fromUtf8("耦合环路已识别"),
        QString::fromUtf8("目标与约束维度一致")
    }));
    al->addWidget(chk);
    auto *btn = makeButton(QString::fromUtf8("构建并检查 MDO 问题"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

DesignPage::DesignPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("方案优化"),
        QString::fromUtf8("变量、约束、设计空间探索、优化任务与多学科求解统一配置"),
        QString::fromUtf8("设计空间"),
        {QString::fromUtf8("MDO 基准设计集 v3"), QString::fromUtf8("巡航效率专项")}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("variables"), QString::fromUtf8("设计变量与约束")},
        {QStringLiteral("exploration"), QString::fromUtf8("设计空间探索")},
        {QStringLiteral("optimization"), QString::fromUtf8("优化设计")},
        {QStringLiteral("mdo"), QString::fromUtf8("MDO 求解引擎")}
    }, QStringLiteral("variables"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(variablesPage(this));
    stack->addWidget(explorationPage(this));
    stack->addWidget(optimizationPage(this));
    stack->addWidget(mdoPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("exploration")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("optimization")) stack->setCurrentIndex(2);
        else if (id == QLatin1String("mdo")) stack->setCurrentIndex(3);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));
}
