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
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

namespace Gecode { namespace FlatZinc {

#ifdef GECODE_HAS_FLOAT_VARS
  namespace {

    void p_int2float(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      IntVar x0 = s.arg2IntVar(ce[0]);
      FloatVar x1 = s.arg2FloatVar(ce[1]);
      channel(s, x0, x1);
    }

    void p_float_lin_cmp(FlatZincSpace& s, FloatRelType frt,
                         const ConExpr& ce, AST::Node*) {
      FloatValArgs fa = s.arg2floatargs(ce[0]);
      FloatVarArgs fv = s.arg2floatvarargs(ce[1]);
      linear(s, fa, fv, frt, ce[2]->getFloat());
    }
    void p_float_lin_cmp_reif(FlatZincSpace& s, FloatRelType frt,
                              const ConExpr& ce, AST::Node*) {
      FloatValArgs fa = s.arg2floatargs(ce[0]);
      FloatVarArgs fv = s.arg2floatvarargs(ce[1]);
      linear(s, fa, fv, frt, ce[2]->getFloat(), s.arg2BoolVar(ce[3]));
    }
    void p_float_lin_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_float_lin_cmp(s,FRT_EQ,ce,ann);
    }
    void p_float_lin_eq_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      p_float_lin_cmp_reif(s,FRT_EQ,ce,ann);
    }
    void p_float_lin_le(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_float_lin_cmp(s,FRT_LQ,ce,ann);
    }
    void p_float_lin_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
      p_float_lin_cmp(s,FRT_LE,ce,ann);
    }
    void p_float_lin_le_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      p_float_lin_cmp_reif(s,FRT_LQ,ce,ann);
    }
    void p_float_lin_lt_reif(FlatZincSpace& s, const ConExpr& ce,
                             AST::Node* ann) {
      p_float_lin_cmp_reif(s,FRT_LE,ce,ann);
    }

    void p_float_times(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      FloatVar z = s.arg2FloatVar(ce[2]);
      mult(s,x,y,z);
    }

    void p_float_div(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      FloatVar z = s.arg2FloatVar(ce[2]);
      div(s,x,y,z);
    }

    void p_float_plus(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      FloatVar z = s.arg2FloatVar(ce[2]);
      rel(s,x+y==z);
    }

    void p_float_sqrt(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      sqrt(s,x,y);
    }

    void p_float_abs(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      abs(s,x,y);
    }

    void p_float_eq(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      rel(s,x,FRT_EQ,y);
    }
    void p_float_eq_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      BoolVar  b = s.arg2BoolVar(ce[2]);
      rel(s,x,FRT_EQ,y,b);
    }
    void p_float_le(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      rel(s,x,FRT_LQ,y);
    }
    void p_float_le_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      BoolVar  b = s.arg2BoolVar(ce[2]);
      rel(s,x,FRT_LQ,y,b);
    }
    void p_float_max(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      FloatVar z = s.arg2FloatVar(ce[2]);
      max(s,x,y,z);
    }
    void p_float_min(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      FloatVar z = s.arg2FloatVar(ce[2]);
      min(s,x,y,z);
    }
    void p_float_lt(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      rel(s, x, FRT_LQ, y);
      rel(s, x, FRT_EQ, y, BoolVar(s,0,0));
    }

    void p_float_lt_reif(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      BoolVar b = s.arg2BoolVar(ce[2]);
      BoolVar b0(s,0,1);
      BoolVar b1(s,0,1);
      rel(s, b == (b0 && !b1));
      rel(s, x, FRT_LQ, y, b0);
      rel(s, x, FRT_EQ, y, b1);
    }

    void p_float_ne(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      rel(s, x, FRT_EQ, y, BoolVar(s,0,0));
    }

#ifdef GECODE_HAS_MPFR
#define P_FLOAT_OP(Op) \
    void p_float_ ## Op (FlatZincSpace& s, const ConExpr& ce, AST::Node*) {\
      FloatVar x = s.arg2FloatVar(ce[0]);\
      FloatVar y = s.arg2FloatVar(ce[1]);\
      Op(s,x,y);\
    }
    P_FLOAT_OP(acos)
    P_FLOAT_OP(asin)
    P_FLOAT_OP(atan)
    P_FLOAT_OP(cos)
    P_FLOAT_OP(exp)
    P_FLOAT_OP(sin)
    P_FLOAT_OP(tan)
    // P_FLOAT_OP(sinh)
    // P_FLOAT_OP(tanh)
    // P_FLOAT_OP(cosh)
