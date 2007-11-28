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

#ifndef GECODE_GIST_SHAPELIST_HH
#define GECODE_GIST_SHAPELIST_HH

#include <vector>
#include <iostream>

namespace Gecode { namespace Gist {

  /// \brief Bounding box
  class BoundingBox {
  public:
    /// Left coordinate
    int left;
    /// Right coordinate 
    int right;
    /// Depth
    int depth;
    /// Constructor
    BoundingBox(int l, int r, int d);
    /// Default constructor
    BoundingBox(void) {}
  };

  /// \brief Extent, representing shape of a tree at one depth level
  class Extent {
  public:
    /// Left extent
    int l;
    /// Right extent
    int r;
    /// Default constructor
    Extent(void);
    /// Construct with \a l0 and \a r0
    Extent(int l0, int r0);
    /// Construct with width \a width
    Extent(int width);
    
    /// Extend extent by \a deltaL and \a deltaR
    void extend(int deltaL, int deltaR);
    /// Move extent by \a delta
    void move(int delta);
  };

  /// \brief The shape of a subtree
  class Shape {
  private:
    /// The shape is a vector of extents, one for each depth level
    std::vector<Extent> shape;
    /// Copy construtor
    Shape(const Shape&);
  public:
    /// Default constructor
    Shape(void);
    /// Construct with single extent \a e
    Shape(Extent e);
    /// Construct with \e for the root and \a subShape for the children
    Shape(Extent e, const Shape* subShape);
    /// Construct from \a subShape
    Shape(const Shape* subShape);
    
    /// Return depth of the shape
    int depth(void) const;
    /// Add \a to the shape
    void add(Extent e);
    /// Return extent at depth \a i
    Extent get(int i);
    /// Extend the shape by \a deltaL and \a deltaR
    void extend(int deltaL, int deltaR);
    /// Move the shape by \a delta
    void move(int delta);
    /// Return if extent exists at \a depth, if yes return it in \a extent
    bool getExtentAtDepth(int depth, Extent& extent);
    BoundingBox getBoundingBox(void);
  };

  /// \brief Container for shapes
  class ShapeList {
  private:
    /// The shapes
    std::vector<Shape*> shapes;
    /// The minimal distance necessary between two nodes
    int minimalSeparation;
    /// Offsets computed
    std::vector<int> offsets;
    /// Compute distance needed between \a shape1 and \a shape2
    int getAlpha(Shape* shape1, Shape* shape2);
    /// Merge \a shape1 and \a shape2 with distance \a alpha
    static Shape* merge(Shape* shape1, Shape* shape2, int alpha);
  public:
    /// Constructor
    ShapeList(int length, int minSeparation);
    /// Return the merged shape
    Shape* getMergedShape(void);
    /// Return offset computed for child \a i
    int getOffsetOfChild(int i);
    /// Return shape no \a i
    Shape*& operator[](int i);
  };
  
}}

#endif

// STATISTICS: gist-any
