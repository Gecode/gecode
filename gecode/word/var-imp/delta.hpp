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

namespace Gecode { namespace Word {
  forceinline WordDelta::WordDelta(void)
    : _zero(0), _one(0), _old_minimum(0), _old_maximum(0),
      _new_minimum(0), _new_maximum(0), _domain_type(WDT_CUBE) {}
  forceinline WordDelta::WordDelta(WordValue zero, WordValue one,
                                   WordDomainType domain_type,
                                   WordValue old_minimum,
                                   WordValue old_maximum,
                                   WordValue new_minimum,
                                   WordValue new_maximum)
    : _zero(zero), _one(one),
      _old_minimum(old_minimum), _old_maximum(old_maximum),
      _new_minimum(new_minimum), _new_maximum(new_maximum),
      _domain_type(domain_type) {}
  forceinline WordValue WordDelta::zero(void) const { return _zero; }
  forceinline WordValue WordDelta::one(void) const { return _one; }
  forceinline WordDomainType WordDelta::domain_type(void) const {
    return _domain_type;
  }
  forceinline WordValue WordDelta::old_minimum(void) const {
    return _old_minimum;
  }
  forceinline WordValue WordDelta::old_maximum(void) const {
    return _old_maximum;
  }
  forceinline WordValue WordDelta::new_minimum(void) const {
    return _new_minimum;
  }
  forceinline WordValue WordDelta::new_maximum(void) const {
    return _new_maximum;
  }
}}

// STATISTICS: word-var
