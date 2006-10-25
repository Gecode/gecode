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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SUPPORT_DYNAMIC_STACK_HH__
#define __GECODE_SUPPORT_DYNAMIC_STACK_HH__

#include "gecode/kernel.hh"

namespace Gecode { namespace Support {

  /**
   * \brief Stack with arbitrary number of elements
   *
   * Requires \code #include "gecode/support/dynamic-stack.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T>
  class DynamicStack {
  private:
    /// Current size of the stack
    int limit;
    /// Top of stack
    int tos;
    /// Elements on stack
    T*  stack;
    /// Resize stack (increase size)
    void resize(void);
  public:
    /// Initialize stack with \a n elements
    DynamicStack(int n=64);
    /// Release memory
    ~DynamicStack(void);

    /// Test whether stack is empty
    bool empty(void) const;
    /// Pop topmost element from stack and return it
    T pop(void);
    /// Return top of stack
    T& top(void);
    /// Push element on stack
    void push(T);


    /// Return number of entries currently on stack
    int entries(void) const;
    /** \brief Return entry at position \a i
     *
     * Position 0 corresponds to the element first pushed,
     * whereas position \c entries()-1 corresponds to the
     * element pushed last.
     */
    T& operator[](int i);
    /** \brief Return entry at position \a i
     *
     * Position 0 corresponds to the element first pushed,
     * whereas position \c entries()-1 corresponds to the
     * element pushed last.
     */
    const T& operator [] (int i) const;

    /// Return size of stack
    size_t size(void) const;
  };


  template <class T>
  void
  DynamicStack<T>::resize(void) {
    int nl = (limit * 3) / 2;
    stack = Memory::brealloc<T>(stack,limit,nl);
    limit = nl;
  }

  template <class T>
  forceinline
  DynamicStack<T>::DynamicStack(int n)
    : limit(n), tos(0), stack(Memory::bmalloc<T>(n)) {}

  template <class T>
  forceinline
  DynamicStack<T>::~DynamicStack(void) {
    Memory::free(stack);
  }

  template <class T>
  forceinline T
  DynamicStack<T>::pop(void) {
    return stack[--tos];
  }

  template <class T>
  forceinline T&
  DynamicStack<T>::top(void) {
    return stack[tos-1];
  }

  template <class T>
  forceinline void
  DynamicStack<T>::push(T x) {
    stack[tos++] = x;
    if (tos==limit)
      resize();
  }

  template <class T>
  forceinline bool
  DynamicStack<T>::empty(void) const {
    return tos==0;
  }

  template <class T>
  forceinline int
  DynamicStack<T>::entries(void) const {
    return tos;
  }

  template <class T>
  forceinline T&
  DynamicStack<T>::operator [] (int i) {
    return stack[i];
  }

  template <class T>
  forceinline const T&
  DynamicStack<T>::operator [] (int i) const {
    return stack[i];
  }

  template <class T>
  forceinline size_t
  DynamicStack<T>::size(void) const {
    return (limit * sizeof(T)) + sizeof(DynamicStack<T>);
  }

}}

#endif

// STATISTICS: support-any
