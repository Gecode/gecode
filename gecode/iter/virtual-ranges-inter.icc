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

namespace Gecode { namespace Iter { namespace Ranges { namespace Virt {

  /**
   * \brief Range iterator for computing intersection (binary)
   *
   * \ingroup FuncIterRangesVirt
   */
  class Inter : public MinMax, public Iterator {
  private:
    Inter(const Inter&);
  protected:
    /// First iterator
    Iterator* i;
    /// Second iterator
    Iterator* j;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    Inter(void);
    /// Initialize with iterator \a i and \a j
    Inter(Iterator* i, Iterator* j);
    /// Destructor
    ~Inter(void);
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
    virtual void operator++(void);
    /// Test whether iterator is still at a range or done
    virtual bool operator()(void);
    //@}
  };


  /**
   * \brief Range iterator for intersection for any number of iterators
   *
   * \ingroup FuncIterRangesVirt
   */
  class NaryInter : public MinMax, public Iterator {
  private:
    NaryInter(const NaryInter&);
  protected:
    /// Array of iterators
    Iterator** is;
    /// Number of iterators
    int n;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    NaryInter(void);
    /// Initialize with \a n iterators in \a i
    NaryInter(Iterator** i, int n);
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
   * Binary intersection
   *
   */

  inline void
  Inter::operator++(void) {
    if (!(*i)() || !(*j)()) goto done;
    do {
      while ((*i)() && (i->max() < j->min())) ++(*i);
      if (!(*i)()) goto done;
      while ((*j)() && (j->max() < i->min())) ++(*j);
      if (!(*j)()) goto done;
    } while (i->max() < j->min());
    // Now the intervals overlap: consume the smaller interval
    ma = std::min(i->max(),j->max());
    mi = std::max(i->min(),j->min());
    if (i->max() < j->max()) ++(*i); else ++(*j);
    return;
  done:
    finish();
  }

  forceinline
  Inter::Inter(void) {}

  forceinline
  Inter::Inter(Iterator* i0, Iterator* j0)
    : i(i0), j(j0) {
    operator++();
  }

  forceinline
  Inter::~Inter(void) { delete i; delete j; }

  forceinline bool
  Inter::operator()(void) { return MinMax::operator()(); }

  forceinline int
  Inter::min(void) const { return MinMax::min(); }

  forceinline int
  Inter::max(void) const { return MinMax::max(); }

  forceinline unsigned int
  Inter::width(void) const { return MinMax::width(); }

  /*
   * Nary intersection
   *
   */

  inline void
  NaryInter::operator++(void) {
    // The next interval to be returned must have a hole
    // between it and the previously returned range.
    mi = ma+2;
    ma = is[0]->max();
    // Intersect with all other intervals
  restart:
    for (int i = n; i--;) {
      // Skip intervals that are too small
      while ((*(is[i]))() && (is[i]->max() < mi))
        ++(*(is[i]));
      if (!(*(is[i]))())
        goto done;

      if (is[i]->min() > ma) {
        mi=is[i]->min();
        ma=is[i]->max();
        goto restart;
      }
      // Now the intervals overlap
      if (mi < is[i]->min())
        mi = is[i]->min();
      if (ma > is[i]->max()) {
        ma = is[i]->max();
      }
    }
    return;
  done:
    finish();
  }

  forceinline
  NaryInter::NaryInter(void) {}

  inline
  NaryInter::NaryInter(Iterator** is0, int n0)
    : is(is0), n(n0) {
    if (!(*(is[0]))()) {
      finish();
    } else {
      ma=is[0]->min()-2;
      operator++();
    }
  }

  forceinline bool
  NaryInter::operator()(void) { return MinMax::operator()(); }

  forceinline int
  NaryInter::min(void) const { return MinMax::min(); }

  forceinline int
  NaryInter::max(void) const { return MinMax::max(); }

  forceinline unsigned int
  NaryInter::width(void) const { return MinMax::width(); }

}}}}

// STATISTICS: iter-any

