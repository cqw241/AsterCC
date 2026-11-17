#!/usr/bin/env python3
"""Development-only mechanical translation of compiler implementation to C++.

This is NOT used by CMake, bootstrap, reproduction, or AsterCC. The checked-in
C++ seed is standalone. This deliberately small translator accepts the subset
used to write the compiler; it is not the Aster language implementation.
"""
import re
import subprocess
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
TOK = re.compile(r'\s+|//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|0x[0-9a-fA-F]+|[0-9]+|[A-Za-z_][A-Za-z_0-9]*|->|::|==|!=|<=|>=|&&|\|\||<<|>>|.')
PREC={'||':1,'&&':2,'|':3,'^':4,'&':5,'==':6,'!=':6,'<':7,'>':7,'<=':7,'>=':7,'<<':8,'>>':8,'+':9,'-':9,'*':10,'/':10,'%':10}
class Parser:
    def __init__(self,s): self.ts=[t for t in TOK.findall(s) if not t.isspace() and not t.startswith(('//','/*'))];self.i=0
    def peek(self): return self.ts[self.i] if self.i<len(self.ts) else ''
    def take(self): t=self.peek();self.i+=1;return t
    def eat(self,t):
        if self.peek()==t:self.i+=1;return True
        return False
    def need(self,t):
        a=self.take();assert a==t,(a,t,self.ts[max(0,self.i-8):self.i+8])
    def ty(self):
        if self.eat('['):self.need(']');return 'Arr<'+self.ty()+'> '
        return self.take()
    def expr(self,minp=0):
        t=self.take()
        if t=='(':
            a='('+self.expr()+')';self.need(')')
        elif t in ('!','-','~'):
            v=self.expr(11);a=('wrap_sub(0,'+v+')') if t=='-' else '('+t+v+')'
        elif t=='[':
            es=[]
            if self.peek()!=']':
                es.append(self.expr())
                while self.eat(','):
                    if self.peek()==']':break
                    es.append(self.expr())
            self.need(']');a='{'+','.join(es)+'}'
        elif t.startswith('"'): a='str('+t+')'
        elif t[0].isdigit():a='i64('+t+')'
        else:
            a='aster_main' if t=='main' else t
            if self.peek()=='{' and t[0].isupper():
                self.take();es=[]
                while self.peek()!='}':
                    self.take();self.need(':');es.append(self.expr())
                    if not self.eat(','):break
                self.need('}');a=t+'{'+','.join(es)+'}'
        while True:
            t=self.peek()
            if t=='(':
                self.take();es=[]
                if self.peek()!=')':
                    es.append(self.expr())
                    while self.eat(','):es.append(self.expr())
                self.need(')');a+='('+','.join(es)+')'
            elif t=='[':
                self.take();b=self.expr();self.need(']');a='index('+a+','+b+')'
            elif t=='.':self.take();a+='.'+self.take()
            elif t=='as':self.take();a='static_cast<'+self.ty()+' >('+a+')'
            elif t in PREC and PREC[t]>=minp:
                self.take();b=self.expr(PREC[t]+1)
                funcs={'+':'wrap_add','-':'wrap_sub','*':'wrap_mul','/':'wrap_div','%':'wrap_rem','<<':'wrap_shl'}
                # '+' is overloaded for strings and defined modulo 2^64 for i64.
                a=(funcs[t]+'('+a+','+b+')') if t in funcs else '('+a+t+b+')'
            else:break
        return a
    def block(self):
        self.need('{');s='{\n'
        while self.peek()!='}':s+=self.stmt()+'\n'
        self.need('}');return s+'}'
    def stmt(self):
        t=self.peek()
        if t=='{':return self.block()
        if t in ('var','let'):
            self.take();n=self.take();ty=self.ty() if self.eat(':') else 'auto';self.need('=');e=self.expr();self.need(';');return ty+' '+n+'='+e+';'
        if t in ('if','while'):
            self.take();e=self.expr();s=t+'('+e+')'+self.block()
            if t=='if' and self.eat('else'):s+='else '+(self.stmt() if self.peek()=='if' else self.block())
            return s
        if t=='return':
            self.take();e='' if self.peek()==';' else self.expr();self.need(';');return 'return '+e+';'
        if t in ('break','continue'):
            self.take();self.need(';');return t+';'
        a=self.expr()
        if self.eat('='):a+='='+self.expr()
        self.need(';');return a+';'
    def program(self):
        structs=[];protos=[];functions=[]
        while self.peek():
            t=self.take()
            if t=='import':
                while self.take()!=';':pass
            elif t=='struct':
                n=self.take();self.need('{');fields=[]
                while self.peek()!='}':
                    f=self.take();self.need(':');fields.append(self.ty()+' '+f+';');self.eat(',')
                self.need('}');structs.append('struct '+n+'{'+''.join(fields)+'};')
            elif t=='fn':
                n=self.take();n='aster_main' if n=='main' else n;self.need('(');ps=[]
                while self.peek()!=')':
                    p=self.take();self.need(':');ps.append(self.ty()+' '+p)
                    if not self.eat(','):break
                self.need(')');self.need('->');ret=self.ty();sig=ret+' '+n+'('+','.join(ps)+')';protos.append(sig+';');functions.append(sig+self.block())
            else:raise ValueError((t,self.i))
        return '\n'.join(structs+protos+functions)
source=(ROOT/'std/collections.aster').read_text()+'\n'+'\n'.join(p.read_text() for p in sorted((ROOT/'compiler').glob('*.aster')))
body=Parser(source).program()
header='// Standalone C++20 Stage 0. Mechanically synchronized; see scripts/sync_seed.py.\n#include "support.hpp"\n'
tail='\nint main(int argc,char**argv){for(int i=0;i<argc;i++)push(seed_args,str(argv[i]));return (int)aster_main();}\n'
(ROOT/'stage0/compiler.cpp').write_text(header+body+tail)
formatter=ROOT/'.toolchain/usr/lib/llvm-18/bin/clang-format'
if formatter.exists():subprocess.run([str(formatter),'-i',str(ROOT/'stage0/compiler.cpp')],check=True)
