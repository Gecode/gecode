#!/usr/bin/python

import types

registry = [
    { 'name': "int.distinct.Val<int.IntView>",
      'type': ['array<intvar>'],
      'post': ['distinct', (0, {}), 'ICL_VAL']
    }
    ,
    { 'name': "int.distinct.Val<int.OffsetView>",
      'type': ['array<int>', 'array<intvar>'],
      'post': ['distinct', (0, {}), (1, {}), 'ICL_VAL']
    }
    ,
    { 'name': "int.linear.Eq<int.ScaleView>",
      'type': ['array<int>', 'array<intvar>', 'array<int>', 'array<intvar>'],
      'post': ['linear', (0, {'concMinus':2}), (1, {'conc':3}), 'IRT_EQ', 0]
    }
    
  ]

argstring = 'FlatZincGecode* s,\n  const ConExpr& ce, AST::Node *ann'

def constructArg(a,t,i):
  if type(a) == types.IntType or type(a) == types.StringType:
    return
  elif a[1] == {}:
    if t[a[0]] == 'int':
      print "    int arg"+repr(i)+" = ce["+repr(a[0])+"]->getInt();"
    elif t[a[0]] == 'intvar':
      print "    IntVar arg"+repr(i)+" =",
      print "s->iv[ce["+repr(a[0])+"]->getIntVar()];"
      return
    elif t[a[0]] == 'array<int>':
      print "    IntArgs arg"+repr(i)+" =",
      print "arg2intargs(ce["+repr(a[0])+"]);"
      return
    elif t[a[0]] == 'array<intvar>':
      print "    IntVarArgs arg"+repr(i)+" =",
      print "arg2intvarargs(s, ce["+repr(a[0])+"]);"
      return
  elif isinstance(a[1], str):
    return
  elif a[1].has_key('conc'):
    if t[a[0]] == 'array<int>':
      assert(t[a[1]['conc']] == 'array<int>')
      print "    AST::Array* arg"+repr(i)+"_a1 = ce["+repr(a[0])+"]->getArray();"
      print "    AST::Array* arg"+repr(i)+"_a2 = ce["+repr(a[1]['conc'])+"]->getArray();"
      print "    IntArgs arg"+repr(i)+"(arg"+repr(i)+"_a1->a.size()+arg"+repr(i)+"_a2->a.size());"
      print "    for (int i=arg"+repr(i)+"_a1->a.size(); i--;)"
      print "      arg"+repr(i)+"[i] = arg"+repr(i)+"_a1->a[i]->getInt();"
      print "    for (int i=arg"+repr(i)+"_a2->a.size(); i--;)"
      print "      arg"+repr(i)+"[i+arg"+repr(i)+"_a1->a.size()] = arg"+repr(i)+"_a2->a[i]->getInt();"
      return
    elif t[a[0]] == 'array<intvar>':
      assert(t[a[1]['conc']] == 'array<intvar>')
      print "    AST::Array* arg"+repr(i)+"_a1 = ce["+repr(a[0])+"]->getArray();"
      print "    AST::Array* arg"+repr(i)+"_a2 = ce["+repr(a[1]['conc'])+"]->getArray();"
      print "    IntVarArgs arg"+repr(i)+"(arg"+repr(i)+"_a1->a.size()+arg"+repr(i)+"_a2->a.size());"
      print "    for (int i=arg"+repr(i)+"_a1->a.size(); i--;)"
      print "      arg"+repr(i)+"[i] = s->iv[arg"+repr(i)+"_a1->a[i]->getIntVar()];"
      print "    for (int i=arg"+repr(i)+"_a2->a.size(); i--;)"
      print "      arg"+repr(i)+"[i+arg"+repr(i)+"_a1->a.size()] = s->iv[arg"+repr(i)+"_a2->a[i]->getIntVar()];"
      return
  elif a[1].has_key('concMinus'):
    assert(t[a[0]] == 'array<int>' and t[a[1]['concMinus']] == 'array<int>')
    print "    AST::Array* arg"+repr(i)+"_a1 = ce["+repr(a[0])+"]->getArray();"
    print "    AST::Array* arg"+repr(i)+"_a2 = ce["+repr(a[1]['concMinus'])+"]->getArray();"
    print "    IntArgs arg"+repr(i)+"(arg"+repr(i)+"_a1->a.size()+arg"+repr(i)+"_a2->a.size());"
    print "    for (int i=arg"+repr(i)+"_a1->a.size(); i--;)"
    print "      arg"+repr(i)+"[i] = arg"+repr(i)+"_a1->a[i]->getInt();"
    print "    for (int i=arg"+repr(i)+"_a2->a.size(); i--;)"
    print "      arg"+repr(i)+"[i+arg"+repr(i)+"_a1->a.size()] = -arg"+repr(i)+"_a2->a[i]->getInt();"
    return

def emit(constraint):
  mangled = constraint['name']
  mangled = mangled.replace('_', '__')
  mangled = mangled.replace('.', '_P')
  mangled = mangled.replace('<', '_I_')
  mangled = mangled.replace('>', '_I_')
    
  print 'class ' + mangled + ' {'
  print 'public:'
  print '  static void post(' + argstring + ') {';
  post = constraint['post']
  for i, arg in enumerate(post[1:]):
    constructArg(arg, constraint['type'], i)
  print '    ' + post[0] + '(s,',
  for i, arg in enumerate(post[1:]):
    if type(arg) == types.StringType:
      print arg,
    elif type(arg) == types.IntType:
      print repr(arg),
    else:
      print 'arg' + repr(i),
    if i<len(post)-2:
      print ', ',
  print '   );'
  print '  }'
  print '  '+mangled+'() { registry.add("'+mangled+'", &post); }'
  print '};'
  print mangled+" _"+mangled+";"

print '#include "registry.hh"'
print '#include "gecode/kernel.hh"'
print '#include "gecode/int.hh"'
print '#include "gecode/minimodel.hh"'
print '#include "gecode/set.hh"'
print 'using namespace Gecode;'

print 'inline IntArgs arg2intargs(AST::Node* arg) {'
print '  AST::Array* a = arg->getArray();'
print '  IntArgs ia(a->a.size());'
print '  for (int i=a->a.size(); i--;)'
print '    ia[i] = a->a[i]->getInt();'
print '  return ia;'
print '}'

print 'inline IntVarArgs arg2intvarargs(FlatZincGecode* s, AST::Node* arg) {'
print '  AST::Array* a = arg->getArray();'
print '  IntVarArgs ia(a->a.size());'
print '  for (int i=a->a.size(); i--;) {'
print '    if (a->a[i]->isIntVar()) {'
print '      ia[i] = s->iv[a->a[i]->getIntVar()];        '
print '    } else {'
print '      int value = a->a[i]->getInt();'
print '      IntVar iv(s, value, value);'
print '      ia[i] = iv;        '
print '    }'
print '  }'
print '  return ia;'
print '}'


for c in registry:
  emit(c)
