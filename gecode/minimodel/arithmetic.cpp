/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <gecode/minimodel.hh>

namespace Gecode {

  namespace MiniModel {
    /// Non-linear arithmetic expressions
    class GECODE_MINIMODEL_EXPORT ArithNonLinExpr : public NonLinExpr {
    public:
      /// The expression type
      enum ArithNonLinExprType {
        ANLE_ABS,   ///< Absolute value expression
        ANLE_MIN_2, ///< Binary minimum expression
        ANLE_MIN_N, ///< N-ary minimum expression
        ANLE_MAX_2, ///< Binary minimum expression
        ANLE_MAX_N, ///< N-ary minimum expression
        ANLE_MULT,  ///< Multiplication expression
        ANLE_DIV,   ///< Division expression
        ANLE_MOD,   ///< Modulo expression
        ANLE_SQR,   ///< Square expression
        ANLE_SQRT   ///< Square root expression
      } t;
      /// The first expression
      LinExpr e0;
      /// The second expression
      LinExpr e1;
      /// Variable array
      IntVar* a;
      /// Size of variable array
      int n;
      /// Constructor
      ArithNonLinExpr(void) : a(NULL), n(0) {}
      /// Destructor
      ~ArithNonLinExpr(void) { heap.free<IntVar>(a,n); }
      /// Post expression
      virtual IntVar post(Home home, IntConLevel icl) const {
        IntVar y;
        switch (t) {
        case ANLE_ABS:
          {
            IntVar x = e0.post(home, icl);
            if (x.min() >= 0)
              y = x;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              abs(home, x, y, icl);
            }
          }
          break;
        case ANLE_MIN_2:
          {
            IntVar x0 = e0.post(home, icl);
            IntVar x1 = e1.post(home, icl);
            if (x0.max() <= x1.min())
              y = x0;
            else if (x1.max() <= x0.min())
              y = x1;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              min(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_MAX_2:
          {
            IntVar x0 = e0.post(home, icl);
            IntVar x1 = e1.post(home, icl);
            if (x0.max() <= x1.min())
              y = x1;
            else if (x1.max() <= x0.min())
              y = x0;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              max(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_MULT:
          {
            IntVar x0 = e0.post(home, icl);
            IntVar x1 = e1.post(home, icl);
            if (x0.assigned() && (x0.val() == 0))
              y = x0;
            else if (x0.assigned() && (x0.val() == 1))
              y = x1;
            else if (x1.assigned() && (x1.val() == 0))
              y = x1;
            else if (x1.assigned() && (x1.val() == 1))
              y = x0;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              mult(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_DIV:
          {
            IntVar x0 = e0.post(home, icl);
            IntVar x1 = e1.post(home, icl);
            if (x1.assigned() && (x1.val() == 1))
              y = x0;
            else if (x0.assigned() && (x0.val() == 0))
              y = x0;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              div(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_MOD:
          {
            IntVar x0 = e0.post(home, icl);
            IntVar x1 = e1.post(home, icl);
            y = IntVar(home, Int::Limits::min, Int::Limits::max);
            div(home, x0, x1, y, icl);
          }
          break;
        case ANLE_SQR:
          {
            IntVar x = e0.post(home, icl);
            if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
              y = x;
            else {
              y = IntVar(home, 0, Int::Limits::max);
              sqr(home, x, y, icl);
            }
          }
          break;
        case ANLE_SQRT:
          {
            IntVar x = e0.post(home, icl);
            if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
              y = x;
            else {
              y = IntVar(home, 0, Int::Limits::max);
              sqrt(home, x, y, icl);
            }
          }
          break;
        default:
          GECODE_NEVER;
        }
        return y;
      }
    };
  }

  LinExpr
  abs(const LinExpr& e) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_ABS;
    ae->e0 = e;
    return LinExpr(ae);
  }

  LinExpr
  min(const LinExpr& e0, const LinExpr& e1) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MIN_2;
    ae->e0 = e0;
    ae->e1 = e1;
    return LinExpr(ae);
  }

  LinExpr
  max(const LinExpr& e0, const LinExpr& e1) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MAX_2;
    ae->e0 = e0;
    ae->e1 = e1;
    return LinExpr(ae);
  }

  LinExpr
  min(const IntVarArgs& x) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MIN_N;
    ae->a = heap.alloc<IntVar>(x.size());
    ae->n = x.size();
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinExpr(ae);
  }

  LinExpr
  max(const IntVarArgs& x) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MAX_N;
    ae->a = heap.alloc<IntVar>(x.size());
    ae->n = x.size();
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinExpr(ae);
  }

  LinExpr
  operator *(const LinExpr& e0, const LinExpr& e1) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MULT;
    ae->e0 = e0;
    ae->e1 = e1;
    return LinExpr(ae);
  }

  LinExpr
  sqr(const LinExpr& e) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_SQR;
    ae->e0 = e;
    return LinExpr(ae);
  }

  LinExpr
  sqrt(const LinExpr& e) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_SQRT;
    ae->e0 = e;
    return LinExpr(ae);
  }

  LinExpr
  operator /(const LinExpr& e0, const LinExpr& e1) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_DIV;
    ae->e0 = e0;
    ae->e1 = e1;
    return LinExpr(ae);
  }

  LinExpr
  operator %(const LinExpr& e0, const LinExpr& e1) {
    MiniModel::ArithNonLinExpr* ae = new MiniModel::ArithNonLinExpr;
    ae->t = MiniModel::ArithNonLinExpr::ANLE_MOD;
    ae->e0 = e0;
    ae->e1 = e1;
    return LinExpr(ae);
  }

}

// STATISTICS: minimodel-any
