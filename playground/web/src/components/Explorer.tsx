import { useEffect, useMemo, useRef, useState } from 'react';
import { Editor } from '@monaco-editor/react';
import type { editor } from 'monaco-editor';
import { Panel, PanelGroup, PanelResizeHandle } from 'react-resizable-panels';
import {
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  ChevronLeft,
  ChevronRight,
  FileDiff,
  GitCompareArrows,
  Network,
  RefreshCw,
  Workflow,
} from 'lucide-react';
import type { Job, Snapshot, TraceDataset, TraceFunction } from '../types';
import { api, errorMessage } from '../api';
import { editorOptions } from '../editor';
import { compareSnapshots } from '../irChanges';
import { Button } from './ui/button';
import { Badge, CopyButton, Empty, ErrorState, formatTime, Spinner } from './common';
import Chart from './Chart';
import CFG from './CFG';
import SafeDiff from './SafeDiff';
const PASS_INFO: Record<string, { name: string; description: string }> = {
  input: {
    name: '输入 SSA',
    description: 'sealed-block 构造后的输入快照；后续 Pass 从这里开始。',
  },
  simplify: {
    name: '常量折叠与代数化简',
    description: '对可确定的常量运算和代数恒等式进行替换，遵循 Aster 整数语义。',
  },
  sccp: {
    name: '稀疏条件常量传播',
    description:
      '联合传播值格与可执行边，在常量条件下排除不可执行分支。',
  },
  cfg: { name: '控制流简化', description: '清理不可达块、修正 phi 前驱并合并满足条件的基本块。' },
  dce: {
    name: '死代码消除',
    description: '删除没有活跃使用的计算，保留调用、存储及可能终止程序的操作。',
  },
  gvn: {
    name: '全局值编号',
    description: '沿支配树查找等价纯运算，以支配当前指令的已有值替换重复计算。',
  },
  licm: {
    name: '循环不变量外提',
    description: '识别自然循环，将可安全提前执行的循环不变纯运算移到唯一前置块。指令数可能不变。',
  },
};
export default function Explorer({
  job,
  stale,
  onWorkbench,
}: {
  job: Job | null;
  stale: boolean;
  onWorkbench: () => void;
}) {
  const currentAvailable = !!job?.artifacts.includes('ssa');
  const [dataset, setDataset] = useState(
    currentAvailable ? `job-${job!.id}` : 'recorded-optimization',
  );
  const [datasets, setDatasets] = useState<TraceDataset[]>([]);
  const [functions, setFunctions] = useState<TraceFunction[]>([]);
  const [fid, setFid] = useState(0);
  const [passIndex, setPassIndex] = useState(0);
  const [beforeIndex, setBeforeIndex] = useState(0);
  const [before, setBefore] = useState<Snapshot | null>(null);
  const [after, setAfter] = useState<Snapshot | null>(null);
  const [view, setView] = useState<'diff' | 'graph'>('diff');
  const [sideBySide, setSideBySide] = useState(true);
  const [loading, setLoading] = useState(true);
  const [loadingSnapshot, setLoadingSnapshot] = useState(false);
  const [error, setError] = useState('');
  const [retry, setRetry] = useState(0);
  const codeRef = useRef<editor.IStandaloneCodeEditor | null>(null);
  const diffRef = useRef<editor.IStandaloneDiffEditor | null>(null);
  useEffect(() => {
    api<TraceDataset[]>('/traces')
      .then(setDatasets)
      .catch((e) => setError(errorMessage(e)));
  }, [retry]);
  useEffect(() => {
    let active = true;
    setLoading(true);
    setError('');
    setBefore(null);
    setAfter(null);
    api<TraceFunction[]>(`/traces/${dataset}/functions`)
      .then((fs) => {
        if (active) {
          setFunctions(fs);
          setFid(fs[0]?.id ?? 0);
          const snaps = fs[0]?.snapshots ?? [];
          const first = snaps.findIndex((s) => s.pass === 'sccp' && s.stats?.changed_instructions);
          const index = first >= 0 ? first : Math.min(1, snaps.length - 1);
          setPassIndex(Math.max(index, 0));
          setBeforeIndex(Math.max(index - 1, 0));
        }
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
  }, [dataset, retry]);
  const fn = functions.find((f) => f.id === fid);
  const selected = fn?.snapshots[passIndex];
  const previous = fn?.snapshots[beforeIndex];
  useEffect(() => {
    if (!selected || !previous) return;
    let active = true;
    setLoadingSnapshot(true);
    setError('');
    Promise.all([
      api<Snapshot>(`/traces/${dataset}/snapshots/${previous.key}`),
      api<Snapshot>(`/traces/${dataset}/snapshots/${selected.key}`),
    ])
      .then(([b, a]) => {
        if (active) {
          setBefore(b);
          setAfter(a);
        }
      })
      .catch((e) => {
        if (active) setError(errorMessage(e));
      })
      .finally(() => {
        if (active) setLoadingSnapshot(false);
      });
    return () => {
      active = false;
    };
  }, [dataset, selected, previous, retry]);
  const choosePass = (index: number) => {
    setPassIndex(index);
    setBeforeIndex(Math.max(0, index - 1));
  };
  const changes = useMemo(
    () => (before && after ? compareSnapshots(before, after) : []),
    [before, after],
  );
  const plot = useMemo(
    () => ({
      grid: { left: 34, right: 12, top: 16, bottom: 25 },
      tooltip: { trigger: 'axis' },
      xAxis: {
        type: 'category',
        data: fn?.snapshots.map((s) => String(s.order)) ?? [],
        axisLabel: { color: '#8798B1', fontSize: 10 },
        axisLine: { lineStyle: { color: '#354156' } },
      },
      yAxis: {
        type: 'value',
        min: 0,
        splitLine: { lineStyle: { color: '#2B3443' } },
        axisLabel: { color: '#8798B1', fontSize: 10 },
      },
      series: [
        {
          type: 'line',
          data:
            fn?.snapshots.map((s, i) =>
              i === 0
                ? (fn.snapshots[1]?.stats?.before ?? after?.instruction_count ?? 0)
                : (s.stats?.after ?? 0),
            ) ?? [],
          symbolSize: 5,
          lineStyle: { color: '#79A8FF', width: 2 },
          itemStyle: { color: '#79A8FF' },
          areaStyle: { color: '#79A8FF', opacity: 0.06 },
        },
      ],
    }),
    [fn, after?.instruction_count],
  );
  const info = PASS_INFO[selected?.pass ?? 'input'];
  const selectedStats = selected?.stats;
  const currentDataset: TraceDataset | undefined = currentAvailable
    ? {
        id: `job-${job!.id}`,
        name: '当前编译 · main.aster',
        available: true,
        source: '实时任务制品',
      }
    : undefined;
  const allDatasets = [...(currentDataset ? [currentDataset] : []), ...datasets];
  return (
    <div className="explorer-page">
      <div className="page-heading">
        <div>
          <div className="page-eyebrow">
            <Workflow size={14} />
            编译追踪
          </div>
          <h1>
            优化探索<span>Optimization Explorer</span>
          </h1>
        </div>
        <div className="page-heading-actions">
          <label className="dataset-picker">
            <span className="sr-only">追踪数据集</span>
            <select value={dataset} onChange={(e) => setDataset(e.target.value)}>
              {!allDatasets.some((d) => d.id === dataset) && (
                <option value={dataset}>先前编译任务</option>
              )}
              {allDatasets.map((d) => (
                <option key={d.id} value={d.id} disabled={!d.available}>
                  {d.name}
                  {!d.available ? '（缺失）' : ''}
                </option>
              ))}
            </select>
          </label>
          <Button
            variant="ghost"
            size="icon"
            aria-label="刷新追踪数据"
            onClick={() => setRetry((v) => v + 1)}
          >
            <RefreshCw size={15} />
          </Button>
          <Button variant="outline" size="sm" onClick={onWorkbench}>
            <ArrowLeft size={14} />
            工作台
          </Button>
        </div>
      </div>
      {stale && dataset.startsWith('job-') && (
        <div className="stale-notice">当前追踪来自上次编译，源码或选项已有修改。</div>
      )}
      {error ? (
        <ErrorState message={error} retry={() => setRetry((v) => v + 1)} />
      ) : loading ? (
        <Spinner label="索引追踪文件…" />
      ) : !fn ? (
        <Empty title="没有追踪快照">切换数据集，或在工作台完成一次编译。</Empty>
      ) : (
        <div className="explorer-layout">
          <aside className="pass-sidebar">
            <div className="sidebar-function">
              <label htmlFor="function-select">函数</label>
              <select
                id="function-select"
                value={fid}
                onChange={(e) => {
                  const f = functions.find((f) => f.id === Number(e.target.value))!;
                  setFid(f.id);
                  choosePass(Math.min(1, f.snapshots.length - 1));
                }}
              >
                {functions.map((f) => (
                  <option key={f.id} value={f.id}>
                    {f.name}
                  </option>
                ))}
              </select>
              <span>
                {functions.length} 个函数 · {fn.snapshots.length - 1} 个 Pass
              </span>
            </div>
            <div className="pass-list" aria-label="Pass 时间线">
              {fn.snapshots.map((s, i) => (
                <button
                  key={s.key}
                  className={'pass-step ' + (i === passIndex ? 'active' : '')}
                  onClick={() => choosePass(i)}
                >
                  <span className="pass-number">{String(s.order).padStart(2, '0')}</span>
                  <div>
                    <strong>{s.pass === 'input' ? '输入 SSA' : s.pass}</strong>
                    <span>
                      {s.stats ? `${s.stats.before} → ${s.stats.after} instructions` : '构造完成'}
                    </span>
                  </div>
                  {s.stats && s.stats.changed_instructions > 0 && (
                    <i className="change-indicator" title="发生变换" />
                  )}
                </button>
              ))}
            </div>
            <div className="pass-chart">
              <div>
                指令数变化<span>Pass 序号</span>
              </div>
              <Chart option={plot} label="逐 Pass 指令数量曲线" />
              <p>不计 arg / const / string 及终结指令。</p>
            </div>
          </aside>
          <section className="pass-content">
            <div className="pass-description">
              <div>
                <Badge tone="blue">{selected?.pass}</Badge>
                <h2>{info?.name ?? selected?.pass}</h2>
                <span className="pass-stage">#{selected?.order}</span>
              </div>
              <p>{info?.description}</p>
              <div className="pass-metrics">
                <span>
                  指令{' '}
                  <strong>
                    {before?.instruction_count ?? '—'}
                    <ArrowRight size={12} />
                    {after?.instruction_count ?? '—'}
                  </strong>
                </span>
                <span>
                  变化记录 <strong>{selectedStats?.changed_instructions ?? 0}</strong>
                </span>
                <span>
                  Pass 耗时 <strong>{selectedStats ? formatTime(selectedStats.ns) : '—'}</strong>
                </span>
                <span className="analysis-metric">
                  依赖 <code>{selectedStats?.requires ?? 'none'}</code>
                </span>
                <span className="analysis-metric">
                  失效 <code>{selectedStats?.invalidates ?? 'none'}</code>
                </span>
              </div>
            </div>
            <div className="trace-toolbar">
              <div className="result-tabs">
                <button
                  className={view === 'diff' ? 'selected' : ''}
                  onClick={() => setView('diff')}
                >
                  <FileDiff size={14} />
                  IR Diff
                </button>
                <button
                  className={view === 'graph' ? 'selected' : ''}
                  onClick={() => setView('graph')}
                >
                  <Network size={14} />
                  CFG / SSA
                </button>
              </div>
              <div className="compare-controls">
                <label>
                  <span className="sr-only">对比起点</span>
                  <select
                    value={beforeIndex}
                    onChange={(e) => setBeforeIndex(Number(e.target.value))}
                  >
                    {fn.snapshots.map((s, i) => (
                      <option key={s.key} value={i}>
                        {s.order}: {s.pass}
                      </option>
                    ))}
                  </select>
                </label>
                <ArrowRight size={13} />
                <label>
                  <span className="sr-only">对比终点</span>
                  <select value={passIndex} onChange={(e) => setPassIndex(Number(e.target.value))}>
                    {fn.snapshots.map((s, i) => (
                      <option key={s.key} value={i}>
                        {s.order}: {s.pass}
                      </option>
                    ))}
                  </select>
                </label>
                <Button
                  size="icon"
                  variant="ghost"
                  aria-label="上一个 Pass"
                  disabled={passIndex === 0}
                  onClick={() => choosePass(passIndex - 1)}
                >
                  <ChevronLeft size={15} />
                </Button>
                <Button
                  size="icon"
                  variant="ghost"
                  aria-label="下一个 Pass"
                  disabled={passIndex === fn.snapshots.length - 1}
                  onClick={() => choosePass(passIndex + 1)}
                >
                  <ChevronRight size={15} />
                </Button>
              </div>
            </div>
            <div className="trace-body">
              {loadingSnapshot ? (
                <Spinner label="读取 SSA 快照…" />
              ) : before && after ? (
                <>
                  {view === 'diff' ? (
                    <div className="diff-container">
                      <div className="diff-labels">
                        <span>
                          {previous?.order}: {previous?.pass}
                        </span>
                        <span>
                          {selected?.order}: {selected?.pass}
                        </span>
                        <Button size="sm" variant="ghost" onClick={() => setSideBySide((v) => !v)}>
                          <GitCompareArrows size={13} />
                          {sideBySide ? '并排' : '行内'}
                        </Button>
                        <CopyButton text={after.text} />
                      </div>
                      <SafeDiff
                        original={before.text}
                        modified={after.text}
                        sideBySide={sideBySide}
                        onMount={(editor) => {
                          diffRef.current = editor;
                        }}
                      />
                    </div>
                  ) : (
                    <PanelGroup direction="horizontal">
                      <Panel defaultSize={65} minSize={35}>
                        <CFG
                          snapshot={after}
                          before={before}
                          onLine={(line) => {
                            codeRef.current?.revealLineInCenter(line);
                            codeRef.current?.setSelection({
                              startLineNumber: line,
                              endLineNumber: line,
                              startColumn: 1,
                              endColumn: 1000,
                            });
                          }}
                        />
                      </Panel>
                      <PanelResizeHandle
                        className="resize-handle horizontal"
                        aria-label="调整图与 SSA 宽度"
                      />
                      <Panel defaultSize={35} minSize={20}>
                        <div className="graph-code">
                          <div className="panel-header">
                            <span>变换后 SSA</span>
                            <CopyButton text={after.text} />
                          </div>
                          <Editor
                            height="100%"
                            theme="aster-dark"
                            language="aster-ssa"
                            value={after.text}
                            onMount={(editor) => {
                              codeRef.current = editor;
                            }}
                            options={{ ...editorOptions, readOnly: true, fontSize: 12 }}
                          />
                        </div>
                      </Panel>
                    </PanelGroup>
                  )}
                </>
              ) : (
                <Empty title="选择一个快照" />
              )}
            </div>
            <div className="change-summary">
              <div>
                <ArrowDown size={13} />
                <strong>快照差异</strong>
                <span>{changes.length} 条值变化</span>
              </div>
              <div className="change-items">
                {changes.length ? (
                  changes.map((c) => (
                    <button
                      key={c.id}
                      onClick={() => {
                        const line = after?.blocks
                          .flatMap((b) => b.instructions)
                          .find((i) => i.id === c.id)?.line;
                        if (line) {
                          if (view === 'diff')
                            diffRef.current?.getModifiedEditor().revealLineInCenter(line);
                          else codeRef.current?.revealLineInCenter(line);
                        }
                      }}
                      title={c.after ?? c.before}
                    >
                      <code>{c.id}</code>
                      <Badge
                        tone={c.kind === 'removed' ? 'bad' : c.kind === 'moved' ? 'blue' : 'good'}
                      >
                        {
                          { removed: '删除', added: '新增', modified: '修改', moved: '移动' }[
                            c.kind
                          ]
                        }
                      </Badge>
                      {c.from && c.to && c.from !== c.to && (
                        <span>
                          {c.from} → {c.to}
                        </span>
                      )}
                    </button>
                  ))
                ) : (
                  <span className="muted">
                    这两个快照中的 SSA 值没有变化。控制流变化请在 Diff 或图中查看。
                  </span>
                )}
              </div>
            </div>
          </section>
        </div>
      )}
    </div>
  );
}
