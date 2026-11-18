import { loader } from '@monaco-editor/react';
import * as monaco from 'monaco-editor/editor/editor.api.js';
import 'monaco-editor/editor/editor.main.js';
import EditorWorker from 'monaco-editor/editor/editor.worker.js?worker';
import JsonWorker from 'monaco-editor/language/json/json.worker.js?worker';
self.MonacoEnvironment = {
  getWorker: (_moduleId, label) => (label === 'json' ? new JsonWorker() : new EditorWorker()),
};
loader.config({ monaco });
monaco.languages.register({ id: 'aster' });
monaco.languages.setMonarchTokensProvider('aster', {
  keywords: [
    'fn',
    'return',
    'let',
    'var',
    'if',
    'else',
    'while',
    'break',
    'continue',
    'struct',
    'enum',
    'import',
    'as',
    'true',
    'false',
  ],
  typeKeywords: ['i64', 'u8', 'bool', 'str', 'void'],
  tokenizer: {
    root: [
      [
        /[a-zA-Z_]\w*/,
        { cases: { '@keywords': 'keyword', '@typeKeywords': 'type', '@default': 'identifier' } },
      ],
      [/\/\*/, 'comment', '@comment'],
      [/\/\/.*$/, 'comment'],
      [/"/, 'string', '@string'],
      [/0x[\da-fA-F]+|\d+/, 'number'],
      [/[{}()[\]]/, '@brackets'],
      [/[;,.]/, 'delimiter'],
      [/[-+*/%<>=!&|^~:]+/, 'operator'],
    ],
    comment: [
      [/[^/*]+/, 'comment'],
      [/\*\//, 'comment', '@pop'],
      [/[/*]/, 'comment'],
    ],
    string: [
      [/[^\\"]+/, 'string'],
      [/\\./, 'string.escape'],
      [/"/, 'string', '@pop'],
    ],
  },
});
monaco.languages.setLanguageConfiguration('aster', {
  comments: { lineComment: '//', blockComment: ['/*', '*/'] },
  brackets: [
    ['{', '}'],
    ['[', ']'],
    ['(', ')'],
  ],
  autoClosingPairs: [
    { open: '{', close: '}' },
    { open: '[', close: ']' },
    { open: '(', close: ')' },
    { open: '"', close: '"' },
  ],
});
monaco.languages.register({ id: 'aster-ssa' });
monaco.languages.setMonarchTokensProvider('aster-ssa', {
  tokenizer: {
    root: [
      [/^function .+$/, 'keyword'],
      [/\bb\d+\b/, 'type'],
      [/%\d+/, 'variable'],
      [/\b(phi|branch|jump|return|const|arg)\b/, 'keyword'],
      [/\b(i64|u8|bool|str|void)\b/, 'type'],
      [/\b\d+\b/, 'number'],
      [/".*?"/, 'string'],
    ],
  },
});
monaco.languages.register({ id: 'llvm-ir' });
monaco.languages.setMonarchTokensProvider('llvm-ir', {
  tokenizer: {
    root: [
      [/;.*$/, 'comment'],
      [/@[-\w.]+/, 'type'],
      [/%[-\w.]+/, 'variable'],
      [
        /\b(define|declare|ret|br|phi|call|load|store|getelementptr|add|mul|icmp|select)\b/,
        'keyword',
      ],
      [/\b(i\d+|void|ptr)\b/, 'type'],
      [/".*?"/, 'string'],
      [/\b\d+\b/, 'number'],
    ],
  },
});
monaco.editor.defineTheme('aster-dark', {
  base: 'vs-dark',
  inherit: true,
  rules: [
    { token: 'keyword', foreground: 'A6B9FF' },
    { token: 'type', foreground: '81C9C0' },
    { token: 'number', foreground: 'E6B886' },
    { token: 'string', foreground: 'B9CE99' },
    { token: 'comment', foreground: '77869C' },
    { token: 'variable', foreground: '9FC4FF' },
  ],
  colors: {
    'editor.background': '#171C25',
    'editor.foreground': '#D5DDEA',
    'editorLineNumber.foreground': '#62718A',
    'editorLineNumber.activeForeground': '#BCCBE0',
    'editor.selectionBackground': '#31446B',
    'editor.lineHighlightBackground': '#202734',
    'editorCursor.foreground': '#A7C5FF',
    'editorWidget.background': '#222B39',
    'editorGutter.background': '#171C25',
    'diffEditor.insertedTextBackground': '#397D5838',
    'diffEditor.removedTextBackground': '#A1435338',
    'diffEditor.insertedLineBackground': '#23483430',
    'diffEditor.removedLineBackground': '#572A3530',
    'editorOverviewRuler.border': '#00000000',
  },
});
export const editorOptions: monaco.editor.IStandaloneEditorConstructionOptions = {
  fontFamily: '"JetBrains Mono", monospace',
  fontSize: 13,
  lineHeight: 23,
  minimap: { enabled: false },
  scrollBeyondLastLine: false,
  padding: { top: 16, bottom: 16 },
  smoothScrolling: true,
  automaticLayout: true,
  renderLineHighlight: 'line',
  overviewRulerBorder: false,
  tabSize: 4,
  wordWrap: 'off',
  bracketPairColorization: { enabled: true },
  fixedOverflowWidgets: true,
};
export { monaco };
