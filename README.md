# AsterCC

AsterCC 是一个可以自举的静态类型优化编译器。C++20 种子编译器与 Aster 编写的编译器都直接生成 LLVM 18 IR；词法、语法、类型检查、SSA、SCCP、GVN 和 LICM 均由项目自身实现。

## 一条命令复现

环境：Ubuntu 24.04 amd64，GCC 13 或更高、CMake 3.20+、Python 3.10+、Make、`apt-get` 和 `dpkg-deb`。

```bash
bash scripts/reproduce.sh
```

脚本将校验过的 LLVM 18.1.3 软件包解压到 `.toolchain/`，不安装系统软件或 Python 依赖。第一次运行需要网络；后续使用本地包缓存。编译所用 libc、libstdc++ 等基础系统库由 Ubuntu 提供。记录的版本、包校验值见 [toolchain.json](artifacts/toolchain.json)。

复现过程依次完成：C++ 种子构建、CTest、O0/O2 三轮自举、逐 Pass 比较、移开种子后的 Stage 3 独立编译、跨阶段行为与制品比较、优化测量、ASan/UBSan 检查及 Markdown 报告生成。任何必要检查失败都会停止。

报告为 [实验报告.md](实验报告.md)，严格沿用课程模板的五个部分。原始要求和模板未修改。

## 使用编译器

```bash
export ASTERCC_ROOT="$PWD"
export LD_LIBRARY_PATH="$PWD/.toolchain/usr/lib/x86_64-linux-gnu:$PWD/.toolchain/usr/lib/llvm-18/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
build/bootstrap/O2/stage3 examples/hello.aster -O2 --verify-ir -o build/hello
build/hello

# 观察自定义 SSA；LLVM IR 是另一种输出，不混为一层
build/bootstrap/O2/stage3 tests/cases/optimization.aster -O0 --emit=ssa -o build/before.ssa
build/bootstrap/O2/stage3 tests/cases/optimization.aster -O2 --emit=ssa -o build/after.ssa
build/bootstrap/O2/stage3 examples/hello.aster --emit=ast -o build/hello.ast.json
```

默认 O0、生成可执行文件。`--emit=ast|ssa|llvm|obj|exe` 选择输出；`--check` 只检查源码；`-O0/-O1/-O2` 同时选择 Aster 和 LLVM 优化级别，`--llvm-opt=0|1|2` 单独覆盖后端级别。`--passes=simplify,sccp,cfg,dce,gvn,licm` 指定完整替代流水线。`--stats-json PATH` 保存统计，`--trace-dir DIR` 保存逐函数、逐 Pass 的 SSA。

导入路径相对于 `ASTERCC_ROOT`；默认根目录为当前目录。原生链接默认使用根目录中的 `build/libasterrt.a`；可通过 `ASTERCC_CLANG` 指定 LLVM 18 Clang 路径。输出 `.o` 时可自行与该运行时链接。

## 源码与验证入口

| 位置 | 内容 |
|---|---|
| `compiler/` | Aster 实现：模型、前端、类型化 SSA 降级、优化、LLVM 发射及驱动 |
| `stage0/` | 可单独构建的 C++20 种子源码及宿主支持 |
| `std/` | Aster 编写的确定性哈希表、字符串构建器和 I/O 包装 |
| `runtime/` | C 平台接口、稳定数组控制块、arena 和进程入口 |
| `tests/` | 按语义组织的程序；生成式、诊断和终止行为检查由 Python 编排 |
| `artifacts/` | 本次实际执行的哈希、Pass 统计、测量及检查记录 |
| `docs/` | 语言契约和实现说明 |

[语言契约](docs/language.md) · [实现说明](docs/architecture.md) · [自举记录](artifacts/bootstrap.json) · [优化数据](artifacts/benchmarks.json)

## 种子来源与自举含义

开发期的 `scripts/sync_seed.py` 将编译器实现所使用的 Aster 子集机械映射为 C++，便于审阅和维护语义一致性；生成的 C++ 已提交，**CMake 和复现脚本均不运行该工具**。它不接受完整 Aster 语言，也不参与编译用户程序。

Stage 0 是原生 C++ 编译器，编译器自身的 Aster 源码经它直接生成 LLVM IR；Stage 1/2/3 的全部编译算法由 Aster 代码实现，只通过 C 运行时调用 LLVM 工具和平台接口。自举固定点比较原始字节，不使用文本过滤、反汇编归一化或拷贝上一阶段来制造一致结果。

首版语言不包含浮点、泛型函数、闭包、裸指针、异常、GC、独立模块 ABI 或跨平台支持。arena 适合本项目的批处理编译任务，长时间运行程序的内存回收留待后续扩展。
