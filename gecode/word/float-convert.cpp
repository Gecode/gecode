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
#include <algorithm>
#include <array>

namespace Gecode {
namespace Word {
namespace {
using Rounding = FloatWordPolicy::Rounding;

WordDomain snapshot(WordView view) {
  return {{view.lo(), view.hi()},
          {view.bounded() ? view.rank_minimum() : view.lo(),
           view.bounded() ? view.rank_maximum() : view.hi()}};
}

// Each encoding range lies within one sign half, so its ranks form one
// interval.
WordDomainResult intersect(WordView view, WordDomain domain, WordEncodingRange range) {
  const WordRankInterval ranks =
      rank_interval(view.domain_type(), view.width(), range.first, range.last);
  domain.interval.minimum = std::max(domain.interval.minimum, ranks.minimum);
  domain.interval.maximum = std::min(domain.interval.maximum, ranks.maximum);
  if (domain.interval.minimum > domain.interval.maximum)
    return {false, {{0, 0}, {0, 0}}};
  return compute_domain_closure(view.width(), view.domain_type(), domain);
}

// Exact bit hull of an admitted cube/interval intersection. Closure supplies
// its endpoints; testing each free bit also captures non-prefix forced bits.
WordCube bit_hull(WordView view, WordDomain domain) {
  const WordValue first = encode_rank(view.domain_type(), view.width(), domain.interval.minimum);
  WordCube hull{first, first};
  for (WordValue bits = domain.cube.hi ^ domain.cube.lo; bits; bits &= bits - 1) {
    const WordValue bit = bits & (WordValue(0) - bits);
    WordDomain alternative = domain;
    if (first & bit)
      alternative.cube.hi &= ~bit;
    else
      alternative.cube.lo |= bit;
    if (compute_domain_closure(view.width(), view.domain_type(), alternative).is_consistent) {
      if (first & bit)
        hull.lo &= ~bit;
      else
        hull.hi |= bit;
    }
  }
  return hull;
}

// Iterates the encodings a cube/interval domain of a view admits, in
// ascending rank order. Each step costs a constant number of bit operations.
class AdmittedEncodings {
  WordDomainType domain_type;
  unsigned int width;
  WordCube cube;
  WordValue current, last;
  bool is_valid;

public:
  AdmittedEncodings(WordView view, WordDomain domain)
      : domain_type(view.domain_type()), width(view.width()), cube(domain.cube),
        last(domain.interval.maximum) {
    is_valid = next_admitted_rank(domain_type, width, cube, domain.interval.minimum, current) &&
               current <= last;
  }
  bool operator()() const { return is_valid; }
  void operator++() {
    is_valid = current < last && next_admitted_rank(domain_type, width, cube, current + 1, current) &&
               current <= last;
  }
  WordValue val() const { return encode_rank(domain_type, width, current); }
};

struct Projection {
  bool present = false;
  WordCube cube{~WordValue(0), 0};
  WordRankInterval interval{~WordValue(0), 0};

  // Hulls of two projections of one view bound their intersection.
  void intersect(const Projection &other) {
    present = present && other.present;
    cube.lo |= other.cube.lo;
    cube.hi &= other.cube.hi;
    interval.minimum = std::max(interval.minimum, other.interval.minimum);
    interval.maximum = std::min(interval.maximum, other.interval.maximum);
  }

  void add(WordCube bits, WordRankInterval bounds) {
    present = true;
    cube.lo &= bits.lo;
    cube.hi |= bits.hi;
    interval.minimum = std::min(interval.minimum, bounds.minimum);
    interval.maximum = std::max(interval.maximum, bounds.maximum);
  }

