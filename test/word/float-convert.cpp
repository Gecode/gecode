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
#include <algorithm>
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
  WordVar source, result;
  ConversionSpace()
      : source(*this, WDT_FP32), result(*this, WDT_BINARY16) {}
  ConversionSpace(ConversionSpace &other) : Space(other) {
    source.update(*this, other.source);
    result.update(*this, other.result);
  }
  Space *copy() override { return new ConversionSpace(*this); }
};

class ConversionBoundaries : public Base {
public:
  ConversionBoundaries() : Base("Word::Float::ConversionBoundaries", TestTags::check()) {}
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
                dom(s, s.source, b.source | sign);
                dom(s, s.result, valid ? expected : (expected ^ 1));
              }
              fp_convert(s, s.source, s.result, Policy{mode});
              if (!assigned_at_post) {
                dom(s, s.source, b.source | sign);
                dom(s, s.result, valid ? expected : (expected ^ 1));
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
  explicit ConversionNetwork(int later = 0) : words(*this, 3) {
    words[0] = WordVar(*this, 32, WDT_FP32, 0x3f800000, 0x3f80e000, known_bits(0xffff07ff, 0x3f800000));
    for (int i = 1; i < 3; ++i) {
      words[i] = WordVar(*this, WDT_BINARY16);
      fp_convert(*this, words[0], words[i], Policy{i == 1 ? Policy::RNE : Policy::RTZ});
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
    for (int later : {1, 2}) {
      ConversionNetwork original;
      if (original.status() == SS_FAILED || solutions(original) != expected)
        return false;
      std::unique_ptr<ConversionNetwork> clone(static_cast<ConversionNetwork *>(original.clone()));
      clone->restrict_result(later);
      ConversionNetwork fresh(later);
      ConversionNetwork unchanged;
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
  static bool both_signs_and_bounds() {
    // The source cube holds +-(1 + 0x1000 ulp) and +-(1 + 0x1001 ulp); in total
    // order the negative encoding of greater magnitude is the minimum.
    ConversionSpace s;
    dom(s, s.source, WordValue(0x3f801000), WordValue(0xbf801001));
    fp_convert(s, s.source, s.result);
    branch(s, WordVarArgs{s.source, s.result}, WORD_VAL_LSB());
    if (s.status() == SS_FAILED || s.source.minimum() != 0xbf801001 ||
        s.source.maximum() != 0x3f801001)
      return false;
    DFS<ConversionSpace> search(&s);
    int count = 0;
    while (std::unique_ptr<ConversionSpace> solution{search.next()}) {
      const WordValue bits = solution->source.val();
      const WordValue expected = ((bits >> 16) & 0x8000) | (0x3c00 + (bits & 1));
      if (solution->result.val() != expected)
        return false;
      ++count;
    }
    if (count != 4)
      return false;
    ConversionNetwork network;
    if (network.status() == SS_FAILED)
      return false;
    Gecode::Word::WordView result(network.words[1]);
    const WordValue low = result.lo(), high = result.hi();
    // Positive encodings rank with the sign bit set.
    const ModEvent event = result.narrow_rank_range(network, 0x8000 | 0x3c01, 0x8000 | 0x3c06);
    if (me_failed(event) || !me_modified(event) || result.lo() != low || result.hi() != high)
      return false;
    if (network.status() == SS_FAILED || network.words[0].maximum() != 0x3f80d000)
      return false;
    return true;
  }

public:
  ConversionLifecycle() : Base("Word::Float::ConversionLifecycle") {}
  bool run() override { return connected() && both_signs_and_bounds(); }
};

class ConversionErrors : public Base {
public:
  ConversionErrors() : Base("Word::Float::ConversionErrors") {}
  bool run() override {
    ConversionSpace s;
    // Two postings of the same pair both keep the negative zero.
    fp_convert(s, s.source, s.result);
    // Fields without an effect on finite conversion do not restrict it.
    fp_convert(s, s.source, s.result,
               Policy{Policy::RNE, Policy::Preserve, Policy::Preserve, Policy::FormatDefault,
                      Policy::Signed, Policy::Canonical, Policy::BeforeRounding,
                      Policy::SignalingComparison});
    dom(s, s.source, WordValue(0x80000000));
    return s.status() != SS_FAILED && s.result.val() == 0x8000;
  }
};

// The expected conversion from the shared exact oracle: special values and
// NaN payloads here, then one rounding of the exact source value.
using ExpectedConversion = FloatResult;
ExpectedConversion oracle(WordValue bits, WordDomainType source_format,
                          WordDomainType result_format, const Policy &policy) {
  const FloatFormat from = float_format(source_format), to = float_format(result_format);
  const bool negative = from.negative(bits);
  const WordValue sign = negative ? to.sign : 0;
  const WordValue magnitude = from.magnitude(bits);
  if (from.is_nan(bits)) {
    if (!to.nan)
      return {7, 1};
    const int invalid = from.is_signaling(bits) ? 1 : 0;
    if (policy.nan == Policy::Canonical)
      return {to.nan, invalid};
    // The payload bits below the quiet bit, aligned at their top
    WordValue payload = 0;
    if (from.quiet && to.quiet)
      for (unsigned int bit = 1; bit < std::min(from.fraction, to.fraction); ++bit)
        if (magnitude & (from.quiet >> bit))
          payload |= to.quiet >> bit;
    return {sign | to.nan | payload, invalid};
  }
  if (from.is_infinity(bits))
    return to.infinity ? ExpectedConversion{sign | to.infinity, 0} : overflow_result(to, policy, negative);
  if (!magnitude || (policy.input_subnormals == Policy::SignedZero && magnitude < from.normal))
    return {policy.zeros == Policy::Positive ? 0 : sign, 0};
  return round_exact(to, from.value(magnitude), negative, policy);
}

class MixedSpace : public Space {
public:
  WordVar source, result;
  IntVar flags;
  MixedSpace(WordDomainType from, WordDomainType to, WordValueSetOption value_set = WVS_EXACT)
      : source(float_variable(*this, from, value_set)),
        result(float_variable(*this, to, value_set)), flags(*this, 0, 31) {}
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
                variant == 3 ? Policy::SaturateFinite : Policy::FormatDefault,
                variant == 3 ? Policy::Positive : Policy::Signed,
                variant == 3   ? Policy::Canonical
                : variant == 2 ? Policy::OperandFirst
                               : Policy::SignalFirst,
                variant == 3 ? Policy::BeforeRounding : Policy::AfterPrecision,
                variant == 2 ? Policy::SignalingComparison : Policy::QuietComparison};
}

bool assigned_conversion(WordValue input, WordDomainType from, WordDomainType to,
                         const Policy &policy, ExpectedConversion expected,
                         WordValueSetOption value_set = WVS_EXACT) {
  MixedSpace space(from, to, value_set);
  dom(space, space.source, input);
  fp_convert(space, space.source, space.result, space.flags, policy);
  if (space.status() == SS_FAILED || !space.result.assigned() || !space.flags.assigned() ||
      space.result.val() != expected.bits || space.flags.val() != expected.flags) {
    if (opt.log)
      olog << ind(2) << "conversion " << from << " -> " << to << " input " << input << " expected "
           << expected.bits << '/' << expected.flags << " mode " << policy.rounding << '\n';
    return false;
  }
  return true;
}

class MixedConversionOracle : public Base {
public:
  MixedConversionOracle() : Base("Word::Float::ConversionMixedOracle") {}
  bool run() override {
    const WordDomainType formats[] = {WDT_FP32, WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1};
    for (auto from : {WDT_E4M3FN, WDT_E2M1})
      for (auto to : formats)
        for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
          for (int variant = 0; variant < 4; ++variant) {
            const Policy policy = scalar_policy(mode, variant);
            for (WordValue bits = 0; bits < (WordValue(1) << float_word_width(from)); ++bits)
              if (!assigned_conversion(bits, from, to, policy, oracle(bits, from, to, policy)))
                return false;
          }
    for (auto from : {WDT_E4M3FN, WDT_E2M1})
      for (auto to : formats)
        for (WordValue bits = 0; bits < (WordValue(1) << float_word_width(from)); ++bits)
          if (!assigned_conversion(bits, from, to, Policy(), oracle(bits, from, to, Policy()),
                                   WVS_NONE))
            return false;
    const WordValue cases[] = {
        0,          1,          0x007f7fff, 0x007f8000, 0x007f8001, 0x007fc000,
        0x007fc001, 0x007fffff, 0x00800000, 0x00800001, 0x3affffff, 0x3b000000,
        0x3b000001, 0x3f7fffff, 0x3f800000, 0x3f807fff, 0x3f808000, 0x3f808001,
        0x3f818000, 0x3f880001, 0x40dfffff, 0x40e00000, 0x40e00001, 0x43e7ffff,
        0x43e80000, 0x43e80001, 0x7f7f0000, 0x7f7f7fff, 0x7f7f8000, 0x7f7fffff,
        0x7f800000, 0x7f800001, 0x7fbfffff, 0x7fc00000, 0x7fc12345, 0x7fffffff};
    for (auto from : {WDT_FP32, WDT_BF16})
      for (auto to : formats)
        for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
          for (int variant = 0; variant < 4; ++variant) {
            const Policy policy = scalar_policy(mode, variant);
            for (WordValue raw : cases)
              for (WordValue sign : {WordValue(0), WordValue(0x80000000)}) {
                const WordValue bits = (raw | sign) >> (from == WDT_BF16 ? 16 : 0);
                if (!assigned_conversion(bits, from, to, policy, oracle(bits, from, to, policy)))
                  return false;
              }
          }
    for (auto to : formats)
      for (WordValue bits = 0; bits <= 0xffff; ++bits)
        if (!assigned_conversion(bits, WDT_BINARY16, to, Policy(),
                                 oracle(bits, WDT_BINARY16, to, Policy())))
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
              if (!assigned_conversion(bits, WDT_BINARY16, to, policy,
                                       oracle(bits, WDT_BINARY16, to, policy)))
                return false;
        for (WordValue boundary : half_boundaries)
          for (WordValue raw = boundary < 2 ? 0 : boundary - 2;
               raw <= boundary + 2 && raw <= 0x7fffffff; ++raw)
            for (WordValue bits : {raw, raw | 0x80000000})
              if (!assigned_conversion(bits, WDT_FP32, WDT_BINARY16, policy,
                                       oracle(bits, WDT_FP32, WDT_BINARY16, policy)))
                return false;
      }
    // Fixed expected values also guard the independent oracle's range rules.
    return assigned_conversion(0x007f8000, WDT_FP32, WDT_BF16, Policy(), {0x80, 24}) &&
           assigned_conversion(0x007fc001, WDT_FP32, WDT_BF16, Policy(), {0x80, 16}) &&
           assigned_conversion(0x007fc001, WDT_FP32, WDT_BF16, scalar_policy(Policy::RNE, 3),
                               {0x80, 24}) &&
           assigned_conversion(0x43e80000, WDT_FP32, WDT_E4M3FN, Policy(), {0x7e, 16}) &&
           assigned_conversion(0x40e00000, WDT_FP32, WDT_E2M1, Policy(), {7, 20}) &&
           assigned_conversion(0x40e00000, WDT_FP32, WDT_E2M1, scalar_policy(Policy::RTZ, 0),
                               {7, 16}) &&
           assigned_conversion(0x3f880001, WDT_FP32, WDT_E4M3FN, Policy(), {0x39, 16}) &&
           assigned_conversion(0x387ff000, WDT_FP32, WDT_BINARY16, Policy(), {0x0400, 16}) &&
           assigned_conversion(0x387ff000, WDT_FP32, WDT_BINARY16, scalar_policy(Policy::RNE, 3),
                               {0x0400, 24}) &&
           assigned_conversion(0x477ff000, WDT_FP32, WDT_BINARY16, Policy(), {0x7c00, 20}) &&
           assigned_conversion(0x477ff000, WDT_FP32, WDT_BINARY16, scalar_policy(Policy::RTZ, 0),
                               {0x7bff, 16}) &&
           assigned_conversion(0x33000000, WDT_FP32, WDT_BINARY16, Policy(), {0, 24}) &&
           assigned_conversion(0x33000000, WDT_FP32, WDT_BINARY16, scalar_policy(Policy::RUP, 0),
                               {0x0001, 24}) &&
           assigned_conversion(0x7f802001, WDT_FP32, WDT_BINARY16, Policy(), {0x7e01, 1}) &&
           assigned_conversion(0x7c01, WDT_BINARY16, WDT_FP32, Policy(), {0x7fc02000, 1}) &&
           assigned_conversion(0x0001, WDT_BINARY16, WDT_FP32, Policy(), {0x33800000, 0}) &&
           assigned_conversion(0x7bff, WDT_BINARY16, WDT_E4M3FN, Policy(), {0x7f, 20}) &&
           assigned_conversion(0x7bff, WDT_BINARY16, WDT_E4M3FN, scalar_policy(Policy::RNE, 3),
                               {0x7e, 20});
  }
};

std::vector<Tuple> mixed_solutions(MixedSpace &space) {
  branch(space, WordVarArgs{space.source, space.result}, WORD_VAL_LSB());
  branch(space, space.flags, INT_VAL_MIN());
  DFS<MixedSpace> search(&space);
  std::vector<Tuple> result;
  while (std::unique_ptr<MixedSpace> solution{search.next()})
    result.push_back({solution->source.val(), solution->result.val(),
                      static_cast<WordValue>(solution->flags.val())});
  std::sort(result.begin(), result.end());
  return result;
}

class MixedConversionInverse : public Base {
public:
  MixedConversionInverse() : Base("Word::Float::ConversionMixedInverse", TestTags::check()) {}
  bool run() override {
    struct Window {
      WordValue first, last;
      WordDomainType target;
    };
    const Window windows[] = {{0x007f7ffc, 0x007f8004, WDT_BF16},
                              {0x007fbffc, 0x007fc004, WDT_BF16},
                              {0x3f807ffc, 0x3f808004, WDT_BF16},
                              {0x43e7fffc, 0x43e80004, WDT_E4M3FN},
                              {0x40dffffc, 0x40e00004, WDT_E2M1},
                              {0x7f7f7ffc, 0x7f7f8004, WDT_BF16},
                              {0x7f7ffffc, 0x7f800004, WDT_BF16},
                              {0x7fbffffc, 0x7fc00004, WDT_BF16},
                              {0x7f7ffffc, 0x7f800004, WDT_E2M1},
                              {0x7fbffffc, 0x7fc00004, WDT_E4M3FN},
                              {0, 8, WDT_E2M1},
                              {0x3f7ffffc, 0x3f800004, WDT_BF16},
                              {0x337ffffc, 0x33800004, WDT_BINARY16},
                              {0x387fdffc, 0x387fe004, WDT_BINARY16},
                              {0x477feffc, 0x477ff004, WDT_BINARY16},
                              {0x7f7ffffc, 0x7f800004, WDT_BINARY16},
                              {0x7f801ffc, 0x7f802004, WDT_BINARY16},
                              {0x7fbffffc, 0x7fc00004, WDT_BINARY16}};
    for (const auto &window : windows)
      for (WordValue sign : {WordValue(0), WordValue(0x80000000)})
        for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
          for (int variant = 0; variant < 4; ++variant) {
            const Policy policy = scalar_policy(mode, variant);
            MixedSpace space(WDT_FP32, window.target);
            rel(space, space.source, WRT_UGQ, sign | window.first);
            rel(space, space.source, WRT_ULQ, sign | window.last);
            fp_convert(space, space.source, space.result, space.flags, policy);
            std::vector<Tuple> expected;
            for (WordValue bits = window.first; bits <= window.last; ++bits) {
              const auto value = oracle(sign | bits, WDT_FP32, window.target, policy);
              expected.push_back({sign | bits, value.bits, static_cast<WordValue>(value.flags)});
            }
            if (mixed_solutions(space) != expected)
              return false;
          }
    // Every directed pair also receives symbolic source and result domains.
    for (auto from : {WDT_FP32, WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1})
      for (auto to : {WDT_FP32, WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1}) {
        const WordValue base = from == WDT_FP32       ? 0x3f800000
                               : from == WDT_BF16     ? 0x3f80
                               : from == WDT_BINARY16 ? 0x3c00
                               : from == WDT_E4M3FN   ? 0x38
                                                      : 0;
        const WordValue sign = WordValue(1) << (float_word_width(from) - 1);
        MixedSpace symbolic(from, to);
        dom(symbolic, symbolic.source, base, base | sign | 3);
        fp_convert(symbolic, symbolic.source, symbolic.result, symbolic.flags);
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
    MixedSpace space(WDT_FP32, WDT_E2M1);
    dom(space, space.result, WordValue(7));
    fp_convert(space, space.source, space.result, space.flags);
    if (space.status() == SS_FAILED || !space.flags.in(0) || !space.flags.in(16) ||
        !space.flags.in(20))
      return false;
    std::unique_ptr<MixedSpace> clone(static_cast<MixedSpace *>(space.clone()));
    rel(*clone, clone->flags, IRT_NQ, 16);
    if (clone->status() == SS_FAILED)
      return false;
    // The Word hull can retain holes; forcing such a hole must still fail.
    dom(*clone, clone->source, WordValue(0x40c00001));
    if (clone->status() != SS_FAILED || space.status() == SS_FAILED || !space.flags.in(16))
      return false;
    // The same holds at the binary16 maximum under RTZ.
    MixedSpace half(WDT_FP32, WDT_BINARY16);
    dom(half, half.result, WordValue(0x7bff));
    fp_convert(half, half.source, half.result, half.flags, scalar_policy(Policy::RTZ, 0));
    if (half.status() == SS_FAILED || !half.flags.in(0) || !half.flags.in(16) ||
        !half.flags.in(20))
      return false;
    std::unique_ptr<MixedSpace> half_clone(static_cast<MixedSpace *>(half.clone()));
    rel(*half_clone, half_clone->flags, IRT_NQ, 16);
    if (half_clone->status() == SS_FAILED)
      return false;
    dom(*half_clone, half_clone->source, WordValue(0x477fe001));
    if (half_clone->status() != SS_FAILED || half.status() == SS_FAILED || !half.flags.in(16))
      return false;
    // NaN sources of both signs give the maximum with an invalid status. In
    // total order the infinities lie between the two NaN partitions, so an
    // infinity drops out once the source sign is known.
    MixedSpace nan(WDT_FP32, WDT_E2M1);
    dom(nan, nan.result, WordValue(7));
    rel(nan, nan.flags, IRT_EQ, 1);
    fp_convert(nan, nan.source, nan.result, nan.flags);
    if (nan.status() == SS_FAILED || !nan.source.in(0xff800001) ||
        !nan.source.in(0x7fc00000))
      return false;
    for (WordValue sign : {WordValue(0), WordValue(0x80000000)}) {
      MixedSpace nan_sign(WDT_FP32, WDT_E2M1);
      dom(nan_sign, nan_sign.source, sign, sign | 0x7fffffff);
      dom(nan_sign, nan_sign.result, WordValue(7));
      rel(nan_sign, nan_sign.flags, IRT_EQ, 1);
      fp_convert(nan_sign, nan_sign.source, nan_sign.result, nan_sign.flags);
      if (nan_sign.status() == SS_FAILED || !nan_sign.source.in(sign | 0x7f800001) ||
          !nan_sign.source.in(sign | 0x7fc00000) ||
          nan_sign.source.in(sign | 0x7f800000))
        return false;
    }
    MixedSpace exact(WDT_FP32, WDT_BF16);
    dom(exact, exact.result, WordValue(0x3f80));
    rel(exact, exact.flags, IRT_EQ, 0);
    fp_convert(exact, exact.source, exact.result, exact.flags);
    if (exact.status() == SS_FAILED || !exact.source.assigned() ||
        exact.source.val() != 0x3f800000)
      return false;
    // Of the binary16 sources that round to BF16 one, a later exact status keeps 1.0.
    MixedSpace widened(WDT_BINARY16, WDT_BF16);
    dom(widened, widened.result, WordValue(0x3f80));
    fp_convert(widened, widened.source, widened.result, widened.flags);
    if (widened.status() == SS_FAILED || widened.source.assigned())
      return false;
    rel(widened, widened.flags, IRT_EQ, 0);
    if (widened.status() == SS_FAILED || !widened.source.assigned() ||
        widened.source.val() != 0x3c00)
      return false;
    // E4M3FN overflow gives a canonical NaN without the source sign, so a
    // result restricted to that NaN keeps the overflowing negative sources.
    Policy canonical;
    canonical.nan = Policy::Canonical;
    MixedSpace overflow(WDT_FP32, WDT_E4M3FN);
    rel(overflow, overflow.source, WRT_UGQ, WordValue(0xc3e7fffc));
    rel(overflow, overflow.source, WRT_ULQ, WordValue(0xc3e80004));
    dom(overflow, overflow.result, WordValue(0x7f));
    fp_convert(overflow, overflow.source, overflow.result, overflow.flags, canonical);
    std::vector<Tuple> expected;
    for (WordValue bits = 0xc3e7fffc; bits <= 0xc3e80004; ++bits) {
      const auto value = oracle(bits, WDT_FP32, WDT_E4M3FN, canonical);
      if (value.bits == 0x7f)
        expected.push_back({bits, value.bits, static_cast<WordValue>(value.flags)});
    }
    if (expected.empty() || mixed_solutions(overflow) != expected)
      return false;
    // Positive zero must narrow the source sign.
    MixedSpace zero(WDT_FP32, WDT_BF16);
    dom(zero, zero.result, WordValue(0x8000));
    fp_convert(zero, zero.source, zero.result, zero.flags, scalar_policy(Policy::RNE, 3));
    return zero.status() == SS_FAILED;
  }
};

// Propagation from random partial domains keeps every tuple that the oracle
// supports, and search finds exactly those tuples.
class MixedConversionRoot : public Base {
  // A nonempty random subset of the given values.
  std::vector<WordValue> random_subset(const std::vector<WordValue> &values) {
    std::vector<WordValue> subset;
    const unsigned int sparsity = 1U + _rand(6U);
    while (subset.empty())
      for (WordValue v : values)
        if (_rand(sparsity) == 0)
          subset.push_back(v);
    return subset;
  }
  static std::vector<WordValue> codes(WordDomainType format) {
    std::vector<WordValue> all;
    for (WordValue v = 0; v < (WordValue(1) << float_word_width(format)); ++v)
      all.push_back(v);
    return all;
  }
  static bool contains(const std::vector<WordValue> &values, WordValue v) {
    return std::find(values.begin(), values.end(), v) != values.end();
  }
  // Restrict a side: a tiny side to a random value set, a BF16 source to a
  // random window of 64 encodings, and a BF16 result not at all.
  std::vector<WordValue> restrict_side(MixedSpace &space, WordVar side, bool is_source) {
    if (float_word_width(side.domain_type()) <= 8) {
      const std::vector<WordValue> values = random_subset(codes(side.domain_type()));
      dom(space, side, WordValArgs(values.begin(), values.end()));
      return values;
    }
    if (!is_source)
      return codes(side.domain_type());
    const WordValue first = (_rand(2U) ? 0x8000 : 0) | (0x3c00 + 64 * _rand(32U));
    dom(space, side, first, first + 63);
    std::vector<WordValue> window;
    for (WordValue v = first; v <= first + 63; ++v)
      window.push_back(v);
    return window;
  }
  bool check_root(MixedSpace &space, WordDomainType from, WordDomainType to,
                  const Policy &policy, const std::vector<WordValue> &sources,
                  const std::vector<WordValue> &results,
                  const std::vector<WordValue> &statuses) {
    std::vector<Tuple> expected;
    for (WordValue a : sources) {
      const auto e = oracle(a, from, to, policy);
      if (contains(results, e.bits) && contains(statuses, static_cast<WordValue>(e.flags)))
        expected.push_back({a, e.bits, static_cast<WordValue>(e.flags)});
    }
    std::sort(expected.begin(), expected.end());
    fp_convert(space, space.source, space.result, space.flags, policy);
    if (space.status() == SS_FAILED)
      return expected.empty();
    for (const Tuple &t : expected)
      if (!space.source.in(t[0]) || !space.result.in(t[1]) ||
          !space.flags.in(static_cast<int>(t[2])))
        return false;
    return mixed_solutions(space) == expected;
  }
  bool keeps_supported(WordDomainType from, WordDomainType to, const Policy &policy) {
    MixedSpace space(from, to);
    const std::vector<WordValue> sources = restrict_side(space, space.source, true);
    const std::vector<WordValue> results = restrict_side(space, space.result, false);
    const std::vector<WordValue> statuses = random_subset({0, 1, 16, 20, 24});
    IntArgs flags;
    for (WordValue f : statuses)
      flags << static_cast<int>(f);
    dom(space, space.flags, IntSet(flags));
    return check_root(space, from, to, policy, sources, results, statuses);
  }
  // FP32 sources 1 to 1.875 in steps of 1/8, each also one ulp above, reach
  // eight E4M3FN results through the preimage scan.
  bool wide_window(const Policy &policy) {
    MixedSpace space(WDT_FP32, WDT_E4M3FN);
    dom(space, space.source, WordValue(0x3f800000), WordValue(0x3ff00001));
    std::vector<WordValue> sources;
    for (WordValue v = 0x3f800000; v <= 0x3ff00001; ++v)
      if ((v & ~WordValue(0x3ff00001)) == 0 && (v & 0x3f800000) == 0x3f800000)
        sources.push_back(v);
    const std::vector<WordValue> results = random_subset(codes(WDT_E4M3FN));
    dom(space, space.result, WordValArgs(results.begin(), results.end()));
    std::vector<WordValue> statuses;
    for (WordValue f = 0; f < 32; ++f)
      statuses.push_back(f);
    return check_root(space, WDT_FP32, WDT_E4M3FN, policy, sources, results, statuses);
  }

public:
  MixedConversionRoot() : Base("Word::Float::ConversionMixedRoot") {}
  bool run() override {
    const WordDomainType pairs[][2] = {{WDT_E4M3FN, WDT_E2M1},
                                        {WDT_E2M1, WDT_E4M3FN},
                                        {WDT_E4M3FN, WDT_BF16},
                                        {WDT_BF16, WDT_E4M3FN}};
    for (int trial = 0; trial < 40; ++trial) {
      const Policy policy =
          scalar_policy(static_cast<Policy::Rounding>(_rand(4U)), static_cast<int>(_rand(4U)));
      for (const auto &pair : pairs)
        if (!keeps_supported(pair[0], pair[1], policy))
          return false;
      if (!wide_window(policy))
        return false;
    }
    return true;
  }
};

// FP32 sources spanning many BF16 or binary16 results, with fragmented
// result cubes, source cube holes and restricted statuses. Propagation gives
// exactly the hull of the supports that an oracle scan of the window finds:
// the bit hull and total-order bounds of both words, and the statuses.
class MixedConversionWide : public Base {
  struct Cube {
    WordValue lo, hi;  // Fixed ones and possible ones
  };
  struct Window {
    WordValue first, last;  // Source encodings of one sign
    Cube source;            // Source cube
    WordDomainType target;
    Cube result;            // Result cube
    unsigned int statuses;  // Admitted statuses, or 0 for no status
    WordValue minimum = 0, maximum = 0;  // Result bounds, unless both zero
  };
  static WordValue order(WordValue bits, unsigned int width) {
    const WordValue sign = WordValue(1) << (width - 1);
    return (bits & sign) ? ~bits & (sign | (sign - 1)) : bits | sign;
  }
  struct Hull {
    bool present = false;
    WordValue all = ~WordValue(0), any = 0, minimum = 0, maximum = 0;
    void add(WordValue bits, unsigned int width) {
      if (!present || order(bits, width) < order(minimum, width))
        minimum = bits;
      if (!present || order(bits, width) > order(maximum, width))
        maximum = bits;
      present = true;
      all &= bits;
      any |= bits;
    }
    bool matches(const WordVar &x) const {
      return x.lo() == all && x.hi() == any && x.minimum() == minimum && x.maximum() == maximum;
    }
  };
  static bool in_cube(WordValue bits, Cube cube) { return (bits & ~cube.hi) == 0 && (~bits & cube.lo) == 0; }
  static bool in_bounds(WordValue bits, const Window &window) {
    return (!window.minimum && !window.maximum) ||
           (order(window.minimum, 16) <= order(bits, 16) && order(bits, 16) <= order(window.maximum, 16));
  }
  static WordValue first_member(WordValue bits, Cube cube) {
    while (!in_cube(bits, cube))
      ++bits;
    return bits;
  }
  // The next member of the cube after the member bits, or all ones after
  // the greatest member.
  static WordValue next_member(WordValue bits, Cube cube) {
    const WordValue next = (((bits | ~cube.hi) + 1) & cube.hi) | cube.lo;
    return next > bits ? next : ~WordValue(0);
  }
  static bool exact_projection(const Window &window, const Policy &policy) {
    Hull sources, results;
    unsigned int statuses = 0;
    for (WordValue bits = first_member(window.first, window.source); bits <= window.last;
         bits = next_member(bits, window.source)) {
      const auto expected = oracle(bits, WDT_FP32, window.target, policy);
      if (!in_cube(expected.bits, window.result) || !in_bounds(expected.bits, window) ||
          (window.statuses && !(window.statuses & (1U << expected.flags))))
        continue;
      sources.add(bits, 32);
      results.add(expected.bits, 16);
      statuses |= 1U << expected.flags;
    }
    MixedSpace space(WDT_FP32, window.target);
    // Negative windows have their least value at the greatest encoding.
    const bool negative = (window.first >> 31) != 0;
    space.source = WordVar(space, 32, WDT_FP32, negative ? window.last : window.first,
                           negative ? window.first : window.last);
    dom(space, space.source, window.source.lo, window.source.hi);
    if (window.minimum || window.maximum)
      space.result = WordVar(space, 16, window.target, window.minimum, window.maximum);
    dom(space, space.result, window.result.lo, window.result.hi);
    if (window.statuses) {
      IntArgs admitted;
      for (int flags = 0; flags < 32; ++flags)
        if (window.statuses & (1U << flags))
          admitted << flags;
      dom(space, space.flags, IntSet(admitted));
      fp_convert(space, space.source, space.result, space.flags, policy);
    } else {
      fp_convert(space, space.source, space.result, policy);
    }
    if (space.status() == SS_FAILED)
      return !sources.present;
    bool exact = sources.present && sources.matches(space.source) && results.matches(space.result);
    if (window.statuses)
      for (int flags = 0; flags < 32; ++flags)
        exact = exact && space.flags.in(flags) == ((statuses & (1U << flags)) != 0);
    if (!exact && opt.log)
      olog << ind(2) << "wide conversion " << window.first << ".." << window.last << " to " << window.target
           << " gives " << space.source << ' ' << space.result << ' ' << space.flags << '\n';
    return exact;
  }

public:
  MixedConversionWide() : Base("Word::Float::ConversionMixedWide") {}
  bool run() override {
    const unsigned int any = 0, exact = 1U << 0, inexact = 1U << 16, tiny = 1U << 24, overflow = 1U << 20;
    const WordValue full = 0xffffffff, half = 0xffff;
    const WordValue negative = 0x80000000;
    const Window windows[] = {
        // Around one, every 512th source over 33 BF16 results.
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16, {0, half & ~WordValue(1)}, any},
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16, {1, half}, inexact},
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16, {0, half & ~WordValue(1)}, exact},
        {0x3f7f0000, 0x3f9fffff, {5, full & ~WordValue(0x1fa)}, WDT_BF16, {0, half}, exact},
        // A fixed source bit within the results splits their preimages.
        {0x3f7f0000, 0x3f9fffff, {0x100000, full & ~WordValue(0x1ff)}, WDT_BF16, {0, half & ~WordValue(8)},
         exact | inexact},
        {negative | 0x3f7f0000, negative | 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16,
         {0, half & ~WordValue(8)}, exact | inexact | tiny},
        // FP32 subnormals to BF16 subnormals and the least normal.
        {1, 0x00ffffff, {0, full & ~WordValue(0xfff)}, WDT_BF16, {1, half}, exact | tiny},
        // Binary16 normals, results one more than a multiple of four.
        {0x3f7e0000, 0x3f83ffff, {0, full & ~WordValue(0x1f)}, WDT_BINARY16, {1, half & ~WordValue(2)},
         exact | inexact},
        {0x3f7e0000, 0x3f83ffff, {0, full & ~WordValue(0x1f)}, WDT_BINARY16, {1, half & ~WordValue(2)}, inexact},
        // Binary16 subnormals up to the least normals.
        {0x38700000, 0x3881ffff, {0, full & ~WordValue(0x7f)}, WDT_BINARY16, {0, half & ~WordValue(4)},
         exact | tiny},
        {0x38700000, 0x3881ffff, {0, full & ~WordValue(0x7f)}, WDT_BINARY16, {0, half & ~WordValue(4)}, tiny},
        // Binary16 maximum and overflow.
        {0x477f0000, 0x4780ffff, {0, full & ~WordValue(0x1f)}, WDT_BINARY16, {0, half & ~WordValue(2)},
         inexact | overflow},
        // Source bit 12 fixed under one binary16 result: zero (or the least
        // subnormal) and overflow.
        {0x32000000, 0x32ffffff, {0, full & ~WordValue(0x1ffc)}, WDT_BINARY16, {0, half}, any},
        {0x47800000, 0x47ffffff, {0x1000, full & ~WordValue(0xffc)}, WDT_BINARY16, {0, half},
         exact | inexact | tiny | overflow},
        // One binary16 result in a rounded region, whose sources all have
        // bit 10 set and so are inexact: the source cube decides the status.
        {0x3f001400, 0x3f002fff, {0x3f000400, 0x3f003fff}, WDT_BINARY16, {0x3801, 0x3801}, exact | inexact},
        // Source bits 0..11 fixed with only inexact sources admitted, from
        // binary16 subnormals up to normals and around one: all zeros (the
        // inexact sources have bit 12 set), and a pattern (all inexact).
        {0x38000000, 0x3880ffff, {0, full & ~WordValue(0xfff)}, WDT_BINARY16, {0, half}, inexact | tiny},
        {0x3f000000, 0x3fffffff, {0, full & ~WordValue(0xfff)}, WDT_BINARY16, {0, half}, inexact},
        {0x3f000000, 0x3fffffff, {0x234, full & ~WordValue(0xfff ^ 0x234)}, WDT_BINARY16, {0, half}, inexact},
        // Two sources per preimage, an exact one and a midpoint: under RNE
        // only even results have inexact sources.
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0x7fff)}, WDT_BF16, {0, half}, inexact},
        {0x3f7e0000, 0x3f83ffff, {0, full & ~WordValue(0xfff)}, WDT_BINARY16, {0, half}, inexact},
        // Only exact sources, so no inexact result.
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0xffff)}, WDT_BF16, {0, half}, inexact},
        // Result bounds inside the image, with and without a cube hole.
        {0x3f7f0000, 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16, {0, half}, any, 0x3f81, 0x3f8e},
        {negative | 0x3f7f0000, negative | 0x3f9fffff, {0, full & ~WordValue(0x1ff)}, WDT_BF16,
         {0, half & ~WordValue(1)}, exact | inexact, 0xbf8f, 0xbf82},
        {0x3f7e0000, 0x3f83ffff, {0, full & ~WordValue(0x1f)}, WDT_BINARY16, {0, half}, inexact, 0x3bf1, 0x3c0d}};
    for (const auto &window : windows)
      for (const Policy &policy : {Policy(), scalar_policy(Policy::RTZ, 0), scalar_policy(Policy::RUP, 2),
                                   scalar_policy(Policy::RNE, 3)})
        if (!exact_projection(window, policy))
          return false;
    return true;
  }
};

