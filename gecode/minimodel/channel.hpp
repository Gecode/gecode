/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2018
 *     Guido Tack, 2018
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

  inline BoolVar
  channel(Home home, IntVar x, IntPropLevel ipl) {
    (void) ipl;
    BoolVar b(home,0,1); channel(home,b,x);
    return b;
  }

  inline IntVar
  channel(Home home, BoolVar b, IntPropLevel ipl) {
    (void) ipl;
    IntVar x(home,0,1); channel(home,b,x);
    return x;
  }

#ifdef GECODE_HAS_FLOAT_VARS
  inline IntVar
  channel(Home home, FloatVar f) {
    int min = static_cast<int>(std::max(static_cast<double>(Int::Limits::min),
                                        std::ceil(f.min())));
    int max = static_cast<int>(std::min(static_cast<double>(Int::Limits::max),
                                        std::floor(f.max())));
    IntVar x(home,min,max);
    channel(home,f,x);
    return x;
  }
#endif

#ifdef GECODE_HAS_SET_VARS
  inline SetVar
  channel(Home home, const IntVarArgs& x, IntPropLevel ipl) {
    (void) ipl;
    SetVar s(home,IntSet::empty,Set::Limits::min,Set::Limits::max);
    rel(home,SOT_UNION,x,s);
    nvalues(home,x,IRT_EQ,expr(home,cardinality(s)));
    return s;
  }
#endif

}

// STATISTICS: minimodel-any

