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

    forceinline bool
    LayoutCursor::may_move_downwards() {
        return NodeCursor<VisualNode>::may_move_downwards() && node()->is_dirty();
    }

    inline LayoutCursor::LayoutCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(the_node, na) {}

    forceinline void
    LayoutCursor::process_current_node() {
        VisualNode* current_node = node();
        if (current_node->is_dirty()) {
            if (current_node->is_hidden()) {
                // do nothing
            } else if (false && current_node->get_number_of_children() < 1) {
                current_node->set_shape(Shape::leaf);
            } else {
                current_node->compute_shape(na);
            }
            current_node->set_dirty(false);
        }
        if (current_node->get_number_of_children() >= 1)
            current_node->set_children_layout_done(true);
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
