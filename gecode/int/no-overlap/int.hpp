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
  forceinline
  Int<d>::Int(Home home, Box<IntDim,d>* b, int n)
    : Base<IntDim,d>(home, b, n) {
    for (int i=n; i--; )
      b[i].subscribe(home,*this);
  }

  template<int d>
  ExecStatus
  Int<d>::post(Home home, Box<IntDim,d>* b, int n) {
    // Eliminate zero-sized boxes
    for (int i=n; i--; )
      if (b[i].empty())
        b[i]=b[--n];
    if (n > 1)
      (void) new (home) Int<d>(home,b,n);
    return ES_OK;
  }

  template<int d>
  forceinline size_t 
  Int<d>::dispose(Space& home) {
    for (int i=n; i--; )
      b[i].cancel(home,*this);
    (void) Base<IntDim,d>::dispose(home);
    return sizeof(*this);
  }


  template<int d>
  forceinline
  Int<d>::Int(Space& home, bool shared, Int<d>& p) 
    : Base<IntDim,d>(home, shared, p, p.n) {}

  template<int d>
  Actor* 
  Int<d>::copy(Space& home, bool share) {
    return new (home) Int<d>(home,share,*this);
  }

  template<int d>
  ExecStatus 
  Int<d>::propagate(Space& home, const ModEventDelta&) {
    Region r(home);

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

    if (e == n)
      return home.ES_SUBSUMED(*this);

    {
      int i = n-1;
      while (e > 0) {
        // Eliminate boxes that do not overlap
        while (db[i] > 0)
          i--;
        b[i].cancel(home, *this);
        b[i] = b[--n];
        e--; i--;
      }
      if (n < 2)
        return home.ES_SUBSUMED(*this);
    }

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop

