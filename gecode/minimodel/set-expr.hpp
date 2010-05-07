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

#ifdef GECODE_HAS_SET_VARS

namespace Gecode {

  /*
   * Operations for expressions
   *
   */
  forceinline
  SetExpr::Node::Node(void) : use(1) {}

  forceinline void*
  SetExpr::Node::operator new(size_t size) {
    return heap.ralloc(size);
  }
  forceinline void
  SetExpr::Node::operator delete(void* p, size_t) {
    heap.rfree(p);
  }

  forceinline
  SetExpr::SetExpr(void) : n(NULL) {}

  forceinline
  SetExpr::SetExpr(const SetExpr& e) : n(e.n) {
    n->use++;
  }

  forceinline
  SetExpr::SetExpr(const SetVar& x) : n(new Node) {
    n->same = 1;
    n->t    = NT_VAR;
    n->l    = NULL;
    n->r    = NULL;
    n->x    = x;
  }

  forceinline
  SetExpr::SetExpr(const IntSet& s) : n(new Node) {
    n->same = 1;
    n->t    = NT_CONST;
    n->l    = NULL;
    n->r    = NULL;
    n->s    = s;
  }

  forceinline
  SetExpr::SetExpr(const LinExpr& e) : n(new Node) {
    n->same = 1;
    n->t    = NT_LEXP;
    n->l    = NULL;
    n->r    = NULL;
    n->e    = e;
  }
  
  forceinline bool
  SetExpr::same(NodeType t0, NodeType t1) {
    return (t0==t1) || (t1==NT_VAR) || (t1==NT_CONST) || (t1==NT_LEXP);
  }

  forceinline
  SetExpr::SetExpr(const SetExpr& l, NodeType t, const SetExpr& r)
    : n(new Node) {
    unsigned int ls = same(t,l.n->t) ? l.n->same : 1;
    unsigned int rs = same(t,r.n->t) ? r.n->same : 1;
    n->same = ls+rs;
    n->t    = t;
    n->l    = l.n;
    n->l->use++;
    n->r    = r.n;
    n->r->use++;
  }

  forceinline
  SetExpr::SetExpr(const SetExpr& l, NodeType t) {
    (void) t;
    assert(t == NT_CMPL);
    if (l.n->t == NT_CMPL) {
      n = l.n->l;
      n->use++;
    } else {
      n = new Node;
      n->same = 1;
      n->t    = NT_CMPL;
      n->l    = l.n;
      n->l->use++;
      n->r    = NULL;
    }
  }

  forceinline SetVar
  SetExpr::post(Home home) const {
    Region r(home);
    SetVar s(home,IntSet::empty,
             IntSet(Set::Limits::min,Set::Limits::max));
    NNF::nnf(r,n,false)->post(home,SRT_EQ,s);
    return s;
  }

  forceinline void
  SetExpr::post(Home home, SetRelType srt, const SetExpr& e) const {
    Region r(home);
    return NNF::nnf(r,n,false)->post(home,srt,NNF::nnf(r,e.n,false));
  }
  forceinline void
  SetExpr::post(Home home, BoolVar b, bool t,
                SetRelType srt, const SetExpr& e) const {
    Region r(home);
    return NNF::nnf(r,n,false)->post(home,b,t,srt,NNF::nnf(r,e.n,false));                  
  }

}

#endif

// STATISTICS: minimodel-any
