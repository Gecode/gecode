/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
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

namespace Gecode { namespace Int { namespace Branch {

  template<class View>
  forceinline
  ValMin<View>::ValMin(void) {}
  template<class View>
  forceinline
  ValMin<View>::ValMin(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<View,int>(home,vbo) {}
  template<class View>
  forceinline int
  ValMin<View>::val(Space&, View x) const {
    return x.min();
  }
  template<class View>
  forceinline ModEvent
  ValMin<View>::tell(Space& home, unsigned int a, View x, int n) {
    return (a == 0) ? x.eq(home,n) : x.gr(home,n);
  }
  template<class View>
  inline Support::Symbol
  ValMin<View>::type(void) {
    return Reflection::mangle<View>("Gecode::Int::Branch::ValMin");
  }

  template<class View>
  forceinline
  ValMed<View>::ValMed(void) {}
  template<class View>
  forceinline
  ValMed<View>::ValMed(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<View,int>(home,vbo) {}
  template<class View>
  forceinline int
  ValMed<View>::val(Space&, View x) const {
    return x.med();
  }
  template<class View>
  forceinline ModEvent
  ValMed<View>::tell(Space& home, unsigned int a, View x, int n) {
    return (a == 0) ? x.eq(home,n) : x.nq(home,n);
  }
  template<class View>
  inline Support::Symbol
  ValMed<View>::type(void) {
    return Reflection::mangle<View>("Gecode::Int::Branch::ValMed");
  }

  template<class View>
  forceinline
  ValRnd<View>::ValRnd(void) {}
  template<class View>
  forceinline
  ValRnd<View>::ValRnd(Space&, const ValBranchOptions& vbo) 
    : r(vbo.seed) {}
  template<class View>
  forceinline int
  ValRnd<View>::val(Space&, View x) {
    unsigned int p = r(x.size());
    for (ViewRanges<View> i(x); i(); ++i) {
      if (i.width() > p)
        return i.min() + static_cast<int>(p);
      p -= i.width();
    }
    GECODE_NEVER;
    return 0;
  }
  template<class View>
  forceinline ModEvent
  ValRnd<View>::tell(Space& home, unsigned int a, View x, int n) {
    return (a == 0) ? x.eq(home,n) : x.nq(home,n);
  }
  template<class View>
  forceinline Support::RandomGenerator
  ValRnd<View>::description(Space&) {
    return r;
  }
  template<class View>
  forceinline void 
  ValRnd<View>::commit(Space&, const Support::RandomGenerator& d, 
                       unsigned int) {
    r = d;
  }
  template<class View>
  forceinline void 
  ValRnd<View>::update(Space&, bool, ValRnd<View>& vr) {
    r = vr.r;
  }
  template<class View>
  forceinline void
  ValRnd<View>::dispose(Space&) {}
  template<class View>
  inline Support::Symbol
  ValRnd<View>::type(void) {
    return Reflection::mangle<View>("Gecode::Int::Branch::ValRnd");
  }

  template<class View>
  forceinline
  ValSplitMin<View>::ValSplitMin(void) {}
  template<class View>
  forceinline
  ValSplitMin<View>::ValSplitMin(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<View,int>(home,vbo) {}
  template<class View>
  forceinline int
  ValSplitMin<View>::val(Space&, View x) const {
    return (x.width() == 2) ? x.min() : ((x.min()+x.max()) / 2);
  }
  template<class View>
  forceinline ModEvent
  ValSplitMin<View>::tell(Space& home, unsigned int a, View x, int n) {
    return (a == 0) ? x.lq(home,n) : x.gr(home,n);
  }
  template<class View>
  inline Support::Symbol
  ValSplitMin<View>::type(void) {
    return Reflection::mangle<View>("Gecode::Int::Branch::ValSplitMin");
  }


  inline Support::Symbol
  NoValue::type(void) {
    return Support::Symbol("Gecode::Int::Branch::NoValue");
  }

  forceinline
  NoValue::operator int(void) const {
    return 0;
  }

  template<class View>
  forceinline
  ValZeroOne<View>::ValZeroOne(void) {}
  template<class View>
  forceinline
  ValZeroOne<View>::ValZeroOne(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<View,NoValue>(home,vbo) {}
  template<class View>
  forceinline NoValue
  ValZeroOne<View>::val(Space&, View) const {
    NoValue n; return n;
  }
  template<class View>
  forceinline ModEvent
  ValZeroOne<View>::tell(Space& home, unsigned int a, View x, NoValue) {
    return (a == 0) ? x.zero(home) : x.one(home);
  }
  template<class View>
  inline Support::Symbol
  ValZeroOne<View>::type(void) {
    return Reflection::mangle<View>("Gecode::Int::Branch::ValZeroOne");
  }

  template<class View>
  forceinline
  AssignValMin<View>::AssignValMin(void) {}
  template<class View>
  forceinline
  AssignValMin<View>::AssignValMin(Space& home, const ValBranchOptions& vbo)
    : ValMin<View>(home,vbo) {}

  template<class View>
  forceinline
  AssignValMed<View>::AssignValMed(void) {}
  template<class View>
  forceinline
  AssignValMed<View>::AssignValMed(Space& home, const ValBranchOptions& vbo)
    : ValMed<View>(home,vbo) {}

  template<class View>
  forceinline
  AssignValZeroOne<View>::AssignValZeroOne(void) {}
  template<class View>
  forceinline
  AssignValZeroOne<View>::AssignValZeroOne(Space& home, 
                                           const ValBranchOptions& vbo)
    : ValZeroOne<View>(home,vbo) {}

}}

#define GECODE_INT_BRANCH_VALTOSTRING(multiplier,eq,nq,isBool) \
public: \
  static std::ostream& toString(std::ostream& os, \
                                Space&, Reflection::VarMap& vm, \
                                const Reflection::BranchingSpec& bs, \
                                unsigned int alt) { \
    int v = bs[0]->toVar(); \
    if (vm.hasName(v)) \
      os << vm.name(v).toString(); \
    else { \
      os << "_v" << v; \
    } \
    if (alt == 0) \
      os << " " << eq << " "; \
    else \
      os << " " << nq << " "; \
    if (isBool) \
      os << multiplier; \
    else \
      os << multiplier*bs[1]->toInt(); \
    return os; \
  }

  template <>
  class ValSelToString<Int::Branch::ValMin<Int::IntView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"==",">",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValMin<Int::MinusView> > {
    GECODE_INT_BRANCH_VALTOSTRING(-1,"==","<",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValMed<Int::IntView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"==","!=",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValRnd<Int::IntView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"==","!=",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValRnd<Int::BoolView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"==","!=",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValSplitMin<Int::IntView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"<=",">",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValSplitMin<Int::MinusView> > {
    GECODE_INT_BRANCH_VALTOSTRING(-1,">=","<",false)
  };
  template <>
  class ValSelToString<Int::Branch::ValZeroOne<Int::BoolView> > {
    GECODE_INT_BRANCH_VALTOSTRING(0,"==","!=",true)
  };
  template <>
  class ValSelToString<Int::Branch::ValZeroOne<Int::NegBoolView> > {
    GECODE_INT_BRANCH_VALTOSTRING(1,"==","!=",true)
  };

#undef GECODE_INT_BRANCH_VALTOSTRING

}


// STATISTICS: int-branch

