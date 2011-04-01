/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2011
 *
 *  Last modified:
 *     $Date: 2010-10-06 22:42:28 +0200 (wo, 06 okt 2010) $ by $Author: schulte $
 *     $Revision: 11464 $
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

  /*
   * Constructors and initialization
   *
   */
  forceinline
  OffsetRefView::OffsetRefView(void) : c(NULL) {}
  forceinline
  OffsetRefView::OffsetRefView(const IntView& y, int& d)
    : DerivedView<IntView>(y), c(&d) {}


  /*
   * Value access
   *
   */
  forceinline int
  OffsetRefView::offset(void) const {
    return *c;
  }
  forceinline int
  OffsetRefView::min(void) const {
    return x.min()+*c;
  }
  forceinline int
  OffsetRefView::max(void) const {
    return x.max()+*c;
  }
  forceinline int
  OffsetRefView::med(void) const {
    return x.med()+*c;
  }
  forceinline int
  OffsetRefView::val(void) const {
    return x.val()+*c;
  }

  forceinline unsigned int
  OffsetRefView::width(void) const {
    return x.width();
  }
  forceinline unsigned int
  OffsetRefView::size(void) const {
    return x.size();
  }
  forceinline unsigned int
  OffsetRefView::regret_min(void) const {
    return x.regret_min();
  }
  forceinline unsigned int
  OffsetRefView::regret_max(void) const {
    return x.regret_max();
  }

  /*
   * Domain tests
   *
   */
  forceinline bool
  OffsetRefView::range(void) const {
    return x.range();
  }
  forceinline bool
  OffsetRefView::in(int n) const {
    return x.in(n-*c);
  }
  forceinline bool
  OffsetRefView::in(double n) const {
    return x.in(n-*c);
  }


  /*
   * Domain update by value
   *
   */
  forceinline ModEvent
  OffsetRefView::lq(Space& home, int n) {
    return x.lq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::lq(Space& home, double n) {
    return x.lq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::le(Space& home, int n) {
    return x.le(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::le(Space& home, double n) {
    return x.le(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::gq(Space& home, int n) {
    return x.gq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::gq(Space& home, double n) {
    return x.gq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::gr(Space& home, int n) {
    return x.gr(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::gr(Space& home, double n) {
    return x.gr(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::nq(Space& home, int n) {
    return x.nq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::nq(Space& home, double n) {
    return x.nq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::eq(Space& home, int n) {
    return x.eq(home,n-*c);
  }
  forceinline ModEvent
  OffsetRefView::eq(Space& home, double n) {
    return x.eq(home,n-*c);
  }


  /*
   * Iterator-based domain update
   *
   */
  template<class I>
  forceinline ModEvent
  OffsetRefView::narrow_r(Space& home, I& i, bool depend) {
    Iter::Ranges::Offset<I> oi(i,-*c);
    return x.narrow_r(home,oi,depend);
  }
  template<class I>
  forceinline ModEvent
  OffsetRefView::inter_r(Space& home, I& i, bool depend) {
    Iter::Ranges::Offset<I> oi(i,-*c);
    return x.inter_r(home,oi,depend);
  }
  template<class I>
  forceinline ModEvent
  OffsetRefView::minus_r(Space& home, I& i, bool depend) {
    Iter::Ranges::Offset<I> oi(i,-*c);
    return x.minus_r(home,oi,depend);
  }
  template<class I>
  forceinline ModEvent
  OffsetRefView::narrow_v(Space& home, I& i, bool depend) {
    Iter::Values::Offset<I> oi(i,-*c);
    return x.narrow_v(home,oi,depend);
  }
  template<class I>
  forceinline ModEvent
  OffsetRefView::inter_v(Space& home, I& i, bool depend) {
    Iter::Values::Offset<I> oi(i,-*c);
    return x.inter_v(home,oi,depend);
  }
  template<class I>
  forceinline ModEvent
  OffsetRefView::minus_v(Space& home, I& i, bool depend) {
    Iter::Values::Offset<I> oi(i,-*c);
    return x.minus_v(home,oi,depend);
  }



  /*
   * Propagator modification events
   *
   */
  forceinline ModEventDelta
  OffsetRefView::med(ModEvent me) {
    return IntView::med(me);
  }


  /*
   * Delta information for advisors
   *
   */
  forceinline int
  OffsetRefView::min(const Delta& d) const {
    return x.min(d)+*c;
  }
  forceinline int
  OffsetRefView::max(const Delta& d) const {
    return x.max(d)+*c;
  }
  forceinline bool
  OffsetRefView::any(const Delta& d) const {
    return x.any(d);
  }



  /*
   * Cloning
   *
   */
  forceinline void
  OffsetRefView::update(Space& home, bool share, OffsetRefView& y) {
    DerivedView<IntView>::update(home,share,y);
    c=y.c;
  }


  /**
   * \brief %Range iterator for offset integer views
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewRanges<OffsetRefView>
    : public Iter::Ranges::Offset<ViewRanges<IntView> > {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ViewRanges(void);
    /// Initialize with ranges for view \a x
    ViewRanges(const OffsetRefView& x);
    /// Initialize with ranges for view \a x
    void init(const OffsetRefView& x);
    //@}
  };

  forceinline
  ViewRanges<OffsetRefView>::ViewRanges(void) {}

  forceinline
  ViewRanges<OffsetRefView>::ViewRanges(const OffsetRefView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::Offset<ViewRanges<IntView> >::init(xi,x.offset());
  }

  forceinline void
  ViewRanges<OffsetRefView>::init(const OffsetRefView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::Offset<ViewRanges<IntView> >::init(xi,x.offset());
  }

  /*
   * View comparison
   *
   */
  forceinline bool
  same(const OffsetRefView& x, const OffsetRefView& y) {
    return same(x.base(),y.base()) && (x.offset() == y.offset());
  }
  forceinline bool
  before(const OffsetRefView& x, const OffsetRefView& y) {
    return before(x.base(),y.base())
      || (same(x.base(),y.base()) && (x.offset() < y.offset()));
  }

}}

// STATISTICS: int-var
