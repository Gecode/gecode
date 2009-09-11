/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *     Guido Tack, 2006
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

#include <gecode/int/gcc.hh>

namespace Gecode {

  void count(Space& home, const IntVarArgs& x,
             const IntVarArgs& c, const IntArgs& v,
             IntConLevel icl) {
    using namespace Int;
    if (v.size() != c.size())
      throw ArgumentSizeMismatch("Int::count");
    if (x.same(home))
      throw ArgumentSame("Int::count");
    if (home.failed())
      return;

    ViewArray<IntView> xv(home, x);
    ViewArray<GCC::CardView> cv(home, c);
    // set the cardinality
    for (int i = v.size(); i--; ) {
      cv[i].card(v[i]);
      cv[i].counter(0);
    }
    switch (icl) {
    case ICL_BND:
      GECODE_ES_FAIL(home, 
                     (GCC::Bnd<GCC::CardView, true>::post(home, xv, cv)));
      break;
    case ICL_DOM:
      GECODE_ES_FAIL(home, 
                     (GCC::Dom<GCC::CardView, true>::post(home, xv, cv)));
      break;
    default:
      GECODE_ES_FAIL(home, 
                     (GCC::Val<GCC::CardView, true>::post(home, xv, cv)));
    }
  }

  // domain is 0..|cards|- 1
  void count(Space& home, const IntVarArgs& x, const IntVarArgs& c,
             IntConLevel icl) {
    IntArgs values(c.size());
    for (int i = c.size(); i--; )
      values[i] = i;
    count(home, x, c, values, icl);
  }

  // constant cards
  void count(Space& home, const IntVarArgs& x,
             const IntSetArgs& c, const IntArgs& v,
             IntConLevel icl) {
    using namespace Int;
    if (v.size() != c.size())
      throw ArgumentSizeMismatch("Int::count");
    if (x.same(home))
      throw ArgumentSame("Int::count");
    for (int i=c.size(); i--; ) {
      Limits::check(v[i],"Int::count");
      Limits::check(c[i].min(),"Int::count");
      Limits::check(c[i].max(),"Int::count");
    }

    if (home.failed())
      return;

    ViewArray<IntView> xv(home, x);

    for (int i = v.size(); i--; ) {
      if (c[i].ranges() > 1) {
        // Found hole, so create temporary variables
        ViewArray<GCC::CardView> cv(home, v.size());
        for (int j = v.size(); j--; ) {
          IntVar card(home, c[j]);
          cv[j] = IntView(card);
          cv[j].card(v[j]);
          cv[j].counter(0);
        }
        switch (icl) {
        case ICL_BND:
          GECODE_ES_FAIL(home, 
                         (GCC::Bnd<GCC::CardView, true>::post(home, xv, cv)));
          break;
        case ICL_DOM:
          GECODE_ES_FAIL(home, 
                         (GCC::Dom<GCC::CardView, true>::post(home, xv, cv)));
          break;
        default:
          GECODE_ES_FAIL(home, 
                         (GCC::Val<GCC::CardView, true>::post(home, xv, cv)));
        }
        return;
      }
    }
    
    // No holes: create OccurBndsViews
    ViewArray<GCC::OccurBndsView> cv(home, c.size());
    for (int i = c.size(  ); i--; ) {
      cv[i].card(v[i]);
      cv[i].counter(0);
      cv[i].min(c[i].min());
      cv[i].max(c[i].max());
    }
    switch (icl) {
    case ICL_BND:
      GECODE_ES_FAIL(home,
        (GCC::Bnd<GCC::OccurBndsView, false>::post(home, xv, cv)));
      break;
    case ICL_DOM:
      GECODE_ES_FAIL(home,
        (GCC::Dom<GCC::OccurBndsView, false>::post(home, xv, cv)));
      break;
    default:
      GECODE_ES_FAIL(home,
        (GCC::Val<GCC::OccurBndsView, false>::post(home, xv, cv)));
    }
  }

  // domain is 0..|cards|- 1
  void count(Space& home, const IntVarArgs& x, const IntSetArgs& c,
             IntConLevel icl) {
    IntArgs values(c.size());
    for (int i = c.size(); i--; )
      values[i] = i;
    count(home, x, c, values, icl);
  }

  void count(Space& home, const IntVarArgs& x,
             const IntSet& c, const IntArgs& v,
             IntConLevel icl) {
    IntSetArgs cards(v.size());
    for (int i = v.size(); i--; )
      cards[i] = c;
    count(home, x, cards, v, icl);
  }

}

// STATISTICS: int-post
