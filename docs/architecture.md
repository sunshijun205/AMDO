# 架构

## 现状

单进程 Qt Widgets：**main → MainWindow → 六个 Page → uihelpers / Theme / chartwidgets**。  
多数页面仍为 UI 原型（演示数据在 `*page.cpp`，操作多经 `wireDummyAction`）。

**分层样板（已落地）**：设计需求 SRD 是唯一完整的 View→Presenter→Service→本地数据 闭环——`RequirementsPage` → `RequirementsPresenter` → `SrdDocumentService` / `SrdDerivationService` / `SrdCompletenessService` / `SrdImportExportService` → `SrdStore`（`srd_draft.json` + `baselines/`）。其余功能域仍为原型。

```mermaid
graph TD
  main --> MW[MainWindow]
  MW --> Stack[QStackedWidget]
  Stack --> R[RequirementsPage]
  Stack --> D[DefinitionPage]
  Stack --> A[AnalysisPage]
  Stack --> O[DesignPage]
  Stack --> C[DecisionPage]
  Stack --> W[WorkflowPage]
  R & D & A & O & C & W --> H[uihelpers / Theme]
  R & D & O & C --> Ch[chartwidgets]
  R -.->|信号| RP[RequirementsPresenter]
  RP --> SrdDoc[SrdDocumentService]
  RP --> SrdDer[SrdDerivationService]
  RP --> SrdCmp[SrdCompletenessService]
  RP --> SrdIo[SrdImportExportService]
  SrdDoc --> SrdStore[SrdStore / JSON]
  SrdIo --> SrdStore
```

**数据流**：

- 原型页：源码字面量 → Page 构造组装 → 屏幕显示；表单可改但不写回模型。
- 设计需求 SRD：四子页编辑 → snapshot 章节 → Presenter → Service 落盘；结果经 Presenter 调用 View 的 `setXxx` 最小刷新；KPI/检查单由 Completeness 按草稿计算；发布写入不可变基线并导出 evaluation-spec。

**配置**：无通用运行时配置 / `QSettings` 业务解析。CMake 固定 AUTOMOC/UIC/RCC、`Qt::Widgets`、include=`./` 与 `./ui`、MinGW UTF-8。`main` 设置 `OrganizationName=AMDO`（供 AppData 路径）。硬编码：`Theme`、窗口尺寸、各页演示字段。UI 上的「执行环境/许可证」等仅为演示。

链接依赖仅 `Qt::Widgets`。`mainwindow.ui` 未入构建。无 `.qrc`。

## 目录

```text
AMDO/
├── AGENTS.md
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h/.cpp
├── mainwindow.ui           # 未加入 PROJECT_SOURCES
├── model/                  # 本地数据（SRD）
├── service/                # 用例（SRD）
├── controller/             # Presenter（SRD）
├── ui/                     # 页面与公共 UI
│   ├── theme.h
│   ├── uihelpers.*
│   ├── chartwidgets.*
│   └── *page.*
├── docs/
└── build-mingw64/
```

`PROJECT_SOURCES` 须与上表源文件对齐；新增文件必须写入 CMake。

## 模块

| 模块 | 位置 | 核心类型 | 职责 |
|------|------|----------|------|
| 入口 | `main.cpp` | — | QApplication / Theme / 显示主窗 |
| 壳 | `mainwindow.*` | `MainWindow` | 导航、页堆栈、主次按钮 |
| 主题 | `ui/theme.h` | `Theme` | 颜色 + 全局 QSS |
| UI 工厂 | `ui/uihelpers.*` | `SubTabBar` 等 | 面板/表格/KPI/假动作 |
| 图表 | `ui/chartwidgets.*` | `*Chart` 等 | 示意绘制 |
| 设计需求 | `ui/requirementspage.*` | `RequirementsPage` | 任务/包线/规范/指标；托管 SRD 分层 |
| SRD Presenter | `controller/requirementspresenter.*` | `RequirementsPresenter` | 编排 SRD 用例与刷新 |
| SRD Service | `service/srd*.*` | `SrdDocumentService` 等 | 草稿/派生/检查/导入导出 |
| SRD 数据 | `model/srd*` | `SrdDocument` / `SrdStore` / `SrdCatalogs` | POD + JSON + 种子目录 |
| 方案定义 | `ui/definitionpage.*` | `DefinitionPage` | 语义/构型/几何/视图 |
| 学科分析 | `ui/analysispage.*` | `AnalysisPage` | 六学科侧栏+表单 |
| 方案优化 | `ui/designpage.*` | `DesignPage` | 变量/探索/优化/MDO |
| 方案决策 | `ui/decisionpage.*` | `DecisionPage` | 评价/比较/报告 |
| 工作流 | `ui/workflowpage.*` | `WorkflowPage` | 编排/执行/监控 |

落点：顶栏/导航 → `mainwindow.cpp`；样式 → `theme.h`；某功能 → 对应 page；复用 → `uihelpers`/`chartwidgets`；新业务分层 → 对照 SRD 样板。  
改导航索引时同步 `updateActions` 文案；改 `objectName` 同步 `Theme`。

**未实现**：求解器、调度引擎、完整工程文件 IO、网络。

页面组织与内部接口见 [ui.md](ui.md)。分层约束与样板流程见 [frontend_constraints.md](frontend_constraints.md)。

## 目标分层

新业务应落在 `controller/` / `service/` / `model/`，以设计需求 SRD 为样板，禁止继续把用例堆进 Page。

## 风险

- 演示数据分散、Page 文件偏大
- 启动即构造六页（日后可懒加载，TODO）
- 闲置 `.ui` 易误导
- 仅设计需求 SRD 一条链路完成分层，其余仍为原型
- 设计需求已按 SRD 闭环落地；分析/优化/决策尚未消费 evaluation-spec
