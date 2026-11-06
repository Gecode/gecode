/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2013
 *     Guido Tack, 2013
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

#include <algorithm>
#include <gecode/search.hh>

namespace Gecode::Search {

    unsigned long long int
    CutoffConstant::operator()() const {
        return c;
    }
    unsigned long long int
    CutoffConstant::operator++() {
        return c;
    }

    unsigned long long int
    CutoffLinear::operator()() const {
        return n;
    }
    unsigned long long int
    CutoffLinear::operator++() {
        n += scale;
        return n;
    }

    unsigned long int CutoffLuby::start[CutoffLuby::n_start] = {
        1, 1, 2, 1, 1, 2, 4, 1, 1, 2, 1, 1, 2, 4, 8, 1, 1, 2, 1, 1, 2, 4, 1, 1, 2, 1, 1, 2, 4, 8,  16, 1,
        1, 2, 1, 1, 2, 4, 1, 1, 2, 1, 1, 2, 4, 8, 1, 1, 2, 1, 1, 2, 4, 1, 1, 2, 1, 1, 2, 4, 8, 16, 32};
    unsigned long long int
    CutoffLuby::operator()() const {
        return scale * luby(i);
    }
    unsigned long long int
    CutoffLuby::operator++() {
        return scale * luby(i++);
    }

    unsigned long long int
    CutoffGeometric::operator()() const {
        return static_cast<unsigned long long int>(scale * n);
    }
    unsigned long long int
    CutoffGeometric::operator++() {
        n *= base;
        return static_cast<unsigned long long int>(scale * n);
    }

    unsigned long long int
    CutoffRandom::operator++() {
        cur = min + step * rnd(n);
        return cur;
    }
    unsigned long long int
    CutoffRandom::operator()() const {
        return cur;
    }

    unsigned long long int
    CutoffAppend::operator()() const {
        if (n > 0) {
            return (*c1)();
        }
        return (*c2)();
    }
    unsigned long long int
    CutoffAppend::operator++() {
        if (n > 0) {
            n--;
            return ++(*c1);
        }
        return ++(*c2);
    }

    unsigned long long int
    CutoffMerge::operator()() const {
        return (*c1)();
    }
    unsigned long long int
    CutoffMerge::operator++() {
        (void)++(*c1);
        std::swap(c1, c2);
        return (*c1)();
    }

    unsigned long long int
    CutoffRepeat::operator()() const {
        return cutoff;
    }
    unsigned long long int
    CutoffRepeat::operator++() {
        i++;
        if (i == n) {
            cutoff = (*c)();
            i = 0;
        }
        return cutoff;
    }

    Cutoff*
    Cutoff::constant(unsigned long long int scale) {
        return new CutoffConstant(scale);
    }
    Cutoff*
    Cutoff::linear(unsigned long long int scale) {
        return new CutoffLinear(scale);
    }
    Cutoff*
    Cutoff::luby(unsigned long long int scale) {
        return new CutoffLuby(scale);
    }
    Cutoff*
    Cutoff::geometric(unsigned long long int base, double scale) {
        return new CutoffGeometric(base, scale);
    }
    Cutoff*
    Cutoff::rnd(unsigned int seed, unsigned long long int min, unsigned long long int max, unsigned long long int n) {
        return new CutoffRandom(seed, min, max, n);
    }
    Cutoff*
    Cutoff::append(Cutoff* c1, unsigned long long int n, Cutoff* c2) {
        return new CutoffAppend(c1, n, c2);
    }
    Cutoff*
    Cutoff::merge(Cutoff* c1, Cutoff* c2) {
        return new CutoffMerge(c1, c2);
    }
    Cutoff*
    Cutoff::repeat(Cutoff* c, unsigned long long int n) {
        return new CutoffRepeat(c, n);
    }

} // namespace Gecode::Search

// STATISTICS: search-other
