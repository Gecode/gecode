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

namespace Gecode { namespace Word { namespace Logic {

  namespace nary_support {
    template<class VY>
    forceinline bool has_changed(VY y, WordValue lo, WordValue hi) {
      return (y.lo() != lo) || (y.hi() != hi);
    }
    forceinline bool is_aliased(WordView x, WordView y) {
      return x == y;
    }
    template<class VY>
    forceinline bool is_aliased(WordView, VY) {
      return false;
    }
  }

  /// Borrowed normalized inputs and result, with the folded scalar constant.
  template<class VY>
  struct NaryLogicOperands {
    ViewArray<WordView>& input;
    VY result;
    WordValue constant;
    WordValue mask;
  };

  /// Scalar configuration needed by one local AND/OR mask calculation.
  struct NaryLogicParameters {
    int size;
    WordValue constant;
    WordValue mask;
  };

  /// Borrows caller-owned input mask arrays; result masks are copied by value.
  struct NaryLogicMasks {
    WordValue* lo;
    WordValue* hi;
    WordValue result_lo;
    WordValue result_hi;
  };

  /// One local pass's support and change facts; failed masks are not published.
  struct NaryLogicPassResult {
    bool has_support;
    bool has_changed;
  };

  /// Completed local closure; input mask storage remains owned by the caller.
  struct NaryLogicResult {
    bool has_support;
    NaryLogicMasks masks;
  };

  /// Determines whether every role is assigned after completed propagation.
  template<class VY>
  forceinline ExecStatus
  get_nary_status(const NaryLogicOperands<VY>& operands) {
    bool is_assigned=operands.result.assigned();
    for (int i=0; i<operands.input.size(); i++) {
      is_assigned &= operands.input[i].assigned();
    }
    return is_assigned ? ES_OK : ES_FIX;
  }

  /// Closes XOR with live reads after tells, without allocating mask storage.
  template<class VY>
  forceinline ExecStatus
  narrow_xor(Home home, const NaryLogicOperands<VY>& operands) {
    ViewArray<WordView>& x=operands.input;
    VY y=operands.result;
    const WordValue mask=operands.mask;
    for (;;) {
      WordValue parity=operands.constant, unknown=0;
      WordValue once=0, twice=0;
      for (int i=0; i<x.size(); i++) {
        parity ^= x[i].lo();
        const WordValue u=x[i].unknown();
        unknown |= u;
        twice |= once&u;
        once |= u;
      }
      const WordValue known=~unknown&mask;
      const WordValue result=parity&known;
      const WordValue ylo=y.lo()|result;
      const WordValue yhi=y.hi()&(result|unknown);
      const bool has_result_support=(ylo&~yhi) == 0;
      if (!has_result_support)
        return ES_FAILED;
      bool has_changed=nary_support::has_changed(y,ylo,yhi);
      if (has_changed)
        GECODE_ME_CHECK(y.narrow(home,ylo,yhi));

      const WordValue exact_one=once&~twice&mask;
      const WordValue y_known=~y.unknown()&mask;
      const WordValue required=y.lo()^parity;
      for (int i=0; i<x.size(); i++) {
        const WordValue old_lo=x[i].lo(), old_hi=x[i].hi();
        const WordValue force=x[i].unknown()&exact_one&y_known;
        const WordValue lo=old_lo|(force&required);
        const WordValue hi=old_hi&~(force&~required);
        const bool has_input_support=(lo&~hi) == 0;
        if (!has_input_support)
          return ES_FAILED;
        const bool has_input_change=(lo != old_lo) || (hi != old_hi);
        if (has_input_change) {
          has_changed=true;
          GECODE_ME_CHECK(x[i].narrow(home,lo,hi));
        }
      }
      if (!has_changed)
        break;
    }
    return get_nary_status(operands);
  }

  /// Intersects one AND pass into borrowed local masks without telling views.
  forceinline NaryLogicPassResult
  intersect_and_masks(const NaryLogicParameters& parameters,
                      NaryLogicMasks& masks) {
    const WordValue constant=parameters.constant, mask=parameters.mask;
    WordValue all_lo=constant, all_hi=constant;
    WordValue once=~constant&mask, twice=0;
    for (int i=0; i<parameters.size; i++) {
      all_lo &= masks.lo[i];
      all_hi &= masks.hi[i];
      const WordValue not_one=~masks.lo[i]&mask;
      twice |= once&not_one;
      once |= not_one;
    }
    const WordValue next_ylo=masks.result_lo|all_lo;
    const WordValue next_yhi=masks.result_hi&all_hi;
    const bool has_result_support=(next_ylo&~next_yhi) == 0;
    if (!has_result_support)
      return {false,false};
    bool has_changed=(next_ylo != masks.result_lo) ||
      (next_yhi != masks.result_hi);
    masks.result_lo=next_ylo; masks.result_hi=next_yhi;
    const WordValue exact_one=once&~twice&mask;
    const WordValue known_zero=~masks.result_hi&mask;
    for (int i=0; i<parameters.size; i++) {
      const WordValue lo=masks.lo[i]|masks.result_lo;
      const WordValue hi=masks.hi[i]&~(known_zero&exact_one&~masks.lo[i]);
      const bool has_input_support=(lo&~hi) == 0;
      if (!has_input_support)
        return {false,false};
      has_changed |= (lo != masks.lo[i]) || (hi != masks.hi[i]);
      masks.lo[i]=lo; masks.hi[i]=hi;
    }
    return {true,has_changed};
  }

