/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
 *     Guido Tack, 2004
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

#include <cstdarg>
#include <iostream>
#include <sstream>

namespace Gecode {

  /**
   * \brief Shared array with arbitrary number of elements
   *
   * Sharing is implemented by reference counting: the same elements
   * are shared among several objects.
   *
   */
  template<class T>
  class SharedArray : public SharedHandle {
  protected:
    /// Implementation of object for shared arrays
    class SAO : public SharedHandle::Object {
    private:
      /// Elements
      T*  a;
      /// Number of elements
      int n;
    public:
      /// Allocate for \a n elements
      SAO(int n);
      /// Create copy of elements
      virtual SharedHandle::Object* copy(void) const;
      /// Delete object
      virtual ~SAO(void);

      /// Access element at position \a i
      T& operator [](int i);
      /// Access element at position \a i
      const T& operator [](int i) const;

      /// Return number of elements
      int size(void) const;
    };
  public:
    /**
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized shared array is init and the assignment operator .
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
    T& operator [](int i);
    /// Access element at position \a i
    const T& operator [](int i) const;

    /// Return number of elements
    int size(void) const;
  };

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates SharedArray
   */
  template<class Char, class Traits, class T>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const SharedArray<T>& x);


  /*
   * Implementation
   *
   */

  /*
   * Shared arrays
   *
   */
  template<class T>
  forceinline
  SharedArray<T>::SAO::SAO(int n0) : n(n0) {
    a = (n>0) ? heap.alloc<T>(n) : NULL;
  }

  template<class T>
  SharedHandle::Object*
  SharedArray<T>::SAO::copy(void) const {
    SAO* o = new SAO(n);
    for (int i=n; i--;)
      o->a[i] = a[i];
    return o;
  }

  template<class T>
  SharedArray<T>::SAO::~SAO(void) {
    if (n>0) {
      heap.free<T>(a,n);
    }
  }

  template<class T>
  forceinline T&
  SharedArray<T>::SAO::operator [](int i) {
    assert((i>=0) && (i<n));
    return a[i];
  }

  template<class T>
  forceinline const T&
  SharedArray<T>::SAO::operator [](int i) const {
    assert((i>=0) && (i<n));
    return a[i];
  }

  template<class T>
  forceinline int
  SharedArray<T>::SAO::size(void) const {
    return n;
  }



  template<class T>
  forceinline
  SharedArray<T>::SharedArray(void) {}

  template<class T>
  forceinline
  SharedArray<T>::SharedArray(int n)
    : SharedHandle(new SAO(n)) {}

  template<class T>
  forceinline
  SharedArray<T>::SharedArray(const SharedArray<T>& sa)
    : SharedHandle(sa) {}

  template<class T>
  forceinline void
  SharedArray<T>::init(int n) {
    assert(object() == NULL);
    object(new SAO(n));
  }

  template<class T>
  forceinline T&
  SharedArray<T>::operator [](int i) {
    assert(object() != NULL);
    return (*static_cast<SAO*>(object()))[i];
  }

  template<class T>
  forceinline const T&
  SharedArray<T>::operator [](int i) const {
    assert(object() != NULL);
    return (*static_cast<SAO*>(object()))[i];
  }

  template<class T>
  forceinline int
  SharedArray<T>::size(void) const {
    assert(object() != NULL);
    return static_cast<SAO*>(object())->size();
  }

  template<class Char, class Traits, class T>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const SharedArray<T>& x) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (x.size() > 0) {
      s << x[0];
      for (int i=1; i<x.size(); i++)
        s << ", " << x[i];
    }
    s << '}';
    return os << s.str();
  }

}

// STATISTICS: kernel-other
