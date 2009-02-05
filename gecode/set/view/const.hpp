/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

  /**
   * \brief %Range iterator for a two-dimensional array
   * \ingroup TaskActorSetView
   */
  class ArrayRanges {
  private:
    int *_ranges;
    int _size;
    int _pos;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    ArrayRanges(void) : _ranges(NULL), _size(0), _pos(0) {}
    /// Initialize with ranges for array \a ranges which is of size \a size
    ArrayRanges(int *ranges, int size)
      : _ranges(ranges), _size(size), _pos(0) {}
    /// Initialize with ranges for array \a ranges which is of size \a size
    void init(int* ranges, int size) {
      _ranges = ranges; _size = size; _pos = 0;
    }
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a range or done
    bool operator ()(void) const { return _pos<_size; }
    /// Move iterator to next range (if possible)
    void operator ++(void) { _pos++; }
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    int min(void) const { return _ranges[_pos*2]; }
    /// Return largest value of range
    int max(void) const { return _ranges[_pos*2+1]; }
    /// Return width of range (distance between minimum and maximum)
    unsigned int width(void) const {
      return _ranges[_pos*2+1]-_ranges[_pos*2]+1;
    }
    //@}
  };

  forceinline
  ConstantView::ConstantView(void) : ranges(NULL), size(0), domSize(0) {}

  forceinline
  void
  ConstantView::init(Space& home, const IntSet& dom) {
    size = dom.ranges();
    domSize = 0;
    if (size > 0) {
      ranges = home.alloc<int>(2*size);
      IntSetRanges dr(dom);
      for (int i=0; dr(); ++dr, i+=2) {
        int min = dr.min(); int max = dr.max();
        ranges[i] = min;
        ranges[i+1] = max;
        domSize += (max-min+1);
      }
    } else {
      ranges = NULL;
    }
  }

  forceinline
  ConstantView::ConstantView(Space& home, const IntSet& dom) {
    init(home, dom);
  }

  forceinline bool
  ConstantView::assigned(void) const { return true; }

  forceinline unsigned int
  ConstantView::glbSize(void) const { return domSize; }

  forceinline unsigned int
  ConstantView::lubSize(void) const { return domSize; }

  forceinline unsigned int
  ConstantView::unknownSize(void) const { return 0; }

  forceinline bool
  ConstantView::contains(int i) const {
    for (unsigned int j=size; j--; ) {
      if (ranges[2*j+1] < i)
        return false;
      if (ranges[2*j] >= i)
        return true;
    }
    return false;
  }

  forceinline bool
  ConstantView::notContains(int i) const {
    return !contains(i);
  }

  forceinline unsigned int
  ConstantView::cardMin(void) const { return domSize; }

  forceinline unsigned int
  ConstantView::cardMax(void) const { return domSize; }

  forceinline int
  ConstantView::lubMin(void) const {
    return size==0 ? BndSet::MIN_OF_EMPTY : ranges[0];
  }

  forceinline int
  ConstantView::lubMax(void) const {
    return size==0 ? BndSet::MAX_OF_EMPTY : ranges[size*2-1];
  }

  forceinline int
  ConstantView::glbMin(void) const { return lubMin(); }

  forceinline int
  ConstantView::glbMax(void) const { return lubMax(); }

  forceinline ModEvent
  ConstantView::cardMin(Space&,unsigned int c) {
    return c<=domSize ? ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::cardMax(Space&,unsigned int c) {
    return c>=domSize ? ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::include(Space&,int c) {
    return contains(c) ? ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::exclude(Space&,int c) {
    return contains(c) ? ME_SET_FAILED : ME_SET_NONE;
  }

  forceinline ModEvent
  ConstantView::intersect(Space&,int c) {
    return (size==0 ||
            (size==1 &&
             ranges[0]==ranges[1] && ranges[0]==c)) ?
      ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::intersect(Space&,int i,int j) {
    return (glbMin()>=i && glbMax()<=j) ?
      ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::include(Space&,int i,int j) {
    Iter::Ranges::Singleton single(i,j);
    ArrayRanges ar(ranges, size);
    return (single() && Iter::Ranges::subset(single, ar)) ?
      ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  ConstantView::exclude(Space&,int i,int j) {
    Iter::Ranges::Singleton single(i,j);
    ArrayRanges ar(ranges, size);
    return (single() && Iter::Ranges::subset(single, ar)) ?
      ME_SET_FAILED : ME_SET_NONE;
  }

  template <class I> ModEvent
  ConstantView::excludeI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    ArrayRanges ar(ranges, size);
    return (i() && Iter::Ranges::subset(i, ar)) ? ME_SET_FAILED : ME_SET_NONE;
  }

  template <class I> ModEvent
  ConstantView::includeI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    ArrayRanges ar(ranges, size);
    return Iter::Ranges::subset(i, ar) ? ME_SET_NONE : ME_SET_FAILED;
  }

  template <class I> ModEvent
  ConstantView::intersectI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    ArrayRanges ar(ranges, size);
    return Iter::Ranges::subset(ar, i) ? ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline void
  ConstantView::schedule(Space& home, Propagator& p, ModEvent me) {
    return SetView::schedule(home,p,me);
  }
  forceinline ModEvent
  ConstantView::me(const ModEventDelta&) {
    return ME_SET_NONE;
  }
  forceinline ModEventDelta
  ConstantView::med(ModEvent me) {
    return SetVarImp::med(me);
  }

  forceinline void
  ConstantView::subscribe(Space& home, Propagator& p, PropCond,bool) {
    schedule(home,p,ME_SET_VAL);
  }
  forceinline void
  ConstantView::cancel(Space&,Propagator&,PropCond) {}

  forceinline void
  ConstantView::subscribe(Space&, Advisor&) {}
  forceinline void
  ConstantView::cancel(Space&,Advisor&) {}

  forceinline void
  ConstantView::update(Space& home, bool, ConstantView& p) {
    // dispose old ranges
    if (size > 0)
      home.free<int>(ranges, 2);

    domSize = p.domSize;
    size = p.size;
    if (size == 0) {
      ranges = NULL;
    } else {
      // copy ranges from p
      ranges = home.alloc<int>(2*size);
      for (unsigned int i=size; i--; ) {
        ranges[2*i]   = p.ranges[2*i];
        ranges[2*i+1] = p.ranges[2*i+1];
      }
    }
  }


  /*
   * Delta information for advisors
   *
   */

  forceinline ModEvent
  ConstantView::modevent(const Delta&) {
    GECODE_NEVER;
    return ME_GEN_NONE;
  }

  forceinline int
  ConstantView::glbMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  ConstantView::glbMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  ConstantView::glbAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  forceinline int
  ConstantView::lubMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  ConstantView::lubMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  ConstantView::lubAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  forceinline
  EmptyView::EmptyView(void) {}



  forceinline bool
  EmptyView::assigned(void) const { return true; }

  forceinline unsigned int
  EmptyView::glbSize(void) const { return 0; }

  forceinline unsigned int
  EmptyView::lubSize(void) const { return 0; }

  forceinline unsigned int
  EmptyView::unknownSize(void) const { return 0; }

  forceinline bool
  EmptyView::contains(int) const { return false; }

  forceinline bool
  EmptyView::notContains(int) const { return true; }

  forceinline unsigned int
  EmptyView::cardMin(void) const { return 0; }

  forceinline unsigned int
  EmptyView::cardMax(void) const { return 0; }

  forceinline int
  EmptyView::lubMin(void) const { return 0; }

  forceinline int
  EmptyView::lubMax(void) const { return 0; }

  forceinline int
  EmptyView::glbMin(void) const { return 0; }

  forceinline int
  EmptyView::glbMax(void) const { return 0; }

  forceinline ModEvent
  EmptyView::cardMin(Space&,unsigned int c) {
    return c==0 ? ME_SET_NONE : ME_SET_FAILED;
  }

  forceinline ModEvent
  EmptyView::cardMax(Space&,unsigned int) {
    return ME_SET_NONE;
  }


  forceinline ModEvent
  EmptyView::include(Space&,int) {
    return ME_SET_FAILED;
  }

  forceinline ModEvent
  EmptyView::exclude(Space&,int) { return ME_SET_NONE; }

  forceinline ModEvent
  EmptyView::intersect(Space&,int) { return ME_SET_NONE; }

  forceinline ModEvent
  EmptyView::intersect(Space&,int,int) { return ME_SET_NONE; }

  forceinline ModEvent
  EmptyView::include(Space&,int,int) {
    return ME_SET_FAILED; }

  forceinline ModEvent
  EmptyView::exclude(Space&,int,int) { return ME_SET_NONE; }

  template <class I> ModEvent
  EmptyView::excludeI(Space&,I&) {
    Iter::Ranges::IsRangeIter<I>();
    return ME_SET_NONE;
  }

  template <class I> ModEvent
  EmptyView::includeI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    return i() ? ME_SET_FAILED : ME_SET_NONE;
  }

  template <class I> ModEvent
  EmptyView::intersectI(Space&,I&) {
    Iter::Ranges::IsRangeIter<I>();
    return ME_SET_NONE;
  }

  forceinline void
  EmptyView::schedule(Space& home, Propagator& p, ModEvent me) {
    return SetView::schedule(home,p,me);
  }
  forceinline ModEvent
  EmptyView::me(const ModEventDelta&) {
    return ME_SET_NONE;
  }
  forceinline ModEventDelta
  EmptyView::med(ModEvent me) {
    return SetVarImp::med(me);
  }

  forceinline void
  EmptyView::subscribe(Space& home, Propagator& p, PropCond,bool) {
    schedule(home,p,ME_SET_VAL);
  }
  forceinline void
  EmptyView::cancel(Space&,Propagator&,PropCond) {}
  forceinline void
  EmptyView::subscribe(Space&, Advisor&) {}
  forceinline void
  EmptyView::cancel(Space&,Advisor&) {}


  forceinline void
  EmptyView::update(Space&, bool, EmptyView&) {}


  /*
   * Delta information for advisors
   *
   */

  forceinline ModEvent
  EmptyView::modevent(const Delta&) {
    GECODE_NEVER;
    return ME_GEN_NONE;
  }

  forceinline int
  EmptyView::glbMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  EmptyView::glbMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  EmptyView::glbAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  forceinline int
  EmptyView::lubMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  EmptyView::lubMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  EmptyView::lubAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  // Constant universe variable

  forceinline
  UniverseView::UniverseView(void) {}

  forceinline bool
  UniverseView::assigned(void) const { return true; }

  forceinline unsigned int
  UniverseView::glbSize(void) const { return Set::Limits::card; }

  forceinline unsigned int
  UniverseView::lubSize(void) const { return Set::Limits::card; }

  forceinline unsigned int
  UniverseView::unknownSize(void) const { return 0; }

  forceinline bool
  UniverseView::contains(int) const { return true; }

  forceinline bool
  UniverseView::notContains(int) const { return false; }

  forceinline unsigned int
  UniverseView::cardMin(void) const { return Set::Limits::card; }

  forceinline unsigned int
  UniverseView::cardMax(void) const { return Limits::card; }

  forceinline int
  UniverseView::lubMin(void) const { return Limits::card; }

  forceinline int
  UniverseView::lubMax(void) const { return Limits::card; }

  forceinline int
  UniverseView::glbMin(void) const { return Limits::card; }

  forceinline int
  UniverseView::glbMax(void) const { return Limits::card; }

  forceinline ModEvent
  UniverseView::cardMin(Space&,unsigned int c) {
    return c>Limits::card ? ME_SET_FAILED : ME_SET_NONE;
  }

  forceinline ModEvent
  UniverseView::cardMax(Space&,unsigned int c) {
    return c>=Limits::card ? ME_SET_NONE : ME_SET_FAILED;
  }


  forceinline ModEvent
  UniverseView::include(Space&,int) {
    return ME_SET_NONE;
  }

  forceinline ModEvent
  UniverseView::exclude(Space&,int) { return ME_SET_FAILED; }

  forceinline ModEvent
  UniverseView::intersect(Space&,int) { return ME_SET_FAILED; }

  forceinline ModEvent
  UniverseView::include(Space&,int,int) { return ME_SET_NONE; }

  forceinline ModEvent
  UniverseView::exclude(Space&,int,int) { return ME_SET_FAILED; }

  template <class I> ModEvent
  UniverseView::excludeI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    return i() ? ME_SET_FAILED : ME_SET_NONE;
  }

  template <class I> forceinline ModEvent
  UniverseView::includeI(Space&,I&) {
    Iter::Ranges::IsRangeIter<I>();
    return ME_SET_NONE;
  }

  forceinline ModEvent
  UniverseView::intersect(Space&,int i,int j) {
    return (i>Limits::min ||
            j<Limits::max) ? ME_SET_FAILED : ME_SET_NONE;
  }

  template <class I> forceinline ModEvent
  UniverseView::intersectI(Space&,I& i) {
    Iter::Ranges::IsRangeIter<I>();
    return (i() &&
            (i.min()>Limits::min ||
             i.max()<Limits::max) ) ?
      ME_SET_FAILED : ME_SET_NONE;
  }

  forceinline void
  UniverseView::schedule(Space& home, Propagator& p, ModEvent me) {
    return SetView::schedule(home,p,me);
  }
  forceinline ModEvent
  UniverseView::me(const ModEventDelta&) {
    return ME_SET_NONE;
  }
  forceinline ModEventDelta
  UniverseView::med(ModEvent me) {
    return SetVarImp::med(me);
  }
  forceinline void
  UniverseView::subscribe(Space& home, Propagator& p, PropCond,bool) {
    schedule(home,p,ME_SET_VAL);
  }
  forceinline void
  UniverseView::cancel(Space&,Propagator&,PropCond) {}

  forceinline void
  UniverseView::subscribe(Space&,Advisor&) {}
  forceinline void
  UniverseView::cancel(Space&,Advisor&) {}


  forceinline void
  UniverseView::update(Space&, bool, UniverseView&) {}


  /*
   * Delta information for advisors
   *
   */

  forceinline ModEvent
  UniverseView::modevent(const Delta&) {
    GECODE_NEVER;
    return ME_GEN_NONE;
  }

  forceinline int
  UniverseView::glbMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  UniverseView::glbMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  UniverseView::glbAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  forceinline int
  UniverseView::lubMin(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline int
  UniverseView::lubMax(const Delta&) const {
    GECODE_NEVER;
    return 0;
  }

  forceinline bool
  UniverseView::lubAny(const Delta&) const {
    GECODE_NEVER;
    return false;
  }

  /*
   * Iterators
   *
   */

  /**
   * \brief %Range iterator for least upper bound of constantly empty set view
   * \ingroup TaskActorSetView
   */
  template <>
  class LubRanges<EmptyView> : public Iter::Ranges::Empty {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    LubRanges(void) {}
    /// Initialize with ranges for view \a x
    LubRanges(const EmptyView& x) { (void)x; }
    /// Initialize with ranges for view \a x
    void init(const EmptyView& x) { (void)x; }
    //@}
  };

  /**
   * \brief %Range iterator for greatest lower bound of constantly empty set view
   * \ingroup TaskActorSetView
   */
  template <>
  class GlbRanges<EmptyView> : public Iter::Ranges::Empty {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    GlbRanges(void) {}
    /// Initialize with ranges for view \a x
    GlbRanges(const EmptyView& x) { (void)x; }
    /// Initialize with ranges for view \a x
    void init(const EmptyView& x) { (void)x; }
    //@}
  };

  /**
   * \brief %Range iterator for least upper bound of constant universe set view
   * \ingroup TaskActorSetView
   */
  template <>
  class LubRanges<UniverseView> : public Iter::Ranges::Singleton {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    LubRanges(void)
      : Iter::Ranges::Singleton(Limits::min,
                                Limits::max) {}
    /// Initialize with ranges for view \a x
    LubRanges(const UniverseView& x)
      : Iter::Ranges::Singleton(Limits::min,
                                Limits::max) {
        (void)x;
      }
    /// Initialize with ranges for view \a x
    void init(const UniverseView& x) { (void)x; }
    //@}
  };

  /**
   * \brief %Range iterator for greatest lower bound of constant universe set view
   * \ingroup TaskActorSetView
   */
  template <>
  class GlbRanges<UniverseView> : public Iter::Ranges::Singleton {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    GlbRanges(void)
      : Iter::Ranges::Singleton(Limits::min,
                                Limits::max) {}
    /// Initialize with ranges for view \a x
    GlbRanges(const UniverseView& x)
      : Iter::Ranges::Singleton(Limits::min,
                                Limits::max) {
      (void)x;
    }
    /// Initialize with ranges for view \a x
    void init(const UniverseView& x) { (void)x; }
    //@}
  };


  /**
   * \brief %Range iterator for least upper bound of constant set view
   * \ingroup TaskActorSetView
   */
  template <>
  class LubRanges<ConstantView> {
  private:
    ArrayRanges ar;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    LubRanges(void) {}
    /// Initialize with ranges for view \a x
    LubRanges(const ConstantView& x) : ar(x.ranges,x.size) {}
    /// Initialize with ranges for view \a x
    void init(const ConstantView& x) {
      ar.init(x.ranges,x.size);
    }
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a value or done
    bool operator ()(void) const { return ar(); }
    /// Move iterator to next value (if possible)
    void operator ++(void) { ++ar; }
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    int min(void) const { return ar.min(); }
    /// Return largest value of range
    int max(void) const { return ar.max(); }
    /// Return width of range (distance between minimum and maximum)
    unsigned int width(void) const { return ar.width(); }
    //@}
  };

  /**
   * \brief %Range iterator for greatest lower bound of constant set view
   * \ingroup TaskActorSetView
   */
  template <>
  class GlbRanges<ConstantView> : public LubRanges<ConstantView> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    GlbRanges(void) {}
    /// Initialize with ranges for view \a x
    GlbRanges(const ConstantView& x) : LubRanges<ConstantView>(x) {}
    /// Initialize with ranges for view \a x
    void init(const ConstantView& x) {
      LubRanges<ConstantView>::init(x);
    }
    //@}
  };
}


  /*
   * Testing
   *
   */
  forceinline bool
  same(const Set::ConstantView& x, const Set::ConstantView& y) {
    if ((x.size != y.size) || (x.domSize != y.domSize))
      return false;
    for (int i=x.size; i--; )
      if (x.ranges[2*i]   != y.ranges[2*i] ||
          x.ranges[2*i+1] != y.ranges[2*i+1])
        return false;
    return true;
  }
  forceinline bool
  before(const Set::ConstantView& x, const Set::ConstantView& y) {
    if (x.size < y.size)
      return true;
    if (x.domSize < y.domSize)
      return true;
    for (int i=x.size; i--; )
      if (x.ranges[2*i]   < y.ranges[2*i] ||
          x.ranges[2*i+1] < y.ranges[2*i+1])
        return true;
    return false;
  }


  forceinline bool
  same(const Set::EmptyView&, const Set::EmptyView&) {
    return true;
  }
  forceinline bool
  before(const Set::EmptyView&, const Set::EmptyView&) {
    return false;
  }

  forceinline bool
  same(const Set::UniverseView&, const Set::UniverseView&) {
    return true;
  }
  forceinline bool
  before(const Set::UniverseView&, const Set::UniverseView&) {
    return false;
  }

}

// STATISTICS: set-var

