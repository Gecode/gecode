/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

namespace Gecode { namespace Int {

  /// Duplicate of an integer view
  class IntTraceView {
  protected:
    /// Ranges capturing the variable domain
    RangeList* dom;
  public:
    /// Default constructor (initializes with no view)
    IntTraceView(void);
    /// Duplicate view \a y
    IntTraceView(Space& home, IntView y);
    /// Give access to ranges
    RangeList* ranges(void) const;
    /// Update duplicated view from view \a y and modification delta \a d
    void prune(Space& home, IntView y, const Delta& d);
    /// Update during cloning
    void update(Space& home, IntTraceView x);
    /// Return slack measure
    static unsigned long long int slack(IntView x);
  };

  forceinline
  IntTraceView::IntTraceView(void) {}

  forceinline
  IntTraceView::IntTraceView(Space& home, IntView y) {
    ViewRanges<IntView> yr(y);
    RangeList::copy(home, dom, yr);
  }

  forceinline RangeList*
  IntTraceView::ranges(void) const {
    return dom;
  }

  forceinline void
  IntTraceView::prune(Space& home, IntView y, const Delta& d) {
    if (y.range() && (dom->next() == NULL)) {
      dom->min(y.min()); dom->max(y.max());
    } else if (!y.any(d) && (y.max(d)+1 == y.min())) {
      // The lower bound has been adjusted
      if (y.min() > dom->max()) {
        RangeList* p = dom;
        RangeList* l = p->next();
        while ((l != NULL) && (l->max() < y.min())) {
          p=l; l=l->next();
        }
        dom->dispose(home,p);
        dom = l;
      }
      dom->min(y.min());
    } else if (!y.any(d) && (y.max()+1 == y.min(d))) {
      // upper bound has been adjusted
      if ((y.max() <= dom->max()) && (dom->next() == NULL)) {
        dom->max(y.max());
      } else {
        RangeList* p = dom;
        RangeList* l = p->next();
        while ((l != NULL) && (l->min() <= y.max())) {
          p=l; l=l->next();
        }
        p->max(y.max());
        if (p->next() != NULL)
          p->next()->dispose(home);
        p->next(NULL);
      }
    } else {
      // Just copy the domain
      ViewRanges<IntView> yr(y);
      RangeList::overwrite(home,dom,yr);
    }
  }

  forceinline void
  IntTraceView::update(Space& home, IntTraceView y) {
    Iter::Ranges::RangeList yr(y.dom);
    RangeList::copy(home,dom,yr);
  }

  forceinline unsigned long long int
  IntTraceView::slack(IntView x) {
    return x.width()-1;
  }


}}

// STATISTICS: int-trace
