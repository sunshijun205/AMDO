#include "workflowpage.h"
#include "uihelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *flowRow(const QList<QFrame *> &nodes, bool loop = false)
{
    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(9);
    hl->addStretch();
    for (int i = 0; i < nodes.size(); ++i) {
        hl->addWidget(nodes[i]);
        if (i + 1 < nodes.size())
            hl->addWidget(makeFlowArrow(loop && i == 0 ? QString::fromUtf8("↺") : QString::fromUtf8("→")));
    }
    hl->addStretch();
    return row;
}

static QWidget *definitionFlow(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("流程节点"), QStringLiteral("14"), QString::fromUtf8("个")},
        {QString::fromUtf8("数据连接"), QStringLiteral("23"), QString::fromUtf8("条")},
        {QString::fromUtf8("分支 / 循环"), QStringLiteral("2 / 1"), QString()},
        {QString::fromUtf8("可用模板"), QStringLiteral("6"), QString::fromUtf8("个")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *flowP = makePanel();
    flowP->layout()->addWidget(makePanelTitle(QString::fromUtf8("流程定义与编排"), QString::fromUtf8("MDO 基准工作流 v5"),
                                              QString::fromUtf8("拖入分析节点，连接输入输出，并定义分支、循环与迭代控制")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("FlowCanvas"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(22, 22, 22, 22);
    cl->setSpacing(20);
    cl->addWidget(flowRow({
        makeFlowNode(QString::fromUtf8("方案输入"), QStringLiteral("HX-01 / v12"), QStringLiteral("done")),
        makeFlowNode(QString::fromUtf8("数据准备"), QString::fromUtf8("单位 / 坐标 / 映射"), QStringLiteral("done")),
        makeFlowNode(QString::fromUtf8("MDO 驱动器"), QString::fromUtf8("MDF · 外循环"), QStringLiteral("active"))
    }));
    auto *branch = new QFrame;
    branch->setStyleSheet(QStringLiteral("border-top: 1px solid #d5dfe4;"));
    auto *bl = new QVBoxLayout(branch);
    bl->setContentsMargins(0, 8, 0, 0);
    auto *brow = new QWidget;
    auto *bhl = new QHBoxLayout(brow);
    bhl->setContentsMargins(0, 0, 0, 0);
    bhl->setSpacing(9);
    bhl->addStretch();
    const QStringList titles = {
        QString::fromUtf8("气动"), QString::fromUtf8("结构"), QString::fromUtf8("重量"),
        QString::fromUtf8("推进"), QString::fromUtf8("操稳"), QString::fromUtf8("任务")
    };
    const QStringList subs = {
        QStringLiteral("VLM"), QString::fromUtf8("梁壳模型"), QString::fromUtf8("重量闭环"),
        QString::fromUtf8("循环分析"), QString::fromUtf8("配平 / 模态"), QString::fromUtf8("航段积分")
    };
    for (int i = 0; i < titles.size(); ++i)
        bhl->addWidget(makeFlowNode(titles[i], subs[i]));
    bhl->addStretch();
    bl->addWidget(brow);
    cl->addWidget(branch);
    cl->addWidget(flowRow({
        makeFlowNode(QString::fromUtf8("耦合收敛判定"), QString::fromUtf8("残差 ≤ 1e-5"), QStringLiteral("control")),
        makeFlowNode(QString::fromUtf8("方案更新"), QString::fromUtf8("变量 / 版本")),
        makeFlowNode(QString::fromUtf8("指标汇总"), QString::fromUtf8("目标 / 约束"))
    }, true));
    auto *cap = new QLabel(QString::fromUtf8("外循环由 MDO 驱动器控制；六个学科节点在耦合闭合后向方案决策输出统一指标集。"));
    cap->setObjectName(QStringLiteral("MutedLabel"));
    cap->setAlignment(Qt::AlignCenter);
    cap->setStyleSheet(QStringLiteral("font-size: 11px;"));
    cl->addWidget(cap);
    flowP->layout()->addWidget(canvas);
    vl->addWidget(flowP);

    auto *nodes = makePanel();
    nodes->layout()->addWidget(makePanelTitle(QString::fromUtf8("节点清单与连接"), QString::fromUtf8("显示 6 / 14")));
    nodes->layout()->addWidget(makeTable(
        {QString::fromUtf8("编号"), QString::fromUtf8("节点"), QString::fromUtf8("类型"), QString::fromUtf8("上游输入"), QString::fromUtf8("下游输出"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("N01"), QString::fromUtf8("方案输入"), QString::fromUtf8("数据准备"), QString::fromUtf8("—"), QString::fromUtf8("标准数据集"), QString::fromUtf8("已配置")},
            {QStringLiteral("N02"), QString::fromUtf8("气动分析"), QString::fromUtf8("学科节点"), QString::fromUtf8("方案参数"), QString::fromUtf8("标准数据集"), QString::fromUtf8("已配置")},
            {QStringLiteral("N03"), QString::fromUtf8("结构分析"), QString::fromUtf8("学科节点"), QString::fromUtf8("方案参数"), QString::fromUtf8("标准数据集"), QString::fromUtf8("已配置")},
            {QStringLiteral("N04"), QString::fromUtf8("重量闭环"), QString::fromUtf8("计算节点"), QString::fromUtf8("方案参数"), QString::fromUtf8("标准数据集"), QString::fromUtf8("已配置")},
            {QStringLiteral("N05"), QString::fromUtf8("MDO 驱动器"), QString::fromUtf8("控制节点"), QString::fromUtf8("目标 / 约束"), QString::fromUtf8("标准数据集"), QString::fromUtf8("已配置")},
            {QStringLiteral("N06"), QString::fromUtf8("任务性能"), QString::fromUtf8("学科节点"), QString::fromUtf8("方案参数"), QString::fromUtf8("任务指标"), QString::fromUtf8("待校验")}
        }));
    vl->addWidget(nodes);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *cur = makePanel();
    cur->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前节点：MDO 驱动器")));
    cur->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("节点类型"), QString::fromUtf8("优化控制")),
        makeField(QString::fromUtf8("执行环境"), QString::fromUtf8("本地集群")),
        makeField(QString::fromUtf8("输入端口"), QString::fromUtf8("18 个")),
        makeField(QString::fromUtf8("输出端口"), QString::fromUtf8("12 个")),
        makeSelectField(QString::fromUtf8("失败策略"), QString::fromUtf8("重试后跳过"), {QString::fromUtf8("立即终止")}),
        makeField(QString::fromUtf8("缓存策略"), QString::fromUtf8("按输入哈希复用"))
    }));
    al->addWidget(cur);
    auto *loopP = makePanel();
    loopP->layout()->addWidget(makePanelTitle(QString::fromUtf8("分支、循环与迭代")));
    loopP->layout()->addWidget(makeSummary({
        {QString::fromUtf8("并行分支"), QString::fromUtf8("气动 / 结构")},
        {QString::fromUtf8("耦合循环"), QString::fromUtf8("6 学科")},
        {QString::fromUtf8("循环上限"), QString::fromUtf8("40 次")},
        {QString::fromUtf8("收敛判据"), QString::fromUtf8("残差 1e-5")}
    }));
    al->addWidget(loopP);
    auto *tmpl = makePanel();
    tmpl->layout()->addWidget(makePanelTitle(QString::fromUtf8("流程模板")));
    tmpl->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("模板"), QString::fromUtf8("MDO 基准流程"), {QString::fromUtf8("单学科串行分析"), QString::fromUtf8("DOE 批量分析")}),
        makeField(QString::fromUtf8("版本"), QString::fromUtf8("v5 / 已发布"))
    }));
    al->addWidget(tmpl);
    auto *btn = makeButton(QString::fromUtf8("校验并保存流程"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *executionPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("执行计划"), QStringLiteral("5"), QString::fromUtf8("阶段")},
        {QString::fromUtf8("最大并行"), QStringLiteral("12"), QString::fromUtf8("任务")},
        {QString::fromUtf8("预计耗时"), QStringLiteral("1 h 15 min"), QString()},
        {QString::fromUtf8("检查点"), QStringLiteral("每 5"), QString::fromUtf8("次迭代")}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *sched = makePanel();
    sched->layout()->addWidget(makePanelTitle(QString::fromUtf8("调度与运行资源"), QString::fromUtf8("执行配置 EXE-08")));
    sched->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("调度模式"), QString::fromUtf8("依赖图自动调度"), {QString::fromUtf8("严格串行")}),
        makeSelectField(QString::fromUtf8("运行后端"), QString::fromUtf8("本地 + 集群"), {QString::fromUtf8("仅本地")}),
        makeField(QString::fromUtf8("最大并行任务"), QStringLiteral("12")),
        makeField(QString::fromUtf8("单任务超时"), QStringLiteral("45 min")),
        makeField(QString::fromUtf8("失败重试"), QString::fromUtf8("2 次")),
        makeField(QString::fromUtf8("许可证策略"), QString::fromUtf8("按节点预留"))
    }, 3));
    vl->addWidget(sched);

    auto *plan = makePanel();
    plan->layout()->addWidget(makePanelTitle(QString::fromUtf8("执行计划"), QString::fromUtf8("按依赖关系自动生成")));
    plan->layout()->addWidget(makeTable(
        {QString::fromUtf8("阶段"), QString::fromUtf8("任务组"), QString::fromUtf8("执行方式"), QString::fromUtf8("资源"), QString::fromUtf8("预计耗时"), QString::fromUtf8("状态")},
        {
            {QStringLiteral("01"), QString::fromUtf8("准备输入与版本锁定"), QString::fromUtf8("串行"), QString::fromUtf8("本地"), QStringLiteral("2 min"), QString::fromUtf8("就绪")},
            {QStringLiteral("02"), QString::fromUtf8("气动 / 结构并行计算"), QString::fromUtf8("并行 2"), QString::fromUtf8("计算队列 A"), QStringLiteral("18 min"), QString::fromUtf8("就绪")},
            {QStringLiteral("03"), QString::fromUtf8("重量 / 推进 / 操稳更新"), QString::fromUtf8("混合"), QString::fromUtf8("计算队列 A"), QStringLiteral("11 min"), QString::fromUtf8("就绪")},
            {QStringLiteral("04"), QString::fromUtf8("耦合闭合与方案更新"), QString::fromUtf8("循环 ≤40"), QStringLiteral("MDO Worker"), QStringLiteral("36 min"), QString::fromUtf8("需检查")},
            {QStringLiteral("05"), QString::fromUtf8("任务性能与指标汇总"), QString::fromUtf8("串行"), QString::fromUtf8("本地"), QStringLiteral("8 min"), QString::fromUtf8("就绪")}
        }));
    vl->addWidget(plan);

    auto *data = makePanel();
    data->layout()->addWidget(makePanelTitle(QString::fromUtf8("数据、缓存与版本")));
    data->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("输入版本"), QStringLiteral("HX-01 / v12")),
        makeField(QString::fromUtf8("结果目录"), QStringLiteral("Run-240904-W05")),
        makeField(QString::fromUtf8("缓存复用"), QString::fromUtf8("相同输入直接复用")),
        makeField(QString::fromUtf8("中间结果"), QString::fromUtf8("全部保留")),
        makeField(QString::fromUtf8("检查点"), QString::fromUtf8("每 5 次外迭代")),
        makeField(QString::fromUtf8("完成后版本"), QString::fromUtf8("自动生成候选方案"))
    }, 3));
    vl->addWidget(data);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行前检查")));
    chk->layout()->addWidget(makeBulletList({
        QString::fromUtf8("14 个节点定义完整"),
        QString::fromUtf8("23 条数据连接单位一致"),
        QString::fromUtf8("12 个许可证席位可用"),
        QString::fromUtf8("耦合循环初值未锁定")
    }));
    chk->layout()->addWidget(makeProgressRow(QString::fromUtf8("配置完整度"), QStringLiteral("94%"), 94));
    al->addWidget(chk);
    auto *stop = makePanel();
    stop->layout()->addWidget(makePanelTitle(QString::fromUtf8("停止与恢复策略")));
    stop->layout()->addWidget(makeMiniFields({
        makeField(QString::fromUtf8("异常阈值"), QString::fromUtf8("3 个连续失败")),
        makeField(QString::fromUtf8("恢复方式"), QString::fromUtf8("最近检查点")),
        makeField(QString::fromUtf8("人工确认"), QString::fromUtf8("仅高风险失败")),
        makeField(QString::fromUtf8("日志级别"), QString::fromUtf8("标准"))
    }));
    al->addWidget(stop);
    auto *btn = makeButton(QString::fromUtf8("启动工作流"), true);
    wireDummyAction(btn, parent);
    al->addWidget(btn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

static QWidget *monitorPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("运行状态"), QString::fromUtf8("执行中"), QString()},
        {QString::fromUtf8("外迭代"), QStringLiteral("18 / 60"), QString()},
        {QString::fromUtf8("并行任务"), QStringLiteral("7 / 12"), QString()},
        {QString::fromUtf8("预计剩余"), QStringLiteral("38 min"), QString()}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *live = makePanel();
    live->layout()->addWidget(makePanelTitle(QString::fromUtf8("实时流程状态"), QStringLiteral("Run-240904-W05 · 18:42:16 更新")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("FlowCanvas"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(22, 22, 22, 22);
    cl->setSpacing(20);
    cl->addWidget(flowRow({
        makeFlowNode(QString::fromUtf8("方案输入"), QString::fromUtf8("完成"), QStringLiteral("done")),
        makeFlowNode(QString::fromUtf8("气动 / 结构"), QString::fromUtf8("完成"), QStringLiteral("done")),
        makeFlowNode(QString::fromUtf8("推进分析"), QString::fromUtf8("运行中 · 68%"), QStringLiteral("running")),
        makeFlowNode(QString::fromUtf8("任务性能"), QString::fromUtf8("等待"), QStringLiteral("pending"))
    }));
    cl->addWidget(flowRow({
        makeFlowNode(QString::fromUtf8("MDO 外循环"), QString::fromUtf8("第 18 / 60 代"), QStringLiteral("active")),
        makeFlowNode(QString::fromUtf8("当前最优"), QStringLiteral("C-027 · 86.4"))
    }, true));
    live->layout()->addWidget(canvas);
    vl->addWidget(live);

    auto *queue = makePanel();
    queue->layout()->addWidget(makePanelTitle(QString::fromUtf8("任务队列"), QString::fromUtf8("6 个活动节点")));
    queue->layout()->addWidget(makeTable(
        {QString::fromUtf8("任务"), QString::fromUtf8("分析节点"), QString::fromUtf8("状态"), QString::fromUtf8("耗时"), QString::fromUtf8("进度")},
        {
            {QStringLiteral("AERO-018"), QString::fromUtf8("气动分析"), QString::fromUtf8("完成"), QStringLiteral("18.4 s"), QStringLiteral("100%")},
            {QStringLiteral("STRU-018"), QString::fromUtf8("结构分析"), QString::fromUtf8("完成"), QStringLiteral("42.7 s"), QStringLiteral("100%")},
            {QStringLiteral("MASS-018"), QString::fromUtf8("重量闭环"), QString::fromUtf8("完成"), QStringLiteral("3.2 s"), QStringLiteral("100%")},
            {QStringLiteral("PROP-018"), QString::fromUtf8("推进分析"), QString::fromUtf8("运行中"), QStringLiteral("12.1 s"), QStringLiteral("68%")},
            {QStringLiteral("FDM-018"), QString::fromUtf8("操稳分析"), QString::fromUtf8("等待"), QString::fromUtf8("—"), QStringLiteral("0%")},
            {QStringLiteral("MISSION-018"), QString::fromUtf8("任务性能"), QString::fromUtf8("等待"), QString::fromUtf8("—"), QStringLiteral("0%")}
        }));
    vl->addWidget(queue);

    auto *logP = makePanel();
    logP->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行日志"), QString::fromUtf8("自动滚动")));
    auto *log = new QLabel(QStringLiteral(
        "18:41:52 [MDO] iteration 18 started\n"
        "18:41:58 [AERO] cache hit · dataset AERO-017\n"
        "18:42:03 [STRUCTURE] solution converged · residual 6.8e-6\n"
        "18:42:12 [PROPULSION] evaluating off-design point 8 / 12\n"
        "18:42:16 [SCHEDULER] 7 active · 5 queued · 0 failed"));
    log->setObjectName(QStringLiteral("LogView"));
    log->setTextInteractionFlags(Qt::TextSelectableByMouse);
    logP->layout()->addWidget(log);
    vl->addWidget(logP);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *conv = makePanel();
    conv->layout()->addWidget(makePanelTitle(QString::fromUtf8("收敛监控")));
    conv->layout()->addWidget(makeProgressRow(QString::fromUtf8("目标改善"), QStringLiteral("73%"), 73));
    conv->layout()->addWidget(makeProgressRow(QString::fromUtf8("耦合残差"), QStringLiteral("8.2e-5"), 82));
    conv->layout()->addWidget(makeProgressRow(QString::fromUtf8("可行方案比例"), QStringLiteral("61%"), 61));
    al->addWidget(conv);
    auto *pt = makePanel();
    pt->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前设计点")));
    pt->layout()->addWidget(makeSummary({
        {QString::fromUtf8("候选方案"), QStringLiteral("C-027")},
        {QString::fromUtf8("任务燃油"), QStringLiteral("14,860 kg")},
        {QStringLiteral("MTOW"), QStringLiteral("68,420 kg")},
        {QString::fromUtf8("约束违反"), QStringLiteral("0")}
    }));
    al->addWidget(pt);
    auto *pause = makeButton(QString::fromUtf8("暂停并保存检查点"));
    auto *view = makeButton(QString::fromUtf8("查看当前最优方案"), true);
    wireDummyAction(pause, parent);
    wireDummyAction(view, parent);
    al->addWidget(pause);
    al->addWidget(view);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

WorkflowPage::WorkflowPage(QWidget *parent)
    : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("工作流编排与执行"),
        QString::fromUtf8("组织分析节点、数据依赖、分支循环与计算资源，并追踪跨学科流程运行"),
        QString::fromUtf8("工作流"),
        {QString::fromUtf8("MDO 基准工作流 v5"), QString::fromUtf8("DOE 批量评估流程")}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("definition"), QString::fromUtf8("流程定义与编排")},
        {QStringLiteral("execution"), QString::fromUtf8("执行设置")},
        {QStringLiteral("monitor"), QString::fromUtf8("运行监控")}
    }, QStringLiteral("definition"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(definitionFlow(this));
    stack->addWidget(executionPage(this));
    stack->addWidget(monitorPage(this));
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("execution")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("monitor")) stack->setCurrentIndex(2);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));
}
