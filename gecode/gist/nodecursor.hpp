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

    template <class Node>
    forceinline
    NodeCursor<Node>::NodeCursor(Node* current_node, const Node::NodeAllocator& na0)
        : _start_node(current_node), _node(current_node), _alternative(current_node->get_alternative(na0)), na(na0) {}

    template <class Node>
    forceinline Node*
    NodeCursor<Node>::node() {
        return _node;
    }

    template <class Node>
    forceinline unsigned int
    NodeCursor<Node>::alternative() {
        return _alternative;
    }

    template <class Node>
    forceinline void
    NodeCursor<Node>::alternative(unsigned int a) {
        _alternative = a;
    }

    template <class Node>
    forceinline Node*
    NodeCursor<Node>::start_node() {
        return _start_node;
    }

    template <class Node>
    forceinline void
    NodeCursor<Node>::node(Node* n) {
        _node = n;
    }

    template <class Node>
    forceinline bool
    NodeCursor<Node>::may_move_upwards() {
        return _node != _start_node && !_node->is_root();
    }

    template <class Node>
    forceinline void
    NodeCursor<Node>::move_upwards() {
        _node = static_cast<Node*>(_node->get_parent(na));
        if (_node->is_root()) {
            _alternative = 0;
        } else {
            Node* p = static_cast<Node*>(_node->get_parent(na));
            for (int i = p->get_number_of_children(); i--;) {
                if (p->get_child(na, i) == _node) {
                    _alternative = i;
                    break;
                }
            }
        }
    }

    template <class Node>
    forceinline bool
    NodeCursor<Node>::may_move_downwards() {
        return _node->get_number_of_children() > 0;
    }

    template <class Node>
    forceinline void
    NodeCursor<Node>::move_downwards() {
        _alternative = 0;
        _node = _node->get_child(na, 0);
    }

    template <class Node>
    forceinline bool
    NodeCursor<Node>::may_move_sidewards() {
        return (!_node->is_root()) && (_node != _start_node) &&
               (_alternative < _node->get_parent(na)->get_number_of_children() - 1);
    }

    template <class Node>
    forceinline void
    NodeCursor<Node>::move_sidewards() {
        _node = static_cast<Node*>(_node->get_parent(na)->get_child(na, ++_alternative));
    }

    forceinline bool
    HideFailedCursor::may_move_downwards() {
        VisualNode* n = node();
        return (!only_dirty || n->is_dirty()) && NodeCursor<VisualNode>::may_move_downwards() &&
               (n->has_solved_children() || n->get_no_of_open_children(na) > 0) && (!n->is_hidden());
    }

    forceinline
    HideFailedCursor::HideFailedCursor(VisualNode* root, const VisualNode::NodeAllocator& na, bool only_dirty_nodes)
        : NodeCursor<VisualNode>(root, na), only_dirty(only_dirty_nodes) {}

    forceinline void
    HideFailedCursor::process_current_node() {
        VisualNode* n = node();
        if (n->get_status() == BRANCH && !n->has_solved_children() && n->get_no_of_open_children(na) == 0) {
            n->set_hidden(true);
            n->set_children_layout_done(false);
            n->dirty_up(na);
        }
    }

    forceinline
    UnhideAllCursor::UnhideAllCursor(VisualNode* root, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(root, na) {}

    forceinline void
    UnhideAllCursor::process_current_node() {
        VisualNode* n = node();
        if (n->is_hidden()) {
            n->set_hidden(false);
            n->dirty_up(na);
        }
    }

    forceinline
    UnstopAllCursor::UnstopAllCursor(VisualNode* root, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(root, na) {}

    forceinline void
    UnstopAllCursor::process_current_node() {
        VisualNode* n = node();
        if (n->get_status() == STOP) {
            n->set_stop(false);
            n->dirty_up(na);
        }
    }

    forceinline
    NextSolCursor::NextSolCursor(VisualNode* the_node, bool backwards, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(the_node, na), back(backwards) {}

    forceinline void
    NextSolCursor::process_current_node() {}

    forceinline bool
    NextSolCursor::not_on_sol() {
        return node() == start_node() || node()->get_status() != SOLVED;
    }

    forceinline bool
    NextSolCursor::may_move_upwards() {
        return not_on_sol() && !node()->is_root();
    }

    forceinline bool
    NextSolCursor::may_move_downwards() {
        return not_on_sol() && !(back && node() == start_node()) && node()->has_solved_children() &&
               NodeCursor<VisualNode>::may_move_downwards();
    }

    forceinline void
    NextSolCursor::move_downwards() {
        NodeCursor<VisualNode>::move_downwards();
        if (back) {
            while (NodeCursor<VisualNode>::may_move_sidewards())
                NodeCursor<VisualNode>::move_sidewards();
        }
    }

    forceinline bool
    NextSolCursor::may_move_sidewards() {
        if (back) {
            return not_on_sol() && !node()->is_root() && alternative() > 0;
        }
        return not_on_sol() && !node()->is_root() &&
               (alternative() < node()->get_parent(na)->get_number_of_children() - 1);
    }

    forceinline void
    NextSolCursor::move_sidewards() {
        if (back) {
            alternative(alternative() - 1);
            node(node()->get_parent(na)->get_child(na, alternative()));
        } else {
            NodeCursor<VisualNode>::move_sidewards();
        }
    }

    forceinline
    StatCursor::StatCursor(VisualNode* root, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(root, na), cur_depth(0), depth(0), failed(0), solved(0), choice(0), open(0) {}

    forceinline void
    StatCursor::process_current_node() {
        VisualNode* n = node();
        switch (n->get_status()) {
        case SOLVED:
            solved++;
            break;
        case FAILED:
            failed++;
            break;
        case BRANCH:
            choice++;
            break;
        case UNDETERMINED:
            open++;
            break;
        default:
            break;
        }
    }

    forceinline void
    StatCursor::move_downwards() {
        cur_depth++;
        depth = std::max(depth, cur_depth);
        NodeCursor<VisualNode>::move_downwards();
    }

    forceinline void
    StatCursor::move_upwards() {
        cur_depth--;
        NodeCursor<VisualNode>::move_upwards();
    }

    forceinline
    BranchLabelCursor::BranchLabelCursor(VisualNode* root, BestNode* cur_best, int c_d, int a_d, bool clear,
                                         VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(root, na), _na(na), _cur_best(cur_best), _c_d(c_d), _a_d(a_d), _clear(clear) {}

    forceinline void
    BranchLabelCursor::process_current_node() {
        VisualNode* n = node();
        if (!_clear) {
            if (!na.has_label(n)) {
                VisualNode* p = n->get_parent(_na);
                if (p) {
                    std::string l = n->get_branch_label(_na, p, p->get_choice(), _cur_best, _c_d, _a_d, alternative());
                    _na.set_label(n, QString(l.c_str()));
                    if (n->get_number_of_children() < 1 && alternative() == p->get_number_of_children() - 1)
                        p->purge(_na);
                } else {
                    _na.set_label(n, "");
                }
            }
        } else {
            _na.clear_label(n);
        }
        n->dirty_up(na);
    }

    forceinline
    DisposeCursor::DisposeCursor(VisualNode* root, const VisualNode::NodeAllocator& na)
        : NodeCursor<VisualNode>(root, na) {}

    forceinline void
    DisposeCursor::process_current_node() {
        node()->dispose();
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
