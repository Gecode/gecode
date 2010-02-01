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


  /*
   * Operators
   *
   */
  inline LinExpr
  operator +(int c, const IntVar& x) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(int c, const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(const IntVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(const BoolVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,c);
  }
  inline LinExpr
  operator +(const IntVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  inline LinExpr
  operator +(const IntVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  inline LinExpr
  operator +(const BoolVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  inline LinExpr
  operator +(const BoolVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_ADD,y);
  }
  inline LinExpr
  operator +(const IntVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_ADD,e);
  }
  inline LinExpr
  operator +(const BoolVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_ADD,e);
  }
  inline LinExpr
  operator +(const LinExpr& e, const IntVar& x) {
    return LinExpr(e,LinExpr::NT_ADD,x);
  }
  inline LinExpr
  operator +(const LinExpr& e, const BoolVar& x) {
    return LinExpr(e,LinExpr::NT_ADD,x);
  }
  inline LinExpr
  operator +(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_ADD,e2);
  }

  inline LinExpr
  operator -(int c, const IntVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,c);
  }
  inline LinExpr
  operator -(int c, const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,c);
  }
  inline LinExpr
  operator -(int c, const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,c);
  }
  inline LinExpr
  operator -(const IntVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  inline LinExpr
  operator -(const BoolVar& x, int c) {
    return LinExpr(x,LinExpr::NT_ADD,-c);
  }
  inline LinExpr
  operator -(const LinExpr& e, int c) {
    return LinExpr(e,LinExpr::NT_ADD,-c);
  }
  inline LinExpr
  operator -(const IntVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  inline LinExpr
  operator -(const IntVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  inline LinExpr
  operator -(const BoolVar& x, const IntVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  inline LinExpr
  operator -(const BoolVar& x, const BoolVar& y) {
    return LinExpr(x,LinExpr::NT_SUB,y);
  }
  inline LinExpr
  operator -(const IntVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_SUB,e);
  }
  inline LinExpr
  operator -(const BoolVar& x, const LinExpr& e) {
    return LinExpr(x,LinExpr::NT_SUB,e);
  }
  inline LinExpr
  operator -(const LinExpr& e, const IntVar& x) {
    return LinExpr(e,LinExpr::NT_SUB,x);
  }
  inline LinExpr
  operator -(const LinExpr& e, const BoolVar& x) {
    return LinExpr(e,LinExpr::NT_SUB,x);
  }
  inline LinExpr
  operator -(const LinExpr& e1, const LinExpr& e2) {
    return LinExpr(e1,LinExpr::NT_SUB,e2);
  }

  inline LinExpr
  operator -(const IntVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,0);
  }
  inline LinExpr
  operator -(const BoolVar& x) {
    return LinExpr(x,LinExpr::NT_SUB,0);
  }
  inline LinExpr
  operator -(const LinExpr& e) {
    return LinExpr(e,LinExpr::NT_SUB,0);
  }

  inline LinExpr
  operator *(int a, const IntVar& x) {
    return LinExpr(x,a);
  }
  inline LinExpr
  operator *(int a, const BoolVar& x) {
    return LinExpr(x,a);
  }
  inline LinExpr
  operator *(const IntVar& x, int a) {
    return LinExpr(x,a);
  }
  inline LinExpr
  operator *(const BoolVar& x, int a) {
    return LinExpr(x,a);
  }
  inline LinExpr
  operator *(const LinExpr& e, int a) {
    return LinExpr(a,e);
  }
  inline LinExpr
  operator *(int a, const LinExpr& e) {
    return LinExpr(a,e);
  }

  inline LinExpr
  sum(const IntVarArgs& x) {
    return LinExpr(x);
  }
  inline LinExpr
  sum(const IntArgs& a, const IntVarArgs& x) {
    return LinExpr(a,x);
  }
  inline LinExpr
  sum(const BoolVarArgs& x) {
    return LinExpr(x);
  }
  inline LinExpr
  sum(const IntArgs& a, const BoolVarArgs& x) {
    return LinExpr(a,x);
  }

  forceinline IntVar
  post(Home, const IntVar& x, IntConLevel) {
    return x;
  }

  inline IntVar
  post(Home home, int n, IntConLevel) {
    IntVar x(home, n, n);
    return x;
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
