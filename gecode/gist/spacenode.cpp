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

    if (copy == NULL) {
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
              copy = static_cast<Space*>(Support::mark(curSpace));
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
          Space* ownBestSpace =
            static_cast<Space*>(Support::funmark(b.ownBest->copy));
          if (ownBestSpace->status() != SS_SOLVED) {
            // in the presence of weakly monotonic propagators, we may have to
            // use search to find the solution here
            ownBestSpace = Gecode::dfs(ownBestSpace);
            if (Support::marked(b.ownBest->copy)) {
              delete static_cast<Space*>(Support::unmark(b.ownBest->copy));
              b.ownBest->copy = 
                static_cast<Space*>(Support::mark(ownBestSpace));
            } else {
              delete b.ownBest->copy;
              b.ownBest->copy = ownBestSpace;
            }
          }
          curSpace->constrain(*ownBestSpace);
          lastBest = b.ownBest;
        }
        curDist++;
        middleNode = middleNode->getChild(b.alternative);
        middleNode->setDistance(curDist);
      }
      copy = static_cast<Space*>(Support::mark(curSpace));

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

    if (copy == NULL && p != NULL && p->copy != NULL && 
        Support::marked(p->copy)) {
      // If parent has a working space, steal it
      copy = p->copy;
      p->copy = NULL;
      if (p->choice != NULL)
        static_cast<Space*>(Support::unmark(copy))->
          commit(*p->choice, getAlternative());

      if (ownBest != NULL) {
        ownBest->acquireSpace(curBest, c_d, a_d);
        Space* ownBestSpace = 
          static_cast<Space*>(Support::funmark(ownBest->copy));
        if (ownBestSpace->status() != SS_SOLVED) {
          // in the presence of weakly monotonic propagators, we may have to
          // use search to find the solution here

          ownBestSpace = Gecode::dfs(ownBestSpace);
          if (Support::marked(ownBest->copy)) {
            delete static_cast<Space*>(Support::unmark(ownBest->copy));
            ownBest->copy =
              static_cast<Space*>(Support::mark(ownBestSpace));
          } else {
            delete ownBest->copy;
            ownBest->copy = ownBestSpace;
          }
        }
        static_cast<Space*>(Support::unmark(copy))->constrain(*ownBestSpace);
      }
      int d = p->getDistance()+1;
      if (d > c_d && c_d >= 0 &&
          static_cast<Space*>(Support::unmark(copy))->status() == SS_BRANCH) {
        copy = static_cast<Space*>(Support::unmark(copy));
        d = 0;
      }
      setDistance(d);
    }

    if (copy == NULL) {
      if (recompute(curBest, c_d, a_d) > c_d && c_d >= 0 &&
          static_cast<Space*>(Support::unmark(copy))->status() == SS_BRANCH) {
        copy = static_cast<Space*>(Support::unmark(copy));
        setDistance(0);
      }
    }

    // always return a fixpoint
    static_cast<Space*>(Support::funmark(copy))->status();
    if (Support::marked(copy) && p != NULL && isOpen() &&
        p->copy != NULL && p->getNoOfOpenChildren() == 1 &&
        p->getParent() != NULL) {
      // last alternative optimization

      // If p->copy was a working space, we would have stolen it by now
      assert(!Support::marked(p->copy));

      copy = static_cast<Space*>(Support::unmark(copy));
      delete p->copy;
      p->copy = NULL;
      setDistance(0);
      p->setDistance(p->getParent()->getDistance()+1);
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
  : Node(NULL, root==NULL),
    copy(root), ownBest(NULL), nstatus(0), choice(NULL) {
    if (root == NULL) {
      setStatus(FAILED);
      setHasSolvedChildren(false);
      setHasFailedChildren(true);
    } else {
      setStatus(UNDETERMINED);
      setHasSolvedChildren(false);
      setHasFailedChildren(false);
    }
  }

  void
  SpaceNode::dispose(void) {
    delete choice;
    delete static_cast<Space*>(Support::funmark(copy));
  }


  int
  SpaceNode::getNumberOfChildNodes(NodeAllocator& na,
                                   BestNode* curBest, Statistics& stats,
                                   int c_d, int a_d) {
    int kids = 0;
    if (isUndetermined()) {
      stats.undetermined--;
      acquireSpace(curBest, c_d, a_d);
      switch (static_cast<Space*>(Support::funmark(copy))->status(stats)) {
      case SS_FAILED:
        {
          purge();
          kids = 0;
          setHasOpenChildren(false);
          setHasSolvedChildren(false);
          setHasFailedChildren(true);
          setStatus(FAILED);
          stats.failures++;
          SpaceNode* p = getParent();
          if (p != NULL)
            p->closeChild(true, false);
        }
        break;
      case SS_SOLVED:
        {
          // Deletes all pending branchers
          (void) static_cast<Space*>(Support::funmark(copy))->choice();
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
          choice = static_cast<Space*>(Support::funmark(copy))->choice();
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
      setNumberOfChildren(kids, na);
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
