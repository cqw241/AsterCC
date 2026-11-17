import { useEffect, useMemo, useState } from 'react';
import {
  ReactFlow,
  Background,
  Controls,
  Handle,
  Position,
  useNodesState,
  useEdgesState,
  MarkerType,
} from '@xyflow/react';
import type { Edge, Node, NodeProps } from '@xyflow/react';
import dagre from '@dagrejs/dagre';
import '@xyflow/react/dist/style.css';
import type { Block, Snapshot } from '../types';
import { compareSnapshots } from '../irChanges';
import { Badge } from './common';
import { Button } from './ui/button';
type BlockNode = Node<
  {
    block: Block;
    selected: string;
    related: Set<string>;
    changes: Map<string, string>;
    onSelect: (value: string, line: number) => void;
  },
  'block'
>;
function BlockCard({ data, selected }: NodeProps<BlockNode>) {
  const b = data.block;
  return (
    <div className={'cfg-block ' + (selected ? 'cfg-block-selected' : '')}>
      <Handle type="target" position={Position.Top} />
      <div className="cfg-block-header">
        <strong>{b.id}</strong>
        <span>{b.instructions.length} values</span>
      </div>
      <div className="cfg-instructions">
        {b.instructions.map((i) => (
          <button
            key={i.id}
            className={
              'nodrag ' +
              (data.selected === i.id
                ? 'value-selected '
                : data.related.has(i.id)
                  ? 'value-related '
                  : '') +
              (data.changes.get(i.id) ?? '')
            }
            title={i.text}
            onClick={() => data.onSelect(i.id, i.line)}
          >
            <code>
              <span>{i.id}</span>
              <span className="cfg-type">:{i.type}</span> = {i.op}{' '}
              <span className="cfg-operands">
                {i.text.slice(i.text.indexOf(' = ') + 3 + i.op.length).trim()}
              </span>
            </code>
          </button>
        ))}
      </div>
      <div className="cfg-terminator">
        <code>{b.terminator}</code>
      </div>
      <Handle type="source" position={Position.Bottom} />
    </div>
  );
}
const nodeTypes = { block: BlockCard };
export default function CFG({
  snapshot,
  before,
  onLine,
}: {
  snapshot: Snapshot;
  before: Snapshot;
  onLine: (line: number) => void;
}) {
  const [nodes, setNodes, onNodesChange] = useNodesState<BlockNode>([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState<Edge>([]);
  const [value, setValue] = useState('');
  const [mode, setMode] = useState<'graph' | 'list'>('graph');
  const changes = useMemo(
    () => new Map(compareSnapshots(before, snapshot).map((c) => [c.id, c.kind])),
    [before, snapshot],
  );
  useEffect(() => {
    const graph = new dagre.graphlib.Graph();
    graph.setDefaultEdgeLabel(() => ({}));
    graph.setGraph({ rankdir: 'TB', nodesep: 60, ranksep: 70, marginx: 25, marginy: 25 });
    snapshot.blocks.forEach((b) =>
      graph.setNode(b.id, { width: 370, height: 80 + b.instructions.length * 24 }),
    );
    snapshot.blocks.forEach((b) => b.edges.forEach((e) => graph.setEdge(b.id, e.target)));
    dagre.layout(graph);
    setNodes(
      snapshot.blocks.map((b) => ({
        id: b.id,
        type: 'block',
        position: {
          x: graph.node(b.id).x - 185,
          y: graph.node(b.id).y - (80 + b.instructions.length * 24) / 2,
        },
        data: { block: b, selected: '', related: new Set(), changes, onSelect: () => {} },
      })),
    );
    setEdges(
      snapshot.blocks.flatMap((b) =>
        b.edges.map((e, i) => ({
          id: `${b.id}-${e.target}-${i}`,
          source: b.id,
          target: e.target,
          label: e.label,
          markerEnd: { type: MarkerType.ArrowClosed, color: '#7187A6' },
          style: {
            stroke: e.label === 'true' ? '#79C6AA' : e.label === 'false' ? '#DCA07E' : '#7187A6',
            strokeWidth: 1.5,
          },
          labelStyle: { fill: '#B9C5D9', fontSize: 11 },
          labelBgStyle: { fill: '#1B202A' },
          type: 'smoothstep',
        })),
      ),
    );
    setValue('');
  }, [snapshot, changes, setNodes, setEdges]);
  const selectedInstruction = snapshot.blocks
    .flatMap((b) => b.instructions)
    .find((i) => i.id === value);
  const related = new Set<string>();
  if (value) {
    selectedInstruction?.uses.forEach((u) => related.add(u.value));
    snapshot.blocks.forEach((b) =>
      b.instructions.forEach((i) => {
        if (i.uses.some((u) => u.value === value)) related.add(i.id);
      }),
    );
  }
  const rendered = nodes.map((n) => ({
    ...n,
    data: {
      ...n.data,
      selected: value,
      related,
      changes,
      onSelect: (id: string, line: number) => {
        setValue(id);
        onLine(line);
      },
    },
  }));
  return (
    <div className="cfg-view">
      <div className="cfg-toolbar">
        <div className="segmented">
          <button className={mode === 'graph' ? 'selected' : ''} onClick={() => setMode('graph')}>
            CFG / SSA
          </button>
          <button className={mode === 'list' ? 'selected' : ''} onClick={() => setMode('list')}>
            基本块列表
          </button>
        </div>
        <span className="cfg-legend">
          <i className="legend-dot green" />
          true
          <i className="legend-dot amber" />
          false
        </span>
        <span className="muted">{snapshot.blocks.length} blocks</span>
      </div>
      <div className="cfg-canvas">
        {mode === 'graph' ? (
          <ReactFlow
            key={snapshot.sha256}
            nodes={rendered}
            edges={edges}
            nodeTypes={nodeTypes}
            onNodesChange={onNodesChange}
            onEdgesChange={onEdgesChange}
            fitView
            fitViewOptions={{ padding: 0.2, maxZoom: 1 }}
            minZoom={0.1}
            maxZoom={1.7}
            colorMode="dark"
            nodesConnectable={false}
            deleteKeyCode={null}
            proOptions={{ hideAttribution: true }}
            aria-label="SSA 控制流图"
          >
            <Background color="#394354" gap={22} size={1} />
            <Controls showInteractive={false} />
          </ReactFlow>
        ) : (
          <div className="block-list">
            {snapshot.blocks.map((b) => (
              <section key={b.id}>
                <h3>
                  {b.id}
                  <span>{b.edges.map((e) => `${e.label} → ${e.target}`).join(' / ')}</span>
                </h3>
                {b.instructions.map((i) => (
                  <button
                    key={i.id}
                    className={value === i.id ? 'selected' : ''}
                    onClick={() => {
                      setValue(i.id);
                      onLine(i.line);
                    }}
                  >
                    <code>{i.text}</code>
                    {changes.has(i.id) && <Badge tone="blue">{changes.get(i.id)}</Badge>}
                  </button>
                ))}
                <code className="cfg-terminator">{b.terminator}</code>
              </section>
            ))}
          </div>
        )}
      </div>
      <div className="value-inspector">
        {selectedInstruction ? (
          <>
            <div>
              <Badge tone="blue">{selectedInstruction.id}</Badge>
              <code>
                {selectedInstruction.op} : {selectedInstruction.type}
              </code>
              <Button size="sm" variant="ghost" onClick={() => setValue('')}>
                清除选择
              </Button>
            </div>
            <p>
              输入：
              {selectedInstruction.uses.length
                ? selectedInstruction.uses
                    .map((u) => `${u.value}${u.predecessor ? ' @' + u.predecessor : ''}`)
                    .join(', ')
                : '无'}
              <span>
                使用者：
                {snapshot.blocks
                  .flatMap((b) => [
                    ...b.instructions
                      .filter((i) => i.uses.some((u) => u.value === value))
                      .map((i) => i.id),
                    ...(b.terminator_uses?.includes(value) ? [`${b.id} terminator`] : []),
                  ])
                  .join(', ') || '无'}
              </span>
            </p>
          </>
        ) : (
          <span>选择一条 SSA 指令，查看其定义、输入和使用者。拖动块标题可调整布局。</span>
        )}
      </div>
    </div>
  );
}
