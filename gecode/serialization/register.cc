/*
 *  CAUTION:
 *    This file has been automatically generated. Do not edit,
 *    edit the following files instead:
 *     - gecode/int.hh
 *     - gecode/set.hh
 *     - gecode/minimodel.hh
 *
 *  This file contains generated code fragments which are
 *  copyrighted as follows:
 *
 *  Main author:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *
 *  The generated code fragments are part of Gecode, the generic
 *  constraint development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "gecode/kernel.hh"
#if defined(GECODE_HAS_INT_VARS)
#include "gecode/int.hh"
#endif
#if defined(GECODE_HAS_SET_VARS)
#include "gecode/set.hh"
#endif
#if defined(GECODE_HAS_INT_VARS)
#include "gecode/minimodel.hh"
#endif

#include "gecode/serialization.hh"

namespace {
#if defined(GECODE_HAS_INT_VARS)
Gecode::IntValBranch toEnum_IntValBranch(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "INT_VAL_MIN"))
    return Gecode::INT_VAL_MIN;
  if (!strcmp(av, "INT_VAL_MED"))
    return Gecode::INT_VAL_MED;
  if (!strcmp(av, "INT_VAL_MAX"))
    return Gecode::INT_VAL_MAX;
  if (!strcmp(av, "INT_VAL_SPLIT_MIN"))
    return Gecode::INT_VAL_SPLIT_MIN;
  if (!strcmp(av, "INT_VAL_SPLIT_MAX"))
    return Gecode::INT_VAL_SPLIT_MAX;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_IntValBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "INT_VAL_MIN"))
    return true;
  if (!strcmp(av, "INT_VAL_MED"))
    return true;
  if (!strcmp(av, "INT_VAL_MAX"))
    return true;
  if (!strcmp(av, "INT_VAL_SPLIT_MIN"))
    return true;
  if (!strcmp(av, "INT_VAL_SPLIT_MAX"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_SET_VARS)
Gecode::SetOpType toEnum_SetOpType(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "SOT_UNION"))
    return Gecode::SOT_UNION;
  if (!strcmp(av, "SOT_DUNION"))
    return Gecode::SOT_DUNION;
  if (!strcmp(av, "SOT_INTER"))
    return Gecode::SOT_INTER;
  if (!strcmp(av, "SOT_MINUS"))
    return Gecode::SOT_MINUS;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetOpType(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SOT_UNION"))
    return true;
  if (!strcmp(av, "SOT_DUNION"))
    return true;
  if (!strcmp(av, "SOT_INTER"))
    return true;
  if (!strcmp(av, "SOT_MINUS"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_SET_VARS)
Gecode::SetValBranch toEnum_SetValBranch(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAL_MIN"))
    return Gecode::SET_VAL_MIN;
  if (!strcmp(av, "SET_VAL_MAX"))
    return Gecode::SET_VAL_MAX;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetValBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAL_MIN"))
    return true;
  if (!strcmp(av, "SET_VAL_MAX"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_SET_VARS)
Gecode::SetVarBranch toEnum_SetVarBranch(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAR_NONE"))
    return Gecode::SET_VAR_NONE;
  if (!strcmp(av, "SET_VAR_MIN_CARD"))
    return Gecode::SET_VAR_MIN_CARD;
  if (!strcmp(av, "SET_VAR_MAX_CARD"))
    return Gecode::SET_VAR_MAX_CARD;
  if (!strcmp(av, "SET_VAR_MIN_UNKNOWN_ELEM"))
    return Gecode::SET_VAR_MIN_UNKNOWN_ELEM;
  if (!strcmp(av, "SET_VAR_MAX_UNKNOWN_ELEM"))
    return Gecode::SET_VAR_MAX_UNKNOWN_ELEM;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetVarBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAR_NONE"))
    return true;
  if (!strcmp(av, "SET_VAR_MIN_CARD"))
    return true;
  if (!strcmp(av, "SET_VAR_MAX_CARD"))
    return true;
  if (!strcmp(av, "SET_VAR_MIN_UNKNOWN_ELEM"))
    return true;
  if (!strcmp(av, "SET_VAR_MAX_UNKNOWN_ELEM"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_INT_VARS)
Gecode::IntVarBranch toEnum_IntVarBranch(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "INT_VAR_NONE"))
    return Gecode::INT_VAR_NONE;
  if (!strcmp(av, "INT_VAR_MIN_MIN"))
    return Gecode::INT_VAR_MIN_MIN;
  if (!strcmp(av, "INT_VAR_MIN_MAX"))
    return Gecode::INT_VAR_MIN_MAX;
  if (!strcmp(av, "INT_VAR_MAX_MIN"))
    return Gecode::INT_VAR_MAX_MIN;
  if (!strcmp(av, "INT_VAR_MAX_MAX"))
    return Gecode::INT_VAR_MAX_MAX;
  if (!strcmp(av, "INT_VAR_SIZE_MIN"))
    return Gecode::INT_VAR_SIZE_MIN;
  if (!strcmp(av, "INT_VAR_SIZE_MAX"))
    return Gecode::INT_VAR_SIZE_MAX;
  if (!strcmp(av, "INT_VAR_DEGREE_MIN"))
    return Gecode::INT_VAR_DEGREE_MIN;
  if (!strcmp(av, "INT_VAR_DEGREE_MAX"))
    return Gecode::INT_VAR_DEGREE_MAX;
  if (!strcmp(av, "INT_VAR_SIZE_DEGREE_MIN"))
    return Gecode::INT_VAR_SIZE_DEGREE_MIN;
  if (!strcmp(av, "INT_VAR_SIZE_DEGREE_MAX"))
    return Gecode::INT_VAR_SIZE_DEGREE_MAX;
  if (!strcmp(av, "INT_VAR_REGRET_MIN_MIN"))
    return Gecode::INT_VAR_REGRET_MIN_MIN;
  if (!strcmp(av, "INT_VAR_REGRET_MIN_MAX"))
    return Gecode::INT_VAR_REGRET_MIN_MAX;
  if (!strcmp(av, "INT_VAR_REGRET_MAX_MIN"))
    return Gecode::INT_VAR_REGRET_MAX_MIN;
  if (!strcmp(av, "INT_VAR_REGRET_MAX_MAX"))
    return Gecode::INT_VAR_REGRET_MAX_MAX;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_IntVarBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "INT_VAR_NONE"))
    return true;
  if (!strcmp(av, "INT_VAR_MIN_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_MIN_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_MAX_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_MAX_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_SIZE_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_SIZE_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_DEGREE_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_DEGREE_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_SIZE_DEGREE_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_SIZE_DEGREE_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_REGRET_MIN_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_REGRET_MIN_MAX"))
    return true;
  if (!strcmp(av, "INT_VAR_REGRET_MAX_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_REGRET_MAX_MAX"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_INT_VARS)
Gecode::IntRelType toEnum_IntRelType(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "IRT_EQ"))
    return Gecode::IRT_EQ;
  if (!strcmp(av, "IRT_NQ"))
    return Gecode::IRT_NQ;
  if (!strcmp(av, "IRT_LQ"))
    return Gecode::IRT_LQ;
  if (!strcmp(av, "IRT_LE"))
    return Gecode::IRT_LE;
  if (!strcmp(av, "IRT_GQ"))
    return Gecode::IRT_GQ;
  if (!strcmp(av, "IRT_GR"))
    return Gecode::IRT_GR;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_IntRelType(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "IRT_EQ"))
    return true;
  if (!strcmp(av, "IRT_NQ"))
    return true;
  if (!strcmp(av, "IRT_LQ"))
    return true;
  if (!strcmp(av, "IRT_LE"))
    return true;
  if (!strcmp(av, "IRT_GQ"))
    return true;
  if (!strcmp(av, "IRT_GR"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_INT_VARS)
Gecode::IntConLevel toEnum_IntConLevel(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "ICL_VAL"))
    return Gecode::ICL_VAL;
  if (!strcmp(av, "ICL_BND"))
    return Gecode::ICL_BND;
  if (!strcmp(av, "ICL_DOM"))
    return Gecode::ICL_DOM;
  if (!strcmp(av, "ICL_DEF"))
    return Gecode::ICL_DEF;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_IntConLevel(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "ICL_VAL"))
    return true;
  if (!strcmp(av, "ICL_BND"))
    return true;
  if (!strcmp(av, "ICL_DOM"))
    return true;
  if (!strcmp(av, "ICL_DEF"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_INT_VARS)
Gecode::BoolOpType toEnum_BoolOpType(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "BOT_AND"))
    return Gecode::BOT_AND;
  if (!strcmp(av, "BOT_OR"))
    return Gecode::BOT_OR;
  if (!strcmp(av, "BOT_IMP"))
    return Gecode::BOT_IMP;
  if (!strcmp(av, "BOT_EQV"))
    return Gecode::BOT_EQV;
  if (!strcmp(av, "BOT_XOR"))
    return Gecode::BOT_XOR;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_BoolOpType(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "BOT_AND"))
    return true;
  if (!strcmp(av, "BOT_OR"))
    return true;
  if (!strcmp(av, "BOT_IMP"))
    return true;
  if (!strcmp(av, "BOT_EQV"))
    return true;
  if (!strcmp(av, "BOT_XOR"))
    return true;
  return false;
}
#endif
#if 1
Gecode::PropKind toEnum_PropKind(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "PK_DEF"))
    return Gecode::PK_DEF;
  if (!strcmp(av, "PK_SPEED"))
    return Gecode::PK_SPEED;
  if (!strcmp(av, "PK_MEMORY"))
    return Gecode::PK_MEMORY;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_PropKind(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "PK_DEF"))
    return true;
  if (!strcmp(av, "PK_SPEED"))
    return true;
  if (!strcmp(av, "PK_MEMORY"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_INT_VARS)
Gecode::IntAssign toEnum_IntAssign(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "INT_ASSIGN_MIN"))
    return Gecode::INT_ASSIGN_MIN;
  if (!strcmp(av, "INT_ASSIGN_MED"))
    return Gecode::INT_ASSIGN_MED;
  if (!strcmp(av, "INT_ASSIGN_MAX"))
    return Gecode::INT_ASSIGN_MAX;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_IntAssign(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "INT_ASSIGN_MIN"))
    return true;
  if (!strcmp(av, "INT_ASSIGN_MED"))
    return true;
  if (!strcmp(av, "INT_ASSIGN_MAX"))
    return true;
  return false;
}
#endif
#if defined(GECODE_HAS_SET_VARS)
Gecode::SetRelType toEnum_SetRelType(Gecode::Reflection::Arg* a) {
  assert(a->isString());
  const char* av = a->toString();
  if (!strcmp(av, "SRT_EQ"))
    return Gecode::SRT_EQ;
  if (!strcmp(av, "SRT_NQ"))
    return Gecode::SRT_NQ;
  if (!strcmp(av, "SRT_SUB"))
    return Gecode::SRT_SUB;
  if (!strcmp(av, "SRT_SUP"))
    return Gecode::SRT_SUP;
  if (!strcmp(av, "SRT_DISJ"))
    return Gecode::SRT_DISJ;
  if (!strcmp(av, "SRT_CMPL"))
    return Gecode::SRT_CMPL;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetRelType(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SRT_EQ"))
    return true;
  if (!strcmp(av, "SRT_NQ"))
    return true;
  if (!strcmp(av, "SRT_SUB"))
    return true;
  if (!strcmp(av, "SRT_SUP"))
    return true;
  if (!strcmp(av, "SRT_DISJ"))
    return true;
  if (!strcmp(av, "SRT_CMPL"))
    return true;
  return false;
}
#endif

/// Check if \a a corresponds to a variable in \a vm with type \a Var
template <class Var>
bool isVar(Gecode::Reflection::VarMap& vm, Gecode::Reflection::Arg* a) {
  if (!a->isVar())
    return false;
  Gecode::Reflection::VarSpec& s = vm.spec(a->toVar());
  typedef typename Gecode::VarViewTraits<Var>::View View;
  typedef typename Gecode::ViewVarImpTraits<View>::VarImp VarImp;
  return s.vti() == VarImp::vti;
}

/// Check if \a a corresponds to an array of \a Var variables in \a vm
template <class Var>
bool isVarArgs(Gecode::Reflection::VarMap& vm, Gecode::Reflection::Arg* a) {
  if (!a->isArray())
    return false;
  Gecode::Reflection::ArrayArg& aa = *a->toArray();
  for (int i=aa.size(); i--;)
    if (!isVar<Var>(vm, aa[i]))
      return false;
  return true;
}

bool isIntSetArgs(Gecode::Reflection::Arg* a) {
  if (!a->isArray())
    return false;
  Gecode::Reflection::ArrayArg& aa = *a->toArray();
  for (int i=aa.size(); i--;)
    if (!aa[i]->isIntArray())
      return false;
  return true;
}

class Register_rel {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::rel";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::rel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::rel(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isEnum_SetOpType(spec[0]) &&
            isVarArgs<Gecode::SetVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::SetOpType x0 = toEnum_SetOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::SetVar(vm.var(a1[i]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isEnum_SetOpType(spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::SetOpType x0 = toEnum_SetOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::rel(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isEnum_SetOpType(spec[0]) &&
            isVarArgs<Gecode::SetVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVar<Gecode::SetVar>(vm, spec[3]))
        {
          Gecode::SetOpType x0 = toEnum_SetOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::SetVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::SetVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isEnum_SetOpType(spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVar<Gecode::SetVar>(vm, spec[3]))
        {
          Gecode::SetOpType x0 = toEnum_SetOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::SetVar x3(vm.var(spec[3]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isEnum_BoolOpType(spec[0]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::BoolOpType x0 = toEnum_BoolOpType(spec[0]);
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetOpType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isEnum_SetRelType(spec[3]) &&
            isVar<Gecode::SetVar>(vm, spec[4]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::SetVar x4(vm.var(spec[4]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (spec[0]->isIntArray() &&
            isEnum_SetOpType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isEnum_SetRelType(spec[3]) &&
            isVar<Gecode::SetVar>(vm, spec[4]))
        {
          Gecode::Reflection::IntArrayArg* a0 = spec[0]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar0(a0);
          Gecode::IntSet x0(ar0);
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::SetVar x4(vm.var(spec[4]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetOpType(spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_SetRelType(spec[3]) &&
            isVar<Gecode::SetVar>(vm, spec[4]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::SetVar x4(vm.var(spec[4]->toVar()));
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetOpType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isEnum_SetRelType(spec[3]) &&
            spec[4]->isIntArray())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::Reflection::IntArrayArg* a4 = spec[4]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar4(a4);
          Gecode::IntSet x4(ar4);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (spec[0]->isIntArray() &&
            isEnum_SetOpType(spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isEnum_SetRelType(spec[3]) &&
            spec[4]->isIntArray())
        {
          Gecode::Reflection::IntArrayArg* a0 = spec[0]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar0(a0);
          Gecode::IntSet x0(ar0);
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::Reflection::IntArrayArg* a4 = spec[4]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar4(a4);
          Gecode::IntSet x4(ar4);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetOpType(spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_SetRelType(spec[3]) &&
            spec[4]->isIntArray())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetOpType x1 = toEnum_SetOpType(spec[1]);
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::SetRelType x3 = toEnum_SetRelType(spec[3]);
          Gecode::Reflection::IntArrayArg* a4 = spec[4]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar4(a4);
          Gecode::IntSet x4(ar4);
          Gecode::rel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
      }
      break;
    case 6:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::rel(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::rel(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::rel(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_BoolOpType(spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolOpType x1 = toEnum_BoolOpType(spec[1]);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::rel(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::rel");
    }
  }
};

class Register_sqrt {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::sqrt";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::sqrt(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqrt");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::sqrt(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqrt");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::sqrt(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqrt");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqrt");
    }
  }
};

class Register_selectInterIn {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::selectInterIn";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 4:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            spec[3]->isIntArray())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::Reflection::IntArrayArg* a3 = spec[3]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar3(a3);
          Gecode::IntSet x3(ar3);
          Gecode::selectInterIn(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectInterIn");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectInterIn");
    }
  }
};

class Register_sequence {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::sequence";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 1:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::sequence(home, x0);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sequence");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sequence");
    }
  }
};

class Register_sorted {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::sorted";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::sorted(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sorted");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::sorted(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sorted");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::sorted(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sorted");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sorted");
    }
  }
};

class Register_exactly {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::exactly";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::exactly(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::exactly");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::exactly(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::exactly");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::exactly(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::exactly");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::exactly");
    }
  }
};

class Register_max {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::max";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::max(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::max(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::max");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::max(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::max(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::max");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::max(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::max(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::max");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::max(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::max");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::max");
    }
  }
};

class Register_distinct {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::distinct";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 1:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::distinct(home, x0);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::distinct");
      }
      break;
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::distinct(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::distinct(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::distinct");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]) &&
            isEnum_PropKind(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::PropKind x2 = toEnum_PropKind(spec[2]);
          Gecode::distinct(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::distinct(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::distinct");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::distinct(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::distinct");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::distinct");
    }
  }
};

class Register_dom {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::dom";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray())
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::dom(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::dom(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isIntArray())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::dom(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isInt() &&
            spec[3]->isInt())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isIntArray() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          Gecode::Reflection::IntArrayArg* a2 = spec[2]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar2(a2);
          Gecode::IntSet x2(ar2);
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::dom(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::dom(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::dom(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::dom(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::dom(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetRelType(spec[1]) &&
            spec[2]->isInt() &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetRelType x1 = toEnum_SetRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::dom(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
      }
      break;
    case 6:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::dom(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::dom");
    }
  }
};

class Register_count {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::count";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::count(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::count(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::count(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::count(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::count(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]) &&
            spec[2]->isIntArray())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::count(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isIntArray())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::count(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::count(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isIntSetArgs(spec[1]) &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::IntSetArgs x1(a1.size());
          for (int i=a1.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa1 = a1[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar1(aa1);
            x1[i] = Gecode::IntSet(aar1);
          }
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isIntArray() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg* a1 = spec[1]->toIntArray();
          Gecode::Reflection::IntArrayArgRanges ar1(a1);
          Gecode::IntSet x1(ar1);
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::count(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
      }
      break;
    case 6:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::count(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::count");
    }
  }
};

class Register_match {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::match";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::match(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::match");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::match");
    }
  }
};

class Register_selectUnion {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::selectUnion";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::selectUnion(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isIntSetArgs(spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::IntSetArgs x0(a0.size());
          for (int i=a0.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa0 = a0[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar0(aa0);
            x0[i] = Gecode::IntSet(aar0);
          }
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::selectUnion(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectUnion");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectUnion");
    }
  }
};

class Register_element {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::element";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::element(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::element");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::element(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::element");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::BoolVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::element(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::element");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::element");
    }
  }
};

class Register_abs {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::abs";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::abs(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::abs");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::abs(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::abs");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::abs(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::abs");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::abs");
    }
  }
};

class Register_atmost {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::atmost";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atmost(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmost");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atmost(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmost");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atmost(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmost");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmost");
    }
  }
};

class Register_circuit {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::circuit";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 1:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::circuit(home, x0);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::circuit");
      }
      break;
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::circuit(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::circuit");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]) &&
            isEnum_PropKind(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::PropKind x2 = toEnum_PropKind(spec[2]);
          Gecode::circuit(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::circuit");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::circuit");
    }
  }
};

class Register_assign {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::assign";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntAssign(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntAssign x1 = toEnum_IntAssign(spec[1]);
          Gecode::assign(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntAssign(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntAssign x1 = toEnum_IntAssign(spec[1]);
          Gecode::assign(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::assign");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::assign");
    }
  }
};

class Register_mult {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::mult";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::mult(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::mult");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::mult(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::mult");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::mult(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::mult");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::mult");
    }
  }
};

class Register_atmostOne {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::atmostOne";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            spec[1]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          unsigned int x1 = static_cast<unsigned int>(spec[1]->toInt());
          Gecode::atmostOne(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmostOne");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atmostOne");
    }
  }
};

class Register_convex {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::convex";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 1:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::convex(home, x0);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::convex");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::convex");
    }
  }
};

class Register_unshare {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::unshare";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 1:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::unshare(home, x0);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::unshare(home, x0);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::unshare");
      }
      break;
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::unshare(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::unshare(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::unshare");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]) &&
            isEnum_PropKind(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::PropKind x2 = toEnum_PropKind(spec[2]);
          Gecode::unshare(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntConLevel(spec[1]) &&
            isEnum_PropKind(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntConLevel x1 = toEnum_IntConLevel(spec[1]);
          Gecode::PropKind x2 = toEnum_PropKind(spec[2]);
          Gecode::unshare(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::unshare");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::unshare");
    }
  }
};

class Register_weights {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::weights";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 4:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (spec[0]->isIntArray() &&
            spec[1]->isIntArray() &&
            isVar<Gecode::SetVar>(vm, spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::weights(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::weights");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::weights");
    }
  }
};

class Register_selectInter {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::selectInter";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::selectInter(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectInter");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectInter");
    }
  }
};

class Register_selectDisjoint {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::selectDisjoint";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::selectDisjoint(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectDisjoint");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectDisjoint");
    }
  }
};

class Register_linear {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::linear";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::linear(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::linear(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::linear(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::linear(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::linear(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
      }
      break;
    case 6:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            spec[2]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isVar<Gecode::BoolVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::BoolVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isEnum_IntConLevel(spec[4]) &&
            isEnum_PropKind(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::IntConLevel x4 = toEnum_IntConLevel(spec[4]);
          Gecode::PropKind x5 = toEnum_PropKind(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
      }
      break;
    case 7:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]) &&
            isEnum_PropKind(spec[6]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::PropKind x6 = toEnum_PropKind(spec[6]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]) &&
            isEnum_PropKind(spec[6]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::PropKind x6 = toEnum_PropKind(spec[6]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            spec[3]->isInt() &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]) &&
            isEnum_PropKind(spec[6]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          int x3 = static_cast<int>(spec[3]->toInt());
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::PropKind x6 = toEnum_PropKind(spec[6]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntRelType(spec[2]) &&
            isVar<Gecode::IntVar>(vm, spec[3]) &&
            isVar<Gecode::BoolVar>(vm, spec[4]) &&
            isEnum_IntConLevel(spec[5]) &&
            isEnum_PropKind(spec[6]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::BoolVar(vm.var(a1[i]->toVar()));
          Gecode::IntRelType x2 = toEnum_IntRelType(spec[2]);
          Gecode::IntVar x3(vm.var(spec[3]->toVar()));
          Gecode::BoolVar x4(vm.var(spec[4]->toVar()));
          Gecode::IntConLevel x5 = toEnum_IntConLevel(spec[5]);
          Gecode::PropKind x6 = toEnum_PropKind(spec[6]);
          Gecode::linear(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::linear");
    }
  }
};

class Register_cumulatives {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::cumulatives";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 7:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt())
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cumulatives");
      }
      break;
    case 8:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cumulatives");
      }
      break;
    case 9:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[4]) &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::ArrayArg& a4 = *spec[4]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x4(a4.size());
          for (int i=a4.size(); i--;)
            x4[i] = Gecode::IntVar(vm.var(a4[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (spec[0]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isIntArray() &&
            isVarArgs<Gecode::IntVar>(vm, spec[3]) &&
            spec[4]->isIntArray() &&
            spec[5]->isIntArray() &&
            spec[6]->isInt() &&
            isEnum_IntConLevel(spec[7]) &&
            isEnum_PropKind(spec[8]))
        {
          Gecode::Reflection::IntArrayArg& a0 = *spec[0]->toIntArray();
          Gecode::IntArgs x0(a0.size());
          for (int i=a0.size(); i--;) x0[i] = a0[i];
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a2 = *spec[2]->toIntArray();
          Gecode::IntArgs x2(a2.size());
          for (int i=a2.size(); i--;) x2[i] = a2[i];
          Gecode::Reflection::ArrayArg& a3 = *spec[3]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x3(a3.size());
          for (int i=a3.size(); i--;)
            x3[i] = Gecode::IntVar(vm.var(a3[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a4 = *spec[4]->toIntArray();
          Gecode::IntArgs x4(a4.size());
          for (int i=a4.size(); i--;) x4[i] = a4[i];
          Gecode::Reflection::IntArrayArg& a5 = *spec[5]->toIntArray();
          Gecode::IntArgs x5(a5.size());
          for (int i=a5.size(); i--;) x5[i] = a5[i];
          bool x6 = static_cast<bool>(spec[6]->toInt());
          Gecode::IntConLevel x7 = toEnum_IntConLevel(spec[7]);
          Gecode::PropKind x8 = toEnum_PropKind(spec[8]);
          Gecode::cumulatives(home, x0,x1,x2,x3,x4,x5,x6,x7,x8);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cumulatives");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cumulatives");
    }
  }
};

class Register_channel {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::channel";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::BoolVar>(vm, spec[1]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolVar x1(vm.var(spec[1]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::SetVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::SetVar(vm.var(a1[i]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::channel(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::channel");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::channel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::channel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::channel(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::channel(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::channel");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::ArrayArg& a1 = *spec[1]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x1(a1.size());
          for (int i=a1.size(); i--;)
            x1[i] = Gecode::IntVar(vm.var(a1[i]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::channel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::BoolVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::channel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::BoolVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::BoolVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::channel(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::channel(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::channel");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::channel(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::channel");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::channel");
    }
  }
};

class Register_cardinality {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::cardinality";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::cardinality(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cardinality");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          unsigned int x1 = static_cast<unsigned int>(spec[1]->toInt());
          unsigned int x2 = static_cast<unsigned int>(spec[2]->toInt());
          Gecode::cardinality(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cardinality");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::cardinality");
    }
  }
};

class Register_selectSet {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::selectSet";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::selectSet(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isIntSetArgs(spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::SetVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::IntSetArgs x0(a0.size());
          for (int i=a0.size(); i--;) {
            Gecode::Reflection::IntArrayArg* aa0 = a0[i]->toIntArray();
            Gecode::Reflection::IntArrayArgRanges aar0(aa0);
            x0[i] = Gecode::IntSet(aar0);
          }
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::SetVar x2(vm.var(spec[2]->toVar()));
          Gecode::selectSet(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectSet");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::selectSet");
    }
  }
};

class Register_min {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::min";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::min(home, x0,x1);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::min(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::min");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::min(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::min(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::min");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::min(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::min(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::min");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::min(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::min");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::min");
    }
  }
};

class Register_sequentialUnion {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::sequentialUnion";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::sequentialUnion(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sequentialUnion");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sequentialUnion");
    }
  }
};

class Register_branch {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::branch";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntVarBranch(spec[1]) &&
            isEnum_IntValBranch(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVarBranch x1 = toEnum_IntVarBranch(spec[1]);
          Gecode::IntValBranch x2 = toEnum_IntValBranch(spec[2]);
          Gecode::branch(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntVarBranch(spec[1]) &&
            isEnum_IntValBranch(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntVarBranch x1 = toEnum_IntVarBranch(spec[1]);
          Gecode::IntValBranch x2 = toEnum_IntValBranch(spec[2]);
          Gecode::branch(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_SET_VARS)
        if (isVarArgs<Gecode::SetVar>(vm, spec[0]) &&
            isEnum_SetVarBranch(spec[1]) &&
            isEnum_SetValBranch(spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::SetVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::SetVar(vm.var(a0[i]->toVar()));
          Gecode::SetVarBranch x1 = toEnum_SetVarBranch(spec[1]);
          Gecode::SetValBranch x2 = toEnum_SetValBranch(spec[2]);
          Gecode::branch(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::branch");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::branch");
    }
  }
};

class Register_atleast {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::atleast";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt())
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::atleast(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atleast");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::atleast(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atleast");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            spec[2]->isInt() &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          int x2 = static_cast<int>(spec[2]->toInt());
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isInt() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          int x1 = static_cast<int>(spec[1]->toInt());
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            spec[1]->isIntArray() &&
            isVar<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::Reflection::IntArrayArg& a1 = *spec[1]->toIntArray();
          Gecode::IntArgs x1(a1.size());
          for (int i=a1.size(); i--;) x1[i] = a1[i];
          Gecode::IntVar x2(vm.var(spec[2]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::atleast(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atleast");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::atleast");
    }
  }
};

class Register_convexHull {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::convexHull";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_SET_VARS)
        if (isVar<Gecode::SetVar>(vm, spec[0]) &&
            isVar<Gecode::SetVar>(vm, spec[1]))
        {
          Gecode::SetVar x0(vm.var(spec[0]->toVar()));
          Gecode::SetVar x1(vm.var(spec[1]->toVar()));
          Gecode::convexHull(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::convexHull");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::convexHull");
    }
  }
};

class Register_sqr {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::sqr";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 2:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::sqr(home, x0,x1);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqr");
      }
      break;
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::sqr(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqr");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVar<Gecode::IntVar>(vm, spec[0]) &&
            isVar<Gecode::IntVar>(vm, spec[1]) &&
            isEnum_IntConLevel(spec[2]) &&
            isEnum_PropKind(spec[3]))
        {
          Gecode::IntVar x0(vm.var(spec[0]->toVar()));
          Gecode::IntVar x1(vm.var(spec[1]->toVar()));
          Gecode::IntConLevel x2 = toEnum_IntConLevel(spec[2]);
          Gecode::PropKind x3 = toEnum_PropKind(spec[3]);
          Gecode::sqr(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqr");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::sqr");
    }
  }
};

class Register_lex {
public:
  /// Identifier for this post function
  static Gecode::Support::Symbol ati(void) {
    return "Gecode::Post::lex";
  }
  
  /// Post constraint described by \a spec
  static void post(Gecode::Space* home, Gecode::Reflection::VarMap& vm,
                   const Gecode::Reflection::ActorSpec& spec) {
    switch(spec.noOfArgs()) {
    case 3:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::lex(home, x0,x1,x2);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::lex(home, x0,x1,x2);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::lex");
      }
      break;
    case 4:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::lex(home, x0,x1,x2,x3);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::lex(home, x0,x1,x2,x3);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::lex");
      }
      break;
    case 5:
      {
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::IntVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::IntVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::IntVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::IntVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::IntVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::lex(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
#if defined(GECODE_HAS_INT_VARS)
        if (isVarArgs<Gecode::BoolVar>(vm, spec[0]) &&
            isEnum_IntRelType(spec[1]) &&
            isVarArgs<Gecode::BoolVar>(vm, spec[2]) &&
            isEnum_IntConLevel(spec[3]) &&
            isEnum_PropKind(spec[4]))
        {
          Gecode::Reflection::ArrayArg& a0 = *spec[0]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x0(a0.size());
          for (int i=a0.size(); i--;)
            x0[i] = Gecode::BoolVar(vm.var(a0[i]->toVar()));
          Gecode::IntRelType x1 = toEnum_IntRelType(spec[1]);
          Gecode::Reflection::ArrayArg& a2 = *spec[2]->toArray();
          Gecode::VarArgArray<Gecode::BoolVar> x2(a2.size());
          for (int i=a2.size(); i--;)
            x2[i] = Gecode::BoolVar(vm.var(a2[i]->toVar()));
          Gecode::IntConLevel x3 = toEnum_IntConLevel(spec[3]);
          Gecode::PropKind x4 = toEnum_PropKind(spec[4]);
          Gecode::lex(home, x0,x1,x2,x3,x4);
          return;
        }
#endif
        throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::lex");
      }
      break;
    default: throw Gecode::Reflection::ReflectionException("Argument type mismatch for Gecode::Post::lex");
    }
  }
};

class PostRegistrar {
  GECODE_REGISTER1(Register_rel);
  GECODE_REGISTER1(Register_sqrt);
  GECODE_REGISTER1(Register_selectInterIn);
  GECODE_REGISTER1(Register_sequence);
  GECODE_REGISTER1(Register_sorted);
  GECODE_REGISTER1(Register_exactly);
  GECODE_REGISTER1(Register_max);
  GECODE_REGISTER1(Register_distinct);
  GECODE_REGISTER1(Register_dom);
  GECODE_REGISTER1(Register_count);
  GECODE_REGISTER1(Register_match);
  GECODE_REGISTER1(Register_selectUnion);
  GECODE_REGISTER1(Register_element);
  GECODE_REGISTER1(Register_abs);
  GECODE_REGISTER1(Register_atmost);
  GECODE_REGISTER1(Register_circuit);
  GECODE_REGISTER1(Register_assign);
  GECODE_REGISTER1(Register_mult);
  GECODE_REGISTER1(Register_atmostOne);
  GECODE_REGISTER1(Register_convex);
  GECODE_REGISTER1(Register_unshare);
  GECODE_REGISTER1(Register_weights);
  GECODE_REGISTER1(Register_selectInter);
  GECODE_REGISTER1(Register_selectDisjoint);
  GECODE_REGISTER1(Register_linear);
  GECODE_REGISTER1(Register_cumulatives);
  GECODE_REGISTER1(Register_channel);
  GECODE_REGISTER1(Register_cardinality);
  GECODE_REGISTER1(Register_selectSet);
  GECODE_REGISTER1(Register_min);
  GECODE_REGISTER1(Register_sequentialUnion);
  GECODE_REGISTER1(Register_branch);
  GECODE_REGISTER1(Register_atleast);
  GECODE_REGISTER1(Register_convexHull);
  GECODE_REGISTER1(Register_sqr);
  GECODE_REGISTER1(Register_lex);
};

} // end anonymous namespace
namespace Gecode { namespace Serialization {
  
  void initRegistry(void) {
    static PostRegistrar r;
    return;
  };

  
}}
