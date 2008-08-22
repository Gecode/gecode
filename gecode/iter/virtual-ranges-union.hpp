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

namespace Gecode { namespace Iter { namespace Ranges {  namespace Virt {

  /**
   * \brief Range iterator for computing union (binary)
   *
   * \ingroup FuncIterRangesVirt
   */

  class Union : public MinMax, public Iterator {
    /// First iterator
    Iterator* i;
    /// Second iterator
    Iterator* j;
    Union(const Union&);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Union(void);
    /// Initialize with iterator \a i and \a j
    Union(Iterator* i, Iterator* j);
    /// Destructor
    ~Union(void);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    virtual int min(void) const;
    /// Return largest value of range
    virtual int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    virtual unsigned int width(void) const;
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator++(void);
    /// Test whether iterator is still at a range or done
    virtual bool operator()(void);
    //@}
  };


  /**
   * \brief Range iterator for union for any number of iterators
   * \ingroup FuncIterRangesVirt
   */

  class NaryUnion : public MinMax, public Iterator {
  private:
    NaryUnion(const NaryUnion&);
  protected:
    /// Order for iterators: by increasing minimum of next range
    class RangeUnionOrder {
    public:
      bool operator()(const Iterator*, const Iterator*) const;
    };
    /// Instance for order
    RangeUnionOrder order;
    /// Priority queue to give access to next range
    PriorityQueue<Iterator*,RangeUnionOrder> r;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryUnion(void);
    /// Initialize with \a n iterators in \a i
    NaryUnion(Iterator** i, int n);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    virtual int min(void) const;
    /// Return largest value of range
    virtual int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    virtual unsigned int width(void) const;
    //@}

    /// \name Iteration control
    //@{
    /// Move iterator to next range (if possible)
    void operator++(void);
    /// Test whether iterator is still at a range or done
    virtual bool operator()(void);
    //@}
  };



  /*
   * Binary union
   *
   */

  forceinline void
  Union::operator++(void) {
    if (!(*i)() && !(*j)()) {
      finish(); return;
    }
    if (!(*i)()) {
      mi = j->min(); ma = j->max(); ++(*j); return;
    }
    if (!(*j)()) {
      mi = i->min(); ma = i->max(); ++(*i); return;
    }
    if (i->min() < j->min()) {
      mi = i->min(); ma = i->max(); ++(*i);
    } else {
      mi = j->min(); ma = j->max(); ++(*j);
    }
    bool goOn;
    do {
      goOn = false;
      if ((*i)() && (i->min() <= ma+1)) {
        ma = std::max(ma,i->max()); ++(*i); goOn=true;
      }
      if ((*j)() && (j->min() <= ma+1)) {
        ma = std::max(ma,j->max()); ++(*j); goOn=true;
      }
    } while (goOn);
  }


  forceinline
  Union::Union(void) {}

  forceinline
  Union::Union(Iterator* i0, Iterator* j0)
    : i(i0), j(j0) {
    operator++();
  }

  forceinline
  Union::~Union(void) { delete i; delete j; }

  forceinline bool
  Union::operator()(void) { return MinMax::operator()(); }

  forceinline int
  Union::min(void) const { return MinMax::min(); }

  forceinline int
  Union::max(void) const { return MinMax::max(); }

  forceinline unsigned int
  Union::width(void) const { return MinMax::width(); }

  /*
   * Nary Union
   *
   */

  forceinline bool
  NaryUnion::RangeUnionOrder::operator()(const Iterator* a,
                                         const Iterator* b) const {
    return a->min() > b->min();
  }

  inline void
  NaryUnion::operator++(void) {
    if (r.empty()) {
      finish(); return;
    }
    mi = r.top()->min();
    ma = r.top()->max();
    do {
      if (ma < r.top()->max())
        ma = r.top()->max();
      ++(*(r.top()));
      if (!((*r.top()))()) {
        r.remove();
        if (r.empty())
          return;
      } else {
        r.fix();
      }
    } while (ma+1 >= r.top()->min());
  }


  forceinline
  NaryUnion::NaryUnion(void) {}

  inline
  NaryUnion::NaryUnion(Iterator** r0, int n)
    : order(), r(n,order) {
    for (int i = n; i--; )
      if ((*(r0[i]))())
        r.insert(r0[i]);
    operator++();
  }

  forceinline bool
  NaryUnion::operator()(void) { return MinMax::operator()(); }

  forceinline int
  NaryUnion::min(void) const { return MinMax::min(); }

  forceinline int
  NaryUnion::max(void) const { return MinMax::max(); }

  forceinline unsigned int
  NaryUnion::width(void) const { return MinMax::width(); }

}}}}

// STATISTICS: iter-any

