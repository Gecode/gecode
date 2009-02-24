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

#include <gecode/cpltset.hh>
#include <gecode/cpltset/propagators.hh>

using namespace Gecode::CpltSet;

namespace Gecode {

  namespace CpltSet { namespace Partition {

    /**
     * \namespace Gecode::CpltSet::Partition
     * \brief Propagators for partition constraints
     */

    template <class View>
    void build_partition(ViewArray<View>& x, bdd& d0) {
      // make it more readable
      typedef Set::LubRanges<View> SetLub;
      namespace IR = Iter::Ranges;

      // disjoint
      int n = x.size();
      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          SetLub lubx(x[i]);
          SetLub luby(x[j]);
          IR::Inter<SetLub, SetLub> inter(lubx, luby);
          IR::ToValues<IR::Inter<SetLub, SetLub> > values(inter);
          ValCache<IR::ToValues<IR::Inter<SetLub, SetLub> > >
            cache(values);

          for (cache.last(); cache(); --cache) {
            int ximin = x[i].initialLubMin();
            int xjmin = x[j].initialLubMin();
            int v = cache.min();
            d0 &= !(x[i].element(v - ximin) & x[j].element(v - xjmin));
          }
        }
      }

      unsigned int xtab = x[0].tableWidth();
      for (int i = n; i--;) {
        if (x[i].tableWidth() > xtab)
          xtab = x[i].tableWidth();
      }

      // just state that the union of all sets is us
      // iff all the variables have the same initial minimum and maximum
      for (unsigned int k = 0; k < xtab; k++) {
        bdd c0 = bdd_false();
        for (int i = 0; i < n; i++) {
          if (k < x[i].tableWidth())
            c0 |= x[i].element(k);
        }
        d0 &= (c0  % bdd_true());
      }
    }

