/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

namespace Gecode { namespace Int {

  /*
   * Support functions for division
   *
   */
  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::floor_div(double y) const {
    return static_cast<Val>(floor(y / a));
  }

  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::ceil_div(double y) const {
    return static_cast<Val>(ceil(y / a));
  }

  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::exact_div(double y, bool& exact) const {
    double ya = y / a;
    if (ceil(ya) == ya) {
      exact = true;  return static_cast<Val>(ya);
    } else {
      exact = false; return 0;
    }
  }

#if GECODE_INT_RND_TWDS_ZERO

  template<class Val, class UnsVal>
  forceinline int
  ScaleView<Val,UnsVal>::floor_div(int y) const {
    return ((y >= 0) ? y : (y-a+1))/a;
  }

  template<class Val, class UnsVal>
  forceinline int
  ScaleView<Val,UnsVal>::ceil_div(int y) const {
    return ((y >= 0) ? (y+a-1) : y)/a;
  }

  template<class Val, class UnsVal>
  forceinline int
  ScaleView<Val,UnsVal>::exact_div(int y, bool& exact) const {
    int ya = y / a;
    if (a * ya == y) {
      exact = true;  return ya;
    } else {
      exact = false; return 0;
    }
  }

#endif


  /*
   * Constructors and initialization
   *
   */
  template<class Val, class UnsVal>
  forceinline
  ScaleView<Val,UnsVal>::ScaleView(void) {}

  template<class Val, class UnsVal>
  forceinline
  ScaleView<Val,UnsVal>::ScaleView(int b, const IntView& y)
    : DerivedView<IntView>(y), a(b) {}