  // Narrow view to the cube and interval of the supports.
  bool publish(Space &home, WordView view) const {
    return present && !me_failed(narrow_word_domain(home, view, cube, interval));
  }
};

using Policy = FloatWordPolicy;

struct Format {
  unsigned int width, fraction;
  int bias;
  WordValue maximum, infinity, nan;
  WordValue sign() const { return WordValue(1) << (width - 1); }
  WordValue normal() const { return WordValue(1) << fraction; }
  bool is_nan(WordValue magnitude) const {
    return nan && magnitude > (infinity ? infinity : maximum);
  }
  // Smallest NaN magnitude, or sign() for a format without NaNs.
  WordValue first_nan() const { return nan ? (infinity ? infinity : maximum) + 1 : sign(); }
};

Format format_facts(WordDomainType format) {
  switch (format) {
  case WDT_FP32:
    return {32, 23, 127, 0x7f7fffff, 0x7f800000, 0x7fc00000};
  case WDT_BF16:
    return {16, 7, 127, 0x7f7f, 0x7f80, 0x7fc0};
  case WDT_BINARY16:
    return {16, 10, 15, 0x7bff, 0x7c00, 0x7e00};
  case WDT_E4M3FN:
    return {8, 3, 7, 0x7e, 0, 0x7f};
  case WDT_E2M1:
    return {4, 1, 1, 7, 0, 0};
  default:
    throw OutOfLimits("fp_convert format");
  }
}

struct Dyadic {
  WordValue significand;
  int exponent;
};

Dyadic decode(WordValue magnitude, const Format &format) {
  const WordValue field = magnitude >> format.fraction;
  return {field ? format.normal() | (magnitude & (format.normal() - 1)) : magnitude,
          (field ? static_cast<int>(field) - format.bias : 1 - format.bias) -
              static_cast<int>(format.fraction)};
}

// Round significand * 2^exponent in units of 2^unit. All nonzero
// significands have at most 24 bits; a very large right shift rounds 0 or 1.
WordValue round_units(Dyadic value, int unit, bool negative, Rounding mode) {
  const int shift = unit - value.exponent;
  if (shift <= 0) {
    assert(-shift < 64);
    return value.significand << -shift;
  }
  const WordValue quotient = shift >= 64 ? 0 : value.significand >> shift;
  const WordValue remainder =
      shift >= 64 ? value.significand : value.significand & ((WordValue(1) << shift) - 1);
  bool increment =
      remainder && ((mode == Policy::RUP && !negative) || (mode == Policy::RDN && negative));
  if (mode == Policy::RNE && shift < 64) {
    const WordValue half = WordValue(1) << (shift - 1);
    increment = remainder > half || (remainder == half && (quotient & 1));
  }
  return quotient + increment;
}

// Compare positive dyadics without shifting their exponents' full distance.
int compare(Dyadic a, Dyadic b) {
  if (!a.significand || !b.significand)
    return (a.significand != 0) - (b.significand != 0);
  const int al = 63 - Support::count_leading_zeros_64(a.significand);
  const int bl = 63 - Support::count_leading_zeros_64(b.significand);
  if (al + a.exponent != bl + b.exponent)
    return al + a.exponent < bl + b.exponent ? -1 : 1;
  const WordValue av = a.significand << (63 - al);
  const WordValue bv = b.significand << (63 - bl);
  return (av > bv) - (av < bv);
}

struct Conversion {
  WordValue bits;
  int flags;
  bool tiny = false;
};

WordValue overflow_bits(bool negative, const Format &to, const Policy &policy) {
  const WordValue sign = negative ? to.sign() : 0;
  if (policy.overflow == Policy::SaturateFinite || !to.nan)
    return sign | to.maximum;
  if (!to.infinity)
    return (policy.nan == Policy::Canonical ? 0 : sign) | to.nan;
  const bool infinity = policy.rounding == Policy::RNE ||
                        (policy.rounding == Policy::RUP && !negative) ||
                        (policy.rounding == Policy::RDN && negative);
  return sign | (infinity ? to.infinity : to.maximum);
}

Conversion convert(WordValue bits, const Format &from, const Format &to, const Policy &policy) {
  const bool negative = (bits & from.sign()) != 0;
  const WordValue sign = negative ? to.sign() : 0;
  const WordValue magnitude = bits & (from.sign() - 1);
  if (from.is_nan(magnitude)) {
    if (!to.nan)
      return {to.maximum, 1};
    const bool signaling = from.infinity && !(magnitude & (from.normal() >> 1));
    if (policy.nan == Policy::Canonical)
      return {to.nan, signaling ? 1 : 0};
    WordValue payload = from.infinity ? magnitude & ((from.normal() >> 1) - 1) : 0;
    if (from.fraction > to.fraction)
      payload >>= from.fraction - to.fraction;
    else
      payload <<= to.fraction - from.fraction;
    return {sign | to.nan | (to.infinity ? payload : 0), signaling ? 1 : 0};
  }
  if (from.infinity && magnitude == from.infinity)
    return to.infinity ? Conversion{sign | to.infinity, 0}
                       : Conversion{overflow_bits(negative, to, policy), 20};
  if (!magnitude || (policy.input_subnormals == Policy::SignedZero && magnitude < from.normal()))
    return {policy.zeros == Policy::Positive ? 0 : sign, 0};

  const Dyadic exact = decode(magnitude, from);
  const int leading = 63 - Support::count_leading_zeros_64(exact.significand);
  const int precision_unit = exact.exponent + leading - static_cast<int>(to.fraction);
  const Dyadic precision{round_units(exact, precision_unit, negative, policy.rounding),
                         precision_unit};
  const bool tiny =
      compare(policy.tininess == Policy::BeforeRounding ? exact : precision, {1, 1 - to.bias}) < 0;
  if (compare(precision, decode(to.maximum, to)) > 0)
    return {overflow_bits(negative, to, policy), 20, tiny};
  const int unit = std::max(precision_unit, 1 - to.bias - static_cast<int>(to.fraction));
  WordValue rounded = round_units(exact, unit, negative, policy.rounding);
  int flags = compare(exact, {rounded, unit}) == 0 ? 0 : (tiny ? 24 : 16);
  WordValue result;
  if (rounded < to.normal()) {
    result = rounded;
  } else {
    int exponent = unit + static_cast<int>(to.fraction);
    if (rounded >= (to.normal() << 1)) {
      rounded >>= 1;
      ++exponent;
    }
    result = (WordValue(exponent + to.bias) << to.fraction) | (rounded - to.normal());
  }
  if (policy.output_subnormals == Policy::SignedZero && result && result < to.normal()) {
    result = 0;
    flags = 24;
  }
  return {result | ((!result && policy.zeros == Policy::Positive) ? 0 : sign), flags, tiny};
}

// Return a Boolean variable that is true exactly when FP32 source has class cls.
BoolVar class_indicator(Home home, WordVar source, FloatWordClass cls) {
  BoolVar truth(home, 0, 1);
  Float::classify(home, source, WDT_FP32, cls, truth);
  return truth;
}

// FP32 self-conversion consists only of classification, bit operations and
// selection. Keep this wide identity relation out of the bounded scan actor.
void post_self_conversion(Home home, const Policy &policy, WordVar source, WordVar result,
                          IntVar status) {
  WordVar value = source;
  BoolVar nan = class_indicator(home, source, FWC_NAN);
  WordVar quiet(home, 32), selected(home, 32);
  if (policy.nan == Policy::Canonical)
    dom(home, quiet, WordValue(0x7fc00000));
  else
    rel(home, value, WOT_OR, WordValue(0x00400000), quiet);
  ite(home, nan, quiet, value, selected);
  value = selected;
  BoolVar flushed(home, 0, 0);
  if (policy.input_subnormals == Policy::SignedZero ||
      policy.output_subnormals == Policy::SignedZero) {
    BoolVar subnormal = class_indicator(home, source, FWC_SUBNORMAL);
    WordVar zero(home, 32), next(home, 32);
    rel(home, source, WOT_AND, WordValue(0x80000000), zero);
    ite(home, subnormal, zero, value, next);
    value = next;
    if (policy.input_subnormals == Policy::Preserve)
      flushed = subnormal;
  }
  if (policy.zeros == Policy::Positive) {
    ite(home, class_indicator(home, value, FWC_ZERO), WordValue(0), value, result);
  } else {
    rel(home, value, WRT_EQ, result);
  }
  if (status.varimp())
    linear(home, IntArgs{1, 24},
           BoolVarArgs{class_indicator(home, source, FWC_SIGNALING_NAN), flushed}, IRT_EQ, status);
}

// Return the first encoding satisfying a monotone predicate, or last+1.
template <class Predicate> WordValue first_true(WordEncodingRange range, Predicate predicate) {
  WordValue lower = range.first, upper = range.last + 1;
  while (lower < upper) {
    const WordValue middle = lower + (upper - lower) / 2;
    if (predicate(middle))
      upper = middle;
    else
      lower = middle + 1;
  }
  return lower;
}

class MixedConversion : public BinaryPropagator<WordView, PC_WORD_DOM> {
  using Base = BinaryPropagator<WordView, PC_WORD_DOM>;
  const Format from, to;
  const Policy policy;
  // Optional status; a null view when the posting has none.
  Int::IntView status;

