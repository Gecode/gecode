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
  forceinline unsigned int
  ConflictGraph::pos(int i, int j) const {
    return static_cast<unsigned int>(i*(nodes()+1)+j);
  }


  forceinline
  ConflictGraph::Neighbors::Neighbors(const ConflictGraph& cg, int i)
    : c(cg.a.next(cg.pos(i))) {}
  forceinline bool
  ConflictGraph::Neighbors::operator ()(const ConflictGraph& cg, int i) const {
    return c<cg.pos(i,cg.nodes());
  }
  forceinline void
  ConflictGraph::Neighbors::inc(const ConflictGraph& cg, int) {
    c = cg.a.next(c+1);
  }
  forceinline int
  ConflictGraph::Neighbors::val(const ConflictGraph& cg, int i) const {
    return static_cast<int>(c - cg.pos(i));
  }


  forceinline
  ConflictGraph::NodeSet::NodeSet(Region& r, const ConflictGraph& cg)
    : Support::RawBitSetBase(r,cg.nodes()) {}
  forceinline
  ConflictGraph::NodeSet::NodeSet(Region& r, const ConflictGraph& cg, 
                                  const NodeSet& ns)
    : Support::RawBitSetBase(r,cg.nodes(),ns) {}
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
  ConflictGraph::ConflictGraph(Space& h, Region& r,
                               const IntVarArgs& b0, int m)
    : home(h), b(b0), bins(static_cast<unsigned int>(m)),
      d(heap.alloc<unsigned int>(nodes())),
      w(heap.alloc<unsigned int>(nodes())), 
      a(heap,(nodes()+1)*nodes()),
      m(r,*this), cm(0), wm(0) {
    for (int i=nodes(); i--; ) {
      d[i] = 0;
      // Set bits for neighbor iteration sentinel
      a.set(pos(i,nodes()));
    }
  }

  forceinline
  ConflictGraph::~ConflictGraph(void) {
    heap.free<unsigned int>(d,nodes());
    heap.free<unsigned int>(w,nodes()); 
    a.dispose(heap,(nodes()+1)*nodes());
  }

  forceinline void
  ConflictGraph::edge(int i, int j, bool add) {
    if (add) {
      assert(!a.get(pos(i,j)) && !a.get(pos(j,i)));
      a.set(pos(i,j)); d[i]++;
      a.set(pos(j,i)); d[j]++;
    } else {
      assert(a.get(pos(i,j)) && a.get(pos(j,i)));
      a.clear(pos(i,j)); d[i]--;
      a.clear(pos(j,i)); d[j]--;
    }
  }

  forceinline bool
  ConflictGraph::adjacent(int i, int j) const {
    return a.get(pos(i,j));
  }

  forceinline bool 
  ConflictGraph::iwn(NodeSet& iwn, const NodeSet& n, int i) {
    Neighbors ii(*this,i);
    Nodes nn(*this,n);
    bool empty = true;
    while (ii(*this,i) && nn(*this,n)) {
      if (ii.val(*this,i) < nn.val(*this,n)) {
        ii.inc(*this,i);
      } else if (nn.val(*this,n) < ii.val(*this,i)) {
        nn.inc(*this,n);
      } else {
        iwn.incl(nn.val(*this,n)); ii.inc(*this,i); nn.inc(*this,n);
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
    unsigned int m = d[p];
    while (i(*this,a)) {
      if (d[i.val(*this,a)] > m) {
        m = d[i.val(*this,a)]; p = i.val(*this,a);
      }
      i.inc(*this,a);
    }
    while (j(*this,b)) {
      if (d[j.val(*this,b)] > m) {
        m = d[j.val(*this,b)]; p = j.val(*this,b);
      }
      j.inc(*this,b);
    }
    return p;
  }

  forceinline ExecStatus
  ConflictGraph::clique(const NodeSet& r, unsigned int cr, unsigned int wr) {
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
        w[i]=d[i];
        if ((d[i] == 1) || (d[i] == 2))
          n.push(i);
      }
      while (!n.empty()) {
        int i = n.pop();
        switch (d[i]) {
        case 0:
          // Might happen as the edges have already been removed
          break;
        case 1: {
          Neighbors ii(*this,i);
          int j=ii.val(*this,i);
          GECODE_ES_CHECK(clique(i,j,w[i]+w[j]));
          // Remove edge
          edge(i,j,false);
          if ((d[j] == 1) || (d[j] == 2))
            n.push(j);
          break;
        }
        case 2: {
          Neighbors ii(*this,i);
          int j=ii.val(*this,i); ii.inc(*this,i);
          int k=ii.val(*this,i);
          if (adjacent(j,k)) {
            GECODE_ES_CHECK(clique(i,j,k,w[i]+w[j]+w[k]));
            // Can the edge j-k still be member of another clique?
            if ((d[j] == 2) || (d[k] == 2))
              edge(j,k,false);
          } else {
            GECODE_ES_CHECK(clique(i,j,w[i]+w[j]));
            GECODE_ES_CHECK(clique(i,k,w[i]+w[k]));
          }
          edge(i,j,false);
          edge(i,k,false);
          if ((d[j] == 1) || (d[j] == 2))
            n.push(j);
          if ((d[k] == 1) || (d[k] == 2))
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
      NodeSet r(reg,*this), p(reg,*this), x(reg,*this);
      bool empty = true;
      for (int i=nodes(); i--; )
        if (d[i] > 0) {
          p.incl(i); empty = false;
        } else {
          // Report clique of size 1
          GECODE_ES_CHECK(clique(i));
        }
      if (empty)
        return ES_OK;
      GECODE_ES_CHECK(bk(r,0,0,p,x));
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

