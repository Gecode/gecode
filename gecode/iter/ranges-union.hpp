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
   *
   * \ingroup FuncIterRanges
   */
  template<class I>
  class NaryUnion : public RangeListIter {
  protected:
    /// Create new range possibly from freelist \a f and init
    RangeList* range(RangeList*& f, int min, int max);
    /// Create new range possibly from freelist \a f and init
    RangeList* range(RangeList*& f, I& i);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryUnion(void);
    /// Initialize with \a n iterators in \a i
    NaryUnion(Region& r, I* i, int n);
    /// Initialize with \a n iterators in \a i
    void init(Region& r, I* i, int n);
    /// Assignment operator
    NaryUnion& operator =(const NaryUnion& m);
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

    if (!i() || (j() && (j.max()+1 < i.min()))) {
      mi = j.min(); ma = j.max(); ++j; return;
    }
    if (!j() || (i() && (i.max()+1 < j.min()))) {
      mi = i.min(); ma = i.max(); ++i; return;
    }

    mi = std::min(i.min(),j.min());
    ma = std::max(i.max(),j.max());

    ++i; ++j;

  next:
    if (i() && (i.min() <= ma+1)) {
      ma = std::max(ma,i.max()); ++i;
      goto next;
    }
    if (j() && (j.min() <= ma+1)) {
      ma = std::max(ma,j.max()); ++j;
      goto next;
    }
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
  forceinline
  NaryUnion<I>::NaryUnion(void) {}

  template<class I>
  forceinline RangeListIter::RangeList*
  NaryUnion<I>::range(RangeList*& f, int min, int max) {
    RangeList* t;
    // Take element from freelist if possible
    if (f != NULL) {
      t = f; f = f->next;
    } else {
      t = new (*rlio) RangeList;
    }
    t->min = min; t->max = max;
    return t;
  }

  template<class I>
  forceinline RangeListIter::RangeList*
  NaryUnion<I>::range(RangeList*& f, I& i) {
    return range(f,i.min(),i.max());
  }

  template<class I>
  void
  NaryUnion<I>::init(Region& r, I* i, int n) {
    RangeListIter::init(r);

    // Freelist for allocation
    RangeList* f = NULL;

    // Rangelist for union
    RangeList* u = NULL;

    int m = 0;
    while ((m < n) && !i[m]())
      m++;

    // Union is empty
    if (m >= n)
      return;

    n--;
    while (!i[n]())
      n--;

    if (m == n) {
      // Union is just a single iterator
      RangeList** c = &u;
      
      for ( ; i[m](); ++i[m]) {
        RangeList* t = range(f,i[m]);
        *c = t; c = &t->next;
      }
      *c = NULL;
    } else {
      // At least two iterators

      // Compute the union for just two iterators
      {
        // The current rangelist
        RangeList** c = &u;
        
        while (i[m]() && i[n]())
          if (i[m].max()+1 < i[n].min()) {
            RangeList* t = range(f,i[m]); ++i[m];
            *c = t; c = &t->next;
          } else if (i[n].max()+1 < i[m].min()) {
            RangeList* t = range(f,i[n]); ++i[n];
            *c = t; c = &t->next;
          } else {
            int min = std::min(i[m].min(),i[n].min());
            int max = std::max(i[m].max(),i[n].max());
            ++i[m]; ++i[n];

          nexta:
            if (i[m]() && (i[m].min() <= max+1)) {
              max = std::max(max,i[m].max()); ++i[m];
              goto nexta;
            }
            if (i[n]() && (i[n].min() <= max+1)) {
              max = std::max(max,i[n].max()); ++i[n];
              goto nexta;
            }
 
            RangeList* t = range(f,min,max);
            *c = t; c = &t->next;
          }
        for ( ; i[m](); ++i[m]) {
          RangeList* t = range(f,i[m]);
          *c = t; c = &t->next;
        }
        for ( ; i[n](); ++i[n]) {
          RangeList* t = range(f,i[n]);
          *c = t; c = &t->next;
        }
        *c = NULL;
        m++; n--;
      }

      // Insert the remaining iterators
      for ( ; m<=n; m++) {
        // The current rangelist
        RangeList** c = &u;
        
        while ((*c != NULL) && i[m]())
          if ((*c)->max+1 < i[m].min()) {
            // Keep range from union
            c = &(*c)->next;
          } else if (i[m].max()+1 < (*c)->min) {
            // Copy range from iterator
            RangeList* t = range(f,i[m]); ++i[m];
            // Insert
            t->next = *c; *c = t; c = &t->next;
          } else {
            // Ranges overlap
            // Compute new minimum
            (*c)->min = std::min((*c)->min,i[m].min());
            // Compute new maximum
            int max = std::max((*c)->max,i[m].max());
            
            // Scan from the next range in the union
            RangeList* s = (*c)->next; 
            ++i[m];
            
          nextb:
            if ((s != NULL) && (s->min <= max+1)) {
              max = std::max(max,s->max);
              RangeList* t = s;
              s = s->next;
              // Put deleted element into freelist
              t->next = f; f = t;
              goto nextb;
            }
            if (i[m]() && (i[m].min() <= max+1)) {
              max = std::max(max,i[m].max()); ++i[m];
              goto nextb;
            }
            // Store computed max and shunt skipped ranges from union
            (*c)->max = max; (*c)->next = s;
          }
        if (*c == NULL) {
          // Copy remaining ranges from iterator
          for ( ; i[m](); ++i[m]) {
            RangeList* t = range(f,i[m]);
            *c = t; c = &t->next;
          }
          *c = NULL;
        }
      }
    }
    set(u);
  }

  template<class I>
  forceinline
  NaryUnion<I>::NaryUnion(Region& r, I* i, int n) {
    init(r,i,n);
  }

  template<class I>
  forceinline NaryUnion<I>&
  NaryUnion<I>::operator =(const NaryUnion<I>& m) {
    return static_cast<NaryUnion&>(RangeListIter::operator =(m));
  }

}}}

// STATISTICS: iter-any

