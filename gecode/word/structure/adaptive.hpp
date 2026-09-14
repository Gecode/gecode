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

#ifndef GECODE_WORD_STRUCTURE_ADAPTIVE_HPP
#define GECODE_WORD_STRUCTURE_ADAPTIVE_HPP

namespace Gecode { namespace Word { namespace Structure {

  /// Preserve late opportunities for non-wrapping unsigned shift bounds.
  class AdaptiveShiftLeft : public BinaryPropagator<UnsignedWordView,PC_WORD_DOM> {
  protected:
    using BinaryPropagator<UnsignedWordView,PC_WORD_DOM>::x0;
    using BinaryPropagator<UnsignedWordView,PC_WORD_DOM>::x1;
    unsigned int amount;
    AdaptiveShiftLeft(Home home, UnsignedWordView x, UnsignedWordView result,
                      unsigned int amount0)
      : BinaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,x,result),
        amount(amount0) {}
    AdaptiveShiftLeft(Space& home, AdaptiveShiftLeft& p)
      : BinaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,p),
        amount(p.amount) {}
    static ExecStatus cube(Home home, UnsignedWordView x,
                           UnsignedWordView result, unsigned int amount) {
      return Fixed<UnsignedWordView,UnsignedWordView>::narrow(
        home,x,result,FO_SHIFT_LEFT,amount,0);
    }
  public:
    virtual Actor* copy(Space& home) {
      return new (home) AdaptiveShiftLeft(home,*this);
    }
    virtual size_t dispose(Space& home) {
      (void) BinaryPropagator<UnsignedWordView,PC_WORD_DOM>::dispose(home);
      return sizeof(*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      if (BoundedShiftLeft<UnsignedWordView>::numeric_regime(x0,amount))
        GECODE_REWRITE(*this,(BoundedShiftLeft<UnsignedWordView>::post(
          home(*this),x0,x1,amount)));
      GECODE_ES_CHECK(cube(home,x0,x1,amount));
      if (BoundedShiftLeft<UnsignedWordView>::numeric_regime(x0,amount))
        GECODE_REWRITE(*this,(BoundedShiftLeft<UnsignedWordView>::post(
          home(*this),x0,x1,amount)));
      return (x0.assigned() && x1.assigned()) ?
        home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView x,
                           UnsignedWordView result, unsigned int amount) {
      if (BoundedShiftLeft<UnsignedWordView>::numeric_regime(x,amount))
        return BoundedShiftLeft<UnsignedWordView>::post(home,x,result,amount);
      GECODE_ES_CHECK(cube(home,x,result,amount));
      if (BoundedShiftLeft<UnsignedWordView>::numeric_regime(x,amount))
        return BoundedShiftLeft<UnsignedWordView>::post(home,x,result,amount);
      if (!(x.assigned() && result.assigned()))
        (void) new (home) AdaptiveShiftLeft(home,x,result,amount);
      return ES_OK;
    }
  };

  /// Keep cube propagation until an unsigned variable shift admits bounds.
  class AdaptiveVariableShiftLeft
    : public TernaryPropagator<UnsignedWordView,PC_WORD_DOM> {
  protected:
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x0;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x1;
    using TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::x2;
    AdaptiveVariableShiftLeft(Home home, UnsignedWordView x,
                              UnsignedWordView amount, UnsignedWordView result)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,x,amount,result) {}
    AdaptiveVariableShiftLeft(Space& home, AdaptiveVariableShiftLeft& p)
      : TernaryPropagator<UnsignedWordView,PC_WORD_DOM>(home,p) {}
    static ExecStatus numeric(Home home, UnsignedWordView x,
                              UnsignedWordView amount, UnsignedWordView result) {
      GECODE_ES_CHECK(VariableShift::post(home,WordView(x.varimp()),
        WordView(amount.varimp()),WordView(result.varimp()),FO_SHIFT_LEFT));
      return VariableShiftLeftBounds::post(home,x,amount,result);
    }
    static ExecStatus cube(Home home, UnsignedWordView x,
                           UnsignedWordView amount, UnsignedWordView result) {
      return VariableShift::narrow(home,WordView(x.varimp()),
        WordView(amount.varimp()),WordView(result.varimp()),FO_SHIFT_LEFT);
    }
    static bool done(UnsignedWordView x, UnsignedWordView amount,
                     UnsignedWordView result) {
      return VariableShift::done(WordView(x.varimp()),WordView(amount.varimp()),
                                WordView(result.varimp()),FO_SHIFT_LEFT);
    }
  public:
    virtual Actor* copy(Space& home) {
      return new (home) AdaptiveVariableShiftLeft(home,*this);
    }
    virtual size_t dispose(Space& home) {
      (void) TernaryPropagator<UnsignedWordView,PC_WORD_DOM>::dispose(home);
      return sizeof(*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::LO,x0.width());
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      if (x1.assigned())
        GECODE_REWRITE(*this,(AdaptiveShiftLeft::post(home(*this),x0,x2,
          VariableShiftSupport::fixed_amount(WordView(x1.varimp())))));
      if (VariableShiftLeftBounds::numeric_regime(x0,x1))
        GECODE_REWRITE(*this,(numeric(home(*this),x0,x1,x2)));
      GECODE_ES_CHECK(cube(home,x0,x1,x2));
      if (x1.assigned())
        GECODE_REWRITE(*this,(AdaptiveShiftLeft::post(home(*this),x0,x2,
          VariableShiftSupport::fixed_amount(WordView(x1.varimp())))));
      if (VariableShiftLeftBounds::numeric_regime(x0,x1))
        GECODE_REWRITE(*this,(numeric(home(*this),x0,x1,x2)));
      return done(x0,x1,x2) ? home.ES_SUBSUMED(*this) : ES_FIX;
    }
    static ExecStatus post(Home home, UnsignedWordView x,
                           UnsignedWordView amount, UnsignedWordView result) {
      if (amount.assigned())
        return AdaptiveShiftLeft::post(home,x,result,
          VariableShiftSupport::fixed_amount(WordView(amount.varimp())));
      if (VariableShiftLeftBounds::numeric_regime(x,amount))
        return numeric(home,x,amount,result);
      GECODE_ES_CHECK(cube(home,x,amount,result));
      if (amount.assigned())
        return AdaptiveShiftLeft::post(home,x,result,
          VariableShiftSupport::fixed_amount(WordView(amount.varimp())));
      if (VariableShiftLeftBounds::numeric_regime(x,amount))
        return numeric(home,x,amount,result);
      if (!done(x,amount,result))
        (void) new (home) AdaptiveVariableShiftLeft(home,x,amount,result);
      return ES_OK;
    }
  };

}}}

#endif

// STATISTICS: word-prop
