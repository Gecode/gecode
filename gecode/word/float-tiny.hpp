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

#ifndef GECODE_WORD_FLOAT_TINY_HPP
#define GECODE_WORD_FLOAT_TINY_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <gecode/float-word.hh>

namespace Gecode {
  namespace Word {
    namespace Float {
      struct TinyResult {
        int code, status;
      };

      /** \brief Result value and status flags of one tiny tuple
       *
       * Unlike TinyResult, the value holds every distance between FP32
       * reference values. An operation without flags reports a tuple outside
       * its relation, such as an unrepresentable endpoint, with status 1, and
       * its posting passes Int::ConstIntView(0) as status, which rejects that
       * tuple.
       */
      struct TinyOutcome {
        WordValue value;
        int status;
      };

      /// The values of a word of width at most 8, in increasing order
      class TinyValues {
        WordValue items[256];
        unsigned int count = 0;

      public:
        explicit TinyValues(const WordView& x) {
          const WordValueSet set = x.values();
          for (unsigned int block = 0; block < 4; ++block)
            for (WordValue bits = set.blocks[block]; bits != 0;
                 bits &= bits - 1)
              items[count++] = (WordValue(block) << block_value_bits) +
                               Support::count_trailing_zeros_64(bits);
        }
        unsigned int size(void) const { return count; }
        WordValue operator[](unsigned int i) const { return items[i]; }
      };

      /** \brief Supported values of one view, published as tightly as its
       * domain can express them
       *
       * A word of width at most 8 or a Boolean collects its exact values. A
       * wider word collects their bit hull and their least and greatest rank
       * in the order of its bounds, so a signed word gets its tightest signed
       * bounds even when the supports lie on both sides of the sign bit.
       */
      class WordSupports {
        unsigned int width;
        WordDomainType domain_type = WDT_CUBE;
        bool is_empty = true;
        WordValueSet values = {{0, 0, 0, 0}};
        WordValue ones = ~WordValue(0), any = 0;
        WordValue least_rank = ~WordValue(0), greatest_rank = 0;

      public:
        explicit WordSupports(const WordView& x)
            : width(x.width()), domain_type(x.domain_type()) {}
        explicit WordSupports(const Int::BoolView&) : width(1) {}
        void add(WordValue value) {
          is_empty = false;
          if (width <= 8) {
            values = with_value(values, value);
            return;
          }
          ones &= value;
          any |= value;
          const WordValue r = rank(domain_type, width, value);
          least_rank = std::min(least_rank, r);
          greatest_rank = std::max(greatest_rank, r);
        }
        bool empty(void) const { return is_empty; }
        /// Narrow the nonempty supports into \a x
        ModEvent publish(Space& home, WordView x) const {
          assert(!is_empty);
          if (width <= 8)
            return x.narrow_values(home, values);
          const ModEvent cube = x.narrow(home, ones, any);
          if (me_failed(cube) || !x.bounded())
            return cube;
          const ModEvent bounds =
              x.narrow_rank_range(home, least_rank, greatest_rank);
          return bounds == ME_WORD_NONE ? cube : bounds;
        }
        /// Narrow the nonempty supports into the Boolean \a x
        ModEvent publish(Space& home, Int::BoolView x) const {
          assert(!is_empty);
          if (!value_set_contains(values, 0))
            return x.one(home);
          if (!value_set_contains(values, 1))
            return x.zero(home);
          return Int::ME_BOOL_NONE;
        }
      };

      /// Record in \a changed whether \a me modified a view; true on failure
      inline bool failed_change(ModEvent me, bool& changed) {
        changed = changed || me_modified(me);
        return me_failed(me);
      }

      /// Narrow \a status exactly to the flag values set in \a flags
      template<class StatusView>
      ModEvent narrow_flags(Space& home, StatusView status,
                            unsigned int flags) {
        int values[32];
        int count = 0;
        for (int flag = 0; flag < 32; ++flag)
          if ((flags & (1U << flag)) != 0)
            values[count++] = flag;
        Iter::Values::Array allowed(values, count);
        return status.inter_v(home, allowed, false);
      }

