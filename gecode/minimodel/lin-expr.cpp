/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
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

  bool
  LinExpr::Node::decrement(void) {
    if (--use == 0) {
      if ((l != NULL) && l->decrement())
        delete l;
      if ((r != NULL) && r->decrement())
        delete r;
      return true;
    }
    return false;
  }

  /*
   * Operations for expressions
   *
   */

  LinExpr::LinExpr(void) :
    n(new Node) {
    n->n_int = n->n_bool = 0;
    n->t = NT_VAR_INT;
    n->l = n->r = NULL;
    n->a = 0;
  }

  const LinExpr&
  LinExpr::operator =(const LinExpr& e) {
    if (this != &e) {
      if (n->decrement())
        delete n;
      n = e.n; n->use++;
    }
    return *this;
  }

  LinExpr::~LinExpr(void) {
    if (n->decrement())
      delete n;
  }


  void
  LinExpr::Node::fill(Int::Linear::Term<Int::IntView>*& ti, 
                      Int::Linear::Term<Int::BoolView>*& tb,
                      double m, double& d) const {
    switch (this->t) {
    case NT_VAR_INT:
      if (a != 0) {
        Int::Limits::check(m*a,"MiniModel::LinExpr");
        ti->a=m*a; ti->x=x_int; ti++;
      }
      break;
    case NT_VAR_BOOL:
      if (a != 0) {
        Int::Limits::check(m*a,"MiniModel::LinExpr");
        tb->a=m*a; tb->x=x_bool; tb++;
      }
      break;
    case NT_SUM_INT:
      for (int i=n_int; i--; ) {
        Int::Limits::check(m*sum.ti[i].a,"MiniModel::LinExpr");
        ti[i].x = sum.ti[i].x; ti[i].a = m*sum.ti[i].a;
      }
      ti += n_int;
      break;
    case NT_SUM_BOOL:
      for (int i=n_bool; i--; ) {
        Int::Limits::check(m*sum.tb[i].a,"MiniModel::LinExpr");
        tb[i].x = sum.tb[i].x; tb[i].a = m*sum.tb[i].a;
      }
      tb += n_bool;
      break;
    case NT_ADD:
      if (l == NULL) {
        Int::Limits::check(m*c,"MiniModel::LinExpr");
        d += m*c;
      } else {
        l->fill(ti,tb,m,d);
      }
      r->fill(ti,tb,m,d);
      break;
    case NT_SUB:
      if (l == NULL) {
        Int::Limits::check(m*c,"MiniModel::LinExpr");
        d += m*c;
      } else {
        l->fill(ti,tb,m,d);
      }
      r->fill(ti,tb,-m,d);
      break;
    case NT_MUL:
      Int::Limits::check(m*a,"MiniModel::LinExpr");
      l->fill(ti,tb,m*a,d);
      break;
    default:
      GECODE_NEVER;
    }
  }

  forceinline int
  LinExpr::Node::fill(Int::Linear::Term<Int::IntView>* ti, 
                      Int::Linear::Term<Int::BoolView>* tb) const {
    double d=0;
    fill(ti,tb,1.0,d);
    Int::Limits::check(d,"MiniModel::LinExpr");
    return static_cast<int>(d);
  }

  void
  LinExpr::post(Home home, IntRelType irt, IntConLevel icl) const {
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int);
      int c = n->fill(its,NULL);
      Int::Linear::post(home, its, n->n_int, irt, -c, icl);
    } else if (n->n_int == 0) {
      // Only Boolean variables
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(NULL,bts);
      Int::Linear::post(home, bts, n->n_bool, irt, -c, icl);
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(its,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,0,min,max);
      IntVar x(home,min,max);
      its[n->n_int].x = x; its[n->n_int].a = 1;
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, 0, icl);
      Int::Linear::post(home, its, n->n_int+1, irt, -c, icl);
    }
  }

  void
  LinExpr::post(Home home, IntRelType irt, const BoolVar& b,
                IntConLevel icl) const {
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int);
      int c = n->fill(its,NULL);
      Int::Linear::post(home, its, n->n_int, irt, -c, b, icl);
    } else if (n->n_int == 0) {
      // Only Boolean variables
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(NULL,bts);
      Int::Linear::post(home, bts, n->n_bool, irt, -c, b, icl);
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(its,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,0,min,max);
      IntVar x(home,min,max);
      its[n->n_int].x = x; its[n->n_int].a = 1;
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, 0, icl);
      Int::Linear::post(home, its, n->n_int+1, irt, -c, b, icl);
    }
  }

  IntVar
  LinExpr::post(Home home, IntConLevel icl) const {
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      int c = n->fill(its,NULL);
      int min, max;
      Int::Linear::estimate(&its[0],n->n_int,c,min,max);
      IntVar x(home, min, max);
      its[n->n_int].x = x; its[n->n_int].a = -1;
      Int::Linear::post(home, its, n->n_int+1, IRT_EQ, -c, icl);
      return x;
    } else if (n->n_int == 0) {
      // Only Boolean variables
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(NULL,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,c,min,max);
      IntVar x(home, min, max);
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, -c, icl);
      return x;
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+2);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(its,bts);
      int x_min, x_max;
      Int::Linear::estimate(&bts[0],n->n_bool,0,x_min,x_max);
      IntVar x(home, x_min, x_max);
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, 0, icl);
      its[n->n_int].x = x; its[n->n_int].a = 1;
      int y_min, y_max;
      Int::Linear::estimate(&its[0],n->n_int+1,c,y_min,y_max);
      IntVar y(home, y_min, y_max);
      its[n->n_int+1].x = y; its[n->n_int+1].a = -1;
      Int::Linear::post(home, its, n->n_int+2, IRT_EQ, -c, icl);
      return y;
    }
  }


  /*
   * Operators
   *
   */
  LinExpr
  operator +(int c, const IntVar& x) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(int c, const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const IntVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const BoolVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const IntVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const IntVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const BoolVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const BoolVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const IntVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_ADD,e);
  }
  LinExpr
  operator +(const BoolVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_ADD,e);
  }
  LinExpr
  operator +(const LinExpr& e, const IntVar& x) {
    return LinExpr(e,LinExpr::NT_ADD,x);
  }
  LinExpr
  operator +(const LinExpr& e, const BoolVar& x) {
    return LinExpr(e,LinExpr::NT_ADD,x);
  }
  LinExpr
  operator +(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_ADD,e2);
  }

  LinExpr
  operator -(int c, const IntVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(int c, const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(const IntVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const BoolVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const IntVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const IntVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const BoolVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const BoolVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const IntVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_SUB,e);
  }
  LinExpr
  operator -(const BoolVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_SUB,e);
  }
  LinExpr
  operator -(const LinExpr& e, const IntVar& x) {
    return LinExpr(e,LinExpr::NT_SUB,x);
  }
  LinExpr
  operator -(const LinExpr& e, const BoolVar& x) {
    return LinExpr(e,LinExpr::NT_SUB,x);
  }
  LinExpr
  operator -(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_SUB,e2);
  }

  LinExpr
  operator -(const IntVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,0);
  }
  LinExpr
  operator -(const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,0);
  }
  LinExpr
  operator -(const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,0);
  }

  LinExpr
  operator *(int a, const IntVar& x) {
    return LinExpr(x,a);
  }
  LinExpr
  operator *(int a, const BoolVar& x) {
    return LinExpr(x,a);
  }
  LinExpr
  operator *(const IntVar& x, int a) {
    return LinExpr(x,a);
  }
  LinExpr
  operator *(const BoolVar& x, int a) {
    return LinExpr(x,a);
  }
  LinExpr
  operator *(const LinExpr& e, int a) {
    return LinExpr(a,e);
  }
  LinExpr
  operator *(int a, const LinExpr& e) {
    return LinExpr(a,e);
  }

  LinExpr
  sum(const IntVarArgs& x) {
    return LinExpr(x);
  }
  LinExpr
  sum(const IntArgs& a, const IntVarArgs& x) {
    return LinExpr(a,x);
  }
  LinExpr
  sum(const BoolVarArgs& x) {
    return LinExpr(x);
  }
  LinExpr
  sum(const IntArgs& a, const BoolVarArgs& x) {
    return LinExpr(a,x);
  }


}

// STATISTICS: minimodel-any
