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
    /// Test whether the domain is an inline value set
    bool has_value_set(void) const { return x->has_value_set(); }
    /// Test whether a tell may narrow beyond its request (see WordVarImp)
    bool may_close_beyond_request(void) const {
      return x->may_close_beyond_request();
    }
    /// Return the exact values of a view of width at most eight
    WordValueSet values(void) const { return x->values(); }
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
      return Word::encode_rank(domain_type(),width(),n);
    }
    WordValue unknown(void) const { return x->unknown(); }
    unsigned int unknown_size(void) const { return x->unknown_size(); }
    WordValue val(void) const { return x->val(); }
    bool in(WordValue n) const { return x->in(n); }
    ModEvent narrow(Space& home, WordValue lo, WordValue hi) {
      return x->narrow(home,lo,hi);
    }
    ModEvent eq(Space& home, WordValue n) { return x->eq(home,n); }
    /// Remove \a n, exactly with a value set (see WordVarImp::nq)
    ModEvent nq(Space& home, WordValue n) { return x->nq(home,n); }
    /// Intersect with \a set (see WordVarImp::narrow_values)
    ModEvent narrow_values(Space& home, const WordValueSet& set) {
      return x->narrow_values(home,set);
    }
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

  /** \brief Intersect typed \a x with \a cube and, in its rank order, with
   * \a interval
   */
  template<class View>
  forceinline ModEvent
  narrow_word_domain(Space& home, View x, WordCube cube,
                     WordRankInterval interval) {
    return x.narrow_domain(home,cube.lo,cube.hi,
                           interval.minimum,interval.maximum);
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

  /** \brief View of a float word with its interval in IEEE total order
   *
   * It gives the ranked tells access to a float word's interval. Integer
   * bound reasoning must not use it, so it declares no integer bounds.
   */
  class FloatOrderWordView : public BoundedWordView {
  public:
    /// Whether this view exposes a ranked interval to integer reasoning
    static const bool supports_bounds=false;
    /// Whether the ranked interval uses signed two's-complement order
    static const bool signed_order=false;
    FloatOrderWordView(void) {}
    FloatOrderWordView(const WordVar& y) : BoundedWordView(y) {
      assert(is_float_domain_type(domain_type()));
    }
    FloatOrderWordView(WordVarImp* y) : BoundedWordView(y) {
      assert(is_float_domain_type(domain_type()));
    }
  };

  /** \brief Combine the results of publishing local records to views
   *
   * Each result is ES_FAILED, ES_NOFIX when a value set closed a view beyond
   * its record, or ES_FIX. Failure wins, then ES_NOFIX.
   */
  forceinline ExecStatus
  combine_publication(ExecStatus x, ExecStatus y) {
    if ((x < ES_OK) || (y < ES_OK)) return ES_FAILED;
    return ((x == ES_NOFIX) || (y == ES_NOFIX)) ? ES_NOFIX : ES_FIX;
  }

  /** \brief Progress of one pass of a local fixpoint loop over records
   *
   * Whether a propagator computes its own fixpoint is decided for each
   * propagator. A cheap one, such as a channel, repeats its pass until
   * nothing changes and reports ES_FIX, as Int::Rel::EqBnd does. A
   * propagator that filters local records of its views repeats its pass
   * while the pass fixes bits, of which there are at most the summed
   * widths, or moves interval endpoints only to values its own deductions
   * computed. Closure of a record with its cube or value set can move an
   * endpoint further, a few values per pass, as when x+1 = z holds for two
   * even 64-bit words. A record loop stops after such a jump with no new
   * bit and reports ES_NOFIX, for a reason stated at the loop: an expensive
   * pass leaves the next one to the kernel, so that cheaper propagators
   * narrow first, and a bounds-only stage hands over to its pending cube
   * stage, which can decide at once (x+1 = z fails by parity). Becoming
   * assigned fixes bits, so a pass that ends with a jump leaves some record
   * unassigned.
   */
  enum class PassProgress {
    FAILED,  ///< A record has no value
    STABLE,  ///< No record changed: the records are at a local fixpoint
    CHANGED, ///< The pass fixed bits or moved endpoints by its deductions
    JUMPED   ///< Closure moved an endpoint past its deduction, with no new bit
  };

  /// Classify a pass that did not fail from what it changed
  forceinline PassProgress
  classify_pass(bool has_changed, bool has_new_bits, bool has_jump) {
    if (!has_changed) return PassProgress::STABLE;
    return (has_jump && !has_new_bits) ?
      PassProgress::JUMPED : PassProgress::CHANGED;
  }

  /// Result of filtering local records with passes and publishing them
  enum class PassResult {
    FAILED,        ///< A deduction or a tell failed
    FIXPOINT,      ///< The records are a fixpoint, published exactly
    STOPPED,       ///< The loop ended after a closure jump (see PassProgress)
    CLOSED_BEYOND  ///< A value set closed a view beyond its record
  };

  /** \brief Combine the publication status and the final pass progress
   *
   * \a publication is ES_FAILED, ES_NOFIX when a value set closed a view
   * beyond its record (see combine_publication), or ES_FIX. \a progress is
   * how the loop ended, STABLE or JUMPED.
   */
  forceinline PassResult
  pass_result(ExecStatus publication, PassProgress progress) {
    if (publication < ES_OK) return PassResult::FAILED;
    if (publication == ES_NOFIX) return PassResult::CLOSED_BEYOND;
    return (progress == PassProgress::JUMPED) ?
      PassResult::STOPPED : PassResult::FIXPOINT;
  }

  /** \brief Run \a pass until its publication closes no value set further
   *
   * A pass filters local records of its views and publishes them. When a
   * value set closed a view beyond its record, the pass's deductions ignore
   * values that are gone, so they are no fixpoint, and views that the
   * closure assigned may violate the relation. Each repetition filters the
   * closed domains again. Value sets exist only on words of at most eight
   * bits, and every repetition removes a value from one, so the repetitions
   * are few. The result is ES_FAILED, ES_FIX for a fixpoint, or ES_NOFIX
   * when the last pass stopped after a closure jump (see PassProgress).
   * Without value sets, publication is exact and \a pass runs once.
   */
  template<class Pass>
  forceinline ExecStatus
  repeat_until_closed(Pass pass) {
    for (;;) {
      switch (pass()) {
      case PassResult::FAILED: return ES_FAILED;
      case PassResult::FIXPOINT: return ES_FIX;
      case PassResult::STOPPED: return ES_NOFIX;
      case PassResult::CLOSED_BEYOND: break;
      }
    }
  }

  /** \brief Schedule the newly posted \a p unless posting filtered to a
   * fixpoint
   *
   * \a status is the result of the post-time filtering: ES_NOFIX when it
   * stopped after a closure jump, so that \a p continues during propagation.
   */
  forceinline void
  schedule_unless_fixpoint(Space& home, Propagator& p, ExecStatus status) {
    if (status == ES_NOFIX)
      WordView::schedule(home,p,ME_WORD_DOM);
  }

  /** \brief Return the values that the cube and bounds of \a x admit but its
   * value set excludes
   *
   * The result is empty for a view without a value set.
   */
  forceinline WordValueSet
  value_set_holes(WordView x) {
    if (!x.has_value_set())
      return {{0,0,0,0}};
    const WordRankInterval interval = x.bounded()
      ? WordRankInterval{x.rank_minimum(),x.rank_maximum()}
      : WordRankInterval{0,0};
    return subtract_value_sets(
      domain_values(x.width(),x.domain_type(),{{x.lo(),x.hi()},interval}),
      x.values());
  }

  /** \brief Return the weakest condition that sees every change to x.in()
   *
   * Membership of a word with bounds or a value set reads more than its
   * cube, and values can leave it under ME_WORD_BND or ME_WORD_HOLE, which
   * only PC_WORD_DOM schedules.
   */
  forceinline PropCond
  membership_condition(WordView x) {
    return (x.bounded() || x.has_value_set()) ? PC_WORD_DOM : PC_WORD_BITS;
  }

  /** \brief Intersect \a x with \a cube and, for a word with a rank
   * interval, with \a interval in its rank order
   *
   * A cube word has no persistent interval, so it takes only \a cube.
   */
  forceinline ModEvent
  narrow_word_domain(Space& home, WordView x, WordCube cube,
                     WordRankInterval interval) {
    switch (x.domain_type()) {
    case WDT_CUBE:
      return x.narrow(home,cube.lo,cube.hi);
    case WDT_UNSIGNED:
      return narrow_word_domain(home,UnsignedWordView(x.varimp()),
                                cube,interval);
    case WDT_SIGNED:
      return narrow_word_domain(home,SignedWordView(x.varimp()),
                                cube,interval);
    case WDT_FP32: case WDT_BF16: case WDT_BINARY16: case WDT_E4M3FN:
    case WDT_E2M1:
      return narrow_word_domain(home,FloatOrderWordView(x.varimp()),
                                cube,interval);
    }
    GECODE_NEVER;
    return ME_WORD_FAILED;
  }

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
    bool may_close_beyond_request(void) const { return false; }
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
