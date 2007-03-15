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
  class SAO {
  public:
    /// Reference count
    unsigned int use;
    /// Elements
    T*  a;
    /// Number of elements
    int n;
    
    /// Allocate for \a n elements
    SAO(int n);
    /// Create copy of elements
    SAO* copy(void) const;
    /// Delete object
    ~SAO(void);

    /// Register new object
    void subscribe(void);
    /// Delete object
    void release(void);
  };

  template <class T>
  forceinline
  SAO<T>::SAO(int n0) : use(1), n(n0) {
    a = (n>0) ? reinterpret_cast<T*>(Memory::malloc(sizeof(T)*n0)) : NULL;
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
  forceinline void
  SAO<T>::subscribe(void) {
    use++;
  }

  template <class T>
  forceinline void
  SAO<T>::release(void) {
    if (--use == 0)
      delete this;
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
  class SharedArray {
  private:
    /// The object referenced
    SAO<T>* sao;
  public:
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
    /// Initialize from shared array \a a (share elements)
    const SharedArray& operator=(const SharedArray&);

    /// Update this array from array \a a (share elements if \a share is true)
    void update(Space* home, bool share, SharedArray& a);

    /// Delete array (elements might be still in use)
    ~SharedArray(void);

    /// Access element at position \a i
    T& operator[](int i);
    /// Access element at position \a i
    const T& operator[](int i) const;

    /// Return number of elements
    int size(void) const;
  };


  template <class T>
  forceinline
  SharedArray<T>::SharedArray(void) : sao(NULL) {}

  template <class T>
  forceinline
  SharedArray<T>::SharedArray(int n) : sao(new SAO<T>(n)) {}

  template <class T>
  forceinline void
  SharedArray<T>::init(int n) {
    assert(sao == NULL);
    sao = new SAO<T>(n);
  }

  template <class T>
  forceinline
  SharedArray<T>::SharedArray(const SharedArray<T>& a) {
    assert(a.sao != NULL);
    sao = a.sao;
    sao->subscribe();
  }

  template <class T>
  forceinline
  SharedArray<T>::~SharedArray(void) {
    assert(sao != NULL);
    if (sao != NULL)
      sao->release();
  }

  template <class T>
  forceinline const SharedArray<T>&
  SharedArray<T>::operator=(const SharedArray<T>& a) {
    if (this != &a) {
      if (sao != NULL)
        sao->release();
      sao = a.sao;
      if (sao != NULL)
        sao->subscribe();
    }
    return *this;
  }

  template <class T>
  inline void
  SharedArray<T>::update(Space* home, bool share, SharedArray<T>& a) {
    assert(a.sao != NULL);
    if (share) {
      sao = a.sao;
      sao->subscribe();
    } else {
      sao = a.sao->copy();
    }
  }

  template <class T>
  forceinline T&
  SharedArray<T>::operator[](int i) {
    assert(sao != NULL);
    return sao->a[i];
  }

  template <class T>
  forceinline const T&
  SharedArray<T>::operator[](int i) const {
    assert(sao != NULL);
    return sao->a[i];
  }

  template <class T>
  forceinline int
  SharedArray<T>::size(void) const {
    assert(sao != NULL);
    return sao->n;
  }

}}

#endif

// STATISTICS: support-any
