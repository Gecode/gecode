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
  
  Node::~Node(void) {
    switch (getTag()) {
    case UNDET:
    case LEAF:
      return;
    case TWO_CHILDREN:
      delete static_cast<VisualNode*>(getPtr());
      if (Support::marked(c.secondChild))
        delete static_cast<VisualNode*>(Support::unmark(c.secondChild));
      break;
    case MORE_CHILDREN:
      for (int i=c.noOfChildren; i--;)
        delete static_cast<VisualNode**>(getPtr())[i];
      heap.free<Node*>(static_cast<Node**>(getPtr()),c.noOfChildren);
    }
  }
    
  void
  Node::setNumberOfChildren(unsigned int n) {
    assert(getTag() == UNDET);
    switch (n) {
    case 0:
      setTag(LEAF);
      break;
    case 1:
      setTag(TWO_CHILDREN);
      c.secondChild = NULL;
      break;
    case 2:
      setTag(TWO_CHILDREN);
      c.secondChild = static_cast<Node*>(Support::mark(NULL));
      break;
    default:
      c.noOfChildren = n;
      childrenOrFirstChild = heap.alloc<Node*>(n);
      Node** children = static_cast<Node**>(childrenOrFirstChild);
      setTag(MORE_CHILDREN);
      for (unsigned int i=n; i--;)
        children[i] = NULL;      
    }
  }

  void
  Node::setChild(unsigned int n, Node* child) {
    assert(getNumberOfChildren() > n);
    if (getTag() == TWO_CHILDREN) {
      if (n == 0) {
        unsigned int tag = getTag();
        childrenOrFirstChild = child;
        setTag(tag);
      } else {
        c.secondChild = static_cast<Node*>(Support::mark(child));
      }
    } else {
      static_cast<Node**>(getPtr())[n] = child;
    }
    child->parent = this;
  }

  void
  Node::addChild(Node* child) {
    child->parent = this;
    
    unsigned int noOfChildren;
    switch (getTag()) {
    case UNDET:
      setNumberOfChildren(1);
      noOfChildren = 1;
      break;
    case LEAF:
      setTag(TWO_CHILDREN);
      c.secondChild = NULL;
      noOfChildren = 1;
      break;
    case TWO_CHILDREN:
      if (Support::marked(c.secondChild)) {
        Node** newChildren = heap.alloc<Node*>(c.noOfChildren+1);
        newChildren[0] = static_cast<Node*>(getPtr());
        newChildren[1] = static_cast<Node*>(Support::unmark(c.secondChild));
        noOfChildren = 3;
      } else {
        c.secondChild = static_cast<Node*>(Support::mark(NULL));
        noOfChildren = 2;
      }
      break;
    case MORE_CHILDREN:
      {
        Node** newChildren = heap.alloc<Node*>(c.noOfChildren+1);
        Node** children = static_cast<Node**>(getPtr());
        for (int i=noOfChildren; i--;)
          newChildren[i] = children[i];
        heap.free<Node*>(children,noOfChildren);
        childrenOrFirstChild = newChildren;
        setTag(MORE_CHILDREN);
        c.noOfChildren++;
        noOfChildren = c.noOfChildren;
      }
      break;
    }
    setChild(noOfChildren-1, child);
  }

}}

// STATISTICS: gist-any
