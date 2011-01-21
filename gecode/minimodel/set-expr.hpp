/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2010
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

  forceinline bool
  SetExpr::same(NodeType t0, NodeType t1) {
    return (t0==t1) || (t1==NT_VAR) || (t1==NT_CONST) || (t1==NT_LEXP);
  }

  inline SetVar
  SetExpr::post(Home home) const {
    Region r(home);
    SetVar s(home,IntSet::empty,
             IntSet(Set::Limits::min,Set::Limits::max));
    NNF::nnf(r,n,false)->post(home,SRT_EQ,s);
    return s;
  }

  inline void
  SetExpr::post(Home home, SetRelType srt, const SetExpr& e) const {
    Region r(home);
    return NNF::nnf(r,n,false)->post(home,srt,NNF::nnf(r,e.n,false));
  }
  inline void
  SetExpr::post(Home home, BoolVar b, bool t,
                SetRelType srt, const SetExpr& e) const {
    Region r(home);
    return NNF::nnf(r,n,false)->post(home,b,t,srt,NNF::nnf(r,e.n,false));                  
  }

}

#endif

// STATISTICS: minimodel-any
