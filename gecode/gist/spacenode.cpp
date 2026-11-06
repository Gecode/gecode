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

#include <gecode/gist/spacenode.hh>
#include <gecode/gist/visualnode.hh>
#include <gecode/gist/stopbrancher.hh>
#include <gecode/search.hh>
#include <stack>

#include <QString>
#include <QVector>

namespace Gecode { namespace Gist {

    /// \brief Representation of a branch in the search tree
    class Branch {
    public:
        /// Alternative number
        int alternative;
        /// The best space known when the branch was created
        SpaceNode* own_best;
        const Choice* choice;

        /// Constructor
        Branch(int a, const Choice* c, SpaceNode* best = nullptr) : alternative(a), own_best(best) {
            choice = c;
        }
    };

    BestNode::BestNode(SpaceNode* s0) : s(s0) {}

    int
    SpaceNode::recompute(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d) {
        int rdist = 0;

        if (copy == nullptr) {
            SpaceNode* cur_node = this;
            SpaceNode* last_fixpoint = nullptr;

            last_fixpoint = cur_node;

            std::stack<Branch> stck;

            int idx = get_index(na);
            while (cur_node->copy == nullptr) {
                SpaceNode* parent = cur_node->get_parent(na);
                int parent_idx = cur_node->get_parent();
                int alternative = cur_node->get_alternative(na);

                SpaceNode* own_best = na.best(idx);
                Branch b(alternative, parent->choice, cur_best == nullptr ? nullptr : own_best);
                stck.push(b);

                cur_node = parent;
                idx = parent_idx;
                rdist++;
            }

            Space* cur_space;
            if (Support::marked(cur_node->copy)) {
                cur_space = static_cast<Space*>(Support::unmark(cur_node->copy));
                cur_node->copy = nullptr;
                a_d = -1;
            } else {
                cur_space = cur_node->copy->clone();
                cur_node->set_distance(0);
            }

            SpaceNode* last_best = nullptr;
            SpaceNode* middle_node = cur_node;
            int cur_dist = 0;

            while (!stck.empty()) {
                if (a_d >= 0 && cur_dist > a_d && cur_dist == rdist / 2) {
                    if (cur_space->status() == SS_FAILED) {
                        copy = static_cast<Space*>(Support::mark(cur_space));
                        return rdist;
                    }
                    middle_node->copy = cur_space->clone();
                }
                Branch b = stck.top();
                stck.pop();

                if (middle_node == last_fixpoint) {
                    cur_space->status();
                }

                cur_space->commit(*b.choice, b.alternative);

                if (b.own_best != nullptr && b.own_best != last_best) {
                    b.own_best->acquire_space(na, cur_best, c_d, a_d);
                    Space* own_best_space = static_cast<Space*>(Support::clear_pointer_tag_bits(b.own_best->copy));
                    if (own_best_space->status() != SS_SOLVED) {
                        // in the presence of weakly monotonic propagators, we may have to
                        // use search to find the solution here
                        own_best_space = Gecode::dfs(own_best_space);
                        if (Support::marked(b.own_best->copy)) {
                            delete static_cast<Space*>(Support::unmark(b.own_best->copy));
                            b.own_best->copy = static_cast<Space*>(Support::mark(own_best_space));
                        } else {
                            delete b.own_best->copy;
                            b.own_best->copy = own_best_space;
                        }
                    }
                    cur_space->constrain(*own_best_space);
                    last_best = b.own_best;
                }
                cur_dist++;
                middle_node = middle_node->get_child(na, b.alternative);
                middle_node->set_distance(cur_dist);
            }
            copy = static_cast<Space*>(Support::mark(cur_space));
        }
        return rdist;
    }

    void
    SpaceNode::acquire_space(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d) {
        SpaceNode* p = get_parent(na);
        int parent_idx = get_parent();
        int idx = get_index(na);

        if (get_status() == UNDETERMINED && cur_best != nullptr && na.best(idx) == nullptr && p != nullptr &&
            cur_best->s != na.best(parent_idx)) {
            na.set_best(idx, cur_best->s->get_index(na));
        }
        SpaceNode* own_best = na.best(idx);

        if (copy == nullptr && p != nullptr && p->copy != nullptr && Support::marked(p->copy)) {
            // If parent has a working space, steal it
            copy = p->copy;
            p->copy = nullptr;
            if (p->choice != nullptr)
                static_cast<Space*>(Support::unmark(copy))->commit(*p->choice, get_alternative(na));

            if (own_best != nullptr) {
                own_best->acquire_space(na, cur_best, c_d, a_d);
                Space* own_best_space = static_cast<Space*>(Support::clear_pointer_tag_bits(own_best->copy));
                if (own_best_space->status() != SS_SOLVED) {
                    // in the presence of weakly monotonic propagators, we may have to
                    // use search to find the solution here

                    own_best_space = Gecode::dfs(own_best_space);
                    if (Support::marked(own_best->copy)) {
                        delete static_cast<Space*>(Support::unmark(own_best->copy));
                        own_best->copy = static_cast<Space*>(Support::mark(own_best_space));
                    } else {
                        delete own_best->copy;
                        own_best->copy = own_best_space;
                    }
                }
                static_cast<Space*>(Support::unmark(copy))->constrain(*own_best_space);
            }
            int d = p->get_distance() + 1;
            if (d > c_d && c_d >= 0 && static_cast<Space*>(Support::unmark(copy))->status() == SS_BRANCH) {
                copy = static_cast<Space*>(Support::unmark(copy));
                d = 0;
            }
            set_distance(d);
        }

        if (copy == nullptr) {
            if (recompute(na, cur_best, c_d, a_d) > c_d && c_d >= 0 &&
                static_cast<Space*>(Support::unmark(copy))->status() == SS_BRANCH) {
                copy = static_cast<Space*>(Support::unmark(copy));
                set_distance(0);
            }
        }

        // always return a fixpoint
        static_cast<Space*>(Support::clear_pointer_tag_bits(copy))->status();
        if (Support::marked(copy) && p != nullptr && is_open() && p->copy != nullptr &&
            p->get_no_of_open_children(na) == 1 && !p->is_root()) {
            // last alternative optimization

            copy = static_cast<Space*>(Support::unmark(copy));
            delete static_cast<Space*>(Support::clear_pointer_tag_bits(p->copy));
            p->copy = nullptr;
            set_distance(0);
            p->set_distance(p->get_parent(na)->get_distance() + 1);
        }
    }

