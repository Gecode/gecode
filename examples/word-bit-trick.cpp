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
#include <gecode/minimodel.hh>
#include <gecode/word.hh>

#include <cstdint>

using namespace Gecode;

namespace {

  /// Bit-twiddling routine checked against its reference
  enum class Routine {
    SumMidpoint,     ///< Midpoint (low + high) / 2
    BitwiseMidpoint, ///< Midpoint (low & high) + ((low ^ high) >> 1)
    RoundUp,         ///< Round up to a power of two by smearing x - 1
    GuardedRoundUp   ///< Round up by smearing x - (x != 0)
  };

  /// Test whether \a routine computes a midpoint of two inputs
  bool
  is_midpoint(Routine routine) {
    return (routine == Routine::SumMidpoint) ||
      (routine == Routine::BitwiseMidpoint);
  }

  /// Return the default word width for \a routine
  unsigned int
  get_default_width(Routine routine) {
    return (routine == Routine::BitwiseMidpoint) ? 10 : 32;
  }

  /// Return the mask of the low \a width bits
  std::uint64_t
  get_mask(unsigned int width) {
    return (width == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << width) - 1);
  }

  /*
   * Each routine is written twice: once for machine integers, truncated to
   * the word width, to check a counterexample independently, and once as a
   * Word expression for the model.
   */

  /// Midpoint as in the reference, which cannot overflow when low <= high
  std::uint64_t
  reference_midpoint(std::uint64_t low, std::uint64_t high, unsigned int width) {
    return (low + ((high - low) & get_mask(width)) / 2) & get_mask(width);
  }
  /// Midpoint as in the reference, which cannot overflow when low <= high
  WordExpr
  reference_midpoint(const WordExpr& low, const WordExpr& high) {
    return low + div(high - low,WordExpr(low.width(),2));
  }

  /// Midpoint from the sum, which wraps around for large inputs
  std::uint64_t
  sum_midpoint(std::uint64_t low, std::uint64_t high, unsigned int width) {
    return ((low + high) & get_mask(width)) / 2;
  }
  /// Midpoint from the sum, which wraps around for large inputs
  WordExpr
  sum_midpoint(const WordExpr& low, const WordExpr& high) {
    return div(low + high,WordExpr(low.width(),2));
  }

  /// Midpoint from the shared bits plus half of the differing bits
  std::uint64_t
  bitwise_midpoint(std::uint64_t low, std::uint64_t high, unsigned int width) {
    return ((low & high) + ((low ^ high) >> 1)) & get_mask(width);
  }
  /// Midpoint from the shared bits plus half of the differing bits
  WordExpr
  bitwise_midpoint(const WordExpr& low, const WordExpr& high) {
    return (low & high) + logical_shift_right(low ^ high,1);
  }

  /// Smallest power of two not below \a x, or 0 if none fits in \a width bits
  std::uint64_t
  reference_round_up(std::uint64_t x, unsigned int width) {
    for (unsigned int i=0; i<width; i++)
      if (x <= (std::uint64_t(1) << i))
        return std::uint64_t(1) << i;
    return 0;
  }
  /// Smallest power of two not below \a x, or 0 if none fits in its width
  WordExpr
  reference_round_up(const WordExpr& x) {
    const unsigned int width = x.width();
    WordExpr power_or_zero(width,0);
    for (unsigned int i=width; i-- > 0; ) {
      const WordExpr power(width,std::uint64_t(1) << i);
      power_or_zero = ite(x <= power,power,power_or_zero);
    }
    return power_or_zero;
  }

  /// Copy the highest set bit of \a x into all lower bits
  std::uint64_t
  smear_right(std::uint64_t x, unsigned int width) {
    for (unsigned int shift=1; shift<width; shift*=2)
      x |= x >> shift;
    return x;
  }
  /// Copy the highest set bit of \a x into all lower bits
  WordExpr
  smear_right(const WordExpr& x) {
    WordExpr smeared(x);
    for (unsigned int shift=1; shift<x.width(); shift*=2)
      smeared = smeared | logical_shift_right(smeared,shift);
    return smeared;
  }

  /// Round up by smearing x - 1, which wraps around for x = 0
  std::uint64_t
  smearing_round_up(std::uint64_t x, unsigned int width) {
    return (smear_right((x - 1) & get_mask(width),width) + 1) & get_mask(width);
  }
  /// Round up by smearing x - 1, which wraps around for x = 0
  WordExpr
  smearing_round_up(const WordExpr& x) {
    const WordExpr one(x.width(),1);
    return smear_right(x - one) + one;
  }

  /// Round up by smearing x - (x != 0), so that 0 rounds up to 1
  std::uint64_t
  guarded_round_up(std::uint64_t x, unsigned int width) {
    const std::uint64_t decrement = (x != 0) ? 1 : 0;
    return (smear_right((x - decrement) & get_mask(width),width) + 1) & get_mask(width);
  }
  /// Round up by smearing x - (x != 0), so that 0 rounds up to 1
  WordExpr
  guarded_round_up(const WordExpr& x) {
    const WordExpr zero(x.width(),0), one(x.width(),1);
    return smear_right(x - ite(x != zero,one,zero)) + one;
  }

}

