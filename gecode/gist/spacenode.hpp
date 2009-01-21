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

namespace Gecode { namespace Gist {

  forceinline SpaceNode*
  SpaceNode::getParent() {
    return static_cast<SpaceNode*>(Node::getParent());
  }

  forceinline SpaceNode*
  SpaceNode::getChild(int i) {
    return static_cast<SpaceNode*>(Node::getChild(i));
  }

  forceinline void
  SpaceNode::setHasOpenChildren(bool b) {
    if (b)
      nstatus |= 1<<(HASOPENCHILDREN-1);
    else
      nstatus &= ~(1<<(HASOPENCHILDREN-1));
  }

  forceinline void
  SpaceNode::setHasFailedChildren(bool b) {
    if (b)
      nstatus |= 1<<(HASFAILEDCHILDREN-1);
    else
      nstatus &= ~(1<<(HASFAILEDCHILDREN-1));
  }

  forceinline void
  SpaceNode::setHasSolvedChildren(bool b) {
    if (b)
      nstatus |= 1<<(HASSOLVEDCHILDREN-1);
    else
      nstatus &= ~(1<<(HASSOLVEDCHILDREN-1));
  }

  forceinline void
  SpaceNode::setStatus(NodeStatus s) {
    nstatus &= ~( STATUSMASK );
    nstatus |= s;
  }

  forceinline NodeStatus
  SpaceNode::getStatus(void) const {
    return static_cast<NodeStatus>(nstatus & STATUSMASK);
  }

  forceinline
  SpaceNode::SpaceNode(void)
  : copy(NULL), workingSpace(NULL), ownBest(NULL) {
    desc.branch = NULL;
    setStatus(UNDETERMINED);
    setHasSolvedChildren(false);
    setHasFailedChildren(false);
  }

  forceinline Space*
  SpaceNode::getSpace(BestNode* curBest, int c_d, int a_d) {
    acquireSpace(curBest,c_d,a_d);
    Space* ret = workingSpace;
    workingSpace = NULL;
    return ret;
  }

  forceinline const Space*
  SpaceNode::getWorkingSpace(void) const {
    return workingSpace;
  }

  forceinline void
  SpaceNode::purge(void) {
    if (getStatus() != SOLVED || ownBest == NULL) {
      // only delete working spaces from solutions if we are not in BAB
      delete workingSpace;
      workingSpace = NULL;
    }
    if (!isRoot()) {
      delete copy;
      copy = NULL;
    }
  }


  forceinline bool
  SpaceNode::isCurrentBest(BestNode* curBest) {
    return curBest != NULL && curBest->s == this;
  }

  forceinline void
  SpaceNode::setSpecialDesc(const SpecialDesc* d) {
    desc.special = d;
  }

  forceinline void
  SpaceNode::setStepDesc(StepDesc* d) {
    desc.step = d;
  }

  forceinline bool
  SpaceNode::isStepNode(void) {
    return getStatus() == STEP;
  }

  forceinline StepDesc*
  SpaceNode::getStepDesc(void) {
    return (isStepNode() ? desc.step : NULL);
  }

  forceinline bool
  SpaceNode::isOpen(void) {
    return getStatus() == UNDETERMINED
           || (nstatus & (1<<(HASOPENCHILDREN-1)));
  }

  forceinline bool
  SpaceNode::hasFailedChildren(void) {
    return nstatus & (1<<(HASFAILEDCHILDREN-1));
  }

  forceinline bool
  SpaceNode::hasSolvedChildren(void) {
    return nstatus & (1<<(HASSOLVEDCHILDREN-1));
  }

  forceinline bool
  SpaceNode::hasOpenChildren(void) {
    return nstatus & (1<<(HASOPENCHILDREN-1));
  }

  forceinline bool
  SpaceNode::hasCopy(void) {
    return copy != NULL;
  }

  forceinline bool
  SpaceNode::hasWorkingSpace(void) {
    return workingSpace != NULL;
  }

  forceinline int
  SpaceNode::getAlternative(void) {
    SpaceNode* p = getParent();
    if (p) {
      for (int i=p->getNumberOfChildren(); i--;)
        if (p->getChild(i) == this)
          return i;
    }
    return -1;
  }

}}

// STATISTICS: gist-any
