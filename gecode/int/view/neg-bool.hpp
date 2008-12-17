/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date: 2008-08-22 11:36:11 +0200 (Fri, 22 Aug 2008) $ by $Author: tack $
 *     $Revision: 7678 $
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
     * Negated Boolean views
     *
     */

    /*
     * Constructors and initialization
     *
     */
    forceinline
    NegBoolView::NegBoolView(void) {}
    forceinline
    NegBoolView::NegBoolView(const BoolView& b)
      : DerivedViewBase<BoolView>(b) {}
    forceinline void
    NegBoolView::init(const BoolView& b) {
      view = b;
    }
    forceinline
    NegBoolView::NegBoolView(Space& home, const Reflection::VarMap& vars,
                             Reflection::Arg* arg)
    : DerivedViewBase<BoolView>(BoolView(home, vars, arg)) {}


    /*
     * Boolean domain tests
     *
     */
    forceinline BoolStatus
    NegBoolView::status(void) const {
      return view.status();
    }
    forceinline bool
    NegBoolView::zero(void) const {
      return view.one();
    }
    forceinline bool
    NegBoolView::one(void) const {
      return view.zero();
    }
    forceinline bool
    NegBoolView::none(void) const {
      return view.none();
    }


    /*
     * Boolean assignment operations
     *
     */
    forceinline ModEvent
    NegBoolView::zero_none(Space& home) {
      return view.one_none(home);
    }
    forceinline ModEvent
    NegBoolView::one_none(Space& home) {
      return view.zero_none(home);
    }

    forceinline ModEvent
    NegBoolView::zero(Space& home) {
      return view.one(home);
    }
    forceinline ModEvent
    NegBoolView::one(Space& home) {
      return view.zero(home);
    }


    /*
     * Value access
     *
     */
    forceinline int
    NegBoolView::min(void) const {
      return view.max();
    }
    forceinline int
    NegBoolView::max(void) const {
      return view.min();
    }
    forceinline int
    NegBoolView::val(void) const {
      return 1-view.val();
    }


    /*
     * Domain tests
     *
     */
    forceinline bool
    NegBoolView::assigned(void) const {
      return view.assigned();
    }

    /*
     * Propagator modification events
     *
     */
    forceinline void
    NegBoolView::schedule(Space& home, Propagator& p, ModEvent me) {
      return BoolView::schedule(home,p,me);
    }
    forceinline ModEvent
    NegBoolView::me(const ModEventDelta& med) {
      return BoolView::me(med);
    }
    forceinline ModEventDelta
    NegBoolView::med(ModEvent me) {
      return BoolView::med(me);
    }


    /*
     * Dependencies
     *
     */
    forceinline void
    NegBoolView::subscribe(Space& home, Propagator& p, PropCond pc,
                           bool process) {
      view.subscribe(home,p,pc,process);
    }
    forceinline void
    NegBoolView::cancel(Space& home, Propagator& p, PropCond pc) {
      view.cancel(home,p,pc);
    }
    forceinline void
    NegBoolView::subscribe(Space& home, Advisor& a) {
      view.subscribe(home,a);
    }
    forceinline void
    NegBoolView::cancel(Space& home, Advisor& a) {
      view.cancel(home,a);
    }


    /*
     * Delta information for advisors
     *
     */
    forceinline ModEvent
    NegBoolView::modevent(const Delta& d) {
      return BoolView::modevent(d);
    }
    forceinline int
    NegBoolView::min(const Delta& d) const {
      return view.max(d);
    }
    forceinline int
    NegBoolView::max(const Delta& d) const {
      return view.min(d);
    }
    forceinline bool
    NegBoolView::any(const Delta& d) const {
      return view.any(d);
    }
    forceinline bool
    NegBoolView::zero(const Delta& d) {
      return BoolView::one(d);
    }
    forceinline bool
    NegBoolView::one(const Delta& d) {
      return BoolView::zero(d);
    }



    /*
     * Cloning
     *
     */
    forceinline void
    NegBoolView::update(Space& home, bool share, NegBoolView& b) {
      view.update(home,share,b.view);
    }

    /*
     * Serialization
     *
     */
    forceinline Reflection::Arg*
    NegBoolView::spec(const Space& home, Reflection::VarMap& m) const {
      return view.spec(home, m);
    }
    inline Support::Symbol
    NegBoolView::type(void) {
      return Support::Symbol("Gecode::Int::NegBoolView");
    }

    /**
     * \brief %Range iterator for negated Boolean variable views
     * \ingroup TaskActorIntView
     */
    template <>
    class ViewRanges<NegBoolView> : public Iter::Ranges::Singleton {
    public:
      /// \name Constructors and initialization
      //@{
      /// Default constructor
      ViewRanges(void);
      /// Initialize with ranges for view \a x
      ViewRanges(const NegBoolView& x);
      /// Initialize with ranges for view \a x
      void init(const NegBoolView& x);
      //@}
    };

    forceinline
    ViewRanges<NegBoolView>::ViewRanges(void) {}

    forceinline
    ViewRanges<NegBoolView>::ViewRanges(const NegBoolView& x)
      : Iter::Ranges::Singleton(x.min(),x.max()) {}

    forceinline void
    ViewRanges<NegBoolView>::init(const NegBoolView& x) {
      Iter::Ranges::Singleton::init(x.min(),x.max());
    }

  }


  /*
   * View comparison
   *
   */
  forceinline bool
  same(const Int::NegBoolView& x, const Int::NegBoolView& y) {
    return same(x.base(),y.base());
  }
  forceinline bool
  before(const Int::NegBoolView& x, const Int::NegBoolView& y) {
    return before(x.base(),y.base());
  }

}

// STATISTICS: int-var
