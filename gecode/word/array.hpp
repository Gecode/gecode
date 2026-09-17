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
  forceinline WordValArgs::WordValArgs(void) : ArgArray<WordValue>(0) {}
  forceinline WordValArgs::WordValArgs(int n) : ArgArray<WordValue>(n) {}
  forceinline WordValArgs::WordValArgs(const SharedArray<WordValue>& x)
    : ArgArray<WordValue>(x.size()) {
    for (int i=0; i<x.size(); i++)
      a[i]=x[i];
  }
  forceinline WordValArgs::WordValArgs(const std::vector<WordValue>& x)
    : ArgArray<WordValue>(x) {}
  forceinline WordValArgs::WordValArgs(std::initializer_list<WordValue> x)
    : ArgArray<WordValue>(x) {}
  template<class InputIterator>
  forceinline WordValArgs::WordValArgs(InputIterator first, InputIterator last)
    : ArgArray<WordValue>(first,last) {}
  forceinline WordValArgs::WordValArgs(int n, const WordValue* e)
    : ArgArray<WordValue>(n,e) {}
  forceinline WordValArgs::WordValArgs(const ArgArray<WordValue>& a)
    : ArgArray<WordValue>(a) {}

  forceinline
  WordLinearRow::WordLinearRow(void)
    : _type(EQUAL), _lower(0), _upper(0), _modulus(0) {}
  forceinline
  WordLinearRow::WordLinearRow(Type type, const IntArgs& a, int lower,
                               int upper, WordValue modulus)
    : _type(type), _coefficients(a), _lower(lower), _upper(upper),
      _modulus(modulus) {}
  forceinline WordLinearRow
  WordLinearRow::equal(const IntArgs& a, int b) {
    return WordLinearRow(EQUAL,a,b,b,0);
  }
  forceinline WordLinearRow
  WordLinearRow::range(const IntArgs& a, int l, int u) {
    return WordLinearRow(RANGE,a,l,u,0);
  }
  forceinline WordLinearRow
  WordLinearRow::congruence(const IntArgs& a, int r, WordValue modulus) {
    return WordLinearRow(CONGRUENCE,a,r,r,modulus);
  }
  forceinline WordLinearRow::Type WordLinearRow::type(void) const {
    return _type;
  }
  forceinline const IntArgs& WordLinearRow::coefficients(void) const {
    return _coefficients;
  }
  forceinline int WordLinearRow::lower(void) const { return _lower; }
  forceinline int WordLinearRow::upper(void) const { return _upper; }
  forceinline WordValue WordLinearRow::modulus(void) const { return _modulus; }

  forceinline WordVarArgs::WordVarArgs(void) {}
  forceinline WordVarArgs::WordVarArgs(int n) : VarArgArray<WordVar>(n) {}
  forceinline WordVarArgs::WordVarArgs(const WordVarArgs& a) : VarArgArray<WordVar>(a) {}
  forceinline WordVarArgs::WordVarArgs(const VarArray<WordVar>& a) : VarArgArray<WordVar>(a) {}
  forceinline WordVarArgs::WordVarArgs(const std::vector<WordVar>& a) : VarArgArray<WordVar>(a) {}
  forceinline WordVarArgs::WordVarArgs(std::initializer_list<WordVar> a) : VarArgArray<WordVar>(a) {}
  template<class InputIterator>
  forceinline WordVarArgs::WordVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<WordVar>(first,last) {}
  forceinline WordVarArray::WordVarArray(void) {}
  forceinline WordVarArray::WordVarArray(Space& home, int n) : VarArray<WordVar>(home,n) {}
  forceinline WordVarArray::WordVarArray(const WordVarArray& a) : VarArray<WordVar>(a) {}
  forceinline WordVarArray::WordVarArray(Space& home, const WordVarArgs& a) : VarArray<WordVar>(home,a) {}
}

// STATISTICS: word-other
