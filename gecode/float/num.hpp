/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

    /*
     * The binary64 neighbours of pi, and their exact halves and doubles
     *
     */

    forceinline FloatNum
    pi_half_lower(void) {
        return 0x1.921fb54442d18p+0;
    }
    forceinline FloatNum
    pi_half_upper(void) {
        return 0x1.921fb54442d19p+0;
    }
    forceinline FloatNum
    pi_lower(void) {
        return 0x1.921fb54442d18p+1;
    }
    forceinline FloatNum
    pi_upper(void) {
        return 0x1.921fb54442d19p+1;
    }
    forceinline FloatNum
    pi_twice_lower(void) {
        return 0x1.921fb54442d18p+2;
    }
    forceinline FloatNum
    pi_twice_upper(void) {
        return 0x1.921fb54442d19p+2;
    }
} // namespace Gecode

// STATISTICS: float-var
