/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *     Vincent Barichard, 2012
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

#ifdef GECODE_HAS_FLOAT_VARS
    /// Non-linear float arithmetic expressions
    class GECODE_MINIMODEL_EXPORT ArithNonLinFloatExpr : public NonLinFloatExpr {
    public:
      /// The expression type
      enum ArithNonLinFloatExprType {
        ANLFE_ABS,   ///< Absolute value expression
        ANLFE_MIN,   ///< Minimum expression
        ANLFE_MAX,   ///< Maximum expression
        ANLFE_MULT,  ///< Multiplication expression
        ANLFE_DIV,   ///< Division expression
        ANLFE_SQR,   ///< Square expression
        ANLFE_SQRT,  ///< Square root expression
#ifdef GECODE_HAS_MPFR
        ANLFE_EXP,   ///< Exponential expression
        ANLFE_LOG,   ///< Logarithm root expression
        ANLFE_ASIN,  ///< expression
        ANLFE_SIN,   ///< expression
        ANLFE_ACOS,  ///< expression
        ANLFE_COS,   ///< expression
        ANLFE_ATAN,  ///< expression
        ANLFE_TAN,   ///< expression
#endif          
        ANLFE_POW,   ///< Pow expression
        ANLFE_NROOT  ///< Nth root expression
      } t;
      /// Expressions
      LinFloatExpr* a;
      /// Size of variable array
      int n;
      /// Integer argument (used in nroot for example)
      int aInt;
      /// Constructors
      ArithNonLinFloatExpr(ArithNonLinFloatExprType t0, int n0)
       : t(t0), a(heap.alloc<LinFloatExpr>(n0)), n(n0), aInt(-1) {}
      ArithNonLinFloatExpr(ArithNonLinFloatExprType t0, int n0, int a0)
       : t(t0), a(heap.alloc<LinFloatExpr>(n0)), n(n0), aInt(a0) {}
      /// Destructor
      ~ArithNonLinFloatExpr(void) { heap.free<LinFloatExpr>(a,n); }
      /// Post expression
      virtual FloatVar post(Home home, FloatVar* ret, FloatConLevel fcl) const {
        FloatVar y;
        switch (t) {
        case ANLFE_ABS:
          {
            FloatVar x = a[0].post(home, fcl);
            if (x.min() >= 0)
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              abs(home, x, y, fcl);
            }
          }
          break;
        case ANLFE_MIN:
          if (n==1) {
            y = result(home,ret, a[0].post(home, fcl));
          } else if (n==2) {
            FloatVar x0 = a[0].post(home, fcl);
            FloatVar x1 = a[1].post(home, fcl);
            if (x0.max() <= x1.min())
              y = result(home,ret,x0);
            else if (x1.max() <= x0.min())
              y = result(home,ret,x1);
            else {
              y = result(home,ret);
              min(home, x0, x1, y, fcl);
            }
          } else {
            FloatVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, fcl);
            y = result(home,ret);
            min(home, x, y, fcl);
          }
          break;
        case ANLFE_MAX:
          if (n==1) {
            y = result(home,ret,a[0].post(home, fcl));
          } else if (n==2) {
            FloatVar x0 = a[0].post(home, fcl);
            FloatVar x1 = a[1].post(home, fcl);
            if (x0.max() <= x1.min())
              y = result(home,ret,x1);
            else if (x1.max() <= x0.min())
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              max(home, x0, x1, y, fcl);
            }
          } else {
            FloatVarArgs x(n);
            for (int i=n; i--;)
              x[i] = a[i].post(home, fcl);
            y = result(home,ret);
            max(home, x, y, fcl);
          }
          break;
        case ANLFE_MULT:
          {
            assert(n == 2);
            FloatVar x0 = a[0].post(home, fcl);
            FloatVar x1 = a[1].post(home, fcl);
            if (x0.assigned() && (x0.val() == 0.0))
              y = result(home,ret,x0);
            else if (x0.assigned() && (x0.val() == 1.0))
              y = result(home,ret,x1);
            else if (x1.assigned() && (x1.val() == 0.0))
              y = result(home,ret,x1);
            else if (x1.assigned() && (x1.val() == 1.0))
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              mult(home, x0, x1, y, fcl);
            }
          }
          break;
        case ANLFE_DIV:
          {
            assert(n == 2);
            FloatVar x0 = a[0].post(home, fcl);
            FloatVar x1 = a[1].post(home, fcl);
            if (x1.assigned() && (x1.val() == 1.0))
              y = result(home,ret,x0);
            else if (x0.assigned() && (x0.val() == 0.0))
              y = result(home,ret,x0);
            else {
              y = result(home,ret);
              div(home, x0, x1, y, fcl);
            }
          }
          break;
        case ANLFE_SQR:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            if (x.assigned() && ((x.val() == 0.0) || (x.val() == 1.0)))
              y = x;
            else {
              y = result(home,ret);
              sqr(home, x, y, fcl);
            }
          }
          break;
        case ANLFE_SQRT:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            if (x.assigned() && ((x.val() == 0.0) || (x.val() == 1.0)))
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              sqrt(home, x, y, fcl);
            }
          }
          break;
        case ANLFE_POW:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            if (x.assigned() && ((x.val() == 0.0) || (x.val() == 1.0)))
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              pow(home, x, y, aInt, fcl);
            }
          }
          break;
        case ANLFE_NROOT:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            if (x.assigned() && ((x.val() == 0.0) || (x.val() == 1.0)))
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              nroot(home, x, y, aInt, fcl);
            }
          }
          break;
