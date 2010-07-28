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

  /// %Tests for relation/operation constraints with constants
  namespace RelOpConst {

    /**
      * \defgroup TaskTestSetRelOpConst Relation/operation constraints with constants
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_33(-3,3);
    static IntSet ds_22(-2,2);
    static IntSet ds_12(-1,2);

    static IntSet iss[] = {IntSet(-1,1), IntSet(-4,-4), IntSet(0,2)};

    /// %Test for set relation constraint with constants
    class RelSIS : public SetTest {
    private:
      IntSet is;
      Gecode::SetOpType sot;
      Gecode::SetRelType srt;
      bool inverse;

      template<class I, class J>
      bool
      sol(I& i, J& j) const {
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(i,j);
        case SRT_NQ: return !Iter::Ranges::equal(i,j);
        case SRT_SUB: return Iter::Ranges::subset(i,j);
        case SRT_SUP: return Iter::Ranges::subset(j,i);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<I,J> inter(i,j);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<J> jc(j);
            return Iter::Ranges::equal(i,jc);
          }
        }
        GECODE_NEVER;
        return false;
      }

    public:
      /// Create and register test
      RelSIS(Gecode::SetOpType sot0, Gecode::SetRelType srt0,
             int intSet, bool inverse0)
       : SetTest("RelOp::ConstSIS::"+str(sot0)+"::"+str(srt0)+"::"+
                 str(intSet)+(inverse0 ? "i" :""),2,ds_22,false)
       , is(iss[intSet]), sot(sot0), srt(srt0), inverse(inverse0) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        IntSetRanges isr(is);
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        switch (sot) {
        case SOT_UNION:
          {
            Iter::Ranges::Union<IntSetRanges, CountableSetRanges>
              u(isr, xr0);
            return sol(u,xr1);
          }
          break;
        case SOT_DUNION:
          {
            Iter::Ranges::Inter<IntSetRanges, CountableSetRanges>
              inter(isr, xr0);
            if (inter())
              return false;
            Iter::Ranges::Union<IntSetRanges, CountableSetRanges>
              u(isr,xr0);
            return sol(u,xr1);
          }
          break;
        case SOT_INTER:
          {
            Iter::Ranges::Inter<IntSetRanges, CountableSetRanges>
              u(isr,xr0);
            return sol(u,xr1);
          }
          break;
        case SOT_MINUS:
          {
            if (!inverse) {
              Iter::Ranges::Diff<IntSetRanges, CountableSetRanges>
                u(isr,xr0);
              return sol(u,xr1);
            } else {
              Iter::Ranges::Diff<CountableSetRanges, IntSetRanges>
                u(xr0,isr);
              return sol(u,xr1);

            }
          }
          break;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is, sot, x[0], srt, x[1]);
        else
          Gecode::rel(home, x[0], sot, is, srt, x[1]);
      }
    };

    /// %Test for set relation constraint with constants
    class RelSSI : public SetTest {
    private:
      IntSet is;
      Gecode::SetOpType sot;
      Gecode::SetRelType srt;

      template<class I, class J>
      bool
      sol(I& i, J& j) const {
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(i,j);
        case SRT_NQ: return !Iter::Ranges::equal(i,j);
        case SRT_SUB: return Iter::Ranges::subset(i,j);
        case SRT_SUP: return Iter::Ranges::subset(j,i);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<I,J> inter(i,j);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<J> jc(j);
            return Iter::Ranges::equal(i,jc);
          }
        }
        GECODE_NEVER;
        return false;
      }

    public:
      /// Create and register test
      RelSSI(Gecode::SetOpType sot0, Gecode::SetRelType srt0,
             int intSet)
       : SetTest("RelOp::ConstSSI::"+str(sot0)+"::"+str(srt0)+"::"+
                 str(intSet),2,ds_22,false)
       , is(iss[intSet]), sot(sot0), srt(srt0) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        IntSetRanges isr(is);
        switch (sot) {
        case SOT_UNION:
          {
            Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
              u(xr0, xr1);
            return sol(u,isr);
          }
          break;
        case SOT_DUNION:
          {
            Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
              inter(xr0, xr1);
            if (inter())
              return false;
            Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
              u(xr0, xr1);
            return sol(u,isr);
          }
          break;
        case SOT_INTER:
          {
            Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
              u(xr0,xr1);
            return sol(u,isr);
          }
          break;
        case SOT_MINUS:
          {
            Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges>
              u(xr0,xr1);
            return sol(u,isr);
          }
          break;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::rel(home, x[0], sot, x[1], srt, is);
      }
    };

    /// %Test for set relation constraint with constants
    class RelISI : public SetTest {
    private:
      IntSet is0;
      IntSet is1;
      Gecode::SetOpType sot;
      Gecode::SetRelType srt;
      bool inverse;

      template<class I, class J>
      bool
      sol(I& i, J& j) const {
        switch (srt) {
        case SRT_EQ: return Iter::Ranges::equal(i,j);
        case SRT_NQ: return !Iter::Ranges::equal(i,j);
        case SRT_SUB: return Iter::Ranges::subset(i,j);
        case SRT_SUP: return Iter::Ranges::subset(j,i);
        case SRT_DISJ:
          {
            Gecode::Iter::Ranges::Inter<I,J> inter(i,j);
            return !inter();
          }
        case SRT_CMPL:
          {
            Gecode::Set::RangesCompl<J> jc(j);
            return Iter::Ranges::equal(i,jc);
          }
        }
        GECODE_NEVER;
        return false;
      }

    public:
      /// Create and register test
      RelISI(Gecode::SetOpType sot0, Gecode::SetRelType srt0,
             int intSet0, int intSet1, bool inverse0)
       : SetTest("RelOp::ConstISI::"+str(sot0)+"::"+str(srt0)+"::"+
                 str(intSet0)+"::"+str(intSet1)+
                 (inverse0 ? "i" : ""),1,ds_33,false)
       , is0(iss[intSet0]), is1(iss[intSet1]), sot(sot0), srt(srt0)
       , inverse(inverse0) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        IntSetRanges isr0(is0);
        IntSetRanges isr1(is1);
        switch (sot) {
        case SOT_UNION:
          {
            Iter::Ranges::Union<IntSetRanges, CountableSetRanges>
              u(isr0, xr0);
            return sol(u,isr1);
          }
          break;
        case SOT_DUNION:
          {
            Iter::Ranges::Inter<IntSetRanges, CountableSetRanges>
              inter(isr0, xr0);
            if (inter())
              return false;
            Iter::Ranges::Union<IntSetRanges, CountableSetRanges>
              u(isr0, xr0);
            return sol(u,isr1);
          }
          break;
        case SOT_INTER:
          {
            Iter::Ranges::Inter<IntSetRanges, CountableSetRanges>
              u(isr0,xr0);
            return sol(u,isr1);
          }
          break;
        case SOT_MINUS:
          {
            if (!inverse) {
              Iter::Ranges::Diff<IntSetRanges, CountableSetRanges>
                u(isr0,xr0);
              return sol(u,isr1);
            } else {
              Iter::Ranges::Diff<CountableSetRanges, IntSetRanges>
                u(xr0,isr0);
              return sol(u,isr1);
            }
          }
          break;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space& home, SetVarArray& x, IntVarArray&) {
        if (!inverse)
          Gecode::rel(home, is0, sot, x[0], srt, is1);
        else
          Gecode::rel(home, x[0], sot, is0, srt, is1);
      }
    };

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;
        for (SetRelTypes srts; srts(); ++srts) {
          for (SetOpTypes sots; sots(); ++sots) {
            for (int i=0; i<=2; i++) {
              (void) new RelSIS(sots.sot(),srts.srt(),i,false);
              (void) new RelSIS(sots.sot(),srts.srt(),i,true);
              (void) new RelSSI(sots.sot(),srts.srt(),i);
              (void) new RelISI(sots.sot(),srts.srt(),i,0,false);
              (void) new RelISI(sots.sot(),srts.srt(),i,1,false);
              (void) new RelISI(sots.sot(),srts.srt(),i,2,false);
              (void) new RelISI(sots.sot(),srts.srt(),i,0,true);
              (void) new RelISI(sots.sot(),srts.srt(),i,1,true);
              (void) new RelISI(sots.sot(),srts.srt(),i,2,true);
            }
          }
        }
      }
    };

    Create c;

    //@}

}}}

// STATISTICS: test-set
