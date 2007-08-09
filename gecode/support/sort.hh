/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#ifndef __GECODE_SUPPORT_SORT_HH__
#define __GECODE_SUPPORT_SORT_HH__

#include <algorithm>

namespace Gecode { namespace Support {

  /// Exchange elements according to order
  template <class Type, class LessThan>
  forceinline void
  exchange(Type &a, Type &b, LessThan &lt) {
    if (lt(b,a)) std::swap(a,b);
  }

  /// Perform quicksort only for more elements
  int const QuickSortCutoff = 20;

  /// Static stack for quicksort
  template <class Type>
  class QuickSortStack {
  private:
    /// Maximal stacksize quicksort ever needs
    static const int maxsize = 32;
    /// Top of stack
    Type** tos;
    /// Stack entries (terminated by NULL entry)
    Type*  stack[2*maxsize+1];
  public:
    QuickSortStack(void);
    bool empty(void) const;
    void push(Type*, Type*);
    void pop(Type*&, Type*&);
  };

  template <class Type>
  forceinline
  QuickSortStack<Type>::QuickSortStack(void) : tos(&stack[0]) {
    *(tos++) = NULL;
  }

  template <class Type>
  forceinline bool
  QuickSortStack<Type>::empty(void) const {
    return *(tos-1) == NULL;
  }

  template <class Type>
  forceinline void
  QuickSortStack<Type>::push(Type* l, Type* r) {
    *(tos++) = l; *(tos++) = r;
  }

  template <class Type>
  forceinline void
  QuickSortStack<Type>::pop(Type*& l, Type*& r) {
    r = *(--tos); l = *(--tos);
  }

  /// Standard insertion sort
  template <class Type, class LessThan>
  forceinline void
  insertion(Type* l, Type* r, LessThan &lt) {
    for (Type* i = r; i > l; i--)
      exchange(*(i-1),*i,lt);
    for (Type* i = l+2; i <= r; i++) {
      Type* j = i;
      Type v = *i;
      while (lt(v,*(j-1))) {
        *j = *(j-1); j--;
      }
      *j = v;
    }
  }

  /// Standard partioning
  template <class Type, class LessThan>
  forceinline Type*
  partition(Type* l, Type* r, LessThan &lt) {
    Type* i = l-1;
    Type* j = r;
    Type v = *r;
    while (true) {
      while (lt(*(++i),v));
      while (lt(v,*(--j))) if (j == l) break;
        if (i >= j) break;
        std::swap(*i,*j);
    }
    std::swap(*i,*r);
    return i;
  }

  /// Standard quick sort
  template <class Type, class LessThan>
  inline void
  quicksort(Type* l, Type* r, LessThan &lt) {
    QuickSortStack<Type> s;
    while (true) {
      std::swap(*(l+((r-l) >> 1)),*(r-1));
      exchange(*l,*(r-1),lt);
      exchange(*l,*r,lt);
      exchange(*(r-1),*r,lt);
      Type* i = partition(l+1,r-1,lt);
      if (i-l > r-i) {
        if (r-i > QuickSortCutoff) {
          s.push(l,i-1); l=i+1; continue;
        }
        if (i-l > QuickSortCutoff) {
          r=i-1; continue;
        }
      } else {
        if (i-l > QuickSortCutoff) {
          s.push(i+1,r); r=i-1; continue;
        }
        if (r-i > QuickSortCutoff) {
          l=i+1; continue;
        }
      }
      if (s.empty())
        break;
      s.pop(l,r);
    }
  }

  /*
   * These are the routines to be used
   *
   */

  /**
   * \brief Insertion sort
   *
   * Sorts by insertion the \a n first elements of array \a x according
   * to the order \a lt as instance of class \a LessThan. The class
   * \a LessThan must implement the single member function
   * \code bool operator()(const Type&, const Type&) \endcode
   * for comparing elements.
   *
   * The algorithm is largely based on the following book:
   * Robert Sedgewick, Algorithms in C++, 3rd edition, 1998, Addison Wesley.
   *
   * Requires \code #include "gecode/support/sort.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class Type, class LessThan>
  forceinline void
  insertion(Type* x, int n, LessThan &lt) {
    if (n < 2)
      return;
    insertion(x,x+n-1,lt);
  }

  /**
   * \brief Quicksort
   *
   * Sorts with quicksort the \a n first elements of array \a x according
   * to the order \a lt as instance of class \a LessThan. The class
   * \a LessThan must implement the single member function
   * \code bool operator()(const Type&, const Type&) \endcode
   * for comparing elements.
   *
   * The algorithm is largely based on the following book:
   * Robert Sedgewick, Algorithms in C++, 3rd edition, 1998, Addison Wesley.
   *
   * Requires \code #include "gecode/support/sort.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class Type, class LessThan>
  forceinline void
  quicksort(Type* x, int n, LessThan &lt) {
    if (n < 2)
      return;
    if (n > QuickSortCutoff)
      quicksort(x,x+n-1,lt);
    insertion(x,x+n-1,lt);
  }

}}

#endif

// STATISTICS: support-any
