/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
 */

namespace Gecode { namespace Word { namespace Structure {

  namespace VariableShiftSupport {

    forceinline bool
    intersects(WordValue lo0, WordValue hi0,
               WordValue lo1, WordValue hi1) {
      return ((lo0 & ~hi1) == 0) && ((lo1 & ~hi0) == 0);
    }

    /// Cube hulls of complete input/count/result support classes.
    struct Projection {
      bool has_support=false;
      WordCube input{0,0}, count{0,0}, result{0,0};

      void include(WordCube input_support, WordCube count_support,
                   WordCube result_support) {
        if (!has_support) {
          input=input_support; count=count_support; result=result_support;
          has_support=true;
        } else {
          input.lo &= input_support.lo; input.hi |= input_support.hi;
          count.lo &= count_support.lo; count.hi |= count_support.hi;
          result.lo &= result_support.lo; result.hi |= result_support.hi;
        }
      }
    };

    forceinline void
    forward(FixedOp op, WordView x, unsigned int amount,
            WordValue& lo, WordValue& hi) {
      const unsigned int width = x.width();
      const WordValue mask = x.mask();
      switch (op) {
      case FO_SHIFT_LEFT:
        lo = (amount >= width) ? 0 : ((x.lo() << amount) & mask);
        hi = (amount >= width) ? 0 : ((x.hi() << amount) & mask);
        break;
      case FO_LOGICAL_SHIFT_RIGHT:
        lo = (amount >= width) ? 0 : (x.lo() >> amount);
        hi = (amount >= width) ? 0 : (x.hi() >> amount);
        break;
      case FO_ARITHMETIC_SHIFT_RIGHT: {
        const WordValue sign = WordValue(1) << (width-1);
        const WordValue extension = (amount >= width) ? mask :
          (mask & ~(mask >> amount));
        lo = (amount >= width) ? 0 : (x.lo() >> amount);
        hi = (amount >= width) ? 0 : (x.hi() >> amount);
        if ((x.lo() & sign) != 0)
          lo |= extension;
        if ((x.hi() & sign) != 0)
          hi |= extension;
        break;
      }
      default:
        GECODE_NEVER;
      }
    }

    forceinline bool
    backward(FixedOp op, WordView x, WordView result,
             unsigned int amount, WordValue shifted_lo,
             WordValue shifted_hi, WordValue& lo, WordValue& hi) {
      if (!intersects(shifted_lo,shifted_hi,result.lo(),result.hi()))
        return false;

      const unsigned int width = x.width();
      const WordValue mask = x.mask();
      lo = x.lo();
      hi = x.hi();
      switch (op) {
      case FO_SHIFT_LEFT:
        if (amount < width) {
          const WordValue relevant = mask >> amount;
          lo |= result.lo() >> amount;
          hi &= (result.hi() >> amount) | (mask & ~relevant);
        }
        break;
      case FO_LOGICAL_SHIFT_RIGHT:
        if (amount < width) {
          lo |= result.lo() << amount;
          hi &= (result.hi() << amount) | low_mask(amount);
        }
        break;
      case FO_ARITHMETIC_SHIFT_RIGHT: {
        const WordValue sign = WordValue(1) << (width-1);
        const WordValue mapped = (amount >= width) ? 0 :
          (mask >> amount);
        const WordValue extension = mask & ~mapped;
        if (amount < width) {
          lo |= (result.lo() & mapped) << amount;
          hi &= ((result.hi() & mapped) << amount) | low_mask(amount);
        }
        if ((result.lo() & extension) != 0)
          lo |= sign;
        if ((result.hi() & extension) != extension)
          hi &= ~sign;
        break;
      }
      default:
        GECODE_NEVER;
      }
      return (lo & ~hi) == 0;
    }

