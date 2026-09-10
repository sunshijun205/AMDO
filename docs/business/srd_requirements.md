# 业务：设计需求（srd_requirements）

在「设计需求」页编辑、检查并冻结一份本机 SRD（需求与评价合同）。  
分层：View → Presenter → Service → 本地 JSON。不计算航程/包线物理，不内嵌 CS-25 全文。

## 1. 业务说明

| 项 | 内容 |
|----|------|
| 目标 | 维护任务场景、飞行工况、适航适用性、评价指标；完整性检查；发布不可变基线并导出评价规格 |
| 用户入口 | 主导航「设计需求」；顶栏「导入需求 / 发布需求基线」 |
| 主操作 | 保存各章、从任务生成建议工况、按认证基础填适用性、发布适用性基线（冻结规范章）、从任务与条款建议需求、创建/发布需求基线、导入 JSON/CSV、导出 evaluation-spec |
| 存储 | `%AppData%/AMDO/飞机概念设计平台/srd/srd_draft.json`；基线 `srd/baselines/srd_vN.json` |
| 错误 | 失败不静默：页顶状态 + `QMessageBox`；`setBusy` 恢复按钮 |
| 非目标 | 任务积分、ISA 求解、完整适航文库、ReqIF、改分析/优化/决策页 |

推荐方案锁定：单文件 JSON；机场边界挂在场景上；半自动生成工况；内置 CS-25 条款种子；CS-25 默认直接适用（电推进相关条款在「电推进：不适用」时标不适用）；规范章冻结 ≠ 整份发布；指标只能从目录实例化；导入 JSON 整包或性能需求 CSV。

## 2. 调用链

```text
MainWindow（导入 / 发布）
  └─ RequirementsPage          View：信号 / setXxx / snapshotXxx
        ↕
  RequirementsPresenter        协调用例与刷新
        ↓
  SrdDocumentService           草稿/基线/章节写入
  SrdDerivationService         建议工况 / 包线点 / 需求
  SrdCompletenessService       KPI 与检查单
  SrdImportExportService       JSON/CSV / evaluation-spec
        ↓
  SrdStore + SrdCatalogs       文件 IO + 只读种子
```

## 3. 相关类型与文件

| 类型 | 层 | 路径 | 职责 |
|------|----|------|------|
| `SrdDocument` 等 | POD | `model/srdtypes.h` | 文档与 KPI/检查项 |
| `SrdCatalogs` | 数据 | `model/srdcatalogs.*` | 指标目录、条款种子、运输机种子草稿 |
| `SrdStore` | 数据 | `model/srdstore.*` | 读写 `amdo.srd.v1` JSON |
| `SrdDocumentService` | Service | `service/srddocumentservice.*` | 加载/保存/发布/增删/合并建议 |
| `SrdCompletenessService` | Service | `service/srdcompletenessservice.*` | 覆盖率与发布门槛 |
| `SrdDerivationService` | Service | `service/srdderivationservice.*` | 跨章建议（不写盘） |
| `SrdImportExportService` | Service | `service/srdimportexportservice.*` | 导入导出 |
| `RequirementsPresenter` | Presenter | `controller/requirementspresenter.*` | 接 View 信号 |
| `RequirementsPage` | View | `ui/requirementspage.*` | 四子页展示与意图；托管方案备注 MVP |

## 4. 关键规则

- 首次无草稿：写入 `SrdCatalogs::seedDocument()`，KPI 按文件内容计算。
- 强制 / 目标 / 期望 → `mandatory` / `objective` / `wish`。
- 工况覆盖率 = 主剖面航段中至少有一条阶段匹配 FC 的比例。
- 需求覆盖率 = 有指标 id、关系、限值和验证方法的 REQ / 全部 REQ。
- 发布适用性基线：规范章 `standardsFrozen=true`（不要求条款已全部映射）。
- 发布需求基线：阻塞项必须清空；成功后写 `srd_vN.json` 并尝试导出 `*-evaluation-spec.json`。
- 只读打开基线后，「另存为新草稿」复制为可编辑草稿，不改旧基线。

CSV 表头：`metricId,relation,value,unit,grade,name`。`metricId` 必须属于指标目录。

## 5. 改动注意

- View 不调用 Service；表单经 `snapshot*Chapter` 交给 Presenter。
- 规范章冻结后 `persistAll` 不再覆盖条款。
- 新增指标必须先登记 `SrdCatalogs::metrics()`。
