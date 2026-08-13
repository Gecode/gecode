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
  Add::Add(Home home, WordView y0, WordView y1, WordView y2)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,y0,y1,y2) {}

  forceinline
  Add::Add(Space& home, Add& p)
    : TernaryPropagator<WordView,PC_WORD_BITS>(home,p) {}

  forceinline Actor*
  Add::copy(Space& home) {
    return new (home) Add(home,*this);
  }

  forceinline PropCost
  Add::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x0.width());
  }

  forceinline unsigned int
  add_bit_values(WordValue lo, WordValue hi, WordValue mask) {
    return ((lo&mask) == 0 ? 1U : 0U) |
      ((hi&mask) != 0 ? 2U : 0U);
  }

  forceinline unsigned int
  add_transition(unsigned int carry, unsigned int tuple) {
    // Tuple bits encode x, y, and z respectively; 2 denotes no transition.
    //                       xyz: 000 001 010 011 100 101 110 111
    static const unsigned char next[2][8] = {
      {0,2,2,0,2,0,1,2},
      {2,0,1,2,1,2,2,1}
    };
    return next[carry][tuple];
  }

  class AddSupportTables {
  public:
    unsigned char forward[256][4];
    unsigned char backward[256][4];
    unsigned char support[256][4][4];
    AddSupportTables(void) {
      for (unsigned int allowed=0; allowed<256; allowed++)
        for (unsigned int states=0; states<4; states++) {
          unsigned int next_states=0;
          unsigned int previous_states=0;
          for (unsigned int carry=0; carry<2; carry++)
            for (unsigned int tuple=0; tuple<8; tuple++)
              if ((allowed & (1U << tuple)) != 0) {
                const unsigned int next=add_transition(carry,tuple);
                if ((next < 2) && ((states & (1U << carry)) != 0))
                  next_states |= 1U << next;
                if ((next < 2) && ((states & (1U << next)) != 0))
                  previous_states |= 1U << carry;
              }
          forward[allowed][states]=
            static_cast<unsigned char>(next_states);
          backward[allowed][states]=
            static_cast<unsigned char>(previous_states);
          for (unsigned int next_states_mask=0;
               next_states_mask<4; next_states_mask++) {
            unsigned int values=0;
            for (unsigned int carry=0; carry<2; carry++) {
              if ((states & (1U << carry)) == 0)
                continue;
              for (unsigned int tuple=0; tuple<8; tuple++)
                if ((allowed & (1U << tuple)) != 0) {
                  const unsigned int next=add_transition(carry,tuple);
                  if ((next < 2) &&
                      ((next_states_mask & (1U << next)) != 0)) {
                    values |= 1U << (((tuple >> 2) & 1U) + 0);
                    values |= 1U << (((tuple >> 1) & 1U) + 2);
                    values |= 1U << ((tuple & 1U) + 4);
                  }
                }
            }
            support[allowed][states][next_states_mask]=
              static_cast<unsigned char>(values);
          }
        }
    }
  };

  forceinline const AddSupportTables&
  add_support_tables(void) {
    static const AddSupportTables tables;
    return tables;
  }

  forceinline ExecStatus
  add_narrow(Home home, WordView x, WordView y, WordView z,
             unsigned int terminal, unsigned int& final) {
    const unsigned int width = x.width();
    const bool xy = x == y;
    const bool xz = x == z;
    const bool yz = y == z;
    unsigned char allowed[64];
    unsigned char forward[65] = {0};
    unsigned char backward[65] = {0};
    const AddSupportTables& tables=add_support_tables();
    forward[0] = 1U;
    for (unsigned int bit=0; bit<width; bit++) {
      const WordValue mask=WordValue(1) << bit;
      const unsigned int x_values=add_bit_values(x.lo(),x.hi(),mask);
      const unsigned int y_values=add_bit_values(y.lo(),y.hi(),mask);
      const unsigned int z_values=add_bit_values(z.lo(),z.hi(),mask);
      unsigned int tuples=0;
      for (unsigned int tuple=0; tuple<8; tuple++) {
        const unsigned int xv=(tuple >> 2) & 1U;
        const unsigned int yv=(tuple >> 1) & 1U;
        const unsigned int zv=tuple & 1U;
        if (((x_values & (1U << xv)) != 0) &&
            ((y_values & (1U << yv)) != 0) &&
            ((z_values & (1U << zv)) != 0) &&
            (!xy || (xv == yv)) && (!xz || (xv == zv)) &&
            (!yz || (yv == zv)))
          tuples |= 1U << tuple;
      }
      allowed[bit]=static_cast<unsigned char>(tuples);
      if (tuples == 0)
        return ES_FAILED;
      const unsigned int states=tables.forward[tuples][forward[bit]];
      forward[bit+1] = static_cast<unsigned char>(states);
      if (states == 0)
        return ES_FAILED;
    }

    final = forward[width] & terminal;
    if (final == 0)
      return ES_FAILED;
    backward[width] = static_cast<unsigned char>(terminal);
    WordValue lo[3] = {0,0,0};
    WordValue hi[3] = {0,0,0};
    for (unsigned int bit=width; bit-- > 0;) {
      const unsigned int states=
        tables.backward[allowed[bit]][backward[bit+1]];
      const unsigned int support=
        tables.support[allowed[bit]][forward[bit]][backward[bit+1]];
      backward[bit] = static_cast<unsigned char>(states);
      const WordValue mask = WordValue(1) << bit;
      for (int i=0; i<3; i++) {
        if ((support & (1U << (2*i+1))) != 0)
          hi[i] |= mask;
        if ((support & (1U << (2*i))) == 0)
          lo[i] |= mask;
      }
    }
    if ((backward[0] & 1U) == 0)
      return ES_FAILED;
    if ((x.lo() != lo[0]) || (x.hi() != hi[0]))
      GECODE_ME_CHECK(x.narrow(home,lo[0],hi[0]));
    if ((y.lo() != lo[1]) || (y.hi() != hi[1]))
      GECODE_ME_CHECK(y.narrow(home,lo[1],hi[1]));
    if ((z.lo() != lo[2]) || (z.hi() != hi[2]))
      GECODE_ME_CHECK(z.narrow(home,lo[2],hi[2]));
    return ES_OK;
  }

  forceinline ExecStatus
  Add::narrow(Home home, WordView x, WordView y, WordView z) {
    unsigned int final;
    return add_narrow(home,x,y,z,3U,final);
  }

  forceinline ExecStatus
  Add::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (!(x0.assigned() && x1.assigned() && x2.assigned()))
      (void) new (home) Add(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Add::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    if (x0.assigned() && x1.assigned() && x2.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline
  NaryAdd::NaryAdd(Home home, ViewArray<WordView>& x0, WordView y0,
                   WordValue c, bool a)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,WordView,PC_WORD_BITS>(home,x0,y0),
      constant(c), aliased(a) {}

  forceinline
  NaryAdd::NaryAdd(Space& home, NaryAdd& p)
    : MixNaryOnePropagator<
      WordView,PC_WORD_BITS,WordView,PC_WORD_BITS>(home,p),
      constant(p.constant), aliased(p.aliased) {}

  forceinline Actor*
  NaryAdd::copy(Space& home) {
    return new (home) NaryAdd(home,*this);
  }

  forceinline PropCost
  NaryAdd::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,
                            static_cast<unsigned int>(x.size())*y.width());
  }

  forceinline bool
  nary_add_bit(WordView x, unsigned int bit, unsigned int value) {
    const WordValue mask=WordValue(1) << bit;
    return value != 0 ? (x.hi()&mask) != 0 : (x.lo()&mask) == 0;
  }

  forceinline bool
  nary_add_support(unsigned long long lo, unsigned long long hi,
                   unsigned int value, unsigned long long next_lo,
                   unsigned long long next_hi) {
    const unsigned long long required_lo=2*next_lo+value;
    const unsigned long long required_hi=2*next_hi+value;
    lo=std::max(lo,required_lo);
    hi=std::min(hi,required_hi);
    if (lo > hi)
      return false;
    if ((lo&1U) != value)
      lo++;
    return lo <= hi;
  }

  forceinline ExecStatus
  NaryAdd::narrow(Home home, ViewArray<WordView>& x, WordView y,
                  WordValue constant, bool aliased) {
    const unsigned int width=y.width();
    Region region;
    WordValue* input_lo=region.alloc<WordValue>(x.size());
    WordValue* input_hi=region.alloc<WordValue>(x.size());
    for (;;) {
      unsigned long long forward_lo[65], forward_hi[65];
      unsigned long long backward_lo[65], backward_hi[65];
      unsigned long long count_lo[64], count_hi[64];
      forward_lo[0]=forward_hi[0]=0;

      for (unsigned int bit=0; bit<width; bit++) {
        const WordValue mask=WordValue(1) << bit;
        unsigned long long lo=(constant&mask) != 0 ? 1U : 0U;
        unsigned long long hi=lo;
        for (int i=0; i<x.size(); i++) {
          lo += (x[i].lo()&mask) != 0 ? 1U : 0U;
          hi += (x[i].hi()&mask) != 0 ? 1U : 0U;
        }
        count_lo[bit]=lo;
        count_hi[bit]=hi;

        unsigned long long total_lo=forward_lo[bit]+lo;
        unsigned long long total_hi=forward_hi[bit]+hi;
        if ((y.unknown()&mask) == 0) {
          const unsigned int value=(y.lo()&mask) != 0 ? 1U : 0U;
          if ((total_lo&1U) != value)
            total_lo++;
          if ((total_hi&1U) != value)
            total_hi--;
        }
        if (total_lo > total_hi)
          return ES_FAILED;
        forward_lo[bit+1]=total_lo >> 1;
        forward_hi[bit+1]=total_hi >> 1;
      }

      backward_lo[width]=forward_lo[width];
      backward_hi[width]=forward_hi[width];
      for (unsigned int bit=width; bit-- > 0;) {
        const WordValue mask=WordValue(1) << bit;
        const unsigned long long value_lo=(y.lo()&mask) != 0 ? 1U : 0U;
        const unsigned long long value_hi=(y.hi()&mask) != 0 ? 1U : 0U;
        const unsigned long long total_lo=2*backward_lo[bit+1]+value_lo;
        const unsigned long long total_hi=2*backward_hi[bit+1]+value_hi;
        unsigned long long carry_lo = total_lo > count_hi[bit] ?
          total_lo-count_hi[bit] : 0;
        if (total_hi < count_lo[bit])
          return ES_FAILED;
        const unsigned long long carry_hi=total_hi-count_lo[bit];
        backward_lo[bit]=std::max(forward_lo[bit],carry_lo);
        backward_hi[bit]=std::min(forward_hi[bit],carry_hi);
        if (backward_lo[bit] > backward_hi[bit])
          return ES_FAILED;
      }

      for (int i=0; i<x.size(); i++)
        input_lo[i]=input_hi[i]=0;
      WordValue result_lo=0, result_hi=0;

      for (unsigned int bit=0; bit<width; bit++) {
        const WordValue mask=WordValue(1) << bit;
        const unsigned long long total_lo=
          backward_lo[bit]+count_lo[bit];
        const unsigned long long total_hi=
          backward_hi[bit]+count_hi[bit];
        const bool result_zero=nary_add_bit(y,bit,0) &&
          nary_add_support(total_lo,total_hi,0,
                           backward_lo[bit+1],backward_hi[bit+1]);
        const bool result_one=nary_add_bit(y,bit,1) &&
          nary_add_support(total_lo,total_hi,1,
                           backward_lo[bit+1],backward_hi[bit+1]);
        if (!result_zero && !result_one)
          return ES_FAILED;
        if (!result_zero)
          result_lo |= mask;
        if (result_one)
          result_hi |= mask;

        for (int i=0; i<x.size(); i++) {
          const unsigned long long own_lo=(x[i].lo()&mask) != 0 ? 1U : 0U;
          const unsigned long long own_hi=(x[i].hi()&mask) != 0 ? 1U : 0U;
          const unsigned long long other_lo=count_lo[bit]-own_lo;
          const unsigned long long other_hi=count_hi[bit]-own_hi;
          bool support[2] = {false,false};
          for (unsigned int value=0; value<2; value++) {
            if (!nary_add_bit(x[i],bit,value))
              continue;
            const unsigned long long lo=backward_lo[bit]+other_lo+value;
            const unsigned long long hi=backward_hi[bit]+other_hi+value;
            for (unsigned int result=0; result<2; result++)
              support[value] |= nary_add_bit(y,bit,result) &&
                nary_add_support(lo,hi,result,backward_lo[bit+1],
                                 backward_hi[bit+1]);
          }
          if (!support[0] && !support[1])
            return ES_FAILED;
          if (!support[0])
            input_lo[i] |= mask;
          if (support[1])
            input_hi[i] |= mask;
        }
      }

      bool changed=(y.lo() != result_lo) || (y.hi() != result_hi);
      if (changed)
        GECODE_ME_CHECK(y.narrow(home,result_lo,result_hi));
      for (int i=0; i<x.size(); i++) {
        const bool input_changed=(x[i].lo() != input_lo[i]) ||
          (x[i].hi() != input_hi[i]);
        changed |= input_changed;
        if (input_changed)
          GECODE_ME_CHECK(x[i].narrow(home,input_lo[i],input_hi[i]));
      }
      // Distinct-view projection is idempotent; aliases need intersection
      // closure after the role-specific masks have been published.
      if (!changed || !aliased)
        break;
    }

    bool assigned=y.assigned();
    for (int i=0; i<x.size(); i++)
      assigned &= x[i].assigned();
    return assigned ? ES_OK : ES_FIX;
  }

  forceinline ExecStatus
  NaryAdd::post(Home home, ViewArray<WordView>& x, WordView y,
                WordValue constant) {
    const bool aliased=shared(x) || shared(x,y);
    ExecStatus es=narrow(home,x,y,constant,aliased);
    if (es == ES_FAILED)
      return ES_FAILED;
    if (es == ES_FIX)
      (void) new (home) NaryAdd(home,x,y,constant,aliased);
    return ES_OK;
  }

  forceinline ExecStatus
  NaryAdd::propagate(Space& home, const ModEventDelta&) {
    ExecStatus es=narrow(home,x,y,constant,aliased);
    if (es == ES_FAILED)
      return ES_FAILED;
    return (es == ES_FIX) ? ES_FIX : home.ES_SUBSUMED(*this);
  }

  forceinline
  AddCarry::AddCarry(Home home, ViewArray<WordView>& z,
                     Int::BoolView carry)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,z,carry) {}

  forceinline
  AddCarry::AddCarry(Space& home, AddCarry& p)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,Int::BoolView,Int::PC_BOOL_VAL>(home,p) {}

  forceinline Actor*
  AddCarry::copy(Space& home) {
    return new (home) AddCarry(home,*this);
  }

  forceinline PropCost
  AddCarry::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x[0].width());
  }

  forceinline ExecStatus
  add_carry_narrow(Home home, WordView x0, WordView x1, WordView x2,
                   Int::BoolView carry) {
    const unsigned int terminal = carry.one() ? 2U : carry.zero() ? 1U : 3U;
    unsigned int final;
    GECODE_ES_CHECK(add_narrow(home,x0,x1,x2,terminal,final));
    if (final == 1U)
      GECODE_ME_CHECK(carry.zero(home));
    else if (final == 2U)
      GECODE_ME_CHECK(carry.one(home));
    return ES_OK;
  }

  forceinline ExecStatus
  AddCarry::post(Home home, WordView x0, WordView x1, WordView x2,
                 Int::BoolView carry) {
    GECODE_ES_CHECK(add_carry_narrow(home,x0,x1,x2,carry));
    if (!(x0.assigned() && x1.assigned() && x2.assigned() &&
          carry.assigned())) {
      ViewArray<WordView> z(home,3);
      z[0]=x0; z[1]=x1; z[2]=x2;
      (void) new (home) AddCarry(home,z,carry);
    }
    return ES_OK;
  }

  forceinline ExecStatus
  AddCarry::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(add_carry_narrow(home,x[0],x[1],x[2],y));
    if (x[0].assigned() && x[1].assigned() && x[2].assigned() &&
        y.assigned())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
