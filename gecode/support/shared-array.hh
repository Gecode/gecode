/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

#ifndef __GECODE_SUPPORT_SHARED_ARRAY_HH__
#define __GECODE_SUPPORT_SHARED_ARRAY_HH__

#include "gecode/kernel.hh"

#include <algorithm>

namespace Gecode { namespace Support {

  /// Implementation of object for shared arrays
  template <class T>
  class SAO : public SharedObject {
  private:
    /// Elements
    T*  a;
    /// Number of elements
    int n;

  public:
    /// Allocate for \a n elements
    SAO(int n);
    /// Create copy of elements
    SAO* copy(void) const;
    /// Delete object
    ~SAO(void);

    /// Access element at position \a i
    T& operator[](int i);
    /// Access element at position \a i
    const T& operator[](int i) const;

    /// Return number of elements
    int size(void) const;
  };

  template <class T>
  forceinline
  SAO<T>::SAO(int n0) : n(n0) {
    a = (n>0) ? static_cast<T*>(Memory::malloc(sizeof(T)*n)) : NULL;
  }

  template <class T>
  forceinline SAO<T>*
  SAO<T>::copy(void) const {
    SAO<T>* o = new SAO<T>(n);
    for (int i=n; i--;)
      new (&(o->a[i])) T(a[i]);
    return o;
  }

  template <class T>
  forceinline
  SAO<T>::~SAO(void) {
    if (n>0) {
      for (int i=n; i--;)
        a[i].~T();
      Memory::free(a);
    }
  }

  template <class T>
  forceinline T&
  SAO<T>::operator[](int i) {
    //    assert((i>=0) && (i<n));
    return a[i];
  }

  template <class T>
  forceinline const T&
  SAO<T>::operator[](int i) const {
    //    assert((i>=0) && (i<n));
    return a[i];
  }

  template <class T>
  forceinline int
  SAO<T>::size(void) const {
    return n;
  }



  /**
   * \brief Shared array with arbitrary number of elements
   *
   * Sharing is implemented by reference counting: the same elements
   * are shared among several objects.
   *
   * Requires \code #include "gecode/support/shared-array.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T>
  class SharedArray : public SharedHandle<SAO<T> > {
  public:
    using SharedHandle<SAO<T> >::object;
    /** 
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized shared array is init and the assignment operator.
     *
     */
    SharedArray(void);
    /// Initialize as array with \a n elements
    SharedArray(int n);
    /**
     * \brief Initialize as array with \a n elements
     *
     * This member function can only be used once and only if the shared
     * array has been constructed with the default constructor.
     *
     */
    void init(int n);
    /// Initialize from shared array \a a (share elements)
    SharedArray(const SharedArray& a);

    /// Access element at position \a i
    T& operator[](int i);
    /// Access element at position \a i
    const T& operator[](int i) const;

    /// Return number of elements
    int size(void) const;
  };


  template <class T>
  forceinline
  SharedArray<T>::SharedArray(void) {}

  template <class T>
  forceinline
  SharedArray<T>::SharedArray(int n) 
    : SharedHandle<SAO<T> >(new SAO<T>(n)) {}

  template <class T>
  forceinline
  SharedArray<T>::SharedArray(const SharedArray<T>& sa) 
    : SharedHandle<SAO<T> >(sa) {}

  template <class T>
  forceinline void
  SharedArray<T>::init(int n) {
    assert(object() == NULL);
    object(new SAO<T>(n));
  }

  template <class T>
  forceinline T&
  SharedArray<T>::operator[](int i) {
    assert(object() != NULL);
    return (*object())[i];
  }

  template <class T>
  forceinline const T&
  SharedArray<T>::operator[](int i) const {
    assert(object() != NULL);
    return (*object())[i];
  }

  template <class T>
  forceinline int
  SharedArray<T>::size(void) const {
    assert(object() != NULL);
    return object()->size();
  }

}}

#endif

// STATISTICS: support-any
