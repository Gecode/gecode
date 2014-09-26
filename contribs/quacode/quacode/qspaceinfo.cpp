/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2014
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <quacode/qspaceinfo.hh>
#include <vector>

using namespace std;

namespace Gecode {
  BoolVarValPrint QSpaceInfo::customBoolVVP = NULL;
  IntVarValPrint QSpaceInfo::customIntVVP = NULL;

  bool Strategy::strategyInit() {
    assert(bx == NULL);
    strategyTotalSize = 0;
    for (int i = domSize.size(); i--; )
      if (domSize[i] > 0) strategyTotalSize = 1 + domSize[i] * (1 + strategyTotalSize);
    bx = new (std::nothrow) Box[strategyTotalSize];
    cur = bx;
    curDepth = 0;
    lastEvent = Strategy::NONE;
    return (bx != NULL);
  }

  void Strategy::strategyReset() {
    if (bx == NULL) return;
    cur = bx;
    curDepth = 0;
    lastEvent = Strategy::NONE;
  }

  void Strategy::add(unsigned int id, unsigned int size) {
    curBranch.resize(static_cast<unsigned int>(curBranch.size())+1);
    domSize.resize(std::max(static_cast<unsigned int>(domSize.size()),id+1),0);
    idxInCurBranch.resize(std::max(static_cast<unsigned int>(idxInCurBranch.size()),id+1),-1);
    domSize[id] = size;
  }

  void Strategy::print(std::ostream& os, Box* p, unsigned int curRemainingStrategySize, int depth) const {
    int vId = p->var.id;
    // We get the right variable for this depth
    unsigned int nbAlt = p->var.nbAlt;
    p++;
    if (depth > 0) {
      for (int i=depth-1; i--; ) os << std::setfill(' ') << std::setw(4) << std::left << "|";
      os << std::setfill('-') << std::setw(4) << std::left << "|";
    }
    os << "v_" << vId << "(" << nbAlt << ")"<< std::endl;
    for (unsigned int i=nbAlt; i--; ) {
      unsigned int newRemainingSize = (curRemainingStrategySize - 1) / domSize[vId] - 1;

      // Print value
      for (int i=depth; i--; ) os << std::setfill(' ') << std::setw(4) << std::left << "|";
      if (p->val.inf == p->val.sup) os << p->val.inf << std::endl;
      else os << "[" << p->val.inf << "," << p->val.sup << "]" << std::endl;

      assert(p->val.inf <= p->val.sup);
      if (!p->val.leaf) print(os, p+1, newRemainingSize, depth+1);
      p += 1 + newRemainingSize;
    }
  }

  void Strategy::print(std::ostream& os) const {
    if (bx && (cur != bx)) print(os,bx,strategyTotalSize,0); os << std::endl;
  }

  bool StaticExpandStrategy::strategyInit() {
    if (Strategy::strategyInit()) {
      cur->var.id = -1;
      cur->var.nbAlt = 1;
      return true;
    }
    return false;
  }

  bool DynamicStrategy::strategyInit() {
    assert(bx == NULL);
    strategyTotalSize = 0;
    int k = domSize.size();
    for (int i = k; i--; )
      if (domSize[i] > 0) strategyTotalSize = 1 + domSize[i] * (1 + strategyTotalSize);
    k--;

    bxBlockSize = strategyTotalSize;
    while (bxBlockSize > sMaxBlockMemory) {
      assert(k > 0);// We can at least store first level of strategy
      k--;
      strategyTotalSize = 0;
      bxBlockSize = 2; // We save space for writing the flag to know that we have to allocate a new block plus the ptr to the new block
      for (int i = k; i--;  )
        if (domSize[i] > 0) {
          strategyTotalSize = 1 + domSize[i] * (1 + strategyTotalSize);
          bxBlockSize = 1 + domSize[i] * (1 + bxBlockSize);
        }
    }
    bx = new (std::nothrow) Box[bxBlockSize];
    cur = bx;
    return (bx != NULL);
  }

  bool DynamicExpandStrategy::strategyInit() {
    if (DynamicStrategy::strategyInit()) {
      cur->var.id = -1;
      cur->var.nbAlt = 1;
      return true;
    }
    return false;
  }

