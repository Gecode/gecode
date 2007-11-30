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

#ifndef GECODE_GIST_QT_DRAWINGCURSOR_HH
#define GECODE_GIST_QT_DRAWINGCURSOR_HH

#include "gecode/gist/nodecursor.hh"
#include <QtGui>

namespace Gecode { namespace Gist {

  /// \brief A cursor that draws a tree on a QWidget
  class DrawingCursor : public Gist::NodeCursor<VisualNode> {
  private:
    /// The painter where the tree is drawn
    QPainter& painter;
    /// The clipping area
    QRect clippingRect;
    /// The current coordinates
    int x, y;
    
    /// The color for failed nodes
    static const QColor red;
    /// The color for solved nodes
    static const QColor green;
    /// The color for choice nodes
    static const QColor blue;
    
    /// Test if current node is clipped
    bool isClipped(void);
  public:
    /// Constructor
    DrawingCursor(Gist::VisualNode* root, QPainter& painter0,
                  const QRect& clippingRect0);

    ///\name Cursor interface
    //@{
    /// Move cursor to parent
    void moveUpwards(void);
    /// Test if cursor may move to child
    bool mayMoveDownwards(void);
    /// Move cursor to child
    void moveDownwards(void);
    /// Move cursor to sibling
    void moveSidewards(void);
    /// Draw the node
    void processCurrentNode(void);
    //@}
  };

}}

#endif

// STATISTICS: gist-any
