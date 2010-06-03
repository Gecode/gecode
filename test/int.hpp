/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

namespace Test { namespace Int {

  /*
   * Assignments
   *
   */
  inline
  Assignment::Assignment(int n0, const Gecode::IntSet& d0)
    : n(n0), d(d0) {}
  inline int
  Assignment::size(void) const {
    return n;
  }
  inline
  Assignment::~Assignment(void) {}


  inline
  CpltAssignment::CpltAssignment(int n, const Gecode::IntSet& d)
    : Assignment(n,d),
      dsv(new Gecode::IntSetValues[static_cast<unsigned int>(n)]) {
    for (int i=n; i--; )
      dsv[i].init(d);
  }
  inline bool
  CpltAssignment::operator()(void) const {
    return dsv[0]();
  }
  inline int
  CpltAssignment::operator[](int i) const {
    assert((i>=0) && (i<n));
    return dsv[i].val();
  }
  inline
  CpltAssignment::~CpltAssignment(void) {
    delete [] dsv;
  }


  forceinline int
  RandomAssignment::randval(void) {
    unsigned int skip = Base::rand(d.size());
    for (Gecode::IntSetRanges it(d); true; ++it) {
      if (it.width() > skip)
        return it.min() + static_cast<int>(skip);
      skip -= it.width();
    }
    GECODE_NEVER;
    return 0;
  }

  inline
  RandomAssignment::RandomAssignment(int n, const Gecode::IntSet& d, int a0)
    : Assignment(n,d), vals(new int[n]), a(a0) {
    for (int i=n; i--; )
      vals[i] = randval();
  }

  inline bool
  RandomAssignment::operator()(void) const {
    return a>0;
  }
  inline int
  RandomAssignment::operator[](int i) const {
    assert((i>=0) && (i<n));
    return vals[i];
  }
  inline
  RandomAssignment::~RandomAssignment(void) {
    delete [] vals;
  }

  forceinline int
  RandomMixAssignment::randval(const Gecode::IntSet& d) {
    unsigned int skip = Base::rand(d.size());
    for (Gecode::IntSetRanges it(d); true; ++it) {
      if (it.width() > skip)
        return it.min() + static_cast<int>(skip);
      skip -= it.width();
    }
    GECODE_NEVER;
    return 0;
  }

  inline
  RandomMixAssignment::RandomMixAssignment(int n0, const Gecode::IntSet& d0,
                                           int n1, const Gecode::IntSet& d1, 
                                           int a0)
    : Assignment(n0+n1,d0),vals(new int[n0+n1]),a(a0),_n1(n1),_d1(d1) {
    for (int i=n0; i--; )
      vals[i] = randval(d);
    for (int i=n1; i--; )
      vals[n0+i] = randval(_d1);
  }

  inline bool
  RandomMixAssignment::operator()(void) const {
    return a>0;
  }

  inline int
  RandomMixAssignment::operator[](int i) const {
    assert((i>=0) && (i<n));
    return vals[i];
  }

  inline
  RandomMixAssignment::~RandomMixAssignment(void) {
    delete [] vals;
  }

  /*
   * Tests with integer constraints
   *
   */
  inline
  Test::Test(const std::string& s, int a, const Gecode::IntSet& d,
             bool r, Gecode::IntConLevel i)
    : Base("Int::"+s), arity(a), dom(d), reified(r), icl(i),
      contest(icl == Gecode::ICL_DOM ? CTL_DOMAIN : CTL_NONE),
      testsearch(true), testfix(true) {}

  inline
  Test::Test(const std::string& s, int a, int min, int max,
             bool r, Gecode::IntConLevel i)
    : Base("Int::"+s), arity(a), dom(min,max), reified(r), icl(i),
      contest(icl == Gecode::ICL_DOM ? CTL_DOMAIN : CTL_NONE),
      testsearch(true), testfix(true) {}

  inline
  std::string
  Test::str(Gecode::ExtensionalPropKind epk) {
    using namespace Gecode;
    switch (epk) {
    case EPK_MEMORY: return "Memory";
    case EPK_SPEED:  return "Speed";
    default: return "Def";
    }
  }

  inline
  std::string
  Test::str(Gecode::IntConLevel icl) {
    using namespace Gecode;
    switch (icl) {
    case ICL_VAL: return "Val";
    case ICL_BND: return "Bnd";
    case ICL_DOM: return "Dom";
    default: return "Def";
    }
  }

  inline
  std::string
  Test::str(Gecode::IntRelType irt) {
    using namespace Gecode;
    switch (irt) {
    case IRT_LQ: return "Lq";
    case IRT_LE: return "Le";
    case IRT_GQ: return "Gq";
    case IRT_GR: return "Gr";
    case IRT_EQ: return "Eq";
    case IRT_NQ: return "Nq";
    default: ;
    }
    GECODE_NEVER;
    return "NONE";
  }

  inline std::string
  Test::str(Gecode::BoolOpType bot) {
    using namespace Gecode;
    switch (bot) {
    case BOT_AND: return "And";
    case BOT_OR:  return "Or";
    case BOT_IMP: return "Imp";
    case BOT_EQV: return "Eqv";
    case BOT_XOR: return "Xor";
    default: GECODE_NEVER;
    }
    GECODE_NEVER;
    return "NONE";
  }

  inline
  std::string
  Test::str(int i) {
    std::stringstream s;
    s << i;
    return s.str();
  }

  inline
  std::string
  Test::str(const Gecode::IntArgs& x) {
    std::string s = "";
    for (int i=0; i<x.size()-1; i++)
      s += str(x[i]) + ",";
    return "[" + s + str(x[x.size()-1]) + "]";
  }

  template<class T>
  inline bool
  Test::cmp(T x, Gecode::IntRelType r, T y) {
    using namespace Gecode;
    switch (r) {
    case IRT_EQ: return x == y;
    case IRT_NQ: return x != y;
    case IRT_LQ: return x <= y;
    case IRT_LE: return x < y;
    case IRT_GR: return x > y;
    case IRT_GQ: return x >= y;
    default: ;
    }
    return false;
  }


  inline
  IntConLevels::IntConLevels(void)
    : i(sizeof(icls)/sizeof(Gecode::IntConLevel)-1) {}
  inline bool
  IntConLevels::operator()(void) const {
    return i>=0;
  }
  inline void
  IntConLevels::operator++(void) {
    i--;
  }
  inline Gecode::IntConLevel
  IntConLevels::icl(void) const {
    return icls[i];
  }


  inline
  IntRelTypes::IntRelTypes(void)
    : i(sizeof(irts)/sizeof(Gecode::IntRelType)-1) {}
  inline void
  IntRelTypes::reset(void) {
    i = sizeof(irts)/sizeof(Gecode::IntRelType)-1;
  }
  inline bool
  IntRelTypes::operator()(void) const {
    return i>=0;
  }
  inline void
  IntRelTypes::operator++(void) {
    i--;
  }
  inline Gecode::IntRelType
  IntRelTypes::irt(void) const {
    return irts[i];
  }

  inline
  BoolOpTypes::BoolOpTypes(void)
    : i(sizeof(bots)/sizeof(Gecode::BoolOpType)-1) {}
  inline bool
  BoolOpTypes::operator()(void) const {
    return i>=0;
  }
  inline void
  BoolOpTypes::operator++(void) {
    i--;
  }
  inline Gecode::BoolOpType
  BoolOpTypes::bot(void) const {
    return bots[i];
  }

}}

// STATISTICS: test-int

