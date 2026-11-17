#!/usr/bin/env python3
"""Check artifact provenance and public emission interfaces, then clean scratch."""
import hashlib,json,os,re,shutil,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def load(name):return json.loads((ROOT/'artifacts'/name).read_text())
def main():
 boot=load('bootstrap.json');assert boot['fixed_point_O0'] and boot['fixed_point_O2'] and boot['cross_stage_traces_equal']
 for name,value in boot['source_sha256'].items():assert sha(ROOT/name)==value,f'source changed after bootstrap: {name}'
 assert sha(ROOT/'build/libasterrt.a')==boot['runtime_archive_sha256']
 for r in boot['stages']:
  assert sha(ROOT/r['binary'])==r['binary_sha256'];assert sha(str(ROOT/r['binary'])+'.ll')==r['llvm_sha256']
 traces=load('trace-hashes.json')
 for level in [0,2]:assert traces[f'O{level}/stage1']==traces[f'O{level}/stage2']==traces[f'O{level}/stage3']
 assert load('tests.json')['passed']==load('tests-stage3.json')['passed']==32
 assert load('sanitizers.json')['status']=='pass'
 assert load('optimization/checks.json')['status']=='pass'
 report=(ROOT/'实验报告.md').read_text()
 for heading in ['一、——实验目的','二、实验环境','三、实验内容及原理','四、关键数据结构和核心算法','4.1 设计的各类数据结构','4.2 实现的各项功能核心算法','4.3 错误处理','4.4 运行结果和分析','五、总结']:assert heading in report
 assert '@@' not in report
 for link in re.findall(r'\]\(([^)]+)\)',report):
  if not link.startswith(('http:','https:','#')):assert (ROOT/link).exists(),f'broken report link: {link}'
 # Exercise the remaining public output interfaces without repeating the suite.
 work=ROOT/'build/delivery';work.mkdir(exist_ok=True);compiler=ROOT/'build/bootstrap/O2/stage3';env=os.environ|{'ASTERCC_ROOT':str(ROOT)}
 def call(args):return subprocess.run([str(x) for x in args],cwd=ROOT,env=env,text=True,capture_output=True,check=True)
 call([compiler,'examples/hello.aster','--emit=obj','-o',work/'hello.o']);call(['gcc',work/'hello.o','build/libasterrt.a','-o',work/'hello']);assert call([work/'hello']).stdout==load('demo.json')['stdout']
 call([compiler,'examples/hello.aster','--passes=simplify,sccp,cfg,dce','--verify-ir','--emit=llvm','-o',work/'custom.ll'])
 assert not list(ROOT.glob('*.docx')) and not list(ROOT.glob('*.pdf'))
 manifest={str(p.relative_to(ROOT)):sha(p) for folder in ['compiler','std','stage0','runtime','scripts','docs','tests','examples'] for p in sorted((ROOT/folder).rglob('*')) if p.is_file() and '__pycache__' not in p.parts}
 manifest['实验报告.md']=sha(ROOT/'实验报告.md');manifest['README.md']=sha(ROOT/'README.md');manifest['CMakeLists.txt']=sha(ROOT/'CMakeLists.txt')
 (ROOT/'artifacts/delivery.json').write_text(json.dumps({'status':'pass','source_provenance':True,'stage_artifacts':True,'report_links':True,'object_linking':True,'custom_pass_pipeline':True,'files':manifest},ensure_ascii=False,indent=2)+'\n')
 shutil.rmtree(ROOT/'artifacts/tmp',ignore_errors=True)
 print('Delivery provenance, report links, object emission and custom pipeline verified; scratch cleaned.')
if __name__=='__main__':main()
