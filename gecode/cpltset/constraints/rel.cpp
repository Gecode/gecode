/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include <gecode/cpltset/propagators.hh>

using namespace Gecode::CpltSet;

namespace Gecode {

  namespace CpltSet { namespace Rel {

    /**
     * \namespace Gecode::CpltSet::Rel
     * \brief Propagators for relation constraints
     */

    template <class View>
    void rel_post(Space& home, View x, CpltSetRelType r, View y) {

      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      unsigned int xoff = x.offset();
      unsigned int yoff = y.offset();
      unsigned int tab = std::max(x.tableWidth(), y.tableWidth());

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      switch(r) {
      case SRT_LE:
        {
          d0 = lexlt(xoff, yoff, tab, tab - 1);
        }
        break;
      case SRT_GR:
        {
          d0 = lexlt(yoff, xoff, tab, tab - 1);
        }
        break;
      case SRT_LQ:
        {
          d0 = lexlq(xoff, yoff, tab, tab - 1);
        }
        break;
      case SRT_GQ:
        {
          d0 = lexlt(yoff, xoff, tab, tab - 1);
        }
        break;
      case SRT_LE_REV:
        {
          d0 = lexltrev(xoff, yoff, tab, 0);
        }
        break;
      case SRT_GR_REV:
        {
          d0 = lexltrev(yoff, xoff, tab, 0);
        }
        break;
      case SRT_LQ_REV:
        {
          d0 = lexlqrev(xoff, yoff, tab, 0);
        }
        break;
      case SRT_GQ_REV:
        {
          d0 = lexltrev(yoff, xoff, tab, 0);
        }
        break;
      default:
        {
          throw CpltSet::InvalidRelation (" COMPL = EQ NEG y ??? ");
          return;
        }
      }

      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View,View>::post(home, x, y, d0)));
    }

