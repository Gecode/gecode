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
#include <gecode/float-word.hh>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <random>
#include <string>

/*
 * The host cross-check compares assigned FP32 arithmetic and binary16
 * conversion with the host's float arithmetic under RNE. It needs IEEE
 * single precision without excess precision, the five exception flags,
 * round to nearest, no flushing of subnormals, and a build that keeps
 * infinities and NaNs (the Autoconf build uses -ffinite-math-only). A
 * host that lacks one of these is left out, and the test logs the guard.
 * Each case is one host operation on volatile operands, so the compiler
 * neither folds nor contracts it, and fma is std::fma. The host's
 * tininess detection is probed and becomes the policy's.
 */
#if defined(FE_INVALID) && defined(FE_DIVBYZERO) && defined(FE_OVERFLOW) && \
    defined(FE_UNDERFLOW) && defined(FE_INEXACT)
#define GECODE_TEST_HOST_FLAGS
#endif
#if defined(__FLT16_MANT_DIG__) && \
    (defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 13)))
#define GECODE_TEST_HOST_BINARY16
#endif

namespace Test {
  namespace Word {
    namespace Float::Host {
      using namespace Gecode;
      using Policy = FloatWordPolicy;

      /// The operations compared with the host
      enum class Operation {
        Add, Subtract, Multiply, Divide, Sqrt, Fma,
        Narrow, ///< fp_convert from FP32 to binary16
        Widen   ///< fp_convert from binary16 to FP32
      };

      /// Why the host does not serve as a reference
      enum class Guard {
        Reliable, NotIec559, ExcessPrecision, MissingFlags, NotNearest,
        FlushesSubnormals, NoFlags, NoBinary16, FiniteMath
      };

      const char* guard_name(Guard guard) {
        switch (guard) {
        case Guard::Reliable: return "reliable";
        case Guard::NotIec559: return "float is not IEC 559";
        case Guard::ExcessPrecision: return "FLT_EVAL_METHOD is not 0";
        case Guard::MissingFlags: return "missing FE_* exception macros";
        case Guard::NotNearest: return "rounding is not to nearest";
        case Guard::FlushesSubnormals: return "subnormals are flushed";
        case Guard::NoFlags: return "operations do not raise flags";
        case Guard::NoBinary16: return "no binary16 conversion with flags";
        case Guard::FiniteMath: return "compiled assuming finite math";
        }
        GECODE_NEVER;
        return "";
      }

      /// A result: the encoding and the status flags
      struct Outcome {
        WordValue bits;
        int flags;
      };

      float float_from(std::uint32_t bits) {
        float x;
        std::memcpy(&x, &bits, sizeof(x));
        return x;
      }
      std::uint32_t bits_of(float x) {
        std::uint32_t bits;
        std::memcpy(&bits, &x, sizeof(bits));
        return bits;
      }

      void clear_flags(void) {
#ifdef GECODE_TEST_HOST_FLAGS
        std::feclearexcept(FE_ALL_EXCEPT);
#endif
      }
      /// Return the raised host exceptions as library status flags
      int raised_flags(void) {
#ifdef GECODE_TEST_HOST_FLAGS
        const int raised = std::fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW |
                                             FE_UNDERFLOW | FE_INEXACT);
        return ((raised & FE_INVALID) ? 1 : 0) | ((raised & FE_DIVBYZERO) ? 2 : 0) |
               ((raised & FE_OVERFLOW) ? 4 : 0) | ((raised & FE_UNDERFLOW) ? 8 : 0) |
               ((raised & FE_INEXACT) ? 16 : 0);
#else
        return 0;
#endif
      }

