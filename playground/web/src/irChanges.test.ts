import { describe, expect, it } from 'vitest';
import fixtures from './ir-fixtures.json';
import { compareSnapshots } from './irChanges';
import type { Snapshot } from './types';
const input = fixtures.input as Snapshot,
  gvn = fixtures.gvn as Snapshot,
  licm = fixtures.licm as Snapshot;
describe('Changes from recorded AsterCC kernel traces', () => {
  it('reports loop-invariant motion without claiming an instruction reduction', () => {
    const changes = compareSnapshots(gvn, licm);
    expect(changes).toEqual([
      {
        id: '%14',
        kind: 'moved',
        before: '%14:i64 = mul  %1, %2',
        after: '%14:i64 = mul  %1, %2',
        from: 'b5',
        to: 'b0',
      },
    ]);
    expect(gvn.instruction_count).toBe(licm.instruction_count);
  });
  it('distinguishes removed duplicate values and changed phi values', () => {
    const changes = compareSnapshots(input, gvn);
    expect(changes.find((c) => c.id === '%15')?.kind).toBe('removed');
    expect(changes.find((c) => c.id === '%30')?.kind).toBe('modified');
    expect(compareSnapshots(input, input)).toEqual([]);
  });
});
