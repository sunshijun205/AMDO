# AMDO 文档入口

飞机概念设计平台（Qt Widgets）知识库。Agent 先读 [`../AGENTS.md`](../AGENTS.md)。

**现状**：六大功能 UI 原型为主；**方案备注**已按 View→Presenter→Service→本地文件落地（见设计需求页）。其余按钮多为 `wireDummyAction`。无网络。

| 文档 | 用途 |
|------|------|
| [frontend_constraints.md](frontend_constraints.md) | **前端约束（MUST）** — 分层/线程/风格/Checklist；含 MVP 流程 |
| [architecture.md](architecture.md) | 现状结构、目录、模块、数据流 |
| [ui.md](ui.md) | 主窗口、子页、内部接口、MVP 接口 |
| [build.md](build.md) | 编译、运行、排查 |

阅读顺序：`AGENTS.md` → `frontend_constraints.md`（含 §10 MVP）→ `architecture.md` / `ui.md` → `build.md`。
