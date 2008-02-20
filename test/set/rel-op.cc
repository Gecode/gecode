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
    class RelDUnionN : public SetTest {
    private:
      int n;
    public:
      /// Create and register test
      RelDUnionN(int n0)
        : SetTest("RelOp::DUnionN::"+str(n0),n0+1,ds_12,false), n(n0) {}
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,2,ds_22,false) {}
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
        : SetTest("RelOp::UnionN::"+str(n0),n0+1,ds_12,false), n(n0) {}
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,2,ds_22,false) {}
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
        : SetTest("RelOp::InterN"+str(n0),n0+1,ds_12,false), n(n0) {}
      /// Test whether \a x is solution
      bool solution(const SetAssignment& x) const {
        if (n == 0) {
          CountableSetRanges xnr(x.lub, x[n]);
          return Iter::Ranges::size(xnr) == Gecode::Set::Limits::card;
        }
          
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,3,ds_12,false) {}
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
        : SetTest(t,2,ds_22,false) {}
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
