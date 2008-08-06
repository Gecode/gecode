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
  if (!strcmp(av, "INT_VAL_RND"))
    return Gecode::INT_VAL_RND;
  if (!strcmp(av, "INT_VAL_SPLIT_MIN"))
    return Gecode::INT_VAL_SPLIT_MIN;
  if (!strcmp(av, "INT_VAL_SPLIT_MAX"))
    return Gecode::INT_VAL_SPLIT_MAX;
  if (!strcmp(av, "INT_VALUES_MIN"))
    return Gecode::INT_VALUES_MIN;
  if (!strcmp(av, "INT_VALUES_MAX"))
    return Gecode::INT_VALUES_MAX;
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
  if (!strcmp(av, "INT_VAL_RND"))
    return true;
  if (!strcmp(av, "INT_VAL_SPLIT_MIN"))
    return true;
  if (!strcmp(av, "INT_VAL_SPLIT_MAX"))
    return true;
  if (!strcmp(av, "INT_VALUES_MIN"))
    return true;
  if (!strcmp(av, "INT_VALUES_MAX"))
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
  if (!strcmp(av, "SET_VAL_MIN_INC"))
    return Gecode::SET_VAL_MIN_INC;
  if (!strcmp(av, "SET_VAL_MIN_EXC"))
    return Gecode::SET_VAL_MIN_EXC;
  if (!strcmp(av, "SET_VAL_MAX_INC"))
    return Gecode::SET_VAL_MAX_INC;
  if (!strcmp(av, "SET_VAL_MAX_EXC"))
    return Gecode::SET_VAL_MAX_EXC;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetValBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAL_MIN_INC"))
    return true;
  if (!strcmp(av, "SET_VAL_MIN_EXC"))
    return true;
  if (!strcmp(av, "SET_VAL_MAX_INC"))
    return true;
  if (!strcmp(av, "SET_VAL_MAX_EXC"))
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
  if (!strcmp(av, "SET_VAR_RND"))
    return Gecode::SET_VAR_RND;
  if (!strcmp(av, "SET_VAR_DEGREE_MIN"))
    return Gecode::SET_VAR_DEGREE_MIN;
  if (!strcmp(av, "SET_VAR_DEGREE_MAX"))
    return Gecode::SET_VAR_DEGREE_MAX;
  if (!strcmp(av, "SET_VAR_MIN_MIN"))
    return Gecode::SET_VAR_MIN_MIN;
  if (!strcmp(av, "SET_VAR_MIN_MAX"))
    return Gecode::SET_VAR_MIN_MAX;
  if (!strcmp(av, "SET_VAR_MAX_MIN"))
    return Gecode::SET_VAR_MAX_MIN;
  if (!strcmp(av, "SET_VAR_MAX_MAX"))
    return Gecode::SET_VAR_MAX_MAX;
  if (!strcmp(av, "SET_VAR_SIZE_MIN"))
    return Gecode::SET_VAR_SIZE_MIN;
  if (!strcmp(av, "SET_VAR_SIZE_MAX"))
    return Gecode::SET_VAR_SIZE_MAX;
  throw Gecode::Reflection::ReflectionException("Internal error");
}
bool isEnum_SetVarBranch(Gecode::Reflection::Arg* a) {
  if (!a->isString())
    return false;
  const char* av = a->toString();
  if (!strcmp(av, "SET_VAR_NONE"))
    return true;
  if (!strcmp(av, "SET_VAR_RND"))
    return true;
  if (!strcmp(av, "SET_VAR_DEGREE_MIN"))
    return true;
  if (!strcmp(av, "SET_VAR_DEGREE_MAX"))
    return true;
  if (!strcmp(av, "SET_VAR_MIN_MIN"))
    return true;
  if (!strcmp(av, "SET_VAR_MIN_MAX"))
    return true;
  if (!strcmp(av, "SET_VAR_MAX_MIN"))
    return true;
  if (!strcmp(av, "SET_VAR_MAX_MAX"))
    return true;
  if (!strcmp(av, "SET_VAR_SIZE_MIN"))
    return true;
  if (!strcmp(av, "SET_VAR_SIZE_MAX"))
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
  if (!strcmp(av, "INT_VAR_RND"))
    return Gecode::INT_VAR_RND;
  if (!strcmp(av, "INT_VAR_DEGREE_MIN"))
    return Gecode::INT_VAR_DEGREE_MIN;
  if (!strcmp(av, "INT_VAR_DEGREE_MAX"))
    return Gecode::INT_VAR_DEGREE_MAX;
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
  if (!strcmp(av, "INT_VAR_RND"))
    return true;
  if (!strcmp(av, "INT_VAR_DEGREE_MIN"))
    return true;
  if (!strcmp(av, "INT_VAR_DEGREE_MAX"))
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

class PostRegistrar {
};

} // end anonymous namespace
namespace Gecode { namespace Serialization {
  
  void initRegistry(void) {
    static PostRegistrar r;
    return;
  };

  
}}
