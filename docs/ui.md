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
六个 Page 均仅 `explicit XxxPage(QWidget* parent = nullptr)`；无对外 signal。

| Page | 子页 id |
|------|---------|
| Requirements | mission / envelope / standards / metrics |
| Definition | semantic / configuration / geometry / visualization |
| Analysis | 无 SubTab；左侧学科切换 |
| Design | variables / exploration / optimization / mdo |
| Decision | single / compare / report |
| Workflow | definition / execution / monitor |

## uihelpers

`makeButton` / `makeField` / `makeSelectField` / `makePanel` / `makeKpis` / `makeTable` / `makeHeading` / `wrapScroll` / `wireDummyAction`

结构体：`KpiItem`、`SummaryItem`、`TableOptions`。  
`SubTabBar`：`currentChanged(QString id)`。`CandidateRow` 展示行。

连接以函数指针 + lambda 为主。

## Theme / chartwidgets

`Theme::styleSheet()` 与颜色辅助。图表类：`paintEvent` 示意绘制；`MissionRail(QStringList)`。

## Analysis 内部（未导出）

`analysispage.cpp` 内 `DomainDef` / `FieldDef` / `SectionDef` — 仅生成表单，非跨模块 API。
