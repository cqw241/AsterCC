#!/usr/bin/env python3
"""Development-only whitespace formatter for this project's Aster sources."""
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
TOKEN=re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|0x[0-9a-fA-F]+|[0-9]+|[A-Za-z_][A-Za-z_0-9]*|->|::|==|!=|<=|>=|&&|\|\||<<|>>|[^\s]')
OPERATORS=set('= + - * / % < > & | ^ ! ~ -> == != <= >= && || << >> as'.split())
def format_source(source):
 ts=TOKEN.findall(source);lines=[];line='';depth=0;previous=''
 def flush():
  nonlocal line
  if line.strip():lines.append('    '*depth+line.strip())
  line=''
 for i,t in enumerate(ts):
  nxt=ts[i+1] if i+1<len(ts) else ''
  if t.startswith(('//','/*')):flush();lines.extend('    '*depth+x for x in t.splitlines());previous='';continue
  if t=='{':line=line.rstrip()+' {';flush();depth+=1
  elif t=='}':
   flush();depth-=1;line='}'
   if nxt not in {',',';',')',']','.','else'}|OPERATORS:flush()
   if nxt=='fn':lines.append('')
  elif t==';':line=line.rstrip()+';';flush()
  elif t==',':
   line=line.rstrip()+', '
   if len(line)>96:flush()
  elif t==':':line=line.rstrip()+': '
  elif t=='[':
   space=' ' if previous in {':','=',',','return','->'} else ''
   line=line.rstrip()+space+t
  elif t in {'.','::','(',')',']'}:line=line.rstrip()+t
  else:
   need=bool(line) and not line.endswith((' ','.','::','(','['))
   if t in OPERATORS or previous in OPERATORS:need=bool(line) and not line.endswith(' ')
   if previous==']' and i>=2 and ts[i-2]=='[':need=False
   if need:line+=' '
   line+=t
  previous=t
 flush();return '\n'.join(lines)+'\n'
if __name__=='__main__':
 for directory in ['compiler','std','examples','tests/cases']:
  for p in (ROOT/directory).rglob('*.aster'):p.write_text(format_source(p.read_text()))
