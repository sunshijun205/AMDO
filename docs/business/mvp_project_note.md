# 业务：方案备注（mvp_project_note）

分层样板业务：在「设计需求」页编辑并持久化一段方案备注文本。  
对应工程约束中的 View → Presenter → Service → 本地数据闭环（见 `docs/frontend_constraints.md` §10）。

## 1. 业务说明

| 项 | 内容 |
|----|------|
| 目标 | 用户可加载 / 编辑 / 保存方案备注；内容写入本机文件，非界面硬编码伪结果 |
| 用户入口 | 主导航「设计需求」→ 页顶面板「方案备注（MVP 示例）」 |
| 主操作 | **重新加载**：从磁盘读入编辑框；**保存备注**：把当前文本落盘 |
| 存储 | `%AppData%/AMDO/飞机概念设计平台/mvp_project_note.txt`（`AppDataLocation`；失败时回退可执行目录） |
| 错误 | 失败不静默：状态行提示 + `QMessageBox`；按钮/编辑区经 `setBusy` 恢复可操作 |
| 非目标 | 不涉及六大导航改动、多用户同步、网络、加密 |

## 2. 调用链

```text
RequirementsPage（组装）
  └─ ProjectNotePanel          View：信号 / setXxx
        ↕
  ProjectNotePresenter         协调加载与保存
        ↓
  ProjectNoteService           用例
        ↓
  ProjectNoteStore + ProjectNote   文件 IO + POD
```

## 3. 相关类型与文件

| 类型 | 层 | 路径 | 职责 |
|------|----|------|------|
| `ProjectNote` | 数据 POD | `model/projectnote.h` | 备注文本与 `textKnown` |
| `ProjectNoteStore` | 本地数据 | `model/projectnotestore.h` `.cpp` | 解析路径；读/写 UTF-8 文件 |
| `ProjectNoteService` | Service | `service/projectnoteservice.h` `.cpp` | `loadNote` / `saveNote`；包装错误语境 |
| `ProjectNotePresenter` | Presenter | `controller/projectnotepresenter.h` `.cpp` | 接 View 信号；调 Service；刷新 View |
| `ProjectNotePanel` | View | `ui/projectnotepanel.h` `.cpp` | 编辑框、按钮、状态；不调 Service |
| `RequirementsPage` | 宿主 View | `ui/requirementspage.h` `.cpp` | 创建并持有 Panel / Store / Service / Presenter |

CMake：上述源文件均在根目录 `CMakeLists.txt` 的 `PROJECT_SOURCES` 中。

## 4. 关键 API

### `ProjectNote`（`model/projectnote.h`）

| 字段 | 含义 |
|------|------|
| `QString text` | 备注正文 |
| `bool textKnown` | 是否已拿到有效读结果；未知时 Presenter 不覆盖编辑框 |

### `ProjectNoteStore`

| 函数 | 说明 |
|------|------|
| `QString filePath() const` | 备注文件绝对路径 |
| `bool load(ProjectNote *out, QString *errorMessage = nullptr) const` | 文件不存在视为空备注成功；打开失败返回 false |
| `bool save(const ProjectNote &note, QString *errorMessage = nullptr) const` | 覆盖写入 UTF-8；失败填 `errorMessage` |

### `ProjectNoteService`

| 函数 | 说明 |
|------|------|
| `explicit ProjectNoteService(ProjectNoteStore *store)` | 不拥有 store |
| `bool loadNote(ProjectNote *out, QString *errorMessage = nullptr)` | 委托 Store；错误前缀「加载备注失败：」 |
| `bool saveNote(const QString &text, QString *errorMessage = nullptr)` | 组 `ProjectNote` 后保存；错误前缀「保存备注失败：」 |
| `QString storagePath() const` | 转发 `store->filePath()`，供状态行展示 |

### `ProjectNotePresenter`

| 函数 | 说明 |
|------|------|
| 构造函数 `(Panel*, Service*, parent)` | 连接信号；立即 `onLoadRequested()` |
| `void onLoadRequested()` | busy → load → `setNote`（仅 `textKnown`）→ 状态 / 错误恢复 |
| `void onSaveRequested(const QString &text)` | busy → save → 状态 / 错误恢复 |

### `ProjectNotePanel`

| 成员 | 说明 |
|------|------|
| 信号 `loadRequested()` | 「重新加载」 |
| 信号 `saveRequested(const QString &text)` | 「保存备注」，参数为编辑框全文 |
| 槽 `setNote` / `setBusy` / `setStatus` / `showError` | 仅由 Presenter 驱动的展示更新 |

### `RequirementsPage`（组装，非业务算法）

构造中：`new ProjectNotePanel` → `ProjectNoteStore` / `ProjectNoteService`（`unique_ptr`）→ `ProjectNotePresenter(..., this)`。  
View 不直接调用 Service。

## 5. 改动注意

- 扩展字段：改 `ProjectNote` + Store 序列化，并保持 `textKnown` 语义与最小刷新。  
- 勿在 `ProjectNotePanel` / `RequirementsPage` 内直接读写文件。  
- 工程分层与 Checklist：`docs/frontend_constraints.md`；总览：`docs/architecture.md` / `docs/ui.md`。
