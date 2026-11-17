#!/usr/bin/env python3
"""Extract checksum-pinned Ubuntu LLVM packages locally; never install them."""
import hashlib,json,os,subprocess,urllib.request
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 target=ROOT/'.toolchain';target.mkdir(exist_ok=True)
 lock=json.loads((ROOT/'scripts/llvm18.lock.json').read_text())
 for pkg in lock['packages']:
  archive=target/Path(pkg['filename']).name
  if not archive.exists():
   # apt resolves the distribution's configured mirror, not an unpinned latest.
   subprocess.run(['apt-get','download',pkg['package']+'='+pkg['version']],cwd=target,check=True)
   candidates=[p for p in target.glob('*.deb') if hashlib.sha256(p.read_bytes()).hexdigest()==pkg['sha256']]
   if not candidates:raise RuntimeError('download did not match lock: '+pkg['package'])
   if candidates[0]!=archive:candidates[0].rename(archive)
  digest=hashlib.sha256(archive.read_bytes()).hexdigest()
  if digest!=pkg['sha256']:raise RuntimeError('checksum mismatch: '+str(archive))
  stamp=target/(pkg['package']+'.extracted')
  if not stamp.exists():subprocess.run(['dpkg-deb','-x',str(archive),str(target)],check=True);stamp.write_text(digest+'\n')
 paths=[str(target/'usr/lib/x86_64-linux-gnu'),str(target/'usr/lib/llvm-18/lib')]
 env=os.environ|{'LD_LIBRARY_PATH':':'.join(paths)+(':'+os.environ['LD_LIBRARY_PATH'] if os.environ.get('LD_LIBRARY_PATH') else '')}
 versions={name:subprocess.check_output([str(target/'usr/lib/llvm-18/bin'/name),'--version'],env=env,text=True).splitlines()[0] for name in ['clang','opt']}
 (ROOT/'artifacts').mkdir(exist_ok=True);(ROOT/'artifacts/toolchain.json').write_text(json.dumps({'versions':versions,'packages':lock['packages']},indent=2)+'\n');print(versions)
if __name__=='__main__':main()
