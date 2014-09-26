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

namespace Gecode {

  forceinline DynamicStrategy*
  DynamicStrategy::fromStaticStrategy(const Strategy& ds) {
    assert(ds.bx == NULL);
    return new DynamicStrategy(ds);
  }

  forceinline DynamicExpandStrategy*
  DynamicExpandStrategy::fromStaticStrategy(const Strategy& ds) {
    assert(ds.bx == NULL);
    return new DynamicExpandStrategy(ds);
  }

  forceinline
  Strategy::Strategy()
    : bx(NULL), strategyTotalSize(0), cur(NULL), curDepth(0), lastEvent(NONE)
  {}

  forceinline
  Strategy::Strategy(const Strategy& s)
    : bx(NULL), domSize(s.domSize), idxInCurBranch(s.idxInCurBranch), strategyTotalSize(s.strategyTotalSize), cur(NULL), curBranch(s.curBranch), curDepth(s.curDepth), lastEvent(s.lastEvent) {
      if (s.bx) {
        bx = new (std::nothrow) Box[strategyTotalSize];
        Box* p = bx;
        const Box* q = s.bx;
        for (unsigned int i=s.strategyTotalSize ; i--; ) *p++ = *q++;
        assert(s.cur);
        cur = bx + (s.cur - s.bx);
      }
    }

  forceinline
  Strategy::~Strategy() {
    if (bx) delete [] bx;
    bx = NULL;
    cur = NULL;
  }

  forceinline void
  Strategy::makeLeaf(void) {
    if (curDepth) (cur - 1)->val.leaf = true;
  }

  forceinline void
  Strategy::backtrackFromFailure(int vId) {
    assert(curDepth > 0);
    curDepth = idxInCurBranch[vId];
    cur = curBranch[curDepth].ptrCur;
  }

  forceinline void
  Strategy::backtrackFromSuccess(int vId) {
    assert(curDepth > 0);
    curDepth = idxInCurBranch[vId];
    // We increase the number of the current alternative
    curBranch[curDepth].ptrId->var.nbAlt++;
    cur = curBranch[curDepth].ptrCur + 1 + curBranch[curDepth].curRemainingStrategySize;
  }

  forceinline void
  Strategy::addVariable(int vId) {
    if ((curDepth > 0) && (curBranch[curDepth-1].id == vId)) {
      // We add again on the same variable so we overwrite the last box
      curDepth--;
      cur = curBranch[curDepth].ptrCur;
    } else {
      // We put the id of the variable only the first time we branch on it
      unsigned int sizeBelow = curDepth?curBranch[curDepth-1].curRemainingStrategySize:strategyTotalSize;
      sizeBelow = (sizeBelow - 1) / domSize[vId] - 1;
      idxInCurBranch[vId] = curDepth;
      (*cur).var.id = vId;
      (*cur).var.needNewBlock = false;
      (*cur++).var.nbAlt = 1;
       curBranch[curDepth] = BPtr(vId,cur-1,sizeBelow);
    }
  }

  forceinline void
  Strategy::addValue(int, int vInf, int vSup) {
    BPtr& bPtr = curBranch[curDepth++];
    bPtr.vInf = vInf;
    bPtr.vSup = vSup;
    bPtr.ptrCur = cur;
    (*cur).val.leaf = false;
    (*cur).val.inf = vInf;
    (*cur++).val.sup = vSup;
  }

  forceinline void
  Strategy::scenarioFailed(void) {
    lastEvent = Strategy::FAILURE;
    makeLeaf();
  }

  forceinline void
  Strategy::scenarioSuccess(const QSpaceInfo&) {
    lastEvent = Strategy::SUCCESS;
    makeLeaf();
  }

  forceinline void
  Strategy::scenarioChoice(int vId, int vInf, int vSup) {
    switch (lastEvent) {
      case Strategy::FAILURE:
        backtrackFromFailure(vId);
        goto branching;
      case Strategy::SUCCESS:
        backtrackFromSuccess(vId);
        goto branching;
      case Strategy::NONE:
      case Strategy::CHOICE:
          addVariable(vId);
        branching:
          lastEvent = Strategy::CHOICE;
          addValue(vId,vInf,vSup);
    }
  }

