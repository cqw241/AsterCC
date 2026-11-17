#!/usr/bin/env python3
"""One focused sanitizer run on the native seed and the platform runtime."""
import json,os,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ENV=os.environ|{'ASTERCC_ROOT':str(ROOT),'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1:print_stacktrace=1'}
def run(command):
 p=subprocess.run([str(x) for x in command],cwd=ROOT,env=ENV,text=True,capture_output=True,timeout=300)
 if p.returncode:raise RuntimeError(f'{command}\n{p.stdout}\n{p.stderr}')
 return {'command':[str(x) for x in command],'stdout':p.stdout,'stderr':p.stderr,'returncode':p.returncode}
def main():
 out=ROOT/'build/sanitize';out.mkdir(exist_ok=True);logs=[]
 logs.append(run(['cmake','-S',ROOT,'-B',out,'-DCMAKE_BUILD_TYPE=Debug','-DASTER_SANITIZE=ON']))
 logs.append(run(['cmake','--build',out,'-j','2']))
 logs.append(run([out/'astercc0',ROOT/'compiler/main.aster','-O2','--verify-ir','--check']))
 # Compile the runtime with GCC's sanitizer libraries, using the very same
 # LLVM IR exercised by the ordinary language test.
 ir=out/'data.ll';obj=out/'data.o';exe=out/'data'
 logs.append(run([ROOT/'build/astercc0',ROOT/'tests/cases/data.aster','-O2','--emit=llvm','-o',ir]))
 logs.append(run([ROOT/'.toolchain/usr/lib/llvm-18/bin/clang','-c','-x','ir',ir,'-o',obj]))
 logs.append(run(['gcc','-std=c11','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer',ROOT/'runtime/runtime.c',obj,'-o',exe]))
 logs.append(run([exe]));(ROOT/'artifacts/sanitizers.json').write_text(json.dumps({'status':'pass','seed_checked_own_source':True,'runtime_case':'data','commands':logs},ensure_ascii=False,indent=2)+'\n');print('ASan/UBSan: seed and runtime passed')
if __name__=='__main__':main()
