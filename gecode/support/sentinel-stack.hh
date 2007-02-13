/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2006-10-25 15:21:37 +0200 (Wed, 25 Oct 2006) $ by $Author: tack $
 *     $Revision: 3791 $
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

#ifndef __GECODE_SUPPORT_SENTINEL_STACK_HH__
#define __GECODE_SUPPORT_SENTINEL_STACK_HH__

#include "gecode/kernel.hh"

namespace Gecode { namespace Support {

  /**
   * \brief A simple stack that uses a sentinel element
   *
   * The sentinel element is used to mark the bottom of the stack
   * and must be given at compile time.
   *
   * Note that the stack does not provide memory management. As it
   * is so simple it is ment to be used with automatically allocated
   * memory.
   *
   * Requires \code #include "gecode/support/sentinel-stack.hh" \endcode
   * \ingroup FuncSupport
   */
  template<class T, T sentinel>
  class SentinelStack {
  private:
    T* tos;
  public:
    /// Create stack that uses the memory area \a p
    SentinelStack(T* s);
    /// Check whether stack is empty
    bool empty(void) const;
    /// Return topmost element
    T top(void) const;
    /// Return element that has just been popped
    T last(void) const;
    /// Return topmost element and pop it
    T pop(void);
    /// Push element
    void push(T x);
  };

  template<class T, T sentinel>
  forceinline
  SentinelStack<T,sentinel>::SentinelStack(T* s) 
    : tos(s) {
    *(tos++) = sentinel;
  }
  template<class T, T sentinel>
  forceinline bool
  SentinelStack<T,sentinel>::empty(void) const {
    return *(tos-1) == sentinel;
  }
  template<class T, T sentinel>
  forceinline T
  SentinelStack<T,sentinel>::top(void) const {
    return *(tos-1);
  }
  template<class T, T sentinel>
  forceinline T
  SentinelStack<T,sentinel>::last(void) const {
    return *tos;
  }
  template<class T, T sentinel>
  forceinline T
  SentinelStack<T,sentinel>::pop(void) {
    return *(--tos);
  }
  template<class T, T sentinel>
  forceinline void
  SentinelStack<T,sentinel>::push(T x) {
    assert(x != sentinel);
    *(tos++) = x;
  }

}}

/**
 * \def GECODE_AUTOSTACK(T,S,X,N)
 * \brief Simple sentinel stack using automatic memory
 *
 * Allocates automatic memory for a sentinel stack \A X with 
 * \a N objects of type \a T with sentinel \a S.
 *
 * As soon as the current scope is left, the memory is freed.
 *
 */

#define GECODE_AUTOSTACK(T,S,X,N)                                       \
  GECODE_AUTOARRAY(T,__GECODE__AS__ ## X ## __LINE__,N+1);              \
  Gecode::Support::SentinelStack<T,S> X(__GECODE__AS__ ## X ## __LINE__);

#endif

// STATISTICS: support-any
