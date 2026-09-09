# UI 与内部接口

- Widgets + **代码布局**（`QLayout`）+ `objectName` + `Theme` QSS。强调色 `#0c9b88`。
- `mainwindow.ui` **未使用**。无 HTTP/对外 SDK。无独立业务 Dialog（仅 `QMessageBox`）。
- View 层约束见 [frontend_constraints.md](frontend_constraints.md)。

## MainWindow

顶栏（品牌/项目/主次按钮）→ 六大 `ModeButton` → `QStackedWidget`（六 Page）。  
`switchMode(int)` 切页并 `updateActions(int)` 改按钮文案（均为 private）。主次按钮 → `wireDummyAction`。  
成员：`m_pages`、`m_primary`、`m_secondary`、`m_modeButtons`。

## 子页

多数 Page：`makeHeading` + `SubTabBar`（`currentChanged`）+ 内层 `QStackedWidget` + `wrapScroll`。  
除 MVP 备注外，六个 Page 构造时组装静态 UI；无对外业务 signal。

| Page | 子页 id |
|------|---------|
| Requirements | mission / envelope / standards / metrics；页顶嵌入 `ProjectNotePanel` |
| Definition | semantic / configuration / geometry / visualization |
| Analysis | 无 SubTab；左侧学科切换 |
| Design | variables / exploration / optimization / mdo |
| Decision | single / compare / report |
| Workflow | definition / execution / monitor |

## MVP：方案备注

| 层 | 类型 | 要点 |
|----|------|------|
| View | `ProjectNotePanel` | 信号 `loadRequested` / `saveRequested`；槽 `setNote` / `setBusy` / `setStatus` / `showError` |
| Presenter | `ProjectNotePresenter` | 连接 View 信号；调用 Service；只刷新备注区 |
| Service | `ProjectNoteService` | `loadNote` / `saveNote`（`bool` + `QString* errorMessage`） |
| 数据 | `ProjectNote` + `ProjectNoteStore` | POD；读写 `%AppData%/AMDO/飞机概念设计平台/mvp_project_note.txt` |

组装：`RequirementsPage` 持有 Store/Service（`unique_ptr`）与 Presenter（`QObject` 子对象），**View 不直接调 Service**。

## uihelpers

`makeButton` / `makeField` / `makeSelectField` / `makePanel` / `makeKpis` / `makeTable` / `makeHeading` / `wrapScroll` / `wireDummyAction`

结构体：`KpiItem`、`SummaryItem`、`TableOptions`。  
`SubTabBar`：`currentChanged(QString id)`。`CandidateRow` 展示行。

连接以函数指针 + lambda 为主。

## Theme / chartwidgets

`Theme::styleSheet()` 与颜色辅助。图表类：`paintEvent` 示意绘制；`MissionRail(QStringList)`。

## Analysis 内部（未导出）

`analysispage.cpp` 内 `DomainDef` / `FieldDef` / `SectionDef` — 仅生成表单，非跨模块 API。
