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

#include <gecode/gist/visualnode.hh>

#include <gecode/gist/layoutcursor.hh>
#include <gecode/gist/nodevisitor.hh>

#include <utility>

namespace Gecode::Gist {

    Shape* Shape::leaf;
    Shape* Shape::hidden;

    /// Allocate shapes statically
    class ShapeAllocator {
    public:
        /// Constructor
        ShapeAllocator() {
            Shape::leaf = Shape::allocate(1);
            (*Shape::leaf)[0] = Extent(Layout::extent);
            Shape::leaf->compute_bounding_box();

            Shape::hidden = Shape::allocate(2);
            (*Shape::hidden)[0] = Extent(Layout::extent);
            (*Shape::hidden)[1] = Extent(Layout::extent);
            Shape::hidden->compute_bounding_box();
        }
        ~ShapeAllocator() {
            Shape::deallocate(Shape::leaf);
            Shape::deallocate(Shape::hidden);
        }
    };

    /// Allocate shapes statically
    ShapeAllocator shape_allocator;

    VisualNode::VisualNode(int p) : SpaceNode(p), offset(0) {
        shape = nullptr;
        set_dirty(true);
        set_children_layout_done(false);
        set_hidden(false);
        set_marked(false);
        set_on_path(false);
        set_bookmarked(false);
    }

    VisualNode::VisualNode(Space* root) : SpaceNode(root), offset(0) {
        shape = nullptr;
        set_dirty(true);
        set_children_layout_done(false);
        set_hidden(false);
        set_marked(false);
        set_on_path(false);
        set_bookmarked(false);
    }

    void
    VisualNode::dispose() {
        Shape::deallocate(shape);
        SpaceNode::dispose();
    }

    void
    VisualNode::dirty_up(const NodeAllocator& na) {
        VisualNode* cur = this;
        while (!cur->is_dirty()) {
            cur->set_dirty(true);
            if (!cur->is_root()) {
                cur = cur->get_parent(na);
            }
        }
    }

    void
    VisualNode::layout(const NodeAllocator& na) {
        LayoutCursor l(this, na);
        PostorderNodeVisitor<LayoutCursor>(l).run();
        // int nodesLayouted = 1;
        // clock_t t0 = clock();
        // while (p.next()) {}
        // while (p.next()) { nodesLayouted++; }
        // double t = (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC) * 1000.0;
        // double nps = static_cast<double>(nodesLayouted) /
        //   (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC);
        // std::cout << "Layout done. " << nodesLayouted << " nodes in "
        //   << t << " ms. " << nps << " nodes/s." << std::endl;
    }

    void
    VisualNode::path_up(const NodeAllocator& na) {
        VisualNode* cur = this;
        while (cur) {
            cur->set_on_path(true);
            cur = cur->get_parent(na);
        }
    }

    void
    VisualNode::un_path_up(const NodeAllocator& na) {
        VisualNode* cur = this;
        while (!cur->is_root()) {
            cur->set_on_path(false);
            cur = cur->get_parent(na);
        }
    }

    int
    VisualNode::get_path_alternative(const NodeAllocator& na) {
        for (int i = get_number_of_children(); i--;) {
            if (get_child(na, i)->is_on_path())
                return i;
        }
        return -1;
    }

    void
    VisualNode::toggle_hidden(const NodeAllocator& na) {
        set_hidden(!is_hidden());
        dirty_up(na);
    }

    void
    VisualNode::hide_failed(const NodeAllocator& na, bool only_dirty) {
        HideFailedCursor c(this, na, only_dirty);
        PreorderNodeVisitor<HideFailedCursor>(c).run();
        dirty_up(na);
    }

    void
    VisualNode::label_branches(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d) {
        bool clear = na.has_label(this);
        BranchLabelCursor c(this, cur_best, c_d, a_d, clear, na);
        PreorderNodeVisitor<BranchLabelCursor>(c).run();
        dirty_up(na);
    }

