/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <gecode/support.hh>

namespace Gecode { namespace Gist {

  class VisualNode;

  static const int NodeBlockSize = 10000;

  /// \brief Base class for nodes of the search tree
  class Node : public Support::BlockClient<VisualNode,NodeBlockSize> {
  private:
    /// The parent of this node, or NULL for the root
    Node* parent;    

    /// Tags that are used to encode the number of children
    enum {
      UNDET, //< Number of children not determined
      LEAF,  //< Leaf node
      TWO_CHILDREN, //< Node with at most two children
      MORE_CHILDREN //< Node with more than two children
    };

    /// The children, or in case there are at most two, the first child
    void* childrenOrFirstChild;
    
    union {
      /// The second child or NULL, in case of at most two children
      Node* secondChild;
      /// The number of children, in case it is greater than 2
      unsigned int noOfChildren;
    } c;

    /// Read the tag of childrenOrFirstChild
    unsigned int getTag(void) const;
    /// Set the tag of childrenOrFirstChild
    void setTag(unsigned int tag);
    /// Return childrenOrFirstChild without tag
    void* getPtr(void) const;

  protected:
    /// Return whether this node is undetermined
    bool isUndetermined(void) const;

  public:
    typedef Support::BlockAllocator<VisualNode,NodeBlockSize> NodeAllocator;
    
    /// Default constructor
    Node(void);
    /// Destructor
    ~Node(void);
    
    /// Return the parent
    Node* getParent(void);
    /// Return child no \a n
    Node* getChild(unsigned int n);
    
    /// Check if this node is the root of a tree
    bool isRoot(void) const;

    /// Set the number of children to \a n
    void setNumberOfChildren(unsigned int n);
    /// Set child number \a n to be \a child
    void setChild(unsigned int n, Node* child);

    /// Add new child node
    void addChild(Node* child);

    /// Return the number of children
    unsigned int getNumberOfChildren(void) const;
  };

}}

#include <gecode/gist/node.hpp>

#endif

// STATISTICS: gist-any
