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
        ANLE_SQRT,  ///< Square root expression
        ANLE_ELMNT  ///< Element expression
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
      virtual IntVar post(Home home, IntVar* ret, IntConLevel icl) const {
        IntVar y;
        switch (t) {
        case ANLE_ABS:
          {
            IntVar x = a[0].post(home, icl);
            if (x.min() >= 0)
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              abs(home, x, y, icl);
            }
          }
          break;
        case ANLE_MIN:
          if (n==1) {
            y = result(home,ret, a[0].post(home, icl));
          } else if (n==2) {
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            if (x0.max() <= x1.min())
              y = result(home,ret,x0);
            else if (x1.max() <= x0.min())
              y = result(home,ret,x1);
            else {
              y = result(home,ret);
              min(home, x0, x1, y, icl);
            }
          } else {
            IntVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, icl);
            y = result(home,ret);
            min(home, x, y, icl);
          }
          break;
        case ANLE_MAX:
          if (n==1) {
            y = result(home,ret,a[0].post(home, icl));
          } else if (n==2) {
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            if (x0.max() <= x1.min())
              y = result(home,ret,x1);
            else if (x1.max() <= x0.min())
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              max(home, x0, x1, y, icl);
            }
          } else {
            IntVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, icl);
            y = result(home,ret);
            max(home, x, y, icl);
          }
          break;
        case ANLE_MULT:
          {
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            if (x0.assigned() && (x0.val() == 0))
              y = result(home,ret,x0);
            else if (x0.assigned() && (x0.val() == 1))
              y = result(home,ret,x1);
            else if (x1.assigned() && (x1.val() == 0))
              y = result(home,ret,x1);
            else if (x1.assigned() && (x1.val() == 1))
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              mult(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_DIV:
          {
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            rel(home, x1, IRT_NQ, 0);
            if (x1.assigned() && (x1.val() == 1))
              y = result(home,ret,x0);
            else if (x0.assigned() && (x0.val() == 0))
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              div(home, x0, x1, y, icl);
            }
          }
          break;
        case ANLE_MOD:
          {
            assert(n == 2);
            IntVar x0 = a[0].post(home, icl);
            IntVar x1 = a[1].post(home, icl);
            y = result(home,ret);
            mod(home, x0, x1, y, icl);
          }
          break;
        case ANLE_SQR:
          {
            assert(n == 1);
            IntVar x = a[0].post(home, icl);
            if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
              y = x;
            else {
              y = result(home,ret);
              sqr(home, x, y, icl);
            }
          }
          break;
        case ANLE_SQRT:
          {
            assert(n == 1);
            IntVar x = a[0].post(home, icl);
            if (x.assigned() && ((x.val() == 0) || (x.val() == 1)))
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              sqrt(home, x, y, icl);
            }
          }
          break;
        case ANLE_ELMNT:
          {
            IntVar z = a[n-1].post(home, icl);
            if (z.assigned() && z.val() >= 0 && z.val() < n-1) {
              y = result(home,ret,a[z.val()].post(home, icl));
            } else {
              IntVarArgs x(n-1);
              bool assigned = true;
              for (int i=n-1; i--;) {
                x[i] = a[i].post(home, icl);
                if (!x[i].assigned())
                  assigned = false;
              }
              y = result(home,ret);
              if (assigned) {
                IntArgs xa(n-1);
                for (int i=n-1; i--;)
                  xa[i] = x[i].val();
                element(home, xa, z, y, icl);
              } else {
                element(home, x, z, y, icl);
              }
            }
          }
          break;
        default:
          GECODE_NEVER;
        }
        return y;
      }
      virtual void post(Home home, IntRelType irt, int c,
                        IntConLevel icl) const {
        if ( (t == ANLE_MIN && (irt == IRT_GQ || irt == IRT_GR)) ||
             (t == ANLE_MAX && (irt == IRT_LQ || irt == IRT_LE)) ) {
          IntVarArgs x(n);
          for (int i=n; i--;)
            x[i] = a[i].post(home, icl);
          rel(home, x, irt, c);
        } else {
          rel(home, post(home,NULL,icl), irt, c);
        }
      }
      virtual void post(Home home, IntRelType irt, int c,
                        BoolVar b, IntConLevel icl) const {
        rel(home, post(home,NULL,icl), irt, c, b);
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

  LinExpr
  element(const IntVarArgs& x, const LinExpr& e) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_ELMNT,x.size()+1);
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    ae->a[x.size()] = e;
    return LinExpr(ae);
  }

  LinExpr
  element(const IntArgs& x, const LinExpr& e) {
    ArithNonLinExpr* ae =
      new ArithNonLinExpr(ArithNonLinExpr::ANLE_ELMNT,x.size()+1);
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    ae->a[x.size()] = e;
    return LinExpr(ae);
  }

}

// STATISTICS: minimodel-any
