/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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
  SetVarArgs::SetVarArgs(void) {}

  forceinline
  SetVarArgs::SetVarArgs(int n)
    : VarArgArray<SetVar>(n) {}

  forceinline
  SetVarArgs::SetVarArgs(const SetVarArgs& a)
    : VarArgArray<SetVar>(a) {}

  forceinline
  SetVarArgs::SetVarArgs(const VarArray<SetVar>& a)
    : VarArgArray<SetVar>(a) {}

  forceinline
  SetVarArgs::SetVarArgs(const std::vector<SetVar>& a)
    : VarArgArray<SetVar>(a) {}

  forceinline
  SetVarArgs::SetVarArgs(std::initializer_list<SetVar> a)
    : VarArgArray<SetVar>(a) {}

  template<class InputIterator>
  forceinline
  SetVarArgs::SetVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<SetVar>(first,last) {}


  forceinline
  SetVarArray::SetVarArray(void) {}

  forceinline
  SetVarArray::SetVarArray(const SetVarArray& a)
    : VarArray<SetVar>(a) {}

  forceinline
  SetVarArray::SetVarArray(Space& home, const SetVarArgs& a)
    : VarArray<SetVar>(home,a) {}

}

// STATISTICS: set-other
