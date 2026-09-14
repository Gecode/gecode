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
 *
 */

#include <gecode/word/rel.hh>

namespace Gecode {

  namespace {
    /// Two typed operand views; their Space retains variable ownership.
    template<class View0, class View1>
    struct Operands {
      const View0 left;
      const View1 right;
    };
    template<class View0, class View1>
    Operands(View0,View1) -> Operands<View0,View1>;

    /// Operand order and relation kind for one private dispatch.
    template<class View0, class View1>
    struct Relation {
      const View0 left;
      const WordRelType type;
      const View1 right;
    };
    template<class View0, class View1>
    Relation(View0,WordRelType,View1) -> Relation<View0,View1>;

    /// A native Boolean view and the mode interpreted at the original switch.
    template<class CtrlView>
    struct Control {
      const CtrlView view;
      const ReifyMode mode;
    };
    template<class CtrlView>
    Control(CtrlView,ReifyMode) -> Control<CtrlView>;

    /// Reverse implication direction when negating the reification control
    ReifyMode negate_reification_mode(ReifyMode rm) {
      switch (rm) {
      case RM_EQV: return RM_EQV;
      case RM_IMP: return RM_PMI;
      case RM_PMI: return RM_IMP;
      default: throw Word::UnknownReifyMode("Word::rel");
      }
    }

