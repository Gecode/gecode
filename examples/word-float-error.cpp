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
 */


#include <gecode/driver.hh>
#include <gecode/float-word.hh>
#include <gecode/int.hh>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

using namespace Gecode;

namespace {

  /// How the E4M3FN multiply-accumulate rounds
  enum class Computation {
    Separate, ///< Round the product a*b, then round the sum
    Fused     ///< Round a*b+c once (fused multiply-add)
  };

  /// Smallest input magnitude
  constexpr double min_magnitude = 0.25;
  /// Largest input magnitude
  constexpr double max_magnitude = 3.75;
  /// Sign bit of an E4M3FN code
  constexpr WordValue sign_code = 0x80;

  /// Return \a bits as a hexadecimal literal with \a digits digits
  std::string
  hex_literal(WordValue bits, int digits) {
    std::ostringstream literal;
    literal << "0x" << std::hex << std::setfill('0') << std::setw(digits) << bits;
    return literal.str();
  }

  /*
   * Plain C++ arithmetic on doubles, used to check the reported inputs
   * independently of the Gecode floating-point propagators.
   */

  /// Return the value of the finite E4M3FN code \a code
  double
  decode_e4m3fn(WordValue code) {
    const int exponent = static_cast<int>((code >> 3) & 0xf);
    const int fraction = static_cast<int>(code & 0x7);
    const double magnitude = (exponent == 0) ?
      std::ldexp(fraction,-9) : std::ldexp(8 + fraction,exponent - 10);
    return ((code & sign_code) != 0) ? -magnitude : magnitude;
  }

  /// Return the value of the finite FP32 encoding \a bits
  double
  decode_fp32(WordValue bits) {
    const int exponent = static_cast<int>((bits >> 23) & 0xff);
    const double fraction = static_cast<double>(bits & 0x7fffff);
    const double magnitude = (exponent == 0) ?
      std::ldexp(fraction,-149) : std::ldexp(fraction + 0x800000,exponent - 150);
    return (((bits >> 31) & 1) != 0) ? -magnitude : magnitude;
  }

  /// Return the E4M3FN spacing 2^(max(floor(log2|x|),-6) - 3) at \a x
  double
  spacing_e4m3fn(double x) {
    const int exponent = (x == 0.0) ? -6 : std::max(std::ilogb(x),-6);
    return std::ldexp(1.0,exponent - 3);
  }

  /// Round \a x to E4M3FN, to nearest even, for |x| below the overflow bound
  double
  round_e4m3fn(double x) {
    const double spacing = spacing_e4m3fn(x);
    return std::nearbyint(x / spacing) * spacing;
  }

  /// The multiply-accumulate computed with plain double arithmetic
  struct PlainResult {
    /// Exact value of a*b+c
    double exact;
    /// Value rounded to E4M3FN as the computation rounds
    double rounded;
    /// Distance between both values in E4M3FN ULPs at the exact value
    double error;
  };

  /// Compute a*b+c for inputs \a a, \a b and \a c, which is exact in double
  PlainResult
  evaluate_plain(Computation computation, double a, double b, double c) {
    const double exact = a*b + c;
    const double rounded = (computation == Computation::Fused) ?
      round_e4m3fn(exact) : round_e4m3fn(round_e4m3fn(a*b) + c);
    return {exact, rounded, std::fabs(rounded - exact) / spacing_e4m3fn(exact)};
  }

}

/** \brief Options for the low-precision error example */
class WordFloatErrorOptions : public Options {
public:
  /// Initialize options
  explicit WordFloatErrorOptions(const char* name)
    : Options(name),
      _computation("computation","rounding of the E4M3FN multiply-accumulate",
                   static_cast<int>(Computation::Separate)) {
    _computation.add(static_cast<int>(Computation::Separate),"separate",
                     "round a*b, then round the sum");
    _computation.add(static_cast<int>(Computation::Fused),"fused",
                     "round a*b+c once (fp_fma)");
    add(_computation);
  }
  /// Return how the multiply-accumulate rounds
  Computation get_computation(void) const {
    return static_cast<Computation>(_computation.value());
  }
private:
  /// Rounding of the E4M3FN multiply-accumulate
  Driver::StringOption _computation;
};

