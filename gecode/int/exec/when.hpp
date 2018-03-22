/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Int { namespace Exec {

  forceinline
  When::When(Home home, BoolView x,
             std::function<void(Space& home)> t0,
             std::function<void(Space& home)> e0)
    : UnaryPropagator<BoolView,PC_BOOL_VAL>(home,x), t(t0), e(e0) {
    home.notice(*this,AP_DISPOSE);
  }

  forceinline
  When::When(Space& home, When& p)
    : UnaryPropagator<BoolView,PC_BOOL_VAL>(home,p), t(p.t), e(p.e) {
  }

  forceinline ExecStatus
  When::post(Home home, BoolView x,
             std::function<void(Space& home)> t,
             std::function<void(Space& home)> e) {
    if (!t)
      throw InvalidFunction("When::When");
    if (!e)
      throw InvalidFunction("When::When");
    if (x.zero()) {
      e(home);
      return home.failed() ? ES_FAILED : ES_OK;
    } else if (x.one()) {
      t(home);
      return home.failed() ? ES_FAILED : ES_OK;
    } else {
      (void) new (home) When(home,x,t,e);
      return ES_OK;
    }
  }

}}}

// STATISTICS: int-prop