    void
    VisualNode::label_path(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d) {
        if (na.has_label(this)) {
            // clear labels on path to root
            VisualNode* p = this;
            while (p) {
                na.clear_label(p);
                p = p->get_parent(na);
            }
        } else {
            // set labels on path to root
            std::vector<std::pair<VisualNode*, int>> path;
            VisualNode* p = this;
            while (p) {
                path.emplace_back(p, p->get_alternative(na));
                p = p->get_parent(na);
            }
            while (!path.empty()) {
                std::pair<VisualNode*, int> cur = path.back();
                path.pop_back();
                if (p) {
                    std::string l = cur.first->get_branch_label(na, p, p->get_choice(), cur_best, c_d, a_d, cur.second);
                    na.set_label(cur.first, QString(l.c_str()));
                }
                p = cur.first;
            }
        }
        dirty_up(na);
    }

    void
    VisualNode::unhide_all(const NodeAllocator& na) {
        UnhideAllCursor c(this, na);
        PreorderNodeVisitor<UnhideAllCursor>(c).run();
        dirty_up(na);
    }

    void
    VisualNode::toggle_stop(const NodeAllocator& na) {
        if (get_status() == STOP)
            set_status(UNSTOP);
        else if (get_status() == UNSTOP)
            set_status(STOP);
        dirty_up(na);
    }

    void
    VisualNode::unstop_all(const NodeAllocator& na) {
        UnstopAllCursor c(this, na);
        PreorderNodeVisitor<UnstopAllCursor>(c).run();
        dirty_up(na);
    }

    void
    VisualNode::changed_status(const NodeAllocator& na) {
        dirty_up(na);
    }

    bool
    VisualNode::contains_coordinate_at_depth(int x, int depth) {
        BoundingBox box = get_shape()->get_bounding_box();
        if (x < box.left || x > box.right || depth >= get_shape()->depth()) {
            return false;
        }
        Extent the_extent;
        if (get_shape()->get_extent_at_depth(depth, the_extent)) {
            return (the_extent.l <= x && x <= the_extent.r);
        }
        return false;
    }

    VisualNode*
    VisualNode::find_node(const NodeAllocator& na, int x, int y) {
        VisualNode* cur = this;
        int depth = y / Layout::dist_y;

        while (depth > 0 && cur != nullptr) {
            if (cur->is_hidden()) {
                break;
            }
            VisualNode* old_cur = cur;
            cur = nullptr;
            for (unsigned int i = 0; i < old_cur->get_number_of_children(); i++) {
                VisualNode* next_child = old_cur->get_child(na, i);
                int new_x = x - next_child->get_offset();
                if (next_child->contains_coordinate_at_depth(new_x, depth - 1)) {
                    cur = next_child;
                    x = new_x;
                    break;
                }
            }
            depth--;
            y -= Layout::dist_y;
        }

        if (cur == this && !cur->contains_coordinate_at_depth(x, 0)) {
            return nullptr;
        }
        return cur;
    }

    std::string
    VisualNode::tool_tip(NodeAllocator&, BestNode*, int, int) {
        return "";
    }

    std::string
    VisualNode::get_branch_label(NodeAllocator& na, VisualNode* p, const Choice* c, BestNode* cur_best, int c_d,
                                 int a_d, int alt) {
        std::ostringstream oss;
        p->acquire_space(na, cur_best, c_d, a_d);
        p->get_working_space()->print(*c, alt, oss);
        return oss.str();
    }

    /// \brief Helper functions for the layout algorithm
    class Layouter {
    public:
        /// Compute distance needed between \a shape1 and \a shape2
        template <class S1, class S2>
        static int get_alpha(const S1& shape1, int depth1, const S2& shape2, int depth2);
        /// Merge \a shape1 and \a shape2 into \a result with distance \a alpha
        template <class S1, class S2>
        static void merge(Extent* result, const S1& shape1, int depth1, const S2& shape2, int depth2, int alpha);
    };

    template <class S1, class S2>
    int
    Layouter::get_alpha(const S1& shape1, int depth1, const S2& shape2, int depth2) {
        int alpha = Layout::minimal_separation;
        int extent_r = 0;
        int extent_l = 0;
        for (int i = 0; i < depth1 && i < depth2; i++) {
            extent_r += shape1[i].r;
            extent_l += shape2[i].l;
            alpha = std::max(alpha, extent_r - extent_l + Layout::minimal_separation);
        }
        return alpha;
    }

