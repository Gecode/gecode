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

#include <algorithm>

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    // Return the common format of x and y after checking it and the policy.
    WordDomainType validate_relation(const Policy& p, WordVar x, WordVar y) {
      const WordDomainType format =
          Word::Float::float_format(x, "floating relation format");
      const WordDomainType y_format =
          Word::Float::float_format(y, "floating relation format");
      Word::Float::check(p, "floating relation policy");
      if (format != y_format)
        throw Word::FormatMismatch("floating relation");
      if (format == WDT_BINARY16)
        throw Word::OutOfLimits("floating relation format");
      return format;
    }

    BoolVar combine(Home home, BoolVar a, BoolOpType op, BoolVar b) {
      BoolVar result(home, 0, 1);
      rel(home, a, op, b, result);
      return result;
    }
    BoolVar inverse(Home home, BoolVar a) {
      BoolVar result(home, 0, 1);
      rel(home, a, IRT_NQ, result);
      return result;
    }
    // The wide decomposition below builds unsigned intermediate words and
    // passes the format of the operands explicitly.
    BoolVar classified(Home home, WordVar x, WordDomainType format,
                       FloatWordClass cls) {
      BoolVar result(home, 0, 1);
      Word::Float::classify(home, x, format, cls, result);
      return result;
    }
    BoolVar bit_relation(Home home, WordVar x, WordRelType relation, WordVar y) {
      BoolVar result(home, 0, 1);
      rel(home, x, relation, y, Reify(result));
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
    WordVar effective_operand(Home home, const Policy& p, WordVar x,
                              WordDomainType format) {
      if (p.input_subnormals == Policy::Preserve)
        return x;
      return choose(home, classified(home, x, format, FWC_SUBNORMAL),
                    signed_zero(home, x), x);
    }

    struct Comparison {
      BoolVar unordered, equal, less, signaling;
    };
    Comparison compare_words(Home home, WordVar a, WordVar b,
                             WordDomainType format) {
      if (a.varimp() == b.varimp()) {
        BoolVar nan = classified(home, a, format, FWC_NAN);
        return {nan, inverse(home, nan), BoolVar(home, 0, 0),
                classified(home, a, format, FWC_SIGNALING_NAN)};
      }

      BoolVar unordered =
          combine(home, classified(home, a, format, FWC_NAN), BOT_OR,
                  classified(home, b, format, FWC_NAN));
      BoolVar ordered = inverse(home, unordered);
      BoolVar zeros =
          combine(home, classified(home, a, format, FWC_ZERO), BOT_AND,
                  classified(home, b, format, FWC_ZERO));
      BoolVar equal = combine(
          home, ordered, BOT_AND,
          combine(home, zeros, BOT_OR, bit_relation(home, a, WRT_EQ, b)));
      BoolVar negative_a = classified(home, a, format, FWC_SIGN);
      BoolVar negative_b = classified(home, b, format, FWC_SIGN);
      BoolVar same_sign = combine(home, negative_a, BOT_EQV, negative_b);
      BoolVar negative_less = combine(home, negative_a, BOT_AND,
                                      bit_relation(home, a, WRT_UGR, b));
      BoolVar positive_less =
          combine(home, inverse(home, negative_a), BOT_AND,
                  bit_relation(home, a, WRT_ULE, b));
      BoolVar same_sign_less =
          combine(home, same_sign, BOT_AND,
                  combine(home, negative_less, BOT_OR, positive_less));
      BoolVar opposite_sign_less =
          combine(home, negative_a, BOT_AND, inverse(home, negative_b));
      BoolVar less = combine(
          home, ordered, BOT_AND,
          combine(home, inverse(home, zeros), BOT_AND,
                  combine(home, same_sign_less, BOT_OR, opposite_sign_less)));
      BoolVar signaling =
          combine(home, classified(home, a, format, FWC_SIGNALING_NAN), BOT_OR,
                  classified(home, b, format, FWC_SIGNALING_NAN));
      return {unordered, equal, less, signaling};
    }
    BoolVar predicate(Home home, const Comparison& c,
                      FloatWordRelType relation) {
      switch (relation) {
      case FWRT_EQ:
        return c.equal;
      case FWRT_NQ:
        return inverse(home, c.equal);
      case FWRT_LE:
        return c.less;
      case FWRT_LQ:
        return combine(home, c.less, BOT_OR, c.equal);
      case FWRT_GR:
        return inverse(home, combine(home, c.unordered, BOT_OR,
                                     combine(home, c.less, BOT_OR, c.equal)));
      case FWRT_GQ:
        return inverse(home, combine(home, c.unordered, BOT_OR, c.less));
      case FWRT_UNORDERED:
        return c.unordered;
      case FWRT_ORDERED:
        return inverse(home, c.unordered);
      default:
        throw Word::OutOfLimits("fp_compare predicate");
      }
    }
    void status_flags(Home home, BoolVar invalid, BoolVar flushed,
                      IntVar status) {
      linear(home, IntArgs{1, 24}, BoolVarArgs{invalid, flushed}, IRT_EQ,
             status);
    }

    // Tiny formats have no signaling NaNs. Their operations compare and
    // select the effective encodings, after input subnormals are treated.
    unsigned int effective_code(unsigned int code, unsigned int sign,
                                const Policy& p) {
      const unsigned int magnitude = code & (sign - 1);
      const unsigned int normal = sign == 8 ? 2 : 8;
      return p.input_subnormals == Policy::SignedZero && magnitude < normal
                 ? code & sign
                 : code;
    }
    bool tiny_nan(unsigned int code, unsigned int sign) {
      return sign == 128 && (code & 127) == 127;
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
          magnitude < (sign == 8 ? 2U : 8U)) {
        code &= sign;
        flags = 24;
      }
      if (p.zeros == Policy::Positive && (code & (sign - 1)) == 0)
        code = 0;
      return {code, flags};
    }
    Selection tiny_selection(unsigned int a, unsigned int b,
                             unsigned int sign, const Policy& p, bool maximum,
                             bool number) {
      const bool an = tiny_nan(a, sign), bn = tiny_nan(b, sign);
      unsigned int code;
      if ((an || bn) && (!number || (an && bn)))
        code = p.nan == Policy::Canonical ? 127U : an ? a : b;
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

    /// Comparison outcomes that the operands, truth and status allow
    struct AllowedOutcomes {
      bool less, equal, greater, unordered;
    };
    using Word::Float::failed_change;

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
        return policy.comparison == Policy::Signaling &&
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
      bool allows(Outcome outcome, Int::BoolView truth,
                  Int::IntView status) const {
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
      ExecStatus narrow_bounds(Space& home, Word::WordView x, Word::WordView y,
                               Int::BoolView truth, Int::IntView status) const {
        const Word::Float::TinyValues xs(x), ys(y);
        const TinyBounds bx = tiny_bounds(xs, sign, policy),
                         by = tiny_bounds(ys, sign, policy);
        const bool numbers = bx.has_number && by.has_number;
        const AllowedOutcomes allowed = {
            numbers && bx.least < by.greatest &&
                allows(Outcome::Less, truth, status),
            numbers && bx.least <= by.greatest && by.least <= bx.greatest &&
                allows(Outcome::Equal, truth, status),
            numbers && bx.greatest > by.least &&
                allows(Outcome::Greater, truth, status),
            (bx.has_nan || by.has_nan) &&
                allows(Outcome::Unordered, truth, status)};
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
    };

    /// One operand of a tiny minimum or maximum, read by rank
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
     * reasoning reads each operand by rank: the key for the minimum and its
     * negation for the maximum, so that both select the least rank. It
     * keeps each operand value that the other operand's least and greatest
     * rank and NaNs can pair with an admitted result, and each result that
     * such a pair can select. Ties of zeros keep both signs whenever a zero
     * result is admitted.
     */
    struct TinySelect {
      Policy policy;
      unsigned int sign;
      bool maximum, number;

      Word::Float::TinyOutcome operator()(WordValue x, WordValue y) const {
        const Selection s = tiny_selection(
            effective_code(static_cast<unsigned int>(x), sign, policy),
            effective_code(static_cast<unsigned int>(y), sign, policy), sign,
            policy, maximum, number);
        return {s.code, s.flags};
      }
      int rank(unsigned int code) const {
        return maximum ? -tiny_key(code, sign) : tiny_key(code, sign);
      }
      unsigned int nan_result(unsigned int code) const {
        return policy.nan == Policy::Canonical ? 127U : code;
      }
      bool admits(unsigned int code, Word::WordView result,
                  Int::IntView status) const {
        const Selection output = tiny_output(code, sign, policy);
        return result.in(output.code) && status.in(output.flags);
      }
      unsigned int effective(WordValue value) const {
        return effective_code(static_cast<unsigned int>(value), sign, policy);
      }
      SelectOperand summarise(const Word::Float::TinyValues& values,
                              Word::WordView result,
                              Int::IntView status) const {
        SelectOperand s;
        for (unsigned int i = 0; i < values.size(); ++i) {
          const unsigned int code = effective(values[i]);
          if (tiny_nan(code, sign)) {
            s.has_nan = true;
            s.has_admitted_nan = s.has_admitted_nan ||
                                 admits(nan_result(code), result, status);
            continue;
          }
          const int r = rank(code);
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
      // A number meets a greater rank (selected itself, ties select an
      // equal encoding), a lesser rank (selects the other number), or a NaN.
      bool keeps_number(unsigned int code, const SelectOperand& other,
                        bool zero_admitted, Word::WordView result,
                        Int::IntView status) const {
        const int r = rank(code);
        const bool admitted = admits(code, result, status);
        if (other.has_number) {
          const bool zero_tie = r == 0 && zero_admitted && other.least <= 0;
          if (r <= other.greatest && (admitted || zero_tie))
            return true;
          if (other.has_admitted_number && other.least_admitted < r)
            return true;
        }
        return other.has_nan && (number ? admitted : other.has_admitted_nan);
      }
      // The first NaN of a pair supplies a NaN result.
      bool keeps_nan(unsigned int code, bool is_first,
                     const SelectOperand& other, Word::WordView result,
                     Int::IntView status) const {
        const bool own = admits(nan_result(code), result, status);
        if (number)
          return other.has_admitted_number ||
                 (other.has_nan && (is_first ? own : other.has_admitted_nan));
        return is_first ? own
                        : (other.has_number && own) || other.has_admitted_nan;
      }
      ModEvent narrow_operand(Space& home, Word::WordView x,
                              const Word::Float::TinyValues& values,
                              bool is_first, const SelectOperand& other,
                              bool zero_admitted, Word::WordView result,
                              Int::IntView status) const {
        Word::WordValueSet kept = {{0, 0, 0, 0}};
        for (unsigned int i = 0; i < values.size(); ++i) {
          const unsigned int code = effective(values[i]);
          const bool keep =
              tiny_nan(code, sign)
                  ? keeps_nan(code, is_first, other, result, status)
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
      void add_candidate(unsigned int code, Candidates& candidates,
                         Word::WordView result, Int::IntView status) const {
        const Selection output = tiny_output(code, sign, policy);
        if (!result.in(output.code) || !status.in(output.flags))
          return;
        candidates.codes = Word::with_value(candidates.codes, output.code);
        candidates.flags |= 1U << output.flags;
      }
      // Candidates of the operand values, as the first or second operand.
      void add_candidates(const Word::Float::TinyValues& values, bool is_first,
                          const SelectOperand& other, Candidates& candidates,
                          Word::WordView result, Int::IntView status) const {
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
          const int r = rank(code);
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
      ExecStatus narrow_bounds(Space& home, Word::WordView x, Word::WordView y,
                               Word::WordView result,
                               Int::IntView status) const {
        const Word::Float::TinyValues xs(x), ys(y);
        const SelectOperand sx = summarise(xs, result, status),
                            sy = summarise(ys, result, status);
        const bool zero_admitted =
            admits(0, result, status) || admits(sign, result, status);
        Candidates candidates;
        add_candidates(xs, true, sy, candidates, result, status);
        add_candidates(ys, false, sx, candidates, result, status);
        if (candidates.has_zero) {
          add_candidate(0, candidates, result, status);
          add_candidate(sign, candidates, result, status);
        }
        if (candidates.flags == 0)
          return ES_FAILED;
        bool changed = false;
        if (failed_change(narrow_operand(home, x, xs, true, sy, zero_admitted,
                                         result, status),
                          changed) ||
            failed_change(narrow_operand(home, y, ys, false, sx, zero_admitted,
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

    void post_compare(Home home, const Policy& p, WordVar x,
                      FloatWordRelType relation, WordVar y, BoolVar truth,
                      IntVar status, IntPropLevel ipl) {
      const WordDomainType format = validate_relation(p, x, y);
      if (relation < FWRT_EQ || relation > FWRT_ORDERED)
        throw Word::OutOfLimits("fp_compare predicate");
      GECODE_POST;
      const unsigned int width = x.width();
      if (width <= 8) {
        GECODE_ES_FAIL(
            (Word::Float::TinyBinary<TinyCompare, Int::BoolView,
                                     Int::IntView>::post(
                home, x, y, truth, status,
                TinyCompare{p, relation, 1U << (width - 1)},
                vbd(ipl) == IPL_DOM)));
        return;
      }
      Comparison c = compare_words(home, effective_operand(home, p, x, format),
                                   effective_operand(home, p, y, format),
                                   format);
      rel(home, truth, IRT_EQ, predicate(home, c, relation));
      status_flags(home,
                   p.comparison == Policy::Signaling ? c.unordered
                                                     : c.signaling,
                   BoolVar(home, 0, 0), status);
    }

    WordVar select_nan(Home home, const Policy& p, WordVar a, WordVar b,
                       WordDomainType format, BoolVar an) {
      const WordValue quiet = WordValue(1) << (format == WDT_FP32 ? 22 : 6);
      if (p.nan == Policy::Canonical) {
        const WordValue canonical = format == WDT_FP32 ? 0x7fc00000 : 0x7fc0;
        return WordVar(home, a.width(), {canonical});
      }
      BoolVar take_a = an;
      if (p.nan == Policy::SignalFirst)
        take_a = combine(
            home, classified(home, a, format, FWC_SIGNALING_NAN), BOT_OR,
            combine(home, an, BOT_AND,
                    inverse(home,
                            classified(home, b, format, FWC_SIGNALING_NAN))));
      WordVar selected = choose(home, take_a, a, b),
              result(home, a.width(), WDT_UNSIGNED);
      rel(home, selected, WOT_OR, quiet, result);
      return result;
    }
    WordVar select_number(Home home, WordVar a, WordVar b,
                          WordDomainType format, const Comparison& c,
                          bool maximum) {
      WordVar ordered =
          choose(home, maximum ? inverse(home, c.less) : c.less, a, b);
      WordVar zero_tie(home, a.width(), WDT_UNSIGNED);
      rel(home, a, maximum ? WOT_AND : WOT_OR, b, zero_tie);
      BoolVar zeros =
          combine(home, classified(home, a, format, FWC_ZERO), BOT_AND,
                  classified(home, b, format, FWC_ZERO));
      return choose(home, zeros, zero_tie, ordered);
    }
    void post_minmax(Home home, const Policy& p, WordVar x, WordVar y,
                     WordVar result, IntVar status, bool maximum, bool number,
                     IntPropLevel ipl) {
      const WordDomainType result_format =
          Word::Float::float_format(result, "floating min/max result");
      const WordDomainType format = validate_relation(p, x, y);
      if (result_format != format)
        throw Word::FormatMismatch("floating min/max result");
      GECODE_POST;
      const unsigned int width = x.width();
      if (width <= 8) {
        GECODE_ES_FAIL(
            (Word::Float::TinyBinary<TinySelect, Word::WordView,
                                     Int::IntView>::post(
                home, x, y, result, status,
                TinySelect{p, 1U << (width - 1), maximum, number},
                vbd(ipl) == IPL_DOM)));
        return;
      }
      WordVar a = effective_operand(home, p, x, format),
              b = effective_operand(home, p, y, format);
      Comparison c = compare_words(home, a, b, format);
      BoolVar an = classified(home, a, format, FWC_NAN),
              bn = classified(home, b, format, FWC_NAN);
      WordVar numeric = select_number(home, a, b, format, c, maximum);
      WordVar nan = select_nan(home, p, a, b, format, an);
      BoolVar use_nan = c.unordered;
      if (number) {
        use_nan = combine(home, an, BOT_AND, bn);
        numeric = choose(home, an, b, choose(home, bn, a, numeric));
      }
      WordVar selected = choose(home, use_nan, nan, numeric);
      BoolVar flushed(home, 0, 0);
      if (p.output_subnormals == Policy::SignedZero) {
        flushed = classified(home, selected, format, FWC_SUBNORMAL);
        selected =
            choose(home, flushed, signed_zero(home, selected), selected);
      }
      status_flags(home, c.signaling, flushed, status);
      if (p.zeros == Policy::Positive)
        selected = choose(home, classified(home, selected, format, FWC_ZERO),
                          WordVar(home, width, {0}), selected);
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
    post_compare(home, policy, x, relation, y, truth, IntVar(home, 0, 31),
                 ipl);
  }
  void fp_compare(Home home, WordVar x, FloatWordRelType relation,
                  WordVar y, BoolVar truth, IntVar status,
                  const FloatWordPolicy& policy, IntPropLevel ipl) {
    post_compare(home, policy, x, relation, y, truth, status, ipl);
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
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), false, false,
                ipl);
  }
  void fp_minimum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, const FloatWordPolicy& policy,
                  IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, status, false, false, ipl);
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
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), true, false,
                ipl);
  }
  void fp_maximum(Home home, WordVar x, WordVar y, WordVar result,
                  IntVar status, const FloatWordPolicy& policy,
                  IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, status, true, false, ipl);
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
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), false, true,
                ipl);
  }
  void fp_minimum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, const FloatWordPolicy& policy,
                         IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, status, false, true, ipl);
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
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), true, true,
                ipl);
  }
  void fp_maximum_number(Home home, WordVar x, WordVar y, WordVar result,
                         IntVar status, const FloatWordPolicy& policy,
                         IntPropLevel ipl) {
    post_minmax(home, policy, x, y, result, status, true, true, ipl);
  }
} // namespace Gecode

// STATISTICS: word-post
