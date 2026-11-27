/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/gist/nodecursor.hh>
#include <gecode/gist/layoutcursor.hh>
#include <QtGui>

namespace Gecode::Gist {

    /// \brief A cursor that draws a tree on a QWidget
    class DrawingCursor : public NodeCursor<VisualNode> {
    private:
        /// The painter where the tree is drawn
        QPainter& painter;
        /// The clipping area
        QRect clipping_rect;
        /// The best solution (for branch-and-bound)
        BestNode* cur_best;
        /// The current coordinates
        double x, y;

        bool copies;

        /// Test if current node is clipped
        bool is_clipped();

    public:
        /// The color for failed nodes
        static const QColor red;
        /// The color for solved nodes
        static const QColor green;
        /// The color for choice nodes
        static const QColor blue;
        /// The color for the best solution
        static const QColor orange;
        /// White color
        static const QColor white;

        /// The color for expanded failed nodes
        static const QColor light_red;
        /// The color for expanded solved nodes
        static const QColor light_green;
        /// The color for expanded choice nodes
        static const QColor light_blue;

        /// Constructor
        DrawingCursor(VisualNode* root, const VisualNode::NodeAllocator& na, BestNode* cur_best0, QPainter& painter0,
                      const QRect& clipping_rect0, bool show_copies);

        ///\name Cursor interface
        //@{
        /// Move cursor to parent
        void move_upwards();
        /// Test if cursor may move to child
        bool may_move_downwards();
        /// Move cursor to child
        void move_downwards();
        /// Move cursor to sibling
        void move_sidewards();
        /// Draw the node
        void process_current_node();
        //@}
    };

} // namespace Gecode::Gist

#include <gecode/gist/drawingcursor.hpp>

#endif

// STATISTICS: gist-any
