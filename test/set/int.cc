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
#include "gecode/minimodel.hh"

using namespace Gecode;

namespace Test { namespace Set {

  /// Tests for combined int/set constraints
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

    /// Test for cardinality constraint
    class Card : public SetTest {
    public:
      /// Create and register test
      Card(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        unsigned int s = 0;
        for (CountableSetRanges xr(x.lub, x[0]);xr();++xr) s+= xr.width();
        if (x.intval() < 0)
          return false;
        return s==(unsigned int)x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::cardinality(home, x[0], y[0]);
      }
    };
    Card _card("Int::Card");

    /// Test for minimal element constraint
    class Min : public SetTest {
    public:
      /// Create and register test
      Min(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        return xr0() && xr0.min()==x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::min(home, x[0], y[0]);
      }
    };
    Min _min("Int::Min");

    /// Test for maximal element constraint
    class Max : public SetTest {
    public:
      /// Create and register test
      Max(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        IntSet x0(xr0);
        return x0.size() > 0 && x0.max()==x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::max(home, x[0], y[0]);
      }
    };
    Max _max("Int::Max");

    /// Test for element constraint
    class Elem : public SetTest {
    public:
      /// Create and register test
      Elem(const char* t)
        : SetTest(t,1,ds_33,true,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for(CountableSetValues xr(x.lub, x[0]);xr();++xr)
          if (xr.val()==x.intval())
            return true;
        return false;
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, x[0], SRT_SUP, y[0]);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y, BoolVar b) {
        Gecode::rel(home, x[0], SRT_SUP, y[0], b);
      }
    };
    Elem _elem("Int::Elem");

    /// Test for negated element constraint
    class NoElem : public SetTest {
    public:
      /// Create and register test
      NoElem(const char* t)
        : SetTest(t,1,ds_33,false,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for(CountableSetValues xr(x.lub, x[0]);xr();++xr)
          if (xr.val()==x.intval())
            return false;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, x[0], SRT_DISJ, y[0]);
      }
    };
    NoElem _noelem("Int::NoElem");

    /// Test for equality constraint
    class The : public SetTest {
    public:
      /// Create and register test
      The(const char* t)
        : SetTest(t,1,ds_33,true,1) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        IntSet x0(xr0);
        return x0.size()==1 && x0.min()==x0.max() &&
          x0.max()==x.intval();
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::rel(home, x[0], SRT_EQ, y[0]);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y, BoolVar b) {
        Gecode::rel(home, x[0], SRT_EQ, y[0], b);
      }
    };
    The _the("Int::The");

    template <class I>
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

    /// Test for set weight constraint
    class Weights : public SetTest {
    public:
      IntArgs elements;
      IntArgs weights;

      /// Create and register test
      Weights(const char* t)
        : SetTest(t,1,ds_33,false,1),
          elements(7), weights(7) {
        for (int i=-3; i<=3; i++)
          elements[i+3] = i;
        for (int i=0; i<7; i++)
          weights[i] = 1;
        weights[1] = -2;
        weights[5] = 6;
      }
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges x0(x.lub, x[0]);
        return x.intval()==weightI(elements,weights,x0);
      }
      /// Post constraint on \a x
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::weights(home, elements, weights, x[0], y[0]);
      }
    };
    Weights _weights("Int::Weights");

    /// Test for match constraint
    class Match : public SetTest {
    public:
      /// Create and register test
      Match(const char* t)
        : SetTest(t,1,ds_33,false,3) {}
      /// Test whether \a x is solution
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
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::match(home, x[0], y);
      }
    };
    Match _match("Int::Match");

    /// Test for integer channel constraint
    class ChannelInt : public SetTest {
    private:
      int ssize, isize;
    public:
      /// Create and register test
      ChannelInt(const char* t, const IntSet& d, int _ssize, int _isize)
        : SetTest(t,_ssize,d,false,_isize), ssize(_ssize), isize(_isize) {}
      /// Test whether \a x is solution
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
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
        Gecode::channel(home, y, x);
      }
    };

    ChannelInt _channelint1("Int::Channel::Int::1", d2, 2, 3);
    ChannelInt _channelint2("Int::Channel::Int::2", d3, 3, 3);

    /// Test for Boolean channel constraint
    class ChannelBool : public SetTest {
    private:
      int isize;
    public:
      /// Create and register test
      ChannelBool(const char* t, const IntSet& d, int _isize)
        : SetTest(t,1,d,false,_isize), isize(_isize) {}
      /// Test whether \a x is solution
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
      virtual void post(Space* home, SetVarArray& x, IntVarArray& y) {
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
