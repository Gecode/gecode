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

#include "gecode/gist/layoutcursor.hh"

namespace Gecode { namespace Gist {

  /// \brief Container for shapes
  class ShapeList {
  private:
    /// The number of shapes
    int numberOfShapes;
    /// The shapes
    Shape** shapes;
    /// Offsets computed
    int* offsets;
    /// Compute distance needed between \a shape1 and \a shape2
    int getAlpha(Extent* shape1, int depth1, Extent* shape2, int depth2);
    /// Merge \a shape1 and \a shape2 into \a result with distance \a alpha
    static void merge(Extent* result,
                      Extent* shape1, int depth1,
                      Extent* shape2, int depth2, int alpha);
  public:
    /// Constructor
    ShapeList(int length);
    /// Destructor
    ~ShapeList(void);
    /// Return the merged shape
    Shape* getMergedShape(Extent extent, bool left = false);
    /// Return offset computed for child \a i
    int getOffsetOfChild(int i);
    /// Return shape no \a i
    Shape*& operator[](int i);
  };

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
  
  forceinline Shape*&
  ShapeList::operator[](int i) {
    assert(i < numberOfShapes);
    return shapes[i];
  }
  
  forceinline int
  ShapeList::getOffsetOfChild(int i) {
    assert(i < numberOfShapes);
    return offsets[i];
  }
  
  int
  ShapeList::getAlpha(Extent* shape1, int depth1,
                      Extent* shape2, int depth2) {
    int alpha = Layout::minimalSeparation;
    int extentR = 0;
    int extentL = 0;
    for (int i=0; i<depth1 && i<depth2; i++) {
      extentR += shape1[i].r;
      extentL += shape2[i].l;
      alpha = std::max(alpha, extentR - extentL + Layout::minimalSeparation);
    }
    return std::min(static_cast<int>(alpha*1.05), alpha+4*Layout::extent);
  }
  
  void
  ShapeList::merge(Extent* result,
                   Extent* shape1, int depth1,
                   Extent* shape2, int depth2, int alpha) {
    if (depth1 == 0) {
      for (int i=depth2; i--;)
        result[i] = shape2[i];
    } else if (depth2 == 0) {
      for (int i=depth1; i--;)
        result[i] = shape1[i];
    } else {
      // Extend the topmost right extent by alpha.  This, in effect,
      // moves the second shape to the right by alpha units.
      int topmostL = shape1[0].l;
      int topmostR = shape2[0].r;      
      int backoffTo1 =
        shape1[0].r - alpha - shape2[0].r;
      int backoffTo2 =
        shape2[0].l + alpha - shape1[0].l;

      result[0] = Extent(topmostL, topmostR+alpha);
        
      // Now, since extents are given in relative units, in order to
      // compute the extents of the merged shape, we can just collect the
      // extents of shape1 and shape2, until one of the shapes ends.  If
      // this happens, we need to "back-off" to the axis of the deeper
      // shape in order to properly determine the remaining extents.
      int i=1;
      for (; i<depth1 && i<depth2; i++) {
        Extent currentExtent1 = shape1[i];
        Extent currentExtent2 = shape2[i];
        int newExtentL = currentExtent1.l;
        int newExtentR = currentExtent2.r;        
        result[i] = Extent(newExtentL, newExtentR);
        backoffTo1 += currentExtent1.r - currentExtent2.r;
        backoffTo2 += currentExtent2.l - currentExtent1.l;
      }
        
      // If shape1 is deeper than shape2, back off to the axis of shape1,
      // and process the remaining extents of shape1.
      if (i<depth1) {
        Extent currentExtent1 = shape1[i];
        int newExtentL = currentExtent1.l;
        int newExtentR = currentExtent1.r;        
        result[i] = Extent(newExtentL, newExtentR+backoffTo1);
        ++i;
        for (; i<depth1; i++) {
          result[i] = shape1[i];
        }
      }
        
      // Vice versa, if shape2 is deeper than shape1, back off to the
      // axis of shape2, and process the remaining extents of shape2.
      if (i<depth2) {
        Extent currentExtent2 = shape2[i];
        int newExtentL = currentExtent2.l;
        int newExtentR = currentExtent2.r;
        result[i] = Extent(newExtentL+backoffTo2, newExtentR);
        ++i;
        for (; i<depth2; i++) {
          result[i] = shape2[i];
        }
      }
    }
    
  }
  