  forceinline
  StaticExpandStrategy::StaticExpandStrategy()
    : Strategy() {}

  forceinline
  StaticExpandStrategy::StaticExpandStrategy(const StaticExpandStrategy& s)
    : Strategy(s) { }

  forceinline
  StaticExpandStrategy::~StaticExpandStrategy() { }

  forceinline void
  StaticExpandStrategy::addValue(int vId, int vInf, int vSup, unsigned int sizeBelow, bool& bForce) {
    // Compute address of value of variable depending on the number of alternative
    if (bForce || ((*cur).var.id != vId)) {
      (*cur).var.id = vId;
      (*cur).var.needNewBlock = false;
      (*cur++).var.nbAlt = 1;
      (*cur).val.leaf = false;
      (*cur).val.inf = vInf;
      (*cur++).val.sup = vSup;
      bForce = true;
    } else {
      Box* ptrVal = cur + 1 + ((*cur).var.nbAlt - 1) * (1 + sizeBelow);
      if (((*ptrVal).val.inf != vInf) || ((*ptrVal).val.sup != vSup)) {
        if ((*cur).var.nbAlt < domSize[vId]) {
          ptrVal +=  1 + sizeBelow;
          (*cur).var.nbAlt++;
        } else {
          // If we reach the last alternative we rewrite all
          (*cur).var.nbAlt = 1;
        }
        cur = ptrVal;
        (*cur).val.leaf = false;
        (*cur).val.inf = vInf;
        (*cur++).val.sup = vSup;
        bForce = true;
      } else {
        // Its the same variable and value so we go next one
        cur = ptrVal + 1;
      }
    }
  }

  forceinline void
  StaticExpandStrategy::scenarioFailed(void) { }

  forceinline void
  StaticExpandStrategy::scenarioSuccess(const QSpaceInfo& qsi) {
    bool bForce = false;
    unsigned int sizeBelow = strategyTotalSize;
    cur = bx;
    unsigned int i = 0;
    const std::vector<QSpaceInfo::LkBinderVarObj>& _linkIdVars = qsi.sharedInfo.linkIdVars();
    for ( ; i<_linkIdVars.size(); i++) {
      sizeBelow = (sizeBelow - 1) / domSize[i] - 1;
      if (_linkIdVars[i].type == QSpaceInfo::LkBinderVarObj::BOOL) {
        const BoolVar& x = qsi._boolVars[_linkIdVars[i].id];
        addValue(i,x.min(),x.max(),sizeBelow,bForce);
      } else {
        const IntVar& x = qsi._intVars[_linkIdVars[i].id];
        addValue(i,x.min(),x.max(),sizeBelow,bForce);
      }
    }
    (cur - 1)->val.leaf = true; // Make last print to be a leaf
    lastEvent = Strategy::SUCCESS;
  }

  forceinline void
  StaticExpandStrategy::scenarioChoice(int, int, int) { }

  forceinline
  DynamicStrategy::DynamicStrategy()
    : Strategy(), bxBlockSize(0) {}

  forceinline
  DynamicStrategy::DynamicStrategy(const DynamicStrategy& s)
    : Strategy(s), bxBlockSize(s.bxBlockSize) {
    }

  forceinline
  DynamicStrategy::DynamicStrategy(const Strategy& s)
    : Strategy(s), bxBlockSize(0) {
      assert(bx == NULL);
    }

  forceinline
  DynamicStrategy::~DynamicStrategy() {
    int i = curDepth;
    while (i >= 0) {
      if (!curBranch[i].blocks.empty()) {
        std::vector<Box*>::iterator it = curBranch[i].blocks.begin();
        std::vector<Box*>::iterator itEnd = curBranch[i].blocks.end();
        for ( ; it != itEnd; ++it) delete [] *it;
        curBranch[i].blocks.clear();
      }
      i--;
    }
  }

