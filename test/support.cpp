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

#include "test/test.hh"

namespace Test { namespace Support {

  /// Zero counts, including full-width boundaries and argument evaluation
  class BitCounts : public Base {
  private:
    static bool check(unsigned long long int value) {
      unsigned int leading=0, trailing=0;
      for (unsigned int bit=64; bit>0; bit--) {
        if ((value & (1ULL << (bit-1))) != 0)
          break;
        leading++;
      }
      for (unsigned int bit=0; bit<64; bit++) {
        if ((value & (1ULL << bit)) != 0)
          break;
        trailing++;
      }
      return Gecode::Support::count_leading_zeros_64(value) == leading &&
        Gecode::Support::count_trailing_zeros_64(value) == trailing &&
        Gecode::Support::Detail::count_leading_zeros_64_fallback(value) == leading &&
        Gecode::Support::Detail::count_trailing_zeros_64_fallback(value) == trailing;
    }
  public:
    BitCounts(void) : Base("Support::BitCounts") {}
    virtual bool run(void) {
      if (!check(0) || !check(~0ULL))
        return false;
      for (unsigned int bit=0; bit<64; bit++) {
        const unsigned long long int value=1ULL << bit;
        if (!check(value) || !check(~value) ||
            !check(value-1) || !check(value+1))
          return false;
      }
      // Vary the interior of every possible nonempty set-bit interval.
      for (unsigned int first=0; first<64; first++)
        for (unsigned int last=first; last<64; last++) {
          const unsigned long long int ends=(1ULL<<first)|(1ULL<<last);
          const unsigned long long int field=(~0ULL<<first) &
            (~0ULL>>(63-last));
          if (!check(ends) || !check(field) ||
              !check(ends | (field & 0x5555555555555555ULL)))
            return false;
        }
      unsigned long long int value=0;
      if ((Gecode::Support::count_leading_zeros_64(value++) != 64) || (value != 1))
        return false;
      if ((Gecode::Support::count_trailing_zeros_64(value++) != 0) || (value != 2))
        return false;
      if ((Gecode::Support::count_leading_zeros_64(value++) != 62) || (value != 3))
        return false;
      value=0;
      return (Gecode::Support::count_trailing_zeros_64(value++) == 64) && (value == 1);
    }
  };

  BitCounts bit_counts;

}}

// STATISTICS: test-core
