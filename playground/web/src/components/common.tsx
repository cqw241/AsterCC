import { AlertCircle, Check, Clipboard, Download, LoaderCircle, RefreshCw } from 'lucide-react';
import { useState } from 'react';
import { Button } from './ui/button';
import { Tooltip } from './ui/tooltip';
export function Spinner({ label = '正在读取…' }: { label?: string }) {
  return (
    <div className="state-message">
      <LoaderCircle size={20} className="spin" />
      <span>{label}</span>
    </div>
  );
}
export function Empty({ title, children }: { title: string; children?: React.ReactNode }) {
  return (
    <div className="empty-state">
      <div className="empty-mark">{'{ }'}</div>
      <strong>{title}</strong>
      {children && <p>{children}</p>}
    </div>
  );
}
export function ErrorState({ message, retry }: { message: string; retry?: () => void }) {
  return (
    <div className="error-state" role="alert">
      <AlertCircle size={18} />
      <div>{message}</div>
      {retry && (
        <Button size="sm" onClick={retry}>
          <RefreshCw size={13} />
          重试
        </Button>
      )}
    </div>
  );
}
export function CopyButton({ text, label = '复制内容' }: { text: string; label?: string }) {
  const [copied, setCopied] = useState(false);
  const [failed, setFailed] = useState(false);
  return (
    <Tooltip label={failed ? '复制失败，请手动选择文本' : copied ? '已复制' : label}>
      <Button
        size="icon"
        variant="ghost"
        aria-label={label}
        onClick={async () => {
          try {
            await navigator.clipboard.writeText(text);
            setCopied(true);
            setFailed(false);
            setTimeout(() => setCopied(false), 1500);
          } catch {
            setFailed(true);
          }
        }}
      >
        {copied ? <Check size={14} /> : <Clipboard size={14} />}
      </Button>
    </Tooltip>
  );
}
export function DownloadLink({ href, label = '下载原始数据' }: { href: string; label?: string }) {
  return (
    <Tooltip label={label}>
      <a
        className="ui-button ui-button-ghost ui-button-icon"
        href={href}
        download
        aria-label={label}
      >
        <Download size={14} />
      </a>
    </Tooltip>
  );
}
export function Badge({
  children,
  tone = 'neutral',
}: {
  children: React.ReactNode;
  tone?: 'neutral' | 'good' | 'warn' | 'bad' | 'blue';
}) {
  return <span className={'badge badge-' + tone}>{children}</span>;
}
export function formatTime(ns: number) {
  return ns >= 1000000 ? `${(ns / 1000000).toFixed(2)} ms` : `${(ns / 1000).toFixed(1)} μs`;
}
