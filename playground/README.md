# AsterCC Playground

本地编译技术展示工作台。使用真实 AsterCC Stage 3 完成编译与原生执行，直接读取逐 Pass SSA、统计和自举实验记录。没有模拟编译结果。

## 一键启动

在 **Playground worktree 根目录**执行：

```bash
bash playground/serve.sh
```

打开 **http://127.0.0.1:8000**。需要 Node.js **22.12+**（本次验证为 24.22.1）、npm、Python **3.12+** 和 `uv`。首次安装依赖需要网络，字体、Monaco Worker 和前端资源均本地打包，启动后不使用 CDN。

脚本创建 `playground/.venv`、按锁文件安装依赖、构建前端，再由 FastAPI 提供整个应用。不会构建或修改编译器，不会重新运行自举实验。

当前分支为 `codex/astercc-playground`。当前 worktree 通过 Git common directory 自动找到原工作区；只读使用其中的编译器、运行时、LLVM 以及实验制品。也可显式指定：

```bash
ASTERCC_ARTIFACT_ROOT="/mnt/d/学习资料/专业课/大三上/编译技术/实验三" \
PORT=8000 bash playground/serve.sh
```

可以用 `ASTERCC_COMPILER=/absolute/path/to/astercc` 选择其他实际编译器，例如原目录的 `build/astercc0`。默认是 `build/bootstrap/O2/stage3`，缺失时界面显示修复命令，不静默切换。`PORT` 可修改展示端口。

若把本分支检出为独立仓库，先用项目原有 `bash scripts/reproduce.sh` 生成制品；后端会优先使用当前仓库的完整构建。

## 开发启动

两个终端都在 worktree 根目录运行：

```bash
# 终端 1：独立 API，OpenAPI 位于 /docs
uv sync --project playground --frozen
uv run --project playground --frozen uvicorn playground.server.app:app \
  --host 127.0.0.1 --port 8000 --reload --reload-dir playground/server
```

```bash
# 终端 2：Vite 热更新，/api 代理到 8000
npm --prefix playground/web ci
npm --prefix playground/web run dev
```

开发地址 **http://127.0.0.1:5173**。Python 依赖仅进入本地隔离环境，不修改 system Python。

## 工作台

首页自动编译优化示例，以 O2 展示真实 SSA；不会自动执行程序。选择示例后点击「编译」或「运行」，也可按 `Ctrl/Cmd+Enter` 编译运行。

- 编辑单个 `main.aster`，可导入当前 worktree 的 `std`。支持源码文件导入、下载、恢复示例和浏览器本地保存。
- Aster O0/O1/O2 与 LLVM 级别默认关联；点击关联图标可独立调整 LLVM 优化。
- 运行参数使用 JSON 字符串数组，例如 `["10000"]`，每项作为一个 argv 参数，不经过 shell。
- AST 提供稳定 ID 节点和原始 JSON；SSA 与 LLVM 提供搜索、复制及下载。使用 `Ctrl/Cmd+F` 搜索代码。
- 错误定位转换 UTF-8 字节列为编辑器 UTF-16 列，支持中文及 emoji 前缀。点击诊断跳转；标准库中的错误保留文件信息。
- 源码或选项改变后，已有结果明确标记为过期。点击运行会先编译当前版本。
- 分栏可以拖动或聚焦后用方向键调整；工具链信息面板提供「重置布局」。窄屏使用源码／编译结果／终端标签。

终端显示实际 stdout、stderr、退出码、耗时和可展开的完整命令。编译调用默认限时 30 秒，执行限时 5 秒，输出限额 1 MiB；停止、超时或超量会终止任务进程组。它是本地原生程序执行器，不提供交互式 PTY 或 stdin 会话。

## 优化探索

支持当前编译任务，以及原项目保存的优化实验和 O0/O2 Stage 1–3 自举追踪。

