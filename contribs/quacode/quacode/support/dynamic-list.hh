/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/support/dynamic-stack.hh
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#ifndef __GECODE_SUPPORT_DYNAMIC_LIST_HH__
#define __GECODE_SUPPORT_DYNAMIC_LIST_HH__

namespace Gecode { namespace Support {

  /**
   * \brief Linked list with arbitrary number of elements
   *
   * \ingroup FuncSupport
   */
  template<class T, class A>
  class DynamicList {
  private:
    /// Link struct
    struct Link {
      T val;
      Link * next;
      Link * prev;
    };
    /// Memory allocator
    A& a;
    /// Current size of list
    int _size;
    /// First element in list
    Link* fst;
    /// Current element in list
    Link* cur;
    /// Elements in list
    Link*  l;
  public:
    /// Initialize list
    DynamicList(A& a);
    /// Copy constructor
    DynamicList(const DynamicList& s);
    /// Assignment operator
    const DynamicList& operator =(const DynamicList&);
    /// Release memory
    ~DynamicList(void);

    /// Test whether list is empty
    bool empty(void) const;
    /// Returns number of elements of list
    int size(void) const;

    /// Test whether the cursor of list is at the end
    bool end(void) const;
    /// Return the current element of the list (pointed by the cursor)
    T operator ()(void);
    /// Move cursor (so the current element) to the first of the list
    void rewind(void);
    /// Move cursor (so the current element) to the last of the list
    void rrewind(void);
    /// Move to next element
    void operator ++(void);
    /// Move to previous element
    void operator --(void);

    /// Reset list to be empty
    void reset(void);
    /// Insert element \a x at head of list
    void insertFront(const T& x);
    /// Insert element \a x before current element
    void insert(const T& x);
    /// Remove current element (pointed by the cursor)
    void removeCurrent(void);
  };


  template<class T, class A>
  forceinline
  DynamicList<T,A>::DynamicList(A& a0)
    : a(a0), _size(0), fst(NULL), cur(NULL) {}

  template<class T, class A>
  forceinline
  DynamicList<T,A>::DynamicList(const DynamicList& s)
    : a(s.a), _size(0), fst(NULL), cur(NULL) {
    if (s.fst != NULL) {
      Link* l = s.fst->prev;
      while (l != s.fst) {
        insertFront(l->val);
        l = l->prev;
      }
      insertFront(l->val);
    }
  }

  template<class T, class A>
  forceinline
  const DynamicList<T,A>& DynamicList<T,A>::operator =(const DynamicList& s) {
    reset();
    if (s.fst != NULL) {
      Link* l = s.fst->prev;
      while (l != s.fst) {
        insertFront(l->val);
        l = l->prev;
      }
      insertFront(l->val);
    }
    return *this;
  }

  template<class T, class A>
  forceinline
  DynamicList<T,A>::~DynamicList(void) {
    reset();
  }

  template<class T, class A>
  forceinline int
  DynamicList<T,A>::size(void) const {
    return _size;
  }

  template<class T, class A>
  forceinline bool
  DynamicList<T,A>::empty(void) const {
    return fst == NULL;
  }

  template<class T, class A>
  forceinline bool
  DynamicList<T,A>::end(void) const {
    return cur == NULL;
  }

  template<class T, class A>
  forceinline T
  DynamicList<T,A>::operator ()(void) {
    assert(!empty() && !end());
    return cur->val;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::rewind(void) {
    cur = fst;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::rrewind(void) {
    if (empty())
      cur = NULL;
    else
      cur = fst->prev;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::operator ++(void) {
    assert(!end());
    if (cur == fst->prev) cur = NULL; // On last element
    else cur = cur->next;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::operator --(void) {
    assert(!end());
    if (cur == fst) cur = NULL; // On first element
    else cur = cur->prev;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::reset(void) {
    rewind();
    while (!end()) removeCurrent();
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::removeCurrent(void) {
    assert(!end());
    Link* tmp = cur;
    if (fst->next == fst) {
      // Only one element in list
      fst = NULL;
      cur = NULL;
    } else {
      if (cur == fst) {
        // On first element
        fst = fst->next;
        cur = fst;
      } else if (cur->next == fst) {
        // On last element
        cur = cur->prev;
      } else {
        cur = cur->next;
      }
      tmp->prev->next = tmp->next;
      tmp->next->prev = tmp->prev;
    }
    a.free(tmp,1);
    --_size;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::insertFront(const T& x) {
    Link * n = a.template alloc<Link>(1);
    n->val = x;
    if (empty()) {
      n->next = n;
      n->prev = n;
    } else {
      n->next = fst;
      n->prev = fst->prev;
      fst->prev = n;
      n->prev->next = n;
    }
    fst = n;
    ++_size;
  }

  template<class T, class A>
  forceinline void
  DynamicList<T,A>::insert(const T& x) {
    if (empty() || (cur == fst)) insertFront(x);
    else {
      Link * n = a.template alloc<Link>(1);
      n->val = x;
      ++_size;
      if (end()) {
        n->next = fst;
        n->prev = fst->prev;
        fst->prev = n;
        n->prev->next = n;
      } else {
        n->next = cur;
        n->prev = cur->prev;
        cur->prev = n;
        n->prev->next = n;
      }
    }
  }

}}

#endif
// STATISTICS: support-any
