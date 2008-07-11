/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2008-04-26 14:10:40 +0200 (Sa, 26 Apr 2008) $ by $Author: tack $
 *     $Revision: 6788 $
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

#include "gecode/gist/layoutcursor.hh"

namespace Gecode { namespace Gist {

  /// Shape of a single node
  const Shape* singletonShape;
  /// Shape of hidden nodes
  const Shape* hiddenShape;
  
  /// Allocate shapes statically
  class ShapeAllocator {
  public:
    /// Constructor
    ShapeAllocator(void) {
      singletonShape = Shape::allocate(Extent(Layout::extent*1.5));
      hiddenShape = Shape::allocate(Extent(Layout::extent), singletonShape);
    }
  };

  /// Allocate shapes statically
  ShapeAllocator shapeAllocator;

  Shape*
  Shape::allocate(Extent e, const Shape* subShape) {
    Shape* shape = Shape::allocate(subShape->depth() + 1);
    (*shape)[0] = e;
    for (int i=subShape->depth(); i--;) {
      (*shape)[i+1] = (*subShape)[i];
    }
    return shape;
  }
  
  Shape*
  Shape::allocate(const Shape* subShape) {
    Shape* shape = Shape::allocate(subShape->depth());
    for (int i=subShape->depth(); i--;) {
      (*shape)[i] = (*subShape)[i];
    }    
    return shape;
  }
    
  bool
  Shape::getExtentAtDepth(int depth, Extent& extent) {
    int currentDepth = 0;
    int extentL = 0;
    int extentR = 0;
    for (int i=0; i<_depth && currentDepth <= depth; i++) {
      Extent currentExtent = shape[i];
      extentL += currentExtent.l;
      extentR += currentExtent.r;
      currentDepth++;
    }
    if (currentDepth == depth + 1) {
      extent = Extent(extentL, extentR);
      return true;
    } else {
      return false;
    }  
  }
  
  BoundingBox
  Shape::getBoundingBox(void) {
    int lastLeft = 0;
    int lastRight = 0;
    int left = 0;
    int right = 0;
    int depth = 0;
    for (int i=0; i<_depth; i++) {
      Extent curExtent = shape[i];
      depth++;
      lastLeft = lastLeft + curExtent.l;
      lastRight = lastRight + curExtent.r;
      if (lastLeft < left)
        left = lastLeft;
      if (lastRight > right)
        right = lastRight;
    }
    return BoundingBox(left, right, depth);    
  }
  
  LayoutCursor::LayoutCursor(VisualNode* theNode)
   : NodeCursor<VisualNode>(theNode) {}
      
  void
  LayoutCursor::processCurrentNode() {
    VisualNode* currentNode = node();
    if (currentNode->isDirty()) {
      Extent extent(Layout::extent);
      int numberOfChildren = currentNode->getNumberOfChildren();
      if (currentNode->isHidden()) {
        currentNode->setShape(Shape::allocate(hiddenShape));
      } else if (numberOfChildren < 1) {
        currentNode->setShape(Shape::allocate(Extent(Layout::extent)));
      } else {
        currentNode->computeShape();
      }
      currentNode->setDirty(false);
    }
    if (currentNode->getNumberOfChildren() >= 1)
      currentNode->setChildrenLayoutDone(true);
  }
    
}}

// STATISTICS: gist-any
