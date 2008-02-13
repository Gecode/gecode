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

#ifndef GECODE_GIST_SPACENODE_HH
#define GECODE_GIST_SPACENODE_HH

#include "gecode/gist/node.hh"
#include "gecode/gist/better.hh"
#include "gecode/kernel.hh"

namespace Gecode { namespace Gist {

  /// \brief Status of nodes in the search tree
  enum NodeStatus {
    SOLVED,       ///< Node representing a solution
    FAILED,       ///< Node representing failure
    BRANCH,       ///< Node representing a branch
    UNDETERMINED, ///< Node that has not been explored yet
    SPECIAL,      ///< Node representing user controlled exploration
    STEP          ///< Node representing one propagation step
  };

  // TODO nikopp: doxygen comments
  class StepDesc {
  public:
    int noOfSteps;
    bool debug;
    StepDesc(int steps);
    void toggleDebug(void);
  };
  
  // TODO nikopp: doxygen comments
  class SpecialDesc {
  public:
    const std::string vn;
    const int v;
    const int rel;
    SpecialDesc(std::string varName, int rel0, int v0);
  };
  
  /// Statistics about the search tree
  class Statistics {
  public:
    int solutions;
    int failures;
    int choices;
    int undetermined;
    int maxDepth;
    
    Statistics(void)
    : solutions(0), failures(0), choices(0), undetermined(1), maxDepth(0) {}
    
    static Statistics dummy;
  };

  class SpaceNode;

  /// \brief Static reference to the currently best space
  class BestNode {
  public:
    /// The currently best node found, or NULL
    SpaceNode* s;
    /// The object used for constraining spaces to be better
    Better* b;
    /// Constructor
    BestNode(SpaceNode* s0, Better* b);
  };

  /// \brief A node of a search tree of Gecode spaces
  class SpaceNode : public Node {
  private:
    /// A copy used for recomputation, or NULL
    Space* copy;
    /// Working space used for computing the status
    Space* workingSpace;
    
    union {
      /// Branching description
      const BranchingDesc* branch;
      /// Special branching description
      const SpecialDesc* special;
      /// Step description
      StepDesc* step;
    } desc;
    
    /// Current status of the node
    NodeStatus status;

#ifdef GECODE_GIST_EXPERIMENTAL
    
    /// Current meta-status of the special node
    NodeStatus metaStatus;

    /// Whether the node is the first step node
    bool firstStep;
    /// Whether the node is the last step node
    bool lastStep;
    
  public:
    /// Debug variable to trigger watchpoint
    static bool watch;
#endif
    
  protected:
    /// Reference to currently best node (for branch-and-bound)
    BestNode* curBest;
  private:
    /// Reference to best space when the node was created
    SpaceNode* ownBest;
    
    /// Number of children that are not fully explored
    int noOfOpenChildren;
    /// Whether the subtree of this node is known to contain failure
    bool _hasFailedChildren;
    /// Whether the subtree of this node is known to contain solutions
    bool _hasSolvedChildren;
    
    /// Recompute workingSpace from a copy higher up, return distance to copy
    int recompute(void);
    /// Try to get workingSpace from parent
    Space* donateSpace(int alt, SpaceNode* ownBest);
    /// Try to get copy from parent if this node is the last alternative
    Space* checkLAO(int alt, SpaceNode* ownBest);
    /// Acquire working space, either through donateSpace or recompute
    void acquireSpace(void);

    /// Mark all ancestors as having solved children
    void solveUp(void);
    /// Book-keeping of open children
    void closeChild(bool hadFailures, bool hadSolutions);
  public:
    /// Construct node for alternative \a alt
    SpaceNode(int alt, BestNode* cb = NULL);
    /// Construct root node from Space \a root and branch-and-bound object \a better
    SpaceNode(Space* root, Better* b);
    /// Destructor
    virtual ~SpaceNode(void);

    /// Return the branch-and-bound wrapper
    Better* getBetterWrapper(void) const;

    /// Return working space.  Receiver must delete the space.
    Space* getSpace(void);

    /// Return whether this node is the currently best solution
    bool isCurrentBest(void);

#ifdef GECODE_GIST_EXPERIMENTAL

    /** Return initial working space (before any propagation is performed).
     * Receiver must delete the space.
     * Note that this does not not work for the root node.
     */
    Space* getInputSpace(void);
    
    /// Return whether this node represents the first step node
    bool isFirstStepNode(void);
    /// Return whether this node represents the last step node
    bool isLastStepNode(void);
    /// Set the node to represent the first step
    void setFirstStepNode(bool firstStep);
    /// Set the node to represent the last step
    void setLastStepNode(bool lastStep);

    /// Whether the subtree of this node is known to contain failure
    void setHasFailedChildren(bool b);
    /// Whether the subtree of this node is known to contain solutions
    void setHasSolvedChildren(bool b);
    
    /// Parse JavaScript \a model and post constraints in space
    void executeJavaScript(const std::string& model);
    /// Replace the current copy of the node with \a s
    void replaceCopy(Space* s);
#endif

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
    int getNumberOfChildNodes(Statistics& stats = Statistics::dummy);
    
    /// Return current status of the node
    NodeStatus getStatus(void);
    /// Change the status of the to \a s
    void setStatus(NodeStatus s);
#ifdef GECODE_GIST_EXPERIMENTAL
    /// Return current meta-status of the node
    NodeStatus getMetaStatus(void);
    /// Change the meta-status of the node to \a s
    void setMetaStatus(NodeStatus s);
#endif
    /// Return whether this node represents a propagation step
    bool isStepNode(void);
    /// Change the SpecialDesc to \a d
    void setSpecialDesc(const SpecialDesc* d);
    /// Change the StepDesc to \a d
    void setStepDesc(StepDesc* d);
    /// Return the StepDesc
    StepDesc* getStepDesc(void);
    
    /// Return alternative number of this node
    int getAlternative(void);
    /// Return whether this node still has open children
    bool isOpen(void);
    /// Opens all nodes on the path up to the root
    void openUp(void);
    /// Return whether the subtree of this node has any failed children
    bool hasFailedChildren(void);
    /// Return whether the subtree of this node has any solved children
    bool hasSolvedChildren(void);
    /// Return number of open children
    int getNoOfOpenChildren(void);
    /// Set number of open children to \a n
    void setNoOfOpenChildren(int n);
    /// Return whether the node has a copy
    bool hasCopy(void);
    /// Return whether the node has a working space
    bool hasWorkingSpace(void);
       
    /// Return the parent
    SpaceNode* getParent(void);
    /// Return child \a i
    SpaceNode* getChild(int i);
    /// Pseudo-constructor to allow creation of nodes of sub-classes from getNoOfChildNodes
    virtual SpaceNode* createChild(int alternative);
    /// Called when the status has changed
    virtual void changedStatus(void);
  };

}}

#endif

// STATISTICS: gist-any
