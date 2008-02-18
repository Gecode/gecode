/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *
 *  Last modified:
 *     $Date: 2007-11-08 15:53:26 +0100 (Do, 08 Nov 2007) $ by $Author: tack $
 *     $Revision: 5219 $
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

  /// Tests for relation/operation constraints with constants
  namespace RelOpConst {

    /**
      * \defgroup TaskTestSetRelOpConst Relation/operation constraints with constants
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_22(-2,2);
    static IntSet ds_12(-1,2);
    
    /// Test for ternary union constraint with constants
    class RelUnionEq : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelUnionEq(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        if (!inverse) {
          Iter::Ranges::Union<IntSetRanges, CountableSetRanges> 
            u(isr,xr0);
          return Iter::Ranges::equal(xr1, u);
        } else {
          Iter::Ranges::Union<CountableSetRanges, IntSetRanges> 
            u(xr0,isr);
          return Iter::Ranges::equal(xr1, u);

        }
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_UNION, x[0], SRT_EQ, x[1]);
        else
          Gecode::rel(home, x[0], SOT_UNION, is, SRT_EQ, x[1]);
      }
    };
    RelUnionEq _relunioneq1(IntSet(-1,1),false,"RelOpConst::UnionEq::1");
    RelUnionEq _relunioneq2(IntSet(-4,-4),false,"RelOpConst::UnionEq::2");
    RelUnionEq _relunioneq1i(IntSet(-1,1),true,"RelOpConst::UnionEq::1i");
    RelUnionEq _relunioneq2i(IntSet(-4,-4),true,"RelOpConst::UnionEq::2i");

    /// Test for ternary partition constraint with constants
    class RelDUnionEq : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelDUnionEq(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Union<IntSetRanges, CountableSetRanges> 
          u(isr,xr0);
        IntSetRanges isr2(is);
        CountableSetRanges xr02(x.lub, x[0]);

        return
          Iter::Ranges::equal(xr1, u) &&
          Iter::Ranges::disjoint(isr2, xr02);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_DUNION, x[0], SRT_EQ, x[1]);
        else
          Gecode::rel(home, x[0], SOT_DUNION, is, SRT_EQ, x[1]);
      }
    };
    RelDUnionEq _reldunioneq1(IntSet(-1,1),false,"RelOpConst::DUnionEq::1");
    RelDUnionEq _reldunioneq2(IntSet(-4,-4),false,"RelOpConst::DUnionEq::2");
    RelDUnionEq _reldunioneq1i(IntSet(-1,1),true,"RelOpConst::DUnionEq::1i");
    RelDUnionEq _reldunioneq2i(IntSet(-4,-4),true,"RelOpConst::DUnionEq::2i");

    /// Test for ternary intersection constraint
    class RelInterEq : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelInterEq(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Inter<IntSetRanges, CountableSetRanges> 
          u(isr,xr0);
        return Iter::Ranges::equal(xr1, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_INTER, x[0], SRT_EQ, x[1]);
        else
          Gecode::rel(home, x[0], SOT_INTER, is, SRT_EQ, x[1]);        
      }
    };
    RelInterEq _relintereq1(IntSet(-1,1),false,"RelOpConst::InterEq::1");
    RelInterEq _relintereq2(IntSet(-4,-4),false,"RelOpConst::InterEq::2");
    RelInterEq _relintereq1i(IntSet(-1,1),true,"RelOpConst::InterEq::1i");
    RelInterEq _relintereq2i(IntSet(-4,-4),true,"RelOpConst::InterEq::2i");

    /// Test for intersection/subset constraint
    class RelInterSub : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelInterSub(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Inter<IntSetRanges, CountableSetRanges> 
          u(isr,xr0);
        return Iter::Ranges::subset(u, xr1);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_INTER, x[0], SRT_SUB, x[1]);
        else
          Gecode::rel(home, x[0], SOT_INTER, is, SRT_SUB, x[1]);
      }
    };
    RelInterSub _relintersub1(IntSet(-1,1),false,"RelOpConst::InterSub::1");
    RelInterSub _relintersub2(IntSet(-4,-4),false,"RelOpConst::InterSub::2");
    RelInterSub _relintersub1i(IntSet(-1,1),true,"RelOpConst::InterSub::1i");
    RelInterSub _relintersub2i(IntSet(-4,-4),true,"RelOpConst::InterSub::2i");

    /// Test for set-minus constraint
    class RelMinusEq : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelMinusEq(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        if (!inverse) {
          Iter::Ranges::Diff<IntSetRanges, CountableSetRanges> u(isr,xr0);
          return Iter::Ranges::equal(xr1, u);
        } else {
          Iter::Ranges::Diff<CountableSetRanges, IntSetRanges> u(xr0,isr);
          return Iter::Ranges::equal(xr1, u);          
        }
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_MINUS, x[0], SRT_EQ, x[1]);
        else
          Gecode::rel(home, x[0], SOT_MINUS, is, SRT_EQ, x[1]);
      }
    };
    RelMinusEq _relminuseq1(IntSet(-1,1),false,"RelOpConst::MinusEq::1");
    RelMinusEq _relminuseq2(IntSet(-4,-4),false,"RelOpConst::MinusEq::2");
    RelMinusEq _relminuseq1i(IntSet(-1,1),true,"RelOpConst::MinusEq::1i");
    RelMinusEq _relminuseq2i(IntSet(-4,-4),true,"RelOpConst::MinusEq::2i");

    /// Test for union/superset constraint
    class RelUnionSup : public SetTest {
    private:
      IntSet is;
      bool inverse;
    public:
      /// Create and register test
      RelUnionSup(const IntSet& is0, bool inverse0, const char* t)
        : SetTest(t,2,ds_22,false), is(is0), inverse(inverse0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Iter::Ranges::Union<IntSetRanges, CountableSetRanges> 
          u(isr,xr0);
        return Iter::Ranges::subset(xr1, u);
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, SOT_UNION, x[0], SRT_SUP, x[1]);
        else
          Gecode::rel(home, x[0], SOT_UNION, is, SRT_SUP, x[1]);
      }
    };
    RelUnionSup _relunionsup1(IntSet(-1,1),false,"RelOpConst::UnionSup::1");
    RelUnionSup _relunionsup2(IntSet(-4,-4),false,"RelOpConst::UnionSup::2");
    RelUnionSup _relunionsup1i(IntSet(-1,1),true,"RelOpConst::UnionSup::1i");
    RelUnionSup _relunionsup2i(IntSet(-4,-4),true,"RelOpConst::UnionSup::2i");

    //@}

}}}

// STATISTICS: test-set
