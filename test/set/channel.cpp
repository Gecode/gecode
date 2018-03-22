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

#include "test/set.hh"
#include "test/int.hh"
#include <gecode/minimodel.hh>

using namespace Gecode;

namespace Test { namespace Set {

  /// %Tests for set channeling constraints
  namespace Channel {

    /**
      * \defgroup TaskTestSetChannel %Set channeling constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet d1(0,2);
    static IntSet d_12(-1,2);

    static IntSet d2(-1,3);
    static IntSet d3(0,3);

    static IntSet d4(0,4);

    static IntSet ds_33(-3,3);

    /// %Test for sorted channeling constraint
    class ChannelSorted : public SetTest {
    public:
      /// Create and register test
      ChannelSorted(const char* t)
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
    ChannelSorted _channelSorted("Channel::Sorted");

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

    ChannelInt _channelint1("Channel::Int::1", d2, 2, 3);
    ChannelInt _channelint2("Channel::Int::2", d3, 3, 3);

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

    ChannelBool _channelbool1("Channel::Bool::1", d2, 3);
    ChannelBool _channelbool2("Channel::Bool::2", d3, 3);
    ChannelBool _channelbool3("Channel::Bool::3", d4, 5);

    /// %Test for set channel constraint
    class ChannelSet : public SetTest {
    private:
      int _x0size, _x1size;
    public:
      /// Create and register test
      ChannelSet(const char* t, const IntSet& d, int x0size, int x1size)
        : SetTest(t,x0size+x1size,d,false), _x0size(x0size), _x1size(x1size) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        for (int i=0; i<_x0size; i++) {
          CountableSetRanges x0ir(x.lub, x[i]);
          IntSet x0is(x0ir);
          if (x0is.min() < 0 || x0is.max() >= _x1size)
            return false;
          for (int j=0; j<_x1size; j++) {
            CountableSetRanges x1ir(x.lub, x[_x0size+j]);
            IntSet x1is(x1ir);
            if (x1is.min() < 0 || x1is.max() >= _x0size)
              return false;
            bool jInI = x0is.in(j);
            bool iInJ = x1is.in(i);
            if (jInI != iInJ)
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs x0(x.slice(0,1,_x0size));
        SetVarArgs x1(x.slice(_x0size));
        Gecode::channel(home, x0,x1);
      }
    };

    ChannelSet _channelSet12("Channel::Set::1::2", d1, 2,2);
    ChannelSet _channelSet13("Channel::Set::1::3", d1, 2,3);
    ChannelSet _channelSet22("Channel::Set::2::2", d3, 2,2);
    ChannelSet _channelSet23("Channel::Set::2::3", d3, 2,3);
    ChannelSet _channelSet32("Channel::Set::3::2", d_12, 2,2);
    ChannelSet _channelSet33("Channel::Set::3::3", d_12, 2,3);

}}}

// STATISTICS: test-set
