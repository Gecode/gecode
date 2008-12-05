/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include <algorithm>

namespace Gecode { namespace Iter { namespace Ranges {

  /**
   * \brief Range iterator for computing intersection (binary)
   *
   * \ingroup FuncIterRanges
   */
  template <class I, class J>
  class Inter : public MinMax {
  private:
    /// Check that \a I is a range iterator;
    IsRangeIter<I> _checkI;
    /// Check that \a J is a range iterator;
    IsRangeIter<J> _checkJ;
  protected:
    /// First iterator
    I i;
    /// Second iterator
    J j;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Inter(void);
    /// Initialize with iterator \a i and \a j
    Inter(I& i, J& j);
    /// Initialize with iterator \a i and \a j
    void init(I& i, J& j);
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}
  };


  /**
   * \brief Range iterator for intersection for any number of iterators
   *
   * \ingroup FuncIterRanges
   */
  template <class I>
  class NaryInter : public MinMax {
  private:
    /// Check that \a I is a range iterator;
    IsRangeIter<I> _checkI;
  protected:
    /// Array of iterators
    I* is;
    /// Number of iterators
    int n;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryInter(void);
    /// Initialize with \a n iterators in \a i
    NaryInter(I* i, int n);
    /// Initialize with \a n iterators in \a i
    void init(I* i, int n);
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}
  };


  /*
   * Binary intersection
   *
   */

  template <class I, class J>
  inline void
  Inter<I,J>::operator ++(void) {
    if (!i() || !j()) goto done;
    do {
      while (i() && (i.max() < j.min())) ++i;
      if (!i()) goto done;
      while (j() && (j.max() < i.min())) ++j;
      if (!j()) goto done;
    } while (i.max() < j.min());
    // Now the intervals overlap: consume the smaller interval
    ma = std::min(i.max(),j.max());
    mi = std::max(i.min(),j.min());
    if (i.max() < j.max()) ++i; else ++j;
    return;
  done:
    finish();
  }

  template <class I, class J>
  forceinline
  Inter<I,J>::Inter(void) {}

  template <class I, class J>
  forceinline
  Inter<I,J>::Inter(I& i0, J& j0)
    : i(i0), j(j0) {
    operator ++();
  }

  template <class I, class J>
  forceinline void
  Inter<I,J>::init(I& i0, J& j0) {
    i = i0; j = j0;
    operator ++();
  }


  /*
   * Nary intersection
   *
   */

  template <class I>
  inline void
  NaryInter<I>::operator ++(void) {
    // The next interval to be returned must have a hole
    // between it and the previously returned range.
    mi = ma+2;
    ma = is[0].max();
    // Intersect with all other intervals
  restart:
    for (int i = n; i--;) {
      // Skip intervals that are too small
      while (is[i]() && (is[i].max() < mi))
        ++is[i];
      if (!is[i]())
        goto done;

      if (is[i].min() > ma) {
        mi=is[i].min();
        ma=is[i].max();
        goto restart;
      }
      // Now the intervals overlap
      if (mi < is[i].min())
        mi = is[i].min();
      if (ma > is[i].max()) {
        ma = is[i].max();
      }
    }
    return;
  done:
    finish();
  }

  template <class I>
  forceinline
  NaryInter<I>::NaryInter(void) {}

  template <class I>
  inline
  NaryInter<I>::NaryInter(I* is0, int n0)
    : is(is0), n(n0) {
    if (!is[0]()) {
      finish();
    } else {
      ma=is[0].min()-2;
      operator ++();
    }
  }

  template <class I>
  inline void
  NaryInter<I>::init(I* is0, int n0) {
    is = is0; n = n0;
    if (!is[0]()) {
      finish();
    } else {
      ma=is[0].min()-2;
      operator ++();
    }
  }

}}}

// STATISTICS: iter-any

