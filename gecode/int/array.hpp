/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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
  IntArgs::IntArgs(void) : ArgArray<int>(0) {}

  forceinline
  IntArgs::IntArgs(int n) : ArgArray<int>(n) {}

  forceinline
  IntArgs::IntArgs(const SharedArray<int>& x)
    : ArgArray<int>(x.size()) {
    for (int i=0; i<x.size(); i++)
      a[i] = x[i];
  }

  forceinline
  IntArgs::IntArgs(const std::vector<int>& x)
    : ArgArray<int>(x) {}

  forceinline
  IntArgs::IntArgs(std::initializer_list<int> x)
    : ArgArray<int>(x) {}

  template<class InputIterator>
  forceinline
  IntArgs::IntArgs(InputIterator first, InputIterator last)
    : ArgArray<int>(first,last) {}

  forceinline
  IntArgs::IntArgs(int n, const int* e)
    : ArgArray<int>(n, e) {}

  forceinline
  IntArgs::IntArgs(const ArgArray<int>& a)
    : ArgArray<int>(a) {}

  forceinline IntArgs
  IntArgs::create(int n, int start, int inc) {
    IntArgs r(n);
    for (int i=0; i<n; i++, start+=inc)
      r[i] = start;
    return r;
  }


  forceinline
  IntVarArgs::IntVarArgs(void) {}

  forceinline
  IntVarArgs::IntVarArgs(int n)
    : VarArgArray<IntVar>(n) {}

  forceinline
  IntVarArgs::IntVarArgs(const IntVarArgs& a)
    : VarArgArray<IntVar>(a) {}

  forceinline
  IntVarArgs::IntVarArgs(const VarArray<IntVar>& a)
    : VarArgArray<IntVar>(a) {}

  forceinline
  IntVarArgs::IntVarArgs(const std::vector<IntVar>& a)
    : VarArgArray<IntVar>(a) {}

  forceinline
  IntVarArgs::IntVarArgs(std::initializer_list<IntVar> a)
    : VarArgArray<IntVar>(a) {}

  template<class InputIterator>
  forceinline
  IntVarArgs::IntVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<IntVar>(first,last) {}


  forceinline
  BoolVarArgs::BoolVarArgs(void) {}

  forceinline
  BoolVarArgs::BoolVarArgs(int n)
    : VarArgArray<BoolVar>(n) {}

  forceinline
  BoolVarArgs::BoolVarArgs(const BoolVarArgs& a)
    : VarArgArray<BoolVar>(a) {}

  forceinline
  BoolVarArgs::BoolVarArgs(const VarArray<BoolVar>& a)
    : VarArgArray<BoolVar>(a) {}

  forceinline
  BoolVarArgs::BoolVarArgs(const std::vector<BoolVar>& a)
    : VarArgArray<BoolVar>(a) {}

  forceinline
  BoolVarArgs::BoolVarArgs(std::initializer_list<BoolVar> a)
    : VarArgArray<BoolVar>(a) {}

  template<class InputIterator>
  forceinline
  BoolVarArgs::BoolVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<BoolVar>(first,last) {}


  forceinline
  IntVarArray::IntVarArray(void) {}

  forceinline
  IntVarArray::IntVarArray(Space& home, int n)
    : VarArray<IntVar>(home,n) {}

  forceinline
  IntVarArray::IntVarArray(const IntVarArray& a)
    : VarArray<IntVar>(a) {}

  forceinline
  IntVarArray::IntVarArray(Space& home, const IntVarArgs& a)
    : VarArray<IntVar>(home,a) {}


  forceinline
  BoolVarArray::BoolVarArray(void) {}

  forceinline
  BoolVarArray::BoolVarArray(Space& home, int n)
    : VarArray<BoolVar>(home,n) {}

  forceinline
  BoolVarArray::BoolVarArray(const BoolVarArray& a)
    : VarArray<BoolVar>(a) {}

  forceinline
  BoolVarArray::BoolVarArray(Space& home, const BoolVarArgs& a)
    : VarArray<BoolVar>(home,a) {}

}

// STATISTICS: int-other
