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

  /// Projected cubes in the binary operand/result role order
  struct BinaryCubes {
    WordValue lo[3];
    WordValue hi[3];
  };

  /// Whether publication failed, matched its projection, or narrowed further
  enum CubePublicationResult { CPR_FAILED, CPR_STABLE, CPR_NARROWED };

  /// Completed narrowing and the remaining carry-or-borrow terminal values
  struct TerminalNarrowResult {
    ExecStatus status;
    unsigned int terminal;
  };

  /// Complete binary addition support, including permitted terminal carries
  struct AddSupportResult {
    bool has_support;
    unsigned int terminal;
    BinaryCubes cubes;
  };

  template<class View>
  forceinline bool
  has_exact_cube(View view, WordValue lo, WordValue hi) {
    return (view.lo() == lo) && (view.hi() == hi);
  }

  forceinline bool
  contains_arithmetic_bit(WordCube cube, unsigned int bit, unsigned int value) {
    const WordValue mask=WordValue(1) << bit;
    return value != 0 ? (cube.hi&mask) != 0 : (cube.lo&mask) == 0;
  }

  template<class View>
  forceinline bool
  contains_arithmetic_bit(View view, unsigned int bit, unsigned int value) {
    return contains_arithmetic_bit(WordCube{view.lo(),view.hi()},bit,value);
  }

  /// Publish in role order, then detect stronger alias or bounded-domain closure
  template<class View>
  forceinline CubePublicationResult
  publish_binary_cubes(Home home, View x, View y, View z,
                        const BinaryCubes& cubes) {
    if (!has_exact_cube(x,cubes.lo[0],cubes.hi[0]))
      if (me_failed(x.narrow(home,cubes.lo[0],cubes.hi[0])))
        return CPR_FAILED;
    if (!has_exact_cube(y,cubes.lo[1],cubes.hi[1]))
      if (me_failed(y.narrow(home,cubes.lo[1],cubes.hi[1])))
        return CPR_FAILED;
    if (!has_exact_cube(z,cubes.lo[2],cubes.hi[2]))
      if (me_failed(z.narrow(home,cubes.lo[2],cubes.hi[2])))
        return CPR_FAILED;
    const bool has_additional_narrowing =
      !has_exact_cube(x,cubes.lo[0],cubes.hi[0]) ||
      !has_exact_cube(y,cubes.lo[1],cubes.hi[1]) ||
      !has_exact_cube(z,cubes.lo[2],cubes.hi[2]);
    return has_additional_narrowing ? CPR_NARROWED : CPR_STABLE;
  }

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

  /** Carry transitions for every bit position, stored in four bit masks.
   * Bit i of c01 means carry 0 can become carry 1 at position i.
   * Boolean matrix composition joins adjacent groups of positions, so a
   * parallel prefix or suffix scan needs at most six steps for a word.
   */
  class AddCarryRelation {
  public:
    WordValue c00, c01, c10, c11;
    forceinline AddCarryRelation(WordValue a, WordValue b,
                                 WordValue c, WordValue d)
      : c00(a), c01(b), c10(c), c11(d) {}
    forceinline AddCarryRelation compose(const AddCarryRelation& r) const {
      return AddCarryRelation((c00&r.c00)|(c01&r.c10),
                              (c00&r.c01)|(c01&r.c11),
                              (c10&r.c00)|(c11&r.c10),
                              (c10&r.c01)|(c11&r.c11));
    }
    forceinline AddCarryRelation shift_lower(unsigned int n) const {
      const WordValue identity=(WordValue(1)<<n)-1;
      return AddCarryRelation((c00<<n)|identity,c01<<n,c10<<n,
                              (c11<<n)|identity);
    }
    forceinline AddCarryRelation shift_upper(unsigned int n) const {
      const WordValue identity=~(~WordValue(0)>>n);
      return AddCarryRelation((c00>>n)|identity,c01>>n,c10>>n,
                              (c11>>n)|identity);
    }
  };

  forceinline AddSupportResult
  compute_add_support(unsigned int width, const BinaryCubes& input,
                      unsigned int aliases, unsigned int terminal) {
    AddSupportResult result={false,0,{}};
    const WordValue mask=width_mask(width);
    // Tuple index is 4*x+2*y+z. Each mask contains all positions at which
    // that tuple is allowed, including equality of aliased operands.
    WordValue t[8];
    for (unsigned int i=0; i<8; i++)
      t[i]=(aliases & (1U<<i)) ?
        ((i&4U) ? input.hi[0] : ~input.lo[0]) &
        ((i&2U) ? input.hi[1] : ~input.lo[1]) &
        ((i&1U) ? input.hi[2] : ~input.lo[2]) & mask : 0;
    const AddCarryRelation step(t[0]|t[3]|t[5],t[6],t[1],
                                t[2]|t[4]|t[7]);
    AddCarryRelation prefix=step;
    // Positions outside the word are identity transitions for the suffix.
    AddCarryRelation suffix(step.c00|~mask,step.c01,step.c10,
                             step.c11|~mask);
    for (unsigned int n=1; n<width; n <<= 1) {
      prefix=prefix.shift_lower(n).compose(prefix);
      suffix=suffix.compose(suffix.shift_upper(n));
    }
    result.terminal=static_cast<unsigned int>(
      ((prefix.c00>>(width-1))&1U) |
      (((prefix.c01>>(width-1))&1U)<<1)) & terminal;
    if (result.terminal == 0)
      return result;
    // Reachable carries immediately before each bit, starting with zero.
    const WordValue f0=(prefix.c00<<1)|1U;
    const WordValue f1=prefix.c01<<1;
    const WordValue end0=WordValue(0)-WordValue((terminal&1U)!=0);
    const WordValue end1=WordValue(0)-WordValue((terminal&2U)!=0);
    // Carries immediately after each bit that can reach an allowed end.
    const WordValue b0=(((suffix.c00&end0)|(suffix.c01&end1))>>1) |
      (end0 & (WordValue(1)<<(width-1)));
    const WordValue b1=(((suffix.c10&end0)|(suffix.c11&end1))>>1) |
      (end1 & (WordValue(1)<<(width-1)));
    const WordValue s00=f0&b0, s01=f0&b1, s10=f1&b0, s11=f1&b1;
    t[0]&=s00; t[1]&=s10; t[2]&=s11; t[3]&=s00;
    t[4]&=s11; t[5]&=s00; t[6]&=s01; t[7]&=s11;
    result.cubes.lo[0]=mask & ~(t[0]|t[1]|t[2]|t[3]);
    result.cubes.lo[1]=mask & ~(t[0]|t[1]|t[4]|t[5]);
    result.cubes.lo[2]=mask & ~(t[0]|t[2]|t[4]|t[6]);
    result.cubes.hi[0]=t[4]|t[5]|t[6]|t[7];
    result.cubes.hi[1]=t[2]|t[3]|t[6]|t[7];
    result.cubes.hi[2]=t[1]|t[3]|t[5]|t[7];
    result.has_support=true;
    return result;
  }

  /// Alternate support calculation and checked publication until cubes agree
  template<class View>
  forceinline TerminalNarrowResult
  narrow_add(Home home, View x, View y, View z, unsigned int terminal) {
    const unsigned int width=x.width();
    const unsigned int aliases=((x == y) ? 0xc3U : 0xffU) &
      ((x == z) ? 0xa5U : 0xffU) & ((y == z) ? 0x99U : 0xffU);
    for (;;) {
      const BinaryCubes input={{x.lo(),y.lo(),z.lo()},
                                {x.hi(),y.hi(),z.hi()}};
      const AddSupportResult support=
        compute_add_support(width,input,aliases,terminal);
      if (!support.has_support)
        return {ES_FAILED,support.terminal};
      const CubePublicationResult publication=
        publish_binary_cubes(home,x,y,z,support.cubes);
      if (publication == CPR_FAILED)
        return {ES_FAILED,support.terminal};
      if (publication == CPR_STABLE)
        return {ES_OK,support.terminal};
    }
  }

  forceinline ExecStatus
  Add::narrow(Home home, WordView x, WordView y, WordView z) {
    return narrow_add(home,x,y,z,3U).status;
  }

  forceinline ExecStatus
  Add::post(Home home, WordView x0, WordView x1, WordView x2) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (!has_only_assigned_views)
      (void) new (home) Add(home,x0,x1,x2);
    return ES_OK;
  }

  forceinline ExecStatus
  Add::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow(home,x0,x1,x2));
    const bool has_only_assigned_views=
      x0.assigned() && x1.assigned() && x2.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  forceinline
  NaryAdd::NaryAdd(Home home, ViewArray<WordView>& x0, WordView y0,
                   WordValue c)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,WordView,PC_WORD_BITS>(home,x0,y0),
      constant(c) {}

  forceinline
  NaryAdd::NaryAdd(Space& home, NaryAdd& p)
    : MixNaryOnePropagator<
      WordView,PC_WORD_BITS,WordView,PC_WORD_BITS>(home,p),
      constant(p.constant) {}

  forceinline Actor*
  NaryAdd::copy(Space& home) {
    return new (home) NaryAdd(home,*this);
  }

  forceinline PropCost
  NaryAdd::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::HI,
                            static_cast<unsigned int>(x.size())*y.width());
  }

  /// Fold assigned occurrences, preserving the order and multiplicity of survivors.
  template<class View>
  forceinline bool
  compact_nary_add(ViewArray<View>& x, WordValue& constant, WordValue mask) {
    const int size=x.size();
    int n=0;
    for (int i=0; i<size; i++) {
      if (x[i].assigned())
        constant+=x[i].val();
      else {
        if (n != i) x[n]=x[i];
        n++;
      }
    }
    constant &= mask;
    // Assigned views have no live subscriptions to cancel.
    x.size(n);
    return n != size;
  }

  forceinline bool
  has_nary_sum_support(unsigned long long lo, unsigned long long hi,
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

  /// Detect repeated unassigned inputs and input/result aliases.
  template<class View>
  forceinline bool
  has_nary_add_aliases(const ViewArray<View>& x, View y) {
    const bool has_detected_alias=shared(x) || shared(x,y);
    if (has_detected_alias)
      return true;
    // shared(ViewArray) does not detect a pair when exactly two unassigned
    // entries remain. Assigned entries can survive the bit-filter phase.
    int first=-1;
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned()) {
        if (first >= 0)
          return shared(x[first],x[i]);
        first=i;
      }
    return false;
  }

  /// Carry and input-count bounds for one n-ary addition pass.
  class NaryAddCarryBounds {
  public:
    unsigned long long forward_lo[65], forward_hi[65];
    unsigned long long backward_lo[65], backward_hi[65];
    unsigned long long count_lo[64], count_hi[64];
  };

  /// Projected cubes, with input masks owned by the filter's Region.
  class NaryAddSupport {
  public:
    WordValue* input_lo;
    WordValue* input_hi;
    WordValue result_lo, result_hi;
    forceinline NaryAddSupport(WordValue* lo, WordValue* hi)
      : input_lo(lo), input_hi(hi), result_lo(0), result_hi(0) {}
  };

  /// Initialized carry bounds, usable as support only when has_support is true
  struct NaryCarryResult {
    bool has_support;
    NaryAddCarryBounds carries;
  };

  /// Completed projection borrowing its input masks from the caller's Region
  struct NarySupportResult {
    bool has_support;
    NaryAddSupport support;
  };

  template<class View>
  forceinline NaryCarryResult
  analyze_nary_add_carries(const ViewArray<View>& x, View y,
                           WordValue constant) {
    NaryCarryResult result={false,{}};
    NaryAddCarryBounds& carries=result.carries;
    const unsigned int width=y.width();
    carries.forward_lo[0]=carries.forward_hi[0]=0;

    for (unsigned int bit=0; bit<width; bit++) {
      const WordValue mask=WordValue(1) << bit;
      unsigned long long lo=(constant&mask) != 0 ? 1U : 0U;
      unsigned long long hi=lo;
      for (int i=0; i<x.size(); i++) {
        lo += (x[i].lo()&mask) != 0 ? 1U : 0U;
        hi += (x[i].hi()&mask) != 0 ? 1U : 0U;
      }
      carries.count_lo[bit]=lo;
      carries.count_hi[bit]=hi;

      unsigned long long total_lo=carries.forward_lo[bit]+lo;
      unsigned long long total_hi=carries.forward_hi[bit]+hi;
      if ((y.unknown()&mask) == 0) {
        const unsigned int value=(y.lo()&mask) != 0 ? 1U : 0U;
        if ((total_lo&1U) != value)
          total_lo++;
        if ((total_hi&1U) != value)
          total_hi--;
      }
      if (total_lo > total_hi)
        return result;
      carries.forward_lo[bit+1]=total_lo >> 1;
      carries.forward_hi[bit+1]=total_hi >> 1;
    }

    carries.backward_lo[width]=carries.forward_lo[width];
    carries.backward_hi[width]=carries.forward_hi[width];
    for (unsigned int bit=width; bit-- > 0;) {
      const WordValue mask=WordValue(1) << bit;
      const unsigned long long value_lo=(y.lo()&mask) != 0 ? 1U : 0U;
      const unsigned long long value_hi=(y.hi()&mask) != 0 ? 1U : 0U;
      const unsigned long long total_lo=2*carries.backward_lo[bit+1]+value_lo;
      const unsigned long long total_hi=2*carries.backward_hi[bit+1]+value_hi;
      unsigned long long carry_lo = total_lo > carries.count_hi[bit] ?
        total_lo-carries.count_hi[bit] : 0;
      if (total_hi < carries.count_lo[bit])
        return result;
      const unsigned long long carry_hi=total_hi-carries.count_lo[bit];
      carries.backward_lo[bit]=std::max(carries.forward_lo[bit],carry_lo);
      carries.backward_hi[bit]=std::min(carries.forward_hi[bit],carry_hi);
      if (carries.backward_lo[bit] > carries.backward_hi[bit])
        return result;
    }
    result.has_support=true;
    return result;
  }

  template<class View>
  forceinline NarySupportResult
  project_nary_add_support(const ViewArray<View>& x, View y,
                           const NaryAddCarryBounds& carries,
                           WordValue* input_lo, WordValue* input_hi) {
    NarySupportResult result={false,NaryAddSupport(input_lo,input_hi)};
    NaryAddSupport& support=result.support;
    const unsigned int width=y.width();
    for (int i=0; i<x.size(); i++)
      support.input_lo[i]=support.input_hi[i]=0;
    support.result_lo=support.result_hi=0;

    for (unsigned int bit=0; bit<width; bit++) {
      const WordValue mask=WordValue(1) << bit;
      const unsigned long long total_lo=
        carries.backward_lo[bit]+carries.count_lo[bit];
      const unsigned long long total_hi=
        carries.backward_hi[bit]+carries.count_hi[bit];
      const bool has_zero_support=contains_arithmetic_bit(y,bit,0) &&
        has_nary_sum_support(total_lo,total_hi,0,
                         carries.backward_lo[bit+1],carries.backward_hi[bit+1]);
      const bool has_one_support=contains_arithmetic_bit(y,bit,1) &&
        has_nary_sum_support(total_lo,total_hi,1,
                         carries.backward_lo[bit+1],carries.backward_hi[bit+1]);
      const bool has_result_support=has_zero_support || has_one_support;
      if (!has_result_support)
        return result;
      if (!has_zero_support)
        support.result_lo |= mask;
      if (has_one_support)
        support.result_hi |= mask;

      for (int i=0; i<x.size(); i++) {
        const unsigned long long own_lo=(x[i].lo()&mask) != 0 ? 1U : 0U;
        const unsigned long long own_hi=(x[i].hi()&mask) != 0 ? 1U : 0U;
        const unsigned long long other_lo=carries.count_lo[bit]-own_lo;
        const unsigned long long other_hi=carries.count_hi[bit]-own_hi;
        bool has_support[2] = {false,false};
        for (unsigned int value=0; value<2; value++) {
          if (!contains_arithmetic_bit(x[i],bit,value))
            continue;
          const unsigned long long lo=carries.backward_lo[bit]+other_lo+value;
          const unsigned long long hi=carries.backward_hi[bit]+other_hi+value;
          for (unsigned int result_value=0; result_value<2; result_value++)
            has_support[value] |= contains_arithmetic_bit(y,bit,result_value) &&
              has_nary_sum_support(lo,hi,result_value,carries.backward_lo[bit+1],
                               carries.backward_hi[bit+1]);
        }
        const bool has_input_support=has_support[0] || has_support[1];
        if (!has_input_support)
          return result;
        if (!has_support[0])
          support.input_lo[i] |= mask;
        if (has_support[1])
          support.input_hi[i] |= mask;
      }
    }
    result.has_support=true;
    return result;
  }

  /// Publish the result before inputs, then compare with the projected cubes
  template<class View>
  forceinline CubePublicationResult
  publish_nary_add_support(Home home, ViewArray<View>& x, View y,
                           const NaryAddSupport& support) {
    const bool has_result_changed=
      !has_exact_cube(y,support.result_lo,support.result_hi);
    if (has_result_changed)
      if (me_failed(y.narrow(home,support.result_lo,support.result_hi)))
        return CPR_FAILED;
    for (int i=0; i<x.size(); i++) {
      const bool has_input_changed=
        !has_exact_cube(x[i],support.input_lo[i],support.input_hi[i]);
      if (has_input_changed)
        if (me_failed(x[i].narrow(home,support.input_lo[i],support.input_hi[i])))
          return CPR_FAILED;
    }
    bool has_additional_narrowing=
      !has_exact_cube(y,support.result_lo,support.result_hi);
    for (int i=0; i<x.size(); i++)
      has_additional_narrowing |=
        !has_exact_cube(x[i],support.input_lo[i],support.input_hi[i]);
    return has_additional_narrowing ? CPR_NARROWED : CPR_STABLE;
  }

  /// Repeat carry projection when publication closes an alias or bounded domain
  template<class View>
  forceinline ExecStatus
  NaryAdd::narrow(Home home, ViewArray<View>& x, View y,
                  WordValue constant) {
    if (x.size() == 0) {
      GECODE_ME_CHECK(y.narrow(home,constant,constant));
      return ES_OK;
    }
    Region region;
    WordValue* input_lo=region.alloc<WordValue>(x.size());
    WordValue* input_hi=region.alloc<WordValue>(x.size());
    for (;;) {
      const NaryCarryResult carries=analyze_nary_add_carries(x,y,constant);
      if (!carries.has_support)
        return ES_FAILED;
      const NarySupportResult projection=
        project_nary_add_support(x,y,carries.carries,input_lo,input_hi);
      if (!projection.has_support)
        return ES_FAILED;
      const CubePublicationResult publication=
        publish_nary_add_support(home,x,y,projection.support);
      if (publication == CPR_FAILED)
        return ES_FAILED;
      if (publication == CPR_STABLE)
        break;
    }
    bool has_only_assigned_views=y.assigned();
    for (int i=0; i<x.size(); i++)
      has_only_assigned_views &= x[i].assigned();
    return has_only_assigned_views ? ES_OK : ES_FIX;
  }

  forceinline ExecStatus
  NaryAdd::post(Home home, ViewArray<WordView>& x, WordView y,
                WordValue constant) {
    compact_nary_add(x,constant,y.mask());
    do {
      const ExecStatus es=narrow(home,x,y,constant);
      if (es != ES_FIX)
        return es;
    } while (compact_nary_add(x,constant,y.mask()));
    (void) new (home) NaryAdd(home,x,y,constant);
    return ES_OK;
  }

  forceinline size_t
  NaryAdd::dispose(Space& home) {
    (void) MixNaryOnePropagator<WordView,PC_WORD_BITS,
      WordView,PC_WORD_BITS>::dispose(home);
    return sizeof(*this);
  }

  forceinline ExecStatus
  NaryAdd::propagate(Space& home, const ModEventDelta&) {
    compact_nary_add(x,constant,y.mask());
    do {
      const ExecStatus es=narrow(home,x,y,constant);
      if (es == ES_FAILED)
        return ES_FAILED;
      if (es == ES_OK)
        return home.ES_SUBSUMED(*this);
    } while (compact_nary_add(x,constant,y.mask()));
    return ES_FIX;
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
  narrow_add_carry(Home home, WordView x0, WordView x1, WordView x2,
                   Int::BoolView carry) {
    const unsigned int terminal = carry.one() ? 2U : carry.zero() ? 1U : 3U;
    const TerminalNarrowResult result=narrow_add(home,x0,x1,x2,terminal);
    GECODE_ES_CHECK(result.status);
    if (result.terminal == 1U)
      GECODE_ME_CHECK(carry.zero(home));
    else if (result.terminal == 2U)
      GECODE_ME_CHECK(carry.one(home));
    return ES_OK;
  }

  forceinline ExecStatus
  AddCarry::post(Home home, WordView x0, WordView x1, WordView x2,
                 Int::BoolView carry) {
    GECODE_ES_CHECK(narrow_add_carry(home,x0,x1,x2,carry));
    const bool has_only_assigned_views=x0.assigned() && x1.assigned() &&
      x2.assigned() && carry.assigned();
    if (!has_only_assigned_views) {
      ViewArray<WordView> z(home,3);
      z[0]=x0; z[1]=x1; z[2]=x2;
      (void) new (home) AddCarry(home,z,carry);
    }
    return ES_OK;
  }

  forceinline ExecStatus
  AddCarry::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(narrow_add_carry(home,x[0],x[1],x[2],y));
    const bool has_only_assigned_views=x[0].assigned() && x[1].assigned() &&
      x[2].assigned() && y.assigned();
    if (has_only_assigned_views)
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