// Conversions between float-typed words, whose bounds are in IEEE total
// order, on negative-only and cross-zero windows.
class ConversionFloatTyped : public Base {
  static bool check_window(WordDomainType from, WordDomainType to, WordValue minimum,
                           WordValue maximum, const Policy &policy) {
    MixedSpace space(from, to);
    space.source = float_window(space, from, minimum, maximum);
    space.result = WordVar(space, to);
    std::vector<Tuple> expected;
    for (WordValue bits : float_order_range(float_word_width(from), minimum, maximum)) {
      const auto value = oracle(bits, from, to, policy);
      if (space.result.in(value.bits))
        expected.push_back({bits, value.bits, static_cast<WordValue>(value.flags)});
    }
    std::sort(expected.begin(), expected.end());
    fp_convert(space, space.source, space.result, space.flags, policy);
    if (space.status() == SS_FAILED)
      return expected.empty();
    if (mixed_solutions(space) == expected)
      return true;
    if (opt.log)
      olog << ind(2) << "float-typed conversion " << from << " -> " << to << " window " << minimum << ".."
           << maximum << '\n';
    return false;
  }
  static bool windows() {
    struct Window {
      WordDomainType from, to;
      WordValue minimum, maximum;
    };
    // Total-order endpoints: the first window of each pair lies within the
    // negative half, the second crosses zero.
    const Window windows[] = {{WDT_FP32, WDT_BF16, 0xbf808004, 0xbf807ffc},
                              {WDT_FP32, WDT_BF16, 0x80000004, 0x00000004},
                              {WDT_FP32, WDT_E4M3FN, 0xc3e80004, 0xc3e7fffc},
                              {WDT_FP32, WDT_E4M3FN, 0x80000004, 0x00000004},
                              {WDT_FP32, WDT_E2M1, 0xc0e00004, 0xc0dffffc},
                              {WDT_FP32, WDT_E2M1, 0x80000004, 0x00000004},
                              {WDT_FP32, WDT_BINARY16, 0xb3800004, 0xb37ffffc},
                              {WDT_FP32, WDT_BINARY16, 0x80000004, 0x00000004},
                              {WDT_BF16, WDT_E2M1, 0xbfc4, 0xbf7c},
                              {WDT_BF16, WDT_E2M1, 0x8004, 0x0004},
                              {WDT_E4M3FN, WDT_FP32, 0xbc, 0xb4},
                              {WDT_E4M3FN, WDT_FP32, 0x84, 0x04},
                              {WDT_E2M1, WDT_FP32, 0xd, 0x9},
                              {WDT_E2M1, WDT_FP32, 0xb, 0x3}};
    for (const Window &w : windows)
      for (auto mode : {Policy::RNE, Policy::RTZ, Policy::RDN, Policy::RUP})
        for (int variant = 0; variant < 4; ++variant)
          if (!check_window(w.from, w.to, w.minimum, w.maximum, scalar_policy(mode, variant)))
            return false;
    return true;
  }
  // A negative BF16 result window on a float-typed result restricts the
  // FP32 source through the preimage scan.
  static bool result_window() {
    MixedSpace space(WDT_FP32, WDT_BF16);
    space.source = float_window(space, WDT_FP32, 0xbf800010, 0xbf7ffff0);
    space.result = float_window(space, WDT_BF16, 0xbf81, 0xbf80);
    std::vector<Tuple> expected;
    for (WordValue bits : float_order_range(32, 0xbf800010, 0xbf7ffff0)) {
      const auto value = oracle(bits, WDT_FP32, WDT_BF16, Policy());
      if (value.bits == 0xbf81 || value.bits == 0xbf80)
        expected.push_back({bits, value.bits, static_cast<WordValue>(value.flags)});
    }
    std::sort(expected.begin(), expected.end());
    fp_convert(space, space.source, space.result, space.flags);
    return !expected.empty() && mixed_solutions(space) == expected;
  }
  // A float-typed source whose sign is unknown is enumerated exactly: BF16
  // codes +-0.25 and +-1 convert to exactly the E2M1 codes of +-0 and +-1.
  static bool unknown_sign() {
    MixedSpace space(WDT_BF16, WDT_E2M1);
    space.source = WordVar(space, 16, WDT_BF16, 0xbf80, 0x3f80, known_bits(0x7eff, 0x3e80));
    space.result = WordVar(space, WDT_E2M1, WVS_EXACT);
    fp_convert(space, space.source, space.result, space.flags);
    if (space.status() == SS_FAILED)
      return false;
    for (WordValue v = 0; v < 16; ++v)
      if (space.result.in(v) != (v == 0 || v == 2 || v == 8 || v == 10))
        return false;
    return true;
  }
  // The preimage of BF16 -1 under RNE is the FP32 magnitudes from the tie
  // below 1 to the tie above it, so a float-typed source narrows to that
  // total-order window.
  static bool bound_deduction() {
    MixedSpace space(WDT_FP32, WDT_BF16);
    space.source = float_window(space, WDT_FP32, 0xbf900000, 0xbf700000);
    space.result = float_window(space, WDT_BF16, 0xbf80, 0xbf80);
    fp_convert(space, space.source, space.result, space.flags);
    const WordValue minimum = 0xbf808000, maximum = 0xbf7f8000;
    return space.status() != SS_FAILED && space.source.minimum() == minimum &&
           space.source.maximum() == maximum &&
           oracle(minimum, WDT_FP32, WDT_BF16, Policy()).bits == 0xbf80 &&
           oracle(maximum, WDT_FP32, WDT_BF16, Policy()).bits == 0xbf80 &&
           oracle(minimum + 1, WDT_FP32, WDT_BF16, Policy()).bits != 0xbf80 &&
           oracle(maximum - 1, WDT_FP32, WDT_BF16, Policy()).bits != 0xbf80;
  }

public:
  ConversionFloatTyped() : Base("Word::Float::ConversionFloatTyped") {}
  bool run() override { return windows() && result_window() && unknown_sign() && bound_deduction(); }
};

