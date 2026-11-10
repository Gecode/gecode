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

#include "test/word/float-oracle.hh"
#include <array>
#include <gecode/float-word.hh>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace Test {
  namespace Word {
    namespace Float {
      namespace {
        using namespace Gecode;
        using Policy = FloatWordPolicy;
        // The expected fused multiply-add from the shared exact oracle: the
        // exact product plus addend of finite operands, rounded once.
        struct Reference {
          const FloatFormat source, destination;
          Reference(WordDomainType s, WordDomainType d)
              : source(float_format(s)), destination(float_format(d)) {}
          FloatResult evaluate(const Policy& p, WordValue a, WordValue b,
                               WordValue c) const {
            WordValue am = source.magnitude(a), bm = source.magnitude(b);
            WordValue cm = destination.magnitude(c);
            assert(am < source.infinity && bm < source.infinity &&
                   cm < destination.infinity);
            if (p.input_subnormals == Policy::SignedZero) {
              if (am < source.normal)
                am = 0;
              if (bm < source.normal)
                bm = 0;
              if (cm < destination.normal)
                cm = 0;
            }
            const bool product_negative =
                (a >= source.sign) != (b >= source.sign);
            const bool addend_negative = c >= destination.sign;
            const ExactRatio av = source.value(am), bv = source.value(bm);
            const ExactRatio cv = destination.value(cm);
            ExactInteger product = av.numerator * bv.numerator * cv.denominator;
            ExactInteger addend = cv.numerator * av.denominator * bv.denominator;
            if (product_negative)
              product = -product;
            if (addend_negative)
              addend = -addend;
            const ExactInteger sum = product + addend;
            const bool negative = sum.zero()
                                      ? (product_negative == addend_negative
                                             ? product_negative
                                             : p.rounding == Policy::RDN)
                                      : sum.neg();
            if (sum.zero())
              return {negative && p.zeros == Policy::Signed ? destination.sign
                                                            : 0,
                      0};
            return round_exact(destination,
                               {sum.abs(), av.denominator * bv.denominator *
                                               cv.denominator},
                               negative, p);
          }
        };

        class FmaSpace : public Space {
        public:
          WordVar x[4];
          IntVar flags;
          explicit FmaSpace(const Reference& ref) : flags(*this, 0, 31) {
            for (int i = 0; i < 4; ++i)
              x[i] = WordVar(*this, i < 2 ? ref.source.type : ref.destination.type);
          }
          FmaSpace(FmaSpace& other) : Space(other) {
            for (int i = 0; i < 4; ++i)
              x[i].update(*this, other.x[i]);
            flags.update(*this, other.flags);
          }
          Space* copy() override { return new FmaSpace(*this); }
          void post(const Policy& p, bool status = true) {
            if (status)
              fp_fma(*this, x[0], x[1], x[2], x[3], flags, p);
            else
              fp_fma(*this, x[0], x[1], x[2], x[3], p);
          }
        };

        bool check_expected(const Reference& ref, const Policy& p, WordValue a,
                            WordValue b, WordValue c, FloatResult expected,
                            bool status = true) {
          FmaSpace s(ref);
          const WordValue inputs[] = {a, b, c};
          for (int i = 0; i < 3; ++i)
            dom(s, s.x[i], inputs[i]);
          s.post(p, status);
          if (s.status() == SS_FAILED)
            return false;
          if (!s.x[3].assigned() ||
              s.x[3].val() != expected.bits ||
              (status &&
               (!s.flags.assigned() || s.flags.val() != expected.flags))) {
            olog << ind(2) << "FMA exact mismatch " << ref.source.type << '/'
                      << ref.destination.type << ' ' << p.rounding << ' '
                      << p.input_subnormals << ' ' << p.output_subnormals << ' '
                      << p.zeros << ' ' << p.tininess << std::hex << " inputs "
                      << a << ' ' << b << ' ' << c << " expected "
                      << expected.bits << std::dec << '/' << expected.flags
                      << '\n';
            return false;
          }
          return true;
        }
        bool check_finite(const Reference& ref, const Policy& p, WordValue a,
                          WordValue b, WordValue c, bool status = true) {
          return check_expected(ref, p, a, b, c, ref.evaluate(p, a, b, c),
                                status);
        }

        const std::array<WordDomainType, 2> signatures[] = {
            {WDT_BF16, WDT_BF16}, {WDT_FP32, WDT_FP32}, {WDT_BF16, WDT_FP32}};

        class Exact : public Base {
        public:
          Exact() : Base("Word::Float::Fma::Wide::Exact") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const std::array<WordValue, 3> catalogue[] = {
                  {s.one + 1, s.one - 2, d.sign + d.one},
                  {1, 1, 0},
                  {s.normal, s.one - 1, 0},
                  {s.normal + 1, s.one - 2, d.sign + 1},
                  {s.maximum, s.one + s.normal, d.sign + d.maximum},
                  {s.maximum, s.maximum, 0},
                  {s.sign, s.one, d.sign},
                  {s.sign + 1, s.one - s.normal, 1}};
              for (int r = 0; r < 4; ++r)
                for (int in = 0; in < 2; ++in)
                  for (int out = 0; out < 2; ++out)
                    for (int ov = 0; ov < 2; ++ov)
                      for (int zero = 0; zero < 2; ++zero)
                        for (int nan = 0; nan < 3; ++nan)
                          for (int tiny = 0; tiny < 2; ++tiny)
                            for (int cmp = 0; cmp < 2; ++cmp) {
                              const Policy p{
                                  static_cast<Policy::Rounding>(r),
                                  static_cast<Policy::Subnormals>(in),
                                  static_cast<Policy::Subnormals>(out),
                                  static_cast<Policy::Overflow>(ov),
                                  static_cast<Policy::Zeros>(zero),
                                  static_cast<Policy::Nan>(nan),
                                  static_cast<Policy::Tininess>(tiny),
                                  static_cast<Policy::Comparison>(cmp)};
                              for (const auto& v : catalogue)
                                if (!check_finite(ref, p, v[0], v[1], v[2]))
                                  return false;
                            }
              // Signed midpoint parity, gradual cancellation, rounded/exact
              // zeros and minimum products under all four directions.
              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                for (WordValue sign : {WordValue(0), s.sign})
                  for (WordValue small : {WordValue(1), WordValue(3),
                                          s.normal - 1, s.normal, s.normal + 1})
                    for (WordValue c : {WordValue(0), WordValue(1), d.sign + 1,
                                        d.normal, d.sign + d.normal})
                      if (!check_finite(ref, p, sign + small, s.one - s.normal,
                                        c))
                        return false;
                for (const auto& v : catalogue)
                  if (!check_finite(ref, p, v[0], v[1], v[2], false))
                    return false;
              }
              const WordValue witness = signature[0] == WDT_FP32   ? 0xa8800000
                                        : signature[1] == WDT_BF16 ? 0xb880
                                                                   : 0xb8800000;
              const auto exact =
                  ref.evaluate(Policy(), s.one + 1, s.one - 2, d.sign + d.one);
              if (exact.bits != witness || exact.flags != 0 ||
                  !check_expected(ref, Policy(), s.one + 1, s.one - 2,
                                  d.sign + d.one, {witness, 0}))
                return false;
              if (s.type == d.type) {
                for (int r = 0; r < 4; ++r)
                  if (!check_expected(ref,
                                      Policy{static_cast<Policy::Rounding>(r)},
                                      s.maximum, s.one + s.normal,
                                      d.sign + d.maximum, {d.maximum, 0}))
                    return false;
              }
              if (s.type == WDT_FP32) {
                const Policy scalar_flush{Policy::RNE,
                                          Policy::Preserve, Policy::SignedZero};
                if (!check_expected(ref, scalar_flush, 0x00800000, 0x3f7fffff,
                                    0, {0x00800000, 24}))
                  return false;
              }
              if (s.type != d.type)
                for (int tiny = 0; tiny < 2; ++tiny) {
                  const Policy p{
                      Policy::RNE, Policy::Preserve,
                      Policy::Preserve, Policy::FormatDefault, Policy::Signed,
                      Policy::SignalFirst, static_cast<Policy::Tininess>(tiny)};
                  if (!check_expected(ref, p, 0x8001, 0x3380, 0x00800000,
                                      {0x00800000, tiny ? 24 : 16}))
                    return false;
                }
            }
            return true;
          }
        };
        class Exceptions : public Base {
        public:
          Exceptions() : Base("Word::Float::Fma::Wide::Exceptions") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const WordValue sq = s.sign + s.infinity + s.quiet + 3;
              const WordValue ss = s.infinity + 5;
              const WordValue dq = d.infinity + d.quiet + 7;
              const WordValue ds = d.sign + d.infinity + 9;
              const unsigned int shift = s.type == d.type ? 0 : 16;
              for (int mode = 0; mode < 3; ++mode) {
                const Policy p{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::FormatDefault, Policy::Signed,
                               static_cast<Policy::Nan>(mode)};
                const WordValue selected =
                    p.nan == Policy::Canonical     ? d.infinity + d.quiet
                    : p.nan == Policy::SignalFirst ? (ss << shift) | d.quiet
                                                   : sq << shift;
                if (!check_expected(ref, p, sq, ss, ds, {selected, 1}) ||
                    !check_expected(
                        ref, p, s.one, s.one, dq,
                        {p.nan == Policy::Canonical ? d.infinity + d.quiet : dq,
                         0}))
                  return false;
                for (int position = 0; position < 3; ++position)
                  for (bool signaling : {false, true}) {
                    WordValue codes[] = {s.one, s.one, d.one};
                    const WordValue code = position == 2
                                               ? (signaling ? ds : dq)
                                               : (signaling ? ss : sq);
                    codes[position] = code;
                    const WordValue result =
                        p.nan == Policy::Canonical
                            ? d.infinity + d.quiet
                            : (position == 2 ? code : code << shift) | d.quiet;
                    if (!check_expected(ref, p, codes[0], codes[1], codes[2],
                                        {result, signaling ? 1 : 0}))
                      return false;
                  }
              }
              for (bool flush : {false, true}) {
                const Policy p{Policy::RNE,
                               flush ? Policy::SignedZero : Policy::Preserve};
                if (!check_expected(ref, p, 0, s.infinity, dq, {dq, 1}) ||
                    !check_expected(ref, p, 1, s.infinity, dq,
                                    {dq, flush ? 1 : 0}) ||
                    !check_expected(ref, p, s.infinity, flush ? 1 : 0, d.one,
                                    {d.infinity + d.quiet, 1}))
                  return false;
              }
              const Policy sat{Policy::RNE, Policy::Preserve,
                               Policy::Preserve, Policy::SaturateFinite};
              if (!check_expected(ref, sat, s.infinity, s.one,
                                  d.sign + d.infinity,
                                  {d.infinity + d.quiet, 1}) ||
                  !check_expected(ref, sat, s.sign + s.infinity, s.one, d.one,
                                  {d.sign + d.infinity, 0}) ||
                  !check_expected(ref, sat, s.one, s.one, d.infinity,
                                  {d.infinity, 0}))
                return false;
            }
            return true;
          }
        };

        /// Total-order window of encodings
        struct Window {
          WordValue minimum, maximum;
        };
        void bounds(Space& s, WordVar word, Window w) {
          float_bounds(s, word, w.minimum, w.maximum);
        }
        /// The window of the encodings with sign bit \a sign, which may be
        /// zero, and magnitudes from \a low to \a high
        Window magnitudes(WordValue sign, WordValue low, WordValue high) {
          return sign != 0 ? Window{sign | high, sign | low}
                           : Window{low, high};
        }
        Window fixed_window(WordValue code) {
          return {code, code};
        }
        using Windows = std::array<Window, 4>;
        using Row = std::array<WordValue, 5>;
        using Rows = std::set<Row>;
        enum class Alias {
          Independent,
          RepeatedMultiplicands,
          RepeatedAddend,
          RepeatedSecondAddend,
          ResultIsFirst,
          ResultIsSecond,
          ResultIsSquare,
          ResultIsFirstAndAddend,
          ResultIsAddend,
          ResultIsSquareAddend,
          ResultIsAll,
          RepeatedInputs
        };
        // The column whose value each column repeats; free columns map to
        // themselves.
        std::array<int, 4> origins(Alias alias) {
          switch (alias) {
          case Alias::Independent:
            return {0, 1, 2, 3};
          case Alias::RepeatedMultiplicands:
            return {0, 0, 2, 3};
          case Alias::RepeatedAddend:
            return {0, 1, 0, 3};
          case Alias::RepeatedSecondAddend:
            return {0, 1, 1, 3};
          case Alias::ResultIsFirst:
            return {0, 1, 2, 0};
          case Alias::ResultIsSecond:
            return {0, 1, 2, 1};
          case Alias::ResultIsSquare:
            return {0, 0, 2, 0};
          case Alias::ResultIsFirstAndAddend:
            return {0, 1, 0, 0};
          case Alias::ResultIsAddend:
            return {0, 1, 2, 2};
          case Alias::ResultIsSquareAddend:
            return {0, 0, 2, 2};
          case Alias::ResultIsAll:
            return {0, 0, 0, 0};
          case Alias::RepeatedInputs:
            return {0, 0, 0, 3};
          }
          GECODE_NEVER;
          return {};
        }
        unsigned int column_width(const Reference& ref, int i) {
          return float_word_width(i < 2 ? ref.source.type : ref.destination.type);
        }
        // The values of column i in a row, in total order: its window when
        // free, otherwise the value of the earlier column it repeats.
        std::vector<WordValue> column_values(const Reference& ref,
                                             const Windows& windows,
                                             const std::array<int, 4>& origin,
                                             const Row& row, int i) {
          if (origin[i] != i)
            return {row[origin[i]]};
          return float_order_range(column_width(ref, i), windows[i].minimum,
                                   windows[i].maximum);
        }
        // Enumerate only free input columns: square alias windows would
        // otherwise iterate the cube of an alias window.
        Rows expected_rows(const Reference& ref, const Policy& p,
                           const Windows& windows, Alias alias, int status) {
          const auto origin = origins(alias);
          Row row{};
          Rows expected;
          for (WordValue a : column_values(ref, windows, origin, row, 0)) {
            row[0] = a;
            for (WordValue b : column_values(ref, windows, origin, row, 1)) {
              row[1] = b;
              for (WordValue c : column_values(ref, windows, origin, row, 2)) {
                row[2] = c;
                const auto result = ref.evaluate(p, a, b, c);
                if ((origin[3] == 3 || result.bits == row[origin[3]]) &&
                    float_order_contains(column_width(ref, 3),
                                         windows[3].minimum, windows[3].maximum,
                                         result.bits) &&
                    (status < 0 || status == result.flags))
                  expected.insert(
                      {a, b, c, result.bits, WordValue(result.flags)});
              }
            }
          }
          return expected;
        }
        void configure(FmaSpace& s, const Windows& windows, Alias alias,
                       int status = -1) {
          const auto origin = origins(alias);
          for (int i = 1; i < 4; ++i)
            s.x[i] = s.x[origin[i]];
          for (int i = 0; i < 4; ++i)
            bounds(s, s.x[i], windows[i]);
          if (status >= 0)
            rel(s, s.flags, IRT_EQ, status);
        }
        bool preserves(FmaSpace& s, const Rows& expected) {
          if (s.status() == SS_FAILED)
            return expected.empty();
          for (const auto& row : expected) {
            for (int i = 0; i < 4; ++i)
              if (!Gecode::Word::WordView(s.x[i]).in(row[i]))
                return false;
            if (!s.flags.in(static_cast<int>(row[4])))
              return false;
          }
          return true;
        }
        void branching(FmaSpace& s) {
          branch(s,
                 WordVarArgs{s.x[0], s.x[1], s.x[2],
                             s.x[3]},
                 WORD_VAL_LSB());
          branch(s, s.flags, INT_VAL_MIN());
        }
        Rows solutions(FmaSpace& s, unsigned int recompute = 8) {
          Rows found;
          Search::Options options;
          options.c_d = recompute;
          options.a_d = recompute;
          DFS<FmaSpace> search(&s, options);
          while (std::unique_ptr<FmaSpace> next{search.next()})
            found.insert({next->x[0].val(), next->x[1].val(),
                          next->x[2].val(), next->x[3].val(),
                          WordValue(next->flags.val())});
          return found;
        }
        bool check_window(const Reference& ref, const Policy& p,
                          const Windows& windows, int prune = -1,
                          Alias alias = Alias::Independent, int status = -1) {
          const Rows expected = expected_rows(ref, p, windows, alias, status);
          FmaSpace s(ref);
          configure(s, windows, alias, status);
          for (int i = 0; i < 3; ++i)
            if (windows[i].minimum != windows[i].maximum &&
                s.x[i].assigned())
              return false;
          s.post(p);
          if (!preserves(s, expected)) {
            olog << ind(2) << "FMA lost root support " << ref.source.type << '/'
                      << ref.destination.type << '\n';
            return false;
          }
          if (s.status() == SS_FAILED)
            return expected.empty();
          if (prune >= 0) {
            const auto view = Gecode::Word::WordView(s.x[prune]);
            if (view.in(windows[prune].minimum) &&
                view.in(windows[prune].maximum)) {
              olog << ind(2) << "FMA did not prune " << ref.source.type << '/'
                        << ref.destination.type << ' ' << prune << '\n';
              return false;
            }
          }
          branching(s);
          const auto found = solutions(s);
          if (found != expected) {
            olog << ind(2) << "FMA tuples " << ref.source.type << '/'
                      << ref.destination.type << ' ' << static_cast<int>(alias)
                      << ' ' << found.size() << '/' << expected.size() << '\n';
            return false;
          }
          return true;
        }
        class Symbolic : public Base {
        public:
          Symbolic() : Base("Word::Float::Fma::Wide::Symbolic") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const bool mixed = s.type != d.type;
              const WordValue witness = s.type == WDT_FP32 ? 0xa8800000
                                        : mixed          ? 0xb8800000
                                                         : 0xb880;
              const WordValue inverse = mixed ? 0x40008000 : d.one + d.normal;
              const Windows cancellation{
                  {{s.one - 1, s.one + 3},
                   {s.one - 4, s.one},
                   {d.sign + d.one + 2, d.sign + d.one - 2},
                   {witness, witness}}};
              const Windows addend{{{s.one, s.one + 3},
                                    {s.one, s.one + 3},
                                    {d.one, d.one + 15},
                                    {inverse, inverse + 2}}};
              const Windows multiplicand{{{s.one, s.one + 31},
                                          {s.one, s.one + 3},
                                          {d.one, d.one + 3},
                                          {inverse + 1, inverse + 2}}};
              const Window all{d.sign | (d.sign - 1), d.sign - 1};
              if (!check_window(ref, Policy(), cancellation, 0) ||
                  !check_window(ref, Policy(), addend, 2) ||
                  !check_window(ref, Policy(), multiplicand, 0) ||
                  !check_window(ref, Policy(),
                                {{{s.one, s.one + 3},
                                  {s.one, s.one + 3},
                                  {d.one, d.one + 3},
                                  all}},
                                3))
                return false;
              if (!check_window(ref, Policy(),
                                {{{s.one, s.one + 3},
                                  {s.one - 4, s.one - 1},
                                  {d.sign + d.one + 3, d.sign + d.one},
                                  {witness, witness}}},
                                0))
                return false;
              for (int r = 0; r < 4; ++r) {
                const Policy p{static_cast<Policy::Rounding>(r)};
                for (Alias alias :
                     {Alias::RepeatedMultiplicands, Alias::RepeatedAddend,
                      Alias::ResultIsFirst, Alias::ResultIsAddend,
                      Alias::RepeatedInputs}) {
                  if (mixed && alias != Alias::RepeatedMultiplicands &&
                      alias != Alias::ResultIsAddend)
                    continue;
                  // Small exact cubes exercise alias identity, including
                  // result aliases whose supported code differs by tuple.
                  const Windows q{{{0, 3},
                                   {0, 3},
                                   {0, 3},
                                   alias == Alias::ResultIsFirst ||
                                           alias == Alias::ResultIsAddend
                                       ? Window{0, 3}
                                       : all}};
                  if (!check_window(ref, p, q, -1, alias))
                    return false;
                }
                for (bool flush : {false, true}) {
                  const Policy pflush{
                      static_cast<Policy::Rounding>(r),
                      flush ? Policy::SignedZero : Policy::Preserve,
                      Policy::SignedZero};
                  if (!check_window(ref, pflush,
                                    {{{s.normal - 1, s.normal + 2},
                                      {s.one - 1, s.one + 1},
                                      {d.sign + 3, d.sign + 1},
                                      all}}))
                    return false;
                }
              }
              // Restricting result and flags independently must not create a
              // tuple absent from the exact relation.
              const Windows q{{{s.one, s.one + 3},
                               {s.one, s.one + 3},
                               {d.one, d.one + 15},
                               {inverse, inverse + 2}}};
              for (int flags : {0, 16, 24})
                if (!check_window(ref, Policy(), q, -1, Alias::Independent,
                                  flags))
                  return false;
            }
            // Both marginal result/flag choices have support, but their
            // crossed pair does not belong to the relation.
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& a = ref.source;
              const auto& z = ref.destination;
              FmaSpace correlated(ref);
              configure(correlated,
                        {{{a.one, a.one},
                          {a.one, a.one},
                          {z.one + 1, z.one + 2},
                          {z.one + z.normal, z.one + z.normal + 1}}},
                        Alias::Independent);
              correlated.post(Policy());
              if (correlated.status() == SS_FAILED || !correlated.flags.in(0) ||
                  !correlated.flags.in(16) ||
                  !Gecode::Word::WordView(correlated.x[3])
                       .in(z.one + z.normal + 1))
                return false;
              dom(correlated, correlated.x[3], z.one + z.normal + 1);
              rel(correlated, correlated.flags, IRT_EQ, 16);
              if (correlated.status() != SS_FAILED)
                return false;
            }
            return true;
          }
        };
        std::array<WordValue, 17> snapshot(FmaSpace& s) {
          std::array<WordValue, 17> result{};
          for (int i = 0; i < 4; ++i) {
            const Gecode::Word::WordView view(s.x[i]);
            result[4 * i] = view.lo();
            result[4 * i + 1] = view.hi();
            result[4 * i + 2] =
                view.bounded() ? view.rank_minimum() : view.lo();
            result[4 * i + 3] =
                view.bounded() ? view.rank_maximum() : view.hi();
          }
          for (IntVarValues flags(s.flags); flags(); ++flags)
            result[16] |= WordValue(1) << flags.val();
          return result;
        }
        void restrict_later(FmaSpace& s, const Reference& ref) {
          dom(s, s.x[0], WordValue(0), s.x[0].mask() ^ 1);
          bounds(s, s.x[1], {ref.source.one + 1, ref.source.one + 3});
          rel(s, s.flags, IRT_NQ, 16);
        }
        class Lifecycle : public Base {
        public:
          Lifecycle() : Base("Word::Float::Fma::Wide::Lifecycle") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& s = ref.source;
              const auto& d = ref.destination;
              const WordValue inverse =
                  s.type != d.type ? 0x40008000 : d.one + d.normal;
              const Windows small{{{s.one, s.one + 3},
                                   {s.one, s.one + 3},
                                   {d.one, d.one + 15},
                                   {inverse, inverse + 2}}};
              const Rows expected =
                  expected_rows(ref, Policy(), small, Alias::Independent, -1);
              Rows restricted;
              for (const auto& row : expected)
                if (!(row[0] & 1) && row[1] >= s.one + 1 && row[4] != 16)
                  restricted.insert(row);
              FmaSpace original(ref);
              configure(original, small, Alias::Independent);
              original.post(Policy());
              branching(original);
              if (!preserves(original, expected))
                return false;
              const auto before = snapshot(original);
              std::unique_ptr<FmaSpace> clone(
                  static_cast<FmaSpace*>(original.clone()));
              restrict_later(*clone, ref);
              FmaSpace fresh(ref);
              configure(fresh, small, Alias::Independent);
              restrict_later(fresh, ref);
              fresh.post(Policy());
              branching(fresh);
              if (!preserves(*clone, restricted) ||
                  !preserves(fresh, restricted) ||
                  solutions(*clone, 1) != restricted ||
                  solutions(fresh, 16) != restricted ||
                  original.status() == SS_FAILED ||
                  snapshot(original) != before ||
                  solutions(original) != expected)
                return false;
              // Above the input-assignment scan cap: endpoint projection must
              // narrow direct and inverse domains without scanning triples.
              const Windows broad{{{s.one, s.one + s.normal},
                                   {s.one, s.one + s.normal},
                                   {d.one, d.one + d.normal},
                                   {d.sign | (d.sign - 1), d.sign - 1}}};
              FmaSpace direct(ref);
              configure(direct, broad, Alias::Independent);
              direct.post(Policy());
              if (direct.status() == SS_FAILED ||
                  Gecode::Word::WordView(direct.x[3]).in(0) ||
                  !Gecode::Word::WordView(direct.x[3])
                       .in(d.one + d.normal))
                return false;
              FmaSpace inverse_model(ref);
              configure(inverse_model, broad, Alias::Independent);
              dom(inverse_model, inverse_model.x[3], d.one + d.normal);
              inverse_model.post(Policy());
              if (inverse_model.status() == SS_FAILED)
                return false;
              for (int i = 0; i < 3; ++i) {
                const auto& f = i < 2 ? s : d;
                const auto view = Gecode::Word::WordView(inverse_model.x[i]);
                if (!view.in(f.one) || view.in(f.one + f.normal))
                  return false;
              }
              // Keep all inputs broad through at least 70 externally
              // scheduled finite projections, then restrict another broad
              // input.
              FmaSpace aged(ref);
              configure(aged, broad, Alias::Independent);
              aged.post(Policy());
              for (WordValue k = 0; k < 70; ++k) {
                bounds(aged, aged.x[0], {s.one, s.one + s.normal - k});
                if (aged.status() == SS_FAILED)
                  return false;
              }
              std::unique_ptr<FmaSpace> aged_clone(
                  static_cast<FmaSpace*>(aged.clone()));
              // Both aged actors must still respond above the scan cap.
              // Even BF16 retains 60*65*129 distinct input assignments.
              for (FmaSpace* model : {&aged, aged_clone.get()}) {
                bounds(*model, model->x[1],
                       {s.one, s.one + s.normal / 2});
                const auto upper =
                    ref.evaluate(Policy(), s.one + s.normal - 69,
                                 s.one + s.normal / 2, d.one + d.normal);
                if (model->status() == SS_FAILED ||
                    model->x[3].maximum() != upper.bits ||
                    model->x[0].assigned() ||
                    model->x[1].assigned() ||
                    model->x[2].assigned())
                  return false;
                const auto stable = snapshot(*model);
                std::unique_ptr<FmaSpace> closure(
                    static_cast<FmaSpace*>(model->clone()));
                closure->post(Policy());
                if (closure->status() == SS_FAILED ||
                    snapshot(*closure) != stable)
                  return false;
              }
              const auto before_scan = snapshot(aged);
              const Windows after{{{s.one, s.one + 3},
                                   {s.one, s.one + 3},
                                   {d.one, d.one + 3},
                                   {d.one + d.normal, d.one + d.normal + 2}}};
              configure(*aged_clone, after, Alias::Independent);
              const Rows supports =
                  expected_rows(ref, Policy(), after, Alias::Independent, -1);
              if (supports.empty() || !preserves(*aged_clone, supports) ||
                  aged.status() == SS_FAILED || snapshot(aged) != before_scan)
                return false;
              branching(*aged_clone);
              if (solutions(*aged_clone) != supports)
                return false;
              // Ground enforcement also survives the delayed restrictions.
              std::unique_ptr<FmaSpace> wrong(
                  static_cast<FmaSpace*>(aged.clone()));
              for (int i = 0; i < 3; ++i)
                dom(*wrong, wrong->x[i], i < 2 ? s.one : d.one);
              dom(*wrong, wrong->x[3], d.one + d.normal + 1);
              if (wrong->status() != SS_FAILED)
                return false;
              for (WordValue as : {WordValue(0), s.sign})
                for (WordValue bs : {WordValue(0), s.sign})
                  for (WordValue cs : {WordValue(0), d.sign}) {
                    FmaSpace signs(ref);
                    const auto value = ref.evaluate(Policy(), as + s.one,
                                                    bs + s.one, cs + d.one);
                    configure(signs,
                              {{magnitudes(as, s.one, s.one + s.normal),
                                magnitudes(bs, s.one, s.one + s.normal),
                                magnitudes(cs, d.one, d.one + d.normal),
                                {value.bits, value.bits}}},
                              Alias::Independent);
                    signs.post(Policy());
                    const Rows known{{as + s.one, bs + s.one, cs + d.one,
                                      value.bits, WordValue(value.flags)}};
                    if (!preserves(signs, known))
                      return false;
                  }
            }
            // Remove the interior status value 16 while preserving min=0
            // and max=24. PC_INT_DOM must wake the actor and prune the result.
            for (WordDomainType format : {WDT_BF16, WDT_FP32}) {
              const Reference ref(format, format);
              const auto& f = ref.source;
            const Windows q{{{f.normal, f.normal + 1},
                             {f.one - 1, f.one + 1},
                             {0, 1},
                             {f.sign | (f.sign - 1), f.sign - 1}}};
              FmaSpace original(ref);
              configure(original, q, Alias::Independent);
              original.post(Policy());
              const Rows expected =
                  expected_rows(ref, Policy(), q, Alias::Independent, -1);
              if (!preserves(original, expected) || original.flags.min() != 0 ||
                  original.flags.max() != 24 || !original.flags.in(16) ||
                  !Gecode::Word::WordView(original.x[3])
                       .in(f.normal + 3))
                return false;
              const auto before = snapshot(original);
              std::unique_ptr<FmaSpace> clone(
                  static_cast<FmaSpace*>(original.clone()));
              rel(*clone, clone->flags, IRT_NQ, 16);
              Rows restricted;
              for (const auto& row : expected)
                if (row[4] != 16)
                  restricted.insert(row);
              if (!preserves(*clone, restricted) || clone->flags.min() != 0 ||
                  clone->flags.max() != 24 ||
                  Gecode::Word::WordView(clone->x[3]).in(f.normal + 3) ||
                  snapshot(original) != before)
                return false;
              branching(*clone);
              if (solutions(*clone) != restricted)
                return false;
            }
            return true;
          }
        };

        class Absorption : public Base {
        public:
          Absorption() : Base("Word::Float::Fma::Wide::Absorption") {}
          bool run() override {
            for (const auto& signature : signatures) {
              const Reference ref(signature[0], signature[1]);
              const auto& a = ref.source;
              const auto& c = ref.destination;
              const WordValue threshold = (128 + c.fraction) * c.normal;
              FmaSpace diagnostic(ref);
              configure(diagnostic,
                        {{{a.one, a.one},
                          {a.one, a.one},
                          {c.one, threshold},
                          {c.one, threshold}}},
                        Alias::ResultIsAddend);
              diagnostic.post(Policy());
              if (diagnostic.status() == SS_FAILED ||
                  !diagnostic.x[2].assigned() ||
                  diagnostic.x[2].val() != threshold ||
                  !diagnostic.flags.assigned() || diagnostic.flags.val() != 16)
                return false;
              for (int rounding = 0; rounding < 4; ++rounding)
                for (WordValue product_sign : {WordValue(0), a.sign})
                  for (WordValue addend_sign : {WordValue(0), c.sign})
                    for (WordValue boundary :
                         {threshold - c.normal, threshold}) {
                      const Policy policy{
                          static_cast<Policy::Rounding>(rounding)};
                    const Window addend =
                        magnitudes(addend_sign, boundary - 2, boundary + 2);
                    const Windows small{
                        {magnitudes(product_sign, a.one, a.one + 2),
                         {a.one, a.one + 2},
                         addend,
                         addend}};
                      const Rows expected =
                          expected_rows(ref, policy, small, Alias::ResultIsAddend, -1);
                      Windows broad = small;
                    broad[0] =
                        magnitudes(product_sign, a.one, a.one + a.normal);
                    broad[1] = {a.one, a.one + a.normal};
                      FmaSpace original(ref);
                      configure(original, broad, Alias::ResultIsAddend);
                      original.post(policy);
                      // Above the scan cap, this exercises the residual
                      // projector before the complete bounded tuple check.
                      if (!preserves(original, expected))
                        return false;
                      if (original.failed())
                        continue;
                      const auto before = snapshot(original);
                      std::unique_ptr<FmaSpace> clone(
                          static_cast<FmaSpace*>(original.clone()));
                      clone->post(policy);
                      if (clone->status() == SS_FAILED ||
                          snapshot(*clone) != before)
                        return false;
                      configure(*clone, small, Alias::ResultIsAddend);
                      if (!preserves(*clone, expected) ||
                          snapshot(original) != before)
                        return false;
                      if (clone->failed())
                        continue;
                      branching(*clone);
                      if (solutions(*clone) != expected)
                        return false;
                    }
            }
            return true;
          }
        };

        // Outcome of propagating a fresh space to its first fixpoint.
        struct Settled {
          SpaceStatus status;
          // Propagator executions needed to reach the fixpoint.
          unsigned long long int propagations;
        };
        Settled settle(FmaSpace& s) {
          StatusStatistics statistics;
          const SpaceStatus status = s.status(statistics);
          return {status, statistics.propagate};
        }
        class MultiplicandAlias : public Base {
        public:
          MultiplicandAlias()
              : Base("Word::Float::Fma::Wide::MultiplicandAlias") {}
          bool run() override {
            // Above the support scan, convergence must not depend on the
            // number of values the projection has to rule out.
            static constexpr unsigned long long int bound = 8;
            const Reference ref(WDT_FP32, WDT_FP32);
            const WordValue one = ref.source.one, normal = ref.source.normal;
            const WordValue up = one + 1;
              const Window binade{one, one + normal - 1};
            // Rounding x*(1+2^-23) moves every normal x up by one ULP.
            for (Alias alias : {Alias::ResultIsFirst, Alias::ResultIsSecond}) {
              unsigned long long int counts[2];
              int count = 0;
              for (const Window& x : {binade, Window{one, one + 0xffff}}) {
                const Windows w = alias == Alias::ResultIsFirst
                                      ? Windows{{x, fixed_window(up), fixed_window(0), x}}
                                      : Windows{{fixed_window(up), x, fixed_window(0), x}};
                FmaSpace s(ref);
                configure(s, w, alias);
                s.post(Policy());
                const Settled settled = settle(s);
                if (settled.status != SS_FAILED || settled.propagations > bound)
                  return false;
                counts[count++] = settled.propagations;
              }
              if (counts[0] != counts[1])
                return false;
            }
            FmaSpace square(ref);
            configure(square, {{binade, binade, fixed_window(0), binade}},
                      Alias::ResultIsSquare);
            square.post(Policy());
            const Settled squared = settle(square);
            if (squared.status == SS_FAILED || squared.propagations > bound ||
                !square.x[0].assigned() ||
                square.x[0].val() != one)
              return false;
            FmaSpace absorbed(ref);
            configure(absorbed, {{binade, fixed_window(up), binade, binade}},
                      Alias::ResultIsFirstAndAddend);
            absorbed.post(Policy());
            const Settled settled = settle(absorbed);
            if (settled.status != SS_FAILED || settled.propagations > bound)
              return false;
            // Windows just above the 2,048-assignment scan cap, each with a
            // few solutions. Near the smallest normal they mix preserved or
            // flushed subnormals with the tie at the power of two. Under RNE
            // x*(1-2^-24) keeps subnormal x and the smallest normal; under
            // RTZ x*(1+2^-23) keeps subnormal x. x+x*b keeps normal x below
            // 1+2^-19 when b is just below half (RNE) or one (RTZ) ULP of 1.
            // x*x+2^-20 rounds to x only at 1-2^-20, and x*x+2^-126 only at
            // the smallest normal.
            const Policy flush{Policy::RNE, Policy::Preserve,
                               Policy::SignedZero};
            const Window low{normal - 16, normal + 2048};
            const Window near_one{one - 32, one + 2032};
            for (const Policy& p :
                 {Policy(), Policy{Policy::RTZ}, flush}) {
              const bool truncates = p.rounding == Policy::RTZ;
              const Window product = fixed_window(truncates ? up : one - 1);
              const Window absorbed_factor =
                  fixed_window(truncates ? 0x33ffffe0 : 0x337fffe0);
              const Windows square_windows =
                  p.output_subnormals == Policy::SignedZero
                      ? Windows{{low, low, fixed_window(normal), low}}
                      : Windows{
                            {near_one, near_one, fixed_window(0x35800000), near_one}};
              const std::pair<Alias, Windows> cases[] = {
                  {Alias::ResultIsFirst, {{low, product, fixed_window(0), low}}},
                  {Alias::ResultIsSecond, {{product, low, fixed_window(0), low}}},
                  {Alias::ResultIsSquare, square_windows},
                  {Alias::ResultIsFirstAndAddend, {{low, absorbed_factor, low, low}}}};
              for (const auto& c : cases)
                if (!check_window(ref, p, c.second, -1, c.first))
                  return false;
            }
            return true;
          }
        };

        /// Encodings (a, b, c) whose result a fixpoint must keep
        using Support = std::array<WordValue, 3>;
        // Whether the views of s keep every support with its result, which
        // must lie in the result window.
        bool keeps(const Reference& ref, const Policy& p, FmaSpace& s,
                   const Windows& windows,
                   const std::vector<Support>& supports) {
          for (const Support& support : supports) {
            const WordValue row[] = {
                support[0], support[1], support[2],
                ref.evaluate(p, support[0], support[1], support[2]).bits};
            for (int i = 0; i < 4; ++i)
              if (!float_order_contains(column_width(ref, i),
                                        windows[i].minimum, windows[i].maximum,
                                        row[i]) ||
                  !Gecode::Word::WordView(s.x[i]).in(row[i]))
                return false;
          }
          return true;
        }
        class Cancellation : public Base {
          struct Case {
            WordDomainType format;
            Policy policy;
            Alias alias;
            Windows windows;
            std::vector<Support> supports;
          };

        public:
          Cancellation() : Base("Word::Float::Fma::Wide::Cancellation") {}
          bool run() override {
            // When c is close to -a*b, |a*b+c| is far below the spacing of
            // b and c. Projecting c <= r - a*b and b <= (r - c) / a then
            // rounds each bound to its neighbour on the other lattice, one
            // encoding per execution, unless the propagator jumps over the
            // run. Each case took hundreds or thousands of executions.
            static constexpr unsigned long long int bound = 8;
            Policy down;
            down.rounding = Policy::RDN;
            Policy up;
            up.rounding = Policy::RUP;
            const Case cases[] = {
                // -b + c around 2^-108, both bounds of b and c creep
                {WDT_FP32,
                 Policy(),
                 Alias::Independent,
                 {{fixed_window(0xbf800000), {0x097fff00, 0x09800400},
                   {0x097fff00, 0x09800400}, {0x805282a9, 0x80507bd0}}},
                 {{0xbf800000, 0x09800014, 0x097fffff},
                  {0xbf800000, 0x097fff40, 0x097fff17},
                  {0xbf800000, 0x097fff29, 0x097fff00}}},
                // Assigned result, b and c across zero; the supports are
                // the bounds of the fixpoint
                {WDT_BF16,
                 Policy(),
                 Alias::Independent,
                 {{fixed_window(0xc6b8), {0x8992, 0x013d}, {0xb4f2, 0x31c6},
                   fixed_window(0x8888)}},
                 {{0xc6b8, 0x8569, 0x8ca8},
                  {0xc6b8, 0x013d, 0x84a8},
                  {0xc6b8, 0x013d, 0x0430}}},
                // a*b + a with b fixed: the product and addend roles of a
                // narrow it by the factor |b| per execution
                {WDT_FP32,
                 down,
                 Alias::RepeatedAddend,
                 {{{0x5e2ec7e7, 0x7b6578e5}, fixed_window(0xbf9a505d),
                   {0x5e2ec7e7, 0x7b6578e5}, {0xdd18e603, 0xdc5e272a}}},
                 {{0x5e2ec7e7, 0xbf9a505d, 0x5e2ec7e7},
                  {0x5e39f011, 0xbf9a505d, 0x5e39f011}}},
                {WDT_BF16,
                 up,
                 Alias::RepeatedAddend,
                 {{{0x81df, 0x78c6}, fixed_window(0xc013), {0x81df, 0x78c6},
                   {0x0000, 0x01b5}}},
                 {{0x818b, 0xc013, 0x818b}, {0x0000, 0xc013, 0x0000}}}};
            for (const Case& c : cases) {
              const Reference ref(c.format, c.format);
              FmaSpace s(ref);
              configure(s, c.windows, c.alias);
              s.post(c.policy);
              const Settled settled = settle(s);
              if (settled.status == SS_FAILED || settled.propagations > bound ||
                  !keeps(ref, c.policy, s, c.windows, c.supports)) {
                olog << ind(2) << "FMA cancellation " << ref.source.type << ' '
                          << settled.propagations << '\n';
                return false;
              }
            }
            return true;
          }
        };
        class CancellationSoundness : public Base {
        public:
          CancellationSoundness()
              : Base("Word::Float::Fma::Wide::CancellationSoundness") {}
          bool run() override {
            // Windows above the 2,048-assignment scan cap, so the
            // projections and their cancellation bounds run, at the root
            // and in every search node.
            const Policy flush{Policy::RNE, Policy::SignedZero,
                               Policy::SignedZero};
            Policy positive;
            positive.zeros = Policy::Positive;
            const Policy policies[] = {Policy(),           Policy{Policy::RTZ},
                                       Policy{Policy::RDN}, Policy{Policy::RUP},
                                       flush,              positive};
            // Thirteen encodings around 1, 3, -3 and -1
            const Window one = magnitudes(0, 0x3f7a, 0x3f86);
            const Window three = magnitudes(0, 0x403a, 0x4046);
            const Window minus_three = magnitudes(0x8000, 0x403a, 0x4046);
            const Window minus_one = magnitudes(0x8000, 0x3f7a, 0x3f86);
            // Subnormals and zeros of both signs
            const Window tiny{0x8006, 0x0006};
            // Across the smallest normal
            const Window edge = magnitudes(0, 0x007a, 0x0086);
            // Sixty-four encodings for the two free columns of a*b+a
            const Window wide_one = magnitudes(0, 0x3f60, 0x3f9f);
            const Window wide_minus_one = magnitudes(0x8000, 0x3f60, 0x3f9f);
            const Window wide_tiny{0x8020, 0x0020};
            const Window finite{0xff7f, 0x7f7f};
            const Window small{0xbc80, 0x3c80};
            const std::pair<Alias, Windows> bf16[] = {
                {Alias::Independent, {{one, three, minus_three, finite}}},
                {Alias::Independent, {{one, three, minus_three, small}}},
                {Alias::Independent, {{tiny, one, tiny, finite}}},
                {Alias::Independent, {{tiny, one, tiny, Window{0x8001, 0x0001}}}},
                {Alias::Independent, {{minus_one, edge, edge, finite}}},
                {Alias::Independent,
                 {{minus_one, edge, edge, Window{0x8004, 0x0004}}}},
                {Alias::RepeatedAddend,
                 {{wide_one, wide_minus_one, wide_one, small}}},
                {Alias::RepeatedAddend,
                 {{wide_tiny, wide_minus_one, wide_tiny,
                   Window{0x8001, 0x0001}}}}};
            // Products of BF16 values are exact in FP32
            const std::pair<Alias, Windows> mixed[] = {
                {Alias::Independent,
                 {{one, three, magnitudes(0x80000000, 0x403ffffa, 0x40400006),
                   Window{0xbc800000, 0x3c800000}}}},
                {Alias::Independent,
                 {{tiny, one, Window{0x80000006, 0x00000006},
                   Window{0xff7fffff, 0x7f7fffff}}}}};
            const Reference narrow(WDT_BF16, WDT_BF16);
            const Reference wide(WDT_BF16, WDT_FP32);
            for (const Policy& p : policies) {
              for (const auto& c : bf16)
                if (!check_window(narrow, p, c.second, -1, c.first))
                  return false;
              for (const auto& c : mixed)
                if (!check_window(wide, p, c.second, -1, c.first))
                  return false;
            }
            return true;
          }
        };

        /// Window of a column whose bits under mask equal value
        struct BitWindow {
          Window window;
          WordValue mask, value;
        };
        using BitWindows = std::array<BitWindow, 4>;
        // Create the columns of s from windows with known bits, aliased as
        // alias says.
        void configure(FmaSpace& s, const Reference& ref,
                       const BitWindows& windows, Alias alias) {
          const auto origin = origins(alias);
          for (int i = 0; i < 4; ++i) {
            const BitWindow& w = windows[i];
            s.x[i] = origin[i] != i
                         ? s.x[origin[i]]
                         : WordVar(s, column_width(ref, i),
                                   i < 2 ? ref.source.type : ref.destination.type,
                                   w.window.minimum, w.window.maximum,
                                   known_bits(w.mask, w.value));
          }
        }
        BitWindow plain(Window w) {
          return {w, 0, 0};
        }
        class Strength : public Base {
          struct Case {
            WordDomainType source, destination;
            Policy policy;
            Alias alias;
            BitWindows windows;
            // Windows that every column lies in at the fixpoint
            Windows hull;
            std::vector<Support> supports;
            unsigned long long int bound;
          };

        public:
          Strength() : Base("Word::Float::Fma::Wide::Strength") {}
          bool run() override {
            // Four distinct words settle by the step rule, at the fixpoint
            // of the widened projection or tighter. Aliased words settle at
            // the fixpoint of repeated single projections, or tighter,
            // however many projections it takes: no narrowing is discarded
            // for being small, and an aliased role never weakens the
            // projection of the unaliased constraint.
            Policy rtz;
            rtz.rounding = Policy::RTZ;
            Policy up;
            up.rounding = Policy::RUP;
            const Window positive{0x00000001, 0x7f7fffff};
            const Window square{0x3f91a0ed, 0x4791a0f2};
            const Window large_square{0x44dea6e4, 0x4cdea6e7};
            const Case cases[] = {
                // a*b + c cancels far below the spacing of b and c, and each
                // projection moves their bounds a little
                {WDT_BF16,
                 WDT_BF16,
                 rtz,
                 Alias::Independent,
                 {{plain(fixed_window(0xbfe7)),
                   {{0x6f7f, 0x7f7f}, 0xe000, 0x6000},
                   {{0x6f80, 0x7f7c}, 0xe003, 0x6000},
                   plain(fixed_window(0xf910))}},
                 {{fixed_window(0xbfe7), {0x78a0, 0x7cf0}, {0x6f80, 0x7d58},
                   fixed_window(0xf910)}},
                 {{0xbfe7, 0x78a0, 0x6f80}, {0xbfe7, 0x7cf0, 0x7d58}},
                 64},
                {WDT_BF16,
                 WDT_FP32,
                 up,
                 Alias::Independent,
                 {{{{0x0000, 0x046f}, 0xf800, 0x0000},
                   {{0x798c, 0x7d8c}, 0xf800, 0x7800},
                   {{0xbd8b494f, 0xbd8b484a}, 0xfffffe00, 0xbd8b4800},
                   plain(fixed_window(0))}},
                 // Repeated interval projections: a [0x00b5, 0x0345],
                 // b [0x79b5, 0x7c45], 5 executions.
                 {{{0x0020, 0x037e}, {0x798c, 0x7d8c},
                   {0xbd8b494f, 0xbd8b484a}, fixed_window(0)}},
                 {{0x00b5, 0x7c45, 0xbd8b4900}, {0x0345, 0x79b5, 0xbd8b4900}},
                 64},
                // a*b + b is b*(a + 1) = -b * 2^-7, which one factored
                // projection settles
                {WDT_FP32,
                 WDT_FP32,
                 Policy(),
                 Alias::RepeatedSecondAddend,
                 {{plain(fixed_window(0xbf810000)), plain(positive),
                   plain(positive), plain({0x80ffffff, 0x80800000})}},
                 {{fixed_window(0xbf810000), {0x03ffffff, 0x047fffff},
                   {0x03ffffff, 0x047fffff}, {0x80ffffff, 0x80800000}}},
                 {{0xbf810000, 0x03ffffff, 0x03ffffff},
                  {0xbf810000, 0x047fffff, 0x047fffff}},
                 8},
                // x*x + c with c assigned: bisection on the codes of x settles
                // x, where bounding one x by the other moves it slowly
                {WDT_FP32,
                 WDT_FP32,
                 Policy(),
                 Alias::RepeatedMultiplicands,
                 {{{{0xbd06550c, 0xbcfe550b}, 0xfe000000, 0xbc000000},
                   {{0xbd06550c, 0xbcfe550b}, 0xfe000000, 0xbc000000},
                   plain(fixed_window(0xba800000)),
                   {{0x80000031, 0x0000002f}, 0x7fffffc0, 0x00000000}}},
                 {{fixed_window(0xbd000000), fixed_window(0xbd000000),
                   fixed_window(0xba800000), fixed_window(0)}},
                 {{0xbd000000, 0xbd000000, 0xba800000}},
                 8},
                // x*x + c with a range of c far wider than the result cell:
                // projecting x and c onto each other would search the lattice
                // points of x*x + c one encoding per pass
                {WDT_FP32,
                 WDT_FP32,
                 Policy(),
                 Alias::RepeatedMultiplicands,
                 {{{{0xc9fa344a, 0xc9ba3447}, 0xff800000, 0xc9800000},
                   {{0xc9fa344a, 0xc9ba3447}, 0xff800000, 0xc9800000},
                   {{0xd46e8249, 0xd46a824a}, 0xfff80000, 0xd4680000},
                   plain(fixed_window(0x462b6410))}},
                 {{{0xc9fa344a, 0xc9eff0ee}, {0xc9fa344a, 0xc9eff0ee},
                   {0xd46e8249, 0xd46a824a}, fixed_window(0x462b6410)}},
                 {{0xc9f5b921, 0xc9f5b921, 0xd46bdbdf}},
                 8},
                // x*x + x: projecting x*(x + 1) with an independent second x
                // alone is weaker than projecting a*b + c
                {WDT_FP32,
                 WDT_FP32,
                 up,
                 Alias::RepeatedInputs,
                 {{plain(square), plain(square), plain(square),
                   plain({0x4a0fad94, 0x4a0fadac})}},
                 {{{0x41f49229, 0x4791a0f2}, {0x41f49229, 0x4791a0f2},
                   {0x41f49229, 0x4791a0f2}, {0x4a0fad94, 0x4a0fadac}}},
                 {{0x44bfb906, 0x44bfb906, 0x44bfb906},
                  {0x44bfb916, 0x44bfb916, 0x44bfb916}},
                 8},
                {WDT_FP32,
                 WDT_FP32,
                 Policy(),
                 Alias::RepeatedInputs,
                 {{plain(large_square), plain(large_square),
                   plain(large_square), plain({0x52c65685, 0x52c65686})}},
                 {{{0x4563fb5a, 0x4cdea6e7}, {0x4563fb5a, 0x4cdea6e7},
                   {0x4563fb5a, 0x4cdea6e7}, {0x52c65685, 0x52c65686}}},
                 {{0x491f556c, 0x491f556c, 0x491f556c}},
                 8}};
            bool passed = true;
            for (const Case& c : cases) {
              const Reference ref(c.source, c.destination);
              FmaSpace s(ref);
              configure(s, ref, c.windows, c.alias);
              s.post(c.policy, false);
              const Settled settled = settle(s);
              bool tight = settled.status != SS_FAILED;
              for (int i = 0; tight && i < 4; ++i)
                for (WordValue bound : {s.x[i].minimum(), s.x[i].maximum()})
                  tight = tight && float_order_contains(column_width(ref, i),
                                                        c.hull[i].minimum,
                                                        c.hull[i].maximum,
                                                        bound);
              Windows windows;
              for (int i = 0; i < 4; ++i)
                windows[i] = c.windows[i].window;
              if (!tight || settled.propagations > c.bound ||
                  !keeps(ref, c.policy, s, windows, c.supports)) {
                olog << ind(2) << "FMA strength " << ref.source.type << '/'
                          << ref.destination.type << ' '
                          << static_cast<int>(c.alias) << ' '
                          << settled.propagations << '\n';
                passed = false;
              }
            }
            return passed;
          }
        };

        // Counts in runs how often the distinct views of four columns change
        // and fails the space after limit changes, so that a posting whose
        // bounds walk fails fast instead of running for minutes. It is
        // cheaper than fp_fma, so it runs after each execution that narrows.
        class Watchdog : public Propagator {
          Gecode::Word::WordView views[4];
          int n;
          unsigned long long int limit;
          unsigned long long int* runs;
          Watchdog(Space& home, Watchdog& other)
              : Propagator(home, other), n(other.n), limit(other.limit),
                runs(other.runs) {
            for (int i = 0; i < n; ++i)
              views[i].update(home, other.views[i]);
          }

        public:
          Watchdog(Home home, const WordVar* x, unsigned long long int l,
                   unsigned long long int* r)
              : Propagator(home), n(0), limit(l), runs(r) {
            for (int i = 0; i < 4; ++i) {
              const Gecode::Word::WordView view(x[i]);
              bool repeated = false;
              for (int j = 0; j < n; ++j)
                repeated = repeated || views[j] == view;
              if (repeated)
                continue;
              views[n] = view;
              views[n].subscribe(home, *this, Gecode::Word::PC_WORD_DOM);
              ++n;
            }
          }
          Actor* copy(Space& home) override {
            return new (home) Watchdog(home, *this);
          }
          size_t dispose(Space& home) override {
            for (int i = 0; i < n; ++i)
              views[i].cancel(home, *this, Gecode::Word::PC_WORD_DOM);
            (void)Propagator::dispose(home);
            return sizeof(*this);
          }
          PropCost cost(const Space&, const ModEventDelta&) const override {
            return PropCost::unary(PropCost::LO);
          }
          void reschedule(Space& home) override {
            for (int i = 0; i < n; ++i)
              views[i].reschedule(home, *this, Gecode::Word::PC_WORD_DOM);
          }
          ExecStatus propagate(Space&, const ModEventDelta&) override {
            return ++*runs > limit ? ES_FAILED : ES_FIX;
          }
        };


        /// The bounds and known bits of a column at a fixpoint
        struct Pinned {
          WordValue minimum, maximum, lo, hi;
        };
        // Whether s, settled with status, has not failed and its columns hold
        // fixpoint (four columns, or none to check); otherwise print the
        // domains they hold, in the form of a pin.
        bool at_fixpoint(FmaSpace& s, SpaceStatus status,
                         const std::vector<Pinned>& fixpoint) {
          if (status == SS_FAILED)
            return false;
          bool pinned = true;
          for (std::size_t i = 0; i < fixpoint.size(); ++i)
            pinned = pinned && s.x[i].minimum() == fixpoint[i].minimum &&
                     s.x[i].maximum() == fixpoint[i].maximum &&
                     s.x[i].lo() == fixpoint[i].lo &&
                     s.x[i].hi() == fixpoint[i].hi;
          if (!pinned)
            for (int i = 0; i < 4; ++i)
              olog << ind(2) << std::hex << "  {0x" << s.x[i].minimum() << ", 0x"
                        << s.x[i].maximum() << ", 0x" << s.x[i].lo() << ", 0x"
                        << s.x[i].hi() << "}\n"
                        << std::dec;
          return pinned;
        }

        /// Whether a fresh propagator on a clone of s, at a fixpoint, finds
        /// nothing to prune
        bool unchanged_by_fresh(FmaSpace& s, const Policy& p) {
          std::unique_ptr<FmaSpace> clone(static_cast<FmaSpace*>(s.clone()));
          clone->post(p, false);
          if (clone->status() == SS_FAILED)
            return false;
          for (int i = 0; i < 4; ++i)
            if (clone->x[i].minimum() != s.x[i].minimum() ||
                clone->x[i].maximum() != s.x[i].maximum() ||
                clone->x[i].lo() != s.x[i].lo() ||
                clone->x[i].hi() != s.x[i].hi())
              return false;
          return true;
        }

        // Postings that must settle within a bound of executions, keep the
        // given supports, and be unchanged by a fresh propagator on a clone
        // of the fixpoint, which a dishonest ES_FIX would leave narrowable.
        // Where the bounds of the fixpoint are the point of a posting, they
        // are pinned as well.
        class PinnedPostings : public Base {
        protected:
          struct Case {
            const char* name;
            WordDomainType format;
            Policy policy;
            Alias alias;
            BitWindows windows;
            std::vector<Support> supports;
            /// The fixpoint, or empty when its bounds are not the point
            std::vector<Pinned> fixpoint = {};
          };
          /// p with the zeros given and tininess detected before rounding
          static Policy before(Policy p, Policy::Zeros zeros) {
            p.zeros = zeros;
            p.tininess = Policy::BeforeRounding;
            return p;
          }
          // Whether every case passes. The check stops at the first case
          // that fails: the watchdog stops bounds that walk over executions,
          // but not a walk inside one execution, so a later case could
          // otherwise run for minutes after the failure is known.
          bool check(const Case* cases, std::size_t count, const char* label,
                     unsigned long long int bound = 1) {
            for (std::size_t n = 0; n < count; ++n) {
              const Case& c = cases[n];
              const Reference ref(c.format, c.format);
              FmaSpace s(ref);
              configure(s, ref, c.windows, c.alias);
              s.post(c.policy, false);
              unsigned long long int changes = 0;
              (void)new (s) Watchdog(s, s.x, 2 * bound, &changes);
              Settled settled = settle(s);
              settled.propagations -= changes;
              Windows windows;
              for (int i = 0; i < 4; ++i)
                windows[i] = c.windows[i].window;
              if (!at_fixpoint(s, settled.status, c.fixpoint) ||
                  settled.propagations > bound ||
                  !keeps(ref, c.policy, s, windows, c.supports) ||
                  !unchanged_by_fresh(s, c.policy)) {
                olog << ind(2) << label << " " << c.name << ": "
                          << settled.propagations << " executions\n";
                return false;
              }
            }
            return true;
          }

        public:
          PinnedPostings(const char* name) : Base(name) {}
        };

        class AliasedKnownBits : public PinnedPostings {
        public:
          AliasedKnownBits()
              : PinnedPostings("Word::Float::Fma::Wide::AliasedKnownBits") {}
          bool run() override {
            // FP32 postings of x*(y + 1) and x*(x + 1) with known bits on x
            // and on the result, and stretches of x without support. The
            // known bits act on each word alone, so such a stretch stays in
            // the domain: the fixpoint is the bounds of the real relaxation,
            // pinned here, reached in one execution. Projections that use
            // the known bits pass by pass moved the bounds of x a few codes
            // per execution, for 107 to 13,986 executions.
            const Policy up{Policy::RUP, Policy::Preserve, Policy::SignedZero,
                            Policy::SaturateFinite, Policy::Positive};
            // a*(b + 1) with b + 1 = 1 + 2^-22 or so: the code of the result
            // is the code of a plus 1 below |a| = 4.146 and plus 2 above it,
            // a is even and the result 3 modulo 4
            const BitWindow a{{0xc09cf554, 0xbf01e456}, 1, 0};
            const BitWindow b{{0x3476d005, 0x3477c33e}, 0x80000000, 0};
            // A subnormal a times 1 - 5.75e-5, a 1 and the result 0 modulo 4
            const BitWindow tiny{{0x003281f6, 0x006f041c}, 3, 1};
            const BitWindow near_one{{0xb8714baf, 0xb87148af}, 3, 2};
            // b*(a + 1) with a + 1 = 0.4999999, b and the result even
            const BitWindow even_b{{0xca588582, 0xb724af59}, 1, 0};
            // x*(x + 1) is about x - x^2 for tiny negative x; x and the result
            // odd
            const BitWindow odd_x{{0xb459ed0b, 0x5e5d0ea1}, 1, 1};
            const Case cases[] = {
                {"a == c, factor near 1", WDT_FP32, up, Alias::RepeatedAddend,
                 {{a, b, a, {{0xc20ab6be, 0xbea31904}, 3, 3}}},
                 {{0xc084c3c2, 0x3476d005, 0xc084c3c2}, {0xbf01e456, 0x3476d005, 0xbf01e456}},
                 {{0xc09cf554, 0xbf01e456, 0x80000000, 0xfffffffe}, {0x3476d005, 0x3477c33e, 0x34760000, 0x3477ffff},
                  {0xc09cf554, 0xbf01e456, 0x80000000, 0xfffffffe}, {0xc09cf557, 0xbf01e457, 0x80000003, 0xffffffff}}},
                {"a == c, subnormal x", WDT_FP32, Policy{Policy::RDN}, Alias::RepeatedAddend,
                 {{tiny, near_one, tiny, {{0x8051d543, 0x0093fb08}, 3, 0}}},
                 {{0x0032ecd5, 0xb8714bae, 0x0032ecd5}, {0x006e9bb5, 0xb87148b6, 0x006e9bb5}},
                 {{0x003281f9, 0x006f0419, 0x00000001, 0x007ffffd}, {0xb8714bae, 0xb87148b2, 0xb8714802, 0xb8714bfe},
                  {0x003281f9, 0x006f0419, 0x00000001, 0x007ffffd}, {0x00328138, 0x006f0278, 0x00000000, 0x007ffffc}}},
                {"b == c, factor near 1/2", WDT_FP32, Policy{Policy::RTZ}, Alias::RepeatedSecondAddend,
                 {{plain(fixed_window(0xbf000002)), even_b, even_b, {{0xc26483e6, 0xaf93d930}, 1, 0}}},
                 {{0xbf000002, 0xc2e483ea, 0xc2e483ea}, {0xbf000002, 0xb7400002, 0xb7400002}},
                 {{0xbf000002, 0xbf000002, 0xbf000002, 0xbf000002}, {0xc2e483ea, 0xb724af5a, 0x80000000, 0xfffffffe},
                  {0xc2e483ea, 0xb724af5a, 0x80000000, 0xfffffffe}, {0xc26483e6, 0xb6a4af56, 0x80000000, 0xfffffffe}}},
                {"a == b == c, tiny x", WDT_FP32, Policy{Policy::RNE, Policy::Preserve, Policy::SignedZero},
                 Alias::RepeatedInputs, {{odd_x, odd_x, odd_x, {{0xe389dceb, 0x1addf81d}, 1, 1}}},
                 {{0xb44a62c1, 0xb44a62c1, 0xb44a62c1}, {0x1addf81d, 0x1addf81d, 0x1addf81d}},
                 {{0xb459ed0b, 0x1addf81d, 0x00000001, 0xbfffffff}, {0xb459ed0b, 0x1addf81d, 0x00000001, 0xbfffffff},
                  {0xb459ed0b, 0x1addf81d, 0x00000001, 0xbfffffff}, {0xb459ed07, 0x1addf81d, 0x00000001, 0xbfffffff}}}};
            return check(cases, std::size(cases), "FMA known bits");
          }
        };

        class AliasedBounds : public PinnedPostings {
        public:
          AliasedBounds()
              : PinnedPostings("Word::Float::Fma::Wide::AliasedBounds") {}
          bool run() override {
            // Postings of the aliased shapes, one for each way the closed form
            // can lose a support, leave a fixpoint unfinished or walk: how it
            // reads each word, the cell of the largest finite value, the cell
            // of the zeros, the factor y + 1 at zero, the a == b path, and
            // postings whose bounds walked for thousands to millions of
            // executions in the code before. Each settles in one execution;
            // the bounds are pinned where they are the point. Tests run in
            // name order, so this one runs before the enumerations: with a
            // read that drops a code, some of those walk inside one
            // execution, which no watchdog can stop.
            const Policy rup = Policy{Policy::RUP};
            const Policy rdn = Policy{Policy::RDN};
            const Policy rtz = Policy{Policy::RTZ};
            const BitWindow finite = plain({0xff7fffff, 0x7f7fffff});
            const BitWindow x_bits{{0x15800000, 0x27ffffff}, 0x08000003, 0};
            const BitWindow square_x{{0xf230445f, 0x9185dab2}, 7, 0};
            const BitWindow high_bit{finite.window, 0x10000000, 0x10000000};
            const BitWindow small_x{{0x0026e41d, 0x0027a41d}, 0x00001fff, 0x0000041d};
            const BitWindow near_one{{0x3ef0, 0x3f03}, 0xfe00, 0x3e00};
            const Case cases[] = {
                // x*x + c with the result even, where every x of the top 4,096
                // codes of its binade gives an odd result. The widened read of
                // the result keeps x up to 0x5f7ffffe; projected by their least
                // and greatest codes, x and the result moved down one code per
                // execution, for 65 executions.
                {"x*x + c, even result, top codes of x give odd results", WDT_FP32,
                 Policy{Policy::RUP, Policy::Preserve, Policy::Preserve, Policy::SaturateFinite},
                 Alias::RepeatedMultiplicands,
                 {{high_bit, high_bit, plain(fixed_window(0xbff7a8d6)), {{0xff800000, 0x7f800000}, 1, 0}}},
                 {{0x5f7fefff, 0x5f7fefff, 0xbff7a8d6}, {0xdf7fefff, 0xdf7fefff, 0xbff7a8d6},
                  {0x3a000000, 0x3a000000, 0xbff7a8d6}},
                 {{0xdf7ffffe, 0x5f7ffffe, 0x10000000, 0xffffffff}, {0xdf7ffffe, 0x5f7ffffe, 0x10000000, 0xffffffff},
                  {0xbff7a8d6, 0xbff7a8d6, 0xbff7a8d6, 0xbff7a8d6}, {0xbff7a8d4, 0x7f7ffffc, 0x00000000, 0xfffffffe}}},
                // The widened read of the result spans every code strictly
                // between pred and succ. y + 1 is close to 1, so each code of
                // x has a code of the result of its own.
                {"widened read of the result between pred and succ", WDT_BF16,
                 Policy{Policy::RNE, Policy::Preserve, Policy::SignedZero},
                 Alias::RepeatedAddend,
                 {{{{0x037e, 0x04e8}, 1, 0}, plain({0x3a61, 0x3a7b}), {{0x037e, 0x04e8}, 1, 0},
                   {{0x0481, 0x0492}, 0x80, 0x80}}},
                 {{0x0484, 0x3a70, 0x0484}},
                 {{0x0482, 0x0492, 0x0480, 0x049e}, {0x3a61, 0x3a7b, 0x3a60, 0x3a7f},
                  {0x0482, 0x0492, 0x0480, 0x049e}, {0x0482, 0x0492, 0x0480, 0x049e}}},
                // The hull read: the result admits every 64th code and is read
                // from its least to its greatest code, not 64 codes beyond.
                {"hull read of a result admitting every 64th code", WDT_BF16,
                 before(Policy{Policy::RTZ, Policy::SignedZero}, Policy::Positive),
                 Alias::RepeatedSecondAddend,
                 {{{{0x8001, 0x8000}, 3, 0}, plain({0xff7f, 0x7f7f}), plain({0xff7f, 0x7f7f}),
                   {{0xd9c5, 0xb84d}, 0x3f, 0}}},
                 {{0x8000, 0xbf80, 0xbf80}},
                 {{0x8000, 0x8000, 0x8000, 0x8000}, {0xd9c0, 0xb880, 0x8000, 0xffff},
                  {0xd9c0, 0xb880, 0x8000, 0xffff}, {0xd9c0, 0xb880, 0x8000, 0xffc0}}},
                // The widened read searches a word without the bits that the
                // closure of its interval fixes, which takes the common prefix
                // of each sign half: x spans both signs.
                {"widened read of x across both signs", WDT_BF16, rup, Alias::RepeatedSecondAddend,
                 {{plain({0x4031, 0x4032}), {{0xff7f, 0x4e1a}, 0x288, 0}, {{0xff7f, 0x4e1a}, 0x288, 0},
                   {{0x803e, 0x0020}, 2, 0}}},
                 {{0x4032, 0x0000, 0x0000}},
                 {{0x4031, 0x4032, 0x4030, 0x4033}, {0x8010, 0x0004, 0x0000, 0x8017},
                  {0x8010, 0x0004, 0x0000, 0x8017}, {0x803c, 0x0010, 0x0000, 0x803c}}},
                // Each word is read by its own known bits: y must not be read
                // by those of the assigned x.
                {"each word read by its own known bits", WDT_BF16,
                 before(Policy{Policy::RUP, Policy::SignedZero, Policy::Preserve, Policy::SaturateFinite},
                        Policy::Signed),
                 Alias::RepeatedSecondAddend,
                 {{plain({0xc21b, 0xba10}), plain(fixed_window(0x8174)), plain(fixed_window(0x8174)),
                   {{0xff80, 0x7f80}, 0x7ff, 7}}},
                 {{0xbf7c, 0x8174, 0x8174}},
                 {{0xbf7c, 0xbf7c, 0xbf7c, 0xbf7c}, {0x8174, 0x8174, 0x8174, 0x8174},
                  {0x8174, 0x8174, 0x8174, 0x8174}, {0x8007, 0x8007, 0x8007, 0x8007}}},
                // The widened read of a multiplicand takes as pred the first
                // admitted code below its least code, here the next code.
                {"pred of a multiplicand is the next admitted code", WDT_BF16,
                 Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero},
                 Alias::RepeatedSecondAddend,
                 {{plain({0x807f, 0x8079}), {{0x82f9, 0x0297}, 0x4100, 0}, {{0x82f9, 0x0297}, 0x4100, 0},
                   {{0x8430, 0x806e}, 7, 2}}},
                 {{0x807c, 0x80f3, 0x80f3}},
                 {{0x807f, 0x8079, 0x8078, 0x807f}, {0x82f3, 0x8083, 0x8000, 0x82ff},
                  {0x82f3, 0x8083, 0x8000, 0x82ff}, {0x82f2, 0x8082, 0x8002, 0x83fa}}},
                // x*1.5 with known bits in the exponent and the low bits of x
                // and the result; the least supported code of x is 0x23aaaaac.
                {"x*1.5 with known exponent bits", WDT_FP32, Policy(), Alias::RepeatedAddend,
                 {{x_bits, plain(fixed_window(0x3f000000)), x_bits,
                   {{0x00000000, 0x7f7ffffe}, 0x06000001, 0x04000001}}},
                 {{0x23aaaaac, 0x3f000000, 0x23aaaaac}, {0x257ffffc, 0x3f000000, 0x257ffffc}},
                 {{0x15800000, 0x27fffffc, 0x00000000, 0x37fffffc}, {0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000},
                  {0x15800000, 0x27fffffc, 0x00000000, 0x37fffffc}, {0x15bffffd, 0x2dffffff, 0x04000001, 0x3dffffff}}},
                // The cell of the largest finite value. Under RDN every sum
                // from FLT_MAX up rounds to FLT_MAX, which the known bits of
                // the result exclude, so x stays below about 2.
                {"RDN cell of the largest finite value", WDT_FP32, rdn, Alias::RepeatedAddend,
                 {{{{0xff7fffff, 0x7f7fffff}, 7, 4}, plain({0x7effffff, 0x7f000004}), {{0xff7fffff, 0x7f7fffff}, 7, 4},
                   {{0xc9576f44, 0x7f800000}, 2, 0}}},
                 {{0x3f000004, 0x7f000000, 0x3f000004}},
                 {{0x89d76f44, 0x3ffffffc, 0x00000004, 0xbffffffc}, {0x7effffff, 0x7f000004, 0x7e000000, 0x7fffffff},
                  {0x89d76f44, 0x3ffffffc, 0x00000004, 0xbffffffc}, {0xc9576f44, 0x7f7ffffd, 0x00000000, 0xfffffffd}}},
                // The finite end of that cell in the widened read. Under RUP
                // every sum below -FLT_MAX rounds to -FLT_MAX, which the known
                // bits of the result exclude: x stays above about -4.78.
                {"RUP finite end of the largest finite cell", WDT_FP32, rup, Alias::RepeatedAddend,
                 {{{{0xff7fffff, 0x7f7fffff}, 2, 0}, plain(fixed_window(0x7e563a1a)), {{0xff7fffff, 0x7f7fffff}, 2, 0},
                   {{0xff800000, 0x7f800000}, 0x40000001, 0x40000000}}},
                 {{0x81d87319, 0x7e563a1a, 0x81d87319}},
                 {{0xc098f591, 0x7f7ffffd, 0x00000000, 0xfffffffd}, {0x7e563a1a, 0x7e563a1a, 0x7e563a1a, 0x7e563a1a},
                  {0xc098f591, 0x7f7ffffd, 0x00000000, 0xfffffffd}, {0xff7ffffc, 0x7f800000, 0x40000000, 0xfffffffe}}},
                // The same under saturation toward zero: |x| stays below 256.
                {"RTZ saturating cell of the largest finite value", WDT_FP32,
                 before(Policy{Policy::RTZ, Policy::SignedZero, Policy::SignedZero, Policy::SaturateFinite},
                        Policy::Positive),
                 Alias::RepeatedAddend,
                 {{{{0xff7fffff, 0x7f7fffff}, 0x800000, 0}, {{0xfb2f41b5, 0xfb2f419c}, 7, 2},
                   {{0xff7fffff, 0x7f7fffff}, 0x800000, 0}, {{0xff800000, 0x7f800000}, 0x10414080, 0x10014080}}},
                 {{0xbf7fffff, 0xfb2f41aa, 0xbf7fffff}},
                 {{0xc37fffff, 0x437fffff, 0x00000000, 0xff7fffff}, {0xfb2f41b2, 0xfb2f41a2, 0xfb2f41a2, 0xfb2f41ba},
                  {0xc37fffff, 0x437fffff, 0x00000000, 0xff7fffff}, {0xff2f41b1, 0x7f2f41b1, 0x10014080, 0xffbfffff}}},
                // Under RNE the tie below the largest finite value rounds to
                // the even code below it: the greatest sum is that tie, so the
                // result keeps no largest finite value.
                {"tie below the largest finite value", WDT_BF16, Policy(), Alias::RepeatedAddend,
                 {{plain({0x7000, 0x7f00}), plain({0x3f00, 0x3f7d}), plain({0x7000, 0x7f00}), plain({0x7000, 0x7f7f})}},
                 {{0x7f00, 0x3f7d, 0x7f00}},
                 {{0x7000, 0x7f00, 0x7000, 0x7fff}, {0x3f00, 0x3f7d, 0x3f00, 0x3f7f},
                  {0x7000, 0x7f00, 0x7000, 0x7fff}, {0x7040, 0x7f7e, 0x7000, 0x7fff}}},
                // A sum within half an ulp above FLT_MAX rounds to it under
                // RNE: FLT_MAX * (1 + 2^-30).
                {"FLT_MAX below the overflow threshold", WDT_FP32, Policy(), Alias::RepeatedAddend,
                 {{plain({0x7f7ff000, 0x7f7fffff}), plain({0x30800000, 0x30800002}), plain({0x7f7ff000, 0x7f7fffff}),
                   plain(fixed_window(0x7f7fffff))}},
                 {{0x7f7fffff, 0x30800000, 0x7f7fffff}, {0x7f7fffff, 0x30800002, 0x7f7fffff}}},
                // The cell of the zeros ends where the cells of the least
                // subnormals begin. Here y + 1 = 2^-24 and the result a zero:
                // under RNE a sum rounds to zero up to half the least
                // subnormal, the tie included, so x stays below the least
                // normal.
                {"RNE cell of the zeros, factor 2^-24", WDT_FP32,
                 before(Policy{Policy::RNE}, Policy::Positive), Alias::RepeatedAddend,
                 {{{{0xff7fffff, 0x7f7fffff}, 0xfff, 0xc0f}, plain(fixed_window(0xbf7fffff)),
                   {{0xff7fffff, 0x7f7fffff}, 0xfff, 0xc0f}, {{0xff800000, 0x7f800000}, 0x88080100, 0x80000000}}},
                 {{0xb255dc0f, 0xbf7fffff, 0xb255dc0f}},
                 {{0xff7ffc0f, 0x007ffc0f, 0x00000c0f, 0xfffffc0f}, {0xbf7fffff, 0xbf7fffff, 0xbf7fffff, 0xbf7fffff},
                  {0xff7ffc0f, 0x007ffc0f, 0x00000c0f, 0xfffffc0f}, {0xf377feff, 0x80000000, 0x80000000, 0xf7f7feff}}},
                // Under output flushing the result is read as the cells of its
                // zeros, not up to the least normals beyond the flushed
                // subnormals, so x keeps only the subnormals and zeros.
                {"cells of the zeros under output flushing", WDT_FP32,
                 Policy{Policy::RTZ, Policy::SignedZero, Policy::SignedZero, Policy::SaturateFinite},
                 Alias::RepeatedAddend,
                 {{finite, plain({0x80000061, 0x00000015}), finite, plain({0x80043624, 0x0015d490})}},
                 {{0x80000000, 0x00000000, 0x80000000}},
                 {{0x807fffff, 0x007fffff, 0x00000000, 0x807fffff}, {0x80000061, 0x00000015, 0x00000000, 0x8000007f},
                  {0x807fffff, 0x007fffff, 0x00000000, 0x807fffff}, {0x80000000, 0x00000000, 0x00000000, 0x80000000}}},
                // The cell of +0 under RDN ends below the least positive
                // subnormal: x keeps no negative normal.
                {"RDN cell of +0 below the least subnormal", WDT_BF16, rdn, Alias::RepeatedAddend,
                 {{{{0xff7f, 0x7f7f}, 0x2000, 0}, plain({0xbf7f, 0xbf7d}), {{0xff7f, 0x7f7f}, 0x2000, 0},
                   {{0x80d3, 0x09c2}, 0x9010, 0}}},
                 {{0x005c, 0xbf7e, 0x005c}},
                 {{0x8000, 0x0dc2, 0x0000, 0x8fff}, {0xbf7f, 0xbf7d, 0xbf7c, 0xbf7f},
                  {0x8000, 0x0dc2, 0x0000, 0x8fff}, {0x0000, 0x09c2, 0x0000, 0x0fef}}},
                // y = -1 makes the factor y + 1 zero: every x has support.
                {"x*(y + 1) at y = -1", WDT_FP32, Policy(), Alias::RepeatedSecondAddend,
                 {{plain(fixed_window(0xbf800000)), plain({0x947f0000, 0x84800000}), plain({0x947f0000, 0x84800000}),
                   {{0x00000000, 0x00000024}, 0x1020, 0}}},
                 {{0xbf800000, 0x947f0000, 0x947f0000}, {0xbf800000, 0x84800000, 0x84800000}}},
                // Under output flushing the results begin at the least normal,
                // which x*b + b reaches with b = 9.
                {"flushed results from the least normal", WDT_BF16,
                 Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero, Policy::SaturateFinite},
                 Alias::RepeatedSecondAddend,
                 {{{{0xc181, 0xc172}, 0xff00, 0xc100}, {{0x0004, 0x00e1}, 0xff00, 0}, {{0x0004, 0x00e1}, 0xff00, 0},
                   {{0x819a, 0x8068}, 0xfe00, 0x8000}}},
                 {{0xc175, 0x0009, 0x0009}}},
                // The successor of the result interval is the next admitted
                // code, so the result ends where the cells of x end.
                {"result successor is the next admitted code", WDT_BF16, Policy(), Alias::RepeatedAddend,
                 {{near_one, {{0x8169, 0x0050}, 0x7e00, 0}, near_one, {{0x3ecb, 0x3f09}, 0xfe01, 0x3e01}}},
                 {{0x3f03, 0x8169, 0x3f03}, {0x3ef1, 0x0050, 0x3ef1}},
                 {{0x3ef0, 0x3f03, 0x3e00, 0x3fff}, {0x8169, 0x0050, 0x0000, 0x81ff},
                  {0x3ef0, 0x3f03, 0x3e00, 0x3fff}, {0x3eef, 0x3f03, 0x3e01, 0x3fff}}},
                // The positive part of x = b starts at its least positive
                // value and the negative part ends at its least negative one:
                // b = 0x0001 and b = 0x8001 give results in the window.
                {"x = b from its least magnitude of each sign", WDT_BF16,
                 Policy{Policy::RDN, Policy::Preserve, Policy::SignedZero},
                 Alias::RepeatedSecondAddend,
                 {{plain({0xff7f, 0x7f7f}), plain({0xff7f, 0x7f7f}), plain({0xff7f, 0x7f7f}),
                   {{0x8f12, 0x8e39}, 0xfe00, 0x8e00}}},
                 {{0xd192, 0x0001, 0x0001}, {0xd143, 0x0001, 0x0001}, {0x5191, 0x8001, 0x8001}}},
                // Postings whose bounds walked in the code before: a == c for
                // 3,144,768 executions, b == c for 279, b == c in BF16 under
                // output flushing for over 5 million, and a == b == c.
                {"a == c, x near -1/2, free y", WDT_FP32, rtz, Alias::RepeatedAddend,
                 {{{{0xbf000003, 0xbeffffff}, 7, 0}, {{0xff7fffff, 0x7f7fffff}, 0x80000001, 0x80000000},
                   {{0xbf000003, 0xbeffffff}, 7, 0}, {{0xc1acbe8d, 0xb870af24}, 7, 7}}},
                 {{0xbf000000, 0x80346dce, 0xbf000000}}},
                {"b == c, x near -1/2, free y", WDT_FP32, Policy(), Alias::RepeatedSecondAddend,
                 {{{{0xbf000007, 0xbf000000}, 0x80000001, 0x80000000}, {{0xff7fffff, 0x7f7fffff}, 2, 0},
                   {{0xff7fffff, 0x7f7fffff}, 2, 0}, {{0xff800000, 0x7f800000}, 0x80000001, 0x80000001}}},
                 {{0xbf000002, 0xbf3ecf4c, 0xbf3ecf4c}}},
                {"b == c, BF16 output flushing", WDT_BF16, Policy{Policy::RNE, Policy::Preserve, Policy::SignedZero},
                 Alias::RepeatedSecondAddend,
                 {{{{0xff7f, 0x7f7f}, 0x40, 0x40}, {{0xff7f, 0x7f7f}, 0x10, 0}, {{0xff7f, 0x7f7f}, 0x10, 0},
                   plain({0x8042, 0x0029})}},
                 {{0xbf7c, 0x8083, 0x8083}}},
                {"a == b == c, negative x, odd result", WDT_FP32,
                 Policy{Policy::RNE, Policy::SignedZero, Policy::SignedZero, Policy::SaturateFinite},
                 Alias::RepeatedInputs, {{square_x, square_x, square_x, {{0xc868e0ce, 0xab2f4017}, 0x40000001, 1}}},
                 {{0xbf7fffd8, 0xbf7fffd8, 0xbf7fffd8}, {0xbeccba88, 0xbeccba88, 0xbeccba88},
                  {0xb33504f8, 0xb33504f8, 0xb33504f8}}},
                // x*(x + 1) rounds to x for the least subnormal x under RTZ:
                // the monotone run of x starts at its least code, the only
                // support of the least odd result.
                {"RTZ x*(x + 1) at the least subnormal", WDT_BF16, rtz, Alias::RepeatedInputs,
                 {{plain({0xff7f, 0x7f7f}), plain({0xff7f, 0x7f7f}), plain({0xff7f, 0x7f7f}),
                   {{0x0001, 0x7f7f}, 1, 1}}},
                 {{0x0001, 0x0001, 0x0001}}},
                // x*(b + 1) with b + 1 just below 2 over the negative
                // subnormals: a subnormal x gives the odd result 2u - 1 for
                // its code u, and an even one is wanted.
                {"a == c over the negative subnormals", WDT_FP32, rup, Alias::RepeatedAddend,
                 {{plain({0x80800003, 0x80000001}), plain(fixed_window(0x3f7ffffe)), plain({0x80800003, 0x80000001}),
                   {finite.window, 1, 0}}},
                 {{0x80800001, 0x3f7ffffe, 0x80800001}, {0x80800003, 0x3f7ffffe, 0x80800003}}},
                // x*x + c with c a range far wider than the result cell, and
                // x*x far below a unit of c: projecting c and the result by
                // their codes moved both one code per pass. The bounds of c
                // and the result are those of the real relaxation, which
                // reads c as a whole and with both reads.
                {"x*x + c with c far wider than the result cell", WDT_FP32,
                 before(Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero, Policy::SaturateFinite},
                        Policy::Positive),
                 Alias::RepeatedMultiplicands,
                 {{small_x, small_x, {{0xff7ffffe, 0x7f7ffffe}, 0x00000003, 0x00000002},
                   {finite.window, 0x00800001, 0x00000001}}},
                 {{0x0026e41d, 0x0026e41d, 0xff7ffffe}, {0x0027a41d, 0x0027a41d, 0xc0000002}},
                 {{0x0026e41d, 0x0027a41d, 0x0026041d, 0x0027e41d}, {0x0026e41d, 0x0027a41d, 0x0026041d, 0x0027e41d},
                  {0xff7ffffe, 0x7f7ffffe, 0x00000002, 0xfffffffe}, {0xff7ffffd, 0x7f7fffff, 0x00000001, 0xff7fffff}}},
                // x from the least subnormal on: every x*x + 1 rounds to 1.
                {"least subnormal square", WDT_BF16, Policy(), Alias::RepeatedMultiplicands,
                 {{plain({0x0001, 0x0802}), plain({0x0001, 0x0802}), plain(fixed_window(0x3f80)),
                   plain({0xff80, 0x7f80})}},
                 {{0x0001, 0x0001, 0x3f80}, {0x0802, 0x0802, 0x3f80}}}};
            return check(cases, std::size(cases), "FMA aliased bounds");
          }
        };

        class CancellationPostings : public PinnedPostings {
        public:
          CancellationPostings()
              : PinnedPostings("Word::Float::Fma::Wide::CancellationPostings") {
          }
          bool run() override {
            // Postings without an aliased addend where a*b + c cancels. With
            // the multiplicand b a single subnormal, the projections of a
            // and c onto each other moved the lower bound of a and the upper
            // bound of c by one encoding per execution, for 15,746
            // executions: x*b + c is linear in a and c, and the propagator
            // solves it in one execution. The other postings pin the
            // cancellation bound of ranges of both multiplicands, and the
            // fixpoint of the step rule where c and the result are assigned.
            const Case single[] = {
                // a*b + c = 0xf84fd9 * 2^-149 with b = 0x7ffffc * 2^-149:
                // a = -0xf84fd9 * 2^-2 and c = 0xf84fd9 * 2^-128 meet it.
                {"single subnormal b, linear in a and c", WDT_FP32,
                 Policy{Policy::RUP, Policy::Preserve, Policy::SignedZero, Policy::FormatDefault, Policy::Positive},
                 Alias::Independent,
                 {{plain({0xcb77cd25, 0x348832d9}), {{0x007ffffc, 0x00800000}, 3, 0}, plain({0x812063c4, 0x0edf9c3d}),
                   plain(fixed_window(0x00f84fd9))}},
                 {{0xca784fd9, 0x007ffffc, 0x0b784fd9}, {0x00000000, 0x00800000, 0x00f84fd9}},
                 {{0xca784fd9, 0x348832d9, 0x00000000, 0xffffffff}, {0x007ffffc, 0x00800000, 0x00000000, 0x00fffffc},
                  {0x00f84fd6, 0x0b784fd9, 0x00000000, 0x0fffffff}, {0x00f84fd9, 0x00f84fd9, 0x00f84fd9, 0x00f84fd9}}},
                // a*b + c = -4 * 2^-133 with a near -2^-125: c cancels a*b
                // down to a subnormal, and the cancellation bound narrows b
                // and c.
                {"RTZ cancellation to a subnormal near -2^-125", WDT_BF16, Policy{Policy::RTZ}, Alias::Independent,
                 {{plain({0x8128, 0x811e}), plain({0xd07b, 0xb681}), {{0xd9c4, 0x7f7f}, 0x4000, 0x0000},
                   {{0x8005, 0x8004}, 0x0081, 0x0000}}},
                 {{0x8122, 0xc016, 0x81bf}, {0x811e, 0xb946, 0x8005}},
                 {{0x8128, 0x811e, 0x8100, 0x813f}, {0xc407, 0xb681, 0x8000, 0xffff},
                  {0x85a7, 0x8005, 0x8000, 0x87ff}, {0x8004, 0x8004, 0x8004, 0x8004}}},
                // c and the result subnormal, b from -0 to a subnormal: the
                // granularity of the subnormal parts sets the cancellation
                // bound, and b = 0 gives the result c for every a.
                {"subnormal c and result, b from -0", WDT_BF16,
                 Policy{Policy::RTZ, Policy::Preserve, Policy::SignedZero},
                 Alias::Independent,
                 {{{{0xd190, 0x2e6f}, 1, 0}, plain({0x8000, 0x0004}), plain({0x00cb, 0x00db}),
                   plain(fixed_window(0x00cf))}},
                 {{0xd190, 0x8000, 0x00cf}, {0x2e6e, 0x8000, 0x00cf}, {0xc040, 0x0004, 0x00db},
                  {0xc140, 0x0001, 0x00db}},
                 {{0xd190, 0x2e6e, 0x0000, 0xfffe}, {0x8000, 0x0004, 0x0000, 0x8007},
                  {0x00cf, 0x00db, 0x00c0, 0x00df}, {0x00cf, 0x00cf, 0x00cf, 0x00cf}}},
                // a*b far smaller than the assigned c: under RDN every sum rounds to
                // the code below c. The cancellation bound on the addend must
                // include the greatest magnitude of the sum, or it excludes c
                // and with it every supported result.
                {"RDN a*b far below the assigned c", WDT_FP32,
                 before(Policy{Policy::RDN, Policy::Preserve, Policy::SignedZero}, Policy::Signed),
                 Alias::Independent,
                 {{{{0x0927f373, 0x0bae6ca3}, 0xfc000003, 0x08000003},
                   {{0x80756bcf, 0x80756bca}, 0xfffffff8, 0x80756bc8}, plain(fixed_window(0x8c94c250)),
                   {{0x8ca6e24b, 0x8c901d1d}, 0xffc00001, 0x8c800001}}},
                 {{0x0927f373, 0x80756bcf, 0x8c94c250}, {0x0bae6ca3, 0x80756bca, 0x8c94c250}}},
                // a*b + c with c and the result assigned: the step rule ends
                // at the fixpoint of the widened projection in one execution.
                // Repeated interval projections: a [0x3d09, 0x5409],
                // b [0x6876, 0x7f76], 2 executions.
                {"RDN c and the result assigned", WDT_BF16,
                 before(Policy{Policy::RDN, Policy::SignedZero, Policy::SignedZero}, Policy::Signed),
                 Alias::Independent,
                 {{{{0x226b, 0x543f}, 0x8000, 0}, {{0x621f, 0x7f7f}, 0xe000, 0x6000}, plain(fixed_window(0xfd03)),
                   plain(fixed_window(0x7926))}},
                 {{0x3e76, 0x7e09, 0xfd03}, {0x52f6, 0x6989, 0xfd03}},
                 {{0x3d05, 0x543f, 0x0000, 0x7fff}, {0x6831, 0x7f7f, 0x6000, 0x7fff},
                  {0xfd03, 0xfd03, 0xfd03, 0xfd03}, {0x7926, 0x7926, 0x7926, 0x7926}}}};
            return check(single, std::size(single), "FMA cancellation posting");
          }
        };

        class AliasedEnumeration : public Base {
          /// A uniform value below n
          WordValue draw(WordValue n) {
            return static_cast<WordValue>(_rand(static_cast<unsigned int>(n)));
          }

        public:
          AliasedEnumeration()
              : Base("Word::Float::Fma::Wide::AliasedEnumeration") {}
          // The rank of a BF16 code in the total order, and its inverse
          static WordValue order_rank(const FloatFormat& f, WordValue code) {
            return code >= f.sign ? f.sign - 1 - (code - f.sign)
                                  : code + f.sign;
          }
          static WordValue order_code(const FloatFormat& f, WordValue rank) {
            return rank >= f.sign ? rank - f.sign
                                  : f.sign + (f.sign - 1 - rank);
          }
          /// A finite BF16 code from the regions where the projections
          /// change their form
          WordValue magnitude_code(const FloatFormat& f) {
            const WordValue fraction = draw(f.normal);
            WordValue exponent = 0;
            switch (draw(8)) {
            case 0: exponent = 0; break;
            case 1: exponent = 1 + draw(3); break;
            case 2: exponent = 125 + draw(4); break;
            case 3: exponent = 230 + draw(25); break;
            case 4: exponent = 100 + draw(28); break;
            default: exponent = draw(255); break;
            }
            return (draw(2) ? f.sign : 0) | (exponent * f.normal + fraction);
          }
          /// The other factor y, often at or next to -1, so that y + 1 is at
          /// or near zero
          WordValue factor_code(const FloatFormat& f) {
            switch (draw(6)) {
            case 0: return f.sign | f.one;
            case 1: return (f.sign | f.one) + draw(5) - 2;
            case 2: return (f.sign | (126 * f.normal)) + draw(4);
            case 3: return draw(2) ? 0 : f.sign;
            default: return magnitude_code(f);
            }
          }
          WordValue bits_mask(const FloatFormat& f) {
            const WordValue masks[] = {0, 0, 0, 1, 3, 7, 2, f.normal,
                                       f.sign | 1, WordValue(0x4001)};
            (void)f;
            return masks[draw(10)];
          }
          /// A window of size codes in rank order around code, within the
          /// ranks low to high
          static Window around(const FloatFormat& f, WordValue code, WordValue size,
                               WordValue below, WordValue low,
                               WordValue high) {
            const WordValue rank = order_rank(f, code);
            WordValue first = rank - std::min(below, rank - low);
            WordValue last = std::min(high, first + size - 1);
            first = std::max(low, std::min(first, last + 1 - size));
            return {order_code(f, first), order_code(f, last)};
          }
          bool run() override {
            // BF16 postings of x*(y + 1) (a == c, b == c) and x*(x + 1)
            // (a == b == c) under random policies, with windows of x, y and
            // the result, known bits, and the factor at and next to -1. The
            // test enumerates every assignment of the inputs in their
            // windows with their known bits, evaluates the result with
            // Reference, and requires every row with a result in the result
            // window to be kept, the fixpoint to take one execution, and a
            // fresh propagator on a clone of it to change nothing. Most
            // postings have more than the 2,048 assignments that the exact
            // scan takes, so the closed form alone decides them.
            const Reference ref(WDT_BF16, WDT_BF16);
            const FloatFormat& f = ref.source;
            const WordValue finite_low = order_rank(f, f.sign | f.maximum);
            const WordValue finite_high = order_rank(f, f.maximum);
            const WordValue result_low = order_rank(f, f.sign | f.infinity);
            const WordValue result_high = order_rank(f, f.infinity);
            for (int n = 0; n < 150; ++n) {
              const Alias aliases[] = {Alias::RepeatedAddend,
                                       Alias::RepeatedSecondAddend,
                                       Alias::RepeatedInputs};
              const Alias alias = aliases[draw(3)];
              const bool square = alias == Alias::RepeatedInputs;
              Policy p;
              p.rounding = static_cast<Policy::Rounding>(draw(4));
              p.input_subnormals = static_cast<Policy::Subnormals>(draw(2));
              p.output_subnormals = static_cast<Policy::Subnormals>(draw(2));
              p.zeros = static_cast<Policy::Zeros>(draw(2));
              p.overflow = static_cast<Policy::Overflow>(draw(2));
              p.tininess = static_cast<Policy::Tininess>(draw(2));
              const WordValue x0 = magnitude_code(f);
              const WordValue y0 = square ? x0 : factor_code(f);
              // Sizes with up to 4,096 assignments, mostly over 2,048
              const WordValue ny = square ? 1 : 1 + draw(draw(2) ? 4 : 64);
              const WordValue nx =
                  square ? 1 + draw(4096) : (2049 + draw(2048)) / ny + 1;
              const Window xw =
                  around(f, x0, nx, draw(nx), finite_low, finite_high);
              const Window yw =
                  square ? xw
                         : around(f, y0, ny, draw(ny), finite_low,
                                  finite_high);
              const WordValue x_mask = bits_mask(f), y_mask = bits_mask(f);
              const BitWindow xb{xw, x_mask, x0 & x_mask};
              const BitWindow yb{yw, square ? 0 : y_mask,
                                 square ? 0 : y0 & y_mask};
              // The result window around the result of (x0, y0)
              const WordValue z0 =
                  alias == Alias::RepeatedAddend
                      ? ref.evaluate(p, x0, y0, x0).bits
                  : alias == Alias::RepeatedSecondAddend
                      ? ref.evaluate(p, y0, x0, x0).bits
                      : ref.evaluate(p, x0, x0, x0).bits;
              const WordValue sizes[] = {1, 2 + draw(8), 1 + draw(300),
                                         result_high - result_low + 1};
              const WordValue nz = sizes[draw(4)];
              const Window zw =
                  around(f, z0, nz, draw(nz), result_low, result_high);
              const WordValue z_mask = bits_mask(f);
              const BitWindow zb{zw, z_mask, z0 & z_mask};
              // The rows (x, y, result)
              std::vector<std::array<WordValue, 3>> rows;
              for (WordValue xr = order_rank(f, xw.minimum);
                   xr <= order_rank(f, xw.maximum); ++xr) {
                const WordValue x = order_code(f, xr);
                if ((x & xb.mask) != xb.value)
                  continue;
                const WordValue y_first = square ? xr : order_rank(f, yw.minimum);
                const WordValue y_last = square ? xr : order_rank(f, yw.maximum);
                for (WordValue yr = y_first; yr <= y_last; ++yr) {
                  const WordValue y = order_code(f, yr);
                  if ((y & yb.mask) != yb.value)
                    continue;
                  const WordValue z =
                      alias == Alias::RepeatedAddend
                          ? ref.evaluate(p, x, y, x).bits
                      : alias == Alias::RepeatedSecondAddend
                          ? ref.evaluate(p, y, x, x).bits
                          : ref.evaluate(p, x, x, x).bits;
                  if ((z & zb.mask) == zb.value &&
                      float_order_contains(16, zw.minimum, zw.maximum, z))
                    rows.push_back({x, y, z});
                }
              }
              const BitWindows windows =
                  alias == Alias::RepeatedAddend
                      ? BitWindows{{xb, yb, xb, zb}}
                  : alias == Alias::RepeatedSecondAddend
                      ? BitWindows{{yb, xb, xb, zb}}
                      : BitWindows{{xb, xb, xb, zb}};
              const int column = alias == Alias::RepeatedSecondAddend ? 1 : 0;
              FmaSpace s(ref);
              configure(s, ref, windows, alias);
              s.post(p, false);
              const Settled settled = settle(s);
              bool passed = settled.propagations <= 1 &&
                            (rows.empty() || settled.status != SS_FAILED);
              for (const auto& row : rows)
                if (passed)
                  passed = Gecode::Word::WordView(s.x[column]).in(row[0]) &&
                           Gecode::Word::WordView(s.x[square ? 0 : 1 - column])
                               .in(row[1]) &&
                           Gecode::Word::WordView(s.x[3]).in(row[2]);
              if (passed && settled.status != SS_FAILED)
                passed = unchanged_by_fresh(s, p);
              if (!passed) {
                olog << ind(2) << "FMA aliased enumeration " << n << ' '
                          << static_cast<int>(alias) << ' '
                          << settled.propagations << ' ' << rows.size()
                          << std::hex << " x " << xw.minimum << ".."
                          << xw.maximum << '/' << xb.mask << " y "
                          << yw.minimum << ".." << yw.maximum << '/'
                          << yb.mask << " r " << zw.minimum << ".."
                          << zw.maximum << '/' << zb.mask << std::dec
                          << " policy " << p.rounding << p.input_subnormals
                          << p.output_subnormals << p.zeros << p.overflow
                          << p.tininess << '\n';
                return false;
              }
            }
            return true;
          }
        };


        class AddendSquareEnumeration : public Base {
          /// A uniform value below n
          WordValue draw(WordValue n) {
            return static_cast<WordValue>(_rand(static_cast<unsigned int>(n)));
          }
          /// A finite code of f from the regions where x*x + c changes form:
          /// subnormal, squares that underflow, near 1, squares near the
          /// largest finite value of the result
          WordValue magnitude_code(const FloatFormat& f) {
            const WordValue fraction = draw(f.normal);
            WordValue exponent = 0;
            switch (draw(7)) {
            case 0: exponent = 0; break;
            case 1: exponent = 1 + draw(40); break;
            case 2: exponent = 125 + draw(4); break;
            case 3: exponent = 188 + draw(4); break;
            case 4: exponent = 230 + draw(25); break;
            default: exponent = draw(255); break;
            }
            return (draw(2) ? f.sign : 0) | (exponent * f.normal + fraction);
          }
          WordValue bits_mask(const FloatFormat& f) {
            const WordValue masks[] = {0,        0, 0, 1, 3, 2, f.normal,
                                       f.sign | 1, f.normal | 1};
            return masks[draw(9)];
          }

        public:
          AddendSquareEnumeration()
              : Base("Word::Float::Fma::Wide::AddendSquareEnumeration") {}
          bool run() override {
            // BF16 and mixed postings of x*x + c (a == b) under random
            // policies, with windows of x, c and the result, known bits, and
            // c near -x*x, zero, subnormal or near the largest finite value;
            // c is assigned in half of them. The test enumerates every x and
            // c in their windows with their known bits, evaluates the result
            // with Reference, and requires every row with a result in the
            // result window to be kept, the fixpoint to take one execution,
            // and a fresh propagator on a clone of it to change nothing.
            // Every x window has more than the 2,048 codes that the exact
            // scan takes, so the closed form alone decides the postings.
            using E = AliasedEnumeration;
            for (int n = 0; n < 30; ++n) {
              const Reference ref(WDT_BF16, n % 3 == 2 ? WDT_FP32 : WDT_BF16);
              const FloatFormat& f = ref.source;
              const FloatFormat& g = ref.destination;
              const unsigned int width = g.type == WDT_FP32 ? 32 : 16;
              Policy p;
              p.rounding = static_cast<Policy::Rounding>(draw(4));
              p.input_subnormals = static_cast<Policy::Subnormals>(draw(2));
              p.output_subnormals = static_cast<Policy::Subnormals>(draw(2));
              p.zeros = static_cast<Policy::Zeros>(draw(2));
              p.overflow = static_cast<Policy::Overflow>(draw(2));
              p.tininess = static_cast<Policy::Tininess>(draw(2));
              const WordValue x0 = magnitude_code(f);
              const WordValue square = ref.evaluate(p, x0, x0, 0).bits;
              WordValue c = 0;
              switch (draw(5)) {
              case 0:
                // Near -x*x, so that the sum cancels
                c = E::order_code(g, E::order_rank(g, square ^ g.sign) +
                                         draw(5) - 2);
                break;
              case 1: c = draw(2) ? 0 : g.sign; break;
              case 2: c = (draw(2) ? g.sign : 0) | draw(g.normal); break;
              case 3: c = (draw(2) ? g.sign : 0) | (g.maximum - draw(4)); break;
              default: c = magnitude_code(g); break;
              }
              if (c % g.sign >= g.infinity)
                c = g.maximum;
              const WordValue nc = draw(2) ? 1 : 2 + draw(8);
              const Window cw =
                  E::around(g, c, nc, draw(nc), E::order_rank(g, g.sign | g.maximum),
                            E::order_rank(g, g.maximum));
              const WordValue c_mask = nc == 1 || draw(2) ? 0 : bits_mask(g);
              const BitWindow cb{cw, c_mask, c & c_mask};
              const WordValue finite_low = E::order_rank(f, f.sign | f.maximum);
              const WordValue finite_high = E::order_rank(f, f.maximum);
              const WordValue nx = 2049 + draw(draw(4) ? 512 : 2048);
              const Window xw =
                  E::around(f, x0, nx, draw(nx), finite_low, finite_high);
              const WordValue x_mask = draw(3) ? 0 : bits_mask(f);
              const BitWindow xb{xw, x_mask, x0 & x_mask};
              const WordValue z0 = ref.evaluate(p, x0, x0, c).bits;
              const WordValue result_low = E::order_rank(g, g.sign | g.infinity);
              const WordValue result_high = E::order_rank(g, g.infinity);
              const WordValue sizes[] = {1, 2 + draw(8), 1 + draw(300),
                                         result_high - result_low + 1};
              const WordValue nz = sizes[draw(4)];
              const Window zw =
                  E::around(g, z0, nz, draw(nz), result_low, result_high);
              const WordValue z_mask = bits_mask(g);
              const BitWindow zb{zw, z_mask, z0 & z_mask};
              std::vector<std::array<WordValue, 3>> rows;
              for (WordValue cr = E::order_rank(g, cw.minimum);
                   cr <= E::order_rank(g, cw.maximum); ++cr) {
                const WordValue a = E::order_code(g, cr);
                if ((a & cb.mask) != cb.value)
                  continue;
                for (WordValue xr = E::order_rank(f, xw.minimum);
                     xr <= E::order_rank(f, xw.maximum); ++xr) {
                  const WordValue x = E::order_code(f, xr);
                  if ((x & xb.mask) != xb.value)
                    continue;
                  const WordValue z = ref.evaluate(p, x, x, a).bits;
                  if ((z & zb.mask) == zb.value &&
                      float_order_contains(width, zw.minimum, zw.maximum, z))
                    rows.push_back({x, a, z});
                }
              }
              FmaSpace s(ref);
              configure(s, ref, {{xb, xb, cb, zb}},
                        Alias::RepeatedMultiplicands);
              s.post(p, false);
              const Settled settled = settle(s);
              bool passed = settled.propagations <= 1 &&
                            (rows.empty() || settled.status != SS_FAILED);
              for (const auto& row : rows)
                if (passed)
                  passed = Gecode::Word::WordView(s.x[0]).in(row[0]) &&
                           Gecode::Word::WordView(s.x[2]).in(row[1]) &&
                           Gecode::Word::WordView(s.x[3]).in(row[2]);
              if (passed && settled.status != SS_FAILED)
                passed = unchanged_by_fresh(s, p);
              if (!passed) {
                olog << ind(2) << "FMA addend square enumeration " << n << ' '
                          << settled.propagations << ' ' << rows.size()
                          << std::hex << " x " << xw.minimum << ".."
                          << xw.maximum << '/' << xb.mask << " c "
                          << cw.minimum << ".." << cw.maximum << '/' << cb.mask
                          << " r " << zw.minimum << ".." << zw.maximum << '/'
                          << zb.mask << std::dec << " policy " << p.rounding
                          << p.input_subnormals << p.output_subnormals
                          << p.zeros << p.overflow << p.tininess << '\n';
                return false;
              }
            }
            return true;
          }
        };

        class AliasedPostings : public Base {
          struct Case {
            const char* name;
            Alias alias;
            Policy policy;
            BitWindow x, y, z;
          };
          using E = AliasedEnumeration;
          /// The supports of one column: the least and greatest code in the
          /// order, and the bits every support has and some support has
          struct Supports {
            bool any = false;
            WordValue least = 0, greatest = 0, all_bits = 0, some_bits = 0;
            void add(const FloatFormat& f, WordValue code) {
              if (!any) {
                any = true;
                least = greatest = all_bits = some_bits = code;
                return;
              }
              if (E::order_rank(f, code) < E::order_rank(f, least))
                least = code;
              if (E::order_rank(f, code) > E::order_rank(f, greatest))
                greatest = code;
              all_bits &= code;
              some_bits |= code;
            }
            /// Whether the domain of x holds every support. A BF16 domain is
            /// an interval in the order and a cube of known bits, so it holds
            /// them all if it holds the least and the greatest and its cube
            /// holds the bits they share and the bits they have.
            bool kept_by(WordVar x) const {
              return !any || (x.in(least) && x.in(greatest) && (all_bits & x.lo()) == x.lo() &&
                              (some_bits & ~x.hi()) == 0);
            }
          };
          /// The supports of the columns x, y and the result of each case,
          /// computed with Reference in the first iteration
          std::vector<std::array<Supports, 3>> supports;

        public:
          AliasedPostings() : Base("Word::Float::Fma::Wide::AliasedPostings") {}
          bool run() override {
            // BF16 postings of a*b + a, a*b + b and x*x + x with known bits
            // whose supports the fixpoint must keep, in one execution: each
            // lost a support under a mutant of the propagator. The test
            // enumerates every code of x in its window with its known bits
            // and every code of the other factor, evaluates the result with
            // Reference, and keeps the rows whose result lies in the result
            // window with its known bits. The rows do not change between
            // iterations, so the first one computes the supports of each
            // column and the others reuse them.
            static constexpr unsigned long long int bound = 1;
            const Reference ref(WDT_BF16, WDT_BF16);
            const FloatFormat& f = ref.source;
            const BitWindow none = plain(fixed_window(0));
            const Policy rne;
            const Policy rdn{Policy::RDN};
            const Policy rup{Policy::RUP};
            const Policy rne_flush_in{Policy::RNE, Policy::SignedZero};
            const Policy rne_flush{Policy::RNE, Policy::Preserve, Policy::SignedZero};
            const Policy rdn_flush{Policy::RDN, Policy::Preserve, Policy::SignedZero};
            const Policy rne_saturate{Policy::RNE, Policy::Preserve, Policy::Preserve, Policy::SaturateFinite};
            const Policy rup_flush_saturate{Policy::RUP, Policy::Preserve, Policy::SignedZero,
                                            Policy::SaturateFinite};
            const Policy rup_flush_saturate_positive{Policy::RUP, Policy::Preserve, Policy::SignedZero,
                                                     Policy::SaturateFinite, Policy::Positive};
            const Case cases[] = {
                // a*b + a with b = -1.0156, so a*(b + 1) = -a/64 rounded: few
                // codes of a are left
                {"factor -1/64", Alias::RepeatedAddend, rne_flush_in, {{0xff7f, 0x7f7f}, 0x4001, 1},
                 plain(fixed_window(0xbf82)), {{0xbfa0, 0xaf80}, 7, 3}},
                // x*x + x with x and the result odd
                {"odd square", Alias::RepeatedInputs, rup_flush_saturate_positive, {{0xce30, 0x76df}, 0x4001, 1},
                 none, {{0xff80, 0x7f80}, 3, 1}},
                // x*(y + 1) with x near 2^-125 and y + 1 crossing zero
                {"subnormal x, y across -1", Alias::RepeatedSecondAddend, rdn, {{0x0109, 0x010b}, 0x7ff, 0x10b},
                 plain({0x8664, 0x0766}), {{0x0101, 0x0116}, 3, 2}},
                // x*(y + 1) rounded down to 2^-125, a result code whose sums
                // start at a power of two
                {"result at a power of two", Alias::RepeatedSecondAddend, rdn, plain({0x0500, 0x0f00}),
                 plain({0xbf7f, 0xbf70}), plain(fixed_window(0x0100))},
                // x*(y + 1) rounded to nearest, y from -2^-8 to -2^-20: the
                // sums of a code u of x round to u - 1 and u. The codes of x
                // are 0 modulo 4 with bit 5 set, those of the result 7 modulo
                // 8 with bit 5 set, so the least supported x, 0x3f68, follows
                // 0x3f60 without support.
                {"offset -1 only", Alias::RepeatedAddend, rne, {{0x3f02, 0x40fe}, 0x23, 0x20},
                 plain({0xbb80, 0xb580}), {{0x3f3f, 0x40ff}, 0x27, 0x27}},
                // x*(y + 1) rounded up with y + 1 = 2 + 2^-6, x from 2^-109
                // to 2040 with bit 10 set, the result with bits 9 and 0 set:
                // whole binades of x have no support
                {"binades without support", Alias::RepeatedAddend, rup, {{0x0900, 0x44ff}, 0x400, 0x400},
                 plain(fixed_window(0x3f82)), {{0xff80, 0x7f80}, 0x201, 0x201}},
                // The same with flushed, saturated results and y + 1 = 0.2383:
                // the binades of x with subnormal results differ
                {"binades with subnormal results", Alias::RepeatedSecondAddend, rup_flush_saturate,
                 plain({0xff7f, 0x7f7f}), plain(fixed_window(0xbf43)), {{0xff80, 0x7f80}, 0x3f, 0x3b}},
                // x*y + x and x*(y + 1) for a fixed y, x over all finite
                // values with known low bits, which fix the offset of the
                // result code from the code of x
                {"known low bits", Alias::RepeatedSecondAddend, rdn, {{0xff7f, 0x7f7f}, 0xf, 0xa},
                 plain(fixed_window(0x40e3)), {{0x88d1, 0x17c9}, 1, 1}},
                {"known low bits, flushed", Alias::RepeatedAddend, rne_flush, {{0xff7f, 0x7f7f}, 3, 0},
                 plain(fixed_window(0xbf3d)), {{0xff80, 0x7f80}, 0xf, 0xd}},
                // The offsets of a factor in a range or of the square do not
                // follow one slope
                {"known low bits, factor range", Alias::RepeatedSecondAddend, rne, {{0xff7f, 0xaa00}, 0x807, 0x806},
                 plain({0x4041, 0x4043}), {{0xff80, 0x7f80}, 3, 1}},
                {"known low bits, square", Alias::RepeatedInputs, rdn, {{0xff7f, 0x7f7f}, 3, 2}, none,
                 {{0xff80, 0x7f80}, 0x420f, 0x4206}},
                // Results at infinity and at the largest finite value
                {"square sum to infinity", Alias::RepeatedInputs, rne, {{0xfbbc, 0xeb82}, 2, 0}, none,
                 {{0x7f80, 0x7f80}, 3, 0}},
                {"saturated maximum", Alias::RepeatedAddend, rne_saturate, plain({0x7e00, 0x7f7f}),
                 plain({0x3f00, 0x3f90}), plain({0x7f70, 0x7f80})},
                {"negative infinity", Alias::RepeatedSecondAddend, rne, plain({0xff7f, 0xfe00}),
                 plain({0x3f00, 0x3f90}), plain({0xff80, 0xff70})},
                // x = -1 gives the exact sum 0, which rounds down to -0
                {"square sum zero at -1", Alias::RepeatedInputs, rdn_flush, {{0xff7f, 0x7f7f}, 1, 0}, none,
                 {{0xff80, 0x7f80}, 0x8001, 0x8000}}};
            const bool first_iteration = supports.empty();
            std::size_t index = 0;
            for (const Case& c : cases) {
              const bool square = c.alias == Alias::RepeatedInputs;
              if (first_iteration)
                supports.push_back({});
              // The supports of the rows (x, y, result) of the posting with a
              // result in the result window and its known bits
              std::array<Supports, 3>& kept = supports[index++];
              const WordValue y_first = square ? 0 : E::order_rank(f, c.y.window.minimum);
              const WordValue y_last = square ? 0 : E::order_rank(f, c.y.window.maximum);
              for (WordValue rank = E::order_rank(f, c.x.window.minimum);
                   first_iteration && rank <= E::order_rank(f, c.x.window.maximum); ++rank) {
                const WordValue x = E::order_code(f, rank);
                if (x % f.sign > f.maximum || (x & c.x.mask) != c.x.value)
                  continue;
                for (WordValue y_rank = y_first; y_rank <= y_last; ++y_rank) {
                  const WordValue y = square ? x : E::order_code(f, y_rank);
                  if ((y & c.y.mask) != c.y.value)
                    continue;
                  const WordValue z = c.alias == Alias::RepeatedAddend         ? ref.evaluate(c.policy, x, y, x).bits
                                      : c.alias == Alias::RepeatedSecondAddend ? ref.evaluate(c.policy, y, x, x).bits
                                                                               : ref.evaluate(c.policy, x, x, x).bits;
                  if ((z & c.z.mask) == c.z.value &&
                      float_order_contains(16, c.z.window.minimum, c.z.window.maximum, z)) {
                    kept[0].add(f, x);
                    kept[1].add(f, y);
                    kept[2].add(f, z);
                  }
                }
              }
              const BitWindows windows = c.alias == Alias::RepeatedAddend         ? BitWindows{{c.x, c.y, c.x, c.z}}
                                         : c.alias == Alias::RepeatedSecondAddend ? BitWindows{{c.y, c.x, c.x, c.z}}
                                                                                  : BitWindows{{c.x, c.x, c.x, c.z}};
              const int column = c.alias == Alias::RepeatedSecondAddend ? 1 : 0;
              FmaSpace s(ref);
              configure(s, ref, windows, c.alias);
              s.post(c.policy, false);
              const Settled settled = settle(s);
              bool passed = settled.propagations <= bound;
              if (passed && kept[0].any)
                passed = settled.status != SS_FAILED && kept[0].kept_by(s.x[column]) &&
                         kept[1].kept_by(s.x[1 - column]) && kept[2].kept_by(s.x[3]);
              if (!passed) {
                olog << ind(2) << "FMA aliased posting " << c.name << ' ' << settled.propagations << '\n';
                return false;
              }
            }
            return true;
          }
        };

        class Posting : public Base {
        public:
          Posting() : Base("Word::Float::Fma::Wide::Posting") {}
          bool run() override {
            for (const auto& signature : signatures)
              for (bool status : {false, true}) {
                const Reference ref(signature[0], signature[1]);
                FmaSpace s(ref);
                s.post(Policy(), status);
                if (s.status() == SS_FAILED)
                  return false;
              }
            const Reference ref(WDT_FP32, WDT_FP32);
            for (int column = 0; column < 4; ++column) {
              FmaSpace s(ref);
              s.x[column] = WordVar(s, WDT_BF16);
              bool caught = false;
              try {
                s.post(Policy());
              } catch (const Gecode::Word::FormatMismatch&) {
                caught = true;
              }
              if (!caught)
                return false;
            }
            FmaSpace status_range(ref);
            status_range.flags = IntVar(status_range, -10, 40);
            status_range.post(Policy());
            if (status_range.flags.min() != 0 || status_range.flags.max() != 31)
              return false;
            return true;
          }
        };
        class FmaFloatTyped : public Base {
          // Total-order window of encodings
          struct Range {
            WordValue minimum, maximum;
          };
          // Check the complete solutions on operand windows a, b and c and
          // result window z, with the result aliased to the addend when
          // \a absorbed holds.
          static bool check(const Reference& ref, const Policy& p, Range a,
                            Range b, Range c, Range z, bool absorbed) {
            const unsigned int source = float_word_width(ref.source.type);
            const unsigned int destination =
                float_word_width(ref.destination.type);
            FmaSpace s(ref);
            s.x[0] = float_window(s, ref.source.type, a.minimum, a.maximum);
            s.x[1] = float_window(s, ref.source.type, b.minimum, b.maximum);
            s.x[2] = float_window(s, ref.destination.type, c.minimum, c.maximum);
            s.x[3] = absorbed ? s.x[2]
                              : float_window(s, ref.destination.type, z.minimum,
                                             z.maximum);
            Rows expected;
            for (WordValue av : float_order_range(source, a.minimum, a.maximum))
              for (WordValue bv :
                   float_order_range(source, b.minimum, b.maximum))
                for (WordValue cv : float_order_range(destination, c.minimum,
                                                      c.maximum)) {
                  const FloatResult e = ref.evaluate(p, av, bv, cv);
                  if (absorbed ? e.bits == cv : s.x[3].in(e.bits))
                    expected.insert({av, bv, cv, e.bits, WordValue(e.flags)});
                }
            s.post(p);
            if (s.status() == SS_FAILED)
              return expected.empty();
            branching(s);
            if (solutions(s) == expected)
              return true;
            if (opt.log)
              olog << ind(2) << "float-typed FMA " << ref.source.type << '/'
                   << ref.destination.type << ' ' << p.rounding << std::hex
                   << ' ' << a.minimum << ".." << a.maximum << ' ' << b.minimum
                   << ".." << b.maximum << ' ' << c.minimum << ".."
                   << c.maximum << ' ' << z.minimum << ".." << z.maximum
                   << std::dec << " absorbed " << absorbed << '\n';
            return false;
          }
          static bool windows(const Reference& ref, bool with_projection) {
            const FloatFormat& f = ref.source;
            const FloatFormat& d = ref.destination;
            const Range all{d.sign | d.infinity, d.infinity};
            // The scan cases have fewer assignments than the scan cap. The
            // projection cases have 13^3 and take the projection path, the
            // last one with the addend absorbing a tiny product.
            struct Case {
              Range a, b, c, z;
              bool absorbed;
            };
            const Case scan[] = {
                {{f.sign | (f.one + 2), f.sign | f.one},
                 {f.one, f.one + 2},
                 {d.sign | (d.one + 1), d.sign | (d.one - 1)},
                 all,
                 false},
                {{f.sign | 2, 2},
                 {f.one - 1, f.one + 1},
                 {d.sign | 2, 2},
                 all,
                 false},
                {{f.one, f.one + 2},
                 {f.sign | (f.one + 1), f.sign | (f.one - 1)},
                 {d.one - 2, d.one + 2},
                 {d.sign | 4, 4},
                 false},
                {{f.sign | (f.one + 2), f.sign | f.one},
                 {f.one, f.one + 2},
                 {d.sign | (d.one + 4), d.sign | (d.one - 4)},
                 {d.sign | (d.one + d.normal + 2), d.sign | (d.one + d.normal)},
                 false},
                {{f.sign | 1, 1},
                 {f.sign | 1, 1},
                 {d.sign | (d.one + 1), d.sign | (d.one - 1)},
                 all,
                 true}};
            const Case projection[] = {
                {{f.sign | 6, 6},
                 {f.one - 6, f.one + 6},
                 {d.sign | 6, 6},
                 {d.sign | d.infinity, 0},
                 false},
                {{f.sign | (f.one + 12), f.sign | f.one},
                 {f.one - 6, f.one + 6},
                 {d.sign | (d.one + 6), d.sign | (d.one - 6)},
                 {d.sign | (d.one + d.normal + 8), d.sign | (d.one + d.normal)},
                 false},
                {{f.sign | 6, 6},
                 {f.one - 6, f.one + 6},
                 {d.sign | (d.one + 6), d.sign | (d.one - 6)},
                 all,
                 true}};
            for (int r = 0; r < 4; ++r)
              for (bool flush : {false, true}) {
                const auto mode = static_cast<Policy::Rounding>(r);
                const Policy p = flush ? Policy{mode, Policy::SignedZero,
                                                Policy::SignedZero}
                                       : Policy{mode};
                for (const Case& c : scan)
                  if (!check(ref, p, c.a, c.b, c.c, c.z, c.absorbed))
                    return false;
                // The projection cases propagate thousands of assignments
                // at every node, so they run under one policy: RNE, or RUP
                // for the absorbing case, where a positive product moves a
                // negative addend.
                if (with_projection)
                  for (const Case& c : projection)
                    if (mode == (c.absorbed ? Policy::RUP : Policy::RNE) &&
                        !flush &&
                        !check(ref, p, c.a, c.b, c.c, c.z, c.absorbed))
                      return false;
              }
            return true;
          }
          // Adding +0 to the float-typed products of the three codes up to -1
          // and 1 gives a float-typed result the total-order bounds -1 - 2 ulp
          // and -1.
          static bool bound_deduction() {
            const Reference ref(WDT_FP32, WDT_FP32);
            FmaSpace s(ref);
            s.x[0] = float_window(s, WDT_FP32, 0xbf800002, 0xbf800000);
            s.x[1] = float_window(s, WDT_FP32, 0x3f800000, 0x3f800000);
            s.x[2] = float_window(s, WDT_FP32, 0, 0);
            s.x[3] = WordVar(s, WDT_FP32);
            s.post(Policy());
            return s.status() != SS_FAILED &&
                   s.x[3].minimum() == 0xbf800002 &&
                   s.x[3].maximum() == 0xbf800000;
          }

          // Position of a BF16 encoding in total order
          static long long order(WordValue code) {
            const long long magnitude = static_cast<long long>(code & 0x7fff);
            return (code & 0x8000) ? -magnitude - 1 : magnitude;
          }
          // Operands with the cube of +-0.25 and +-1 admit 64 assignments, so
          // the scan path keeps exactly the tightest cube and total-order
          // bounds of the supported encodings in every column.
          static bool scan_exactness() {
            const Reference ref(WDT_BF16, WDT_BF16);
            FmaSpace s(ref);
            for (int i = 0; i < 3; ++i)
              s.x[i] = WordVar(s, 16, WDT_BF16, 0xffff, 0x7fff, known_bits(0x7eff, 0x3e80));
            s.x[3] = WordVar(s, WDT_BF16);
            const WordValue codes[] = {0xbf80, 0xbe80, 0x3e80, 0x3f80};
            std::array<WordValue, 4> lo, hi, minimum, maximum;
            lo.fill(0xffff);
            hi.fill(0);
            minimum.fill(0x7fff);
            maximum.fill(0xffff);
            for (WordValue a : codes)
              for (WordValue b : codes)
                for (WordValue c : codes) {
                  const WordValue row[] = {a, b, c,
                                           ref.evaluate(Policy(), a, b, c).bits};
                  for (int i = 0; i < 4; ++i) {
                    lo[i] &= row[i];
                    hi[i] |= row[i];
                    if (order(row[i]) < order(minimum[i]))
                      minimum[i] = row[i];
                    if (order(row[i]) > order(maximum[i]))
                      maximum[i] = row[i];
                  }
                }
            s.post(Policy());
            if (s.status() == SS_FAILED)
              return false;
            for (int i = 0; i < 4; ++i) {
              const WordVar x = s.x[i];
              if (x.lo() != lo[i] || x.hi() != hi[i] ||
                  x.minimum() != minimum[i] || x.maximum() != maximum[i])
                return false;
            }
            return true;
          }

        public:
          FmaFloatTyped() : Base("Word::Float::Fma::Wide::FloatTyped") {}
          bool run() override {
            // The projection path runs on the BF16 and the mixed signature.
            for (const auto& signature : signatures)
              if (!windows(Reference(signature[0], signature[1]),
                           signature[0] == WDT_BF16))
                return false;
            return bound_deduction() && scan_exactness();
          }
        };
        Exact exact;
        Exceptions exceptions;
        Symbolic symbolic;
        Lifecycle lifecycle;
        Absorption absorption;
        MultiplicandAlias multiplicand_alias;
        Cancellation cancellation;
        CancellationSoundness cancellation_soundness;
        // One multiplicand is fixed and the sum cancels far below the
        // addend, so the supports of the free multiplicand form a sparse
        // lattice: interval projections retreat its endpoint by about half
        // an encoding per pass, where linear_extreme takes the extremes of
        // the linear sum at once, also with a result that aliases the fixed
        // multiplicand or the addend. The witnesses are supports, among
        // them the extreme ones of each window.
        class Lattice : public Base {
          struct Case {
            WordDomainType format;
            Policy::Rounding rounding;
            Windows windows;
            WordValue mask, known;
            std::vector<Row> witnesses;
            Alias alias = Alias::Independent;
            /// Whether subnormal inputs and results are flushed to zero
            bool flush = false;
          };
          static bool check(const Case& c, bool swap) {
            const Reference ref(c.format, c.format);
            Policy p;
            p.rounding = c.rounding;
            if (c.flush) {
              p.input_subnormals = Policy::SignedZero;
              p.output_subnormals = Policy::SignedZero;
            }
            FmaSpace s(ref);
            Windows w = c.windows;
            Alias alias = c.alias;
            if (swap) {
              std::swap(w[0], w[1]);
              if (alias == Alias::ResultIsFirst)
                alias = Alias::ResultIsSecond;
            }
            configure(s, w, alias);
            const WordValue sign = ref.destination.sign;
            dom(s, s.x[3], c.known, c.known | (~c.mask & (2 * sign - 1)));
            s.post(p, false);
            const Settled settled = settle(s);
            if (settled.status == SS_FAILED || settled.propagations > 8) {
              if (opt.log)
                olog << ind(2) << "fma lattice: " << settled.propagations
                     << " propagations\n";
              return false;
            }
            for (Row row : c.witnesses) {
              if (ref.evaluate(p, row[0], row[1], row[2]).bits != row[3])
                return false;
              if (swap)
                std::swap(row[0], row[1]);
              for (int i = 0; i < 4; ++i)
                if (!Gecode::Word::WordView(s.x[i]).in(row[i]))
                  return false;
            }
            return true;
          }

        public:
          Lattice() : Base("Word::Float::Fma::Wide::Lattice", TestTags::check()) {}
          bool run() override {
            const Case cases[] = {
                {WDT_BF16,
                 Policy::RTZ,
                 {{{0xcf75, 0xc775},
                   fixed_window(0xb5f3),
                   {0xc5e8, 0xbde8},
                   {0x3d8e, 0x3d90}}},
                 0,
                 0,
                 {{0xcb98, 0xb5f3, 0xc210, 0x3d90, 0},
                  {0xc7c6, 0xb5f3, 0xbde9, 0x3d8e, 0},
                  {0xc7c6, 0xb5f3, 0xbde8, 0x3d8f, 0}}},
                {WDT_BF16,
                 Policy::RNE,
                 {{{0xb7f0, 0xb3f0},
                   fixed_window(0x3b7f),
                   {0x2e40, 0x3640},
                   fixed_window(0x3191)}},
                 0x3,
                 0x1,
                 {{0xb63e, 0x3b7f, 0x3283, 0x3191, 0},
                  {0xb3f0, 0x3b7f, 0x31a0, 0x3191, 0}}},
                {WDT_FP32,
                 Policy::RTZ,
                 {{{0xb9c29f6f, 0xb8c29f6f},
                   fixed_window(0xc6496fd4),
                   {0xc0192632, 0xc0192232},
                   {0x33e851d7, 0x33e851da}}},
                 0x1,
                 0x0,
                 {{0xb9429f6f, 0xc6496fd4, 0xc0192432, 0x33e851d8, 0}}},
                // A result window around zero: only exact cancellation
                // reaches it, by the power of two among the multiplicands.
                {WDT_FP32,
                 Policy::RTZ,
                 {{{0xc2800400, 0xc27ffc00},
                   fixed_window(0xbff3af3d),
                   {0xc2f3b33d, 0xc2f3ab3d},
                   {0x8000000a, 0x0000000f}}},
                 0,
                 0,
                 {{0xc2800000, 0xbff3af3d, 0xc2f3af3d, 0x00000000, 0}}},
                // A result aliasing an assigned multiplicand: the linear sum
                // applies as without the alias.
                {WDT_BF16,
                 Policy::RNE,
                 {{fixed_window(0x803a),
                   {0xd9b2, 0x99b3},
                   {0x9968, 0x669b},
                   fixed_window(0x803a)}},
                 0,
                 0,
                 {{0x803a, 0x99b3, 0x803a, 0x803a, 0},
                  {0x803a, 0xc37f, 0x83e8, 0x803a, 0}},
                 Alias::ResultIsFirst},
                // Flushed results around zero, which the result aliasing
                // the addend reaches only through its zero.
                {WDT_FP32,
                 Policy::RTZ,
                 {{fixed_window(0x3f7ae148),
                   {0x8077d54d, 0x01000000},
                   {0x80024149, 0x80000000},
                   {0x80024149, 0x80000000}}},
                 0,
                 0,
                 {{0x3f7ae148, 0x8077d54d, 0x80000000, 0x80000000, 0},
                  {0x3f7ae148, 0x80000000, 0x80000000, 0x80000000, 0}},
                 Alias::ResultIsAddend,
                 true}};
            for (const Case& c : cases)
              for (bool swap : {false, true})
                if (!check(c, swap))
                  return false;
            return true;
          }
        };

        // A result aliasing a multiplicand on small BF16 windows: the
        // propagated domains equal the hulls of the brute-force supports.
        class AliasProjection : public Base {
          static bool check(const Reference& ref, const Policy& p,
                            const Windows& w, Alias alias) {
            FmaSpace s(ref);
            configure(s, w, alias);
            s.post(p, false);
            const Rows expected = expected_rows(ref, p, w, alias, -1);
            if (!preserves(s, expected))
              return false;
            if (expected.empty())
              return true;
            for (int i = 0; i < 3; ++i) {
              std::set<WordValue> column;
              for (const Row& row : expected)
                column.insert(row[i]);
              const auto hull = float_order_extremes(16, column);
              if (s.x[i].minimum() != hull.first ||
                  s.x[i].maximum() != hull.second) {
                if (opt.log)
                  olog << ind(2) << "fma alias projection " << i << ": " << std::hex
                       << s.x[i].minimum() << ".." << s.x[i].maximum()
                       << " hull " << hull.first << ".." << hull.second
                       << std::dec << '\n';
                return false;
              }
            }
            return true;
          }

        public:
          AliasProjection() : Base("Word::Float::Fma::Wide::AliasProjection") {}
          bool run() override {
            const Reference ref(WDT_BF16, WDT_BF16);
            // RTZ agrees with RDN on these mostly positive windows.
            const Window small{0x0010, 0x0030}, tiny{0x0005, 0x0020},
                near_one{0x3f40, 0x3fa0};
            for (Policy::Rounding r : {Policy::RNE, Policy::RDN, Policy::RUP}) {
              Policy p;
              p.rounding = r;
              if (!check(ref, p, {{small, near_one, {0x0000, 0x0002}, small}},
                         Alias::ResultIsFirst) ||
                  !check(ref, p, {{near_one, tiny, {0x8001, 0x0001}, tiny}},
                         Alias::ResultIsSecond))
                return false;
            }
            // The aliased multiplicand is +0, so the product is a zero and
            // the result +0 needs c = -0 or +0: the addend narrows to these
            // two codes.
            const Window zero{0x0000, 0x0000};
            Policy rtz;
            rtz.rounding = Policy::RTZ;
            if (!check(ref, rtz, {{zero, {0x3e8b, 0x402f}, {0x8072, 0x0021}, zero}},
                       Alias::ResultIsFirst) ||
                !check(ref, Policy(), {{{0x3eef, 0x3f16}, zero, {0x802a, 0x0072}, zero}},
                       Alias::ResultIsSecond))
              return false;
            return true;
          }
        };
        // A tiny product, a = 0x110001ee and b in [-3.38, -0.98], against
        // a full addend with its low two bits zero and a full result with
        // its low four bits 1101. Where the addend's spacing exceeds four
        // times |a*b| the result is the addend itself, and in the binades
        // below it the addend plus a few codes, so interval projections
        // move addend and result 16 codes per pass, about
        // 1024 per propagation over some 10^9 codes. Read by addend binade
        // (narrow_addend_ends), one status() of at most 8 executions ends
        // at a fixpoint whose moved bounds are the witnesses b = -3 with c =
        // 2^-69 and with c = 0x9dfffffc, whose results the reference
        // confirms.
        class AddendBinades : public Base {
        public:
          AddendBinades() : Base("Word::Float::Fma::Wide::AddendBinades") {}
          bool run() override {
            const FloatRegression c{
                "RNE tiny product against a full addend",
                {FloatOperation::Fma,
                 WDT_FP32,
                 float_dump_policy(0, 1, 1, 0, 0, 1, 1),
                 true,
                 {{0x110001ee, 0x110001ee, 0, 0},
                  {0xc05853c2, 0xbf7b21c0, 0, 0},
                  {0xff7fffff, 0x7f7fffff, 0x3, 0},
                  {0xff7fffff, 0x7f7fffff, 0xf, 0xd}}},
                {0x110001ee, std::nullopt, std::nullopt, std::nullopt},
                {{0x110001ee, 0xc0400000, 0x1d000000, 0x1cfffffd},
                 {0x110001ee, 0xc0400000, 0x9dfffffc, 0x9dfffffd}}};
            if (!check_float_regression(c, 8))
              return false;
            const Reference ref(WDT_FP32, WDT_FP32);
            for (const auto& w : c.witnesses)
              if (ref.evaluate(c.posting.policy, w[0], w[1], w[2]).bits != w[3])
                return false;
            return true;
          }
        };

        // Both multiplicands assigned: an addend range with one known
        // fraction bit and a result range with known low bits, the first
        // across zero, where the sums cancel. Interval projections move
        // addend and result by 64 and 1024 codes per propagation, more than
        // 2000 propagations; in the second they walk 177 propagations. One
        // status() of at most 8 executions ends at a fixpoint. In the first,
        // the product as a constant of the sum (fixed_product) gives addend
        // ends that are witnesses and give the result's ends, as the
        // reference confirms. In the second, the step rule ends at the
        // fixpoint of the widened projection, pinned below.
        class FixedProduct : public Base {
        public:
          FixedProduct() : Base("Word::Float::Fma::Wide::FixedProduct") {}
          bool run() override {
            const FloatRegression cases[] = {
                {"RTZ fixed product, sums cancelling across zero",
                 {FloatOperation::Fma,
                  WDT_FP32,
                  float_dump_policy(1, 0, 0, 0, 0, 0, 0),
                  false,
                  {{0xbe739753, 0xbe739753, 0, 0},
                   {0x3286396e, 0x3286396e, 0, 0},
                   {0x317394a5, 0x31ac12f7, 0x80010000, 0x10000},
                   {0xb24e3f92, 0x7f7fffff, 0x7, 0x5}}},
                 {0xbe739753, 0x3286396e, std::nullopt, std::nullopt},
                 {{0xbe739753, 0x3286396e, 0x317f6f76, 0xa87e7405},
                  {0xbe739753, 0x3286396e, 0x317fefb5, 0x2cfffec5}}},
                {"RTZ fixed product, step rule fixpoint pinned",
                 {FloatOperation::Fma,
                  WDT_FP32,
                  float_dump_policy(1, 1, 0, 0, 1, 0, 0),
                  false,
                  {{0x43ab771d, 0x43ab771d, 0, 0},
                   {0xc3bc89cc, 0xc3bc89cc, 0, 0},
                   {0xff7fffff, 0x7f7fffff, 0, 0},
                   {0xc98b47f1, 0xc1a56681, 0x871012, 0x840000}}},
                 {0x43ab771d, 0xc3bc89cc, std::nullopt, std::nullopt},
                 {{0x43ab771d, 0xc3bc89cc, 0xc96a4dea, 0xc984efed},
                  {0x43ab771d, 0xc3bc89cc, 0x47f44f89, 0xc5840005}},
                 false,
                 FloatMovedBounds::Free}};
            const Reference ref(WDT_FP32, WDT_FP32);
            bool passed = true;
            for (const FloatRegression& c : cases) {
              passed &= check_float_regression(c, 8);
              for (const auto& w : c.witnesses)
                passed &= ref.evaluate(c.posting.policy, w[0], w[1], w[2])
                              .bits == w[3];
            }
            // Repeated interval projections: c [0xc96a4dea, 0x47f44f89],
            // result maximum 0xc5840005.
            const FloatReplay r = replay_float_posting(cases[1].posting, 8);
            passed &= r.finished && !r.failed &&
                      r.domains[2].minimum == 0xc9786e0e &&
                      r.domains[2].maximum == 0x47fc853a &&
                      r.domains[3].minimum == 0xc984efed &&
                      r.domains[3].maximum == 0xc1ac0000;
            return passed;
          }
        };

        // FP32 fma postings whose interval projections move a bound of a and
        // b a few codes each, for 25,592 and 28,605 executions (8.6 and
        // 8.2 s). The step rule settles each
        // in one execution. The limit of 2 counts the runs of the replay's
        // watcher, after posting and after the execution.
        class StepBudget : public Base {
          static FloatRegression settles(const char* name, Policy p,
                                         FloatWord a, FloatWord b,
                                         FloatWord c, FloatWord z) {
            return {name,
                    {FloatOperation::Fma, WDT_FP32, p, false, {a, b, c, z}},
                    {},
                    {},
                    false,
                    FloatMovedBounds::Free};
          }

        public:
          StepBudget() : Base("Word::Float::Fma::Wide::StepBudget") {}
          bool run() override {
            const FloatRegression cases[] = {
                settles("saturating, b near -1, a*b cancelling c near 7e35", float_dump_policy(0, 0, 0, 1, 0, 0, 1),
                        {0x7aea6f69, 0x7b0a6f6b, 0, 0},
                        {0xbf9d4436, 0xbf5d4434, 0x80000000, 0x80000000},
                        {0x7b09efd6, 0x7b09efd8, 0x3, 0x2},
                        {0xee37b5dd, 0xee37b5d6, 0x80000000, 0x80000000}),
                settles("RUP, fixed small negative result, c near 28520", float_dump_policy(3, 1, 0, 1, 1, 0, 1),
                        {0xca51b49a, 0xc251b49a, 0x1, 0},
                        {0x3cb4335b, 0x3db4335c, 0x3, 0x1},
                        {0x46ded181, 0x46ded182, 0x3, 0x1},
                        {0xb9852a10, 0xb9852a10, 0, 0})};
            bool passed = true;
            for (const FloatRegression& c : cases)
              passed &= check_float_regression(c, 2);
            return passed;
          }
        };

        // The step rule on random small BF16 windows of finite operands
        // over every policy field, with and without a status: status()
        // keeps exactly the solutions of the oracle (check_window compares
        // them by search), settles in one execution, and ends at a fixpoint,
        // which a fresh propagator on a clone leaves unchanged. Most windows
        // hold more assignments than plan_scan enumerates, so that the step
        // rule projects them.
        class StepRule : public Base {
        public:
          StepRule() : Base("Word::Float::Fma::Wide::StepRule") {}
          bool run() override {
            const Reference ref(WDT_BF16, WDT_BF16);
            for (int n = 0; n < 100; ++n) {
              const Policy p = float_dump_policy(
                  static_cast<int>(_rand(4)), static_cast<int>(_rand(2)),
                  static_cast<int>(_rand(2)), static_cast<int>(_rand(2)),
                  static_cast<int>(_rand(2)), static_cast<int>(_rand(3)),
                  static_cast<int>(_rand(2)));
              // The oracle evaluates finite operands only, so the operand
              // windows are redrawn until both ends are finite
              Windows w;
              for (int i = 0; i < 4; ++i)
                do
                  draw_float_window(_rand, WDT_BF16,
                                    i == 3 && _rand(3) == 0 ? 1U << 16 : 32,
                                    w[i].minimum, w[i].maximum);
                while (i < 3 && (!ref.source.is_finite(w[i].minimum) ||
                                 !ref.source.is_finite(w[i].maximum)));
              const int status =
                  _rand(4) == 0 ? static_cast<int>(_rand(32)) : -1;
              bool passed = check_window(ref, p, w, -1, Alias::Independent, status);
              if (passed) {
                FmaSpace s(ref);
                configure(s, w, Alias::Independent, status);
                s.post(p);
                StatusStatistics statistics;
                if (s.status(statistics) != SS_FAILED)
                  passed = statistics.propagate <= 1 && unchanged_by_fresh(s, p);
              }
              if (!passed) {
                olog << ind(2) << "FMA step rule " << std::hex;
                for (const Window& v : w)
                  olog << v.minimum << ' ' << v.maximum << ' ';
                olog << std::dec << status << " policy " << p.rounding << ' '
                     << p.input_subnormals << ' ' << p.output_subnormals << ' '
                     << p.overflow << ' ' << p.zeros << ' ' << p.nan << ' '
                     << p.tininess << '\n';
                return false;
              }
            }
            return true;
          }
        };

        // fp_fma(a, b, c, b) and fp_fma(a, b, c, a), with the free
        // multiplicand on a lattice of powers of two and the addend on
        // +-1.625*2^e. With one pass of the alias classes per execution, the
        // free multiplicand and the addend narrowed each other by one binade
        // per execution: 247 executions (FP32 and BF16), 250 with both signs
        // and 124 for the last. The step rule settles each in one execution;
        // the supports are those with the least and greatest a, b and c.
        class AliasBudget : public PinnedPostings {
        public:
          AliasBudget() : PinnedPostings("Word::Float::Fma::Wide::AliasBudget") {}
          bool run() override {
            const BitWindow powers{{0x00800000, 0x7e800000}, 0x807fffff, 0};
            const BitWindow signed_powers{{0xfe800000, 0x7e800000}, 0x007fffff, 0};
            const BitWindow near_four{{0x40600000, 0x40a40000}, 0xff000000, 0x40000000};
            const BitWindow addend{{0xff500000, 0x7f500000}, 0x007fffff, 0x00500000};
            const BitWindow bf16_powers{{0x0080, 0x7e80}, 0x807f, 0};
            const BitWindow bf16_odd_powers{{0x2c80, 0x7d80}, 0x80ff, 0x0080};
            const BitWindow bf16_near_four{{0x4060, 0x40a4}, 0xff00, 0x4000};
            const BitWindow bf16_addend{{0xff50, 0x7f50}, 0x007f, 0x0050};
            const BitWindow bf16_odd_addend{{0xff50, 0x7f50}, 0x00ff, 0x0050};
            const std::vector<Support> fp32{{0x3e000000, 0x406db6db, 0x40500000},
                                            {0x3f800000, 0x40600000, 0x00500000},
                                            {0x3f800000, 0x40a40000, 0x00500000},
                                            {0x3f800000, 0x40a40000, 0xb4500000}};
            const Case cases[] = {
                {"FP32 r == b", WDT_FP32, Policy(), Alias::ResultIsSecond,
                 {{powers, near_four, addend, near_four}}, fp32},
                {"FP32 r == a", WDT_FP32, Policy(), Alias::ResultIsFirst,
                 {{near_four, powers, addend, near_four}},
                 {{0x406db6db, 0x3e000000, 0x40500000},
                  {0x40600000, 0x3f800000, 0x00500000},
                  {0x40a40000, 0x3f800000, 0xb4500000}}},
                {"FP32 r == b, a of both signs", WDT_FP32, Policy(), Alias::ResultIsSecond,
                 {{signed_powers, near_four, addend, near_four}}, fp32},
                {"BF16 r == b", WDT_BF16, Policy(), Alias::ResultIsSecond,
                 {{bf16_powers, bf16_near_four, bf16_addend, bf16_near_four}},
                 {{0x3e00, 0x406e, 0x4050}, {0x3f80, 0x4060, 0x0050},
                  {0x3f80, 0x40a4, 0x0050}, {0x3f80, 0x4081, 0xbc50}}},
                {"BF16 r == b, odd exponents", WDT_BF16, Policy(), Alias::ResultIsSecond,
                 {{bf16_odd_powers, bf16_near_four, bf16_odd_addend, bf16_near_four}},
                 {{0x3e80, 0x408a, 0x4050}, {0x3f80, 0x4060, 0x0050},
                  {0x3f80, 0x40a4, 0x0050}, {0x3f80, 0x4081, 0xbc50}}}};
            return check(cases, std::size(cases), "FMA alias budget", 2);
          }
        };

        // The step rule on the shapes whose result aliases an input, on
        // random small BF16 windows of finite operands over every policy
        // field, with and without a status: status() keeps exactly the
        // solutions of the oracle (check_window compares them by search),
        // settles in one execution, and ends at a fixpoint, which a fresh
        // propagator on a clone leaves unchanged. The free words hold more
        // assignments together than plan_scan enumerates.
        class AliasStepRule : public Base {
        public:
          AliasStepRule() : Base("Word::Float::Fma::Wide::AliasStepRule") {}
          bool run() override {
            const Reference ref(WDT_BF16, WDT_BF16);
            for (Alias alias :
                 {Alias::ResultIsFirst, Alias::ResultIsSecond, Alias::ResultIsSquare,
                  Alias::ResultIsFirstAndAddend, Alias::ResultIsAddend,
                  Alias::ResultIsSquareAddend, Alias::ResultIsAll})
              for (int n = 0; n < 40; ++n)
                if (!check(ref, alias))
                  return false;
            return true;
          }

        private:
          bool check(const Reference& ref, Alias alias) {
            const Policy p = float_dump_policy(
                static_cast<int>(_rand(4)), static_cast<int>(_rand(2)),
                static_cast<int>(_rand(2)), static_cast<int>(_rand(2)),
                static_cast<int>(_rand(2)), static_cast<int>(_rand(3)),
                static_cast<int>(_rand(2)));
            const auto origin = origins(alias);
            int free = 0;
            for (int i = 0; i < 3; ++i)
              free += origin[i] == i ? 1 : 0;
            // Above the scan: 32^3, 64^2 or 4096 assignments
            const unsigned int size = free == 3 ? 32 : free == 2 ? 64 : 4096;
            // The oracle evaluates finite operands only, so the windows are
            // redrawn until both ends are finite; aliased columns repeat the
            // window of their origin
            Windows w;
            for (int i = 0; i < 4; ++i)
              if (origin[i] != i)
                w[i] = w[origin[i]];
              else
                do
                  draw_float_window(_rand, WDT_BF16, size, w[i].minimum,
                                    w[i].maximum);
                while (!ref.source.is_finite(w[i].minimum) ||
                       !ref.source.is_finite(w[i].maximum));
            const int status = _rand(4) == 0 ? static_cast<int>(_rand(32)) : -1;
            bool passed = check_window(ref, p, w, -1, alias, status);
            if (passed) {
              FmaSpace s(ref);
              configure(s, w, alias, status);
              s.post(p);
              StatusStatistics statistics;
              if (s.status(statistics) != SS_FAILED)
                passed = statistics.propagate <= 1 && unchanged_by_fresh(s, p);
            }
            if (!passed) {
              olog << ind(2) << "FMA alias step rule " << static_cast<int>(alias)
                   << std::hex;
              for (const Window& v : w)
                olog << ' ' << v.minimum << ' ' << v.maximum;
              olog << std::dec << ' ' << status << " policy " << p.rounding << ' '
                   << p.input_subnormals << ' ' << p.output_subnormals << ' '
                   << p.overflow << ' ' << p.zeros << ' ' << p.nan << ' '
                   << p.tininess << '\n';
            }
            return passed;
          }
        };

        Strength strength;
        AliasedKnownBits aliased_known_bits;
        AliasedBounds aliased_bounds;
        CancellationPostings cancellation_postings;
        AliasedEnumeration aliased_enumeration;
        AddendSquareEnumeration addend_square_enumeration;
        AliasedPostings aliased_postings;
        AliasProjection alias_projection;
        Lattice lattice;
        AddendBinades addend_binades;
        FixedProduct fixed_product;
        StepBudget step_budget;
        StepRule step_rule;
        AliasBudget alias_budget;
        AliasStepRule alias_step_rule;
        Posting posting;
        FmaFloatTyped fma_float_typed;
      } // namespace
    } // namespace Float
  } // namespace Word
} // namespace Test
// STATISTICS: test-word
