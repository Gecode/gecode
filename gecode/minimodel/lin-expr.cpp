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

  LinExpr::LinExpr(int c) :
    n(new Node) {
    n->n_int = n->n_bool = 0;
    n->t = NT_CONST;
    n->l = n->r = NULL;
    n->a = 0;
    Int::Limits::check(c,"MiniModel::LinExpr");
    n->c = c;
  }

  LinExpr::LinExpr(const IntVar& x, int a) :
    n(new Node) {
    n->n_int = 1;
    n->n_bool = 0;
    n->t = NT_VAR_INT;
    n->l = n->r = NULL;
    n->a = a;
    n->x_int = x;
  }

  LinExpr::LinExpr(const BoolVar& x, int a) :
    n(new Node) {
    n->n_int = 0;
    n->n_bool = 1;
    n->t = NT_VAR_BOOL;
    n->l = n->r = NULL;
    n->a = a;
    n->x_bool = x;
  }

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

  LinExpr::LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1) :
    n(new Node) {
    n->n_int = e0.n->n_int + e1.n->n_int;
    n->n_bool = e0.n->n_bool + e1.n->n_bool;
    n->t = t;
    n->l = e0.n; n->l->use++;
    n->r = e1.n; n->r->use++;
  }

  LinExpr::LinExpr(const LinExpr& e, NodeType t, int c) :
    n(new Node) {
    n->n_int = e.n->n_int;
    n->n_bool = e.n->n_bool;
    n->t = t;
    n->l = NULL;
    n->r = e.n; n->r->use++;
    n->c = c;
  }

  LinExpr::LinExpr(int a, const LinExpr& e) :
    n(new Node) {
    n->n_int = e.n->n_int;
    n->n_bool = e.n->n_bool;
    n->t = NT_MUL;
    n->l = e.n; n->l->use++;
    n->r = NULL;
    n->a = a;
  }

  LinExpr::LinExpr(NonLinExpr* e) :
    n(new Node) {
    n->n_int = 1;
    n->n_bool = 0;
    n->t = NT_NONLIN;
    n->l = n->r = NULL;
    n->a = 0;
    n->sum.ne = e;
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
  LinExpr::Node::fill(Home home, IntConLevel icl,
                      Int::Linear::Term<Int::IntView>*& ti, 
                      Int::Linear::Term<Int::BoolView>*& tb,
                      double m, double& d) const {
    switch (this->t) {
    case NT_CONST:
      Int::Limits::check(m*c,"MiniModel::LinExpr");
      d += m*c;
      break;
    case NT_VAR_INT:
      Int::Limits::check(m*a,"MiniModel::LinExpr");
      ti->a=static_cast<int>(m*a); ti->x=x_int; ti++;
      break;
    case NT_NONLIN:
      ti->a=static_cast<int>(m); ti->x=sum.ne->post(home, NULL, icl); ti++;
      break;
    case NT_VAR_BOOL:
      Int::Limits::check(m*a,"MiniModel::LinExpr");
      tb->a=static_cast<int>(m*a); tb->x=x_bool; tb++;
      break;
    case NT_SUM_INT:
      for (int i=n_int; i--; ) {
        Int::Limits::check(m*sum.ti[i].a,"MiniModel::LinExpr");
        ti[i].x = sum.ti[i].x; ti[i].a = static_cast<int>(m*sum.ti[i].a);
      }
      ti += n_int;
      break;
    case NT_SUM_BOOL:
      for (int i=n_bool; i--; ) {
        Int::Limits::check(m*sum.tb[i].a,"MiniModel::LinExpr");
        tb[i].x = sum.tb[i].x; tb[i].a = static_cast<int>(m*sum.tb[i].a);
      }
      tb += n_bool;
      break;
    case NT_ADD:
      if (l == NULL) {
        Int::Limits::check(m*c,"MiniModel::LinExpr");
        d += m*c;
      } else {
        l->fill(home,icl,ti,tb,m,d);
      }
      r->fill(home,icl,ti,tb,m,d);
      break;
    case NT_SUB:
      if (l == NULL) {
        Int::Limits::check(m*c,"MiniModel::LinExpr");
        d += m*c;
      } else {
        l->fill(home,icl,ti,tb,m,d);
      }
      r->fill(home,icl,ti,tb,-m,d);
      break;
    case NT_MUL:
      Int::Limits::check(m*a,"MiniModel::LinExpr");
      l->fill(home,icl,ti,tb,m*a,d);
      break;
    default:
      GECODE_NEVER;
    }
  }


  /*
   * Operators
   *
   */
  LinExpr
  operator +(int c, const IntVar& x) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)+x.val()))
      return LinExpr(static_cast<double>(c)+x.val());
    else
      return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(int c, const BoolVar& x) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)+x.val()))
      return LinExpr(static_cast<double>(c)+x.val());
    else
      return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const IntVar& x, int c) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)+x.val()))
      return LinExpr(static_cast<double>(c)+x.val());
    else
      return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const BoolVar& x, int c) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)+x.val()))
      return LinExpr(static_cast<double>(c)+x.val());
    else
      return LinExpr(x,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  LinExpr
  operator +(const IntVar& x, const IntVar& y) {
    if (x.assigned())
      return x.val() + y;
    else if (y.assigned())
      return x + y.val();
    else
      return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const IntVar& x, const BoolVar& y) {
    if (x.assigned())
      return x.val() + y;
    else if (y.assigned())
      return x + y.val();
    else
      return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const BoolVar& x, const IntVar& y) {
    if (x.assigned())
      return x.val() + y;
    else if (y.assigned())
      return x + y.val();
    else
      return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const BoolVar& x, const BoolVar& y) {
    if (x.assigned())
      return x.val() + y;
    else if (y.assigned())
      return x + y.val();
    else
      return LinExpr(x,LinExpr::NT_ADD,y);
  }
  LinExpr
  operator +(const IntVar& x, const LinExpr& e) {
    if (x.assigned())
      return x.val() + e;
    else
      return LinExpr(x,LinExpr::NT_ADD,e);
  }
  LinExpr
  operator +(const BoolVar& x, const LinExpr& e) {
    if (x.assigned())
      return x.val() + e;
    else
      return LinExpr(x,LinExpr::NT_ADD,e);
  }
  LinExpr
  operator +(const LinExpr& e, const IntVar& x) {
    if (x.assigned())
      return e + x.val();
    else
      return LinExpr(e,LinExpr::NT_ADD,x);
  }
  LinExpr
  operator +(const LinExpr& e, const BoolVar& x) {
    if (x.assigned())
      return e + x.val();
    else
      return LinExpr(e,LinExpr::NT_ADD,x);
  }
  LinExpr
  operator +(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_ADD,e2);
  }

  LinExpr
  operator -(int c, const IntVar& x) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)-x.val()))
      return LinExpr(static_cast<double>(c)-x.val());
    else
      return LinExpr(x,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(int c, const BoolVar& x) {
    if (x.assigned() && Int::Limits::valid(static_cast<double>(c)-x.val()))
      return LinExpr(static_cast<double>(c)-x.val());
    else
      return LinExpr(x,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,c);
  }
  LinExpr
  operator -(const IntVar& x, int c) {
    if (x.assigned() && Int::Limits::valid(x.val()-static_cast<double>(c)))
      return LinExpr(x.val()-static_cast<double>(c));
    else
      return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const BoolVar& x, int c) {
    if (x.assigned() && Int::Limits::valid(x.val()-static_cast<double>(c)))
      return LinExpr(x.val()-static_cast<double>(c));
    else
      return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,-c);
  }
  LinExpr
  operator -(const IntVar& x, const IntVar& y) {
    if (x.assigned())
      return x.val() - y;
    else if (y.assigned())
      return x - y.val();
    else
      return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const IntVar& x, const BoolVar& y) {
    if (x.assigned())
      return x.val() - y;
    else if (y.assigned())
      return x - y.val();
    else
      return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const BoolVar& x, const IntVar& y) {
    if (x.assigned())
      return x.val() - y;
    else if (y.assigned())
      return x - y.val();
    else
      return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const BoolVar& x, const BoolVar& y) {
    if (x.assigned())
      return x.val() - y;
    else if (y.assigned())
      return x - y.val();
    else
      return LinExpr(x,LinExpr::NT_SUB,y);
  }
  LinExpr
  operator -(const IntVar& x, const LinExpr& e) {
    if (x.assigned())
      return x.val() - e;
    else
      return LinExpr(x,LinExpr::NT_SUB,e);
  }
  LinExpr
  operator -(const BoolVar& x, const LinExpr& e) {
    if (x.assigned())
      return x.val() - e;
    else
      return LinExpr(x,LinExpr::NT_SUB,e);
  }
  LinExpr
  operator -(const LinExpr& e, const IntVar& x) {
    if (x.assigned())
      return e - x.val();
    else
      return LinExpr(e,LinExpr::NT_SUB,x);
  }
  LinExpr
  operator -(const LinExpr& e, const BoolVar& x) {
    if (x.assigned())
      return e - x.val();
    else
      return LinExpr(e,LinExpr::NT_SUB,x);
  }
  LinExpr
  operator -(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_SUB,e2);
  }

  LinExpr
  operator -(const IntVar& x) {
    if (x.assigned())
      return LinExpr(-x.val());
    else
      return LinExpr(x,LinExpr::NT_SUB,0);
  }
  LinExpr
  operator -(const BoolVar& x) {
    if (x.assigned())
      return LinExpr(-x.val());
    else
      return LinExpr(x,LinExpr::NT_SUB,0);
  }
  LinExpr
  operator -(const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,0);
  }

  LinExpr
  operator *(int a, const IntVar& x) {
    if (a == 0)
      return LinExpr(0.0);
    else if (x.assigned() && 
             Int::Limits::valid(static_cast<double>(a)*x.val()))
      return LinExpr(static_cast<double>(a)*x.val());
    else
      return LinExpr(x,a);
  }
  LinExpr
  operator *(int a, const BoolVar& x) {
    if (a == 0)
      return LinExpr(0.0);
    else if (x.assigned() && 
             Int::Limits::valid(static_cast<double>(a)*x.val()))
      return LinExpr(static_cast<double>(a)*x.val());
    else
      return LinExpr(x,a);
  }
  LinExpr
  operator *(const IntVar& x, int a) {
    if (a == 0)
      return LinExpr(0.0);
    else if (x.assigned() && 
             Int::Limits::valid(static_cast<double>(a)*x.val()))
      return LinExpr(static_cast<double>(a)*x.val());
    else
      return LinExpr(x,a);
  }
  LinExpr
  operator *(const BoolVar& x, int a) {
    if (a == 0)
      return LinExpr(0.0);
    else if (x.assigned() && 
             Int::Limits::valid(static_cast<double>(a)*x.val()))
      return LinExpr(static_cast<double>(a)*x.val());
    else
      return LinExpr(x,a);
  }
  LinExpr
  operator *(const LinExpr& e, int a) {
    if (a == 0)
      return LinExpr(0.0);
    else
      return LinExpr(a,e);
  }
  LinExpr
  operator *(int a, const LinExpr& e) {
    if (a == 0)
      return LinExpr(0.0);
    else
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


  IntVar
  expr(Home home, const LinExpr& e, IntConLevel icl) {
    if (!home.failed())
      return e.post(home,icl);
    IntVar x(home,Int::Limits::min,Int::Limits::max);
    return x;
  }

}

// STATISTICS: minimodel-any
