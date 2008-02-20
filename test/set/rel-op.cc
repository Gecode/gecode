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

    static IntSet ds_22(-2,2);
    static IntSet ds_12(-1,2);
    
    /// Test for ternary relation constraint
    class Rel : public SetTest {
    private:
      Gecode::SetOpType  sot;
      Gecode::SetRelType srt;
      int share;

      template <class I, class J>
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
      Rel(Gecode::SetOpType sot0, Gecode::SetRelType srt0, int share0=0)
        : SetTest("RelOp::"+str(sot0)+"::"+str(srt0)+"::S"+str(share0),
                  share0 == 0 ? 3 : 2,ds_22,false)
        , sot(sot0), srt(srt0), share(share0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        int a,b,c;
        switch (share) {
          case 0: a=x[0]; b=x[1]; c=x[2]; break;
          case 1: a=x[0]; b=x[0]; c=x[0]; break;
          case 2: a=x[0]; b=x[0]; c=x[1]; break;
          case 3: a=x[0]; b=x[1]; c=x[0]; break;
          case 4: a=x[0]; b=x[1]; c=x[1]; break;
        }
        CountableSetRanges xr0(x.lub, a);
        CountableSetRanges xr1(x.lub, b);
        CountableSetRanges xr2(x.lub, c);
        switch (sot) {
        case SOT_UNION:
          {
            Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> 
              u(xr0,xr1);
            return sol(u,xr2);
          }
          break;
        case SOT_DUNION:
          {
            Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> 
              inter(xr0,xr1);
            if (inter())
              return false;
            Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> 
              u(xr0,xr1);
            return sol(u,xr2);
          }
          break;
        case SOT_INTER:
          {
            Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> 
              u(xr0,xr1);
            return sol(u,xr2);
          }
          break;
        case SOT_MINUS:
          {
            Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges> 
              u(xr0,xr1);
            return sol(u,xr2);
          }
          break;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        SetVar a,b,c;
        switch (share) {
          case 0: a=x[0]; b=x[1]; c=x[2]; break;
          case 1: a=x[0]; b=x[0]; c=x[0]; break;
          case 2: a=x[0]; b=x[0]; c=x[1]; break;
          case 3: a=x[0]; b=x[1]; c=x[0]; break;
          case 4: a=x[0]; b=x[1]; c=x[1]; break;
        }
        Gecode::rel(home, a, sot, b, srt, c);
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
            for (int i=0; i<=4; i++) {
              (void) new Rel(sots.sot(),srts.srt(),i);
            }
          }
        }      
      }
    };
   
    Create c;

    /// Test for n-ary partition constraint
    class RelN : public SetTest {
    private:
      Gecode::SetOpType sot;
      int n;
      int shared;
    public:
      /// Create and register test
      RelN(Gecode::SetOpType sot0, int n0, int shared0 = 0)
        : SetTest("RelOp::N::"+str(sot0)+"::"+str(n0)+"::S"+str(shared0),
                  shared0 == 0 ? n0+1 : (shared0 <= 2 ? 3 : 2),ds_12,false)
        , sot(sot0), n(n0), shared(shared0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        int realN = shared == 0 ? n : (shared <= 2 ? 2 : 1);

        GECODE_AUTOARRAY(CountableSetRanges, isrs, realN);
        
        switch (shared) {
        case 0:
          for (int i=realN; i--; )        
            isrs[i].init(x.lub, x[i]);
          break;
        case 1:
          isrs[0].init(x.lub, x[0]);
          isrs[1].init(x.lub, x[0]);
          isrs[2].init(x.lub, x[1]);
          break;
        case 2:
          isrs[0].init(x.lub, x[0]);
          isrs[1].init(x.lub, x[1]);
          isrs[2].init(x.lub, x[2]);
          break;
        case 3:
          isrs[0].init(x.lub, x[0]);
          isrs[1].init(x.lub, x[1]);
          isrs[2].init(x.lub, x[0]);
          break;
        default:
          GECODE_NEVER;
        }
        
        CountableSetRanges xnr(x.lub, x[realN]);

        switch (sot) {
        case SOT_DUNION:
          {
            if (shared == 1 && (isrs[0]() || isrs[1]()))
              return false;
            if (shared == 3 && (isrs[0]() || isrs[2]()))
              return false;
            unsigned int cardSum = 0;
            if (shared == 1) {
              CountableSetRanges x2r(x.lub, x[2]);
              cardSum = Iter::Ranges::size(x2r);
            } else if (shared == 3) {
              CountableSetRanges x1r(x.lub, x[1]);
              cardSum = Iter::Ranges::size(x1r);
            } else {
              for (int i=0; i<realN; i++) {
                CountableSetRanges xir(x.lub, x[i]);
                cardSum += Iter::Ranges::size(xir);
              }
            }
            CountableSetRanges xnr2(x.lub, x[realN]);
            if (cardSum != Iter::Ranges::size(xnr2))
              return false;
          }
          // fall through to union case
        case SOT_UNION:
          {
            Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, realN);
            return Iter::Ranges::equal(u, xnr);
          }
        case SOT_INTER:
          {
            Iter::Ranges::NaryInter<CountableSetRanges> u(isrs, realN);
            return Iter::Ranges::equal(u, xnr);
          }
        default:
          GECODE_NEVER;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space* home, SetVarArray& x, IntVarArray&) {
        int size = shared == 0 ? x.size()-1 : 3;
        SetVarArgs xs(size);
        SetVar xn;
        switch (shared) {
        case 0:
          for (int i=x.size()-1; i--;)
            xs[i]=x[i];
          xn = x[x.size()-1];
          break;
        case 1:
          xs[0] = x[0];
          xs[1] = x[0];
          xs[2] = x[1];
          xn = x[2];
          break;
        case 2:
          xs[0] = x[0]; xs[1] = x[1]; xs[2] = x[2]; xn = x[2];
          break;
        case 3:
          xs[0] = x[0]; xs[1] = x[1]; xs[2] = x[0]; xn = x[0];
          break;
        default:
          GECODE_NEVER;
          break;
        }
        Gecode::rel(home, sot, xs, xn);
      }
    };

    /// Help class to create and register tests
    class CreateN {
    public:
      /// Perform creation and registration
      CreateN(void) {
        for (int i=0; i<=3; i++) {
          (void) new RelN(Gecode::SOT_UNION, i, 0);
          (void) new RelN(Gecode::SOT_DUNION, i, 0);
          (void) new RelN(Gecode::SOT_INTER, i, 0);

          if (i>0) {
            (void) new RelN(Gecode::SOT_UNION, 0, i);
            (void) new RelN(Gecode::SOT_DUNION, 0, i);
            (void) new RelN(Gecode::SOT_INTER, 0, i);
          }
        }
      }
    };

    CreateN cn;

    //@}

}}}

// STATISTICS: test-set