/** \brief Options for the bit-trick checking example */
class WordBitTrickOptions : public Options {
public:
  /// Initialize options
  explicit WordBitTrickOptions(const char* name)
    : Options(name),
      _routine("routine","routine to check against its reference",
               static_cast<int>(Routine::SumMidpoint)),
      _width("width","word width (default: 10 for midpoint-bitwise, otherwise 32)",0) {
    _routine.add(static_cast<int>(Routine::SumMidpoint),"midpoint-sum",
                 "(low + high) / 2 (faulty: overflows)");
    _routine.add(static_cast<int>(Routine::BitwiseMidpoint),"midpoint-bitwise",
                 "(low & high) + ((low ^ high) >> 1) (correct)");
    _routine.add(static_cast<int>(Routine::RoundUp),"round-up",
                 "smear x - 1, then add one (faulty: zero)");
    _routine.add(static_cast<int>(Routine::GuardedRoundUp),"round-up-guarded",
                 "smear x - (x != 0), then add one (correct)");
    add(_routine); add(_width);
  }
  /// Return the routine to check
  Routine get_routine(void) const {
    return static_cast<Routine>(_routine.value());
  }
  /// Return the word width
  unsigned int get_width(void) const {
    return (_width.value() == 0) ? get_default_width(get_routine()) : _width.value();
  }
private:
  /// Routine to check
  Driver::StringOption _routine;
  /// Word width, or 0 for the routine's default
  Driver::UnsignedIntOption _width;
};

/**
 * \brief %Example: Checking bit tricks against their reference
 *
 * Compilers, code reviewers and library authors replace straightforward
 * integer code with bit tricks for speed.  A trick that is wrong on rare
 * inputs is hard to find by testing.  The classic case is the
 * binary-search midpoint (low + high) / 2: the sum overflows when both
 * indices are large, and the bug sat in widely used libraries for years
 * (Joshua Bloch, "Nearly All Binary Searches and Mergesorts are Broken",
 * 2006).  This example checks a routine against its reference on every
 * input of a given word width.  It either finds an input on which the two
 * disagree, or, when search finishes without one, proves that they agree
 * on all inputs.
 *
 * The routines, selected with \c -routine, are
 *  - \c midpoint-sum: (low + high) / 2 against the reference
 *    low + (high - low) / 2, for 0 <= low <= high.  The sum wraps around,
 *    so a counterexample exists.
 *  - \c midpoint-bitwise: (low & high) + ((low ^ high) >> 1) against the
 *    same reference.  Shared bits count in full and differing bits by
 *    half, so no sum can overflow and the routines agree.
 *  - \c round-up: rounding up to a power of two by decrementing, copying
 *    the highest set bit into all lower bits, and incrementing.  The
 *    reference returns the smallest power of two not below x, or 0 when
 *    none fits in the word, which is what the trick returns for values
 *    above the largest power.  The decrement wraps around for x = 0,
 *    which the trick rounds to 0 instead of 1.
 *  - \c round-up-guarded: the same trick decrementing by (x != 0), which
 *    agrees with the reference.
 *
 * The routine and its reference are written as MiniModel Word expressions
 * with modular arithmetic on unsigned words of width \c -width.  The model
 * posts the precondition and that the two results differ, and branches on
 * the inputs.  A solution is a counterexample, which the example prints
 * with both results, recomputed by plain integer code.  A run whose
 * summary reports no solutions, without saying that the search engine
 * stopped, has exhausted the search and so proves that the routines agree.
 *
 * The inputs are unsigned interval domains, and branching splits them
 * upper half first.  Interval reasoning then finds the midpoint overflow
 * with one node per bit, and the round-up routines need a number of nodes
 * that grows with the square of the width, so these routines are checked
 * on 32-bit words by default.  Proving that the two midpoints agree is
 * harder: propagation cannot tell that the carries of the two
 * computations agree before the inputs are almost assigned, so search
 * visits about one node per input pair.  The proof for
 * \c midpoint-bitwise therefore uses 10-bit words by default, which takes
 * well under a second; each further bit multiplies the time by about four,
 * and a 32-bit proof does not finish in reasonable time.
 *
 * \ingroup Example
 */
