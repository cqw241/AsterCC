import type { Snapshot } from './types';
export interface IRChange {
  id: string;
  kind: 'removed' | 'added' | 'modified' | 'moved';
  before?: string;
  after?: string;
  from?: string;
  to?: string;
}
export function compareSnapshots(before: Snapshot, after: Snapshot): IRChange[] {
  const a = new Map(
    before.blocks.flatMap((b) => b.instructions.map((i) => [i.id, { ...i, block: b.id }] as const)),
  );
  const b = new Map(
    after.blocks.flatMap((b) => b.instructions.map((i) => [i.id, { ...i, block: b.id }] as const)),
  );
  const changes: IRChange[] = [];
  for (const [id, x] of a) {
    const y = b.get(id);
    if (!y) changes.push({ id, kind: 'removed', before: x.text, from: x.block });
    else if (x.block !== y.block || x.text !== y.text)
      changes.push({
        id,
        kind: x.text !== y.text ? 'modified' : 'moved',
        before: x.text,
        after: y.text,
        from: x.block,
        to: y.block,
      });
  }
  for (const [id, y] of b)
    if (!a.has(id)) changes.push({ id, kind: 'added', after: y.text, to: y.block });
  return changes;
}
