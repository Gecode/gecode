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
#ifndef GECODE_WORD_TRACE_DELTA_HPP
#define GECODE_WORD_TRACE_DELTA_HPP

namespace Gecode {

  forceinline
  WordTraceDelta::WordTraceDelta(Word::WordTraceView o, Word::WordView n,
                                 const Delta&)
    : _zero(o.hi() & ~n.hi()), _one(n.lo() & ~o.lo()),
      _domain_type(n.domain_type()),
      _old_minimum(o.bounded() ? o.minimum() : 0),
      _old_maximum(o.bounded() ? o.maximum() : 0),
      _new_minimum(n.bounded() ? n.rank_minimum() : 0),
      _new_maximum(n.bounded() ? n.rank_maximum() : 0) {}
  forceinline WordValue WordTraceDelta::zero(void) const { return _zero; }
  forceinline WordValue WordTraceDelta::one(void) const { return _one; }
  forceinline WordDomainType WordTraceDelta::domain_type(void) const {
    return _domain_type;
  }
  forceinline bool WordTraceDelta::bounded(void) const {
    return _domain_type != WDT_CUBE;
  }
  forceinline WordValue WordTraceDelta::old_minimum(void) const {
    return _old_minimum;
  }
  forceinline WordValue WordTraceDelta::old_maximum(void) const {
    return _old_maximum;
  }
  forceinline WordValue WordTraceDelta::new_minimum(void) const {
    return _new_minimum;
  }
  forceinline WordValue WordTraceDelta::new_maximum(void) const {
    return _new_maximum;
  }
  forceinline bool WordTraceDelta::bits_changed(void) const {
    return (_zero | _one) != 0;
  }
  forceinline bool WordTraceDelta::bounds_changed(void) const {
    return bounded() && ((_old_minimum != _new_minimum) ||
                         (_old_maximum != _new_maximum));
  }

}

// STATISTICS: word-trace

#endif
