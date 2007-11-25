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

  enum NodeStatus {
    SOLVED, FAILED, BRANCH, UNDETERMINED
  };

  class BestSpace {
  public:
    Space* s;
    Better* b;
    BestSpace(Space* s0, Better* b);
  };

  class SpaceNode : public Node {
  private:
    Space* copy;
    Space* workingSpace;
    
    const BranchingDesc* desc;
    
    NodeStatus status;
    
  protected:
    BestSpace* curBest;
  private:
    Space*     ownBest;
    
    int noOfOpenChildren;
    bool _hasFailedChildren;
    bool _hasSolvedChildren;
    
    int recompute(void);
    Space* donateSpace(int alt, Space* ownBest);
    Space* checkLAO(int alt, Space* ownBest);
    void acquireSpace(void);
    
    void solveUp(void);
    void closeChild(bool hadFailures, bool hadSolutions);
  public:
    SpaceNode(int alt, BestSpace* cb = NULL);
    SpaceNode(Space* root, Better* b);
    virtual ~SpaceNode(void);
    
    Space* getSpace(void);
    
    int getNumberOfChildNodes(void);
    NodeStatus getStatus(void);
    void setStatus(NodeStatus s);
    
    int getAlternative(void);
    
    bool isOpen(void);
    bool hasFailedChildren(void);
    bool hasSolvedChildren(void);
    int getNoOfOpenChildren(void);
    
    virtual SpaceNode* createChild(int alternative);
    virtual void changedStatus(void);
  };

}}

#endif

// STATISTICS: gist-any
