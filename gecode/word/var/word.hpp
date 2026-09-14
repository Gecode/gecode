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

namespace Gecode {
  forceinline void WordVar::_init(Space& home, unsigned int width,
                                  WordValue lo, WordValue hi) {
    x = new (home) Word::WordVarImp(home,width,lo,hi);
  }
  forceinline void WordVar::_init(Space& home, unsigned int width,
                                  WordValue lo, WordValue hi,
                                  WordDomainType domain_type,
                                  WordValue minimum, WordValue maximum) {
    if (domain_type == WDT_CUBE) {
      x = new (home) Word::WordVarImp(home,width,lo,hi);
    } else {
      x = new (home) Word::BoundedWordVarImp(
        home,width,lo,hi,domain_type,minimum,maximum);
    }
  }
  forceinline WordVar::WordVar(void) {}
  forceinline WordVar::WordVar(const WordVar& y)
    : VarImpVar<Word::WordVarImp>(y.varimp()) {}
  forceinline unsigned int WordVar::width(void) const { return x->width(); }
  forceinline WordValue WordVar::mask(void) const { return x->mask(); }
  forceinline WordValue WordVar::lo(void) const { return x->lo(); }
  forceinline WordValue WordVar::hi(void) const { return x->hi(); }
  forceinline WordDomainType WordVar::domain_type(void) const {
    return x->domain_type();
  }
  forceinline bool WordVar::bounded(void) const { return x->bounded(); }
  forceinline WordValue WordVar::minimum(void) const { return x->minimum(); }
  forceinline WordValue WordVar::maximum(void) const { return x->maximum(); }
  forceinline WordValue WordVar::unknown(void) const { return x->unknown(); }
  forceinline unsigned int WordVar::unknown_size(void) const { return x->unknown_size(); }
  forceinline bool WordVar::assigned(void) const { return x->assigned(); }
  forceinline bool WordVar::in(WordValue n) const { return x->in(n); }
  forceinline WordValue WordVar::val(void) const {
    if (!x->assigned()) throw Word::ValOfUnassignedVar("WordVar::val");
    return x->val();
  }
}

// STATISTICS: word-var
