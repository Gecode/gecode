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

  /// %Tests for domain constraints
  namespace Dom {

    /**
      * \defgroup TaskTestSetDom Domain constraints
      * \ingroup TaskTestSet
      */
    //@{

    static const int d1r[4][2] = {
      {-4,-3},{-1,-1},{1,1},{3,5}
    };
    static IntSet d1(d1r,4);

    static const int d1cr[5][2] = {
      {Gecode::Set::Limits::min,-5},
      {-2,-2},{0,0},{2,2},
      {6,Gecode::Set::Limits::max}
    };
    static IntSet d1c(d1cr,5);

    static IntSet ds_33(-3,3);

    static const int d2r[2][2] = {
      {Gecode::Set::Limits::min,-4}, {4,Gecode::Set::Limits::max}
    };
    static IntSet ds_33c(d2r,2);
    static IntSet ds_55(-5,5);

    namespace {
      static int minSymDiff(const SetAssignment& x, const IntSet& is) {
        typedef Iter::Ranges::Diff<CountableSetRanges,IntSetRanges> DiffA;
        CountableSetRanges xr00(x.lub, x[0]);
        IntSetRanges xr10(is);
        DiffA a(xr00,xr10);
        typedef Iter::Ranges::Diff<IntSetRanges,CountableSetRanges> DiffB;
        CountableSetRanges xr01(x.lub, x[0]);
        IntSetRanges xr11(is);
        DiffB b(xr11,xr01);
        Iter::Ranges::Union<DiffA,DiffB> u(a,b);
        return u() ? u.min() : Gecode::Set::Limits::max+1;
      }
      template<class I>
      static bool in(int i, I& c, bool eq=false) {
        if (eq && i==Gecode::Set::Limits::max+1)
          return true;
        Iter::Ranges::Singleton s(i,i);
        return Iter::Ranges::subset(s,c);
      }
    }

    /// %Test for equality with a range
    class DomRange : public SetTest {
    private:
      Gecode::SetRelType srt;
      IntSet is;
    public:
      /// Create and register test
      DomRange(SetRelType srt0)
        : SetTest("Dom::Range::"+str(srt0),1,ds_55,true), srt(srt0)
        , is(srt == Gecode::SRT_CMPL ? ds_33c: ds_33) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        IntSetRanges dr(is);
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(xr, dr);
        case SRT_LQ: return (!xr()) || in(minSymDiff(x,is),dr,true);
        case SRT_LE: return xr() ? in(minSymDiff(x,is),dr) : dr();
        case SRT_GQ: return (!dr()) || in(minSymDiff(x,is),xr,true);
        case SRT_GR: return dr() ? in(minSymDiff(x,is),xr) : xr();
        case SRT_NQ: return !Iter::Ranges::equal(xr, dr);
        case SRT_SUB: return Iter::Ranges::subset(xr, dr);
        case SRT_SUP: return Iter::Ranges::subset(dr, xr);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
              inter(xr, dr);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
            return Iter::Ranges::equal(xr,drc);
          }
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::dom(home, x[0], srt, is);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,BoolVar b) {
        Gecode::dom(home, x[0], srt, is, b);
      }
    };
    DomRange _domrange_eq(SRT_EQ);
    DomRange _domrange_lq(SRT_LQ);
    DomRange _domrange_le(SRT_LE);
    DomRange _domrange_gq(SRT_GQ);
    DomRange _domrange_gr(SRT_GR);
    DomRange _domrange_nq(SRT_NQ);
    DomRange _domrange_sub(SRT_SUB);
    DomRange _domrange_sup(SRT_SUP);
    DomRange _domrange_disj(SRT_DISJ);
    DomRange _domrange_cmpl(SRT_CMPL);

    /// %Test for equality with an integer range
    class DomIntRange : public SetTest {
    private:
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      DomIntRange(Gecode::SetRelType srt0)
        : SetTest("Dom::IntRange::"+str(srt0),1,ds_55,true), srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        IntSet is(-3,-1);
        IntSetRanges dr(is);
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(xr, dr);
        case SRT_LQ: return (!xr()) || in(minSymDiff(x,is),dr,true);
        case SRT_LE: return xr() ? in(minSymDiff(x,is),dr) : dr();
        case SRT_GQ: return (!dr()) || in(minSymDiff(x,is),xr,true);
        case SRT_GR: return dr() ? in(minSymDiff(x,is),xr) : xr();
        case SRT_NQ: return !Iter::Ranges::equal(xr, dr);
        case SRT_SUB: return Iter::Ranges::subset(xr, dr);
        case SRT_SUP: return Iter::Ranges::subset(dr, xr);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
              inter(xr, dr);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
            return Iter::Ranges::equal(xr,drc);
          }
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::dom(home, x[0], srt, -3, -1);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,BoolVar b) {
        Gecode::dom(home, x[0], srt, -3, -1, b);
      }
    };
    DomIntRange _domintrange_eq(SRT_EQ);
    DomIntRange _domintrange_lq(SRT_LQ);
    DomIntRange _domintrange_le(SRT_LE);
    DomIntRange _domintrange_gq(SRT_GQ);
    DomIntRange _domintrange_gr(SRT_GR);
    DomIntRange _domintrange_nq(SRT_NQ);
    DomIntRange _domintrange_sub(SRT_SUB);
    DomIntRange _domintrange_sup(SRT_SUP);
    DomIntRange _domintrange_disj(SRT_DISJ);
    DomIntRange _domintrange_cmpl(SRT_CMPL);

    /// %Test for equality with an integer
    class DomInt : public SetTest {
    private:
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      DomInt(Gecode::SetRelType srt0)
        : SetTest("Dom::Int::"+str(srt0),1,ds_55,true), srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        IntSet is(-3,-3);
        IntSetRanges dr(is);
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(xr, dr);
        case SRT_LQ: return (!xr()) || in(minSymDiff(x,is),dr,true);
        case SRT_LE: return xr() ? in(minSymDiff(x,is),dr) : dr();
        case SRT_GQ: return (!dr()) || in(minSymDiff(x,is),xr,true);
        case SRT_GR: return dr() ? in(minSymDiff(x,is),xr) : xr();
        case SRT_NQ: return !Iter::Ranges::equal(xr, dr);
        case SRT_SUB: return Iter::Ranges::subset(xr, dr);
        case SRT_SUP: return Iter::Ranges::subset(dr, xr);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
              inter(xr, dr);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
            return Iter::Ranges::equal(xr,drc);
          }
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::dom(home, x[0], srt, -3);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,BoolVar b) {
        Gecode::dom(home, x[0], srt, -3, b);
      }
    };
    DomInt _domint_eq(SRT_EQ);
    DomInt _domint_lq(SRT_LQ);
    DomInt _domint_le(SRT_LE);
    DomInt _domint_gq(SRT_GQ);
    DomInt _domint_gr(SRT_GR);
    DomInt _domint_nq(SRT_NQ);
    DomInt _domint_sub(SRT_SUB);
    DomInt _domint_sup(SRT_SUP);
    DomInt _domint_disj(SRT_DISJ);
    DomInt _domint_cmpl(SRT_CMPL);

    /// %Test for equality with a domain
    class DomDom : public SetTest {
    private:
      Gecode::SetRelType srt;
      Gecode::IntSet is;
    public:
      /// Create and register test
      DomDom(Gecode::SetRelType srt0)
        : SetTest("Dom::Dom::"+str(srt0),1,d1,true), srt(srt0)
        , is(srt == Gecode::SRT_CMPL ? d1c: d1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        IntSetRanges dr(is);
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(xr, dr);
        case SRT_LQ: return (!xr()) || in(minSymDiff(x,is),dr,true);
        case SRT_LE: return xr() ? in(minSymDiff(x,is),dr) : dr();
        case SRT_GQ: return (!dr()) || in(minSymDiff(x,is),xr,true);
        case SRT_GR: return dr() ? in(minSymDiff(x,is),xr) : xr();
        case SRT_NQ: return !Iter::Ranges::equal(xr, dr);
        case SRT_SUB: return Iter::Ranges::subset(xr, dr);
        case SRT_SUP: return Iter::Ranges::subset(dr, xr);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
              inter(xr, dr);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
            return Iter::Ranges::equal(xr,drc);
          }
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::dom(home, x[0], srt, is);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,BoolVar b) {
        Gecode::dom(home, x[0], srt, is, b);
      }
    };
    DomDom _domdom_eq(SRT_EQ);
    DomDom _domdom_lq(SRT_LQ);
    DomDom _domdom_le(SRT_LE);
    DomDom _domdom_gq(SRT_GQ);
    DomDom _domdom_gr(SRT_GR);
    DomDom _domdom_nq(SRT_NQ);
    DomDom _domdom_sub(SRT_SUB);
    DomDom _domdom_sup(SRT_SUP);
    DomDom _domdom_disj(SRT_DISJ);
    DomDom _domdom_cmpl(SRT_CMPL);

    /// %Test for cardinality range
    class CardRange : public SetTest {
    public:
      /// Create and register test
      CardRange(void)
        : SetTest("Dom::CardRange",1,d1,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        unsigned int card = Iter::Ranges::size(xr);
        return card >= 2 && card <= 3;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::cardinality(home, x[0], 2, 3);
      }
    };
    CardRange _cardRange;

}}}

// STATISTICS: test-set
