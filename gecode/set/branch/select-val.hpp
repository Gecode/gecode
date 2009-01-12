/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode { namespace Set { namespace Branch {

  template <bool inc>
  forceinline
  ValMin<inc>::ValMin(void) {}
  template <bool inc>
  forceinline
  ValMin<inc>::ValMin(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<SetView,int>(home,vbo) {}
  template <bool inc>
  forceinline int
  ValMin<inc>::val(Space&, SetView x) const {
    UnknownRanges<SetView> u(x);
    return u.min();
  }
  template <bool inc>
  forceinline ModEvent
  ValMin<inc>::tell(Space& home, unsigned int a, SetView x, int v) {
    return ((a == 0) == inc) ? x.include(home,v) : x.exclude(home,v);
  }
  template <bool inc>
  inline Support::Symbol
  ValMin<inc>::type(void) {
    return Reflection::mangle("Gecode::Set::Branch::ValMin",inc);
  }

  template <bool inc>
  forceinline
  ValMax<inc>::ValMax(void) {}
  template <bool inc>
  forceinline
  ValMax<inc>::ValMax(Space& home, const ValBranchOptions& vbo)
    : ValSelBase<SetView,int>(home,vbo) {}
  template <bool inc>
  forceinline int
  ValMax<inc>::val(Space&, SetView x) const {
    int max = 0;
    for (UnknownRanges<SetView> u(x); u(); ++u)
      max = u.max();
    return max;
  }
  template <bool inc>
  forceinline ModEvent
  ValMax<inc>::tell(Space& home, unsigned int a, SetView x, int v) {
    return ((a == 0) == inc) ? x.include(home,v) : x.exclude(home,v);
  }
  template <bool inc>
  inline Support::Symbol
  ValMax<inc>::type(void) {
    return Reflection::mangle("Gecode::Set::Branch::ValMax",inc);
  }

}}

#define GECODE_SET_BRANCH_VALTOSTRING(eq,nq) \
public: \
  static std::ostream& toString(std::ostream& os, \
                                Space&, Reflection::VarMap& vm, \
                                const Reflection::BranchingSpec& bs, \
                                unsigned int alt) { \
    os << bs[1]->toInt(); \
    if (alt == 0) \
      os << " " << eq << " "; \
    else \
      os << " " << nq << " "; \
    int v = bs[0]->toVar(); \
    if (vm.hasName(v)) \
      os << vm.name(v).toString(); \
    else { \
      os << "_v" << v; \
    } \
    return os; \
  }

  template <>
  class ValSelToString<Set::Branch::ValMin<true> > {
    GECODE_SET_BRANCH_VALTOSTRING("in","not in")
  };
  template <>
  class ValSelToString<Set::Branch::ValMin<false> > {
    GECODE_SET_BRANCH_VALTOSTRING("not in","in")
  };
  template <>
  class ValSelToString<Set::Branch::ValMax<true> > {
    GECODE_SET_BRANCH_VALTOSTRING("in","not in")
  };
  template <>
  class ValSelToString<Set::Branch::ValMax<false> > {
    GECODE_SET_BRANCH_VALTOSTRING("not in","in")
  };
#undef GECODE_SET_BRANCH_VALTOSTRING

}

// STATISTICS: set-branch
