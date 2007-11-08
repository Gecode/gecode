/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {
  
  /// Tests for relation constraints
  namespace Rel {

    /**
      * \defgroup TaskTestSetRel Relation constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_33(-3,3);

    /// Test for binary disequality constraint
    class RelBinNEq : public SetTest {
    public:
      /// Create and register test
      RelBinNEq(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return !Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_NQ, x[1]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_NQ, x[1], b);
      }
    };
    RelBinNEq _relneq("Rel::BinNEq");

    /// Sharing test for binary disequality constraint
    class RelBinNEqS : public SetTest {
    public:
      /// Create and register test
      RelBinNEqS(const char* t)
        : SetTest(t,1,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return false;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_NQ, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_NQ, x[0], b);
      }
    };
    RelBinNEqS _relneqs("Rel::Sharing::BinNEqS");

    /// Test for binary equality constraint
    class RelBinEq : public SetTest {
    public:
      /// Create and register test
      RelBinEq(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_EQ, x[1]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_EQ, x[1], b);
      }
    };
    RelBinEq _releq("Rel::BinEq");

    /// Sharing test for binary equality constraint
    class RelBinEqS : public SetTest {
    public:
      /// Create and register test
      RelBinEqS(const char* t)
        : SetTest(t,1,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_EQ, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_EQ, x[0], b);
      }
    };
    RelBinEqS _releqs("Rel::Sharing::BinEqS");

    /// Test for binary subset constraint
    class RelBinSub : public SetTest {
    public:
      /// Create and register test
      RelBinSub(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_SUB, x[1]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_SUB, x[1], b);
      }
    };
    RelBinSub _relsub("Rel::BinSub");

    /// Sharing test for binary subset constraint
    class RelBinSubS : public SetTest {
    public:
      /// Create and register test
      RelBinSubS(const char* t)
        : SetTest(t,1,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_SUB, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_SUB, x[0], b);
      }
    };
    RelBinSubS _relsubs("Rel::Sharing::BinSubS");

    /// Test for binary disjointness constraint
    class RelBinDisj : public SetTest {
    public:
      /// Create and register test
      RelBinDisj(const char* t)
        : SetTest(t,2,ds_33) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::disjoint(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_DISJ, x[1]);
      }
    };
    RelBinDisj _reldisj("Rel::BinDisj");

    /// Sharing test for binary disjointness constraint
    class RelBinDisjS : public SetTest {
    public:
      /// Create and register test
      RelBinDisjS(const char* t)
        : SetTest(t,1,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return !xr0();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_DISJ, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_DISJ, x[0], b);
      }
    };
    RelBinDisjS _reldisjs("Rel::Sharing::BinDisjS");

    /// Test for binary complement constraint
    class RelBinCompl : public SetTest {
    public:
      /// Create and register test
      RelBinCompl(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Gecode::Set::RangesCompl<CountableSetRanges> xr1c(xr1);
        return Iter::Ranges::equal(xr0, xr1c);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_CMPL, x[1]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_CMPL, x[1], b);
      }
    };
    RelBinCompl _relcompl("Rel::BinCompl");

    /// Sharing test for binary complement constraint
    class RelBinComplS : public SetTest {
    public:
      /// Create and register test
      RelBinComplS(const char* t)
        : SetTest(t,1,ds_33,true) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return false;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SRT_CMPL, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
        Gecode::rel(home, x[0], SRT_CMPL, x[0], b);
      }
    };
    RelBinComplS _relcompls("Rel::Sharing::BinComplS");

    //@}

}}}

// STATISTICS: test-set
