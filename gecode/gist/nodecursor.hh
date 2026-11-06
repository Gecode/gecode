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

#ifndef GECODE_GIST_NODECURSOR_HH
#define GECODE_GIST_NODECURSOR_HH

#include <gecode/gist/visualnode.hh>

namespace Gecode { namespace Gist {

    /// \brief A cursor that can be run over a tree
    template <class Node>
    class NodeCursor {
    private:
        /// The node where the iteration starts
        Node* _start_node;
        /// The current node
        Node* _node;
        /// The current alternative
        unsigned int _alternative;

    protected:
        /// The node allocator
        const Node::NodeAllocator& na;
        /// Set current node to \a n
        void node(Node* n);
        /// Return start node
        Node* start_node();

    public:
        /// Construct cursor, initially set to \a current_node
        NodeCursor(Node* current_node, const Node::NodeAllocator& na);
        /// Return current node
        Node* node();
        /// Return current alternative
        unsigned int alternative();
        /// Set current alternative
        void alternative(unsigned int a);

        /// \name Cursor interface
        //@{
        /// Test if the cursor may move to the parent node
        bool may_move_upwards();
        /// Move cursor to the parent node
        void move_upwards();
        /// Test if cursor may move to the first child node
        bool may_move_downwards();
        /// Move cursor to the first child node
        void move_downwards();
        /// Test if cursor may move to the first sibling
        bool may_move_sidewards();
        /// Move cursor to the first sibling
        void move_sidewards();
        //@}
    };

    /// \brief A cursor that marks failed subtrees as hidden
    class HideFailedCursor : public NodeCursor<VisualNode> {
    private:
        bool only_dirty;

    public:
        /// Constructor
        HideFailedCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na, bool only_dirty_nodes);
        /// \name Cursor interface
        //@{
        /// Test if the cursor may move to the first child node
        bool may_move_downwards();
        /// Process node
        void process_current_node();
        //@}
    };

    /// \brief A cursor that marks all nodes in the tree as not hidden
    class UnhideAllCursor : public NodeCursor<VisualNode> {
    public:
        /// Constructor
        UnhideAllCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na);
        /// \name Cursor interface
        //@{
        /// Process node
        void process_current_node();
        //@}
    };

    /// \brief A cursor that marks all nodes in the tree as not stopping
    class UnstopAllCursor : public NodeCursor<VisualNode> {
    public:
        /// Constructor
        UnstopAllCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na);
        /// \name Cursor interface
        //@{
        /// Process node
        void process_current_node();
        //@}
    };

    /// \brief A cursor that finds the next solution
    class NextSolCursor : public NodeCursor<VisualNode> {
    private:
        /// Whether to search backwards
        bool back;
        /// Whether the current node is not a solution
        bool not_on_sol();

    public:
        /// Constructor
        NextSolCursor(VisualNode* the_node, bool backwards, const VisualNode::NodeAllocator& na);
        /// \name Cursor interface
        //@{
        /// Do nothing
        void process_current_node();
        /// Test if the cursor may move to the parent node
        bool may_move_upwards();
        /// Test if cursor may move to the first child node
        bool may_move_downwards();
        /// Move cursor to the first child node
        void move_downwards();
        /// Test if cursor may move to the first sibling
        bool may_move_sidewards();
        /// Move cursor to the first sibling
        void move_sidewards();
        //@}
    };

    /// \brief A cursor that collects statistics
    class StatCursor : public NodeCursor<VisualNode> {
    private:
        /// Current depth
        int cur_depth;

    public:
        /// Depth of the search tree
        int depth;
        /// Number of failed nodes
        int failed;
        /// Number of solved nodes
        int solved;
        /// Number of choice nodes
        int choice;
        /// Number of open nodes
        int open;

        /// Constructor
        StatCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na);

        /// \name Cursor interface
        //@{
        /// Collect statistics
        void process_current_node();
        /// Move cursor to the first child node
        void move_downwards();
        /// Move cursor to the parent node
        void move_upwards();
        //@}
    };

    /// \brief A cursor that labels branches
    class BranchLabelCursor : public NodeCursor<VisualNode> {
    private:
        /// The node allocator
        VisualNode::NodeAllocator& _na;
        /// Current best solution
        BestNode* _cur_best;
        /// Recomputation distance
        int _c_d;
        /// Adaptive rcomputation distance
        int _a_d;
        /// Whether to clear labels
        bool _clear;

    public:
        /// Constructor
        BranchLabelCursor(VisualNode* the_node, BestNode* cur_best, int c_d, int a_d, bool clear,
                          VisualNode::NodeAllocator& na);
        /// \name Cursor interface
        //@{
        void process_current_node();
        //@}
    };

    /// \brief A cursor that frees all memory
    class DisposeCursor : public NodeCursor<VisualNode> {
    public:
        /// Constructor
        DisposeCursor(VisualNode* the_node, const VisualNode::NodeAllocator& na);

        /// \name Cursor interface
        //@{
        /// Dispose node
        void process_current_node();
        //@}
    };

}} // namespace Gecode::Gist

#include <gecode/gist/nodecursor.hpp>

#endif

// STATISTICS: gist-any
