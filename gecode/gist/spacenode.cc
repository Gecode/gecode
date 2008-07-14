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

#include "gecode/gist/spacenode.hh"
#include "gecode/gist/visualnode.hh"
#include "gecode/search.hh"
#include <stack>

#ifdef GECODE_HAS_DDS
#include "gecode/dds.hh"
#endif

namespace Gecode { namespace Gist {
  
  // TODO nikopp: doxygen comments
  enum BranchKind {
    BK_NORMAL, BK_SPECIAL_IN, BK_SPECIAL_OUT, BK_STEP
  };

  /// \brief Representation of a branch in the search tree
  class Branch {
  public:
    /// Alternative number
    int alternative;
    /// The best space known when the branch was created
    SpaceNode* ownBest;
    const BranchKind branchKind;
    union {
      /// Special branching description
      const SpecialDesc* special;
      /// Branching description
      const BranchingDesc* branch;
      /// Step description
      const StepDesc* step;
    } desc;
    
    /// Constructor
    Branch(int a, const BranchingDesc* d, SpaceNode* best = NULL, BranchKind bk = BK_NORMAL)
      : alternative(a), ownBest(best), branchKind(bk) {
        desc.branch = d;
      }
    Branch(int a, const SpecialDesc* d, BranchKind bk, SpaceNode* best = NULL)
      : alternative(a), ownBest(best), branchKind(bk) {
        desc.special = d;
      }
    Branch(int a, const StepDesc* d, BranchKind bk, SpaceNode* best = NULL)
      : alternative(a), ownBest(best), branchKind(bk) {
        desc.step = d;
      }
 };

 Statistics Statistics::dummy;

  /// \brief Initial configuration settings for search
  class Config {
  public:
    /// Distance for adaptive recomputation
    static const int a_d = 2;
    /// Distance for fixed recomputation
    static const int mrd = 8;
  };
  
  StepDesc::StepDesc(int steps) : noOfSteps(steps), debug(false) { }
  
  void
  StepDesc::toggleDebug(void) {
   debug = !debug;
  }
  
  SpecialDesc::SpecialDesc(std::string varName, int rel0, int v0)
  : vn(varName), v(v0), rel(rel0) { }

  BestNode::BestNode(SpaceNode* s0, Better* b0) : s(s0), b(b0) {}
  
  int
  SpaceNode::recompute(BestNode* curBest) {
    int rdist = 0;
    
    if (workingSpace == NULL) {
      SpaceNode* curNode = this;
      SpaceNode* lastFixpoint = NULL;

      if(curNode->getStatus() != SPECIAL && curNode->getStatus() != STEP) {
        lastFixpoint = curNode;
      }
      
      std::stack<Branch> stck;
      bool specialNodeOnPath = false;
      
      while (curNode->copy == NULL) {
        SpaceNode* parent = curNode->getParent();
        int alternative = curNode->getAlternative();

        if(curNode->getStatus() == STEP) {
          Branch b(alternative, curNode->desc.step, BK_STEP);
          stck.push(b);
          if(lastFixpoint == NULL && parent->getStatus() == BRANCH) {
             lastFixpoint = parent;
          }
      } else if(curNode->getStatus() == SPECIAL) {
          Branch b(alternative, curNode->desc.special, BK_SPECIAL_IN);
          stck.push(b);
          if(lastFixpoint == NULL && parent->getStatus() == BRANCH) {
            lastFixpoint = parent;
          }
          specialNodeOnPath = true;
          // TODO nikopp: if the root node is failed, it neither has a copy nor a parent and we are not allowed to do the following
        } else if(parent->getStatus() == SPECIAL || parent->getStatus() == STEP) {
           Branch b(alternative, curNode->desc.special, BK_SPECIAL_OUT);
          stck.push(b);
        } else {
          Branch b(alternative, parent->desc.branch,
                   curBest == NULL ? NULL : curNode->ownBest);
          stck.push(b);
        }
        
        curNode = parent;
        rdist++;
      }
      Space* curSpace = curNode->copy->clone();
      
      SpaceNode* lastBest = NULL;
      SpaceNode* middleNode = curNode;
      int curDist = 0;

      while (!stck.empty()) {
        if (Config::a_d >= 0 &&
            curDist > Config::a_d &&
            middleNode->getStatus() != SPECIAL &&
            middleNode->getStatus() != STEP &&
            curDist == rdist / 2) {
              curSpace->status();
              middleNode->copy = curSpace->clone();
        }
        Branch b = stck.top(); stck.pop();
        
        if(middleNode == lastFixpoint) {
          curSpace->status();
        }
        
        switch (b.branchKind) {
        case BK_NORMAL:
            {
              curSpace->commit(b.desc.branch, b.alternative);
            }
            break;
        case BK_STEP:
            {
            }
            break;
        case BK_SPECIAL_IN:
            {
            }
            break;
        case BK_SPECIAL_OUT:
         // really do nothing
          break;
        }

        if (b.ownBest != NULL && b.ownBest != lastBest) {
          b.ownBest->acquireSpace(curBest);
          curBest->b->constrain(curSpace, b.ownBest->workingSpace);
          lastBest = b.ownBest;
        }
        curDist++;
        middleNode = middleNode->getChild(b.alternative);
      }
      workingSpace = curSpace;
      
    }
    return rdist;
  }
  
