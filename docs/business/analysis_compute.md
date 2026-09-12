# 业务：学科运行计算（analysis_compute）

在「学科分析」页点「运行学科计算」，按关联的飞机修订与分析集工况，逐学科产出结果。
分层：View → Presenter → Service(计算) → Model(结果 POD)。当前为**低保真原型 + MOCK 占位**。

> ⚠️ 现状：仅「气动」做了真实/估算实现；结构/重量/推进/操稳/任务性能为 **MOCK 占位**，仅打通链路。
> 所有 MOCK 项在数据与界面均显式标注 `fidelity=MOCK`，**不是真实结果**，后期须逐项替换。

## 1. 业务说明

| 项 | 内容 |
|----|------|
| 入口 | 「学科分析」页顶栏「运行学科计算」按钮 |
| 输入 | 飞机参数：`sourceRevision`(R00N)→ `aircraft_vN.json`；**工况：优先取关联 SRD `sourceSrd`(srd_vN) 的飞行包线点（`sourceCondition` 选定），未关联时回退分析集手填高度/马赫** |
| 输出 | `analysis/results/` 下按运行关联命名：有用例→`case_N.evaluation.json`（贴 PDF）；无用例→`analysis_<修订_需求_cond工况>.json`。同一关联重复运行覆盖同名，不同关联互不覆盖。另有结果对话框。 |
| 保真度 | `精确`(ISA/AR) / `低保真估算`(L/D，含假设系数) / `MOCK`(占位) |
| 非目标 | 真正的求解器、几何网格(OCCT/TiGL)、CFD/FEM、优化闭环 |

## 2. 调用链

```text
AnalysisPage（运行学科计算）
      ↓ runRequested
AnalysisPresenter::onRunRequested   （编辑态先落盘草稿）
      ↓
AnalysisComputeService::run(analysis)
      ├─ resolveInputs：
      │    · sourceRevision → aircraft_vN 参数
      │    · 工况：sourceSrd + sourceCondition → srd_vN 飞行包线点(mach/altitudeKm)；
      │            未关联则回退分析集手填高度/马赫
      └─ 逐学科 IDisciplineComputer::compute(inputs)
      ↓  AnalysisRunResult
AnalysisPresenter → view.showRunResult()（对话框） + saveResult()（JSON）
```

## 2.1 工况来源（对齐 PDF：设计工况来自设计需求）

- 「学科分析」版本条新增「设计工况」下拉：随「关联设计需求」选择，列出该 SRD 基线的**飞行包线点**（`Ma / km`）。
- 选定后，计算的高度/马赫取自该 SRD 包线点（`SrdEnvelopePoint`），大气模型参考 `SrdEnvironment`。
- 未选（“用分析集工况”）时，回退到分析集 `气动·飞行与流动条件` 手填的高度/马赫。
- 气动结果中「大气温度」项的 note 会标明工况来源（SRD 包线点 或 分析集手填）。

## 3. 相关类型与文件

| 类型 | 层 | 路径 | 职责 |
|------|----|------|------|
| `AnalysisResultItem` / `AnalysisDisciplineResult` / `AnalysisRunResult` | POD | `model/analysisresult.h` | 结果项与保真度 |
| `IDisciplineComputer` | 接口 | `service/analysiscomputeservice.h` | 学科计算统一接口 |
| `AeroComputer` 等六计算器 | Service | `service/analysiscomputeservice.*` | 各学科计算 |
| `AnalysisComputeService` | Service | `service/analysiscomputeservice.*` | 解析输入/编排/持久化 |

## 4. 各学科现状与 TODO

| 学科 | 现状 | 真实/估算项 | 仍 MOCK / 待细化（TODO） |
|------|------|-------------|--------------------------|
| 气动 aero | 部分真实 | ISA 大气(T/p/ρ/a)、真空速 V、动压 q、展弦比 AR、雷诺数 Re（精确）；L/D（估算，含假设 Cd0/e） | CL/CD/导数：接极曲线/涡格法 |
| 推进 propulsion | 部分真实 | 安装后总推力=n×额定净推力×(1−安装损失)（来自配置） | SFC/流量/裕度：需发动机图谱 |
| 任务性能 mission | 部分真实 | 起飞/着陆场长余量=可用跑道−限制（精确算术） | 航程/燃油/实际场长：需 SFC/重量/L-D 闭环 |
| 结构 structure | 全 MOCK | — | 梁壳 FEM、载荷映射、模态、裕度(需几何网格/求解器) |
| 重量 mass | 全 MOCK | — | Class-II 统计估重、重量闭环、CG/惯量(需 MTOW/部件质量) |
| 操稳 dynamics | 全 MOCK | — | 配平、线化导数(需 VLM/气动导数) |

## 5. 关键规则

- L/D 估算写死了假设系数 `Cd0=0.020`、`e=0.80`（见 `analysiscomputeservice.cpp` 顶部常量）；TODO：改为从气动配置/极曲线求解。
- 设计工况优先取关联 SRD(`sourceSrd`)的飞行包线点(`sourceCondition` 选定)；未关联时回退分析集手填高度/马赫。
- TODO：进一步支持 `SrdFlightCondition`(相/高度/速度字符串)与载荷工况选择；大气模型深度对接 `SrdEnvironment`。
- 结果文件按关联命名：有 `sourceCase` → `case_N.evaluation.json`（贴 PDF）；否则用「修订+需求+工况」组合键。TODO：加运行清单 `index.json` 供 UI 列出/切换历史结果。
- MOCK 占位值仅为量级示意，**禁止**据此下工程结论。

## 6. 后续路线

1. 补输入(MTOW/SFC/任务) → 打开重量/任务的真实低保真计算(Breguet 等)。
2. 结果接入「单方案评价」：对 SRD `evaluation-spec` 判可行性/裕度。
3. 中/高保真(VLM/FEM/CFD) 需单独评估是否引入求解器/几何内核。
