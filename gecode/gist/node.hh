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

#ifndef GECODE_GIST_NODE_HH
#define GECODE_GIST_NODE_HH

#include <gecode/kernel.hh>

#include <QHash>
#include <QString>

namespace Gecode { namespace Gist {

    class VisualNode;

    /// Node allocator
    template <class T>
    class NodeAllocatorBase {
    private:
        /// Size of each block of nodes
        static const int node_block_size = 1 << 14;
        /// Blocks of nodes
        class Block {
        public:
            /// The actual nodes
            T b[node_block_size];
            /// The index of the best known previous solution for each node
            int best[node_block_size];
        };
        /// Array of blocks
        Block** b;
        /// Size of the array
        int n;
        /// Current block number
        int cur_b;
        /// Current node number in current block
        int cur_t;
        /// Allocate new block, potentially reallocate block array
        void allocate();
        /// Flag whether search uses branch-and-bound
        bool _bab;
        /// Hash table mapping nodes to label text
        QHash<T*, QString> labels;

    public:
        /// Constructor
        NodeAllocatorBase(bool bab);
        /// Destructor
        ~NodeAllocatorBase();
        /// Allocate new node with parent \a p
        int allocate(int p);
        /// Allocate new root node for space \a root
        int allocate(Space* root);
        /// Return node for index \a i
        T* operator[](int i) const;
        /// Return index of best node before \a i
        T* best(int i) const;
        /// Set index of best node before \a i to \a b
        void set_best(int i, int b);
        /// Return branch-and-bound flag
        bool bab() const;
        /// Return branching label flag
        bool show_labels() const;
        /// Set branching label flag
        void show_labels(bool b);
        /// Return whether node \a n has a label
        bool has_label(T* n) const;
        /// Set label of node \a n to \a l
        void set_label(T* n, const QString& l);
        /// Remove label of node \a n
        void clear_label(T* n);
        /// Get label of node \a n
        QString get_label(T* n) const;
    };

    /// \brief Base class for nodes of the search tree
    class Node {
    private:
        /// Tags that are used to encode the number of children
        enum {
            UNDET,        //< Number of children not determined
            LEAF,         //< Leaf node
            TWO_CHILDREN, //< Node with at most two children
            MORE_CHILDREN //< Node with more than two children
        };

        /// The children, or in case there are at most two, the first child
        void* children_or_first_child;

        /** The number of children, in case it is greater than 2, or the first
         *  child (if negative)
         */
        int no_of_children;

        /// The parent of this node, or nullptr for the root
        int parent;

        /// Read the tag of childrenOrFirstChild
        unsigned int get_tag() const;
        /// Set the tag of childrenOrFirstChild
        void set_tag(unsigned int tag);
        /// Return childrenOrFirstChild without tag
        void* get_ptr() const;
        /// Return childrenOrFirstChild as integer
        int get_first_child() const;

    protected:
        /// Return whether this node is undetermined
        bool is_undetermined() const;

        /// Return index of child no \a n
        int get_child(int n) const;

    public:
        using NodeAllocator = NodeAllocatorBase<VisualNode>;

        /// Construct node with parent \a p
        Node(int p, bool failed = false);

        /// Return the parent
        int get_parent() const;
        /// Return the parent
        VisualNode* get_parent(const NodeAllocator& na) const;
        /// Return child no \a n
        VisualNode* get_child(const NodeAllocator& na, int n) const;

        /// Return index of this node
        int get_index(const NodeAllocator& na) const;

        /// Check if this node is the root of a tree
        bool is_root() const;

        /// Set the number of children to \a n and initialize children
        void set_number_of_children(unsigned int n, NodeAllocator& na);

        /// Return the number of children
        unsigned int get_number_of_children() const;
    };

}} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
