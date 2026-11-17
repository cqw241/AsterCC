import { useEffect, useLayoutEffect, useRef } from 'react';
import type { editor } from 'monaco-editor';
import { editorOptions, monaco } from '../editor';
/** Reset the widget before disposing its models; this ordering matters in Monaco. */
export default function SafeDiff({
  original,
  modified,
  sideBySide,
  onMount,
}: {
  original: string;
  modified: string;
  sideBySide: boolean;
  onMount: (instance: editor.IStandaloneDiffEditor) => void;
}) {
  const host = useRef<HTMLDivElement>(null);
  const instance = useRef<editor.IStandaloneDiffEditor | null>(null);
  const models = useRef<{ original: editor.ITextModel; modified: editor.ITextModel } | null>(null);
  const mount = useRef(onMount);
  mount.current = onMount;
  useLayoutEffect(() => {
    const diff = monaco.editor.createDiffEditor(host.current!, {
      ...editorOptions,
      theme: 'aster-dark',
      readOnly: true,
      originalEditable: false,
      renderOverviewRuler: false,
      renderSideBySide: sideBySide,
      enableSplitViewResizing: true,
    });
    const pair = {
      original: monaco.editor.createModel(original, 'aster-ssa'),
      modified: monaco.editor.createModel(modified, 'aster-ssa'),
    };
    diff.setModel(pair);
    instance.current = diff;
    models.current = pair;
    mount.current(diff);
    return () => {
      diff.setModel(null);
      diff.dispose();
      pair.original.dispose();
      pair.modified.dispose();
      instance.current = null;
      models.current = null;
    };
    // The widget owns its models for its mounted lifetime; contents update below.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);
  useEffect(() => {
    models.current?.original.setValue(original);
    models.current?.modified.setValue(modified);
  }, [original, modified]);
  useEffect(() => {
    instance.current?.updateOptions({ renderSideBySide: sideBySide });
  }, [sideBySide]);
  return <div ref={host} style={{ flex: 1, minHeight: 0 }} />;
}