    forceinline bool
    overshift_hull(WordView amount, unsigned int width,
                   WordValue& lo, WordValue& hi) {
      if (amount.hi() < static_cast<WordValue>(width))
        return false;
      lo = 0;
      hi = 0;
      for (unsigned int bit=0; bit<amount.width(); bit++) {
        const WordValue bit_mask = WordValue(1) << bit;
        const bool zero = ((amount.lo() & bit_mask) == 0) &&
          ((amount.hi() & ~bit_mask) >= static_cast<WordValue>(width));
        const bool one = (amount.hi() & bit_mask) != 0;
        if (!zero)
          lo |= bit_mask;
        if (one)
          hi |= bit_mask;
      }
      return true;
    }

    /// Collect each admitted in-range count without modifying the views.
    inline void
    include_in_range(Projection& projection, WordView x, WordView amount,
                     WordView result, FixedOp op) {
      const unsigned int width=x.width();
      for (unsigned int a=0; a<width; a++) {
        if (!amount.in(static_cast<WordValue>(a)))
          continue;
        WordValue candidate_result_lo, candidate_result_hi;
        VariableShiftSupport::forward(
          op,x,a,candidate_result_lo,candidate_result_hi);
        WordValue candidate_x_lo, candidate_x_hi;
        if (!VariableShiftSupport::backward(
              op,x,result,a,candidate_result_lo,candidate_result_hi,
              candidate_x_lo,candidate_x_hi))
          continue;
        candidate_result_lo |= result.lo();
        candidate_result_hi &= result.hi();
        projection.include({candidate_x_lo,candidate_x_hi},{a,a},
                           {candidate_result_lo,candidate_result_hi});
      }
    }

    /// Keep zero and sign-filled arithmetic overshifts as separate supports.
    inline void
    include_overshift(Projection& projection, WordView x, WordView amount,
                      WordView result, FixedOp op) {
      const unsigned int width=x.width();
      WordValue over_lo, over_hi;
      if (VariableShiftSupport::overshift_hull(
            amount,width,over_lo,over_hi)) {
        const WordValue sign = WordValue(1) << (width-1);
        if ((op == FO_SHIFT_LEFT) ||
            (op == FO_LOGICAL_SHIFT_RIGHT)) {
          if (result.in(0)) {
            projection.include({x.lo(),x.hi()},{over_lo,over_hi},{0,0});
          }
        } else {
          if (((x.lo() & sign) == 0) && result.in(0)) {
            projection.include({x.lo(),x.hi() & ~sign},{over_lo,over_hi},{0,0});
          }
          if (((x.hi() & sign) != 0) && result.in(x.mask())) {
            projection.include({x.lo() | sign,x.hi()},
                               {over_lo,over_hi},{x.mask(),x.mask()});
          }
        }
      }
    }

    /// Membership consumes ranked bounds only for bounded representations.
    forceinline PropCond
    select_membership_condition(WordView view) {
      return view.domain_type() == WDT_CUBE ? PC_WORD_BITS : PC_WORD_DOM;
    }

    forceinline unsigned int
    fixed_amount(WordView amount) {
      return (amount.val() >= static_cast<WordValue>(amount.width())) ?
        amount.width() : static_cast<unsigned int>(amount.val());
    }

  }

  forceinline
  VariableShift::VariableShift(Home home, WordView x, WordView amount,
                               WordView result, FixedOp op0)
    : TernaryPropagator<WordView,PC_GEN_NONE>(home,x,amount,result),
      op(op0) {
    x0.subscribe(home,*this,PC_WORD_BITS);
    x1.subscribe(home,*this,VariableShiftSupport::select_membership_condition(x1));
    x2.subscribe(home,*this,VariableShiftSupport::select_membership_condition(x2));
  }

  forceinline
  VariableShift::VariableShift(Space& home, VariableShift& p)
    : TernaryPropagator<WordView,PC_GEN_NONE>(home,p), op(p.op) {}

