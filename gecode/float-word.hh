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

#ifndef GECODE_FLOAT_WORD_HH
#define GECODE_FLOAT_WORD_HH

#include <gecode/word.hh>

namespace Gecode {
  /** \brief Return the width of float \a format
   *
   * Throws Word::OutOfLimits if \a format is not a float domain type.
   */
  GECODE_WORD_EXPORT unsigned int float_word_width(WordDomainType format);

  /** \brief Return an assigned word of float \a format that encodes \a value
   *
   * The word's bits encode \a value exactly, so it can serve as a constant
   * operand of the float operations, for example
   * <tt>fp_add(home, x, float_constant(home, WDT_FP32, 1.5), y)</tt>.
   * It has the format's default value-set option (WVS_DEFAULT).
   *
   * No rounding takes place: \a value must be a finite value of \a format,
   * or an infinity of a format that has infinities. -0.0 encodes as the
   * negative zero. A NaN has no unique encoding and is rejected; a word
   * with a specific NaN takes its encoding as a bit pattern instead.
   *
   * Throws Word::OutOfLimits if \a format is not a float domain type, if
   * \a value is a NaN, or if \a format cannot represent \a value exactly,
   * including a value beyond the largest finite magnitude and an infinity
   * in E4M3FN or E2M1.
   */
  GECODE_WORD_EXPORT WordVar float_constant(Space& home, WordDomainType format,
                                            double value);

  /** \brief Numerical policy value
   *
   * A plain assignable value whose defaults select IEEE 754 behaviour with
   * round-to-nearest-even. A posting without a policy uses this default
   * value. Every combination of field values is supported.
   * Every numerical posting checks all fields, also those the operation
   * ignores, and throws Word::OutOfLimits for a value outside its
   * enumeration. Each posting keeps its own copy, so later changes to the
   * caller's policy do not affect it, and postings in one space can use
   * different policies. A field without effect on an operation is ignored:
   * rounding, overflow and tininess do not change minimum or maximum
   * selection, and comparisons use only the input treatment and
   * \c comparison. Raw observations and sign operations take no policy.
   * The rules are in \ref TaskModelWordFloat.
   */
  class FloatWordPolicy {
  public:
    /// Rounding direction
    enum class Rounding : int {
      RNE, ///< Nearest, ties to even
      RTZ, ///< Toward zero
      RDN, ///< Toward negative infinity
      RUP  ///< Toward positive infinity
    };
    /// Nearest, ties to even
    static constexpr Rounding RNE = Rounding::RNE;
    /// Toward zero
    static constexpr Rounding RTZ = Rounding::RTZ;
    /// Toward negative infinity
    static constexpr Rounding RDN = Rounding::RDN;
    /// Toward positive infinity
    static constexpr Rounding RUP = Rounding::RUP;

    /// Treatment of subnormal operands or results
    enum class Subnormals : int {
      Preserve,  ///< Keep subnormals
      SignedZero ///< Replace a subnormal by zero with its sign
    };
    /// Keep subnormals
    static constexpr Subnormals Preserve = Subnormals::Preserve;
    /// Replace a subnormal by zero with its sign
    static constexpr Subnormals SignedZero = Subnormals::SignedZero;

    /// Overflow result
    enum class Overflow : int {
      Format,        ///< Format-dependent result (see overflow)
      SaturateFinite ///< Largest finite magnitude
    };
    /// Format-dependent result (see overflow)
    static constexpr Overflow Format = Overflow::Format;
    /// Largest finite magnitude
    static constexpr Overflow SaturateFinite = Overflow::SaturateFinite;

    /// Sign of zero results
    enum class Zeros : int {
      Signed,  ///< Keep the specified sign of zero results
      Positive ///< Every zero result is positive zero
    };
    /// Keep the specified sign of zero results
    static constexpr Zeros Signed = Zeros::Signed;
    /// Every zero result is positive zero
    static constexpr Zeros Positive = Zeros::Positive;