  /// Intersects one OR pass into borrowed local masks without telling views.
  forceinline NaryLogicPassResult
  intersect_or_masks(const NaryLogicParameters& parameters,
                     NaryLogicMasks& masks) {
    const WordValue constant=parameters.constant, mask=parameters.mask;
    WordValue all_lo=constant, all_hi=constant;
    WordValue once=constant, twice=0;
    for (int i=0; i<parameters.size; i++) {
      all_lo |= masks.lo[i];
      all_hi |= masks.hi[i];
      const WordValue may_one=masks.hi[i];
      twice |= once&may_one;
      once |= may_one;
    }
    const WordValue next_ylo=masks.result_lo|all_lo;
    const WordValue next_yhi=masks.result_hi&all_hi;
    const bool has_result_support=(next_ylo&~next_yhi) == 0;
    if (!has_result_support)
      return {false,false};
    bool has_changed=(next_ylo != masks.result_lo) ||
      (next_yhi != masks.result_hi);
    masks.result_lo=next_ylo; masks.result_hi=next_yhi;
    const WordValue exact_one=once&~twice&mask;
    for (int i=0; i<parameters.size; i++) {
      const WordValue lo=masks.lo[i]|(masks.result_lo&exact_one&masks.hi[i]);
      const WordValue hi=masks.hi[i]&masks.result_hi;
      const bool has_input_support=(lo&~hi) == 0;
      if (!has_input_support)
        return {false,false};
      has_changed |= (lo != masks.lo[i]) || (hi != masks.hi[i]);
      masks.lo[i]=lo; masks.hi[i]=hi;
    }
    return {true,has_changed};
  }

  /// Closes AND/OR masks and result aliases before exposing them to publication.
  template<NaryOperation op, class VY>
  forceinline NaryLogicResult
  close_nary_masks(const NaryLogicOperands<VY>& operands,
                   NaryLogicMasks masks) {
    const NaryLogicParameters parameters={
      operands.input.size(),operands.constant,operands.mask
    };
    for (;;) {
      const NaryLogicPassResult pass=op == NO_AND ?
        intersect_and_masks(parameters,masks) :
        intersect_or_masks(parameters,masks);
      if (!pass.has_support)
        return {false,masks};
      bool has_changed=pass.has_changed;
      for (int i=0; i<operands.input.size(); i++) {
        if (nary_support::is_aliased(operands.input[i],operands.result)) {
          const WordValue lo=masks.lo[i]|masks.result_lo;
          const WordValue hi=masks.hi[i]&masks.result_hi;
          const bool has_support=(lo&~hi) == 0;
          if (!has_support)
            return {false,masks};
          has_changed |= (lo != masks.lo[i]) || (hi != masks.hi[i]) ||
            (lo != masks.result_lo) || (hi != masks.result_hi);
          masks.lo[i]=masks.result_lo=lo;
          masks.hi[i]=masks.result_hi=hi;
        }
      }
      if (!has_changed)
        return {true,masks};
    }
  }

  /// Tells the result first, then each changed input, forwarding early failure.
  template<class VY>
  forceinline ExecStatus
  publish_nary_masks(Home home, const NaryLogicOperands<VY>& operands,
                     const NaryLogicMasks& masks) {
    VY y=operands.result;
    if (nary_support::has_changed(y,masks.result_lo,masks.result_hi))
      GECODE_ME_CHECK(y.narrow(home,masks.result_lo,masks.result_hi));
    for (int i=0; i<operands.input.size(); i++) {
      if (nary_support::has_changed(operands.input[i],masks.lo[i],masks.hi[i]))
        GECODE_ME_CHECK(operands.input[i].narrow(home,masks.lo[i],masks.hi[i]));
    }
    return ES_OK;
  }

