/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/support.hh>
#include <gecode/gist/node.hh>
#include <gecode/gist/visualnode.hh>
#include <cassert>

namespace Gecode { namespace Gist {

  void
  Node::setNumberOfChildren(unsigned int n, NodeAllocator& na) {
    assert(getTag() == UNDET);
    switch (n) {
    case 0:
      setTag(LEAF);
      break;
    case 1:
      childrenOrFirstChild = new (na) VisualNode(this);
      c.secondChild = NULL;
      setTag(TWO_CHILDREN);
      break;
    case 2:
      childrenOrFirstChild = new (na) VisualNode(this);
      c.secondChild =
        static_cast<Node*>(Support::mark(new (na) VisualNode(this)));
      setTag(TWO_CHILDREN);
      break;
    default:
      c.noOfChildren = n;
      childrenOrFirstChild = heap.alloc<Node*>(n);
      Node** children = static_cast<Node**>(childrenOrFirstChild);
      setTag(MORE_CHILDREN);
      for (unsigned int i=n; i--;)
        children[i] = new (na) VisualNode(this);
    }
  }

}}

// STATISTICS: gist-any