    template <class View>
    void rel_post(Space& home, View x, SetRelType r, View y) {

      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      unsigned int tab = std::max(x.tableWidth(), y.tableWidth());

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      switch(r) {
      case SRT_SUB:
        {
          for (int i = 0; i < (int) tab; i++) {
            d0 &= (x.element(i)) >>= (y.element(i));
          }
        }
        break;
      case SRT_SUP:
        {
          for (int i = 0; i < (int) tab; i++) {
            d0 &= (y.element(i)) >>= (x.element(i));
          }
        }
        break;
        // try whether changing the bit order is faster for conjunction
      case SRT_DISJ:
        {
          Set::LubRanges<View> lubx(x);
          Set::LubRanges<View> luby(y);
          Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
            Set::LubRanges<View> > inter(lubx, luby);
          Gecode::Iter::Ranges::ToValues<
            Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
              Set::LubRanges<View> > > ival(inter);

          ValCache<
            Gecode::Iter::Ranges::ToValues<
              Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
                Set::LubRanges<View> > > > cache(ival);

          if (!cache()) {
            return;
          } else {
            cache.last();
            for (; cache(); --cache) {
              int v = cache.min();
              int minx = x.initialLubMin();
              int miny = y.initialLubMin();
              d0 &= (!(x.element(v - minx) & y.element(v - miny)));
            }
          }
          GECODE_ES_FAIL(home, (BinRelDisj<View,View>::post(home, x, y, d0)));
          return;
        }
        break;
      case SRT_EQ:
        {
          int xshift = 0;
          for (int i = 0; i < (int) tab; i++) {
            if (y.initialLubMin() + i < x.initialLubMin() ||
                y.initialLubMin() + i > x.initialLubMax()) {
              d0 &= (bdd_false() % y.element(i));
              xshift++;
            } else {
              d0 &= (x.element(i - xshift) % y.element(i));
            }
          }
        }
        break;
      case SRT_NQ:
        {
          Set::LubRanges<View> lubx(x);
          Set::LubRanges<View> luby(y);
          Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
            Set::LubRanges<View> > inter(lubx, luby);
          Gecode::Iter::Ranges::ToValues<
            Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
              Set::LubRanges<View> > > ival(inter);
          if (!ival()) {
            return;
          } else {
            for (; ival(); ++ival) {
              int v = ival.val();
              assert(v >= x.initialLubMin());
              assert(v <= x.initialLubMax());
              d0 &= (x.element(v - x.initialLubMin()) %
                     y.element(v - x.initialLubMax()));
            }
          }
          d0 = !d0;
        }
        break;
      default:
        {
          throw CpltSet::InvalidRelation (" COMPL = EQ NEG y ??? ");
          return;
        }
      }

      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View,View>::post(home, x, y, d0)));
    }

    template <class View0, class View1>
    void rel_post(Space& home, View0 x, CpltSetRelType r, View1 s) {
      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      unsigned int xoff = x.offset();
      unsigned int yoff = s.offset();
      unsigned int tab = std::max(x.tableWidth(), s.tableWidth());

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      switch(r) {
        // introducing lexicographic ordering constraint
        // over the bitvectors of the corresponding sets
        // x[0] <_lex x[1], i.e. {2} <_lex {1} because 010 <_lex 100
      case SRT_LE:
        {
          d0 = lexlt(xoff, yoff, tab, tab - 1);
        }
        break;
      case SRT_GR:
        {
          d0 = lexlt(yoff, xoff, tab, tab - 1);
        }
        break;
      case SRT_LQ:
        {
          d0 = lexlq(xoff, yoff, tab, tab - 1);
        }
        break;
      case SRT_GQ:
        {
          d0 = lexlt(yoff, xoff, tab, tab - 1);
        }
        break;
      default:
        {
          throw CpltSet::InvalidRelation (" COMPL = EQ NEG y ??? ");
          return;
        }
      }

      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View0, View1>::post(home, x, s, d0)));
    }


    template <class View0, class View1>
    void rel_post(Space& home, View0 x, SetRelType r, View1 s) {
      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      unsigned int tab = std::max(x.tableWidth(), s.tableWidth());

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      switch(r) {
      case SRT_SUB:
        {
          // x < s
          int xshift = x.initialLubMin() - s.initialLubMin();
          for (int i = 0; i < (int) tab; i++) {
            if (s.initialLubMin() + i >= x.initialLubMin()) {
              if (s.initialLubMin() + i <= x.initialLubMax()) {
                d0 &= (x.element(i - xshift)) >>= (s.element(i));
              } else {
                // d0 &= s.element(i);
              }
            } else {
              // d0 &= s.element(i);
            }
          }
        }

        if (s.assigned()) {
          // assigned
          d0 &= s.dom();
        }
        break;
      case SRT_SUP:
        {
          for (int i = 0; i < (int) tab; i++) {
            d0 &= (s.element(i)) >>= (x.element(i));
          }
        }
        break;
      case SRT_DISJ:
        {
          for (int i = 0; i < (int) tab; i++) {
            d0 &= !(s.element(i) & x.element(i));
          }
        }
        break;
      case SRT_EQ:
        {
          int xshift = 0;
          for (int i = 0; i < (int) tab; i++) {
            if (s.initialLubMin() + i < x.initialLubMin() ||
                s.initialLubMin() + i > x.initialLubMax()) {
              d0 &= (bdd_false() % s.element(i));
              xshift++;
            } else {
              d0 &= (x.element(i - xshift) % s.element(i));
            }
          }
        }
        break;
      case SRT_NQ:
        {
          for (int i = 0; i < (int) tab; i++) {
            d0 &= ((x.element(i)) % (s.element(i)));
          }
          d0 = !d0;
        }
        break;
      default:
        {
          throw CpltSet::InvalidRelation (" COMPL = EQ NEG y ??? ");
          return;
        }
      }

      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View0, View1>::post(home, x, s, d0)));
    }

    // BddOp and BddRel
    //
    template <class View>
    void rel_post(Space&, ViewArray<View>&, CpltSetOpType,
                  CpltSetRelType) {
      throw CpltSet::InvalidRelation(" no bdd rel implemented lex smaller ....");
    }

    // BddOp and SetRel
    template <class View>
    void rel_post(Space& home, ViewArray<View>& x, CpltSetOpType o,
                  SetRelType r) {
      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      int x0_tab = x[0].tableWidth();

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      for (int i = x0_tab; i--; ) {
        bdd op = bdd_true();
        switch(o) {
        case SOT_SYMDIFF:
          {
            op = ((x[0].element(i) & (!x[1].element(i))) |
                  (!x[0].element(i) & (x[1].element(i)))) ;
            break;
          }
        default:
          {
            throw CpltSet::InvalidRelation(" other op rel relations not yet implemented ");
            return;
          }
        }
        switch (r) {
        case SRT_EQ:
          {
            d0 &= (op % x[2].element(i));
            break;
          }
        default:
          {
            throw CpltSet::InvalidRelation(" other rel relations not yet implemented ");
            return;
          }
        }
      }

      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }

    // SetOp and BddRel
    //
    template <class View>
    void rel_post(Space&, ViewArray<View>&, SetOpType,
                  CpltSetRelType) {
      throw CpltSet::InvalidRelation(" no bdd rel implemented lex smaller with setoptype....");
    }
    // not yet implemented Bddrel SRT_LE and lex-stuff

    // SetOp and SetRel
    template <class View>
    void rel_post(Space& home, ViewArray<View>& x,
                  SetOpType o, SetRelType r) {
      if (home.failed()) return;
      // important:
      // the offset order is linear from left to right for the viewarray

      int x0_tab = x[0].tableWidth();

      // Initialize the bdd representing the constraint
      bdd d0 = bdd_true();

      for (int i = x0_tab; i--; ) {
        bdd op = bdd_true();
        switch(o) {
        case SOT_UNION:
          {
            op = (x[0].element(i) | x[1].element(i));
            break;
          }
        case SOT_DUNION:
          {
            op = (x[0].element(i) | x[1].element(i));
            // for disjointness see below
            break;
          }
        case SOT_INTER:
          {
            op = x[0].element(i) & x[1].element(i);
            break;
          }
        case SOT_MINUS:
          {
            op = x[0].element(i) & (!x[1].element(i));
            break;
          }
        default:
          {
            GECODE_NEVER;
            return;
          }
        }
        switch (r) {
        case SRT_EQ:
          {
            d0 &= (op % x[2].element(i));
            if (o == SOT_DUNION)
              d0 &= !(x[0].element(i) &  x[1].element(i));
            break;
          }
        default:
          {
            throw CpltSet::InvalidRelation(" other rel relations not yet implemented ");
            return;
          }
        }
      }
      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }


    template <class Rel>
    void rel_con(Space& home, const CpltSetVar& x, Rel r,
                 const CpltSetVar& y) {
      CpltSetView xv(x);
      CpltSetView yv(y);
      rel_post(home, xv, r, yv);
    }

    template <class Rel, class Op>
    forceinline void
    rel_con_bdd(Space& home, const CpltSetVar& x, Op o, const CpltSetVar& y,
                Rel r, const CpltSetVar& z) {
      ViewArray<CpltSetView> bv(home, 3);
      bv[0] = x;
      bv[1] = y;
      bv[2] = z;
      rel_post(home, bv, o, r);
    }

  }} // end namespace CpltSet::Rel

  using namespace CpltSet::Rel;

  void rel(Space& home, CpltSetVar x, CpltSetRelType r, CpltSetVar y) {
    rel_con(home, x, r, y);
  }

  void rel(Space& home, CpltSetVar x, CpltSetOpType o, CpltSetVar y,
           CpltSetRelType r, CpltSetVar z) {
    rel_con_bdd(home, x, o, y, r, z);
  }

  void rel(Space& home, CpltSetVar x, CpltSetOpType o, CpltSetVar y,
           SetRelType r, CpltSetVar z) {
    rel_con_bdd(home, x, o, y, r, z);
  }

  void rel(Space& home, CpltSetVar x, SetOpType o, CpltSetVar y,
           CpltSetRelType r, CpltSetVar z) {
    rel_con_bdd(home, x, o, y, r, z);
  }

  void rel(Space& home, CpltSetVar x, SetOpType o, CpltSetVar y, SetRelType r,
           CpltSetVar z) {
    rel_con_bdd(home, x, o, y, r, z);
  }

  void rel(Space& home, CpltSetVar x, SetRelType r, CpltSetVar y) {
    rel_con(home, x, r, y);
  }

}

// STATISTICS: cpltset-post