  void DynamicStrategy::strategyReset() {
    if (bx == NULL) return;
    int i = curDepth;
    while (i > 0) {
      if (!curBranch[i].blocks.empty()) {
        std::vector<Box*>::iterator it = curBranch[i].blocks.begin();
        std::vector<Box*>::iterator itEnd = curBranch[i].blocks.end();
        for ( ; it != itEnd; ++it) delete [] *it;
        curBranch[i].blocks.clear();
      }
      i--;
    }
    cur = bx;
    curDepth = 0;
    lastEvent = Strategy::NONE;
  }

  void DynamicStrategy::print(std::ostream& os, Box* p, unsigned int curRemainingBlockSize, int depth) const {
    if (p->var.needNewBlock) {
      p++;
      print(os,p->nextBlock.ptr,p->nextBlock.size,depth);
      return;
    }
    int vId = p->var.id;
    // We get the right variable for this depth
    unsigned int nbAlt = p->var.nbAlt;
    p++;
    if (depth > 0) {
      for (int i=depth-1; i--; ) os << std::setfill(' ') << std::setw(4) << std::left << "|";
      os << std::setfill('-') << std::setw(4) << std::left << "|";
    }
    os << "v_" << vId << "(" << nbAlt << ")"<< std::endl;
    for (unsigned int i=nbAlt; i--; ) {
      unsigned int newRemainingSize = (curRemainingBlockSize - 1) / domSize[vId] - 1;

      // Print value
      for (int i=depth; i--; ) os << std::setfill(' ') << std::setw(4) << std::left << "|";
      if (p->val.inf == p->val.sup) os << p->val.inf << std::endl;
      else os << "[" << p->val.inf << "," << p->val.sup << "]" << std::endl;

      assert(p->val.inf <= p->val.sup);
      if (!p->val.leaf) print(os, p+1, newRemainingSize, depth+1);
      p += 1 + newRemainingSize;
    }
  }

  void DynamicStrategy::print(std::ostream& os) const {
    if (bx && (cur != bx)) print(os,bx,bxBlockSize,0); os << std::endl;
  }

  QSpaceInfo::QSpaceSharedInfoO::QSpaceSharedInfoO(StrategyMethod sm)
    : s(NULL) {
    if (sm & StrategyMethodValues::BUILD) {
      if (sm & StrategyMethodValues::DYNAMIC) {
        if (sm & StrategyMethodValues::EXPAND) s = new DynamicExpandStrategy();
        else s = new DynamicStrategy();
      } else {
        if (sm & StrategyMethodValues::EXPAND) s = new StaticExpandStrategy();
        else s = new Strategy();
      }
    }
  }

  QSpaceInfo::QSpaceSharedInfoO::QSpaceSharedInfoO(const QSpaceSharedInfoO& qsi)
    : _linkIdVars(qsi._linkIdVars), v(qsi.v), s(NULL) {
      s = qsi.s->copy();
    }

  QSpaceInfo::QSpaceSharedInfoO::~QSpaceSharedInfoO(void) {
    if (s) delete s;
    s = NULL;
  }

  void
  QSpaceInfo::QSpaceSharedInfoO::add(const QSpaceInfo& qsi, const BrancherHandle& bh,
                                     TQuantifier _q,
                                     const BoolVarArgs& x) {
    assert(bh.id() == (v.size() + 1));
    unsigned int offset = 0;
    if (v.size() > 0) offset = v[v.size()-1].offset + v[v.size()-1].size;
    v.resize(bh.id());
    v[bh.id()-1] = QBI(_q,offset,x.size());
    for (int i=0; i<x.size(); i++) {
      if (qsi.curStrategyMethod & StrategyMethodValues::EXPAND)
        _linkIdVars.push_back(LkBinderVarObj(qsi._boolVars.size()+i,LkBinderVarObj::BOOL));
      if (s) s->add(offset+i,((_q==EXISTS)?1:x[i].size()));
    }
  }

  void
  QSpaceInfo::QSpaceSharedInfoO::add(const QSpaceInfo& qsi, const BrancherHandle& bh,
                                     TQuantifier _q,
                                     const IntVarArgs& x) {
    assert(bh.id() == (v.size() + 1));
    unsigned int offset = 0;
    if (v.size() > 0) offset = v[v.size()-1].offset + v[v.size()-1].size;
    v.resize(bh.id());
    v[bh.id()-1] = QBI(_q,offset,x.size());
    for (int i=0; i<x.size() ; i++) {
      if (qsi.curStrategyMethod & StrategyMethodValues::EXPAND)
        _linkIdVars.push_back(LkBinderVarObj(qsi._intVars.size()+i,LkBinderVarObj::INT));
      if (s) s->add(offset+i,((_q==EXISTS)?1:x[i].size()));
    }
  }

