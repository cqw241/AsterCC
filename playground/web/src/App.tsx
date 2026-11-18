import { lazy, Suspense, useCallback, useEffect, useRef, useState } from 'react';
import {
  Activity,
  ArrowUpRight,
  Blocks,
  CheckCircle2,
  Code2,
  FileCode2,
  GitBranch,
  Info,
  PanelLeftClose,
  RefreshCw,
  TerminalSquare,
} from 'lucide-react';
import type { Example, Health, Job } from './types';
import { api, errorMessage, loadSetting, post, saveSetting } from './api';
import { Button } from './components/ui/button';
import { Tooltip, TooltipProvider } from './components/ui/tooltip';
import { ErrorState, Spinner } from './components/common';
import Workbench from './components/Workbench';
const Explorer = lazy(() => import('./components/Explorer'));
const Dashboard = lazy(() => import('./components/Dashboard'));
type Page = 'workbench' | 'explorer' | 'bootstrap';
const initialPage = (): Page => {
  const p = location.hash.slice(1);
  return p === 'explorer' || p === 'bootstrap' ? p : 'workbench';
};
const busyStatuses = ['queued', 'compiling', 'collecting', 'running'];
export default function App() {
  const [page, setPage] = useState<Page>(initialPage);
  const [health, setHealth] = useState<Health | null>(null);
  const [examples, setExamples] = useState<Example[]>([]);
  const [source, setSource] = useState(() => loadSetting('source', ''));
  const [selectedExample, setSelectedExample] = useState(() =>
    loadSetting('example', 'optimization'),
  );
  const [optimization, setOptimization] = useState(() => loadSetting('optimization', 2));
  const [llvm, setLlvm] = useState(() => loadSetting('llvm', 2));
  const [linked, setLinked] = useState(() => loadSetting('linked', true));
  const [argumentsText, setArgumentsText] = useState(() => loadSetting('arguments', '[]'));
  const [job, setJob] = useState<Job | null>(null);
  const [requesting, setRequesting] = useState(false);
  const [error, setError] = useState('');
  const [bootError, setBootError] = useState('');
  const [showInfo, setShowInfo] = useState(false);
  const [layoutVersion, setLayoutVersion] = useState(0);
  const initialized = useRef(false);
  const busy = requesting || (!!job && busyStatuses.includes(job.status));
  const stale =
    !!job &&
    (job.source !== source || job.optimization !== optimization || job.llvm_optimization !== llvm);
  const refresh = useCallback(async () => {
    try {
      const [h, e] = await Promise.all([api<Health>('/health'), api<Example[]>('/examples')]);
      setHealth(h);
      setExamples(e);
      setBootError('');
    } catch (e) {
      setBootError(errorMessage(e));
    }
  }, []);
  useEffect(() => {
    void refresh();
  }, [refresh]);
  useEffect(() => {
    const handler = () => setPage(initialPage());
    window.addEventListener('hashchange', handler);
    return () => window.removeEventListener('hashchange', handler);
  }, []);
  useEffect(() => {
    saveSetting('source', source);
  }, [source]);
  useEffect(() => {
    saveSetting('optimization', optimization);
    saveSetting('llvm', llvm);
    saveSetting('linked', linked);
    saveSetting('arguments', argumentsText);
    saveSetting('example', selectedExample);
  }, [optimization, llvm, linked, argumentsText, selectedExample]);
  const waitForJob = async (current: Job): Promise<Job> => {
    setJob(current);
    while (busyStatuses.includes(current.status)) {
      await new Promise((resolve) => setTimeout(resolve, 200));
      current = await api<Job>(`/jobs/${current.id}`);
      setJob(current);
    }
    return current;
  };
  const compile = async (text: string, level: number, backend: number) =>
    waitForJob(
      await post<Job>('/jobs', { source: text, optimization: level, llvm_optimization: backend }),
    );
  const action = async (run = false, initialSource?: string) => {
    if (busy) return;
    let args: string[] = [];
    if (run) {
      try {
        const parsed: unknown = JSON.parse(argumentsText);
        if (!Array.isArray(parsed) || !parsed.every((v) => typeof v === 'string'))
          throw new Error();
        args = parsed;
      } catch {
        setError('运行参数需要是字符串数组，例如 ["10000"]。');
        return;
      }
    }
    setRequesting(true);
    setError('');
    try {
      let current = job;
      if (!run || !current || stale || !current.artifacts.includes('binary'))
        current = await compile(initialSource ?? source, optimization, llvm);
      if (run && current.status === 'ready')
        await waitForJob(await post<Job>(`/jobs/${current.id}/run`, { arguments: args }));
    } catch (e) {
      setError(errorMessage(e));
      setJob((previous) =>
        previous && busyStatuses.includes(previous.status)
          ? { ...previous, status: 'failed', error: '任务状态连接中断，请重新编译。' }
          : previous,
      );
    } finally {
      setRequesting(false);
    }
  };
  useEffect(() => {
    if (initialized.current || !examples.length || !health?.ready) return;
    initialized.current = true;
    const example = examples.find((e) => e.id === selectedExample) ?? examples[0];
    const initial = source || example.source;
    if (!source) {
      setSource(initial);
      setArgumentsText(JSON.stringify(example.arguments));
    }
    void action(false, initial);
    // Initialization runs once after the local compiler becomes available.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [examples, health]);
  const actionRef = useRef(action);
  actionRef.current = action;
  useEffect(() => {
    const key = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
        event.preventDefault();
        void actionRef.current(true);
      }
    };
    window.addEventListener('keydown', key);
    return () => window.removeEventListener('keydown', key);
  }, []);
  const chooseExample = (id: string) => {
    const e = examples.find((e) => e.id === id);
    if (!e) return;
    setSelectedExample(id);
    setSource(e.source);
    setArgumentsText(JSON.stringify(e.arguments));
    setError('');
  };
  const navigate = (next: Page) => {
    location.hash = next;
    setPage(next);
  };
  const cancel = async () => {
    if (job)
      try {
        setJob(await post<Job>(`/jobs/${job.id}/cancel`));
      } catch (e) {
        setError(errorMessage(e));
      }
  };
  return (
    <TooltipProvider delayDuration={250}>
      <div className="app-shell">
        <header className="app-header">
          <a
            href="#workbench"
            className="brand"
            onClick={() => navigate('workbench')}
            aria-label="AsterCC 首页"
          >
            <svg viewBox="0 0 32 32" aria-hidden="true">
              <path d="m16 2 3.8 10.2L30 16l-10.2 3.8L16 30l-3.8-10.2L2 16l10.2-3.8Z" />
            </svg>
            <span>
              Aster<span className="brand-light">CC</span>
            </span>
            <span className="brand-tag">Playground</span>
          </a>
          <nav className="main-nav" aria-label="工作区">
            <button
              aria-label="工作台"
              className={page === 'workbench' ? 'active' : ''}
              onClick={() => navigate('workbench')}
            >
              <Code2 size={16} />
              <span>工作台</span>
            </button>
            <button
              aria-label="优化探索"
              className={page === 'explorer' ? 'active' : ''}
              onClick={() => navigate('explorer')}
            >
              <Blocks size={16} />
              <span>优化探索</span>
            </button>
            <button
              aria-label="自举仪表盘"
              className={page === 'bootstrap' ? 'active' : ''}
              onClick={() => navigate('bootstrap')}
            >
              <GitBranch size={16} />
              <span>自举仪表盘</span>
            </button>
          </nav>
          <div className="header-actions">
            <span className={'connection ' + (health?.ready ? 'connected' : '')}>
              <i />
              {health?.ready ? '本地编译器就绪' : '等待编译器'}
            </span>
            <Tooltip label="工具链与快捷键">
              <Button
                size="icon"
                variant="ghost"
                aria-label="工具链与快捷键"
                onClick={() => setShowInfo(!showInfo)}
              >
                <Info size={17} />
              </Button>
            </Tooltip>
          </div>
        </header>
        {showInfo && (
          <aside className="info-panel">
            <div>
              <strong>{health?.version ?? '编译器未连接'}</strong>
              <p>本地编译 · 原生执行 · 自定义 SSA</p>
            </div>
            <code>{health?.compiler ?? '正在连接…'}</code>
            <p>
              <kbd>Ctrl / ⌘</kbd> + <kbd>Enter</kbd>{' '}
              编译运行；分栏可拖动，聚焦分隔条后使用方向键调整。
            </p>
            <p>运行参数使用 JSON 字符串数组。停止与超时会终止当前任务的进程组。</p>
            <Button
              size="sm"
              onClick={() => {
                setLayoutVersion((v) => v + 1);
                saveSetting('layout', [48, 52]);
              }}
            >
              <PanelLeftClose size={14} />
              重置布局
            </Button>
            <Button size="sm" variant="ghost" onClick={() => void refresh()}>
              <RefreshCw size={14} />
              刷新工具链状态
            </Button>
          </aside>
        )}
        {bootError && (
          <div className="global-notice">
            <ErrorState message={'本地服务连接失败：' + bootError} retry={() => void refresh()} />
          </div>
        )}
        {health && !health.ready && (
          <div className="global-notice">
            <ErrorState
              message="编译器、运行时或 LLVM 缺失。编辑器仍可使用。"
              retry={() => void refresh()}
            />
            <code>{health.repair_command}</code>
          </div>
        )}
        <main className={'main-content page-' + page}>
          {page === 'workbench' ? (
            <Workbench
              {...{
                examples,
                source,
                setSource,
                selectedExample,
                chooseExample,
                optimization,
                llvm,
                linked,
                setLinked,
                argumentsText,
                setArgumentsText,
                job,
                stale,
                busy,
                error,
                layoutVersion,
              }}
              ready={!!health?.ready}
              setOptimization={(v) => {
                setOptimization(v);
                if (linked) setLlvm(v);
              }}
              setLlvm={setLlvm}
              onCompile={() => void action()}
              onRun={() => void action(true)}
              onCancel={() => void cancel()}
              onExplore={() => navigate('explorer')}
            />
          ) : (
            <Suspense fallback={<Spinner label="正在加载工作区…" />}>
              {page === 'explorer' ? (
                <Explorer job={job} stale={stale} onWorkbench={() => navigate('workbench')} />
              ) : (
                <Dashboard />
              )}
            </Suspense>
          )}
        </main>
        <footer className="status-bar">
          <div>
            <Activity size={12} />
            <span>
              {busy
                ? '任务处理中'
                : job?.status === 'failed'
                  ? '编译失败'
                  : job?.status === 'cancelled'
                    ? '任务已停止'
                    : '就绪'}
            </span>
            {stale && <span className="status-warning">结果对应上次编译</span>}
          </div>
          <div className="status-detail">
            <FileCode2 size={12} />
            <span>Aster</span>
            <span>UTF-8</span>
            <CheckCircle2 size={12} />
            <span>SSA 验证</span>
            <span className="status-version">LLVM 18</span>
            <a href="#bootstrap" onClick={() => navigate('bootstrap')}>
              实验记录
              <ArrowUpRight size={11} />
            </a>
          </div>
          <span className="mobile-status">
            <TerminalSquare size={12} />
            本地执行
          </span>
        </footer>
      </div>
    </TooltipProvider>
  );
}
