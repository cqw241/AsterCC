#!/usr/bin/env python3
"""Check that passes actually transform IR, as well as preserving execution."""
import json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 out=ROOT/'artifacts/optimization';out.mkdir(parents=True,exist_ok=True)
 env=os.environ|{'ASTERCC_ROOT':str(ROOT)}
 command=[str(ROOT/'build/astercc0'),str(ROOT/'tests/cases/optimization.aster'),'-O2','--verify-ir','--emit=ssa','--trace-dir',str(out),'--stats-json',str(out/'stats.json'),'-o',str(out/'optimized.ssa')]
 subprocess.run(command,env=env,cwd=ROOT,check=True)
 rows=json.loads((out/'stats.json').read_text())
 for name in ['sccp','dce','gvn']:
  assert any(r['pass']==name and r['after']<r['before'] for r in rows),f'{name} performed no expected elimination'
 assert any(r['pass']=='licm' and r['changed_instructions']>0 for r in rows),'LICM performed no expected motion'
 assert 'phi' in (out/'0-00-input.ssa').read_text(),'loop SSA missing phi'
 (out/'checks.json').write_text(json.dumps({'status':'pass','verified':['loop-phi','sccp-elimination','dce-elimination','gvn-elimination','licm-motion']},indent=2)+'\n')
 print('SSA construction and optimizer transformations verified')
if __name__=='__main__':main()
