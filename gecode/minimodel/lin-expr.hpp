/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2010
 *     Guido Tack, 2004
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

namespace Gecode {

  /*
   * Operations for nodes
   *
   */
  forceinline
  LinExpr::Node::Node(void) : use(1) {
  }

  forceinline
  LinExpr::Node::~Node(void) {
    switch (t) {
    case NT_SUM_INT:
      if (n_int > 0)
        heap.free<Int::Linear::Term<Int::IntView> >(sum.ti,n_int);
      break;
    case NT_SUM_BOOL:
      if (n_bool > 0)
        heap.free<Int::Linear::Term<Int::BoolView> >(sum.tb,n_bool);
      break;
    case NT_NONLIN:
      delete sum.ne;
      break;
    default: ;
    }
  }

  forceinline void*
  LinExpr::Node::operator new(size_t size) {
    return heap.ralloc(size);
  }

  forceinline void
  LinExpr::Node::operator delete(void* p, size_t) {
    heap.rfree(p);
  }


  /*
   * Operations for expressions
   *
   */
  forceinline
  LinExpr::LinExpr(const LinExpr& e)
    : n(e.n) {
    n->use++;
  }

  forceinline int
  LinExpr::Node::fill(Home home, IntConLevel icl,
                      Int::Linear::Term<Int::IntView>* ti, 
                      Int::Linear::Term<Int::BoolView>* tb) const {
    double d=0;
    fill(home,icl,ti,tb,1.0,d);
    Int::Limits::check(d,"MiniModel::LinExpr");
    return static_cast<int>(d);
  }