  bool has_status() const { return status.varimp() != nullptr; }
  bool admits_status(int flags) const { return !has_status() || status.in(flags); }

  // The hull of the supports of each view, and for a view of width at most
  // eight also their exact values.
  struct Supports {
    Projection source, result;
    WordValueSet source_values = {{0, 0, 0, 0}}, result_values = {{0, 0, 0, 0}};
    unsigned int statuses = 0;
  };

  void support_range(Supports &supports, WordDomain source, WordValue result, int flags) const {
    supports.source.add(bit_hull(x0, source), source.interval);
    const WordValue r = rank(x1.domain_type(), to.width, result);
    supports.result.add({result, result}, {r, r});
    if (to.width <= 8)
      supports.result_values = with_value(supports.result_values, result);
    supports.statuses |= 1U << flags;
  }

  void support_pair(Supports &supports, WordValue source, WordValue result, int flags) const {
    if (!x1.in(result) || !admits_status(flags) || ((x0 == x1) && source != result))
      return;
    const WordValue r = rank(x0.domain_type(), from.width, source);
    supports.source.add({source, source}, {r, r});
    const WordValue t = rank(x1.domain_type(), to.width, result);
    supports.result.add({result, result}, {t, t});
    if (from.width <= 8)
      supports.source_values = with_value(supports.source_values, source);
    if (to.width <= 8)
      supports.result_values = with_value(supports.result_values, result);
    supports.statuses |= 1U << flags;
  }

