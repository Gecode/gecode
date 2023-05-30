/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Jip J. Dekker <jip.dekker@monash.edu>
 *
 *  Copyright:
 *     Jip J. Dekker, 2023
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

#ifndef FLATZINC_COMPLETE_HH
#define FLATZINC_COMPLETE_HH

#include <gecode/flatzinc.hh>
#include <gecode/int.hh>

using namespace Gecode::Int;

namespace Gecode { namespace FlatZinc {

  class Complete : public UnaryPropagator<BoolView, PC_BOOL_VAL> {
  protected:
    using UnaryPropagator<BoolView,PC_BOOL_VAL>::x0;

    /// Constructor for cloning \a p
    Complete(Space& home, Complete& p) : UnaryPropagator<BoolView, PC_BOOL_VAL>(home,p) {}
    /// Constructor for posting
    Complete(FlatZincSpace& home, BoolView x0) : UnaryPropagator<BoolView, PC_BOOL_VAL>(home, x0) {}
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) Complete(home,*this); }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      assert(x0.assigned());
      if (x0.val()) {
        static_cast<FlatZincSpace&>(home).restart_data().mark_complete = true;
      }
      return home.ES_SUBSUMED(*this);
    }

    static ExecStatus post(FlatZincSpace& home, BoolView x0) {
      if (x0.assigned()) {
        home.restart_data().mark_complete = x0.val();
      } else {
        (void) new (home) Complete(home, x0);
      }
      return ES_OK;
    }
  };

}}

#endif // FLATZINC_COMPLETE_HH
