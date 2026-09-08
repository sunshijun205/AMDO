#include "requirementspage.h"
#include "chartwidgets.h"
#include "uihelpers.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *missionPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("任务场景"), QStringLiteral("4"), QString::fromUtf8("个")},
        {QString::fromUtf8("需求条目"), QStringLiteral("46"), QString::fromUtf8("项")},
        {QString::fromUtf8("强制需求"), QStringLiteral("18"), QString::fromUtf8("项")},
        {QString::fromUtf8("待澄清"), QStringLiteral("3"), QString::fromUtf8("项")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *railPanel = makePanel();
    railPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("主设计任务剖面"), QString::fromUtf8("任务 M-001")));
    railPanel->layout()->addWidget(new MissionRail({
        QString::fromUtf8("滑行/起飞"), QString::fromUtf8("爬升"), QString::fromUtf8("巡航"),
        QString::fromUtf8("下降"), QString::fromUtf8("备降"), QString::fromUtf8("盘旋"),
        QString::fromUtf8("着陆")
    }));
    vl->addWidget(railPanel);

    TableOptions opt;
    vl->addWidget([&] {
        auto *p = makePanel();
        p->layout()->addWidget(makePanelTitle(QString::fromUtf8("任务与使用场景"), QString::fromUtf8("4 个场景")));
        p->layout()->addWidget(makeTable(
            {QString::fromUtf8("场景"), QString::fromUtf8("任务边界"), QString::fromUtf8("环境/运营条件"), QString::fromUtf8("状态")},
            {
                {QString::fromUtf8("主设计任务"), QString::fromUtf8("180 座 / 5,500 km"), QString::fromUtf8("标准日"), QString::fromUtf8("已定义")},
                {QString::fromUtf8("高温高原起飞"), QString::fromUtf8("满载 / 2,100 m机场"), QStringLiteral("ISA+25°C"), QString::fromUtf8("已定义")},
                {QString::fromUtf8("备降任务"), QString::fromUtf8("200 nmi + 30 min盘旋"), QString::fromUtf8("标准日"), QString::fromUtf8("已定义")},
                {QString::fromUtf8("短程高频运营"), QString::fromUtf8("180 座 / 900 km"), QString::fromUtf8("年利用率 3,400 h"), QString::fromUtf8("草案")}
            }, opt));
        return p;
    }());

    TableOptions chipOpt;
    chipOpt.chipColumns = {4};
    vl->addWidget([&] {
        auto *p = makePanel();
        p->layout()->addWidget(makePanelTitle(QString::fromUtf8("任务与性能需求"), QString::fromUtf8("5 项核心需求")));
        p->layout()->addWidget(makeTable(
            {QString::fromUtf8("需求"), QString::fromUtf8("关系"), QString::fromUtf8("数值"), QString::fromUtf8("单位"), QString::fromUtf8("等级")},
            {
                {QString::fromUtf8("最大设计航程"), QString::fromUtf8("≥"), QStringLiteral("5,500"), QStringLiteral("km"), QString::fromUtf8("强制")},
                {QString::fromUtf8("设计商载"), QString::fromUtf8("≥"), QStringLiteral("18,000"), QStringLiteral("kg"), QString::fromUtf8("强制")},
                {QString::fromUtf8("巡航速度"), QString::fromUtf8("≥"), QStringLiteral("0.78"), QStringLiteral("Ma"), QString::fromUtf8("目标")},
                {QString::fromUtf8("任务燃油"), QString::fromUtf8("≤"), QStringLiteral("15,500"), QStringLiteral("kg"), QString::fromUtf8("目标")},
                {QString::fromUtf8("直接运营成本"), QString::fromUtf8("≤"), QString::fromUtf8("基线 -8"), QStringLiteral("%"), QString::fromUtf8("期望")}
            }, chipOpt));
        return p;
    }());

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);

    auto *def = makePanel();
    def->layout()->addWidget(makePanelTitle(QString::fromUtf8("场景定义")));
    def->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("任务类型"), QString::fromUtf8("客运运输"), {QString::fromUtf8("货运"), QString::fromUtf8("支线运输")}),
        makeField(QString::fromUtf8("典型乘员"), QStringLiteral("2 + 4")),
        makeField(QString::fromUtf8("年利用率"), QStringLiteral("3,400 h")),
        makeField(QString::fromUtf8("设计寿命"), QStringLiteral("60,000 FC"))
    }));
    al->addWidget(def);

    auto *src = makePanel();
    src->layout()->addWidget(makePanelTitle(QString::fromUtf8("需求来源")));
    src->layout()->addWidget(makeSummary({
        {QString::fromUtf8("市场需求"), QString::fromUtf8("15 项")},
        {QString::fromUtf8("运营方输入"), QString::fromUtf8("9 项")},
        {QString::fromUtf8("法规派生"), QString::fromUtf8("18 项")},
        {QString::fromUtf8("设计目标"), QString::fromUtf8("4 项")}
    }));
    al->addWidget(src);
    auto *save = makeButton(QString::fromUtf8("保存任务需求"), true);
    wireDummyAction(save, parent);
    al->addWidget(save);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *envelopePage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("设计工况"), QStringLiteral("12"), QString::fromUtf8("个")},
        {QString::fromUtf8("包线维度"), QStringLiteral("5"), QString::fromUtf8("维")},
        {QString::fromUtf8("大气模型"), QStringLiteral("ISA 1976"), QString()},
        {QString::fromUtf8("载荷因数"), QStringLiteral("-1.0～2.5"), QStringLiteral("g")}
    }));

    auto *two = new QWidget;
    auto *hl = new QHBoxLayout(two);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *chartP = makePanel();
    chartP->layout()->addWidget(makePanelTitle(QString::fromUtf8("高度—速度飞行包线"), QString::fromUtf8("设计包线 v4")));
    chartP->layout()->addWidget(new EnvelopeChart);
    auto *env = makePanel();
    env->layout()->addWidget(makePanelTitle(QString::fromUtf8("环境与边界条件")));
    env->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("大气模型"), QStringLiteral("US Standard 1976"), {QString::fromUtf8("自定义大气")}),
        makeField(QString::fromUtf8("温度偏差"), QStringLiteral("ISA +15°C")),
        makeField(QString::fromUtf8("侧风上限"), QStringLiteral("20 kt")),
        makeField(QString::fromUtf8("跑道坡度"), QStringLiteral("±2.0 %")),
        makeSelectField(QString::fromUtf8("阵风模型"), QStringLiteral("1-cos"), {QStringLiteral("Von Kármán")}),
        makeField(QString::fromUtf8("结冰条件"), QString::fromUtf8("附录 C"))
    }));
    env->layout()->addWidget(makeProgressRow(QString::fromUtf8("工况覆盖完整度"), QStringLiteral("88%"), 88));
    hl->addWidget(chartP, 1);
    hl->addWidget(env, 1);
    lay->addWidget(two);

    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计工况集"), QString::fromUtf8("显示 5 / 12")));
    tbl->layout()->addWidget(makeTable(
        {QString::fromUtf8("编号"), QString::fromUtf8("阶段"), QString::fromUtf8("高度"), QString::fromUtf8("速度"), QString::fromUtf8("构型"), QString::fromUtf8("大气")},
        {
            {QStringLiteral("FC-01"), QString::fromUtf8("起飞"), QStringLiteral("0 m"), QStringLiteral("0.20 Ma"), QString::fromUtf8("襟翼 15°"), QStringLiteral("ISA+15")},
            {QStringLiteral("FC-02"), QString::fromUtf8("初始爬升"), QStringLiteral("1,500 m"), QStringLiteral("0.35 Ma"), QString::fromUtf8("清洁"), QStringLiteral("ISA+15")},
            {QStringLiteral("FC-03"), QString::fromUtf8("巡航"), QStringLiteral("11,000 m"), QStringLiteral("0.78 Ma"), QString::fromUtf8("清洁"), QStringLiteral("ISA")},
            {QStringLiteral("FC-04"), QString::fromUtf8("最大机动"), QStringLiteral("7,500 m"), QStringLiteral("0.72 Ma"), QString::fromUtf8("清洁"), QStringLiteral("ISA")},
            {QStringLiteral("FC-05"), QString::fromUtf8("着陆进近"), QStringLiteral("0 m"), QStringLiteral("0.18 Ma"), QString::fromUtf8("全襟翼"), QStringLiteral("ISA+15")}
        }));
    lay->addWidget(tbl);
    return root;
}