    /// NaN result selection
    enum class Nan : int {
      SignalFirst,  ///< First signaling NaN, otherwise first quiet NaN
      OperandFirst, ///< First NaN, regardless of signaling class
      Canonical     ///< Canonical quiet NaN without sign or payload
    };
    /// First signaling NaN, otherwise first quiet NaN
    static constexpr Nan SignalFirst = Nan::SignalFirst;
    /// First NaN, regardless of signaling class
    static constexpr Nan OperandFirst = Nan::OperandFirst;
    /// Canonical quiet NaN without sign or payload
    static constexpr Nan Canonical = Nan::Canonical;

    /// Underflow tininess detection
    enum class Tininess : int {
      AfterPrecision, ///< After rounding to precision with unbounded exponent
      BeforeRounding  ///< On the exact result before rounding
    };
    /// After rounding to precision with unbounded exponent
    static constexpr Tininess AfterPrecision = Tininess::AfterPrecision;
    /// On the exact result before rounding
    static constexpr Tininess BeforeRounding = Tininess::BeforeRounding;

    /// NaN operands that make a comparison invalid
    enum class Comparison : int {
      Quiet,    ///< Only signaling NaNs
      Signaling ///< Any NaN
    };
    /// Only signaling NaNs
    static constexpr Comparison Quiet = Comparison::Quiet;
    /// Any NaN
    static constexpr Comparison Signaling = Comparison::Signaling;


    /// Rounding direction for arithmetic and conversion (default RNE).
    Rounding rounding = RNE;
    /// SignedZero reads each subnormal operand as zero with its sign
    /// (default Preserve).
    Subnormals input_subnormals = Preserve;
    /// SignedZero replaces each subnormal result by zero with its sign
    /// (default Preserve).
    Subnormals output_subnormals = Preserve;
    /// Overflow result (default Format). SaturateFinite always gives the
    /// largest finite magnitude. Format depends on the destination: with
    /// infinity, it gives infinity for RNE, positive RUP and negative RDN,
    /// and otherwise the largest finite magnitude; with NaN but no infinity
    /// (E4M3FN), it gives NaN, signed unless nan is Canonical (0x7f); with
    /// neither (E2M1), it gives the largest finite magnitude.
    Overflow overflow = Format;
    /// Positive replaces every zero result by positive zero (default Signed).
    Zeros zeros = Signed;
    /// NaN result selection and quieting for arithmetic, conversion and
    /// min/max; Canonical discards sign and payload (default SignalFirst).
    Nan nan = SignalFirst;
    /// Underflow tininess after rounding to precision or before rounding
    /// (default AfterPrecision).
    Tininess tininess = AfterPrecision;
    /// Comparisons raise Invalid for signaling NaNs only (Quiet) or for any
    /// NaN (Signaling) (default Quiet).
    Comparison comparison = Quiet;
  };

  namespace Word {
    namespace Float {
      /// Throw Word::OutOfLimits with \a l if a field of \a policy is invalid.
      GECODE_WORD_EXPORT void check(const FloatWordPolicy& policy,
                                    const char* l);
      /// Return the float format of \a x; throw Word::OutOfLimits with \a l
      /// if its domain type is not a float format.
      GECODE_WORD_EXPORT WordDomainType float_format(WordVar x, const char* l);
    } // namespace Float
  } // namespace Word

