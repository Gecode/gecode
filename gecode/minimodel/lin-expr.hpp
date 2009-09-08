/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
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
  template<class Var>
  forceinline
  LinExpr<Var>::Node::Node(void) : use(1) {
  }

  template<class Var>
  forceinline void*
  LinExpr<Var>::Node::operator new(size_t size) {
    return heap.ralloc(size);
  }

  template<class Var>
  forceinline void
  LinExpr<Var>::Node::operator delete(void* p, size_t) {
    heap.rfree(p);
  }

  template<class Var>
  bool
  LinExpr<Var>::Node::decrement(void) {
    if (--use == 0) {
      if ((l != NULL) && l->decrement())
        delete l;
      if ((r != NULL) && r->decrement())
        delete r;
      return true;
    }
    return false;
  }

  template<class Var>
  int
  LinExpr<Var>::Node::fill(Int::Linear::Term<View> t[],
                           int i, int m, int c_i, int& c_o) const {
    switch (this->t) {
    case NT_VAR:
      c_o = c_i;
      if (a != 0) {
        t[i].a=m*a; t[i].x=x;
        return i+1;
      } else {
        return i;
      }
    case NT_ADD:
      if (l == NULL) {
        return r->fill(t,i,m,c_i+m*c,c_o);
      } else {
        int c_m = 0;
        i = l->fill(t,i,m,c_i,c_m);
        return r->fill(t,i,m,c_m,c_o);
      }
    case NT_SUB:
      if (l == NULL) {
        return r->fill(t,i,-m,c_i+m*c,c_o);
      } else {
        int c_m = 0;
        i = l->fill(t,i,m,c_i,c_m);
        return r->fill(t,i,-m,c_m,c_o);
      }
    case NT_MUL:
      return l->fill(t,i,a*m,c_i,c_o);
    default:
      GECODE_NEVER;
    }
    GECODE_NEVER;
    return 0;
  }


  /*
   * Operations for expressions
   *
   */

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(void) :
    n(new Node) {
    n->n = 0;
    n->t = NT_VAR;
    n->l = n->r = NULL;
    n->a = 0;
  }

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(const Var& x, int a) :
    n(new Node) {
    n->n = 1;
    n->t = NT_VAR;
    n->l = n->r = NULL;
    n->a = a;
    n->x = x;
  }

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1) :
    n(new Node) {
    n->n = e0.n->n+e1.n->n;
    n->t = t;
    n->l = e0.n; n->l->use++;
    n->r = e1.n; n->r->use++;
  }

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(const LinExpr& e, NodeType t, int c) :
    n(new Node) {
    n->n = e.n->n;
    n->t = t;
    n->l = NULL;
    n->r = e.n; n->r->use++;
    n->c = c;
  }

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(int a, const LinExpr& e) :
    n(new Node) {
    n->n = e.n->n;
    n->t = NT_MUL;
    n->l = e.n; n->l->use++;
    n->r = NULL;
    n->a = a;
  }

  template<class Var>
  inline
  LinExpr<Var>::LinExpr(const LinExpr<Var>& e)
    : n(e.n) {
    n->use++;
  }

  template<class Var>
  inline const LinExpr<Var>&
  LinExpr<Var>::operator =(const LinExpr<Var>& e) {
    if (this != &e) {
      if (n->decrement())
        delete n;
      n = e.n; n->use++;
    }
    return *this;
  }

  template<class Var>
  forceinline
  LinExpr<Var>::~LinExpr(void) {
    if (n->decrement())
      delete n;
  }

  template<class Var>
  inline void
  LinExpr<Var>::post(Space& home, IntRelType irt, IntConLevel icl) const {
    Region r(home);
    Int::Linear::Term<typename VarViewTraits<Var>::View>* ts =
      r.alloc<Int::Linear::Term<typename VarViewTraits<Var>::View> >(n->n);
    int c_o = 0;
    int i = n->fill(ts,0,1,0,c_o);
    Int::Linear::post(home, ts, i, irt, -c_o, icl);
  }

  template<class Var>
  inline void
  LinExpr<Var>::post(Space& home, IntRelType irt, const BoolVar& b,
                     IntConLevel icl) const {
    Region r(home);
    Int::Linear::Term<typename VarViewTraits<Var>::View>* ts =
      r.alloc<Int::Linear::Term<typename VarViewTraits<Var>::View> >(n->n);
    int c_o = 0;
    int i = n->fill(ts,0,1,0,c_o);
    Int::Linear::post(home, ts, i, irt, -c_o, b, icl);
  }

  template<>
  inline IntVar
  LinExpr<IntVar>::post(Space& home, IntConLevel icl) const {
    Region r(home);
    Int::Linear::Term<Int::IntView>* ts =
      r.alloc<Int::Linear::Term<Int::IntView> >(n->n+1);
    int c_o = 0;
    int i = n->fill(ts,0,1,0,c_o);
    int min, max;
    Int::Linear::estimate(&ts[0],i,c_o,min,max);
    IntVar x(home, min, max);
    ts[i].x = x; ts[i].a = -1;
    Int::Linear::post(home, ts, i+1, IRT_EQ, -c_o, icl);
    return x;
  }

  template<>
  inline IntVar
  LinExpr<BoolVar>::post(Space& home, IntConLevel icl) const {
    Region r(home);
    Int::Linear::Term<Int::BoolView>* ts =
      r.alloc<Int::Linear::Term<Int::BoolView> >(n->n);
    int c_o = 0;
    int i = n->fill(ts,0,1,0,c_o);
    int min, max;
    Int::Linear::estimate(&ts[0],i,c_o,min,max);
    IntVar x(home, min, max);
    Int::Linear::post(home, ts, i, IRT_EQ, x, -c_o, icl);
    return x;
  }

  inline LinExpr<IntVar>
  operator +(int c, const IntVar& x) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_ADD,c);
  }
  inline LinExpr<IntVar>
  operator +(int c, const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_ADD,c);
  }
  inline LinExpr<IntVar>
  operator +(const IntVar& x, int c) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_ADD,c);
  }
  inline LinExpr<IntVar>
  operator +(const LinExpr<IntVar>& e, int c) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_ADD,c);
  }
  inline LinExpr<IntVar>
  operator +(const IntVar& x, const IntVar& y) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_ADD,y);
  }
  inline LinExpr<IntVar>
  operator +(const IntVar& x, const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_ADD,e);
  }
  inline LinExpr<IntVar>
  operator +(const LinExpr<IntVar>& e, const IntVar& x) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_ADD,x);
  }
  inline LinExpr<IntVar>
  operator +(const LinExpr<IntVar>& e1, const LinExpr<IntVar>& e2) {
    return LinExpr<IntVar>(e1,LinExpr<IntVar>::NT_ADD,e2);
  }

  inline LinExpr<IntVar>
  operator -(int c, const IntVar& x) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_SUB,c);
  }
  inline LinExpr<IntVar>
  operator -(int c, const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_SUB,c);
  }
  inline LinExpr<IntVar>
  operator -(const IntVar& x, int c) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_ADD,-c);
  }
  inline LinExpr<IntVar>
  operator -(const LinExpr<IntVar>& e, int c) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_ADD,-c);
  }
  inline LinExpr<IntVar>
  operator -(const IntVar& x, const IntVar& y) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_SUB,y);
  }
  inline LinExpr<IntVar>
  operator -(const IntVar& x, const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_SUB,e);
  }
  inline LinExpr<IntVar>
  operator -(const LinExpr<IntVar>& e, const IntVar& x) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_SUB,x);
  }
  inline LinExpr<IntVar>
  operator -(const LinExpr<IntVar>& e1, const LinExpr<IntVar>& e2) {
    return LinExpr<IntVar>(e1,LinExpr<IntVar>::NT_SUB,e2);
  }
  inline LinExpr<IntVar>
  operator -(const IntVar& x) {
    return LinExpr<IntVar>(x,LinExpr<IntVar>::NT_SUB,0);
  }
  inline LinExpr<IntVar>
  operator -(const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(e,LinExpr<IntVar>::NT_SUB,0);
  }

  inline LinExpr<IntVar>
  operator *(int a, const IntVar& x) {
    return LinExpr<IntVar>(x,a);
  }
  inline LinExpr<IntVar>
  operator *(const IntVar& x, int a) {
    return LinExpr<IntVar>(x,a);
  }
  inline LinExpr<IntVar>
  operator *(const LinExpr<IntVar>& e, int a) {
    return LinExpr<IntVar>(a,e);
  }
  inline LinExpr<IntVar>
  operator *(int a, const LinExpr<IntVar>& e) {
    return LinExpr<IntVar>(a,e);
  }


  inline LinExpr<BoolVar>
  operator +(int c, const BoolVar& x) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_ADD,c);
  }
  inline LinExpr<BoolVar>
  operator +(int c, const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_ADD,c);
  }
  inline LinExpr<BoolVar>
  operator +(const BoolVar& x, int c) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_ADD,c);
  }
  inline LinExpr<BoolVar>
  operator +(const LinExpr<BoolVar>& e, int c) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_ADD,c);
  }
  inline LinExpr<BoolVar>
  operator +(const BoolVar& x, const BoolVar& y) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_ADD,y);
  }
  inline LinExpr<BoolVar>
  operator +(const BoolVar& x, const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_ADD,e);
  }
  inline LinExpr<BoolVar>
  operator +(const LinExpr<BoolVar>& e, const BoolVar& x) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_ADD,x);
  }
  inline LinExpr<BoolVar>
  operator +(const LinExpr<BoolVar>& e1, const LinExpr<BoolVar>& e2) {
    return LinExpr<BoolVar>(e1,LinExpr<BoolVar>::NT_ADD,e2);
  }

  inline LinExpr<BoolVar>
  operator -(int c, const BoolVar& x) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_SUB,c);
  }
  inline LinExpr<BoolVar>
  operator -(int c, const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_SUB,c);
  }
  inline LinExpr<BoolVar>
  operator -(const BoolVar& x, int c) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_ADD,-c);
  }
  inline LinExpr<BoolVar>
  operator -(const LinExpr<BoolVar>& e, int c) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_ADD,-c);
  }
  inline LinExpr<BoolVar>
  operator -(const BoolVar& x, const BoolVar& y) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_SUB,y);
  }
  inline LinExpr<BoolVar>
  operator -(const BoolVar& x, const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_SUB,e);
  }
  inline LinExpr<BoolVar>
  operator -(const LinExpr<BoolVar>& e, const BoolVar& x) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_SUB,x);
  }
  inline LinExpr<BoolVar>
  operator -(const LinExpr<BoolVar>& e1, const LinExpr<BoolVar>& e2) {
    return LinExpr<BoolVar>(e1,LinExpr<BoolVar>::NT_SUB,e2);
  }
  inline LinExpr<BoolVar>
  operator -(const BoolVar& x) {
    return LinExpr<BoolVar>(x,LinExpr<BoolVar>::NT_SUB,0);
  }
  inline LinExpr<BoolVar>
  operator -(const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(e,LinExpr<BoolVar>::NT_SUB,0);
  }

  inline LinExpr<BoolVar>
  operator *(int a, const BoolVar& x) {
    return LinExpr<BoolVar>(x,a);
  }
  inline LinExpr<BoolVar>
  operator *(const BoolVar& x, int a) {
    return LinExpr<BoolVar>(x,a);
  }
  inline LinExpr<BoolVar>
  operator *(const LinExpr<BoolVar>& e, int a) {
    return LinExpr<BoolVar>(a,e);
  }
  inline LinExpr<BoolVar>
  operator *(int a, const LinExpr<BoolVar>& e) {
    return LinExpr<BoolVar>(a,e);
  }


  forceinline IntVar
  post(Space&, const IntVar& x, IntConLevel) {
    return x;
  }

  inline IntVar
  post(Space& home, int n, IntConLevel) {
    IntVar x(home, n, n);
    return x;
  }

  template<class Var>
  inline IntVar
  post(Space& home, const LinExpr<Var>& e, IntConLevel icl) {
    if (!home.failed())
      return e.post(home,icl);
    IntVar x(home,Int::Limits::min,Int::Limits::max);
    return x;
  }

}

// STATISTICS: minimodel-any