      /** \brief Status of an operation posted without a status variable
       *
       * Every flag value is admitted and no narrowing has an effect, so the
       * propagators that take a status view need no IntVar for it.
       */
      class OmittedStatus {
      public:
        bool in(int) const { return true; }
        void subscribe(Space&, Propagator&, PropCond) {}
        void cancel(Space&, Propagator&, PropCond) {}
        void reschedule(Space&, Propagator&, PropCond) {}
        void update(Space&, OmittedStatus&) {}
        template<class Iterator>
        ModEvent inter_v(Space&, Iterator&, bool) {
          return Int::ME_INT_NONE;
        }
      };

      /** \brief Exact projection of a unary operation on tiny encodings
       *
       * Relates a source encoding x0 of width at most 8, a result word x1 of
       * width below 32 and a status x2. Operation maps one source encoding to
       * its result code and status flags through
       * `TinyResult operator()(WordValue code) const`; it holds only
       * space-independent data. An operation without flags passes
       * Int::ConstIntView(0) as status.
       *
       * Each propagation is one pass over the at most 256 remaining source
       * encodings. It keeps exactly the source, result and status values of
       * the supported tuples: exactly on value-set words, and as their
       * tightest cube and bounds on words without a set (see WordSupports
       * for results wider than 8 bits). The operation
       * therefore takes no propagation level. An alias of source and result
       * keeps the encodings that the operation maps to themselves.
       */
      template<class Operation, class StatusView>
      class TinyUnary
          : public MixTernaryPropagator<WordView, PC_WORD_DOM, WordView,
                                        PC_WORD_DOM, StatusView,
                                        Int::PC_INT_DOM> {
        using Base = MixTernaryPropagator<WordView, PC_WORD_DOM, WordView,
                                          PC_WORD_DOM, StatusView,
                                          Int::PC_INT_DOM>;
        using Base::x0;
        using Base::x1;
        using Base::x2;
        const Operation operation;

        TinyUnary(Home home, WordView source, WordView result,
                  StatusView status, const Operation& op)
            : Base(home, source, result, status), operation(op) {}
        TinyUnary(Space& home, TinyUnary& other)
            : Base(home, other), operation(other.operation) {}

        /// Keep the values of the tuples that \a operation supports.
        static ExecStatus narrow_to_supports(Space& home, WordView source,
                                             WordView result,
                                             StatusView status,
                                             const Operation& operation) {
          const WordValueSet candidates = source.values();
          const bool aliased = source == result;
          WordValueSet sources = {{0, 0, 0, 0}};
          WordSupports results(result);
          unsigned int statuses = 0;
          for (WordValue code = 0; code <= source.mask(); ++code) {
            if (!value_set_contains(candidates, code))
              continue;
            const TinyResult r = operation(code);
            const WordValue output = static_cast<WordValue>(r.code);
            if (!result.in(output) || !status.in(r.status) ||
                (aliased && output != code))
              continue;
            sources = with_value(sources, code);
            results.add(output);
            statuses |= 1U << r.status;
          }
          if (is_empty_value_set(sources))
            return ES_FAILED;
          GECODE_ME_CHECK(source.narrow_values(home, sources));
          GECODE_ME_CHECK(results.publish(home, result));
          for (int flag = 0; flag <= 31; ++flag)
            if ((statuses & (1U << flag)) == 0)
              GECODE_ME_CHECK(status.nq(home, flag));
          return ES_OK;
        }

      public:
        /// Post unless an assigned source decides result and status.
        static ExecStatus post(Home home, WordView source, WordView result,
                               StatusView status, const Operation& operation) {
          assert(source.width() <= 8 && result.width() < 32);
          if (source.assigned())
            return narrow_to_supports(home, source, result, status, operation);
          (void)new (home) TinyUnary(home, source, result, status, operation);
          return ES_OK;
        }
        Actor* copy(Space& home) override {
          return new (home) TinyUnary(home, *this);
        }
        PropCost cost(const Space&, const ModEventDelta&) const override {
          return PropCost::binary(PropCost::HI);
        }
        size_t dispose(Space& home) override {
          (void)Base::dispose(home);
          return sizeof(*this);
        }
        // One pass is a fixpoint. For the supported sources S, narrow_values
        // keeps exactly S on a value-set word, and otherwise the values of
        // the smallest cube and bounds containing S, which closure never
        // shrinks below S. The same holds for the results R, also for a wide
        // result that keeps the hull and unsigned interval of R, and the status
        // keeps exactly the supported flags. A second pass finds every
        // earlier support again and no source outside the published domain,
        // so its sources, results and flags contain S, R and the flags and
        // lie within the published domains. Publishing them changes nothing,
        // with or without sets on either side. Aliased source and result are
        // one view, so their supports agree.
        ExecStatus propagate(Space& home, const ModEventDelta&) override {
          GECODE_ES_CHECK(narrow_to_supports(home, x0, x1, x2, operation));
          return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
        }
      };