#ifdef GECODE_HAS_MPFR
        case ANLFE_EXP:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            if (x.assigned() && (x.val() == 0.0))
              y = result(home,ret,x);
            else {
              y = result(home,ret);
              exp(home, x, y, fcl);
            }
          }
          break;
        case ANLFE_LOG:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            log(home, x, y, fcl);
          }
          break;
        case ANLFE_ASIN:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            asin(home, x, y, fcl);
          }
          break;
        case ANLFE_SIN:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            sin(home, x, y, fcl);
          }
          break;
        case ANLFE_ACOS:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            acos(home, x, y, fcl);
          }
          break;
        case ANLFE_COS:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            cos(home, x, y, fcl);
          }
          break;
        case ANLFE_ATAN:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            atan(home, x, y, fcl);
          }
          break;
        case ANLFE_TAN:
          {
            assert(n == 1);
            FloatVar x = a[0].post(home, fcl);
            y = result(home,ret);
            tan(home, x, y, fcl);
          }
          break;
#endif
        default:
          GECODE_NEVER;
        }
        return y;
      }
      virtual void post(Home home, FloatRelType frt, FloatVal c,
                        FloatConLevel fcl) const {
        if ( (t == ANLFE_MIN && frt == FRT_GQ) ||
             (t == ANLFE_MAX && frt == FRT_LQ) ) {
          FloatVarArgs x(n);
          for (int i=n; i--;)
            x[i] = a[i].post(home, fcl);
          rel(home, x, frt, c);
        } else {
          rel(home, post(home,NULL,fcl), frt, c);
        }
      }
      virtual void post(Home home, FloatRelType frt, FloatVal c,
                        BoolVar b, bool t, FloatConLevel fcl) const {
        rel(home, post(home,NULL,fcl), frt, c, b, t);
      }
    };
    /// Check if \a e is of type \a t
    bool hasType(const LinFloatExpr& e, ArithNonLinFloatExpr::ArithNonLinFloatExprType t) {
      return e.nlfe() &&
             dynamic_cast<ArithNonLinFloatExpr*>(e.nlfe()) != NULL &&
             dynamic_cast<ArithNonLinFloatExpr*>(e.nlfe())->t == t;
    }
#endif

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

