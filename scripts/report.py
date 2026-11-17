#!/usr/bin/env python3
"""Generate the course-template Markdown report from real run artifacts."""
import hashlib,json,os,platform,re,subprocess,sys
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo
ROOT=Path(__file__).resolve().parents[1]
def read(name):return json.loads((ROOT/'artifacts'/name).read_text())
def command(args):return subprocess.check_output(args,text=True,cwd=ROOT).strip()
def table(headers,rows):return '\n'.join(['| '+' | '.join(headers)+' |','| '+' | '.join(['---']*len(headers))+' |']+['| '+' | '.join(map(str,row))+' |' for row in rows])
def main():
 boot=read('bootstrap.json');tests=read('tests.json');stage3=read('tests-stage3.json');bench=read('benchmarks.json');san=read('sanitizers.json');tool=read('toolchain.json')
 env=os.environ|{'ASTERCC_ROOT':str(ROOT)};exe=ROOT/'build/demo'
 cmd=[str(ROOT/'build/bootstrap/O2/stage3'),'examples/hello.aster','-O2','--verify-ir','-o',str(exe)]
 subprocess.run(cmd,cwd=ROOT,env=env,check=True);demo=subprocess.check_output([str(exe)],text=True)
 (ROOT/'artifacts/demo.json').write_text(json.dumps({'command':cmd,'source':'examples/hello.aster','stdout':demo,'returncode':0},ensure_ascii=False,indent=2)+'\n')
 cpu=next(x.split(':',1)[1].strip() for x in Path('/proc/cpuinfo').read_text().splitlines() if x.startswith('model name'))
 mem=next(x.split(':',1)[1].strip() for x in Path('/proc/meminfo').read_text().splitlines() if x.startswith('MemTotal'))
 environment={'date':datetime.now(ZoneInfo('Asia/Shanghai')).isoformat(timespec='seconds'),'os':platform.platform(),'cpu':cpu,'memory':mem,'g++':command(['g++','--version']).splitlines()[0],'cmake':command(['cmake','--version']).splitlines()[0],'python':sys.version.split()[0],**tool['versions']}
 (ROOT/'artifacts/environment.json').write_text(json.dumps(environment,ensure_ascii=False,indent=2)+'\n')
 stages=boot['stages'];hash0=next(r['binary_sha256'] for r in stages if r['optimization']==0 and r['stage']==3);hash2=next(r['binary_sha256'] for r in stages if r['optimization']==2 and r['stage']==3)
 sourcefiles=list((ROOT/'compiler').glob('*.aster'));asterlines=sum(len(p.read_text().splitlines()) for p in sourcefiles);functions=sum(len(re.findall(r'^fn\s',p.read_text(),re.M)) for p in sourcefiles)
 snapshots=ROOT/'artifacts/optimization';input_ir=(snapshots/'0-00-input.ssa').read_text();output_ir=(snapshots/'optimized.ssa').read_text();passrows=json.loads((snapshots/'stats.json').read_text())
 diagnostic=next(r['stderr'] for r in tests['results'] if r['case']=='diagnostic-type')
 behavior=next(r['stdout'] for r in stage3['results'] if r['case']=='effects' and r['optimization']==2)
 speedrows=[]
 for name in ['constants','gvn','licm','arrays']:
  baseline=next(r for r in bench['rows'] if r['benchmark']==name and r['configuration']=='A0-L0');own=next(r for r in bench['rows'] if r['benchmark']==name and r['configuration']=='A2-L0')
  speedrows.append([name,baseline['instructions'],own['instructions'],f"{baseline['runtime_ms']/own['runtime_ms']:.3f}×"])
 values={
 '@@DATE@@':environment['date'],
 '@@ENV@@':table(['项目','实际值'],[(k,str(v)) for k,v in environment.items()]),
 '@@LINES@@':str(asterlines),'@@FUNCTIONS@@':str(functions),
 '@@STAGES@@':table(['优化','阶段','原生 ELF SHA-256 前 16 位','LLVM IR SHA-256 前 16 位','构建秒数'],[[f'O{r["optimization"]}',r['stage'],r['binary_sha256'][:16],r['llvm_sha256'][:16],f'{r["seconds"]:.3f}'] for r in stages]),
 '@@HASH0@@':hash0,'@@HASH2@@':hash2,
 '@@TRACE0@@':str(next(r['trace_files'] for r in stages if r['optimization']==0)),
 '@@TRACE2@@':str(next(r['trace_files'] for r in stages if r['optimization']==2)),
 '@@TEST0@@':str(tests['passed']),'@@TEST3@@':str(stage3['passed']),
 '@@DEMO@@':demo.rstrip(),'@@EFFECT@@':behavior.rstrip(),'@@DIAGNOSTIC@@':diagnostic.rstrip(),
 '@@BENCH@@':table(['案例','配置','IR 指令','基本块','编译 ms','执行中位数 ms'],[[r['benchmark'],r['configuration'],r['instructions'],r['blocks'],f'{r["compile_ms"]:.3f}',f'{r["runtime_ms"]:.3f}'] for r in bench['rows']]),
 '@@SPEED@@':table(['案例','A0-L0 指令','A2-L0 指令','固定 LLVM O0 的运行加速比'],speedrows),
 '@@PASSES@@':table(['函数','Pass','变动指令','优化前','优化后'],[[r['function'],r['pass'],r['changed_instructions'],r['before'],r['after']] for r in passrows]),
 '@@BEFORE@@':input_ir.rstrip(),'@@AFTER@@':output_ir.rstrip(),
 }
 report=r'''# AsterCC 自举优化编译器实验报告

## 一、——实验目的

本实验实现一个能够编译自身的静态类型优化编译器 AsterCC。实验从词法、语法和类型检查出发，构造自定义 SSA 中间表示，完成模块化优化，再经 LLVM 18 后端生成可运行的 x86-64 Linux 程序。

验收目标是实际完成 C++ Stage 0 → Aster Stage 1 → Stage 2 → Stage 3，并使 Stage 2/3 的 LLVM IR 和原生可执行文件逐字节相同。同时用独立结果计算、已知行为程序和逐 Pass 检查检验优化语义，区分自定义优化和 LLVM 优化的贡献。

本报告及其数据由 `bash scripts/reproduce.sh` 生成。数据记录时间为 @@DATE@@，采用 Asia/Shanghai 时区。姓名、学号等身份信息未作假定。

## 二、实验环境

@@ENV@@

LLVM 使用锁定为 18.1.3 的 Ubuntu 软件包，在项目 `.toolchain/` 内解压，包 SHA-256 固定在 [工具链锁文件](scripts/llvm18.lock.json)。基础 libc 等库由当前 Ubuntu 提供，完整版本和包记录见 [工具链记录](artifacts/toolchain.json)、[环境记录](artifacts/environment.json)。不安装共享 Python 环境或第三方 Python 项目依赖。

Stage 0 使用 C++20 和 CMake；运行时使用 C11。Aster 编译器主体目前为 @@LINES@@ 行、@@FUNCTIONS@@ 个 Aster 函数，按前端、SSA、优化、LLVM 发射及驱动划分源码模块。标准库另外用 Aster 实现确定性哈希表和字符串构建器。

## 三、实验内容及原理

### 语言和编译目标

Aster 包含严格静态类型、函数和递归、具名结构体、枚举、动态数组、UTF-8 字节串、模块导入和命名空间；控制流支持条件、循环、提前返回及循环跳转。结构体按值复制，数组采用共享引用语义；数组扩容不破坏别名。详细契约见 [语言说明](docs/language.md)。

```aster
import std.io;
struct Point { x: i64, y: i64, }
fn main() -> i64 {
    var values: []i64 = [3, 1, 4];
    push(values, 9);
    io::println_int(values[0] + len(values));
    return 0;
}
```

整数运算采用确定的回绕语义，条件必须为 bool。字符串不可变，索引返回字节 u8。调用和表达式从左到右求值，逻辑运算短路。语言不包含裸指针、隐式类型强转、泛型函数、浮点、闭包或垃圾回收。类型名在整程序编译单元内唯一，函数按模块区分；这使首版能够集中完成编译器自身所需的能力。

### 编译结构

```mermaid
flowchart LR
    A[Aster 源码与模块] --> B[Token 与 AST arena]
    B --> C[名称解析 静态检查 类型化降级]
    C --> D[自定义 SSA]
    D --> E[SCCP GVN LICM CFG DCE]
    E --> F[LLVM 18 文本 IR]
    F --> G[Clang 目标代码生成与链接]
    G --> H[原生可执行程序]
```

前端采用手写词法分析、Pratt 表达式解析和递归下降语句解析。全部模块先完成声明与 AST 收集，因此允许前向调用和相互递归。名称解析、静态检查与 SSA 降级在一次遍历中协作完成，操作数确认类型后才生成相应指令，并将推断类型记录到 AST。终止语句之后的语法仍在脱离入口的块中接受静态检查，最终剔除不可达块。

LLVM 接收的已经是 AsterCC 自行构造和优化过的 SSA。AsterCC 不调用 LLVM 的 mem2reg 来完成自己的 SSA，也不将 SCCP、GVN、LICM 委托给 LLVM。LLVM 负责目标相关优化、指令选择、寄存器分配、汇编和链接。

### 自举和运行时

| 阶段 | 实现语言 | 构建来源 |
| --- | --- | --- |
| Stage 0 | C++20 | GCC 编译提交的 C++ 源码 |
| Stage 1 | Aster | Stage 0 编译全部 Aster 编译器源码 |
| Stage 2 | Aster | Stage 1 编译相同源码 |
| Stage 3 | Aster | Stage 2 编译相同源码 |

C 运行时仅承担分配、字节缓冲、数组存储、文件、终端和进程等平台操作，不包含词法、语法、类型、IR、优化或代码发射算法。数组控制块固定，内部缓冲几何增长；分配记录在进程级 arena，程序退出时统一释放。

开发期维护工具 `sync_seed.py` 将编译器实现使用的 Aster 子集机械映射为独立 C++20 源码。提交的 C++ 种子无需该工具即可构建，CMake 和正式复现均不调用它。编译用户程序及 Stage 1/2/3 时没有 Aster 到 C++ 的路径；Aster 源码直接生成 LLVM IR。该取舍降低双份实现的漂移，但意味着两者不是独立设计的语义裁判。

## 四、关键数据结构和核心算法（可从整体功能设计上进行描述，也可按照完成的各项功能设计或任务进行描述）

### 4.1 设计的各类数据结构

| 数据结构 | 主要字段和作用 |
| --- | --- |
| Token | 文本、种类、源文件 ID 和字节偏移，支持定位诊断 |
| Node | 标签、类型、操作数 ID、子节点数组和 Token ID，组成 AST arena |
| Function / Record | 参数、返回、函数体、模块、字段和类型声明 |
| Inst | 操作、结果类型、操作数、phi 入边、所在块、替换 ID、删除标记 |
| Block | 指令、前驱、局部变量到达定义、待完成 phi、封闭状态、终结指令 |
| IR | 单函数的指令/块/局部变量 arena、当前块和循环出口栈 |
| StringMap | 开放寻址键值数组、占用标记和元素数，确定性重哈希 |
| Lens | 已求值的赋值路径，记录数组、索引和记录字段，避免重复求值 |

所有编译器节点以稳定整数 ID 引用，避免把宿主指针、宿主哈希表迭代顺序或地址写入输出。结构体字段赋值通过“提取旧值—插入新字段—重建上层值”保留值语义；数组元素更新则写回共享存储。

自定义 IR 区分常量、参数、phi、整数运算、记录构造/提取/插入、数组操作、调用与控制流。替换链允许优化器重定向已有使用；查询时取得 canonical value。终结指令单独存放，不能被当作普通无用表达式删除。

### 4.2 实现的各项功能核心算法（可结合图、表、文字等进行展示）

#### 词法、解析和静态类型

词法器进行最长运算符匹配，支持嵌套块注释、字符串转义、十进制/十六进制整数及范围检查。Pratt 解析以运算符优先级约束递归，调用、索引、字段和转换作为后缀处理。记录字面量采用大写类型名，避免将 `if condition {` 误解为构造表达式。

类型检查验证名称、参数、返回、字段、数组元素、条件、转换、可变性及递归值布局。结构体不能无限按值包含自身，但可以经数组句柄递归。数组字面量使用预期元素类型或首元素推断，不采用动态类型或隐式装箱。

#### Sealed-block SSA 构造

采用 Braun 等人的按需 SSA 构造思路 [1]。读取局部变量时查询当前块的缓存定义；单前驱沿边查找，多前驱建立 phi；尚未知道全部前驱的块先登记不完整 phi。在回边封闭后补全，并消除所有输入相同的平凡 phi。

```text
read(block, variable):
    已有定义                 → 返回缓存值
    块尚未封闭               → 建立并缓存不完整 phi
    只有一个前驱             → 递归读取前驱
    多个前驱                 → 先缓存 phi，再递归填入各入边值
seal(block):
    填入待完成 phi 的全部入边，消除平凡 phi
```

正确性依赖三个构造不变式：局部变量每次赋值产生新值；phi 在递归填充前就成为该块定义；块封闭后不再添加源程序构造阶段的前驱。while header 在 body 和所有 continue 回边出现后封闭，exit 在所有 break 出口出现后封闭。短路逻辑使用真实分支和合流 phi，不提前执行右操作数。

按需查询只为发生读取的变量创建合流值。语言仅有结构化控制流，适合该构造算法；本实现没有对任意不可约 CFG 作最小 phi 数量的额外承诺。

#### SCCP 和 CFG 简化

SCCP 使用 Unknown、Constant、Overdefined 三层格，独立维护可执行边 [2]。值变化调度使用者所在块，新增可执行边调度目标块；phi 只合并已执行入边。有限值格与边集合均单调变化，因此工作列表收敛。

已知条件化为单向跳转，删除不可达块并修复 phi；CFG Pass 再合并无条件跳转的单前驱后继。不同值的合流保留 Overdefined，不把当前尚未知晓的值误当作零。

#### 支配树 GVN 和 LICM

支配集合用有限集合不动点求解，再按严格支配链的深度恢复直接支配树。GVN 沿树的前序使用确定性哈希表查找同类型纯表达式；替换必须满足支配和块内顺序。常量按类型及内容参与值编号，避免相同字面量仅因 AST ID 不同而错失消除机会。

LICM 识别支配意义上的回边，通过反向前驱遍历取得自然循环。仅处理具有唯一无条件前置块的循环；当操作数都在循环外或已经外提时，将可安全提前执行的纯运算移动到前置块，迭代处理依赖。

数组读取、字符串索引、除法、分配、存储和调用不会被投机外提。比如零次迭代循环里的越界读取和除零不能提前变成程序终止；该模式已包含在行为检查中。DCE 从终结指令和可观察操作反向标记，保留可能终止程序的操作。

#### Pass 管理和验证

O1 为 simplify → SCCP → CFG → DCE；O2 再加入 GVN → LICM → simplify → SCCP → CFG → DCE。分析不跨 Pass 缓存，按需重算支配关系，统计中记录依赖和失效信息。

SSA 验证器检查块封闭、类型、phi 前驱及边的唯一性、支配关系、块内顺序和终结指令。`--verify-ir` 在初始构造及每个 Pass 后执行。生成的 LLVM IR 另经 LLVM 自身验证 [3]，形成自定义层与后端层的两级结构检查。

#### 确定性自举

每阶段重新编译同一源码，不拷贝上一阶段产物。固定源标识、模块顺序、稳定 ID、后端版本和链接参数，不嵌入时间、构建目录或阶段号。比较原始 `.ll` 和 ELF 字节，不做文本过滤。

逐 Pass 输出按函数 ID、Pass 序号和名称命名。脚本比较全部摘要，若不同则报告首个文件，定位差异发生在构造、SCCP、GVN 等哪一步。运行耗时不进入确定性摘要。该跨阶段可观察性是项目的工程特色，不将已有 SSA 或优化算法宣称为新的理论算法。

### 4.3 错误处理（编程过程中容易遇到的错误及处理）

| 易错点 | 本项目的处理 |
| --- | --- |
| 字符串内容恰好是 `[` 或 `}` | 语法判断依据 Token 种类，避免把字符串内容当分隔符 |
| 循环回边尚未添加就完成 phi | header 延迟封闭，continue 和普通回边全部登记后再补全 |
| 记录字段更新意外修改副本 | 使用聚合值重建，数组句柄的共享语义则单独保留 |
| 赋值目标中的索引带副作用 | Lens 缓存已求值路径，索引表达式只执行一次 |
| LLVM 的有符号溢出与除法陷阱语义 | 不使用 nsw/nuw；种子折叠经无符号位模式运算；运行时处理最小整数除以 -1 |
| 不可变字符串交给 C 平台接口 | LLVM 字节常量补 NUL，逻辑长度仍独立记录，保留嵌入 NUL 的语言语义 |
| UTF-8 字节在宿主 C++ 中变成负 char | 宿主字符串索引返回 u8，保证种子与 Aster 字节语义相同 |
| CFG 变化后分析结果失效 | 不跨 Pass 复用缓存；修复 phi，按需重新计算分析 |

下面是实际执行的类型错误诊断，包含原始输入位置、源码与标记：

```text
@@DIAGNOSTIC@@
```

前端诊断退出状态为 1，用法错误为 2。越界和除零遵循语言定义，以状态 70 终止；文件错误报告明确路径。没有使用吞掉错误继续生成代码的兜底策略。

### 4.4 运行结果和分析（体现输入和输出）

#### 原生程序

实际输入为 `examples/hello.aster`，包含 0 到 9 的循环累加；命令如下：

```bash
build/bootstrap/O2/stage3 examples/hello.aster -O2 --verify-ir -o build/demo
build/demo
```

实际输出：

```text
@@DEMO@@
```

#### 自举固定点

@@STAGES@@

两条链中 Stage 1/2/3 的 LLVM IR 均逐字节一致，Stage 2/3 ELF 也逐字节一致。Stage 3 完整 ELF SHA-256 为：

```text
O0: @@HASH0@@
O2: @@HASH2@@
```

每阶段 O0 保存 @@TRACE0@@ 个跟踪摘要，O2 保存 @@TRACE2@@ 个；相同优化级别的三阶段摘要全部一致。记录见 [bootstrap.json](artifacts/bootstrap.json) 和 [trace-hashes.json](artifacts/trace-hashes.json)。源码与固定运行时的校验值也记录在自举数据中。

独立性检查把 Stage 3 和运行时搬到另一目录，将 Stage 0 临时移开，以新源码编译 Fibonacci(20)，实际输出 `6765`。ELF 动态依赖检查确认 Stage 3 不依赖 libstdc++。该检查使用新编译的程序，不是运行预先生成的样例。

#### 行为、变换与内存检查

Stage 0 通过 @@TEST0@@ 项行为检查，Stage 3 通过相同的 @@TEST3@@ 项；用例覆盖函数递归、模块、记录副本、数组别名及增长、嵌套数组、字符串及 UTF-8、枚举、整数边界、循环跳转、短路和求值顺序、静态错误、越界与除零。16 个固定种子生成的算术表达式使用独立 Python 位模式计算作为参考，固定种子为 20261117。

对六组有效程序，在 O0/O1/O2 分别比较两个编译器的 typed AST、规范自定义 SSA 和 LLVM IR，共 54 次跨阶段制品比较；各次编译的 LLVM IR 同时接受 LLVM 验证。另有 5 项结构性断言确认循环 phi 存在，以及 SCCP、DCE、GVN 确实消除指令、LICM 确实移动指令。

副作用用例中，索引先记录 1，右值再记录 7，随后左右加数依次记录 2、3；短路分支未执行额外记录。末尾零次循环包含越界读取及除零，不能被 LICM 提前执行。实际输出：

```text
@@EFFECT@@
```

ASan/UBSan 检查通过：C++ 种子使用 O2 检查自身完整 Aster 源码；C 运行时执行包含数组增长、记录、哈希表和 UTF-8 的 data 用例，无 sanitizer 报告。检查规模按行为组织，没有为每个内部函数镜像实现编写测试。原始记录见 [Stage 0 检查](artifacts/tests.json)、[Stage 3 检查](artifacts/tests-stage3.json)、[变换断言](artifacts/optimization/checks.json)、[sanitizer 记录](artifacts/sanitizers.json)。

#### 优化前后的自定义 SSA

下面是优化用例 kernel 的初始 SSA；包含循环变量、分支合流及重复乘法：

```text
@@BEFORE@@
```

以下为同一用例 O2 后的完整 SSA：

```text
@@AFTER@@
```

实际逐 Pass 统计如下。变动指令统计同时包含替换、删除、操作数规范化和所在块变化，因此不等于单纯的删除数量；LICM 即使总指令数不变也有变动记录。

@@PASSES@@

完整跟踪见 [优化制品目录](artifacts/optimization)。

#### 性能与优化贡献

使用 Stage 3 编译四个程序，输入迭代次数均为 3,000,000，其余参数固定记录在 JSON。每个配置预热一次，执行五次，报告中位数；所有输出与独立参考结果一致。执行时间包含进程启动，编译时间包含验证、统计与后端链接，因此小幅差异不作强结论。

A0-L0 表示自定义 O0、LLVM O0；A2-L0 固定 LLVM O0 观察自身优化；A2-L2 再打开 LLVM O2。IR 指令统计不包含常量、参数和字符串伪指令，也不计控制流终结指令，三个配置采用同一口径。

@@BENCH@@

固定 LLVM O0 时的直接比较为：

@@SPEED@@

常量传播可直接减少控制流与算术；GVN 消除重复表达式；LICM 把不变量移出循环，因此总指令数可能不变但动态执行量减少。数组案例保留实际读取与边界语义，只移动独立的纯运算。A2-L2 的结果体现 LLVM 的后续贡献，与自定义优化分别报告，不将其混记为 AsterCC Pass 的收益。

每次测量样本、原始输出、输出 SHA-256、二进制规模和 Pass 统计见 [benchmarks.json](artifacts/benchmarks.json)。这些是本机小规模内核的测量，不代表大型应用或其他硬件的普遍加速比。

## 五、总结

本实验交付了真实可运行的静态类型语言和编译器，并在 O0、O2 两个配置完成 Stage 0 → Stage 1 → Stage 2 → Stage 3。自定义 SSA 构造与优化都能由 Aster 自身执行；固定点、逐 Pass 跟踪、独立运行及语义结果相互补充，使自举结果可以复核。

主要工程体会是先固定可观察语义，再选择优化算法。回绕整数、短路、数组共享和可能终止的操作都会影响合法变换，不能只根据代数形式或未使用的结果判断是否可删除。稳定 ID、明确的副作用分类和阶段摘要，也比只比较最终输出更容易定位编译器自身的错误。

目前的取舍是：支配集合和 SCCP 边集合使用矩阵，空间为 O(B²)；运行时为进程级 arena；模块整程序编译、类型名全局唯一；没有别名分析、MemorySSA、过程间优化、向量化、垃圾回收或调试信息。后续可在保持现有可复核自举链的基础上，改用稀疏支配结构、加入更精细的内存分析和长期运行的回收机制。

固定点证明当前源码与工具链下的编译稳定，不等于完整的语义正确性或信任链安全证明。由于种子与 Aster 实现同源，额外的独立参考结果和已知行为检查尤为必要。本报告区分实现、测试证据与尚未覆盖的能力，不把计划功能当作已完成结果。

参考资料：

1. Braun et al. [Simple and Efficient Construction of Static Single Assignment Form](https://pp.ipd.kit.edu/uploads/publikationen/braun13cc.pdf), CC 2013。
2. Wegman and Zadeck. [Constant Propagation with Conditional Branches](https://rsim.cs.illinois.edu/arch/qual_papers/compilers/wegman91.pdf), TOPLAS 1991。
3. LLVM. [LLVM 18 Language Reference](https://releases.llvm.org/18.1.8/docs/LangRef.html) 与 [opt 工具说明](https://releases.llvm.org/18.1.8/docs/CommandGuide/opt.html)。
'''
 for key,value in values.items():report=report.replace(key,value)
 assert '@@' not in report
 (ROOT/'实验报告.md').write_text(report)
 print('Markdown report generated from recorded runs: 实验报告.md')
if __name__=='__main__':main()