  Shape*
  ShapeList::getMergedShape(Extent extent, bool left) {
    if (numberOfShapes == 1) {
      offsets[0] = 0;
      Shape* result = Shape::allocate(extent, shapes[0]);
      (*result)[1].extend(- extent.l, - extent.r);
      return result;
    } else {
      // alphaL[] and alphaR[] store the necessary distances between the
      // axes of the shapes in the list: alphaL[i] gives the distance
      // between shape[i] and shape[i-1], when shape[i-1] and shape[i]
      // are merged left-to-right; alphaR[i] gives the distance between
      // shape[i] and shape[i+1], when shape[i] and shape[i+1] are merged
      // right-to-left.
      GECODE_AUTOARRAY(int, alphaL, numberOfShapes);
      GECODE_AUTOARRAY(int, alphaR, numberOfShapes);
        
      // distance between the leftmost and the rightmost axis in the list
      int width = 0;

      int maxDepth = 0;
      for (int i = numberOfShapes; i--;)
        maxDepth = std::max(maxDepth, shapes[i]->depth());
        
      GECODE_AUTOARRAY(Extent, currentShapeL, maxDepth);
      int ldepth = shapes[0]->depth();
      for (int i=ldepth; i--;)
        currentShapeL[i] = (*shapes[0])[i];

      // After merging, we can pick the result of either merging left or right
      // Here we chose the result of merging right
      Shape* mergedShape = Shape::allocate(maxDepth+1);
      (*mergedShape)[0] = extent;
      int rdepth = shapes[numberOfShapes - 1]->depth();
      for (int i=rdepth; i--;)
        (*mergedShape)[i+1] = (*shapes[numberOfShapes - 1])[i];
      Extent* currentShapeR = &(*mergedShape)[1];

      for (int i = 1; i < numberOfShapes; i++) {
        // Merge left-to-right.  Note that due to the asymmetry of the
        // merge operation, nextAlphaL is the distance between the
        // *leftmost* axis in the shape list, and the axis of
        // nextShapeL; what we are really interested in is the distance
        // between the *previous* axis and the axis of nextShapeL.
        // This explains the correction.

        Shape* nextShapeL = shapes[i];
        int nextAlphaL = getAlpha(&currentShapeL[0], ldepth,
                                  &(*nextShapeL)[0], nextShapeL->depth());
        merge(&currentShapeL[0], &currentShapeL[0], ldepth,
              &(*nextShapeL)[0], nextShapeL->depth(), nextAlphaL);
        ldepth = std::max(ldepth,nextShapeL->depth());
        alphaL[i] = nextAlphaL - width;
        width = nextAlphaL;
            
        // Merge right-to-left.  Here, a correction of nextAlphaR is
        // not required.
        Shape* nextShapeR = shapes[numberOfShapes - 1 - i];
        int nextAlphaR = getAlpha(&(*nextShapeR)[0], nextShapeR->depth(),
                                  &currentShapeR[0], rdepth);
        merge(&currentShapeR[0], &(*nextShapeR)[0], nextShapeR->depth(),
              &currentShapeR[0], rdepth, nextAlphaR);
        rdepth = std::max(rdepth,nextShapeR->depth());
        alphaR[numberOfShapes - i] = nextAlphaR;
      }

      // The merged shape has to be adjusted to its topmost extent
      (*mergedShape)[1].extend(- extent.l, - extent.r);

      // After the loop, the merged shape has the same axis as the
      // leftmost shape in the list.  What we want is to move the axis
      // such that it is the center of the axis of the leftmost shape in
      // the list and the axis of the rightmost shape.
      int halfWidth = left ? 0 : width / 2;
      (*mergedShape)[1].move(- halfWidth);
        
      // Finally, for the offset lists.  Now that the axis of the merged
      // shape is at the center of the two extreme axes, the first shape
      // needs to be offset by -halfWidth units with respect to the new
      // axis.  As for the offsets for the other shapes, we take the
      // median of the alphaL and alphaR values, as suggested in
      // Kennedy's paper.
      int offset = - halfWidth;
      offsets[0] = offset;
      for (int i = 1; i < numberOfShapes; i++) {
        offset += (alphaL[i] + alphaR[i]) / 2;
        offsets[i] = offset;
      }
      return mergedShape;
    }
  }

  ShapeList::ShapeList(int length)
    : numberOfShapes(length),
      shapes(Memory::bmalloc<Shape*>(length)), 
      offsets(Memory::bmalloc<int>(length)) {}

  ShapeList::~ShapeList(void) {
    Memory::free(offsets);
    Memory::free(shapes);
  }

  LayoutCursor::LayoutCursor(VisualNode* theNode)
   : NodeCursor<VisualNode>(theNode) {}
      
  void
  LayoutCursor::processCurrentNode() {
    VisualNode* currentNode = node();
    if (currentNode->isDirty()) {
      Extent extent(Layout::extent);
      int numberOfChildren = currentNode->getNumberOfChildren();
      Shape* shape;
      if (currentNode->isHidden()) {
        shape = Shape::allocate(hiddenShape);
      } else if (numberOfChildren < 1) {
        shape = Shape::allocate(extent);
      } else {
        ShapeList childShapes(numberOfChildren);
        for (int i=numberOfChildren; i--;) {
          childShapes[i]=currentNode->getChild(i)->getShape();
        }

        shape = childShapes.getMergedShape(extent,
                                           currentNode->getStatus() == STEP);
        for (int i=numberOfChildren; i--;) {
          currentNode->getChild(i)->
            setOffset(childShapes.getOffsetOfChild(i));
        }
      }
      Shape::deallocate(currentNode->getShape());
      currentNode->setShape(shape);
      currentNode->setBoundingBox(shape->getBoundingBox());
      currentNode->setDirty(false);
    }
    if (currentNode->getNumberOfChildren() >= 1)
      currentNode->setChildrenLayoutDone(true);
  }
    
}}

// STATISTICS: gist-any
