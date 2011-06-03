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

  template<class Dim, int n>
  forceinline const Dim&
  Box<Dim,n>::operator [](int i) const {
    assert((i >= 0) && (i < n));
    return d[i];
  }
  template<class Dim, int n>
  forceinline Dim&
  Box<Dim,n>::operator [](int i) {
    assert((i >= 0) && (i < n));
    return d[i];
  }

  template<class Dim, int n>
  forceinline bool
  Box<Dim,n>::mandatory(void) const {
    for (int i=0; i<n; i++)
      if (!d[i].mandatory())
        return false;
    return true;
  }

  template<class Dim, int n>
  forceinline bool
  Box<Dim,n>::empty(void) const {
    for (int i=0; i<n; i++)
      if (d[i].empty())
        return true;
    return false;
  }

  template<class Dim, int n>
  forceinline bool
  Box<Dim,n>::nooverlap(const Box<Dim,n>& box) const {
    for (int i=0; i<n; i++)
      if ((d[i].lec() <= box.d[i].ssc()) || (box.d[i].lec() <= d[i].ssc()))
        return true;
    return false;
  }

  template<class Dim, int n>
  forceinline ExecStatus
  Box<Dim,n>::nooverlap(Space& home, Box<Dim,n>& box) {
    for (int i=0; i<n; i++)
      if ((d[i].sec() <= box.d[i].lsc()) || 
          (box.d[i].sec() <= d[i].lsc())) {
        // Does not overlap for dimension i
        for (int j=i+1; j<n; j++)
          if ((d[j].sec() <= box.d[j].lsc()) || 
              (box.d[j].sec() <= d[j].lsc()))
            return ES_OK;
        // Does not overlap for only dimension i, hence propagate
        d[i].nooverlap(home, box.d[i]);
        box.d[i].nooverlap(home, d[i]);
        return ES_OK;
      }
    // Overlaps in all dimensions
    return ES_FAILED;
  }

  template<class Dim, int n>
  forceinline void
  Box<Dim,n>::update(Space& home, bool share, Box<Dim,n>& b) {
    for (int i=0; i<n; i++)
      d[i].update(home,share,b.d[i]);
  }

  template<class Dim, int n>
  forceinline void
  Box<Dim,n>::subscribe(Space& home, Propagator& p) {
    for (int i=0; i<n; i++)
      d[i].subscribe(home,p);
  }
  template<class Dim, int n>
  forceinline void
  Box<Dim,n>::subscribe(Space& home, Advisor& a) {
    for (int i=0; i<n; i++)
      d[i].subscribe(home,a);
  }
  template<class Dim, int n>
  forceinline void
  Box<Dim,n>::cancel(Space& home, Propagator& p) {
    for (int i=0; i<n; i++)
      d[i].cancel(home,p);
  }
  template<class Dim, int n>
  forceinline void
  Box<Dim,n>::cancel(Space& home, Advisor& a) {
    for (int i=0; i<n; i++)
      d[i].cancel(home,a);
  }

}}}

// STATISTICS: int-prop

