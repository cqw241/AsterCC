import { useEffect, useMemo, useState } from 'react';
import {
  ArrowRight,
  CheckCircle2,
  ChevronDown,
  CircleAlert,
  Clock3,
  Database,
  Download,
  ExternalLink,
  Fingerprint,
  GitBranch,
  RefreshCw,
  ShieldCheck,
  TestTube2,
} from 'lucide-react';
import type { BenchmarkRow, DashboardData } from '../types';
import { api, errorMessage } from '../api';
import { Button } from './ui/button';
import { Badge, CopyButton, DownloadLink, Empty, ErrorState, Spinner } from './common';
import Chart from './Chart';
export default function Dashboard() {
  const [data, setData] = useState<DashboardData | null>(null),
    [loading, setLoading] = useState(true),
    [error, setError] = useState(''),
    [retry, setRetry] = useState(0);
  const [level, setLevel] = useState(2),
    [metric, setMetric] = useState<'runtime_ms' | 'compile_ms' | 'instructions'>('runtime_ms');
  const [testCompiler, setTestCompiler] = useState<'tests' | 'tests-stage3'>('tests-stage3'),
    [testQuery, setTestQuery] = useState(''),
    [benchmark, setBenchmark] = useState('constants');
  useEffect(() => {
    let active = true;
    setLoading(true);
    setError('');
    api<DashboardData>('/dashboard')
      .then((d) => {
        if (active) setData(d);
      })
      .catch((e) => {
        if (active) setError(errorMessage(e));
      })
      .finally(() => {
        if (active) setLoading(false);
      });
    return () => {
      active = false;
    };
  }, [retry]);
  const rows = data?.records.benchmarks?.rows;
  const plot = useMemo(
    () => ({
      grid: { top: 42, left: 55, right: 24, bottom: 32 },
      legend: { top: 0, textStyle: { color: '#A8B6CA' }, itemWidth: 12, itemHeight: 8 },
      tooltip: { trigger: 'axis', axisPointer: { type: 'shadow' } },
      xAxis: {
        type: 'category',
        data: [...new Set(rows?.map((r) => r.benchmark) ?? [])],
        axisLine: { lineStyle: { color: '#364155' } },
        axisLabel: { color: '#A8B6CA' },
      },
      yAxis: {
        type: 'value',
        name: metric === 'instructions' ? 'instructions' : 'ms',
        nameTextStyle: { color: '#8798B1' },
        splitLine: { lineStyle: { color: '#2B3443' } },
        axisLabel: { color: '#8798B1' },
      },
      series: ['A0-L0', 'A2-L0', 'A2-L2'].map((config, i) => ({
        name: config,
        type: 'bar',
        barMaxWidth: 28,
        itemStyle: { color: ['#627797', '#79A8FF', '#79C6AA'][i], borderRadius: [3, 3, 0, 0] },
        data: [...new Set(rows?.map((r) => r.benchmark) ?? [])].map(
          (n) =>
            rows?.find((r) => r.benchmark === n && r.configuration === config)?.[metric] ?? null,
        ),
      })),
    }),
    [rows, metric],
  );
  if (error)
    return (
      <div className="dashboard-page">
        <ErrorState message={error} retry={() => setRetry((v) => v + 1)} />
      </div>
    );
  if (loading || !data) return <Spinner label="读取实验记录并校验当前制品…" />;
  const boot = data.records.bootstrap,
    stages = data.stages.filter((s) => s.optimization === level),
    second = stages.find((s) => s.stage === 2),
    third = stages.find((s) => s.stage === 3);
  const currentEqual =
    !!second?.current_binary_sha256 &&
    !!third?.current_binary_sha256 &&
    second.current_binary_sha256 === third.current_binary_sha256 &&
    !!second.current_llvm_sha256 &&
    second.current_llvm_sha256 === third.current_llvm_sha256;
  const currentMatches = stages.length === 3 && stages.every((s) => s.current_matches),
    test = data.records[testCompiler];
  const tests =
      test?.results.filter((r) =>
        `${r.case} ${r.status}`.toLowerCase().includes(testQuery.toLowerCase()),
      ) ?? [],
    benchmarkRows = rows?.filter((r) => r.benchmark === benchmark) ?? [];
  return (
    <div className="dashboard-page">
      <div className="page-heading">
        <div>
          <div className="page-eyebrow">
            <Database size={14} />
            可复核的实验记录
          </div>
          <h1>
            自举仪表盘<span>Bootstrap Dashboard</span>
          </h1>
        </div>
        <div className="page-heading-actions">
          <Badge tone={currentMatches ? 'good' : 'warn'}>
            {currentMatches ? '当前制品与记录一致' : '请检查制品状态'}
          </Badge>
          <Button variant="outline" onClick={() => setRetry((v) => v + 1)}>
            <RefreshCw size={14} />
            刷新并校验
          </Button>
        </div>
      </div>
      <div className="dashboard-note">
        <Clock3 size={14} />
        <span>实验记录与制品校验</span>
      </div>
      <section className="dashboard-section bootstrap-section">
        <div className="section-heading">
          <h2>
            <GitBranch size={17} />
            自举固定点
          </h2>
          <div className="section-actions">
            <div className="segmented">
              {[0, 2].map((v) => (
                <button
                  className={level === v ? 'selected' : ''}
                  onClick={() => setLevel(v)}
                  key={v}
                >
                  O{v}
                </button>
              ))}
            </div>
            <DownloadLink href="/api/records/bootstrap" label="下载自举记录" />
          </div>
        </div>
        {!boot ? (
          <Empty title="缺少 bootstrap.json">在原项目中运行复现脚本后刷新。</Empty>
        ) : (
          <>
            <div className="bootstrap-pipeline">
              <div className="stage-card seed-card">
                <div className="stage-label">
                  <span>Stage 0</span>
                  <Badge>C++20</Badge>
                </div>
                <h3>种子编译器</h3>
                <p>GCC → 原生编译器</p>
                <code>{data.seed.sha256?.slice(0, 16) ?? '文件缺失'}</code>
                <div className="stage-footer">
                  {data.seed.bytes ? `${(data.seed.bytes / 1024).toFixed(1)} KiB` : '不可用'}
                  <span>构建耗时未记录</span>
                </div>
              </div>
              {stages.map((s) => (
                <div className="pipeline-step" key={s.stage}>
                  <ArrowRight size={18} className="pipeline-arrow" />
                  <div className="stage-card">
                    <div className="stage-label">
                      <span>Stage {s.stage}</span>
                      <Badge
                        tone={s.current_matches ? 'good' : s.current_binary_sha256 ? 'warn' : 'bad'}
                      >
                        {s.current_matches ? '一致' : s.current_binary_sha256 ? '已变化' : '缺失'}
                      </Badge>
                    </div>
                    <h3>Aster 编译器</h3>
                    <p>由 Stage {s.stage - 1} 编译</p>
                    <code>{s.binary_sha256.slice(0, 16)}</code>
                    <div className="stage-footer">
                      {(s.binary_bytes / 1024).toFixed(1)} KiB<span>{s.seconds.toFixed(3)} s</span>
                    </div>
                  </div>
                </div>
              ))}
            </div>
            <div className="verification-strip">
              <div>
                <CheckCircle2 size={16} />
                <span>历史固定点</span>
                <strong>
                  {(level === 0 ? boot.fixed_point_O0 : boot.fixed_point_O2) ? '通过' : '未通过'}
                </strong>
              </div>
              <div className={currentEqual ? '' : 'verification-warn'}>
                <Fingerprint size={16} />
                <span>当前 Stage 2 / 3</span>
                <strong>{currentEqual ? 'LLVM + ELF 逐字节一致' : '缺失或不一致'}</strong>
              </div>
              <div>
                <ShieldCheck size={16} />
                <span>独立运行记录</span>
                <strong>
                  {boot.standalone?.seed_hidden && boot.standalone.relocated
                    ? '移开种子后通过'
                    : '未验证'}
                </strong>
              </div>
              <div>
                <GitBranch size={16} />
                <span>跨阶段追踪记录</span>
                <strong>{boot.cross_stage_traces_equal ? '一致' : '未通过'}</strong>
              </div>
            </div>
            <details className="hash-details">
              <summary>
                <ChevronDown size={14} />
                完整 SHA-256 与制品核对
              </summary>
              <div className="hash-table">
                {stages.map((s) => (
                  <section key={s.stage}>
                    <strong>Stage {s.stage}</strong>
                    <div>
                      <span>ELF</span>
                      <code>{s.binary_sha256}</code>
                      <CopyButton text={s.binary_sha256} />
                    </div>
                    <div>
                      <span>LLVM</span>
                      <code>{s.llvm_sha256}</code>
                      <CopyButton text={s.llvm_sha256} />
                    </div>
                    {!s.current_matches && (
                      <div className="hash-mismatch">
                        <CircleAlert size={13} />
                        <span>
                          当前 ELF：{s.current_binary_sha256 ?? '缺失'}
                          <br />
                          当前 LLVM：{s.current_llvm_sha256 ?? '缺失'}
                        </span>
                      </div>
                    )}
                  </section>
                ))}
              </div>
            </details>
          </>
        )}
      </section>
      <section className="dashboard-section performance-section">
        <div className="section-heading">
          <h2>
            <Clock3 size={17} />
            优化性能对比
          </h2>
          <div className="section-actions">
            <div className="segmented">
              {(
                [
                  ['runtime_ms', '运行时间'],
                  ['compile_ms', '编译时间'],
                  ['instructions', 'IR 指令'],
                ] as const
              ).map(([key, name]) => (
                <button
                  key={key}
                  className={metric === key ? 'selected' : ''}
                  onClick={() => setMetric(key)}
                >
                  {name}
                </button>
              ))}
            </div>
            <DownloadLink href="/api/records/benchmarks" label="下载性能测量数据" />
          </div>
        </div>
        {rows ? (
          <>
            <div className="benchmark-chart">
              <Chart option={plot} label="三个优化配置在四个程序上的性能对比" />
            </div>
            <div className="benchmark-caption">
              <span>
                A = 自定义优化 · L = LLVM 优化；预热 {data.records.benchmarks!.warmups} 次，测量{' '}
                {data.records.benchmarks!.measurements} 次，运行时间取中位数。
              </span>
              <label>
                查看样本
                <select value={benchmark} onChange={(e) => setBenchmark(e.target.value)}>
                  {[...new Set(rows.map((r) => r.benchmark))].map((n) => (
                    <option key={n}>{n}</option>
                  ))}
                </select>
              </label>
            </div>
            <div className="table-scroll">
              <table>
                <thead>
                  <tr>
                    <th>配置</th>
                    <th>IR 指令</th>
                    <th>基本块</th>
                    <th>编译 / ms</th>
                    <th>运行中位数 / ms</th>
                    <th>五次样本 / ms</th>
                    <th>结果</th>
                  </tr>
                </thead>
                <tbody>
                  {benchmarkRows.map((r) => (
                    <BenchmarkTableRow
                      key={r.configuration}
                      row={r}
                      equal={benchmarkRows.every(
                        (x) => x.output_sha256 === r.output_sha256 && x.stdout === r.stdout,
                      )}
                    />
                  ))}
                </tbody>
              </table>
            </div>
            <details className="hash-details">
              <summary>
                <ChevronDown size={14} />
                输出与校验值
              </summary>
              {benchmarkRows.map((r) => (
                <div className="benchmark-output" key={r.configuration}>
                  <Badge tone="blue">{r.configuration}</Badge>
                  <code>{r.stdout.trim()}</code>
                  <code className="hash-code">{r.output_sha256}</code>
                  <CopyButton text={r.output_sha256} />
                </div>
              ))}
            </details>
          </>
        ) : (
          <Empty title="缺少性能测量记录" />
        )}
      </section>
      <div className="dashboard-bottom">
        <section className="dashboard-section tests-section">
          <div className="section-heading">
            <h2>
              <TestTube2 size={17} />
              行为测试
            </h2>
            <div className="section-actions">
              <select
                aria-label="测试编译器"
                value={testCompiler}
                onChange={(e) => setTestCompiler(e.target.value as typeof testCompiler)}
              >
                <option value="tests-stage3">Stage 3</option>
                <option value="tests">Stage 0</option>
              </select>
              <DownloadLink href={'/api/records/' + testCompiler} label="下载测试记录" />
            </div>
          </div>
          {test ? (
            <>
              <div className="test-summary">
                <span>
                  <strong>{test.results.filter((r) => r.status === 'pass').length}</strong> /{' '}
                  {test.results.length} 通过
                </span>
                <Badge tone={test.results.every((r) => r.status === 'pass') ? 'good' : 'bad'}>
                  {test.results.every((r) => r.status === 'pass') ? '全部通过' : '存在失败'}
                </Badge>
                <input
                  aria-label="筛选测试"
                  placeholder="筛选测试名称…"
                  value={testQuery}
                  onChange={(e) => setTestQuery(e.target.value)}
                />
              </div>
              <div className="test-results">
                {tests.map((r, i) => (
                  <details key={i}>
                    <summary>
                      <CheckCircle2
                        size={13}
                        className={r.status === 'pass' ? 'text-good' : 'text-bad'}
                      />
                      <span>{r.case}</span>
                      {r.optimization !== undefined && <code>O{r.optimization}</code>}
                      <Badge tone={r.status === 'pass' ? 'good' : 'bad'}>{r.status}</Badge>
                    </summary>
                    <pre>{r.stdout || r.stderr || '该项未记录终端输出。'}</pre>
                  </details>
                ))}
                {!tests.length && <Empty title="没有匹配的测试" />}
              </div>
            </>
          ) : (
            <Empty title="缺少测试记录" />
          )}
        </section>
        <section className="dashboard-section evidence-section">
          <div className="section-heading">
            <h2>
              <ShieldCheck size={17} />
              验证与来源
            </h2>
          </div>
          <div className="evidence-row">
            <div>
              <strong>ASan / UBSan</strong>
              <p>种子编译自身源码 + C 运行时</p>
            </div>
            <Badge tone={data.records.sanitizers?.status === 'pass' ? 'good' : 'warn'}>
              {data.records.sanitizers?.status ?? '记录缺失'}
            </Badge>
          </div>
          <div className="evidence-row">
            <div>
              <strong>优化结构检查</strong>
              <p>{data.records.optimization?.verified.join(' / ') ?? '记录缺失'}</p>
            </div>
            <Badge tone={data.records.optimization?.status === 'pass' ? 'good' : 'warn'}>
              {data.records.optimization?.status ?? '未知'}
            </Badge>
          </div>
          <div className="evidence-row">
            <div>
              <strong>编译器源码来源</strong>
              <p>核对当前 worktree 与自举记录的源码摘要</p>
            </div>
            <Badge
              tone={
                data.provenance.length > 0 && data.provenance.every((p) => p.matches)
                  ? 'good'
                  : 'warn'
              }
            >
              {data.provenance.filter((p) => p.matches).length}/{data.provenance.length} 一致
            </Badge>
          </div>
          <details className="hash-details">
            <summary>
              <ChevronDown size={14} />
              源码逐文件校验
            </summary>
            {data.provenance.map((p) => (
              <div className="source-check" key={p.path}>
                <code>{p.path}</code>
                <Badge tone={p.matches ? 'good' : 'warn'}>
                  {p.matches ? '一致' : '不同或缺失'}
                </Badge>
              </div>
            ))}
          </details>
          <div className="record-files">
            {data.sources.map((s) => (
              <div key={s.key}>
                <div>
                  <strong>{s.key}</strong>
                  <p title={s.path}>
                    {s.modified
                      ? new Date(s.modified * 1000).toLocaleString('zh-CN', {
                          timeZone: 'Asia/Shanghai',
                          hour12: false,
                        })
                      : '文件缺失'}
                  </p>
                </div>
                {s.available ? (
                  <a href={'/api/records/' + s.key} download aria-label={'下载 ' + s.key}>
                    <Download size={14} />
                  </a>
                ) : (
                  <Badge tone="warn">缺失</Badge>
                )}
              </div>
            ))}
          </div>
          <details className="hash-details">
            <summary>
              <ChevronDown size={14} />
              工具链原始清单
            </summary>
            <pre className="toolchain-json">{JSON.stringify(data.records.toolchain, null, 2)}</pre>
          </details>
          <p className="evidence-footnote">
            <ExternalLink size={12} />
            数据目录：artifacts/
          </p>
        </section>
      </div>
    </div>
  );
}
function BenchmarkTableRow({ row: r, equal }: { row: BenchmarkRow; equal: boolean }) {
  return (
    <tr>
      <td>
        <code>{r.configuration}</code>
      </td>
      <td>{r.instructions}</td>
      <td>{r.blocks}</td>
      <td>{r.compile_ms.toFixed(3)}</td>
      <td className="benchmark-value">{r.runtime_ms.toFixed(3)}</td>
      <td>
        <code className="samples">{r.samples_ms.map((x) => x.toFixed(3)).join(' / ')}</code>
      </td>
      <td>
        <Badge tone={equal ? 'good' : 'bad'}>{equal ? '输出一致' : '输出不同'}</Badge>
      </td>
    </tr>
  );
}
