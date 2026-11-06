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

namespace Gecode::Gist {

    forceinline
    Extent::Extent()
        : l(-1), r(-1) {}

    forceinline
    Extent::Extent(int l0, int r0)
        : l(l0), r(r0) {}

    inline Extent::Extent(int width) {
        int half_width = width / 2;
        l = 0 - half_width;
        r = 0 + half_width;
    }

    inline void
    Extent::extend(int delta_l, int delta_r) {
        l += delta_l;
        r += delta_r;
    }

    inline void
    Extent::move(int delta) {
        l += delta;
        r += delta;
    }

    forceinline int
    Shape::depth() const {
        return _depth;
    }

    forceinline void
    Shape::set_depth(int d) {
        assert(d <= _depth);
        _depth = d;
    }

    forceinline const Extent&
    Shape::operator[](int i) const {
        assert(i < _depth);
        return shape[i];
    }

    forceinline Extent&
    Shape::operator[](int i) {
        assert(i < _depth);
        return shape[i];
    }

    inline Shape*
    Shape::allocate(int d) {
        assert(d >= 1);
        Shape* ret;
        ret = static_cast<Shape*>(heap.ralloc(sizeof(Shape) + (d - 1) * sizeof(Extent)));
        ret->_depth = d;
        return ret;
    }

    forceinline void
    Shape::deallocate(Shape* shape) {
        if (shape != hidden && shape != leaf)
            heap.rfree(shape);
    }

    forceinline bool
    Shape::get_extent_at_depth(int d, Extent& extent) {
        if (d > depth())
            return false;
        extent = Extent(0, 0);
        for (int i = 0; i <= d; i++) {
            Extent current_extent = (*this)[i];
            extent.l += current_extent.l;
            extent.r += current_extent.r;
        }
        return true;
    }

    forceinline void
    Shape::compute_bounding_box() {
        int last_left = 0;
        int last_right = 0;
        bb.left = 0;
        bb.right = 0;
        for (int i = 0; i < depth(); i++) {
            last_left = last_left + (*this)[i].l;
            last_right = last_right + (*this)[i].r;
            bb.left = std::min(bb.left, last_left);
            bb.right = std::max(bb.right, last_right);
        }
    }

    forceinline const BoundingBox&
    Shape::get_bounding_box() const {
        return bb;
    }

    forceinline bool
    VisualNode::is_hidden() {
        return get_flag(HIDDEN);
    }

    forceinline void
    VisualNode::set_hidden(bool h) {
        set_flag(HIDDEN, h);
    }

    forceinline void
    VisualNode::set_stop(bool h) {
        if (get_status() == BRANCH && h)
            set_status(STOP);
        else if (get_status() == STOP && !h)
            set_status(UNSTOP);
    }

    forceinline int
    VisualNode::get_offset() {
        return offset;
    }

    forceinline void
    VisualNode::set_offset(int n) {
        offset = n;
    }

    forceinline bool
    VisualNode::is_dirty() {
        return get_flag(DIRTY);
    }

    forceinline void
    VisualNode::set_dirty(bool d) {
        set_flag(DIRTY, d);
    }

    forceinline bool
    VisualNode::children_layout_is_done() {
        return get_flag(CHILDREN_LAYOUT_DONE);
    }

    forceinline void
    VisualNode::set_children_layout_done(bool d) {
        set_flag(CHILDREN_LAYOUT_DONE, d);
    }

    forceinline bool
    VisualNode::is_marked() {
        return get_flag(MARKED);
    }

    forceinline void
    VisualNode::set_marked(bool m) {
        set_flag(MARKED, m);
    }

    forceinline bool
    VisualNode::is_bookmarked() {
        return get_flag(BOOKMARKED);
    }

    forceinline void
    VisualNode::set_bookmarked(bool m) {
        set_flag(BOOKMARKED, m);
    }

    forceinline bool
    VisualNode::is_on_path() {
        return get_flag(ONPATH);
    }

    forceinline void
    VisualNode::set_on_path(bool b) {
        set_flag(ONPATH, b);
    }

    forceinline Shape*
    VisualNode::get_shape() {
        return is_hidden() ? Shape::hidden : shape;
    }

    forceinline BoundingBox
    VisualNode::get_bounding_box() {
        return get_shape()->get_bounding_box();
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