  forceinline void
  DynamicStrategy::backtrackFromSuccess(int vId) {
    assert(curDepth > 0);
    // We build the list of blocks to delete if node will be deleted
    std::vector<Box*> blocks;
    while (curDepth > idxInCurBranch[vId]) {
      if (!curBranch[curDepth].blocks.empty()) {
        blocks.insert(blocks.end(), curBranch[curDepth].blocks.begin(), curBranch[curDepth].blocks.end());
        curBranch[curDepth].blocks.clear();
      }
      curDepth--;
    }
    // We increase the number of the current alternative
    curBranch[curDepth].ptrId->var.nbAlt++;
    curBranch[curDepth].blocks.insert(curBranch[curDepth].blocks.end(),blocks.begin(),blocks.end());
    cur = curBranch[curDepth].ptrCur + 1 + curBranch[curDepth].curRemainingBlockSize;
  }

  forceinline void
  DynamicStrategy::backtrackFromFailure(int vId) {
    assert(curDepth > 0);
    // We delete all blocks on the path to the variable to backtrack
    while (curDepth > idxInCurBranch[vId]) {
      if (!curBranch[curDepth].blocks.empty()) {
        std::vector<Box*>::iterator it = curBranch[curDepth].blocks.begin();
        std::vector<Box*>::iterator itEnd = curBranch[curDepth].blocks.end();
        for ( ; it != itEnd; ++it) delete [] *it;
        curBranch[curDepth].blocks.clear();
      }
      curDepth--;
    }
    cur = curBranch[curDepth].ptrCur;
  }

  forceinline void
  DynamicStrategy::addVariable(int vId) {
    if ((curDepth > 0) && (curBranch[curDepth-1].id == vId)) {
      // We add again on the same variable so we overwrite the last box
      curDepth--;
      cur = curBranch[curDepth].ptrCur;
    } else {
      // We put the id of the variable only the first time we branch on it
      Box* newAllocatedBlock = NULL;
      unsigned int sizeBelow = curDepth?curBranch[curDepth-1].curRemainingStrategySize:strategyTotalSize;
      unsigned int allocatedBlockSize = curDepth?curBranch[curDepth-1].curRemainingBlockSize:bxBlockSize;

      if (sizeBelow == 0) {
        // We need to allocate a new block
        sizeBelow = 0;
        int k = domSize.size()-1;
        for (int i = k; i >= vId; i-- ) {
          if (domSize[i] > 0) sizeBelow = 1 + domSize[i] * (1 + sizeBelow);
        }
        k--;

        allocatedBlockSize = sizeBelow;
        while (allocatedBlockSize > sMaxBlockMemory) {
          if (k <= vId) {
            //We can at least store first level of strategy
            std::cerr << "Can't allocate size for one level of strategy, try to disable it." << std::endl;
            GECODE_NEVER;
          }
          k--;
          sizeBelow = 0;
          allocatedBlockSize = 2; // We save space for writing the flag to know that we have to allocate a new block plus the ptr to the new block
          for (int i = k; i >= vId; i-- )
            if (domSize[i] > 0) {
              sizeBelow = 1 + domSize[i] * (1 + sizeBelow);
              allocatedBlockSize = 1 + domSize[i] * (1 + allocatedBlockSize);
            }
        }
        newAllocatedBlock = new Box[allocatedBlockSize];
        (*cur).var.id = -1;
        (*cur++).var.needNewBlock = true;
        (*cur).nextBlock.ptr = newAllocatedBlock;
        (*cur).nextBlock.size = allocatedBlockSize;
        cur = newAllocatedBlock;
      }

      sizeBelow = (sizeBelow - 1) / domSize[vId] - 1;
      allocatedBlockSize = (allocatedBlockSize - 1) / domSize[vId] - 1;
      idxInCurBranch[vId] = curDepth;
      (*cur).var.id = vId;
      (*cur).var.needNewBlock = false;
      (*cur++).var.nbAlt = 1;
       curBranch[curDepth] = BPtr(vId,cur-1,sizeBelow,newAllocatedBlock,allocatedBlockSize);
    }
  }

  forceinline void
  DynamicStrategy::scenarioFailed(void) {
    lastEvent = Strategy::FAILURE;
    makeLeaf();
  }

  forceinline void
  DynamicStrategy::scenarioSuccess(const QSpaceInfo&) {
    lastEvent = Strategy::SUCCESS;
    makeLeaf();
  }

