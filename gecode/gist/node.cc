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

#include "gecode/support.hh"
#include "gecode/gist/node.hh"
#include <cassert>

namespace Gecode { namespace Gist {
  
  Node::Node(void) : parent(NULL), children(NULL), noOfChildren(-1) {}

  Node::~Node(void) {
    if (noOfChildren != -1)
      for (int i=noOfChildren; i--;)
        delete children[i];
    Memory::free(children);
  }

  Node*
  Node::getParent(void) { return parent; }

  Node*
  Node::getChild(int n) {
    assert(n < noOfChildren);
    return children[n];
  }
    
  bool
  Node::isRoot(void) { return parent == NULL; }

  int
  Node::getDepth(void) { return isRoot() ? 1 : parent->getDepth() + 1; }
    
  void
  Node::setNumberOfChildren(int n) {
    assert(noOfChildren == -1);
    noOfChildren = n;
    children = Memory::bmalloc<Node*>(n);
    for (int i=n; i--;)
      children[i] = NULL;
  }

  void
  Node::setChild(int n, Node* child) {
    assert(noOfChildren > n);
    children[n] = child;
    child->parent = this;
    child->alternative = n;
  }

  void
  Node::addChild(Node* child) {
    child->parent = this;
    child->alternative = noOfChildren == -1 ? 0 : noOfChildren;

    if(noOfChildren == -1) {
      setNumberOfChildren(1);
    } else {
      Node** newChildren = Memory::bmalloc<Node*>(noOfChildren+1);
      for (int i=noOfChildren; i--;)
        newChildren[i] = children[i];
      Memory::free(children);
      children = newChildren;
      noOfChildren++;
    }
    children[noOfChildren-1] = child;
  }

  int
  Node::getNumberOfChildren(void) {
    return noOfChildren;
  }
  
  int
  Node::getAlternative(void) {
    return alternative;
  }
}}

// STATISTICS: gist-any
