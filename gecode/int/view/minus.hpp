/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode {

  namespace Int {

    /*
     * Constructors and initialization
     *
     */
    forceinline
    MinusView::MinusView(void) {}
    forceinline
    MinusView::MinusView(const IntView& x)
      : DerivedViewBase<IntView>(x) {}
    forceinline void
    MinusView::init(const IntView& x) {
      view = x;
    }


    /*
     * Value access
     *
     */
    forceinline int
    MinusView::min(void) const {
      return -view.max();
    }
    forceinline int
    MinusView::max(void) const {
      return -view.min();
    }
    forceinline int
    MinusView::val(void) const {
      return -view.val();
    }

    forceinline unsigned int
    MinusView::width(void) const {
      return view.width();
    }
    forceinline unsigned int
    MinusView::size(void) const {
      return view.size();
    }
    forceinline unsigned int
    MinusView::regret_min(void) const {
      return view.regret_max();
    }
    forceinline unsigned int
    MinusView::regret_max(void) const {
      return view.regret_min();
    }


    /*
     * Domain tests
     *
     */
    forceinline bool
    MinusView::range(void) const {
      return view.range();
    }
    forceinline bool
    MinusView::in(int n) const {
      return view.in(-n);
    }
    forceinline bool
    MinusView::in(double n) const {
      return view.in(-n);
    }


    /*
     * Domain update by value
     *
     */
    forceinline ModEvent
    MinusView::lq(Space& home, int n) {
      return view.gq(home,-n);
    }
    forceinline ModEvent
    MinusView::lq(Space& home, double n) {
      return view.gq(home,-n);
    }
    forceinline ModEvent
    MinusView::le(Space& home, int n) {
      return view.gr(home,-n);
    }
    forceinline ModEvent
    MinusView::le(Space& home, double n) {
      return view.gr(home,-n);
    }
    forceinline ModEvent
    MinusView::gq(Space& home, int n) {
      return view.lq(home,-n);
    }
    forceinline ModEvent
    MinusView::gq(Space& home, double n) {
      return view.lq(home,-n);
    }
    forceinline ModEvent
    MinusView::gr(Space& home, int n) {
      return view.le(home,-n);
    }
    forceinline ModEvent
    MinusView::gr(Space& home, double n) {
      return view.le(home,-n);
    }
    forceinline ModEvent
    MinusView::nq(Space& home, int n) {
      return view.nq(home,-n);
    }
    forceinline ModEvent
    MinusView::nq(Space& home, double n) {
      return view.nq(home,-n);
    }
    forceinline ModEvent
    MinusView::eq(Space& home, int n) {
      return view.eq(home,-n);
    }
    forceinline ModEvent
    MinusView::eq(Space& home, double n) {
      return view.eq(home,-n);
    }


    /*
     * Iterator-based domain update
     *
     */
    template<class I>
    forceinline ModEvent
    MinusView::narrow_r(Space& home, I& i, bool) {
      Iter::Ranges::Minus<I> mi(i);
      return view.narrow_r(home,mi,false);
    }
    template<class I>
    forceinline ModEvent
    MinusView::inter_r(Space& home, I& i, bool) {
      Iter::Ranges::Minus<I> mi(i);
      return view.inter_r(home,mi,false);
    }
    template<class I>
    forceinline ModEvent
    MinusView::minus_r(Space& home, I& i, bool) {
      Iter::Ranges::Minus<I> mi(i);
      return view.minus_r(home,mi,false);
    }
    template<class I>
    forceinline ModEvent
    MinusView::narrow_v(Space& home, I& i, bool) {
      Iter::Values::Minus<I> mi(i);
      return view.narrow_v(home,mi,false);
    }
    template<class I>
    forceinline ModEvent
    MinusView::inter_v(Space& home, I& i, bool) {
      Iter::Values::Minus<I> mi(i);
      return view.inter_v(home,mi,false);
    }
    template<class I>
    forceinline ModEvent
    MinusView::minus_v(Space& home, I& i, bool) {
      Iter::Values::Minus<I> mi(i);
      return view.minus_v(home,mi,false);
    }


    /*
     * Propagator modification events
     *
     */
    forceinline ModEventDelta
    MinusView::med(ModEvent me) {
      return IntView::med(me);
    }


    /*
     * Delta information for advisors
     *
     */
    forceinline ModEvent
    MinusView::modevent(const Delta& d) {
      return IntView::modevent(d);
    }
    forceinline int
    MinusView::min(const Delta& d) const {
      return -view.max(d);
    }
    forceinline int
    MinusView::max(const Delta& d) const {
      return -view.min(d);
    }
    forceinline bool
    MinusView::any(const Delta& d) const {
      return view.any(d);
    }



    /*
     * Cloning
     *
     */

    forceinline void
    MinusView::update(Space& home, bool share, MinusView& x) {
      view.update(home,share,x.view);
    }


    /**
     * \brief %Range iterator for minus integer views
     * \ingroup TaskActorIntView
     */
    template<>
    class ViewRanges<MinusView> : public IntVarImpBwd {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ViewRanges(void);
      /// Initialize with ranges for view \a x
      ViewRanges(const MinusView& x);
      /// Initialize with ranges for view \a x
      void init(const MinusView& x);
      //@}

      /// \name Range access
      //@{
      /// Return smallest value of range
      int min(void) const;
      /// Return largest value of range
      int max(void) const;
      //@}
    };

    forceinline
    ViewRanges<MinusView>::ViewRanges(void) {}

    forceinline
    ViewRanges<MinusView>::ViewRanges(const MinusView& x)
      : IntVarImpBwd(x.base().var()) {}

    forceinline void
    ViewRanges<MinusView>::init(const MinusView& x) {
      IntVarImpBwd::init(x.base().var());
    }

    forceinline int
    ViewRanges<MinusView>::min(void) const {
      return -IntVarImpBwd::max();
    }
    forceinline int
    ViewRanges<MinusView>::max(void) const {
      return -IntVarImpBwd::min();
    }

    inline int
    MinusView::med(void) const {
      if (view.range())
        return (min()+max())/2 - ((min()+max())%2 < 0 ? 1 : 0);

      unsigned int i = view.size() / 2;
      if (size() % 2 == 0)
        i--;
      ViewRanges<MinusView> r(*this);
      while (i >= r.width()) {
        i -= r.width();
        ++r;
      }
      return r.min() + static_cast<int>(i);
    }

  }


  /*
   * View comparison
   *
   */
  forceinline bool
  same(const Int::MinusView& x, const Int::MinusView& y) {
    return same(x.base(),y.base());
  }
  forceinline bool
  before(const Int::MinusView& x, const Int::MinusView& y) {
    return before(x.base(),y.base());
  }

}

// STATISTICS: int-var

