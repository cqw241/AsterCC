# 实验运行证据

这些文件由 `bash scripts/reproduce.sh` 的实际运行生成。

| 文件 | 内容 |
|---|---|
| `bootstrap.json` | 各阶段原生文件与 LLVM IR 哈希、耗时、源码/运行时校验值及独立运行结果 |
| `trace-hashes.json` | 每个函数、每个 Pass 的原始跟踪文件摘要；耗时不参与比较 |
| `bootstrap/` | 编译命令、原始日志、LLVM 验证日志和逐阶段 Pass 统计 |
| `tests.json` / `tests-stage3.json` | 种子及 Stage 3 的实际输出、诊断与检查状态 |
| `optimization/` | 小型优化例子的原始跟踪、统计及变换断言 |
| `benchmarks.json` | 三组优化配置的全部样本、输出、SHA-256 和编译器 Pass 统计 |
| `sanitizers.json` | ASan/UBSan 的完整命令、输出和退出状态 |
| `demo.json` | 报告展示的原生示例及真实输出 |
| `toolchain.json` / `environment.json` | 锁定软件包和实际执行环境 |
| `delivery.json` | 最终文件哈希、源码与自举产物一致性、报告链接及输出接口检查 |

大体积的 ELF、完整编译器 LLVM IR、逐阶段原始跟踪保存在可重建的 `build/` 中。测试临时程序由固定脚本生成，成功收尾后清理；其输出和诊断保留在 JSON。修改编译器后必须重新运行复现命令，不能沿用旧自举数据。
