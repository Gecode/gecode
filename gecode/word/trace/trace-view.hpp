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
#ifndef GECODE_WORD_TRACE_TRACE_VIEW_HPP
#define GECODE_WORD_TRACE_TRACE_VIEW_HPP

namespace Gecode { namespace Word {

  /// Duplicate of a word view for tracing
  class WordTraceView {
  protected:
    WordValue _lo;
    WordValue _hi;
    WordDomainType _domain_type;
    WordValue _minimum;
    WordValue _maximum;
  public:
    WordTraceView(void)
      : _lo(0), _hi(0), _domain_type(WDT_CUBE),
        _minimum(0), _maximum(0) {}
    WordTraceView(Space&, WordView x)
      : _lo(x.lo()), _hi(x.hi()), _domain_type(x.domain_type()),
        _minimum(x.bounded() ? x.rank_minimum() : 0),
        _maximum(x.bounded() ? x.rank_maximum() : 0) {}
    WordValue lo(void) const { return _lo; }
    WordValue hi(void) const { return _hi; }
    WordDomainType domain_type(void) const { return _domain_type; }
    bool bounded(void) const { return _domain_type != WDT_CUBE; }
    WordValue rank_minimum(void) const { return _minimum; }
    WordValue rank_maximum(void) const { return _maximum; }
    void prune(Space&, WordView x, const Delta&) {
      _lo = x.lo(); _hi = x.hi(); _domain_type = x.domain_type();
      _minimum = x.bounded() ? x.rank_minimum() : 0;
      _maximum = x.bounded() ? x.rank_maximum() : 0;
    }
    void update(Space&, WordTraceView x) {
      _lo = x._lo; _hi = x._hi; _domain_type = x._domain_type;
      _minimum = x._minimum; _maximum = x._maximum;
    }
    static unsigned long long int slack(WordView x) {
      unsigned long long int s = x.unknown_size();
      if (!x.bounded())
        return s;
      WordValue span = x.rank_maximum() - x.rank_minimum();
      while (span != 0) {
        s++;
        span >>= 1;
      }
      return s;
    }
  };

}}

// STATISTICS: word-trace

#endif
