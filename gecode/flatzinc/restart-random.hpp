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

#ifndef GECODE_FLATZINC_RESTART_RANDOM_HPP
#define GECODE_FLATZINC_RESTART_RANDOM_HPP

namespace Gecode { namespace FlatZinc { namespace Internal {

  /// Sample an offset for an inclusive integer restart range
  template<class Random>
  unsigned long long int
  uniform_int_offset(Random& random, unsigned long long int width) {
    const unsigned long long int chunk_width = 1ULL << 31;

    // Retain the established seeded sequence for ordinary integer ranges.
    if ((width <= chunk_width) || (width > (1ULL << 32)))
      return random(width);

    // Draw uniformly from [0,2^62), rejecting its incomplete final bucket.
    const unsigned long long int source_width = 1ULL << 62;
    const unsigned long long int limit =
      source_width - (source_width % width);
    unsigned long long int sample;
    do {
      sample =
        (static_cast<unsigned long long int>(
           random(static_cast<unsigned int>(chunk_width))) << 31) |
        random(static_cast<unsigned int>(chunk_width));
    } while (sample >= limit);
    return sample % width;
  }

}}}

#endif

// STATISTICS: flatzinc-other