static QWidget *standardsPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("适用规范"), QStringLiteral("3"), QString::fromUtf8("套")},
        {QString::fromUtf8("适用条款"), QStringLiteral("128"), QString::fromUtf8("项")},
        {QString::fromUtf8("已完成映射"), QStringLiteral("112"), QString::fromUtf8("项")},
        {QString::fromUtf8("待确认"), QStringLiteral("7"), QString::fromUtf8("项")}
    }));

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
    base->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("适航类别"), QString::fromUtf8("CS-25 大型飞机"), {QStringLiteral("FAR Part 25")}),
        makeSelectField(QString::fromUtf8("运行规则"), QString::fromUtf8("CAT 商业运输"), {QString::fromUtf8("非商业运行")}),
        makeField(QString::fromUtf8("审定修订版"), QStringLiteral("Amendment 28")),
        makeField(QString::fromUtf8("噪声标准"), QStringLiteral("ICAO Annex 16 Ch.14")),
        makeField(QString::fromUtf8("排放标准"), QStringLiteral("CAEP/8")),
        makeField(QString::fromUtf8("特殊条件"), QString::fromUtf8("电推进：不适用"))
    }, 3));
    vl->addWidget(base);

    auto *mat = makePanel();
    mat->layout()->addWidget(makePanelTitle(QString::fromUtf8("条款适用性矩阵"), QString::fromUtf8("显示 5 / 128")));
    mat->layout()->addWidget(makeTable(
        {QString::fromUtf8("条款"), QString::fromUtf8("主题"), QString::fromUtf8("责任域"), QString::fromUtf8("适用性"), QString::fromUtf8("需求映射")},
        {
            {QStringLiteral("CS-25.121"), QString::fromUtf8("起飞/爬升性能"), QString::fromUtf8("任务与性能"), QString::fromUtf8("适用"), QString::fromUtf8("已映射")},
            {QStringLiteral("CS-25.143"), QString::fromUtf8("操纵性一般要求"), QString::fromUtf8("操稳与飞行动力学"), QString::fromUtf8("适用"), QString::fromUtf8("已映射")},
            {QStringLiteral("CS-25.301"), QString::fromUtf8("载荷与强度"), QString::fromUtf8("结构"), QString::fromUtf8("适用"), QString::fromUtf8("已映射")},
            {QStringLiteral("CS-25.331"), QString::fromUtf8("对称机动条件"), QString::fromUtf8("结构"), QString::fromUtf8("适用"), QString::fromUtf8("待确认")},
            {QStringLiteral("CS-25.1309"), QString::fromUtf8("设备与系统安全性"), QString::fromUtf8("系统"), QString::fromUtf8("适用"), QString::fromUtf8("草案")}
        }));
    vl->addWidget(mat);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *judge = makePanel();
    judge->layout()->addWidget(makePanelTitle(QString::fromUtf8("适用性判定")));
    judge->layout()->addWidget(makeSummary({
        {QString::fromUtf8("直接适用"), QStringLiteral("96")},
        {QString::fromUtf8("条件适用"), QStringLiteral("24")},
        {QString::fromUtf8("不适用"), QStringLiteral("8")},
        {QString::fromUtf8("需等效安全"), QStringLiteral("0")}
    }));
    al->addWidget(judge);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("完整性检查")));
    chk->layout()->addWidget(makeBulletList({
        QString::fromUtf8("7 项条款缺少责任人"),
        QString::fromUtf8("3 项需求尚未建立验证方法"),
        QString::fromUtf8("所有强制约束已有来源"),
        QString::fromUtf8("适用版本无冲突")
    }));
    al->addWidget(chk);
    auto *pub = makeButton(QString::fromUtf8("发布适用性基线"), true);
    wireDummyAction(pub, parent);
    al->addWidget(pub);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *metricsPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("评价指标"), QStringLiteral("24"), QString::fromUtf8("项")},
        {QString::fromUtf8("强制约束"), QStringLiteral("18"), QString::fromUtf8("项")},
        {QString::fromUtf8("目标指标"), QStringLiteral("11"), QString::fromUtf8("项")},
        {QString::fromUtf8("需求覆盖率"), QStringLiteral("93"), QStringLiteral("%")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    TableOptions chipOpt;
    chipOpt.chipColumns = {2};
    auto *tbl = makePanel();
    tbl->layout()->addWidget(makePanelTitle(QString::fromUtf8("评价指标与需求约束"), QString::fromUtf8("显示 6 / 53")));
    tbl->layout()->addWidget(makeTable(
        {QString::fromUtf8("需求编号"), QString::fromUtf8("指标"), QString::fromUtf8("责任域"), QString::fromUtf8("约束/目标"), QString::fromUtf8("等级"), QString::fromUtf8("来源"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("REQ-PERF-001"), QString::fromUtf8("设计航程"), QString::fromUtf8("任务与性能"), QString::fromUtf8("≥ 5,500 km"), QString::fromUtf8("强制"), QString::fromUtf8("任务 M-001"), QString::fromUtf8("已验证")},
            {QStringLiteral("REQ-WGT-004"), QString::fromUtf8("最大起飞重量"), QString::fromUtf8("重量与质量特性"), QString::fromUtf8("≤ 72,000 kg"), QString::fromUtf8("强制"), QString::fromUtf8("市场约束"), QString::fromUtf8("已分配")},
            {QStringLiteral("REQ-AERO-007"), QString::fromUtf8("巡航升阻比"), QString::fromUtf8("气动"), QString::fromUtf8("≥ 18.5"), QString::fromUtf8("目标"), QString::fromUtf8("效率目标"), QString::fromUtf8("已分配")},
            {QStringLiteral("REQ-STR-011"), QString::fromUtf8("屈曲裕度"), QString::fromUtf8("结构"), QString::fromUtf8("≥ 0.15"), QString::fromUtf8("强制"), QStringLiteral("CS-25.301"), QString::fromUtf8("已分配")},
            {QStringLiteral("REQ-DYN-006"), QString::fromUtf8("短周期阻尼比"), QString::fromUtf8("操稳与飞行动力学"), QString::fromUtf8("≥ 0.30"), QString::fromUtf8("强制"), QStringLiteral("CS-25.181"), QString::fromUtf8("待验证")},
            {QStringLiteral("REQ-COST-003"), QString::fromUtf8("直接运营成本"), QString::fromUtf8("总体"), QString::fromUtf8("≤ 基线 -8%"), QString::fromUtf8("期望"), QString::fromUtf8("商业目标"), QString::fromUtf8("草案")}
        }, chipOpt));

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *attr = makePanel();
    attr->layout()->addWidget(makePanelTitle(QString::fromUtf8("需求属性")));
    attr->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("约束类型"), QString::fromUtf8("指标性约束"), {QString::fromUtf8("强制性约束")}),
        makeField(QString::fromUtf8("优先级"), QStringLiteral("P1")),
        makeSelectField(QString::fromUtf8("验证方法"), QString::fromUtf8("分析"), {QString::fromUtf8("试验"), QString::fromUtf8("检查")}),
        makeField(QString::fromUtf8("成熟度"), QString::fromUtf8("已分配"))
    }));
    al->addWidget(attr);
    auto *trace = makePanel();
    trace->layout()->addWidget(makePanelTitle(QString::fromUtf8("追溯关系")));
    trace->layout()->addWidget(makeSummary({
        {QString::fromUtf8("上游来源"), QString::fromUtf8("任务 M-001")},
        {QString::fromUtf8("下游变量"), QString::fromUtf8("4 个")},
        {QString::fromUtf8("分析响应"), QString::fromUtf8("2 个")},
        {QString::fromUtf8("验证证据"), QStringLiteral("Run-240904")}
    }));
    al->addWidget(trace);
    auto *btn = makeButton(QString::fromUtf8("创建需求基线"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(tbl, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

RequirementsPage::RequirementsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("设计需求与评价定义"),
        QString::fromUtf8("从任务场景、适航边界到可追溯的指标与约束"),
        QString::fromUtf8("需求基线"),
        {QString::fromUtf8("SRD 基线 v6"), QString::fromUtf8("市场需求草案")}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("mission"), QString::fromUtf8("任务与使用场景")},
        {QStringLiteral("envelope"), QString::fromUtf8("工况与飞行包线")},
        {QStringLiteral("standards"), QString::fromUtf8("规范与适用性")},
        {QStringLiteral("metrics"), QString::fromUtf8("评价指标与需求约束")}
    }, QStringLiteral("mission"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(missionPage(this));
    stack->addWidget(envelopePage(this));
    stack->addWidget(standardsPage(this));
    stack->addWidget(metricsPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("envelope")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("standards")) stack->setCurrentIndex(2);
        else if (id == QLatin1String("metrics")) stack->setCurrentIndex(3);
        else stack->setCurrentIndex(0);
    });

    outer->addWidget(wrapScroll(body));
}