  void scan_sources(Supports &supports, WordDomain sources) const {
    for (AdmittedEncodings source(x0, sources); source(); ++source) {
      const WordValue code = source.val();
      // A value set can exclude encodings that its cube and interval admit.
      if (x0.has_value_set() && !x0.in(code))
        continue;
      const auto output = convert(code, from, to, policy);
      support_pair(supports, code, output.bits, output.flags);
    }
  }

  // Each class/sign branch is monotone in the output magnitude. Subdivide
  // its exact output preimage at range-event changes and the one exact value.
  void project_preimage(Supports &supports, WordDomain sources, WordEncodingRange range,
                        WordValue result) const {
    std::array<WordValue, 6> cuts;
    size_t n = 0;
    cuts[n++] = range.first;
    cuts[n++] = range.last + 1;
    const WordValue magnitude = range.first & (from.sign() - 1);
    if (magnitude && magnitude <= from.maximum &&
        !(policy.input_subnormals == Policy::SignedZero && magnitude < from.normal())) {
      cuts[n++] = first_true(
          range, [&](WordValue bits) { return convert(bits, from, to, policy).flags & 4; });
      cuts[n++] =
          first_true(range, [&](WordValue bits) { return !convert(bits, from, to, policy).tiny; });
      if ((result & (to.sign() - 1)) <= to.maximum) {
        const WordValue exact = convert(result, to, from, Policy()).bits;
        if (exact >= range.first && exact <= range.last) {
          cuts[n++] = exact;
          cuts[n++] = exact + 1;
        }
      }
    }
    std::sort(cuts.begin(), cuts.begin() + n);
    for (size_t i = 1; i < n; ++i) {
      if (cuts[i] == cuts[i - 1])
        continue;
      const int flags = convert(cuts[i - 1], from, to, policy).flags;
      if (!admits_status(flags))
        continue;
      const auto source = intersect(x0, sources, {cuts[i - 1], cuts[i] - 1});
      if (source.is_consistent)
        support_range(supports, source.domain, result, flags);
    }
  }

