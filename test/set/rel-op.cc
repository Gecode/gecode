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

  /// Tests for relation/operation constraints
  namespace RelOp {

    /**
      * \defgroup TaskTestSetRelOp Relation/operation constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_33(-2,2);
    static IntSet ds_22(-1,2);
    
    /// Test for ternary union constraint
    class RelUnionEq : public SetTest {
    public:
      /// Create and register test
      RelUnionEq(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> 
          u(xr0,xr1);
        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_EQ, x[2]);
      }
    };
    RelUnionEq _relunioneq("RelOp::UnionEq");

    /// Sharing test for ternary union constraint
    class RelUnionEqS1 : public SetTest {
    public:
      /// Create and register test
      RelUnionEqS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_EQ, x[1]);
      }
    };
    RelUnionEqS1 _relunioneqs1("RelOp::Sharing::UnionEqS1");

    /// Sharing test for ternary union constraint
    class RelUnionEqS2 : public SetTest {
    public:
      /// Create and register test
      RelUnionEqS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr1, xr0);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_EQ, x[0]);
      }
    };
    RelUnionEqS2 _relunioneqs2("RelOp::Sharing::UnionEqS2");

    /// Sharing test for ternary union constraint
    class RelUnionEqS3 : public SetTest {
    public:
      /// Create and register test
      RelUnionEqS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_EQ, x[0]);
      }
    };
    RelUnionEqS3 _relunioneqs3("RelOp::Sharing::UnionEqS3");

    /// Test for ternary partition constraint
    class RelDUnionEq : public SetTest {
    public:
      /// Create and register test
      RelDUnionEq(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> 
          u(xr0,xr1);
        CountableSetRanges xr02(x.lub, x[0]);
        CountableSetRanges xr12(x.lub, x[1]);

        return
          Iter::Ranges::equal(xr2, u) &&
          Iter::Ranges::disjoint(xr02, xr12);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_DUNION, x[1], SRT_EQ, x[2]);
      }
    };
    RelDUnionEq _reldunioneq("RelOp::DUnionEq");

    /// Sharing test for ternary partition constraint
    class RelDUnionEqS1 : public SetTest {
    public:
      /// Create and register test
      RelDUnionEqS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return !xr0() && !xr1();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_DUNION, x[0], SRT_EQ, x[1]);
      }
    };
    RelDUnionEqS1 _reldunioneqs1("RelOp::Sharing::DUnionEqS1");

    /// Sharing test for ternary partition constraint
    class RelDUnionEqS2 : public SetTest {
    public:
      /// Create and register test
      RelDUnionEqS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return !xr0();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_DUNION, x[1], SRT_EQ, x[1]);
      }
    };
    RelDUnionEqS2 _reldunioneqs2("RelOp::Sharing::DUnionEqS2");

    /// Sharing test for ternary partition constraint
    class RelDUnionEqS3 : public SetTest {
    public:
      /// Create and register test
      RelDUnionEqS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return !xr0();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_DUNION, x[0], SRT_EQ, x[0]);
      }
    };
    RelDUnionEqS3 _reldunioneqs3("RelOp::Sharing::DUnionEqS3");

    /// Test for ternary intersection constraint
    class RelInterEq : public SetTest {
    public:
      /// Create and register test
      RelInterEq(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> 
          u(xr0,xr1);
        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_EQ, x[2]);
      }
    };
    RelInterEq _relintereq("RelOp::InterEq");

    /// Sharing test for ternary intersection constraint
    class RelInterEqS1 : public SetTest {
    public:
      /// Create and register test
      RelInterEqS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_EQ, x[0]);
      }
    };
    RelInterEqS1 _relintereqs1("RelOp::Sharing::InterEqS1");

    /// Sharing test for ternary intersection constraint
    class RelInterEqS2 : public SetTest {
    public:
      /// Create and register test
      RelInterEqS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_EQ, x[1]);
      }
    };
    RelInterEqS2 _relintereqs2("RelOp::Sharing::InterEqS2");

    /// Sharing test for ternary intersection constraint
    class RelInterEqS3 : public SetTest {
    public:
      /// Create and register test
      RelInterEqS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_EQ, x[0]);
      }
    };
    RelInterEqS3 _relintereqs3("RelOp::Sharing::InterEqS3");

    /// Test for intersection/subset constraint
    class RelInterSub : public SetTest {
    public:
      /// Create and register test
      RelInterSub(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> 
          u(xr0,xr1);
        return Iter::Ranges::subset(u, xr2);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_SUB, x[2]);
      }
    };
    RelInterSub _relintersub("RelOp::InterSub");

    /// Sharing test for intersection/subset constraint
    class RelInterSubS1 : public SetTest {
    public:
      /// Create and register test
      RelInterSubS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_SUB, x[1]);
      }
    };
    RelInterSubS1 _relintersub1("RelOp::Sharing::InterSub1");

    /// Sharing test for intersection/subset constraint
    class RelInterSubS2 : public SetTest {
    public:
      /// Create and register test
      RelInterSubS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr0, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_SUB, x[1]);
      }
    };
    RelInterSubS2 _relintersub2("RelOp::Sharing::InterSub2");

    /// Sharing test for intersection/subset constraint
    class RelInterSubS3 : public SetTest {
    public:
      /// Create and register test
      RelInterSubS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_SUB, x[0]);
      }
    };
    RelInterSubS3 _relintersub3("RelOp::Sharing::InterSub3");

    /// Test for set-minus constraint
    class RelMinusEq : public SetTest {
    public:
      /// Create and register test
      RelMinusEq(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_MINUS, x[1], SRT_EQ, x[2]);
      }
    };
    RelMinusEq _relminuseq("RelOp::MinusEq");

    /// Sharing test for set-minus constraint
    class RelMinusEqS1 : public SetTest {
    public:
      /// Create and register test
      RelMinusEqS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr12(x.lub, x[1]);
        Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

        return Iter::Ranges::equal(xr12, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_MINUS, x[1], SRT_EQ, x[1]);
      }
    };
    RelMinusEqS1 _relminuseqs1("RelOp::Sharing::MinusEqS1");

    /// Sharing test for set-minus constraint
    class RelMinusEqS2 : public SetTest {
    public:
      /// Create and register test
      RelMinusEqS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr1(x.lub, x[1]);
        return !xr1();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_MINUS, x[0], SRT_EQ, x[1]);
      }
    };
    RelMinusEqS2 _relminuseqs2("RelOp::Sharing::MinusEqS2");

    /// Sharing test for set-minus constraint
    class RelMinusEqS3 : public SetTest {
    public:
      /// Create and register test
      RelMinusEqS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return !xr0();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_MINUS, x[0], SRT_EQ, x[0]);
      }
    };
    RelMinusEqS3 _relminuseqs3("RelOp::Sharing::MinusEqS3");

    /// Test for union/superset constraint
    class RelUnionSup : public SetTest {
    public:
      /// Create and register test
      RelUnionSup(const char* t)
        : SetTest(t,3,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> 
          u(xr0,xr1);
        return Iter::Ranges::subset(xr2, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_SUP, x[2]);
      }
    };
    RelUnionSup _relunionsup("RelOp::UnionSup");

    /// Sharing test for union/superset constraint
    class RelUnionSupS1 : public SetTest {
    public:
      /// Create and register test
      RelUnionSupS1(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_SUP, x[1]);
      }
    };
    RelUnionSupS1 _relunionsups1("RelOp::Sharing::UnionSupS1");

    /// Sharing test for union/superset constraint
    class RelUnionSupS2 : public SetTest {
    public:
      /// Create and register test
      RelUnionSupS2(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr1, xr0);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_SUP, x[1]);
      }
    };
    RelUnionSupS2 _relunionsups2("RelOp::Sharing::UnionSupS2");

    /// Sharing test for union/superset constraint
    class RelUnionSupS3 : public SetTest {
    public:
      /// Create and register test
      RelUnionSupS3(const char* t)
        : SetTest(t,1,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        (void)x;
        return true;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_SUP, x[0]);
      }
    };
    RelUnionSupS3 _relunionsups3("RelOp::Sharing::UnionSupS3");

    /// Test for n-ary partition constraint
    class RelDUnionN : public SetTest {
    private:
      int n;
    public:
      /// Create and register test
      RelDUnionN(int n0)
        : SetTest("RelOp::DUnionN::"+str(n0),n0+1,ds_22,false), n(n0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        unsigned int cardSum = 0;
        for (int i=0; i<n; i++) {
          CountableSetRanges xir(x.lub, x[i]);
          cardSum += Iter::Ranges::size(xir);
        }
        {
          CountableSetRanges xnr(x.lub, x[n]);
          if (cardSum != Iter::Ranges::size(xnr))
            return false;
        }

        GECODE_AUTOARRAY(CountableSetRanges, isrs, n);
        for (int i=n; i--; )        
          isrs[i].init(x.lub, x[i]);
        Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, n);
        CountableSetRanges xnr(x.lub, x[n]);
        return Iter::Ranges::equal(u, xnr);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-1);
        for (int i=x.size()-1; i--;)
          xs[i]=x[i];
        Gecode::rel(home, SOT_DUNION, xs, x[x.size()-1]);
      }
    };
    RelDUnionN _reldunionn0(0);
    RelDUnionN _reldunionn1(1);
    RelDUnionN _reldunionn3(3);

    /// Sharing test for n-ary partition constraint
    class RelDUnionNS1 : public SetTest {
    public:
      /// Create and register test
      RelDUnionNS1(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        return !xr0() && Iter::Ranges::equal(xr1,xr2);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[0];
        xs[2]=x[1];
        Gecode::rel(home, SOT_DUNION, xs, x[2]);
      }
    };
    RelDUnionNS1 _reldunionns1("RelOp::Sharing::DUnionNS1");

    /// Sharing test for n-ary partition constraint
    class RelDUnionNS2 : public SetTest {
    public:
      /// Create and register test
      RelDUnionNS2(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return !xr0() && !xr1();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[2];
        Gecode::rel(home, SOT_DUNION, xs, x[2]);
      }
    };
    RelDUnionNS2 _reldunionns2("RelOp::Sharing::DUnionNS2");

    /// Sharing test for n-ary partition constraint
    class RelDUnionNS3 : public SetTest {
    public:
      /// Create and register test
      RelDUnionNS3(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return !xr0() && !xr1();
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[0];
        Gecode::rel(home, SOT_DUNION, xs, x[0]);
      }
    };
    RelDUnionNS3 _reldunionns3("RelOp::Sharing::DUnionNS3");

    /// Test for n-ary union constraint
    class RelUnionN : public SetTest {
    private:
      int n;
    public:
      /// Create and register test
      RelUnionN(int n0)
        : SetTest("RelOp::UnionN::"+str(n0),n0+1,ds_22,false), n(n0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        GECODE_AUTOARRAY(CountableSetRanges, isrs, n);
        for (int i=n; i--;)
          isrs[i].init(x.lub, x[i]);
        Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, n);
        CountableSetRanges xnr(x.lub, x[n]);
        return Iter::Ranges::equal(u, xnr);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-1);
        for (int i=x.size()-1; i--;)
          xs[i]=x[i];
        Gecode::rel(home, SOT_UNION, xs, x[x.size()-1]);
      }
    };
    RelUnionN _relunionn0(0);
    RelUnionN _relunionn1(1);
    RelUnionN _relunionn2(2);
    RelUnionN _relunionn3(3);

    /// Sharing test for n-ary union constraint
    class RelUnionNS1 : public SetTest {
    public:
      /// Create and register test
      RelUnionNS1(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Union<CountableSetRanges,CountableSetRanges> u(xr0,xr1);
        CountableSetRanges xr2(x.lub, x[2]);
        return Iter::Ranges::equal(u,xr2);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[0];
        xs[2]=x[1];
        Gecode::rel(home, SOT_UNION, xs, x[2]);
      }
    };
    RelUnionNS1 _relunionns1("RelOp::Sharing::UnionNS1");

    /// Sharing test for n-ary union constraint
    class RelUnionNS2 : public SetTest {
    public:
      /// Create and register test
      RelUnionNS2(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr21(x.lub, x[2]);
        CountableSetRanges xr22(x.lub, x[2]);
        return Iter::Ranges::subset(xr0,xr21) && 
               Iter::Ranges::subset(xr1,xr22);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[2];
        Gecode::rel(home, SOT_UNION, xs, x[2]);
      }
    };
    RelUnionNS2 _relunionns2("RelOp::Sharing::UnionNS2");

    /// Sharing test for n-ary union constraint
    class RelUnionNS3 : public SetTest {
    public:
      /// Create and register test
      RelUnionNS3(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr1,xr0);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[0];
        Gecode::rel(home, SOT_UNION, xs, x[0]);
      }
    };
    RelUnionNS3 _relunionns3("RelOp::Sharing::UnionNS3");

    /// Test for n-ary intersection constraint
    class RelInterN : public SetTest {
    private:
      int n;
    public:
      /// Create and register test
      RelInterN(int n0)
        : SetTest("RelOp::InterN"+str(n0),n0+1,ds_22,false), n(n0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        GECODE_AUTOARRAY(CountableSetRanges, isrs, n);
        for (int i=n; i--; )
          isrs[i].init(x.lub, x[i]);
        Iter::Ranges::NaryInter<CountableSetRanges> u(isrs, n);
        CountableSetRanges xnr(x.lub, x[n]);
        return Iter::Ranges::equal(u, xnr);

      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-1);
        for (int i=x.size()-1; i--;)
          xs[i]=x[i];
        Gecode::rel(home, SOT_INTER, xs, x[x.size()-1]);
      }
    };
    RelInterN _relintern0(0);
    RelInterN _relintern1(1);
    RelInterN _relintern2(2);
    RelInterN _relintern3(3);

    /// Sharing test for n-ary intersection constraint
    class RelInterNS1 : public SetTest {
    public:
      /// Create and register test
      RelInterNS1(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> u(xr0,xr1);
        CountableSetRanges xr2(x.lub, x[2]);
        return Iter::Ranges::equal(u,xr2);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[0];
        xs[2]=x[1];
        Gecode::rel(home, SOT_INTER, xs, x[2]);
      }
    };
    RelInterNS1 _relinterns1("RelOp::Sharing::InterNS1");

    /// Sharing test for n-ary intersection constraint
    class RelInterNS2 : public SetTest {
    public:
      /// Create and register test
      RelInterNS2(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr21(x.lub, x[2]);
        CountableSetRanges xr22(x.lub, x[2]);
        return Iter::Ranges::subset(xr21,xr0) && 
               Iter::Ranges::subset(xr22,xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[2];
        Gecode::rel(home, SOT_INTER, xs, x[2]);
      }
    };
    RelInterNS2 _relinterns2("RelOp::Sharing::InterNS2");

    /// Sharing test for n-ary intersection constraint
    class RelInterNS3 : public SetTest {
    public:
      /// Create and register test
      RelInterNS3(const char* t)
        : SetTest(t,2,ds_33,false) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr0,xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(3);
        xs[0]=x[0];
        xs[1]=x[1];
        xs[2]=x[0];
        Gecode::rel(home, SOT_INTER, xs, x[0]);
      }
    };
    RelInterNS3 _relinterns3("RelOp::Sharing::InterNS3");

    //@}

}}}

// STATISTICS: test-set