  inline void
  LinExpr::post(Home home, IntRelType irt, IntConLevel icl) const {
    if (home.failed()) return;
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      if (n->t==NT_ADD && n->l == NULL && n->r->t==NT_NONLIN) {
        n->r->sum.ne->post(home,irt,-n->c,icl);
      } else if (n->t==NT_SUB && n->r->t==NT_NONLIN && n->l==NULL) {
        switch (irt) {
        case IRT_LQ: irt=IRT_GQ; break;
        case IRT_LE: irt=IRT_GR; break;
        case IRT_GQ: irt=IRT_LQ; break;
        case IRT_GR: irt=IRT_LE; break;
        default: break;
        }
        n->r->sum.ne->post(home,irt,n->c,icl);
      } else if (irt==IRT_EQ &&
                 n->t==NT_SUB && n->r->t==NT_NONLIN &&
                 n->l != NULL && n->l->t==NT_VAR_INT
                 && n->l->a==1) {
        (void) n->r->sum.ne->post(home,&n->l->x_int,icl);
      } else if (irt==IRT_EQ &&
                 n->t==NT_SUB && n->r->t==NT_VAR_INT &&
                 n->l != NULL && n->l->t==NT_NONLIN
                 && n->r->a==1) {
        (void) n->l->sum.ne->post(home,&n->r->x_int,icl);
      } else {
        Int::Linear::Term<Int::IntView>* its =
          r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int);
        int c = n->fill(home,icl,its,NULL);
        Int::Linear::post(home, its, n->n_int, irt, -c, icl);
      }
    } else if (n->n_int == 0) {
      // Only Boolean variables
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(home,icl,NULL,bts);
      Int::Linear::post(home, bts, n->n_bool, irt, -c, icl);
    } else if (n->n_bool == 1) {
      // Integer variables and only one Boolean variable
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(1);
      int c = n->fill(home,icl,its,bts);
      IntVar x(home,0,1);
      channel(home,bts[0].x,x);
      its[n->n_int].x = x;
      its[n->n_int].a = bts[0].a;
      Int::Linear::post(home, its, n->n_int+1, irt, -c, icl);
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(home,icl,its,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,0,min,max);
      IntVar x(home,min,max);
      its[n->n_int].x = x; its[n->n_int].a = 1;
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, 0, icl);
      Int::Linear::post(home, its, n->n_int+1, irt, -c, icl);
    }
  }

  inline void
  LinExpr::post(Home home, IntRelType irt, const BoolVar& b,
                IntConLevel icl) const {
    if (home.failed()) return;
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      if (n->t==NT_ADD && n->l==NULL && n->r->t==NT_NONLIN) {
        n->r->sum.ne->post(home,irt,-n->c,b,icl);
      } else if (n->t==NT_SUB && n->l==NULL && n->r->t==NT_NONLIN) {
        switch (irt) {
        case IRT_LQ: irt=IRT_GQ; break;
        case IRT_LE: irt=IRT_GR; break;
        case IRT_GQ: irt=IRT_LQ; break;
        case IRT_GR: irt=IRT_LE; break;
        default: break;
        }
        n->r->sum.ne->post(home,irt,n->c,b,icl);
      } else {
        Int::Linear::Term<Int::IntView>* its =
          r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int);
        int c = n->fill(home,icl,its,NULL);
        Int::Linear::post(home, its, n->n_int, irt, -c, b, icl);
      }
    } else if (n->n_int == 0) {
      // Only Boolean variables
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(home,icl,NULL,bts);
      Int::Linear::post(home, bts, n->n_bool, irt, -c, b, icl);
    } else if (n->n_bool == 1) {
      // Integer variables and only one Boolean variable
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(1);
      int c = n->fill(home,icl,its,bts);
      IntVar x(home,0,1);
      channel(home,bts[0].x,x);
      its[n->n_int].x = x;
      its[n->n_int].a = bts[0].a;
      Int::Linear::post(home, its, n->n_int+1, irt, -c, b, icl);
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(home,icl,its,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,0,min,max);
      IntVar x(home,min,max);
      its[n->n_int].x = x; its[n->n_int].a = 1;
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, 0, icl);
      Int::Linear::post(home, its, n->n_int+1, irt, -c, b, icl);
    }
  }

  inline IntVar
  LinExpr::post(Home home, IntConLevel icl) const {
    if (home.failed()) return IntVar(home,0,0);
    Region r(home);
    if (n->n_bool == 0) {
      // Only integer variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+1);
      int c = n->fill(home,icl,its,NULL);
      if ((n->n_int == 1) && (c == 0) && (its[0].a == 1))
        return its[0].x;
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
      int c = n->fill(home,icl,NULL,bts);
      int min, max;
      Int::Linear::estimate(&bts[0],n->n_bool,c,min,max);
      IntVar x(home, min, max);
      Int::Linear::post(home, bts, n->n_bool, IRT_EQ, x, -c, icl);
      return x;
    } else if (n->n_bool == 1) {
      // Integer variables and single Boolean variable
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+2);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(1);
      int c = n->fill(home,icl,its,bts);
      IntVar x(home, 0, 1);
      channel(home, x, bts[0].x);
      its[n->n_int].x = x; its[n->n_int].a = bts[0].a;
      int y_min, y_max;
      Int::Linear::estimate(&its[0],n->n_int+1,c,y_min,y_max);
      IntVar y(home, y_min, y_max);
      its[n->n_int+1].x = y; its[n->n_int+1].a = -1;
      Int::Linear::post(home, its, n->n_int+2, IRT_EQ, -c, icl);
      return y;
    } else {
      // Both integer and Boolean variables
      Int::Linear::Term<Int::IntView>* its =
        r.alloc<Int::Linear::Term<Int::IntView> >(n->n_int+2);
      Int::Linear::Term<Int::BoolView>* bts =
        r.alloc<Int::Linear::Term<Int::BoolView> >(n->n_bool);
      int c = n->fill(home,icl,its,bts);
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

  forceinline NonLinExpr*
  LinExpr::nle(void) const {
    return n->t == NT_NONLIN ? n->sum.ne : NULL;
  }

}

// STATISTICS: minimodel-any