  WordValue output_magnitude(WordValue bits) const {
    return convert(bits, from, to, policy).bits & (to.sign() - 1);
  }

  // Project the sources in range whose output magnitude is that of the
  // admitted result encoding candidate.
  void project_candidate(Supports &supports, WordDomain sources, WordEncodingRange range,
                         WordValue candidate) const {
    const WordValue code = candidate & (to.sign() - 1);
    const WordValue first =
        first_true(range, [&](WordValue bits) { return output_magnitude(bits) >= code; });
    if (first > range.last || output_magnitude(first) != code)
      return;
    const WordValue last =
        first_true({first, range.last},
                   [&](WordValue bits) { return output_magnitude(bits) > code; }) -
        1;
    const WordValue result = convert(first, from, to, policy).bits;
    if (!x1.in(result))
      return;
    project_preimage(supports, sources, {first, last}, result);
  }

  // Enumerate the admitted result encodings for the source range of one
  // branch, whose outputs carry branch_sign and have magnitudes in
  // magnitudes. Positive zeros and canonical NaNs drop the sign, so each
  // piece below keeps one sign and lies within one sign partition of the
  // result.
  void scan_candidates(Supports &supports, WordDomain sources, WordDomain results,
                       WordEncodingRange range, WordValue branch_sign, WordEncodingRange magnitudes) const {
    const WordValue cuts[] = {0, 1, to.first_nan(), to.sign()};
    for (unsigned int piece = 0; piece < 3; ++piece) {
      const WordValue first = std::max(magnitudes.first, cuts[piece]);
      const WordValue last = std::min(magnitudes.last, cuts[piece + 1] - 1);
      if (first > last)
        continue;
      const bool drops_sign = (!first && policy.zeros == Policy::Positive) ||
                              (to.is_nan(first) && policy.nan == Policy::Canonical);
      const WordValue result_sign = drops_sign ? 0 : branch_sign;
      const auto admitted = intersect(x1, results, {result_sign | first, result_sign | last});
      if (!admitted.is_consistent)
        continue;
      for (AdmittedEncodings candidate(x1, admitted.domain); candidate(); ++candidate)
        project_candidate(supports, sources, range, candidate.val());
    }
  }

  void scan_preimages(Supports &supports, WordDomain sources) const {
    const WordDomain results = snapshot(x1);
    // Zero, input subnormals, finite normals, infinity, sNaN and qNaN.
    const WordValue boundaries[] = {0, 1, 0x800000, 0x7f800000, 0x7f800001, 0x7fc00000, 0x80000000};
    for (WordValue sign : {WordValue(0), from.sign()})
      for (unsigned int part = 0; part < 6; ++part) {
        const auto available =
            intersect(x0, sources, {sign | boundaries[part], sign | (boundaries[part + 1] - 1)});
        if (!available.is_consistent)
          continue;
        const WordEncodingRange range =
            encoding_range(x0.domain_type(), from.width, available.domain.interval);
        const WordValue branch_sign = part >= 4
                                          ? convert(range.first, from, to, policy).bits & to.sign()
                                      : sign ? to.sign()
                                             : 0;
        scan_candidates(supports, sources, results, range, branch_sign,
                        {output_magnitude(range.first), output_magnitude(range.last)});
      }
  }

  // A view of width at most eight receives its exact supported values,
  // a wider view their hull.
  static bool publish(Space &home, WordView view, const Projection &hull,
                      const WordValueSet &values) {
    if (view.width() > 8)
      return hull.publish(home, view);
    return hull.present && !me_failed(view.narrow_values(home, values));
  }

