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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
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
  
  /// Maximal stacksize quicksort ever needs
  static const int QuickSortStack_maxsize = 32;
  
  /// Static stack for quicksort
  template <class Type>
  class QuickSortStack {
  private:
    Type** tos;
    Type*  stack[2*QuickSortStack_maxsize];
  public:
    QuickSortStack(void);
    bool empty(void) const;
    void push(Type*, Type*);
    void pop(Type*&, Type*&);
  };
  
  template <class Type>
  forceinline
  QuickSortStack<Type>::QuickSortStack(void) : tos(&stack[0]) {
    *(tos++) = 0;
  }
  
  template <class Type>
  forceinline bool
  QuickSortStack<Type>::empty(void) const {
    return !*(tos-1);
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
  inline void
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
  inline Type*
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
    s.push(l,r);
    while (!s.empty()) {
      s.pop(l,r);
    nopush:
      if (r-l <= QuickSortCutoff)
	continue;
      std::swap(*(l+((r-l) >> 1)),*(r-1));
      exchange(*l,*(r-1),lt);
      exchange(*l,*r,lt);
      exchange(*(r-1),*r,lt);
      Type* i = partition(l+1,r-1,lt);
      if (i-l > r-i) {
	s.push(l,i-1); l=i+1; goto nopush;
      } else {
	s.push(i+1,r); r=i-1; goto nopush;
      }
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
