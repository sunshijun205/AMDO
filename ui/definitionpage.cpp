#include "definitionpage.h"
#include "chartwidgets.h"
#include "uihelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

static QWidget *semanticPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("对象实例"), QStringLiteral("146"), QString::fromUtf8("个")},
        {QString::fromUtf8("类型定义"), QStringLiteral("38"), QString::fromUtf8("类")},
        {QString::fromUtf8("接口"), QStringLiteral("24"), QString::fromUtf8("个")},
        {QString::fromUtf8("语义完整度"), QStringLiteral("96"), QStringLiteral("%")}
    }));

    auto *top = new QWidget;
    auto *hl = new QHBoxLayout(top);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *mid = new QWidget;
    auto *ml = new QHBoxLayout(mid);
    ml->setContentsMargins(0, 0, 0, 0);
    ml->setSpacing(12);

    auto *treeP = makePanel();
    treeP->layout()->addWidget(makePanelTitle(QString::fromUtf8("方案对象树"), QStringLiteral("HX-01 / v12")));
    auto *tree = new QTreeWidget;
    tree->setHeaderHidden(true);
    tree->setColumnCount(2);
    tree->setRootIsDecorated(true);
    auto *rootItem = new QTreeWidgetItem(tree, {QStringLiteral("Aircraft HX-01"), QString::fromUtf8("根对象")});
    rootItem->setExpanded(true);
    const QStringList children = {
        QStringLiteral("Airframe"), QStringLiteral("PropulsionSystem"), QStringLiteral("EnergySystem"),
        QStringLiteral("FlightControlSystem"), QStringLiteral("MissionDefinition"), QStringLiteral("AnalysisModels")
    };
    const QStringList counts = {QStringLiteral("6"), QStringLiteral("2"), QStringLiteral("4"),
                                QStringLiteral("7"), QStringLiteral("1"), QStringLiteral("6")};
    for (int i = 0; i < children.size(); ++i)
        new QTreeWidgetItem(rootItem, {children[i], counts[i]});
    tree->setMinimumHeight(220);
    treeP->layout()->addWidget(tree);

    auto *attr = makePanel();
    attr->layout()->addWidget(makePanelTitle(QString::fromUtf8("对象类型、属性与标识")));
    attr->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("对象类型"), QStringLiteral("Aircraft")),
        makeField(QString::fromUtf8("永久标识"), QStringLiteral("urn:hx01:aircraft")),
        makeField(QString::fromUtf8("显示名称"), QStringLiteral("HX-01")),
        makeField(QString::fromUtf8("版本状态"), QString::fromUtf8("基线 v12")),
        makeField(QString::fromUtf8("长度单位"), QStringLiteral("m")),
        makeField(QString::fromUtf8("质量单位"), QStringLiteral("kg")),
        makeField(QString::fromUtf8("坐标系"), QString::fromUtf8("机体系 X前 Y右 Z下")),
        makeField(QString::fromUtf8("原点"), QString::fromUtf8("机鼻基准面"))
    }));
    ml->addWidget(treeP, 1);
    ml->addWidget(attr, 1);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *spec = makePanel();
    spec->layout()->addWidget(makePanelTitle(QString::fromUtf8("模型规范")));
    spec->layout()->addWidget(makeSummary({
        {QStringLiteral("Schema"), QStringLiteral("AircraftDM 2.3")},
        {QString::fromUtf8("命名空间"), QStringLiteral("hx01/core")},
        {QString::fromUtf8("引用策略"), QString::fromUtf8("永久 ID")},
        {QString::fromUtf8("交换格式"), QStringLiteral("JSON-LD")}
    }));
    al->addWidget(spec);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("语义检查")));
    chk->layout()->addWidget(makeBulletList({
        QString::fromUtf8("对象类型全部有效"),
        QString::fromUtf8("2 个属性缺少单位"),
        QString::fromUtf8("1 个循环引用待处理"),
        QString::fromUtf8("坐标系引用一致")
    }));
    al->addWidget(chk);
    al->addStretch();

    hl->addWidget(mid, 1);
    hl->addWidget(aside);
    lay->addWidget(top);

    auto *iface = makePanel();
    iface->layout()->addWidget(makePanelTitle(QString::fromUtf8("引用与接口"), QString::fromUtf8("显示 4 / 24")));
    iface->layout()->addWidget(makeTable(
        {QString::fromUtf8("接口编号"), QString::fromUtf8("对象关系"), QString::fromUtf8("类型"), QString::fromUtf8("交换内容"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("IF-001"), QStringLiteral("Wing ↔ Fuselage"), QString::fromUtf8("结构连接"), QString::fromUtf8("4 个参考点"), QString::fromUtf8("有效")},
            {QStringLiteral("IF-006"), QStringLiteral("Engine ↔ Nacelle"), QString::fromUtf8("安装接口"), QString::fromUtf8("轴线 + 安装面"), QString::fromUtf8("有效")},
            {QStringLiteral("IF-009"), QStringLiteral("FuelTank ↔ Mission"), QString::fromUtf8("资源接口"), QString::fromUtf8("可用燃油量"), QString::fromUtf8("有效")},
            {QStringLiteral("IF-012"), QStringLiteral("ControlSurface ↔ Dynamics"), QString::fromUtf8("控制接口"), QString::fromUtf8("偏角与速率"), QString::fromUtf8("待确认")}
        }));
    lay->addWidget(iface);
    return root;
}