  forceinline void
  DynamicStrategy::scenarioChoice(int vId, int vInf, int vSup) {
    switch (lastEvent) {
      case Strategy::FAILURE:
        backtrackFromFailure(vId);
        goto branching;
      case Strategy::SUCCESS:
        backtrackFromSuccess(vId);
        goto branching;
      case Strategy::NONE:
      case Strategy::CHOICE:
          addVariable(vId);
        branching:
          lastEvent = Strategy::CHOICE;
          addValue(vId,vInf,vSup);
    }
  }

  forceinline
  DynamicExpandStrategy::DynamicExpandStrategy()
    : DynamicStrategy() {}

  forceinline
  DynamicExpandStrategy::DynamicExpandStrategy(const DynamicExpandStrategy& s)
    : DynamicStrategy(s) { }

  forceinline
  DynamicExpandStrategy::DynamicExpandStrategy(const Strategy& s)
    : DynamicStrategy(s) { }

  forceinline
  DynamicExpandStrategy::~DynamicExpandStrategy() {
    // To force destructor of Dynamicstrategy to free all allocated blocks
    curDepth = curBranch.size() - 1;
  }

  forceinline void
  DynamicExpandStrategy::addValue(int vId, int vInf, int vSup, unsigned int& sizeBelow, unsigned int& allocatedBlockSizeBelow, bool& bForce) {
    if (sizeBelow == 0) {
      // We need to allocate a new block
      sizeBelow = 0;
      int k = domSize.size()-1;
      for (int i = k; i >= vId; i-- ) {
        if (domSize[i] > 0) sizeBelow = 1 + domSize[i] * (1 + sizeBelow);
      }
      k--;

      allocatedBlockSizeBelow = sizeBelow;
      while (allocatedBlockSizeBelow > sMaxBlockMemory) {
        if (k <= vId) {
          //We can at least store first level of strategy
          std::cerr << "Can't allocate size for one level of strategy, try to disable it." << std::endl;
          GECODE_NEVER;
        }
        k--;
        sizeBelow = 0;
        allocatedBlockSizeBelow = 2; // We save space for writing the flag to know that we have to allocate a new block plus the ptr to the new block
        for (int i = k; i >= vId; i-- )
          if (domSize[i] > 0) {
            sizeBelow = 1 + domSize[i] * (1 + sizeBelow);
            allocatedBlockSizeBelow = 1 + domSize[i] * (1 + allocatedBlockSizeBelow);
          }
      }
      Box* newAllocatedBlock = new Box[allocatedBlockSizeBelow];
      // We remember the new allocated block in the curBranch data structure to free it when
      // backtrack
      curBranch[vId].blocks.push_back(newAllocatedBlock);
      // Link new block to previous one
      (*cur).var.id = -1;
      (*cur).var.nbAlt = sizeBelow; // This block doesn't need the nbAlt field, so we use it to store sizeBelow
      (*cur++).var.needNewBlock = true;
      (*cur).nextBlock.ptr = newAllocatedBlock;
      (*cur).nextBlock.size = allocatedBlockSizeBelow;
      cur = newAllocatedBlock;
      bForce = true;
    }
    sizeBelow = (sizeBelow - 1) / domSize[vId] - 1;
    allocatedBlockSizeBelow = (allocatedBlockSizeBelow - 1) / domSize[vId] - 1;

    // Compute address of value of variable depending on the number of alternative
    if (bForce || ((*cur).var.id != vId)) {
      // Add the current value and variable
      (*cur).var.id = vId;
      (*cur).var.needNewBlock = false;
      (*cur++).var.nbAlt = 1;
      (*cur).val.leaf = false;
      (*cur).val.inf = vInf;
      (*cur++).val.sup = vSup;
      bForce = true;
    } else {
      Box* ptrVal = cur + 1 + ((*cur).var.nbAlt - 1) * (1 + allocatedBlockSizeBelow);
      if (((*ptrVal).val.inf != vInf) || ((*ptrVal).val.sup != vSup)) {
        if ((*cur).var.nbAlt < domSize[vId]) {
          ptrVal +=  1 + allocatedBlockSizeBelow;
          (*cur).var.nbAlt++;
          // We build the list of blocks to delete if node will be deleted
          int curDepth = curBranch.size()-1;
          std::vector<Box*> blocks;
          while (curDepth > vId) {
            if (!curBranch[curDepth].blocks.empty()) {
              blocks.insert(blocks.end(), curBranch[curDepth].blocks.begin(), curBranch[curDepth].blocks.end());
              curBranch[curDepth].blocks.clear();
            }
            curDepth--;
          }
          curBranch[curDepth].blocks.insert(curBranch[curDepth].blocks.end(),blocks.begin(),blocks.end());
        } else {
          // If we reach the last alternative we rewrite all
          (*cur).var.nbAlt = 1;
          // We free all blocks allowed behond that point as we rewrite all
          int curDepth = curBranch.size()-1;
          while (curDepth > vId) {
            if (!curBranch[curDepth].blocks.empty()) {
              std::vector<Box*>::iterator it = curBranch[curDepth].blocks.begin();
              std::vector<Box*>::iterator itEnd = curBranch[curDepth].blocks.end();
              for ( ; it != itEnd; ++it) delete [] *it;
              curBranch[curDepth].blocks.clear();
            }
            curDepth--;
          }
        }
        cur = ptrVal;
        (*cur).val.leaf = false;
        (*cur).val.inf = vInf;
        (*cur++).val.sup = vSup;
        bForce = true;
      } else {
        // Its the same variable and value so we go next one
        cur = ptrVal + 1;
        if (cur->var.needNewBlock) {
          sizeBelow = cur->var.nbAlt; // This block doesn't need the nbAlt field, so we use it to store sizeBelow
          cur++;
          allocatedBlockSizeBelow = cur->nextBlock.size;
          cur = cur->nextBlock.ptr;
        }
      }
    }
  }

