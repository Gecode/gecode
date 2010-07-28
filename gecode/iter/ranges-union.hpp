/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include <algorithm>

namespace Gecode { namespace Iter { namespace Ranges {

  /// Simple fixed-size priority queue for iterators
  template<class I>
  class PriorityQueue  {
  protected:
    /// The class holding the shared queue (organized as heap)
    class SharedPriorityQueue  {
    public:
      /// Number of elements currently in queue
      int n;
      /// Maximal size
      int size;
      /// How many references to shared queue exist
      unsigned int ref;
      /// Elements (will be most likely more than one)
      I pq[1];

      /// Allocate queue with \a n elements
      static SharedPriorityQueue* allocate(int n);
      /// Reorganize after smallest element has changed
      void fixdown(void);
      /// Reorganize after element at position \a n has changed
      void fixup(int n);
    };
    /// Handle to shared queue
    SharedPriorityQueue* spq;

  public:
    /// Default constructor (creates empty queue)
    PriorityQueue(void);
    /// Create for \a n iter<ators
    PriorityQueue(int n);
    /// Initialize for \a n iterators
    void init(int n);
    /// Assign queue from queue \a p (elements are shared)
    PriorityQueue(const PriorityQueue& p);
    /// Assign queue from queue \a p (elements are shared)
    const PriorityQueue& operator =(const PriorityQueue&);
    /// Release queue
    ~PriorityQueue(void);

    /// Test whether queue is empty
    bool empty(void) const;
    /// Insert element \a x according to order
    void insert(const I& x);
    /// Remove smallest element
    void remove(void);
    /// Provide access to smallest element
    I& top(void);
    /// Reorder queue after smallest element has changed (might not be smallest any longer)
    void fix(void);
  };

  template<class I>
  forceinline typename PriorityQueue<I>::SharedPriorityQueue*
  PriorityQueue<I>::SharedPriorityQueue::allocate(int n) {
    SharedPriorityQueue* spq
      = static_cast<SharedPriorityQueue*>
      (heap.ralloc(sizeof(SharedPriorityQueue) + (n-1)*sizeof(I)));
    spq->size = n;
    spq->n    = 0;
    spq->ref  = 1;
    return spq;
  }

  template<class I>
  forceinline void
  PriorityQueue<I>::SharedPriorityQueue::fixdown(void) {
    int k = 0;
    while ((k << 1) < n) {
      int j = k << 1;
      if ((j < n-1) && (pq[j].min() > pq[j+1].min()))
        j++;
      if (pq[k].min() <= pq[j].min())
        break;
      std::swap(pq[k], pq[j]);
      k = j;
    }
  }

  template<class I>
  forceinline void
  PriorityQueue<I>::SharedPriorityQueue::fixup(int k) {
    while ((k > 0) && (pq[k >> 1].min() > pq[k].min())) {
      std::swap(pq[k],pq[k >> 1]);
      k >>= 1;
    }
  }

  template<class I>
  forceinline
  PriorityQueue<I>::PriorityQueue(void)
    : spq(NULL) {}

  template<class I>
  forceinline
  PriorityQueue<I>::PriorityQueue(int n)
    : spq(SharedPriorityQueue::allocate(n)) {}

  template<class I>
  forceinline void
  PriorityQueue<I>::init(int n) {
    spq = SharedPriorityQueue::allocate(n);
  }

  template<class I>
  forceinline
  PriorityQueue<I>::PriorityQueue(const PriorityQueue<I>& p)
    : spq(p.spq) {
    if (spq != NULL)
      spq->ref++;
  }

  template<class I>
  forceinline const PriorityQueue<I>&
  PriorityQueue<I>::operator =(const PriorityQueue<I>& p) {
    if (this != &p) {
      if ((spq != NULL) && (--spq->ref == 0))
        heap.rfree(spq);
      spq = p.spq;
      if (spq != NULL)
        spq->ref++;
    }
    return *this;
  }

  template<class I>
  forceinline
  PriorityQueue<I>::~PriorityQueue(void) {
    if (--spq->ref == 0)
      heap.rfree(spq);
  }

