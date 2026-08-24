/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

namespace Gecode { namespace Word { namespace Distinct {

  template<class View>
  forceinline
  Bnd<View>::Bnd(Home home, ViewArray<View>& x0)
    : NaryPropagator<View,PC_WORD_BND>(home,x0) {}

  template<class View>
  forceinline
  Bnd<View>::Bnd(Space& home, Bnd& p)
    : NaryPropagator<View,PC_WORD_BND>(home,p) {}

  template<class View>
  ExecStatus
  Bnd<View>::narrow(Home home, ViewArray<View>& x) {
    const unsigned int n=static_cast<unsigned int>(x.size());
    bool modified;
    do {
      modified=false;
      for (int i=0; i<x.size(); i++) {
        const WordValue lower=x[i].rank_minimum();
        for (int j=0; j<x.size(); j++) {
          const WordValue upper=x[j].rank_maximum();
          if (lower > upper)
            continue;

          const WordValue span=upper-lower;
          const unsigned int capacity =
            (span >= static_cast<WordValue>(n-1U)) ?
            n : static_cast<unsigned int>(span+1U);
          unsigned int inside=0;
          for (int k=0; k<x.size(); k++)
            inside += (x[k].rank_minimum() >= lower) &&
              (x[k].rank_maximum() <= upper);
          if (inside > capacity)
            return ES_FAILED;
          if (inside != capacity)
            continue;

          for (int k=0; k<x.size(); k++) {
            const WordValue minimum=x[k].rank_minimum();
            const WordValue maximum=x[k].rank_maximum();
            if ((minimum >= lower) && (minimum <= upper) &&
                (maximum > upper)) {
              ModEvent me=x[k].narrow_range(home,upper+1U,maximum);
              if (me_failed(me))
                return ES_FAILED;
              modified |= me_modified(me);
            } else if ((maximum >= lower) && (maximum <= upper) &&
                       (minimum < lower)) {
              ModEvent me=x[k].narrow_range(home,minimum,lower-1U);
              if (me_failed(me))
                return ES_FAILED;
              modified |= me_modified(me);
            }
          }
        }
      }
    } while (modified);
    return ES_OK;
  }

  template<class View>
  ExecStatus
  Bnd<View>::post(Home home, ViewArray<View>& x) {
    if (x.size() < 2)
      return ES_OK;
    GECODE_ES_CHECK(narrow(home,x));
    bool assigned=true;
    for (int i=0; assigned && (i<x.size()); i++)
      assigned=x[i].assigned();
    if (!assigned)
      (void) new (home) Bnd(home,x);
    return ES_OK;
  }

  template<class View>
  Actor*
  Bnd<View>::copy(Space& home) {
    return new (home) Bnd(home,*this);
  }

  template<class View>
  PropCost
  Bnd<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::cubic(PropCost::LO,x.size());
  }

  template<class View>
  ExecStatus
  Bnd<View>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x));
    bool assigned=true;
    for (int i=0; assigned && (i<x.size()); i++)
      assigned=x[i].assigned();
    return assigned ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