  forceinline void
  DynamicExpandStrategy::scenarioFailed(void) { }

  forceinline void
  DynamicExpandStrategy::scenarioSuccess(const QSpaceInfo& qsi) {
    bool bForce = false;
    unsigned int sizeBelow = strategyTotalSize;
    unsigned int allocatedBlockSizeBelow = bxBlockSize;
    cur = bx;
    unsigned int i = 0;
    const std::vector<QSpaceInfo::LkBinderVarObj>& _linkIdVars = qsi.sharedInfo.linkIdVars();
    for ( ; i<_linkIdVars.size(); i++) {
      if (_linkIdVars[i].type == QSpaceInfo::LkBinderVarObj::BOOL) {
        const BoolVar& x = qsi._boolVars[_linkIdVars[i].id];
        addValue(i,x.min(),x.max(),sizeBelow,allocatedBlockSizeBelow,bForce);
      } else {
        const IntVar& x = qsi._intVars[_linkIdVars[i].id];
        addValue(i,x.min(),x.max(),sizeBelow,allocatedBlockSizeBelow,bForce);
      }
    }
    (cur - 1)->val.leaf = true; // Make last print to be a leaf
    lastEvent = Strategy::SUCCESS;
  }

  forceinline void
  DynamicExpandStrategy::scenarioChoice(int, int, int) { }