  /** \brief Direct numerical conversion under an optional policy
   *
   * Supports all pairs among FP32, BF16, binary16, E4M3FN and E2M1,
   * including self-conversion. Each conversion rounds directly to the result
   * format, never through an intermediate format. Unlike reinterpret(),
   * conversion applies the rounding, NaN, subnormal and zero fields of the
   * policy, also when both formats are equal. Invalid policy fields throw
   * Word::OutOfLimits.
   *
   * A tiny source or result keeps exactly the encodings that some admitted
   * source, result and status combination supports when it has a value set,
   * and otherwise their cube and bounds. Each propagation filters the tiny
   * sides in one exact pass, so the operation takes no propagation level.
   * Wide filtering is sound but can retain holes.
   *
   * A NaN keeps its sign, and its payload keeps its high bits: narrowing
   * drops low payload bits and widening appends zeros. The result is quiet
   * even when no payload bit remains. E4M3FN NaNs convert with an empty
   * payload, and conversion to E4M3FN keeps only the sign. Payload loss sets
   * no flag; a signaling source sets I. Converting a NaN to E2M1 gives 0x7
   * with I. An infinity stays infinite in FP32, BF16 and binary16, also
   * under SaturateFinite. In E4M3FN and E2M1 it gives the overflow result
   * with O and X, so FP32 infinity converts to 0x7f in E4M3FN, or to 0x7e
   * with SaturateFinite.
   */
  GECODE_WORD_EXPORT void
  fp_convert(Home home, WordVar source, WordVar result,
             const FloatWordPolicy& policy = FloatWordPolicy());
  /// Conversion with correlated per-operation flags in 0..31.
  GECODE_WORD_EXPORT void
  fp_convert(Home home, WordVar source, WordVar result, IntVar status,
             const FloatWordPolicy& policy = FloatWordPolicy());

  /** \brief Raw classes, including the sign bit (true for negative encodings)
   *
   * Classes inspect the raw encoding: no policy applies and subnormals are
   * never flushed. In E4M3FN, the infinite and signaling NaN classes are
   * always false; in E2M1, so are the infinite and NaN classes.
   */
  enum class FloatWordClass : int {
    FWC_ZERO,          ///< Zero
    FWC_SUBNORMAL,     ///< Subnormal
    FWC_NORMAL,        ///< Normal
    FWC_FINITE,        ///< Zero, subnormal or normal
    FWC_INFINITE,      ///< Infinity
    FWC_NAN,           ///< NaN
    FWC_QUIET_NAN,     ///< Quiet NaN
    FWC_SIGNALING_NAN, ///< Signaling NaN
    FWC_SIGN           ///< Sign bit set
  };
  /// Zero
  static constexpr FloatWordClass FWC_ZERO = FloatWordClass::FWC_ZERO;
  /// Subnormal
  static constexpr FloatWordClass FWC_SUBNORMAL = FloatWordClass::FWC_SUBNORMAL;
  /// Normal
  static constexpr FloatWordClass FWC_NORMAL = FloatWordClass::FWC_NORMAL;
  /// Zero, subnormal or normal
  static constexpr FloatWordClass FWC_FINITE = FloatWordClass::FWC_FINITE;
  /// Infinity
  static constexpr FloatWordClass FWC_INFINITE = FloatWordClass::FWC_INFINITE;
  /// NaN
  static constexpr FloatWordClass FWC_NAN = FloatWordClass::FWC_NAN;
  /// Quiet NaN
  static constexpr FloatWordClass FWC_QUIET_NAN = FloatWordClass::FWC_QUIET_NAN;
  /// Signaling NaN
  static constexpr FloatWordClass FWC_SIGNALING_NAN =
    FloatWordClass::FWC_SIGNALING_NAN;
  /// Sign bit set
  static constexpr FloatWordClass FWC_SIGN = FloatWordClass::FWC_SIGN;
  /** \brief Constrain truth to exact equivalence with the raw classification
   *
   * On a tiny word, truth is fixed once all remaining encodings agree, and
   * an assigned truth removes the encodings of the other outcome: exactly
   * with a value set, otherwise down to the cube and bounds of the rest.
   */
  GECODE_WORD_EXPORT void fp_classify(Home home, WordVar x,
                                      FloatWordClass cls, BoolVar truth);

  /** \brief Toggle the sign bit, preserving payloads and signaling NaNs
   *
   * The sign operations change only the sign bit: they do not quiet NaNs,
   * flush subnormals or set flags. Both words must have the same format;
   * otherwise they throw Word::FormatMismatch. On tiny words, one exact
   * pass keeps the supported encodings: exactly with value sets, otherwise
   * their cube and bounds. x and y may be the same word: negation then
   * fails and fp_abs() keeps the encodings without the sign bit.
   */
  GECODE_WORD_EXPORT void fp_negate(Home home, WordVar x, WordVar y);
  /// Clear the sign bit, preserving payloads and signaling NaNs.
  GECODE_WORD_EXPORT void fp_abs(Home home, WordVar x, WordVar y);

