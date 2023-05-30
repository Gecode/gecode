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

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#include <gecode/set/element.hh>
#endif
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

using namespace Gecode::Int;

#ifdef GECODE_HAS_FLOAT_VARS
namespace Gecode { namespace Int {

  /// VarArg type for %Set views
  template<>
  class ViewToVarArg<Gecode::Float::FloatView> {
  public:
    typedef Gecode::FloatVarArgs argtype;
  };

}}
#endif

namespace Gecode { namespace FlatZinc {

  template<class View, class SetLastVal>
  class LastVal : public Propagator {
  protected:
    /// Array of views and their indices
    IdxViewArray<View> x;

    /// Constructor for cloning \a p
    LastVal(Space& home, LastVal& p) : Propagator(home,p) {
      x.update(home, p.x);
    };
    /// Constructor for posting
    LastVal(FlatZincSpace& home, IdxViewArray<View> x0) : Propagator(home), x(x0) {
      x.subscribe(home, *this, PC_GEN_ASSIGNED);
      for (int i = 0; i < x.size(); ++i) {
        SetLastVal::init_last_val(home, x[i].idx, x[i].view);
      }
    }
  public:
    /// Record (and forget) the values of any assigned views in \a x in the right position in \a last_val.
    static void record_assigned(FlatZincSpace& home, IdxViewArray<View>& x) {
      int n = x.size();
      for (int i = n; i--; ) {
        if (x[i].view.assigned()) {
          SetLastVal::set_last_val(home, x[i].idx, x[i].view);
          x[i] = x[--n];
        }
      }
      x.size(n);
    }

    /// Copy propagator during cloning
    virtual Actor* copy(Space& home){ return new (home) LastVal(home,*this); }
    virtual void reschedule(Space &home) {
      x.reschedule(home, *this, PC_GEN_ASSIGNED);
    }
    /// Cost function
    virtual PropCost cost(const Space& /* home */, const ModEventDelta& /* med */) const { return PropCost::record(); }
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& /* med */) {
      record_assigned(static_cast<FlatZincSpace&>(home), x);
      if (x.size() == 0) {
        return home.ES_SUBSUMED(*this);
      } else {
        return ES_FIX;
      }
    }

    /// Post propagator
    static ExecStatus post(FlatZincSpace& home, const typename ViewToVarArg<View>::argtype& x) {
      IdxViewArray<View> ix(home, x);
      record_assigned(home, ix);
      if (ix.size() > 0) {
        (void) new (home) LastVal(home, ix);
      }
      return ES_OK;
    }
  };

  class SetIntLastVal {
  public:
    static void init_last_val(FlatZincSpace& home, size_t idx, IntView& var) {
      home.restart_data().last_val_int[idx] = var.min();
    }
    static void set_last_val(FlatZincSpace& home, size_t idx, IntView& var) {
      home.restart_data().last_val_int[idx] = var.val();
    }
  };
  typedef LastVal<IntView, SetIntLastVal> LastValInt;

  class SetBoolLastVal {
  public:
    static void init_last_val(FlatZincSpace& home, size_t idx, BoolView& var) {
      home.restart_data().last_val_bool[idx] = var.min();
    }
    static void set_last_val(FlatZincSpace& home, size_t idx, BoolView& var) {
      home.restart_data().last_val_bool[idx] = var.val();
    }
  };
  typedef LastVal<BoolView, SetBoolLastVal> LastValBool;

#ifdef GECODE_HAS_FLOAT_VARS
  class SetFloatLastVal {
  public:
    static void init_last_val(FlatZincSpace& home, size_t idx, Float::FloatView& var) {
      home.restart_data().last_val_float[idx] = var.min();
    }
    static void set_last_val(FlatZincSpace& home, size_t idx, Float::FloatView& var) {
      home.restart_data().last_val_float[idx] = var.val();
    }
  };
  typedef LastVal<Float::FloatView, SetFloatLastVal> LastValFloat;
#endif

#ifdef GECODE_HAS_SET_VARS
  class SetIntSetLastVal {
  public:
    static void init_last_val(FlatZincSpace& home, size_t idx, Set::SetView& var) {
      Set::GlbRanges<Set::SetView> lb(var);
      home.restart_data().last_val_set[idx] = IntSet(lb);
    }
    static void set_last_val(FlatZincSpace& home, size_t idx, Set::SetView& var) {
      Set::GlbRanges<Set::SetView> lb(var);
      home.restart_data().last_val_set[idx] = IntSet(lb);
    }
  };
  typedef LastVal<Set::SetView, SetIntSetLastVal> LastValSet;
#endif

}}

#endif //__FLATZINC_LASTVAL_HH__