  /// Owns AND/OR scratch storage and repeats closure after bounded publication.
  template<NaryOperation op, class VY>
  forceinline ExecStatus
  narrow_nary_masks(Home home, const NaryLogicOperands<VY>& operands) {
    ViewArray<WordView>& x=operands.input;
    const VY y=operands.result;
    constexpr int kStackSize=32;
    WordValue stack[2*kStackSize];
    Region region;
    WordValue* xlo=x.size() <= kStackSize ? stack :
      region.alloc<WordValue>(2*x.size());
    WordValue* xhi=xlo+x.size();
    bool has_bounded_view=y.bounded();
    for (int i=0; i<x.size(); i++) {
      has_bounded_view |= x[i].bounded();
    }
    for (;;) {
      for (int i=0; i<x.size(); i++) {
        xlo[i]=x[i].lo();
        xhi[i]=x[i].hi();
      }
      const NaryLogicMasks before={xlo,xhi,y.lo(),y.hi()};
      const NaryLogicResult result=close_nary_masks<op>(operands,before);
      if (!result.has_support)
        return ES_FAILED;
      const NaryLogicMasks& masks=result.masks;
      GECODE_ES_CHECK(publish_nary_masks(home,operands,masks));
      bool is_synchronized=(masks.result_lo == y.lo()) &&
        (masks.result_hi == y.hi());
      for (int i=0; i<x.size(); i++) {
        is_synchronized &= (masks.lo[i] == x[i].lo()) &&
          (masks.hi[i] == x[i].hi());
      }
      const bool is_closed=!has_bounded_view || is_synchronized;
      if (is_closed)
        return get_nary_status(operands);
    }
  }

  template<NaryOperation op, class VY>
  forceinline
  Nary<op,VY>::Nary(Home home, ViewArray<WordView>& x0, VY y0,
                    WordValue c)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,VY,PC_WORD_BITS>(home,x0,y0), constant(c) {}

  template<NaryOperation op, class VY>
  forceinline
  Nary<op,VY>::Nary(Space& home, Nary<op,VY>& p)
    : MixNaryOnePropagator<
        WordView,PC_WORD_BITS,VY,PC_WORD_BITS>(home,p),
      constant(p.constant) {}

  /// Fold assigned inputs into the operation's constant before dropping them.
  template<NaryOperation op>
  forceinline bool
  compact_nary_logic(ViewArray<WordView>& x, WordValue& constant) {
    const int size=x.size();
    int n=0;
    for (int i=0; i<size; i++) {
      if (x[i].assigned()) {
        if (op == NO_AND) constant &= x[i].val();
        else if (op == NO_OR) constant |= x[i].val();
        else constant ^= x[i].val();
      } else {
        if (n != i) x[n]=x[i];
        n++;
      }
    }
    x.size(n);
    return n != size;
  }

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::narrow(Home home, ViewArray<WordView>& x, VY y,
                          WordValue constant) {
    const WordValue mask=y.mask();
    const bool is_absorbing=((op == NO_AND) && (constant == 0)) ||
      ((op == NO_OR) && (constant == mask));
    if (is_absorbing || (x.size() == 0)) {
      GECODE_ME_CHECK(y.eq(home,constant));
      return ES_OK;
    }
    const NaryLogicOperands<VY> operands={x,y,constant,mask};
    // XOR finishes before the AND/OR path acquires any scratch storage.
    if (op == NO_XOR)
      return narrow_xor(home,operands);
    return narrow_nary_masks<op>(home,operands);
  }

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::post(Home home, ViewArray<WordView>& x, VY y,
                    WordValue constant) {
    do {
      const ExecStatus es=narrow(home,x,y,constant);
      if (es != ES_FIX)
        return es;
    } while (compact_nary_logic<op>(x,constant));
    (void) new (home) Nary<op,VY>(home,x,y,constant);
    return ES_OK;
  }

  template<NaryOperation op, class VY>
  forceinline Actor*
  Nary<op,VY>::copy(Space& home) {
    return new (home) Nary<op,VY>(home,*this);
  }

  template<NaryOperation op, class VY>
  forceinline size_t
  Nary<op,VY>::dispose(Space& home) {
    (void) MixNaryOnePropagator<
      WordView,PC_WORD_BITS,VY,PC_WORD_BITS>::dispose(home);
    return sizeof(*this);
  }

  template<NaryOperation op, class VY>
  forceinline PropCost
  Nary<op,VY>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::linear(PropCost::LO,x.size());
  }

  template<NaryOperation op, class VY>
  forceinline ExecStatus
  Nary<op,VY>::propagate(Space& home, const ModEventDelta& med) {
    if (WordView::me(med) == ME_WORD_VAL)
      compact_nary_logic<op>(x,constant);
    do {
      const ExecStatus es=narrow(home,x,y,constant);
      if (es == ES_FAILED)
        return ES_FAILED;
      if (es == ES_OK)
        return home.ES_SUBSUMED(*this);
    } while (compact_nary_logic<op>(x,constant));
    return ES_FIX;
  }

}}}

// STATISTICS: word-prop