    template <class S1, class S2>
    void
    Layouter::merge(Extent* result, const S1& shape1, int depth1, const S2& shape2, int depth2, int alpha) {
        if (depth1 == 0) {
            for (int i = depth2; i--;)
                result[i] = shape2[i];
        } else if (depth2 == 0) {
            for (int i = depth1; i--;)
                result[i] = shape1[i];
        } else {
            // Extend the topmost right extent by alpha.  This, in effect,
            // moves the second shape to the right by alpha units.
            int topmost_l = shape1[0].l;
            int topmost_r = shape2[0].r;
            int backoff_to1 = shape1[0].r - alpha - shape2[0].r;
            int backoff_to2 = shape2[0].l + alpha - shape1[0].l;

            result[0] = Extent(topmost_l, topmost_r + alpha);

            // Now, since extents are given in relative units, in order to
            // compute the extents of the merged shape, we can just collect the
            // extents of shape1 and shape2, until one of the shapes ends.  If
            // this happens, we need to "back-off" to the axis of the deeper
            // shape in order to properly determine the remaining extents.
            int i = 1;
            for (; i < depth1 && i < depth2; i++) {
                Extent current_extent1 = shape1[i];
                Extent current_extent2 = shape2[i];
                int new_extent_l = current_extent1.l;
                int new_extent_r = current_extent2.r;
                result[i] = Extent(new_extent_l, new_extent_r);
                backoff_to1 += current_extent1.r - current_extent2.r;
                backoff_to2 += current_extent2.l - current_extent1.l;
            }

            // If shape1 is deeper than shape2, back off to the axis of shape1,
            // and process the remaining extents of shape1.
            if (i < depth1) {
                Extent current_extent1 = shape1[i];
                int new_extent_l = current_extent1.l;
                int new_extent_r = current_extent1.r;
                result[i] = Extent(new_extent_l, new_extent_r + backoff_to1);
                ++i;
                for (; i < depth1; i++) {
                    result[i] = shape1[i];
                }
            }

            // Vice versa, if shape2 is deeper than shape1, back off to the
            // axis of shape2, and process the remaining extents of shape2.
            if (i < depth2) {
                Extent current_extent2 = shape2[i];
                int new_extent_l = current_extent2.l;
                int new_extent_r = current_extent2.r;
                result[i] = Extent(new_extent_l + backoff_to2, new_extent_r);
                ++i;
                for (; i < depth2; i++) {
                    result[i] = shape2[i];
                }
            }
        }
    }

    void
    VisualNode::set_shape(Shape* s) {
        if (shape != s)
            Shape::deallocate(shape);
        shape = s;
        shape->compute_bounding_box();
    }

