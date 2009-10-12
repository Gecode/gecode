/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

  /*
   * Operations for nodes
   *
   */
  forceinline void*
  BoolExpr::Node::operator new(size_t size) {
    return heap.ralloc(size);
  }

  forceinline void
  BoolExpr::Node::operator delete(void* p, size_t) {
    heap.rfree(p);
  }

  forceinline
  BoolExpr::Node::Node(void) : use(1) {}

  bool
  BoolExpr::Node::decrement(void) {
    if (--use == 0) {
      if ((l != NULL) && l->decrement())
        delete l;
      if ((r != NULL) && r->decrement())
        delete r;
      return true;
    }
    return false;
  }

  BoolExpr::BoolExpr(const BoolVar& x) : n(new Node) {
    n->same = 1;
    n->t    = NT_VAR;
    n->l    = NULL;
    n->r    = NULL;
    n->x    = x;
  }

  BoolExpr::BoolExpr(const BoolExpr& l, NodeType t, const BoolExpr& r)
    : n(new Node) {
    unsigned int ls = ((l.n->t == t) || (l.n->t == NT_VAR)) ? l.n->same : 1;
    unsigned int rs = ((r.n->t == t) || (r.n->t == NT_VAR)) ? r.n->same : 1;
    n->same = ls+rs;
    n->t    = t;
    n->l    = l.n;
    n->l->use++;
    n->r    = r.n;
    n->r->use++;
  }

  BoolExpr::BoolExpr(const BoolExpr& l, NodeType t) {
    (void) t;
    assert(t == NT_NOT);
    if (l.n->t == NT_NOT) {
      n = l.n->l;
      n->use++;
    } else {
      n = new Node;
      n->same = 1;
      n->t    = NT_NOT;
      n->l    = l.n;
      n->l->use++;
      n->r    = NULL;
    }
  }

  BoolExpr::BoolExpr(const LinRel<IntVar>& rl)
    : n(new Node) {
    n->same   = 1;
    n->t      = NT_RLIN_INT;
    n->l      = NULL;
    n->r      = NULL;
    n->rl_int = rl;
  }

  BoolExpr::BoolExpr(const LinRel<BoolVar>& rl)
    : n(new Node) {
    n->same    = 1;
    n->t       = NT_RLIN_BOOL;
    n->l       = NULL;
    n->r       = NULL;
    n->rl_bool = rl;
  }

  const BoolExpr&
  BoolExpr::operator =(const BoolExpr& e) {
    if (this != &e) {
      if (n->decrement())
        delete n;
      n = e.n;
      n->use++;
    }
    return *this;
  }

  BoolExpr::~BoolExpr(void) {
    if (n->decrement())
      delete n;
  }

  /*
   * Operations for negation normalform
   *
   */
  forceinline void
  BoolExpr::NNF::operator delete(void*) {}

  forceinline void
  BoolExpr::NNF::operator delete(void*, Region&) {}

  forceinline void*
  BoolExpr::NNF::operator new(size_t s, Region& r) {
    return r.ralloc(s);
  }

  BoolVar
  BoolExpr::NNF::post(Home home, IntConLevel icl) const {
    if ((t == NT_VAR) && !u.a.neg)
      return u.a.x->x;
    BoolVar b(home,0,1);
    switch (t) {
    case NT_VAR:
      assert(u.a.neg);
      rel(home, u.a.x->x, IRT_NQ, b);
      break;
    case NT_RLIN_INT:
      u.a.x->rl_int.post(home, b, !u.a.neg, icl);
      break;
    case NT_RLIN_BOOL:
      u.a.x->rl_bool.post(home, b, !u.a.neg, icl);
      break;
    case NT_AND:
      {
        BoolVarArgs bp(p), bn(n);
        int ip=0, in=0;
        post(home, NT_AND, bp, bn, ip, in, icl);
        clause(home, BOT_AND, bp, bn, b);
      }
      break;
    case NT_OR:
      {
        BoolVarArgs bp(p), bn(n);
        int ip=0, in=0;
        post(home, NT_OR, bp, bn, ip, in, icl);
        clause(home, BOT_OR, bp, bn, b);
      }
      break;
    case NT_EQV:
      {
        bool n = false;
        BoolVar l;
        if (u.b.l->t == NT_VAR) {
          l = u.b.l->u.a.x->x;
          if (u.b.l->u.a.neg) n = !n;
        } else {
          l = u.b.l->post(home,icl);
        }
        BoolVar r;
        if (u.b.r->t == NT_VAR) {
          r = u.b.r->u.a.x->x;
          if (u.b.r->u.a.neg) n = !n;
        } else {
          r = u.b.r->post(home,icl);
        }
        rel(home, l, n ? BOT_XOR : BOT_EQV, r, b, icl);
      }
      break;
    default:
      GECODE_NEVER;
    }
    return b;
  }

  void
  BoolExpr::NNF::post(Home home, NodeType t,
                      BoolVarArgs& bp, BoolVarArgs& bn,
                      int& ip, int& in,
                      IntConLevel icl) const {
    if (this->t != t) {
      switch (this->t) {
      case NT_VAR:
        if (u.a.neg) {
          bn[in++]=u.a.x->x;
        } else {
          bp[ip++]=u.a.x->x;
        }
        break;
      case NT_RLIN_INT:
        {
          BoolVar b(home,0,1);
          u.a.x->rl_int.post(home, b, !u.a.neg, icl);
          bp[ip++]=b;
        }
        break;
      case NT_RLIN_BOOL:
        {
          BoolVar b(home,0,1);
          u.a.x->rl_bool.post(home, b, !u.a.neg, icl);
          bp[ip++]=b;
        }
        break;
      default:
        bp[ip++] = post(home, icl);
        break;
      }
    } else {
      u.b.l->post(home, t, bp, bn, ip, in, icl);
      u.b.r->post(home, t, bp, bn, ip, in, icl);
    }
  }

  void
  BoolExpr::NNF::post(Home home, bool b, IntConLevel icl) const {
    if (b) {
      switch (t) {
      case NT_VAR:
        rel(home, u.a.x->x, IRT_EQ, u.a.neg ? 0 : 1);
        break;
      case NT_RLIN_INT:
        u.a.x->rl_int.post(home, !u.a.neg, icl);
        break;
      case NT_RLIN_BOOL:
        u.a.x->rl_bool.post(home, !u.a.neg, icl);
        break;
      case NT_AND:
        u.b.l->post(home, true, icl);
        u.b.r->post(home, true, icl);
        break;
      case NT_OR:
        {
          BoolVarArgs bp(p), bn(n);
          int ip=0, in=0;
          post(home, NT_OR, bp, bn, ip, in, icl);
          clause(home, BOT_OR, bp, bn, 1);
        }
        break;
      case NT_EQV:
        rel(home, post(home, icl), IRT_EQ, 1);
        break;
      default:
        GECODE_NEVER;
      }
    } else {
      switch (t) {
      case NT_VAR:
        rel(home, u.a.x->x, IRT_EQ, u.a.neg ? 1 : 0);
        break;
      case NT_RLIN_INT:
        u.a.x->rl_int.post(home, u.a.neg, icl);
        break;
      case NT_RLIN_BOOL:
        u.a.x->rl_bool.post(home, u.a.neg, icl);
        break;
      case NT_AND:
        {
          BoolVarArgs bp(p), bn(n);
          int ip=0, in=0;
          post(home, NT_AND, bp, bn, ip, in, icl);
          clause(home, BOT_AND, bp, bn, 0);
        }
        break;
      case NT_OR:
        u.b.l->post(home, false, icl);
        u.b.r->post(home, false, icl);
        break;
      case NT_EQV:
        rel(home, post(home, icl), IRT_EQ, 0);
        break;
      default:
        GECODE_NEVER;
      }
    }
  }

  BoolExpr::NNF*
  BoolExpr::NNF::nnf(Region& r, Node* n, bool neg) {
    switch (n->t) {
    case NT_VAR: case NT_RLIN_INT: case NT_RLIN_BOOL:
      {
        NNF* x = new (r) NNF;
        x->t = n->t; x->u.a.neg = neg; x->u.a.x = n;
        if (neg) {
          x->p = 0; x->n = 1;
        } else {
          x->p = 1; x->n = 0;
        }
        return x;
      }
    case NT_NOT:
      return nnf(r,n->l,!neg);
    case NT_AND: case NT_OR:
      {
        NodeType t = ((n->t == NT_AND) == neg) ? NT_OR : NT_AND;
        NNF* x = new (r) NNF;
        x->t = t;
        x->u.b.l = nnf(r,n->l,neg);
        x->u.b.r = nnf(r,n->r,neg);
        unsigned int p_l, n_l;
        if ((x->u.b.l->t == t) || (x->u.b.l->t == NT_VAR)) {
          p_l=x->u.b.l->p; n_l=x->u.b.l->n;
        } else {
          p_l=1; n_l=0;
        }
        unsigned int p_r, n_r;
        if ((x->u.b.r->t == t) || (x->u.b.r->t == NT_VAR)) {
          p_r=x->u.b.r->p; n_r=x->u.b.r->n;
        } else {
          p_r=1; n_r=0;
        }
        x->p = p_l+p_r;
        x->n = n_l+n_r;
        return x;
      }
    case NT_EQV:
      {
        NNF* x = new (r) NNF;
        x->t = NT_EQV;
        x->u.b.l = nnf(r,n->l,neg);
        x->u.b.r = nnf(r,n->r,false);
        x->p = 2; x->n = 0;
        return x;
      }
    default:
      GECODE_NEVER;
    }
    GECODE_NEVER;
    return NULL;
  }

}

// STATISTICS: minimodel-any
