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

#include <algorithm>

namespace Gecode { namespace Support {

  /**
   * \brief Array with arbitrary number of elements
   *
   * \ingroup FuncSupport
   */
  template <class T>
  class DynamicArray {
  private:
    /// Size of array
    int n;
    /// Array elements
    T*  x;
    /// Resize to at least \a n + 1 elements
    void resize(int n);
  public:
    /// Initialize with size \a m
    DynamicArray(int m = 32);
    /// Copy elements from array \a a
    DynamicArray(const DynamicArray<T>& a);
    /// Release memory
    ~DynamicArray(void);

    /// Assign array (copy elements from \a a)
    const DynamicArray<T>& operator =(const DynamicArray<T>& a);

    /// Return element at position \a i (possibly resize)
    T& operator[](int i);
    /// Return element at position \a i
    const T& operator [](int) const;

    /// Cast in to pointer of type \a T
    operator T*(void);
  };


  template <class T>
  forceinline
  DynamicArray<T>::DynamicArray(int m)
    : n(m), x(Memory::bmalloc<T>(static_cast<size_t>(n))) {}

  template <class T>
  forceinline
  DynamicArray<T>::DynamicArray(const DynamicArray<T>& a)
    : n(a.n), x(Memory::bmalloc<T>(n)) {
    (void) Memory::bcopy<T>(x,a.x,n);
  }

  template <class T>
  forceinline
  DynamicArray<T>::~DynamicArray(void) {
    Memory::free(x);
  }

  template <class T>
  forceinline const DynamicArray<T>&
  DynamicArray<T>::operator =(const DynamicArray<T>& a) {
    if (this != &a) {
      if (n < a.n) {
        Memory::free(x); n = a.n; x = Memory::bmalloc<T>(n);
      }
      (void) Memory::bcopy(x,a.x,n);
    }
    return *this;
  }

  template <class T>
  void
  DynamicArray<T>::resize(int i) {
    int m = std::max(i+1, (3*n)/2);
    x = Memory::brealloc(x,static_cast<size_t>(n),static_cast<size_t>(m));
    n = m;
  }

  template <class T>
  forceinline T&
  DynamicArray<T>::operator [](int i) {
    if (i >= n) resize(i);
    assert(n > i);
    return x[i];
  }

  template <class T>
  forceinline const T&
  DynamicArray<T>::operator [](int i) const {
    assert(n > i);
    return x[i];
  }

  template <class T>
  forceinline
  DynamicArray<T>::operator T*(void) {
    return x;
  }

}}

// STATISTICS: support-any