  /*
   * Value access
   *
   */
  template<class Val, class UnsVal>
  forceinline int
  ScaleView<Val,UnsVal>::scale(void) const {
    return a;
  }
  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::min(void) const {
    Val c = x.min(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::max(void) const {
    Val c = x.max(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::med(void) const {
    Val c = x.med(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::val(void) const {
    Val c = x.val(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline UnsVal
  ScaleView<Val,UnsVal>::size(void) const {
    return static_cast<UnsVal>(x.size());
  }

  template<class Val, class UnsVal>
  forceinline UnsVal
  ScaleView<Val,UnsVal>::width(void) const {
    UnsVal c = x.width(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline UnsVal
  ScaleView<Val,UnsVal>::regret_min(void) const {
    UnsVal c = x.regret_min(); c *= a; return c;
  }

  template<class Val, class UnsVal>
  forceinline UnsVal
  ScaleView<Val,UnsVal>::regret_max(void) const {
    UnsVal c = x.regret_max(); c *= a; return c;
  }


  /*
   * Domain tests
   *
   */
  template<class Val, class UnsVal>
  forceinline bool
  ScaleView<Val,UnsVal>::range(void) const {
    return x.range();
  }
  template<class Val, class UnsVal>
  forceinline bool
  ScaleView<Val,UnsVal>::in(Val n) const {
    bool exact;
    int nda = exact_div(n, exact);
    return exact && x.in(nda);
  }




  /*
   * Domain update by value
   *
   */
  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::lq(Space& home, Val n) {
    return (n >= max()) ? ME_INT_NONE : x.lq(home,floor_div(n));
  }

  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::le(Space& home, Val n) {
    return (n > max()) ? ME_INT_NONE : x.le(home,floor_div(n));
  }

  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::gq(Space& home, Val n) {
    return (n <= min()) ? ME_INT_NONE : x.gq(home,ceil_div(n));
  }
  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::gr(Space& home, Val n) {
    return (n < min()) ? ME_INT_NONE : x.gr(home,ceil_div(n));
  }

  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::nq(Space& home, Val n) {
    bool exact;
    int nda = exact_div(n,exact);
    return exact ? x.nq(home,nda) :  ME_INT_NONE;
  }

  template<class Val, class UnsVal>
  forceinline ModEvent
  ScaleView<Val,UnsVal>::eq(Space& home, Val n) {
    bool exact;
    int nda = exact_div(n,exact);
    return exact ? x.eq(home,nda) : ME_INT_FAILED;
  }


  /*
   * Propagator modification events
   *
   */
  template<class Val, class UnsVal>
  forceinline ModEventDelta
  ScaleView<Val,UnsVal>::med(ModEvent me) {
    return IntView::med(me);
  }



  /*
   * Delta information for advisors
   *
   */
  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::min(const Delta& d) const {
    Val c = x.min(d); c *= a; return c;
  }
  template<class Val, class UnsVal>
  forceinline Val
  ScaleView<Val,UnsVal>::max(const Delta& d) const {
    Val c = x.max(d); c *= a; return c;
  }
  template<class Val, class UnsVal>
  forceinline bool
  ScaleView<Val,UnsVal>::any(const Delta& d) const {
    return x.any(d);
  }



  /*
   * Cloning
   *
   */
  template<class Val, class UnsVal>
  forceinline void
  ScaleView<Val,UnsVal>::update(Space& home, bool share,
                                ScaleView<Val,UnsVal>& y) {
    DerivedView<IntView>::update(home,share,y);
    a=y.a;
  }



  /**
   * \brief %Range iterator for integer-precision scale integer views
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewRanges<IntScaleView>
    : public Iter::Ranges::ScaleUp<int,unsigned int,ViewRanges<IntView> > {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ViewRanges(void);
    /// Initialize with ranges for view \a x
    ViewRanges(const IntScaleView& x);
    /// Initialize with ranges for view \a x
    void init(const IntScaleView& x);
    //@}
  };

  forceinline
  ViewRanges<IntScaleView>::ViewRanges(void) {}
  forceinline
  ViewRanges<IntScaleView>::ViewRanges(const IntScaleView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::ScaleUp<int,unsigned int,ViewRanges<IntView> >::init
      (xi,x.scale());
  }
  forceinline void
  ViewRanges<IntScaleView>::init(const IntScaleView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::ScaleUp<int,unsigned int,ViewRanges<IntView> >::init
      (xi,x.scale());
  }


  /**
   * \brief %Range iterator for double-precision scale integer views
   * \ingroup TaskActorIntView
   */
  template<>
  class ViewRanges<DoubleScaleView>
    : public Iter::Ranges::ScaleUp<double,double,ViewRanges<IntView> > {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ViewRanges(void);
    /// Initialize with ranges for view \a x
    ViewRanges(const DoubleScaleView& x);
    /// Initialize with ranges for view \a x
    void init(const DoubleScaleView& x);
    //@}
  };

  forceinline
  ViewRanges<DoubleScaleView>::ViewRanges(void) {}
  forceinline
  ViewRanges<DoubleScaleView>::ViewRanges(const DoubleScaleView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::ScaleUp<double,double,ViewRanges<IntView> >::init
      (xi,x.scale());
  }
  forceinline void
  ViewRanges<DoubleScaleView>::init(const DoubleScaleView& x) {
    ViewRanges<IntView> xi(x.base());
    Iter::Ranges::ScaleUp<double,double,ViewRanges<IntView> >::init
      (xi,x.scale());
  }


  /*
   * View comparison
   *
   */
  template<class Val, class UnsVal>
  forceinline bool
  same(const ScaleView<Val,UnsVal>& x, const ScaleView<Val,UnsVal>& y) {
    return same(x.base(),y.base()) && (x.scale() == y.scale());
  }
  template<class Val, class UnsVal>
  forceinline bool
  before(const ScaleView<Val,UnsVal>& x, const ScaleView<Val,UnsVal>& y) {
    return before(x.base(),y.base())
      || (same(x.base(),y.base()) && (x.scale() < y.scale()));
  }

}}

// STATISTICS: int-var

