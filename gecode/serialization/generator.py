#!/usr/bin/python

import types
import registry

def isintvartype(t):
  return t == 'intvar' or t == 'offsetvar' or \
         t == 'scalevar' or t == 'minusvar' or t == 'constint'

def isboolvartype(t):
 return t == 'boolvar' or t == 'negboolvar'

def issetvartype(t):
  return t == 'setvar'

def isvartype(t):
  return isintvartype(t) or isboolvartype(t) or issetvartype(t)

def checktype(t,args):
  print "    if (",
  for i, ty in enumerate(t):
    aty = ty.replace("]","")
    if isvartype(aty):
      if i > 0:
        print '        ',
      print '_typeDefs.'+ty.replace("]","")+'.equal(spec[',
      print repr(i)+']->type()) &&'
  print '        true) {'
  for i, ty in enumerate(t):
    if ty.startswith("]"):
      aty = ty.replace("]","")
      if aty == 'int':
        print '      Reflection::ArrayArg<int>* a ='
        print '        spec['+repr(i)+']->toArray<int>();'
        print '      IntArgs arg'+repr(i)+'(a->size());'
        print '      for (int i=a->size(); i--;) {'
        print '        arg'+repr(i)+'[i] = (*a)[i];'
        print '      }'
      if aty == 'constint':
        print '      Reflection::ArrayArg<Reflection::Arg*>* a ='
        print '        spec['+repr(i)+']->typedArg()->toArray<Reflection::Arg*>();'
        print '      IntArgs arg'+repr(i)+'(a->size());'
        print '      for (int i=a->size(); i--;) {'
        print '        arg'+repr(i)+'[i] = (*a)[i]->toInt();'
        print '      }'
      elif aty == 'intvar' or aty == 'minusvar':
        print '      Reflection::ArrayArg<Reflection::Arg*>* a'+repr(i)+' ='
        print '        spec['+repr(i)+ \
          ']->typedArg()->toArray<Reflection::Arg*>();'
        print '      IntVarArgs arg'+repr(i)+'(a'+repr(i)+'->size());'
        print '      for (int i=a'+repr(i)+'->size(); i--;) {'
        print '        arg'+repr(i)+'[i] = IntVar(Int::IntView(static_cast<Int::IntVarImp*>('+\
          'v[(*a'+repr(i)+')[i]->toVar()])));'
        print '      }'
      elif aty == 'boolvar' or aty == 'negboolvar':
        print '      Reflection::ArrayArg<Reflection::Arg*>* a'+repr(i)+' ='
        print '        spec['+repr(i)+ \
          ']->typedArg()->toArray<Reflection::Arg*>();'
        print '      BoolVarArgs arg'+repr(i)+'(a'+repr(i)+'->size());'
        print '      for (int i=a'+repr(i)+'->size(); i--;) {'
        print '        arg'+repr(i)+'[i] = BoolVar(Int::BoolView(static_cast<Int::BoolVarImp*>('+\
          'v[(*a'+repr(i)+')[i]->toVar()])));'
        print '      }'
      elif aty == 'offsetvar' or aty == 'scalevar':
        print '      Reflection::ArrayArg<Reflection::Arg*>* a'+repr(i)+' ='
        print '        spec['+repr(i)+ \
          ']->typedArg()->toArray<Reflection::Arg*>();'
        print '      IntArgs arg'+repr(i)+'_0(a'+repr(i)+'->size());'
        print '      IntVarArgs arg'+repr(i)+'_1(a'+repr(i)+'->size());'
        print '      for (int i=a'+repr(i)+'->size(); i--;) {'
        print '        arg'+repr(i)+'_0[i] = (*a'+repr(i)+')[i]->first()->toInt();'
        print '        arg'+repr(i)+'_1[i] = IntVar(Int::IntView(static_cast<Int::IntVarImp*>('+\
          'v[(*a'+repr(i)+')[i]->second()->toVar()])));'
        print '      }'
    else:
      if ty == 'int':
        print '      int arg'+repr(i)+' = spec['+repr(i)+']->toInt();'
      elif ty == 'intset':
        print '      Reflection::ArrayArg<int>* a'+repr(i)+' ='
        print '        spec['+repr(i)+']->toArray<int>();'
        print '      IntArrayIter iai'+repr(i)+'(a'+repr(i)+');'
        print '      IntSet arg'+repr(i)+'(iai'+repr(i)+');'
      elif ty == 'intvar' or ty == 'minusvar':
        print '      IntVar arg'+repr(i)+\
          ' = IntVar(Int::IntView(static_cast<Int::IntVarImp*>('+\
          'v[spec['+repr(i)+']->typedArg()->toVar()])));'
      elif ty == 'constint':
        print '      int arg'+repr(i)+\
          ' = spec['+repr(i)+']->typedArg()->toInt();'
      elif ty == 'boolvar' or ty == 'negboolvar':
        print '      BoolVar arg'+repr(i)+\
          ' = BoolVar(Int::BoolView(static_cast<Int::BoolVarImp*>('+\
          'v[spec['+repr(i)+']->typedArg()->toVar()])));'
      elif ty == 'offsetvar' or ty == 'scalevar':
        print '      int arg'+repr(i)+'_0 = spec['+repr(i)+']->typedArg()->first()->toInt();'
        print '      IntVar arg'+repr(i)+'_1'+\
          ' = IntVar(Int::IntView(static_cast<Int::IntVarImp*>('+\
          'v[spec['+repr(i)+']->typedArg()->second()->toVar()])));'
  for i,arg in enumerate(args):
    if type(arg) == types.ListType:
      if t[arg[0]] == 'int':
        print '      IntArgs argL'+repr(i)+'('+repr(len(arg))+');'
      elif isintvartype(t[arg[0]]):
        print '      IntVarArgs argL'+repr(i)+'('+repr(len(arg))+');'
      elif isboolvartype(t[arg[0]]):
        print '      BoolVarArgs argL'+repr(i)+'('+repr(len(arg))+');'
      elif issetvartype(t[arg[0]]):
        print '      SetVarArgs argL'+repr(i)+'('+repr(len(arg))+');'
      for j,k in enumerate(arg):
        print '      argL'+repr(i)+'['+repr(j)+'] = arg'+repr(k)+';'