  StrategyMethod
  QSpaceInfo::QSpaceSharedInfoO::strategyInit(StrategyMethod sm) {
    if (sm & StrategyMethodValues::BUILD) {
      assert(s && !s->allocated());
      if (sm & StrategyMethodValues::DYNAMIC) {
        LABEL_PREFER_DYNAMIC:
        if (!s->strategyInit()) goto LABEL_NO_BUILD;
      } else {
        assert((dynamic_cast<Strategy*>(s) != NULL) || (dynamic_cast<StaticExpandStrategy*>(s) != NULL));
        if (!s->strategyInit()) {
          if (sm & StrategyMethodValues::FAILTHROUGH) {
            sm = sm | StrategyMethodValues::DYNAMIC;
            Strategy *_s = NULL;
            if (sm & StrategyMethodValues::EXPAND)
              _s = DynamicExpandStrategy::fromStaticStrategy(*s);
            else
              _s = DynamicStrategy::fromStaticStrategy(*s);
            delete s;
            s = _s;
            goto LABEL_PREFER_DYNAMIC;
          } else goto LABEL_NO_BUILD;
        }
      }
    } else {
      LABEL_NO_BUILD:
      delete s;
      s = NULL;
      sm = 0; // No build
    }
    return sm;
  }

  QSpaceInfo::QSpaceSharedInfo::QSpaceSharedInfo(void)
    : SharedHandle(NULL) {}

  QSpaceInfo::QSpaceSharedInfo::QSpaceSharedInfo(const QSpaceSharedInfo& bi)
    : SharedHandle(bi) {}

  void
  QSpaceInfo::QSpaceSharedInfo::init(StrategyMethod sm) {
    assert(object() == NULL);
    object(new QSpaceSharedInfoO(sm));
  }

  QSpaceInfo::QSpaceInfo(void)
    : bRecordStrategy(false),
      curStrategyMethod(StrategyMethodValues::BUILD
//                        | StrategyMethodValues::DYNAMIC
                        | StrategyMethodValues::FAILTHROUGH
//                        | StrategyMethodValues::EXPAND
                        )
      , nbWatchConstraint(0) {
      sharedInfo.init(curStrategyMethod);
    }

  QSpaceInfo::~QSpaceInfo(void) { }

  QSpaceInfo::QSpaceInfo(Space& home, bool share, QSpaceInfo& qs)
    : bRecordStrategy(qs.bRecordStrategy),
      curStrategyMethod(qs.curStrategyMethod),
      nbWatchConstraint(qs.nbWatchConstraint) {
      sharedInfo.update(home, share, qs.sharedInfo);
      if (qs.curStrategyMethod & StrategyMethodValues::EXPAND) {
        _boolVars.update(home, share, qs._boolVars);
        _intVars.update(home, share, qs._intVars);
      }
    }

  void
  QSpaceInfo::QSpaceSharedInfo::add(const QSpaceInfo& qsi, const BrancherHandle& bh,
                                    TQuantifier _q,
                                    const IntVarArgs& x) {
    static_cast<QSpaceSharedInfoO*>(object())->add(qsi,bh,_q,x);
  }

  void
  QSpaceInfo::QSpaceSharedInfo::add(const QSpaceInfo& qsi, const BrancherHandle& bh,
                                    TQuantifier _q,
                                    const BoolVarArgs& x) {
    static_cast<QSpaceSharedInfoO*>(object())->add(qsi,bh,_q,x);
  }

  void
  QSpaceInfo::updateQSpaceInfo(const BrancherHandle& bh,
                               TQuantifier _q,
                               const BoolVarArgs& x) {
    sharedInfo.add(*this,bh,_q,x);
    if (curStrategyMethod & StrategyMethodValues::EXPAND)
      _boolVars = BoolVarArray(dynamic_cast<Space&>(*this),_boolVars+x);
  }

  void
  QSpaceInfo::updateQSpaceInfo(const BrancherHandle& bh,
                               TQuantifier _q,
                               const IntVarArgs& x) {
    sharedInfo.add(*this,bh,_q,x);
    if (curStrategyMethod & StrategyMethodValues::EXPAND)
      _intVars = IntVarArray(dynamic_cast<Space&>(*this),_intVars+x);
  }

