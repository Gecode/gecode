/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

namespace Gecode { namespace Int {

    inline bool
    Limits::valid(int n) {
      return ((n >= min) && (n <= max));
    }

    inline bool
    Limits::valid(double n) {
      return ((n >= min) && (n <= max));
    }

    inline void
    Limits::check(int n, const char* l) {
      if ((n < min) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::check(double n, const char* l) {
      if ((n < min) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::positive(int n, const char* l) {
      if ((n <= 0) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::positive(double n, const char* l) {
      if ((n <= 0.0) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::nonnegative(int n, const char* l) {
      if ((n < 0) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::nonnegative(double n, const char* l) {
      if ((n < 0.0) || (n > max))
        throw OutOfLimits(l);
    }

    inline void
    Limits::double_check(double n, const char* l) {
      if ((n < double_min) || (n > double_max))
        throw OutOfLimits(l);
    }

}}

// STATISTICS: int-var
