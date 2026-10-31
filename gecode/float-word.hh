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

  /** \brief Numerical policy value (see docs/float-word-contracts.md)
   *
   * A plain assignable value whose defaults select IEEE 754 behaviour with
   * round-to-nearest-even. Every numerical posting checks all fields, also
   * those the operation ignores, and throws Word::OutOfLimits for a value
   * outside its enumeration. Each posting keeps its own copy, so later changes
   * to the caller's policy do not affect it. Raw observations and sign
   * operations take no policy.
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
    } // namespace Float
  } // namespace Word

  /** \brief Exact floating encoding handle; no floating arithmetic on the host
   *
   * The format labels this handle, not the underlying WordVar. Ordinary copies
   * share both variable identities; update() forwards both when cloning.
   * Existing Word arrays and branchers accept bits() explicitly.
   * Tiny postings require a caller-created encoding binding. Create it once
   * using the four-argument constructor and copy that binding for every use.
   * No operation allocates a replacement IntVar. Wide words have no binding.
   * \ingroup TaskModelWordVars
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
   * bindings. Unlike reinterpret_bits(), conversion applies the rounding, NaN,
   * subnormal and zero fields of the policy. Invalid policy fields and missing
   * bindings throw Word exceptions. Wide filtering is sound but can retain
   * holes.
   */
  GECODE_WORD_EXPORT void fp_convert(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar source, FloatWordVar result);
  /// Conversion with correlated per-operation flags in 0..31.
  GECODE_WORD_EXPORT void fp_convert(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar source, FloatWordVar result,
                                     IntVar status);

  /// Raw classes, including the sign bit (true for negative encodings).
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
  /// Toggle the sign bit, preserving payloads and signaling NaNs.
  GECODE_WORD_EXPORT void fp_negate(Home home, FloatWordVar x, FloatWordVar y);
  /// Clear the sign bit, preserving payloads and signaling NaNs.
  GECODE_WORD_EXPORT void fp_abs(Home home, FloatWordVar x, FloatWordVar y);

  /** \brief Same-format exact square root, with optional correlated flags
   *
   * Supports E2M1, E4M3FN, BF16 and FP32. Tiny handles require shared
   * encoding bindings; wide filtering uses rounding regions and may retain
   * unsupported interior encodings until assignment.
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
  /// Numerical comparison, with exact Boolean equivalence.
  GECODE_WORD_EXPORT void fp_compare(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordRelType predicate,
                                     FloatWordVar y, BoolVar truth);
  GECODE_WORD_EXPORT void fp_compare(Home home, const FloatWordPolicy& policy,
                                     FloatWordVar x, FloatWordRelType predicate,
                                     FloatWordVar y, BoolVar truth,
                                     IntVar status);
  /// NaN-propagating minimum.
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
   * operand raises invalid.
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
   * operand raises invalid.
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
   * it allocates no status variable. Wide filtering is sound and incomplete;
   * assigned tuples always obey the exact operation contract.
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
   * Both zeros use emin-(p-1). Values need not belong to the reference lattice;
   * spacing above its maximum finite value is not capped. Tiny operands require
   * their shared encoding binding. Unknown formats throw Word::OutOfLimits.
   * Filtering is sound but can be incomplete; nonfinite inputs fail.
   */
  GECODE_WORD_EXPORT void ulp_spacing(Home home, FloatWordFormat reference_format,
                                      FloatWordVar reference, IntVar exponent);
  /** \brief Exact unsigned distance between finite representable values
   *
   * Endpoints must belong exactly to reference_format, without rounding.
   * Both zero signs have rank zero; original encodings keep their signs.
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
