/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

namespace Gecode {

  /**
   * \brief Lists of ranges (intervals)
   *
   * This class implements a simple datastructure for storing sets of
   * integers as lists of ranges (intervals). Memory is managed as
   * space-allocated free lists.
   *
   * \ingroup FuncMemSpace
   */
  class RangeList : public FreeList {
  protected:
    /// Minimum of range
    int        _min;
    /// Maximum of range
    int        _max;
  public:
    /// \name Constructors
    //@{
    /// Default constructor (noop)
    RangeList(void);
    /// Initialize with minimum \a min and maximum \a max
    RangeList(int min, int max);
    /// Initialize with minimum \a min and maximum \a max and successor \a n
    RangeList(int min, int max, RangeList* n);
    //@}

    /// \name Access
    //@{
    /// Return minimum
    int min(void) const;
    /// Return maximum
    int max(void) const;
    /// Return width (distance between maximum and minimum)
    unsigned int width(void) const;

    /// Return next element
    RangeList* next(void) const;
    /// Return pointer to next element
    RangeList** nextRef(void);
    //@}

    /// \name Update
    //@{
    /// Set minimum to \a n
    void min(int n);
    /// Set maximum to \a n
    void max(int n);
    /// Set next range to \a n
    void next(RangeList* n);
    //@}

    /// \name Iterator operations
    //@{
    /// Create rangelist \a r from range iterator \a i
    template<class Iter>
    static void copy(Space& home, RangeList*& r, Iter& i);
    /// Overwrite rangelist \a r with ranges from range iterator \a i
    template<class Iter>
    static void overwrite(Space& home, RangeList*& r, Iter& i);
    /// Insert (as union) ranges from iterator \a i into \a r
    template<class I>
    static void insert(Space& home, RangeList*& r, I& i);
    //@}

    /// \name Memory management
    //@{
    /// Free memory for all elements between this and \a l (inclusive)
    void dispose(Space& home, RangeList* l);
    /// Free memory for all elements reachable from this
    void dispose(Space& home);

    /// Allocate memory from space
    static void* operator new(size_t s, Space& home);
    /// Placement-new operator (noop)
    static void* operator new(size_t s, void* p);
    /// No-op (for exceptions)
    static void  operator delete(void*);
    /// No-op (use dispose instead)
    static void  operator delete(void*, Space& home);
    /// No-op (use dispose instead)
    static void  operator delete(void*, void*);
    //@}
  };

  /*
   * Range lists
   *
   */

  forceinline
  RangeList::RangeList(void) {}

  forceinline
  RangeList::RangeList(int min, int max, RangeList* n)
    : FreeList(n), _min(min), _max(max) {}

  forceinline
  RangeList::RangeList(int min, int max)
    : _min(min), _max(max) {}

  forceinline RangeList*
  RangeList::next(void) const {
    return static_cast<RangeList*>(FreeList::next());
  }

  forceinline RangeList**
  RangeList::nextRef(void) {
    return reinterpret_cast<RangeList**>(FreeList::nextRef());
  }

  forceinline void
  RangeList::min(int n) {
    _min = n;
  }
  forceinline void
  RangeList::max(int n) {
    _max = n;
  }
  forceinline void
  RangeList::next(RangeList* n) {
    FreeList::next(n);
  }

  forceinline int
  RangeList::min(void) const {
    return _min;
  }
  forceinline int
  RangeList::max(void) const {
    return _max;
  }
  forceinline unsigned int
  RangeList::width(void) const {
    return static_cast<unsigned int>(_max - _min + 1);
  }


  forceinline void
  RangeList::operator delete(void*) {}

  forceinline void
  RangeList::operator delete(void*, Space&) {
    GECODE_NEVER;
  }

  forceinline void
  RangeList::operator delete(void*, void*) {
    GECODE_NEVER;
  }

  forceinline void*
  RangeList::operator new(size_t, Space& home) {
    return home.fl_alloc<sizeof(RangeList)>();
  }

  forceinline void*
  RangeList::operator new(size_t, void* p) {
    return p;
  }

  forceinline void
  RangeList::dispose(Space& home, RangeList* l) {
    home.fl_dispose<sizeof(RangeList)>(this,l);
  }

  forceinline void
  RangeList::dispose(Space& home) {
    RangeList* l = this;
    while (l->next() != NULL)
      l=l->next();
    dispose(home,l);
  }

  template<class Iter>
  forceinline void
  RangeList::copy(Space& home, RangeList*& r, Iter& i) {
    RangeList sentinel; sentinel.next(r);
    RangeList* p = &sentinel;
    while (i()) {
      RangeList* n = new (home) RangeList(i.min(),i.max());
      p->next(n); p=n; ++i;
    }
    p->next(NULL);
    r = sentinel.next();
  }

  template<class Iter>
  forceinline void
  RangeList::overwrite(Space& home, RangeList*& r, Iter& i) {
    RangeList sentinel; sentinel.next(r);
    RangeList* p = &sentinel;
    RangeList* c = p->next();
    while ((c != NULL) && i()) {
      c->min(i.min()); c->max(i.max());
      p=c; c=c->next(); ++i;
    }
    if ((c == NULL) && !i())
      return;
    if (c == NULL) {
      assert(i());
      // New elements needed
      do {
        RangeList* n = new (home) RangeList(i.min(),i.max());
        p->next(n); p=n; ++i;
      } while (i());
    } else {
      // Dispose excess elements
      while (c->next() != NULL)
        c=c->next();
      p->next()->dispose(home,c);
    }
    p->next(NULL);
    r = sentinel.next();
  }

  template<class I>
  forceinline void
  RangeList::insert(Space& home, RangeList*& r, I& i) {
    RangeList sentinel;
    sentinel.next(r);
    RangeList* p = &sentinel;
    RangeList* c = p->next();
    while ((c != NULL) && i()) {
      if ((c->max()+1 < i.min())) {
        p=c; c=c->next();
      } else if (i.max()+1 < c->min()) {
        RangeList* n = new (home) RangeList(i.min(),i.max(),c);
        p->next(n); p=n; ++i;
      } else {
        int min = std::min(c->min(),i.min());
        int max = std::max(c->max(),i.max());
        RangeList* f=c;
        p=c; c=c->next(); ++i;
      next:
        if ((c != NULL) && (c->min() <= max+1)) {
          max = std::max(max,c->max());
          p=c; c=c->next();
          goto next;
        }
        if (i() && (i.min() <= max+1)) {
          max = std::max(max,i.max());
          ++i;
          goto next;
        }
        // Dispose now unused elements
        if (f->next() != p)
          f->next()->dispose(home,p);
        f->min(min); f->max(max); f->next(c);
      }
    }
    if (c == NULL) {
      while (i()) {
        RangeList* n = new (home) RangeList(i.min(),i.max());
        p->next(n); p=n;
        ++i;
      }
      p->next(NULL);
    }
    r = sentinel.next();
  }

}
// STATISTICS: kernel-other
