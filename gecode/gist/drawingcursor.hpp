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

namespace Gecode { namespace Gist {

    inline void
    DrawingCursor::move_upwards() {
        x -= node()->get_offset();
        y -= Layout::dist_y;
        NodeCursor<VisualNode>::move_upwards();
    }

    forceinline bool
    DrawingCursor::is_clipped() {
        if (clipping_rect.width() == 0 && clipping_rect.x() == 0 && clipping_rect.height() == 0 &&
            clipping_rect.y() == 0)
            return false;
        BoundingBox b = node()->get_bounding_box();
        return (x + b.left > clipping_rect.x() + clipping_rect.width() || x + b.right < clipping_rect.x() ||
                y > clipping_rect.y() + clipping_rect.height() ||
                y + (node()->get_shape()->depth() + 1) * Layout::dist_y < clipping_rect.y());
    }

    inline bool
    DrawingCursor::may_move_downwards() {
        return NodeCursor<VisualNode>::may_move_downwards() && !node()->is_hidden() &&
               node()->children_layout_is_done() && !is_clipped();
    }

    inline void
    DrawingCursor::move_downwards() {
        NodeCursor<VisualNode>::move_downwards();
        x += node()->get_offset();
        y += Layout::dist_y;
    }

    inline void
    DrawingCursor::move_sidewards() {
        x -= node()->get_offset();
        NodeCursor<VisualNode>::move_sidewards();
        x += node()->get_offset();
    }

}} // namespace Gecode::Gist
// STATISTICS: gist-any
