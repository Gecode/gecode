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

#ifndef __FLATZINC_LASTVAL_HH__
#define __FLATZINC_LASTVAL_HH__

#include <gecode/flatzinc.hh>
#include <gecode/int.hh>
#include <gecode/int/idx-view.hh>

using namespace Gecode::Int;

namespace Gecode { namespace FlatZinc {

  class LastValInt : public UnaryPropagator<IntView, PC_INT_VAL> {
  protected:
    using UnaryPropagator<IntView, PC_INT_VAL>::x0;
    size_t i;

    /// Constructor for cloning \a p
    LastValInt(Home home, LastValInt& p) : UnaryPropagator<IntView, PC_INT_VAL>(home,p), i(p.i) {};
    /// Constructor for posting
    LastValInt(FlatZincSpace& home, IntView x0, size_t i) : UnaryPropagator<IntView, PC_INT_VAL>(home, x0), i(i) {
      home.restart_data().last_val_int[i] = x0.min();
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) LastValInt(home,*this); }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      assert(x0.assigned());
      static_cast<FlatZincSpace&>(home).restart_data().last_val_int[i] = x0.val();
      return home.ES_SUBSUMED(*this);
    }

    static ExecStatus post(FlatZincSpace& home, IntView x0, size_t i) {
      if (x0.assigned()) {
        home.restart_data().last_val_int[i] = x0.val();
      } else {
        (void) new (home) LastValInt(home, x0, i);
      }
      return ES_OK;
    }
  };

  /**
   * Record (and forget) the values of any assigned views in \a x in the right position in \a last_val.
   */
  void record_assigned_int(IdxViewArray<IntView>& x, std::vector<int>& last_val) {
    int n = x.size();
    for (int i = n; i--; ) {
      if (x[i].view.assigned()) {
        last_val[x[i].idx] = x[i].view.val();
        x[i] = x[--n];
      }
    }
    x.size(n);
  }

  class NaryLastValInt : public Propagator {
  protected:
    /// Array of views and their indices
    IdxViewArray<IntView> x;

    /// Constructor for cloning \a p
    NaryLastValInt(Space& home, NaryLastValInt& p) : Propagator(home,p) {
      x.update(home, p.x);
    };
    /// Constructor for posting
    NaryLastValInt(FlatZincSpace& home, IdxViewArray<IntView> x0) : Propagator(home), x(x0) {
      x.subscribe(home, *this, PC_INT_VAL);
      for (int i = 0; i < x.size(); ++i) {
        home.restart_data().last_val_int[x[i].idx] = x[i].view.min();
      }
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) NaryLastValInt(home,*this); }
    virtual void reschedule(Space &home) {
      x.reschedule(home,*this,PC_INT_VAL);
    }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      record_assigned_int(x, static_cast<FlatZincSpace&>(home).restart_data().last_val_int);
      if (x.size() == 0) {
        return home.ES_SUBSUMED(*this);
      } else {
        return ES_FIX;
      }
    }

    static ExecStatus post(FlatZincSpace& home, IntVarArgs x) {
      IdxViewArray<IntView> ix(home, x);
      record_assigned_int(ix, static_cast<FlatZincSpace&>(home).restart_data().last_val_int);
      if (ix.size() > 0) {
        (void) new (home) NaryLastValInt(home, ix);
      }
      return ES_OK;
    }
  };


  class LastValBool : public UnaryPropagator<BoolView, PC_BOOL_VAL> {
  protected:
    using UnaryPropagator<BoolView, PC_BOOL_VAL> ::x0;
    size_t i;

    /// Constructor for cloning \a p
    LastValBool(Home home, LastValBool& p) : UnaryPropagator<BoolView, PC_BOOL_VAL> (home,p), i(p.i) {};
    /// Constructor for posting
    LastValBool(FlatZincSpace& home, BoolView x0, size_t i) : UnaryPropagator<BoolView, PC_BOOL_VAL>(home, x0), i(i) {
      home.restart_data().last_val_bool[i] = x0.min();
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) LastValBool(home,*this); }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      assert(x0.assigned());
      static_cast<FlatZincSpace&>(home).restart_data().last_val_bool[i] = x0.val();
      return home.ES_SUBSUMED(*this);
    }

    static ExecStatus post(FlatZincSpace& home, BoolView x0, size_t i) {
      if (x0.assigned()) {
        home.restart_data().last_val_bool[i] = x0.val();
      } else {
        (void) new (home) LastValBool(home, x0, i);
      }
      return ES_OK;
    }
  };

#ifdef GECODE_HAS_SET_VARS
  class LastValSet : public UnaryPropagator<Set::SetView, Set::PC_SET_VAL> {
  protected:
    using UnaryPropagator<Set::SetView, Set::PC_SET_VAL> ::x0;
    size_t i;

    /// Constructor for cloning \a p
    LastValSet(Home home, LastValSet& p) : UnaryPropagator<Set::SetView, Set::PC_SET_VAL> (home,p), i(p.i) {};
    /// Constructor for posting
    LastValSet(FlatZincSpace& home, Set::SetView x0, size_t i) : UnaryPropagator<Set::SetView, Set::PC_SET_VAL>(home, x0), i(i) {
      Set::GlbRanges<Set::SetView> lb(x0);
      home.restart_data().last_val_set[i] = IntSet(lb);
    }

  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) LastValSet(home,*this); }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      assert(x0.assigned());
      Set::GlbRanges<Set::SetView> lb(x0);
      static_cast<FlatZincSpace&>(home).restart_data().last_val_set[i] = IntSet(lb);
      return home.ES_SUBSUMED(*this);
    }

    static ExecStatus post(FlatZincSpace& home, Set::SetView x0, size_t i) {
      if (!x0.assigned()) {
        (void) new (home) LastValSet(home, x0, i);
      }
      return ES_OK;
    }
  };
#endif

#ifdef GECODE_HAS_FLOAT_VARS
  class LastValFloat : public UnaryPropagator<Float::FloatView, Float::PC_FLOAT_VAL> {
  protected:
    using UnaryPropagator<Float::FloatView, Float::PC_FLOAT_VAL> ::x0;
    size_t i;

    /// Constructor for cloning \a p
    LastValFloat(Home home, LastValFloat& p) : UnaryPropagator<Float::FloatView, Float::PC_FLOAT_VAL> (home,p), i(p.i) {};
    /// Constructor for posting
    LastValFloat(FlatZincSpace& home, Float::FloatView x0, size_t i) : UnaryPropagator<Float::FloatView, Float::PC_FLOAT_VAL>(home, x0), i(i) {
      home.restart_data().last_val_float[i] = x0.min();
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) LastValFloat(home,*this); }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      assert(x0.assigned());
      static_cast<FlatZincSpace&>(home).restart_data().last_val_float[i] = x0.val();
      return home.ES_SUBSUMED(*this);
    }

    static ExecStatus post(FlatZincSpace& home, Float::FloatView x0, size_t i) {
      if (x0.assigned()) {
        home.restart_data().last_val_float[i] = x0.val();
      } else {
        (void) new (home) LastValFloat(home, x0, i);
      }
      return ES_OK;
    }
  };
#endif

}}

#endif //__FLATZINC_LASTVAL_HH__
