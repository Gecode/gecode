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

#ifndef GECODE_WORD_FLOAT_HPP
#define GECODE_WORD_FLOAT_HPP

#include <cassert>
#include <gecode/float-word.hh>

namespace Gecode {
  namespace Word {
    namespace Float {
      using Policy = FloatWordPolicy;

      /**
       * \name Operation status flags
       *
       * The bits of an operation status in 0..31, as documented in
       * \ref TaskModelWordFloat.
       */
      //@{
      /// Invalid operation
      constexpr int invalid = 1;
      /// Division of a nonzero finite operand by zero
      constexpr int divide_by_zero = 2;
      /// Rounded result exceeds the largest finite magnitude
      constexpr int overflow = 4;
      /// Tiny and inexact result
      constexpr int underflow = 8;
      /// Rounded result differs from the exact result
      constexpr int inexact = 16;
      //@}

      /**
       * \brief Encoding facts of a float format
       *
       * Magnitude codes are encodings without their sign bit. A format
       * without NaNs (E2M1) has no canonical NaN: canonical_nan() asserts
       * has_nan(), so no code is mistaken for one.
       */
      class FormatFacts {
      public:
        /// Width of an encoding
        unsigned int width;
        /// Number of stored fraction bits
        unsigned int fraction_bits;
        /// Exponent bias
        int bias;
        /// Largest finite magnitude code
        WordValue maximum;
        /// Magnitude code of infinity, or 0 for a format without infinities
        WordValue infinity;
      private:
        /// Canonical NaN magnitude code, or 0 for a format without NaNs
        WordValue nan;
      public:
        /// Initialize from the facts of one format
        constexpr FormatFacts(unsigned int width0, unsigned int fraction_bits0,
                              int bias0, WordValue maximum0,
                              WordValue infinity0, WordValue nan0)
            : width(width0), fraction_bits(fraction_bits0), bias(bias0),
              maximum(maximum0), infinity(infinity0), nan(nan0) {}
        /// Return the sign bit
        constexpr WordValue sign(void) const {
          return WordValue(1) << (width - 1);
        }
        /// Return the magnitude code of the least normal number
        constexpr WordValue normal(void) const {
          return WordValue(1) << fraction_bits;
        }
        /// Test whether the format has infinities
        constexpr bool has_infinity(void) const { return infinity != 0; }
        /// Test whether the format has NaNs
        constexpr bool has_nan(void) const { return nan != 0; }
        /// Return the canonical NaN magnitude code of a format with NaNs
        constexpr WordValue canonical_nan(void) const {
          assert(has_nan());
          return nan;
        }
        /// Return the quiet bit of a NaN, or 0 for a format without one
        constexpr WordValue quiet(void) const {
          return has_infinity() ? normal() >> 1 : 0;
        }
        /// Test whether the magnitude code \a magnitude is a NaN
        constexpr bool is_nan(WordValue magnitude) const {
          return has_nan() &&
                 magnitude > (has_infinity() ? infinity : maximum);
        }
      };

      /// Return the encoding facts of the float format \a format
      constexpr FormatFacts format_facts(WordDomainType format) {
        switch (format) {
        case WDT_FP32:
          return {32, 23, 127, 0x7f7fffff, 0x7f800000, 0x7fc00000};
        case WDT_BF16:
          return {16, 7, 127, 0x7f7f, 0x7f80, 0x7fc0};
        case WDT_BINARY16:
          return {16, 10, 15, 0x7bff, 0x7c00, 0x7e00};
        case WDT_E4M3FN:
          return {8, 3, 7, 0x7e, 0, 0x7f};
        default:
          assert(format == WDT_E2M1);
          return {4, 1, 1, 0x7, 0, 0};
        }
      }

      /// An end of an interval: its least or its greatest value
      enum class End { Low, High };

      /// A sign of values: of one side of a range, or a negation applied
      enum class Sign { Positive, Negative };

      /// How a rounding direction rounds the magnitude of an inexact result
      enum class MagnitudeRounding { Down, Up, Nearest };

      /// Consistency that the propagation level of a posting selects
      enum class Consistency {
        Default, ///< The default level, also for IPL_VAL and IPL_BND
        Domain   ///< IPL_DOM
      };

      /// Return the consistency that \a ipl selects
      inline Consistency consistency(IntPropLevel ipl) {
        return vbd(ipl) == IPL_DOM ? Consistency::Domain : Consistency::Default;
      }

      /// Return how \a rounding rounds the magnitudes of results of the sign
      /// \a negative
      constexpr MagnitudeRounding magnitude_rounding(Policy::Rounding rounding,
                                                     bool negative) {
        switch (rounding) {
        case Policy::RTZ:
          return MagnitudeRounding::Down;
        case Policy::RUP:
          return negative ? MagnitudeRounding::Down : MagnitudeRounding::Up;
        case Policy::RDN:
          return negative ? MagnitudeRounding::Up : MagnitudeRounding::Down;
        default:
          return MagnitudeRounding::Nearest;
        }
      }

      /// Test whether an overflowing result of the sign \a negative becomes
      /// an infinity under \a p in a format with infinities
      constexpr bool overflows_to_infinity(const Policy& p, bool negative) {
        return p.overflow == Policy::FormatDefault &&
               magnitude_rounding(p.rounding, negative) !=
                   MagnitudeRounding::Down;
      }

      /// Test whether the directed \a rounding rounds the signed value of a
      /// result of the sign \a negative toward positive infinity
      constexpr bool rounds_upward(Policy::Rounding rounding, bool negative) {
        return rounding == Policy::RUP ||
               (rounding == Policy::RTZ && negative);
      }

      /// The nonnegative value significand * 2^exponent
      struct Dyadic {
        WordValue significand;
        int exponent;
      };

      /// Return the value of the finite magnitude code \a magnitude of \a f
      inline Dyadic decode(const FormatFacts& f, WordValue magnitude) {
        const WordValue field = magnitude >> f.fraction_bits;
        return {field ? f.normal() | (magnitude & (f.normal() - 1))
                      : magnitude,
                (field ? static_cast<int>(field) - f.bias : 1 - f.bias) -
                    static_cast<int>(f.fraction_bits)};
      }

      /// Compare the dyadics \a a and \a b, returning -1, 0 or 1, without
      /// shifting by the full distance of their exponents
      inline int compare(Dyadic a, Dyadic b) {
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

      /// Throw Word::OutOfLimits with \a l if a field of \a policy is invalid.
      void check(const FloatWordPolicy& policy, const char* l);
      /// Return the float format of \a x; throw Word::OutOfLimits with \a l
      /// if its domain type is not a float format.
      WordDomainType float_format(WordVar x, const char* l);
      /** \brief Post fp_classify() on the bits of \a x read in \a format
       *
       * \a x may have any domain type of the width of \a format, so
       * decompositions can classify intermediate words. \a format must be a
       * float format of the width of \a x and \a cls a class of
       * FloatWordClass: the public posting that calls it checks both.
       */
      void classify(Home home, WordVar x, WordDomainType format,
                    FloatWordClass cls, BoolVar truth);
    } // namespace Float
  } // namespace Word
} // namespace Gecode

#endif

// STATISTICS: word-other
