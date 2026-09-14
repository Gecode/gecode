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

namespace Gecode { namespace Word { namespace Arithmetic {

  forceinline
  Neg::Neg(Home home, WordView y0, WordView y1)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1) {}

  forceinline
  Neg::Neg(Space& home, Neg& p)
    : BinaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Neg::copy(Space& home) {
    return new (home) Neg(home,*this);
  }

  forceinline PropCost
  Neg::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  /// One admitted negation transition and its outgoing carry
  struct NegTransitionResult {
    bool is_allowed;
    unsigned int next_carry;
  };

  /// Forward/backward carry reachability for a complete negation pass
  struct NegCarryResult {
    bool has_support;
    unsigned char forward[65];
    unsigned char backward[65];
  };

  /// Projected operand/result cubes for negation
  struct NegSupportResult {
    WordCube x;
    WordCube z;
  };

  forceinline NegTransitionResult
  compute_negation_transition(WordCube x, WordCube z, bool has_alias,
                              unsigned int bit, unsigned int carry,
                              unsigned int xv, unsigned int zv) {
    const bool is_admitted=contains_arithmetic_bit(x,bit,xv) &&
      contains_arithmetic_bit(z,bit,zv) && (!has_alias || (xv == zv));
    if (!is_admitted)
      return {false,0};
    const unsigned int sum=(1U-xv)+carry;
    if (zv != (sum & 1U))
      return {false,0};
    return {true,sum >> 1};
  }

  forceinline NegCarryResult
  analyze_negation_carries(unsigned int width, WordCube x, WordCube z,
                           bool has_alias) {
    NegCarryResult result={false,{},{}};
    result.forward[0]=2U;
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int states=0;
      for (unsigned int carry=0; carry<2; carry++) {
        if ((result.forward[bit] & (1U << carry)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            const NegTransitionResult transition=
              compute_negation_transition(x,z,has_alias,bit,carry,xv,zv);
            if (transition.is_allowed)
              states |= 1U << transition.next_carry;
          }
      }
      result.forward[bit+1]=static_cast<unsigned char>(states);
      if (states == 0)
        return result;
    }
    result.backward[width]=3U;
    for (unsigned int bit=width; bit-- > 0;) {
      unsigned int states=0;
      for (unsigned int carry=0; carry<2; carry++)
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            const NegTransitionResult transition=
              compute_negation_transition(x,z,has_alias,bit,carry,xv,zv);
            const bool has_suffix_support=transition.is_allowed &&
              ((result.backward[bit+1] & (1U << transition.next_carry)) != 0);
            if (has_suffix_support)
              states |= 1U << carry;
          }
      result.backward[bit]=static_cast<unsigned char>(states);
    }
    result.has_support=(result.backward[0] & 2U) != 0;
    return result;
  }

  forceinline NegSupportResult
  project_negation_support(unsigned int width, WordCube x, WordCube z,
                           bool has_alias, const NegCarryResult& carries) {
    WordValue lo[2]={0,0}, hi[2]={0,0};
    for (unsigned int bit=0; bit<width; bit++) {
      unsigned int support[2][2]={{0,0},{0,0}};
      for (unsigned int carry=0; carry<2; carry++) {
        if ((carries.forward[bit] & (1U << carry)) == 0)
          continue;
        for (unsigned int xv=0; xv<2; xv++)
          for (unsigned int zv=0; zv<2; zv++) {
            const NegTransitionResult transition=
              compute_negation_transition(x,z,has_alias,bit,carry,xv,zv);
            const bool has_suffix_support=transition.is_allowed &&
              ((carries.backward[bit+1] & (1U << transition.next_carry)) != 0);
            if (has_suffix_support)
              support[0][xv]=support[1][zv]=1U;
          }
      }
      const WordValue mask=WordValue(1) << bit;
      for (int i=0; i<2; i++) {
        if (support[i][1] != 0)
          hi[i] |= mask;
        if (support[i][0] == 0)
          lo[i] |= mask;
      }
    }
    return {{lo[0],hi[0]},{lo[1],hi[1]}};
  }

  template<class View>
  forceinline CubePublicationResult
  publish_negation_cubes(Home home, View x, View z,
                          const NegSupportResult& support) {
    if (me_failed(x.narrow(home,support.x.lo,support.x.hi)))
      return CPR_FAILED;
    if (me_failed(z.narrow(home,support.z.lo,support.z.hi)))
      return CPR_FAILED;
    const bool has_additional_narrowing=
      !has_exact_cube(x,support.x.lo,support.x.hi) ||
      !has_exact_cube(z,support.z.lo,support.z.hi);
    return has_additional_narrowing ? CPR_NARROWED : CPR_STABLE;
  }

