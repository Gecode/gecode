/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2007-11-26 15:48:52 +0100 (Mo, 26 Nov 2007) $ by $Author: tack $
 *     $Revision: 5437 $
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

#ifndef GECODE_GIST_LAYOUTCURSOR_HH
#define GECODE_GIST_LAYOUTCURSOR_HH

#include "gecode/gist/nodecursor.hh"
#include "gecode/gist/visualnode.hh"

namespace Gecode { namespace Gist {
  
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
    /// The shape is an array of extents, one for each depth level
    Extent* shape;
    /// The depth of this shape
    int _depth;
    /// Copy construtor
    Shape(const Shape&);
  public:
    /// Constructor shape of depth \a d
    Shape(int d);
    /// Construct with single extent \a e
    Shape(Extent e);
    /// Construct with \e for the root and \a subShape for the children
    Shape(Extent e, const Shape* subShape);
    /// Construct from \a subShape
    Shape(const Shape* subShape);
    // Destructor
    ~Shape(void);

    static const Shape singletonShape;
    static const Shape unitShape;
    
    /// Return depth of the shape
    int depth(void) const;
    /// Return extent at depth \a i
    const Extent& operator[](int i) const;
    /// Return extent at depth \a i
    Extent& operator[](int i);
    /// Extend the shape by \a deltaL and \a deltaR
    void extend(int deltaL, int deltaR);
    /// Move the shape by \a delta
    void move(int delta);
    /// Return if extent exists at \a depth, if yes return it in \a extent
    bool getExtentAtDepth(int depth, Extent& extent);
    /// Return bounding box
    BoundingBox getBoundingBox(void);
  };
  
  /// \brief A cursor that computes a tree layout for VisualNodes
  class LayoutCursor : public NodeCursor<VisualNode> {
  public:
    /// Constructor
    LayoutCursor(VisualNode* theNode);

    /// \name Cursor interface
    //@{
    /// Test if the cursor may move to the first child node
    bool mayMoveDownwards(void);
    /// Compute layout for current node
    void processCurrentNode(void);
    //@}
  };

}}

#include "gecode/gist/layoutcursor.icc"

#endif

// STATISTICS: gist-any
