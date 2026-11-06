#include <algorithm>

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

    forceinline void
    SpaceNode::set_flag(int flag, bool value) {
        if (value)
            status_bits |= 1 << (flag - 1);
        else
            status_bits &= ~(1 << (flag - 1));
    }

    forceinline bool
    SpaceNode::get_flag(int flag) const {
        return (status_bits & (1 << (flag - 1))) != 0;
    }

    forceinline void
    SpaceNode::set_has_open_children(bool b) {
        set_flag(HAS_OPEN_CHILDREN, b);
    }

    forceinline void
    SpaceNode::set_has_failed_children(bool b) {
        set_flag(HAS_FAILED_CHILDREN, b);
    }

    forceinline void
    SpaceNode::set_has_solved_children(bool b) {
        set_flag(HAS_SOLVED_CHILDREN, b);
    }

    forceinline void
    SpaceNode::set_status(NodeStatus s) {
        status_bits &= ~status_mask;
        status_bits |= std::to_underlying(s) << 20;
    }

    forceinline NodeStatus
    SpaceNode::get_status() const {
        return static_cast<NodeStatus>((status_bits & status_mask) >> 20);
    }

    forceinline void
    SpaceNode::set_distance(unsigned int d) {
        d = std::min(d, max_distance);
        status_bits &= ~distance_mask;
        status_bits |= d;
    }

    forceinline unsigned int
    SpaceNode::get_distance() const {
        return status_bits & distance_mask;
    }

    forceinline
    SpaceNode::SpaceNode(int p)
        : Node(p), copy(nullptr), status_bits(0) {
        choice = nullptr;
        set_status(UNDETERMINED);
        set_has_solved_children(false);
        set_has_failed_children(false);
    }

    forceinline Space*
    SpaceNode::get_space(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d) {
        acquire_space(na, cur_best, c_d, a_d);
        Space* ret;
        if (Support::marked(copy)) {
            ret = static_cast<Space*>(Support::unmark(copy));
            copy = nullptr;
        } else {
            ret = copy->clone();
        }
        return ret;
    }

    forceinline const Space*
    SpaceNode::get_working_space() const {
        assert(copy != nullptr);
        if (Support::marked(copy))
            return static_cast<Space*>(Support::unmark(copy));
        return copy;
    }

    forceinline void
    SpaceNode::purge(const NodeAllocator& na) {
        if (!is_root() && (get_status() != SOLVED || !na.bab())) {
            // only delete copies from solutions if we are not in BAB
            if (Support::marked(copy))
                delete static_cast<Space*>(Support::unmark(copy));
            else
                delete copy;
            copy = nullptr;
        }
    }

    forceinline bool
    SpaceNode::is_current_best(BestNode* cur_best) {
        return cur_best != nullptr && cur_best->s == this;
    }

    forceinline bool
    SpaceNode::is_open() {
        return ((get_status() == UNDETERMINED) || get_flag(HAS_OPEN_CHILDREN));
    }

    forceinline bool
    SpaceNode::has_failed_children() {
        return get_flag(HAS_FAILED_CHILDREN);
    }

    forceinline bool
    SpaceNode::has_solved_children() {
        return get_flag(HAS_SOLVED_CHILDREN);
    }

    forceinline bool
    SpaceNode::has_open_children() {
        return get_flag(HAS_OPEN_CHILDREN);
    }

    forceinline bool
    SpaceNode::has_copy() {
        return copy != nullptr;
    }

    forceinline bool
    SpaceNode::has_working_space() {
        return copy != nullptr && Support::marked(copy);
    }

    forceinline int
    SpaceNode::get_alternative(const NodeAllocator& na) const {
        SpaceNode* p = get_parent(na);
        if (p == nullptr)
            return -1;
        for (int i = static_cast<int>(p->get_number_of_children()); i--;)
            if (p->get_child(na, i) == this)
                return i;
        GECODE_NEVER;
        return -1;
    }

    forceinline const Choice*
    SpaceNode::get_choice() {
        return choice;
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
