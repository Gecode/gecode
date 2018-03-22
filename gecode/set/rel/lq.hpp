/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2011
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

namespace Gecode { namespace Set { namespace Rel {

  /**
   * \brief Representation of the characteristic functions of two sets
   */
  class CharacteristicSets {
  protected:
    /// Size of the combined upper bounds
    unsigned int xsize;
    /// Storage for the characteristic functions
    Support::BitSetBase b;
    /// Elements in the combined upper bounds
    int* ub;
    /// Whether lower bound of x was updated
    bool xlm;
    /// Whether upper bound of x was updated
    bool xum;
    /// Whether lower bound of y was updated
    bool ylm;
    /// Whether upper bound of y was updated
    bool yum;
    /// Get bit \a i
    bool get(unsigned int i) const;
    /// Set bit \a i to value \a j
    void set(unsigned int i, bool j);

    /// \brief Value iterator for characteristic function
    class CSIter {
    public:
      /// Pointer to the underlying set
      CharacteristicSets* cs;
      /// Current position
      unsigned int i;
      /// Offset from start of bitset
      unsigned int xoff;
      /// Offset for each element (0=lower bound, 1=upper bound)
      unsigned int yoff;
      /// Move iterator to next element
      void operator++ (void);
      /// Default constructor
      CSIter(void);
      /// Constructor
      CSIter(CharacteristicSets& cs0, unsigned int xoff0, unsigned int yoff0);
      /// Test if iterator is finished
      bool operator() (void) const;
      /// Value of current iterator position
      int val(void) const;
    };

  public:
    /// Constructor
    template<class View0, class View1>
    CharacteristicSets(Region& re, View0 x, View1 y);

    /// Return minimum of element \a i for variable x
    bool xmin(unsigned int i) const;
    /// Return maximum of element \a i for variable x
    bool xmax(unsigned int i) const;
    /// Return minimum of element \a i for variable y
    bool ymin(unsigned int i) const;
    /// Return maximum of element \a i for variable y
    bool ymax(unsigned int i) const;

    /// Set minimum of element \a i for variable x to \a j
    void xmin(unsigned int i, bool j);
    /// Set maximum of element \a i for variable x to \a j
    void xmax(unsigned int i, bool j);
    /// Set minimum of element \a i for variable y to \a j
    void ymin(unsigned int i, bool j);
    /// Set maximum of element \a i for variable y to \a j
    void ymax(unsigned int i, bool j);

    /// Update upper bound of \f$x_i\f$ to \a j
    ModEvent xlq(unsigned int i, bool j);
    /// Update lower bound of \f$x_i\f$ to \a j
    ModEvent xgq(unsigned int i, bool j);
    /// Update upper bound of \f$y_i\f$ to \a j
    ModEvent ylq(unsigned int i, bool j);
    /// Update lower bound of \f$y_i\f$ to \a j
    ModEvent ygq(unsigned int i, bool j);

    /// Return size of combined upper bounds
    unsigned int size(void) const;

    /// Prune \a x and \a y using computed bounds
    template<class View0, class View1>
    ExecStatus prune(Space& home, View0 x, View1 y);

  };


  forceinline bool
  CharacteristicSets::get(unsigned int i) const {
    return b.get(i);
  }
  forceinline void
  CharacteristicSets::set(unsigned int i, bool j) {
    if (j)
      b.set(i);
    else
      b.clear(i);
  }
  forceinline unsigned int
  CharacteristicSets::size(void) const {
    return xsize;
  }

  forceinline
  CharacteristicSets::CSIter::CSIter(void) {}
  forceinline
  CharacteristicSets::CSIter::CSIter(CharacteristicSets& cs0,
                                     unsigned int xoff0, unsigned int yoff0)
    : cs(&cs0), i(0), xoff(xoff0), yoff(yoff0) {
    while ((i < cs->xsize) && !cs->get(xoff+2*i+yoff))
      i++;
  }
  forceinline void
  CharacteristicSets::CSIter::operator++ (void) {
    i++;
    while ((i < cs->xsize) && !cs->get(xoff+2*i+yoff))
      i++;
  }
  forceinline bool
  CharacteristicSets::CSIter::operator() (void) const {
    return i<cs->xsize;
  }
  forceinline int
  CharacteristicSets::CSIter::val(void) const {
    return cs->ub[i];
  }


  forceinline bool
  CharacteristicSets::xmin(unsigned int i) const {
    return get(2*i);
  }
  forceinline bool
  CharacteristicSets::xmax(unsigned int i) const {
    return get(2*i+1);
  }
  forceinline bool
  CharacteristicSets::ymin(unsigned int i) const {
    return get(2*xsize+2*i);
  }
  forceinline bool
  CharacteristicSets::ymax(unsigned int i) const {
    return get(2*xsize+2*i+1);
  }

  forceinline void
  CharacteristicSets::xmin(unsigned int i, bool j) {
    set(2*i,j);
  }
  forceinline void
  CharacteristicSets::xmax(unsigned int i, bool j) {
    set(2*i+1,j);
  }
  forceinline void
  CharacteristicSets::ymin(unsigned int i, bool j) {
    set(2*xsize+2*i,j);
  }
  forceinline void
  CharacteristicSets::ymax(unsigned int i, bool j) {
    set(2*xsize+2*i+1,j);
  }

  forceinline ModEvent
  CharacteristicSets::xlq(unsigned int i, bool j) {
    if (!j) {
      if (xmin(i)) return ME_SET_FAILED;
      if (xmax(i)) {
        xmax(i,false);
        xum=true;
      }
    }
    return ME_SET_NONE;
  }
  forceinline ModEvent
  CharacteristicSets::xgq(unsigned int i, bool j) {
    if (j) {
      if (!xmax(i)) return ME_SET_FAILED;
      if (!xmin(i)) {
        xmin(i,true);
        xlm=true;
      }
    }
    return ME_SET_NONE;
  }
  forceinline ModEvent
  CharacteristicSets::ylq(unsigned int i, bool j) {
    if (!j) {
      if (ymin(i)) return ME_SET_FAILED;
      if (ymax(i)) {
        ymax(i,false);
        yum=true;
      }
    }
    return ME_SET_NONE;
  }
  forceinline ModEvent
  CharacteristicSets::ygq(unsigned int i, bool j) {
    if (j) {
      if (!ymax(i)) return ME_SET_FAILED;
      if (!ymin(i)) {
        ymin(i,true);
        ylm=true;
      }
    }
    return ME_SET_NONE;
  }

  template<class View0, class View1>
  forceinline ExecStatus
  CharacteristicSets::prune(Space& home, View0 x, View1 y) {
    if (xlm) {
      CSIter i(*this,0,0);
      Iter::Values::ToRanges<CSIter> ir(i);
      GECODE_ME_CHECK(x.includeI(home,ir));
    }
    if (xum) {
      CSIter i(*this,0,1);
      Iter::Values::ToRanges<CSIter> ir(i);
      GECODE_ME_CHECK(x.intersectI(home,ir));
    }
    if (ylm) {
      CSIter i(*this,2*xsize,0);
      Iter::Values::ToRanges<CSIter> ir(i);
      GECODE_ME_CHECK(y.includeI(home,ir));
    }
    if (yum) {
      CSIter i(*this,2*xsize,1);
      Iter::Values::ToRanges<CSIter> ir(i);
      GECODE_ME_CHECK(y.intersectI(home,ir));
    }
    return ES_OK;
  }

  template<class View0, class View1>
  forceinline
  CharacteristicSets::CharacteristicSets(Region& re,View0 x, View1 y)
    : xlm(false), xum(false), ylm(false), yum(false) {
    LubRanges<View0> xlub(x);
    LubRanges<View1> ylub(y);
    Iter::Ranges::Union<LubRanges<View0>,LubRanges<View1> > xylub(xlub,ylub);
    Iter::Ranges::Cache xylubc(re,xylub);
    xsize = Iter::Ranges::size(xylubc);
    b.init(re,4*xsize);
    ub = re.alloc<int>(xsize);
    xylubc.reset();
    Iter::Ranges::ToValues<Iter::Ranges::Cache> xylubv(xylubc);
    LubRanges<View0> xur(x);
    GlbRanges<View0> xlr(x);
    LubRanges<View1> yur(y);
    GlbRanges<View1> ylr(y);
    Iter::Ranges::ToValues<LubRanges<View0> > xuv(xur);
    Iter::Ranges::ToValues<GlbRanges<View0> > xlv(xlr);
    Iter::Ranges::ToValues<LubRanges<View1> > yuv(yur);
    Iter::Ranges::ToValues<GlbRanges<View1> > ylv(ylr);
    for (unsigned int i=0; xylubv(); ++xylubv, ++i) {
      ub[i] = xylubv.val();
      if (xlv() && xylubv.val()==xlv.val()) {
        b.set(2*i);
        ++xlv;
      }
      if (xuv() && xylubv.val()==xuv.val()) {
        b.set(2*i+1);
        ++xuv;
      }
      if (ylv() && xylubv.val()==ylv.val()) {
        b.set(2*xsize+2*i);
        ++ylv;
      }
      if (yuv() && xylubv.val()==yuv.val()) {
        b.set(2*xsize+2*i+1);
        ++yuv;
      }
    }
  }

  template<class View0, class View1, bool strict>
  forceinline
  Lq<View0,View1,strict>::Lq(Home home, View0 x, View1 y)
    : MixBinaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>(home,x,y) {}

  template<class View0, class View1, bool strict>
  forceinline
  Lq<View0,View1,strict>::Lq(Space& home, Lq& p)
    : MixBinaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>(home,p) {}

  template<class View0, class View1, bool strict>
  ExecStatus
  Lq<View0,View1,strict>::post(Home home, View0 x, View1 y) {
    if (strict)
      GECODE_ME_CHECK(y.cardMin(home,1));
    if (same(x,y))
      return strict ? ES_FAILED : ES_OK;
    (void) new (home) Lq(home,x,y);
    return ES_OK;
  }

  template<class View0, class View1, bool strict>
  Actor*
  Lq<View0,View1,strict>::copy(Space& home) {
    return new (home) Lq(home,*this);
  }

  template<class View0, class View1, bool strict>
  ExecStatus
  Lq<View0,View1,strict>::propagate(Space& home, const ModEventDelta&) {
    if ( (!strict) && x1.cardMax()==0) {
      GECODE_ME_CHECK(x0.cardMax(home,0));
    }

    if (x0.cardMax()==0) {
      return home.ES_SUBSUMED(*this);
    }

    if (x0.glbMin() < x1.lubMin())
      return ES_FAILED;
    if (x1.glbMin() < x0.lubMin())
      return home.ES_SUBSUMED(*this);

    bool assigned = x0.assigned() && x1.assigned();

    Region re;
    CharacteristicSets cs(re,x0,x1);

    /*
     * State 1
     *
     */
    unsigned int i=0;
    unsigned int firsti=0;
    unsigned int n=cs.size();
    while ((i<n) && (cs.xmin(i) == cs.ymax(i))) {
      // case: =, >=
      GECODE_ME_CHECK(cs.xlq(i,cs.ymax(i)));
      GECODE_ME_CHECK(cs.ygq(i,cs.xmin(i)));
      i++;
    }

    if (i == n) {// case: $
      if (strict) {
        return ES_FAILED;
      } else {
        GECODE_ES_CHECK(cs.prune(home,x0,x1));
        return home.ES_SUBSUMED(*this);
      }
    }

    // Possible cases left: <, <=, > (yields failure), ?
    GECODE_ME_CHECK(cs.xlq(i,cs.ymax(i)));
    GECODE_ME_CHECK(cs.ygq(i,cs.xmin(i)));

    if (cs.xmax(i) < cs.ymin(i)) { // case: < (after tell)
      GECODE_ES_CHECK(cs.prune(home,x0,x1));
      return home.ES_SUBSUMED(*this);
    }

    firsti=i;

    /*
     * State 2
     *   prefix: (?|<=)
     *
     */
    i++;

    while ((i < n) &&
           (cs.xmin(i) == cs.ymax(i)) &&
           (cs.xmax(i) == cs.ymin(i))) { // case: =
      i++;
    }

    if (i == n) { // case: $
      if (strict)
        goto rewrite_le;
      else
        goto rewrite_lq;
    }

    if (cs.xmax(i) < cs.ymin(i)) // case: <
      goto rewrite_lq;

    if (cs.xmin(i) > cs.ymax(i)) // case: >
      goto rewrite_le;

    if (cs.xmax(i) <= cs.ymin(i)) {
      // case: <= (invariant: not =, <)
      /*
       * State 3
       *   prefix: (?|<=),<=
       *
       */
      i++;

      while ((i < n) && (cs.xmax(i) == cs.ymin(i))) {// case: <=, =
        i++;
      }

      if (i == n) { // case: $
        if (strict) {
          GECODE_ES_CHECK(cs.prune(home,x0,x1));
          return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;
        } else {
          goto rewrite_lq;
        }
      }

      if (cs.xmax(i) < cs.ymin(i)) // case: <
        goto rewrite_lq;

      GECODE_ES_CHECK(cs.prune(home,x0,x1));
      return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;
    }

    if (cs.xmin(i) >= cs.ymax(i)) {
      // case: >= (invariant: not =, >)
      /*
       * State 4
       *   prefix: (?|<=) >=
       *
       */
      i++;

      while ((i < n) && (cs.xmin(i) == cs.ymax(i))) {
        // case: >=, =
        i++;
      }

      if (i == n) { // case: $
        if (strict) {
          goto rewrite_le;
        } else {
          GECODE_ES_CHECK(cs.prune(home,x0,x1));
          return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;
        }
      }

      if (cs.xmin(i) > cs.ymax(i)) // case: >
        goto rewrite_le;

      GECODE_ES_CHECK(cs.prune(home,x0,x1));
      return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;
    }

    GECODE_ES_CHECK(cs.prune(home,x0,x1));
    return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;

  rewrite_le:
    GECODE_ME_CHECK(cs.xlq(firsti,false));
    GECODE_ME_CHECK(cs.ygq(firsti,true));
    GECODE_ES_CHECK(cs.prune(home,x0,x1));
    return home.ES_SUBSUMED(*this);
  rewrite_lq:
    GECODE_ES_CHECK(cs.prune(home,x0,x1));
    return assigned ? home.ES_SUBSUMED(*this) : ES_NOFIX;
  }

}}}

// STATISTICS: set-prop