      /// Return the host conversion of \a a in direction \a operation
      Outcome host_convert(Operation operation, std::uint32_t a) {
#ifdef GECODE_TEST_HOST_BINARY16
        if (operation == Operation::Narrow) {
          volatile float x = float_from(a);
          volatile _Float16 h = 0;
          clear_flags();
          h = static_cast<_Float16>(x);
          const int flags = raised_flags();
          const _Float16 result = h;
          std::uint16_t bits;
          std::memcpy(&bits, &result, sizeof(bits));
          return {bits, flags};
        }
        const std::uint16_t source = static_cast<std::uint16_t>(a);
        _Float16 value;
        std::memcpy(&value, &source, sizeof(value));
        volatile _Float16 h = value;
        volatile float r = 0.0f;
        clear_flags();
        r = static_cast<float>(h);
        const int flags = raised_flags();
        return {bits_of(r), flags};
#else
        (void) operation;
        (void) a;
        return {0, -1};
#endif
      }

      /// Return the host result of \a operation on \a a, \a b and \a c
      Outcome host(Operation operation, std::uint32_t a, std::uint32_t b = 0,
                   std::uint32_t c = 0) {
        if ((operation == Operation::Narrow) || (operation == Operation::Widen))
          return host_convert(operation, a);
        volatile float x = float_from(a), y = float_from(b), z = float_from(c);
        volatile float r = 0.0f;
        clear_flags();
        switch (operation) {
        case Operation::Add: r = x + y; break;
        case Operation::Subtract: r = x - y; break;
        case Operation::Multiply: r = x * y; break;
        case Operation::Divide: r = x / y; break;
        case Operation::Sqrt: r = std::sqrt(static_cast<float>(x)); break;
        case Operation::Fma:
          r = std::fma(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
          break;
        default: GECODE_NEVER;
        }
        const int flags = raised_flags();
        return {bits_of(r), flags};
      }

      /// Test whether binary16 conversion rounds and raises flags
      bool binary16_converts(void) {
        const Outcome third = host(Operation::Narrow, 0x3eaaaaab);
        const Outcome large = host(Operation::Narrow, 0x47800000);
        const Outcome signaling = host(Operation::Widen, 0x7d00);
        return (third.bits == 0x3555) && (third.flags == 16) && (large.bits == 0x7c00) &&
               (large.flags == 20) && ((signaling.bits & 0x7fc00000) == 0x7fc00000) &&
               (signaling.flags == 1);
      }

      /// Return whether the host serves as a reference for \a operation
      Guard host_guard(Operation operation) {
        if (!std::numeric_limits<float>::is_iec559)
          return Guard::NotIec559;
#if !defined(FLT_EVAL_METHOD) || (FLT_EVAL_METHOD != 0)
        return Guard::ExcessPrecision;
#elif !defined(GECODE_TEST_HOST_FLAGS)
        return Guard::MissingFlags;
#elif defined(__FINITE_MATH_ONLY__) && (__FINITE_MATH_ONLY__ != 0)
        return Guard::FiniteMath;
#else
        if (std::fegetround() != FE_TONEAREST)
          return Guard::NotNearest;
        // Half the least normal is an exact subnormal, kept without flags.
        const Outcome half = host(Operation::Multiply, 0x00800000, 0x3f000000);
        const Outcome least = host(Operation::Add, 0x00000001, 0x00000000);
        if ((half.bits != 0x00400000) || (half.flags != 0) || (least.bits != 1))
          return Guard::FlushesSubnormals;
        if ((host(Operation::Divide, 0x3f800000, 0x40400000).flags != 16) ||
            (host(Operation::Divide, 0x3f800000, 0x00000000).flags != 2) ||
            (host(Operation::Sqrt, 0xbf800000).flags != 1) ||
            (host(Operation::Multiply, 0x7f000000, 0x7f000000).flags != 20) ||
            (host(Operation::Add, 0x3f800000, 0x3f800000).flags != 0))
          return Guard::NoFlags;
        if (((operation == Operation::Narrow) || (operation == Operation::Widen)) &&
            !binary16_converts())
          return Guard::NoBinary16;
        return Guard::Reliable;
#endif
      }

      /** \brief Return the host's tininess detection
       *
       * (1+2^-23)*(2^-126-2^-149) is 2^-126-2^-172: below the least normal
       * before rounding, and the least normal at precision. Only detection
       * before rounding raises underflow.
       */
      Policy::Tininess host_tininess(void) {
        const Outcome probe = host(Operation::Multiply, 0x3f800001, 0x007fffff);
        return ((probe.flags & 8) != 0) ? Policy::BeforeRounding : Policy::AfterPrecision;
      }

      class HostSpace : public Space {
      public:
        HostSpace(void) {}
        HostSpace(HostSpace& s) : Space(s) {}
        virtual Space* copy(void) { return new HostSpace(*this); }
      };

      /// Return the library result of \a operation on assigned operands
      Outcome library(Operation operation, WordValue a, WordValue b, WordValue c,
                      const Policy& policy) {
        HostSpace home;
        const WordDomainType source = (operation == Operation::Widen) ? WDT_BINARY16 : WDT_FP32;
        const WordDomainType target = (operation == Operation::Narrow) ? WDT_BINARY16 : WDT_FP32;
        WordVar x(home, source), y(home, source), z(home, source), r(home, target);
        IntVar status(home, 0, 31);
        dom(home, x, a);
        dom(home, y, b);
        dom(home, z, c);
        switch (operation) {
        case Operation::Add: fp_add(home, x, y, r, status, policy); break;
        case Operation::Subtract: fp_subtract(home, x, y, r, status, policy); break;
        case Operation::Multiply: fp_multiply(home, x, y, r, status, policy); break;
        case Operation::Divide: fp_divide(home, x, y, r, status, policy); break;
        case Operation::Sqrt: fp_sqrt(home, x, r, status, policy); break;
        case Operation::Fma: fp_fma(home, x, y, z, r, status, policy); break;
        case Operation::Narrow:
        case Operation::Widen: fp_convert(home, x, r, status, policy); break;
        }
        if ((home.status() == SS_FAILED) || !r.assigned() || !status.assigned())
          return {0, -1};
        return {r.val(), status.val()};
      }

      /// Return an FP32 encoding, biased to the boundaries of the format
      std::uint32_t pick(std::mt19937_64& random) {
        static const std::uint32_t special[] = {
            0x00000000, 0x00000001, 0x007fffff, 0x00800000, 0x3f800000, 0x7f7fffff,
            0x7f800000, 0x7fc00000, 0x7fffffff, 0x7fa00000, 0x7f800001};
        const std::uint32_t bits = static_cast<std::uint32_t>(random());
        const std::uint32_t sign = bits & 0x80000000U, fraction = bits & 0x007fffffU;
        const unsigned int kind = static_cast<unsigned int>(random() % 8);
        const unsigned int offset = static_cast<unsigned int>(random() % 128);
        switch (kind) {
        case 0: return bits;
        case 1: return sign | special[offset % (sizeof(special) / sizeof(special[0]))];
        case 2: return sign | fraction;
        case 3: return sign | ((1 + offset % 24) << 23) | fraction;
        case 4: return sign | ((230 + offset % 25) << 23) | fraction;
        // Short significands make exact sums, products and roots likely.
        case 5: return sign | ((100 + offset % 55) << 23) | (fraction & 0x007f0000U);
        default: return sign | ((64 + offset) << 23) | fraction;
        }
      }

      /// Return a binary16 conversion operand of \a operation
      std::uint32_t pick_conversion(Operation operation, std::mt19937_64& random) {
        const std::uint32_t bits = static_cast<std::uint32_t>(random());
        if (operation == Operation::Widen)
          return bits & 0xffffU;
        if ((random() % 2) == 0)
          return pick(random);
        // Around the binary16 range, from subnormal to overflow.
        return (bits & 0x807fffffU) | ((102 + static_cast<std::uint32_t>(random() % 42)) << 23);
      }

      /// Return \a bits moved by at most two units in the last place
      std::uint32_t nudge(std::uint32_t bits, std::mt19937_64& random) {
        return bits + static_cast<std::uint32_t>(random() % 5) - 2U;
      }

      /// Operands of one case, related so as to reach exact and cancelling results
      struct Operands {
        std::uint32_t a, b, c;
      };
      Operands operands(Operation operation, std::mt19937_64& random) {
        if ((operation == Operation::Narrow) || (operation == Operation::Widen))
          return {pick_conversion(operation, random), 0, 0};
        Operands o{pick(random), pick(random), pick(random)};
        if ((random() % 4) != 0)
          return o;
        switch (operation) {
        case Operation::Add: o.b = nudge(o.a ^ 0x80000000U, random); break;
        case Operation::Subtract: o.b = nudge(o.a, random); break;
        case Operation::Divide: o.a = bits_of(float_from(o.b) * float_from(o.c)); break;
        case Operation::Sqrt:
          o.a = bits_of(float_from(o.b & 0xfffff000U) * float_from(o.b & 0xfffff000U));
          break;
        case Operation::Fma:
          o.c = nudge(bits_of(float_from(o.a) * float_from(o.b)) ^ 0x80000000U, random);
          break;
        default: break;
        }
        return o;
      }

      /** \brief Test whether IEEE 754 leaves the flags of the case open
       *
       * fma of zero, infinity and a quiet NaN may or may not raise invalid.
       */
      bool open_flags(Operation operation, const Operands& o) {
        const FloatFormat f = float_format(WDT_FP32);
        return (operation == Operation::Fma) &&
               ((f.is_zero(o.a) && f.is_infinity(o.b)) || (f.is_infinity(o.a) && f.is_zero(o.b))) &&
               f.is_nan(o.c) && !f.is_signaling(o.c);
      }

      /// Compare assigned results with the host's float arithmetic
      class Cases : public Base {
        Operation operation;
        std::uint64_t seed;
        static const int cases = 3000;
      public:
        Cases(const char* name, Operation operation0, std::uint64_t seed0)
            : Base(std::string("Word::Float::Host::") + name), operation(operation0), seed(seed0) {}
        virtual bool run(void) {
          const Guard guard = host_guard(operation);
          if (guard != Guard::Reliable) {
            olog << ind(2) << "host not compared: " << guard_name(guard) << '\n';
            return true;
          }
          Policy policy;
          policy.tininess = host_tininess();
          const FloatFormat target =
              float_format((operation == Operation::Narrow) ? WDT_BINARY16 : WDT_FP32);
          std::mt19937_64 random(seed);
          for (int i = 0; i < cases; ++i) {
            const Operands o = operands(operation, random);
            const Outcome expected = host(operation, o.a, o.b, o.c);
            const Outcome found = library(operation, o.a, o.b, o.c, policy);
            const bool nan = target.is_nan(expected.bits) || target.is_nan(found.bits);
            const bool bits_agree = nan ? (target.is_nan(expected.bits) && target.is_nan(found.bits))
                                        : (expected.bits == found.bits);
            if (bits_agree && ((expected.flags == found.flags) || open_flags(operation, o)))
              continue;
            olog << ind(2) << "case " << i << std::hex << ": 0x" << o.a << " 0x" << o.b << " 0x"
                 << o.c << " host 0x" << expected.bits << std::dec << '/' << expected.flags
                 << " library 0x" << std::hex << found.bits << std::dec << '/' << found.flags
                 << '\n';
            return false;
          }
          return true;
        }
      };

      Cases add("Add", Operation::Add, 0x5eed0001);
      Cases subtract("Subtract", Operation::Subtract, 0x5eed0002);
      Cases multiply("Multiply", Operation::Multiply, 0x5eed0003);
      Cases divide("Divide", Operation::Divide, 0x5eed0004);
      Cases sqrt("Sqrt", Operation::Sqrt, 0x5eed0005);
      Cases fma("Fma", Operation::Fma, 0x5eed0006);
      Cases narrow("Convert::Narrow", Operation::Narrow, 0x5eed0007);
      Cases widen("Convert::Widen", Operation::Widen, 0x5eed0008);
    } // namespace Float::Host
  } // namespace Word
} // namespace Test

// STATISTICS: test-word