      /// Propagation condition of a result word
      inline PropCond result_condition(const WordView&) { return PC_WORD_DOM; }
      /// Propagation condition of a result Boolean
      inline PropCond result_condition(const Int::BoolView&) {
        return Int::PC_BOOL_VAL;
      }
      /// Test whether the result word \a x admits \a value
      inline bool admits(const WordView& x, WordValue value) {
        return x.in(value);
      }
      /// Test whether the result Boolean \a x admits \a value
      inline bool admits(const Int::BoolView& x, WordValue value) {
        return value <= 1 && x.in(static_cast<int>(value));
      }
      /// Test whether the result word \a x is the operand \a y
      inline bool is_operand(const WordView& x, const WordView& y) {
        return x == y;
      }
      /// A result Boolean is never an operand word
      inline bool is_operand(const Int::BoolView&, const WordView&) {
        return false;
      }

      /** \brief Projection of a binary operation on tiny encodings
       *
       * Relates operands x0 and x1, words of width at most 8 and possibly of
       * different widths, to a result word of width below 64 or a Boolean,
       * and to a status. An operation without flags passes
       * Int::ConstIntView(0) as status, and an operation posted without a
       * status variable passes OmittedStatus. Operation holds only
       * space-independent data and provides
       *  - `TinyOutcome operator()(WordValue x0, WordValue x1) const`, the
       *    exact outcome of one pair of operand encodings, and
       *  - `ExecStatus narrow_bounds(Space&, WordView, WordView, ResultView,
       *    StatusView) const`, the default reasoning, which may fail and
       *    returns ES_NOFIX after changing a view and ES_FIX otherwise.
       *
       * The exact pass enumerates all remaining operand pairs and keeps
       * exactly the values of the supported tuples: exactly on value-set
       * words and Booleans, otherwise as their tightest cube and bounds (see
       * WordSupports). A domain-consistent propagator runs it every time.
       * Otherwise the default reasoning runs until an operand is assigned or
       * both operands are one view, and from then on the exact pass, which
       * then visits at most 256 pairs. The exact pass checks a result aliased
       * with an operand per tuple; the default reasoning treats it as an
       * independent view, which is sound.
       */
      template<class Operation, class ResultView, class StatusView>
      class TinyBinary : public Propagator {
        WordView x0, x1;
        ResultView result;
        StatusView status;
        const Operation operation;
        const bool domain_consistent;

        TinyBinary(Home home, WordView first, WordView second, ResultView r,
                   StatusView s, const Operation& op, bool consistent)
            : Propagator(home), x0(first), x1(second), result(r), status(s),
              operation(op), domain_consistent(consistent) {
          x0.subscribe(home, *this, PC_WORD_DOM);
          x1.subscribe(home, *this, PC_WORD_DOM);
          result.subscribe(home, *this, result_condition(result));
          status.subscribe(home, *this, Int::PC_INT_DOM);
        }
        TinyBinary(Space& home, TinyBinary& other)
            : Propagator(home, other), operation(other.operation),
              domain_consistent(other.domain_consistent) {
          x0.update(home, other.x0);
          x1.update(home, other.x1);
          result.update(home, other.result);
          status.update(home, other.status);
        }

        /// Test whether propagation enumerates all operand pairs
        bool enumerates_pairs(void) const {
          return domain_consistent || x0.assigned() || x1.assigned() ||
                 x0 == x1;
        }

