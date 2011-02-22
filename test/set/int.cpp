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
#include "test/int.hh"
#include <gecode/minimodel.hh>

using namespace Gecode;

namespace Test { namespace Set {

  /// %Tests for combined int/set constraints
  namespace Int {

    /**
      * \defgroup TaskTestSetInt Combined integer/set constraints
      * \ingroup TaskTestSet
      */
    //@{

    static const int d1r[4][2] = {
      {-4,-3},{-1,-1},{1,1},{3,5}
    };
    static IntSet d1(d1r,4);

    static IntSet d2(-1,3);
    static IntSet d3(0,3);

    static IntSet d4(0,4);

    static IntSet ds_33(-3,3);

    /// %Test for cardinality constraint
    class Card : public SetTest {
    public:
      /// Create and register test
      Card(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        unsigned int s = 0;
        for (CountableSetRanges xr(x.lub, x[0]);xr();++xr) s+= xr.width();
        if (x.intval() < 0)
          return false;
        return s==(unsigned int)x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::cardinality(home, x[0], y[0]);
      }
    };
    Card _card("Int::Card");

    /// %Test for minimal element constraint
    class Min : public SetTest {
    public:
      /// Create and register test
      Min(const char* t)
        : SetTest(t,1,ds_33,true,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return xr0() && xr0.min()==x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::min(home, x[0], y[0]);
      }
      /// Post reified constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y,
                        BoolVar b) {
        Gecode::min(home, x[0], y[0], b);
      }
    };
    Min _min("Int::Min");

    /// %Test for negated minimal element constraint
    class NotMin : public SetTest {
    public:
      /// Create and register test
      NotMin(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return !(xr0() && xr0.min()==x.intval());
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::notMin(home, x[0], y[0]);
      }
    };
    NotMin _notmin("Int::NotMin");

    /// %Test for maximal element constraint
    class Max : public SetTest {
    public:
      /// Create and register test
      Max(const char* t)
        : SetTest(t,1,ds_33,true,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        IntSet x0(xr0);
        return x0.ranges() > 0 && x0.max()==x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::max(home, x[0], y[0]);
      }
      /// Post reified constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y,
                        BoolVar b) {
        Gecode::max(home, x[0], y[0], b);
      }
    };
    Max _max("Int::Max");

    /// %Test for negated maximal element constraint
    class NotMax : public SetTest {
    public:
      /// Create and register test
      NotMax(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        IntSet x0(xr0);
        return !(x0.ranges() > 0 && x0.max()==x.intval());
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::notMax(home, x[0], y[0]);
      }
    };
    NotMax _notmax("Int::NotMax");

