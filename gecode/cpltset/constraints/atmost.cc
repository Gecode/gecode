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

#include "gecode/cpltset/propagators.hh"

using namespace Gecode::CpltSet;

namespace Gecode {
  
  namespace CpltSet { namespace AtMost {
    
    /**
     * \namespace Gecode::CpltSet::AtMost
     * \brief Propagators for intersection constraints with cardinality 
     * restrictions
     */
    
    template <class View>
    void 
    atmostOne_post(Space* home, ViewArray<View>& x, int c) {
      if (home->failed()) return;
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

      bdd d0 = bdd_true();     
      for (int i = 0; i < n; i++) {
        unsigned int xoff = x[i].offset();
        unsigned int xtab = x[i].tableWidth();
        d0 &= cardcheck(xtab, xoff, c, c);    
      }

      for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
          d0 &= extcardcheck(x[i], x[j], 0, 1);
        }
      }
      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }

    template <class View>
    void 
    atmost_post(Space* home, ViewArray<View>& x, int c, 
                SetRelType, int) {
      if (home->failed()) return;

      unsigned int x1_tab = x[1].tableWidth();

      // cardinality description for the intersection x \cap y
      bdd d0 = bdd_true();

      // equivalence of intersection x \cap y with intermediate variable z
      for (unsigned int i = x1_tab; i--;) {
        d0 &= ((x[0].element(i) & x[1].element(i)) % (x[2].element(i)));
      }

      // cardinality description of intermediate variable z
      bdd c0 = bdd_true();
      unsigned int off = x[2].offset();
      unsigned int tab = x[2].tableWidth();

      c0 = cardcheck(tab, off, 0, c);

      // combination of intersection and cardinality
      d0 &= c0;

      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));

    }

    template <class View>
    void 
    atmost_post(Space* home, ViewArray<View>& x, int c, 
                CpltSetRelType lex, int card) {
      if (home->failed()) return;

      unsigned int x1_tab = x[1].tableWidth();

      // cardinality description for the intersection x \cap y
      bdd d0 = bdd_true();

      // equivalence of intersection x \cap y with intermediate variable z
      for (unsigned int i = x1_tab; i--;) {
        d0 &= ((x[0].element(i) & x[1].element(i)) % (x[2].element(i)));
      }

      // cardinality description of intermediate variable z
      bdd c0 = bdd_true();
      unsigned int off = x[2].offset();
      unsigned int tab = x[2].tableWidth();

      c0 = cardcheck(tab, off, 0, c);

      // combination of intersection and cardinality
      d0 &= c0;

      // lexicographic constraint x[0] < x[1]

      unsigned int xoff = x[0].offset();
      unsigned int yoff = x[1].offset();
      unsigned int xtab = x[0].tableWidth();
      unsigned int ytab = x[1].tableWidth();
      switch (lex) {
      case SRT_LE:
        {
          d0 &= lexlt(xoff, yoff, xtab, xtab - 1);
          break;
        }
      case SRT_GR:
        {
          d0 &= lexlt(yoff, xoff, xtab, xtab - 1);
          break;
        }
      case SRT_LQ:
        {
          d0 &= lexlq(xoff, yoff, xtab, xtab - 1);
          break;
        }
      case SRT_GQ:
        {
          d0 &= lexlq(yoff, xoff, xtab, xtab - 1);
          break;
        }
      case SRT_LE_REV:
        {
          d0 &= lexltrev(xoff, yoff, xtab, 0);
          break;
        }
      case SRT_GR_REV:
        {
          d0 &= lexltrev(yoff, xoff, xtab, 0);
          break;
        }
      case SRT_LQ_REV:
        {
          d0 &= lexlqrev(xoff, yoff, xtab, 0);
          break;
        }
      case SRT_GQ_REV:
        {
          d0 &= lexlqrev(yoff, xoff, xtab, 0);
          break;
        }
      default:
        {
          // dont use additional lexicographic ordering
          break;
        }
      }


      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));

    }

    template <class View>
    void 
    atmost_post(Space* home, View& x, View& y, int c, CpltSetRelType lex,
                int card) {
      if (home->failed()) return;

      unsigned int xoff = x.offset();
      unsigned int yoff = y.offset();
      unsigned int xtab = x.tableWidth();
      unsigned int ytab = y.tableWidth();

      // cardinality description for the intersection x \cap y
      bdd d0 = bdd_true();
      d0 = extcardcheck(x, y, 0, c);

      // extra lexicographic order on bit strings x \sim_{lex} y
      switch (lex) {
      case SRT_LE:
        {
          d0 &= lexlt(xoff, yoff, xtab, xtab - 1);
          break;
        }
      case SRT_GR:
        {
          d0 &= lexlt(yoff, xoff, xtab, xtab - 1);
          break;
        }
      case SRT_LQ:
        {
          d0 &= lexlq(xoff, yoff, xtab, xtab - 1);
          break;
        }
      case SRT_GQ:
        {
          d0 &= lexlq(yoff, xoff, xtab, xtab - 1);
          break;
        }
      case SRT_LE_REV:
        {
          d0 &= lexltrev(xoff, yoff, xtab, 0);
          break;
        }
      case SRT_GR_REV:
        {
          d0 &= lexltrev(yoff, xoff, xtab, 0);
          break;
        }
      case SRT_LQ_REV:
        {
          d0 &= lexlqrev(xoff, yoff, xtab, 0);
          break;
        }
      case SRT_GQ_REV:
        {
          d0 &= lexlqrev(yoff, xoff, xtab, 0);
          break;
        }
      default:
        {
          // dont use additional lexicographic ordering
          break;
        }
      }

      // extra cardinality information on x and y
      if (card > -1) {
        d0 &= cardcheck(xtab, xoff, card, card);
        d0 &= cardcheck(ytab, yoff, card, card);
      }

      if (x.assigned()) {
        d0 &= x.dom();
      }

      if (y.assigned()) {
        d0 &= y.dom();
      }
      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View,View>::post(home, x, y, d0)));
    }

    template <class View>
    void 
    atmost_post(Space* home, View& x, View& y, int c, SetRelType,
                int card) {
      if (home->failed()) return;

      unsigned int xoff = x.offset();
      unsigned int yoff = y.offset();
      unsigned int xtab = x.tableWidth();
      unsigned int ytab = y.tableWidth();

      // cardinality description for the intersection x \cap y
      bdd d0 = bdd_true();
      d0 = extcardcheck(x, y, 0, c);

      // dont use additional lexicographic ordering

      // extra cardinality information on x and y
      if (card > -1) {
        d0 &= cardcheck(xtab, xoff, card, card);
        d0 &= cardcheck(ytab, yoff, card, card);
      }

      if (x.assigned()) {
        d0 &= x.dom();
      }

      if (y.assigned()) {
        d0 &= y.dom();
      }
      GECODE_ES_FAIL(home,
        (BinaryCpltSetPropagator<View,View>::post(home, x, y, d0)));
    }

    template <class Rel>
    forceinline void 
    atmost_con(Space* home, const CpltSetVar& x, const CpltSetVar& y, int c, 
               Rel lex, int card) {
      CpltSetView xv(x);
      CpltSetView yv(y);
      atmost_post(home, xv, yv, c, lex, card);
    }

    template <class Rel>
    forceinline void 
    atmost_con(Space* home, const CpltSetVar& x, const CpltSetVar& y,
               const CpltSetVar& z, int c, Rel lex, int card) {
      ViewArray<CpltSetView> bv(home, 3);
      bv[0] = x;
      bv[1] = y;
      bv[2] = z;
      atmost_post(home, bv, c, lex, card);
    }


    forceinline void 
    atmostOne_con(Space* home, const CpltSetVarArgs& x, int c) {
      int n = x.size();
      ViewArray<CpltSetView> bv(home, n);
      for (int i = 0; i < n; i++) {
        bv[i] = x[i];
      }
      atmostOne_post(home, bv, c);
    }
  
    
  }} // end namespace CpltSet::AtMost

  using namespace CpltSet::AtMost;

  void
  exactly(Space* home, CpltSetVar x, IntSet& is, unsigned int c) {
    if (home->failed()) return;
    Set::Limits::check(c, "CpltSet::exactly");

    ViewArray<CpltSetView> bv(home, 1);
    bv[0] = x;
    // Subsumption check
    CpltSetVarGlbRanges glb(x);
    if (glb()) {
      IntSetRanges ir(is);
      Gecode::Iter::Ranges::Inter<CpltSetVarGlbRanges, IntSetRanges> 
        inter(glb, ir);
      if (inter()) {
        int s = inter.width();
        ++inter;
        if (!inter() && s == 1) {
          return;
        } else {
          home->fail();
          return;
        }
      }
    }

    CpltSetVarUnknownRanges delta(x);
    IntSetRanges irange(is);
    Gecode::Iter::Ranges::Inter<CpltSetVarUnknownRanges, IntSetRanges> 
      interdel(delta, irange);
    if (!interdel()) {
      home->fail();
      return;
    } 

    int mi = interdel.min();
    int ma = interdel.max();
    int s = interdel.width();
    ++interdel;
    if (!interdel() && s == 1) {
      GECODE_ME_FAIL(home, bv[0].include(home, mi));
      return;
    }

    Iter::Ranges::SingletonAppend<
      Gecode::Iter::Ranges::Inter<CpltSetVarUnknownRanges, IntSetRanges>
      > si(mi,ma,interdel);

    unsigned int xtab = bv[0].tableWidth();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].initialLubMin();
    
    bdd d = cardConst(xtab, xoff, xmin, c, c, si);
    GECODE_ES_FAIL(home,
      UnaryCpltSetPropagator<CpltSetView>::post(home, bv[0], d));
  }

  void
  atmost(Space* home, CpltSetVar x, IntSet& is, unsigned int c) {
    Set::Limits::check(c, "CpltSet::atmost");
    if (home->failed()) return;
    ViewArray<CpltSetView> bv(home, 1);
    bv[0] = x;

    unsigned int xtab = bv[0].tableWidth();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].initialLubMin();
    IntSetRanges ir(is);
    bdd d = cardConst(xtab, xoff, xmin, 0, c, ir);

    GECODE_ES_FAIL(home,
      UnaryCpltSetPropagator<CpltSetView>::post(home, bv[0], d));
  }

  void 
  atmost(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c) {
    Set::Limits::check(c, "CpltSet::atmost");
    atmost_con(home, x, y, c, SRT_EQ, -1);
  }

  void 
  atmostLex(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c, 
            CpltSetRelType lex) {
    Set::Limits::check(c, "CpltSet::atmostLex");
    atmost_con(home, x, y, c, lex, -1);
  }

  void 
  atmostLexCard(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c, 
                CpltSetRelType lex, unsigned int d) {
    Set::Limits::check(c, "CpltSet::atmostLexCard");
    Set::Limits::check(d, "CpltSet::atmostLexCard");
    atmost_con(home, x, y, c, lex, d);
  }

  void 
  atmostCard(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c,
             unsigned int d) {
    Set::Limits::check(c, "CpltSet::atmostCard");
    Set::Limits::check(d, "CpltSet::atmostCard");
    atmost_con(home, x, y, c, SRT_EQ, d);
  }

  void 
  atmost(Space* home, CpltSetVar x, CpltSetVar y, CpltSetVar z,
         unsigned int c) {
    Set::Limits::check(c, "CpltSet::atmost");
    atmost_con(home, x, y, z, c, SRT_EQ, -1);
  }

  void 
  atmostOne(Space* home, const CpltSetVarArgs& x, unsigned int c) {
    Set::Limits::check(c, "CpltSet::atmostOne");
    atmostOne_con(home, x, c);
  }


}

// STATISTICS: cpltset-post
