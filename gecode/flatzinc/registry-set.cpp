/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *     Mikael Zayenz Lagerkvist, 2009
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

#include <gecode/flatzinc/registry-internal.hh>
#include <gecode/minimodel.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

namespace Gecode { namespace FlatZinc {

#ifdef GECODE_HAS_SET_VARS
  namespace {
    void p_set_OP(FlatZincSpace& s, SetOpType op,
                  const ConExpr& ce, AST::Node *) {
      rel(s, s.arg2SetVar(ce[0]), op, s.arg2SetVar(ce[1]),
          SRT_EQ, s.arg2SetVar(ce[2]));
    }
    void p_set_union(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_UNION, ce, ann);
    }
    void p_set_intersect(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_INTER, ce, ann);
    }
    void p_set_diff(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_set_OP(s, SOT_MINUS, ce, ann);
    }

    void p_set_symdiff(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      SetVar x = s.arg2SetVar(ce[0]);
      SetVar y = s.arg2SetVar(ce[1]);

      SetVarLubRanges xub(x);
      IntSet xubs(xub);
      SetVar x_y(s,IntSet::empty,xubs);
      rel(s, x, SOT_MINUS, y, SRT_EQ, x_y);

      SetVarLubRanges yub(y);
      IntSet yubs(yub);
      SetVar y_x(s,IntSet::empty,yubs);
      rel(s, y, SOT_MINUS, x, SRT_EQ, y_x);

      rel(s, x_y, SOT_UNION, y_x, SRT_EQ, s.arg2SetVar(ce[2]));
    }

    void p_array_set_OP(FlatZincSpace& s, SetOpType op,
                        const ConExpr& ce, AST::Node *) {
      SetVarArgs xs = s.arg2setvarargs(ce[0]);
      rel(s, op, xs, s.arg2SetVar(ce[1]));
    }
    void p_array_set_union(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_array_set_OP(s, SOT_UNION, ce, ann);
    }
    void p_array_set_partition(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      p_array_set_OP(s, SOT_DUNION, ce, ann);
    }


    void p_set_rel(FlatZincSpace& s, SetRelType srt, const ConExpr& ce) {
      rel(s, s.arg2SetVar(ce[0]), srt, s.arg2SetVar(ce[1]));
    }

