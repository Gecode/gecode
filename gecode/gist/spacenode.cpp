/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#include <gecode/gist/spacenode.hh>
#include <gecode/gist/visualnode.hh>
#include <gecode/gist/stopbrancher.hh>
#include <gecode/search.hh>
#include <stack>

namespace Gecode { namespace Gist {

  /// \brief Representation of a branch in the search tree
  class Branch {
  public:
    /// Alternative number
    int alternative;
    /// The best space known when the branch was created
    SpaceNode* ownBest;
    const Choice* choice;

    /// Constructor
    Branch(int a, const Choice* c, SpaceNode* best = NULL)
      : alternative(a), ownBest(best) {
        choice = c;
      }
  };

  BestNode::BestNode(SpaceNode* s0) : s(s0) {}

  int
  SpaceNode::recompute(BestNode* curBest, int c_d, int a_d) {
    int rdist = 0;

    if (workingSpace == NULL) {
      SpaceNode* curNode = this;
      SpaceNode* lastFixpoint = NULL;

      lastFixpoint = curNode;

      std::stack<Branch> stck;

      while (curNode->copy == NULL) {
        SpaceNode* parent = curNode->getParent();
        int alternative = curNode->getAlternative();

        Branch b(alternative, parent->choice,
                 curBest == NULL ? NULL : curNode->ownBest);
        stck.push(b);

        curNode = parent;
        rdist++;
      }
      Space* curSpace = curNode->copy->clone();
      curNode->setDistance(0);

      SpaceNode* lastBest = NULL;
      SpaceNode* middleNode = curNode;
      int curDist = 0;

      while (!stck.empty()) {
        if (a_d >= 0 &&
            curDist > a_d &&
            curDist == rdist / 2) {
            if (curSpace->status() == SS_FAILED) {
              workingSpace = curSpace;
              return rdist;
            } else {
              middleNode->copy = curSpace->clone();
            }
        }
        Branch b = stck.top(); stck.pop();

        if(middleNode == lastFixpoint) {
          curSpace->status();
        }

        curSpace->commit(*b.choice, b.alternative);

        if (b.ownBest != NULL && b.ownBest != lastBest) {
          b.ownBest->acquireSpace(curBest, c_d, a_d);
          if (b.ownBest->workingSpace->status() != SS_SOLVED) {
            // in the presence of weakly monotonic propagators, we may have to
            // use search to find the solution here
            Space* dfsSpace = Gecode::dfs(b.ownBest->workingSpace);
            delete b.ownBest->workingSpace;
            b.ownBest->workingSpace = dfsSpace;
          }
          curSpace->constrain(*b.ownBest->workingSpace);
          lastBest = b.ownBest;
        }
        curDist++;
        middleNode = middleNode->getChild(b.alternative);
        middleNode->setDistance(curDist);
      }
      workingSpace = curSpace;

    }
    return rdist;
  }

  void
  SpaceNode::acquireSpace(BestNode* curBest, int c_d, int a_d) {
    SpaceNode* p = getParent();

    if (getStatus() == UNDETERMINED && curBest != NULL && ownBest == NULL &&
        p != NULL && curBest->s != p->ownBest) {
          ownBest = curBest->s;
    }

    if (workingSpace == NULL && p != NULL && p->workingSpace != NULL) {
      // If parent has a working space, steal it
      workingSpace = p->workingSpace;
      p->workingSpace = NULL;
      if (p->choice != NULL)
        workingSpace->commit(*p->choice, getAlternative());

      if (ownBest != NULL) {
        ownBest->acquireSpace(curBest, c_d, a_d);
        if (ownBest->workingSpace->status() != SS_SOLVED) {
          // in the presence of weakly monotonic propagators, we may have to
          // use search to find the solution here
          Space* dfsSpace = Gecode::dfs(ownBest->workingSpace);
          delete ownBest->workingSpace;
          ownBest->workingSpace = dfsSpace;
        }
        workingSpace->constrain(*ownBest->workingSpace);
      }
      int d = p->getDistance()+1;
      if (d > c_d && c_d >= 0 &&
          workingSpace->status() == SS_BRANCH) {
        copy = workingSpace->clone();
        d = 0;
      }
      setDistance(d);
    }

    if (workingSpace == NULL) {
      if (recompute(curBest, c_d, a_d) > c_d && c_d >= 0 &&
          workingSpace->status() == SS_BRANCH) {
        copy = workingSpace->clone();
        setDistance(0);
      }
    }

    // always return a fixpoint
    workingSpace->status();
    if (copy == NULL && p != NULL && isOpen() &&
        p->copy != NULL && p->getNoOfOpenChildren() == 1 &&
        p->getParent() != NULL) {
      // last alternative optimization
      copy = p->copy;
      p->copy = NULL;
      setDistance(0);
      p->setDistance(p->getParent()->getDistance()+1);
      
      if(p->choice != NULL)
        copy->commit(*p->choice, getAlternative());

      if (ownBest != NULL) {
        ownBest->acquireSpace(curBest, c_d, a_d);
        if (ownBest->workingSpace->status() != SS_SOLVED) {
          // in the presence of weakly monotonic propagators, we may have to
          // use search to find the solution here
          Space* dfsSpace = Gecode::dfs(ownBest->workingSpace);
          delete ownBest->workingSpace;
          ownBest->workingSpace = dfsSpace;
        }
        copy->constrain(*ownBest->workingSpace);
      }
      if (copy->status() == SS_FAILED) {
        delete copy;
        copy = NULL;
      }
    }
  }

