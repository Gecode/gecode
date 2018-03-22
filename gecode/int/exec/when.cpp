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

#include <gecode/int/exec.hh>

namespace Gecode { namespace Int { namespace Exec {

  Actor*
  When::copy(Space& home) {
    return new (home) When(home,*this);
  }

  ExecStatus
  When::propagate(Space& home, const ModEventDelta&) {
    if (x0.zero()) {
      GECODE_VALID_FUNCTION(e());
      e()(home);
    } else {
      assert(x0.one());
      GECODE_VALID_FUNCTION(t());
      t()(home);
    }
    if (home.failed())
      return ES_FAILED;
    return home.ES_SUBSUMED(*this);
  }

  size_t
  When::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    t.~SharedData<std::function<void(Space& home)>>();
    e.~SharedData<std::function<void(Space& home)>>();
    (void) UnaryPropagator<BoolView,PC_BOOL_VAL>::dispose(home);
    return sizeof(*this);
  }

}}}

// STATISTICS: int-prop

