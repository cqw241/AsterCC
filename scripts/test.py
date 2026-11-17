#!/usr/bin/env python3
"""Behavior-focused regression and bounded differential tests; stdlib only."""
import argparse, json, os, random, subprocess, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ENV=os.environ|{'ASTERCC_ROOT':str(ROOT)}
EXPECTED={
 'control':'31\n1\n25\n',
 'data':'1\n9\n2\n8\n2\n12\n1\n99\n1\n7\nHello Aster\n42\n158\n-1\n3\n90\n1\n你好🌟\n10\n',
 'integers':'-9223372036854775808\n-9223372036854775808\n0\n1\n-4\n4\n6\n8\n1\n1\n',
 'effects':'4\n1\n7\n2\n3\n7\n5\n1\n0\n',
 'optimization':'1290\n',
}
NEGATIVE={
 'lexical':('fn main()->i64 { @ return 0; }','invalid character'),
 'number':('fn main()->i64 { return 123xyz; }','invalid integer literal'),
 'syntax':('fn main()->i64 { let a = ;return 0; }','expected expression'),
 'name':('fn main()->i64 { return missing; }','undefined variable'),
 'unreachable':('fn main()->i64 { return 0;println_int(missing); }','undefined variable'),
 'type':('fn main()->i64 { let a:i64=true;return 0; }','type mismatch'),
 'immutable':('fn main()->i64 { let a=1;a=2;return 0; }','cannot assign'),
 'return':('fn main()->i64 { let a=1; }','missing return'),
 'break':('fn main()->i64 { break;return 0; }','outside a loop'),
 'layout':('struct R { child:R, } fn main()->i64 { return 0; }','infinitely recursive'),
}
TRAPS={
 'bounds':'fn main()->i64 { let a:[]i64=[1];println_int(a[1]);return 0; }',
 'division':'fn div(a:i64,b:i64)->i64 { return a/b; } fn main()->i64 { div(1,0);return 0; }',
}
def call(cmd,ok=True):
 p=subprocess.run([str(x) for x in cmd],cwd=ROOT,env=ENV,text=True,capture_output=True,timeout=180)
 if ok and p.returncode:raise AssertionError(f'{cmd}\n{p.stdout}\n{p.stderr}')
 return p
def wrap(n):return (n+(1<<63))%(1<<64)-(1<<63)
def generated():
 r=random.Random(20261117);lines=[];expect=[]
 for k in range(16):
  a,b,c=[r.randrange(-(1<<62),1<<62) for _ in range(3)]
  op=r.choice(['+','-','*','^','&','|'])
  value={'+':lambda:wrap(a+b),'-':lambda:wrap(a-b),'*':lambda:wrap(a*b),'^':lambda:a^b,'&':lambda:a&b,'|':lambda:a|b}[op]()
  value=wrap(value+c);lines.append(f'let v{k}:i64=({a} {op} {b}) + {c};println_int(v{k});');expect.append(str(value))
 return 'fn main()->i64 {\n'+'\n'.join(lines)+'\nreturn 0; }','\n'.join(expect)+'\n'
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--compiler',default='build/astercc0');ap.add_argument('--reference');ap.add_argument('--output',default='artifacts/tests.json');o=ap.parse_args()
 compiler=Path(o.compiler).resolve();work=ROOT/'artifacts/tmp/tests'/compiler.name;work.mkdir(parents=True,exist_ok=True);results=[]
 cases={k:(ROOT/'tests/cases'/f'{k}.aster',v) for k,v in EXPECTED.items()}
 src,expect=generated();p=work/'generated.aster';p.write_text(src);cases['generated']=(p,expect)
 for name,(source,expected) in cases.items():
  for level in range(3):
   out=work/f'{name}-O{level}';t=time.monotonic();call([compiler,source,f'-O{level}','--verify-ir','-o',out]);p=call([out]);assert p.stdout==expected,(name,level,p.stdout,expected)
   call([ROOT/'.toolchain/usr/lib/llvm-18/bin/opt','-passes=verify','-disable-output',str(out)+'.ll'])
   if o.reference:
    reference=work/f'{name}-O{level}.reference.ll';call([Path(o.reference).resolve(),source,f'-O{level}','--emit=llvm','-o',reference]);assert reference.read_bytes()==Path(str(out)+'.ll').read_bytes(),f'cross-stage LLVM mismatch: {name} O{level}'
    for kind in ['ssa','ast']:
     own=work/f'{name}-O{level}.{kind}';ref=work/f'{name}-O{level}.reference.{kind}'
     call([compiler,source,f'-O{level}',f'--emit={kind}','-o',own]);call([Path(o.reference).resolve(),source,f'-O{level}',f'--emit={kind}','-o',ref]);assert own.read_bytes()==ref.read_bytes(),f'cross-stage {kind} mismatch: {name} O{level}'
   results.append({'case':name,'optimization':level,'status':'pass','seconds':round(time.monotonic()-t,6),'stdout':p.stdout})
 for name,(source,message) in NEGATIVE.items():
  src=work/f'bad-{name}.aster';src.write_text(source);p=call([compiler,src,'--check'],False);assert p.returncode!=0 and message in p.stderr,(name,p.returncode,p.stderr);assert ':1:' in p.stderr and '^' in p.stderr,p.stderr;results.append({'case':'diagnostic-'+name,'status':'pass','stderr':p.stderr})
 for name,source in TRAPS.items():
  src=work/f'trap-{name}.aster';src.write_text(source)
  for level in [0,2]:
   out=work/f'trap-{name}-O{level}';call([compiler,src,f'-O{level}','--verify-ir','-o',out]);p=call([out],False);assert p.returncode==70 and 'aster:' in p.stderr,(name,p.returncode,p.stderr);results.append({'case':'trap-'+name,'optimization':level,'status':'pass','stderr':p.stderr})
 path=ROOT/o.output;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps({'compiler':str(compiler.relative_to(ROOT)),'passed':len(results),'results':results},ensure_ascii=False,indent=2)+'\n');print(f'{compiler.name}: {len(results)} behavioral checks passed')
if __name__=='__main__':main()
