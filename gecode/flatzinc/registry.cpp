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
#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/blackbox.hh>

namespace Gecode { namespace FlatZinc {

  Registry& registry(void) {
    static Registry r;
    return r;
  }

  void
  Registry::post(FlatZincSpace& s, const ConExpr& ce) {
    std::map<std::string,poster>::iterator i = r.find(ce.id);
    if (i == r.end()) {
      throw FlatZinc::Error("Registry",
        std::string("Constraint ")+ce.id+" not found", ce.ann);
    }
    i->second(s, ce, ce.ann);
  }

  void
  Registry::add(const std::string& id, poster p) {
    r[id] = p;
    r["gecode_" + id] = p;
    r["fzn_" + id] = p;
  }

  namespace {

    void p_distinct(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = s.arg2intvarargs(ce[0]);
      IntPropLevel ipl = s.ann2ipl(ann);
      unshare(s, va);
      distinct(s, va, ipl == IPL_DEF ? IPL_BND : ipl);
    }

    void p_distinctOffset(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = s.arg2intvarargs(ce[1]);
      unshare(s, va);
      AST::Array* offs = ce.args->a[0]->getArray();
      IntArgs oa(offs->a.size());
      for (int i=offs->a.size(); i--; ) {
        oa[i] = offs->a[i]->getInt();
      }
      IntPropLevel ipl = s.ann2ipl(ann);
      distinct(s, oa, va, ipl == IPL_DEF ? IPL_BND : ipl);
    }

    void p_all_equal(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs va = s.arg2intvarargs(ce[0]);
      rel(s, va, IRT_EQ, s.ann2ipl(ann));
    }

    void p_int_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                   AST::Node* ann) {
      if (ce[0]->isIntVar()) {
        if (ce[1]->isIntVar()) {
          rel(s, s.arg2IntVar(ce[0]), irt, s.arg2IntVar(ce[1]),
              s.ann2ipl(ann));
        } else {
          rel(s, s.arg2IntVar(ce[0]), irt, ce[1]->getInt(), s.ann2ipl(ann));
        }
      } else {
        rel(s, s.arg2IntVar(ce[1]), swap(irt), ce[0]->getInt(),
            s.ann2ipl(ann));
      }
    }
    void p_int_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_EQ, ce, ann);
    }
    void p_int_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_NQ, ce, ann);
    }
    void p_int_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_GQ, ce, ann);
    }
    void p_int_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_GR, ce, ann);
    }
    void p_int_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_LQ, ce, ann);
    }
    void p_int_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP(s, IRT_LE, ce, ann);
    }
    void p_int_CMP_reif(FlatZincSpace& s, IntRelType irt, ReifyMode rm,
                        const ConExpr& ce, AST::Node* ann) {
      if (rm == RM_EQV && ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_int_CMP(s, irt, ce, ann);
        } else {
          p_int_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      if (ce[0]->isIntVar()) {
        if (ce[1]->isIntVar()) {
          rel(s, s.arg2IntVar(ce[0]), irt, s.arg2IntVar(ce[1]),
                 Reify(s.arg2BoolVar(ce[2]), rm), s.ann2ipl(ann));
        } else {
          rel(s, s.arg2IntVar(ce[0]), irt, ce[1]->getInt(),
                 Reify(s.arg2BoolVar(ce[2]), rm), s.ann2ipl(ann));
        }
      } else {
        rel(s, s.arg2IntVar(ce[1]), swap(irt), ce[0]->getInt(),
               Reify(s.arg2BoolVar(ce[2]), rm), s.ann2ipl(ann));
      }
    }

    /* Comparisons */
    void p_int_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_EQ, RM_EQV, ce, ann);
    }
    void p_int_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_NQ, RM_EQV, ce, ann);
    }
    void p_int_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GQ, RM_EQV, ce, ann);
    }
    void p_int_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GR, RM_EQV, ce, ann);
    }
    void p_int_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LQ, RM_EQV, ce, ann);
    }
    void p_int_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LE, RM_EQV, ce, ann);
    }

    void p_int_eq_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_EQ, RM_IMP, ce, ann);
    }
    void p_int_ne_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_NQ, RM_IMP, ce, ann);
    }
    void p_int_ge_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GQ, RM_IMP, ce, ann);
    }
    void p_int_gt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_GR, RM_IMP, ce, ann);
    }
    void p_int_le_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LQ, RM_IMP, ce, ann);
    }
    void p_int_lt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_CMP_reif(s, IRT_LE, RM_IMP, ce, ann);
    }

    /* linear (in-)equations */
    void p_int_lin_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                       AST::Node* ann) {
      IntArgs ia = s.arg2intargs(ce[0]);
      int singleIntVar;
      if (s.isBoolArray(ce[1],singleIntVar)) {
        if (singleIntVar != -1) {
          if (std::abs(ia[singleIntVar]) == 1 && ce[2]->getInt() == 0) {
            IntVar siv = s.arg2IntVar(ce[1]->getArray()->a[singleIntVar]);
            BoolVarArgs iv = s.arg2boolvarargs(ce[1], 0, singleIntVar);
            IntArgs ia_tmp(ia.size()-1);
            int count = 0;
            for (int i=0; i<ia.size(); i++) {
              if (i != singleIntVar)
                ia_tmp[count++] = ia[singleIntVar] == -1 ? ia[i] : -ia[i];
            }
            IntRelType t = (ia[singleIntVar] == -1 ? irt : swap(irt));
            linear(s, ia_tmp, iv, t, siv, s.ann2ipl(ann));
          } else {
            IntVarArgs iv = s.arg2intvarargs(ce[1]);
            linear(s, ia, iv, irt, ce[2]->getInt(), s.ann2ipl(ann));
          }
        } else {
          BoolVarArgs iv = s.arg2boolvarargs(ce[1]);
          linear(s, ia, iv, irt, ce[2]->getInt(), s.ann2ipl(ann));
        }
      } else {
        IntVarArgs iv = s.arg2intvarargs(ce[1]);
        linear(s, ia, iv, irt, ce[2]->getInt(), s.ann2ipl(ann));
      }
    }
    void p_int_lin_CMP_reif(FlatZincSpace& s, IntRelType irt, ReifyMode rm,
                            const ConExpr& ce, AST::Node* ann) {
      if (rm == RM_EQV && ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_int_lin_CMP(s, irt, ce, ann);
        } else {
          p_int_lin_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      IntArgs ia = s.arg2intargs(ce[0]);
      int singleIntVar;
      if (s.isBoolArray(ce[1],singleIntVar)) {
        if (singleIntVar != -1) {
          if (std::abs(ia[singleIntVar]) == 1 && ce[2]->getInt() == 0) {
            IntVar siv = s.arg2IntVar(ce[1]->getArray()->a[singleIntVar]);
            BoolVarArgs iv = s.arg2boolvarargs(ce[1], 0, singleIntVar);
            IntArgs ia_tmp(ia.size()-1);
            int count = 0;
            for (int i=0; i<ia.size(); i++) {
              if (i != singleIntVar)
                ia_tmp[count++] = ia[singleIntVar] == -1 ? ia[i] : -ia[i];
            }
            IntRelType t = (ia[singleIntVar] == -1 ? irt : swap(irt));
            linear(s, ia_tmp, iv, t, siv, Reify(s.arg2BoolVar(ce[3]), rm),
                   s.ann2ipl(ann));
          } else {
            IntVarArgs iv = s.arg2intvarargs(ce[1]);
            linear(s, ia, iv, irt, ce[2]->getInt(),
                   Reify(s.arg2BoolVar(ce[3]), rm), s.ann2ipl(ann));
          }
        } else {
          BoolVarArgs iv = s.arg2boolvarargs(ce[1]);
          linear(s, ia, iv, irt, ce[2]->getInt(),
                 Reify(s.arg2BoolVar(ce[3]), rm), s.ann2ipl(ann));
        }
      } else {
        IntVarArgs iv = s.arg2intvarargs(ce[1]);
        linear(s, ia, iv, irt, ce[2]->getInt(),
               Reify(s.arg2BoolVar(ce[3]), rm),
               s.ann2ipl(ann));
      }
    }
    void p_int_lin_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_EQ, ce, ann);
    }
    void p_int_lin_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_EQ, RM_EQV, ce, ann);
    }
    void p_int_lin_eq_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_EQ, RM_IMP, ce, ann);
    }
    void p_int_lin_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_NQ, ce, ann);
    }
    void p_int_lin_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_NQ, RM_EQV, ce, ann);
    }
    void p_int_lin_ne_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_NQ, RM_IMP, ce, ann);
    }
    void p_int_lin_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_LQ, ce, ann);
    }
    void p_int_lin_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LQ, RM_EQV, ce, ann);
    }
    void p_int_lin_le_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LQ, RM_IMP, ce, ann);
    }
    void p_int_lin_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_LE, ce, ann);
    }
    void p_int_lin_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LE, RM_EQV, ce, ann);
    }
    void p_int_lin_lt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_LE, RM_IMP, ce, ann);
    }
    void p_int_lin_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_GQ, ce, ann);
    }
    void p_int_lin_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GQ, RM_EQV, ce, ann);
    }
    void p_int_lin_ge_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GQ, RM_IMP, ce, ann);
    }
    void p_int_lin_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP(s, IRT_GR, ce, ann);
    }
    void p_int_lin_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GR, RM_EQV, ce, ann);
    }
    void p_int_lin_gt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_int_lin_CMP_reif(s, IRT_GR, RM_IMP, ce, ann);
    }

    void p_bool_lin_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                        AST::Node* ann) {
      IntArgs ia = s.arg2intargs(ce[0]);
      BoolVarArgs iv = s.arg2boolvarargs(ce[1]);
      if (ce[2]->isIntVar())
        linear(s, ia, iv, irt, s.iv[ce[2]->getIntVar()], s.ann2ipl(ann));
      else
        linear(s, ia, iv, irt, ce[2]->getInt(), s.ann2ipl(ann));
    }
    void p_bool_lin_CMP_reif(FlatZincSpace& s, IntRelType irt, ReifyMode rm,
                            const ConExpr& ce, AST::Node* ann) {
      if (rm == RM_EQV && ce[2]->isBool()) {
        if (ce[2]->getBool()) {
          p_bool_lin_CMP(s, irt, ce, ann);
        } else {
          p_bool_lin_CMP(s, neg(irt), ce, ann);
        }
        return;
      }
      IntArgs ia = s.arg2intargs(ce[0]);
      BoolVarArgs iv = s.arg2boolvarargs(ce[1]);
      if (ce[2]->isIntVar())
        linear(s, ia, iv, irt, s.iv[ce[2]->getIntVar()],
               Reify(s.arg2BoolVar(ce[3]), rm),
               s.ann2ipl(ann));
      else
        linear(s, ia, iv, irt, ce[2]->getInt(),
               Reify(s.arg2BoolVar(ce[3]), rm),
               s.ann2ipl(ann));
    }
    void p_bool_lin_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_EQ, ce, ann);
    }
    void p_bool_lin_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_EQ, RM_EQV, ce, ann);
    }
    void p_bool_lin_eq_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_EQ, RM_IMP, ce, ann);
    }
    void p_bool_lin_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_NQ, ce, ann);
    }
    void p_bool_lin_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_NQ, RM_EQV, ce, ann);
    }
    void p_bool_lin_ne_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_NQ, RM_IMP, ce, ann);
    }
    void p_bool_lin_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_LQ, ce, ann);
    }
    void p_bool_lin_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_LQ, RM_EQV, ce, ann);
    }
    void p_bool_lin_le_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_LQ, RM_IMP, ce, ann);
    }
    void p_bool_lin_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP(s, IRT_LE, ce, ann);
    }
    void p_bool_lin_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_LE, RM_EQV, ce, ann);
    }
    void p_bool_lin_lt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_LE, RM_IMP, ce, ann);
    }
    void p_bool_lin_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_GQ, ce, ann);
    }
    void p_bool_lin_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_GQ, RM_EQV, ce, ann);
    }
    void p_bool_lin_ge_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_GQ, RM_IMP, ce, ann);
    }
    void p_bool_lin_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_lin_CMP(s, IRT_GR, ce, ann);
    }
    void p_bool_lin_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_GR, RM_EQV, ce, ann);
    }
    void p_bool_lin_gt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      p_bool_lin_CMP_reif(s, IRT_GR, RM_IMP, ce, ann);
    }

    /* arithmetic constraints */

    void p_int_plus(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      if (!ce[0]->isIntVar()) {
        rel(s, ce[0]->getInt() + s.arg2IntVar(ce[1])
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      } else if (!ce[1]->isIntVar()) {
        rel(s, s.arg2IntVar(ce[0]) + ce[1]->getInt()
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      } else if (!ce[2]->isIntVar()) {
        rel(s, s.arg2IntVar(ce[0]) + s.arg2IntVar(ce[1])
                == ce[2]->getInt(), s.ann2ipl(ann));
      } else {
        rel(s, s.arg2IntVar(ce[0]) + s.arg2IntVar(ce[1])
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      }
    }

    void p_int_minus(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      if (!ce[0]->isIntVar()) {
        rel(s, ce[0]->getInt() - s.arg2IntVar(ce[1])
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      } else if (!ce[1]->isIntVar()) {
        rel(s, s.arg2IntVar(ce[0]) - ce[1]->getInt()
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      } else if (!ce[2]->isIntVar()) {
        rel(s, s.arg2IntVar(ce[0]) - s.arg2IntVar(ce[1])
                == ce[2]->getInt(), s.ann2ipl(ann));
      } else {
        rel(s, s.arg2IntVar(ce[0]) - s.arg2IntVar(ce[1])
                == s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      }
    }

    void p_int_times(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      mult(s, x0, x1, x2, s.ann2ipl(ann));
    }
    void p_int_pow(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      pow(s, x0, ce[1]->getInt(), x2, s.ann2ipl(ann));
    }
    void p_int_div(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      IntVarArgs x = {x0, x1, x2};
      unshare(s, x);
      div(s,x[0],x[1],x[2], s.ann2ipl(ann));
    }
    void p_int_mod(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      IntVarArgs x = {x0, x1, x2};
      unshare(s, x);
      mod(s,x[0],x[1],x[2], s.ann2ipl(ann));
    }

    void p_int_min(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      min(s, x0, x1, x2, s.ann2ipl(ann));
    }
    void p_int_max(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      IntVar x2 = s.arg2IntVar(ce[2]);
      max(s, x0, x1, x2, s.ann2ipl(ann));
    }
    void p_int_negate(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      rel(s, x0 == -x1, s.ann2ipl(ann));
    }

    /* Boolean constraints */
    void p_bool_CMP(FlatZincSpace& s, IntRelType irt, const ConExpr& ce,
                   AST::Node* ann) {
      rel(s, s.arg2BoolVar(ce[0]), irt, s.arg2BoolVar(ce[1]),
          s.ann2ipl(ann));
    }
    void p_bool_CMP_reif(FlatZincSpace& s, IntRelType irt, ReifyMode rm,
                         const ConExpr& ce, AST::Node* ann) {
      rel(s, s.arg2BoolVar(ce[0]), irt, s.arg2BoolVar(ce[1]),
          Reify(s.arg2BoolVar(ce[2]), rm), s.ann2ipl(ann));
    }
    void p_bool_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_EQ, ce, ann);
    }
    void p_bool_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_EQ, RM_EQV, ce, ann);
    }
    void p_bool_eq_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_EQ, RM_IMP, ce, ann);
    }
    void p_bool_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_NQ, ce, ann);
    }
    void p_bool_ne_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_NQ, RM_EQV, ce, ann);
    }
    void p_bool_ne_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_NQ, RM_IMP, ce, ann);
    }
    void p_bool_ge(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_GQ, ce, ann);
    }
    void p_bool_ge_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GQ, RM_EQV, ce, ann);
    }
    void p_bool_ge_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GQ, RM_IMP, ce, ann);
    }
    void p_bool_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_LQ, ce, ann);
    }
    void p_bool_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LQ, RM_EQV, ce, ann);
    }
    void p_bool_le_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LQ, RM_IMP, ce, ann);
    }
    void p_bool_gt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_GR, ce, ann);
    }
    void p_bool_gt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GR, RM_EQV, ce, ann);
    }
    void p_bool_gt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_GR, RM_IMP, ce, ann);
    }
    void p_bool_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP(s, IRT_LE, ce, ann);
    }
    void p_bool_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LE, RM_EQV, ce, ann);
    }
    void p_bool_lt_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_bool_CMP_reif(s, IRT_LE, RM_IMP, ce, ann);
    }

