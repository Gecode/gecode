/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode { namespace Int { namespace Arithmetic {

  template<class IntType>
  forceinline
  bool even(IntType n) {
    return (n % 2) == 0;
  }

  template<class IntType>
  inline
  IntType pow(IntType x, int n) {
    assert(n > 0);
    IntType p = 1;
    do {
      if (even(n)) {
        x *= x; n >>= 1;
      } else {
        p *= x; n--;
      }
    } while (n > 0);
    return p;
  }

  forceinline
  int tpow(int _x, int n) {
    assert(n > 0);
    long long int p = 1;
    long long int x = _x;
    do {
      if (even(n)) {
        x *= x; n >>= 1;
      } else {
        p *= x; n--;
      }
      if (p > Limits::max)
        return Limits::max+1;
    } while (n > 0);
    return static_cast<int>(p);
  }

  /// Test whether \f$r^n>x\f$
  forceinline
  bool powgr(int r, int n, int x) {
    assert((r >= 0) && (n > 0));
    unsigned long long int y = static_cast<unsigned long long int>(r);
    unsigned long long int p = static_cast<unsigned long long int>(1);
    do {
      if (even(n)) {
        y *= y; n >>= 1;
        if (y > x)
          return true;
      } else {
        p *= y; n--;
        if (p > x)
          return true;
      }
    } while (n > 0);
    assert(y <= x);
    return false;
  }

  inline
  int fnroot(int x, int n) {
    if (x < 2)
      return x;
    /*
     * We look for l such that: l^n <= x < (l+1)^n
     */
    int l = 1;
    int u = x;
    do {
      int m = (l + u) >> 1;
      if (powgr(m,n,x)) u=m; else l=m;
    } while (l+1 < u);
    assert((pow(static_cast<long long int>(l),n) <= x) && 
           (x < pow(static_cast<long long int>(l+1),n)));
    return l;
  }

  /// Test whether \f$r^n<x\f$
  forceinline
  bool powle(int r, int n, int x) {
    assert((r >= 0) && (n > 0));
    unsigned long long int y = static_cast<unsigned long long int>(r);
    unsigned long long int p = static_cast<unsigned long long int>(1);
    do {
      if (even(n)) {
        y *= y; n >>= 1;
        if (y >= x)
          return false;
      } else {
        p *= y; n--;
        if (p >= x)
          return false;
      }
    } while (n > 0);
    assert(y < x);
    return true;
  }

  /// Return \f$\lceil \sqrt[n]{x}\rceil\f$ where \a x must be non-negative and \f$n>0\f$
  inline
  int cnroot(int x, int n) {
    if (x < 2)
      return x;
    /*
     * We look for u such that: (u-1)^n < x <= u^n
     */
    int l = 1;
    int u = x;
    do {
      int m = (l + u) >> 1;
      if (powle(m,n,x)) l=m; else u=m;
    } while (l+1 < u);
    assert((pow(static_cast<long long int>(u-1),n) < x) && 
           (x <= pow(static_cast<long long int>(u),n)));
    return u;
  }

}}}

// STATISTICS: int-other