static QWidget *configurationPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("构型类型"), QString::fromUtf8("常规布局"), QString()},
        {QString::fromUtf8("安装对象"), QStringLiteral("23"), QString::fromUtf8("个")},
        {QString::fromUtf8("连接关系"), QStringLiteral("31"), QString::fromUtf8("个")},
        {QString::fromUtf8("完整性"), QStringLiteral("94"), QStringLiteral("%")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *cfg = makePanel();
    cfg->layout()->addWidget(makePanelTitle(QString::fromUtf8("构型与总体布局")));
    cfg->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("飞机类别"), QString::fromUtf8("窄体运输机"), {QString::fromUtf8("支线客机"), QString::fromUtf8("公务机")}),
        makeField(QString::fromUtf8("机翼布局"), QString::fromUtf8("下单翼")),
        makeSelectField(QString::fromUtf8("尾翼构型"), QString::fromUtf8("常规尾翼"), {QString::fromUtf8("T 型尾翼"), QString::fromUtf8("V 尾")}),
        makeField(QString::fromUtf8("发动机布置"), QString::fromUtf8("翼下双发")),
        makeField(QString::fromUtf8("起落架形式"), QString::fromUtf8("前三点式")),
        makeField(QString::fromUtf8("客舱布局"), QString::fromUtf8("3-3 单通道"))
    }, 3));
    vl->addWidget(cfg);

    auto *sys = makePanel();
    sys->layout()->addWidget(makePanelTitle(QString::fromUtf8("系统、设备与安装定位"), QString::fromUtf8("显示 5 / 23")));
    sys->layout()->addWidget(makeTable(
        {QString::fromUtf8("对象"), QString::fromUtf8("方案"), QString::fromUtf8("安装/定位"), QString::fromUtf8("材料或属性"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("机翼"), QString::fromUtf8("后掠下单翼"), QString::fromUtf8("基准面 X=14.2 m"), QString::fromUtf8("CFRP 翼盒"), QString::fromUtf8("有效")},
            {QString::fromUtf8("动力装置"), QString::fromUtf8("翼下双发"), QStringLiteral("Y=±6.4 m"), QString::fromUtf8("涡扇 + 短舱"), QString::fromUtf8("有效")},
            {QString::fromUtf8("起落架"), QString::fromUtf8("前三点式"), QString::fromUtf8("机身/机翼连接"), QString::fromUtf8("钛合金接头"), QString::fromUtf8("有效")},
            {QString::fromUtf8("燃油系统"), QString::fromUtf8("中央翼盒 + 两侧机翼"), QString::fromUtf8("4 个油箱"), QString::fromUtf8("整体油箱"), QString::fromUtf8("有效")},
            {QString::fromUtf8("飞控系统"), QString::fromUtf8("电传操纵"), QString::fromUtf8("3 余度"), QString::fromUtf8("分布式安装"), QString::fromUtf8("待确认")}
        }));
    vl->addWidget(sys);

    auto *mat = makePanel();
    mat->layout()->addWidget(makePanelTitle(QString::fromUtf8("材料、物理属性与连接")));
    mat->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("主结构材料"), QStringLiteral("CFRP + Al-Li")),
        makeField(QString::fromUtf8("连接策略"), QString::fromUtf8("参数化连接点")),
        makeField(QString::fromUtf8("表面属性"), QString::fromUtf8("气动光顺 B")),
        makeField(QString::fromUtf8("密度来源"), QString::fromUtf8("材料库 v5")),
        makeField(QString::fromUtf8("对称关系"), QString::fromUtf8("XZ 面镜像")),
        makeField(QString::fromUtf8("拓扑检查"), QString::fromUtf8("自动"))
    }));
    vl->addWidget(mat);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *prog = makePanel();
    prog->layout()->addWidget(makePanelTitle(QString::fromUtf8("完整性检查")));
    prog->layout()->addWidget(makeProgressRow(QString::fromUtf8("对象完整"), QStringLiteral("100%"), 100));
    prog->layout()->addWidget(makeProgressRow(QString::fromUtf8("接口完整"), QStringLiteral("94%"), 94));
    prog->layout()->addWidget(makeProgressRow(QString::fromUtf8("分析就绪"), QStringLiteral("88%"), 88));
    al->addWidget(prog);
    auto *todo = makePanel();
    todo->layout()->addWidget(makePanelTitle(QString::fromUtf8("待处理")));
    todo->layout()->addWidget(makeBulletList({
        QString::fromUtf8("飞控系统 2 个安装点未确认"),
        QString::fromUtf8("短舱—机翼间隙缺少下限"),
        QString::fromUtf8("着陆构型舵面状态未定义")
    }));
    al->addWidget(todo);
    auto *btn = makeButton(QString::fromUtf8("执行完整性检查"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *geometryPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("几何参数"), QStringLiteral("86"), QString::fromUtf8("个")},
        {QString::fromUtf8("设计变量"), QStringLiteral("18"), QString::fromUtf8("个")},
        {QString::fromUtf8("驱动关系"), QStringLiteral("27"), QString::fromUtf8("条")},
        {QString::fromUtf8("更新状态"), QString::fromUtf8("已同步"), QString()}
    }));

    auto *two = new QWidget;
    auto *hl = new QHBoxLayout(two);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *view = makePanel();
    view->layout()->addWidget(makePanelTitle(QString::fromUtf8("参数化几何预览"), QString::fromUtf8("俯视参数骨架")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("AircraftView"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->addWidget(new AircraftTopView);
    view->layout()->addWidget(canvas);
    auto *dims = makePanel();
    dims->layout()->addWidget(makePanelTitle(QString::fromUtf8("尺寸与外形参数体系"), QString::fromUtf8("显示 6 / 86")));
    dims->layout()->addWidget(makeTable(
        {QString::fromUtf8("参数"), QString::fromUtf8("符号"), QString::fromUtf8("数值"), QString::fromUtf8("单位"), QString::fromUtf8("驱动方式")},
        {
            {QString::fromUtf8("机身总长"), QStringLiteral("L_fus"), QStringLiteral("38.20"), QStringLiteral("m"), QString::fromUtf8("基准变量")},
            {QString::fromUtf8("机翼面积"), QStringLiteral("S_ref"), QStringLiteral("124.0"), QString::fromUtf8("m²"), QString::fromUtf8("设计变量")},
            {QString::fromUtf8("机翼展长"), QStringLiteral("b"), QStringLiteral("34.15"), QStringLiteral("m"), QString::fromUtf8("由 S, AR 驱动")},
            {QString::fromUtf8("平均气动弦"), QStringLiteral("MAC"), QStringLiteral("4.16"), QStringLiteral("m"), QString::fromUtf8("自动计算")},
            {QString::fromUtf8("机翼后掠角"), QStringLiteral("Λ25"), QStringLiteral("25.0"), QStringLiteral("deg"), QString::fromUtf8("设计变量")},
            {QString::fromUtf8("水平尾翼面积"), QStringLiteral("S_ht"), QStringLiteral("31.4"), QString::fromUtf8("m²"), QString::fromUtf8("尾容量系数驱动")}
        }));
    hl->addWidget(view, 1);
    hl->addWidget(dims, 1);
    lay->addWidget(two);

    auto *bottom = new QWidget;
    auto *bl = new QHBoxLayout(bottom);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(12);
    auto *rel = makePanel();
    rel->layout()->addWidget(makePanelTitle(QString::fromUtf8("几何约束与驱动关系")));
    rel->layout()->addWidget(makeTable(
        {QString::fromUtf8("关系"), QString::fromUtf8("输入"), QString::fromUtf8("输出"), QString::fromUtf8("方式"), QString::fromUtf8("状态")},
        {
            {QString::fromUtf8("翼展计算"), QStringLiteral("S_ref, AR"), QStringLiteral("b"), QString::fromUtf8("公式驱动"), QString::fromUtf8("有效")},
            {QString::fromUtf8("尾翼面积"), QStringLiteral("V_h, MAC, l_h"), QStringLiteral("S_ht"), QString::fromUtf8("容量系数"), QString::fromUtf8("有效")},
            {QString::fromUtf8("发动机间隙"), QStringLiteral("D_nac, Y_eng"), QStringLiteral("Clearance"), QString::fromUtf8("不等式约束"), QString::fromUtf8("临界")}
        }));
    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *upd = makePanel();
    upd->layout()->addWidget(makePanelTitle(QString::fromUtf8("更新设置")));
    upd->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("更新模式"), QString::fromUtf8("参数变化后自动"), {QString::fromUtf8("手动更新")}),
        makeField(QString::fromUtf8("几何容差"), QStringLiteral("1e-4 m")),
        makeField(QString::fromUtf8("失败处理"), QString::fromUtf8("回退上一版本")),
        makeField(QString::fromUtf8("联动分析"), QString::fromUtf8("质量 + 气动"))
    }));
    al->addWidget(upd);
    auto *btn = makeButton(QString::fromUtf8("更新几何并检查"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();
    bl->addWidget(rel, 1);
    bl->addWidget(aside);
    lay->addWidget(bottom);
    return root;
}

static QWidget *visualizationPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *toolbar = new QWidget;
    auto *tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(9);
    tl->addWidget(makeSelectField(QString::fromUtf8("显示构型"), QString::fromUtf8("巡航构型"), {QString::fromUtf8("起飞构型"), QString::fromUtf8("着陆构型")}));
    tl->addWidget(makeSelectField(QString::fromUtf8("视图"), QString::fromUtf8("等轴测"), {QString::fromUtf8("俯视"), QString::fromUtf8("侧视"), QString::fromUtf8("正视")}));
    tl->addWidget(makeSelectField(QString::fromUtf8("着色方式"), QString::fromUtf8("按系统"), {QString::fromUtf8("按材料"), QString::fromUtf8("按学科")}));
    tl->addStretch();
    auto *cut = makeButton(QString::fromUtf8("剖切"));
    auto *measure = makeButton(QString::fromUtf8("测量"));
    auto *save = makeButton(QString::fromUtf8("保存视图"), true);
    wireDummyAction(cut, parent);
    wireDummyAction(measure, parent);
    wireDummyAction(save, parent);
    tl->addWidget(cut);
    tl->addWidget(measure);
    tl->addWidget(save);
    lay->addWidget(toolbar);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *view = makePanel();
    view->layout()->addWidget(makePanelTitle(QString::fromUtf8("三维方案视图"), QString::fromUtf8("HX-01 · 巡航构型")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("AircraftView"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->addWidget(new AircraftIsoView);
    view->layout()->addWidget(canvas);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *disp = makePanel();
    disp->layout()->addWidget(makePanelTitle(QString::fromUtf8("显示与选择")));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("机体结构"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("推进系统"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("设备与系统"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("参考几何"), false));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("接口与连接点"), false));
    al->addWidget(disp);
    auto *sel = makePanel();
    sel->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前选择：右发动机")));
    sel->layout()->addWidget(makeSummary({
        {QString::fromUtf8("对象"), QStringLiteral("Engine-02")},
        {QString::fromUtf8("位置"), QStringLiteral("X 15.2 / Y 6.4")},
        {QString::fromUtf8("方案参数"), QString::fromUtf8("7 个")},
        {QString::fromUtf8("关联分析"), QString::fromUtf8("推进 / 气动 / 重量")}
    }));
    al->addWidget(sel);
    auto *btn = makeButton(QString::fromUtf8("查看对象与分析关联"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(view, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

DefinitionPage::DefinitionPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("飞机方案定义"),
        QString::fromUtf8("以统一语义模型组织构型、系统、几何及其关联关系"),
        QString::fromUtf8("方案版本"),
        {QStringLiteral("HX-01 / v12"), QStringLiteral("HX-01 / v11")}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("semantic"), QString::fromUtf8("飞机语义数据模型")},
        {QStringLiteral("configuration"), QString::fromUtf8("总体方案配置")},
        {QStringLiteral("geometry"), QString::fromUtf8("参数化几何")},
        {QStringLiteral("visualization"), QString::fromUtf8("三维可视化")}
    }, QStringLiteral("semantic"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(semanticPage(this));
    stack->addWidget(configurationPage(this));
    stack->addWidget(geometryPage(this));
    stack->addWidget(visualizationPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("configuration")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("geometry")) stack->setCurrentIndex(2);
        else if (id == QLatin1String("visualization")) stack->setCurrentIndex(3);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));
}
