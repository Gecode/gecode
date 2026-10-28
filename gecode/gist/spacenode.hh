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

#ifndef GECODE_GIST_SPACENODE_HH
#define GECODE_GIST_SPACENODE_HH

#include <gecode/gist/node.hh>
#include <gecode/kernel.hh>

namespace Gecode { namespace Gist {

    /** \brief Status of nodes in the search tree
     */
    enum NodeStatus {
        SOLVED,       ///< Node representing a solution
        FAILED,       ///< Node representing failure
        BRANCH,       ///< Node representing a branch
        UNDETERMINED, ///< Node that has not been explored yet
        STOP,         ///< Node representing stop point
        UNSTOP,       ///< Node representing ignored stop point
    };

    static const unsigned int first_bit = 24;                //< First free bit in status word
    static const unsigned int status_mask = 7 << 20;         //< Mask for accessing status
    static const unsigned int max_distance = (1 << 20) - 1;  //< Maximum representable distance
    static const unsigned int distance_mask = (1 << 20) - 1; //< Mask for accessing distance

    /// %Statistics about the search tree
    class Statistics : public StatusStatistics {
    public:
        /// Number of solutions
        int solutions;
        /// Number of failures
        int failures;
        /// Number of choice nodes
        int choices;
        /// Number of open, undetermined nodes
        int undetermined;
        /// Maximum depth of the tree
        int max_depth;

        /// Constructor
        Statistics(void) : solutions(0), failures(0), choices(0), undetermined(1), max_depth(0) {}
    };

    class SpaceNode;

    /// \brief Static reference to the currently best space
    class BestNode {
    public:
        /// The currently best node found, or nullptr
        SpaceNode* s;
        /// Constructor
        BestNode(SpaceNode* s0);
    };

    /// \brief A node of a search tree of %Gecode spaces
    class SpaceNode : public Node {
    protected:
        /** \brief A copy used for recomputation, or nullptr
         *
         * If the copy is marked, it is a working copy, i.e.,
         * it does not have to be kept for recomputation.
         */
        Space* copy;

    protected:
        const Choice* choice;

        /** \brief Status of the node
         *
         * If the node has a working copy, the first 20 bits encode the distance
         * to the closest copy. The next 5 bits encode the NodeStatus, and the
         * remaining bits are used by the VisualNode class for further flags.
         */
        unsigned int status_bits;

        /// Set distance from copy
        void set_distance(unsigned int d);

        /// Return distance from copy
        unsigned int get_distance(void) const;

        /// Set status flag
        void set_flag(int flag, bool value);

        /// Return status flag
        bool get_flag(int flag) const;

        /// Flags for SpaceNodes
        enum SpaceNodeFlags { HAS_OPEN_CHILDREN = first_bit, HAS_FAILED_CHILDREN, HAS_SOLVED_CHILDREN };
        /// Last bit used for SpaceNode flags
        static const int last_bit = HAS_SOLVED_CHILDREN;

    private:
        /// Set whether the node has children that are not fully explored
        void set_has_open_children(bool b);
        /// Set whether the subtree of this node is known to contain failure
        void set_has_failed_children(bool b);
        /// Set whether the subtree of this node is known to contain solutions
        void set_has_solved_children(bool b);

        /// Recompute workingSpace from a copy higher up, return distance to copy
        int recompute(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d);

        /// Book-keeping of open children
        void close_child(const NodeAllocator& na, bool had_failures, bool had_solutions);

    protected:
        /// Set status to \a s
        void set_status(NodeStatus s);
        /// Acquire working space, either from parent or by recomputation
        void acquire_space(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d);

    public:
        /// Construct node with parent \a p
        SpaceNode(int p);
        /// Construct root node from Space \a root and branch-and-bound object \a better
        SpaceNode(Space* root);

        /// Return working space.  Receiver must delete the space.
        Space* get_space(NodeAllocator& na, BestNode* cur_best, int c_d, int a_d);

        /// Return working space (if present).
        const Space* get_working_space(void) const;

        /// Clear working space and copy (if present and this is not the root).
        void purge(const NodeAllocator& na);

        /// Free allocated memory
        void dispose(void);

        /// Return whether this node is the currently best solution
        bool is_current_best(BestNode* cur_best);

        /** \brief Compute and return the number of children
         *
         * On a node whose status is already determined, this function
         * just returns the number of children.  On an undetermined node,
         * it first acquires a Space (possibly through recomputation), and
         * then asks for its status.  If the space is solved or failed, the
         * node's status will be set accordingly, and 0 will be returned.
         * Otherwise, the status is SS_BRANCH, and as many new children will
         * be created as the branch has alternatives, and the number returned.
         */
        int get_number_of_child_nodes(NodeAllocator& na, BestNode* cur_best, Statistics& stats, int c_d, int a_d);

        /// Return current status of the node
        NodeStatus get_status(void) const;

        /// Return whether this node still has open children
        bool is_open(void);
        /// Return whether the subtree of this node has any failed children
        bool has_failed_children(void);
        /// Return whether the subtree of this node has any solved children
        bool has_solved_children(void);
        /// Return whether the subtree of this node has any open children
        bool has_open_children(void);
        /// Return number of open children
        int get_no_of_open_children(const NodeAllocator& na);
        /// Set number of open children to \a n
        void set_no_of_open_children(int n);
        /// Return whether the node has a copy
        bool has_copy(void);
        /// Return whether the node has a working space
        bool has_working_space(void);

        /// Return alternative number of this node
        int get_alternative(const NodeAllocator& na) const;
        /// Return choice of this node
        const Choice* get_choice(void);
    };

}} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
