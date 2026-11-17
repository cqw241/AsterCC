#!/usr/bin/env python3
"""Separate Aster optimizations from LLVM; never assume a speedup exists."""
import hashlib,json,os,statistics,subprocess,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ENV=os.environ|{'ASTERCC_ROOT':str(ROOT)}
CONFIGS=[('A0-L0',0,0),('A2-L0',2,0),('A2-L2',2,2)]
INPUTS={'constants':['3000000'],'gvn':['3000000','7'],'licm':['3000000','7','9'],'arrays':['3000000','7']}
def expected(name,argv):
 n=int(argv[0])
 if name=='constants':return 3*n*(n-1)//2+7*n
 if name=='gvn':return 3*(int(argv[1])*n*(n-1)//2+17*n)
 if name=='licm':
  a,b=map(int,argv[1:]);v=(a*b+19)*(a+b+3);return sum(v^i for i in range(n))
 total=sum(range(256))*(n//256)+sum(range(n%256));return total*(int(argv[1])**2+3)
def call(cmd):
 return subprocess.run([str(x) for x in cmd],cwd=ROOT,env=ENV,text=True,capture_output=True,check=True,timeout=180)
def main():
 work=ROOT/'build/benchmarks';work.mkdir(parents=True,exist_ok=True);rows=[]
 for name,argv in INPUTS.items():
  correct=str(expected(name,argv))+'\n'
  for label,aster,llvm in CONFIGS:
   exe=work/f'{name}-{label}';stats=work/f'{name}-{label}.json';start=time.perf_counter_ns()
   call([ROOT/'build/bootstrap/O2/stage3',ROOT/f'examples/benchmarks/{name}.aster',f'-O{aster}',f'--llvm-opt={llvm}','--verify-ir','--stats-json',stats,'-o',exe]);compile_ms=(time.perf_counter_ns()-start)/1e6
   call([ROOT/'build/bootstrap/O2/stage3',ROOT/f'examples/benchmarks/{name}.aster',f'-O{aster}','--emit=ssa','-o',str(exe)+'.ssa'])
   warm=call([exe,*argv]);assert warm.stdout==correct,(name,label,warm.stdout,correct)
   durations=[]
   for _ in range(5):
    start=time.perf_counter_ns();p=call([exe,*argv]);durations.append((time.perf_counter_ns()-start)/1e6);assert p.stdout==correct
   text=Path(str(exe)+'.ssa').read_text();instructions=sum(line.startswith('  %') and ' = const ' not in line and ' = string ' not in line and ' = arg ' not in line for line in text.splitlines());blocks=sum(line.startswith('b') and line.endswith(':') for line in text.splitlines())
   rows.append({'benchmark':name,'configuration':label,'input':argv,'instructions':instructions,'blocks':blocks,'compile_ms':round(compile_ms,6),'runtime_ms':round(statistics.median(durations),6),'samples_ms':durations,'binary_bytes':exe.stat().st_size,'stdout':p.stdout,'output_sha256':hashlib.sha256(p.stdout.encode()).hexdigest(),'pass_stats':json.loads(stats.read_text())})
   print(f'{name:10} {label}: {instructions:3} IR instructions, median {rows[-1]["runtime_ms"]:.3f} ms',flush=True)
 (ROOT/'artifacts/benchmarks.json').write_text(json.dumps({'compiler':'build/bootstrap/O2/stage3','warmups':1,'measurements':5,'rows':rows},indent=2)+'\n')
if __name__=='__main__':main()
