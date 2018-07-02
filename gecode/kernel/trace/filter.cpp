/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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

#include <gecode/kernel.hh>

namespace Gecode {

  /*
   * Trace filter expressions
   *
   */
  bool
  TFE::Node::decrement(void) {
    if (--use == 0) {
      if ((l != NULL) && l->decrement())
        delete l;
      if ((r != NULL) && r->decrement())
        delete r;
      return true;
    }
    return false;
  }


  forceinline void
  TFE::init(Group g, char what) {
    n = new Node;
    n->t = NT_GROUP;
    n->g = g;
    n->n = 1;
    n->w = what;
  }

  inline TFE
  TFE::negate(void) const {
    Node* m = new Node;
    m->t = NT_NEGATE;
    m->n = n->n;
    m->l = n; n->use++;
    return TFE(m);
  }

  TFE::TFE(PropagatorGroup g) {
    init(g,(1 << ViewTraceInfo::PROPAGATOR) | (1 << ViewTraceInfo::POST));
  }

  TFE::TFE(BrancherGroup g) {
    init(g,(1 << ViewTraceInfo::BRANCHER));
  }

  TFE
  TFE::other(void) {
    TFE e;
    e.init(Group::all,(1 << ViewTraceInfo::OTHER));
    return e;
  }

  TFE::TFE(const TFE& e) : n(e.n) {
    n->use++;
  }

  TFE&
  TFE::operator =(const TFE& e) {
    if (&e != this) {
      if (n->decrement())
        delete n;
      n = e.n;
      n->use++;
    }
    return *this;
  }

  TFE&
  TFE::operator +=(const TFE& e) {
    Node* a = new Node;
    a->t = NT_ADD;
    a->n = n->n + e.n->n;
    a->l = n;
    a->r = e.n; e.n->use++;
    n = a;
    return *this;
  }

  TFE&
  TFE::operator -=(const TFE& e) {
    return operator +=(e.negate());
  }

  TFE::~TFE(void) {
    if (n->decrement())
      delete n;
  }


  TFE
  operator -(const TFE& e) {
    return e.negate();
  }

  TFE
  propagator(PropagatorGroup g) {
    TFE e;
    e.init(g,(1 << ViewTraceInfo::PROPAGATOR));
    return e;
  }

  TFE
  post(PropagatorGroup g) {
    TFE e;
    e.init(g,(1 << ViewTraceInfo::POST));
    return e;
  }



  /*
   * Trace filters
   *
   */


  forceinline
  TraceFilter::TFO::StackFrame::StackFrame(void) {}
  forceinline
  TraceFilter::TFO::StackFrame::StackFrame(TFE::Node* n0, bool neg0)
    : n(n0), neg(neg0) {}

  void
  TraceFilter::TFO::fill(TFE::Node* n) {
    Region region;
    Support::DynamicStack<StackFrame,Region> next(region);
    int i=0;
    next.push(StackFrame(n,false));
    do {
      StackFrame s = next.pop();
      switch (s.n->t) {
      case TFE::NT_GROUP:
        f[i].g = s.n->g; f[i].neg = s.neg; f[i].what=s.n->w;
        i++;
        break;
      case TFE::NT_NEGATE:
        next.push(StackFrame(s.n->l,!s.neg));
        break;
      case TFE::NT_ADD:
        next.push(StackFrame(s.n->l,s.neg));
        next.push(StackFrame(s.n->r,s.neg));
        break;
      default: GECODE_NEVER;
      }
    } while (!next.empty());
  }

  TraceFilter::TFO::~TFO(void) {
    heap.free<Filter>(f,n);
  }

  TraceFilter::TraceFilter(void) : SharedHandle(new TFO) {}

  TraceFilter::TraceFilter(const TFE& e) : SharedHandle(new TFO(e)) {}

  TraceFilter::TraceFilter(PropagatorGroup g) : SharedHandle(new TFO(g)) {}

  TraceFilter::TraceFilter(BrancherGroup g) : SharedHandle(new TFO(g)) {}

  TraceFilter::TraceFilter(const TraceFilter& tf) : SharedHandle(tf) {}

  TraceFilter&
  TraceFilter::operator =(const TraceFilter& tf) {
    return static_cast<TraceFilter&>(SharedHandle::operator =(tf));
  }

  TraceFilter TraceFilter::all;

}

// STATISTICS: kernel-trace