#ifdef GECODE_HAS_FLOAT_VARS
  LinFloatExpr
  abs(const LinFloatExpr& e) {
    if (hasType(e, ArithNonLinFloatExpr::ANLFE_ABS))
      return e;
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_ABS,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  min(const LinFloatExpr& e0, const LinFloatExpr& e1) {
    int n = 0;
    if (hasType(e0, ArithNonLinFloatExpr::ANLFE_MIN))
      n += static_cast<ArithNonLinFloatExpr*>(e0.nlfe())->n;
    else
      n += 1;
    if (hasType(e1, ArithNonLinFloatExpr::ANLFE_MIN))
      n += static_cast<ArithNonLinFloatExpr*>(e1.nlfe())->n;
    else
      n += 1;
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MIN,n);
    int i=0;
    if (hasType(e0, ArithNonLinFloatExpr::ANLFE_MIN)) {
      ArithNonLinFloatExpr* e0e = static_cast<ArithNonLinFloatExpr*>(e0.nlfe());
      for (; i<e0e->n; i++)
        ae->a[i] = e0e->a[i];
    } else {
      ae->a[i++] = e0;
    }
    if (hasType(e1, ArithNonLinFloatExpr::ANLFE_MIN)) {
      ArithNonLinFloatExpr* e1e = static_cast<ArithNonLinFloatExpr*>(e1.nlfe());
      int curN = i;
      for (; i<curN+e1e->n; i++)
        ae->a[i] = e1e->a[i-curN];
    } else {
      ae->a[i++] = e1;
    }
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  min(const FloatVarArgs& x) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MIN,x.size());
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  max(const LinFloatExpr& e0, const LinFloatExpr& e1) {
    int n = 0;
    if (hasType(e0, ArithNonLinFloatExpr::ANLFE_MAX))
      n += static_cast<ArithNonLinFloatExpr*>(e0.nlfe())->n;
    else
      n += 1;
    if (hasType(e1, ArithNonLinFloatExpr::ANLFE_MAX))
      n += static_cast<ArithNonLinFloatExpr*>(e1.nlfe())->n;
    else
      n += 1;
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MAX,n);
    int i=0;
    if (hasType(e0, ArithNonLinFloatExpr::ANLFE_MAX)) {
      ArithNonLinFloatExpr* e0e = static_cast<ArithNonLinFloatExpr*>(e0.nlfe());
      for (; i<e0e->n; i++)
        ae->a[i] = e0e->a[i];
    } else {
      ae->a[i++] = e0;
    }
    if (hasType(e1, ArithNonLinFloatExpr::ANLFE_MAX)) {
      ArithNonLinFloatExpr* e1e = static_cast<ArithNonLinFloatExpr*>(e1.nlfe());
      int curN = i;
      for (; i<curN+e1e->n; i++)
        ae->a[i] = e1e->a[i-curN];
    } else {
      ae->a[i++] = e1;
    }
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  max(const FloatVarArgs& x) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MAX,x.size());
    for (int i=x.size(); i--;)
      ae->a[i] = x[i];
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  operator *(const FloatVar& e0, const FloatVar& e1) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MULT,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  operator *(const LinFloatExpr& e0, const FloatVar& e1) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MULT,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  operator *(const FloatVar& e0, const LinFloatExpr& e1) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MULT,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  operator *(const LinFloatExpr& e0, const LinFloatExpr& e1) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_MULT,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  operator /(const LinFloatExpr& e0, const LinFloatExpr& e1) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_DIV,2);
    ae->a[0] = e0;
    ae->a[1] = e1;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  sqr(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_SQR,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  sqrt(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_SQRT,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  pow(const LinFloatExpr& e, int exp) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_POW,1,exp);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  nroot(const LinFloatExpr& e, int exp) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_NROOT,1,exp);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

#ifdef GECODE_HAS_MPFR

  LinFloatExpr
  exp(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_EXP,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  log(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_LOG,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  asin(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_ASIN,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  sin(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_SIN,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  acos(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_ACOS,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  cos(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_COS,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  atan(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_ATAN,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }

  LinFloatExpr
  tan(const LinFloatExpr& e) {
    ArithNonLinFloatExpr* ae =
      new ArithNonLinFloatExpr(ArithNonLinFloatExpr::ANLFE_TAN,1);
    ae->a[0] = e;
    return LinFloatExpr(ae);
  }
#endif

#endif

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