/**
 * \brief %Example: Worst-case error of an FP8 multiply-accumulate
 *
 * Machine-learning accelerators run kernels in low-precision formats such
 * as BF16 and FP8 to save memory, bandwidth and energy.  Deciding whether
 * a kernel may run in such a format needs its worst-case error, not the
 * average error that sampling gives: a rare input with a large error can
 * break a computation that is accurate almost everywhere.  This example
 * takes the multiply-accumulate a*b+c, the step that dot products and
 * matrix multiplications repeat, and computes it both in FP8 E4M3FN and
 * in FP32.  Branch-and-bound search finds the inputs on which the two
 * deviate most and, by exhausting the search, proves that no input
 * deviates more.
 *
 * The inputs are E4M3FN values with magnitudes in [1/4, 15/4], an
 * activation-like range of four binades with eight values each.  The
 * inputs a and b are positive and c takes either sign, so a and b take
 * 32 values and c takes 64.  Each input is a WordVar of domain type
 * WDT_E4M3FN, which carries the exact set of its values.  The model
 * compares a, b and the magnitude of c with the range bounds, created by
 * float_constant() from their values, so the domain of c keeps the hole
 * between its negative and positive values.  Multiplication
 * commutes, so the model requires a <= b.  Rounding is to nearest, ties to
 * even, which commutes with negation: negating a and c negates every
 * result, so positive a and b lose no generality.
 *
 * The reference converts the inputs exactly to FP32 and computes a*b+c
 * there.  A product of two E4M3FN values has at most eight significant
 * bits and, on this range, the sum at most 15, so FP32 rounds neither and
 * its result is the exact value of a*b+c.  The reference therefore
 * multiplies and then adds, which gives the same result as fp_fma and
 * propagates much faster.  The error is |r8 - r32| / ulp(r32): the distance between
 * the E4M3FN result r8 and the FP32 result r32, in units of the E4M3FN
 * spacing at r32.  The model computes it with ulp_error, whose exact
 * fixed-point digits are the lexicographic objective.  It does not use
 * ulp_steps, which needs both values on the E4M3FN grid, since the FP32
 * result mostly lies between E4M3FN values.
 *
 * The option \c -computation selects how the E4M3FN computation rounds:
 *  - \c separate (default) rounds the product a*b to E4M3FN and then
 *    rounds the sum, as hardware without a fused multiply-add does.  When
 *    the rounded product cancels against c, nothing of the exact result
 *    remains: for a = 9/32, b = 15/8 and c = -1/2, the exact value is
 *    7/256, but the computation returns 0, an error of 14 ULPs.
 *  - \c fused rounds a*b+c once with fp_fma.  The worst error is half an
 *    ULP, the bound for correct rounding.
 *
 * The example prints the worst inputs, the rounded product for separate
 * rounding, both results and the error.  It then recomputes the reported
 * inputs with plain double arithmetic, which is exact here, rounds to
 * E4M3FN by scaling with the local spacing, and checks that it obtains
 * the same results and error.
 *
 * The search splits the domain of each input at its median in the total
 * order of E4M3FN and tries the lower half first.  Propagation prunes
 * little before all three inputs are assigned, so the proof is in effect
 * an enumeration of the 33,792 input combinations that the symmetries
 * leave, cut short where the branch-and-bound bound rules a subtree out.
 * It takes one to three seconds in a Release build.  Each further binade
 * adds eight values to every input, and a two-term dot product a*b+c*d
 * multiplies the search by the size of a fourth domain.  BF16 has 128 values per
 * binade and the model searches more slowly in it, so even one binade
 * would take many minutes.  A search stopped by the \c -node, \c -fail or
 * \c -time limit reports that the search engine stopped; its last
 * solution is only a lower bound on the worst error.
 *
 * \ingroup Example
 */
