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

#include "gecode/gist/nodecursor.hh"
#include "gecode/gist/shapelist.hh"

namespace Gecode { namespace Gist {
    
  LayoutCursor::LayoutCursor(VisualNode* theNode)
   : NodeCursor<VisualNode>(theNode) {}
    
  bool
  LayoutCursor::mayMoveDownwards(void) {
    return NodeCursor<VisualNode>::mayMoveDownwards() &&
           node()->isDirty() /*&&
           (! n->isHidden() )*/;
  }
  
  void
  LayoutCursor::processCurrentNode() {
    VisualNode* currentNode = node();
    if (currentNode->isDirty()) {
      Extent extent(20);
      int numberOfChildren = currentNode->getNumberOfChildren();
      Shape* shape;
      if (numberOfChildren == -1) {
        shape = new Shape(extent);
      } else if (currentNode->isHidden()) {
        shape = new Shape(&VisualNode::unitShape);
#ifdef GECODE_GIST_EXPERIMENTAL
        if(currentNode->isStepNode()) {
          delete shape;
          shape = new Shape(currentNode->getChild(0)->getShape());
        }
#endif
      } else if (numberOfChildren == 0) {
        shape = new Shape(extent);
      } else {
        ShapeList childShapes(numberOfChildren, 10);
        for (int i=0; i<numberOfChildren; i++) {
          childShapes[i]=currentNode->getChild(i)->getShape();
        }

        Shape* subtreeShape = 
          childShapes.getMergedShape(currentNode->getStatus() == STEP);
        subtreeShape->extend(- extent.l, - extent.r);
        shape = new Shape(extent, subtreeShape);
        delete subtreeShape;
        for (int i=0; i<numberOfChildren; i++) {
          currentNode->getChild(i)->setOffset(childShapes.getOffsetOfChild(i));
        }
      }
      currentNode->setShape(shape);
      currentNode->setBoundingBox(shape->getBoundingBox());
      currentNode->setDirty(false);
    }
  }
  
  HideFailedCursor::HideFailedCursor(VisualNode* root) : NodeCursor<VisualNode>(root) {}
  
  bool
  HideFailedCursor::mayMoveDownwards(void) {
    VisualNode* n = node();
#ifdef GECODE_GIST_EXPERIMENTAL
    return NodeCursor<VisualNode>::mayMoveDownwards() &&
           (n->hasSolvedChildren() || n->getNoOfOpenChildren() > 0) &&
           (! n->isHidden()) || n->isStepNode();
#else
    return NodeCursor<VisualNode>::mayMoveDownwards() &&
           (n->hasSolvedChildren() || n->getNoOfOpenChildren() > 0) &&
           (! n->isHidden());
#endif
  }
  
  void
  HideFailedCursor::processCurrentNode(void) {
    VisualNode* n = node();
    if ((n->getStatus() == BRANCH ||
         ((n->getStatus() == SPECIAL || n->getStatus() == STEP) && n->hasFailedChildren())) &&
        !n->hasSolvedChildren() &&
        n->getNoOfOpenChildren() == 0) {
      n->setHidden(true);
      n->dirtyUp();
#ifdef GECODE_GIST_EXPERIMENTAL
      if(n->isExpanded() && !n->isCollapsed() && n->getParent()->isHidden()) {
        n->setCollapsed(!n->isCollapsed());
      }
#endif
    }
  }

  UnhideAllCursor::UnhideAllCursor(VisualNode* root) : NodeCursor<VisualNode>(root) {}
  
  void
  UnhideAllCursor::processCurrentNode(void) {
    VisualNode* n = node();
    if (n->isHidden()) {
      n->setHidden(false);
      n->dirtyUp();
    }
#ifdef GECODE_GIST_EXPERIMENTAL
    if (n->isCollapsed()) {
      n->setCollapsed(false);
      n->dirtyUp();
    }
#endif
  }
  
}}

// STATISTICS: gist-any