    void
    VisualNode::compute_shape(const NodeAllocator& na) {
        int number_of_shapes = get_number_of_children();
        Extent extent;
        if (na.has_label(this)) {
            int ll = na.get_label(this).length();
            ll *= 7;
            VisualNode* p = get_parent(na);
            int alt = 0;
            int n_alt = 1;
            if (p) {
                alt = get_alternative(na);
                n_alt = p->get_number_of_children();
            }
            extent = Extent(Layout::extent);
            if (alt == 0 && n_alt > 1) {
                extent.l = std::min(extent.l, -ll);
            } else if (alt == n_alt - 1 && n_alt > 1) {
                extent.r = std::max(extent.r, ll);
            } else {
                extent.l = std::min(extent.l, -ll);
                extent.r = std::max(extent.r, ll);
            }
        } else {
            if (number_of_shapes == 0) {
                set_shape(Shape::leaf);
                return;
            }
            extent = Extent(Layout::extent);
        }

        int max_depth = 0;
        for (int i = number_of_shapes; i--;)
            max_depth = std::max(max_depth, get_child(na, i)->get_shape()->depth());
        Shape* merged_shape;
        if (get_shape() && get_shape() != Shape::leaf && get_shape()->depth() >= max_depth + 1) {
            merged_shape = get_shape();
            merged_shape->set_depth(max_depth + 1);
        } else {
            merged_shape = Shape::allocate(max_depth + 1);
        }
        (*merged_shape)[0] = extent;
        if (number_of_shapes < 1) {
            set_shape(merged_shape);
        } else if (number_of_shapes == 1) {
            get_child(na, 0)->set_offset(0);
            const Shape* child_shape = get_child(na, 0)->get_shape();
            for (int i = child_shape->depth(); i--;)
                (*merged_shape)[i + 1] = (*child_shape)[i];
            (*merged_shape)[1].extend(-extent.l, -extent.r);
            set_shape(merged_shape);
        } else {
            // alpha stores the necessary distances between the
            // axes of the shapes in the list: alpha[i].first gives the distance
            // between shape[i] and shape[i-1], when shape[i-1] and shape[i]
            // are merged left-to-right; alpha[i].second gives the distance between
            // shape[i] and shape[i+1], when shape[i] and shape[i+1] are merged
            // right-to-left.
            Region r;
            std::pair<int, int>* alpha = r.alloc<std::pair<int, int>>(number_of_shapes);

            // distance between the leftmost and the rightmost axis in the list
            int width = 0;

            Extent* current_shape_l = r.alloc<Extent>(max_depth);
            int ldepth = get_child(na, 0)->get_shape()->depth();
            for (int i = ldepth; i--;)
                current_shape_l[i] = (*get_child(na, 0)->get_shape())[i];

            // After merging, we can pick the result of either merging left or right
            // Here we chose the result of merging right
            Shape* r_shape = get_child(na, number_of_shapes - 1)->get_shape();
            int rdepth = r_shape->depth();
            for (int i = rdepth; i--;)
                (*merged_shape)[i + 1] = (*r_shape)[i];
            Extent* current_shape_r = &(*merged_shape)[1];

            for (int i = 1; i < number_of_shapes; i++) {
                // Merge left-to-right.  Note that due to the asymmetry of the
                // merge operation, nextAlphaL is the distance between the
                // *leftmost* axis in the shape list, and the axis of
                // nextShapeL; what we are really interested in is the distance
                // between the *previous* axis and the axis of nextShapeL.
                // This explains the correction.

                Shape* next_shape_l = get_child(na, i)->get_shape();
                int next_alpha_l = Layouter::get_alpha<Extent*, Shape>(&current_shape_l[0], ldepth, *next_shape_l,
                                                                       next_shape_l->depth());
                Layouter::merge<Extent*, Shape>(&current_shape_l[0], &current_shape_l[0], ldepth, *next_shape_l,
                                                next_shape_l->depth(), next_alpha_l);
                ldepth = std::max(ldepth, next_shape_l->depth());
                alpha[i].first = next_alpha_l - width;
                width = next_alpha_l;

                // Merge right-to-left.  Here, a correction of nextAlphaR is
                // not required.
                Shape* next_shape_r = get_child(na, number_of_shapes - 1 - i)->get_shape();
                int next_alpha_r = Layouter::get_alpha<Shape, Extent*>(*next_shape_r, next_shape_r->depth(),
                                                                       &current_shape_r[0], rdepth);
                Layouter::merge<Shape, Extent*>(&current_shape_r[0], *next_shape_r, next_shape_r->depth(),
                                                &current_shape_r[0], rdepth, next_alpha_r);
                rdepth = std::max(rdepth, next_shape_r->depth());
                alpha[number_of_shapes - i].second = next_alpha_r;
            }

            // The merged shape has to be adjusted to its topmost extent
            (*merged_shape)[1].extend(-extent.l, -extent.r);

            // After the loop, the merged shape has the same axis as the
            // leftmost shape in the list.  What we want is to move the axis
            // such that it is the center of the axis of the leftmost shape in
            // the list and the axis of the rightmost shape.
            int half_width = false ? 0 : width / 2;
            (*merged_shape)[1].move(-half_width);

            // Finally, for the offset lists.  Now that the axis of the merged
            // shape is at the center of the two extreme axes, the first shape
            // needs to be offset by -halfWidth units with respect to the new
            // axis.  As for the offsets for the other shapes, we take the
            // median of the alphaL and alphaR values, as suggested in
            // Kennedy's paper.
            int offset = -half_width;
            get_child(na, 0)->set_offset(offset);
            for (int i = 1; i < number_of_shapes; i++) {
                offset += (alpha[i].first + alpha[i].second) / 2;
                get_child(na, i)->set_offset(offset);
            }
            set_shape(merged_shape);
        }
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