  ExecStatus publish_status(Space &home, unsigned int statuses) {
    if (!has_status())
      return ES_OK;
    for (int flag = 0; flag <= 31; ++flag)
      if ((statuses & (1U << flag)) == 0)
        GECODE_ME_CHECK(status.nq(home, flag));
    return ES_OK;
  }

  MixedConversion(Home home, WordVar source, WordVar result, const Policy &p, Int::IntView flags)
      : Base(home, source, result), from(format_facts(source.domain_type())),
        to(format_facts(result.domain_type())), policy(p), status(flags) {
    if (has_status())
      status.subscribe(home, *this, Int::PC_INT_DOM);
  }
  MixedConversion(Space &home, MixedConversion &other)
      : Base(home, other), from(other.from), to(other.to), policy(other.policy) {
    if (other.has_status())
      status.update(home, other.status);
  }

public:
  Actor *copy(Space &home) override { return new (home) MixedConversion(home, *this); }
  void reschedule(Space &home) override {
    Base::reschedule(home);
    if (has_status())
      status.reschedule(home, *this, Int::PC_INT_DOM);
  }
  size_t dispose(Space &home) override {
    if (has_status())
      status.cancel(home, *this, Int::PC_INT_DOM);
    Base::dispose(home);
    return sizeof(*this);
  }
  PropCost cost(const Space &, const ModEventDelta &) const override {
    return PropCost::binary(PropCost::HI);
  }
  /// Post the conversion; a null \a status means the posting has none.
  static void post(Home home, const Policy &policy, WordVar source, WordVar result,
                   IntVar status) {
    (void)new (home) MixedConversion(home, source, result, policy, status);
  }
  ExecStatus propagate(Space &home, const ModEventDelta &) override {
    Supports supports;
    const WordDomain sources = snapshot(x0);
    if (from.width <= 16 || x0.assigned())
      scan_sources(supports, sources);
    else
      scan_preimages(supports, sources);
    if (x0 == x1) {
      supports.source.intersect(supports.result);
      supports.result = supports.source;
      supports.source_values = intersect_value_sets(supports.source_values, supports.result_values);
      supports.result_values = supports.source_values;
    }
    if (!publish(home, x0, supports.source, supports.source_values) ||
        !publish(home, x1, supports.result, supports.result_values))
      return ES_FAILED;
    GECODE_ES_CHECK(publish_status(home, supports.statuses));
    if (x0.assigned() && x1.assigned() && (!has_status() || status.assigned()))
      return home.ES_SUBSUMED(*this);
    // Supports retain every admitted tuple, including equality of aliased
    // views. A narrower view keeps exactly its supported values, or the
    // smallest cube and bounds containing them, and a wider view the hull
    // of its supports. A second pass finds these supports again within the
    // published domains, so publication is idempotent.
    return ES_FIX;
  }
};
} // namespace
} // namespace Word

namespace {
// Check both formats before posting, and post the conversion.
void post_convert(Home home, const FloatWordPolicy &policy, WordVar source, WordVar result,
                  IntVar status) {
  const WordDomainType from = Word::Float::float_format(source, "fp_convert format");
  const WordDomainType to = Word::Float::float_format(result, "fp_convert format");
  Word::Float::check(policy, "fp_convert policy");
  GECODE_POST;
  if (status.varimp())
    dom(home, status, 0, 31);
  if (from == WDT_FP32 && to == WDT_FP32)
    Word::post_self_conversion(home, policy, source, result, status);
  else
    Word::MixedConversion::post(home, policy, source, result, status);
}
} // namespace

void fp_convert(Home home, WordVar source, WordVar result, const FloatWordPolicy &policy) {
  post_convert(home, policy, source, result, IntVar());
}
void fp_convert(Home home, WordVar source, WordVar result, IntVar status,
                const FloatWordPolicy &policy) {
  post_convert(home, policy, source, result, status);
}
} // namespace Gecode

// STATISTICS: word-post