    /// Select a bit-based actor while preserving relation direction
    template<class View0, class View1>
    void post_rel(Home home, const Relation<View0,View1>& relation) {
      const auto& [x,wrt,y]=relation;
      switch (wrt) {
      case WRT_EQ:
        GECODE_ES_FAIL((Word::Rel::Eq<View0,View1>::post(home,x,y)));
        break;
      case WRT_NQ:
        GECODE_ES_FAIL((Word::Rel::Nq<View0,View1>::post(home,x,y)));
        break;
      case WRT_ULQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View0,View1,false>::post(home,x,y)));
        break;
      case WRT_ULE:
        GECODE_ES_FAIL((Word::Rel::Le<View0,View1,false>::post(home,x,y)));
        break;
      case WRT_UGQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View1,View0,false>::post(home,y,x)));
        break;
      case WRT_UGR:
        GECODE_ES_FAIL((Word::Rel::Le<View1,View0,false>::post(home,y,x)));
        break;
      case WRT_SLQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View0,View1,true>::post(home,x,y)));
        break;
      case WRT_SLE:
        GECODE_ES_FAIL((Word::Rel::Le<View0,View1,true>::post(home,x,y)));
        break;
      case WRT_SGQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View1,View0,true>::post(home,y,x)));
        break;
      case WRT_SGR:
        GECODE_ES_FAIL((Word::Rel::Le<View1,View0,true>::post(home,y,x)));
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }

    /// Select the equality actor for a runtime reification mode
    template<class View0, class View1, class CtrlView>
    void post_re_eq(Home home, const Operands<View0,View1>& operands,
                    const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_EQV>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_IMP>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_PMI>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    /// Select the ordering actor for a runtime reification mode
    template<class View0, class View1, class CtrlView, bool is_signed>
    void post_re_lq(Home home, const Operands<View0,View1>& operands,
                    const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_EQV,is_signed>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_IMP,is_signed>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_PMI,is_signed>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    /// Post negated ordering with reversed implication direction
    template<class View0, class View1, class CtrlView, bool is_signed>
    void post_re_not_lq(Home home, const Operands<View0,View1>& operands,
                        const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      Int::NegBoolView nb(b);
      post_re_lq<View0,View1,Int::NegBoolView,is_signed>(
        home,Operands{x,y},Control{nb,negate_reification_mode(rm)});
    }

    /// Select a bounded actor for views with compatible rank orders
    template<class View0, class View1>
    void post_bound_rel(Home home, const Relation<View0,View1>& relation) {
      const auto& [x,wrt,y]=relation;
      switch (wrt) {
      case WRT_EQ:
        GECODE_ES_FAIL((Word::Rel::BoundEq<View0,View1>::post(
          home,x,y)));
        break;
      case WRT_NQ:
        GECODE_ES_FAIL(Word::Rel::post_bounded_disequality(home,x,y));
        break;
      case WRT_ULQ: case WRT_SLQ:
        GECODE_ES_FAIL((Word::Rel::BoundLq<
          View0,View1,false>::post(home,x,y)));
        break;
      case WRT_ULE: case WRT_SLE:
        GECODE_ES_FAIL((Word::Rel::BoundLq<
          View0,View1,true>::post(home,x,y)));
        break;
      case WRT_UGQ: case WRT_SGQ:
        GECODE_ES_FAIL((Word::Rel::BoundLq<
          View1,View0,false>::post(home,y,x)));
        break;
      case WRT_UGR: case WRT_SGR:
        GECODE_ES_FAIL((Word::Rel::BoundLq<
          View1,View0,true>::post(home,y,x)));
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }

    /// Select bounded equality for a runtime reification mode
    template<class View0, class View1, class CtrlView>
    void post_re_bound_eq(Home home, const Operands<View0,View1>& operands,
                          const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReBoundEq<
          View0,View1,CtrlView,RM_EQV>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReBoundEq<
          View0,View1,CtrlView,RM_IMP>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReBoundEq<
          View0,View1,CtrlView,RM_PMI>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    /// Select bounded ordering for a runtime reification mode
    template<class View0, class View1, class CtrlView>
    void post_re_bound_lq(Home home, const Operands<View0,View1>& operands,
                          const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReBoundLq<
          View0,View1,CtrlView,RM_EQV>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReBoundLq<
          View0,View1,CtrlView,RM_IMP>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReBoundLq<
          View0,View1,CtrlView,RM_PMI>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    /// Post negated bounded ordering with reversed implication direction
    template<class View0, class View1, class CtrlView>
    void post_re_bound_not_lq(Home home, const Operands<View0,View1>& operands,
                              const Control<CtrlView>& control) {
      const auto& [x,y]=operands;
      const auto& [b,rm]=control;
      Int::NegBoolView nb(b);
      post_re_bound_lq(
        home,Operands{x,y},Control{nb,negate_reification_mode(rm)});
    }

    /// Select a bounded actor for views with compatible rank orders
    template<class View0, class View1>
    void post_bound_rel(Home home, const Relation<View0,View1>& relation,
                        Reify r) {
      const auto& [x,wrt,y]=relation;
      Int::BoolView b(r.var());
      switch (wrt) {
      case WRT_EQ:
        post_re_bound_eq(home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_NQ: {
        Int::NegBoolView nb(b);
        post_re_bound_eq(
          home,Operands{x,y},Control{nb,negate_reification_mode(r.mode())});
        break;
      }
      case WRT_ULQ: case WRT_SLQ:
        post_re_bound_lq(home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_ULE: case WRT_SLE:
        post_re_bound_not_lq(home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_UGQ: case WRT_SGQ:
        post_re_bound_lq(home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_UGR: case WRT_SGR:
        post_re_bound_not_lq(home,Operands{x,y},Control{b,r.mode()});
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }

    /**
     * Test eligibility for bounded propagation in this relation's order.
     * Throws Word::UnknownRelation for an unknown WordRelType; a recognized
     * but incompatible relation returns false.
     */
    forceinline bool can_use_bounds(WordDomainType domain_type,
                                      WordRelType wrt) {
      switch (wrt) {
      case WRT_EQ: case WRT_NQ:
        return domain_type != WDT_CUBE;
      case WRT_ULQ: case WRT_ULE: case WRT_UGQ: case WRT_UGR:
        return domain_type == WDT_UNSIGNED;
      case WRT_SLQ: case WRT_SLE: case WRT_SGQ: case WRT_SGR:
        return domain_type == WDT_SIGNED;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }

    /// Select a bit-based actor while preserving relation direction
    template<class View0, class View1>
    void post_rel(Home home, const Relation<View0,View1>& relation,
                  Reify r) {
      const auto& [x,wrt,y]=relation;
      Int::BoolView b(r.var());
      switch (wrt) {
      case WRT_EQ:
        post_re_eq(home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_NQ: {
        Int::NegBoolView nb(b);
        post_re_eq(
          home,Operands{x,y},Control{nb,negate_reification_mode(r.mode())});
        break;
      }
      case WRT_ULQ:
        post_re_lq<View0,View1,Int::BoolView,false>(
          home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_ULE:
        post_re_not_lq<View1,View0,Int::BoolView,false>(
          home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_UGQ:
        post_re_lq<View1,View0,Int::BoolView,false>(
          home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_UGR:
        post_re_not_lq<View0,View1,Int::BoolView,false>(
          home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_SLQ:
        post_re_lq<View0,View1,Int::BoolView,true>(
          home,Operands{x,y},Control{b,r.mode()});
        break;
      case WRT_SLE:
        post_re_not_lq<View1,View0,Int::BoolView,true>(
          home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_SGQ:
        post_re_lq<View1,View0,Int::BoolView,true>(
          home,Operands{y,x},Control{b,r.mode()});
        break;
      case WRT_SGR:
        post_re_not_lq<View0,View1,Int::BoolView,true>(
          home,Operands{x,y},Control{b,r.mode()});
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }
  }

  void
  rel(Home home, WordVar x, WordRelType wrt, WordVar y) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    Word::WordView xv(x), yv(y);
    if (xv == yv) {
      post_rel(home,Relation{xv,wrt,yv});
      return;
    }
    const bool has_compatible_bounds=
      (x.domain_type() == y.domain_type()) &&
      can_use_bounds(x.domain_type(),wrt);
    if (has_compatible_bounds) {
      if (x.domain_type() == WDT_UNSIGNED)
        post_bound_rel(home,Relation{
          Word::UnsignedWordView(x),wrt,
          Word::UnsignedWordView(y)});
      else
        post_bound_rel(home,Relation{
          Word::SignedWordView(x),wrt,
          Word::SignedWordView(y)});
    } else {
      post_rel(home,Relation{xv,wrt,yv});
    }
  }

  void
  rel(Home home, WordVar x, WordRelType wrt, WordVar y, Reify r) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    Word::WordView xv(x), yv(y);
    if (xv == yv) {
      post_rel(home,Relation{xv,wrt,yv},r);
      return;
    }
    const bool has_compatible_bounds=
      (x.domain_type() == y.domain_type()) &&
      can_use_bounds(x.domain_type(),wrt);
    if (has_compatible_bounds) {
      if (x.domain_type() == WDT_UNSIGNED)
        post_bound_rel(home,Relation{
          Word::UnsignedWordView(x),wrt,
          Word::UnsignedWordView(y)},r);
      else
        post_bound_rel(home,Relation{
          Word::SignedWordView(x),wrt,
          Word::SignedWordView(y)},r);
    } else {
      post_rel(home,Relation{xv,wrt,yv},r);
    }
  }

  void
  rel(Home home, WordVar x, WordRelType wrt,
      unsigned int width, WordValue value) {
    if (x.width() != width)
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    if (can_use_bounds(x.domain_type(),wrt)) {
      if (x.domain_type() == WDT_UNSIGNED)
        post_bound_rel(home,Relation{
          Word::UnsignedWordView(x),wrt,
          Word::Rel::RankedWordConstView<false>(width,value)});
      else
        post_bound_rel(home,Relation{
          Word::SignedWordView(x),wrt,
          Word::Rel::RankedWordConstView<true>(width,value)});
    } else {
      Word::ConstWordView c(width,value);
      post_rel(home,Relation{Word::WordView(x),wrt,c});
    }
  }

  void
  rel(Home home, WordVar x, WordRelType wrt,
      unsigned int width, WordValue value, Reify r) {
    if (x.width() != width)
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    if (can_use_bounds(x.domain_type(),wrt)) {
      if (x.domain_type() == WDT_UNSIGNED)
        post_bound_rel(home,Relation{
          Word::UnsignedWordView(x),wrt,
          Word::Rel::RankedWordConstView<false>(width,value)},r);
      else
        post_bound_rel(home,Relation{
          Word::SignedWordView(x),wrt,
          Word::Rel::RankedWordConstView<true>(width,value)},r);
    } else {
      Word::ConstWordView c(width,value);
      post_rel(home,Relation{Word::WordView(x),wrt,c},r);
    }
  }

}

// STATISTICS: word-post