        /// Keep the values of the tuples that \a operation supports.
        static ExecStatus narrow_to_supports(Space& home, WordView x0,
                                             WordView x1, ResultView result,
                                             StatusView status,
                                             const Operation& operation) {
          const TinyValues firsts(x0), seconds(x1);
          const bool same_operands = x0 == x1;
          const bool result_is_first = is_operand(result, x0);
          const bool result_is_second = is_operand(result, x1);
          WordSupports first_supports(x0), second_supports(x1),
              result_supports(result);
          unsigned int flags = 0;
          for (unsigned int i = 0; i < firsts.size(); ++i) {
            const unsigned int begin = same_operands ? i : 0;
            const unsigned int end = same_operands ? i + 1 : seconds.size();
            for (unsigned int j = begin; j < end; ++j) {
              const WordValue v = firsts[i], w = seconds[j];
              const TinyOutcome outcome = operation(v, w);
              assert(outcome.status >= 0 && outcome.status < 32);
              if (!admits(result, outcome.value) ||
                  !status.in(outcome.status) ||
                  (result_is_first && outcome.value != v) ||
                  (result_is_second && outcome.value != w))
                continue;
              first_supports.add(v);
              second_supports.add(w);
              result_supports.add(outcome.value);
              flags |= 1U << outcome.status;
            }
          }
          if (first_supports.empty())
            return ES_FAILED;
          GECODE_ME_CHECK(first_supports.publish(home, x0));
          GECODE_ME_CHECK(second_supports.publish(home, x1));
          GECODE_ME_CHECK(result_supports.publish(home, result));
          GECODE_ME_CHECK(narrow_flags(home, status, flags));
          return ES_OK;
        }

      public:
        /// Post unless assigned operands decide result and status.
        static ExecStatus post(Home home, WordView x0, WordView x1,
                               ResultView result, StatusView status,
                               const Operation& operation,
                               bool domain_consistent) {
          assert(x0.width() <= 8 && x1.width() <= 8);
          if (x0.assigned() && x1.assigned())
            return narrow_to_supports(home, x0, x1, result, status,
                                      operation);
          (void)new (home) TinyBinary(home, x0, x1, result, status, operation,
                                      domain_consistent);
          return ES_OK;
        }
        Actor* copy(Space& home) override {
          return new (home) TinyBinary(home, *this);
        }
        PropCost cost(const Space&, const ModEventDelta&) const override {
          return domain_consistent ? PropCost::quadratic(PropCost::HI, 256)
                                   : PropCost::binary(PropCost::HI);
        }
        void reschedule(Space& home) override {
          x0.reschedule(home, *this, PC_WORD_DOM);
          x1.reschedule(home, *this, PC_WORD_DOM);
          result.reschedule(home, *this, result_condition(result));
          status.reschedule(home, *this, Int::PC_INT_DOM);
        }
        size_t dispose(Space& home) override {
          x0.cancel(home, *this, PC_WORD_DOM);
          x1.cancel(home, *this, PC_WORD_DOM);
          result.cancel(home, *this, result_condition(result));
          status.cancel(home, *this, Int::PC_INT_DOM);
          (void)Propagator::dispose(home);
          return sizeof(*this);
        }
        // An exact pass is a fixpoint, as in TinyUnary: narrow_values and
        // WordSupports keep exactly the supported values or their tightest
        // cube and bounds, which closure never shrinks below them, and the
        // status keeps exactly the supported flags. A second pass finds every
        // earlier supported tuple again, since its values all remain, and no
        // tuple outside the published domains, so it publishes nothing new.
        // Alias checks hold per tuple, so aliased views receive equal
        // supports. Once the exact pass runs, every later propagation runs
        // it too: assignment and aliasing persist, so the default reasoning
        // never prunes after an exact pass reported ES_FIX.
        ExecStatus propagate(Space& home, const ModEventDelta&) override {
          if (!enumerates_pairs()) {
            const ExecStatus bounds =
                operation.narrow_bounds(home, x0, x1, result, status);
            if (bounds == ES_FAILED || !enumerates_pairs())
              return bounds;
          }
          GECODE_ES_CHECK(
              narrow_to_supports(home, x0, x1, result, status, operation));
          return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this)
                                                  : ES_FIX;
        }
      };
    } // namespace Float
  } // namespace Word
} // namespace Gecode

#endif

// STATISTICS: word-other
