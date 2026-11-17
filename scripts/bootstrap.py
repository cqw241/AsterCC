#!/usr/bin/env python3
"""Actual native bootstrap, including per-pass trace comparison and isolation."""
import hashlib, json, os, shutil, subprocess, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ENV=os.environ|{'ASTERCC_ROOT':str(ROOT)}
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(command,log):
 log.parent.mkdir(parents=True,exist_ok=True)
 start=time.monotonic();p=subprocess.run([str(x) for x in command],cwd=ROOT,env=ENV,text=True,capture_output=True,timeout=300)
 log.write_text('$ '+' '.join(map(str,command))+'\n'+p.stdout+p.stderr)
 if p.returncode:raise RuntimeError(f'bootstrap failed ({p.returncode}): {log}\n{p.stderr}')
 return time.monotonic()-start
def main():
 seed=ROOT/'build/astercc0';source=ROOT/'compiler/main.aster';records=[];trace_manifest={}
 opt=ROOT/'.toolchain/usr/lib/llvm-18/bin/opt'
 for level in [0,2]:
  directory=ROOT/f'build/bootstrap/O{level}';directory.mkdir(parents=True,exist_ok=True);previous=seed;stages=[]
  for stage in [1,2,3]:
   output=directory/f'stage{stage}';trace=directory/f'trace{stage}';shutil.rmtree(trace,ignore_errors=True)
   stats=ROOT/f'artifacts/bootstrap/O{level}-stage{stage}-stats.json'
   seconds=run([previous,source,f'-O{level}',f'--llvm-opt={level}','--verify-ir','--trace-dir',trace,'--stats-json',stats,'-o',output],ROOT/f'artifacts/bootstrap/O{level}-stage{stage}.log')
   run([opt,'-passes=verify','-disable-output',str(output)+'.ll'],ROOT/f'artifacts/bootstrap/O{level}-stage{stage}-llvm-verify.log')
   hashes={p.name:sha(p) for p in sorted(trace.glob('*.ssa'))};trace_manifest[f'O{level}/stage{stage}']=hashes
   if stages:
    before=trace_manifest[f'O{level}/stage{stage-1}'];different=sorted(k for k in hashes.keys()|before.keys() if hashes.get(k)!=before.get(k))
    if different:raise AssertionError(f'first cross-stage pass divergence: O{level} {different[0]}')
   row={'optimization':level,'stage':stage,'producer':str(previous.relative_to(ROOT)),'binary':str(output.relative_to(ROOT)),'binary_sha256':sha(output),'llvm_sha256':sha(str(output)+'.ll'),'binary_bytes':output.stat().st_size,'seconds':round(seconds,6),'trace_files':len(hashes)}
   records.append(row);stages.append(output);previous=output;print(f'O{level} Stage {stage}: {row["binary_sha256"][:16]} ({seconds:.3f}s)',flush=True)
  assert sha(stages[1])==sha(stages[2]),f'O{level}: Stage 2/3 binary mismatch'
  assert sha(str(stages[0])+'.ll')==sha(str(stages[1])+'.ll')==sha(str(stages[2])+'.ll'),f'O{level}: LLVM mismatch'
 # Execute an unseen program with a relocated Stage 3, a separate runtime and
 # no seed binary at its original location. Restore the seed even on failure.
 isolated=ROOT/'build/standalone';isolated.mkdir(exist_ok=True);shutil.copy2(ROOT/'build/bootstrap/O2/stage3',isolated/'astercc');(isolated/'build').mkdir(exist_ok=True);shutil.copy2(ROOT/'build/libasterrt.a',isolated/'build/libasterrt.a')
 fresh=isolated/'fresh.aster';fresh.write_text('fn fib(n:i64)->i64 { if n<2 { return n; }return fib(n-1)+fib(n-2); } fn main()->i64 { println_int(fib(20));return 0; }\n')
 hidden=seed.with_suffix('.temporarily-hidden');seed.rename(hidden)
 try:
  isolated_env=ENV|{'ASTERCC_ROOT':str(isolated),'ASTERCC_CLANG':str(ROOT/'.toolchain/usr/lib/llvm-18/bin/clang')}
  p=subprocess.run([str(isolated/'astercc'),str(fresh),'-O2','--verify-ir','-o',str(isolated/'fresh')],cwd=isolated,env=isolated_env,text=True,capture_output=True,timeout=180);assert p.returncode==0,p.stderr
  p=subprocess.run([str(isolated/'fresh')],text=True,capture_output=True,check=True);assert p.stdout=='6765\n'
  linked=subprocess.run(['ldd',str(isolated/'astercc')],text=True,capture_output=True,check=True).stdout;assert 'libstdc++' not in linked
 finally:hidden.rename(seed)
 source_files=sorted(p for folder in ['compiler','std','stage0','runtime'] for p in (ROOT/folder).glob('*') if p.is_file())
 result={'source_sha256':{str(p.relative_to(ROOT)):sha(p) for p in source_files},'runtime_archive_sha256':sha(ROOT/'build/libasterrt.a'),'stages':records,'fixed_point_O0':True,'fixed_point_O2':True,'cross_stage_traces_equal':True,'standalone':{'seed_hidden':True,'relocated':True,'stdout':p.stdout,'no_libstdcpp':True,'ldd':linked}}
 (ROOT/'artifacts/bootstrap.json').write_text(json.dumps(result,indent=2)+'\n');(ROOT/'artifacts/trace-hashes.json').write_text(json.dumps(trace_manifest,indent=2)+'\n')
 print('O0/O2 fixed points, all pass traces and isolated Stage 3 verified.')
if __name__=='__main__':main()
