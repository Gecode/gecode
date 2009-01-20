/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Int { namespace Rel {

  template <class View>
  inline
  Lex<View>::Lex(Space& home,
                 ViewArray<View>& x0, ViewArray<View>& y0, bool s)
    : Propagator(home), x(x0), y(y0), strict(s) {
    x.subscribe(home, *this, PC_INT_BND);
    y.subscribe(home, *this, PC_INT_BND);
  }

  template <class View>
  forceinline
  Lex<View>::Lex(Space& home, bool share, Lex<View>& p)
    : Propagator(home,share,p), strict(p.strict) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
  }

  template <class View>
  Actor*
  Lex<View>::copy(Space& home, bool share) {
    return new (home) Lex<View>(home,share,*this);
  }

  template <class View>
  PropCost
  Lex<View>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LOW, x.size());
  }

  template <class View>
  ExecStatus
  Lex<View>::propagate(Space& home, const ModEventDelta&) {
    /*
     * State 1
     *
     */
    {
      int i = 0;
      int n = x.size();

      while ((i < n) && (x[i].min() == y[i].max())) {
        // case: =, >=
        GECODE_ME_CHECK(x[i].lq(home,y[i].max()));
        GECODE_ME_CHECK(y[i].gq(home,x[i].min()));
        i++;
      }

      if (i == n) // case: $
        return strict ? ES_FAILED : ES_SUBSUMED(*this,sizeof(*this));

      // Possible cases left: <, <=, > (yields failure), ?
      GECODE_ME_CHECK(x[i].lq(home,y[i].max()));
      GECODE_ME_CHECK(y[i].gq(home,x[i].min()));

      if (x[i].max() < y[i].min()) // case: < (after tell)
        return ES_SUBSUMED(*this,home);

      // x[i] can never be equal to y[i] (otherwise: >=)
      assert(!(x[i].assigned() && y[i].assigned() &&
               x[i].val() == y[i].val()));
      // Remove all elements between 0...i-1 as they are assigned and equal
      x.drop_fst(i); y.drop_fst(i);
      // After this, execution continues at [1]
    }

    /*
     * State 2
     *   prefix: (?|<=)
     *
     */
    {
      int i = 1;
      int n = x.size();

      while ((i < n) &&
             (x[i].min() == y[i].max()) &&
             (x[i].max() == y[i].min())) { // case: =
        assert(x[i].assigned() && y[i].assigned() &&
               (x[i].val() == y[i].val()));
        i++;
      }

      if (i == n) { // case: $
        if (strict)
          goto rewrite_le;
        else
          goto rewrite_lq;
      }

      if (x[i].max() < y[i].min()) // case: <
        goto rewrite_lq;

      if (x[i].min() > y[i].max()) // case: >
        goto rewrite_le;

      if (i > 1) {
        // Remove equal elements [1...i-1], keep element [0]
        x[i-1]=x[0]; x.drop_fst(i-1);
        y[i-1]=y[0]; y.drop_fst(i-1);
      }
    }

    if (x[1].max() <= y[1].min()) {
      // case: <= (invariant: not =, <)
      /*
       * State 3
       *   prefix: (?|<=),<=
       *
       */
      int i = 2;
      int n = x.size();

      while ((i < n) && (x[i].max() == y[i].min())) // case: <=, =
        i++;

      if (i == n) { // case: $
        if (strict)
          return ES_FIX;
        else
          goto rewrite_lq;
      }

      if (x[i].max() < y[i].min()) // case: <
        goto rewrite_lq;

      if (x[i].min() > y[i].max()) { // case: >
        // Eliminate [i]...[n-1]
        for (int j=i; j<n; j++) {
          x[j].cancel(home,*this,PC_INT_BND);
          y[j].cancel(home,*this,PC_INT_BND);
        }
        x.size(i); y.size(i);
        strict = true;
      }

      return ES_FIX;
    }

    if (x[1].min() >= y[1].max()) {
      // case: >= (invariant: not =, >)
      /*
       * State 4
       *   prefix: (?|<=) >=
       *
       */
      int i = 2;
      int n = x.size();

      while ((i < n) && (x[i].min() == y[i].max()))
        // case: >=, =
        i++;

      if (i == n) { // case: $
        if (strict)
          goto rewrite_le;
        else
          return ES_FIX;
      }

      if (x[i].min() > y[i].max()) // case: >
        goto rewrite_le;

      if (x[i].max() < y[i].min()) { // case: <
        // Eliminate [i]...[n-1]
        for (int j=i; j<n; j++) {
          x[j].cancel(home,*this,PC_INT_BND);
          y[j].cancel(home,*this,PC_INT_BND);
        }
        x.size(i); y.size(i);
        strict = false;
      }

      return ES_FIX;
    }

    return ES_FIX;

  rewrite_le:
    GECODE_REWRITE(*this,Le<View>::post(home,x[0],y[0]));
  rewrite_lq:
    GECODE_REWRITE(*this,Lq<View>::post(home,x[0],y[0]));
  }

  template <class View>
  ExecStatus
  Lex<View>::post(Space& home,
                  ViewArray<View>& x, ViewArray<View>& y, bool strict){
    if (x.size() == 0)
      return strict ? ES_FAILED : ES_OK;
    if (x.size() == 1) {
      if (strict)
        return Le<View>::post(home,x[0],y[0]);
      else
        return Lq<View>::post(home,x[0],y[0]);
    }
    (void) new (home) Lex<View>(home,x,y,strict);
    return ES_OK;
  }

  template <class View>
  size_t
  Lex<View>::dispose(Space& home) {
    assert(!home.failed());
    x.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop
