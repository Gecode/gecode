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
#include <stack>

#ifdef GECODE_GIST_EXPERIMENTAL
#include "gecode/serialization.hh"
#ifdef GECODE_HAS_INT_VARS
#include "gecode/int.hh"
#endif
#ifdef GECODE_HAS_SET_VARS
#include "gecode/set.hh"
#endif
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
    static const int a_d = 100000;
    /// Distance for fixed recomputation
    static const int mrd = 4;
  };
  
  StepDesc::StepDesc(int steps) : noOfSteps(steps), debug(false) { }
  
  void
  StepDesc::toggleDebug(void) {
   debug = !debug;
  }
  
  SpecialDesc::SpecialDesc(std::string varName, int rel0, int v0)
  : vn(varName), v(v0), rel(rel0) { }

  BestNode::BestNode(SpaceNode* s0, Better* b0) : s(s0), b(b0) {}
  
  SpaceNode*
  SpaceNode::getParent() {
    return static_cast<SpaceNode*>(Node::getParent());
  }

  SpaceNode*
  SpaceNode::getChild(int i) {
    return static_cast<SpaceNode*>(Node::getChild(i));
  }

  int
  SpaceNode::recompute(void) {
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

        if(curNode->getStatus() == STEP) {
          Branch b(curNode->alternative, curNode->desc.step, BK_STEP);
          stck.push(b);
          if(lastFixpoint == NULL && parent->getStatus() == BRANCH) {
             lastFixpoint = parent;
          }
      } else if(curNode->getStatus() == SPECIAL) {
          Branch b(curNode->alternative, curNode->desc.special, BK_SPECIAL_IN);
          stck.push(b);
          if(lastFixpoint == NULL && parent->getStatus() == BRANCH) {
            lastFixpoint = parent;
          }
          specialNodeOnPath = true;
          // TODO nikopp: if the root node is failed, it neither has a copy nor a parent and we are not allowed to do the following
        } else if(parent->getStatus() == SPECIAL || parent->getStatus() == STEP) {
           Branch b(curNode->alternative, curNode->desc.special, BK_SPECIAL_OUT);
          stck.push(b);
        } else {
          Branch b(curNode->alternative, parent->desc.branch,
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

#ifdef GECODE_GIST_EXPERIMENTAL

      // TODO nikopp: as long as the SpecialDesc is evaluated in the switch below, we need this varMap here
      Reflection::VarMap vm;
      if(specialNodeOnPath) {
        curSpace->getVars(vm, false);
      }

#endif      

      while (!stck.empty()) {
        if (Config::a_d >= 0 &&
            curDist > Config::a_d &&
            middleNode->getStatus() != SPECIAL &&
            middleNode->getStatus() != STEP &&
            curDist == rdist / 2) {
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
#ifdef GECODE_GIST_EXPERIMENTAL
              if(b.desc.step->noOfSteps >0) {
                if(b.desc.step->debug) {
                  watch = !watch;
                }
                for (int i = 0; i < b.desc.step->noOfSteps; ++i) {
                  curSpace->step();
                }
              }
              else {
                if(middleNode->getStatus() == BRANCH) {
                  curSpace->commit(middleNode->desc.branch, b.alternative);
                }
                // make sure that the propagators enter the queue
                curSpace->stable();
              }
#endif
            }
            break;
        case BK_SPECIAL_IN:
            {
#ifdef GECODE_GIST_EXPERIMENTAL
              if(b.desc.special != NULL) {

                const Support::Symbol vName = b.desc.special->vn.c_str();

                if(false) {
#ifdef GECODE_HAS_INT_VARS
                } else if(vm.spec(vName).vti() == Int::IntVarImp::vti) {
                  IntVar iv = vm.var(vName);
                  rel(curSpace, iv, static_cast<IntRelType>(b.desc.special->rel), b.desc.special->v);
#endif
#ifdef GECODE_HAS_SET_VARS
                } else if(vm.spec(vName).vti() == Set::SetVarImp::vti) {
                  SetVar sv = vm.var(vName);
                  dom(curSpace, sv, static_cast<SetRelType>(b.desc.special->rel), b.desc.special->v);
#endif
                } else {
                  // TODO nikopp: implement other possibilities
                }
              }
#endif
            }
            break;
        case BK_SPECIAL_OUT:
         // really do nothing
          break;
        }

        if (b.ownBest != NULL && b.ownBest != lastBest) {
          b.ownBest->acquireSpace();
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
  
  Space*
  SpaceNode::donateSpace(int alt, SpaceNode* ownBest) {
    Space* ret = workingSpace;
    if (ret != NULL) {
      workingSpace = NULL;
      if (desc.branch != NULL)
        ret->commit(desc.branch, alt);
      
      if (ownBest != NULL) {
        ownBest->acquireSpace();
        curBest->b->constrain(ret, ownBest->workingSpace);
      }
    }
    return ret;
  }
  
  Space*
  SpaceNode::checkLAO(int alt, SpaceNode* ownBest) {
    Space* ret = NULL;
    if (copy != NULL && noOfOpenChildren == 1 && getParent() != NULL) {
      // last alternative optimization
      ret = copy;
      copy = NULL;
      
      if(desc.branch != NULL)
        ret->commit(desc.branch, alt);

      if (ownBest != NULL) {
        ownBest->acquireSpace();
        curBest->b->constrain(ret, ownBest->workingSpace);
      }
    }
    return ret;
  }
  
  void
  SpaceNode::acquireSpace(void) {
    SpaceNode* p = getParent();

    if (status == UNDETERMINED && curBest != NULL && ownBest == NULL &&
        p != NULL && curBest->s != p->ownBest) {
          ownBest = curBest->s;
    }

    if (status != SPECIAL && status != STEP) {
      if (workingSpace == NULL && p != NULL) {
        workingSpace = p->donateSpace(alternative, ownBest);
      }
    }

    if (workingSpace == NULL) {
      if (recompute() > Config::mrd && Config::mrd >= 0 &&
          status != SPECIAL && status != STEP &&
          workingSpace->status() == SS_BRANCH) {
            copy = workingSpace->clone();
      }
    }

    if (status != SPECIAL && status != STEP) {
      // always return a fixpoint
      workingSpace->status();
      if (copy == NULL && p != NULL && isOpen()) {
        copy = p->checkLAO(alternative, ownBest);
        if (copy != NULL && copy->status() == SS_FAILED) {
          delete copy;
          copy = NULL;
        }
      }
    }
  }
  
  Better*
  SpaceNode::getBetterWrapper(void) const {
    return curBest == NULL ? NULL : curBest->b;
  }
  
  void
  SpaceNode::solveUp(void) {
    SpaceNode* p = getParent();
    _hasSolvedChildren = true;
    if (p != NULL && !p->_hasSolvedChildren) {
        p->solveUp();
    }
  }
  
  void
  SpaceNode::closeChild(bool hadFailures, bool hadSolutions) {
    noOfOpenChildren--;
    _hasFailedChildren = _hasFailedChildren || hadFailures;
    _hasSolvedChildren = _hasSolvedChildren || hadSolutions;
    SpaceNode* p = getParent();
    if (noOfOpenChildren == 0) {
      // stats.close();
      if (p != NULL) {
        delete copy;
        copy = NULL;        
        p->closeChild(_hasFailedChildren, _hasSolvedChildren);
      }
    } else if (hadSolutions) {
        solveUp();
    }
  }

  SpaceNode::SpaceNode(int alt, BestNode* cb)
  : copy(NULL), workingSpace(NULL), curBest(cb), ownBest(NULL) {
    desc.branch = NULL;
    alternative = alt;
    status = UNDETERMINED;
#ifdef GECODE_GIST_EXPERIMENTAL
    metaStatus = UNDETERMINED;
    firstStep = false;
    lastStep = false;
#endif
    _hasSolvedChildren = false;
    _hasFailedChildren = false;
  }

  SpaceNode::SpaceNode(Space* root, Better* b)
  : workingSpace(root), curBest(NULL), ownBest(NULL) {
    desc.branch = NULL;
    if (root == NULL) {
      status = FAILED;
#ifdef GECODE_GIST_EXPERIMENTAL
      metaStatus = UNDETERMINED;
      firstStep = false;
      lastStep = false;
#endif
      _hasSolvedChildren = false;
      _hasFailedChildren = true;
      setNumberOfChildren(0);
      copy = root;
      return;
    }
    if (!root->failed())
      copy = root->clone();
    else
      copy = root;
    status = UNDETERMINED;
#ifdef GECODE_GIST_EXPERIMENTAL
    metaStatus = UNDETERMINED;
    firstStep = false;
    lastStep = false;
#endif
    _hasSolvedChildren = false;
    _hasFailedChildren = false;
    if (b != NULL)
      curBest = new BestNode(NULL, b);
  }

  SpaceNode::~SpaceNode(void) {
    if(status == SPECIAL)
      delete desc.special;
    else if (status == STEP)
      delete desc.step;
    else
      delete desc.branch;
    delete copy;
    delete workingSpace;
    if (getParent() == NULL)
      delete curBest;
  }
    
  Space*
  SpaceNode::getSpace(void) {
    acquireSpace();
    Space* ret = workingSpace;
    workingSpace = NULL;
    return ret;
  }

  bool
  SpaceNode::isCurrentBest(void) {
    return curBest != NULL && curBest->s == this;
  }
    
#ifdef GECODE_GIST_EXPERIMENTAL
  
  bool SpaceNode::watch = false;
  
  Space*
  SpaceNode::getInputSpace(void) {
    if(status == SPECIAL)
      return (getSpace());
    
    SpaceNode* p = getParent();
    
    Space* ret = p->getSpace();
    
    if(p->getStatus() != SPECIAL)
      ret->commit(p->desc.branch, alternative);
    
    return ret;
  }

  bool
  SpaceNode::isFirstStepNode(void) {
    if(isStepNode())
      return firstStep;
    return false;
  }
  
  void
  SpaceNode::setFirstStepNode(bool fs) {
    firstStep = fs;
  }

  void
  SpaceNode::setLastStepNode(bool ls) {
    lastStep = ls;
  }

  void
  SpaceNode::setHasFailedChildren(bool b) {
    _hasFailedChildren = b;
  }
  
  void
  SpaceNode::setHasSolvedChildren(bool b) {
    _hasSolvedChildren = b;
  }
  
  bool
  SpaceNode::isLastStepNode(void) {
    if(isStepNode())
      return lastStep;
    return false;
  }
  
  void
  SpaceNode::executeJavaScript(const std::string& model) {
    if(copy != NULL) {
      Gecode::fromJavaScript(copy, model);
      copy->status();
    }
  }
  
  void
  SpaceNode::replaceCopy(Space* s) {
    delete copy;
    copy = s;
  }
#endif

  int
  SpaceNode::getNumberOfChildNodes(Statistics& stats) {
    int kids = getNumberOfChildren();
    if (kids == -1) {
      stats.undetermined--;
      acquireSpace();
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
          _hasSolvedChildren = false;
          _hasFailedChildren = true;
          status = FAILED;
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
          status = SOLVED;
          // we may have got a copy from LAO
          if (getParent() != NULL) {
            delete copy;
            copy = NULL;
          }
          _hasSolvedChildren = true;
          _hasFailedChildren = false;
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
        status = BRANCH;
        stats.choices++;
        stats.undetermined += kids;
        // stats.newOpen();
        // stats.newDepth(getDepth() + 1);
        break;
      }
      changedStatus();
      setNumberOfChildren(kids);
      noOfOpenChildren = kids;
      for (int i=kids; i--;) {
        SpaceNode* child = createChild(i);
        setChild(i, child);
      }
    }
    return kids;
  }
  
  NodeStatus
  SpaceNode::getStatus(void) {
    return status;
  }
  
#ifdef GECODE_GIST_EXPERIMENTAL

  NodeStatus
  SpaceNode::getMetaStatus(void) {
    return metaStatus;
  }
  
  void
  SpaceNode::setMetaStatus(NodeStatus s) {
    metaStatus = s;
  }
  
#endif
  
  void
  SpaceNode::setStatus(NodeStatus s) {
    status = s;
  }
    
  void
  SpaceNode::setSpecialDesc(const SpecialDesc* d) {
    desc.special = d;
  }
    
  void
  SpaceNode::setStepDesc(StepDesc* d) {
    desc.step = d;
  }
    
  StepDesc*
  SpaceNode::getStepDesc(void) {
    StepDesc* ret = NULL;
    if(isStepNode()) {
      ret = desc.step;
    }
    return ret;
  }
    
  bool
  SpaceNode::isStepNode(void) {
    return getStatus() == STEP;
  }

 int
  SpaceNode::getAlternative(void) {
    return alternative;
  }
    
  bool
  SpaceNode::isOpen(void) {
    return status == UNDETERMINED || noOfOpenChildren > 0;
  }
  
  void
  SpaceNode::openUp(void) {
    if(!isOpen() && parent != NULL) {
      getParent()->openUp();
    }
    noOfOpenChildren++;
  }
  
  bool
  SpaceNode::hasFailedChildren(void) {
    return _hasFailedChildren;
  }
  
  bool
  SpaceNode::hasSolvedChildren(void) {
    return _hasSolvedChildren;
  }

  int
  SpaceNode::getNoOfOpenChildren(void) {
    return noOfOpenChildren;
  }
        
  void
  SpaceNode::setNoOfOpenChildren(int n) {
    noOfOpenChildren = n;
  }
        
  bool
  SpaceNode::hasCopy(void) {
    return copy != NULL;
  }

  bool
  SpaceNode::hasWorkingSpace(void) {
    return workingSpace != NULL;
  }

  SpaceNode*
  SpaceNode::createChild(int alternative) {
    return new SpaceNode(alternative, curBest);
  }
  
  void
  SpaceNode::changedStatus() {}
  
}}

// STATISTICS: gist-any