  forceinline ExecStatus
  VariableShift::narrow(Home home, WordView x, WordView amount,
                        WordView result, FixedOp op) {
    bool modified;
    do {
      VariableShiftSupport::Projection projection;
      VariableShiftSupport::include_in_range(projection,x,amount,result,op);
      VariableShiftSupport::include_overshift(projection,x,amount,result,op);

      if (!projection.has_support)
        return ES_FAILED;
      modified = false;
      if ((x.lo() != projection.input.lo) || (x.hi() != projection.input.hi)) {
        ModEvent me = x.narrow(home,projection.input.lo,projection.input.hi);
        if (me_failed(me))
          return ES_FAILED;
        modified |= me_modified(me);
      }
      if ((amount.lo() != projection.count.lo) || (amount.hi() != projection.count.hi)) {
        ModEvent me = amount.narrow(home,projection.count.lo,projection.count.hi);
        if (me_failed(me))
          return ES_FAILED;
        modified |= me_modified(me);
      }
      if ((result.lo() != projection.result.lo) || (result.hi() != projection.result.hi)) {
        ModEvent me = result.narrow(home,projection.result.lo,projection.result.hi);
        if (me_failed(me))
          return ES_FAILED;
        modified |= me_modified(me);
      }
    } while (modified);
    return ES_OK;
  }

  forceinline bool
  VariableShift::done(WordView x, WordView amount, WordView result,
                      FixedOp op) {
    if (!x.assigned() || !result.assigned())
      return false;
    const unsigned int width = x.width();
    for (unsigned int a=0; a<width; a++)
      if (amount.in(static_cast<WordValue>(a))) {
        WordValue lo, hi;
        VariableShiftSupport::forward(op,x,a,lo,hi);
        if (lo != result.val())
          return false;
      }
    WordValue lo, hi;
    if (VariableShiftSupport::overshift_hull(amount,width,lo,hi)) {
      WordValue shifted_lo, shifted_hi;
      VariableShiftSupport::forward(op,x,width,shifted_lo,shifted_hi);
      if (shifted_lo != result.val())
        return false;
    }
    return true;
  }

  forceinline ExecStatus
  VariableShift::post(Home home, WordView x, WordView amount,
                      WordView result, FixedOp op) {
    if (amount.assigned())
      return Fixed<WordView,WordView>::post(
        home,x,result,op,VariableShiftSupport::fixed_amount(amount));
    GECODE_ES_CHECK(narrow(home,x,amount,result,op));
    if (amount.assigned())
      return Fixed<WordView,WordView>::post(
        home,x,result,op,VariableShiftSupport::fixed_amount(amount));
    if (!done(x,amount,result,op))
      (void) new (home) VariableShift(home,x,amount,result,op);
    return ES_OK;
  }

  forceinline Actor*
  VariableShift::copy(Space& home) {
    return new (home) VariableShift(home,*this);
  }

  forceinline PropCost
  VariableShift::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  forceinline size_t
  VariableShift::dispose(Space& home) {
    x0.cancel(home,*this,PC_WORD_BITS);
    x1.cancel(home,*this,VariableShiftSupport::select_membership_condition(x1));
    x2.cancel(home,*this,VariableShiftSupport::select_membership_condition(x2));
    (void) TernaryPropagator<WordView,PC_GEN_NONE>::dispose(home);
    return sizeof(*this);
  }

  forceinline void
  VariableShift::reschedule(Space& home) {
    x0.reschedule(home,*this,PC_WORD_BITS);
    x1.reschedule(home,*this,VariableShiftSupport::select_membership_condition(x1));
    x2.reschedule(home,*this,VariableShiftSupport::select_membership_condition(x2));
  }

  forceinline ExecStatus
  VariableShift::propagate(Space& home, const ModEventDelta&) {
    if (x1.assigned())
      GECODE_REWRITE(*this,(Fixed<WordView,WordView>::post(
        home(*this),x0,x2,op,VariableShiftSupport::fixed_amount(x1))));
    GECODE_ES_CHECK(narrow(home,x0,x1,x2,op));
    if (x1.assigned())
      GECODE_REWRITE(*this,(Fixed<WordView,WordView>::post(
        home(*this),x0,x2,op,VariableShiftSupport::fixed_amount(x1))));
    return done(x0,x1,x2,op) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: word-prop