    template <class View0, class View1>
    void build_partition(ViewArray<View0>& x, View1& y, bdd& d0) {
      // make it more readable
      typedef Set::LubRanges<View0> SetLub;
      namespace IR = Iter::Ranges;

      // disjoint
      int n = x.size();
      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          SetLub lubx(x[i]);
          SetLub luby(x[j]);
          IR::Inter<SetLub, SetLub> inter(lubx, luby);
          IR::ToValues<IR::Inter<SetLub, SetLub> > values(inter);
          ValCache<IR::ToValues<IR::Inter<SetLub, SetLub> > >
            cache(values);

          for (cache.last(); cache(); --cache) {
            int ximin = x[i].initialLubMin();
            int xjmin = x[j].initialLubMin();
            int v = cache.min();
            d0 &= !(x[i].element(v - ximin) & x[j].element(v - xjmin));
          }
        }
      }

      unsigned int ytab = y.tableWidth();
      int ymin = y.initialLubMin();
      for (unsigned int k = 0; k < ytab; k++) {
        bdd c0 = bdd_false();
        for (int i = 0; i < n; i++) {
          int xmin = x[i].initialLubMin();
          int xmax = x[i].initialLubMax();
          int shift = std::max(ymin, xmin) - std::min(xmin, ymin);
          if (xmin <= ymin + (int) k && ymin + (int) k <= xmax) {
            c0 |= x[i].element(k - shift);
          }
        }
        d0 &= (c0  % y.element(k));
      }
    }

    template <class View>
    void build_lexorder(ViewArray<View>& x, bdd& d0, CpltSetRelType lex) {

      int n = x.size();
      unsigned int xtab = x[0].tableWidth();

      for (int i = n; i--;)
        if (x[i].tableWidth() > xtab)
          xtab = x[i].tableWidth();

      // std::cerr << "with lex \n";
      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          unsigned int xai_off = x[i].offset();
          unsigned int xaj_off = x[j].offset();
          switch (lex) {
          case SRT_LE:
            {
              d0 &= lexlt(xai_off, xaj_off, xtab, xtab - 1);
              break;
            }
          case SRT_GR:
            {
              d0 &= lexlt(xaj_off, xai_off, xtab, xtab - 1);
              break;
            }
          case SRT_LQ:
            {
              d0 &= lexlq(xai_off, xaj_off, xtab, xtab - 1);
              break;
            }
          case SRT_GQ:
            {
              d0 &= lexlq(xaj_off, xai_off, xtab, xtab - 1);
              break;
            }
          case SRT_LE_REV:
            {
              d0 &= lexltrev(xai_off, xaj_off, xtab, 0);
              break;
            }
          case SRT_GR_REV:
            {
              d0 &= lexltrev(xaj_off, xai_off, xtab, 0);
              break;
            }
          case SRT_LQ_REV:
            {
              d0 &= lexlqrev(xai_off, xaj_off, xtab, 0);
              break;
            }
          case SRT_GQ_REV:
            {
              d0 &= lexlqrev(xaj_off, xai_off, xtab, 0);
              break;
            }
          default:
            {
              throw
                CpltSet::InvalidRelation("partition rel not implemented");
              break;
            }
          }
        }
      }
    }

    template <class View>
    void partition_post(Space& home, ViewArray<View>& x, bool withlex,
                        CpltSetRelType lex, bool withcard, int d) {

      if (home.failed()) return;

      int n = x.size();

      bdd d0 = bdd_true();
      build_partition(x, d0);

      // forall i: x_{i - 1} \prec_{lex_{bit}} x_i
      if (withlex)
        build_lexorder(x, d0, lex);

      if (withcard) {
        for (int i = n; i--; ) {
          unsigned int off   = x[i].offset();
          unsigned int range = x[i].tableWidth();
          d0 &= cardcheck(home, range, off, d, d);
        }
      }

      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }

    template <class View>
    void partition_post(Space& home, ViewArray<View>& x, View& y,
                        bool, SetRelType, bool, int) {
      if (home.failed()) return;

      int n = x.size();

      bdd d0 = bdd_true();
      build_partition(x, y, d0);

      ViewArray<View> naryone(home, x.size() + 1);
      for (int i = 0; i < n; i++)
        naryone[i] = x[i];
      naryone[n] = y;

      GECODE_ES_FAIL(home,
        NaryCpltSetPropagator<View>::post(home, naryone, d0));
    }

    template <class View0, class View1>
    void partition_post(Space& home, ViewArray<View0>& x, View1& y,
                        bool withlex, SetRelType lex,
                        bool withcard, int d) {
      if (home.failed()) return;

      bdd d0 = bdd_true();
      build_partition(x, y, d0);

      GECODE_ES_FAIL(home,
        (NaryOneCpltSetPropagator<View0, View1>::post(home, x, y, d0)));
    }

    template <class View>
    void partition_post(Space& home, ViewArray<View>& x, bool,
                        SetRelType, bool withcard, int d) {
      if (home.failed()) return;

      int n = x.size();

      int minx = x[0].initialLubMin();
      int maxx = x[0].initialLubMax();
      unsigned int xtab = x[0].tableWidth();
      for (int i = n; i--;) {
        if (x[i].initialLubMin() < minx) {
          minx = x[i].initialLubMin();
        }
        if (x[i].initialLubMax() > maxx) {
          maxx = x[i].initialLubMax();
        }
        if (x[i].tableWidth() > xtab) {
          xtab = x[i].tableWidth();
        }
      }

      // build partition
      bdd d0 = bdd_true();

      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          Set::LubRanges<View> lubx(x[i]);
          Set::LubRanges<View> luby(x[j]);
          Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
            Set::LubRanges<View> > inter(lubx, luby);
          Gecode::Iter::Ranges::ToValues<
            Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
              Set::LubRanges<View> > > values(inter);
          ValCache<
            Gecode::Iter::Ranges::ToValues<
                Gecode::Iter::Ranges::Inter<Set::LubRanges<View>,
                  Set::LubRanges<View> > > > cache(values);
          cache.last();
          for (; cache(); --cache) {
            int ximin = x[i].initialLubMin();
            int xjmin = x[j].initialLubMin();
            int v = cache.min();
            d0 &= !(x[i].element(v - ximin) & x[j].element(v - xjmin));
          }

        }
      }

      // no lex ordering

      // just state that the union of all sets is us
      for (unsigned int k = 0; k < xtab; k++) {
        bdd c0 = bdd_false();
        for (int i = 0; i < n; i++) {
          if (k < x[i].tableWidth()) {
            c0 |= x[i].element(k);
          }
        }
        d0 &= (c0  % bdd_true());
      }

      if (withcard) {
        for (int i = n; i--; ) {
          unsigned int off   = x[i].offset();
          unsigned int range = x[i].tableWidth();
          d0 &= cardcheck(home, range, off, d, d);
        }
      }

      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }

    template <class Rel>
    forceinline void
    partition_con(Space& home, const CpltSetVarArgs& x, bool withlex, Rel lex,
                  bool withcard, int d) {
      ViewArray<CpltSetView> bv(home, x);
      partition_post(home, bv, withlex, lex, withcard, d);
    }

    // For testing purposes only supported for bddviews
    template <class Rel>
    forceinline void
    partition_con(Space& home, const CpltSetVarArgs& x, const CpltSetVar& y,
                  bool withlex, Rel lex, bool withcard, int d) {
      ViewArray<CpltSetView> bv(home, x);
      CpltSetView yv(y);
      partition_post(home, bv, yv, withlex, lex, withcard, d);
    }

  }} // end namespace CpltSet::Partition

  using namespace CpltSet::Partition;

  void partition(Space& home, const CpltSetVarArgs& x) {
    partition_con(home, x, false, SRT_EQ, false, -1);
  }

  void partition(Space& home, const CpltSetVarArgs& x, const CpltSetVar& y) {
    partition_con(home, x, y, false, SRT_EQ, false, -1);
  }

  void partitionLex(Space& home, const CpltSetVarArgs& x,
                    CpltSetRelType lex) {
    partition_con(home, x, true, lex, false, -1);
  }

  void partitionLexCard(Space& home, const CpltSetVarArgs& x,
                        CpltSetRelType lex, unsigned int c) {
    Set::Limits::check(c, "CpltSet::partitionLexCard");
    partition_con(home, x, true, lex, true, c);
  }

  void partitionCard(Space& home, const CpltSetVarArgs& x, unsigned int c) {
    Set::Limits::check(c, "CpltSet::partitionCard");
    partition_con(home, x, false, SRT_EQ, true, c);
  }
}

// STATISTICS: cpltset-post