  forceinline void
  QSpaceInfo::QSpaceSharedInfoO::strategyReset(void) {
    if (s) s->strategyReset();
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfoO::strategyPrint(std::ostream& os) const {
    if (s) s->print(os);
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfoO::scenarioSuccess(const QSpaceInfo& qsi) {
    s->scenarioSuccess(qsi);
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfoO::scenarioFailed(void) {
    s->scenarioFailed();
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfoO::scenarioChoice(unsigned int id, int pos, int vInf, int vSup) {
    int vId = v[id-1].offset + pos;
    s->scenarioChoice(vId,vInf,vSup);
  }

  forceinline const std::vector<QSpaceInfo::LkBinderVarObj>&
  QSpaceInfo::QSpaceSharedInfoO::linkIdVars(void) const {
    return _linkIdVars;
  }

  forceinline StrategyMethod
  QSpaceInfo::QSpaceSharedInfo::strategyInit(StrategyMethod sm) {
    return static_cast<QSpaceSharedInfoO*>(object())->strategyInit(sm);
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfo::strategyReset(void) {
    return static_cast<QSpaceSharedInfoO*>(object())->strategyReset();
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfo::strategyPrint(std::ostream& os) const {
    return static_cast<QSpaceSharedInfoO*>(object())->strategyPrint(os);
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfo::scenarioSuccess(const QSpaceInfo& qsi) {
    return static_cast<QSpaceSharedInfoO*>(object())->scenarioSuccess(qsi);
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfo::scenarioFailed(void) {
    return static_cast<QSpaceSharedInfoO*>(object())->scenarioFailed();
  }

  forceinline void
  QSpaceInfo::QSpaceSharedInfo::scenarioChoice(unsigned int id, int pos, int vInf, int vSup) {
    return static_cast<QSpaceSharedInfoO*>(object())->scenarioChoice(id,pos,vInf,vSup);
  }

  forceinline TQuantifier
  QSpaceInfo::QSpaceSharedInfo::brancherQuantifier(unsigned int id) const {
    return static_cast<QSpaceSharedInfoO*>(object())->brancherQuantifier(id);
  }

  forceinline unsigned int
  QSpaceInfo::QSpaceSharedInfo::brancherOffset(unsigned int id) const {
    return static_cast<QSpaceSharedInfoO*>(object())->brancherOffset(id);
  }

  forceinline int
  QSpaceInfo::QSpaceSharedInfo::getLastBrancherId(void) const {
    return static_cast<QSpaceSharedInfoO*>(object())->getLastBrancherId();
  }

  forceinline const std::vector<QSpaceInfo::LkBinderVarObj>&
  QSpaceInfo::QSpaceSharedInfo::linkIdVars(void) const {
    return static_cast<QSpaceSharedInfoO*>(object())->linkIdVars();
  }

  forceinline void
  QSpaceInfo::eventNewInstance(void) const { }

  forceinline TQuantifier
  QSpaceInfo::brancherQuantifier(unsigned int id) const {
    assert(id > 0);
    return sharedInfo.brancherQuantifier(id);
  }

  forceinline unsigned int
  QSpaceInfo::brancherOffset(unsigned int id) const {
    assert(id > 0);
    return sharedInfo.brancherOffset(id);
  }

  forceinline StrategyMethod
  QSpaceInfo::strategyMethod(void) const {
    return curStrategyMethod;
  }

  forceinline void
  QSpaceInfo::strategyMethod(StrategyMethod sm) {
    curStrategyMethod = sm;
  }

  forceinline StrategyMethod
  QSpaceInfo::strategyInit(void) {
    curStrategyMethod = sharedInfo.strategyInit(curStrategyMethod);
    bRecordStrategy = (curStrategyMethod & StrategyMethodValues::BUILD);
    return curStrategyMethod;
  }

  forceinline void
  QSpaceInfo::strategyReset(void) {
    return sharedInfo.strategyReset();
  }

  forceinline void
  QSpaceInfo::strategyPrint(std::ostream& os) const {
    return sharedInfo.strategyPrint(os);
  }

  forceinline void
  QSpaceInfo::strategySuccess(void) {
  }

  forceinline void
  QSpaceInfo::strategyFailed(void) {
  }

  forceinline void
  QSpaceInfo::scenarioSuccess(void) {
    if (bRecordStrategy) sharedInfo.scenarioSuccess(*this);
  }

  forceinline void
  QSpaceInfo::scenarioFailed(void) {
    if (bRecordStrategy) sharedInfo.scenarioFailed();
  }

  template<class VarType> forceinline
  void QSpaceInfo::doubleChoice (const Space &home, const BrancherHandle& bh,
                                 unsigned int,
                                 VarType x, int pos, const int&,
                                 std::ostream& ) {
    const QSpaceInfo& qsi = dynamic_cast<const QSpaceInfo&>(home);
    if (qsi.bRecordStrategy) const_cast<QSpaceInfo&>(qsi).sharedInfo.scenarioChoice(bh.id(),pos,x.min(),x.max());
  }

  template<> forceinline
  void QSpaceInfo::runCustomChoice (const Space &home, const BrancherHandle& bh,
                                    unsigned int alt,
                                    BoolVar x, int pos, const int& val,
                                    std::ostream& os) {
    assert(QSpaceInfo::customBoolVVP);
    (*QSpaceInfo::customBoolVVP)(home,bh,alt,x,pos,val,os);
  }

  template<> forceinline
  void QSpaceInfo::runCustomChoice (const Space &home, const BrancherHandle& bh,
                                    unsigned int alt,
                                    IntVar x, int pos, const int& val,
                                    std::ostream& os) {
    assert(QSpaceInfo::customIntVVP);
    (*QSpaceInfo::customIntVVP)(home,bh,alt,x,pos,val,os);
  }

  template<class VarType> forceinline
  void QSpaceInfo::tripleChoice (const Space &home, const BrancherHandle& bh,
                                 unsigned int alt,
                                 VarType x, int pos, const int& val,
                                 std::ostream& os) {
    const QSpaceInfo& qsi = dynamic_cast<const QSpaceInfo&>(home);
    if (qsi.bRecordStrategy) const_cast<QSpaceInfo&>(qsi).sharedInfo.scenarioChoice(bh.id(),pos,x.min(),x.max());
    runCustomChoice(home,bh,alt,x,pos,val,os);
  }

  forceinline unsigned int
  QSpaceInfo::watchConstraints(void) const {
    return nbWatchConstraint;
  }

  forceinline void
  QSpaceInfo::addWatchConstraint(void) {
    ++nbWatchConstraint;
  }

  forceinline void
  QSpaceInfo::delWatchConstraint(void) {
    --nbWatchConstraint;
  }

  forceinline void
  QSpaceInfo::setForAll(Home home, BoolVar x) {
    // Create the unwatched variable and the watch constraint
    BoolVar uw_x = BoolVar(home,x.min(),x.max());
    _unWatchedBoolVariables << uw_x;
    _watchedBoolVariables << x;
    Int::Watch<Int::BoolView>::post(home,x,uw_x,uw_x.size());
  }

  forceinline void
  QSpaceInfo::setForAll(Home home, const BoolVarArgs& x) {
    for (int i=0; i<x.size(); i++) {
      // Create the unwatched variable and the watch constraint
      BoolVar uw_x = BoolVar(home,x[i].min(),x[i].max());
      _unWatchedBoolVariables << uw_x;
      _watchedBoolVariables << x[i];
      Int::Watch<Int::BoolView>::post(home,x[i],uw_x,uw_x.size());
    }
  }

  forceinline void
  QSpaceInfo::setForAll(Home home, IntVar x) {
    // Create the unwatched variable and the watch constraint
    IntVar uw_x = IntVar(home,x.min(),x.max());
    _unWatchedIntVariables << uw_x;
    _watchedIntVariables << x;
    Int::Watch<Int::IntView>::post(home,x,uw_x,uw_x.size());
  }

  forceinline void
  QSpaceInfo::setForAll(Home home, const IntVarArgs& x) {
    for (int i=0; i<x.size(); i++) {
      // Create the unwatched variable and the watch constraint
      IntVar uw_x = IntVar(home,x[i].min(),x[i].max());
      _unWatchedIntVariables << uw_x;
      _watchedIntVariables << x[i];
      Int::Watch<Int::IntView>::post(home,x[i],uw_x,uw_x.size());
    }
  }

  forceinline BoolVar
  QSpaceInfo::getUnWatched(BoolVar x) {
    for (int i=0; i < _watchedBoolVariables.size(); i++)
      if (_watchedBoolVariables[i].same(x)) return _unWatchedBoolVariables[i];
    GECODE_NEVER;
    return x; // Only for removing warning when compilation mode is Release
  }

  forceinline IntVar
  QSpaceInfo::getUnWatched(IntVar x) {
    for (int i=0; i < _watchedIntVariables.size(); i++)
      if (_watchedIntVariables[i].same(x)) return _unWatchedIntVariables[i];
    GECODE_NEVER;
    return x; // Only for removing warning when compilation mode is Release
  }

  forceinline BoolVar*
  QSpaceInfo::unWatched(BoolVar x) {
    for (int i=0; i < _watchedBoolVariables.size(); i++)
      if (_watchedBoolVariables[i].same(x)) return &_unWatchedBoolVariables[i];
    return NULL;
  }

  forceinline IntVar*
  QSpaceInfo::unWatched(IntVar x) {
    for (int i=0; i < _watchedIntVariables.size(); i++)
      if (_watchedIntVariables[i].same(x)) return &_unWatchedIntVariables[i];
    return NULL;
  }

}