class MixedConversionNetwork : public Space {
public:
  WordVarArray words;
  explicit MixedConversionNetwork(bool later = false) : words(*this, 3) {
    words[0] = WordVar(*this, WDT_FP32);
    dom(*this, words[0], WordValue(0x3f800000), WordValue(0xbf83c000));
    for (int i = 1; i < 3; ++i) {
      words[i] = WordVar(*this, WDT_BF16);
      fp_convert(*this, words[0], words[i], Policy{i == 1 ? Policy::RNE : Policy::RTZ});
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
        const WordValue nearest = oracle(input, WDT_FP32, WDT_BF16, Policy()).bits;
        const WordValue truncated =
            oracle(input, WDT_FP32, WDT_BF16, scalar_policy(Policy::RTZ, 0)).bits;
        if (nearest != truncated) {
          expected.push_back({input, nearest, truncated});
          if (nearest & 1)
            restricted.push_back({input, nearest, truncated});
        }
      }
    MixedConversionNetwork original;
    if (original.status() == SS_FAILED || network_solutions(original) != expected)
      return false;
    std::unique_ptr<MixedConversionNetwork> clone(
        static_cast<MixedConversionNetwork *>(original.clone()));
    clone->restrict_result();
    MixedConversionNetwork fresh(true);
    if (clone->status() == SS_FAILED || fresh.status() == SS_FAILED)
      return false;
    for (int i = 0; i < 3; ++i)
      if (clone->words[i].lo() != fresh.words[i].lo() ||
          clone->words[i].hi() != fresh.words[i].hi() ||
          clone->words[i].minimum() != fresh.words[i].minimum() ||
          clone->words[i].maximum() != fresh.words[i].maximum())
        return false;
    return network_solutions(*clone) == restricted && network_solutions(fresh) == restricted &&
           network_solutions(original) == expected;
  }
  static bool self_aliases() {
    for (const Policy &policy : {Policy(), scalar_policy(Policy::RNE, 1),
                                 scalar_policy(Policy::RNE, 2), scalar_policy(Policy::RNE, 3)})
      for (WordValue bits : {WordValue(0), WordValue(0x80000000), WordValue(1),
                             WordValue(0x7f800001), WordValue(0xffc12345)}) {
        MixedSpace space(WDT_FP32, WDT_FP32);
        fp_convert(space, space.source, space.source, space.flags, policy);
        dom(space, space.source, bits);
        const auto expected = oracle(bits, WDT_FP32, WDT_FP32, policy);
        if ((space.status() != SS_FAILED) != (bits == expected.bits))
          return false;
        if (bits == expected.bits && space.flags.val() != expected.flags)
          return false;
      }
    for (const Policy &policy : {Policy(), scalar_policy(Policy::RNE, 1),
                                 scalar_policy(Policy::RNE, 2), scalar_policy(Policy::RNE, 3)})
      for (WordValue bits :
           {WordValue(0), WordValue(0x8000), WordValue(1), WordValue(0x7c01), WordValue(0xfe12)}) {
        MixedSpace space(WDT_BINARY16, WDT_BINARY16);
        fp_convert(space, space.source, space.source, space.flags, policy);
        dom(space, space.source, bits);
        const auto expected = oracle(bits, WDT_BINARY16, WDT_BINARY16, policy);
        if ((space.status() != SS_FAILED) != (bits == expected.bits))
          return false;
        if (bits == expected.bits && space.flags.val() != expected.flags)
          return false;
      }
    for (auto format : {WDT_E2M1, WDT_E4M3FN})
      for (const Policy &policy : {Policy(), scalar_policy(Policy::RNE, 1),
                                   scalar_policy(Policy::RNE, 2), scalar_policy(Policy::RNE, 3)})
        for (WordValue bits = 0; bits < (WordValue(1) << float_word_width(format)); ++bits) {
          MixedSpace space(format, format);
          fp_convert(space, space.source, space.source, space.flags, policy);
          dom(space, space.source, bits);
          const auto expected = oracle(bits, format, format, policy);
          if ((space.status() != SS_FAILED) != (bits == expected.bits))
            return false;
          if (bits == expected.bits && space.flags.val() != expected.flags)
            return false;
        }
    return true;
  }
  static void restrict_tiny(MixedSpace &space, bool remove) {
    dom(space, space.source, WordValArgs{0x38, 0x39, 0x3a, 0x3b});
    fp_convert(space, space.source, space.result, space.flags);
    if (remove)
      rel(space, space.source, WRT_NQ, WordValue(0x39));
  }
  static bool tiny_clone() {
    MixedSpace original(WDT_E4M3FN, WDT_BF16);
    restrict_tiny(original, false);
    if (original.status() == SS_FAILED)
      return false;
    std::unique_ptr<MixedSpace> clone(static_cast<MixedSpace *>(original.clone()));
    rel(*clone, clone->source, WRT_NQ, WordValue(0x39));
    MixedSpace fresh(WDT_E4M3FN, WDT_BF16);
    restrict_tiny(fresh, true);
    if (clone->status() == SS_FAILED || fresh.status() == SS_FAILED ||
        !original.source.in(0x39))
      return false;
    if (clone->source.lo() != fresh.source.lo() ||
        clone->source.hi() != fresh.source.hi() ||
        clone->result.minimum() != fresh.result.minimum() ||
        clone->result.maximum() != fresh.result.maximum())
      return false;
    const std::vector<Tuple> expected{{0x38, 0x3f80, 0}, {0x3a, 0x3fa0, 0}, {0x3b, 0x3fb0, 0}};
    if (mixed_solutions(*clone) != expected || mixed_solutions(fresh) != expected)
      return false;
    std::vector<Tuple> unmodified = expected;
    unmodified.insert(unmodified.begin() + 1, {0x39, 0x3f90, 0});
    return mixed_solutions(original) == unmodified;
  }
  // Only the exact result and status survive assignment before or after posting.
  static bool terminal_check(WordDomainType from, WordDomainType to, WordValue input) {
    const auto expected = oracle(input, from, to, Policy());
    MixedSpace posted(from, to);
    fp_convert(posted, posted.source, posted.result, posted.flags);
    if (posted.status() == SS_FAILED)
      return false;
    for (bool before : {false, true})
      for (int corruption : {0, 1, 2}) {
        std::unique_ptr<MixedSpace> space(before ? new MixedSpace(from, to)
                                                 : static_cast<MixedSpace *>(posted.clone()));
        dom(*space, space->source, input);
        dom(*space, space->result, expected.bits ^ (corruption == 1 ? 1 : 0));
        rel(*space, space->flags, IRT_EQ, expected.flags ^ (corruption == 2 ? 1 : 0));
        if (before)
          fp_convert(*space, space->source, space->result, space->flags);
        if ((space->status() != SS_FAILED) != (corruption == 0))
          return false;
      }
    return true;
  }
  static bool terminal_checks() {
    for (auto to : {WDT_FP32, WDT_BF16, WDT_BINARY16, WDT_E4M3FN, WDT_E2M1}) {
      for (WordValue input : {WordValue(0x3f880001), WordValue(0x007f8000), WordValue(0x7f800001),
                              WordValue(0xffffffff), WordValue(0x387ff000), WordValue(0x477ff000)})
        if (!terminal_check(WDT_FP32, to, input))
          return false;
      for (WordValue input : {WordValue(0x0001), WordValue(0x7c01)})
        if (!terminal_check(WDT_BINARY16, to, input))
          return false;
    }
    return true;
  }

public:
  MixedConversionLifecycle() : Base("Word::Float::ConversionMixedLifecycle") {}
  bool run() override { return connected() && self_aliases() && tiny_clone() && terminal_checks(); }
};

MixedConversionLifecycle mixed_lifecycle;
MixedConversionRoot mixed_root;
MixedConversionWide mixed_wide;
MixedConversionOracle mixed_oracle;
MixedConversionInverse mixed_inverse;
ConversionFloatTyped conversion_float_typed;
ConversionBoundaries conversion_boundaries;
ConversionLifecycle conversion_lifecycle;
ConversionErrors conversion_errors;
} // namespace
} // namespace Float
} // namespace Word
} // namespace Test

// STATISTICS: test-word