  void
  SpaceNode::closeChild(bool hadFailures, bool hadSolutions) {
    setHasFailedChildren(hasFailedChildren() || hadFailures);
    setHasSolvedChildren(hasSolvedChildren() || hadSolutions);

    bool allClosed = true;
    for (int i=getNumberOfChildren(); i--;) {
      if (static_cast<SpaceNode*>(getChild(i))->isOpen()) {
        allClosed = false;
        break;
      }
    }

    if (allClosed) {
      setHasOpenChildren(false);
      for (unsigned int i=0; i<getNumberOfChildren(); i++)
        setHasSolvedChildren(hasSolvedChildren() ||
          static_cast<SpaceNode*>(getChild(i))->hasSolvedChildren());
      SpaceNode* p = static_cast<SpaceNode*>(getParent());
      if (p != NULL) {
        delete copy;
        copy = NULL;
        p->closeChild(hasFailedChildren(), hasSolvedChildren());
      }
    } else {
      
      if (hadSolutions) {
        setHasSolvedChildren(true);
        SpaceNode* p = getParent();
        while (p != NULL && !p->hasSolvedChildren()) {
          p->setHasSolvedChildren(true);
          p = p->getParent();
        }
      }
      if (hadFailures) {
        SpaceNode* p = getParent();
        while (p != NULL && !p->hasFailedChildren()) {
          p->setHasFailedChildren(true);
          p = p->getParent();
        }        
      }
    }

  }

  SpaceNode::SpaceNode(Space* root)
  : workingSpace(root), ownBest(NULL), nstatus(0) {
    choice = NULL;
    if (root == NULL) {
      setStatus(FAILED);
      setHasSolvedChildren(false);
      setHasFailedChildren(true);
      setNumberOfChildren(0);
      copy = root;
      return;
    }
    if (!root->failed())
      copy = root->clone();
    else
      copy = root;
    setStatus(UNDETERMINED);
    setHasSolvedChildren(false);
    setHasFailedChildren(false);
  }

  SpaceNode::~SpaceNode(void) {
    delete choice;
    delete copy;
    delete workingSpace;
  }


  int
  SpaceNode::getNumberOfChildNodes(NodeAllocator& na,
                                   BestNode* curBest, Statistics& stats,
                                   int c_d, int a_d) {
    int kids = 0;
    if (isUndetermined()) {
      stats.undetermined--;
      acquireSpace(curBest, c_d, a_d);
      switch (workingSpace->status(stats)) {
      case SS_FAILED:
        {
          purge();
          kids = 0;
          setHasOpenChildren(false);
          setHasSolvedChildren(false);
          setHasFailedChildren(true);
          setStatus(FAILED);
          stats.failures++;
          // stats.newDepth(getDepth());
          SpaceNode* p = getParent();
          if (p != NULL)
            p->closeChild(true, false);
        }
        break;
      case SS_SOLVED:
        {
          // Deletes all pending branchers
          (void) workingSpace->choice();
          kids = 0;
          setStatus(SOLVED);
          setHasOpenChildren(false);
          setHasSolvedChildren(true);
          setHasFailedChildren(false);
          stats.solutions++;
          if (curBest != NULL) {
            curBest->s = this;
          }
          SpaceNode* p = getParent();
          if (p != NULL)
            p->closeChild(false, true);
        }
        break;
      case SS_BRANCH:
        {
          choice = workingSpace->choice();
          kids = choice->alternatives();
          setHasOpenChildren(true);
          if (dynamic_cast<const StopChoice*>(choice)) {
            setStatus(STOP);
          } else {
            setStatus(BRANCH);
            stats.choices++;
          }
          stats.undetermined += kids;
        }
        break;
      }
      static_cast<VisualNode*>(this)->changedStatus();
      setNumberOfChildren(kids);
      for (int i=kids; i--;) {
        setChild(i, new (na) VisualNode());
      }
    } else {
      kids = getNumberOfChildren();
    }
    return kids;
  }

  int
  SpaceNode::getNoOfOpenChildren(void) {
    if (!hasOpenChildren())
      return 0;
    int noOfOpenChildren = 0;
    for (int i=getNumberOfChildren(); i--;)
      noOfOpenChildren += (static_cast<SpaceNode*>(getChild(i))->isOpen());
    return noOfOpenChildren;
  }

}}

// STATISTICS: gist-any
