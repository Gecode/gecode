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

#ifndef GECODE_GIST_VISUALNODE_HH
#define GECODE_GIST_VISUALNODE_HH

#include "gecode/gist/spacenode.hh"
#include "gecode/gist/shapelist.hh"
#include "gecode/kernel.hh"
#include <vector>
#ifdef GECODE_GIST_EXPERIMENTAL
#include <QList>
#endif

namespace Gecode { namespace Gist {

  /// \brief Node class that supports visual layout
  class VisualNode : public SpaceNode {
  protected:
    /// Relative offset from the parent node
    int offset;
    /// Whether the node needs re-layout
    bool dirty;
    /// Whether the node is hidden
    bool hidden;
    /// Whether the node is marked
    bool marked;
    
    /// Whether the node is on the selected path
    bool onPath;
    /// Whether the node is the head of the selected path
    bool lastOnPath;
    /// The alternative that is next on the path
    int pathAlternative;

#ifdef GECODE_GIST_EXPERIMENTAL
    /// Whether the node is expanded
    bool expanded;
    /// The parent of the node before expansion
    VisualNode* realParent;
    /// The alternative of the node before expansion
    int realAlternative;
    /// The points in time at which the node was inspected
    QList<int> pits;
#endif

    /// Heat value 
    int heat;

    /// Shape of this node
    Shape* shape;
    /// Bounding box of this node
    BoundingBox box;
    /// Check if the \a x at depth \a depth lies in this subtree
    bool containsCoordinateAtDepth(int x, int depth);
  public:
    /// Constructor
    VisualNode(int alternative, BestNode* b);
    /// Constructor for root node from \a root and \a b
    VisualNode(Space* root, Better* b);
    /// Destructor
    virtual ~VisualNode(void);
    
    /// Shape of a single node
    static const Shape singletonShape;
    /// Unit shape
    static const Shape unitShape;

    /// Return if node is hidden
    bool isHidden(void);
    /// Set hidden state to \a h
    void setHidden(bool h);
    /// Mark all nodes up the path to the parent as dirty
    void dirtyUp(void);
    /// Compute layout for the subtree of this node
    void layout(void);
    /// Return offset off this node from its parent
    int getOffset(void);
    /// Set offset of this node, relative to its parent
    void setOffset(int n);
    /// Return whether node is marked as dirty
    bool isDirty(void);
    /// Mark node as dirty
    void setDirty(bool d);
    /// Return whether node is marked
    bool isMarked(void);
    /// Set mark of this node
    void setMarked(bool m);
    /// Set all nodes from the node to the root to be on the path
    void pathUp(void);
    /// Set all nodes from the node to the root not to be on the path
    void unPathUp(void);
    /// Return whether node is on the path
    bool isOnPath(void);
    /// Return whether node is the head of the path
    bool isLastOnPath(void);
    /// Return the alternative of the child that is on the path (-1 if none)
    int getPathAlternative(void);
    /// Set the path attributes of the node
    void setPathInfos(bool onPath0, int pathAlternative0 = -1, bool lastOnPath0 = false);
    
#ifdef GECODE_GIST_EXPERIMENTAL
    /// Return whether node is expanded
    bool isExpanded(void);
    /// Set the node to be \a expanded
    void setExpanded(bool expanded);
    /// Set the node's real parent
    void setRealParent(VisualNode* parent);
    /// Get the node's real parent
    VisualNode* getRealParent(void);
    /// Set the node's real alternative
    void setRealAlternative(int alternative);
    /// Get the node's real alternative
    int getRealAlternative(void);
    /// Add a new point in time
    void addPit(int pit);
    /// Get the list of points in time
    QList<int> getPits(void);
#endif

    /// Return heat value
    int getHeat(void) const;
    /// Set heat value to \a h
    void setHeat(int h);
    
    /// Toggle whether this node is hidden
    void toggleHidden(void);
    /// Hide all failed subtrees of this node
    void hideFailed(void);
    /// Unhide all nodes in the subtree of this node
    void unhideAll(void);
    
    /// Return the shape of this node
    Shape* getShape(void);
    /// Set the shape of this node
    void setShape(Shape* s);
    /// Set the bounding box
    void setBoundingBox(BoundingBox b);
    /// Return the bounding box
    BoundingBox getBoundingBox(void);
    /// Create a child for alternative \a alternative
    virtual SpaceNode* createChild(int alternative);
#ifdef GECODE_GIST_EXPERIMENTAL
    /// Create a child step node
    VisualNode* createStepChild(int alt, StepDesc* d, bool fstStep, bool lstStep);
    /// Steal child node no \a n
    VisualNode* stealChild(int n);
#endif
    /// Signal that the status has changed
    virtual void changedStatus();
    /// Return child \a i
    VisualNode* getChild(int i);
    /// Find a node in this subtree at coordinates \a x, \a y
    VisualNode* findNode(int x, int y);    
  };

}}

#endif

// STATISTICS: gist-any
