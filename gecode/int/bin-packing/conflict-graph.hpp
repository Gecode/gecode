/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Stefano Gualandi <stefano.gualandi@gmail.com>
 *
 *  Copyright:
 *     Stefano Gualandi, 2013
 *     Christian Schulte, 2014
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

#include <gecode/int/rel.hh>
#include <gecode/int/distinct.hh>

namespace Gecode { namespace Int { namespace BinPacking {

  forceinline int
  ConflictGraph::nodes(void) const {
    return b.size();
  }

  forceinline
  ConflictGraph::NodeSet::NodeSet(void) {}
  forceinline
  ConflictGraph::NodeSet::NodeSet(Region& r, const ConflictGraph& cg)
    : Support::RawBitSetBase(r,cg.nodes()) {}
  forceinline
  ConflictGraph::NodeSet::NodeSet(Region& r, const ConflictGraph& cg, 
                                  const NodeSet& ns)
    : Support::RawBitSetBase(r,cg.nodes(),ns) {}
  forceinline void
  ConflictGraph::NodeSet::init(Region& r, const ConflictGraph& cg) {
    Support::RawBitSetBase::init(r,cg.nodes());
  }
  forceinline bool
  ConflictGraph::NodeSet::in(int i) const {
    return RawBitSetBase::get(static_cast<unsigned int>(i));
  }
  forceinline void
  ConflictGraph::NodeSet::incl(int i) {
    RawBitSetBase::set(static_cast<unsigned int>(i));
  }
  forceinline void
  ConflictGraph::NodeSet::excl(int i) {
    RawBitSetBase::clear(static_cast<unsigned int>(i));
  }
  forceinline void
  ConflictGraph::NodeSet::copy(const ConflictGraph& cg, const NodeSet& ns) {
    RawBitSetBase::copy(static_cast<unsigned int>(cg.nodes()),ns);
  }
  forceinline void
  ConflictGraph::NodeSet::empty(const ConflictGraph& cg) {
    RawBitSetBase::clearall(static_cast<unsigned int>(cg.nodes()));
  }

  forceinline
  ConflictGraph::Node::Node(void) {}

  forceinline
  ConflictGraph::Nodes::Nodes(const ConflictGraph&, const NodeSet& ns)
    : c(ns.next(0)) {}
  forceinline bool
  ConflictGraph::Nodes::operator ()(const ConflictGraph& cg, 
                                    const NodeSet&) const {
    return c<static_cast<unsigned int>(cg.nodes());
  }
  forceinline void
  ConflictGraph::Nodes::inc(const ConflictGraph&,
                            const NodeSet& ns) {
    c = ns.next(c+1);
  }
  forceinline int
  ConflictGraph::Nodes::val(const ConflictGraph&,
                            const NodeSet&) const {
    return static_cast<int>(c);
  }



  forceinline
  ConflictGraph::ConflictGraph(Space& h, Region& reg,
                               const IntVarArgs& b0, int m)
    : home(h), b(b0), 
      bins(static_cast<unsigned int>(m)),
      node(heap.alloc<Node>(nodes())),
      r(reg,*this), cr(0), wr(0),
      m(reg,*this), cm(0), wm(0) {
    for (int i=nodes(); i--; ) {
      node[i].n.init(reg,*this);
      node[i].d=node[i].w=0;
    }
  }

  forceinline
  ConflictGraph::~ConflictGraph(void) {
    heap.free<Node>(node,nodes());
  }

  forceinline void
  ConflictGraph::edge(int i, int j, bool add) {
    if (add) {
      assert(!node[i].n.in(j) && !node[j].n.in(i));
      node[i].n.incl(j); node[i].d++; node[i].w++;
      node[j].n.incl(i); node[j].d++; node[j].w++;
    } else {
      assert(node[i].n.in(j) && node[j].n.in(i));
      node[i].n.excl(j); node[i].d--;
      node[j].n.excl(i); node[j].d--;
    }
  }

  forceinline bool
  ConflictGraph::adjacent(int i, int j) const {
    assert(node[i].n.in(j) == node[j].n.in(i));
    return node[i].n.in(j);
  }

  forceinline bool 
  ConflictGraph::iwn(NodeSet& iwn, const NodeSet& n, int i) {
    Nodes ii(*this,node[i].n);
    Nodes nn(*this,n);
    bool empty = true;
    while (ii(*this,node[i].n) && nn(*this,n)) {
      if (ii.val(*this,node[i].n) < nn.val(*this,n)) {
        ii.inc(*this,node[i].n);
      } else if (nn.val(*this,n) < ii.val(*this,node[i].n)) {
        nn.inc(*this,n);
      } else {
        iwn.incl(nn.val(*this,n)); ii.inc(*this,node[i].n); nn.inc(*this,n);
        empty = false;
      }
    }
    return empty;
  }