    /// %Test for element constraint
    class Elem : public SetTest {
    public:
      /// Create and register test
      Elem(const char* t)
        : SetTest(t,1,ds_33,true,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (CountableSetValues xr(x.lub, x[0]);xr();++xr)
          if (xr.val()==x.intval())
            return true;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, x[0], SRT_SUP, y[0]);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y, BoolVar b) {
        Gecode::rel(home, x[0], SRT_SUP, y[0], b);
      }
    };
    Elem _elem("Int::Elem");

    /// %Test for negated element constraint
    class NoElem : public SetTest {
    public:
      /// Create and register test
      NoElem(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (CountableSetValues xr(x.lub, x[0]);xr();++xr)
          if (xr.val()==x.intval())
            return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, x[0], SRT_DISJ, y[0]);
      }
    };
    NoElem _noelem("Int::NoElem");

    /// %Test for relation constraint
    class Rel : public SetTest {
    private:
      Gecode::SetRelType srt;
      bool inverse;
    public:
      /// Create and register test
      Rel(Gecode::SetRelType srt0, bool inverse0)
        : SetTest("Int::Rel::"+str(srt0)+(inverse0 ? "::i" : ""),
                  1,ds_33,true,1)
        , srt(srt0)
        , inverse(inverse0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr(x.lub, x[0]);
        IntSet is(x.intval(), x.intval());
        IntSetRanges dr(is);
        Gecode::SetRelType rsrt = srt;
        if (inverse) {
          switch (srt) {
            case SRT_SUB: rsrt = SRT_SUP; break;
            case SRT_SUP: rsrt = SRT_SUB; break;
            default: break;
          }
        }
        switch (rsrt) {
        case SRT_EQ: return Iter::Ranges::equal(xr, dr);
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
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        if (!inverse)
          Gecode::rel(home, x[0], srt, y[0]);
        else
          Gecode::rel(home, y[0], srt, x[0]);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y,
                        BoolVar b) {
        if (!inverse)
          Gecode::rel(home, x[0], srt, y[0], b);
        else
          Gecode::rel(home, y[0], srt, x[0], b);
      }
    };
    Rel _rel_eq(Gecode::SRT_EQ,false);
    Rel _rel_nq(Gecode::SRT_NQ,false);
    Rel _rel_sub(Gecode::SRT_SUB,false);
    Rel _rel_sup(Gecode::SRT_SUP,false);
    Rel _rel_disj(Gecode::SRT_DISJ,false);
    Rel _rel_cmpl(Gecode::SRT_CMPL,false);
    Rel _rel_eqi(Gecode::SRT_EQ,true);
    Rel _rel_nqi(Gecode::SRT_NQ,true);
    Rel _rel_subi(Gecode::SRT_SUB,true);
    Rel _rel_supi(Gecode::SRT_SUP,true);
    Rel _rel_disji(Gecode::SRT_DISJ,true);
    Rel _rel_cmpli(Gecode::SRT_CMPL,true);

    /// %Test for integer relation constraint
    class IntRel : public SetTest {
    private:
      Gecode::IntRelType irt;
      bool inverse;
    public:
      /// Create and register test
      IntRel(Gecode::IntRelType irt0, bool inverse0)
        : SetTest("Int::IntRel::"+Test::Int::Test::str(irt0)+
                  (inverse0 ? "::i" : ""),1,ds_33,false,1)
        , irt(irt0)
        , inverse(inverse0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetValues xr(x.lub, x[0]);
        if (!xr())
          return false;
        for (; xr(); ++xr) {
          switch (irt) {
          case Gecode::IRT_EQ:
            if (xr.val() != x.intval()) return false;
            break;
          case Gecode::IRT_NQ:
            if (xr.val() == x.intval()) return false;
            break;
          case Gecode::IRT_GR:
            if (!inverse && xr.val() <= x.intval()) return false;
            if (inverse && xr.val() >= x.intval()) return false;
            break;
          case Gecode::IRT_GQ:
            if (!inverse && xr.val() < x.intval()) return false;
            if (inverse && xr.val() > x.intval()) return false;
            break;
          case Gecode::IRT_LE:
            if (!inverse && xr.val() >= x.intval()) return false;
            if (inverse && xr.val() <= x.intval()) return false;
            break;
          case Gecode::IRT_LQ:
            if (!inverse && xr.val() > x.intval()) return false;
            if (inverse && xr.val() < x.intval()) return false;
            break;
          default:
            GECODE_NEVER;
            return false;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        if (!inverse)
          Gecode::rel(home, x[0], irt, y[0]);
        else
          Gecode::rel(home, y[0], irt, x[0]);
      }
    };
    IntRel _intrel_eq(Gecode::IRT_EQ,false);
    IntRel _intrel_nq(Gecode::IRT_NQ,false);
    IntRel _intrel_gr(Gecode::IRT_GR,false);
    IntRel _intrel_gq(Gecode::IRT_GQ,false);
    IntRel _intrel_le(Gecode::IRT_LE,false);
    IntRel _intrel_lq(Gecode::IRT_LQ,false);
    IntRel _intrel_eqi(Gecode::IRT_EQ,true);
    IntRel _intrel_nqi(Gecode::IRT_NQ,true);
    IntRel _intrel_gri(Gecode::IRT_GR,true);
    IntRel _intrel_gqi(Gecode::IRT_GQ,true);
    IntRel _intrel_lei(Gecode::IRT_LE,true);
    IntRel _intrel_lqi(Gecode::IRT_LQ,true);


    template<class I>
    int weightI(const IntArgs& elements,
                const IntArgs& weights,
                I& iter) {
      int sum = 0;
      int i = 0;
      for (Iter::Ranges::ToValues<I> v(iter); v(); ++v) {
        // Skip all elements below the current
        while (elements[i]<v.val()) i++;
        assert(elements[i] == v.val());
        sum += weights[i];
      }
      return sum;
    }

    /// %Test for set weight constraint
    class Weights : public SetTest {
    public:
      IntArgs elements;
      IntArgs weights;
      int minWeight;
      int maxWeight;
      /// Create and register test
      Weights(const char* t, IntArgs& el, IntArgs& w,
              int min = -10000, int max = 10000)
        : SetTest(t,1,ds_33,false,1),
          elements(el), weights(w), minWeight(min), maxWeight(max) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges x0(x.lub, x[0]);
        return x.intval()==weightI(elements,weights,x0) &&
               x.intval() >= minWeight && x.intval() <= maxWeight;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, minWeight <= y[0]);
        Gecode::rel(home, maxWeight >= y[0]);
        Gecode::weights(home, elements, weights, x[0], y[0]);
      }
    };

    const int el1v[] = {-3,-2,-1,0,1,2,3};
    IntArgs el1(7,el1v);
    const int w1v[] = {1,-2,1,1,1,6,1};
    IntArgs w1(7,w1v);
    Weights _weights1("Int::Weights::1", el1, w1);

    const int w2v[] = {-1,-1,-1,10,-1,-1,-1};
    IntArgs w2(7,w2v);
    Weights _weights2("Int::Weights::2", el1, w2);
    Weights _weights3("Int::Weights::3", el1, w2, 3);

    const int w4v[] = {1,1,0,0,0,0,0};
    IntArgs w4(7,w4v);
    Weights _weights4("Int::Weights::4", el1, w4);

    /// %Test for match constraint
    class Match : public SetTest {
    public:
      /// Create and register test
      Match(const char* t)
        : SetTest(t,1,ds_33,false,3) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        if (x.ints()[0]>=x.ints()[1] ||
            x.ints()[1]>=x.ints()[2])
          return false;
        CountableSetValues xr(x.lub, x[0]);
        if (!xr())
          return false;
        if (xr.val() != x.ints()[0])
          return false;
        ++xr;
        if (!xr())
          return false;
        if (xr.val() != x.ints()[1])
          return false;
        ++xr;
        if (!xr())
          return false;
        if (xr.val() != x.ints()[2])
          return false;
        ++xr;
        if (xr())
          return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::channelSorted(home, y, x[0]);
      }
    };
    Match _match("Int::Match");

    /// %Test for integer channel constraint
    class ChannelInt : public SetTest {
    private:
      int ssize, isize;
    public:
      /// Create and register test
      ChannelInt(const char* t, const IntSet& d, int _ssize, int _isize)
        : SetTest(t,_ssize,d,false,_isize), ssize(_ssize), isize(_isize) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=0; i<isize; i++) {
          if (x.ints()[i] < 0 || x.ints()[i] >= ssize)
            return false;
          Iter::Ranges::Singleton single(i,i);
          CountableSetRanges csr(x.lub, x[x.ints()[i]]);
          if (!Iter::Ranges::subset(single, csr))
            return false;
        }
        for (int i=0; i<ssize; i++) {
          int size = 0;
          for (CountableSetValues csv(x.lub, x[i]); csv(); ++csv) {
            if (csv.val() < 0 || csv.val() >= isize) return false;
            if (x.ints()[csv.val()] != i) return false;
            size++;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        Gecode::channel(home, y, x);
      }
    };

    ChannelInt _channelint1("Int::Channel::Int::1", d2, 2, 3);
    ChannelInt _channelint2("Int::Channel::Int::2", d3, 3, 3);

    /// %Test for Boolean channel constraint
    class ChannelBool : public SetTest {
    private:
      int isize;
    public:
      /// Create and register test
      ChannelBool(const char* t, const IntSet& d, int _isize)
        : SetTest(t,1,d,false,_isize), isize(_isize) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=0; i<isize; i++) {
          if (x.ints()[i] < 0 || x.ints()[i] > 1)
            return false;
        }
        int cur = 0;
        for (CountableSetValues csv(x.lub, x[0]); csv(); ++csv) {
          if (csv.val() < 0 || csv.val() >= isize) return false;
          if (x.ints()[csv.val()] != 1) return false;
          for (; cur<csv.val(); cur++)
            if (x.ints()[cur] != 0) return false;
          cur = csv.val() + 1;
        }
        for (; cur<isize; cur++)
          if (x.ints()[cur] != 0) return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        BoolVarArgs b(y.size());
        for (int i=y.size(); i--;)
          b[i] = channel(home, y[i]);
        Gecode::channel(home, b, x[0]);
      }
    };

    ChannelBool _channelbool1("Int::Channel::Bool::1", d2, 3);
    ChannelBool _channelbool2("Int::Channel::Bool::2", d3, 3);
    ChannelBool _channelbool3("Int::Channel::Bool::3", d4, 5);

}}}

// STATISTICS: test-set