- 函数按稳定 ID 索引；Pass 按数字序号排序。两次 SCCP 等重复 Pass 分别保留，跨模块同名函数不会用名称错误合并。
- 左侧展示真实 Pass 顺序、变化记录和指令数；右侧可比较任意两个快照，支持并排／行内 Diff。
- 指令数沿用编译器 `count_ir`：不含 `arg`、`const`、`string`，也不含终结指令。不会按文本行数计数。
- CFG 节点来自原始 SSA 基本块，边来自 branch／jump 终结指令；分支标注 true／false，保留循环回边。
- 选择 SSA 值查看输入、使用者及 phi 的前驱关系；图和代码联动。提供键盘可操作的基本块列表。
- 差异以稳定值 ID 比较，识别新增、删除、修改和块间移动。LICM 指令数不变时仍能看到移动。
- Pass 耗时、分析依赖、失效信息及 `changed_instructions` 均来自原始统计。变化记录与值差异数可能不同，控制流变化需要对照 Diff。

算法说明是静态解释；图和 Diff 来自真实快照。界面不会声称记录了 SCCP 格变迁或工作列表内部事件。

## 自举仪表盘

读取 `artifacts/bootstrap.json`、两组测试、性能、sanitizer、优化检查与工具链清单。Stage 2/3 的 LLVM 和 ELF 哈希会重新计算，区分历史结果、当前文件一致、缺失或变化。

性能图包含 A0-L0、A2-L0、A2-L2 三种配置；展示原实验的一次预热、五次测量、中位数、全部样本及输出 SHA-256。不隐藏变慢的测量结果，也不把历史测量当成实时测量。原始记录可下载，缺失文件按项显示缺失状态。

## 接口与任务文件

FastAPI 的 `/docs` 提供完整请求契约。主要接口：

| 接口 | 用途 |
|---|---|
| `GET /api/health` | 实际编译器路径、SHA-256、运行时和 Clang 状态 |
| `GET /api/examples` | 示例源码及默认参数 |
| `POST /api/jobs` | `{source, optimization, llvm_optimization}`，返回 202 和任务 |
| `GET /api/jobs/{id}` | 当前阶段、诊断、命令结果、统计及制品索引 |
| `POST /api/jobs/{id}/run` | `{arguments: string[]}`，执行已生成程序 |
| `POST /api/jobs/{id}/cancel` | 停止当前任务 |
| `GET /api/jobs/{id}/artifacts/{key}` | `ast / ssa / llvm / stats / source / binary`；`?download=true` 下载 |
| `GET /api/traces` | 已记录追踪数据集 |
| `GET /api/traces/{id}/functions` | 函数与逐 Pass 索引；实时任务使用 `job-{id}` |
| `GET /api/traces/{id}/snapshots/{key}` | 原始 SSA、摘要与派生图结构 |
| `GET /api/dashboard` | 实验记录、当前制品和来源校验 |
| `GET /api/records/{key}` | 原始实验 JSON 下载 |

任务目录位于 **当前 worktree** 的 `build/playground/jobs/{id}/`。含 `main.aster`、`program`、`program.ll`、`ast.json`、`final.ssa`、`stats.json`、`trace/` 和 `job.json`。标准库与运行时使用符号链接，原工作区只读。任务查询在服务生命周期内有效；重启后重新编译即可，磁盘文件保留用于复核。

后端只适配 CLI 和解析展示数据；所有语义、SSA 构造、优化和 LLVM 发射仍由编译器负责。首次编译生成原生程序、LLVM、逐函数追踪和统计，再调用相同编译器输出 AST；不混用两个编译器的产物。

## 检查与展示证据

```bash
bash playground/check.sh
```

检查包含真实编译器的后端集成测试、真实 SSA 快照驱动的前端差异测试、TypeScript 检查及生产构建。不会重跑整套自举。

浏览器验收脚本使用 `playwright-cli`，先启动展示服务，再在 worktree 根目录执行：

```bash
playwright-cli --help
playwright-cli list
# 没有会话时，按本机配置连接 Chrome 扩展
source /home/chai/.config/playwright-cli/env.zsh
playwright-cli attach --extension=chrome
playwright-cli -s=chrome tab-new http://127.0.0.1:8000
playwright-cli -s=chrome run-code --filename=playground/tests/browser-smoke.js
```

此浏览器脚本会重置本站 `astercc:` 偏好并编辑示例，用于验收专用标签页。检查结果和截图见 [验证记录](evidence/validation.md)。
