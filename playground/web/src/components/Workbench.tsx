import { useEffect, useRef, useState } from 'react';
import Editor, { type OnMount } from '@monaco-editor/react';
import type { editor } from 'monaco-editor';
import { Panel, PanelGroup, PanelResizeHandle } from 'react-resizable-panels';
import {
  Braces,
  ChevronRight,
  CircleAlert,
  Download,
  FileCode2,
  FileInput,
  FileJson2,
  Link2,
  LoaderCircle,
  Play,
  RotateCcw,
  Square,
  Terminal,
  Trash2,
  Unlink2,
  Workflow,
  Zap,
} from 'lucide-react';
import { downloadText, errorMessage, loadSetting, saveSetting } from '../api';
import type { AST, Example, Job } from '../types';
import { editorOptions, monaco } from '../editor';
import { Button } from './ui/button';
import { Tooltip } from './ui/tooltip';
import { Badge, CopyButton, DownloadLink, Empty, ErrorState, Spinner } from './common';
interface Props {
  examples: Example[];
  source: string;
  setSource: (s: string) => void;
  selectedExample: string;
  chooseExample: (id: string) => void;
  optimization: number;
  setOptimization: (v: number) => void;
  llvm: number;
  setLlvm: (v: number) => void;
  linked: boolean;
  setLinked: (v: boolean) => void;
  argumentsText: string;
  setArgumentsText: (v: string) => void;
  job: Job | null;
  stale: boolean;
  busy: boolean;
  ready: boolean;
  error: string;
  layoutVersion: number;
  onCompile: () => void;
  onRun: () => void;
  onCancel: () => void;
  onExplore: () => void;
}
const statusLabels: Record<string, string> = {
  queued: '等待编译',
  compiling: '生成原生程序',
  collecting: '收集 AST',
  running: '执行中',
  ready: '编译成功',
  failed: '编译失败',
  cancelled: '已停止',
};
export default function Workbench(p: Props) {
  const [view, setView] = useState<'ast' | 'ssa' | 'llvm'>('ssa');
  const [mobileTab, setMobileTab] = useState<'source' | 'result' | 'terminal'>('source');
  const [terminalTab, setTerminalTab] = useState<'output' | 'diagnostics'>('output');
  const [content, setContent] = useState('');
  const [loading, setLoading] = useState(false);
  const [contentError, setContentError] = useState('');
  const [retry, setRetry] = useState(0);
  const [cleared, setCleared] = useState(0);
  const [fileError, setFileError] = useState('');
  const editorRef = useRef<editor.IStandaloneCodeEditor | null>(null);
  const fileRef = useRef<HTMLInputElement>(null);
  const jobId = p.job?.id;
  const hasArtifact = p.job?.artifacts.includes(view) ?? false;
  useEffect(() => {
    setCleared(0);
  }, [jobId]);
  useEffect(() => {
    if (!jobId || !hasArtifact) {
      setContent('');
      return;
    }
    const controller = new AbortController();
    setLoading(true);
    setContentError('');
    fetch(`/api/jobs/${jobId}/artifacts/${view}`, { signal: controller.signal })
      .then(async (r) => {
        if (!r.ok) throw new Error('制品读取失败');
        return r.text();
      })
      .then(setContent)
      .catch((e) => {
        if (!controller.signal.aborted) setContentError(errorMessage(e));
      })
      .finally(() => {
        if (!controller.signal.aborted) setLoading(false);
      });
    return () => controller.abort();
  }, [jobId, view, hasArtifact, retry]);
  useEffect(() => {
    const model = editorRef.current?.getModel();
    if (!model) return;
    monaco.editor.setModelMarkers(
      model,
      'astercc',
      p.stale
        ? []
        : (p.job?.diagnostics ?? [])
            .filter((d) => d.editable)
            .map((d) => ({
              severity: monaco.MarkerSeverity.Error,
              message: d.message,
              startLineNumber: d.line,
              endLineNumber: d.line,
              startColumn: d.column,
              endColumn: d.column + 1,
            })),
    );
  }, [p.job?.diagnostics, p.stale]);
  const mount: OnMount = (editor) => {
    editorRef.current = editor;
    const model = editor.getModel();
    if (model)
      monaco.editor.setModelMarkers(
        model,
        'astercc',
        p.stale
          ? []
          : (p.job?.diagnostics ?? [])
              .filter((d) => d.editable)
              .map((d) => ({
                severity: monaco.MarkerSeverity.Error,
                message: d.message,
                startLineNumber: d.line,
                endLineNumber: d.line,
                startColumn: d.column,
                endColumn: d.column + 1,
              })),
      );
  };
  const locate = (line: number, column: number) => {
    setMobileTab('source');
    setTimeout(() => {
      editorRef.current?.revealPositionInCenter({ lineNumber: line, column });
      editorRef.current?.setPosition({ lineNumber: line, column });
      editorRef.current?.focus();
    }, 50);
  };
  const example = p.examples.find((e) => e.id === p.selectedExample);
  const commands = p.job?.commands.slice(cleared) ?? [];
  const terminalText = commands.map((c) => `${c.stdout}${c.stderr}`).join('\n');
  return (
    <div className="workbench">
      <div className="workbench-toolbar">
        <div className="toolbar-options">
          <label className="example-picker">
            <span className="sr-only">选择示例</span>
            <FileCode2 size={15} />
            <select
              value={p.selectedExample}
              onChange={(e) => p.chooseExample(e.target.value)}
              disabled={!p.examples.length || p.busy}
            >
              {p.examples.map((e) => (
                <option key={e.id} value={e.id}>
                  {e.name}
                </option>
              ))}
            </select>
          </label>
          <div className="toolbar-divider" />
          <div className="optimization-switch" role="group" aria-label="Aster 优化级别">
            {[0, 1, 2].map((v) => (
              <button
                key={v}
                disabled={p.busy}
                aria-pressed={p.optimization === v}
                className={p.optimization === v ? 'selected' : ''}
                onClick={() => p.setOptimization(v)}
              >
                O{v}
              </button>
            ))}
          </div>
          <Tooltip
            label={p.linked ? 'LLVM 级别跟随 Aster；点击独立设置' : 'LLVM 级别独立；点击恢复跟随'}
          >
            <Button
              size="icon"
              variant="ghost"
              aria-label={p.linked ? '解除 LLVM 级别关联' : '关联 LLVM 级别'}
              onClick={() => {
                p.setLinked(!p.linked);
                if (!p.linked) p.setLlvm(p.optimization);
              }}
              disabled={p.busy}
            >
              {p.linked ? <Link2 size={14} /> : <Unlink2 size={14} />}
            </Button>
          </Tooltip>
          <label className="llvm-picker">
            LLVM
            <select
              aria-label="LLVM 优化级别"
              value={p.llvm}
              disabled={p.linked || p.busy}
              onChange={(e) => p.setLlvm(Number(e.target.value))}
            >
              {[0, 1, 2].map((v) => (
                <option key={v} value={v}>
                  O{v}
                </option>
              ))}
            </select>
          </label>
        </div>
        <div className="compile-actions">
          <Button
            variant="outline"
            onClick={p.onCompile}
            disabled={p.busy || !p.ready || !p.source.trim()}
          >
            <Zap size={14} />
            编译
          </Button>
          {p.busy ? (
            <Button variant="destructive" onClick={p.onCancel} disabled={!p.job}>
              <Square size={13} />
              停止
            </Button>
          ) : (
            <Button variant="default" onClick={p.onRun} disabled={!p.ready || !p.source.trim()}>
              <Play size={14} />
              运行<span className="shortcut">⌘ ↵</span>
            </Button>
          )}
        </div>
      </div>
      {(p.error || fileError) && (
        <div className="inline-error">
          <ErrorState message={p.error || fileError} />
        </div>
      )}
      <div className="mobile-tabs">
        {(['source', 'result', 'terminal'] as const).map((t) => (
          <button
            key={t}
            className={mobileTab === t ? 'selected' : ''}
            onClick={() => setMobileTab(t)}
          >
            {{ source: '源码', result: '编译结果', terminal: '终端' }[t]}
          </button>
        ))}
      </div>
      <div className={'workbench-panels mobile-' + mobileTab}>
        <PanelGroup direction="vertical" autoSaveId={'astercc-vertical-' + p.layoutVersion}>
          <Panel defaultSize={73} minSize={35}>
            <PanelGroup
              direction="horizontal"
              key={p.layoutVersion}
              onLayout={(sizes) => saveSetting('layout', sizes)}
            >
              <Panel
                defaultSize={loadSetting('layout', [48, 52])[0]}
                minSize={25}
                className="source-panel"
              >
                <section className="editor-section">
                  <div className="panel-header">
                    <div className="file-tab">
                      <FileCode2 size={15} />
                      <strong>main.aster</strong>
                      <span className="file-dot" title="保存在本地浏览器" />
                    </div>
                    <div className="panel-tools">
                      <Tooltip label="导入 Aster 文件">
                        <Button
                          variant="ghost"
                          size="icon"
                          aria-label="导入 Aster 文件"
                          onClick={() => fileRef.current?.click()}
                        >
                          <FileInput size={14} />
                        </Button>
                      </Tooltip>
                      <Tooltip label="下载源码">
                        <Button
                          variant="ghost"
                          size="icon"
                          aria-label="下载源码"
                          onClick={() => downloadText(p.source, 'main.aster')}
                        >
                          <Download size={14} />
                        </Button>
                      </Tooltip>
                      <Tooltip label="恢复当前示例">
                        <Button
                          variant="ghost"
                          size="icon"
                          aria-label="恢复当前示例"
                          disabled={!example || p.busy}
                          onClick={() => p.chooseExample(p.selectedExample)}
                        >
                          <RotateCcw size={14} />
                        </Button>
                      </Tooltip>
                    </div>
                  </div>
                  <input
                    ref={fileRef}
                    type="file"
                    accept=".aster,text/plain"
                    hidden
                    onChange={async (e) => {
                      const file = e.target.files?.[0];
                      if (file) {
                        if (file.size > 262144) {
                          setFileError('源文件不能超过 256 KiB。');
                        } else {
                          p.setSource(await file.text());
                          setFileError('');
                        }
                        e.target.value = '';
                      }
                    }}
                  />
                  <div className="editor-body">
                    <Editor
                      height="100%"
                      theme="aster-dark"
                      language="aster"
                      path="inmemory://main.aster"
                      value={p.source}
                      onChange={(v) => p.setSource(v ?? '')}
                      onMount={mount}
                      options={editorOptions}
                      loading={<Spinner label="加载编辑器…" />}
                    />
                  </div>
                  <div className="editor-caption">
                    <span>{example?.description ?? '编辑 Aster 源码，编译以查看IR。'}</span>
                    <span>{p.source.split('\n').length} 行</span>
                  </div>
                </section>
              </Panel>
              <PanelResizeHandle
                className="resize-handle horizontal"
                aria-label="调整源码与结果宽度"
              />
              <Panel
                defaultSize={loadSetting('layout', [48, 52])[1]}
                minSize={25}
                className="result-panel"
              >
                <section className="editor-section">
                  <div className="panel-header">
                    <div className="result-tabs" role="tablist" aria-label="编译制品">
                      {(['ast', 'ssa', 'llvm'] as const).map((t) => (
                        <button
                          role="tab"
                          aria-selected={view === t}
                          className={view === t ? 'selected' : ''}
                          key={t}
                          onClick={() => setView(t)}
                        >
                          {t === 'ast' ? (
                            <FileJson2 size={14} />
                          ) : t === 'ssa' ? (
                            <Workflow size={14} />
                          ) : (
                            <Braces size={14} />
                          )}
                          {{ ast: 'AST', ssa: 'SSA', llvm: 'LLVM IR' }[t]}
                        </button>
                      ))}
                    </div>
                    <div className="panel-tools">
                      {content && (
                        <>
                          <CopyButton text={content} />
                          <DownloadLink
                            href={`/api/jobs/${jobId}/artifacts/${view}?download=true`}
                            label="下载编译制品"
                          />
                        </>
                      )}
                      <Tooltip label="在优化探索中查看">
                        <Button
                          variant="ghost"
                          size="icon"
                          aria-label="打开优化探索"
                          onClick={p.onExplore}
                        >
                          <Workflow size={14} />
                        </Button>
                      </Tooltip>
                    </div>
                  </div>
                  {p.stale && (
                    <div className="stale-notice">源码或选项已修改，当前展示上次编译结果。</div>
                  )}
                  <div className="editor-body">
                    {loading ? (
                      <Spinner label="读取编译制品…" />
                    ) : contentError ? (
                      <ErrorState message={contentError} retry={() => setRetry((v) => v + 1)} />
                    ) : !content ? (
                      <Empty title={p.busy ? '编译正在进行' : '等待编译结果'}>
                        {p.busy
                          ? '完成后将在这里显示编译制品。'
                          : '编译源码，查看 AST、自定义 SSA 和 LLVM IR。'}
                      </Empty>
                    ) : view === 'ast' ? (
                      <ASTView content={content} onLocate={locate} source={p.job?.source ?? ''} />
                    ) : (
                      <Editor
                        key={view}
                        height="100%"
                        theme="aster-dark"
                        language={view === 'ssa' ? 'aster-ssa' : 'llvm-ir'}
                        value={content}
                        options={{ ...editorOptions, readOnly: true }}
                      />
                    )}
                  </div>
                  <div className="editor-caption">
                    <span>
                      {view === 'ssa'
                        ? 'AsterCC 自定义 SSA'
                        : view === 'llvm'
                          ? 'LLVM 18 文本 IR'
                          : '类型化 AST · 稳定节点 ID'}
                    </span>
                    {p.job && (
                      <Badge
                        tone={
                          p.job.status === 'ready'
                            ? 'good'
                            : p.job.status === 'failed'
                              ? 'bad'
                              : 'neutral'
                        }
                      >
                        {statusLabels[p.job.status] ?? p.job.status}
                      </Badge>
                    )}
                  </div>
                </section>
              </Panel>
            </PanelGroup>
          </Panel>
          <PanelResizeHandle className="resize-handle vertical" aria-label="调整终端高度" />
          <Panel defaultSize={27} minSize={15} maxSize={60} className="terminal-panel">
            <section className="terminal-section">
              <div className="panel-header">
                <div className="result-tabs">
                  <button
                    className={terminalTab === 'output' ? 'selected' : ''}
                    onClick={() => setTerminalTab('output')}
                  >
                    <Terminal size={14} />
                    终端
                  </button>
                  <button
                    className={terminalTab === 'diagnostics' ? 'selected' : ''}
                    onClick={() => setTerminalTab('diagnostics')}
                  >
                    <CircleAlert size={14} />
                    诊断 <span className="count">{p.job?.diagnostics.length ?? 0}</span>
                  </button>
                </div>
                <div className="panel-tools">
                  <label className="arguments-input">
                    参数
                    <input
                      aria-label="运行参数 JSON 数组"
                      value={p.argumentsText}
                      onChange={(e) => p.setArgumentsText(e.target.value)}
                      placeholder='["10000"]'
                      spellCheck={false}
                    />
                  </label>
                  <CopyButton text={terminalText} label="复制终端输出" />
                  <Tooltip label="清空终端">
                    <Button
                      size="icon"
                      variant="ghost"
                      aria-label="清空终端"
                      onClick={() => setCleared(p.job?.commands.length ?? 0)}
                    >
                      <Trash2 size={14} />
                    </Button>
                  </Tooltip>
                </div>
              </div>
              <div className="terminal-content" role="log" aria-live="polite">
                {terminalTab === 'diagnostics' ? (
                  p.job?.diagnostics.length ? (
                    <div className="diagnostic-list">
                      {p.job.diagnostics.map((d, i) => (
                        <button
                          key={i}
                          disabled={!d.editable || p.stale}
                          onClick={() => locate(d.line, d.column)}
                        >
                          <CircleAlert size={15} />
                          <span>{d.message}</span>
                          <code>
                            {d.file.split('/').at(-1)}:{d.line}:{d.column}
                          </code>
                        </button>
                      ))}
                    </div>
                  ) : (
                    <div className="terminal-muted">
                      {p.job?.status === 'failed'
                        ? '该错误没有源码位置，请查看终端。'
                        : '没有源码诊断。'}
                    </div>
                  )
                ) : (
                  <>
                    {!commands.length && !p.busy && (
                      <div className="terminal-muted">
                        <span className="prompt">❯</span>编译和执行输出会显示在这里。
                        <span className="terminal-hint">Ctrl / ⌘ + Enter 运行</span>
                      </div>
                    )}
                    {commands.map((c, i) => (
                      <div className="terminal-command" key={i}>
                        <div className="command-heading">
                          <span className="prompt">❯</span>
                          <span>
                            {c.command[0].endsWith('/program')
                              ? '执行 main.aster'
                              : c.command.includes('--emit=ast')
                                ? '生成 AST'
                                : '编译 main.aster'}
                            {c.command
                              .filter((a) => /^-O\d$|^--llvm-opt=/.test(a))
                              .map((a) => ' ' + a)}
                          </span>
                          <span className={c.returncode === 0 ? 'exit-good' : 'exit-bad'}>
                            exit {c.returncode} · {(c.seconds * 1000).toFixed(1)} ms
                          </span>
                        </div>
                        {c.stdout && <pre>{c.stdout}</pre>}
                        {c.stderr && (
                          <pre className="stderr">
                            {c.stderr.replaceAll(
                              /[^\s]*\/jobs\/[a-f0-9]+\/main\.aster/g,
                              'main.aster',
                            )}
                          </pre>
                        )}
                        {c.timed_out && (
                          <div className="stderr">已达到时间限制，进程组已终止。</div>
                        )}
                        {c.truncated && <div className="stderr">输出超过 1 MiB，程序已停止。</div>}
                        <details className="command-details">
                          <summary>实际命令</summary>
                          <code>{c.command.map((a) => JSON.stringify(a)).join(' ')}</code>
                        </details>
                      </div>
                    ))}
                    {p.busy && (
                      <div className="terminal-pending">
                        <LoaderCircle size={13} className="spin" />
                        {statusLabels[p.job?.status ?? 'queued']}…
                      </div>
                    )}
                    {p.job?.status === 'cancelled' && (
                      <p className="terminal-muted">任务已停止。</p>
                    )}
                  </>
                )}
              </div>
            </section>
          </Panel>
        </PanelGroup>
      </div>
    </div>
  );
}
function ASTView({
  content,
  onLocate,
  source,
}: {
  content: string;
  onLocate: (line: number, column: number) => void;
  source: string;
}) {
  const [mode, setMode] = useState<'tree' | 'json'>('tree');
  const [query, setQuery] = useState('');
  let ast: AST;
  try {
    ast = JSON.parse(content) as AST;
  } catch {
    return <ErrorState message="AST JSON 无法解析。" />;
  }
  const locate = (offset: number) => {
    const prefix = new TextDecoder().decode(new TextEncoder().encode(source).slice(0, offset));
    const lines = prefix.split('\n');
    onLocate(lines.length, lines.at(-1)!.length + 1);
  };
  return (
    <div className="ast-view">
      <div className="ast-toolbar">
        <div className="segmented">
          <button className={mode === 'tree' ? 'selected' : ''} onClick={() => setMode('tree')}>
            节点
          </button>
          <button className={mode === 'json' ? 'selected' : ''} onClick={() => setMode('json')}>
            JSON
          </button>
        </div>
        <input
          aria-label="搜索 AST 节点"
          placeholder="搜索节点、类型或名称…"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
        />
      </div>
      {mode === 'json' ? (
        <Editor
          height="100%"
          theme="aster-dark"
          language="json"
          value={JSON.stringify(ast, null, 2)}
          options={{ ...editorOptions, readOnly: true }}
        />
      ) : (
        <div className="ast-tree">
          <div className="ast-summary">
            {ast.functions.length} 个函数 · {ast.nodes.length} 个节点 · {ast.records.length}{' '}
            个结构体
          </div>
          {!query &&
            ast.functions.map((f, i) => (
              <details key={i} className="ast-function">
                <summary>
                  <ChevronRight size={13} />
                  <span className="token-fn">fn</span>
                  <strong>{f.name}</strong>
                  <code>→ {f.return}</code>
                </summary>
                <div className="ast-fields">
                  <code>body: #{f.body}</code>
                  {f.parameters.map((p) => (
                    <code key={p.name}>
                      {p.name}: {p.type}
                    </code>
                  ))}
                </div>
              </details>
            ))}
          {ast.nodes
            .filter(
              (n) =>
                !query ||
                `${n.id} ${n.tag} ${n.text} ${n.type}`.toLowerCase().includes(query.toLowerCase()),
            )
            .map((n) => (
              <details key={n.id} className="ast-node">
                <summary>
                  <ChevronRight size={12} />
                  <code className="ast-id">#{n.id}</code>
                  <span className="token-type">{n.tag}</span>
                  <span className="ast-node-text">{n.text}</span>
                  {n.type && <code>{n.type}</code>}
                </summary>
                <div className="ast-fields">
                  <code>
                    a: {n.a} · b: {n.b} · c: {n.c}
                  </code>
                  <code>children: [{n.children.join(', ')}]</code>
                  <code>
                    file: {n.file} · byte offset: {n.offset}
                  </code>
                  {n.file === 0 && (
                    <Button size="sm" variant="ghost" onClick={() => locate(n.offset)}>
                      定位源码
                    </Button>
                  )}
                </div>
              </details>
            ))}
        </div>
      )}
    </div>
  );
}
