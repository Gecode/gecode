/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include <gecode/minimodel.hh>

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

    namespace {
      static int minSymDiff(const SetAssignment& x, int i, const IntSet& is) {
        typedef Iter::Ranges::Diff<CountableSetRanges,IntSetRanges> DiffA;
        CountableSetRanges xr00(x.lub, x[i]);
        IntSetRanges xr10(is);
        DiffA a(xr00,xr10);
        typedef Iter::Ranges::Diff<IntSetRanges,CountableSetRanges> DiffB;
        CountableSetRanges xr01(x.lub, x[i]);
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
      DomRange(SetRelType srt0, int n) :
        SetTest("Dom::Range::"+str(srt0)+"::"+str(n),n,ds_33,(n == 1)),
        srt(srt0), is(srt == Gecode::SRT_CMPL ? ds_33c: ds_33) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=x.size(); i--; ) {
          CountableSetRanges xr(x.lub, x[i]);
          IntSetRanges dr(is);
          switch (srt) {
          case SRT_EQ:
            if (!Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_LQ:
            if (!((!xr()) || in(minSymDiff(x,i,is),dr,true)))
              return false;
            break;
          case SRT_LE:
            if (!(xr() ? in(minSymDiff(x,i,is),dr) : dr()))
              return false;
            break;
          case SRT_GQ:
            if (!((!dr()) || in(minSymDiff(x,i,is),xr,true)))
              return false;
            break;
          case SRT_GR:
            if (!(dr() ? in(minSymDiff(x,i,is),xr) : xr()))
              return false;
            break;
          case SRT_NQ:
            if (Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_SUB:
            if (!Iter::Ranges::subset(xr, dr))
              return false;
            break;
          case SRT_SUP:
            if (!Iter::Ranges::subset(dr, xr))
              return false;
            break;
          case SRT_DISJ:
            {
              Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
                inter(xr, dr);
              if (inter())
                return false;
            }
            break;
          case SRT_CMPL:
            {
              Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
              if (!Iter::Ranges::equal(xr,drc))
                return false;
            }
            break;
          default: GECODE_NEVER;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (x.size() == 1)
          Gecode::dom(home, x[0], srt, is);
        else
          Gecode::dom(home, x, srt, is);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&, Reify r) {
        assert(x.size() == 1);
        if (Base::rand(2) != 0) {
          Gecode::dom(home, x[0], srt, is, r);
        } else {
           switch (r.mode()) {
           case Gecode::RM_EQV:
             Gecode::rel(home, Gecode::dom(x[0], srt, is) == r.var()); break;
           case Gecode::RM_IMP:
             Gecode::rel(home, Gecode::dom(x[0], srt, is) << r.var()); break;
           case Gecode::RM_PMI:
             Gecode::rel(home, Gecode::dom(x[0], srt, is) >> r.var()); break;
           default: GECODE_NEVER;
           }
        }
      }
    };

    /// %Test for equality with an integer range
    class DomIntRange : public SetTest {
    private:
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      DomIntRange(Gecode::SetRelType srt0, int n)
        : SetTest("Dom::IntRange::"+str(srt0)+"::"+str(n),1,ds_33,n==1),
          srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=x.size(); i--; ) {
          CountableSetRanges xr(x.lub, x[i]);
          IntSet is(-3,-1);
          IntSetRanges dr(is);
          switch (srt) {
          case SRT_EQ:
            if (!Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_LQ:
            if (!((!xr()) || in(minSymDiff(x,i,is),dr,true)))
              return false;
            break;
          case SRT_LE:
            if (!(xr() ? in(minSymDiff(x,i,is),dr) : dr()))
              return false;
            break;
          case SRT_GQ:
            if (!((!dr()) || in(minSymDiff(x,i,is),xr,true)))
              return false;
            break;
          case SRT_GR:
            if (!(dr() ? in(minSymDiff(x,i,is),xr) : xr()))
              return false;
            break;
          case SRT_NQ:
            if (!(!Iter::Ranges::equal(xr, dr)))
              return false;
            break;
          case SRT_SUB:
            if (!(Iter::Ranges::subset(xr, dr)))
              return false;
            break;
          case SRT_SUP:
            if (!(Iter::Ranges::subset(dr, xr)))
              return false;
            break;
          case SRT_DISJ:
            {
              Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
                inter(xr, dr);
              if (inter())
                return false;
            }
            break;
          case SRT_CMPL:
            {
              Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
              if (!Iter::Ranges::equal(xr,drc))
                return false;
            }
            break;
          default: GECODE_NEVER;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (x.size() == 1)
          Gecode::dom(home, x[0], srt, -3, -1);
        else
          Gecode::dom(home, x, srt, -3, -1);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&, Reify r) {
        assert(x.size() == 1);
        if (Base::rand(2) != 0) {
          Gecode::dom(home, x[0], srt, -3, -1, r);
        } else {
           switch (r.mode()) {
           case Gecode::RM_EQV:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3, -1) == r.var()); break;
           case Gecode::RM_IMP:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3, -1) << r.var()); break;
           case Gecode::RM_PMI:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3, -1) >> r.var()); break;
           default: GECODE_NEVER;
           }
        }
      }
    };

    /// %Test for equality with an integer
    class DomInt : public SetTest {
    private:
      Gecode::SetRelType srt;
    public:
      /// Create and register test
      DomInt(Gecode::SetRelType srt0, int n) :
        SetTest("Dom::Int::"+str(srt0)+"::"+str(n),n,ds_33,n==1),
        srt(srt0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        IntSet is(-3,-3);
        for (int i=x.size(); i--; ) {
          CountableSetRanges xr(x.lub, x[i]);
          IntSetRanges dr(is);
          switch (srt) {
          case SRT_EQ:
            if (!Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_LQ:
            if (!((!xr()) || in(minSymDiff(x,i,is),dr,true)))
              return false;
            break;
          case SRT_LE:
            if (!(xr() ? in(minSymDiff(x,i,is),dr) : dr()))
              return false;
            break;
          case SRT_GQ:
            if (!((!dr()) || in(minSymDiff(x,i,is),xr,true)))
              return false;
            break;
          case SRT_GR:
            if (!(dr() ? in(minSymDiff(x,i,is),xr) : xr()))
              return false;
            break;
          case SRT_NQ:
            if (Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_SUB:
            if (!(Iter::Ranges::subset(xr, dr)))
              return false;
            break;
          case SRT_SUP:
            if (!(Iter::Ranges::subset(dr, xr)))
              return false;
            break;
          case SRT_DISJ:
            {
              Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
                inter(xr, dr);

              if (inter())
                return false;
              break;
            }
          case SRT_CMPL:
            {
              Gecode::Set::RangesCompl<IntSetRanges> drc(dr);

              if (!Iter::Ranges::equal(xr,drc))
                return false;
              break;
            }
          default: GECODE_NEVER;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (x.size() == 1)
          Gecode::dom(home, x[0], srt, -3);
        else
          Gecode::dom(home, x, srt, -3);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&, Reify r) {
        assert(x.size() == 1);
        if (Base::rand(2) != 0) {
          Gecode::dom(home, x[0], srt, -3, r);
        } else {
           switch (r.mode()) {
           case Gecode::RM_EQV:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3) == r.var()); break;
           case Gecode::RM_IMP:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3) << r.var()); break;
           case Gecode::RM_PMI:
             Gecode::rel(home, Gecode::dom(x[0], srt, -3) >> r.var()); break;
           default: GECODE_NEVER;
           }
        }
      }
    };

    /// %Test for equality with a domain
    class DomDom : public SetTest {
    private:
      Gecode::SetRelType srt;
      Gecode::IntSet is;
    public:
      /// Create and register test
      DomDom(Gecode::SetRelType srt0, int n) :
        SetTest("Dom::Dom::"+str(srt0)+"::"+str(n),n,d1,(n == 1)),
        srt(srt0), is(srt == Gecode::SRT_CMPL ? d1c: d1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=x.size(); i--; ) {
          CountableSetRanges xr(x.lub, x[i]);
          IntSetRanges dr(is);
          switch (srt) {
          case SRT_EQ:
            if (!Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_LQ:
            if (!((!xr()) || in(minSymDiff(x,i,is),dr,true)))
              return false;
            break;
          case SRT_LE:
            if (!(xr() ? in(minSymDiff(x,i,is),dr) : dr()))
              return false;
            break;
          case SRT_GQ:
            if (!((!dr()) || in(minSymDiff(x,i,is),xr,true)))
              return false;
            break;
          case SRT_GR:
            if (!(dr() ? in(minSymDiff(x,i,is),xr) : xr()))
              return false;
            break;
          case SRT_NQ:
            if (Iter::Ranges::equal(xr, dr))
              return false;
            break;
          case SRT_SUB:
            if (!Iter::Ranges::subset(xr, dr))
              return false;
            break;
          case SRT_SUP:
            if (!Iter::Ranges::subset(dr, xr))
              return false;
            break;
          case SRT_DISJ:
            {
              Gecode::Iter::Ranges::Inter<CountableSetRanges,IntSetRanges>
                inter(xr, dr);
              if (inter())
                return false;
            }
            break;
          case SRT_CMPL:
            {
              Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
              if (!Iter::Ranges::equal(xr,drc))
                return false;
            }
            break;
          default: GECODE_NEVER;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (x.size() == 1)
          Gecode::dom(home, x[0], srt, is);
        else
          Gecode::dom(home, x, srt, is);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&, Reify r) {
        assert(x.size() == 1);
        Gecode::dom(home, x[0], srt, is, r);
      }
    };

    /// %Test for cardinality range
    class CardRange : public SetTest {
    public:
      /// Create and register test
      CardRange(int n)
        : SetTest("Dom::CardRange::"+str(n),n,d1,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=x.size(); i--; ) {
          CountableSetRanges xr(x.lub, x[i]);
          unsigned int card = Iter::Ranges::size(xr);
          if ((card < 2) || (card > 3))
            return false;
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (x.size() == 1)
          Gecode::cardinality(home, x[0], 2, 3);
        else
          Gecode::cardinality(home, x, 2, 3);
      }
    };

    DomRange _domrange_eq1(SRT_EQ,1);
    DomRange _domrange_lq1(SRT_LQ,1);
    DomRange _domrange_le1(SRT_LE,1);
    DomRange _domrange_gq1(SRT_GQ,1);
    DomRange _domrange_gr1(SRT_GR,1);
    DomRange _domrange_nq1(SRT_NQ,1);
    DomRange _domrange_sub1(SRT_SUB,1);
    DomRange _domrange_sup1(SRT_SUP,1);
    DomRange _domrange_disj1(SRT_DISJ,1);
    DomRange _domrange_cmpl1(SRT_CMPL,1);
    DomRange _domrange_eq2(SRT_EQ,2);
    DomRange _domrange_lq2(SRT_LQ,2);
    DomRange _domrange_le2(SRT_LE,2);
    DomRange _domrange_gq2(SRT_GQ,2);
    DomRange _domrange_gr2(SRT_GR,2);
    DomRange _domrange_nq2(SRT_NQ,2);
    DomRange _domrange_sub2(SRT_SUB,2);
    DomRange _domrange_sup2(SRT_SUP,2);
    DomRange _domrange_disj2(SRT_DISJ,2);
    DomRange _domrange_cmpl2(SRT_CMPL,2);

    DomIntRange _domintrange_eq1(SRT_EQ,1);
    DomIntRange _domintrange_lq1(SRT_LQ,1);
    DomIntRange _domintrange_le1(SRT_LE,1);
    DomIntRange _domintrange_gq1(SRT_GQ,1);
    DomIntRange _domintrange_gr1(SRT_GR,1);
    DomIntRange _domintrange_nq1(SRT_NQ,1);
    DomIntRange _domintrange_sub1(SRT_SUB,1);
    DomIntRange _domintrange_sup1(SRT_SUP,1);
    DomIntRange _domintrange_disj1(SRT_DISJ,1);
    DomIntRange _domintrange_cmpl1(SRT_CMPL,1);
    DomIntRange _domintrange_eq2(SRT_EQ,2);
    DomIntRange _domintrange_lq2(SRT_LQ,2);
    DomIntRange _domintrange_le2(SRT_LE,2);
    DomIntRange _domintrange_gq2(SRT_GQ,2);
    DomIntRange _domintrange_gr2(SRT_GR,2);
    DomIntRange _domintrange_nq2(SRT_NQ,2);
    DomIntRange _domintrange_sub2(SRT_SUB,2);
    DomIntRange _domintrange_sup2(SRT_SUP,2);
    DomIntRange _domintrange_disj2(SRT_DISJ,2);
    DomIntRange _domintrange_cmpl2(SRT_CMPL,2);

    DomInt _domint_eq1(SRT_EQ,1);
    DomInt _domint_lq1(SRT_LQ,1);
    DomInt _domint_le1(SRT_LE,1);
    DomInt _domint_gq1(SRT_GQ,1);
    DomInt _domint_gr1(SRT_GR,1);
    DomInt _domint_nq1(SRT_NQ,1);
    DomInt _domint_sub1(SRT_SUB,1);
    DomInt _domint_sup1(SRT_SUP,1);
    DomInt _domint_disj1(SRT_DISJ,1);
    DomInt _domint_cmpl1(SRT_CMPL,1);
    DomInt _domint_eq2(SRT_EQ,2);
    DomInt _domint_lq2(SRT_LQ,2);
    DomInt _domint_le2(SRT_LE,2);
    DomInt _domint_gq2(SRT_GQ,2);
    DomInt _domint_gr2(SRT_GR,2);
    DomInt _domint_nq2(SRT_NQ,2);
    DomInt _domint_sub2(SRT_SUB,2);
    DomInt _domint_sup2(SRT_SUP,2);
    DomInt _domint_disj2(SRT_DISJ,2);
    DomInt _domint_cmpl2(SRT_CMPL,2);

    DomDom _domdom_eq1(SRT_EQ,1);
    DomDom _domdom_lq1(SRT_LQ,1);
    DomDom _domdom_le1(SRT_LE,1);
    DomDom _domdom_gq1(SRT_GQ,1);
    DomDom _domdom_gr1(SRT_GR,1);
    DomDom _domdom_nq1(SRT_NQ,1);
    DomDom _domdom_sub1(SRT_SUB,1);
    DomDom _domdom_sup1(SRT_SUP,1);
    DomDom _domdom_disj1(SRT_DISJ,1);
    DomDom _domdom_cmpl1(SRT_CMPL,1);
    DomDom _domdom_eq2(SRT_EQ,2);
    DomDom _domdom_lq2(SRT_LQ,2);
    DomDom _domdom_le2(SRT_LE,2);
    DomDom _domdom_gq2(SRT_GQ,2);
    DomDom _domdom_gr2(SRT_GR,2);
    DomDom _domdom_nq2(SRT_NQ,2);
    DomDom _domdom_sub2(SRT_SUB,2);
    DomDom _domdom_sup2(SRT_SUP,2);
    DomDom _domdom_disj2(SRT_DISJ,2);
    DomDom _domdom_cmpl2(SRT_CMPL,2);

    CardRange _cr1(1);
    CardRange _cr2(2);

}}}

// STATISTICS: test-set