def emit(c):
  mangled = c['name'];
  mangled = mangled.replace('.', '_P')
  
  print 'class ' + mangled + ' {'
  print 'public:\n  static void post(Space* home, const std::vector<VarBase*>& v, '
  print '                   Reflection::ActorSpec& spec) {'
  for inst in c['inst']:
    checktype(inst['type'], inst['post'][1:])
    print '      '+inst['post'][0]+'(home, ',
    last = len(inst['post'])-2
    for i, arg in enumerate(inst['post'][1:]):
      if type(arg) == types.StringType:
        print arg,
      elif type(arg) == types.IntType:
        print 'arg' + repr(arg),
      elif type(arg) == types.TupleType:
        print 'arg' + repr(arg[0]) + '_' +repr(arg[1]),
      elif type(arg) == types.ListType:
        print 'argL' + repr(i),
      if i<last: print ', ',
    print ');'
    print '      return;'
    print '    }'
  print '    throw Exception("Registry","No suitable post function defined");'
  print '  }'
  print '  '+mangled+'() { registry.add("'+c['name']+'", &post); }'
  print '};'
  print mangled+' _'+mangled+';'
  print

print 'namespace Gecode { namespace Serialization { namespace {'
print 'class TypeDefs {'
print 'public:'
print '  Reflection::Type intvar;'
print '  Reflection::Type boolvar;'
print '  Reflection::Type offsetvar;'
print '  Reflection::Type minusvar;'
print '  Reflection::Type negboolvar;'
print '  Reflection::Type constint;'
print '  TypeDefs() : intvar("int.IntView"), boolvar("int.BoolView"), '
print '    offsetvar("int.OffsetView"), minusvar("int.MinusView"),'
print '    negboolvar("int.NegBoolView"), constint("int.ConstIntView") {}'
print '};'
print 'TypeDefs _typeDefs;'

for c in registry.registry:
  emit(c)

print '}}}'
