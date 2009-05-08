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

  template <class Var> class VarArray;
  template <class Var> class VarArgArray;

  /**
   * \brief %Variable arrays
   *
   * %Variable arrays store variables. They are typically used
   * for storing the variables being part of a solution.
   *
   * Never use them for temporary purposes, use argument arrays
   * instead.
   *
   * %Variable arrays can be enlarged dynamically. For memory efficiency, the
   * initial array is allocated in the space. When adding variables, it is
   * automatically resized and allocated on the heap.
   *
   * \ingroup TaskVar
   */
  template <class Var>
  class VarArray {
  protected:
    /// Number of variables (size)
    int n;
    /// Allocated size of the array
    int capacity;
    /// Array of variables
    Var* x;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (array of size 0)
    VarArray(void);
    /// Allocate array with \a m variables
    VarArray(Space& home, int m);
    /// Initialize from variable argument array \a a (copy elements)
    VarArray(Space& home, const VarArgArray<Var>&);
    /// Initialize from variable array \a a (share elements)
    VarArray(const VarArray<Var>& a);
    /// Initialize from variable array \a a (share elements)
    const VarArray<Var>& operator =(const VarArray<Var>& a);
    /// Destructor
    ~VarArray(void);
    //@}

    /// \name Array size
    //@{
    /// Return size of array (number of elements)
    int size(void) const;
    /**
      * \brief Insert or remove (uninitialized!) elements at the end such
      * that size becomes \a m
      */
    void resize(Space& home, int m);
    //@}

    /// \name Array elements
    //@{
    /// Return variable at position \a i
    Var& operator [](int i);
    /// Return variable at position \a i
    const Var& operator [](int i) const;
    /// Insert a new element \a v at the end of the array (increase size by 1)
    void add(Space& home, const Var& v);
    //@}

    /// \name Cloning
    //@{
    /**
     * \brief Update array to be a clone of array \a a
     *
     * If \a share is true, sharing is retained for all shared
     * data structures. Otherwise, for each of them an independent
     * copy is created.
     */
    void update(Space&, bool share, VarArray<Var>& a);
    //@}
  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };


  /**
   * \brief View arrays
   *
   * View arrays store views. They are typically used for storing the
   * views with which propagators and branchings compute.
   * \ingroup TaskActor
   */

  template <class View>
  class ViewArray {
  private:
    /// Number of views (size)
    int  n;
    /// Views
    View* x;
    /// Sort order for views
    template <class X>
    class ViewLess {
    public:
      bool operator ()(const X&, const X&);
    };
    /// Sort \a n views \a x according to \a ViewLess
    static void sort(View* x, int n);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor (array of size 0)
    ViewArray(void);
    /// Allocate array with \a m views
    ViewArray(Space& home, int m);
    /// Initialize from view array \a a (share elements)
    ViewArray(const ViewArray<View>& a);
    /// Initialize from view array \a a (copy elements)
    ViewArray(Space& home, const ViewArray<View>& a);
    /// Initialize from view array \a a (share elements)
    const ViewArray<View>& operator =(const ViewArray<View>& a);
    /**
     * \brief Initialize from variable argument array \a a (copy elements)
     *
     * Note that the view type \a View must provide a constructor
     * for the associated \a Var type.
     */
    template <class Var>
    ViewArray(Space& home, const VarArgArray<Var>& a)
      : n(a.size()) {
      // This may not be in the hpp file (to satisfy the MS compiler)
      if (n>0) {
        x = home.alloc<View>(n);
        for (int i=n; i--; )
          x[i]=a[i];
      } else {
        x = NULL;
      }
    }
    //@}

    /// \name Array size
    //@{
    /// Return size of array (number of elements)
    int size(void) const;
    /// Decrease size of array (number of elements)
    void size(int n);
    //@}

    /// \name Array elements
    //@{
    /// Return view at position \a i
    View& operator [](int i);
    /// Return view at position \a i
    const View& operator [](int i) const;
    //@}

    /// \name Dependencies
    //@{
    /**
     * \brief Subscribe propagator \a p with propagation condition \a pc to variable
     *
     * In case \a process is false, the propagator is just subscribed but
     * not processed for execution (this must be used when creating
     * subscriptions during propagation).
     */
    void subscribe(Space& home, Propagator& p, PropCond pc, bool process=true);
    /// Cancel subscription of propagator \a p with propagation condition \a pc to all views
    void cancel(Space& home, Propagator& p, PropCond pc);
    /// Subscribe advisor \a a to variable
    void subscribe(Space& home, Advisor& a);
    /// Cancel subscription of advisor \a a
    void cancel(Space& home, Advisor& a);
    //@}

    /// \name Cloning
    //@{
    /**
     * \brief Update array to be a clone of array \a a
     *
     * If \a share is true, sharing is retained for all shared
     * data structures. Otherwise, for each of them an independent
     * copy is created.
     */
    void update(Space&, bool share, ViewArray<View>& a);
    //@}


    /// \name Moving elements
    //@{
    /// Move assigned view from position 0 to position \a i (shift elements to the left)
    void move_fst(int i);
    /// Move assigned view from position \c size()-1 to position \a i (truncate array by one)
    void move_lst(int i);
    /** \brief Move view from position 0 to position \a i (shift elements to the left)
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to view at position \a i.
     */
    void move_fst(int i, Space& home, Propagator& p, PropCond pc);
    /** \brief Move view from position \c size()-1 to position \a i (truncate array by one)
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to view at position \a i.
     */
    void move_lst(int i, Space& home, Propagator& p, PropCond pc);
    /** \brief Move view from position 0 to position \a i (shift elements to the left)
     *
     * Before moving, cancel subscription of advisor \a a
     * to view at position \a i.
     */
    void move_fst(int i, Space& home, Advisor& a);
    /** \brief Move view from position \c size()-1 to position \a i (truncate array by one)
     *
     * Before moving, cancel subscription of advisor \a a to view
     * at position \a i.
     */
    void move_lst(int i, Space& home, Advisor& a);
    //@}

    /// \name Dropping elements
    //@{
    /// Drop assigned views from positions 0 to \a i-1 from array
    void drop_fst(int i);
    /// Drop assigned views from positions \a i+1 to \c size()-1 from array
    void drop_lst(int i);
    /** \brief Drop views from positions 0 to \a i-1 from array
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to views at positions 0 to \a i-1.
     */
    void drop_fst(int i, Space& home, Propagator& p, PropCond pc);
    /** \brief Drop assigned views from positions \a i+1 to \c size()-1 from array
     *
     * Before moving, cancel subscription of propagator \a p with
     * propagation condition \a pc to views at positions \a i+1 to
     * \c size()-1.
     */
    void drop_lst(int i, Space& home, Propagator& p, PropCond pc);
    /** \brief Drop views from positions 0 to \a i-1 from array
     *
     * Before moving, cancel subscription of advisor \a a to views at
     * positions 0 to \a i-1.
     */
    void drop_fst(int i, Space& home, Advisor& a);
    /** \brief Drop assigned views from positions \a i+1 to \c size()-1 from array
     *
     * Before moving, cancel subscription of advisor \a a to views at
     * positions \a i+1 to \c size()-1.
     */
    void drop_lst(int i, Space& home, Advisor& a);
    //@}

    /// \name View equality
    //@{
    /**
     * \brief Test whether array has multiple occurence of the same view
     *
     * Note that assigned views are ignored.
     */
    bool same(const Space& home) const;
    /**
     * \brief Test whether array contains a view being the same as \a y
     *
     * Note that assigned views are ignored.
     */
    bool same(const Space& home, const View& y) const;
    /// Remove all duplicate views from array (changes element order)
    void unique(const Space& home);
    //@}

    /// \name View sharing
    //@{
    /**
     * \brief Test whether array contains shared views
     *
     * Note that assigned views are ignored.
     */
    bool shared(const Space& home) const;
    /**
     * \brief Test whether array contains a view being shared with \a y
     *
     * Note that assigned views are ignored.
     */
    template <class ViewY>
    bool shared(const Space& home, const ViewY& y) const;
    /**
     * \brief Test whether array together with array \a y contains shared views
     *
     * Note that assigned views are ignored.
     */
    template <class ViewY>
    bool shared(const Space& home, const ViewArray<ViewY>& y) const;
    //@}

  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };

  /**
   * \brief Base-class for argument arrays
   *
   * Argument arrays are used as convenient mechanism of passing arguments
   * when calling functions as they combine both the size and the elements
   * of an array. For a small number of elements, memory is allocated by
   * creating an argument array object. Otherwise the memory is allocated
   * from the heap.
   *
   * This base-class is not to be used directly, use PrimArgArray for
   * argument arrays of primitive types and VarArgArray for argument
   * arrays storing variables.
   */
  template <class T>
  class ArgArrayBase {
  protected:
    /// Number of elements
    int n;
    /// Element array
    T*  a;
    /// How much elements are possible inside array
    static const int onstack_size = 16;
    /// In-array storage for elements
    T onstack[onstack_size];
    /// Allocate memory for \a n elements
    T* allocate(int n);
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate array with \a n elements
    ArgArrayBase(int n);
    /// Initialize from argument array \a a (copy elements)
    ArgArrayBase(const ArgArrayBase<T>& a);
    /// Initialize from view array \a a (copy elements)
    const ArgArrayBase<T>& operator =(const ArgArrayBase<T>& a);
    //@}

    /// \name Array size
    //@{
    /// Return size of array (number of elements)
    int size(void) const;
    //@}

    /// \name Array elements
    //@{
    /// Return element at position \a i
    T& operator [](int i);
    /// Return element at position \a i
    const T& operator [](int i) const;
    //@}

    /// \name Destructor
    //@{
    /// Destructor
    ~ArgArrayBase(void);
    //@}
  private:
    static void* operator new(size_t);
    static void  operator delete(void*,size_t);
  };


  /**
   * \brief Argument array for primtive types
   *
   * Argument arrays are used as convenient mechanism of passing arguments
   * when calling functions as they combine both the size and the elements
   * of an array. For a small number of elements, memory is allocated by
   * creating an argument array object. Otherwise the memory is allocated
   * from the heap.
   *
   * \ingroup TaskVar
   */
  template <class T>
  class PrimArgArray : public ArgArrayBase<T> {
  protected:
    using ArgArrayBase<T>::a;
  public:
    using ArgArrayBase<T>::size;
    /// \name Constructors and initialization
    //@{
    /// Allocate array with \a n elements
    explicit PrimArgArray(int n);
    /// Allocate array with \a n elements and initialize with \a e0, ...
    PrimArgArray(int n, T e0, ...);
    /// Allocate array with \a n elements and initialize with elements from array \a e
    PrimArgArray(int n, const T* e);
    /// Initialize from primitive argument array \a a (copy elements)
    PrimArgArray(const PrimArgArray<T>& a);
    //@}
  };

  /**
   * \brief Argument array for non-primitive types
   *
   * Argument arrays are used as convenient mechanism of passing arguments
   * when calling functions as they combine both the size and the elements
   * of an array. For a small number of elements, memory is allocated by
   * creating an argument array object. Otherwise the memory is allocated
   * from the heap.
   *
   * \ingroup TaskVar
   */
  template <class T>
  class ArgArray : public ArgArrayBase<T> {
  protected:
    using ArgArrayBase<T>::a;
  public:
    using ArgArrayBase<T>::size;
    /// \name Constructors and initialization
    //@{
    /// Allocate array with \a n elements
    explicit ArgArray(int n);
    /// Allocate array with \a n elements and initialize with elements from array \a e
    ArgArray(int n, const T* e);
    /// Initialize from primitive argument array \a a (copy elements)
    ArgArray(const ArgArray<T>& a);
    //@}
  };

  /**
   * \brief Argument array for variables
   *
   * Argument arrays are used as convenient mechanism of passing arguments
   * when calling functions as they combine both the size and the elements
   * of an array. For a small number of elements, memory is allocated by
   * creating an argument array object. Otherwise the memory is allocated
   * from the heap.
   *
   * \ingroup TaskVar
   */
  template <class Var>
  class VarArgArray : public ArgArrayBase<Var> {
  protected:
    using ArgArrayBase<Var>::a;
    using ArgArrayBase<Var>::n;
    /// Sort order for variables
    class VarLess {
    public:
      bool operator ()(const Var&, const Var&);
    };
  public:
    using ArgArrayBase<Var>::size;
    /// \name Constructors and initialization
    //@{
    /// Allocate array with \a n elements
    explicit VarArgArray(int n);
    /// Initialize from variable argument array \a a (copy elements)
    VarArgArray(const VarArgArray<Var>& a);
    /// Initialize from variable array \a a (copy elements)
    VarArgArray(const VarArray<Var>& a);
    //@}
    /// \name Variable equality
    //@{
    /**
     * \brief Test whether array contains same variable multiply
     *
     * Note that assigned variables are ignored.
     */
    bool same(const Space& home) const;
    /**
     * \brief Test whether array contains variable \a y
     *
     * Note that assigned variables are ignored.
     */
    bool same(const Space& home, const Var& y) const;
    /**
     * \brief Test whether all elements from array and \a y contains same variable multiply
     *
     * Note that assigned variables are ignored.
     */
    bool same(const Space& home, const VarArgArray<Var>& y) const;
    //@}
  };

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates VarArray
   */
  template<class Char, class Traits, class Var>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const VarArray<Var>& x);

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates ViewArray
   */
  template<class Char, class Traits, class View>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ViewArray<View>& x);

  /**
   * \brief Print array elements enclosed in curly brackets
   * \relates ArgArrayBase
   */
  template<class Char, class Traits, class T>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ArgArrayBase<T>& x);



  /** \brief Traits of arrays in Gecode
   *
   * This class collects the traits of an array in Gecode.
   * The traits used are the following.
   *     - <code>typedef Type storage_type</code>  where \c Type is the type
   *       of an appropriate storage type for this array.
   *     - <code>typedef Type value_type</code>  where \c Type is the type
   *       of the elements of this array.
   *     - <code>typedef Type args_type</code>  where \c Type is the type
   *       of the appropriate Args-array type (e.g., \c BoolVarArgs if \c A is
   *       \c BoolVarArray).
   */
  template <class A>
  class ArrayTraits {};

  /*
   * Implementation
   *
   */

  /*
   * Variable arrays
   *
   * These arrays are usually allocated in the space, but if they are resized,
   * the new array is allocated on the heap. The size (n) and capacity show
   * where the array is allocated: it is in the space if and only if
   * n==capacity.
   *
   */

  template <class Var>
  forceinline
  VarArray<Var>::VarArray(void) : n(0), capacity(0), x(NULL) {}

  template <class Var>
  forceinline
  VarArray<Var>::VarArray(Space& home, int n0)
    : n(n0), capacity(n0) {
    // Allocate from space
    x = (n>0) ? home.alloc<Var>(n) : NULL;
  }

  template <class Var>
  forceinline
  VarArray<Var>::VarArray(const VarArray<Var>& a) {
    n = a.n; capacity = a.capacity; x = a.x;
  }

  template <class Var>
  forceinline
  VarArray<Var>::~VarArray(void) {
    if (n != capacity) {
      // Array was allocated on the heap
      heap.free<Var>(x, capacity);
    }
  }

  template <class Var>
  forceinline const VarArray<Var>&
  VarArray<Var>::operator =(const VarArray<Var>& a) {
    n = a.n; capacity = a.capacity; x = a.x;
    return *this;
  }

  template <class Var>
  forceinline int
  VarArray<Var>::size(void) const {
    return n;
  }

  template <class Var>
  forceinline void
  VarArray<Var>::resize(Space& home, int m) {
    int newsize;
    if (m<n) {
      // Shrink array and leave capacity at m+1, forcing heap allocation
      newsize = m+1;
    } else if (m<capacity) {
      // As m>=n<capacity, the array is heap allocated, just resize
      n = m;
      return;
    } else {
      // Resize, so that newsize != m forcing heap allocation
      newsize = std::max(m+1, (3*capacity)/2);
    }

    if (n == capacity) {
      // Array was space allocated, copy to heap with new size
      Var* oldx = x;
      x = heap.copy<Var>(heap.alloc<Var>(newsize), x, n);
      home.rfree(oldx, capacity);
    } else {
      // Array was heap allocated, just reallocate with new size
      x = heap.realloc<Var>(x, n, newsize);
    }
    capacity = newsize; n = m;
    // After resizing, the array is always heap allocated
    assert(capacity != n);
  }

  template <class Var>
  forceinline Var&
  VarArray<Var>::operator [](int i) {
    assert((i >= 0) && (i < size()));
    return x[i];
  }

  template <class Var>
  forceinline const Var&
  VarArray<Var>::operator [](int i) const {
    assert((i >= 0) && (i < size()));
    return x[i];
  }

  template <class Var>
  forceinline void
  VarArray<Var>::add(Space& home, const Var& v) {
    resize(home, n+1);
    new (&(*this)[n-1]) Var(v);
  }

  template <class Var>
  forceinline void
  VarArray<Var>::update(Space& home, bool share, VarArray<Var>& a) {
    capacity = a.n;
    n = capacity;
    if (capacity > 0) {
      x = home.alloc<Var>(capacity);
      for (int i = capacity; i--; )
        x[i].update(home, share, a.x[i]);
    } else {
      x = NULL;
    }
  }

  template <class Var>
  void*
  VarArray<Var>::operator new(size_t) {
    return NULL;
  }

  template <class Var>
  void
  VarArray<Var>::operator delete(void*,size_t) {
  }

  /*
   * View arrays
   *
   */

  template <class View>
  forceinline
  ViewArray<View>::ViewArray(void) : n(0), x(NULL) {}

  template <class View>
  forceinline
  ViewArray<View>::ViewArray(Space& home, int n0)
    : n(n0) {
    x = (n>0) ? home.alloc<View>(n) : NULL;
  }

  template <class View>
  ViewArray<View>::ViewArray(Space& home, const ViewArray<View>& a)
    : n(a.size()) {
    if (n>0) {
      x = home.alloc<View>(n);
      for (int i = n; i--; )
        x[i] = a[i];
    } else {
      x = NULL;
    }
  }

  template <class View>
  forceinline
  ViewArray<View>::ViewArray(const ViewArray<View>& a)
    : n(a.n), x(a.x) {}

  template <class View>
  forceinline const ViewArray<View>&
  ViewArray<View>::operator =(const ViewArray<View>& a) {
    n = a.n; x = a.x;
    return *this;
  }

  template <class View>
  forceinline int
  ViewArray<View>::size(void) const {
    return n;
  }

  template <class View>
  forceinline void
  ViewArray<View>::size(int n0) {
    n = n0;
  }

  template <class View>
  forceinline View&
  ViewArray<View>::operator [](int i) {
    assert((i >= 0) && (i < size()));
    return x[i];
  }

  template <class View>
  forceinline const View&
  ViewArray<View>::operator [](int i) const {
    assert((i >= 0) && (i < size()));
    return x[i];
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_fst(int i) {
    // move x[0] to x[i]
    assert(x[i].assigned());
    x[i]=x[0]; x++; n--;
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_lst(int i) {
    // move x[n-1] to x[i]
    assert(x[i].assigned());
    n--; x[i]=x[n];
  }

  template <class View>
  forceinline void
  ViewArray<View>::drop_fst(int i) {
    // Drop elements from 0..i-1
    assert(i>=0);
    x += i; n -= i;
  }

  template <class View>
  forceinline void
  ViewArray<View>::drop_lst(int i) {
    // Drop elements from i+1..n-1
    assert(i<n);
    n = i+1;
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_fst(int i, Space& home, Propagator& p, PropCond pc) {
    // Move x[0] to x[i]
    x[i].cancel(home,p,pc);
    x[i]=x[0]; x++; n--;
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_lst(int i, Space& home, Propagator& p, PropCond pc) {
    // Move x[n-1] to x[i]
    x[i].cancel(home,p,pc);
    n--; x[i]=x[n];
  }

  template <class View>
  void
  ViewArray<View>::drop_fst(int i, Space& home, Propagator& p, PropCond pc) {
    // Drop elements from 0..i-1
    assert(i>=0);
    for (int j=i; j--; )
      x[j].cancel(home,p,pc);
    x += i; n -= i;
  }

  template <class View>
  void
  ViewArray<View>::drop_lst(int i, Space& home, Propagator& p, PropCond pc) {
    // Drop elements from i+1..n-1
    assert(i<n);
    for (int j=i+1; j<n; j++)
      x[j].cancel(home,p,pc);
    n = i+1;
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_fst(int i, Space& home, Advisor& a) {
    // Move x[0] to x[i]
    x[i].cancel(home,a);
    x[i]=x[0]; x++; n--;
  }

  template <class View>
  forceinline void
  ViewArray<View>::move_lst(int i, Space& home, Advisor& a) {
    // Move x[n-1] to x[i]
    x[i].cancel(home,a);
    n--; x[i]=x[n];
  }

  template <class View>
  void
  ViewArray<View>::drop_fst(int i, Space& home, Advisor& a) {
    // Drop elements from 0..i-1
    assert(i>=0);
    for (int j=i; j--; )
      x[j].cancel(home,a);
    x += i; n -= i;
  }

  template <class View>
  void
  ViewArray<View>::drop_lst(int i, Space& home, Advisor& a) {
    // Drop elements from i+1..n-1
    assert(i<n);
    for (int j=i+1; j<n; j++)
      x[j].cancel(home,a);
    n = i+1;
  }

  template <class View>
  void
  ViewArray<View>::update(Space& home, bool share, ViewArray<View>& y) {
    n = y.n;
    if (n > 0) {
      x = home.alloc<View>(n);
      for (int i = n; i--; )
        x[i].update(home, share, y.x[i]);
    } else {
      x = NULL;
    }
  }

  template <class View>
  void
  ViewArray<View>::subscribe(Space& home, Propagator& p, PropCond pc,
                             bool process) {
    for (int i = n; i--; )
      x[i].subscribe(home,p,pc,process);
  }

  template <class View>
  void
  ViewArray<View>::cancel(Space& home, Propagator& p, PropCond pc) {
    for (int i = n; i--; )
      x[i].cancel(home,p,pc);
  }

  template <class View>
  void
  ViewArray<View>::subscribe(Space& home, Advisor& a) {
    for (int i = n; i--; )
      x[i].subscribe(home,a);
  }

  template <class View>
  void
  ViewArray<View>::cancel(Space& home, Advisor& a) {
    for (int i = n; i--; )
      x[i].cancel(home,a);
  }

  template <class View>
  forceinline bool
  __before(const View& x, const View& y) {
    return before(x,y);
  }

  template <class View> template <class X>
  forceinline bool
  ViewArray<View>::ViewLess<X>::operator ()(const X& a, const X& b) {
    return __before(a,b);
  }

  template <class View>
  void
  ViewArray<View>::sort(View* y, int m) {
    ViewLess<View> vl;
    Support::quicksort<View,ViewLess<View> >(y,m,vl);
  }

  template <class X, class Y>
  forceinline bool
  __same(const X& x, const Y& y) {
    return same(x,y);
  }
  template <class X, class Y>
  forceinline bool
  __shared(const X& x, const Y& y) {
    return shared(x,y);
  }

  template <class View>
  bool
  ViewArray<View>::same(const Space& home) const {
    if (n < 2)
      return false;
    Region r(home);
    View* y = r.alloc<View>(n);
    for (int i = n; i--; )
      y[i] = x[i];
    sort(y,n);
    for (int i = n-1; i--; )
      if (!y[i].assigned() && __same(y[i+1],y[i])) {
        r.free<View>(y,n);
        return true;
      }
    r.free<View>(y,n);
    return false;
  }

  template <class View>
  bool
  ViewArray<View>::same(const Space&, const View& y) const {
    if (y.assigned())
      return false;
    for (int i = n; i--; )
      if (__same(x[i],y))
        return true;
    return false;
  }

  template <class View>
  void
  ViewArray<View>::unique(const Space&) {
    if (n < 2)
      return;
    sort(x,n);
    int j = 0;
    for (int i = 1; i<n; i++)
      if (!__same(x[j],x[i]))
        x[++j] = x[i];
    n = j+1;
  }

  template <class View>
  bool
  ViewArray<View>::shared(const Space& home) const {
    if (n < 2)
      return false;
    Region r(home);
    View* y = r.alloc<View>(n);
    for (int i = n; i--; )
      y[i] = x[i];
    sort(y,n);
    for (int i = n-1; i--; )
      if (!y[i].assigned() && __shared(y[i+1],y[i])) {
        r.free<View>(y,n);
        return true;
      }
    r.free<View>(y,n);
    return false;
  }

  template <class View> template <class ViewY>
  bool
  ViewArray<View>::shared(const Space&, const ViewY& y) const {
    if (y.assigned())
      return false;
    for (int i = n; i--; )
      if (!x[i].assigned() && __shared(x[i],y))
        return true;
    return false;
  }

  template <class View> template <class ViewY>
  bool
  ViewArray<View>::shared(const Space& home, const ViewArray<ViewY>& y) const {
    if ((size() < 1) || (y.size() < 1))
      return false;
    Region r(home);
    View* xs = r.alloc<View>(size());
    for (int i=size(); i--; )
      xs[i] = x[i];
    ViewLess<View> xvl;
    Support::quicksort<View,ViewLess<View> >(xs,size(),xvl);
    ViewY* ys = r.alloc<ViewY>(y.size());
    for (int j=y.size(); j--; )
      ys[j] = y[j];
    ViewLess<ViewY> yvl;
    Support::quicksort<ViewY,ViewLess<ViewY> >(ys,y.size(),yvl);
    {
      int i=0, j=0;
      while ((i < size()) && (j < y.size()))
        if (!x[i].assigned() && __shared(x[i],y[j])) {
          r.free<View>(xs,size());
          r.free<ViewY>(ys,y.size());
          return true;
        } else if (before(x[i],y[j])) {
          i++;
        } else {
          j++;
        }
    }
    r.free<View>(xs,size());
    r.free<ViewY>(ys,y.size());
    return false;
  }

  template <class View>
  void*
  ViewArray<View>::operator new(size_t) {
    return NULL;
  }

  template <class View>
  void
  ViewArray<View>::operator delete(void*,size_t) {
  }


  /*
   * Argument arrays: base class
   *
   */

  template <class T>
  forceinline T*
  ArgArrayBase<T>::allocate(int n) {
    return (n > onstack_size) ?
      heap.alloc<T>(static_cast<unsigned int>(n)) : &onstack[0];
  }

  template <class T>
  forceinline
  ArgArrayBase<T>::ArgArrayBase(int n0)
    : n(n0), a(allocate(n)) {}

  template <class T>
  inline
  ArgArrayBase<T>::ArgArrayBase(const ArgArrayBase<T>& aa)
    : n(aa.n), a(allocate(n)) {
    heap.copy<T>(a,aa.a,n);
  }

  template <class T>
  forceinline
  ArgArrayBase<T>::~ArgArrayBase(void) {
    if (n > onstack_size)
      heap.free(a,n);
  }

  template <class T>
  forceinline const ArgArrayBase<T>&
  ArgArrayBase<T>::operator =(const ArgArrayBase<T>& aa) {
    if (&aa != this) {
      if (n > onstack_size)
        heap.free(a,n);
      n = aa.n;
      a = allocate(aa.n);
      heap.copy<T>(a,aa.a,n);
    }
    return *this;
  }

  template <class T>
  forceinline int
  ArgArrayBase<T>::size(void) const {
    return n;
  }

  template <class T>
  forceinline T&
  ArgArrayBase<T>::operator [](int i) {
    assert((i>=0) && (i < n));
    return a[i];
  }

  template <class T>
  forceinline const T&
  ArgArrayBase<T>::operator [](int i) const {
    assert((i>=0) && (i < n));
    return a[i];
  }


  /*
   * Argument arrays for primitive types
   *
   */

  template <class T>
  forceinline
  PrimArgArray<T>::PrimArgArray(int n)
    : ArgArrayBase<T>(n) {}

  template <class T>
  PrimArgArray<T>::PrimArgArray(int n, T a0, ...)
    : ArgArrayBase<T>(n) {
    va_list args;
    va_start(args, a0);
    a[0] = a0;
    for (int i = 1; i < n; i++)
      a[i] = va_arg(args,T);
    va_end(args);
  }

  template <class T>
  PrimArgArray<T>::PrimArgArray(int n, const T* a0)
    : ArgArrayBase<T>(n) {
    for (int i=n; i--; )
      a[i] = a0[i];
  }

  template <class T>
  forceinline
  PrimArgArray<T>::PrimArgArray(const PrimArgArray<T>& aa)
    : ArgArrayBase<T>(aa) {}


  /*
   * Argument arrays for non-primitive types
   *
   */

  template <class T>
  forceinline
  ArgArray<T>::ArgArray(int n)
    : ArgArrayBase<T>(n) {}

  template <class T>
  ArgArray<T>::ArgArray(int n, const T* a0)
    : ArgArrayBase<T>(n) {
    for (int i=n; i--; )
      a[i] = a0[i];
  }

  template <class T>
  forceinline
  ArgArray<T>::ArgArray(const ArgArray<T>& aa)
    : ArgArrayBase<T>(aa) {}



  /*
   * Argument arrays for variables
   *
   */

  template <class T>
  forceinline
  VarArgArray<T>::VarArgArray(int n)
    : ArgArrayBase<T>(n) {}

  template <class T>
  forceinline
  VarArgArray<T>::VarArgArray(const VarArgArray<T>& aa)
    : ArgArrayBase<T>(aa) {}

  template <class T>
  inline
  VarArgArray<T>::VarArgArray(const VarArray<T>& x)
    : ArgArrayBase<T>(x.size()) {
    for (int i=x.size(); i--; )
      a[i]=x[i];
  }

  template <class Var>
  forceinline bool
  VarArgArray<Var>::VarLess::operator ()(const Var& a, const Var& b) {
    return a.var() < b.var();
  }

  template <class Var>
  bool
  VarArgArray<Var>::same(const Space& home) const {
    if (n < 2)
      return false;
    Region r(home);
    Var* y = r.alloc<Var>(n);
    for (int i = n; i--; )
      y[i] = a[i];
    VarLess vl;
    Support::quicksort<Var,VarLess>(y,n,vl);
    for (int i = n-1; i--; )
      if (!y[i].assigned() && (y[i+1].var() == y[i].var())) {
        r.free<Var>(y,n);
        return true;
      }
    r.free<Var>(y,n);
    return false;
  }

  template <class Var>
  bool
  VarArgArray<Var>::same(const Space& home, const VarArgArray<Var>& y) const {
    int m = n + y.n;
    if (m < 2)
      return false;
    Region r(home);
    Var* z = r.alloc<Var>(m);
    for (int i = n; i--; )
      z[i] = a[i];
    for (int i = y.n; i--; )
      z[i+n] = y.a[i];
    VarLess vl;
    Support::quicksort<Var,VarLess>(z,m,vl);
    for (int i = m-1; i--; )
      if (!z[i].assigned() && (z[i+1].var() == z[i].var())) {
        r.free<Var>(z,m);
        return true;
      }
    r.free<Var>(z,m);
    return false;
  }

  template <class Var>
  bool
  VarArgArray<Var>::same(const Space&, const Var& y) const {
    if (y.assigned())
      return false;
    for (int i = n; i--; )
      if (a[i].var() == y.var())
        return true;
    return false;
  }






  /*
   * Interdependent code
   *
   */

  template <class Var>
  inline
  VarArray<Var>::VarArray(Space& home, const VarArgArray<Var>& a)
    : n(a.size()), capacity(a.size()) {
    if (capacity>0) {
      x = home.alloc<Var>(capacity);
      for (int i = capacity; i--; )
        x[i] = a[i];
    } else {
      x = NULL;
    }
  }


  /*
   * Printing of arrays
   *
   */
  template<class Char, class Traits, class Var>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const VarArray<Var>& x) {
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

  template<class Char, class Traits, class View>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const ViewArray<View>& x) {
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

  template<class Char, class Traits, class T>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
             const ArgArrayBase<T>& x) {
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
