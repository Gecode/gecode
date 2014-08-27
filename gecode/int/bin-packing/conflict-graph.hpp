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
  ConflictGraph::NodeSet::NodeSet(Region& r, int n)
    : Support::RawBitSetBase(r,static_cast<unsigned int>(n)) {}
  forceinline
  ConflictGraph::NodeSet::NodeSet(Region& r, int n, 
                                  const NodeSet& ns)
    : Support::RawBitSetBase(r,static_cast<unsigned int>(n),ns) {}
  forceinline void
  ConflictGraph::NodeSet::init(Region& r, int n) {
    Support::RawBitSetBase::init(r,static_cast<unsigned int>(n));
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
  ConflictGraph::NodeSet::copy(int n, const NodeSet& ns) {
    RawBitSetBase::copy(static_cast<unsigned int>(n),ns);
  }
  forceinline void
  ConflictGraph::NodeSet::empty(int n) {
    RawBitSetBase::clearall(static_cast<unsigned int>(n));
  }

  forceinline
  ConflictGraph::Node::Node(void) {}

  forceinline
  ConflictGraph::Nodes::Nodes(const NodeSet& ns0)
    : ns(ns0), c(ns.next(0)) {}
  forceinline void
  ConflictGraph::Nodes::operator ++(void) {
    c = ns.next(c+1);
  }
  forceinline int
  ConflictGraph::Nodes::operator ()(void) const {
    return static_cast<int>(c);
  }



  forceinline
  ConflictGraph::ConflictGraph(Space& h, Region& reg,
                               const IntVarArgs& b0, int m)
    : home(h), b(b0), 
      bins(static_cast<unsigned int>(m)),
      node(heap.alloc<Node>(nodes())),
      r(reg,nodes()), cr(0), wr(0),
      m(reg,nodes()), cm(0), wm(0) {
    for (int i=nodes(); i--; ) {
      node[i].n.init(reg,nodes());
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
    Nodes ii(node[i].n), nn(n);
    bool empty = true;
    while ((ii() < nodes()) && (nn() < nodes())) {
      if (ii() < nn()) {
        ++ii;
      } else if (nn() < ii()) {
        ++nn;
      } else {
        iwn.incl(nn()); ++ii; ++nn;
        empty = false;
      }
    }
    return empty;
  }

  forceinline int
  ConflictGraph::pivot(const NodeSet& a, const NodeSet& b) const {
    Nodes i(a), j(b);
    assert((i() < nodes()) || (j() < nodes()));
    int p;
    if (i() < nodes()) {
      p = i(); ++i;
    } else {
      p = j(); ++j;
    }
    unsigned int m = node[p].d;
    while (i() < nodes()) {
      if (node[i()].d > m) {
        p=i(); m=node[p].d; 
      }
      ++i;
    }
    while (j() < nodes()) {
      if (node[j()].d > m) {
        p=j(); m=node[p].d; 
      }
      ++j;
    }
    return p;
  }

  forceinline ExecStatus
  ConflictGraph::clique(void) {
    // Remember clique
    if ((cr > cm) || ((cr == cm) && (wr > wm))) {
      m.copy(nodes(),r); cm=cr; wm=wr;
      if (cm > bins)
        return ES_FAILED;
    }
    // Compute corresponding variables
    ViewArray<IntView> bv(home,cr);
    int i=0;
    for (Nodes c(r); c() < nodes(); ++c)
      bv[i++] = b[c()];
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
      m.empty(nodes()); m.incl(i); m.incl(j); cm=2; wm=w;
      if (cm > bins)
        return ES_FAILED;
    }
    return Rel::Nq<IntView>::post(home,b[i],b[j]);
  }

  forceinline ExecStatus
  ConflictGraph::clique(int i, int j, int k, unsigned int w) {
    if ((3 > cm) || ((3 == cm) && (w > wm))) {
      m.empty(nodes()); m.incl(i); m.incl(j); m.incl(k); cm=3; wm=w;
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
          Nodes ii(node[i].n);
          int j=ii();
          GECODE_ES_CHECK(clique(i,j,node[i].w+node[j].w));
          // Remove edge
          edge(i,j,false);
          if ((node[j].d == 1) || (node[j].d == 2))
            n.push(j);
          break;
        }
        case 2: {
          Nodes ii(node[i].n);
          int j=ii(); ++ii;
          int k=ii();
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
      NodeSet p(reg,nodes()), x(reg,nodes());
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
    for (Nodes i(m); i() < nodes(); ++i)
      n[j++]=i();
    assert(j == static_cast<int>(cm));
    IntSet s(n,cm);
    return s;
  }

}}}

// STATISTICS: int-prop