    void p_set_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_EQ, ce);
    }
    void p_set_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_NQ, ce);
    }
    void p_set_subset(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_SUB, ce);
    }
    void p_set_superset(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_SUP, ce);
    }
    void p_set_le(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_LQ, ce);
    }
    void p_set_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel(s, SRT_LE, ce);
    }
    void p_set_card(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      if (!ce[1]->isIntVar()) {
        cardinality(s, s.arg2SetVar(ce[0]), ce[1]->getInt(),
                    ce[1]->getInt());
      } else {
        cardinality(s, s.arg2SetVar(ce[0]), s.arg2IntVar(ce[1]));
      }
    }
    void p_set_in(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      if (!ce[1]->isSetVar()) {
        IntSet d = s.arg2intset(ce[1]);
        if (ce[0]->isBoolVar()) {
          IntSetRanges dr(d);
          Iter::Ranges::Singleton sr(0,1);
          Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
          IntSet d01(i);
          if (d01.size() == 0) {
            s.fail();
          } else {
            rel(s, s.arg2BoolVar(ce[0]), IRT_GQ, d01.min());
            rel(s, s.arg2BoolVar(ce[0]), IRT_LQ, d01.max());
          }
        } else {
          dom(s, s.arg2IntVar(ce[0]), d);
        }
      } else {
        if (!ce[0]->isIntVar()) {
          dom(s, s.arg2SetVar(ce[1]), SRT_SUP, ce[0]->getInt());
        } else {
          rel(s, s.arg2SetVar(ce[1]), SRT_SUP, s.arg2IntVar(ce[0]));
        }
      }
    }
    void p_set_rel_reif(FlatZincSpace& s, SetRelType srt, const ConExpr& ce) {
      rel(s, s.arg2SetVar(ce[0]), srt, s.arg2SetVar(ce[1]),
          s.arg2BoolVar(ce[2]));
    }

    void p_set_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_EQ,ce);
    }
    void p_set_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_LQ,ce);
    }
    void p_set_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_LE,ce);
    }
    void p_set_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      p_set_rel_reif(s,SRT_NQ,ce);
    }
    void p_set_subset_reif(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node *) {
      p_set_rel_reif(s,SRT_SUB,ce);
    }
    void p_set_superset_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node *) {
      p_set_rel_reif(s,SRT_SUP,ce);
    }
    void p_set_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann, ReifyMode rm) {
      if (!ce[1]->isSetVar()) {
        if (rm==RM_EQV) {
          RegistryDetail::post_int_in_reif(s,ce,ann);
        } else {
          assert(rm==RM_IMP);
          RegistryDetail::post_int_in_imp(s,ce,ann);
        }
      } else {
        if (!ce[0]->isIntVar()) {
          dom(s, s.arg2SetVar(ce[1]), SRT_SUP, ce[0]->getInt(),
              Reify(s.arg2BoolVar(ce[2]),rm));
        } else {
          rel(s, s.arg2SetVar(ce[1]), SRT_SUP, s.arg2IntVar(ce[0]),
              Reify(s.arg2BoolVar(ce[2]),rm));
        }
      }
    }
    void p_set_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_set_in_reif(s,ce,ann,RM_EQV);
    }
    void p_set_in_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_set_in_reif(s,ce,ann,RM_IMP);
    }
    void p_set_disjoint(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      rel(s, s.arg2SetVar(ce[0]), SRT_DISJ, s.arg2SetVar(ce[1]));
    }

    void p_link_set_to_booleans(FlatZincSpace& s, const ConExpr& ce,
                                AST::Node *) {
      SetVar x = s.arg2SetVar(ce[0]);
      int idx = ce[2]->getInt();
      assert(idx >= 0);
      rel(s, x || IntSet(Set::Limits::min,idx-1));
      BoolVarArgs y = s.arg2boolvarargs(ce[1],idx);
      unshare(s, y);
      channel(s, y, x);
    }

    void p_array_set_element(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node*) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isSetVar()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = s.arg2IntVar(ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntSetArgs sv = s.arg2intsetargs(ce[1],1);
        element(s, sv, selector, s.arg2SetVar(ce[2]));
      } else {
        SetVarArgs sv = s.arg2setvarargs(ce[1], 1);
        element(s, sv, selector, s.arg2SetVar(ce[2]));
      }
    }

    void p_array_set_element_op(FlatZincSpace& s, const ConExpr& ce,
                                AST::Node*, SetOpType op,
                                const IntSet& universe =
                                IntSet(Set::Limits::min,Set::Limits::max)) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (a->a[i]->isSetVar()) {
          isConstant = false;
          break;
        }
      }
      SetVar selector = s.arg2SetVar(ce[0]);
      dom(s, selector, SRT_DISJ, 0);
      if (isConstant) {
        IntSetArgs sv = s.arg2intsetargs(ce[1], 1);
        element(s, op, sv, selector, s.arg2SetVar(ce[2]), universe);
      } else {
        SetVarArgs sv = s.arg2setvarargs(ce[1], 1);
        element(s, op, sv, selector, s.arg2SetVar(ce[2]), universe);
      }
    }

    void p_array_set_element_union(FlatZincSpace& s, const ConExpr& ce,
                                       AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_UNION);
    }

    void p_array_set_element_intersect(FlatZincSpace& s, const ConExpr& ce,
                                       AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_INTER);
    }

    void p_array_set_element_intersect_in(FlatZincSpace& s,
                                              const ConExpr& ce,
                                              AST::Node* ann) {
      IntSet d = s.arg2intset(ce[3]);
      p_array_set_element_op(s, ce, ann, SOT_INTER, d);
    }

    void p_array_set_element_partition(FlatZincSpace& s, const ConExpr& ce,
                                           AST::Node* ann) {
      p_array_set_element_op(s, ce, ann, SOT_DUNION);
    }

    void p_set_convex(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      convex(s, s.arg2SetVar(ce[0]));
    }

    void p_array_set_seq(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      SetVarArgs sv = s.arg2setvarargs(ce[0]);
      if (sv.size() > 0)
        sequence(s, sv);
    }

    void p_array_set_seq_union(FlatZincSpace& s, const ConExpr& ce,
                               AST::Node *) {
      SetVarArgs sv = s.arg2setvarargs(ce[0]);
      sequence(s, sv, s.arg2SetVar(ce[1]));
    }

    void p_int_set_channel(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node *) {
      int xoff=ce[1]->getInt();
      assert(xoff >= 0);
      int yoff=ce[3]->getInt();
      assert(yoff >= 0);
      IntVarArgs xv = s.arg2intvarargs(ce[0], xoff);
      SetVarArgs yv = s.arg2setvarargs(ce[2], yoff, 1, IntSet(0, xoff-1));
      IntSet xd(yoff,yv.size()-1);
      for (int i=xoff; i<xv.size(); i++) {
        dom(s, xv[i], xd);
      }
      IntSet yd(xoff,xv.size()-1);
      for (int i=yoff; i<yv.size(); i++) {
        dom(s, yv[i], SRT_SUB, yd);
      }
      channel(s,xv,yv);
    }

    void p_range(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      int xoff=ce[1]->getInt();
      assert(xoff >= 0);
      IntVarArgs xv = s.arg2intvarargs(ce[0],xoff);
      element(s, SOT_UNION, xv, s.arg2SetVar(ce[2]), s.arg2SetVar(ce[3]));
    }

    void p_weights(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      IntArgs e = s.arg2intargs(ce[0]);
      IntArgs w = s.arg2intargs(ce[1]);
      SetVar x = s.arg2SetVar(ce[2]);
      IntVar y = s.arg2IntVar(ce[3]);
      weights(s,e,w,x,y);
    }

    void p_inverse_set(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      int xoff = ce[2]->getInt();
      int yoff = ce[3]->getInt();
      SetVarArgs x = s.arg2setvarargs(ce[0],xoff);
      SetVarArgs y = s.arg2setvarargs(ce[1],yoff);
      channel(s, x, y);
    }

    void p_precede_set(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      SetVarArgs x = s.arg2setvarargs(ce[0]);
      int p_s = ce[1]->getInt();
      int p_t = ce[2]->getInt();
      precede(s,x,p_s,p_t);
    }


    SetRelType set_relation(AST::Node* arg) {
      switch (arg->getInt()) {
      case 1: return SRT_EQ;
      case 2: return SRT_NQ;
      case 3: return SRT_SUB;
      case 4: return SRT_SUP;
      case 5: return SRT_DISJ;
      case 6: return SRT_CMPL;
      case 7: return SRT_LQ;
      case 8: return SRT_LE;
      case 9: return SRT_GQ;
      case 10: return SRT_GR;
      default: throw FlatZinc::Error("Registry", "invalid set relation");
      }
    }
    void p_set_ite(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      ite(s, s.arg2BoolVar(ce[0]), s.arg2SetVar(ce[1]), s.arg2SetVar(ce[2]),
          s.arg2SetVar(ce[3]));
    }
    void p_set_convex_hull(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      convex(s, s.arg2SetVar(ce[0]), s.arg2SetVar(ce[1]));
    }
    void p_set_atmost_one(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      int cardinality = ce[1]->getInt();
      if (cardinality < 0)
        throw FlatZinc::Error("Registry", "set cardinality must be nonnegative");
      SetVarArgs x = s.arg2setvarargs(ce[0]);
      if ((x.size() < 2) || (cardinality <= 1)) {
        // The intersection bound is trivial here. atmostOne skips cardinality
        // for fewer than two sets, and its filter divides by cardinality - 1.
        for (int i=0; i<x.size(); i++)
          Gecode::cardinality(s, x[i], cardinality, cardinality);
      } else {
        atmostOne(s, x, static_cast<unsigned int>(cardinality));
      }
    }
    void p_set_channel_sorted(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      channelSorted(s, s.arg2intvarargs(ce[0]), s.arg2SetVar(ce[1]));
    }
    void p_array_set_intersect(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      rel(s, SOT_INTER, s.arg2setvarargs(ce[0]), s.arg2SetVar(ce[1]));
    }
    void p_set_element2d(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      SetVarArgs x = s.arg2setvarargs(ce[0]);
      int width = ce[2]->getInt(), height = ce[4]->getInt();
      if ((width <= 0) || (height <= 0) ||
          (static_cast<long long>(width)*height != x.size()))
        throw FlatZinc::Error("Registry", "invalid set matrix dimensions");
      element(s, x, s.arg2IntVar(ce[1]), width,
              s.arg2IntVar(ce[3]), height, s.arg2SetVar(ce[5]));
    }

    void p_set_min(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      min(s, s.arg2SetVar(ce[0]), s.arg2IntVar(ce[1]));
    }
    template<ReifyMode mode>
    void p_set_min_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      min(s, s.arg2SetVar(ce[0]), s.arg2IntVar(ce[1]),
             Reify(s.arg2BoolVar(ce[2]), mode));
    }

    void p_set_max(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      max(s, s.arg2SetVar(ce[0]), s.arg2IntVar(ce[1]));
    }
    template<ReifyMode mode>
    void p_set_max_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      max(s, s.arg2SetVar(ce[0]), s.arg2IntVar(ce[1]),
             Reify(s.arg2BoolVar(ce[2]), mode));
    }



    void p_set_rel_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      rel(s, s.arg2SetVar(ce[0]), set_relation(ce[1]), s.arg2SetVar(ce[2]),
          Reify(s.arg2BoolVar(ce[3]), RM_IMP));
    }

  }

  namespace RegistryDetail {

    void register_set(Registry& r) {
      r.add("set_eq", &p_set_eq);
      r.add("set_le", &p_set_le);
      r.add("set_lt", &p_set_lt);
      r.add("equal", &p_set_eq);
      r.add("set_ne", &p_set_ne);
      r.add("set_union", &p_set_union);
      r.add("array_set_element", &p_array_set_element);
      r.add("array_var_set_element", &p_array_set_element);
      r.add("set_intersect", &p_set_intersect);
      r.add("set_diff", &p_set_diff);
      r.add("set_symdiff", &p_set_symdiff);
      r.add("set_subset", &p_set_subset);
      r.add("set_superset", &p_set_superset);
      r.add("set_card", &p_set_card);
      r.add("set_in", &p_set_in);
      r.add("set_eq_reif", &p_set_eq_reif);
      r.add("set_le_reif", &p_set_le_reif);
      r.add("set_lt_reif", &p_set_lt_reif);
      r.add("equal_reif", &p_set_eq_reif);
      r.add("set_ne_reif", &p_set_ne_reif);
      r.add("set_subset_reif", &p_set_subset_reif);
      r.add("set_superset_reif", &p_set_superset_reif);
      r.add("set_in_reif", &p_set_in_reif);
      r.add("set_in_imp", &p_set_in_imp);
      r.add("disjoint", &p_set_disjoint);
      r.add("gecode_link_set_to_booleans",
            &p_link_set_to_booleans);

      r.add("array_set_union", &p_array_set_union);
      r.add("array_set_partition", &p_array_set_partition);
      r.add("set_convex", &p_set_convex);
      r.add("array_set_seq", &p_array_set_seq);
      r.add("array_set_seq_union", &p_array_set_seq_union);
      r.add("gecode_array_set_element_union",
            &p_array_set_element_union);
      r.add("gecode_array_set_element_intersect",
            &p_array_set_element_intersect);
      r.add("gecode_array_set_element_intersect_in",
            &p_array_set_element_intersect_in);
      r.add("gecode_array_set_element_partition",
            &p_array_set_element_partition);
      r.add("gecode_int_set_channel",
            &p_int_set_channel);
      r.add("gecode_range",
            &p_range);
      r.add("gecode_set_weights",
            &p_weights);
      r.add("gecode_inverse_set", &p_inverse_set);
      r.add("gecode_precede_set", &p_precede_set);
      r.add("gecode_set_rel_imp_fzn", &p_set_rel_imp);
      r.add("gecode_set_ite", &p_set_ite);
      r.add("gecode_set_convex_hull", &p_set_convex_hull);
      r.add("gecode_set_atmost_one", &p_set_atmost_one);
      r.add("gecode_set_channel_sorted", &p_set_channel_sorted);
      r.add("gecode_array_set_intersect", &p_array_set_intersect);
      r.add("gecode_set_element2d", &p_set_element2d);
      r.add("gecode_set_min", &p_set_min);
      r.add("gecode_set_min_reif", &p_set_min_reif<RM_EQV>);
      r.add("gecode_set_min_imp", &p_set_min_reif<RM_IMP>);
      r.add("gecode_set_max", &p_set_max);
      r.add("gecode_set_max_reif", &p_set_max_reif<RM_EQV>);
      r.add("gecode_set_max_imp", &p_set_max_reif<RM_IMP>);
    }

  }
#endif

}}

// STATISTICS: flatzinc-any
