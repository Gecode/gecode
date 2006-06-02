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

  /**
   * \brief Shared array with arbitrary number of elements
   *
   * Sharing is implemented by reference counting: the same elements
   * are shared among several objects.
   *
   * If the template parameter \a fd is true, the destructors of all objects
   * in the array will be called when the array is deallocated, i.e. when
   * its reference count drops to zero.
   *
   * Requires \code #include "gecode/support/shared-array.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class T, bool fd=false>
  class SharedArray {
  private:
    /// Information for array elements
    class Object {
    public:
      /// Reference count
      unsigned int use;
      /// Number of elements
      int n;
      /// Start of elements
      T   a[1];
      
      /// Allocate new information for \a n elements
      static Object* allocate(int n);
      /// Create copy of elements
      Object* copy(void) const;
      
      /// Register new object
      void subscribe(void);
      /// Delete object
      void release(void);
    };
    /// The object referenced
    Object* sao;
  public:
    /// Initialize as empty array
    SharedArray(void);
    /// Initialize as array with \a n elements
    SharedArray(int n);
    /// Initialize from shared array \a a (share elements)
    SharedArray(const SharedArray<T,fd>& a);
    /// Initialize from shared array \a a (share elements)
    const SharedArray& operator=(const SharedArray&);

    /// Update this array from array \a a (share elements if \a share is true)
    void update(bool share, SharedArray& a);

    /// Delete array (elements might be still in use)
    ~SharedArray(void);

    /// Access element at position \a i
    T& operator[](int i);
    /// Access element at position \a i
    const T& operator[](int i) const;

    /// Return number of elements
    int size(void) const;
    /// Change size to \a n
    void size(int n);

    /// Shrink array to \a n elements
    void shrink(int n);
    /// Ensure that array has at least \a n elements
    void ensure(int n);
  };


  template <class T, bool fd>
  forceinline typename SharedArray<T,fd>::Object*
  SharedArray<T,fd>::Object::allocate(int n) {
    assert(n>0);
    Object* o = reinterpret_cast<Object*>
      (Memory::malloc(sizeof(T)*(n-1) + sizeof(Object)));
    o->use = 1;
    o->n   = n;
    return o;
  }

  template <class T, bool fd>
  forceinline typename SharedArray<T,fd>::Object*
  SharedArray<T,fd>::Object::copy(void) const {
    assert(n>0);
    Object* o = allocate(n);
    for (int i=n; i--;)
      new (&(o->a[i])) T(a[i]);
    return o;
  }

  template <class T, bool fd>
  forceinline void
  SharedArray<T,fd>::Object::subscribe(void) {
    use++;
  }

  template <class T, bool fd>
  forceinline void
  SharedArray<T,fd>::Object::release(void) {
    if (--use == 0) {
      if (fd)
	for (int i=n; i--;)
	  a[i].~T();
      Memory::free(this);
    }
  }


  template <class T, bool fd>
  forceinline
  SharedArray<T,fd>::SharedArray(void) : sao(NULL) {}

  template <class T, bool fd>
  forceinline
  SharedArray<T,fd>::SharedArray(int n) {
    sao = (n>0) ? Object::allocate(n) : NULL;
  }
  template <class T, bool fd>
  forceinline
  SharedArray<T,fd>::SharedArray(const SharedArray<T,fd>& a) {
    sao = a.sao;
    if (sao != NULL)
      sao->subscribe();
  }

  template <class T, bool fd>
  forceinline
  SharedArray<T,fd>::~SharedArray(void) {
    if (sao != NULL)
      sao->release();
  }

  template <class T, bool fd>
  forceinline const SharedArray<T,fd>&
  SharedArray<T,fd>::operator=(const SharedArray<T,fd>& a) {
    if (this != &a) {
      if (sao != NULL)
	sao->release();
      sao = a.sao;
      if (sao != NULL)
	sao->subscribe();
    }
    return *this;
  }

  template <class T, bool fd>
  inline void
  SharedArray<T,fd>::update(bool share, SharedArray<T,fd>& a) {
    if (sao != NULL)
      sao->release();
    if (share) {
      sao = a.sao;
      if (sao != NULL)
	sao->subscribe();
    } else {
      sao = (a.sao == NULL) ? NULL : a.sao->copy();
    }
  }

  template <class T, bool fd>
  forceinline T&
  SharedArray<T,fd>::operator[](int i) {
    return sao->a[i];
  }

  template <class T, bool fd>
  forceinline const T&
  SharedArray<T,fd>::operator[](int i) const {
    return sao->a[i];
  }

  template <class T, bool fd>
  forceinline int
  SharedArray<T,fd>::size(void) const {
    return (sao == NULL) ? 0 : sao->n;
  }

  template <class T, bool fd>
  forceinline void
  SharedArray<T,fd>::size(int n) {
    if (n==0) {
      if (sao != NULL)
	sao->release();
      sao = NULL;
    } else {
      sao->n = n;
    }
  }

  template <class T, bool fd>
  inline void
  SharedArray<T,fd>::shrink(int n) {
    assert(n < sao->n);
    Object* nsao = Object::allocate(n);
    for (int i = n; i--; )
      new (&(nsao->a[i])) T(sao->a[i]);
    sao->release();
    sao = nsao;
  }

  template <class T, bool fd>
  inline void
  SharedArray<T,fd>::ensure(int n) {
    if (sao == NULL) {
      if (n>0)
	sao = Object::allocate(n);
      return;
    }
    if (n >= sao->n) {
      int m = std::max(2*sao->n,n);
      Object* nsao = Object::allocate(m);
      for (int i = sao->n; i--; )
	new (&(nsao->a[i])) T(sao->a[i]);
      sao->release();
      sao = nsao;
    }
  }

}}

#endif

// STATISTICS: support-any
