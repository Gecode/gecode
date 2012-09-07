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

#include <gecode/minimodel.hh>

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {

  /// %Tests for set element constraints
  namespace Element {

    /**
      * \defgroup TaskTestSetElement Element constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_12(-1,2);
    static IntSet ds_13(-1,3);

    /// %Test for %ElementUnion constraint
    class ElementUnion : public SetTest {
    public:
      /// Create and register test
      ElementUnion(const char* t)
        : SetTest(t,5,ds_12,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int selected = 0;
        for (CountableSetValues sel2(x.lub, x[3]); sel2();
             ++sel2, selected++) {}
        CountableSetValues x4v(x.lub, x[4]);
        if (selected==0)
          return !x4v();
        CountableSetRanges* sel = new CountableSetRanges[selected];
        CountableSetValues selector(x.lub, x[3]);
        for (int i=selected; i--;++selector) {
          if (selector.val()>=3 || selector.val()<0) {
            delete[] sel;
            return false;
          }
          sel[i].init(x.lub, x[selector.val()]);
        }
        
        FakeSpace* fs = new FakeSpace;
        bool ret;
        {
          Region r(*fs);
          Iter::Ranges::NaryUnion u(r, sel, selected);
          
          CountableSetRanges z(x.lub, x[4]);
          ret = Iter::Ranges::equal(u, z);
        }
        delete[] sel;
        delete fs;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-2);
        for (int i=x.size()-2; i--;)
          xs[i]=x[i];
        Gecode::element(home, SOT_UNION, xs, x[x.size()-2], x[x.size()-1]);
      }
    };
    ElementUnion _elementunion("Element::Union");

    /// %Test for %ElementUnionConst constraint
    class ElementUnionConst : public SetTest {
    private:
      const IntSet i0;
      const IntSet i1;
      const IntSet i2;
    public:
      /// Create and register test
      ElementUnionConst(const char* t)
        : SetTest(t,2,ds_13,false), i0(-3,-3), i1(-1,1), i2(0,2) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int selected = 0;
        for (CountableSetValues sel2(x.lub, x[0]); sel2();
             ++sel2, selected++) {}
        CountableSetValues x4v(x.lub, x[1]);
        if (selected==0)
          return !x4v();
        IntSet iss[] = {i0, i1, i2};
        IntSetRanges* sel = new IntSetRanges[selected];
        CountableSetValues selector(x.lub, x[0]);
        for (int i=selected; i--;++selector) {
          if (selector.val()>=3 || selector.val()<0) {
            delete[] sel;
            return false;
          }
          sel[i].init(iss[selector.val()]);
        }

        FakeSpace* fs = new FakeSpace;
        bool ret;
        {
          Region r(*fs);
          Iter::Ranges::NaryUnion u(r, sel, selected);

          CountableSetRanges z(x.lub, x[1]);
          ret = Iter::Ranges::equal(u, z);
        }
        delete[] sel;
        delete fs;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        IntSetArgs xs(3);
        xs[0] = i0; xs[1] = i1; xs[2] = i2;
        Gecode::element(home, SOT_UNION, xs, x[0], x[1]);
      }
    };
    ElementUnionConst _elementunionconst("Element::UnionConst");

    /// %Test for %ElementInter constraint
    class ElementInter : public SetTest {
    public:
      /// Create and register test
      ElementInter(const char* t)
        : SetTest(t,5,ds_12,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int selected = 0;
        for (CountableSetValues sel2(x.lub, x[3]); sel2();
             ++sel2, selected++) {}
        CountableSetRanges x4r(x.lub, x[4]);
        if (selected==0)
          return Iter::Ranges::size(x4r)==Gecode::Set::Limits::card;
        CountableSetRanges* sel = new CountableSetRanges[selected];
        CountableSetValues selector(x.lub, x[3]);
        for (int i=selected; i--;++selector) {
          if (selector.val()>=3 || selector.val()<0) {
            delete[] sel;
            return false;
          }
          sel[i].init(x.lub, x[selector.val()]);
        }
        FakeSpace* fs = new FakeSpace;
        bool ret;
        {
          Region r(*fs);
          Iter::Ranges::NaryInter u(r, sel, selected);

          CountableSetRanges z(x.lub, x[4]);
          ret = Iter::Ranges::equal(u, z);
        }
        delete fs;
        delete [] sel;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-2);
        for (int i=x.size()-2; i--;)
          xs[i]=x[i];
        Gecode::element(home, SOT_INTER, xs, x[x.size()-2], x[x.size()-1]);
      }
    };
    ElementInter _elementinter("Element::Inter");

    /// %Test for %ElementInter constraint
    class ElementInterIn : public SetTest {
    public:
      /// Create and register test
      ElementInterIn(const char* t)
        : SetTest(t,5,ds_12,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int selected = 0;
        for (CountableSetValues sel2(x.lub, x[3]); sel2();
             ++sel2, selected++) {}
        CountableSetRanges x4r(x.lub, x[4]);
        if (selected==0)
          return Iter::Ranges::size(x4r)==4;
        CountableSetRanges* sel = new CountableSetRanges[selected];
        CountableSetValues selector(x.lub, x[3]);
        for (int i=selected; i--;++selector) {
          if (selector.val()>=3 || selector.val()<0) {
            delete[] sel;
            return false;
          }
          sel[i].init(x.lub, x[selector.val()]);
        }
        FakeSpace* fs = new FakeSpace;
        bool ret;
        {
          Region r(*fs);
          Iter::Ranges::NaryInter u(r,sel, selected);
          
          CountableSetRanges z(x.lub, x[4]);
          ret = Iter::Ranges::equal(u, z);
        }
        delete fs;
        delete [] sel;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-2);
        for (int i=x.size()-2; i--;)
          xs[i]=x[i];
        Gecode::element(home, SOT_INTER, xs, x[x.size()-2], x[x.size()-1], 
          ds_12);
      }
    };
    ElementInterIn _elementinterin("Element::InterIn");

    /// %Test for %ElementDisjoint constraint
    class ElementDisjoint : public SetTest {
    public:
      /// Create and register test
      ElementDisjoint(const char* t)
        : SetTest(t,5,ds_12,false) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        int selected = 0;
        for (CountableSetValues sel2(x.lub, x[3]); sel2();
             ++sel2, selected++) {
          if (sel2.val() < 0)
            return false;
        }
        CountableSetValues x4v(x.lub, x[4]);
        if (selected == 0)
          return !x4v();
        CountableSetRanges* sel = new CountableSetRanges[selected];
        CountableSetValues selector(x.lub, x[3]);
        unsigned int cardsum = 0;
        for (int i=selected; i--;++selector) {
          if (selector.val()>=3 || selector.val()<0) {
            delete[] sel;
            return false;
          }
          sel[i].init(x.lub, x[selector.val()]);
          CountableSetRanges xicard(x.lub, x[selector.val()]);
          cardsum += Iter::Ranges::size(xicard);
        }
        
        bool ret;
        FakeSpace* fs = new FakeSpace;
        {
          Region r(*fs);
          Iter::Ranges::NaryUnion u(r, sel, selected);
          ret = Iter::Ranges::size(u) == cardsum;
          u.reset();
          CountableSetRanges z(x.lub, x[4]);
          ret &= Iter::Ranges::equal(u, z);
        }
        delete fs;
        delete[] sel;
        return ret;
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        SetVarArgs xs(x.size()-2);
        for (int i=x.size()-2; i--;)
          xs[i]=x[i];
        Gecode::element(home, SOT_DUNION, xs, x[x.size()-2], x[x.size()-1]);
      }
    };
    ElementDisjoint _elementdisjoint("Element::Disjoint");

    /// %Test for %ElementSet constraint
    class ElementSet : public SetTest {
    public:
      /// Create and register test
      ElementSet(const char* t)
        : SetTest(t,4,ds_12,false,true) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        if (x.intval() < 0 || x.intval() > 2)
          return false;
        CountableSetRanges z(x.lub, x[3]);
        CountableSetRanges y(x.lub, x[x.intval()]);
        return Iter::Ranges::equal(y, z);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        SetVarArgs xs(x.size()-1);
        for (int i=x.size()-1; i--;)
          xs[i]=x[i];
        Gecode::element(home, xs, y[0], x[x.size()-1]);
      }
    };
    ElementSet _elementset("Element::Set");

    /// %Test for %ElementSetConst constraint
    class ElementSetConst : public SetTest {
    private:
      const IntSet i0;
      const IntSet i1;
      const IntSet i2;
    public:
      /// Create and register test
      ElementSetConst(const char* t)
        : SetTest(t,1,ds_13,false,true), i0(-3,-3), i1(-1,1), i2(0,2) {}
      /// %Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        if (x.intval() < 0 || x.intval() > 2)
          return false;
        CountableSetRanges xr(x.lub, x[0]);
        IntSet iss[] = {i0, i1, i2};
        IntSetRanges isr(iss[x.intval()]);
        return Iter::Ranges::equal(xr, isr);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray& y) {
        IntSetArgs xs(3);
        xs[0] = i0; xs[1] = i1; xs[2] = i2;
        Gecode::element(home, xs, y[0], x[0]);
      }
    };
    ElementSetConst _elementsetconst("Element::SetConst");

    /// %Test for matrix element with integer set array and set variable
    class MatrixIntSet : public SetTest {
     protected:
       /// Array for test matrix
       Gecode::IntSetArgs tm;
     public:
       /// Create and register test
       MatrixIntSet(void)
         : SetTest("Element::Matrix::IntSet",1,IntSet(0,3),false,2), 
           tm(4) {
         tm[0]=IntSet(0,0); tm[1]=IntSet(1,1);
         tm[2]=IntSet(2,2); tm[3]=IntSet(3,3);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const SetAssignment& x) const {
         // Get integer assignment
         const Int::Assignment& y = x.ints();
         // x-coordinate: y[0], y-coordinate: y[1], result: x[0]
         using namespace Gecode;
         if ((y[0] > 1) || (y[1] > 1))
           return false;
         Matrix<IntSetArgs> m(tm,2,2);
         IntSetRanges a(m(y[0],y[1]));
         CountableSetRanges b(x.lub, x[0]);
         return Iter::Ranges::equal(a,b);
       }
       /// Post constraint on \a x and \a y
       virtual void post(Gecode::Space& home, Gecode::SetVarArray& x,
                         Gecode::IntVarArray& y) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         Matrix<IntSetArgs> m(tm,2,2);
         element(home, m, y[0], y[1], x[0]);
       }
     };

    MatrixIntSet _emis;

    //@}

}}}

// STATISTICS: test-set
