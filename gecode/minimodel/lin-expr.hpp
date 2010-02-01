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

  forceinline
  LinExpr::LinExpr(const IntVar& x, int a) :
    n(new Node) {
    n->n_int = 1;
    n->n_bool = 0;
    n->t = NT_VAR_INT;
    n->l = n->r = NULL;
    n->a = a;
    n->x_int = x;
  }

  forceinline
  LinExpr::LinExpr(const BoolVar& x, int a) :
    n(new Node) {
    n->n_int = 0;
    n->n_bool = 1;
    n->t = NT_VAR_BOOL;
    n->l = n->r = NULL;
    n->a = a;
    n->x_bool = x;
  }

  forceinline
  LinExpr::LinExpr(const IntVarArgs& x) :
    n(new Node) {
    n->n_int = x.size();
    n->n_bool = 0;
    n->t = NT_SUM_INT;
    n->l = n->r = NULL;
    if (x.size() > 0) {
      n->sum.ti = heap.alloc<Int::Linear::Term<Int::IntView> >(x.size());
      for (int i=x.size(); i--; ) {
        n->sum.ti[i].x = x[i];
        n->sum.ti[i].a = 1;
      }
    }
  }

  forceinline
  LinExpr::LinExpr(const IntArgs& a, const IntVarArgs& x) :
    n(new Node) {
    if (a.size() != x.size())
      throw Int::ArgumentSizeMismatch("MiniModel::LinExpr");
    n->n_int = x.size();
    n->n_bool = 0;
    n->t = NT_SUM_INT;
    n->l = n->r = NULL;
    if (x.size() > 0) {
      n->sum.ti = heap.alloc<Int::Linear::Term<Int::IntView> >(x.size());
      for (int i=x.size(); i--; ) {
        n->sum.ti[i].x = x[i];
        n->sum.ti[i].a = a[i];
      }
    }
  }

  forceinline
  LinExpr::LinExpr(const BoolVarArgs& x) :
    n(new Node) {
    n->n_int = 0;
    n->n_bool = x.size();
    n->t = NT_SUM_BOOL;
    n->l = n->r = NULL;
    if (x.size() > 0) {
      n->sum.tb = heap.alloc<Int::Linear::Term<Int::BoolView> >(x.size());
      for (int i=x.size(); i--; ) {
        n->sum.tb[i].x = x[i];
        n->sum.tb[i].a = 1;
      }
    }
  }

  forceinline
  LinExpr::LinExpr(const IntArgs& a, const BoolVarArgs& x) :
    n(new Node) {
    if (a.size() != x.size())
      throw Int::ArgumentSizeMismatch("MiniModel::LinExpr");
    n->n_int = 0;
    n->n_bool = x.size();
    n->t = NT_SUM_BOOL;
    n->l = n->r = NULL;
    if (x.size() > 0) {
      n->sum.tb = heap.alloc<Int::Linear::Term<Int::BoolView> >(x.size());
      for (int i=x.size(); i--; ) {
        n->sum.tb[i].x = x[i];
        n->sum.tb[i].a = a[i];
      }
    }
  }

  forceinline
  LinExpr::LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1) :
    n(new Node) {
    n->n_int = e0.n->n_int + e1.n->n_int;
    n->n_bool = e0.n->n_bool + e1.n->n_bool;
    n->t = t;
    n->l = e0.n; n->l->use++;
    n->r = e1.n; n->r->use++;
  }

  forceinline
  LinExpr::LinExpr(const LinExpr& e, NodeType t, int c) :
    n(new Node) {
    n->n_int = e.n->n_int;
    n->n_bool = e.n->n_bool;
    n->t = t;
    n->l = NULL;
    n->r = e.n; n->r->use++;
    n->c = c;
  }

  forceinline
  LinExpr::LinExpr(int a, const LinExpr& e) :
    n(new Node) {
    n->n_int = e.n->n_int;
    n->n_bool = e.n->n_bool;
    n->t = NT_MUL;
    n->l = e.n; n->l->use++;
    n->r = NULL;
    n->a = a;
  }



  inline IntVar
  post(Home home, const LinExpr& e, IntConLevel icl) {
    if (!home.failed())
      return e.post(home,icl);
    IntVar x(home,Int::Limits::min,Int::Limits::max);
    return x;
  }

}

// STATISTICS: minimodel-any
