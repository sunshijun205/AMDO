#include "model/analysiscatalogs.h"

QVector<AnalysisDomain> AnalysisCatalogs::domains()
{
    QVector<AnalysisDomain> d;
    d.append({
        QStringLiteral("aero"), QString::fromUtf8("气动"),
        QString::fromUtf8("气动分析设置"), QString::fromUtf8("模型、流动条件、几何离散与收敛控制"),
        {QString::fromUtf8("概念设计｜VLM 快速评估"), QString::fromUtf8("高升力｜面板法修正"), QString::fromUtf8("巡航｜RANS CFD 校核")},
        {
            {QString::fromUtf8("分析方法"), {
                {QString::fromUtf8("求解方法"), QStringLiteral("select"), QStringLiteral("Vortex Lattice Method"), QString(), {}},
                {QString::fromUtf8("计算保真度"), QStringLiteral("select"), QString::fromUtf8("中等（构型级）"), QString(), {}},
                {QString::fromUtf8("几何来源"), QStringLiteral("select"), QString::fromUtf8("基准构型 / 自动同步"), QString(), {}},
                {QString::fromUtf8("气动数据库"), QStringLiteral("select"), QString::fromUtf8("实时计算 + 缓存"), QString(), {}},
                {QString::fromUtf8("代理模型"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}},
                {QString::fromUtf8("复用训练数据"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}}
            }},
            {QString::fromUtf8("飞行与流动条件"), {
                {QString::fromUtf8("大气模型"), QStringLiteral("select"), QStringLiteral("US Standard Atmosphere 1976"), QString(), {}},
                {QString::fromUtf8("流动区域"), QStringLiteral("select"), QString::fromUtf8("亚声速 / 可压缩修正"), QString(), {}},
                {QString::fromUtf8("高度"), QStringLiteral("number"), QStringLiteral("11000"), QStringLiteral("m"), {}},
                {QString::fromUtf8("马赫数"), QStringLiteral("number"), QStringLiteral("0.78"), QStringLiteral("Ma"), {}},
                {QString::fromUtf8("迎角范围"), QStringLiteral("text"), QString::fromUtf8("-4 ～ 14"), QStringLiteral("deg"), {}},
                {QString::fromUtf8("侧滑角范围"), QStringLiteral("text"), QString::fromUtf8("-6 ～ 6"), QStringLiteral("deg"), {}}
            }},
            {QString::fromUtf8("几何与离散"), {
                {QString::fromUtf8("机翼展向面元"), QStringLiteral("number"), QStringLiteral("48"), QString::fromUtf8("个"), {}},
                {QString::fromUtf8("机翼弦向面元"), QStringLiteral("number"), QStringLiteral("20"), QString::fromUtf8("个"), {}},
                {QString::fromUtf8("余弦加密"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}},
                {QString::fromUtf8("机身建模"), QStringLiteral("select"), QString::fromUtf8("等效面板体"), QString(), {}},
                {QString::fromUtf8("尾迹模型"), QStringLiteral("select"), QString::fromUtf8("自由尾迹 / 12步"), QString(), {}},
                {QString::fromUtf8("螺旋桨滑流"), QStringLiteral("select"), QString::fromUtf8("执行盘耦合"), QString(), {}}
            }},
            {QString::fromUtf8("修正与求解控制"), {
                {QString::fromUtf8("转捩模型"), QStringLiteral("select"), QString::fromUtf8("eⁿ / 指定 Ncrit"), QString(), {}},
                {QString::fromUtf8("黏性修正"), QStringLiteral("select"), QString::fromUtf8("二维极曲线映射"), QString(), {}},
                {QString::fromUtf8("机身升力修正"), QStringLiteral("number"), QStringLiteral("1.20"), QString::fromUtf8("系数"), {}},
                {QString::fromUtf8("配平阻力修正"), QStringLiteral("number"), QStringLiteral("1.02"), QString::fromUtf8("系数"), {}},
                {QString::fromUtf8("收敛容差"), QStringLiteral("number"), QStringLiteral("1e-5"), QString::fromUtf8("残差"), {}},
                {QString::fromUtf8("最大迭代步"), QStringLiteral("number"), QStringLiteral("300"), QString::fromUtf8("步"), {}}
            }}
        },
        {{QString::fromUtf8("主方法"), QStringLiteral("VLM")}, {QString::fromUtf8("工况点"), QStringLiteral("10 × 7")},
         {QString::fromUtf8("离散规模"), QString::fromUtf8("约 8.6k 面元")}, {QString::fromUtf8("输出"), QString::fromUtf8("气动力 / 分布 / 导数")}},
        {QString::fromUtf8("几何封闭性与控制面定义"), QString::fromUtf8("参考面积、弦长、力矩中心"),
         QString::fromUtf8("工况覆盖设计包线"), QString::fromUtf8("高升力与推进干扰模型")}
    });

    d.append({
        QStringLiteral("structure"), QString::fromUtf8("结构"),
        QString::fromUtf8("结构分析设置"), QString::fromUtf8("理想化模型、材料、载荷映射、网格与边界条件"),
        {QString::fromUtf8("概念设计｜梁壳混合模型"), QString::fromUtf8("机翼｜壳单元静强度"), QString::fromUtf8("气动弹性｜模态 + 定常载荷")},
        {
            {QString::fromUtf8("分析类型与求解器"), {
                {QString::fromUtf8("分析类型"), QStringLiteral("select"), QString::fromUtf8("线性静力 + 模态"), QString(), {}},
                {QString::fromUtf8("求解器"), QStringLiteral("select"), QStringLiteral("Nastran / CalculiX"), QString(), {}},
                {QString::fromUtf8("模型保真度"), QStringLiteral("select"), QString::fromUtf8("梁-壳混合"), QString(), {}},
                {QString::fromUtf8("单位制"), QStringLiteral("select"), QString::fromUtf8("SI（N, kg, m）"), QString(), {}},
                {QString::fromUtf8("几何非线性"), QStringLiteral("check"), QString::fromUtf8("关闭"), QString(), {}},
                {QString::fromUtf8("材料非线性"), QStringLiteral("check"), QString::fromUtf8("关闭"), QString(), {}}
            }},
            {QString::fromUtf8("材料与属性"), {
                {QString::fromUtf8("材料库"), QStringLiteral("select"), QString::fromUtf8("航空铝 + CFRP"), QString(), {}},
                {QString::fromUtf8("铺层方案"), QStringLiteral("select"), QString::fromUtf8("准各向同性基线"), QString(), {}},
                {QString::fromUtf8("梁截面"), QStringLiteral("select"), QString::fromUtf8("参数化翼盒"), QString(), {}},
                {QString::fromUtf8("厚度来源"), QStringLiteral("select"), QString::fromUtf8("结构尺寸变量"), QString(), {}},
                {QString::fromUtf8("安全系数"), QStringLiteral("number"), QStringLiteral("1.50"), QString(), {}},
                {QString::fromUtf8("许用准则"), QStringLiteral("select"), QStringLiteral("屈服 / Tsai-Wu"), QString(), {}}
            }},
            {QString::fromUtf8("网格与连接"), {
                {QString::fromUtf8("单元类型"), QStringLiteral("select"), QStringLiteral("CQUAD4 + CBEAM"), QString(), {}},
                {QString::fromUtf8("全局尺寸"), QStringLiteral("number"), QStringLiteral("0.18"), QStringLiteral("m"), {}},
                {QString::fromUtf8("局部加密"), QStringLiteral("select"), QString::fromUtf8("翼根 / 起落架接头"), QString(), {}},
                {QString::fromUtf8("单元阶次"), QStringLiteral("select"), QString::fromUtf8("一阶"), QString(), {}},
                {QString::fromUtf8("刚性连接"), QStringLiteral("select"), QString::fromUtf8("固定点自动生成 RBE2"), QString(), {}},
                {QString::fromUtf8("接触"), QStringLiteral("select"), QString::fromUtf8("绑定接触"), QString(), {}}
            }},
            {QString::fromUtf8("载荷与边界条件"), {
                {QString::fromUtf8("载荷工况"), QStringLiteral("select"), QString::fromUtf8("2.5g 拉起 + 阵风"), QString(), {}},
                {QString::fromUtf8("气动载荷映射"), QStringLiteral("select"), QString::fromUtf8("保守插值"), QString(), {}},
                {QString::fromUtf8("约束对象"), QStringLiteral("select"), QString::fromUtf8("机翼根部固定点"), QString(), {}},
                {QString::fromUtf8("约束自由度"), QStringLiteral("select"), QStringLiteral("Tx Ty Tz Rx Ry Rz"), QString(), {}},
                {QString::fromUtf8("载荷步数"), QStringLiteral("number"), QStringLiteral("4"), QString::fromUtf8("个"), {}},
                {QString::fromUtf8("收敛容差"), QStringLiteral("number"), QStringLiteral("1e-6"), QString::fromUtf8("能量"), {}}
            }}
        },
        {{QString::fromUtf8("主方法"), QString::fromUtf8("梁壳有限元")}, {QString::fromUtf8("网格规模"), QString::fromUtf8("约 42k 单元")},
         {QString::fromUtf8("工况"), QString::fromUtf8("4 个载荷步")}, {QString::fromUtf8("输出"), QString::fromUtf8("应力 / 变形 / 模态 / 裕度")}},
        {QString::fromUtf8("材料与厚度分区完整"), QString::fromUtf8("连接、固定点和自由度明确"),
         QString::fromUtf8("气动网格与结构网格坐标一致"), QString::fromUtf8("载荷包线包含极限与终极载荷")}
    });

    d.append({
        QStringLiteral("mass"), QString::fromUtf8("重量与质量特性"),
        QString::fromUtf8("重量与质量特性分析设置"), QString::fromUtf8("估算方法、构型状态、装载情景与不确定性"),
        {QString::fromUtf8("概念设计｜部件分解估重"), QString::fromUtf8("任务反算｜重量闭环"), QString::fromUtf8("装载包线｜重心与惯量")},
        {
            {QString::fromUtf8("估算方法"), {
                {QString::fromUtf8("重量层级"), QStringLiteral("select"), QString::fromUtf8("Class II 部件分解"), QString(), {}},
                {QString::fromUtf8("结构重量方法"), QStringLiteral("select"), QStringLiteral("Raymer / Torenbeek 融合"), QString(), {}},
                {QString::fromUtf8("推进重量来源"), QStringLiteral("select"), QString::fromUtf8("发动机数据库 + 安装修正"), QString(), {}},
                {QString::fromUtf8("系统重量方法"), QStringLiteral("select"), QString::fromUtf8("统计回归"), QString(), {}},
                {QString::fromUtf8("CAD 质量覆盖"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}},
                {QString::fromUtf8("历史校准系数"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}}
            }},
            {QString::fromUtf8("基准与构型状态"), {
                {QString::fromUtf8("坐标原点"), QStringLiteral("select"), QString::fromUtf8("机鼻基准面"), QString(), {}},
                {QString::fromUtf8("轴系定义"), QStringLiteral("select"), QString::fromUtf8("机体系 X前 Y右 Z下"), QString(), {}},
                {QString::fromUtf8("构型状态"), QStringLiteral("select"), QString::fromUtf8("巡航 / 起飞 / 着陆"), QString(), {}},
                {QString::fromUtf8("重量基准"), QStringLiteral("select"), QString::fromUtf8("最大起飞重量"), QString(), {}},
                {QString::fromUtf8("对称性"), QStringLiteral("select"), QString::fromUtf8("关于 XZ 面"), QString(), {}},
                {QString::fromUtf8("单位制"), QStringLiteral("select"), QString::fromUtf8("kg / m / kg·m²"), QString(), {}}
            }},
            {QString::fromUtf8("部件与装载"), {
                {QString::fromUtf8("部件树"), QStringLiteral("select"), QString::fromUtf8("结构 + 推进 + 设备 + 有效载荷"), QString(), {}},
                {QString::fromUtf8("燃油箱模型"), QStringLiteral("select"), QString::fromUtf8("多油箱 / 顺序供油"), QString(), {}},
                {QString::fromUtf8("乘员与载荷"), QStringLiteral("select"), QString::fromUtf8("3 种装载情景"), QString(), {}},
                {QString::fromUtf8("挂载状态"), QStringLiteral("select"), QString::fromUtf8("清洁构型"), QString(), {}},
                {QString::fromUtf8("重心范围"), QStringLiteral("text"), QString::fromUtf8("15 ～ 35"), QStringLiteral("%MAC"), {}},
                {QString::fromUtf8("惯量计算"), QStringLiteral("select"), QString::fromUtf8("部件积分 + 平行轴"), QString(), {}}
            }},
            {QString::fromUtf8("闭环与不确定性"), {
                {QString::fromUtf8("重量迭代"), QStringLiteral("select"), QString::fromUtf8("任务燃油—起飞重量闭环"), QString(), {}},
                {QString::fromUtf8("收敛容差"), QStringLiteral("number"), QStringLiteral("0.10"), QStringLiteral("%"), {}},
                {QString::fromUtf8("最大迭代步"), QStringLiteral("number"), QStringLiteral("50"), QString::fromUtf8("步"), {}},
                {QString::fromUtf8("模型裕度"), QStringLiteral("number"), QStringLiteral("8.0"), QStringLiteral("%"), {}},
                {QString::fromUtf8("输入不确定性"), QStringLiteral("select"), QString::fromUtf8("三角分布"), QString(), {}},
                {QString::fromUtf8("敏感性输出"), QStringLiteral("select"), QStringLiteral("重量 / 航程 / L/D / SFC"), QString(), {}}
            }}
        },
        {{QString::fromUtf8("估重层级"), QStringLiteral("Class II")}, {QString::fromUtf8("装载情景"), QString::fromUtf8("3 个")},
         {QString::fromUtf8("闭环变量"), QStringLiteral("MTOW / 燃油")}, {QString::fromUtf8("输出"), QString::fromUtf8("重量分解 / CG / 惯量")}},
        {QString::fromUtf8("部件质量无重复计入"), QString::fromUtf8("燃油与有效载荷状态一致"),
         QString::fromUtf8("MAC 与基准坐标已同步"), QString::fromUtf8("重量裕度按成熟度分配")}
    });

    d.append({
        QStringLiteral("propulsion"), QString::fromUtf8("推进与能源"),
        QString::fromUtf8("推进与能源分析设置"), QString::fromUtf8("动力架构、部件模型、安装效应与能量管理"),
        {QString::fromUtf8("涡扇｜发动机台架图谱"), QString::fromUtf8("涡桨｜螺旋桨匹配"), QString::fromUtf8("混动｜电池 + 电机 + 涡轮")},
        {
            {QString::fromUtf8("动力架构与模型"), {
                {QString::fromUtf8("推进类型"), QStringLiteral("select"), QString::fromUtf8("涡扇发动机"), QString(), {}},
                {QString::fromUtf8("发动机数量"), QStringLiteral("number"), QStringLiteral("2"), QString::fromUtf8("台"), {}},
                {QString::fromUtf8("发动机模型"), QStringLiteral("select"), QString::fromUtf8("性能图谱 / Deck"), QString(), {}},
                {QString::fromUtf8("缩放方式"), QStringLiteral("select"), QString::fromUtf8("设计点推力缩放"), QString(), {}},
                {QString::fromUtf8("离设计点计算"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}},
                {QString::fromUtf8("部件老化裕度"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}}
            }},
            {QString::fromUtf8("设计点与工作网格"), {
                {QString::fromUtf8("设计点高度"), QStringLiteral("number"), QStringLiteral("11000"), QStringLiteral("m"), {}},
                {QString::fromUtf8("设计点马赫数"), QStringLiteral("number"), QStringLiteral("0.78"), QStringLiteral("Ma"), {}},
                {QString::fromUtf8("额定净推力"), QStringLiteral("number"), QStringLiteral("118"), QStringLiteral("kN"), {}},
                {QString::fromUtf8("温度偏差"), QStringLiteral("number"), QStringLiteral("0"), QStringLiteral("K"), {}},
                {QString::fromUtf8("油门范围"), QStringLiteral("text"), QString::fromUtf8("0.2 ～ 1.0"), QString(), {}},
                {QString::fromUtf8("工作网格"), QStringLiteral("text"), QString::fromUtf8("12 高度 × 9 马赫"), QString(), {}}
            }},
            {QString::fromUtf8("进排气与推进器"), {
                {QString::fromUtf8("进气道模型"), QStringLiteral("select"), QString::fromUtf8("总压恢复曲线"), QString(), {}},
                {QString::fromUtf8("喷管模型"), QStringLiteral("select"), QString::fromUtf8("收敛喷管 / 自动临界"), QString(), {}},
                {QString::fromUtf8("短舱阻力耦合"), QStringLiteral("select"), QString::fromUtf8("气动模块传入"), QString(), {}},
                {QString::fromUtf8("安装损失"), QStringLiteral("number"), QStringLiteral("3.0"), QStringLiteral("%"), {}},
                {QString::fromUtf8("推进器模型"), QStringLiteral("select"), QString::fromUtf8("无（涡扇）"), QString(), {}},
                {QString::fromUtf8("滑流/尾流输出"), QStringLiteral("select"), QString::fromUtf8("传递至气动模块"), QString(), {}}
            }},
            {QString::fromUtf8("能源与约束"), {
                {QString::fromUtf8("燃料类型"), QStringLiteral("select"), QStringLiteral("Jet-A / LHV 数据库"), QString(), {}},
                {QString::fromUtf8("比油耗模型"), QStringLiteral("select"), QString::fromUtf8("图谱插值"), QString(), {}},
                {QString::fromUtf8("电池模型"), QStringLiteral("select"), QString::fromUtf8("未启用"), QString(), {}},
                {QString::fromUtf8("热限制"), QStringLiteral("select"), QStringLiteral("T4 / EGT 限制"), QString(), {}},
                {QString::fromUtf8("能量管理"), QStringLiteral("select"), QString::fromUtf8("按任务段调度"), QString(), {}},
                {QString::fromUtf8("收敛容差"), QStringLiteral("number"), QStringLiteral("1e-4"), QString::fromUtf8("功率平衡"), {}}
            }}
        },
        {{QString::fromUtf8("架构"), QString::fromUtf8("双发涡扇")}, {QString::fromUtf8("工作网格"), QString::fromUtf8("108 点")},
         {QString::fromUtf8("耦合"), QString::fromUtf8("气动 / 任务")}, {QString::fromUtf8("输出"), QString::fromUtf8("推力 / 流量 / SFC / 裕度")}},
        {QString::fromUtf8("发动机图谱覆盖全部飞行点"), QString::fromUtf8("推力定义与坐标方向一致"),
         QString::fromUtf8("安装损失未与气动重复计算"), QString::fromUtf8("燃油热值和储备策略一致")}
    });

    d.append({
        QStringLiteral("dynamics"), QString::fromUtf8("操稳与飞行动力学"),
        QString::fromUtf8("操稳与飞行动力学分析设置"), QString::fromUtf8("导数来源、配平、线化、控制系统与时域仿真"),
        {QString::fromUtf8("稳定性｜导数与模态"), QString::fromUtf8("操纵性｜配平与操纵力"), QString::fromUtf8("六自由度｜时域响应")},
        {
            {QString::fromUtf8("分析任务与模型"), {
                {QString::fromUtf8("分析任务"), QStringLiteral("select"), QString::fromUtf8("配平 + 线化模态"), QString(), {}},
                {QString::fromUtf8("运动模型"), QStringLiteral("select"), QString::fromUtf8("六自由度刚体"), QString(), {}},
                {QString::fromUtf8("气动导数来源"), QStringLiteral("select"), QString::fromUtf8("VLM + 半经验修正"), QString(), {}},
                {QString::fromUtf8("推进导数"), QStringLiteral("select"), QString::fromUtf8("推进模块线化"), QString(), {}},
                {QString::fromUtf8("地面效应"), QStringLiteral("check"), QString::fromUtf8("关闭"), QString(), {}},
                {QString::fromUtf8("动力学耦合"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}}
            }},
            {QString::fromUtf8("配平条件"), {
                {QString::fromUtf8("飞行状态"), QStringLiteral("select"), QString::fromUtf8("定高平飞"), QString(), {}},
                {QString::fromUtf8("高度"), QStringLiteral("number"), QStringLiteral("11000"), QStringLiteral("m"), {}},
                {QString::fromUtf8("速度"), QStringLiteral("number"), QStringLiteral("230"), QStringLiteral("m/s"), {}},
                {QString::fromUtf8("质量状态"), QStringLiteral("select"), QString::fromUtf8("巡航中段"), QString(), {}},
                {QString::fromUtf8("目标量"), QStringLiteral("select"), QStringLiteral("γ=0, Nz=1, β=0"), QString(), {}},
                {QString::fromUtf8("自由配平量"), QStringLiteral("select"), QString::fromUtf8("α, δe, 油门"), QString(), {}}
            }},
            {QString::fromUtf8("控制与扰动"), {
                {QString::fromUtf8("控制面模型"), QStringLiteral("select"), QString::fromUtf8("升降舵 / 副翼 / 方向舵"), QString(), {}},
                {QString::fromUtf8("执行机构"), QStringLiteral("select"), QString::fromUtf8("二阶 + 速率限制"), QString(), {}},
                {QString::fromUtf8("控制律"), QStringLiteral("select"), QString::fromUtf8("开环基线"), QString(), {}},
                {QString::fromUtf8("阵风模型"), QStringLiteral("select"), QString::fromUtf8("1-cos 离散阵风"), QString(), {}},
                {QString::fromUtf8("初始扰动"), QStringLiteral("select"), QString::fromUtf8("俯仰 2°"), QString(), {}},
                {QString::fromUtf8("控制分配"), QStringLiteral("select"), QString::fromUtf8("不启用"), QString(), {}}
            }},
            {QString::fromUtf8("线化与积分"), {
                {QString::fromUtf8("线化方法"), QStringLiteral("select"), QString::fromUtf8("数值中心差分"), QString(), {}},
                {QString::fromUtf8("导数步长"), QStringLiteral("number"), QStringLiteral("0.5"), QStringLiteral("%"), {}},
                {QString::fromUtf8("积分器"), QStringLiteral("select"), QStringLiteral("RK45 自适应"), QString(), {}},
                {QString::fromUtf8("仿真时长"), QStringLiteral("number"), QStringLiteral("60"), QStringLiteral("s"), {}},
                {QString::fromUtf8("最大步长"), QStringLiteral("number"), QStringLiteral("0.02"), QStringLiteral("s"), {}},
                {QString::fromUtf8("相对容差"), QStringLiteral("number"), QStringLiteral("1e-6"), QString(), {}}
            }}
        },
        {{QString::fromUtf8("任务"), QString::fromUtf8("配平 + 线化")}, {QString::fromUtf8("自由度"), QStringLiteral("6 DOF")},
         {QString::fromUtf8("配平变量"), QString::fromUtf8("3 个")}, {QString::fromUtf8("输出"), QString::fromUtf8("导数 / 模态 / 响应 / 品质")}},
        {QString::fromUtf8("质量、重心与惯量状态匹配"), QString::fromUtf8("控制面符号和铰链轴一致"),
         QString::fromUtf8("导数覆盖当前迎角与马赫数"), QString::fromUtf8("配平未知量与约束数量相等")}
    });

    d.append({
        QStringLiteral("mission"), QString::fromUtf8("任务与性能"),
        QString::fromUtf8("任务与性能分析设置"), QString::fromUtf8("任务段、性能模型、机场环境与约束闭环"),
        {QString::fromUtf8("运输机｜典型设计任务"), QString::fromUtf8("性能包线｜点性能扫描"), QString::fromUtf8("备降与储备｜法规任务")},
        {
            {QString::fromUtf8("任务定义"), {
                {QString::fromUtf8("任务模板"), QStringLiteral("select"), QString::fromUtf8("起飞-爬升-巡航-下降-备降"), QString(), {}},
                {QString::fromUtf8("任务基准"), QStringLiteral("select"), QString::fromUtf8("设计航程 + 有效载荷"), QString(), {}},
                {QString::fromUtf8("航段数量"), QStringLiteral("number"), QStringLiteral("8"), QString::fromUtf8("段"), {}},
                {QString::fromUtf8("积分方式"), QStringLiteral("select"), QString::fromUtf8("逐段准稳态积分"), QString(), {}},
                {QString::fromUtf8("储备任务"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}},
                {QString::fromUtf8("自动连续性"), QStringLiteral("check"), QString::fromUtf8("启用"), QString(), {}}
            }},
            {QString::fromUtf8("航段与控制量"), {
                {QString::fromUtf8("当前航段"), QStringLiteral("select"), QString::fromUtf8("03 巡航"), QString(), {}},
                {QString::fromUtf8("终止条件"), QStringLiteral("select"), QString::fromUtf8("达到航程"), QString(), {}},
                {QString::fromUtf8("高度"), QStringLiteral("number"), QStringLiteral("11000"), QStringLiteral("m"), {}},
                {QString::fromUtf8("马赫数"), QStringLiteral("number"), QStringLiteral("0.78"), QStringLiteral("Ma"), {}},
                {QString::fromUtf8("控制策略"), QStringLiteral("select"), QString::fromUtf8("定高度 / 定马赫数"), QString(), {}},
                {QString::fromUtf8("升阻比来源"), QStringLiteral("select"), QString::fromUtf8("气动模块实时插值"), QString(), {}}
            }},
            {QString::fromUtf8("机场与性能约束"), {
                {QString::fromUtf8("出发机场高度"), QStringLiteral("number"), QStringLiteral("0"), QStringLiteral("m"), {}},
                {QString::fromUtf8("跑道长度"), QStringLiteral("number"), QStringLiteral("2800"), QStringLiteral("m"), {}},
                {QString::fromUtf8("大气条件"), QStringLiteral("select"), QStringLiteral("ISA + 15°C"), QString(), {}},
                {QString::fromUtf8("起飞场长限制"), QStringLiteral("number"), QStringLiteral("2500"), QStringLiteral("m"), {}},
                {QString::fromUtf8("着陆场长限制"), QStringLiteral("number"), QStringLiteral("1800"), QStringLiteral("m"), {}},
                {QString::fromUtf8("爬升梯度"), QStringLiteral("number"), QStringLiteral("2.4"), QStringLiteral("%"), {}}
            }},
            {QString::fromUtf8("闭环与求解控制"), {
                {QString::fromUtf8("闭环变量"), QStringLiteral("select"), QString::fromUtf8("起飞重量 / 任务燃油"), QString(), {}},
                {QString::fromUtf8("匹配方式"), QStringLiteral("select"), QString::fromUtf8("推重比—翼载荷"), QString(), {}},
                {QString::fromUtf8("时间步长"), QStringLiteral("number"), QStringLiteral("10"), QStringLiteral("s"), {}},
                {QString::fromUtf8("收敛容差"), QStringLiteral("number"), QStringLiteral("0.05"), QStringLiteral("%"), {}},
                {QString::fromUtf8("最大迭代步"), QStringLiteral("number"), QStringLiteral("80"), QString::fromUtf8("步"), {}},
                {QString::fromUtf8("失效处理"), QStringLiteral("select"), QString::fromUtf8("降阶模型并记录告警"), QString(), {}}
            }}
        },
        {{QString::fromUtf8("任务段"), QString::fromUtf8("8 段")}, {QString::fromUtf8("闭环"), QStringLiteral("MTOW / 燃油")},
         {QString::fromUtf8("约束"), QString::fromUtf8("6 项")}, {QString::fromUtf8("输出"), QString::fromUtf8("航程 / 油耗 / 时间 / 包线")}},
        {QString::fromUtf8("航段初末状态连续"), QString::fromUtf8("储备燃油规则已明确"),
         QString::fromUtf8("气动和推进数据覆盖航迹"), QString::fromUtf8("性能约束与适航类别一致")}
    });
    return d;
}

AnalysisDocument AnalysisCatalogs::seedDocument()
{
    AnalysisDocument doc;
    doc.schemaVersion = QStringLiteral("amdo.analysis.v1");
    doc.id = QStringLiteral("analysis-draft");
    doc.title = QString::fromUtf8("学科分析集");
    doc.status = QStringLiteral("draft");

    const QVector<AnalysisDomain> domainList = domains();
    for (int di = 0; di < domainList.size(); ++di) {
        const AnalysisDomain &dm = domainList[di];
        for (int si = 0; si < dm.sections.size(); ++si) {
            const AnalysisSection &sec = dm.sections[si];
            for (int fi = 0; fi < sec.fields.size(); ++fi) {
                const AnalysisField &f = sec.fields[fi];
                doc.values.insert(analysisFieldKey(dm.id, sec.title, f.label), f.defaultValue);
            }
        }
    }
    doc.loadedKnown = true;
    return doc;
}
