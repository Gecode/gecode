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

#include "test/word.hh"
#include <array>
#include <gecode/float-word.hh>
#include <memory>
#include <vector>

namespace Test {
namespace Word {
namespace Float {
namespace {
using namespace Gecode;
using Policy = FloatWordPolicy;

class ConversionSpace : public Space {
public:
  FloatWordVar source, result;
  ConversionSpace(WordDomainType source_kind = WDT_UNSIGNED,
                  WordDomainType result_kind = WDT_UNSIGNED)
      : source(*this, FWF_FP32, source_kind), result(*this, FWF_BINARY16, result_kind) {}
  ConversionSpace(ConversionSpace &other) : Space(other) {
    source.update(*this, other.source);
    result.update(*this, other.result);
  }
  Space *copy() override { return new ConversionSpace(*this); }
};

class ConversionBoundaries : public Base {
public:
  ConversionBoundaries() : Base("Word::Float::ConversionBoundaries") {}
  bool run() override {
    struct Boundary {
      WordValue source, nearest, truncated;
    };
    const Boundary cases[] = {{0, 0, 0},
                              {1, 0, 0},
                              {0x007fffff, 0, 0},
                              {0x33000000, 0, 0}, // Half of minimum subnormal: even zero.
                              {0x33000001, 1, 0},
                              {0x337fffff, 1, 0},
                              {0x33800000, 1, 1},
                              {0x33c00000, 2, 1}, // Odd lower subnormal tie.
                              {0x387fdfff, 0x3ff, 0x3ff},
                              {0x387fe000, 0x400, 0x3ff}, // Normal/subnormal tie.
                              {0x38800000, 0x400, 0x400},
                              {0x3f800fff, 0x3c00, 0x3c00},
                              {0x3f801000, 0x3c00, 0x3c00}, // Even lower normal tie.
                              {0x3f801001, 0x3c01, 0x3c00},
                              {0x3f803000, 0x3c02, 0x3c01}, // Odd lower normal tie.
                              {0x477fe000, 0x7bff, 0x7bff},
                              {0x477fefff, 0x7bff, 0x7bff},
                              {0x477ff000, 0x7c00, 0x7bff},
                              {0x7f7fffff, 0x7c00, 0x7bff}};
    for (const Boundary &b : cases)
      for (WordValue sign : {WordValue(0), WordValue(0x80000000)})
        for (Policy::Rounding mode : {Policy::RNE, Policy::RTZ})
          for (bool assigned_at_post : {false, true}) {
            const WordValue expected =
                (mode == Policy::RNE ? b.nearest : b.truncated) | (sign >> 16);
            for (bool valid : {false, true}) {
              ConversionSpace s;
              if (assigned_at_post) {
                dom(s, s.source.bits(), b.source | sign);
                dom(s, s.result.bits(), valid ? expected : (expected ^ 1));
              }
              fp_convert(s, Policy{mode}, s.source, s.result);
              if (!assigned_at_post) {
                dom(s, s.source.bits(), b.source | sign);
                dom(s, s.result.bits(), valid ? expected : (expected ^ 1));
              }
              if ((s.status() != SS_FAILED) != valid)
                return false;
            }
          }
    return true;
  }
};

using Tuple = std::array<WordValue, 3>;
class ConversionNetwork : public Space {
public:
  WordVarArray words;
  ConversionNetwork(WordDomainType source_kind, WordDomainType result_kind, int later = 0)
      : words(*this, 3) {
    words[0] = WordVar(*this, 32, source_kind);
    dom(*this, words[0], WordValue(0x3f800000), WordValue(0x3f80f800));
    rel(*this, words[0], WRT_ULQ, WordValue(0x3f80e000));
    for (int i = 1; i < 3; ++i) {
      words[i] = WordVar(*this, 16, result_kind);
      fp_convert(*this, Policy{i == 1 ? Policy::RNE : Policy::RTZ},
                 FloatWordVar(FWF_FP32, words[0]), FloatWordVar(FWF_BINARY16, words[i]));
    }
    rel(*this, words[1], WRT_NQ, words[2]);
    restrict_result(later);
    branch(*this, words, WORD_VAR_NONE(), WORD_VAL_LSB());
  }
  ConversionNetwork(ConversionNetwork &other) : Space(other) { words.update(*this, other.words); }
  Space *copy() override { return new ConversionNetwork(*this); }
  void restrict_result(int later) {
    if (later == 1)
      dom(*this, words[1], WordValue(1), WordValue(0xffff));
    if (later == 2) {
      rel(*this, words[1], WRT_UGQ, WordValue(0x3c01));
      rel(*this, words[1], WRT_ULQ, WordValue(0x3c06));
    }
  }
};

std::vector<Tuple> solutions(ConversionNetwork &space) {
  DFS<ConversionNetwork> search(&space);
  std::vector<Tuple> result;
  while (std::unique_ptr<ConversionNetwork> solution{search.next()})
    result.push_back(
        {solution->words[0].val(), solution->words[1].val(), solution->words[2].val()});
  std::sort(result.begin(), result.end());
  return result;
}

bool same_domains(ConversionNetwork &a, ConversionNetwork &b) {
  for (int i = 0; i < 3; ++i) {
    if (a.words[i].lo() != b.words[i].lo() || a.words[i].hi() != b.words[i].hi())
      return false;
    if (a.words[i].bounded() && (a.words[i].minimum() != b.words[i].minimum() ||
                                 a.words[i].maximum() != b.words[i].maximum()))
      return false;
  }
  return true;
}

class ConversionLifecycle : public Base {
  static bool connected() {
    const std::vector<Tuple> expected{{0x3f801800, 0x3c01, 0x3c00}, {0x3f803000, 0x3c02, 0x3c01},
                                      {0x3f803800, 0x3c02, 0x3c01}, {0x3f805800, 0x3c03, 0x3c02},
                                      {0x3f807000, 0x3c04, 0x3c03}, {0x3f807800, 0x3c04, 0x3c03},
                                      {0x3f809800, 0x3c05, 0x3c04}, {0x3f80b000, 0x3c06, 0x3c05},
                                      {0x3f80b800, 0x3c06, 0x3c05}, {0x3f80d800, 0x3c07, 0x3c06}};
    for (WordDomainType source_kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED})
      for (WordDomainType result_kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED})
        for (int later : {1, 2}) {
          ConversionNetwork original(source_kind, result_kind);
          if (original.status() == SS_FAILED || solutions(original) != expected)
            return false;
          std::unique_ptr<ConversionNetwork> clone(
              static_cast<ConversionNetwork *>(original.clone()));
          clone->restrict_result(later);
          ConversionNetwork fresh(source_kind, result_kind, later);
          ConversionNetwork unchanged(source_kind, result_kind);
          if (clone->status() == SS_FAILED || fresh.status() == SS_FAILED ||
              unchanged.status() == SS_FAILED || !same_domains(*clone, fresh) ||
              !same_domains(original, unchanged))
            return false;
          std::vector<Tuple> restricted;
          for (const Tuple &tuple : expected)
            if (later == 1 ? (tuple[1] & 1) != 0 : tuple[1] >= 0x3c01 && tuple[1] <= 0x3c06)
              restricted.push_back(tuple);
          if (solutions(*clone) != restricted || solutions(fresh) != restricted ||
              solutions(original) != expected)
            return false;
        }
    return true;
  }
  static bool signed_and_bounds() {
    // A signed source spans both sign partitions; its endpoint encodings are
    // deliberately in the opposite order from an unsigned interval.
    for (WordDomainType kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
      ConversionSpace s(WDT_SIGNED, kind);
      dom(s, s.source.bits(), WordValue(0x3f801000), WordValue(0xbf801001));
      fp_convert(s, Policy(), s.source, s.result);
      branch(s, WordVarArgs{s.source.bits(), s.result.bits()}, WORD_VAL_LSB());
      if (s.status() == SS_FAILED || s.source.bits().minimum() != 0xbf801000 ||
          s.source.bits().maximum() != 0x3f801001)
        return false;
      DFS<ConversionSpace> search(&s);
      int count = 0;
      while (std::unique_ptr<ConversionSpace> solution{search.next()}) {
        const WordValue bits = solution->source.bits().val();
        const WordValue expected = ((bits >> 16) & 0x8000) | (0x3c00 + (bits & 1));
        if (solution->result.bits().val() != expected)
          return false;
        ++count;
      }
      if (count != 4)
        return false;
    }
    ConversionNetwork s(WDT_UNSIGNED, WDT_UNSIGNED);
    if (s.status() == SS_FAILED)
      return false;
    Gecode::Word::WordView result(s.words[1]);
    const WordValue low = result.lo(), high = result.hi();
    const ModEvent event = result.narrow_rank_range(s, 0x3c01, 0x3c06);
    if (me_failed(event) || !me_modified(event) || result.lo() != low || result.hi() != high)
      return false;
    if (s.status() == SS_FAILED || s.words[0].maximum() != 0x3f80d000)
      return false;
    return true;
  }

public:
  ConversionLifecycle() : Base("Word::Float::ConversionLifecycle") {}
  bool run() override { return connected() && signed_and_bounds(); }
};

class ConversionErrors : public Base {
public:
  ConversionErrors() : Base("Word::Float::ConversionErrors") {}
  bool run() override {
    ConversionSpace s;
    try {
      // A tiny result needs its encoding binding.
      fp_convert(s, Policy(), s.source, FloatWordVar(FWF_E2M1, WordVar(s, 4)));
      return false;
    } catch (const Gecode::Word::OutOfLimits &) {
    }
    // Two postings of the same pair both keep the negative zero.
    fp_convert(s, Policy(), s.source, s.result);
    // Fields without an effect on finite conversion do not restrict it.
    fp_convert(s,
               Policy{Policy::RNE, Policy::Preserve, Policy::Preserve, Policy::Format,
                      Policy::Signed, Policy::Canonical, Policy::BeforeRounding,
                      Policy::Signaling},
               s.source, s.result);
    dom(s, s.source.bits(), WordValue(0x80000000));
    return s.status() != SS_FAILED && s.result.bits().val() == 0x8000;
  }
};

// Independent oracle: search the destination's neighboring mathematical
// values and compare the exact input to their midpoint. This does not use
// production rounding, conversion or preimage code.
struct OracleFormat {
  unsigned int width, fraction;
  int bias;
  WordValue maximum, infinity, nan;
};
OracleFormat oracle_format(FloatWordFormat format) {
  switch (format) {
  case FWF_FP32:
    return {32, 23, 127, 0x7f7fffff, 0x7f800000, 0x7fc00000};
  case FWF_BF16:
    return {16, 7, 127, 0x7f7f, 0x7f80, 0x7fc0};
  case FWF_BINARY16:
    return {16, 10, 15, 0x7bff, 0x7c00, 0x7e00};
  case FWF_E4M3FN:
    return {8, 3, 7, 126, 0, 127};
  case FWF_E2M1:
    return {4, 1, 1, 7, 0, 0};
  default:
    throw Gecode::Word::OutOfLimits("conversion oracle format");
  }
}
struct Rational {
  WordValue numerator;
  int power;
};
Rational rational(WordValue code, OracleFormat format) {
  const WordValue exponent = code >> format.fraction;
  return {exponent
              ? (WordValue(1) << format.fraction) + (code & ((WordValue(1) << format.fraction) - 1))
              : code,
          (exponent ? static_cast<int>(exponent) - format.bias : 1 - format.bias) -
              static_cast<int>(format.fraction)};
}
int rational_compare(Rational a, Rational b) {
  // Normalize to odd numerators first. Equal leading exponents require
  // aligning at most 24 bits, independently of the encoded exponent range.
  if (!a.numerator || !b.numerator)
    return (a.numerator != 0) - (b.numerator != 0);
  while (!(a.numerator & 1)) {
    a.numerator >>= 1;
    ++a.power;
  }
  while (!(b.numerator & 1)) {
    b.numerator >>= 1;
    ++b.power;
  }
  if (a.power > b.power) {
    if (a.power - b.power >= 32)
      return 1;
    a.numerator <<= a.power - b.power;
  } else {
    if (b.power - a.power >= 32)
      return -1;
    b.numerator <<= b.power - a.power;
  }
  return (a.numerator > b.numerator) - (a.numerator < b.numerator);
}
Rational midpoint(Rational a, Rational b) {
  const int power = std::min(a.power, b.power);
  return {(a.numerator << (a.power - power)) + (b.numerator << (b.power - power)), power - 1};
}
struct ExpectedConversion {
  WordValue bits;
  int flags;
};
// The conversion of a value too large for format to, with result sign bit sign.
ExpectedConversion overflow(OracleFormat to, const Policy &policy, WordValue sign) {
  const bool negative = sign != 0;
  if (policy.overflow == Policy::SaturateFinite || !to.nan)
    return {sign | to.maximum, 20};
  if (!to.infinity)
    return {(policy.nan == Policy::Canonical ? 0 : sign) | to.nan, 20};
  const bool infinite = policy.rounding == Policy::RNE ||
                        (policy.rounding == Policy::RUP && !negative) ||
                        (policy.rounding == Policy::RDN && negative);
  return {sign | (infinite ? to.infinity : to.maximum), 20};
}
ExpectedConversion oracle(WordValue bits, FloatWordFormat source_format,
                          FloatWordFormat result_format, const Policy &policy) {
  const auto from = oracle_format(source_format), to = oracle_format(result_format);
  const WordValue source_sign = WordValue(1) << (from.width - 1);
  const bool negative = (bits & source_sign) != 0;
  const WordValue sign = negative ? WordValue(1) << (to.width - 1) : 0;
  const WordValue magnitude = bits & (source_sign - 1);
  if (from.nan && magnitude > (from.infinity ? from.infinity : from.maximum)) {
    if (!to.nan)
      return {7, 1};
    const int invalid = from.infinity && !(magnitude & (WordValue(1) << (from.fraction - 1)));
    if (policy.nan == Policy::Canonical)
      return {to.nan, invalid};
    WordValue payload = 0;
    if (from.infinity && to.infinity)
      for (unsigned int bit = 1; bit < std::min(from.fraction, to.fraction); ++bit)
        if (magnitude & (WordValue(1) << (from.fraction - 1 - bit)))
          payload |= WordValue(1) << (to.fraction - 1 - bit);
    return {sign | to.nan | payload, invalid};
  }
  if (from.infinity && magnitude == from.infinity)
    return to.infinity ? ExpectedConversion{sign | to.infinity, 0} : overflow(to, policy, sign);
  if (!magnitude || (policy.input_subnormals == Policy::SignedZero &&
                     magnitude < (WordValue(1) << from.fraction)))
    return {policy.zeros == Policy::Positive ? 0 : sign, 0};
  const Rational exact = rational(magnitude, from);
  WordValue lo = 0, hi = to.maximum + 1;
  while (lo < hi) {
    const WordValue middle = (lo + hi + 1) / 2;
    if (rational_compare(rational(middle, to), exact) <= 0)
      lo = middle;
    else
      hi = middle - 1;
  }
  if (lo > to.maximum)
    return overflow(to, policy, sign);
  WordValue selected = lo;
  const bool exact_lower = rational_compare(exact, rational(lo, to)) == 0;
  if (!exact_lower) {
    if ((policy.rounding == Policy::RUP && !negative) ||
        (policy.rounding == Policy::RDN && negative))
      ++selected;
    else if (policy.rounding == Policy::RNE) {
      const int side = rational_compare(exact, midpoint(rational(lo, to), rational(lo + 1, to)));
      selected += side > 0 || (side == 0 && (lo & 1));
    }
  }
  if (selected > to.maximum)
    return overflow(to, policy, sign);
  bool tiny;
  if (policy.tininess == Policy::BeforeRounding || policy.rounding == Policy::RTZ ||
      (policy.rounding == Policy::RUP && negative) ||
      (policy.rounding == Policy::RDN && !negative)) {
    tiny = rational_compare(exact, {1, 1 - to.bias}) < 0;
  } else if (policy.rounding == Policy::RNE) {
    tiny = rational_compare(exact, {(WordValue(1) << (to.fraction + 2)) - 1,
                                    1 - to.bias - static_cast<int>(to.fraction) - 2}) < 0;
  } else {
    tiny = rational_compare(exact, {(WordValue(1) << (to.fraction + 1)) - 1,
                                    1 - to.bias - static_cast<int>(to.fraction) - 1}) <= 0;
  }
  int flags = rational_compare(exact, rational(selected, to)) == 0 ? 0 : (tiny ? 24 : 16);
  if (policy.output_subnormals == Policy::SignedZero && selected &&
      selected < (WordValue(1) << to.fraction)) {
    selected = 0;
    flags = 24;
  }
  return {selected | ((!selected && policy.zeros == Policy::Positive) ? 0 : sign), flags};
}

FloatWordVar conversion_var(Space &home, FloatWordFormat format, WordDomainType kind) {
  const unsigned int width = float_word_width(format);
  WordVar bits(home, width, kind);
  return width <= 8 ? FloatWordVar(home, format, bits, IntVar(home, 0, (1 << width) - 1))
                    : FloatWordVar(format, bits);
}
class MixedSpace : public Space {
public:
  FloatWordVar source, result;
  IntVar flags;
  MixedSpace(FloatWordFormat from, FloatWordFormat to, WordDomainType sk = WDT_UNSIGNED,
             WordDomainType rk = WDT_UNSIGNED)
      : source(conversion_var(*this, from, sk)), result(conversion_var(*this, to, rk)),
        flags(*this, 0, 31) {}
  MixedSpace(MixedSpace &other) : Space(other) {
    source.update(*this, other.source);
    result.update(*this, other.result);
    flags.update(*this, other.flags);
  }
  Space *copy() override { return new MixedSpace(*this); }
};

Policy scalar_policy(Policy::Rounding mode, int variant) {
  return Policy{mode,
                variant == 1 ? Policy::SignedZero : Policy::Preserve,
                variant == 2 ? Policy::SignedZero : Policy::Preserve,
                variant == 3 ? Policy::SaturateFinite : Policy::Format,
                variant == 3 ? Policy::Positive : Policy::Signed,
                variant == 3   ? Policy::Canonical
                : variant == 2 ? Policy::OperandFirst
                               : Policy::SignalFirst,
                variant == 3 ? Policy::BeforeRounding : Policy::AfterPrecision,
                variant == 2 ? Policy::Signaling : Policy::Quiet};
}

bool assigned_conversion(WordValue input, FloatWordFormat from, FloatWordFormat to,
                         const Policy &policy, ExpectedConversion expected) {
  MixedSpace space(from, to);
  dom(space, space.source.bits(), input);
  fp_convert(space, policy, space.source, space.result, space.flags);
  if (space.status() == SS_FAILED || !space.result.bits().assigned() || !space.flags.assigned() ||
      space.result.bits().val() != expected.bits || space.flags.val() != expected.flags) {
    if (opt.log)
      olog << "conversion " << from << " -> " << to << " input " << input << " expected "
           << expected.bits << '/' << expected.flags << " mode " << policy.rounding << '\n';
    return false;
  }
  return true;
}

class MixedConversionOracle : public Base {
public:
  MixedConversionOracle() : Base("Word::Float::ConversionMixedOracle") {}
  bool run() override {
    const FloatWordFormat formats[] = {FWF_FP32, FWF_BF16, FWF_BINARY16, FWF_E4M3FN, FWF_E2M1};
    for (auto from : {FWF_E4M3FN, FWF_E2M1})
      for (auto to : formats)
        for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
          for (int variant = 0; variant < 4; ++variant) {
            const Policy policy = scalar_policy(mode, variant);
            for (WordValue bits = 0; bits < (WordValue(1) << float_word_width(from)); ++bits)
              if (!assigned_conversion(bits, from, to, policy, oracle(bits, from, to, policy)))
                return false;
          }
    const WordValue cases[] = {
        0,          1,          0x007f7fff, 0x007f8000, 0x007f8001, 0x007fc000,
        0x007fc001, 0x007fffff, 0x00800000, 0x00800001, 0x3affffff, 0x3b000000,
        0x3b000001, 0x3f7fffff, 0x3f800000, 0x3f807fff, 0x3f808000, 0x3f808001,
        0x3f818000, 0x3f880001, 0x40dfffff, 0x40e00000, 0x40e00001, 0x43e7ffff,
        0x43e80000, 0x43e80001, 0x7f7f0000, 0x7f7f7fff, 0x7f7f8000, 0x7f7fffff,
        0x7f800000, 0x7f800001, 0x7fbfffff, 0x7fc00000, 0x7fc12345, 0x7fffffff};
    for (auto from : {FWF_FP32, FWF_BF16})
      for (auto to : formats)
        for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
          for (int variant = 0; variant < 4; ++variant) {
            const Policy policy = scalar_policy(mode, variant);
            for (WordValue raw : cases)
              for (WordValue sign : {WordValue(0), WordValue(0x80000000)}) {
                const WordValue bits = (raw | sign) >> (from == FWF_BF16 ? 16 : 0);
                if (!assigned_conversion(bits, from, to, policy, oracle(bits, from, to, policy)))
                  return false;
              }
          }
    for (auto to : formats)
      for (WordValue bits = 0; bits <= 0xffff; ++bits)
        if (!assigned_conversion(bits, FWF_BINARY16, to, Policy(),
                                 oracle(bits, FWF_BINARY16, to, Policy())))
          return false;
    // Binary16 sources at destination ties and their neighbours.
    const WordValue half_cases[] = {
        0,      1,      2,      0x3ff,  0x400,  0x3c00, 0x3c03, 0x3c04, 0x3c05, 0x3c0c,
        0x3c40, 0x3c41, 0x3cc0, 0x3d00, 0x3d01, 0x3f00, 0x4600, 0x4700, 0x5f00, 0x5f80,
        0x7bff, 0x7c00, 0x7c01, 0x7dff, 0x7e00, 0x7e01, 0x7fff};
    // FP32 sources around binary16 subnormal, normal, overflow and NaN boundaries.
    const WordValue half_boundaries[] = {
        0,          1,          0x007fffff, 0x00800000, 0x33000000, 0x33800000, 0x33c00000,
        0x387fc000, 0x387fe000, 0x387ff000, 0x38800000, 0x3f801000, 0x3f803000, 0x477fe000,
        0x477ff000, 0x47800000, 0x7f7fffff, 0x7f800000, 0x7f800001, 0x7f801fff, 0x7f802000,
        0x7fbfffff, 0x7fc00000, 0x7fc02000, 0x7fffffff};
    for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
      for (int variant = 0; variant < 4; ++variant) {
        const Policy policy = scalar_policy(mode, variant);
        for (auto to : formats)
          for (WordValue raw : half_cases)
            for (WordValue bits : {raw, raw | 0x8000})
              if (!assigned_conversion(bits, FWF_BINARY16, to, policy,
                                       oracle(bits, FWF_BINARY16, to, policy)))
                return false;
        for (WordValue boundary : half_boundaries)
          for (WordValue raw = boundary < 2 ? 0 : boundary - 2;
               raw <= boundary + 2 && raw <= 0x7fffffff; ++raw)
            for (WordValue bits : {raw, raw | 0x80000000})
              if (!assigned_conversion(bits, FWF_FP32, FWF_BINARY16, policy,
                                       oracle(bits, FWF_FP32, FWF_BINARY16, policy)))
                return false;
      }
    // Fixed expected values also guard the independent oracle's range rules.
    return assigned_conversion(0x007f8000, FWF_FP32, FWF_BF16, Policy(), {0x80, 24}) &&
           assigned_conversion(0x007fc001, FWF_FP32, FWF_BF16, Policy(), {0x80, 16}) &&
           assigned_conversion(0x007fc001, FWF_FP32, FWF_BF16, scalar_policy(Policy::RNE, 3),
                               {0x80, 24}) &&
           assigned_conversion(0x43e80000, FWF_FP32, FWF_E4M3FN, Policy(), {0x7e, 16}) &&
           assigned_conversion(0x40e00000, FWF_FP32, FWF_E2M1, Policy(), {7, 20}) &&
           assigned_conversion(0x40e00000, FWF_FP32, FWF_E2M1, scalar_policy(Policy::RTZ, 0),
                               {7, 16}) &&
           assigned_conversion(0x3f880001, FWF_FP32, FWF_E4M3FN, Policy(), {0x39, 16}) &&
           assigned_conversion(0x387ff000, FWF_FP32, FWF_BINARY16, Policy(), {0x0400, 16}) &&
           assigned_conversion(0x387ff000, FWF_FP32, FWF_BINARY16, scalar_policy(Policy::RNE, 3),
                               {0x0400, 24}) &&
           assigned_conversion(0x477ff000, FWF_FP32, FWF_BINARY16, Policy(), {0x7c00, 20}) &&
           assigned_conversion(0x477ff000, FWF_FP32, FWF_BINARY16, scalar_policy(Policy::RTZ, 0),
                               {0x7bff, 16}) &&
           assigned_conversion(0x33000000, FWF_FP32, FWF_BINARY16, Policy(), {0, 24}) &&
           assigned_conversion(0x33000000, FWF_FP32, FWF_BINARY16, scalar_policy(Policy::RUP, 0),
                               {0x0001, 24}) &&
           assigned_conversion(0x7f802001, FWF_FP32, FWF_BINARY16, Policy(), {0x7e01, 1}) &&
           assigned_conversion(0x7c01, FWF_BINARY16, FWF_FP32, Policy(), {0x7fc02000, 1}) &&
           assigned_conversion(0x0001, FWF_BINARY16, FWF_FP32, Policy(), {0x33800000, 0}) &&
           assigned_conversion(0x7bff, FWF_BINARY16, FWF_E4M3FN, Policy(), {0x7f, 20}) &&
           assigned_conversion(0x7bff, FWF_BINARY16, FWF_E4M3FN, scalar_policy(Policy::RNE, 3),
                               {0x7e, 20});
  }
};

std::vector<Tuple> mixed_solutions(MixedSpace &space) {
  branch(space, WordVarArgs{space.source.bits(), space.result.bits()}, WORD_VAL_LSB());
  branch(space, space.flags, INT_VAL_MIN());
  DFS<MixedSpace> search(&space);
  std::vector<Tuple> result;
  while (std::unique_ptr<MixedSpace> solution{search.next()})
    result.push_back({solution->source.bits().val(), solution->result.bits().val(),
                      static_cast<WordValue>(solution->flags.val())});
  std::sort(result.begin(), result.end());
  return result;
}

class MixedConversionInverse : public Base {
public:
  MixedConversionInverse() : Base("Word::Float::ConversionMixedInverse") {}
  bool run() override {
    struct Window {
      WordValue first, last;
      FloatWordFormat target;
    };
    const Window windows[] = {{0x007f7ffc, 0x007f8004, FWF_BF16},
                              {0x007fbffc, 0x007fc004, FWF_BF16},
                              {0x3f807ffc, 0x3f808004, FWF_BF16},
                              {0x43e7fffc, 0x43e80004, FWF_E4M3FN},
                              {0x40dffffc, 0x40e00004, FWF_E2M1},
                              {0x7f7f7ffc, 0x7f7f8004, FWF_BF16},
                              {0x7f7ffffc, 0x7f800004, FWF_BF16},
                              {0x7fbffffc, 0x7fc00004, FWF_BF16},
                              {0x7f7ffffc, 0x7f800004, FWF_E2M1},
                              {0x7fbffffc, 0x7fc00004, FWF_E4M3FN},
                              {0, 8, FWF_E2M1},
                              {0x3f7ffffc, 0x3f800004, FWF_BF16},
                              {0x337ffffc, 0x33800004, FWF_BINARY16},
                              {0x387fdffc, 0x387fe004, FWF_BINARY16},
                              {0x477feffc, 0x477ff004, FWF_BINARY16},
                              {0x7f7ffffc, 0x7f800004, FWF_BINARY16},
                              {0x7f801ffc, 0x7f802004, FWF_BINARY16},
                              {0x7fbffffc, 0x7fc00004, FWF_BINARY16}};
    for (const auto &window : windows)
      for (WordValue sign : {WordValue(0), WordValue(0x80000000)})
        for (auto kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED})
          for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
            for (int variant = 0; variant < 4; ++variant) {
              const Policy policy = scalar_policy(mode, variant);
              MixedSpace space(FWF_FP32, window.target, kind,
                               kind == WDT_SIGNED ? WDT_CUBE : WDT_SIGNED);
              rel(space, space.source.bits(), WRT_UGQ, sign | window.first);
              rel(space, space.source.bits(), WRT_ULQ, sign | window.last);
              fp_convert(space, policy, space.source, space.result, space.flags);
              std::vector<Tuple> expected;
              for (WordValue bits = window.first; bits <= window.last; ++bits) {
                const auto value = oracle(sign | bits, FWF_FP32, window.target, policy);
                expected.push_back({sign | bits, value.bits, static_cast<WordValue>(value.flags)});
              }
              if (mixed_solutions(space) != expected)
                return false;
            }
    // Every directed pair also receives symbolic source and result domains.
    for (auto from : {FWF_FP32, FWF_BF16, FWF_BINARY16, FWF_E4M3FN, FWF_E2M1})
      for (auto to : {FWF_FP32, FWF_BF16, FWF_BINARY16, FWF_E4M3FN, FWF_E2M1}) {
        const WordValue base = from == FWF_FP32       ? 0x3f800000
                               : from == FWF_BF16     ? 0x3f80
                               : from == FWF_BINARY16 ? 0x3c00
                               : from == FWF_E4M3FN   ? 0x38
                                                      : 0;
        const WordValue sign = WordValue(1) << (float_word_width(from) - 1);
        MixedSpace symbolic(from, to, WDT_SIGNED);
        dom(symbolic, symbolic.source.bits(), base, base | sign | 3);
        fp_convert(symbolic, Policy(), symbolic.source, symbolic.result, symbolic.flags);
        std::vector<Tuple> expected;
        for (WordValue negative : {WordValue(0), sign})
          for (WordValue low = 0; low < 4; ++low) {
            const WordValue bits = base | negative | low;
            const auto value = oracle(bits, from, to, Policy());
            expected.push_back({bits, value.bits, static_cast<WordValue>(value.flags)});
          }
        if (mixed_solutions(symbolic) != expected)
          return false;
      }
    // A maximum result has exact, inexact and overflowing sources. Restrict
    // the flags after a stable propagation, without changing Int bounds.
    MixedSpace space(FWF_FP32, FWF_E2M1);
    dom(space, space.result.bits(), WordValue(7));
    fp_convert(space, Policy(), space.source, space.result, space.flags);
    if (space.status() == SS_FAILED || !space.flags.in(0) || !space.flags.in(16) ||
        !space.flags.in(20))
      return false;
    std::unique_ptr<MixedSpace> clone(static_cast<MixedSpace *>(space.clone()));
    rel(*clone, clone->flags, IRT_NQ, 16);
    if (clone->status() == SS_FAILED)
      return false;
    // The Word hull can retain holes; forcing such a hole must still fail.
    dom(*clone, clone->source.bits(), WordValue(0x40c00001));
    if (clone->status() != SS_FAILED || space.status() == SS_FAILED || !space.flags.in(16))
      return false;
    // The same holds at the binary16 maximum under RTZ.
    MixedSpace half(FWF_FP32, FWF_BINARY16);
    dom(half, half.result.bits(), WordValue(0x7bff));
    fp_convert(half, scalar_policy(Policy::RTZ, 0), half.source, half.result, half.flags);
    if (half.status() == SS_FAILED || !half.flags.in(0) || !half.flags.in(16) ||
        !half.flags.in(20))
      return false;
    std::unique_ptr<MixedSpace> half_clone(static_cast<MixedSpace *>(half.clone()));
    rel(*half_clone, half_clone->flags, IRT_NQ, 16);
    if (half_clone->status() == SS_FAILED)
      return false;
    dom(*half_clone, half_clone->source.bits(), WordValue(0x477fe001));
    if (half_clone->status() != SS_FAILED || half.status() == SS_FAILED || !half.flags.in(16))
      return false;
    MixedSpace nan(FWF_FP32, FWF_E2M1);
    dom(nan, nan.result.bits(), WordValue(7));
    rel(nan, nan.flags, IRT_EQ, 1);
    fp_convert(nan, Policy(), nan.source, nan.result, nan.flags);
    if (nan.status() == SS_FAILED || !nan.source.bits().in(0xff800001) ||
        !nan.source.bits().in(0x7fc00000) || nan.source.bits().in(0x7f800000))
      return false;
    MixedSpace exact(FWF_FP32, FWF_BF16);
    dom(exact, exact.result.bits(), WordValue(0x3f80));
    rel(exact, exact.flags, IRT_EQ, 0);
    fp_convert(exact, Policy(), exact.source, exact.result, exact.flags);
    if (exact.status() == SS_FAILED || !exact.source.bits().assigned() ||
        exact.source.bits().val() != 0x3f800000)
      return false;
    // Of the binary16 sources that round to BF16 one, a later exact status keeps 1.0.
    MixedSpace widened(FWF_BINARY16, FWF_BF16);
    dom(widened, widened.result.bits(), WordValue(0x3f80));
    fp_convert(widened, Policy(), widened.source, widened.result, widened.flags);
    if (widened.status() == SS_FAILED || widened.source.bits().assigned())
      return false;
    rel(widened, widened.flags, IRT_EQ, 0);
    if (widened.status() == SS_FAILED || !widened.source.bits().assigned() ||
        widened.source.bits().val() != 0x3c00)
      return false;
    // E4M3FN overflow gives a canonical NaN without the source sign, so a
    // result restricted to that NaN keeps the overflowing negative sources.
    Policy canonical;
    canonical.nan = Policy::Canonical;
    for (auto kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
      MixedSpace overflow(FWF_FP32, FWF_E4M3FN, kind, kind);
      rel(overflow, overflow.source.bits(), WRT_UGQ, WordValue(0xc3e7fffc));
      rel(overflow, overflow.source.bits(), WRT_ULQ, WordValue(0xc3e80004));
      dom(overflow, overflow.result.bits(), WordValue(0x7f));
      fp_convert(overflow, canonical, overflow.source, overflow.result, overflow.flags);
      std::vector<Tuple> expected;
      for (WordValue bits = 0xc3e7fffc; bits <= 0xc3e80004; ++bits) {
        const auto value = oracle(bits, FWF_FP32, FWF_E4M3FN, canonical);
        if (value.bits == 0x7f)
          expected.push_back({bits, value.bits, static_cast<WordValue>(value.flags)});
      }
      if (expected.empty() || mixed_solutions(overflow) != expected)
        return false;
    }
    // Positive zero must narrow the source sign, including a signed domain.
    MixedSpace zero(FWF_FP32, FWF_BF16, WDT_SIGNED);
    dom(zero, zero.result.bits(), WordValue(0x8000));
    fp_convert(zero, scalar_policy(Policy::RNE, 3), zero.source, zero.result, zero.flags);
    return zero.status() == SS_FAILED;
  }
};

class MixedConversionNetwork : public Space {
public:
  WordVarArray words;
  MixedConversionNetwork(WordDomainType kind, bool later = false) : words(*this, 3) {
    words[0] = WordVar(*this, 32, kind);
    dom(*this, words[0], WordValue(0x3f800000), WordValue(0xbf83c000));
    for (int i = 1; i < 3; ++i) {
      words[i] = WordVar(*this, 16, kind == WDT_SIGNED ? WDT_UNSIGNED : WDT_SIGNED);
      fp_convert(*this, Policy{i == 1 ? Policy::RNE : Policy::RTZ},
                 FloatWordVar(FWF_FP32, words[0]), FloatWordVar(FWF_BF16, words[i]));
    }
    rel(*this, words[1], WRT_NQ, words[2]);
    if (later)
      restrict_result();
    branch(*this, words, WORD_VAR_NONE(), WORD_VAL_LSB());
  }
  MixedConversionNetwork(MixedConversionNetwork &other) : Space(other) {
    words.update(*this, other.words);
  }
  Space *copy() override { return new MixedConversionNetwork(*this); }
  void restrict_result() { dom(*this, words[1], WordValue(1), WordValue(0xffff)); }
};
std::vector<Tuple> network_solutions(MixedConversionNetwork &space) {
  DFS<MixedConversionNetwork> search(&space);
  std::vector<Tuple> result;
  while (std::unique_ptr<MixedConversionNetwork> solution{search.next()})
    result.push_back(
        {solution->words[0].val(), solution->words[1].val(), solution->words[2].val()});
  std::sort(result.begin(), result.end());
  return result;
}

class MixedConversionLifecycle : public Base {
  static bool connected() {
    std::vector<Tuple> expected, restricted;
    for (WordValue sign : {WordValue(0), WordValue(0x80000000)})
      for (WordValue i = 0; i < 16; ++i) {
        const WordValue input = sign | 0x3f800000 | (i << 14);
        const WordValue nearest = oracle(input, FWF_FP32, FWF_BF16, Policy()).bits;
        const WordValue truncated =
            oracle(input, FWF_FP32, FWF_BF16, scalar_policy(Policy::RTZ, 0)).bits;
        if (nearest != truncated) {
          expected.push_back({input, nearest, truncated});
          if (nearest & 1)
            restricted.push_back({input, nearest, truncated});
        }
      }
    for (auto kind : {WDT_CUBE, WDT_UNSIGNED, WDT_SIGNED}) {
      MixedConversionNetwork original(kind);
      if (original.status() == SS_FAILED || network_solutions(original) != expected)
        return false;
      std::unique_ptr<MixedConversionNetwork> clone(
          static_cast<MixedConversionNetwork *>(original.clone()));
      clone->restrict_result();
      MixedConversionNetwork fresh(kind, true);
      if (clone->status() == SS_FAILED || fresh.status() == SS_FAILED)
        return false;
      for (int i = 0; i < 3; ++i)
        if (clone->words[i].lo() != fresh.words[i].lo() ||
            clone->words[i].hi() != fresh.words[i].hi() ||
            (clone->words[i].bounded() && (clone->words[i].minimum() != fresh.words[i].minimum() ||
                                           clone->words[i].maximum() != fresh.words[i].maximum())))
          return false;
      if (network_solutions(*clone) != restricted || network_solutions(fresh) != restricted ||
          network_solutions(original) != expected)
        return false;
    }
    return true;
  }
  static bool self_aliases() {
    for (const Policy &policy : {Policy(), scalar_policy(Policy::RNE, 1),
                                 scalar_policy(Policy::RNE, 2), scalar_policy(Policy::RNE, 3)})
      for (WordValue bits : {WordValue(0), WordValue(0x80000000), WordValue(1),
                             WordValue(0x7f800001), WordValue(0xffc12345)}) {
        MixedSpace space(FWF_FP32, FWF_FP32);
        fp_convert(space, policy, space.source, space.source, space.flags);
        dom(space, space.source.bits(), bits);
        const auto expected = oracle(bits, FWF_FP32, FWF_FP32, policy);
        if ((space.status() != SS_FAILED) != (bits == expected.bits))
          return false;
        if (bits == expected.bits && space.flags.val() != expected.flags)
          return false;
      }
    for (const Policy &policy : {Policy(), scalar_policy(Policy::RNE, 1),
                                 scalar_policy(Policy::RNE, 2), scalar_policy(Policy::RNE, 3)})
      for (WordValue bits :
           {WordValue(0), WordValue(0x8000), WordValue(1), WordValue(0x7c01), WordValue(0xfe12)}) {
        MixedSpace space(FWF_BINARY16, FWF_BINARY16);
        fp_convert(space, policy, space.source, space.source, space.flags);
        dom(space, space.source.bits(), bits);
        const auto expected = oracle(bits, FWF_BINARY16, FWF_BINARY16, policy);
        if ((space.status() != SS_FAILED) != (bits == expected.bits))
          return false;
        if (bits == expected.bits && space.flags.val() != expected.flags)
          return false;
      }
    MixedSpace same(FWF_E2M1, FWF_E2M1);
    fp_convert(same, Policy(), same.source, same.source, same.source.encoding());
    if (same.status() == SS_FAILED || !same.source.bits().assigned() ||
        same.source.bits().val() != 0)
      return false;
    MixedSpace codes(FWF_E4M3FN, FWF_E2M1);
    FloatWordVar shared(codes, FWF_E2M1, codes.result.bits(), codes.source.encoding());
    fp_convert(codes, Policy(), codes.source, shared);
    if (codes.status() == SS_FAILED || !codes.source.bits().assigned() ||
        codes.source.bits().val() != 0 || shared.bits().val() != 0)
      return false;
    MixedSpace flags(FWF_E4M3FN, FWF_E2M1);
    fp_convert(flags, Policy(), flags.source, flags.result, flags.result.encoding());
    if (flags.status() == SS_FAILED || !flags.result.bits().assigned() ||
        flags.result.bits().val() != 0 || flags.source.bits().val() != 0)
      return false;
    return true;
  }
  static void restrict_tiny(MixedSpace &space, bool remove) {
    dom(space, space.source.encoding(), IntSet({0x38, 0x39, 0x3a, 0x3b}));
    fp_convert(space, Policy(), space.source, space.result, space.flags);
    if (remove)
      rel(space, space.source.encoding(), IRT_NQ, 0x39);
  }
  static bool tiny_clone() {
    MixedSpace original(FWF_E4M3FN, FWF_BF16);
    restrict_tiny(original, false);
    if (original.status() == SS_FAILED)
      return false;
    std::unique_ptr<MixedSpace> clone(static_cast<MixedSpace *>(original.clone()));
    rel(*clone, clone->source.encoding(), IRT_NQ, 0x39);
    MixedSpace fresh(FWF_E4M3FN, FWF_BF16);
    restrict_tiny(fresh, true);
    if (clone->status() == SS_FAILED || fresh.status() == SS_FAILED ||
        !original.source.encoding().in(0x39))
      return false;
    if (clone->source.bits().lo() != fresh.source.bits().lo() ||
        clone->source.bits().hi() != fresh.source.bits().hi() ||
        clone->result.bits().minimum() != fresh.result.bits().minimum() ||
        clone->result.bits().maximum() != fresh.result.bits().maximum())
      return false;
    const std::vector<Tuple> expected{{0x38, 0x3f80, 0}, {0x3a, 0x3fa0, 0}, {0x3b, 0x3fb0, 0}};
    if (mixed_solutions(*clone) != expected || mixed_solutions(fresh) != expected)
      return false;
    std::vector<Tuple> unmodified = expected;
    unmodified.insert(unmodified.begin() + 1, {0x39, 0x3f90, 0});
    return mixed_solutions(original) == unmodified;
  }
  // Only the exact result and status survive assignment before or after posting.
  static bool terminal_check(FloatWordFormat from, FloatWordFormat to, WordValue input) {
    const auto expected = oracle(input, from, to, Policy());
    MixedSpace posted(from, to);
    fp_convert(posted, Policy(), posted.source, posted.result, posted.flags);
    if (posted.status() == SS_FAILED)
      return false;
    for (bool before : {false, true})
      for (int corruption : {0, 1, 2}) {
        std::unique_ptr<MixedSpace> space(before ? new MixedSpace(from, to)
                                                 : static_cast<MixedSpace *>(posted.clone()));
        dom(*space, space->source.bits(), input);
        dom(*space, space->result.bits(), expected.bits ^ (corruption == 1 ? 1 : 0));
        rel(*space, space->flags, IRT_EQ, expected.flags ^ (corruption == 2 ? 1 : 0));
        if (before)
          fp_convert(*space, Policy(), space->source, space->result, space->flags);
        if ((space->status() != SS_FAILED) != (corruption == 0))
          return false;
      }
    return true;
  }
  static bool terminal_checks() {
    for (auto to : {FWF_FP32, FWF_BF16, FWF_BINARY16, FWF_E4M3FN, FWF_E2M1}) {
      for (WordValue input : {WordValue(0x3f880001), WordValue(0x007f8000), WordValue(0x7f800001),
                              WordValue(0xffffffff), WordValue(0x387ff000), WordValue(0x477ff000)})
        if (!terminal_check(FWF_FP32, to, input))
          return false;
      for (WordValue input : {WordValue(0x0001), WordValue(0x7c01)})
        if (!terminal_check(FWF_BINARY16, to, input))
          return false;
    }
    return true;
  }

public:
  MixedConversionLifecycle() : Base("Word::Float::ConversionMixedLifecycle") {}
  bool run() override { return connected() && self_aliases() && tiny_clone() && terminal_checks(); }
};

MixedConversionLifecycle mixed_lifecycle;
MixedConversionOracle mixed_oracle;
MixedConversionInverse mixed_inverse;
ConversionBoundaries conversion_boundaries;
ConversionLifecycle conversion_lifecycle;
ConversionErrors conversion_errors;
} // namespace
} // namespace Float
} // namespace Word
} // namespace Test

// STATISTICS: test-word
