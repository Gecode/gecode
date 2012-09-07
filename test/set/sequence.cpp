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

  /// %Tests for sequence constraints
  namespace Sequence {

    /**
      * \defgroup TaskTestSetSequence Sequence constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_33(-1,2);

    /// %Test for sequence constraint
    class Sequence : public SetTest {
    public:
      /// Create and register test
      Sequence(const char* t)
        : SetTest(t,4,ds_33,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int max = Gecode::Set::Limits::min - 1;
        for (int i=0; i<4; i++) {
          CountableSetRanges xir(x.lub, x[i]);
          IntSet xi(xir);
          if (xi.ranges() > 0) {
            int oldMax = max;
            max = xi.max();
            if (xi.min() <= oldMax)
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::sequence(home, x);
      }
    };
    Sequence _sequence("Sequence::Sequence");

    /// %Test for sequential-union constraint
    class SeqU : public SetTest {
    private:
      int n;
    public:
      /// Create and register test
      SeqU(int n0)
        : SetTest("Sequence::SeqU"+str(n0),n0+1,ds_33,false), n(n0) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int max = Gecode::Set::Limits::min - 1;
        for (int i=0; i<n; i++) {
          CountableSetRanges xir(x.lub, x[i]);
          IntSet xi(xir);
          if (xi.ranges() > 0) {
            int oldMax = max;
            max = xi.max();
            if (xi.min() <= oldMax)
              return false;
          }
        }
        CountableSetRanges* isrs = new CountableSetRanges[n];
        for (int i=n; i--; )
          isrs[i].init(x.lub, x[i]);
        FakeSpace* fs = new FakeSpace;
        bool ret;
        {
          Region r(*fs);
          Iter::Ranges::NaryUnion u(r, isrs, n);
          CountableSetRanges xnr(x.lub, x[n]);
          ret = Iter::Ranges::equal(u, xnr);
        }
        delete[] isrs;
        delete fs;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-1);
        for (int i=x.size()-1; i--;)
          xs[i]=x[i];
        Gecode::sequence(home, xs, x[x.size()-1]);
      }
    };
    SeqU _sequ0(0);
    SeqU _sequ1(1);
    SeqU _sequ3(3);

}}}

// STATISTICS: test-set