  forceinline int
  ConflictGraph::pivot(const NodeSet& a, const NodeSet& b) const {
    Nodes i(*this,a), j(*this,b);
    assert(i(*this,a) || j(*this,b));
    int p;
    if (i(*this,a)) {
      p = i.val(*this,a); i.inc(*this,a);
    } else {
      p = j.val(*this,b); j.inc(*this,b);
    }
    unsigned int m = node[p].d;
    while (i(*this,a)) {
      if (node[i.val(*this,a)].d > m) {
        m = node[i.val(*this,a)].d; p = i.val(*this,a);
      }
      i.inc(*this,a);
    }
    while (j(*this,b)) {
      if (node[j.val(*this,b)].d > m) {
        m = node[j.val(*this,b)].d; p = j.val(*this,b);
      }
      j.inc(*this,b);
    }
    return p;
  }

  forceinline ExecStatus
  ConflictGraph::clique(void) {
    // Remember clique
    if ((cr > cm) || ((cr == cm) && (wr > wm))) {
      m.copy(*this,r); cm=cr; wm=wr;
      if (cm > bins)
        return ES_FAILED;
    }
    // Compute corresponding variables
    ViewArray<IntView> bv(home,cr);
    int i=0;
    for (Nodes c(*this,r); c(*this,r); c.inc(*this,r))
      bv[i++] = b[c.val(*this,r)];
    assert(i == static_cast<int>(cr));
    return Distinct::Dom<IntView>::post(home,bv);
  }

  forceinline ExecStatus
  ConflictGraph::clique(int i) {
    if (1 > cm) {
      assert(m.none(nodes()));
      m.incl(i); cm=1; wm=0;
    }
    return ES_OK;
  }

  forceinline ExecStatus
  ConflictGraph::clique(int i, int j, unsigned int w) {
    if ((2 > cm) || ((2 == cm) && (w > wm))) {
      m.empty(*this); m.incl(i); m.incl(j); cm=2; wm=w;
      if (cm > bins)
        return ES_FAILED;
    }
    return Rel::Nq<IntView>::post(home,b[i],b[j]);
  }

  forceinline ExecStatus
  ConflictGraph::clique(int i, int j, int k, unsigned int w) {
    if ((3 > cm) || ((3 == cm) && (w > wm))) {
      m.empty(*this); m.incl(i); m.incl(j); m.incl(k); cm=3; wm=w;
      if (cm > bins)
        return ES_FAILED;
    }
    // Compute corresponding variables
    ViewArray<IntView> bv(home,3);
    bv[0]=b[i]; bv[1]=b[j]; bv[2]=b[k];
    return Distinct::Dom<IntView>::post(home,bv);
  }

  forceinline ExecStatus
  ConflictGraph::post(void) {
    // Find some simple cliques of sizes 2 and 3.
    {
      Region reg(home);
      // Nodes can be entered twice (for degree 2 and later for degree 1)
      Support::StaticStack<int,Region> n(reg,2*nodes());
      // Make a copy of the degree information to be used as weights
      // and record all nodes of degree 1 and 2.
      for (int i=nodes(); i--; ) {
        if ((node[i].d == 1) || (node[i].d == 2))
          n.push(i);
      }
      while (!n.empty()) {
        int i = n.pop();
        switch (node[i].d) {
        case 0:
          // Might happen as the edges have already been removed
          break;
        case 1: {
          Nodes ii(*this,node[i].n);
          int j=ii.val(*this,node[i].n);
          GECODE_ES_CHECK(clique(i,j,node[i].w+node[j].w));
          // Remove edge
          edge(i,j,false);
          if ((node[j].d == 1) || (node[j].d == 2))
            n.push(j);
          break;
        }
        case 2: {
          Nodes ii(*this,node[i].n);
          int j=ii.val(*this,node[i].n); ii.inc(*this,node[i].n);
          int k=ii.val(*this,node[i].n);
          if (adjacent(j,k)) {
            GECODE_ES_CHECK(clique(i,j,k,
                                   node[i].w+node[j].w+node[k].w));
            // Can the edge j-k still be member of another clique?
            if ((node[j].d == 2) || (node[k].d == 2))
              edge(j,k,false);
          } else {
            GECODE_ES_CHECK(clique(i,j,node[i].w+node[j].w));
            GECODE_ES_CHECK(clique(i,k,node[i].w+node[k].w));
          }
          edge(i,j,false);
          edge(i,k,false);
          if ((node[j].d == 1) || (node[j].d == 2))
            n.push(j);
          if ((node[k].d == 1) || (node[k].d == 2))
            n.push(k);
          break;
        }
        default: GECODE_NEVER;
        }
      }
    }
    // Initialize for Bron-Kerbosch
    {
      Region reg(home);
      NodeSet p(reg,*this), x(reg,*this);
      bool empty = true;
      for (int i=nodes(); i--; )
        if (node[i].d > 0) {
          p.incl(i); empty = false;
        } else {
          // Report clique of size 1
          GECODE_ES_CHECK(clique(i));
        }
      if (empty)
        return ES_OK;
      GECODE_ES_CHECK(bk(p,x));
    }
    return ES_OK;
  }

  forceinline IntSet
  ConflictGraph::maxclique(void) const {
    Region reg(home);
    int* n=reg.alloc<int>(cm);
    int j=0;
    for (Nodes i(*this,m); i(*this,m); i.inc(*this,m))
      n[j++]=i.val(*this,m);
    assert(j == static_cast<int>(cm));
    IntSet s(n,cm);
    return s;
  }

}}}

// STATISTICS: int-prop