  template<class I>
  forceinline bool
  PriorityQueue<I>::empty(void) const {
    return (spq == NULL) || (spq->n == 0);
  }


  template<class I>
  forceinline void
  PriorityQueue<I>::insert(const I& x) {
    spq->pq[spq->n++] = x;
    spq->fixup(spq->n-1);
  }

  template<class I>
  forceinline void
  PriorityQueue<I>::remove(void) {
    spq->pq[0] = spq->pq[--spq->n];
    spq->fixdown();
  }

  template<class I>
  forceinline I&
  PriorityQueue<I>::top(void) {
    return spq->pq[0];
  }

  template<class I>
  forceinline void
  PriorityQueue<I>::fix(void) {
    spq->fixdown();
  }

  /**
   * \brief Range iterator for computing union (binary)
   *
   * \ingroup FuncIterRanges
   */

  template<class I, class J>
  class Union : public MinMax {
  protected:
    /// First iterator
    I i;
    /// Second iterator
    J j;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Union(void);
    /// Initialize with iterator \a i and \a j
    Union(I& i, J& j);
    /// Initialize with iterator \a i and \a j
    void init(I& i, J& j);
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}
  };


  /**
   * \brief Range iterator for union for any number of iterators
   * \ingroup FuncIterRanges
   */

  template<class I>
  class NaryUnion : public MinMax {
  protected:
    /// Priority queue to give access to next range
    PriorityQueue<I> r;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryUnion(void);
    /// Initialize with \a n iterators in \a i
    NaryUnion(I* i, int n);
    /// Initialize with \a n iterators in \a i
    void init(I* i, int n);
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}
  };



  /*
   * Binary union
   *
   */

  template<class I, class J>
  inline void
  Union<I,J>::operator ++(void) {
    if (!i() && !j()) {
      finish(); return;
    }
    if (!i()) {
      mi = j.min(); ma = j.max(); ++j; return;
    }
    if (!j()) {
      mi = i.min(); ma = i.max(); ++i; return;
    }
    if (i.min() < j.min()) {
      mi = i.min(); ma = i.max(); ++i;
    } else {
      mi = j.min(); ma = j.max(); ++j;
    }
    bool goOn;
    do {
      goOn = false;
      if (i() && (i.min() <= ma+1)) {
        ma = std::max(ma,i.max()); ++i; goOn=true;
      }
      if (j() && (j.min() <= ma+1)) {
        ma = std::max(ma,j.max()); ++j; goOn=true;
      }
    } while (goOn);
  }


  template<class I, class J>
  forceinline
  Union<I,J>::Union(void) {}

  template<class I, class J>
  forceinline
  Union<I,J>::Union(I& i0, J& j0)
    : i(i0), j(j0) {
    operator ++();
  }

  template<class I, class J>
  forceinline void
  Union<I,J>::init(I& i0, J& j0) {
    i = i0; j = j0;
    operator ++();
  }



  /*
   * Nary Union
   *
   */

  template<class I>
  inline void
  NaryUnion<I>::operator ++(void) {
    if (r.empty()) {
      finish(); return;
    }
    mi = r.top().min();
    ma = r.top().max();
    do {
      if (ma < r.top().max())
        ma = r.top().max();
      ++(r.top());
      if (!(r.top())()) {
        r.remove();
        if (r.empty())
          return;
      } else {
        r.fix();
      }
    } while (ma+1 >= r.top().min());
  }


  template<class I>
  forceinline
  NaryUnion<I>::NaryUnion(void) {}

  template<class I>
  inline
  NaryUnion<I>::NaryUnion(I* r0, int n)
    : r(n) {
    for (int i = n; i--; )
      if (r0[i]())
        r.insert(r0[i]);
    operator ++();
  }

  template<class I>
  inline void
  NaryUnion<I>::init(I* r0, int n) {
    r.init(n);
    for (int i = n; i--; )
      if (r0[i]())
        r.insert(r0[i]);
    operator ++();
  }

}}}

// STATISTICS: iter-any

