/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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

#include <gecode/int/bin-packing.hh>

namespace Gecode { namespace Int { namespace BinPacking {

  ExecStatus
  ConflictGraph::bk(NodeSet& r, unsigned int cr, unsigned int wr,
                    NodeSet& p, NodeSet& x) {
    assert(!(p.none(nodes()) && x.none(nodes())));
    // Iterate over neighbors of pivot node
    int piv = pivot(p,x);
    Neighbors n(*this,piv);
    // Iterate over elements of p 
    Nodes i(*this,p);
    // The loop iterates over elements in i - n
    while (i(*this,p)) {
      int iv = i.val(*this,p);
      int nv = n.val(*this,piv);
      if (n(*this,piv) && (iv == nv)) {
        i.inc(*this,p); n.inc(*this,piv);
      } else if (n(*this,piv) && (iv > nv)) {
        n.inc(*this,piv);
      } else {
        i.inc(*this,p); n.inc(*this,piv);

        Region reg(home);

        // Found i.val() to be in i - n
        NodeSet np(reg,*this);
        bool npe = iwn(np,p,iv); p.excl(iv);
        NodeSet nx(reg,*this);
        bool nxe = iwn(nx,x,iv); x.incl(iv);

        r.incl(iv);
        if (npe && nxe) {
          // Found a max clique
          GECODE_ES_CHECK(clique(r,cr+1,wr+w[iv]));
        } else {
          GECODE_ES_CHECK(bk(r,cr+1,wr+w[iv],np,nx));
        }
        r.excl(iv);
      }
    }
    return ES_OK;
  }
  
}}}

// STATISTICS: int-prop