  /** \brief Same-format exact square root, with optional correlated flags
   *
   * Supports E2M1, E4M3FN, BF16 and FP32. On tiny words, one exact pass
   * keeps the encodings and flags of supported combinations: exactly with
   * value sets, otherwise their cube and bounds; the operation takes no
   * propagation level. Wide filtering uses rounding regions and may retain
   * unsupported interior encodings until assignment. The overflow and
   * comparison fields have no effect, since a finite square root in the
   * same format cannot overflow. The square root of a zero is that zero,
   * and of a negative nonzero value, including negative infinity, it is
   * invalid. In E2M1, sqrt(0.5) sets U and X under every rounding direction
   * and both tininess rules, also when RUP rounds it to the normal 1.
   */
  GECODE_WORD_EXPORT void
  fp_sqrt(Home home, WordVar source, WordVar result,
          const FloatWordPolicy& policy = FloatWordPolicy());
  GECODE_WORD_EXPORT void
  fp_sqrt(Home home, WordVar source, WordVar result, IntVar status,
          const FloatWordPolicy& policy = FloatWordPolicy());

  /** \brief Numerical relations between floating words
   *
   * All ordered relations are false when an operand is a NaN.
   */
  enum class FloatWordRelType : int {
    FWRT_EQ,        ///< Equal (\f$=\f$)
    FWRT_NQ,        ///< Not equal, or unordered
    FWRT_LE,        ///< Less (\f$<\f$)
    FWRT_LQ,        ///< Less or equal (\f$\leq\f$)
    FWRT_GR,        ///< Greater (\f$>\f$)
    FWRT_GQ,        ///< Greater or equal (\f$\geq\f$)
    FWRT_UNORDERED, ///< At least one operand is a NaN
    FWRT_ORDERED    ///< Neither operand is a NaN
  };
  /// Equal (\f$=\f$)
  static constexpr FloatWordRelType FWRT_EQ = FloatWordRelType::FWRT_EQ;
  /// Not equal, or unordered
  static constexpr FloatWordRelType FWRT_NQ = FloatWordRelType::FWRT_NQ;
  /// Less (\f$<\f$)
  static constexpr FloatWordRelType FWRT_LE = FloatWordRelType::FWRT_LE;
  /// Less or equal (\f$\leq\f$)
  static constexpr FloatWordRelType FWRT_LQ = FloatWordRelType::FWRT_LQ;
  /// Greater (\f$>\f$)
  static constexpr FloatWordRelType FWRT_GR = FloatWordRelType::FWRT_GR;
  /// Greater or equal (\f$\geq\f$)
  static constexpr FloatWordRelType FWRT_GQ = FloatWordRelType::FWRT_GQ;
  /// At least one operand is a NaN
  static constexpr FloatWordRelType FWRT_UNORDERED =
    FloatWordRelType::FWRT_UNORDERED;
  /// Neither operand is a NaN
  static constexpr FloatWordRelType FWRT_ORDERED =
    FloatWordRelType::FWRT_ORDERED;
  /** \brief Numerical comparison, with exact Boolean equivalence
   *
   * Two operands compare as less, equal, greater or unordered. Infinities
   * order normally, the two zeros are equal and a NaN operand makes the
   * outcome unordered, so FWRT_NQ holds and the other relations except
   * FWRT_UNORDERED fail. The negation of FWRT_LE is therefore not FWRT_GQ,
   * and comparing a NaN with itself under FWRT_EQ is false. The status
   * can only be I: with Quiet it is set for a signaling operand, with
   * Signaling for any NaN operand. Subnormal inputs follow
   * \c input_subnormals; other policy fields have no effect.
   *
   * On tiny words, with or without value sets, \a ipl selects the
   * propagation:
   *  - IPL_DOM is domain consistent on value-set words: every
   *    propagation enumerates the remaining operand pairs and keeps
   *    exactly the supported operand encodings, truth values and flags.
   *    A word without a value set keeps the tightest cube and bounds of
   *    its supported encodings.
   *  - The default level, also for IPL_VAL and IPL_BND, decides truth and
   *    status from the outcomes that the least and greatest numerical
   *    values and the NaNs of the operands allow, and keeps each operand
   *    encoding that an allowed outcome supports against the other
   *    operand's numerical bounds. Once an operand is assigned, or both
   *    operands are the same word, it filters as IPL_DOM.
   *
   * FP32 and BF16 keep their propagation at every level: it is sound,
   * exact on assigned operands, and can retain unsupported values.
   */
  GECODE_WORD_EXPORT void fp_compare(Home home, WordVar x,
                                     FloatWordRelType predicate, WordVar y,
                                     BoolVar truth,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_compare(Home home, WordVar x,
                                     FloatWordRelType predicate, WordVar y,
                                     BoolVar truth, IntVar status,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_compare(Home home, WordVar x,
                                     FloatWordRelType predicate, WordVar y,
                                     BoolVar truth,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_compare(Home home, WordVar x,
                                     FloatWordRelType predicate, WordVar y,
                                     BoolVar truth, IntVar status,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  /** \brief NaN-propagating minimum
   *
   * A NaN operand gives the NaN that the policy selects from the original
   * operands. Otherwise minimum and maximum select an operand: the minimum
   * of two zeros of opposite signs is -0 and their maximum is +0. Rounding,
   * overflow and tininess have no effect. Input subnormals are treated
   * first; output subnormals and \c zeros then apply to the selection. A
   * signaling operand sets I, and flushing a selected subnormal sets U and
   * X; no other flag is possible.
   *
   * On tiny words, with or without value sets, \a ipl selects the
   * propagation of all four minimum and maximum operations:
   *  - IPL_DOM is domain consistent on value-set words: every
   *    propagation enumerates the remaining operand pairs and keeps
   *    exactly the supported operand and result encodings and flags. A
   *    word without a value set keeps the tightest cube and bounds of
   *    its supported encodings.
   *  - The default level, also for IPL_VAL and IPL_BND, keeps in one pass
   *    over each word the encodings that the least and greatest
   *    numerical values and the NaNs of the other words support. Ties of
   *    zeros keep both signs. Once an operand is assigned, or both operands
   *    are the same word, it filters as IPL_DOM.
   *
   * FP32 and BF16 keep their propagation at every level: it is sound,
   * exact on assigned operands, and can retain unsupported values.
   */
  GECODE_WORD_EXPORT void fp_minimum(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum(Home home, WordVar x, WordVar y,
                                     WordVar result, IntVar status,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum(Home home, WordVar x, WordVar y,
                                     WordVar result, IntVar status,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  /// NaN-propagating maximum, propagated as fp_minimum().
  GECODE_WORD_EXPORT void fp_maximum(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum(Home home, WordVar x, WordVar y,
                                     WordVar result, IntVar status,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum(Home home, WordVar x, WordVar y,
                                     WordVar result,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum(Home home, WordVar x, WordVar y,
                                     WordVar result, IntVar status,
                                     const FloatWordPolicy& policy,
                                     IntPropLevel ipl = IPL_DEF);
  /** \brief Number-preferring minimum (IEEE 754-2019 \c minimumNumber)
   *
   * The result is a NaN only when both operands are NaNs; otherwise a NaN
   * operand, quiet or signaling, yields the other operand. Any signaling
   * operand raises invalid. Selection between numbers and propagation
   * follow fp_minimum().
   */
  GECODE_WORD_EXPORT void fp_minimum_number(Home home, WordVar x, WordVar y,
                                            WordVar result,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum_number(Home home, WordVar x, WordVar y,
                                            WordVar result, IntVar status,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum_number(Home home, WordVar x, WordVar y,
                                            WordVar result,
                                            const FloatWordPolicy& policy,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_minimum_number(Home home, WordVar x, WordVar y,
                                            WordVar result, IntVar status,
                                            const FloatWordPolicy& policy,
                                            IntPropLevel ipl = IPL_DEF);
  /** \brief Number-preferring maximum (IEEE 754-2019 \c maximumNumber)
   *
   * The result is a NaN only when both operands are NaNs; otherwise a NaN
   * operand, quiet or signaling, yields the other operand. Any signaling
   * operand raises invalid. Selection between numbers and propagation
   * follow fp_minimum().
   */
  GECODE_WORD_EXPORT void fp_maximum_number(Home home, WordVar x, WordVar y,
                                            WordVar result,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum_number(Home home, WordVar x, WordVar y,
                                            WordVar result, IntVar status,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum_number(Home home, WordVar x, WordVar y,
                                            WordVar result,
                                            const FloatWordPolicy& policy,
                                            IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_maximum_number(Home home, WordVar x, WordVar y,
                                            WordVar result, IntVar status,
                                            const FloatWordPolicy& policy,
                                            IntPropLevel ipl = IPL_DEF);
  /** \brief Exact encoded arithmetic with optional correlated status
   *
   * All words must have the same format: E2M1, E4M3FN, BF16 or FP32.
   * Every policy field combination is supported. Status is the correlated
   * five-flag mask in 0..31. Omitting it allocates no status variable.
   * Subtraction selects a NaN from the original operands before it negates
   * b. Assigned operands always give the exact result and flags.
   *
   * On tiny words, with or without value sets, \a ipl selects the
   * propagation:
   *  - IPL_DOM is domain consistent on value-set words: every
   *    propagation enumerates the remaining operand pairs and keeps
   *    exactly the supported operand and result encodings and flags. A
   *    word without a value set keeps the tightest cube and bounds of
   *    its supported encodings.
   *  - The default level, also for IPL_VAL and IPL_BND, reads each word
   *    once. The result keeps the encodings between the rounded least and
   *    greatest exact results of the operands' numerical bounds, the NaNs
   *    of NaN operands and of overflow, and for division the results of
   *    division by zero. Each operand keeps the encodings that the other
   *    operand's numerical bounds can combine into an exact result that
   *    rounds to an admitted result, and the encodings that pair with an
   *    admitted NaN or division by zero. The status keeps the flags that
   *    these outcomes allow. Once an operand is assigned, or both operands
   *    are the same word, it filters as IPL_DOM.
   *
   * FP32 and BF16 keep their propagation at every level: it is sound,
   * exact on assigned operands, and can retain unsupported values. While
   * an operand admits a NaN or an infinity, wide addition and subtraction
   * wait for both operands to be assigned.
   */
  GECODE_WORD_EXPORT void fp_add(Home home, WordVar a, WordVar b,
                                 WordVar result,
                                 IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_add(Home home, WordVar a, WordVar b,
                                 WordVar result, IntVar status,
                                 IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_add(Home home, WordVar a, WordVar b,
                                 WordVar result,
                                 const FloatWordPolicy& policy,
                                 IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_add(Home home, WordVar a, WordVar b,
                                 WordVar result, IntVar status,
                                 const FloatWordPolicy& policy,
                                 IntPropLevel ipl = IPL_DEF);
  /// Subtraction a-b, propagated as fp_add().
  GECODE_WORD_EXPORT void fp_subtract(Home home, WordVar a, WordVar b,
                                      WordVar result,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_subtract(Home home, WordVar a, WordVar b,
                                      WordVar result, IntVar status,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_subtract(Home home, WordVar a, WordVar b,
                                      WordVar result,
                                      const FloatWordPolicy& policy,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_subtract(Home home, WordVar a, WordVar b,
                                      WordVar result, IntVar status,
                                      const FloatWordPolicy& policy,
                                      IntPropLevel ipl = IPL_DEF);
  /// Multiplication a*b, propagated as fp_add().
  GECODE_WORD_EXPORT void fp_multiply(Home home, WordVar a, WordVar b,
                                      WordVar result,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_multiply(Home home, WordVar a, WordVar b,
                                      WordVar result, IntVar status,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_multiply(Home home, WordVar a, WordVar b,
                                      WordVar result,
                                      const FloatWordPolicy& policy,
                                      IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_multiply(Home home, WordVar a, WordVar b,
                                      WordVar result, IntVar status,
                                      const FloatWordPolicy& policy,
                                      IntPropLevel ipl = IPL_DEF);
  /// Division a/b, propagated as fp_add().
  GECODE_WORD_EXPORT void fp_divide(Home home, WordVar a, WordVar b,
                                    WordVar result,
                                    IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_divide(Home home, WordVar a, WordVar b,
                                    WordVar result, IntVar status,
                                    IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_divide(Home home, WordVar a, WordVar b,
                                    WordVar result,
                                    const FloatWordPolicy& policy,
                                    IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_divide(Home home, WordVar a, WordVar b,
                                    WordVar result, IntVar status,
                                    const FloatWordPolicy& policy,
                                    IntPropLevel ipl = IPL_DEF);

  /** \brief Exact single-rounding fused multiply-add
   *
   * Supports matching E2M1, E4M3FN, BF16 or FP32 words and
   * BF16*BF16+FP32 with FP32 result. Status is the optional correlated
   * mask in 0..31; omitting it allocates no status variable. Aliases
   * preserve variable identities. Assigned inputs always receive exact
   * checking. No intermediate product rounding or full ternary table is
   * used.
   *
   * On tiny words, with or without value sets, \a ipl selects the
   * propagation:
   *  - IPL_DOM is domain consistent on value-set words: every
   *    propagation enumerates the remaining triples of distinct input
   *    words, with no limit, and keeps exactly the supported operand and
   *    result encodings and flags. A word without a value set keeps the
   *    tightest cube and bounds of its supported encodings.
   *  - The default level, also for IPL_VAL and IPL_BND, reads each word
   *    once. The result keeps the encodings between the rounded least and
   *    greatest exact results of the operands' numerical bounds and the
   *    NaNs of NaN operands and of overflow. Each operand keeps the
   *    encodings that the other operands' numerical bounds can combine
   *    into an exact result that rounds to an admitted result, and the
   *    encodings that pair with an admitted NaN. The status keeps the flags
   *    that these outcomes allow. Once at most one distinct input word is
   *    unassigned, it filters as IPL_DOM.
   *
   * FP32 and BF16 keep their propagation at every level. It checks assigned
   * inputs exactly and, once no input admits a NaN or an infinity, projects
   * the finite intervals of the words onto each other at every domain size.
   * It is sound and can retain unsupported values. As extra work on top, a
   * propagation whose distinct wide inputs admit at most 2,048 assignments,
   * counted before any enumeration, scans them all and keeps the tightest
   * cube and bounds of the supported encodings and exactly the supported
   * flags.
   *
   * A finite product above the largest finite value is not an infinity:
   * FP32 max*2-max is max with no flag. The mixed signature decodes and
   * flushes each operand in its own format and rounds the exact a*b+c once
   * to FP32. A selected BF16 NaN keeps the high bits of its payload in FP32,
   * as in fp_convert(), without a conversion flag.
   */
  GECODE_WORD_EXPORT void fp_fma(Home home, WordVar a, WordVar b,
                                 WordVar c, WordVar result,
                                 IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_fma(Home home, WordVar a, WordVar b,
                                 WordVar c, WordVar result,
                                 IntVar status, IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_fma(Home home, WordVar a, WordVar b,
                                 WordVar c, WordVar result,
                                 const FloatWordPolicy& policy,
                                 IntPropLevel ipl = IPL_DEF);
  GECODE_WORD_EXPORT void fp_fma(Home home, WordVar a, WordVar b,
                                 WordVar c, WordVar result,
                                 IntVar status,
                                 const FloatWordPolicy& policy,
                                 IntPropLevel ipl = IPL_DEF);


  namespace Word {
    namespace Float {
      /** \brief Post fp_classify() on the bits of \a x read in \a format
       *
       * \a x may have any domain type of the width of \a format, so
       * decompositions can classify intermediate words. \a format must be a
       * float format.
       */
      GECODE_WORD_EXPORT void classify(Home home, WordVar x,
                                       WordDomainType format,
                                       FloatWordClass cls, BoolVar truth);
    } // namespace Float
  } // namespace Word
} // namespace Gecode
#endif

// STATISTICS: word-post
