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
  /**
   * \defgroup TaskModelWordFloat Floating-point words
   * \ingroup TaskModelWord
   *
   * A FloatWordVar labels a WordVar with a floating-point format. The
   * numerical constraints relate encoded operands to an encoded result or a
   * Boolean, and to an optional status mask, under an explicit
   * FloatWordPolicy. They are exact: no host floating-point arithmetic,
   * rounding mode, exception state, compiler fusion or evaluation order
   * supplies any part of their meaning. Filtering can retain unsupported
   * values, but every complete assignment satisfies the exact relation.
   *
   * \anchor FloatWordFormats
   * <b>Formats.</b> A finite encoding with sign s, stored exponent E and
   * fraction F, in a format with bias b and f fraction bits, denotes
   * \f$(-1)^s (2^f + F)\, 2^{E-b-f}\f$ when E is nonzero and
   * \f$(-1)^s F\, 2^{1-b-f}\f$ when E is zero. The precision is p = f + 1.
   *
   * | Format   | Bits | Exponent/fraction | Bias | Smallest normal | Smallest positive | Maximum finite       |
   * | -------- | ---- | ----------------- | ---- | --------------- | ----------------- | -------------------- |
   * | FP32     | 32   | 8/23              | 127  | 2^-126          | 2^-149            | 0x7f7fffff           |
   * | BF16     | 16   | 8/7               | 127  | 2^-126          | 2^-133            | 0x7f7f               |
   * | binary16 | 16   | 5/10              | 15   | 2^-14           | 2^-24             | 65504, 0x7bff        |
   * | E4M3FN   | 8    | 4/3               | 7    | 2^-6            | 2^-9              | 448, 0x7e            |
   * | E2M1     | 4    | 2/1               | 1    | 1               | 0.5               | 6, 0x7               |
   *
   * Every format has two zeros, and the high bit is the sign. FP32, BF16 and
   * binary16 have infinities (0x7f800000, 0x7f80, 0x7c00) and NaNs: a
   * maximal exponent with a nonzero fraction. The top fraction bit is set
   * in a quiet NaN and clear in a signaling NaN; the remaining fraction bits
   * are the payload. E4M3FN has no infinity and two quiet NaNs, 0x7f and
   * 0xff, without payload. E2M1 has neither; its positive codes 0 to 7 denote
   * 0, 0.5, 1, 1.5, 2, 3, 4 and 6.
   *
   * Arithmetic, fused multiply-add, square root, comparison and minimum or
   * maximum support FP32, BF16, E4M3FN and E2M1, with one format for all
   * operands and the result. The only mixed signature is
   * BF16 * BF16 + FP32 with an FP32 result for fp_fma(). Conversion,
   * classification, sign operations and ULP observations also support
   * binary16. Mismatched formats throw Word::FormatMismatch and unsupported
   * formats throw Word::OutOfLimits. Exceptional operand values are never
   * posting errors.
   *
   * \anchor FloatWordStatus
   * <b>Status.</b> The status of a numerical operation is an IntVar in 0..31
   * holding the flags invalid (1), divide-by-zero (2), overflow (4),
   * underflow (8) and inexact (16), written I, D, O, U and X below. The
   * flags belong to one operation: they start clear and are not sticky.
   * The status is correlated with the result, so a posting constrains the
   * tuples of operands, result and status, not each of them separately.
   * Omitting the status projects it out.
   * Classification and sign operations have no status.
   *
   * \anchor FloatWordEvaluation
   * <b>Evaluation.</b> A numerical operation proceeds in this order:
   *  1. With \c input_subnormals equal to SignedZero, each subnormal operand
   *     becomes the zero with its sign. This sets no flag. The rules below
   *     refer to these effective operands.
   *  2. NaN operands and the invalid, divide-by-zero and infinite cases
   *     below decide the result.
   *  3. Otherwise the exact real result is rounded once, directly to the
   *     destination's finite values including its subnormals. Division uses
   *     the exact quotient and fused multiply-add the exact a*b+c. Square
   *     root compares the squares of neighbouring results with the input;
   *     RNE compares the input with the square of their midpoint.
   *  4. Overflow, tininess and inexactness are decided as described below.
   *     Then \c output_subnormals and finally \c zeros apply.
   *
   * There is no intermediate FP32 computation for BF16 or tiny formats.
   *
   * \anchor FloatWordNan
   * <b>NaNs.</b> The operand order is (a,b), (a,b,c) for fused multiply-add,
   * and (a) for square root and conversion. SignalFirst selects the first
   * signaling NaN, or otherwise the first quiet NaN; OperandFirst selects the
   * first NaN. Both return the selected NaN with its sign and payload and
   * the quiet bit set. Canonical returns the positive canonical NaN:
   * 0x7fc00000 in FP32, 0x7fc0 in BF16, 0x7e00 in binary16 and 0x7f in
   * E4M3FN. A generated invalid result is
   * that positive canonical NaN under every selection mode. A signaling
   * operand sets I, even when another operand supplies the result.
   *
   * \anchor FloatWordInvalid
   * <b>Invalid operations.</b> If no operand is a NaN, these cases return
   * the canonical NaN and set only I:
   *
   * | Operation       | Invalid operands                                       |
   * | --------------- | ------------------------------------------------------ |
   * | fp_add()        | Infinities of opposite signs                           |
   * | fp_subtract()   | Infinities of the same sign                            |
   * | fp_multiply()   | Zero and infinity, in either order                     |
   * | fp_divide()     | Zero by zero, or infinity by infinity                  |
   * | fp_fma()        | Zero times infinity, or infinite product plus opposite infinity |
   * | fp_sqrt()       | Negative nonzero, including negative infinity          |
   *
   * A NaN operand otherwise suppresses these checks, but fp_fma() detects
   * zero times infinity also when c is a quiet NaN; it then returns the
   * selected NaN with I. E2M1 has no NaN: an invalid result is 0x7 (+6)
   * with I.
   *
   * \anchor FloatWordInfinity
   * <b>Infinities and division by zero.</b> A finite nonzero value divided
   * by zero sets only D. The result has the exclusive-or of the operand
   * signs and is infinite in FP32 and BF16 under both overflow policies.
   * In E4M3FN it is the NaN with that sign (positive with Canonical), or
   * the largest finite magnitude with SaturateFinite; in E2M1 it is the
   * largest finite magnitude. This is not an overflow. Infinity divided by
   * zero is infinite and a finite value divided by infinity is zero, with
   * no flag. Other valid arithmetic on infinities gives the signed infinity
   * without flags, also under SaturateFinite.
   *
   * \anchor FloatWordZeros
   * <b>Signed zeros.</b> A zero result has the following sign before
   * \c zeros applies; Positive then makes every zero result positive. The
   * sign of a zero never sets X on its own.
   *
   * | Case                                                    | Sign of the zero              |
   * | ------------------------------------------------------- | ----------------------------- |
   * | Sum of zeros of equal sign                              | Their sign                    |
   * | Sum of zeros of opposite signs, or exact cancellation   | Negative under RDN, otherwise positive |
   * | Difference                                              | As the sum with b negated     |
   * | Exact zero product or quotient                          | Exclusive-or of operand signs |
   * | fp_fma() with zero a*b+c                                | As the sum of a*b and c       |
   * | Square root or conversion of a zero                     | The sign of the input         |
   * | Nonzero exact result rounded or flushed to zero         | The sign of the exact result  |
   *
   * In fp_fma(), a zero product has the exclusive-or of the signs of a and b.
   *
   * \anchor FloatWordRange
   * <b>Overflow and underflow.</b> Let r be the exact finite nonzero result,
   * M the largest finite magnitude and N the smallest positive normal of the
   * destination. The auxiliary value u rounds r to precision p in the policy's
   * direction with an unbounded exponent. It only decides the range flags; the
   * result is always rounded from r.
   *
   * Overflow means |u| > M. It sets O and X and returns the result that
   * \c overflow selects, even when that result is a NaN or finite. A value
   * above M whose u is M sets only X. Under RNE, converting 464 to E4M3FN
   * gives 448 with X, as the tie rounds u to 448, while converting 7 to E2M1
   * overflows, as the tie rounds u to 8. Under directed rounding the test
   * also uses u, not r.
   *
   * | Destination   | Format                                                         | SaturateFinite       |
   * | ------------- | -------------------------------------------------------------- | -------------------- |
   * | With infinity | Infinity for RNE, positive RUP and negative RDN; else largest  | Largest finite       |
   * | E4M3FN        | NaN; Canonical gives 0x7f                                      | Largest finite (448) |
   * | E2M1          | Largest finite (6)                                             | Largest finite (6)   |
   *
   * Every overflow result has the sign of r, except a Canonical NaN.
   *
   * E4M3FN overflow gives a NaN under every rounding direction, because the
   * format has no infinity; SaturateFinite selects the finite alternative.
   * For example, converting FP32 480 to E4M3FN under RTZ gives 0x7f with
   * O and X, because u = 480 > 448. SaturateFinite gives 0x7e (448) with
   * the same flags.
   *
   * The result is tiny when |u| < N under AfterPrecision and when |r| < N
   * under BeforeRounding. AfterPrecision does not inspect the final encoding:
   * a tiny u can round to N in the destination. With \c output_subnormals
   * equal to Preserve, X means that the result differs from r and U means
   * tiny and inexact, so an exact subnormal result sets neither. SignedZero
   * replaces a nonzero subnormal result by the zero with its sign and sets U
   * and X, even when the subnormal was exact. Invalid and divide-by-zero
   * results set only their own flag, and a NaN from a NaN operand or an
   * invalid operation is never inexact.
   */

  /** \addtogroup TaskModelWordFloat
   * @{
   */
  /// Closed encoding formats, independent of numerical operation policy.
  enum class FloatWordFormat : int {
    FWF_FP32,     ///< IEEE 754 binary32
    FWF_BF16,     ///< bfloat16: binary32 exponent range, 7 fraction bits
    FWF_BINARY16, ///< IEEE 754 binary16
    FWF_E4M3FN,   ///< OCP FP8 E4M3FN: NaN but no infinity
    FWF_E2M1      ///< OCP FP4 E2M1: neither infinity nor NaN
  };
  /// IEEE 754 binary32
  static constexpr FloatWordFormat FWF_FP32 = FloatWordFormat::FWF_FP32;
  /// bfloat16: binary32 exponent range, 7 fraction bits
  static constexpr FloatWordFormat FWF_BF16 = FloatWordFormat::FWF_BF16;
  /// IEEE 754 binary16
  static constexpr FloatWordFormat FWF_BINARY16 = FloatWordFormat::FWF_BINARY16;
  /// OCP FP8 E4M3FN: NaN but no infinity
  static constexpr FloatWordFormat FWF_E4M3FN = FloatWordFormat::FWF_E4M3FN;
  /// OCP FP4 E2M1: neither infinity nor NaN
  static constexpr FloatWordFormat FWF_E2M1 = FloatWordFormat::FWF_E2M1;

  /// Return the encoded width; reject unknown format IDs.
  GECODE_WORD_EXPORT unsigned int float_word_width(FloatWordFormat format);

  /** \brief Numerical policy value
   *
   * A plain assignable value whose defaults select IEEE 754 behaviour with
   * round-to-nearest-even. Every combination of field values is supported.
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
  /** @} */

  namespace Word {
    namespace Float {
      /// Throw Word::OutOfLimits with \a l if a field of \a policy is invalid.
      GECODE_WORD_EXPORT void check(const FloatWordPolicy& policy,
                                    const char* l);
    } // namespace Float
  } // namespace Word

  /** \addtogroup TaskModelWordFloat
   * @{
   */
  /** \brief Exact floating encoding handle; no floating arithmetic on the host
   *
   * The format labels this handle, not the underlying WordVar, so formats
   * of equal width remain different. Ordinary copies share both variable
   * identities; update() forwards both when cloning. Existing Word arrays
   * and branchers accept bits() explicitly.
   *
   * E4M3FN and E2M1 are the tiny formats. Their numerical operations,
   * classification and sign operations work on an encoding IntVar bound to
   * the bits, and throw Word::OutOfLimits for a handle without one. Create
   * the binding once with the four-argument constructor and copy that
   * handle for every use, so that each restriction of the codes reaches all
   * operations. Binding the same bits to a second IntVar can lose interior
   * holes. No operation allocates a replacement IntVar. FP32, BF16 and
   * binary16 are the wide formats and have no binding.
   */
  class GECODE_WORD_EXPORT FloatWordVar {
  private:
    WordVar word;
    FloatWordFormat format_id;
    IntVar codes;

  public:
    /// Uninitialized handle, only for assignment or clone update.
    FloatWordVar(void);
    /// Construct a word without an encoding IntVar.
    FloatWordVar(Space& home, FloatWordFormat format,
                 WordDomainType domain = WDT_UNSIGNED);
    /// Explicitly label existing bits; check their width.
    explicit FloatWordVar(FloatWordFormat format, WordVar bits);
    /// Bind caller-owned tiny bits and codes by posting channel_encoding once.
    FloatWordVar(Space& home, FloatWordFormat format, WordVar bits,
                 IntVar encoding);
    /// Exact encoded constant; rejects bits outside the format width.
    static FloatWordVar from_bits(Space& home, FloatWordFormat format,
                                  WordValue bits);
    WordVar bits(void) const;
    FloatWordFormat format(void) const;
    bool has_encoding(void) const;
    /// Shared encoding domain; throws Word::OutOfLimits when absent.
    IntVar encoding(void) const;
    /// Rebind both variables during Space cloning.
    void update(Space& home, FloatWordVar& other);
    /// Relabel equal-width bits, preserving any encoding binding.
    FloatWordVar reinterpret_bits(FloatWordFormat format) const;
  };

  /** \brief Direct numerical conversion under an explicit operation policy
   *
   * Supports all pairs among FP32, BF16, binary16, E4M3FN and E2M1,
   * including self-conversion. Tiny operands require their shared encoding
   * bindings. Each conversion rounds directly to the result format, never
   * through an intermediate format. Unlike reinterpret_bits(), conversion
   * applies the rounding, NaN, subnormal and zero fields of the policy, also
   * when both formats are equal. Invalid policy fields and missing bindings
   * throw Word exceptions. Wide filtering is sound but can retain holes.
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
  GECODE_WORD_EXPORT void fp_convert(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar source, FloatWordVar result);
  /// Conversion with correlated per-operation flags in 0..31.
  GECODE_WORD_EXPORT void fp_convert(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar source, FloatWordVar result,
                                     IntVar status);

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
  /// Constrain truth to exact equivalence with the raw classification.
  GECODE_WORD_EXPORT void fp_classify(Home home, FloatWordVar x,
                                      FloatWordClass cls, BoolVar truth);
  /** \brief Toggle the sign bit, preserving payloads and signaling NaNs
   *
   * The sign operations change only the sign bit: they do not quiet NaNs,
   * flush subnormals or set flags. Both handles must have the same format;
   * otherwise they throw Word::FormatMismatch.
   */
  GECODE_WORD_EXPORT void fp_negate(Home home, FloatWordVar x, FloatWordVar y);
  /// Clear the sign bit, preserving payloads and signaling NaNs.
  GECODE_WORD_EXPORT void fp_abs(Home home, FloatWordVar x, FloatWordVar y);

  /** \brief Same-format exact square root, with optional correlated flags
   *
   * Supports E2M1, E4M3FN, BF16 and FP32. Tiny handles require shared
   * encoding bindings; wide filtering uses rounding regions and may retain
   * unsupported interior encodings until assignment. The overflow and
   * comparison fields have no effect, since a finite square root in the
   * same format cannot overflow. The square root of a zero is that zero,
   * and of a negative nonzero value, including negative infinity, it is
   * invalid. In E2M1, sqrt(0.5) sets U and X under every rounding direction
   * and both tininess rules, also when RUP rounds it to the normal 1.
   */
  GECODE_WORD_EXPORT void fp_sqrt(Home home, const FloatWordPolicy& policy,
                                  FloatWordVar source, FloatWordVar result);
  GECODE_WORD_EXPORT void fp_sqrt(Home home, const FloatWordPolicy& policy,
                                  FloatWordVar source, FloatWordVar result,
                                  IntVar status);

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
   */
  GECODE_WORD_EXPORT void fp_compare(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordRelType predicate,
                                     FloatWordVar y, BoolVar truth);
  GECODE_WORD_EXPORT void fp_compare(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordRelType predicate,
                                     FloatWordVar y, BoolVar truth,
                                     IntVar status);
  /** \brief NaN-propagating minimum
   *
   * A NaN operand gives the NaN that the policy selects from the original
   * operands. Otherwise minimum and maximum select an operand: the minimum
   * of two zeros of opposite signs is -0 and their maximum is +0. Rounding,
   * overflow and tininess have no effect. Input subnormals are treated
   * first; output subnormals and \c zeros then apply to the selection. A
   * signaling operand sets I, and flushing a selected subnormal sets U and
   * X; no other flag is possible.
   */
  GECODE_WORD_EXPORT void fp_minimum(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordVar y,
                                     FloatWordVar result);
  GECODE_WORD_EXPORT void fp_minimum(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordVar y,
                                     FloatWordVar result, IntVar status);
  /// NaN-propagating maximum.
  GECODE_WORD_EXPORT void fp_maximum(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordVar y,
                                     FloatWordVar result);
  GECODE_WORD_EXPORT void fp_maximum(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordVar y,
                                     FloatWordVar result, IntVar status);
  /** \brief Number-preferring minimum (IEEE 754-2019 \c minimumNumber)
   *
   * The result is a NaN only when both operands are NaNs; otherwise a NaN
   * operand, quiet or signaling, yields the other operand. Any signaling
   * operand raises invalid. Selection between numbers follows fp_minimum().
   */
  GECODE_WORD_EXPORT void fp_minimum_number(Home home,
                                            const FloatWordPolicy& policy,
                                            FloatWordVar x, FloatWordVar y,
                                            FloatWordVar result);
  GECODE_WORD_EXPORT void
  fp_minimum_number(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                    FloatWordVar y, FloatWordVar result, IntVar status);
  /** \brief Number-preferring maximum (IEEE 754-2019 \c maximumNumber)
   *
   * The result is a NaN only when both operands are NaNs; otherwise a NaN
   * operand, quiet or signaling, yields the other operand. Any signaling
   * operand raises invalid. Selection between numbers follows fp_minimum().
   */
  GECODE_WORD_EXPORT void fp_maximum_number(Home home,
                                            const FloatWordPolicy& policy,
                                            FloatWordVar x, FloatWordVar y,
                                            FloatWordVar result);
  GECODE_WORD_EXPORT void
  fp_maximum_number(Home home, const FloatWordPolicy& policy, FloatWordVar x,
                    FloatWordVar y, FloatWordVar result, IntVar status);
  /** \brief Exact encoded arithmetic with optional correlated status
   *
   * All handles must have the same format: E2M1, E4M3FN, BF16 or FP32.
   * E2M1/E4M3FN use shared encoding bindings. Every policy field combination
   * is supported. Status is the correlated five-flag mask in 0..31. Omitting
   * it allocates no status variable. Subtraction selects a NaN from the
   * original operands before it negates b. Tiny filtering keeps exactly the
   * supported values of the shared encoding domains. Wide filtering is
   * sound and incomplete; assigned tuples always obey the exact operation
   * contract.
   */
  GECODE_WORD_EXPORT void fp_add(Home home, const FloatWordPolicy& policy,
                                 FloatWordVar a, FloatWordVar b,
                                 FloatWordVar result);
  GECODE_WORD_EXPORT void fp_add(Home home, const FloatWordPolicy& policy,
                                 FloatWordVar a, FloatWordVar b,
                                 FloatWordVar result, IntVar status);
  GECODE_WORD_EXPORT void fp_subtract(Home home, const FloatWordPolicy& policy,
                                      FloatWordVar a, FloatWordVar b,
                                      FloatWordVar result);
  GECODE_WORD_EXPORT void fp_subtract(Home home, const FloatWordPolicy& policy,
                                      FloatWordVar a, FloatWordVar b,
                                      FloatWordVar result, IntVar status);
  GECODE_WORD_EXPORT void fp_multiply(Home home, const FloatWordPolicy& policy,
                                      FloatWordVar a, FloatWordVar b,
                                      FloatWordVar result);
  GECODE_WORD_EXPORT void fp_multiply(Home home, const FloatWordPolicy& policy,
                                      FloatWordVar a, FloatWordVar b,
                                      FloatWordVar result, IntVar status);
  GECODE_WORD_EXPORT void fp_divide(Home home, const FloatWordPolicy& policy,
                                    FloatWordVar a, FloatWordVar b,
                                    FloatWordVar result);
  GECODE_WORD_EXPORT void fp_divide(Home home, const FloatWordPolicy& policy,
                                    FloatWordVar a, FloatWordVar b,
                                    FloatWordVar result, IntVar status);

  /** \brief Exact single-rounding fused multiply-add
   *
   * Supports matching E2M1, E4M3FN, BF16 or FP32 handles and
   * BF16*BF16+FP32 with FP32 result. Tiny handles require shared encoding
   * bindings. Status is the optional correlated mask in 0..31.
   * Aliases preserve variable identities. Per propagation, at most 2,048
   * assignments to distinct wide inputs, or 32,768 to distinct tiny inputs,
   * are scanned; the count is computed before any enumeration. Larger wide
   * domains use conservative finite interval projections; larger tiny domains
   * defer.
   * Assigned inputs always receive exact checking. No intermediate product
   * rounding or full ternary table is used.
   *
   * A finite product above the largest finite value is not an infinity:
   * FP32 max*2-max is max with no flag. The mixed signature decodes and
   * flushes each operand in its own format and rounds the exact a*b+c once
   * to FP32. A selected BF16 NaN keeps the high bits of its payload in FP32,
   * as in fp_convert(), without a conversion flag.
   */
  GECODE_WORD_EXPORT void fp_fma(Home home, const FloatWordPolicy& policy,
                                 FloatWordVar a, FloatWordVar b,
                                 FloatWordVar c, FloatWordVar result);
  GECODE_WORD_EXPORT void fp_fma(Home home, const FloatWordPolicy& policy,
                                 FloatWordVar a, FloatWordVar b,
                                 FloatWordVar c, FloatWordVar result,
                                 IntVar status);

  /** \brief Exact local spacing 2^exponent in a named reference format
   *
   * Finite values use max(floor(log2(abs(reference))), emin) - (p-1).
   * Both zeros use emin-(p-1). A power of two belongs to the binade that
   * starts at it. Values need not belong to the reference lattice, and the
   * spacing above its maximum finite value is not capped: FP32 512 has
   * exponent 6 in E4M3FN. The observation reads encoded values only and
   * takes no policy. Tiny operands require their shared encoding binding.
   * Unknown formats throw Word::OutOfLimits. Filtering is sound but can be
   * incomplete; nonfinite inputs fail.
   */
  GECODE_WORD_EXPORT void ulp_spacing(Home home, FloatWordFormat reference_format,
                                      FloatWordVar reference, IntVar exponent);
  /** \brief Exact unsigned distance between finite representable values
   *
   * Endpoints must belong exactly to reference_format, without rounding.
   * Without its sign bit an encoding is a magnitude code m; its rank is -m
   * when negative and m otherwise, and the distance is the absolute
   * difference of the ranks. BF16 1 to 4 is 256 steps and -1 to 1 is 32512.
   * Both zero signs have rank zero; original encodings keep their signs.
   * The distance is unsigned whatever its storage: the span between the
   * finite extremes is 4278190078 in FP32, 65278 in BF16, 63486 in binary16,
   * 252 in E4M3FN and 14 in E2M1.
   * distance has float_word_width(reference_format) bits, including all 32 bits
   * for FP32. Storage may be cube, unsigned-bound or signed-bound.
   * Tiny operands require their shared encoding bindings. Unknown formats or
   * missing bindings throw Word::OutOfLimits; wrong result width throws
   * Word::WidthMismatch. Nonfinite or unrepresentable fixed endpoints fail.
   * Filtering is sound but can be incomplete. Input/result aliases are allowed.
   */
  GECODE_WORD_EXPORT void ulp_steps(Home home, FloatWordFormat reference_format,
                                    FloatWordVar a, FloatWordVar b,
                                    WordVar distance);
  /** \brief Exact normalized absolute error abs(a-b)/ulp(reference)
   *
   * Each value retains its own format and must be finite. No value needs to
   * belong to reference_format's lattice. Tiny values require shared encoding
   * bindings. The error is Q * 2^-ulp_error_scale(reference_format), where the
   * unsigned numerator Q is written as most-significant-first base-2^30
   * digits, with leading zeros retained. Create the digit array with
   * ulp_error_digits(); the relation also bounds every digit. Unknown
   * formats, missing tiny bindings or a digit array of another size throw
   * Word::OutOfLimits. Nonfinite inputs or incompatible digit bounds fail.
   * Aliases are allowed; filtering is sound but can be incomplete. Ordinary
   * lexical array relations and IntLexMaximizeSpace consume the digits
   * without rounding the objective.
   *
   * The posting performs no rounding, conversion or flushing, and both zero
   * signs denote zero. With BF16 as reference format and reference 1, FP32 1
   * and 1+2^-8 have error 1/2, although 1+2^-8 is not a BF16 value; BF16 1
   * and 4 have error 384. To compare the error E with a rational threshold
   * t, compare Q with a constant digit array of the same size, for example
   * with rel() and IRT_LQ, where S is ulp_error_scale(reference_format):
   *
   * | Error condition | Condition on Q                 |
   * | --------------- | ------------------------------ |
   * | E <= t          | Q <= floor(t*2^S)              |
   * | E < t           | Q <= ceil(t*2^S)-1             |
   * | E >= t          | Q >= ceil(t*2^S)               |
   * | E > t           | Q >= floor(t*2^S)+1            |
   *
   * A negative threshold, or one outside the digits' range, makes the
   * condition fail or always hold; it must not wrap. Costs with different
   * reference formats need aligned scales before comparison. The example
   * examples/word-float-error.cpp maximizes this error, and
   * docs/float-word-ulp.md walks through it.
   */
  GECODE_WORD_EXPORT void ulp_error(Home home, FloatWordFormat reference_format,
                                    FloatWordVar a, FloatWordVar b,
                                    FloatWordVar reference,
                                    const IntVarArgs& numerator_digits);
  /** \brief Fresh ulp_error numerator digits for reference_format
   *
   * The array has the size ulp_error requires, and each digit has the range
   * of its position. Unknown formats throw Word::OutOfLimits.
   */
  GECODE_WORD_EXPORT IntVarArray
  ulp_error_digits(Space& home, FloatWordFormat reference_format);
  /// Return S in error = Q * 2^-S; unknown formats throw Word::OutOfLimits.
  GECODE_WORD_EXPORT int ulp_error_scale(FloatWordFormat reference_format);
} // namespace Gecode
#endif

// STATISTICS: word-post