    void
    SpaceNode::close_child(const NodeAllocator& na, bool had_failures, bool had_solutions) {
        set_has_failed_children(has_failed_children() || had_failures);
        set_has_solved_children(has_solved_children() || had_solutions);

        bool all_closed = true;
        for (int i = get_number_of_children(); i--;) {
            if (get_child(na, i)->is_open()) {
                all_closed = false;
                break;
            }
        }

        if (all_closed) {
            set_has_open_children(false);
            for (unsigned int i = 0; i < get_number_of_children(); i++)
                set_has_solved_children(has_solved_children() || get_child(na, i)->has_solved_children());
            SpaceNode* p = get_parent(na);
            if (p != nullptr) {
                delete static_cast<Space*>(Support::clear_pointer_tag_bits(copy));
                copy = nullptr;
                p->close_child(na, has_failed_children(), has_solved_children());
            }
        } else {

            if (had_solutions) {
                set_has_solved_children(true);
                SpaceNode* p = get_parent(na);
                while (p != nullptr && !p->has_solved_children()) {
                    p->set_has_solved_children(true);
                    p = p->get_parent(na);
                }
            }
            if (had_failures) {
                SpaceNode* p = get_parent(na);
                while (p != nullptr && !p->has_failed_children()) {
                    p->set_has_failed_children(true);
                    p = p->get_parent(na);
                }
            }
        }
    }

    SpaceNode::SpaceNode(Space* root) : Node(-1, root == nullptr), copy(root), choice(nullptr), status_bits(0) {
        if (root == nullptr) {
            set_status(FAILED);
            set_has_solved_children(false);
            set_has_failed_children(true);
        } else {
            set_status(UNDETERMINED);
            set_has_solved_children(false);
            set_has_failed_children(false);
        }
    }

    void
    SpaceNode::dispose() {
        delete choice;
        delete static_cast<Space*>(Support::clear_pointer_tag_bits(copy));
    }

    int
    SpaceNode::get_number_of_child_nodes(NodeAllocator& na, BestNode* cur_best, Statistics& stats, int c_d, int a_d) {
        int kids = 0;
        if (is_undetermined()) {
            stats.undetermined--;
            acquire_space(na, cur_best, c_d, a_d);
            QVector<QString> labels;
            switch (static_cast<Space*>(Support::clear_pointer_tag_bits(copy))->status(stats)) {
            case SS_FAILED: {
                purge(na);
                kids = 0;
                set_has_open_children(false);
                set_has_solved_children(false);
                set_has_failed_children(true);
                set_status(FAILED);
                stats.failures++;
                SpaceNode* p = get_parent(na);
                if (p != nullptr)
                    p->close_child(na, true, false);
            } break;
            case SS_SOLVED: {
                // Deletes all pending branchers
                (void)static_cast<Space*>(Support::clear_pointer_tag_bits(copy))->choice();
                kids = 0;
                set_status(SOLVED);
                set_has_open_children(false);
                set_has_solved_children(true);
                set_has_failed_children(false);
                stats.solutions++;
                if (cur_best != nullptr) {
                    cur_best->s = this;
                }
                SpaceNode* p = get_parent(na);
                if (p != nullptr)
                    p->close_child(na, false, true);
            } break;
            case SS_BRANCH: {
                Space* s = static_cast<Space*>(Support::clear_pointer_tag_bits(copy));
                choice = s->choice();
                kids = choice->alternatives();
                set_has_open_children(true);
                if (dynamic_cast<const StopChoice*>(choice)) {
                    set_status(STOP);
                } else {
                    set_status(BRANCH);
                    stats.choices++;
                }
                stats.undetermined += kids;
                for (int i = 0; i < kids; i++) {
                    std::ostringstream oss;
                    s->print(*choice, i, oss);
                    labels.push_back(QString(oss.str().c_str()));
                }
            } break;
            }
            static_cast<VisualNode*>(this)->changed_status(na);
            set_number_of_children(kids, na);
        } else {
            kids = get_number_of_children();
        }
        return kids;
    }

    int
    SpaceNode::get_no_of_open_children(const NodeAllocator& na) {
        if (!has_open_children())
            return 0;
        int no_of_open_children = 0;
        for (int i = get_number_of_children(); i--;)
            no_of_open_children += (get_child(na, i)->is_open());
        return no_of_open_children;
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