class WordFloatError : public IntLexMaximizeScript {
public:
  /// Actual model
  explicit WordFloatError(const WordFloatErrorOptions& opt)
    : IntLexMaximizeScript(opt), computation(opt.get_computation()),
      a(*this,WDT_E4M3FN), b(*this,WDT_E4M3FN), c(*this,WDT_E4M3FN),
      error(ulp_error_digits(*this,WDT_E4M3FN)) {
    WordVar c_magnitude(*this,WDT_E4M3FN);
    fp_abs(*this,c,c_magnitude);
    restrict_to_range(a);
    restrict_to_range(b);
    restrict_to_range(c_magnitude);
    // On positive codes, the unsigned order of the bits is the value order.
    rel(*this,a,WRT_ULQ,b);
    post_e4m3fn_result();
    post_reference();
    ulp_error(*this,WDT_E4M3FN,result,reference,reference,error);
    branch(*this,WordVarArgs{a,b,c},WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
  }
  /// Constructor for cloning \a s
  WordFloatError(WordFloatError& s)
    : IntLexMaximizeScript(s), computation(s.computation) {
    a.update(*this,s.a);
    b.update(*this,s.b);
    c.update(*this,s.c);
    if (computation == Computation::Separate)
      product.update(*this,s.product);
    result.update(*this,s.result);
    reference.update(*this,s.reference);
    error.update(*this,s.error);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordFloatError(*this);
  }
  /// Return the error digits, most significant first, to maximize
  IntVarArgs cost(void) const override {
    return error;
  }
  /// Print the inputs, both results and the error, checked by plain arithmetic
  void print(std::ostream& os) const override {
    const double result_value = decode_e4m3fn(result.val());
    const double reference_value = decode_fp32(reference.val());
    const PlainResult plain =
      evaluate_plain(computation,get_value(a),get_value(b),get_value(c));
    const std::streamsize precision = os.precision(10);
    os << "\tinputs    = a " << get_value(a) << " (" << get_code(a)
       << "), b " << get_value(b) << " (" << get_code(b)
       << "), c " << get_value(c) << " (" << get_code(c) << ")" << std::endl;
    if (computation == Computation::Separate)
      os << "\tproduct   = " << get_value(product)
         << " (E4M3FN " << get_code(product) << ")" << std::endl;
    os << "\tE4M3FN    = " << result_value << " (" << get_code(result) << ")" << std::endl
       << "\tFP32      = " << reference_value
       << " (" << hex_literal(reference.val(),8) << ")" << std::endl
       << "\terror     = " << get_error_ulps() << " E4M3FN ULPs" << std::endl
       << "\tcheck     = "
       << (((plain.rounded == result_value) && (plain.exact == reference_value) &&
            (plain.error == get_error_ulps())) ?
           "plain double arithmetic agrees" :
           "ERROR: plain double arithmetic disagrees")
       << std::endl;
    os.precision(precision);
  }
private:
  /// How the E4M3FN multiply-accumulate rounds
  const Computation computation;
  /// Inputs of the multiply-accumulate, on which the search branches
  WordVar a, b, c;
  /// Rounded E4M3FN product, for separate rounding only
  WordVar product;
  /// E4M3FN result of a*b+c
  WordVar result;
  /// FP32 result of a*b+c, which is exact
  WordVar reference;
  /// Digits of the error, most significant first
  IntVarArray error;
  /// Restrict \a x to values from the smallest to the largest magnitude
  void restrict_to_range(WordVar x) {
    const BoolVar holds(*this,1,1);
    fp_compare(*this,x,FWRT_GQ,
               float_constant(*this,WDT_E4M3FN,min_magnitude),holds);
    fp_compare(*this,x,FWRT_LQ,
               float_constant(*this,WDT_E4M3FN,max_magnitude),holds);
  }
  /// Post the E4M3FN multiply-accumulate as selected by the computation
  void post_e4m3fn_result(void) {
    result = WordVar(*this,WDT_E4M3FN);
    switch (computation) {
    case Computation::Separate:
      product = WordVar(*this,WDT_E4M3FN);
      fp_multiply(*this,a,b,product);
      fp_add(*this,product,c,result);
      return;
    case Computation::Fused:
      fp_fma(*this,a,b,c,result);
      return;
    }
    GECODE_NEVER;
  }
  /// Post the exact FP32 multiply-accumulate of the inputs
  void post_reference(void) {
    /*
     * The product of two E4M3FN values has at most eight significant bits,
     * so the FP32 product is exact, and fp_multiply followed by fp_add
     * rounds only once, like fp_fma, which propagates far more slowly.  On
     * the input range, every product is a multiple of 2^-10 and every c a
     * multiple of 2^-5, and |a*b+c| < 18.  The sum therefore has at most 15
     * significant bits, so the FP32 sum is exact too.
     */
    WordVar a32(*this,WDT_FP32), b32(*this,WDT_FP32), c32(*this,WDT_FP32);
    WordVar product32(*this,WDT_FP32);
    fp_convert(*this,a,a32);
    fp_convert(*this,b,b32);
    fp_convert(*this,c,c32);
    reference = WordVar(*this,WDT_FP32);
    fp_multiply(*this,a32,b32,product32);
    fp_add(*this,product32,c32,reference);
  }
  /// Return the code of the assigned E4M3FN variable \a x as a literal
  static std::string get_code(const WordVar& x) {
    return hex_literal(x.val(),2);
  }
  /// Return the value of the assigned E4M3FN variable \a x
  static double get_value(const WordVar& x) {
    return decode_e4m3fn(x.val());
  }
  /// Return the assigned error in E4M3FN ULPs, which is exact in double
  double get_error_ulps(void) const {
    const int n = error.size();
    const int scale = ulp_error_scale(WDT_E4M3FN);
    double ulps = 0.0;
    for (int i=0; i<n; i++)
      ulps += std::ldexp(error[i].val(),30*(n - 1 - i) - scale);
    return ulps;
  }
};

/** \brief Main-function
 *  \relates WordFloatError
 */
int
main(int argc, char* argv[]) {
  WordFloatErrorOptions opt("WordFloatError");
  opt.solutions(0);
  opt.print_last(true);
  opt.parse(argc,argv);
  Script::run<WordFloatError,BAB,WordFloatErrorOptions>(opt);
  return 0;
}

// STATISTICS: example-any