class WordBitTrick : public Script {
public:
  /// Actual model
  explicit WordBitTrick(const WordBitTrickOptions& opt)
    : Script(opt), routine(opt.get_routine()), width(opt.get_width()),
      inputs(*this,is_midpoint(routine) ? 2 : 1,width,WDT_UNSIGNED) {
    if (is_midpoint(routine))
      rel(*this,WordExpr(inputs[0]) <= WordExpr(inputs[1]));
    routine_result = expr(*this,routine_expr());
    reference_result = expr(*this,reference_expr());
    rel(*this,WordExpr(routine_result) != WordExpr(reference_result));
    branch(*this,inputs,WORD_VAR_NONE(),WORD_VAL_SPLIT_MAX());
  }
  /// Constructor for cloning \a s
  WordBitTrick(WordBitTrick& s)
    : Script(s), routine(s.routine), width(s.width) {
    inputs.update(*this,s.inputs);
    routine_result.update(*this,s.routine_result);
    reference_result.update(*this,s.reference_result);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordBitTrick(*this);
  }
  /// Print a counterexample with both results computed by plain integer code
  void print(std::ostream& os) const override {
    const std::uint64_t plain_routine = evaluate_routine();
    const std::uint64_t plain_reference = evaluate_reference();
    os << std::hex << "\tinput     = ";
    if (is_midpoint(routine))
      os << "low 0x" << inputs[0].val() << ", high 0x" << inputs[1].val();
    else
      os << "x 0x" << inputs[0].val();
    os << std::endl
       << "\troutine   = 0x" << plain_routine
       << " (model 0x" << routine_result.val() << ")" << std::endl
       << "\treference = 0x" << plain_reference
       << " (model 0x" << reference_result.val() << ")" << std::endl
       << std::dec << "\t"
       << ((plain_routine != plain_reference) ?
           "the routine disagrees with its reference" :
           "ERROR: plain integer code finds no disagreement")
       << std::endl;
  }
private:
  /// Routine to check
  const Routine routine;
  /// Word width
  const unsigned int width;
  /// Inputs: low and high for a midpoint, x for rounding up
  WordVarArray inputs;
  /// Result of the routine
  WordVar routine_result;
  /// Result of the reference
  WordVar reference_result;
  /// Return the routine as an expression over the inputs
  WordExpr routine_expr(void) const {
    switch (routine) {
    case Routine::SumMidpoint:
      return sum_midpoint(inputs[0],inputs[1]);
    case Routine::BitwiseMidpoint:
      return bitwise_midpoint(inputs[0],inputs[1]);
    case Routine::RoundUp:
      return smearing_round_up(inputs[0]);
    case Routine::GuardedRoundUp:
      return guarded_round_up(inputs[0]);
    }
    GECODE_NEVER;
    return WordExpr(width,0);
  }
  /// Return the reference as an expression over the inputs
  WordExpr reference_expr(void) const {
    return is_midpoint(routine) ? reference_midpoint(inputs[0],inputs[1]) :
      reference_round_up(inputs[0]);
  }
  /// Evaluate the routine on the assigned inputs with plain integer code
  std::uint64_t evaluate_routine(void) const {
    switch (routine) {
    case Routine::SumMidpoint:
      return sum_midpoint(inputs[0].val(),inputs[1].val(),width);
    case Routine::BitwiseMidpoint:
      return bitwise_midpoint(inputs[0].val(),inputs[1].val(),width);
    case Routine::RoundUp:
      return smearing_round_up(inputs[0].val(),width);
    case Routine::GuardedRoundUp:
      return guarded_round_up(inputs[0].val(),width);
    }
    GECODE_NEVER;
    return 0;
  }
  /// Evaluate the reference on the assigned inputs with plain integer code
  std::uint64_t evaluate_reference(void) const {
    return is_midpoint(routine) ?
      reference_midpoint(inputs[0].val(),inputs[1].val(),width) :
      reference_round_up(inputs[0].val(),width);
  }
};

/** \brief Main-function
 *  \relates WordBitTrick
 */
int
main(int argc, char* argv[]) {
  WordBitTrickOptions opt("WordBitTrick");
  opt.parse(argc,argv);
  if ((opt.get_width() < 2) || (opt.get_width() > 64)) {
    std::cerr << "-width must be between 2 and 64" << std::endl;
    return 1;
  }
  Script::run<WordBitTrick,DFS,WordBitTrickOptions>(opt);
  return 0;
}

// STATISTICS: example-any
