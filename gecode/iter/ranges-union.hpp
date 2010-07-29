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
   * \brief Range iterator for computing union (binary)
   *
   * \ingroup FuncIterRanges
   */

  template<class I, class J>
  class Union : public MinMax {
  protected:
    /// First iterator
    I i;
    /// Second iterator
    J j;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Union(void);
    /// Initialize with iterator \a i and \a j
    Union(I& i, J& j);
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
   * \brief Range iterator for union for any number of iterators
   * \ingroup FuncIterRanges
   */

  template<class I>
  class NaryUnion : public MinMax {
  protected:
    /// Iterators
    I* i;
    /// Number of still active iterators
    int n;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryUnion(void);
    /// Initialize with \a n iterators in \a i
    NaryUnion(I* i, int n);
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
   * Binary union
   *
   */

  template<class I, class J>
  inline void
  Union<I,J>::operator ++(void) {
    if (!i() && !j()) {
      finish(); return;
    }
    if (!i()) {
      mi = j.min(); ma = j.max(); ++j; return;
    }
    if (!j()) {
      mi = i.min(); ma = i.max(); ++i; return;
    }
    if (i.min() < j.min()) {
      mi = i.min(); ma = i.max(); ++i;
    } else {
      mi = j.min(); ma = j.max(); ++j;
    }
    bool goOn;
    do {
      goOn = false;
      if (i() && (i.min() <= ma+1)) {
        ma = std::max(ma,i.max()); ++i; goOn=true;
      }
      if (j() && (j.min() <= ma+1)) {
        ma = std::max(ma,j.max()); ++j; goOn=true;
      }
    } while (goOn);
  }


  template<class I, class J>
  forceinline
  Union<I,J>::Union(void) {}

  template<class I, class J>
  forceinline
  Union<I,J>::Union(I& i0, J& j0)
    : i(i0), j(j0) {
    operator ++();
  }

  template<class I, class J>
  forceinline void
  Union<I,J>::init(I& i0, J& j0) {
    i = i0; j = j0;
    operator ++();
  }



  /*
   * Nary Union
   *
   */

  template<class I>
  forceinline void
  NaryUnion<I>::operator ++(void) {
    if (n == 0) {
      finish(); return;
    }
    mi = i[0].min();
    ma = i[0].max();
    do {
      if (ma < i[0].max())
        ma = i[0].max();
      ++i[0];
      if (!i[0]()) {
        if (--n == 0)
          return;
        i[0] = i[n];
      }
      int k = 0;
      while ((k << 1) < n) {
        int j = k << 1;
        if ((j < n-1) && (i[j].min() > i[j+1].min()))
          j++;
        if (i[k].min() <= i[j].min())
          break;
        std::swap(i[k],i[j]);
        k = j;
      }
    } while (ma+1 >= i[0].min());
  }


  template<class I>
  forceinline
  NaryUnion<I>::NaryUnion(void) {}

  template<class I>
  void
  NaryUnion<I>::init(I* i0, int n0) {
    i=i0; n=n0;
    while ((n > 0) && !i[0]())
      i[0] = i[--n];
    int j=1;
    while (j < n)
      if (!i[j]())
        i[j] = i[--n];
      else
        for (int k=j++; (k > 0) && (i[k >> 1].min() > i[k].min()); k >>= 1)
          std::swap(i[k],i[k >> 1]);
    operator ++();
  }

  template<class I>
  forceinline
  NaryUnion<I>::NaryUnion(I* i0, int n0) {
    init(i0,n0);
  }

}}}

// STATISTICS: iter-any

