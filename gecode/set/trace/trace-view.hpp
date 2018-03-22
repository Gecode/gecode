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

namespace Gecode { namespace Set {

  /// Duplicate of a set view
  class SetTraceView {
  protected:
    /// Copy of the greatest lower bound
    RangeList* _glb;
    /// Copy of the least upper bound
    RangeList* _lub;
  public:
    /// Default constructor (initializes with no view)
    SetTraceView(void);
    /// Duplicate view \a x
    SetTraceView(Space& home, SetView x);
    /// Return range list for greatest lower bound
    RangeList* glb(void) const;
    /// Return range list for leat upper bound
    RangeList* lub(void) const;
    /// Update duplicated view from view \a y and delta \a d
    void prune(Space& home, SetView y, const Delta& d);
    /// Update during cloning
    void update(Space& home, SetTraceView x);
    /// Return slack for \a x
    static unsigned long long int slack(SetView x);
  };

  forceinline
  SetTraceView::SetTraceView(void) {}
  forceinline
  SetTraceView::SetTraceView(Space& home, SetView x) {
    GlbRanges<SetView> glbi(x);
    RangeList::copy(home,_glb,glbi);
    LubRanges<SetView> lubi(x);
    RangeList::copy(home,_lub,lubi);
  }
  forceinline RangeList*
  SetTraceView::glb(void) const {
    return _glb;
  }
  forceinline RangeList*
  SetTraceView::lub(void) const {
    return _lub;
  }
  forceinline void
  SetTraceView::prune(Space& home, SetView x, const Delta&) {
    GlbRanges<SetView> glbi(x);
    RangeList::overwrite(home,_glb,glbi);
    LubRanges<SetView> lubi(x);
    RangeList::overwrite(home,_lub,lubi);
  }
  forceinline void
  SetTraceView::update(Space& home, SetTraceView x) {
    Iter::Ranges::RangeList glbi(x._glb);
    RangeList::copy(home,_glb,glbi);
    Iter::Ranges::RangeList lubi(x._lub);
    RangeList::copy(home,_lub,lubi);
  }

  forceinline unsigned long long int
  SetTraceView::slack(SetView x) {
    return x.unknownSize();
  }

}}

// STATISTICS: set-trace
