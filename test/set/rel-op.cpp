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

  /// %Tests for relation/operation constraints
  namespace RelOp {

    /**
      * \defgroup TaskTestSetRelOp Relation/operation constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_22(-2,2);
    static IntSet ds_12(-1,2);

    /// %Test for ternary relation constraint
    class Rel : public SetTest {
    private:
      Gecode::SetOpType  sot;
      Gecode::SetRelType srt;
      int share;

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
      Rel(Gecode::SetOpType sot0, Gecode::SetRelType srt0, int share0=0)
        : SetTest("RelOp::"+str(sot0)+"::"+str(srt0)+"::S"+str(share0),
                  share0 == 0 ? 3 : 2,ds_22,false)
        , sot(sot0), srt(srt0), share(share0) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        int a=0, b=0, c=0;
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
      void post(Space& home, SetVarArray& x, IntVarArray&) {
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

    /// %Test for n-ary partition constraint
    class RelN : public SetTest {
    private:
      Gecode::SetOpType sot;
      int n;
      int shared;
      bool withConst;
      IntSet is;
    public:
      /// Create and register test
      RelN(Gecode::SetOpType sot0, int n0, int shared0, bool withConst0)
        : SetTest("RelOp::N::"+str(sot0)+"::"+str(n0)+"::S"+str(shared0)+
                  "::C"+str(withConst0 ? 1 : 0),
                  shared0 == 0 ? n0+1 : (shared0 <= 2 ? 3 : 2),ds_12,false)
        , sot(sot0), n(n0), shared(shared0), withConst(withConst0)
        , is(0,1) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        int realN = shared == 0 ? n : 3;

        CountableSetRanges* isrs = new CountableSetRanges[realN];

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

        int result = shared == 0 ? x.size() - 1 : (shared <= 2 ? 2 : 0);
        CountableSetRanges xnr(x.lub, x[result]);

        switch (sot) {
        case SOT_DUNION:
          {
            if (shared == 1 && (isrs[0]() || isrs[1]())) {
              delete[] isrs; return false;
            }
            if (shared == 3 && (isrs[0]() || isrs[2]())) {
              delete[] isrs; return false;
            }
            unsigned int cardSum = 0;
            if (shared == 1 || shared == 3) {
              CountableSetRanges x1r(x.lub, x[1]);
              cardSum = Iter::Ranges::size(x1r);
            } else {
              for (int i=0; i<realN; i++) {
                CountableSetRanges xir(x.lub, x[i]);
                cardSum += Iter::Ranges::size(xir);
              }
            }
            if (withConst)
              cardSum += 2;
            CountableSetRanges xnr2(x.lub, x[result]);
            if (cardSum != Iter::Ranges::size(xnr2)) {
              delete[] isrs; return false;
            }
          }
          // fall through to union case
        case SOT_UNION:
          {
            FakeSpace* fs = new FakeSpace;
            bool eq;
            if (withConst) {
              Region r(*fs);
              Iter::Ranges::NaryUnion u(r, isrs, realN);
              IntSetRanges isr(is);
              Iter::Ranges::Union<IntSetRanges,
                Iter::Ranges::NaryUnion> uu(isr, u);
              eq = Iter::Ranges::equal(uu, xnr);
            } else {
              Region r(*fs);
              Iter::Ranges::NaryUnion u(r, isrs, realN);
              eq = Iter::Ranges::equal(u, xnr);
            }
            delete [] isrs;
            delete fs;
            return eq;
          }
        case SOT_INTER:
          {
            if (withConst) {
              FakeSpace* fs = new FakeSpace;
              bool eq;
              {
                Region r(*fs);
                Iter::Ranges::NaryInter u(r, isrs, realN);
                IntSetRanges isr(is);
                Iter::Ranges::Inter<IntSetRanges,
                  Iter::Ranges::NaryInter> uu(isr, u);
                eq = (realN == 0 ? Iter::Ranges::equal(isr, xnr) :
                           Iter::Ranges::equal(uu, xnr));
                delete [] isrs;
              }
              delete fs;
              return eq;
            } else {
              if (realN == 0) {
                bool ret =
                  Iter::Ranges::size(xnr) ==  Gecode::Set::Limits::card;
                delete [] isrs;
                return ret;
              } else {
                FakeSpace* fs = new FakeSpace;
                bool eq;
                {
                  Region r(*fs);
                  Iter::Ranges::NaryInter u(r,isrs, realN);
                  eq = Iter::Ranges::equal(u, xnr);
                }
                delete [] isrs;
                delete fs;
                return eq;
              }
            }
          }
        default:
          GECODE_NEVER;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space& home, SetVarArray& x, IntVarArray&) {
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
          xs[0] = x[0]; xs[1] = x[0]; xs[2] = x[1]; xn = x[2];
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
        if (!withConst)
          Gecode::rel(home, sot, xs, xn);
        else
          Gecode::rel(home, sot, xs, is, xn);
      }
    };

    /// Help class to create and register tests
    class CreateN {
    public:
      /// Perform creation and registration
      CreateN(void) {
        for (int wc=0; wc<=1; wc++) {
          for (int i=0; i<=3; i++) {
            (void) new RelN(Gecode::SOT_UNION, i, 0, wc);
            (void) new RelN(Gecode::SOT_DUNION, i, 0, wc);
            (void) new RelN(Gecode::SOT_INTER, i, 0, wc);

            if (i>0) {
              (void) new RelN(Gecode::SOT_UNION, 0, i, wc);
              (void) new RelN(Gecode::SOT_DUNION, 0, i, wc);
              (void) new RelN(Gecode::SOT_INTER, 0, i, wc);
            }
          }
        }
      }
    };

    CreateN cn;

    /// %Test for n-ary partition constraint
    class RelIntN : public SetTest {
    private:
      Gecode::SetOpType sot;
      int n;
      bool withConst;
      IntSet is;
    public:
      /// Create and register test
      RelIntN(Gecode::SetOpType sot0, int n0, bool withConst0)
        : SetTest("RelOp::IntN::"+str(sot0)+"::"+str(n0)+
                  "::C"+str(withConst0 ? 1 : 0),
                  1,ds_12,false,n0)
        , sot(sot0), n(n0), withConst(withConst0)
        , is(0,1) {}
      /// %Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        int* isrs = new int[n];
        for (int i=0; i<n; i++)
          isrs[i] = x.ints()[i];

        IntSet iss(isrs,n);
        CountableSetRanges xnr(x.lub, x[0]);

        switch (sot) {
        case SOT_DUNION:
          {
            IntSetRanges issr(iss);
            unsigned int cardSum = Iter::Ranges::size(issr);
            if (cardSum != static_cast<unsigned int>(n)) {
              delete[] isrs;
              return false;
            }
            if (withConst) {
              IntSetRanges isr(is);
              cardSum += Iter::Ranges::size(isr);
            }
            CountableSetRanges xnr2(x.lub, x[0]);
            if (cardSum != Iter::Ranges::size(xnr2)) {
              delete[] isrs;
              return false;
            }
          }
          // fall through to union case
        case SOT_UNION:
          {
            if (withConst) {
              IntSetRanges issr(iss);
              IntSetRanges isr(is);
              Iter::Ranges::Union<IntSetRanges,IntSetRanges > uu(isr, issr);
              delete[] isrs;
              return Iter::Ranges::equal(uu, xnr);
            } else {
              IntSetRanges issr(iss);
              delete[] isrs;
              return Iter::Ranges::equal(issr, xnr);
            }
          }
        case SOT_INTER:
          {
            bool allEqual = true;
            for (int i=1; i<n; i++) {
              if (isrs[i] != isrs[0]) {
                allEqual = false;
                break;
              }
            }
            if (withConst) {
              if (allEqual) {
                if (n == 0) {
                  IntSetRanges isr(is);
                  delete[] isrs;
                  return Iter::Ranges::equal(isr, xnr);
                } else {
                  Iter::Ranges::Singleton s(isrs[0],isrs[0]);
                  IntSetRanges isr(is);
                  Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton>
                    uu(isr, s);
                  delete[] isrs;
                  return Iter::Ranges::equal(uu, xnr);
                }
              } else {
                delete[] isrs;
                return Iter::Ranges::size(xnr) == 0;
              }
            } else {
              if (allEqual) {
                if (n == 0) {
                  delete[] isrs;
                  return Iter::Ranges::size(xnr) == Gecode::Set::Limits::card;
                } else {
                  Iter::Ranges::Singleton s(isrs[0],isrs[0]);
                  delete[] isrs;
                  return Iter::Ranges::equal(s, xnr);
                }
              } else {
                delete[] isrs;
                return Iter::Ranges::size(xnr) == 0;
              }
            }
          }
        default:
          GECODE_NEVER;
        }
        GECODE_NEVER;
        return false;
      }
      /// Post constraint on \a x
      void post(Space& home, SetVarArray& x, IntVarArray& y) {
        if (!withConst)
          Gecode::rel(home, sot, y, x[0]);
        else
          Gecode::rel(home, sot, y, is, x[0]);
      }
    };

    /// Help class to create and register tests
    class CreateIntN {
    public:
      /// Perform creation and registration
      CreateIntN(void) {
        for (int wc=0; wc<=1; wc++) {
          for (int i=0; i<=3; i++) {
            (void) new RelIntN(Gecode::SOT_UNION, i, wc);
            (void) new RelIntN(Gecode::SOT_DUNION, i, wc);
            (void) new RelIntN(Gecode::SOT_INTER, i, wc);
          }
        }
      }
    };

    CreateIntN intcn;

    //@}

}}}

// STATISTICS: test-set
