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

#ifndef __GECODE_SUPPORT_STATIC_STACK_HH__
#define __GECODE_SUPPORT_STATIC_STACK_HH__

#include "gecode/kernel.hh"

namespace Gecode { namespace Support {

  /**
   * \brief Stack with fixed number of elements
   *
   * Requires \code #include "gecode/support/static-stack.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T>
  class StaticStack {
  private:
    /// Stack space
    T* stack;
    /// Top of stack
    unsigned int tos;
  public:
    /// Initialize for \a n elements
    StaticStack(const int n);
    /// Release memory
    ~StaticStack(void);

    /// Reset stack (pop all elements)
    void reset(void);
    /// Test whether stack is empty
    bool empty(void) const;

    /// Pop topmost element from stack and return it
    T pop(void);
    /// Return element on top of stack
    T& top(void);
    /// Return element that has just been popped
    T& last(void);
    /// Push element \a x on top of stack
    void push(T x);

  };

  template <class T>
  forceinline
  StaticStack<T>::StaticStack(const int n)
    : tos(0) {
    stack = reinterpret_cast<T*>(Memory::malloc((n+1)*sizeof(T)));
  }

  template <class T>
  forceinline
  StaticStack<T>::~StaticStack(void) {
    Memory::free(stack);
  }

  template <class T>
  forceinline bool
  StaticStack<T>::empty(void) const {
    return tos==0;
  }

  template <class T>
  forceinline void
  StaticStack<T>::reset(void) {
    tos = 0;
  }

  template <class T>
  forceinline T
  StaticStack<T>::pop(void) {
    return stack[--tos];
  }

  template <class T>
  forceinline T&
  StaticStack<T>::top(void) {
    return stack[tos-1];
  }

  template <class T>
  forceinline T&
  StaticStack<T>::last(void) {
    return stack[tos];
  }

  template <class T>
  forceinline void
  StaticStack<T>::push(T x) {
    stack[tos++] = x;
  }

}}

#endif

// STATISTICS: support-any
