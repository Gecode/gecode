/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode { namespace Set {

  /*
   * Constructors and access
   *
   */

  forceinline
  SetView::SetView(void) {}
  forceinline
  SetView::SetView(const SetVar& y)
    : VarViewBase<SetVarImp>(y.var()) {}
  forceinline
  SetView::SetView(SetVarImp* y)
    : VarViewBase<SetVarImp>(y) {}

  /*
   * Variable information
   *
   */

  forceinline bool
  SetView::assigned(void)  const {
    return varimp->assigned();
  }

  forceinline unsigned int
  SetView::glbSize(void) const { return varimp->glbSize(); }

  forceinline unsigned int
  SetView::lubSize(void) const { return varimp->lubSize(); }

  forceinline unsigned int
  SetView::unknownSize(void) const { return varimp->lubSize() - varimp->glbSize(); }

  forceinline bool
  SetView::contains(int i) const { return (varimp->knownIn(i)); }

  forceinline bool
  SetView::notContains(int i) const { return (varimp->knownOut(i)); }

  forceinline unsigned int
  SetView::cardMin(void) const { return varimp->cardMin(); }

  forceinline unsigned int
  SetView::cardMax(void) const { return varimp->cardMax(); }

  forceinline int
  SetView::lubMin(void) const { return varimp->lubMin(); }

  forceinline int
  SetView::lubMax(void) const { return varimp->lubMax(); }

  forceinline int
  SetView::lubMinN(unsigned int n) const { return varimp->lubMinN(n); }

  forceinline int
  SetView::glbMin(void) const { return varimp->glbMin(); }

  forceinline int
  SetView::glbMax(void) const { return varimp->glbMax(); }

  /*
   * Tells
   *
   */

  forceinline ModEvent
  SetView::cardMin(Space& home, unsigned int m) {
    return varimp-> cardMin(home, m);
  }

  forceinline ModEvent
  SetView::cardMax(Space& home, unsigned int m) {
    return varimp-> cardMax(home, m);
  }

  forceinline ModEvent
  SetView::include (Space& home,int from, int to) {
    return (varimp->include(home,from,to));
  }

  forceinline ModEvent
  SetView::include (Space& home,int n) {
    return (varimp->include(home,n));
  }

  forceinline ModEvent
  SetView::exclude (Space& home,int n) {
    return (varimp->exclude(home, n));
  }

  forceinline ModEvent
  SetView::intersect (Space& home,int from, int to) {
    return (varimp->intersect(home,from,to));
  }

  forceinline ModEvent
  SetView::intersect (Space& home,int n) {
    return (varimp->intersect(home,n));
  }

  template<class I> ModEvent
  SetView::includeI (Space& home, I& iter) {
    Iter::Ranges::IsRangeIter<I>();
    return (varimp->includeI(home, iter));
  }

  forceinline ModEvent
  SetView::exclude (Space& home,int from, int to)
  { return (varimp->exclude(home,from,to)); }

  template<class I> ModEvent
  SetView::excludeI(Space& home, I& iter) {
    Iter::Ranges::IsRangeIter<I>();
    return varimp->excludeI(home, iter);
  }

  template<class I> ModEvent
  SetView::intersectI(Space& home, I& iter) {
    Iter::Ranges::IsRangeIter<I>();
    return varimp->intersectI(home, iter);
  }



  /*
   * Cloning
   *
   */

  forceinline void
  SetView::update(Space& home, bool share, SetView& y) {
    varimp = y.varimp->copy(home,share);
  }


  /*
   * Delta information for advisors
   *
   */

  forceinline ModEvent
  SetView::modevent(const Delta& d) { return SetVarImp::modevent(d); }

  forceinline int
  SetView::glbMin(const Delta& d) const { return SetVarImp::glbMin(d); }

  forceinline int
  SetView::glbMax(const Delta& d) const { return SetVarImp::glbMax(d); }

  forceinline bool
  SetView::glbAny(const Delta& d) const { return SetVarImp::glbAny(d); }

  forceinline int
  SetView::lubMin(const Delta& d) const { return SetVarImp::lubMin(d); }

  forceinline int
  SetView::lubMax(const Delta& d) const { return SetVarImp::lubMax(d); }

  forceinline bool
  SetView::lubAny(const Delta& d) const { return SetVarImp::lubAny(d); }


  /**
   * \brief %Range iterator for least upper bound of set variable views
   * \ingroup TaskActorSetView
   */
  template<>
  class LubRanges<SetView> : public LubRanges<SetVarImp*> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    LubRanges(void);
    /// Initialize with ranges for view \a x
    LubRanges(const SetView& x);
    /// Initialize with ranges for view \a x
    void init(const SetView& x);
    //@}
  };

  forceinline
  LubRanges<SetView>::LubRanges(void) {}

  forceinline
  LubRanges<SetView>::LubRanges(const SetView& x)
    : LubRanges<SetVarImp*>(x.var()) {}

  forceinline void
  LubRanges<SetView>::init(const SetView& x) {
    LubRanges<SetVarImp*>::init(x.var());
  }


  /**
   * \brief %Range iterator for greatest lower bound of set variable views
   * \ingroup TaskActorSetView
   */
  template<>
  class GlbRanges<SetView> : public GlbRanges<SetVarImp*> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    GlbRanges(void);
    /// Initialize with ranges for view \a x
    GlbRanges(const SetView& x);
    /// Initialize with ranges for view \a x
    void init(const SetView& x);
  };

  forceinline
  GlbRanges<SetView>::GlbRanges(void) {}

  forceinline
  GlbRanges<SetView>::GlbRanges(const SetView& x)
    : GlbRanges<SetVarImp*>(x.var()) {}

  forceinline void
  GlbRanges<SetView>::init(const SetView& x) {
    GlbRanges<SetVarImp*>::init(x.var());
  }

}}

// STATISTICS: set-var