  template<class BranchType> std::vector<BrancherHandle>
  QSpaceInfo::branch(Home home, const BoolVar& x, BranchType vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp) {
    BoolVarArgs vaX;
    vaX << x;
    return this->branch<BranchType>(home,vaX,vars,vals,bf,vvp);
  }

  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const BoolVar& x, IntVarBranch vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp);
  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const BoolVar& x, TieBreak<IntVarBranch> vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp);

  template<class BranchType> std::vector<BrancherHandle>
  QSpaceInfo::branch(Home home, const IntVar& x, BranchType vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp) {
    IntVarArgs vaX;
    vaX << x;
    return this->branch<BranchType>(home,vaX,vars,vals,bf,vvp);
  }

  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const IntVar& x, IntVarBranch vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp);
  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const IntVar& x, TieBreak<IntVarBranch> vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp);

  template<class BranchType> std::vector<BrancherHandle>
  QSpaceInfo::branch(Home home, const BoolVarArgs& x, BranchType vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp) {
    std::vector<BrancherHandle> result;
    assert(x.size() > 0);
    if (home.failed()) return result;

    // Get the quantifier of the first variable of the quantifier
    TQuantifier curQ = EXISTS;
    if (unWatched(x[0])) curQ = FORALL;
    int i = 0;
    // We iterate over blocks of variables with the same quantifier
    while (i < x.size()) {
      BoolVarArgs UW_X;
      BoolVar* uwxi = NULL;
      TQuantifier qi = EXISTS;

      while (i < x.size())
      {
        uwxi = unWatched(x[i]);
        qi = (uwxi?FORALL:EXISTS);

        if (qi != curQ) break; // End of current block go branching
        if (qi == EXISTS)
          UW_X << x[i];
        else
          UW_X << *uwxi;

        i++;
      }

      // Add brancher for unwatched variables
      BrancherHandle bh;
      if (vvp) {
        customBoolVVP = vvp;
        bh = Gecode::branch(home,UW_X,vars,vals,bf,&tripleChoice);
      } else {
        bh = Gecode::branch(home,UW_X,vars,vals,bf,&doubleChoice);
      }

      // Update shared info
      updateQSpaceInfo(bh,curQ,UW_X);
      result.push_back(bh);
      // Update current quantifier
      curQ = qi;
    }

    return result;
  }

  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const BoolVarArgs& x, IntVarBranch vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp);
  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const BoolVarArgs& x, TieBreak<IntVarBranch> vars, IntValBranch vals, BoolBranchFilter bf, BoolVarValPrint vvp);

  template<class BranchType> std::vector<BrancherHandle>
  QSpaceInfo::branch(Home home, const IntVarArgs& x, BranchType vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp) {
    std::vector<BrancherHandle> result;
    assert(x.size() > 0);
    if (home.failed()) return result;

    // Get the quantifier of the first variable of the quantifier
    TQuantifier curQ = EXISTS;
    if (unWatched(x[0])) curQ = FORALL;
    int i = 0;
    // We iterate over blocks of variables with the same quantifier
    while (i < x.size()) {
      IntVarArgs UW_X;
      IntVar* uwxi = NULL;
      TQuantifier qi = EXISTS;

      while (i < x.size())
      {
        uwxi = unWatched(x[i]);
        qi = (uwxi?FORALL:EXISTS);

        if (qi != curQ) break; // End of current block go branching
        if (qi == EXISTS)
          UW_X << x[i];
        else
          UW_X << *uwxi;

        i++;
      }

      // Add brancher for unwatched variables
      BrancherHandle bh;
      if (vvp) {
        customIntVVP = vvp;
        bh = Gecode::branch(home,UW_X,vars,vals,bf,&tripleChoice);
      } else {
        bh = Gecode::branch(home,UW_X,vars,vals,bf,&doubleChoice);
      }

      // Update shared info
      updateQSpaceInfo(bh,curQ,UW_X);
      result.push_back(bh);
      // Update current quantifier
      curQ = qi;
    }

    return result;
  }

  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const IntVarArgs& x, IntVarBranch vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp);
  template QUACODE_EXPORT std::vector<BrancherHandle> QSpaceInfo::branch<>(Home home, const IntVarArgs& x, TieBreak<IntVarBranch> vars, IntValBranch vals, IntBranchFilter bf, IntVarValPrint vvp);

}
