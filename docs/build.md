# 构建

## 已验证环境

Windows · Qt **5.15.2** mingw81_64 · g++ **8.1.0** · CMake **3.30.5** · Generator **MinGW Makefiles** · Debug · 产物 `build-mingw64/AMDO.exe`

路径因机器而异。Kit 与 Generator 勿混用；Debug/Release 勿混链 Qt。

## 命令

```powershell
$env:Path = "C:\Qt\Tools\mingw810_64\bin;C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\CMake_64\bin;" + $env:Path

cmake -S . -B build-mingw64 -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64"
cmake --build build-mingw64 --target AMDO
```

Release：换目录 `build-mingw64-release` 与 `-DCMAKE_BUILD_TYPE=Release`（Release 全链路 **待确认**）。

清理：`Remove-Item -Recurse -Force build-mingw64` 或 `cmake --build build-mingw64 --target clean`。

运行：PATH 含 Qt `bin` 后执行 `.\build-mingw64\AMDO.exe`。**约定：Build 通过后先 `Stop-Process -Name AMDO -Force -ErrorAction SilentlyContinue` 清理旧实例，再用 `Start-Process ".\build-mingw64\AMDO.exe"` 后台启动，免手动开 Qt；仅在 Build 成功时运行，且不阻塞会话。**

新增源文件 → 写入 `PROJECT_SOURCES` → 重新 build（必要时重新 cmake）。

设计需求 SRD 落盘路径（Windows 典型）：`%AppData%/AMDO/飞机概念设计平台/srd/`（`srd_draft.json` 与 `baselines/`）。

## 依赖：yaml-cpp（评价规格 `evaluation-spec.yaml`）

`SrdImportExportService::exportEvaluationSpec` 用 `yaml-cpp` 0.8.0 序列化。`CMakeLists.txt` 按**本地优先、联网兜底**顺序获取，克隆本仓库的机器即使没有 yaml-cpp 也能编译：

1. **本地已安装** — `find_package(yaml-cpp 0.8)`：命中 vcpkg / 系统包 / 手动 `install` 的 yaml-cpp，**可离线**。
2. **仓库内 vendored** — 若存在 `third_party/yaml-cpp/CMakeLists.txt` 则 `add_subdirectory`，**完全离线**（仓库自带源码时用）。
3. **联网 FetchContent** — 从 `https://github.com/jbeder/yaml-cpp.git`（tag `0.8.0`）拉取到 `build-mingw64/_deps/`，**首次配置需可访问 GitHub**；成功后缓存复用，之后可离线 build。

三种来源统一别名为 `yaml-cpp::yaml-cpp` 供 `AMDO` 链接。FetchContent 路径为静态库（`libyaml-cppd.a`），无额外运行时 DLL；若走本地共享库安装，注意运行时 DLL。

### 别人 clone 后编译不过（yaml-cpp 相关）——Agent 自动修复顺序

当配置/链接报 yaml-cpp 找不到（如 `Could NOT find yaml-cpp` 或 FetchContent 因无网失败），按下列**从省事到兜底**依次尝试，成功一步即可：

```powershell
# 前置：PATH 已含 Qt/MinGW/CMake（见上「命令」）

# ① 有网 → 直接重配，让 FetchContent 拉取（最常见）
cmake -S . -B build-mingw64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64"

# ② 有网但想用本地包管理器安装后走 find_package（可离线复用）
#    vcpkg 示例：
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg   # 若未装
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg.exe install yaml-cpp:x64-mingw-static
cmake -S . -B build-mingw64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64" `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# ③ 无网 → 用离线源码目录（把 yaml-cpp 源码放到某路径），让 FetchContent 指向本地：
#    -DFETCHCONTENT_SOURCE_DIR_YAML-CPP=<yaml-cpp 源码目录>
cmake -S . -B build-mingw64 -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_64" `
  "-DFETCHCONTENT_SOURCE_DIR_YAML-CPP=C:/src/yaml-cpp"

# ④ 无网且要长期离线 → 把 yaml-cpp 源码放进仓库 third_party/yaml-cpp（含其 CMakeLists.txt），
#    CMake 会自动 add_subdirectory，无需任何命令行参数。

# 任一成功后：
cmake --build build-mingw64 --target AMDO
```

> 校验成功：配置日志出现 `Built target yaml-cpp`（FetchContent/vendored）或 `find_package` 命中；最终 `Linking CXX executable AMDO.exe` + `Built target AMDO`。

## 排查

| 问题 | 处理 |
|------|------|
| 找不到 Qt | 检查 `CMAKE_PREFIX_PATH` |
| 找不到 / 拉不到 yaml-cpp | 见「依赖：yaml-cpp」小节的自动修复顺序（有网重配 / vcpkg / 离线源码目录 / vendored） |
| 改了文件无效果 | 是否写入 `PROJECT_SOURCES` |
| 改 `.ui` 无效 | 未入构建；改 `mainwindow.cpp` |
| moc/链接异常 | `Q_OBJECT` + 源文件已入目标 |
| 缺 DLL | PATH 加 Qt bin / windeployqt（**待确认**） |
| 样式不对 | `objectName` ↔ `Theme`；动态改名后 polish |
| 中文乱码 | UTF-8 源文件 + MinGW UTF-8 编译选项 |
| 按钮「没功能」 | 原型预期：`QMessageBox` |
| 子线程改 UI 崩溃/警告 | 改用 `Qt::QueuedConnection` / 信号回 UI 线程 |
