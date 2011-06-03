/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

namespace Gecode { namespace Int { namespace NoOverlap {

  template<int d>
  forceinline Advisor&
  View<d>::advisor(void) {
    Advisors<Advisor> as(c);
    return as.advisor();
  }

  template<int d>
  forceinline
  View<d>::View(Home home, Box<ViewDim,d>* b, int n, int m0)
    : Base<ViewDim,d>(home,b,n), m(m0), c(home), todo(NOTHING) {
    for (int i=n; i--; )
      b[i].subscribe(home,*this);
    Advisor& a = *new (home) Advisor(home,*this,c);
    for (int i=m; i--; )
      b[n+i].subscribe(home,a);
  }

  template<int d>
  ExecStatus
  View<d>::post(Home home, Box<ViewDim,d>* b, int n) {
    // Eliminate zero-sized boxes
    for (int i=n; i--; )
      if (b[i].empty())
        b[i]=b[--n];
    // Partition into mandatory and optional boxes
    if (n > 1) {
      int p = Base<ViewDim,d>::partition(b, 0, n);
      (void) new (home) View<d>(home,b,p,n-p);
    }
    return ES_OK;
  }

  template<int d>
  forceinline size_t 
  View<d>::dispose(Space& home) {
    for (int i=n; i--; )
      b[i].cancel(home,*this);
    Advisor& a = advisor();
    for (int i=m; i--; )
      b[n+i].cancel(home,a);
    c.dispose(home);
    (void) Base<ViewDim,d>::dispose(home);
    return sizeof(*this);
  }


  template<int d>
  forceinline
  View<d>::View(Space& home, bool shared, View<d>& p) 
    : Base<ViewDim,d>(home, shared, p, p.n + p.m), m(p.m), todo(NOTHING) {
    c.update(home,shared,p.c);
  }

  template<int d>
  Actor* 
  View<d>::copy(Space& home, bool share) {
    return new (home) View<d>(home,share,*this);
  }

  template<int d>
  ExecStatus
  View<d>::advise(Space&, Advisor&, const Delta& delta) {
    // Decides whether the propagator must be run
    switch (IntView::modevent(delta)) {
    case ME_INT_VAL:
      todo = ELIMINATE;
      return ES_NOFIX;
    case ME_INT_BND:
      todo = MANDATORY;
      return ES_NOFIX;
    default:
      return ES_FIX;
    }
  }

  template<int d>
  ExecStatus 
  View<d>::propagate(Space& home, const ModEventDelta&) {
    Region r(home);

    if (todo != NOTHING) {
      Advisor& a = advisor();
      // Eliminate empty boxes
      if (todo == ELIMINATE)
        for (int i=m; i--; )
          if (b[n+i].empty()) {
            b[n+i].cancel(home,a);
            b[n+i] = b[n+(--m)];
          }
      // Reconsider mandatory boxes
      if (m > 0) {
        int p = Base<ViewDim,d>::partition(b+n, 0, m);
        for (int i=p; i--; ) {
          b[n+i].cancel(home,a);
          b[n+i].subscribe(home,*this);
        }
        n += p; m -= p;
      }
      todo = NOTHING;
    }

    // Number of disjoint boxes
    int* db = r.alloc<int>(n);
    for (int i=n; i--; )
      db[i] = n-1;

    // Number of boxes to be eliminated
    int e = 0;

    for (int i=n; i--; ) {
      assert(b[i].mandatory());
      for (int j=i; j--; ) 
        if (b[i].nooverlap(b[j])) {
          assert(db[i] > 0); assert(db[j] > 0);
          if (--db[i] == 0) e++;
          if (--db[j] == 0) e++;
          continue;
        } else {
          GECODE_ES_CHECK(b[i].nooverlap(home,b[j]));
        }
    }

    if (m == 0) {
      if (e == n)
        return home.ES_SUBSUMED(*this);
      int i = n-1;
      while (e > 0) {
        // Eliminate boxes that do not overlap
        while (db[i] > 0)
          i--;
        b[i].cancel(home, *this);
        b[i] = b[--n]; b[n] = b[n+m];
        e--; i--;
      }
      if (n < 2)
        return home.ES_SUBSUMED(*this);
    }

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop

