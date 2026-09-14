/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_WORD_VIEW_HPP
#define GECODE_WORD_VIEW_HPP

namespace Gecode { namespace Word {
  /** \brief View of a word variable
   *  \ingroup TaskModelWordVars
   */
  class WordView : public VarImpView<WordVar> {
  public:
    /// Whether this view exposes a persistent ranked interval
    static const bool supports_bounds=false;
    /// Whether the ranked interval uses signed two's-complement order
    static const bool signed_order=false;
    WordView(void) {}
    WordView(const WordVar& y) : VarImpView<WordVar>(y.varimp()) {}
    WordView(WordVarImp* y) : VarImpView<WordVar>(y) {}

    unsigned int width(void) const { return x->width(); }
    WordValue mask(void) const { return x->mask(); }
    WordValue lo(void) const { return x->lo(); }
    WordValue hi(void) const { return x->hi(); }
    WordDomainType domain_type(void) const { return x->domain_type(); }
    bool bounded(void) const { return x->bounded(); }
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue minimum(void) const { return x->minimum(); }
    /// Return the canonical endpoint encoding (two's-complement when signed)
    WordValue maximum(void) const { return x->maximum(); }
    /// Return the canonical minimum in the variable's internal order
    WordValue rank_minimum(void) const {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->minimum();
    }
    /// Return the canonical maximum in the variable's internal order
    WordValue rank_maximum(void) const {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->maximum();
    }
    /// Convert internal order rank \a n to its encoded Word value
    WordValue encode_rank(WordValue n) const {
      assert(bounded());
      return Word::rank(domain_type(),width(),n);
    }
    WordValue unknown(void) const { return x->unknown(); }
    unsigned int unknown_size(void) const { return x->unknown_size(); }
    WordValue val(void) const { return x->val(); }
    bool in(WordValue n) const { return x->in(n); }
    ModEvent narrow(Space& home, WordValue lo, WordValue hi) {
      return x->narrow(home,lo,hi);
    }
    ModEvent eq(Space& home, WordValue n) { return x->eq(home,n); }
    /// Intersect the ranked interval of a bounded view
    ModEvent narrow_rank_range(Space& home, WordValue minimum,
                               WordValue maximum) {
      assert(bounded());
      return static_cast<BoundedWordVarImp*>(x)->
        narrow_range(home,minimum,maximum);
    }
    WordValue zero(const Delta& d) const { return WordVarImp::zero(d); }
    WordValue one(const Delta& d) const { return WordVarImp::one(d); }
    WordDomainType domain_type(const Delta& d) const {
      return WordVarImp::domain_type(d);
    }
    /// Return the old internal order-rank minimum from \a d
    WordValue old_minimum(const Delta& d) const {
      return WordVarImp::old_minimum(d);
    }
    /// Return the old internal order-rank maximum from \a d
    WordValue old_maximum(const Delta& d) const {
      return WordVarImp::old_maximum(d);
    }
    /// Return the new internal order-rank minimum from \a d
    WordValue new_minimum(const Delta& d) const {
      return WordVarImp::new_minimum(d);
    }
    /// Return the new internal order-rank maximum from \a d
    WordValue new_maximum(const Delta& d) const {
      return WordVarImp::new_maximum(d);
    }
    static ModEventDelta med(ModEvent me) { return VarImpView<WordVar>::med(me); }
  protected:
    using VarImpView<WordVar>::x;
  };

  /// Share ranked-domain queries and tells between the two bounded view types
  class BoundedWordView : public WordView {
  public:
    WordValue rank_minimum(void) const { return imp()->minimum(); }
    WordValue rank_maximum(void) const { return imp()->maximum(); }
    ModEvent narrow_domain(Space& home, WordValue lo, WordValue hi,
                           WordValue minimum, WordValue maximum) {
      return imp()->narrow_domain(home,lo,hi,minimum,maximum);
    }
    ModEvent narrow_range(Space& home, WordValue minimum,
                          WordValue maximum) {
      return imp()->narrow_range(home,minimum,maximum);
    }
  protected:
    BoundedWordView(void) {}
    BoundedWordView(const WordVar& y) : WordView(y) {}
    BoundedWordView(WordVarImp* y) : WordView(y) {}
  private:
    BoundedWordVarImp* imp(void) const {
      return static_cast<BoundedWordVarImp*>(x);
    }
  };

  /// Proposed rank intervals with at most this many values narrow exactly
  const WordValue exact_narrowing_size=WordValue(1) << 16;
  /// Larger intervals take an endpoint move of at least 1/64 of their size
  const WordValue significant_move_ratio=64;

  /** \brief Return the propagator-side narrowing of \a current to \a proposed
   *
   * Let s' be the size of the intersection of the two rank intervals. Each
   * endpoint moves only when s' <= exact_narrowing_size or the move is at
   * least s'/64. Both endpoints move when the closure of the intersection
   * with the cube of \a current leaves at most one value, so a move that
   * assigns or fails the view is never skipped. An empty result makes the
   * caller's tell fail. The rule depends only on the two domains; dom(),
   * branchers and the var-imp tells stay exact.
   */
  forceinline WordRankInterval
  significant_interval(unsigned int width, WordDomainType domain_type,
                       WordDomain current, WordRankInterval proposed) {
    const WordRankInterval next={
      std::max(current.interval.minimum,proposed.minimum),
      std::min(current.interval.maximum,proposed.maximum)};
    if (next.minimum > next.maximum)
      return next;
    // The span is s'-1, which is representable even when s' is 2^64.
    const WordValue span=next.maximum-next.minimum;
    if (span < exact_narrowing_size)
      return next;
    // For integers, delta >= s'/64 is equivalent to delta > (s'-1)/64.
    const WordValue threshold=span/significant_move_ratio;
    const bool is_minimum_significant=
      next.minimum-current.interval.minimum > threshold;
    const bool is_maximum_significant=
      current.interval.maximum-next.maximum > threshold;
    if (is_minimum_significant && is_maximum_significant)
      return next;
    const bool is_cube_empty=(current.cube.lo & ~current.cube.hi) != 0;
    if (is_cube_empty)
      return next;
    const WordDomainResult closure=
      compute_domain_closure(width,domain_type,{current.cube,next});
    const bool has_at_most_one_value=!closure.is_consistent ||
      (closure.domain.interval.minimum == closure.domain.interval.maximum);
    if (has_at_most_one_value)
      return next;
    return {is_minimum_significant ? next.minimum : current.interval.minimum,
            is_maximum_significant ? next.maximum : current.interval.maximum};
  }

  /// Return the significant narrowing of view \a x with cube \a cube
  template<class View>
  forceinline WordRankInterval
  significant_view_interval(View x, WordCube cube, WordRankInterval proposed) {
    return significant_interval(
      x.width(),x.domain_type(),
      {{x.lo() | cube.lo,x.hi() & cube.hi},
       {x.rank_minimum(),x.rank_maximum()}},
      proposed);
  }

  /** \brief Narrow the rank interval of \a x towards \a proposed under the rule
   *
   * An applied move and its closure can make a skipped move significant, so
   * repeat until \a x is unchanged. Each endpoint is applied at most once.
   */
  template<class View>
  forceinline ModEvent
  narrow_significant_range(Space& home, View x, WordRankInterval proposed) {
    ModEvent event=ME_WORD_NONE;
    for (;;) {
      const WordRankInterval next=
        significant_view_interval(x,{0,x.mask()},proposed);
      const ModEvent me=x.narrow_range(home,next.minimum,next.maximum);
      if (me_failed(me))
        return me;
      if (!me_modified(me))
        return event;
      event=WordVarImp::me_combine(event,me);
    }
  }

  /** \brief Apply \a cube in full and the rank interval \a proposed under the rule
   *
   * Repeats like narrow_significant_range.
   */
  template<class View>
  forceinline ModEvent
  narrow_significant_domain(Space& home, View x, WordCube cube,
                            WordRankInterval proposed) {
    ModEvent event=ME_WORD_NONE;
    for (;;) {
      const WordRankInterval next=
        significant_view_interval(x,cube,proposed);
      const ModEvent me=
        x.narrow_domain(home,cube.lo,cube.hi,next.minimum,next.maximum);
      if (me_failed(me))
        return me;
      if (!me_modified(me))
        return event;
      event=WordVarImp::me_combine(event,me);
    }
  }

  /// Typed view of an unsigned-bounded word
  class UnsignedWordView : public BoundedWordView {
  public:
    /// Whether this view exposes a persistent ranked interval
    static const bool supports_bounds=true;
    /// Whether the ranked interval uses signed two's-complement order
    static const bool signed_order=false;
    UnsignedWordView(void) {}
    UnsignedWordView(const WordVar& y) : BoundedWordView(y) {
      assert(domain_type() == WDT_UNSIGNED);
    }
    UnsignedWordView(WordVarImp* y) : BoundedWordView(y) {
      assert(domain_type() == WDT_UNSIGNED);
    }
  };

  /// Typed view of a signed-bounded word
  class SignedWordView : public BoundedWordView {
  public:
    /// Whether this view exposes a persistent ranked interval
    static const bool supports_bounds=true;
    /// Whether the ranked interval uses signed two's-complement order
    static const bool signed_order=true;
    SignedWordView(void) {}
    SignedWordView(const WordVar& y) : BoundedWordView(y) {
      assert(domain_type() == WDT_SIGNED);
    }
    SignedWordView(WordVarImp* y) : BoundedWordView(y) {
      assert(domain_type() == WDT_SIGNED);
    }
  };

  /**
   * \brief Constant view of an explicitly-sized word value
   * \ingroup TaskModelWordVars
   */
  class ConstWordView : public ConstView<WordView> {
  public:
    /// Whether this view exposes a persistent ranked interval
    static const bool supports_bounds=false;
    /// Whether the ranked interval uses signed two's-complement order
    static const bool signed_order=false;
    ConstWordView(void) : _width(1), _value(0) {}
    /// Construct the constant \a value with explicit \a width
    ConstWordView(unsigned int width, WordValue value)
      : _width(width), _value(value) {
      check_domain(width,value,value,"ConstWordView::ConstWordView");
    }

    unsigned int width(void) const { return _width; }
    WordValue mask(void) const { return width_mask(_width); }
    WordValue lo(void) const { return _value; }
    WordValue hi(void) const { return _value; }
    WordDomainType domain_type(void) const { return WDT_CUBE; }
    bool bounded(void) const { return false; }
    WordValue unknown(void) const { return 0; }
    unsigned int unknown_size(void) const { return 0; }
    WordValue val(void) const { return _value; }
    bool in(WordValue value) const { return value == _value; }

    /// Restrict to the cube described by \a lo and \a hi
    ModEvent narrow(Space&, WordValue lo, WordValue hi) {
      const bool has_excess_bits = ((lo | hi) & ~mask()) != 0;
      const bool is_empty_cube = (lo & ~hi) != 0;
      if (has_excess_bits || is_empty_cube)
        return ME_WORD_FAILED;
      const bool is_admitted =
        ((_value & lo) == lo) && ((_value & ~hi) == 0);
      return is_admitted ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    ModEvent eq(Space&, WordValue value) {
      return (_value == value) ? ME_WORD_NONE : ME_WORD_FAILED;
    }
    /// Return newly fixed-zero bits from a delta (none for constants)
    WordValue zero(const Delta&) const { return 0; }
    /// Return newly fixed-one bits from a delta (none for constants)
    WordValue one(const Delta&) const { return 0; }

    /// Update this view during cloning
    void update(Space& home, ConstWordView& y) {
      ConstView<WordView>::update(home,y);
      _width = y._width;
      _value = y._value;
    }
    /// Arbitrary stable ordering between constant views
    bool operator <(const ConstWordView& y) const {
      return (_width < y._width) ||
        ((_width == y._width) && (_value < y._value));
    }
  private:
    unsigned int _width;
    WordValue _value;
  };
}}

// STATISTICS: word-var

#endif