  /// Compute negation support again when publication narrows beyond projection
  template<class View>
  forceinline ExecStatus
  narrow_negation(Home home, View x, View z) {
    const unsigned int width=x.width();
    const bool has_alias=x == z;
    for (;;) {
      const WordCube input={x.lo(),x.hi()}, result={z.lo(),z.hi()};
      const NegCarryResult carries=
        analyze_negation_carries(width,input,result,has_alias);
      if (!carries.has_support)
        return ES_FAILED;
      const NegSupportResult support=
        project_negation_support(width,input,result,has_alias,carries);
      const CubePublicationResult publication=
        publish_negation_cubes(home,x,z,support);
      if (publication == CPR_FAILED)
        return ES_FAILED;
      if (publication == CPR_STABLE)
        return ES_OK;
    }
  }

  forceinline ExecStatus
  Neg::narrow(Home home, WordView x, WordView z) {
    return narrow_negation(home,x,z);
  }

  forceinline ExecStatus
  Neg::post(Home home, WordView x0, WordView x1) {
    GECODE_ES_CHECK(narrow(home,x0,x1));
    const bool has_only_assigned_views=x0.assigned() && x1.assigned();
    if (!has_only_assigned_views)
      (void) new (home) Neg(home,x0,x1);
    return ES_OK;
  }

  forceinline ExecStatus
  Neg::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1));
    const bool has_only_assigned_views=x0.assigned() && x1.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline
  Sub::Sub(Home home, WordView y0, WordView y1, WordView y2)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1,y2) {}

  forceinline
  Sub::Sub(Space& home, Sub& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Sub::copy(Space& home) {
    return new (home) Sub(home,*this);
  }

  forceinline PropCost
  Sub::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  template<class View>
  forceinline TerminalNarrowResult
  narrow_subtract(Home home, View x, View y, View z, unsigned int terminal) {
    // x-y-borrow_in = z-2*borrow_out is the same bit transition as
    // y+z+carry_in = x+2*carry_out. Operand aliases and terminal borrow
    // restrictions therefore use exactly the addition support algorithm.
    return narrow_add(home,y,z,x,terminal);
  }

  forceinline ExecStatus
  Sub::narrow(Home home, WordView x, WordView y, WordView z) {
    return narrow_subtract(home,x,y,z,3U).status;
  }

  forceinline ExecStatus
  Sub::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (!has_only_assigned_views)
      (void) new (home) Sub(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Sub::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline
  SubBorrow::SubBorrow(Home home, ViewArray<WordView>& z,
                       Int::BoolView borrow)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,z,borrow) {}

  forceinline
  SubBorrow::SubBorrow(Space& home, SubBorrow& p)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,p) {}

  forceinline Actor*
  SubBorrow::copy(Space& home) {
    return new (home) SubBorrow(home,*this);
  }

  forceinline PropCost
  SubBorrow::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x[0].width());
  }

  forceinline ExecStatus
  narrow_subtract_borrow(Home home, WordView x0, WordView x1, WordView x2,
                    Int::BoolView borrow) {
    const unsigned int terminal = borrow.one() ? 2U : borrow.zero() ? 1U : 3U;
    const TerminalNarrowResult result=
      narrow_subtract(home,x0,x1,x2,terminal);
    GECODE_ES_CHECK(result.status);
    if (result.terminal == 1U)
      GECODE_ME_CHECK(borrow.zero(home));
    else if (result.terminal == 2U)
      GECODE_ME_CHECK(borrow.one(home));
    return ES_OK;
  }

  forceinline ExecStatus
  SubBorrow::post(Home home, WordView x0, WordView x1, WordView x2,
                  Int::BoolView borrow) {
    GECODE_ES_CHECK(narrow_subtract_borrow(home,x0,x1,x2,borrow));
    const bool has_only_assigned_views=x0.assigned() && x1.assigned() &&
      x2.assigned() && borrow.assigned();
    if (!has_only_assigned_views) {
      ViewArray<WordView> z(home,3);
      z[0]=x0; z[1]=x1; z[2]=x2;
      (void) new (home) SubBorrow(home,z,borrow);
    }
    return ES_OK;
  }

  forceinline ExecStatus
  SubBorrow::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow_subtract_borrow(home,x[0],x[1],x[2],y));
    const bool has_only_assigned_views=x[0].assigned() && x[1].assigned() &&
      x[2].assigned() && y.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
