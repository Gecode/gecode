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

#include <gecode/float-word.hh>
#include <gecode/word/float-tiny.hpp>
#include <gecode/word/float.hpp>

#include <algorithm>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    /** \brief Return the common format of x and y, and of \a result unless
     * it is null, after checking the formats and then the policy
     *
     * Every word must have a float format, then x and y must match, their
     * format must be supported, and the result must match them.
     */
    WordDomainType validate_relation(const Policy& p, WordVar x, WordVar y,
                                     const WordVar* result, const char* name) {
      const WordDomainType format = Word::Float::float_format(x, name);
      const WordDomainType y_format = Word::Float::float_format(y, name);
      const WordDomainType result_format =
          result != nullptr ? Word::Float::float_format(*result, name) : format;
      if (format != y_format)
        throw Word::FormatMismatch(name);
      if (format == WDT_BINARY16)
        throw Word::OutOfLimits(name);
      if (result_format != format)
        throw Word::FormatMismatch(name);
      Word::Float::check(p, name);
      return format;
    }

    BoolVar combine(Home home, BoolVar a, BoolOpType op, BoolVar b) {
      BoolVar result(home, 0, 1);
      rel(home, a, op, b, result);
      return result;
    }
    /// Return the clause of \a op over \a positive and negated \a negative
    BoolVar literals(Home home, BoolOpType op, const BoolVarArgs& positive,
                     const BoolVarArgs& negative) {
      BoolVar result(home, 0, 1);
      clause(home, op, positive, negative, result);
      return result;
    }
    BoolVar bit_relation(Home home, WordVar x, WordRelType relation, WordVar y) {
      BoolVar result(home, 0, 1);
      rel(home, x, relation, y, Reify(result));
      return result;
    }
    BoolVar constant_relation(Home home, WordVar x, WordRelType relation,
                              WordValue value) {
      BoolVar result(home, 0, 1);
      rel(home, x, relation, value, Reify(result));
      return result;
    }
    WordVar choose(Home home, BoolVar condition, WordVar a, WordVar b) {
      WordVar result(home, a.width(), WDT_UNSIGNED);
      ite(home, condition, a, b, result);
      return result;
    }
    WordVar signed_zero(Home home, WordVar x) {
      WordVar result(home, x.width(), WDT_UNSIGNED);
      rel(home, x, WOT_AND, WordValue(1) << (x.width() - 1), result);
      return result;
    }

    /// Whether OperandClasses counts a subnormal as a number or a zero
    enum class SubnormalClass { Number, Zero };
    /// The class of the subnormal inputs of \a p
    SubnormalClass input_class(const Policy& p) {
      return p.input_subnormals == Policy::SignedZero ? SubnormalClass::Zero
                                                      : SubnormalClass::Number;
    }

    /** \brief Classes of one wide operand, each posted on first use
     *
     * A decomposition classifies each distinct operand word once and shares
     * the Booleans. Every class but the sign is a range of the magnitude,
     * the code without its sign bit, so it costs one reified relation on
     * the shared magnitude. With SubnormalClass::Zero, as for
     * \c input_subnormals SignedZero, zero() also holds for the
     * subnormals, which become zeros of their sign. The other classes are
     * the same for a subnormal and its effective zero.
     */
    class OperandClasses {
    public:
      OperandClasses(Home home0, WordVar x0, WordDomainType format,
                     SubnormalClass subnormal)
          : home(home0), x(x0), fraction(Word::Float::format_facts(format).fraction_bits),
            subnormal_class(subnormal) {}
      /// The operand word
      WordVar word(void) const {
        return x;
      }
      /// The code without its sign bit
      WordVar magnitude(void) {
        if (magnitude_word.varimp() == nullptr) {
          magnitude_word = WordVar(home, x.width() - 1, WDT_UNSIGNED);
          extract(home, x, 0, x.width() - 1, magnitude_word);
        }
        return magnitude_word;
      }
      /// Code of positive infinity
      WordValue infinity(void) const {
        return ((WordValue(1) << (x.width() - 1 - fraction)) - 1)
               << fraction;
      }
      /// Whether the operand is a NaN
      BoolVar nan(void) {
        if (is_nan.varimp() == nullptr)
          is_nan = constant_relation(home, magnitude(), WRT_UGR, infinity());
        return is_nan;
      }
      /// Whether the effective operand is a zero, per the subnormal class
      BoolVar zero(void) {
        if (is_zero.varimp() == nullptr) {
          is_zero = BoolVar(home, 0, 1);
          post_zero(magnitude(), is_zero);
        }
        return is_zero;
      }
      /// Whether zero() counts a subnormal as a number or a zero
      SubnormalClass subnormals(void) const {
        return subnormal_class;
      }
      /// Whether the sign bit is set, also for zeros and NaNs
      BoolVar negative(void) {
        if (is_negative.varimp() == nullptr) {
          is_negative = BoolVar(home, 0, 1);
          channel(home, x, x.width() - 1, is_negative);
        }
        return is_negative;
      }
      /** \brief The order key of the code
       *
       * The key complements the magnitude bits of a negative code, so the
       * signed order of keys is the order of numbers with -0 before +0:
       * a negative key is -1 minus the magnitude.
       */
      WordVar key(void) {
        if (order_key.varimp() == nullptr) {
          const unsigned int width = x.width();
          const WordValue magnitude_bits = (WordValue(1) << (width - 1)) - 1;
          WordVar mask = choose(home, negative(),
                                WordVar(home, width, {magnitude_bits}),
                                WordVar(home, width, {0}));
          order_key = WordVar(home, width, WDT_SIGNED);
          rel(home, x, WOT_XOR, mask, order_key);
        }
        return order_key;
      }
      /// Whether the operand is a signaling NaN
      BoolVar signaling(void) {
        if (is_signaling.varimp() == nullptr) {
          BoolVar quiet(home, 0, 1);
          channel(home, x, fraction - 1, quiet);
          is_signaling = literals(home, BOT_AND, {nan()}, {quiet});
        }
        return is_signaling;
      }

    private:
      /// Post that \a truth holds when \a code_magnitude is that of a zero
      void post_zero(WordVar code_magnitude, BoolVar truth) {
        if (subnormal_class == SubnormalClass::Zero)
          rel(home, code_magnitude, WRT_ULE, WordValue(1) << fraction,
              Reify(truth));
        else
          rel(home, code_magnitude, WRT_EQ, 0, Reify(truth));
      }

      Home home;
      WordVar x;
      const unsigned int fraction;
      const SubnormalClass subnormal_class;
      WordVar magnitude_word, order_key;
      BoolVar is_nan, is_zero, is_negative, is_signaling;
    };

    /** \brief Return whether a precedes b, as numbers with -0 before +0
     *
     * NaN operands give an arbitrary order. On original operands with
     * \c input_subnormals SignedZero this is also the order of the
     * effective operands unless both become zeros: a subnormal lies
     * strictly between the zeros and every other value of its sign.
     */
    BoolVar precedes(Home home, OperandClasses& a, OperandClasses& b) {
      return bit_relation(home, a.key(), WRT_SLE, b.key());
    }
    void status_flags(Home home, BoolVar invalid, BoolVar flushed,
                      IntVar status) {
      linear(home,
             IntArgs{Word::Float::invalid,
                     Word::Float::underflow | Word::Float::inexact},
             BoolVarArgs{invalid, flushed}, IRT_EQ, status);
    }
    /// The NaN tests whose disjunction makes a and b unordered
    BoolVarArgs nan_tests(OperandClasses& a, OperandClasses& b,
                          BoolVar unordered) {
      if (unordered.varimp() != nullptr)
        return BoolVarArgs{unordered};
      return BoolVarArgs{a.nan(), b.nan()};
    }
    /** \brief Post \a truth as \a relation between the distinct words of a
     * and b
     *
     * Unless it is null, \a unordered is the disjunction of the NaN tests
     * of a and b, and the predicate links the truth to it, so that a status
     * sharing it correlates with the truth. A clause that holds the single
     * tests learns only their disjunction from a false truth, so the order
     * clauses use \a unordered in their place, which propagates as they
     * do through its own disjunction.
     */
    void post_predicate(Home home, OperandClasses& a, OperandClasses& b,
                        FloatWordRelType relation, BoolVar truth,
                        BoolVar unordered) {
      const bool shared = unordered.varimp() != nullptr;
      // Both operands are zeros only when they are ordered, and equal codes
      // are equal numbers unless they are NaNs. Precedence decides the
      // remaining ordered pairs, also when one effective operand is a zero.
      // With input flushing a subnormal is an effective zero that keeps the
      // nonzero key of its code, so the precedence of a pair in which one
      // operand may be a zero cannot tell whether the other is one too.
      // The order relations add the order of the zeros against the sign
      // bit, which the precedence of the codes does not see.
      switch (relation) {
      case FWRT_EQ:
      case FWRT_NQ: {
        // Two zeros and equal codes of a number are ordered, so the NaN
        // test of a suffices for the clause.
        BoolVar same = combine(home, combine(home, a.zero(), BOT_AND, b.zero()),
                               BOT_OR,
                               bit_relation(home, a.word(), WRT_EQ, b.word()));
        if (relation == FWRT_EQ)
          clause(home, BOT_AND, {same}, {a.nan()}, truth);
        else
          clause(home, BOT_OR, {a.nan()}, {same}, truth);
        // A NaN b is never equal, which the equality of the words cannot
        // tell, so a link adds b's test, or the shared one that covers
        // both. The clause keeps a's test alone, so that a false truth of
        // two same operands makes a a NaN.
        rel(home, shared ? unordered : b.nan(),
            relation == FWRT_EQ ? BOT_AND : BOT_IMP, truth,
            relation == FWRT_EQ ? 0 : 1);
        break;
      }
      case FWRT_LE:
      case FWRT_GR: {
        OperandClasses& less = relation == FWRT_LE ? a : b;
        OperandClasses& greater = relation == FWRT_LE ? b : a;
        BoolVarArgs excluded = nan_tests(a, b, unordered);
        excluded << combine(home, a.zero(), BOT_AND, b.zero());
        clause(home, BOT_AND, {precedes(home, less, greater)}, excluded,
               truth);
        if (a.subnormals() == SubnormalClass::Zero) {
          // The numbers below a zero of either sign are exactly those with
          // a set sign bit that are not zeros, and those above it exactly
          // those with a clear sign bit that are not zeros. A true truth
          // excludes NaNs and two zeros, so with an effective zero greater
          // it sets the sign bit of less, and with an effective zero less
          // it clears that of greater.
          clause(home, BOT_OR, {less.negative()}, {truth, greater.zero()}, 1);
          clause(home, BOT_OR, BoolVarArgs(),
                 {truth, less.zero(), greater.negative()}, 1);
        }
        break;
      }
      case FWRT_LQ:
      case FWRT_GQ: {
        OperandClasses& less = relation == FWRT_LQ ? a : b;
        OperandClasses& greater = relation == FWRT_LQ ? b : a;
        BoolVar zeros = combine(home, a.zero(), BOT_AND, b.zero());
        BoolVar not_greater = literals(home, BOT_OR, {zeros},
                                       {precedes(home, greater, less)});
        clause(home, BOT_AND, {not_greater}, nan_tests(a, b, unordered),
               truth);
        if (a.subnormals() == SubnormalClass::Zero) {
          // Every number with a clear sign bit, +0 included, is at least
          // a zero of either sign, and every number with a set sign bit,
          // -0 included, at most one. So an effective zero less and a
          // greater with a clear sign bit, or an effective zero greater
          // and a less with a set sign bit, satisfy the relation unless
          // the other operand is a NaN, as a zero is not.
          clause(home, BOT_OR,
                 {truth, greater.negative(), shared ? unordered : greater.nan()},
                 {less.zero()}, 1);
          clause(home, BOT_OR, {truth, shared ? unordered : less.nan()},
                 {greater.zero(), less.negative()}, 1);
        }
        break;
      }
      case FWRT_UNORDERED:
        if (shared)
          rel(home, unordered, IRT_EQ, truth);
        else
          rel(home, a.nan(), BOT_OR, b.nan(), truth);
        break;
      case FWRT_ORDERED:
        clause(home, BOT_AND, BoolVarArgs(), nan_tests(a, b, unordered),
               truth);
        break;
      default:
        GECODE_NEVER;
      }
    }
    /// Post \a truth as \a relation of the word of a with itself
    void post_reflexive(Home home, OperandClasses& a,
                        FloatWordRelType relation, BoolVar truth) {
      switch (relation) {
      case FWRT_LE:
      case FWRT_GR:
        rel(home, truth, IRT_EQ, 0);
        break;
      case FWRT_NQ:
      case FWRT_UNORDERED:
        rel(home, a.nan(), IRT_EQ, truth);
        break;
      case FWRT_EQ:
      case FWRT_LQ:
      case FWRT_GQ:
      case FWRT_ORDERED:
        rel(home, a.nan(), IRT_NQ, truth);
        break;
      default:
        GECODE_NEVER;
      }
    }

    /// The facts of the tiny format (E2M1 or E4M3FN) with sign bit \a sign
    Word::Float::FormatFacts tiny_facts(unsigned int sign) {
      constexpr Word::Float::FormatFacts e2m1 =
          Word::Float::format_facts(WDT_E2M1);
      return sign == e2m1.sign() ? e2m1 : Word::Float::format_facts(WDT_E4M3FN);
    }
    // Tiny formats have no signaling NaNs. Their operations compare and
    // select the effective encodings, after input subnormals are treated.
    unsigned int effective_code(unsigned int code, unsigned int sign,
                                const Policy& p) {
      const unsigned int magnitude = code & (sign - 1);
      return p.input_subnormals == Policy::SignedZero &&
                     magnitude < tiny_facts(sign).normal()
                 ? code & sign
                 : code;
    }
    bool tiny_nan(unsigned int code, unsigned int sign) {
      return tiny_facts(sign).is_nan(code & (sign - 1));
    }
    // Tiny magnitude codes increase with the value they denote, so the key
    // of a number orders it numerically. Both zeros have key 0.
    int tiny_key(unsigned int code, unsigned int sign) {
      const int magnitude = static_cast<int>(code & (sign - 1));
      return (code & sign) != 0 ? -magnitude : magnitude;
    }
    int tiny_order(unsigned int a, unsigned int b, unsigned int sign) {
      const int ak = tiny_key(a, sign), bk = tiny_key(b, sign);
      return ak < bk ? -1 : ak > bk ? 1 : 0;
    }

    enum class Outcome { Less, Equal, Greater, Unordered };
    Outcome tiny_outcome(unsigned int a, unsigned int b, unsigned int sign) {
      if (tiny_nan(a, sign) || tiny_nan(b, sign))
        return Outcome::Unordered;
      const int order = tiny_order(a, b, sign);
      return order < 0 ? Outcome::Less
             : order > 0 ? Outcome::Greater
                         : Outcome::Equal;
    }
    bool holds(FloatWordRelType relation, Outcome outcome) {
      const bool unordered = outcome == Outcome::Unordered;
      switch (relation) {
      case FWRT_EQ:
        return outcome == Outcome::Equal;
      case FWRT_NQ:
        return outcome != Outcome::Equal;
      case FWRT_LE:
        return outcome == Outcome::Less;
      case FWRT_LQ:
        return outcome == Outcome::Less || outcome == Outcome::Equal;
      case FWRT_GR:
        return outcome == Outcome::Greater;
      case FWRT_GQ:
        return outcome == Outcome::Greater || outcome == Outcome::Equal;
      case FWRT_UNORDERED:
        return unordered;
      case FWRT_ORDERED:
        return !unordered;
      default:
        GECODE_NEVER;
      }
      return false;
    }

    struct Selection {
      unsigned int code;
      int flags;
    };
    // Output subnormal treatment and then the zeros policy of a selection.
    Selection tiny_output(unsigned int code, unsigned int sign,
                          const Policy& p) {
      int flags = 0;
      const unsigned int magnitude = code & (sign - 1);
      if (p.output_subnormals == Policy::SignedZero && magnitude > 0 &&
          magnitude < tiny_facts(sign).normal()) {
        code &= sign;
        flags = Word::Float::underflow | Word::Float::inexact;
      }
      if (p.zeros == Policy::Positive && (code & (sign - 1)) == 0)
        code = 0;
      return {code, flags};
    }
    /// The operand that a selection prefers
    enum class Extreme { Minimum, Maximum };
    /// How a selection treats a NaN operand
    enum class NanRule {
      Propagate,   ///< A NaN operand selects a NaN (fp_minimum)
      PreferNumber ///< A number is selected over a NaN (fp_minimum_number)
    };
    /// The position of an operand of a binary operation
    enum class Position { First, Second };

    Selection tiny_selection(unsigned int a, unsigned int b,
                             unsigned int sign, const Policy& p,
                             Extreme extreme, NanRule nans) {
      const bool maximum = extreme == Extreme::Maximum;
      const bool number = nans == NanRule::PreferNumber;
      const bool an = tiny_nan(a, sign), bn = tiny_nan(b, sign);
      unsigned int code;
      if ((an || bn) && (!number || (an && bn)))
        code = p.nan == Policy::Canonical
                   ? static_cast<unsigned int>(tiny_facts(sign).canonical_nan())
               : an ? a
                    : b;
      else if (an || bn)
        code = an ? b : a;
      else if ((a & (sign - 1)) == 0 && (b & (sign - 1)) == 0)
        code = maximum ? a & b : a | b;
      else
        code = (maximum ? tiny_order(a, b, sign) >= 0
                        : tiny_order(a, b, sign) <= 0)
                   ? a
                   : b;
      return tiny_output(code, sign, p);
    }

    /// NaNs and numerical bounds of the effective encodings of a tiny word
    struct TinyBounds {
      bool has_nan = false, has_number = false;
      int least = 0, greatest = 0;
    };
    TinyBounds tiny_bounds(const Word::Float::TinyValues& values,
                           unsigned int sign, const Policy& p) {
      TinyBounds bounds;
      for (unsigned int i = 0; i < values.size(); ++i) {
        const unsigned int code =
            effective_code(static_cast<unsigned int>(values[i]), sign, p);
        if (tiny_nan(code, sign)) {
          bounds.has_nan = true;
          continue;
        }
        const int key = tiny_key(code, sign);
        bounds.least = bounds.has_number ? std::min(bounds.least, key) : key;
        bounds.greatest =
            bounds.has_number ? std::max(bounds.greatest, key) : key;
        bounds.has_number = true;
      }
      return bounds;
    }

    /// Comparison outcomes, as those that operand bounds allow
    struct AllowedOutcomes {
      bool less, equal, greater, unordered;
    };
    /// Outcomes that some pair of values within \a x and \a y can have
    AllowedOutcomes possible_outcomes(const TinyBounds& x,
                                      const TinyBounds& y) {
      const bool numbers = x.has_number && y.has_number;
      return {numbers && x.least < y.greatest,
              numbers && x.least <= y.greatest && y.least <= x.greatest,
              numbers && x.greatest > y.least, x.has_nan || y.has_nan};
    }
    using Word::Float::failed_change;

    /// Test whether a given status is assigned
    bool status_fixed(Int::IntView status) {
      return status.assigned();
    }
    /// An omitted status admits every flag and constrains nothing
    bool status_fixed(Word::Float::OmittedStatus) {
      return true;
    }

    /** \brief Tiny comparison of x and y
     *
     * The exact outcome compares the effective encodings. The default
     * reasoning decides truth and status from the outcomes that the least
     * and greatest numerical values and the NaNs of x and y allow, and keeps
     * each operand value that an allowed outcome supports against the other
     * operand's bounds.
     */
    struct TinyCompare {
      Policy policy;
      FloatWordRelType relation;
      unsigned int sign;

      int flag(Outcome outcome) const {
        return policy.comparison == Policy::SignalingComparison &&
                       outcome == Outcome::Unordered
                   ? 1
                   : 0;
      }
      Word::Float::TinyOutcome operator()(WordValue x, WordValue y) const {
        const Outcome outcome = tiny_outcome(
            effective_code(static_cast<unsigned int>(x), sign, policy),
            effective_code(static_cast<unsigned int>(y), sign, policy), sign);
        return {holds(relation, outcome) ? 1U : 0U, flag(outcome)};
      }
      template<class StatusView>
      bool allows(Outcome outcome, Int::BoolView truth,
                  StatusView status) const {
        return truth.in(holds(relation, outcome) ? 1 : 0) &&
               status.in(flag(outcome));
      }
      /// Test whether an allowed outcome supports \a value of an operand
      /// against the \a other operand, both read as the first operand.
      bool supports(WordValue value, const TinyBounds& other,
                    const AllowedOutcomes& allowed) const {
        const unsigned int code =
            effective_code(static_cast<unsigned int>(value), sign, policy);
        if (tiny_nan(code, sign))
          return allowed.unordered;
        const int key = tiny_key(code, sign);
        return (allowed.unordered && other.has_nan) ||
               (other.has_number &&
                ((allowed.less && key < other.greatest) ||
                 (allowed.equal && other.least <= key &&
                  key <= other.greatest) ||
                 (allowed.greater && key > other.least)));
      }
      ModEvent narrow_operand(Space& home, Word::WordView x,
                              const Word::Float::TinyValues& values,
                              const TinyBounds& other,
                              const AllowedOutcomes& allowed) const {
        Word::WordValueSet kept = {{0, 0, 0, 0}};
        for (unsigned int i = 0; i < values.size(); ++i)
          if (supports(values[i], other, allowed))
            kept = Word::with_value(kept, values[i]);
        return x.narrow_values(home, kept);
      }
      template<class StatusView>
      ExecStatus narrow_bounds(Space& home, Word::WordView x, Word::WordView y,
                               Int::BoolView truth, StatusView status) const {
        const Word::Float::TinyValues xs(x), ys(y);
        const TinyBounds bx = tiny_bounds(xs, sign, policy),
                         by = tiny_bounds(ys, sign, policy);
        const AllowedOutcomes possible = possible_outcomes(bx, by);
        const AllowedOutcomes allowed = {
            possible.less && allows(Outcome::Less, truth, status),
            possible.equal && allows(Outcome::Equal, truth, status),
            possible.greater && allows(Outcome::Greater, truth, status),
            possible.unordered && allows(Outcome::Unordered, truth, status)};
        unsigned int truths = 0, flags = 0;
        const Outcome outcomes[] = {Outcome::Less, Outcome::Equal,
                                    Outcome::Greater, Outcome::Unordered};
        const bool is_allowed[] = {allowed.less, allowed.equal,
                                   allowed.greater, allowed.unordered};
        for (int i = 0; i < 4; ++i)
          if (is_allowed[i]) {
            truths |= holds(relation, outcomes[i]) ? 2U : 1U;
            flags |= 1U << flag(outcomes[i]);
          }
        if (truths == 0)
          return ES_FAILED;
        // The second operand sees the outcomes of (y, x).
        const AllowedOutcomes mirrored = {allowed.greater, allowed.equal,
                                          allowed.less, allowed.unordered};
        bool changed = false;
        if (failed_change(truths == 2U   ? truth.one(home)
                          : truths == 1U ? truth.zero(home)
                                         : Int::ME_BOOL_NONE,
                          changed) ||
            failed_change(Word::Float::narrow_flags(home, status, flags),
                          changed) ||
            failed_change(narrow_operand(home, x, xs, by, allowed), changed) ||
            failed_change(narrow_operand(home, y, ys, bx, mirrored), changed))
          return ES_FAILED;
        return changed ? ES_NOFIX : ES_FIX;
      }
      /** \brief Test whether truth and status are fixed and every pair of
       * operand values gives them
       *
       * Every outcome that the operand bounds make possible must give the
       * fixed truth and status, so every remaining tuple is a support. A
       * word compared with itself is equal or unordered. Otherwise the
       * bounds over-approximate the equal outcome, so this misses some
       * entailed comparisons, such as disjoint operands with overlapping
       * bounds under EQ.
       */
      template<class StatusView>
      bool entailed(Word::WordView x, Word::WordView y, Int::BoolView truth,
                    StatusView status) const {
        if (!truth.assigned() || !status_fixed(status))
          return false;
        const TinyBounds bx =
            tiny_bounds(Word::Float::TinyValues(x), sign, policy);
        const AllowedOutcomes possible =
            x == y ? AllowedOutcomes{false, bx.has_number, false, bx.has_nan}
                   : possible_outcomes(
                         bx, tiny_bounds(Word::Float::TinyValues(y), sign,
                                         policy));
        return (!possible.less || allows(Outcome::Less, truth, status)) &&
               (!possible.equal || allows(Outcome::Equal, truth, status)) &&
               (!possible.greater ||
                allows(Outcome::Greater, truth, status)) &&
               (!possible.unordered ||
                allows(Outcome::Unordered, truth, status));
      }
    };

    static_assert(Word::Float::TinyBinary<TinyCompare, Int::BoolView,
                                          Int::IntView>::reports_entailment &&
                      Word::Float::TinyBinary<
                          TinyCompare, Int::BoolView,
                          Word::Float::OmittedStatus>::reports_entailment,
                  "TinyBinary must detect TinyCompare::entailed()");

    /// One operand of a tiny minimum or maximum, read by position in the
    /// selection order
    struct SelectOperand {
      bool has_nan = false, has_number = false;
      /// Whether some NaN gives an admitted result as the first NaN
      bool has_admitted_nan = false;
      /// Whether some number gives an admitted result when selected
      bool has_admitted_number = false;
      int least = 0, greatest = 0, least_admitted = 0;
    };

    /** \brief Tiny minimum or maximum of x and y
     *
     * The exact outcome selects from the effective encodings. The default
     * reasoning reads each operand by its position in the selection order:
     * the key for the minimum and its negation for the maximum, so that both
     * select the least position. It keeps each operand value that the other
     * operand's least and greatest position and NaNs can pair with an
     * admitted result, and each result that such a pair can select. Ties of
     * zeros keep both signs whenever a zero result is admitted.
     */
    struct TinySelect {
      Policy policy;
      unsigned int sign;
      Extreme extreme;
      NanRule nans;

      /// Whether a number is selected over a NaN
      bool prefers_number(void) const {
        return nans == NanRule::PreferNumber;
      }
      Word::Float::TinyOutcome operator()(WordValue x, WordValue y) const {
        const Selection s = tiny_selection(
            effective_code(static_cast<unsigned int>(x), sign, policy),
            effective_code(static_cast<unsigned int>(y), sign, policy), sign,
            policy, extreme, nans);
        return {s.code, s.flags};
      }
      /// The position of the number \a code in the selection order, in
      /// which the least position is selected and both zeros share 0
      int order_position(unsigned int code) const {
        return extreme == Extreme::Maximum ? -tiny_key(code, sign)
                                           : tiny_key(code, sign);
      }
      unsigned int nan_result(unsigned int code) const {
        return policy.nan == Policy::Canonical
                   ? static_cast<unsigned int>(tiny_facts(sign).canonical_nan())
                   : code;
      }
      template<class StatusView>
      bool admits(unsigned int code, Word::WordView result,
                  StatusView status) const {
        const Selection output = tiny_output(code, sign, policy);
        return result.in(output.code) && status.in(output.flags);
      }
      unsigned int effective(WordValue value) const {
        return effective_code(static_cast<unsigned int>(value), sign, policy);
      }
      template<class StatusView>
      SelectOperand summarise(const Word::Float::TinyValues& values,
                              Word::WordView result,
                              StatusView status) const {
        SelectOperand s;
        for (unsigned int i = 0; i < values.size(); ++i) {
          const unsigned int code = effective(values[i]);
          if (tiny_nan(code, sign)) {
            s.has_nan = true;
            s.has_admitted_nan = s.has_admitted_nan ||
                                 admits(nan_result(code), result, status);
            continue;
          }
          const int r = order_position(code);
          s.least = s.has_number ? std::min(s.least, r) : r;
          s.greatest = s.has_number ? std::max(s.greatest, r) : r;
          s.has_number = true;
          if (admits(code, result, status)) {
            s.least_admitted =
                s.has_admitted_number ? std::min(s.least_admitted, r) : r;
            s.has_admitted_number = true;
          }
        }
        return s;
      }
      // A number meets a greater position (selected itself, ties select an
      // equal encoding), a lesser position (selects the other number), or a
      // NaN.
      template<class StatusView>
      bool keeps_number(unsigned int code, const SelectOperand& other,
                        bool zero_admitted, Word::WordView result,
                        StatusView status) const {
        const int r = order_position(code);
        const bool admitted = admits(code, result, status);
        if (other.has_number) {
          const bool zero_tie = r == 0 && zero_admitted && other.least <= 0;
          if (r <= other.greatest && (admitted || zero_tie))
            return true;
          if (other.has_admitted_number && other.least_admitted < r)
            return true;
        }
        return other.has_nan &&
               (prefers_number() ? admitted : other.has_admitted_nan);
      }
      // The first NaN of a pair supplies a NaN result.
      template<class StatusView>
      bool keeps_nan(unsigned int code, Position position,
                     const SelectOperand& other, Word::WordView result,
                     StatusView status) const {
        const bool is_first = position == Position::First;
        const bool own = admits(nan_result(code), result, status);
        if (prefers_number())
          return other.has_admitted_number ||
                 (other.has_nan && (is_first ? own : other.has_admitted_nan));
        return is_first ? own
                        : (other.has_number && own) || other.has_admitted_nan;
      }
      template<class StatusView>
      ModEvent narrow_operand(Space& home, Word::WordView x,
                              const Word::Float::TinyValues& values,
                              Position position, const SelectOperand& other,
                              bool zero_admitted, Word::WordView result,
                              StatusView status) const {
        Word::WordValueSet kept = {{0, 0, 0, 0}};
        for (unsigned int i = 0; i < values.size(); ++i) {
          const unsigned int code = effective(values[i]);
          const bool keep =
              tiny_nan(code, sign)
                  ? keeps_nan(code, position, other, result, status)
                  : keeps_number(code, other, zero_admitted, result, status);
          if (keep)
            kept = Word::with_value(kept, values[i]);
        }
        return x.narrow_values(home, kept);
      }
      /// Results and flags of the selectable candidates
      struct Candidates {
        Word::WordValueSet codes = {{0, 0, 0, 0}};
        unsigned int flags = 0;
        bool has_zero = false;
      };
      template<class StatusView>
      void add_candidate(unsigned int code, Candidates& candidates,
                         Word::WordView result, StatusView status) const {
        const Selection output = tiny_output(code, sign, policy);
        if (!result.in(output.code) || !status.in(output.flags))
          return;
        candidates.codes = Word::with_value(candidates.codes, output.code);
        candidates.flags |= 1U << output.flags;
      }
      // Candidates of the operand values, as the first or second operand.
      template<class StatusView>
      void add_candidates(const Word::Float::TinyValues& values,
                          Position position, const SelectOperand& other,
                          Candidates& candidates, Word::WordView result,
                          StatusView status) const {
        const bool is_first = position == Position::First;
        const bool number = prefers_number();
        for (unsigned int i = 0; i < values.size(); ++i) {
          const unsigned int code = effective(values[i]);
          if (tiny_nan(code, sign)) {
            const bool selectable =
                number ? is_first && other.has_nan
                       : is_first || other.has_number;
            if (selectable)
              add_candidate(nan_result(code), candidates, result, status);
            continue;
          }
          const int r = order_position(code);
          const bool selectable =
              (other.has_number &&
               (is_first ? r <= other.greatest : r < other.greatest)) ||
              (number && other.has_nan);
          if (!selectable)
            continue;
          candidates.has_zero = candidates.has_zero || r == 0;
          add_candidate(code, candidates, result, status);
        }
      }
      template<class StatusView>
      ExecStatus narrow_bounds(Space& home, Word::WordView x, Word::WordView y,
                               Word::WordView result,
                               StatusView status) const {
        const Word::Float::TinyValues xs(x), ys(y);
        const SelectOperand sx = summarise(xs, result, status),
                            sy = summarise(ys, result, status);
        const bool zero_admitted =
            admits(0, result, status) || admits(sign, result, status);
        Candidates candidates;
        add_candidates(xs, Position::First, sy, candidates, result, status);
        add_candidates(ys, Position::Second, sx, candidates, result, status);
        if (candidates.has_zero) {
          add_candidate(0, candidates, result, status);
          add_candidate(sign, candidates, result, status);
        }
        if (candidates.flags == 0)
          return ES_FAILED;
        bool changed = false;
        if (failed_change(narrow_operand(home, x, xs, Position::First, sy,
                                         zero_admitted,
                                         result, status),
                          changed) ||
            failed_change(narrow_operand(home, y, ys, Position::Second, sx,
                                         zero_admitted,
                                         result, status),
                          changed) ||
            failed_change(result.narrow_values(home, candidates.codes),
                          changed) ||
            failed_change(
                Word::Float::narrow_flags(home, status, candidates.flags),
                changed))
          return ES_FAILED;
        return changed ? ES_NOFIX : ES_FIX;
      }
    };

    /// Post a tiny binary operation, with the status unless it is null
    template<class Operation, class ResultView>
    ExecStatus post_tiny(Home home, WordVar x, WordVar y, ResultView result,
                         IntVar* status, const Operation& operation,
                         IntPropLevel ipl) {
      const Word::Float::Consistency level = Word::Float::consistency(ipl);
      if (status != nullptr)
        return Word::Float::TinyBinary<Operation, ResultView, Int::IntView>::
            post(home, x, y, result, *status, operation, level);
      return Word::Float::TinyBinary<Operation, ResultView,
                                     Word::Float::OmittedStatus>::
          post(home, x, y, result, Word::Float::OmittedStatus(), operation,
               level);
    }

    /// Narrow a given status to the flag range 0..31
    ExecStatus narrow_status(Home home, IntVar* status) {
      if (status == nullptr)
        return ES_OK;
      Int::IntView flags(*status);
      GECODE_ME_CHECK(flags.gq(home, 0));
      GECODE_ME_CHECK(flags.lq(home, 31));
      return ES_OK;
    }

    void post_compare(Home home, const Policy& p, WordVar x,
                      FloatWordRelType relation, WordVar y, BoolVar truth,
                      IntVar* status, IntPropLevel ipl) {
      const char* name = "Word::fp_compare";
      const WordDomainType format = validate_relation(p, x, y, nullptr, name);
      if (relation < FWRT_EQ || relation > FWRT_ORDERED)
        throw Word::OutOfLimits(name);
      GECODE_POST;
      GECODE_ES_FAIL(narrow_status(home, status));
      const unsigned int width = x.width();
      if (width <= 8) {
        GECODE_ES_FAIL(post_tiny(home, x, y, Int::BoolView(truth), status,
                                 TinyCompare{p, relation, 1U << (width - 1)},
                                 ipl));
        return;
      }
      OperandClasses a(home, x, format, input_class(p)),
          b(home, y, format, input_class(p));
      const bool reflexive = x.varimp() == y.varimp();
      // A signaling comparison flags exactly the unordered operands. The
      // truth and the status share that one test, so that each informs the
      // other: a false ORDERED sets the flag, the flag makes NQ true.
      const bool signaling_status =
          status != nullptr && p.comparison == Policy::SignalingComparison;
      BoolVar unordered;
      if (signaling_status && !reflexive)
        unordered = combine(home, a.nan(), BOT_OR, b.nan());
      if (reflexive)
        post_reflexive(home, a, relation, truth);
      else
        post_predicate(home, a, b, relation, truth, unordered);
      if (status == nullptr)
        return;
      BoolVar invalid;
      if (signaling_status)
        invalid = reflexive ? a.nan() : unordered;
      else
        invalid = reflexive ? a.signaling()
                            : combine(home, a.signaling(), BOT_OR,
                                      b.signaling());
      channel(home, invalid, *status);
    }

    /// The quieted NaN that the policy selects when a or b is a NaN
    WordVar select_nan(Home home, const Policy& p, OperandClasses& a,
                       OperandClasses& b, WordDomainType format) {
      const unsigned int width = a.word().width();
      if (p.nan == Policy::Canonical)
        return WordVar(home, width,
                       {Word::Float::format_facts(format).canonical_nan()});
      WordVar selected = a.word();
      if (a.word().varimp() != b.word().varimp()) {
        // The first signaling NaN, otherwise the first NaN.
        BoolVar take_a = a.nan();
        if (p.nan == Policy::SignalFirst)
          take_a = combine(home, a.nan(), BOT_AND,
                           literals(home, BOT_OR, {a.signaling()},
                                    {b.signaling()}));
        selected = choose(home, take_a, a.word(), b.word());
      }
      WordVar result(home, width, WDT_UNSIGNED);
      rel(home, selected, WOT_OR, Word::Float::format_facts(format).quiet(),
          result);
      return result;
    }
    void post_minmax(Home home, const Policy& p, WordVar x, WordVar y,
                     WordVar result, IntVar* status, Extreme extreme,
                     NanRule nans, IntPropLevel ipl, const char* name) {
      const bool maximum = extreme == Extreme::Maximum;
      const WordDomainType format = validate_relation(p, x, y, &result, name);
      GECODE_POST;
      GECODE_ES_FAIL(narrow_status(home, status));
      const unsigned int width = x.width();
      if (width <= 8) {
        GECODE_ES_FAIL(post_tiny(
            home, x, y, Word::WordView(result), status,
            TinySelect{p, 1U << (width - 1), extreme, nans}, ipl));
        return;
      }
      // Select among the original operands, then flush the selection: NaNs
      // are never subnormal, the precedence of the originals orders the
      // effective operands unless both are zeros, and the tie of two
      // effective zeros flushes to the zero of the selected sign.
      const bool flush_inputs = p.input_subnormals == Policy::SignedZero;
      const bool flush_outputs = p.output_subnormals == Policy::SignedZero;
      OperandClasses a(home, x, format, input_class(p)),
          b(home, y, format, input_class(p));
      const bool reflexive = x.varimp() == y.varimp();
      WordVar numeric = x;
      BoolVar use_nan = a.nan();
      if (!reflexive) {
        // Two zeros select the negative zero for the minimum and the
        // positive zero for the maximum, bit by bit.
        WordVar tie(home, width, WDT_UNSIGNED);
        rel(home, x, maximum ? WOT_AND : WOT_OR, y, tie);
        BoolVar zeros = combine(home, a.zero(), BOT_AND, b.zero());
        BoolVar less = precedes(home, a, b);
        numeric = maximum ? choose(home, less, y, choose(home, zeros, tie, x))
                          : choose(home, less, x, choose(home, zeros, tie, y));
        if (nans == NanRule::PreferNumber) {
          use_nan = combine(home, a.nan(), BOT_AND, b.nan());
          numeric =
              choose(home, a.nan(), y, choose(home, b.nan(), x, numeric));
        } else {
          use_nan = combine(home, a.nan(), BOT_OR, b.nan());
        }
      }
      WordVar selected =
          choose(home, use_nan, select_nan(home, p, a, b, format), numeric);
      // The choice of the NaN cannot tell from a result that is not a NaN
      // that no NaN was selected, as its operands need not be NaNs. But a
      // selected NaN is a NaN, a number selects an operand that is not a
      // NaN or the tie of two zeros, and the treatment of zeros keeps
      // exactly the NaNs, so the result is a NaN exactly when use_nan is.
      // The test sits on the flushed result, where it propagates further
      // than on the selection, and otherwise on the selection.
      BoolVar flushed(home, 0, 0);
      if (flush_inputs || flush_outputs) {
        // A zero or subnormal selection becomes the zero of its sign; only
        // a subnormal that output flushing replaces sets flags.
        OperandClasses s(home, selected, format, SubnormalClass::Zero);
        if (status != nullptr && !flush_inputs)
          flushed = literals(home, BOT_AND, {s.zero()},
                             {constant_relation(home, s.magnitude(), WRT_EQ,
                                                0)});
        selected = p.zeros == Policy::Positive
                       ? choose(home, s.zero(), WordVar(home, width, {0}),
                                selected)
                       : choose(home, s.zero(), signed_zero(home, selected),
                                selected);
        // Both branches of the choice can be zeros, so it cannot tell from
        // the result whether the selection was replaced. The flushed result
        // is a zero exactly when the selection is a zero or subnormal, and
        // never subnormal: a replaced selection gives a zero of either
        // policy, and a kept one, which a NaN always is, has a magnitude of
        // at least the least normal. So both "the result magnitude is 0"
        // and "it is below the least normal" are equivalent to s.zero(),
        // under either flush setting, zeros policy and NaN policy.
        OperandClasses r(home, selected, format, SubnormalClass::Number);
        rel(home, r.magnitude(), WRT_EQ, 0, Reify(s.zero()));
        rel(home, r.magnitude(), WRT_ULE,
            Word::Float::format_facts(format).normal(), Reify(s.zero()));
        rel(home, r.magnitude(), WRT_UGR, r.infinity(), Reify(use_nan));
      } else {
        OperandClasses s(home, selected, format, SubnormalClass::Number);
        if (p.zeros == Policy::Positive) {
          selected =
              choose(home, s.zero(), WordVar(home, width, {0}), selected);
          // As above, the result is +0 exactly when the selection is a
          // zero.
          rel(home, selected, WRT_EQ, 0, Reify(s.zero()));
        }
        rel(home, s.magnitude(), WRT_UGR, s.infinity(), Reify(use_nan));
      }
      if (status != nullptr)
        status_flags(home,
                     reflexive ? a.signaling()
                               : combine(home, a.signaling(), BOT_OR,
                                         b.signaling()),
                     flushed, *status);
      rel(home, result, WRT_EQ, selected);
    }
  } // namespace
  void fp_compare(Home home, WordVar x, FloatWordRelType relation,
                  WordVar y, BoolVar truth, IntPropLevel ipl) {
    fp_compare(home, x, relation, y, truth, FloatWordPolicy(), ipl);
  }
  void fp_compare(Home home, WordVar x, FloatWordRelType relation,
                  WordVar y, BoolVar truth, IntVar status,
                  IntPropLevel ipl) {
    fp_compare(home, x, relation, y, truth, status, FloatWordPolicy(), ipl);
  }
  void fp_compare(Home home, WordVar x, FloatWordRelType relation,
                  WordVar y, BoolVar truth, const FloatWordPolicy& policy,
                  IntPropLevel ipl) {
    post_compare(home, policy, x, relation, y, truth, nullptr, ipl);
  }
  void fp_compare(Home home, WordVar x, FloatWordRelType relation,
                  WordVar y, BoolVar truth, IntVar status,
                  const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_compare(home, policy, x, relation, y, truth, &status, ipl);
  }
  void fp_minimum(Home home, WordVar x, WordVar y, WordVar result,
                  IntPropLevel ipl) {
    fp_minimum(home, x, y, result, FloatWordPolicy(), ipl);
  }
  void fp_minimum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, IntPropLevel ipl) {
    fp_minimum(home, x, y, result, status, FloatWordPolicy(), ipl);
  }
  void fp_minimum(Home home, WordVar x, WordVar y, WordVar result,
                  const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, nullptr, Extreme::Minimum,
                NanRule::Propagate, ipl,
                "Word::fp_minimum");
  }
  void fp_minimum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, const FloatWordPolicy& policy,
                  IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, &status, Extreme::Minimum,
                NanRule::Propagate, ipl,
                "Word::fp_minimum");
  }
  void fp_maximum(Home home, WordVar x, WordVar y, WordVar result,
                  IntPropLevel ipl) {
    fp_maximum(home, x, y, result, FloatWordPolicy(), ipl);
  }
  void fp_maximum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, IntPropLevel ipl) {
    fp_maximum(home, x, y, result, status, FloatWordPolicy(), ipl);
  }
  void fp_maximum(Home home, WordVar x, WordVar y, WordVar result,
                  const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, nullptr, Extreme::Maximum,
                NanRule::Propagate, ipl,
                "Word::fp_maximum");
  }
  void fp_maximum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, const FloatWordPolicy& policy,
                  IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, &status, Extreme::Maximum,
                NanRule::Propagate, ipl,
                "Word::fp_maximum");
  }
  void fp_minimum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntPropLevel ipl) {
    fp_minimum_number(home, x, y, result, FloatWordPolicy(), ipl);
  }
  void fp_minimum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, IntPropLevel ipl) {
    fp_minimum_number(home, x, y, result, status, FloatWordPolicy(), ipl);
  }
  void fp_minimum_number(Home home, WordVar x, WordVar y, WordVar result,
                         const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, nullptr, Extreme::Minimum,
                NanRule::PreferNumber, ipl,
                "Word::fp_minimum_number");
  }
  void fp_minimum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, const FloatWordPolicy& policy,
                         IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, &status, Extreme::Minimum,
                NanRule::PreferNumber, ipl,
                "Word::fp_minimum_number");
  }
  void fp_maximum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntPropLevel ipl) {
    fp_maximum_number(home, x, y, result, FloatWordPolicy(), ipl);
  }
  void fp_maximum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, IntPropLevel ipl) {
    fp_maximum_number(home, x, y, result, status, FloatWordPolicy(), ipl);
  }
  void fp_maximum_number(Home home, WordVar x, WordVar y, WordVar result,
                         const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, nullptr, Extreme::Maximum,
                NanRule::PreferNumber, ipl,
                "Word::fp_maximum_number");
  }
  void fp_maximum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, const FloatWordPolicy& policy,
                         IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, &status, Extreme::Maximum,
                NanRule::PreferNumber, ipl,
                "Word::fp_maximum_number");
  }
} // namespace Gecode

// STATISTICS: word-post