#define BOOL_OP(op) \
    BoolVar b0 = s.arg2BoolVar(ce[0]); \
    BoolVar b1 = s.arg2BoolVar(ce[1]); \
    if (ce[2]->isBool()) { \
      rel(s, b0, op, b1, ce[2]->getBool(), s.ann2ipl(ann)); \
    } else { \
      rel(s, b0, op, b1, s.bv[ce[2]->getBoolVar()], s.ann2ipl(ann)); \
    }

#define BOOL_ARRAY_OP(op) \
    BoolVarArgs bv = s.arg2boolvarargs(ce[0]); \
    if (ce.size()==1) { \
      rel(s, op, bv, 1, s.ann2ipl(ann)); \
    } else if (ce[1]->isBool()) { \
      rel(s, op, bv, ce[1]->getBool(), s.ann2ipl(ann)); \
    } else { \
      rel(s, op, bv, s.bv[ce[1]->getBoolVar()], s.ann2ipl(ann)); \
    }

    void p_bool_or(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_OR);
    }
    void p_bool_or_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar b0 = s.arg2BoolVar(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      BoolVar b2 = s.arg2BoolVar(ce[2]);
      clause(s, BOT_OR, BoolVarArgs()<<b0<<b1, BoolVarArgs()<<b2, 1,
             s.ann2ipl(ann));
    }
    void p_bool_and(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_AND);
    }
    void p_bool_and_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar b0 = s.arg2BoolVar(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      BoolVar b2 = s.arg2BoolVar(ce[2]);
      rel(s, b2, BOT_IMP, b0, 1, s.ann2ipl(ann));
      rel(s, b2, BOT_IMP, b1, 1, s.ann2ipl(ann));
    }
    void p_array_bool_and(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_AND);
    }
    void p_array_bool_and_imp(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node* ann)
    {
      BoolVarArgs bv = s.arg2boolvarargs(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      for (unsigned int i=bv.size(); i--;)
        rel(s, b1, BOT_IMP, bv[i], 1, s.ann2ipl(ann));
    }
    void p_array_bool_or(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_OR);
    }
    void p_array_bool_or_imp(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann)
    {
      BoolVarArgs bv = s.arg2boolvarargs(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      clause(s, BOT_OR, bv, BoolVarArgs()<<b1, 1, s.ann2ipl(ann));
    }
    void p_array_bool_xor(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann)
    {
      BOOL_ARRAY_OP(BOT_XOR);
    }
    void p_array_bool_xor_imp(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node* ann)
    {
      BoolVarArgs bv = s.arg2boolvarargs(ce[0]);
      BoolVar tmp(s,0,1);
      rel(s, BOT_XOR, bv, tmp, s.ann2ipl(ann));
      rel(s, s.arg2BoolVar(ce[1]), BOT_IMP, tmp, 1);
    }
    void p_array_bool_clause(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      BoolVarArgs bvp = s.arg2boolvarargs(ce[0]);
      BoolVarArgs bvn = s.arg2boolvarargs(ce[1]);
      clause(s, BOT_OR, bvp, bvn, 1, s.ann2ipl(ann));
    }
    void p_array_bool_clause_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      BoolVarArgs bvp = s.arg2boolvarargs(ce[0]);
      BoolVarArgs bvn = s.arg2boolvarargs(ce[1]);
      BoolVar b0 = s.arg2BoolVar(ce[2]);
      clause(s, BOT_OR, bvp, bvn, b0, s.ann2ipl(ann));
    }
    void p_array_bool_clause_imp(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      BoolVarArgs bvp = s.arg2boolvarargs(ce[0]);
      BoolVarArgs bvn = s.arg2boolvarargs(ce[1]);
      BoolVar b0 = s.arg2BoolVar(ce[2]);
      clause(s, BOT_OR, bvp, bvn, b0, s.ann2ipl(ann));
    }
    void p_bool_xor(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_XOR);
    }
    void p_bool_xor_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar b0 = s.arg2BoolVar(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      BoolVar b2 = s.arg2BoolVar(ce[2]);
      clause(s, BOT_OR, BoolVarArgs()<<b0<<b1, BoolVarArgs()<<b2, 1,
             s.ann2ipl(ann));
      clause(s, BOT_OR, BoolVarArgs(), BoolVarArgs()<<b0<<b1<<b2, 1,
             s.ann2ipl(ann));
    }
    void p_bool_l_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar b0 = s.arg2BoolVar(ce[0]);
      BoolVar b1 = s.arg2BoolVar(ce[1]);
      if (ce[2]->isBool()) {
        rel(s, b1, BOT_IMP, b0, ce[2]->getBool(), s.ann2ipl(ann));
      } else {
        rel(s, b1, BOT_IMP, b0, s.bv[ce[2]->getBoolVar()], s.ann2ipl(ann));
      }
    }
    void p_bool_r_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BOOL_OP(BOT_IMP);
    }
    void p_bool_not(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar x0 = s.arg2BoolVar(ce[0]);
      BoolVar x1 = s.arg2BoolVar(ce[1]);
      rel(s, x0, BOT_XOR, x1, 1, s.ann2ipl(ann));
    }

    /* element constraints */
    void p_array_int_element(FlatZincSpace& s, const ConExpr& ce,
                                 AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isInt()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = s.arg2IntVar(ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntSharedArray sia = s.arg2intsharedarray(ce[1]);
        element(s, sia, selector, -1, s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      } else {
        IntVarArgs iv = s.arg2intvarargs(ce[1]);
        element(s, iv, selector, -1, s.arg2IntVar(ce[2]), s.ann2ipl(ann));
      }
    }
    void p_array_int_element_offset(FlatZincSpace& s, const ConExpr& ce,
                                    AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[2]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isInt()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = s.arg2IntVar(ce[0]);
      int offset = ce[1]->getInt();
      rel(s, selector >= offset);
      if (isConstant) {
        IntSharedArray sia = s.arg2intsharedarray(ce[2]);
        element(s, sia, selector, -offset, s.arg2IntVar(ce[3]), s.ann2ipl(ann));
      } else {
        IntVarArgs iv = s.arg2intvarargs(ce[2]);
        element(s, iv, selector, -offset, s.arg2IntVar(ce[3]), s.ann2ipl(ann));
      }
    }
    void p_array_int_element2d(FlatZincSpace& s, const ConExpr& ce,
                               AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[2]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isInt()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector0 = s.arg2IntVar(ce[0]);
      IntVar selector1 = s.arg2IntVar(ce[1]);
      IntSet idxset0 = s.arg2intset(ce[3]);
      IntSet idxset1 = s.arg2intset(ce[4]);

      int w = idxset1.size();
      int s1off = idxset1.min();
      int h = idxset0.size();
      int s0off = idxset0.min();

      if (isConstant) {
        IntSharedArray sia = s.arg2intsharedarray(ce[2], 0);
        element(s, sia, selector1, -s1off, w, selector0, -s0off, h, s.arg2IntVar(ce[5]), s.ann2ipl(ann));
      } else {
        IntVarArgs iv = s.arg2intvarargs(ce[2], 0);
        element(s, iv, selector1, -s1off, w, selector0, -s0off, h, s.arg2IntVar(ce[5]), s.ann2ipl(ann));
      }
    }
    void p_array_bool_element(FlatZincSpace& s, const ConExpr& ce,
                                  AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[1]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isBool()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = s.arg2IntVar(ce[0]);
      rel(s, selector > 0);
      if (isConstant) {
        IntSharedArray sia = s.arg2boolsharedarray(ce[1], 1);
        element(s, sia, selector, s.arg2BoolVar(ce[2]), s.ann2ipl(ann));
      } else {
        BoolVarArgs iv = s.arg2boolvarargs(ce[1], 1);
        element(s, iv, selector, s.arg2BoolVar(ce[2]), s.ann2ipl(ann));
      }
    }
    void p_array_bool_element_offset(FlatZincSpace& s, const ConExpr& ce,
                                     AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[2]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isBool()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector = s.arg2IntVar(ce[0]);
      int offset = ce[1]->getInt();
      rel(s, selector >= offset);
      if (isConstant) {
        IntSharedArray sia = s.arg2boolsharedarray(ce[2]);
        element(s, sia, selector, -offset, s.arg2BoolVar(ce[3]), s.ann2ipl(ann));
      } else {
        BoolVarArgs iv = s.arg2boolvarargs(ce[2]);
        element(s, iv, selector, -offset, s.arg2BoolVar(ce[3]), s.ann2ipl(ann));
      }
    }
    void p_array_bool_element2d(FlatZincSpace& s, const ConExpr& ce,
                                AST::Node* ann) {
      bool isConstant = true;
      AST::Array* a = ce[2]->getArray();
      for (int i=a->a.size(); i--;) {
        if (!a->a[i]->isBool()) {
          isConstant = false;
          break;
        }
      }
      IntVar selector0 = s.arg2IntVar(ce[0]);
      IntVar selector1 = s.arg2IntVar(ce[1]);
      IntSet idxset0 = s.arg2intset(ce[3]);
      IntSet idxset1 = s.arg2intset(ce[4]);

      int w = idxset1.size();
      int s1off = idxset1.min();
      int h = idxset0.size();
      int s0off = idxset0.min();

      if (isConstant) {
        IntSharedArray sia = s.arg2boolsharedarray(ce[2], 0);
        element(s, sia, selector1, -s1off, w, selector0, -s0off, h, s.arg2BoolVar(ce[5]), s.ann2ipl(ann));
      } else {
        BoolVarArgs iv = s.arg2boolvarargs(ce[2], 0);
        element(s, iv, selector1, -s1off, w, selector0, -s0off, h, s.arg2BoolVar(ce[5]), s.ann2ipl(ann));
      }
    }

    /* coercion constraints */
    void p_bool2int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVar x0 = s.arg2BoolVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      if (ce[0]->isBoolVar() && ce[1]->isIntVar()) {
        s.aliasBool2Int(ce[1]->getIntVar(), ce[0]->getBoolVar());
      }
      channel(s, x0, x1, s.ann2ipl(ann));
    }

    void p_int_in(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
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
    }
  }

  namespace RegistryDetail {

    void post_int_in_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      IntSet d = s.arg2intset(ce[1]);
      if (ce[0]->isBoolVar()) {
        IntSetRanges dr(d);
        Iter::Ranges::Singleton sr(0,1);
        Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
        IntSet d01(i);
        if (d01.size() == 0) {
          rel(s, s.arg2BoolVar(ce[2]) == 0);
        } else if (d01.max() == 0) {
          rel(s, s.arg2BoolVar(ce[2]) == !s.arg2BoolVar(ce[0]));
        } else if (d01.min() == 1) {
          rel(s, s.arg2BoolVar(ce[2]) == s.arg2BoolVar(ce[0]));
        } else {
          rel(s, s.arg2BoolVar(ce[2]) == 1);
        }
      } else {
        dom(s, s.arg2IntVar(ce[0]), d, s.arg2BoolVar(ce[2]));
      }
    }
    void post_int_in_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node *) {
      IntSet d = s.arg2intset(ce[1]);
      if (ce[0]->isBoolVar()) {
        IntSetRanges dr(d);
        Iter::Ranges::Singleton sr(0,1);
        Iter::Ranges::Inter<IntSetRanges,Iter::Ranges::Singleton> i(dr,sr);
        IntSet d01(i);
        if (d01.size() == 0) {
          rel(s, s.arg2BoolVar(ce[2]) == 0);
        } else if (d01.max() == 0) {
          rel(s, s.arg2BoolVar(ce[2]) >> !s.arg2BoolVar(ce[0]));
        } else if (d01.min() == 1) {
          rel(s, s.arg2BoolVar(ce[2]) >> s.arg2BoolVar(ce[0]));
        }
      } else {
        dom(s, s.arg2IntVar(ce[0]), d, Reify(s.arg2BoolVar(ce[2]),RM_IMP));
      }
    }

  }

  namespace {

    /* constraints from the standard library */

    void p_abs(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      IntVar x1 = s.arg2IntVar(ce[1]);
      abs(s, x0, x1, s.ann2ipl(ann));
    }

    void p_array_int_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv0 = s.arg2intvarargs(ce[0]);
      IntVarArgs iv1 = s.arg2intvarargs(ce[1]);
      rel(s, iv0, IRT_LE, iv1, s.ann2ipl(ann));
    }

    void p_array_int_lq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv0 = s.arg2intvarargs(ce[0]);
      IntVarArgs iv1 = s.arg2intvarargs(ce[1]);
      rel(s, iv0, IRT_LQ, iv1, s.ann2ipl(ann));
    }

    void p_array_bool_lt(FlatZincSpace& s, const ConExpr& ce,
                         AST::Node* ann) {
      BoolVarArgs bv0 = s.arg2boolvarargs(ce[0]);
      BoolVarArgs bv1 = s.arg2boolvarargs(ce[1]);
      rel(s, bv0, IRT_LE, bv1, s.ann2ipl(ann));
    }

    void p_array_bool_lq(FlatZincSpace& s, const ConExpr& ce,
                         AST::Node* ann) {
      BoolVarArgs bv0 = s.arg2boolvarargs(ce[0]);
      BoolVarArgs bv1 = s.arg2boolvarargs(ce[1]);
      rel(s, bv0, IRT_LQ, bv1, s.ann2ipl(ann));
    }

    void p_count(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[0]);
      if (!ce[1]->isIntVar()) {
        if (!ce[2]->isIntVar()) {
          count(s, iv, ce[1]->getInt(), IRT_EQ, ce[2]->getInt(),
                s.ann2ipl(ann));
        } else {
          count(s, iv, ce[1]->getInt(), IRT_EQ, s.arg2IntVar(ce[2]),
                s.ann2ipl(ann));
        }
      } else if (!ce[2]->isIntVar()) {
        count(s, iv, s.arg2IntVar(ce[1]), IRT_EQ, ce[2]->getInt(),
              s.ann2ipl(ann));
      } else {
        count(s, iv, s.arg2IntVar(ce[1]), IRT_EQ, s.arg2IntVar(ce[2]),
              s.ann2ipl(ann));
      }
    }

    void p_count_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[0]);
      IntVar x = s.arg2IntVar(ce[1]);
      IntVar y = s.arg2IntVar(ce[2]);
      BoolVar b = s.arg2BoolVar(ce[3]);
      IntVar c(s,0,Int::Limits::max);
      count(s,iv,x,IRT_EQ,c,s.ann2ipl(ann));
      rel(s, b == (c==y));
    }
    void p_count_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[0]);
      IntVar x = s.arg2IntVar(ce[1]);
      IntVar y = s.arg2IntVar(ce[2]);
      BoolVar b = s.arg2BoolVar(ce[3]);
      IntVar c(s,0,Int::Limits::max);
      count(s,iv,x,IRT_EQ,c,s.ann2ipl(ann));
      rel(s, b >> (c==y));
    }

    void count_rel(IntRelType irt,
                   FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[1]);
      count(s, iv, ce[2]->getInt(), irt, ce[0]->getInt(), s.ann2ipl(ann));
    }

    void p_at_most(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      count_rel(IRT_LQ, s, ce, ann);
    }

    void p_at_least(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      count_rel(IRT_GQ, s, ce, ann);
    }

    void p_bin_packing_load(FlatZincSpace& s, const ConExpr& ce,
                            AST::Node* ann) {
      int minIdx = ce[3]->getInt();
      IntVarArgs load = s.arg2intvarargs(ce[0]);
      IntVarArgs l;
      IntVarArgs bin = s.arg2intvarargs(ce[1]);
      for (int i=bin.size(); i--;)
        rel(s, bin[i] >= minIdx);
      if (minIdx > 0) {
        for (int i=minIdx; i--;)
          l << IntVar(s,0,0);
      } else if (minIdx < 0) {
        IntVarArgs bin2(bin.size());
        for (int i=bin.size(); i--;)
          bin2[i] = expr(s, bin[i]-minIdx, s.ann2ipl(ann));
        bin = bin2;
      }
      l << load;
      IntArgs sizes = s.arg2intargs(ce[2]);

      IntVarArgs allvars = l + bin;
      unshare(s, allvars);
      binpacking(s, allvars.slice(0,1,l.size()), allvars.slice(l.size(),1,bin.size()),
                 sizes, s.ann2ipl(ann));
    }

    void p_global_cardinality(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node* ann) {
      IntVarArgs iv0 = s.arg2intvarargs(ce[0]);
      IntArgs cover = s.arg2intargs(ce[1]);
      IntVarArgs iv1 = s.arg2intvarargs(ce[2]);

      Region re;
      IntSet cover_s(cover);
      IntSetRanges cover_r(cover_s);
      IntVarRanges* iv0_ri = re.alloc<IntVarRanges>(iv0.size());
      for (int i=iv0.size(); i--;)
        iv0_ri[i] = IntVarRanges(iv0[i]);
      Iter::Ranges::NaryUnion iv0_r(re,iv0_ri,iv0.size());
      Iter::Ranges::Diff<Iter::Ranges::NaryUnion,IntSetRanges>
        extra_r(iv0_r,cover_r);
      Iter::Ranges::ToValues<Iter::Ranges::Diff<
        Iter::Ranges::NaryUnion,IntSetRanges> > extra(extra_r);
      for (; extra(); ++extra) {
        cover << extra.val();
        iv1 << IntVar(s,0,iv0.size());
      }
      IntPropLevel ipl = s.ann2ipl(ann);
      if (ipl==IPL_DEF)
        ipl=IPL_BND;
      if (ipl==IPL_DOM) {
        IntVarArgs allvars = iv0+iv1;
        unshare(s, allvars);
        count(s, allvars.slice(0,1,iv0.size()),
                 allvars.slice(iv0.size(),1,iv1.size()),
                 cover, ipl);
      } else {
        unshare(s, iv0);
        count(s, iv0, iv1, cover, ipl);
      }
    }

    void p_global_cardinality_closed(FlatZincSpace& s, const ConExpr& ce,
                                     AST::Node* ann) {
      IntVarArgs iv0 = s.arg2intvarargs(ce[0]);
      IntArgs cover = s.arg2intargs(ce[1]);
      IntVarArgs iv1 = s.arg2intvarargs(ce[2]);
      IntPropLevel ipl = s.ann2ipl(ann);
      if (ipl==IPL_DEF)
        ipl=IPL_BND;
      if (ipl==IPL_DOM) {
        IntVarArgs allvars = iv0+iv1;
        unshare(s, allvars);
        count(s, allvars.slice(0,1,iv0.size()),
                 allvars.slice(iv0.size(),1,iv1.size()),
                 cover, ipl);
      } else {
        unshare(s, iv0);
        count(s, iv0, iv1, cover, ipl);
      }
    }

    void p_global_cardinality_low_up(FlatZincSpace& s, const ConExpr& ce,
                                     AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs cover = s.arg2intargs(ce[1]);

      IntArgs lbound = s.arg2intargs(ce[2]);
      IntArgs ubound = s.arg2intargs(ce[3]);
      IntSetArgs y(cover.size());
      for (int i=cover.size(); i--;)
        y[i] = IntSet(lbound[i],ubound[i]);

      IntSet cover_s(cover);
      Region re;
      IntVarRanges* xrs = re.alloc<IntVarRanges>(x.size());
      for (int i=x.size(); i--;)
        xrs[i].init(x[i]);
      Iter::Ranges::NaryUnion u(re, xrs, x.size());
      Iter::Ranges::ToValues<Iter::Ranges::NaryUnion> uv(u);
      for (; uv(); ++uv) {
        if (!cover_s.in(uv.val())) {
          cover << uv.val();
          y << IntSet(0,x.size());
        }
      }
      unshare(s, x);
      IntPropLevel ipl = s.ann2ipl(ann);
      if (ipl==IPL_DEF)
        ipl=IPL_BND;
      count(s, x, y, cover, ipl);
    }

    void p_global_cardinality_low_up_closed(FlatZincSpace& s,
                                            const ConExpr& ce,
                                            AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs cover = s.arg2intargs(ce[1]);

      IntArgs lbound = s.arg2intargs(ce[2]);
      IntArgs ubound = s.arg2intargs(ce[3]);
      IntSetArgs y(cover.size());
      for (int i=cover.size(); i--;)
        y[i] = IntSet(lbound[i],ubound[i]);
      unshare(s, x);
      IntPropLevel ipl = s.ann2ipl(ann);
      if (ipl==IPL_DEF)
        ipl=IPL_BND;
      count(s, x, y, cover, ipl);
    }

    void p_minimum(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[1]);
      min(s, iv, s.arg2IntVar(ce[0]), s.ann2ipl(ann));
    }

    void p_maximum(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs iv = s.arg2intvarargs(ce[1]);
      max(s, iv, s.arg2IntVar(ce[0]), s.ann2ipl(ann));
    }

    void p_minimum_arg(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x = s.arg2IntVar(ce[2]);
      IntVarArgs iv = x + s.arg2intvarargs(ce[0]);
      unshare(s, iv);
      int offset = ce[1]->getInt();
      argmin(s, iv.slice(1), offset, x, true, s.ann2ipl(ann));
    }

    void p_maximum_arg(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVar x = s.arg2IntVar(ce[2]);
      IntVarArgs iv = x + s.arg2intvarargs(ce[0]);
      unshare(s, iv);
      int offset = ce[1]->getInt();
      argmax(s, iv.slice(1), offset, x, true, s.ann2ipl(ann));
    }

    void p_minimum_arg_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs bv = s.arg2boolvarargs(ce[0]);
      unshare(s, bv);
      int offset = ce[1]->getInt();
      argmin(s, bv, offset, s.arg2IntVar(ce[2]), true, s.ann2ipl(ann));
    }

    void p_maximum_arg_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs bv = s.arg2boolvarargs(ce[0]);
      unshare(s, bv);
      int offset = ce[1]->getInt();
      argmax(s, bv, offset, s.arg2IntVar(ce[2]), true, s.ann2ipl(ann));
    }

    void p_regular(FlatZincSpace& s, IntVarArgs iv, int q, int symbols_min,
                   int symbols_max,  IntArgs d, int q0, AST::SetLit* finals,
                   AST::Node* ann) {
      int symbols = symbols_max - symbols_min + 1;

      int noOfTrans = 0;
      for (int i=0; i<q; i++) {
        for (int j=0; j<symbols; j++) {
          if (d[i*symbols+j] > 0)
            noOfTrans++;
        }
      }

      Region re;
      DFA::Transition* t = re.alloc<DFA::Transition>(noOfTrans+1);
      noOfTrans = 0;
      for (int i=1; i<=q; i++) {
        for (int j=0; j<symbols; j++) {
          if (d[(i-1)*symbols+j] > 0) {
            t[noOfTrans].i_state = i;
            t[noOfTrans].symbol  = symbols_min + j;
            t[noOfTrans].o_state = d[(i-1)*symbols+j];
            noOfTrans++;
          }
        }
      }
      t[noOfTrans].i_state = -1;

      // Final states
      int* f;
      if (finals->interval) {
        f = static_cast<int*>(heap.ralloc(sizeof(int)*(finals->max-finals->min+2)));
        for (int i=finals->min; i<=finals->max; i++)
          f[i-finals->min] = i;
        f[finals->max-finals->min+1] = -1;
      } else {
        f = static_cast<int*>(heap.ralloc(sizeof(int)*(finals->s.size()+1)));
        for (int j=finals->s.size(); j--; )
          f[j] = finals->s[j];
        f[finals->s.size()] = -1;
      }

      DFA dfa(q0,t,f);
      free(f);
      unshare(s, iv);
      extensional(s, iv, s.getSharedDFA(dfa), s.ann2ipl(ann));
    }

    void p_regular(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_regular(s, s.arg2intvarargs(ce[0]), ce[1]->getInt(), 1, ce[2]->getInt(),
                s.arg2intargs(ce[3]), ce[4]->getInt(), ce[5]->getSet(), ann);
    }

    void p_regular_set(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_regular(s, s.arg2intvarargs(ce[0]), ce[1]->getInt(), ce[2]->getInt(),
                ce[3]->getInt(), s.arg2intargs(ce[4]), ce[5]->getInt(),
                ce[6]->getSet(), ann);
    }


    void p_regular_nfa_int(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      int q = ce[1]->getInt(), first_symbol = ce[2]->getInt();
      int symbols = ce[3]->getInt(), initial = ce[5]->getInt();
      IntSetArgs table = s.arg2intsetargs(ce[4]);
      IntSet finals = s.arg2intset(ce[6]);
      if ((q <= 0) || (symbols <= 0) ||
          (static_cast<long long>(q)*symbols != table.size()) ||
          (initial < 1) || (initial > q) ||
          ((finals.size() != 0) && ((finals.min() < 1) || (finals.max() > q))) ||
          (first_symbol < Int::Limits::min) ||
          (static_cast<long long>(first_symbol)+symbols-1 > Int::Limits::max))
        throw FlatZinc::Error("Registry", "invalid NFA dimensions or states");
      std::vector<DFA::Transition> transitions;
      for (int state=0; state<q; state++)
        for (int symbol=0; symbol<symbols; symbol++) {
          const IntSet& targets = table[state*symbols+symbol];
          if ((targets.size() != 0) && ((targets.min() < 1) || (targets.max() > q)))
            throw FlatZinc::Error("Registry", "invalid NFA transition");
          for (IntSetValues target(targets); target(); ++target)
            transitions.push_back(DFA::Transition(state+1,first_symbol+symbol,target.val()));
        }
      transitions.push_back(DFA::Transition(-1,0,0));
      std::vector<int> accepting;
      for (IntSetValues final(finals); final(); ++final)
        accepting.push_back(final.val());
      accepting.push_back(-1);
      DFA automaton = DFA::nfa(initial,transitions.data(),accepting.data());
      unshare(s,x);
      extensional(s,x,s.getSharedDFA(automaton),s.ann2ipl(ann));
    }

    void
    p_sort(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVarArgs y = s.arg2intvarargs(ce[1]);
      IntVarArgs xy(x.size()+y.size());
      for (int i=x.size(); i--;)
        xy[i] = x[i];
      for (int i=y.size(); i--;)
        xy[i+x.size()] = y[i];
      unshare(s, xy);
      for (int i=x.size(); i--;)
        x[i] = xy[i];
      for (int i=y.size(); i--;)
        y[i] = xy[i+x.size()];
      sorted(s, x, y, s.ann2ipl(ann));
    }

    void
    p_inverse_offsets(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      int xoff = ce[1]->getInt();
      IntVarArgs y = s.arg2intvarargs(ce[2]);
      int yoff = ce[3]->getInt();
      IntVarArgs xy = x + y;
      unshare(s, xy);
      channel(s, xy.slice(0, 1, x.size()), xoff, xy.slice(x.size()), yoff, s.ann2ipl(ann));
    }

    void
    p_increasing_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      rel(s,x,IRT_LQ,s.ann2ipl(ann));
    }

    void
    p_increasing_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      rel(s,x,IRT_LQ,s.ann2ipl(ann));
    }

    void
    p_decreasing_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      rel(s,x,IRT_GQ,s.ann2ipl(ann));
    }

    void
    p_decreasing_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      rel(s,x,IRT_GQ,s.ann2ipl(ann));
    }

    void
    p_table_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs tuples = s.arg2intargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,s.ann2ipl(ann));
    }

    void
    p_table_int_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs tuples = s.arg2intargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,Reify(s.arg2BoolVar(ce[2]),RM_EQV),s.ann2ipl(ann));
    }

    void
    p_table_int_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs tuples = s.arg2intargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,Reify(s.arg2BoolVar(ce[2]),RM_IMP),s.ann2ipl(ann));
    }

    void
    p_table_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      IntArgs tuples = s.arg2boolargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,s.ann2ipl(ann));
    }

    void
    p_table_bool_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      IntArgs tuples = s.arg2boolargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,Reify(s.arg2BoolVar(ce[2]),RM_EQV),s.ann2ipl(ann));
    }

    void
    p_table_bool_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      IntArgs tuples = s.arg2boolargs(ce[1]);
      TupleSet ts = s.arg2tupleset(tuples,x.size());
      unshare(s,x);
      extensional(s,x,ts,Reify(s.arg2BoolVar(ce[2]),RM_IMP),s.ann2ipl(ann));
    }

    void p_cumulative_opt(FlatZincSpace& s, const ConExpr& ce,
                      AST::Node* ann) {
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      IntArgs duration = s.arg2intargs(ce[1]);
      IntArgs height = s.arg2intargs(ce[2]);
      BoolVarArgs opt = s.arg2boolvarargs(ce[3]);
      IntVar bound = s.arg2IntVar(ce[4]);
      unshare(s,start);
      cumulative(s,bound,start,duration,height,opt,s.ann2ipl(ann));
    }

    void p_among_seq_int(FlatZincSpace& s, const ConExpr& ce,
                         AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntSet S = s.arg2intset(ce[1]);
      int q = ce[2]->getInt();
      int l = ce[3]->getInt();
      int u = ce[4]->getInt();
      unshare(s, x);
      sequence(s, x, S, q, l, u, s.ann2ipl(ann));
    }

    void p_among_seq_bool(FlatZincSpace& s, const ConExpr& ce,
                          AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      bool val = ce[1]->getBool();
      int q = ce[2]->getInt();
      int l = ce[3]->getInt();
      int u = ce[4]->getInt();
      IntSet S(val, val);
      unshare(s, x);
      sequence(s, x, S, q, l, u, s.ann2ipl(ann));
    }

    void p_schedule_unary(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs p = s.arg2intargs(ce[1]);
      unshare(s,x);
      unary(s, x, p, s.ann2ipl(ann));
    }

    void p_schedule_unary_optional(FlatZincSpace& s, const ConExpr& ce,
                                   AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs p = s.arg2intargs(ce[1]);
      BoolVarArgs m = s.arg2boolvarargs(ce[2]);
      unshare(s,x);
      unary(s, x, p, m, s.ann2ipl(ann));
    }

    void p_circuit(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      int off = ce[0]->getInt();
      IntVarArgs xv = s.arg2intvarargs(ce[1]);
      unshare(s,xv);
      circuit(s,off,xv,s.ann2ipl(ann));
    }
    void p_circuit_cost_array(FlatZincSpace& s, const ConExpr& ce,
                              AST::Node *ann) {
      IntArgs c = s.arg2intargs(ce[0]);
      IntVarArgs xv = s.arg2intvarargs(ce[1]);
      IntVarArgs yv = s.arg2intvarargs(ce[2]);
      IntVar z = s.arg2IntVar(ce[3]);
      unshare(s,xv);
      circuit(s,c,xv,yv,z,s.ann2ipl(ann));
    }
    void p_circuit_cost(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      IntArgs c = s.arg2intargs(ce[0]);
      IntVarArgs xv = s.arg2intvarargs(ce[1]);
      IntVar z = s.arg2IntVar(ce[2]);
      unshare(s,xv);
      circuit(s,c,xv,z,s.ann2ipl(ann));
    }

    void p_nooverlap(FlatZincSpace& s, const ConExpr& ce, AST::Node *ann) {
      IntVarArgs x0 = s.arg2intvarargs(ce[0]);
      IntVarArgs w = s.arg2intvarargs(ce[1]);
      IntVarArgs y0 = s.arg2intvarargs(ce[2]);
      IntVarArgs h = s.arg2intvarargs(ce[3]);
      if (w.assigned() && h.assigned()) {
        IntArgs iw(w.size());
        for (int i=w.size(); i--;)
          iw[i] = w[i].val();
        IntArgs ih(h.size());
        for (int i=h.size(); i--;)
          ih[i] = h[i].val();
        nooverlap(s,x0,iw,y0,ih,s.ann2ipl(ann));

        int miny = y0[0].min();
        int maxy = y0[0].max();
        int maxdy = ih[0];
        for (int i=1; i<y0.size(); i++) {
          miny = std::min(miny,y0[i].min());
          maxy = std::max(maxy,y0[i].max());
          maxdy = std::max(maxdy,ih[i]);
        }
        int minx = x0[0].min();
        int maxx = x0[0].max();
        int maxdx = iw[0];
        for (int i=1; i<x0.size(); i++) {
          minx = std::min(minx,x0[i].min());
          maxx = std::max(maxx,x0[i].max());
          maxdx = std::max(maxdx,iw[i]);
        }
        if (miny > Int::Limits::min && maxy < Int::Limits::max) {
          cumulative(s,maxdy+maxy-miny,x0,iw,ih);
          cumulative(s,maxdx+maxx-minx,y0,ih,iw);
        }
      } else {
        IntVarArgs x1(x0.size()), y1(y0.size());
        for (int i=x0.size(); i--; )
          x1[i] = expr(s, x0[i] + w[i]);
        for (int i=y0.size(); i--; )
          y1[i] = expr(s, y0[i] + h[i]);
        nooverlap(s,x0,w,x1,y0,h,y1,s.ann2ipl(ann));
      }
    }

    void p_precede(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      int p_s = ce[1]->getInt();
      int p_t = ce[2]->getInt();
      precede(s,x,p_s,p_t,s.ann2ipl(ann));
    }

    void p_nvalue(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[1]);
      if (ce[0]->isIntVar()) {
        IntVar y = s.arg2IntVar(ce[0]);
        nvalues(s,x,IRT_EQ,y,s.ann2ipl(ann));
      } else {
        nvalues(s,x,IRT_EQ,ce[0]->getInt(),s.ann2ipl(ann));
      }
    }

    void p_among(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[1]);
      IntSet v = s.arg2intset(ce[2]);
      if (ce[0]->isIntVar()) {
        IntVar n = s.arg2IntVar(ce[0]);
        unshare(s, x);
        count(s,x,v,IRT_EQ,n,s.ann2ipl(ann));
      } else {
        unshare(s, x);
        count(s,x,v,IRT_EQ,ce[0]->getInt(),s.ann2ipl(ann));
      }
    }

    void p_member_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVar y = s.arg2IntVar(ce[1]);
      member(s,x,y,s.ann2ipl(ann));
    }
    void p_member_int_reif(FlatZincSpace& s, const ConExpr& ce,
                           AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVar y = s.arg2IntVar(ce[1]);
      BoolVar b = s.arg2BoolVar(ce[2]);
      member(s,x,y,b,s.ann2ipl(ann));
    }
    void p_member_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      BoolVar y = s.arg2BoolVar(ce[1]);
      member(s,x,y,s.ann2ipl(ann));
    }
    void p_member_bool_reif(FlatZincSpace& s, const ConExpr& ce,
                            AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      BoolVar y = s.arg2BoolVar(ce[1]);
      member(s,x,y,s.arg2BoolVar(ce[2]),s.ann2ipl(ann));
    }

    /// Read a `blackbox_exec` / `blackbox_dll` source annotation into \a mode,
    /// \a target (the executable/library) and \a args (its argument
    /// list). Supports both the single-argument form (no arguments) and the
    /// `(target, args)` form.
    void blackbox_source(AST::Node* ann, std::string& mode,
                         std::string& target,
                         std::vector<std::string>& args) {
      auto string_arg = [](AST::Node* n, const char* what) {
        if ((n == nullptr) || !n->isString()) {
          throw FlatZinc::Error("Registry",
                                std::string("Malformed blackbox annotation: ") +
                                what + " must be a string.");
        }
        return n->getString();
      };
      AST::Call* c = nullptr;
      bool has_dll = (ann != nullptr) && ann->hasCall("blackbox_dll");
      bool has_exec = (ann != nullptr) && ann->hasCall("blackbox_exec");
      if (has_dll && has_exec) {
        throw FlatZinc::Error(
            "Registry", "Blackbox constraint has multiple execution method annotations.");
      } else if (has_dll) {
        c = ann->getCall("blackbox_dll");
        mode = "dll";
      } else if (has_exec) {
        c = ann->getCall("blackbox_exec");
        mode = "exec";
      } else {
        throw FlatZinc::Error(
            "Registry", "Blackbox constraint is missing a valid annotation specifying "
            "execution method.");
      }
      if ((c == nullptr) || (c->args == nullptr)) {
        throw FlatZinc::Error("Registry",
                              "Malformed blackbox annotation: missing target.");
      }
      // For a single-argument call `args` is the bare argument node; for the
      // `(target, args)` form it is an array of the two arguments.
      if (AST::Array* arr = dynamic_cast<AST::Array*>(c->args)) {
        if (arr->a.size() != 2) {
          throw FlatZinc::Error(
              "Registry", "Malformed blackbox annotation: expected a target string and "
              "an argument array.");
        }
        target = string_arg(arr->a[0], "target");
        if (!arr->a[1]->isArray()) {
          throw FlatZinc::Error(
              "Registry", "Malformed blackbox annotation: argument list must be an array "
              "of strings.");
        }
        AST::Array* al = arr->a[1]->getArray();
        for (unsigned int i = 0; i < al->a.size(); i++) {
          args.push_back(string_arg(al->a[i], "argument"));
        }
      } else {
        target = string_arg(c->args, "target");
      }
    }

    void p_blackbox(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      std::string mode;
      std::string target;
      std::vector<std::string> args;
      blackbox_source(ann, mode, target, args);
      IntVarArgs int_input = s.arg2intvarargs(ce[0]);
      IntVarArgs int_output = s.arg2intvarargs(ce[2]);
#ifdef GECODE_HAS_FLOAT_VARS
      FloatVarArgs float_input = s.arg2floatvarargs(ce[1]);
      FloatVarArgs float_output = s.arg2floatvarargs(ce[3]);
#else
      if (!ce[1]->getArray()->a.empty() || !ce[3]->getArray()->a.empty()) {
        throw FlatZinc::Error("Registry",
        "Blackbox propagator cannot use floating point values when Gecode is compiled without floating point decision variable support.");
      }
#endif
      FlatZinc::blackbox(s, BlackBoxAccess::context(s), int_input, int_output,
#ifdef GECODE_HAS_FLOAT_VARS
float_input, float_output,
#endif
      mode, target, args);
    }

    void p_blackbox_bounds(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      std::string mode;
      std::string target;
      std::vector<std::string> args;
      blackbox_source(ann, mode, target, args);
      IntVarArgs ivar = s.arg2intvarargs(ce[0]);
#ifdef GECODE_HAS_FLOAT_VARS
      FloatVarArgs fvar = s.arg2floatvarargs(ce[1]);
#else
      if (!ce[1]->getArray()->a.empty()) {
        throw FlatZinc::Error("Registry",
        "Blackbox propagator cannot use floating point values when Gecode is compiled without floating point decision variable support.");
      }
#endif
      IntArgs flat_reason = s.arg2intargs(ce[2]);
      std::vector<int> reason(flat_reason.size());
      for (int i = 0; i < flat_reason.size(); i++) {
        reason[i] = flat_reason[i];
      }
      FlatZinc::blackbox_bounds(s, BlackBoxAccess::context(s), ivar,
#ifdef GECODE_HAS_FLOAT_VARS
fvar,
#endif
      mode, target, args, reason);
    }


    // Relation codes are documented in gecode_fzn.mzn.
    IntRelType int_relation(AST::Node* arg) {
      switch (arg->getInt()) {
      case 1: return IRT_EQ;
      case 2: return IRT_NQ;
      case 3: return IRT_LQ;
      case 4: return IRT_LE;
      case 5: return IRT_GQ;
      case 6: return IRT_GR;
      default: throw FlatZinc::Error("Registry", "invalid integer relation");
      }
    }

    void p_inter_distance(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      if (ce[1]->isIntVar()) {
        // The native propagator rejects aliases between x and the distance.
        x << s.arg2IntVar(ce[1]);
        unshare(s, x);
        inter_distance(s, x.slice(0,1,x.size()-1), x[x.size()-1],
                       s.ann2ipl(ann));
      } else {
        if (ce[1]->getInt() < 0) {
          s.fail();
          return;
        }
        unshare(s, x);
        inter_distance(s, x, ce[1]->getInt(), s.ann2ipl(ann));
      }
    }

    void p_distinct_optional(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      BoolVarArgs present = s.arg2boolvarargs(ce[1]);
      if (x.size() != present.size())
        throw FlatZinc::Error("Registry", "optional distinct arrays must have equal lengths");
      int lo = Int::Limits::max, hi = Int::Limits::min, n = 0;
      bool mandatory = true;
      for (int i=0; i<x.size(); i++)
        if (!present[i].zero()) {
          lo = std::min(lo,x[i].min());
          hi = std::max(hi,x[i].max());
          n++;
          mandatory = mandatory && present[i].one();
        }
      if (n < 2)
        return;
      // Native optional distinct needs unused values outside the active domains.
      if ((hi < Int::Limits::max-n) || (lo > Int::Limits::min+n)) {
        unshare(s, x);
        distinct(s, present, x, s.ann2ipl(ann));
      } else if (mandatory) {
        IntVarArgs active;
        for (int i=0; i<x.size(); i++)
          if (present[i].one()) active << x[i];
        unshare(s, active);
        distinct(s, active, s.ann2ipl(ann));
      } else {
        for (int i=0; i<x.size(); i++)
          for (int j=i+1; j<x.size(); j++)
            rel(s, x[i], IRT_NQ, x[j],
                Reify(expr(s, present[i] && present[j]), RM_IMP), s.ann2ipl(ann));
      }
    }
    void p_distinct_except(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      int c = ce[1]->getInt();
      // An excluded value outside the variable limits cannot occur in x.
      if ((c < Int::Limits::min) || (c > Int::Limits::max)) {
        unshare(s, x);
        distinct(s, x, s.ann2ipl(ann));
        return;
      }
      int lo = Int::Limits::max, hi = Int::Limits::min, n = 0;
      for (int i=0; i<x.size(); i++)
        if (!(x[i].assigned() && (x[i].val() == c))) {
          lo = std::min(lo,x[i].min());
          hi = std::max(hi,x[i].max());
          n++;
        }
      if (n < 2)
        return;
      // The native propagator needs unused values outside the active domains.
      if ((hi < Int::Limits::max-n) || (lo > Int::Limits::min+n)) {
        unshare(s, x);
        distinct(s, x, c, s.ann2ipl(ann));
      } else {
        for (int i=0; i<x.size(); i++)
          for (int j=i+1; j<x.size(); j++)
            rel(s, x[i], IRT_NQ, x[j],
                Reify(expr(s, (x[i] != c) && (x[j] != c)), RM_IMP), s.ann2ipl(ann));
      }
    }
    void p_bool_int_channel(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      unshare(s, x);
      channel(s, x, s.arg2IntVar(ce[1]), ce[2]->getInt(), s.ann2ipl(ann));
    }
    void p_sort_permutation(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVarArgs y = s.arg2intvarargs(ce[1]);
      IntVarArgs z = s.arg2intvarargs(ce[2]);
      if ((x.size() != y.size()) || (x.size() != z.size()))
        throw FlatZinc::Error("Registry", "sort arrays must have equal lengths");
      int offset = ce[3]->getInt();
      for (int i=0; i<z.size(); i++)
        z[i] = expr(s, z[i]-offset);
      IntVarArgs xyz;
      xyz << x << y << z;
      unshare(s, xyz);
      sorted(s, xyz.slice(0,1,x.size()), xyz.slice(x.size(),1,y.size()),
             xyz.slice(x.size()+y.size()), s.ann2ipl(ann));
    }

    void p_count_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      count(s, s.arg2intvarargs(ce[0]), s.arg2IntVar(ce[1]),
            int_relation(ce[2]), s.arg2IntVar(ce[3]), s.ann2ipl(ann));
    }
    void p_count_matches(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntArgs y = s.arg2intargs(ce[1]);
      IntRelType irt = int_relation(ce[2]);
      IntVar n = s.arg2IntVar(ce[3]);
      IntPropLevel ipl = s.ann2ipl(ann);
      if (x.size() != y.size())
        throw Int::ArgumentSizeMismatch("Int::count");
      // Native matching counts use offset views for x[i]-y[i].
      bool safe = true;
      for (int i=0; i<x.size(); i++)
        if ((y[i] < Int::Limits::min) || (y[i] > Int::Limits::max) ||
            (static_cast<long long>(x[i].min())-y[i] < Int::Limits::min) ||
            (static_cast<long long>(x[i].max())-y[i] > Int::Limits::max)) {
          safe = false;
          break;
        }
      if (safe) {
        count(s, x, y, irt, n, ipl);
      } else {
        BoolVarArgs matches(x.size());
        for (int i=0; i<x.size(); i++) {
          if (x[i].in(y[i])) {
            matches[i] = BoolVar(s,0,1);
            rel(s, x[i], IRT_EQ, y[i], Reify(matches[i]), ipl);
          } else {
            matches[i] = BoolVar(s,0,0);
          }
        }
        linear(s, matches, irt, n, ipl);
      }
    }
    void p_array_int_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      rel(s, s.arg2intvarargs(ce[0]), IRT_NQ,
          s.arg2intvarargs(ce[1]), s.ann2ipl(ann));
    }
    void p_array_bool_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      rel(s, s.arg2boolvarargs(ce[0]), IRT_NQ,
          s.arg2boolvarargs(ce[1]), s.ann2ipl(ann));
    }
    void p_not_all_equal_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      rel(s, s.arg2intvarargs(ce[0]), IRT_NQ, s.ann2ipl(ann));
    }
    void p_int_ite(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      ite(s, s.arg2BoolVar(ce[0]), s.arg2IntVar(ce[1]), s.arg2IntVar(ce[2]),
          s.arg2IntVar(ce[3]), s.ann2ipl(ann));
    }
    void p_bool_ite(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      ite(s, s.arg2BoolVar(ce[0]), s.arg2BoolVar(ce[1]), s.arg2BoolVar(ce[2]),
          s.arg2BoolVar(ce[3]), s.ann2ipl(ann));
    }
    void p_gcd(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      gcd(s, s.arg2IntVar(ce[0]), s.arg2IntVar(ce[1]), s.arg2IntVar(ce[2]),
          s.ann2ipl(ann));
    }
    template<ReifyMode mode>
    void p_gcd_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      gcd(s, s.arg2IntVar(ce[0]), s.arg2IntVar(ce[1]), s.arg2IntVar(ce[2]),
          Reify(s.arg2BoolVar(ce[3]), mode), s.ann2ipl(ann));
    }
    void p_divides(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      divides(s, s.arg2IntVar(ce[0]), s.arg2IntVar(ce[1]),
              Reify(BoolVar(s,1,1),RM_EQV), s.ann2ipl(ann));
    }
    template<ReifyMode mode>
    void p_divides_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      divides(s, s.arg2IntVar(ce[0]), s.arg2IntVar(ce[1]),
              Reify(s.arg2BoolVar(ce[2]),mode), s.ann2ipl(ann));
    }
    void p_product(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      product(s, s.arg2intvarargs(ce[0]), s.arg2IntVar(ce[1]), s.ann2ipl(ann));
    }
    template<ReifyMode mode>
    void p_product_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      product(s, s.arg2intvarargs(ce[0]), s.arg2IntVar(ce[1]),
              Reify(s.arg2BoolVar(ce[2]),mode), s.ann2ipl(ann));
    }
    template<bool fixed>
    void p_product_mod(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVar y = s.arg2IntVar(ce[2]);
      if (fixed)
        product_mod(s,x,ce[1]->getInt(),y,s.ann2ipl(ann));
      else
        product_mod(s,x,s.arg2IntVar(ce[1]),y,s.ann2ipl(ann));
    }
    template<bool fixed, ReifyMode mode>
    void p_product_mod_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntVar y = s.arg2IntVar(ce[2]);
      Reify r(s.arg2BoolVar(ce[3]),mode);
      if (fixed)
        product_mod(s,x,ce[1]->getInt(),y,r,s.ann2ipl(ann));
      else
        product_mod(s,x,s.arg2IntVar(ce[1]),y,r,s.ann2ipl(ann));
    }
    void p_minimum_distance(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs x = s.arg2intvarargs(ce[0]);
      IntDistance d(ce[1]->getInt(),s.arg2intargs(ce[2]));
      MinDistancePropKind kind = ann && ann->hasAtom("gecode_decomposed_propagation")
        ? MDP_DECOMPOSED : MDP_SINGLE;
      if (ce.size() == 5)
        minimum_distance(s,x,s.arg2IntVar(ce[3]),d,
                         s.arg2intargs(ce[4]),s.ann2ipl(ann),kind);
      else
        minimum_distance(s,x,s.arg2IntVar(ce[3]),d,s.ann2ipl(ann),kind);
    }

    void p_int_nroot(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      nroot(s, s.arg2IntVar(ce[0]), ce[1]->getInt(), s.arg2IntVar(ce[2]),
            s.ann2ipl(ann));
    }

    void p_nooverlap_optional(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      nooverlap(s, s.arg2intvarargs(ce[0]), s.arg2intargs(ce[1]),
                s.arg2intvarargs(ce[2]), s.arg2intargs(ce[3]),
                s.arg2boolvarargs(ce[4]), s.ann2ipl(ann));
    }
    void p_nooverlap_flex(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      nooverlap(s, s.arg2intvarargs(ce[0]), s.arg2intvarargs(ce[1]),
                s.arg2intvarargs(ce[2]), s.arg2intvarargs(ce[3]),
                s.arg2intvarargs(ce[4]), s.arg2intvarargs(ce[5]),
                s.arg2boolvarargs(ce[6]), s.ann2ipl(ann));
    }

    void p_arg_min_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs xy = s.arg2intvarargs(ce[0]);
      xy << s.arg2IntVar(ce[2]);
      unshare(s, xy);
      argmin(s, xy.slice(0,1,xy.size()-1), ce[1]->getInt(),
                xy[xy.size()-1], ce[3]->getBool(), s.ann2ipl(ann));
    }

    void p_arg_max_int(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs xy = s.arg2intvarargs(ce[0]);
      xy << s.arg2IntVar(ce[2]);
      unshare(s, xy);
      argmax(s, xy.slice(0,1,xy.size()-1), ce[1]->getInt(),
                xy[xy.size()-1], ce[3]->getBool(), s.ann2ipl(ann));
    }

    void p_arg_min_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      argmin(s, x, ce[1]->getInt(), s.arg2IntVar(ce[2]),
                ce[3]->getBool(), s.ann2ipl(ann));
    }

    void p_arg_max_bool(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      BoolVarArgs x = s.arg2boolvarargs(ce[0]);
      argmax(s, x, ce[1]->getInt(), s.arg2IntVar(ce[2]),
                ce[3]->getBool(), s.ann2ipl(ann));
    }

    void p_unary_flex(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      unshare(s, start);
      unary(s, start, s.arg2intvarargs(ce[1]), s.arg2intvarargs(ce[2]),
            s.ann2ipl(ann));
    }

    void p_cumulative_flex(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      unshare(s, start);
      cumulative(s, s.arg2IntVar(ce[4]), start,
                 s.arg2intvarargs(ce[1]), s.arg2intvarargs(ce[2]),
                 s.arg2intargs(ce[3]), s.ann2ipl(ann));
    }

    void p_unary_flex_optional(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      unshare(s, start);
      unary(s, start, s.arg2intvarargs(ce[1]), s.arg2intvarargs(ce[2]),
            s.arg2boolvarargs(ce[3]), s.ann2ipl(ann));
    }


    void p_cumulative_flex_optional(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      IntVarArgs start = s.arg2intvarargs(ce[0]);
      unshare(s, start);
      cumulative(s, s.arg2IntVar(ce[4]), start, s.arg2intvarargs(ce[1]),
                 s.arg2intvarargs(ce[2]), s.arg2intargs(ce[3]),
                 s.arg2boolvarargs(ce[5]), s.ann2ipl(ann));
    }

    class BuiltinPosters {
    public:
      BuiltinPosters(void) {
        registry().add("all_different_int", &p_distinct);
        registry().add("all_different_offset", &p_distinctOffset);
        registry().add("gecode_all_different_offset_fzn", &p_distinctOffset);
        registry().add("all_equal_int", &p_all_equal);
        registry().add("int_eq", &p_int_eq);
        registry().add("int_ne", &p_int_ne);
        registry().add("int_ge", &p_int_ge);
        registry().add("int_gt", &p_int_gt);
        registry().add("int_le", &p_int_le);
        registry().add("int_lt", &p_int_lt);
        registry().add("int_eq_reif", &p_int_eq_reif);
        registry().add("int_ne_reif", &p_int_ne_reif);
        registry().add("int_ge_reif", &p_int_ge_reif);
        registry().add("int_gt_reif", &p_int_gt_reif);
        registry().add("int_le_reif", &p_int_le_reif);
        registry().add("int_lt_reif", &p_int_lt_reif);
        registry().add("int_eq_imp", &p_int_eq_imp);
        registry().add("int_ne_imp", &p_int_ne_imp);
        registry().add("int_ge_imp", &p_int_ge_imp);
        registry().add("int_gt_imp", &p_int_gt_imp);
        registry().add("int_le_imp", &p_int_le_imp);
        registry().add("int_lt_imp", &p_int_lt_imp);
        registry().add("int_lin_eq", &p_int_lin_eq);
        registry().add("int_lin_eq_reif", &p_int_lin_eq_reif);
        registry().add("int_lin_eq_imp", &p_int_lin_eq_imp);
        registry().add("int_lin_ne", &p_int_lin_ne);
        registry().add("int_lin_ne_reif", &p_int_lin_ne_reif);
        registry().add("int_lin_ne_imp", &p_int_lin_ne_imp);
        registry().add("int_lin_le", &p_int_lin_le);
        registry().add("int_lin_le_reif", &p_int_lin_le_reif);
        registry().add("int_lin_le_imp", &p_int_lin_le_imp);
        registry().add("int_lin_lt", &p_int_lin_lt);
        registry().add("int_lin_lt_reif", &p_int_lin_lt_reif);
        registry().add("int_lin_lt_imp", &p_int_lin_lt_imp);
        registry().add("int_lin_ge", &p_int_lin_ge);
        registry().add("int_lin_ge_reif", &p_int_lin_ge_reif);
        registry().add("int_lin_ge_imp", &p_int_lin_ge_imp);
        registry().add("int_lin_gt", &p_int_lin_gt);
        registry().add("int_lin_gt_reif", &p_int_lin_gt_reif);
        registry().add("int_lin_gt_imp", &p_int_lin_gt_imp);
        registry().add("int_plus", &p_int_plus);
        registry().add("int_minus", &p_int_minus);
        registry().add("int_times", &p_int_times);
        registry().add("gecode_int_pow", &p_int_pow);
        registry().add("int_div", &p_int_div);
        registry().add("int_mod", &p_int_mod);
        registry().add("int_min", &p_int_min);
        registry().add("int_max", &p_int_max);
        registry().add("int_abs", &p_abs);
        registry().add("int_negate", &p_int_negate);
        registry().add("bool_eq", &p_bool_eq);
        registry().add("bool_eq_reif", &p_bool_eq_reif);
        registry().add("bool_eq_imp", &p_bool_eq_imp);
        registry().add("bool_ne", &p_bool_ne);
        registry().add("bool_ne_reif", &p_bool_ne_reif);
        registry().add("bool_ne_imp", &p_bool_ne_imp);
        registry().add("bool_ge", &p_bool_ge);
        registry().add("bool_ge_reif", &p_bool_ge_reif);
        registry().add("bool_ge_imp", &p_bool_ge_imp);
        registry().add("bool_le", &p_bool_le);
        registry().add("bool_le_reif", &p_bool_le_reif);
        registry().add("bool_le_imp", &p_bool_le_imp);
        registry().add("bool_gt", &p_bool_gt);
        registry().add("bool_gt_reif", &p_bool_gt_reif);
        registry().add("bool_gt_imp", &p_bool_gt_imp);
        registry().add("bool_lt", &p_bool_lt);
        registry().add("bool_lt_reif", &p_bool_lt_reif);
        registry().add("bool_lt_imp", &p_bool_lt_imp);
        registry().add("bool_or", &p_bool_or);
        registry().add("bool_or_imp", &p_bool_or_imp);
        registry().add("bool_and", &p_bool_and);
        registry().add("bool_and_imp", &p_bool_and_imp);
        registry().add("bool_xor", &p_bool_xor);
        registry().add("bool_xor_imp", &p_bool_xor_imp);
        registry().add("array_bool_and", &p_array_bool_and);
        registry().add("array_bool_and_imp", &p_array_bool_and_imp);
        registry().add("array_bool_or", &p_array_bool_or);
        registry().add("array_bool_or_imp", &p_array_bool_or_imp);
        registry().add("array_bool_xor", &p_array_bool_xor);
        registry().add("array_bool_xor_imp", &p_array_bool_xor_imp);
        registry().add("bool_clause", &p_array_bool_clause);
        registry().add("bool_clause_reif", &p_array_bool_clause_reif);
        registry().add("bool_clause_imp", &p_array_bool_clause_imp);
        registry().add("bool_left_imp", &p_bool_l_imp);
        registry().add("bool_right_imp", &p_bool_r_imp);
        registry().add("bool_not", &p_bool_not);
        registry().add("array_int_element", &p_array_int_element);
        registry().add("array_var_int_element", &p_array_int_element);
        registry().add("gecode_int_element", &p_array_int_element_offset);
        registry().add("gecode_var_int_element", &p_array_int_element_offset);
        registry().add("gecode_int_element2d", &p_array_int_element2d);
        registry().add("array_bool_element", &p_array_bool_element);
        registry().add("array_var_bool_element", &p_array_bool_element);
        registry().add("gecode_bool_element", &p_array_bool_element_offset);
        registry().add("gecode_var_bool_element", &p_array_bool_element_offset);
        registry().add("gecode_bool_element2d", &p_array_bool_element2d);
        registry().add("bool2int", &p_bool2int);
        registry().add("int_in", &p_int_in);
        registry().add("int_in_reif", &RegistryDetail::post_int_in_reif);
        registry().add("int_in_imp", &RegistryDetail::post_int_in_imp);
#ifndef GECODE_HAS_SET_VARS
        registry().add("set_in", &p_int_in);
        registry().add("set_in_reif", &RegistryDetail::post_int_in_reif);
        registry().add("set_in_imp", &RegistryDetail::post_int_in_imp);
#endif

        registry().add("array_int_lt", &p_array_int_lt);
        registry().add("array_int_lq", &p_array_int_lq);
        registry().add("array_bool_lt", &p_array_bool_lt);
        registry().add("array_bool_lq", &p_array_bool_lq);
        registry().add("count", &p_count);
        registry().add("count_reif", &p_count_reif);
        registry().add("count_imp", &p_count_imp);
        registry().add("count_eq", &p_count);
        registry().add("count_eq_reif", &p_count_reif);
        registry().add("count_eq_imp", &p_count_imp);
        registry().add("at_least_int", &p_at_least);
        registry().add("at_most_int", &p_at_most);
        registry().add("gecode_bin_packing_load", &p_bin_packing_load);
        registry().add("gecode_global_cardinality", &p_global_cardinality);
        registry().add("gecode_global_cardinality_closed",
          &p_global_cardinality_closed);
        registry().add("global_cardinality_low_up",
          &p_global_cardinality_low_up);
        registry().add("global_cardinality_low_up_closed",
          &p_global_cardinality_low_up_closed);
        registry().add("array_int_minimum", &p_minimum);
        registry().add("array_int_maximum", &p_maximum);
        registry().add("gecode_minimum_arg_int_offset", &p_minimum_arg);
        registry().add("gecode_maximum_arg_int_offset", &p_maximum_arg);
        registry().add("gecode_minimum_arg_bool_offset", &p_minimum_arg_bool);
        registry().add("gecode_maximum_arg_bool_offset", &p_maximum_arg_bool);
        registry().add("array_int_maximum", &p_maximum);
        registry().add("gecode_regular", &p_regular);
        registry().add("gecode_regular_set", &p_regular_set);
        registry().add("sort", &p_sort);
        registry().add("inverse_offsets", &p_inverse_offsets);
        registry().add("increasing_int", &p_increasing_int);
        registry().add("increasing_bool", &p_increasing_bool);
        registry().add("decreasing_int", &p_decreasing_int);
        registry().add("decreasing_bool", &p_decreasing_bool);
        registry().add("gecode_table_int", &p_table_int);
        registry().add("gecode_table_int_reif", &p_table_int_reif);
        registry().add("gecode_table_int_imp", &p_table_int_imp);
        registry().add("gecode_table_bool", &p_table_bool);
        registry().add("gecode_table_bool_reif", &p_table_bool_reif);
        registry().add("gecode_table_bool_imp", &p_table_bool_imp);
        registry().add("cumulatives", &RegistryDetail::post_cumulatives);
        registry().add("gecode_among_seq_int", &p_among_seq_int);
        registry().add("gecode_among_seq_bool", &p_among_seq_bool);

        registry().add("bool_lin_eq", &p_bool_lin_eq);
        registry().add("bool_lin_ne", &p_bool_lin_ne);
        registry().add("bool_lin_le", &p_bool_lin_le);
        registry().add("bool_lin_lt", &p_bool_lin_lt);
        registry().add("bool_lin_ge", &p_bool_lin_ge);
        registry().add("bool_lin_gt", &p_bool_lin_gt);

        registry().add("bool_lin_eq_reif", &p_bool_lin_eq_reif);
        registry().add("bool_lin_eq_imp", &p_bool_lin_eq_imp);
        registry().add("bool_lin_ne_reif", &p_bool_lin_ne_reif);
        registry().add("bool_lin_ne_imp", &p_bool_lin_ne_imp);
        registry().add("bool_lin_le_reif", &p_bool_lin_le_reif);
        registry().add("bool_lin_le_imp", &p_bool_lin_le_imp);
        registry().add("bool_lin_lt_reif", &p_bool_lin_lt_reif);
        registry().add("bool_lin_lt_imp", &p_bool_lin_lt_imp);
        registry().add("bool_lin_ge_reif", &p_bool_lin_ge_reif);
        registry().add("bool_lin_ge_imp", &p_bool_lin_ge_imp);
        registry().add("bool_lin_gt_reif", &p_bool_lin_gt_reif);
        registry().add("bool_lin_gt_imp", &p_bool_lin_gt_imp);

        registry().add("gecode_schedule_unary", &p_schedule_unary);
        registry().add("gecode_schedule_unary_optional", &p_schedule_unary_optional);
        registry().add("gecode_schedule_cumulative_optional", &p_cumulative_opt);

        registry().add("gecode_circuit", &p_circuit);
        registry().add("gecode_circuit_cost_array", &p_circuit_cost_array);
        registry().add("gecode_circuit_cost", &p_circuit_cost);
        registry().add("gecode_nooverlap", &p_nooverlap);
        registry().add("gecode_nooverlap_fzn", &p_nooverlap);
        registry().add("gecode_precede", &p_precede);
        registry().add("nvalue",&p_nvalue);
        registry().add("among",&p_among);
        registry().add("member_int",&p_member_int);
        registry().add("gecode_member_int_reif",&p_member_int_reif);
        registry().add("member_bool",&p_member_bool);
        registry().add("gecode_member_bool_reif",&p_member_bool_reif);

        registry().add("gecode_unary_flex", &p_unary_flex);
        registry().add("gecode_cumulative_flex", &p_cumulative_flex);
        registry().add("gecode_cumulatives_flex", &RegistryDetail::post_cumulatives);
        registry().add("gecode_unary_flex_optional", &p_unary_flex_optional);
        registry().add("gecode_cumulative_flex_optional", &p_cumulative_flex_optional);
        registry().add("gecode_array_int_ne", &p_array_int_ne);
        registry().add("gecode_not_all_equal_int", &p_not_all_equal_int);
        registry().add("gecode_array_bool_ne", &p_array_bool_ne);
        registry().add("gecode_inter_distance", &p_inter_distance);
        registry().add("gecode_inter_distance_fzn", &p_inter_distance);
        registry().add("gecode_bool_int_channel", &p_bool_int_channel);
        registry().add("gecode_sort_permutation", &p_sort_permutation);
        registry().add("gecode_count_int_fzn", &p_count_int);
        registry().add("gecode_count_matches_fzn", &p_count_matches);
        registry().add("gecode_int_ite", &p_int_ite);
        registry().add("gecode_bool_ite", &p_bool_ite);
        registry().add("gecode_gcd_int", &p_gcd);
        registry().add("gecode_gcd_int_reif", &p_gcd_reif<RM_EQV>);
        registry().add("gecode_gcd_int_imp", &p_gcd_reif<RM_IMP>);
        registry().add("gecode_divides_int", &p_divides);
        registry().add("gecode_divides_int_reif", &p_divides_reif<RM_EQV>);
        registry().add("gecode_divides_int_imp", &p_divides_reif<RM_IMP>);
        registry().add("gecode_product_int", &p_product);
        registry().add("gecode_product_int_reif", &p_product_reif<RM_EQV>);
        registry().add("gecode_product_int_imp", &p_product_reif<RM_IMP>);
        registry().add("gecode_product_mod_fixed_fzn", &p_product_mod<true>);
        registry().add("gecode_product_mod_fixed_fzn_reif", &p_product_mod_reif<true,RM_EQV>);
        registry().add("gecode_product_mod_fixed_fzn_imp", &p_product_mod_reif<true,RM_IMP>);
        registry().add("gecode_product_mod_var_fzn", &p_product_mod<false>);
        registry().add("gecode_product_mod_var_fzn_reif", &p_product_mod_reif<false,RM_EQV>);
        registry().add("gecode_product_mod_var_fzn_imp", &p_product_mod_reif<false,RM_IMP>);
        registry().add("gecode_minimum_distance_fzn", &p_minimum_distance);
        registry().add("gecode_minimum_distance_required_fzn", &p_minimum_distance);
        registry().add("gecode_int_nroot", &p_int_nroot);
        registry().add("gecode_nooverlap_optional", &p_nooverlap_optional);
        registry().add("gecode_all_different_optional", &p_distinct_optional);
        registry().add("gecode_all_different_except", &p_distinct_except);
        registry().add("gecode_regular_nfa_int_fzn", &p_regular_nfa_int);
        registry().add("gecode_nooverlap_flex_optional", &p_nooverlap_flex);
        registry().add("gecode_arg_min_int", &p_arg_min_int);
        registry().add("gecode_arg_max_int", &p_arg_max_int);
        registry().add("gecode_arg_min_bool", &p_arg_min_bool);
        registry().add("gecode_arg_max_bool", &p_arg_max_bool);

        registry().add("gecode_blackbox", &p_blackbox);
        registry().add("gecode_blackbox_bounds", &p_blackbox_bounds);

#ifdef GECODE_HAS_SET_VARS
        RegistryDetail::register_set(registry());
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        RegistryDetail::register_float(registry());
#endif
      }
    };
    BuiltinPosters builtin_posters;


  }
}}

// STATISTICS: flatzinc-any
