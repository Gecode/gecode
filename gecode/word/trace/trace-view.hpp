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
  public:
    WordTraceView(void) : _lo(0), _hi(0) {}
    WordTraceView(Space&, WordView x) : _lo(x.lo()), _hi(x.hi()) {}
    WordValue lo(void) const { return _lo; }
    WordValue hi(void) const { return _hi; }
    void prune(Space&, WordView x, const Delta&) {
      _lo = x.lo(); _hi = x.hi();
    }
    void update(Space&, WordTraceView x) {
      _lo = x._lo; _hi = x._hi;
    }
    static unsigned long long int slack(WordView x) {
      return x.unknown_size();
    }
  };

}}

// STATISTICS: word-trace

#endif