#undef P_FLOAT_OP

    void p_float_ln(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      log(s,x,y);
    }
    void p_float_log10(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      log(s,10.0,x,y);
    }
    void p_float_log2(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatVar x = s.arg2FloatVar(ce[0]);
      FloatVar y = s.arg2FloatVar(ce[1]);
      log(s,2.0,x,y);
    }

#endif


    void p_float_ite(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      ite(s, s.arg2BoolVar(ce[0]), s.arg2FloatVar(ce[1]), s.arg2FloatVar(ce[2]),
          s.arg2FloatVar(ce[3]));
    }
    void p_float_pow(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      pow(s, s.arg2FloatVar(ce[0]), ce[1]->getInt(), s.arg2FloatVar(ce[2]));
    }
    void p_float_array_min(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      min(s, s.arg2floatvarargs(ce[0]), s.arg2FloatVar(ce[1]));
    }
    void p_float_array_max(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      max(s, s.arg2floatvarargs(ce[0]), s.arg2FloatVar(ce[1]));
    }
#ifdef GECODE_HAS_MPFR
    void p_float_pow_base(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      pow(s, ce[0]->getFloat(), s.arg2FloatVar(ce[1]), s.arg2FloatVar(ce[2]));
    }
#endif

    void p_float_rel_imp(FlatZincSpace& s, const ConExpr& ce, AST::Node*) {
      FloatRelType r;
      switch (ce[1]->getInt()) {
      case 1: r = FRT_EQ; break;
      case 2: r = FRT_NQ; break;
      case 3: r = FRT_LQ; break;
      case 4: r = FRT_LE; break;
      case 5: r = FRT_GQ; break;
      case 6: r = FRT_GR; break;
      default: throw FlatZinc::Error("Registry", "invalid float relation");
      }
      rel(s, s.arg2FloatVar(ce[0]), r, s.arg2FloatVar(ce[2]),
          Reify(s.arg2BoolVar(ce[3]), RM_IMP));
    }

  }

  namespace RegistryDetail {

    void register_float(Registry& r) {
      r.add("int2float",&p_int2float);

      r.add("gecode_float_rel_imp_fzn", &p_float_rel_imp);
      r.add("gecode_float_ite", &p_float_ite);
      r.add("gecode_float_pow", &p_float_pow);
      r.add("gecode_float_array_min", &p_float_array_min);
      r.add("gecode_float_array_max", &p_float_array_max);

      r.add("float_abs",&p_float_abs);
      r.add("float_sqrt",&p_float_sqrt);
      r.add("float_eq",&p_float_eq);
      r.add("float_eq_reif",&p_float_eq_reif);
      r.add("float_le",&p_float_le);
      r.add("float_le_reif",&p_float_le_reif);
      r.add("float_lt",&p_float_lt);
      r.add("float_lt_reif",&p_float_lt_reif);
      r.add("float_ne",&p_float_ne);
      r.add("float_times",&p_float_times);
      r.add("float_div",&p_float_div);
      r.add("float_plus",&p_float_plus);
      r.add("float_max",&p_float_max);
      r.add("float_min",&p_float_min);

      r.add("float_lin_eq",&p_float_lin_eq);
      r.add("float_lin_eq_reif",&p_float_lin_eq_reif);
      r.add("float_lin_le",&p_float_lin_le);
      r.add("float_lin_lt",&p_float_lin_lt);
      r.add("float_lin_le_reif",&p_float_lin_le_reif);
      r.add("float_lin_lt_reif",&p_float_lin_lt_reif);

#ifdef GECODE_HAS_MPFR
      r.add("gecode_float_pow_base", &p_float_pow_base);
      r.add("float_acos",&p_float_acos);
      r.add("float_asin",&p_float_asin);
      r.add("float_atan",&p_float_atan);
      r.add("float_cos",&p_float_cos);
      // r.add("float_cosh",&p_float_cosh);
      r.add("float_exp",&p_float_exp);
      r.add("float_ln",&p_float_ln);
      r.add("float_log10",&p_float_log10);
      r.add("float_log2",&p_float_log2);
      r.add("float_sin",&p_float_sin);
      // r.add("float_sinh",&p_float_sinh);
      r.add("float_tan",&p_float_tan);
      // r.add("float_tanh",&p_float_tanh);
#endif
    }

  }
#endif

}}

// STATISTICS: flatzinc-any
