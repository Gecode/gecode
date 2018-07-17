/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Vincent Barichard, 2012
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

namespace Gecode {

  /*
   * Implementation
   *
   */

  forceinline
  FloatValArgs::FloatValArgs(void) : ArgArray<FloatVal>(0) {}

  forceinline
  FloatValArgs::FloatValArgs(int n) : ArgArray<FloatVal>(n) {}

  forceinline
  FloatValArgs::FloatValArgs(const SharedArray<FloatVal>& x)
    : ArgArray<FloatVal>(x.size()) {
    for (int i=x.size(); i--;)
      a[i] = x[i];
  }
  forceinline
  FloatValArgs::FloatValArgs(const std::vector<FloatVal>& x)
    : ArgArray<FloatVal>(x) {}
  template<class InputIterator>
  forceinline
  FloatValArgs::FloatValArgs(InputIterator first, InputIterator last)
    : ArgArray<FloatVal>(first,last) {}

  forceinline
  FloatValArgs::FloatValArgs(int n, const FloatVal* e)
    : ArgArray<FloatVal>(n, e) {}

  forceinline
  FloatValArgs::FloatValArgs(const ArgArray<FloatVal>& a)
    : ArgArray<FloatVal>(a) {}


  forceinline
  FloatVarArgs::FloatVarArgs(void) {}

  forceinline
  FloatVarArgs::FloatVarArgs(int n)
    : VarArgArray<FloatVar>(n) {}

  forceinline
  FloatVarArgs::FloatVarArgs(const FloatVarArgs& a)
    : VarArgArray<FloatVar>(a) {}

  forceinline
  FloatVarArgs::FloatVarArgs(const VarArray<FloatVar>& a)
    : VarArgArray<FloatVar>(a) {}

  forceinline
  FloatVarArgs::FloatVarArgs(const std::vector<FloatVar>& a)
    : VarArgArray<FloatVar>(a) {}

  forceinline
  FloatVarArgs::FloatVarArgs(std::initializer_list<FloatVar> a)
    : VarArgArray<FloatVar>(a) {}

  template<class InputIterator>
  forceinline
  FloatVarArgs::FloatVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<FloatVar>(first,last) {}


  forceinline
  FloatVarArray::FloatVarArray(void) {}

  forceinline
  FloatVarArray::FloatVarArray(Space& home, int n)
    : VarArray<FloatVar>(home,n) {}

  forceinline
  FloatVarArray::FloatVarArray(const FloatVarArray& a)
    : VarArray<FloatVar>(a) {}

  forceinline
  FloatVarArray::FloatVarArray(Space& home, const FloatVarArgs& a)
    : VarArray<FloatVar>(home,a) {}

}

// STATISTICS: float-other
