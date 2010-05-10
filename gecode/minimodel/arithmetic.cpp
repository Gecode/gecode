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
        ANLE_MIN,   ///< Minimum expression
        ANLE_MAX,   ///< Maximum expression
        ANLE_MULT,  ///< Multiplication expression
        ANLE_DIV,   ///< Division expression
        ANLE_MOD,   ///< Modulo expression
        ANLE_SQR,   ///< Square expression
        ANLE_SQRT   ///< Square root expression
      } t;
      /// Expressions
      LinExpr* a;
      /// Size of variable array
      int n;
      /// Constructor
      ArithNonLinExpr(ArithNonLinExprType t0, int n0)
       : t(t0), a(heap.alloc<LinExpr>(n0)), n(n0) {}
      /// Destructor
      ~ArithNonLinExpr(void) { heap.free<LinExpr>(a,n); }
      /// Post expression
      virtual IntVar post(Home home, IntConLevel icl) const {
        IntVar y;
        switch (t) {
        case ANLE_ABS:
          {
            IntVar x = a[0].post(home, icl);
            if (x.min() >= 0)
              y = x;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              abs(home, x, y, icl);
            }
          }
          break;
        case ANLE_MIN:
          if (n==1) {
            y = a[0].post(home, icl);
          } else if (n==2) {
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            if (x0.max() <= x1.min())
              y = x0;
            else if (x1.max() <= x0.min())
              y = x1;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              min(home, x0, x1, y, icl);
            }
          } else {
            IntVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, icl);
            y = IntVar(home, Int::Limits::min, Int::Limits::max);
            min(home, x, y, icl);
          }
          break;
        case ANLE_MAX:
          if (n==1) {
            y = a[0].post(home, icl);
          } else if (n==2) {
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            if (x0.max() <= x1.min())
              y = x1;
            else if (x1.max() <= x0.min())
              y = x0;
            else {
              y = IntVar(home, Int::Limits::min, Int::Limits::max);
              max(home, x0, x1, y, icl);
            }
          } else {
            IntVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, icl);
            y = IntVar(home, Int::Limits::min, Int::Limits::max);
            max(home, x, y, icl);
          }
          break;
        case ANLE_MULT:
          {
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
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
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
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
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            y = IntVar(home, Int::Limits::min, Int::Limits::max);
            div(home, x0, x1, y, icl);
          }
          break;
        case ANLE_SQR:
          {
            assert(n == 1);
            IntVar x = a[0].post(home, icl);
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
            assert(n == 1);
            IntVar x = a[0].post(home, icl);
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
    /// Check if \a e is of type \a t
    bool hasType(const LinExpr& e, ArithNonLinExpr::ArithNonLinExprType t) {
      return e.nle() &&
             dynamic_cast<ArithNonLinExpr*>(e.nle()) != NULL &&
             dynamic_cast<ArithNonLinExpr*>(e.nle())->t == t;
    }
  }

  using namespace MiniModel;

  LinExpr
  abs(const LinExpr& e) {
    if (hasType(e, ArithNonLinExpr::ANLE_ABS))
      return e;
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_ABS,1);
    ae->a[0] = e;
    return LinExpr(ae);
  }

  LinExpr
  min(const LinExpr& e0, const LinExpr& e1) {
    int n = 0;
    if (hasType(e0, ArithNonLinExpr::ANLE_MIN))
      n += static_cast<ArithNonLinExpr*>(e0.nle())->n;
    else
      n += 1;
    if (hasType(e1, ArithNonLinExpr::ANLE_MIN))
      n += static_cast<ArithNonLinExpr*>(e1.nle())->n;
    else
      n += 1;
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MIN,n);
    int i=0;
    if (hasType(e0, ArithNonLinExpr::ANLE_MIN)) {
      ArithNonLinExpr* e0e = static_cast<ArithNonLinExpr*>(e0.nle());
      for (; i<e0e->n; i++)
        ae->a[i] = e0e->a[i];
    } else {
      ae->a[i++] = e0;
    }
    if (hasType(e1, ArithNonLinExpr::ANLE_MIN)) {
      ArithNonLinExpr* e1e = static_cast<ArithNonLinExpr*>(e1.nle());
      int curN = i;
      for (; i<curN+e1e->n; i++)
        ae->a[i] = e1e->a[i-curN];
    } else {
      ae->a[i++] = e1;
    }
    return LinExpr(ae);
  }

  LinExpr
  max(const LinExpr& e0, const LinExpr& e1) {
    int n = 0;
    if (hasType(e0, ArithNonLinExpr::ANLE_MAX))
      n += static_cast<ArithNonLinExpr*>(e0.nle())->n;
    else
      n += 1;
    if (hasType(e1, ArithNonLinExpr::ANLE_MAX))
      n += static_cast<ArithNonLinExpr*>(e1.nle())->n;
    else
      n += 1;
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MAX,n);
    int i=0;
    if (hasType(e0, ArithNonLinExpr::ANLE_MAX)) {
      ArithNonLinExpr* e0e = static_cast<ArithNonLinExpr*>(e0.nle());
      for (; i<e0e->n; i++)
        ae->a[i] = e0e->a[i];
    } else {
      ae->a[i++] = e0;
    }
    if (hasType(e1, ArithNonLinExpr::ANLE_MAX)) {
      ArithNonLinExpr* e1e = static_cast<ArithNonLinExpr*>(e1.nle());
      int curN = i;
      for (; i<curN+e1e->n; i++)
        ae->a[i] = e1e->a[i-curN];
    } else {
      ae->a[i++] = e1;
    }
    return LinExpr(ae);
  }

  LinExpr
  min(const IntVarArgs& x) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MIN,x.size());
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinExpr(ae);
  }

  LinExpr
  max(const IntVarArgs& x) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MAX,x.size());
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinExpr(ae);
  }

  LinExpr
  operator *(const LinExpr& e0, const LinExpr& e1) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MULT,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinExpr(ae);
  }

  LinExpr
  sqr(const LinExpr& e) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_SQR,1);
    ae->a[0] = e;
    return LinExpr(ae);
  }

  LinExpr
  sqrt(const LinExpr& e) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_SQRT,1);
    ae->a[0] = e;
    return LinExpr(ae);
  }

  LinExpr
  operator /(const LinExpr& e0, const LinExpr& e1) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_DIV,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinExpr(ae);
  }

  LinExpr
  operator %(const LinExpr& e0, const LinExpr& e1) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_MOD,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinExpr(ae);
  }

}

// STATISTICS: minimodel-any