  void
  SpaceNode::acquireSpace(BestNode* curBest) {
    SpaceNode* p = getParent();

    if (getStatus() == UNDETERMINED && curBest != NULL && ownBest == NULL &&
        p != NULL && curBest->s != p->ownBest) {
          ownBest = curBest->s;
    }

    if (getStatus() != SPECIAL && getStatus() != STEP) {
      if (workingSpace == NULL && p != NULL && p->workingSpace != NULL) {
        // If parent has a working space, steal it
        workingSpace = p->workingSpace;
        p->workingSpace = NULL;
        if (p->desc.branch != NULL)
          workingSpace->commit(p->desc.branch, getAlternative());
      
        if (ownBest != NULL) {
          ownBest->acquireSpace(curBest);
          curBest->b->constrain(workingSpace, ownBest->workingSpace);
        }
      }
    }

    if (workingSpace == NULL) {
      if (recompute(curBest) > Config::mrd && Config::mrd >= 0 &&
          getStatus() != SPECIAL && getStatus() != STEP &&
          workingSpace->status() == SS_BRANCH) {
            copy = workingSpace->clone();
      }
    }

    if (getStatus() != SPECIAL && getStatus() != STEP) {
      // always return a fixpoint
      workingSpace->status();
      if (copy == NULL && p != NULL && isOpen() &&
          p->copy != NULL && p->getNoOfOpenChildren() == 1 &&
          p->getParent() != NULL) {
        // last alternative optimization
        copy = p->copy;
        p->copy = NULL;
    
        if(p->desc.branch != NULL)
          copy->commit(p->desc.branch, getAlternative());

        if (ownBest != NULL) {
          ownBest->acquireSpace(curBest);
          curBest->b->constrain(copy, ownBest->workingSpace);
        }
        if (copy->status() == SS_FAILED) {
          delete copy;
          copy = NULL;
        }
      }
    }
  }
  
  void
  SpaceNode::closeChild(bool hadFailures, bool hadSolutions) {
    setHasFailedChildren(hasFailedChildren() || hadFailures);
    if (getStatus() == DECOMPOSE) {
      setHasSolvedChildren(hasSolvedChildren() && hadSolutions);
      if (!hasSolvedChildren()) {
        for (unsigned int i=0; i<getNumberOfChildren(); i++) {
          SpaceNode* c = static_cast<SpaceNode*>(getChild(i));
          if (c->isOpen()) {
            c->setStatus(COMPONENT_IGNORED);
            c->setNumberOfChildren(0);
          }
        }
      }
    }
    else
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
    } else if (hadSolutions && getStatus() != DECOMPOSE) {
      setHasSolvedChildren(true);
      SpaceNode* p = getParent();
      while (p != NULL && !p->hasSolvedChildren()) {
        p->setHasSolvedChildren(true);
        p = p->getParent();
      }
    }

  }

  SpaceNode::SpaceNode(Space* root)
  : workingSpace(root), ownBest(NULL) {
    desc.branch = NULL;
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
    if(getStatus() == SPECIAL)
      delete desc.special;
    else if (getStatus() == STEP)
      delete desc.step;
    else
      delete desc.branch;
    delete copy;
    delete workingSpace;
  }


  int
  SpaceNode::getNumberOfChildNodes(NodeAllocator& na,
                                   BestNode* curBest, Statistics& stats) {
    int kids;
    if (isUndetermined()) {
      stats.undetermined--;
      acquireSpace(curBest);
      switch (workingSpace->status()) {
      case SS_FAILED:
        {
          delete workingSpace;
          workingSpace = NULL;
          // we may have got a copy from LAO!
          if (getParent() != NULL) {
            delete copy;
            copy = NULL;
          }
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
          kids = 0;
          setStatus(SOLVED);
          // we may have got a copy from LAO
          if (getParent() != NULL) {
            delete copy;
            copy = NULL;
          }
          if (ownBest == NULL) {
            // do not store working spaces in solutions, except if we do BAB
            delete workingSpace;
            workingSpace = NULL;
          }
          setHasOpenChildren(false);
          setHasSolvedChildren(true);
          setHasFailedChildren(false);
          stats.solutions++;
          // stats.newDepth(getDepth());
          if (curBest != NULL) {
            curBest->s = this;
          }
          SpaceNode* p = getParent();
          if (p != NULL)
            p->closeChild(false, true);
        }
        break;
      case SS_BRANCH:
        desc.branch = workingSpace->description();
        kids = desc.branch->alternatives();
        setHasOpenChildren(true);
#ifdef GECODE_HAS_DDS
        if (dynamic_cast<const Decomposition::SingletonDescBase*>(
              desc.branch)) {
          setStatus(SINGLETON);
          setHasSolvedChildren(true);
          setHasFailedChildren(false);
          kids = 0;
          SpaceNode* p = static_cast<SpaceNode*>(getParent());
          if (p != NULL)
            p->closeChild(false, true);
        } else if (dynamic_cast<const Decomposition::DecompDesc*>(
            desc.branch)) {
          setStatus(DECOMPOSE);
          setHasSolvedChildren(true);
        } else {
#endif
          setStatus(BRANCH);
#ifdef GECODE_HAS_DDS
        }
#endif
        stats.choices++;
        stats.undetermined += kids;
        // stats.newOpen();
        // stats.newDepth(getDepth() + 1);
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
