/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

  /// Traits of IntVarArray
  template <>
  class ArrayTraits<IntVarArray> {
  public:
    typedef IntVarArgs   storage_type;
    typedef IntVar       value_type;
    typedef IntVarArgs   args_type;
  };

  /// Traits of IntVarArgs
  template <>
  class ArrayTraits<IntVarArgs> {
  public:
    typedef IntVarArgs storage_type;
    typedef IntVar     value_type;
    typedef IntVarArgs args_type;
  };

  /// Traits of IntArgs
  template <>
  class ArrayTraits<IntArgs> {
  public:
    typedef IntArgs storage_type;
    typedef int     value_type;
    typedef IntArgs args_type;
  };

  /// Traits of IntSetArgs
  template <>
  class ArrayTraits<IntSetArgs> {
  public:
    typedef IntSetArgs storage_type;
    typedef IntSet     value_type;
    typedef IntSetArgs args_type;
  };

  /// Traits of BoolVarArray
  template <>
  class ArrayTraits<BoolVarArray> {
  public:
    typedef BoolVarArgs   storage_type;
    typedef BoolVar       value_type;
    typedef BoolVarArgs   args_type;
  };

  /// Traits of BoolVarArgs
  template <>
  class ArrayTraits<BoolVarArgs> {
  public:
    typedef BoolVarArgs storage_type;
    typedef BoolVar     value_type;
    typedef BoolVarArgs args_type;
  };

  /*
   * Implementation
   *
   */

  forceinline
  IntVarArray::IntVarArray(void) {}

  forceinline
  IntVarArray::IntVarArray(Space& home, int n)
    : VarArray<IntVar>(home,n) {}

  forceinline
  IntVarArray::IntVarArray(const IntVarArray& a)
    : VarArray<IntVar>(a) {}


  forceinline
  BoolVarArray::BoolVarArray(void) {}

  forceinline
  BoolVarArray::BoolVarArray(Space& home, int n)
    : VarArray<BoolVar>(home,n) {}

  forceinline
  BoolVarArray::BoolVarArray(const BoolVarArray& a)
    : VarArray<BoolVar>(a) {}

}

// STATISTICS: int-other
