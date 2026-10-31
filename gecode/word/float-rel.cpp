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

namespace Gecode {
  namespace {
    using Policy = FloatWordPolicy;
    void validate_relation(const Policy& p, FloatWordVar x, FloatWordVar y) {
      Word::Float::check(p, "floating relation policy");
      if (x.format() != y.format())
        throw Word::FormatMismatch("floating relation");
      if (x.format() == FWF_BINARY16)
        throw Word::OutOfLimits("floating relation format");
      if (float_word_width(x.format()) <= 8) {
        (void)x.encoding();
        (void)y.encoding();
      }
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
    BoolVar classified(Home home, FloatWordVar x, FloatWordClass cls) {
      BoolVar result(home, 0, 1);
      fp_classify(home, x, cls, result);
      return result;
    }
    BoolVar bit_relation(Home home, FloatWordVar x, WordRelType relation,
                         FloatWordVar y) {
      BoolVar result(home, 0, 1);
      rel(home, x.bits(), relation, y.bits(), Reify(result));
      return result;
    }
    FloatWordVar choose(Home home, BoolVar condition, FloatWordVar a,
                        FloatWordVar b) {
      FloatWordVar result(home, a.format());
      ite(home, condition, a.bits(), b.bits(), result.bits());
      return result;
    }
    FloatWordVar signed_zero(Home home, FloatWordVar x) {
      FloatWordVar result(home, x.format());
      const unsigned int width = float_word_width(x.format());
      rel(home, x.bits(), WOT_AND, WordValue(1) << (width - 1),
          result.bits());
      return result;
    }
    FloatWordVar effective_operand(Home home, const Policy& p,
                                   FloatWordVar x) {
      if (p.input_subnormals == Policy::Preserve)
        return x;
      return choose(home, classified(home, x, FWC_SUBNORMAL),
                    signed_zero(home, x), x);
    }

    struct Comparison {
      BoolVar unordered, equal, less, signaling;
    };
    Comparison compare_words(Home home, FloatWordVar a, FloatWordVar b) {
      if (a.bits().varimp() == b.bits().varimp()) {
        BoolVar nan = classified(home, a, FWC_NAN);
        return {nan, inverse(home, nan), BoolVar(home, 0, 0),
                classified(home, a, FWC_SIGNALING_NAN)};
      }

      BoolVar unordered = combine(home, classified(home, a, FWC_NAN), BOT_OR,
                                  classified(home, b, FWC_NAN));
      BoolVar ordered = inverse(home, unordered);
      BoolVar zeros = combine(home, classified(home, a, FWC_ZERO), BOT_AND,
                              classified(home, b, FWC_ZERO));
      BoolVar equal = combine(
          home, ordered, BOT_AND,
          combine(home, zeros, BOT_OR, bit_relation(home, a, WRT_EQ, b)));
      BoolVar negative_a = classified(home, a, FWC_SIGN);
      BoolVar negative_b = classified(home, b, FWC_SIGN);
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
          combine(home, classified(home, a, FWC_SIGNALING_NAN), BOT_OR,
                  classified(home, b, FWC_SIGNALING_NAN));
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

    // Tiny formats have no signaling NaNs. The table carries complete tuples,
    // including status, through the caller's existing encoding domains.
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
    int tiny_order(unsigned int a, unsigned int b, unsigned int sign) {
      const int av = (a & sign) ? -static_cast<int>(a & (sign - 1))
                                : static_cast<int>(a);
      const int bv = (b & sign) ? -static_cast<int>(b & (sign - 1))
                                : static_cast<int>(b);
      return av < bv ? -1 : av > bv ? 1 : 0;
    }
    bool tiny_predicate(unsigned int a, unsigned int b, unsigned int sign,
                        FloatWordRelType relation) {
      const bool unordered = tiny_nan(a, sign) || tiny_nan(b, sign);
      const int order = tiny_order(a, b, sign);
      switch (relation) {
      case FWRT_EQ:
        return !unordered && order == 0;
      case FWRT_NQ:
        return unordered || order != 0;
      case FWRT_LE:
        return !unordered && order < 0;
      case FWRT_LQ:
        return !unordered && order <= 0;
      case FWRT_GR:
        return !unordered && order > 0;
      case FWRT_GQ:
        return !unordered && order >= 0;
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

    // Extensional constraints require distinct arguments. Project aliases
    // only after checking equality of their tuple positions.
    class TinyTable {
      IntVarArgs unique;
      int positions[4];
      TupleSet tuples;
    public:
      explicit TinyTable(const IntVarArgs& vars) {
        assert(vars.size() == 4);
        for (int i = 0; i < 4; ++i) {
          int j = 0;
          while (j < unique.size() && unique[j].varimp() != vars[i].varimp())
            ++j;
          if (j == unique.size())
            unique << vars[i];
          positions[i] = j;
        }
        tuples.init(unique.size());
      }
      void add(const IntArgs& row) {
        IntArgs projected(unique.size());
        for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < i; ++j)
            if (positions[i] == positions[j] && row[i] != row[j])
              return;
          projected[positions[i]] = row[i];
        }
        tuples.add(projected);
      }
      void post(Home home) {
        tuples.finalize();
        extensional(home, unique, tuples);
      }
    };
    void tiny_compare(Home home, const Policy& p, FloatWordVar x,
                      FloatWordRelType relation, FloatWordVar y, BoolVar truth,
                      IntVar status) {
      const unsigned int sign = 1U << (float_word_width(x.format()) - 1);
      IntVar result(home, 0, 1);
      channel(home, truth, result);
      TinyTable table(
          IntVarArgs{x.encoding(), y.encoding(), result, status});
      for (IntVarValues a(x.encoding()); a(); ++a)
        for (IntVarValues b(y.encoding()); b(); ++b) {
          const unsigned int av = effective_code(a.val(), sign, p),
                             bv = effective_code(b.val(), sign, p);
          const bool invalid = p.comparison == Policy::Signaling &&
                               (tiny_nan(av, sign) || tiny_nan(bv, sign));
          table.add(IntArgs{a.val(), b.val(),
                            tiny_predicate(av, bv, sign, relation), invalid});
        }
      table.post(home);
    }
    void tiny_minmax(Home home, const Policy& p, FloatWordVar x,
                     FloatWordVar y, FloatWordVar result, IntVar status,
                     bool maximum, bool number) {
      const unsigned int sign = 1U << (float_word_width(x.format()) - 1);
      TinyTable table(IntVarArgs{x.encoding(), y.encoding(),
                                 result.encoding(), status});
      for (IntVarValues a(x.encoding()); a(); ++a)
        for (IntVarValues b(y.encoding()); b(); ++b) {
          const Selection selected = tiny_selection(
              effective_code(a.val(), sign, p),
              effective_code(b.val(), sign, p), sign, p, maximum, number);
          table.add(IntArgs{a.val(), b.val(), static_cast<int>(selected.code),
                            selected.flags});
        }
      table.post(home);
    }

    void post_compare(Home home, const Policy& p, FloatWordVar x,
                      FloatWordRelType relation, FloatWordVar y, BoolVar truth,
                      IntVar status) {
      validate_relation(p, x, y);
      if (relation < FWRT_EQ || relation > FWRT_ORDERED)
        throw Word::OutOfLimits("fp_compare predicate");
      GECODE_POST;
      if (float_word_width(x.format()) <= 8) {
        tiny_compare(home, p, x, relation, y, truth, status);
        return;
      }
      Comparison c = compare_words(home, effective_operand(home, p, x),
                                   effective_operand(home, p, y));
      rel(home, truth, IRT_EQ, predicate(home, c, relation));
      status_flags(home,
                   p.comparison == Policy::Signaling ? c.unordered
                                                     : c.signaling,
                   BoolVar(home, 0, 0), status);
    }

    FloatWordVar select_nan(Home home, const Policy& p, FloatWordVar a,
                            FloatWordVar b, BoolVar an) {
      const WordValue quiet = WordValue(1)
                              << (a.format() == FWF_FP32 ? 22 : 6);
      if (p.nan == Policy::Canonical)
        return FloatWordVar::from_bits(
            home, a.format(), a.format() == FWF_FP32 ? 0x7fc00000 : 0x7fc0);
      BoolVar take_a = an;
      if (p.nan == Policy::SignalFirst)
        take_a = combine(
            home, classified(home, a, FWC_SIGNALING_NAN), BOT_OR,
            combine(home, an, BOT_AND,
                    inverse(home, classified(home, b, FWC_SIGNALING_NAN))));
      FloatWordVar selected = choose(home, take_a, a, b),
                   result(home, a.format());
      rel(home, selected.bits(), WOT_OR, quiet, result.bits());
      return result;
    }
    FloatWordVar select_number(Home home, FloatWordVar a, FloatWordVar b,
                               const Comparison& c, bool maximum) {
      FloatWordVar ordered =
          choose(home, maximum ? inverse(home, c.less) : c.less, a, b);
      FloatWordVar zero_tie(home, a.format());
      rel(home, a.bits(), maximum ? WOT_AND : WOT_OR, b.bits(),
          zero_tie.bits());
      BoolVar zeros = combine(home, classified(home, a, FWC_ZERO), BOT_AND,
                              classified(home, b, FWC_ZERO));
      return choose(home, zeros, zero_tie, ordered);
    }
    void post_minmax(Home home, const Policy& p, FloatWordVar x,
                     FloatWordVar y, FloatWordVar result, IntVar status,
                     bool maximum, bool number) {
      validate_relation(p, x, y);
      if (result.format() != x.format())
        throw Word::FormatMismatch("floating min/max result");
      if (float_word_width(x.format()) <= 8)
        (void)result.encoding();
      GECODE_POST;
      if (float_word_width(x.format()) <= 8) {
        tiny_minmax(home, p, x, y, result, status, maximum, number);
        return;
      }
      FloatWordVar a = effective_operand(home, p, x),
                   b = effective_operand(home, p, y);
      Comparison c = compare_words(home, a, b);
      BoolVar an = classified(home, a, FWC_NAN),
              bn = classified(home, b, FWC_NAN);
      FloatWordVar numeric = select_number(home, a, b, c, maximum);
      FloatWordVar nan = select_nan(home, p, a, b, an);
      BoolVar use_nan = c.unordered;
      if (number) {
        use_nan = combine(home, an, BOT_AND, bn);
        numeric = choose(home, an, b, choose(home, bn, a, numeric));
      }
      FloatWordVar selected = choose(home, use_nan, nan, numeric);
      BoolVar flushed(home, 0, 0);
      if (p.output_subnormals == Policy::SignedZero) {
        flushed = classified(home, selected, FWC_SUBNORMAL);
        selected =
            choose(home, flushed, signed_zero(home, selected), selected);
      }
      status_flags(home, c.signaling, flushed, status);
      if (p.zeros == Policy::Positive)
        selected = choose(home, classified(home, selected, FWC_ZERO),
                          FloatWordVar::from_bits(home, x.format(), 0),
                          selected);
      rel(home, result.bits(), WRT_EQ, selected.bits());
    }
  } // namespace
  void fp_compare(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordRelType relation, FloatWordVar y, BoolVar truth) {
    post_compare(home, policy, x, relation, y, truth, IntVar(home, 0, 31));
  }
  void fp_compare(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordRelType relation, FloatWordVar y, BoolVar truth,
                  IntVar status) {
    post_compare(home, policy, x, relation, y, truth, status);
  }
  void fp_minimum(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordVar y, FloatWordVar result) {
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), false, false);
  }
  void fp_minimum(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordVar y, FloatWordVar result, IntVar status) {
    post_minmax(home, policy, x, y, result, status, false, false);
  }
  void fp_maximum(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordVar y, FloatWordVar result) {
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), true, false);
  }
  void fp_maximum(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                  FloatWordVar y, FloatWordVar result, IntVar status) {
    post_minmax(home, policy, x, y, result, status, true, false);
  }
  void fp_minimum_number(Home home, const FloatWordPolicy& policy,
                         FloatWordVar x, FloatWordVar y,
                         FloatWordVar result) {
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), false, true);
  }
  void fp_minimum_number(Home home, const FloatWordPolicy& policy,
                         FloatWordVar x, FloatWordVar y, FloatWordVar result,
                         IntVar status) {
    post_minmax(home, policy, x, y, result, status, false, true);
  }
  void fp_maximum_number(Home home, const FloatWordPolicy& policy,
                         FloatWordVar x, FloatWordVar y,
                         FloatWordVar result) {
    post_minmax(home, policy, x, y, result, IntVar(home, 0, 31), true, true);
  }
  void fp_maximum_number(Home home, const FloatWordPolicy& policy,
                         FloatWordVar x, FloatWordVar y, FloatWordVar result,
                         IntVar status) {
    post_minmax(home, policy, x, y, result, status, true, true);
  }
} // namespace Gecode

// STATISTICS: word-post
