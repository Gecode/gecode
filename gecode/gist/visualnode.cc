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

#include "gecode/gist/visualnode.hh"

#include "gecode/gist/layoutcursor.hh"
#include "gecode/gist/nodevisitor.hh"

namespace Gecode { namespace Gist {

  VisualNode::VisualNode(int alternative, BestNode* b)
  : SpaceNode(alternative, b), offset(0), dirty(true), 
    childrenLayoutDone(false), hidden(false), marked(false)
  , onPath(false), lastOnPath(false), pathAlternative(-1)
  , heat(0)
  , shape(NULL), box(0,0,0)
  {}

  VisualNode::VisualNode(Space* root, Better* b)
  : SpaceNode(root, b), offset(0), dirty(true), childrenLayoutDone(false), 
    hidden(false), marked(false)
  , onPath(false), lastOnPath(false), pathAlternative(-1)
  , heat(0)
  , shape(NULL), box(0,0,0)
  {}

  VisualNode::~VisualNode(void) {
    delete shape;
  }

  void
  VisualNode::dirtyUp(void) {
    VisualNode* cur = this;
    while (!cur->dirty) {
      cur->dirty = true;
      if (! cur->isRoot()) {
        cur = cur->getParent();
      }
    }
  }
  
  void
  VisualNode::layout(void) {
    LayoutCursor l(this);
    PostorderNodeVisitor<LayoutCursor> p(l);
    // int nodesLayouted = 1;
    // clock_t t0 = clock();
    while (p.next()) {}
    // while (p.next()) { nodesLayouted++; }
    // double t = (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC) * 1000.0;
    // double nps = static_cast<double>(nodesLayouted) / 
    //   (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC);
    // std::cout << "Layout done. " << nodesLayouted << " nodes in "
    //   << t << " ms. " << nps << " nodes/s." << std::endl;
  }
    
  void
  VisualNode::setPathInfos(bool onPath0, int pathAlternative0, bool lastOnPath0) {
    assert(pathAlternative0 <= noOfChildren);
    assert(!lastOnPath0 || pathAlternative0 == -1);

    onPath = onPath0;
    pathAlternative = pathAlternative0;
    lastOnPath = lastOnPath0;
  }
  
  void VisualNode::pathUp(void) {
    if(isRoot())
      return;
    
    VisualNode* p = getParent();
    p->setPathInfos(true, alternative);
    p->pathUp();
  }

  void VisualNode::unPathUp(void) {
    if(isRoot())
      return;
    
    setPathInfos(false);

    VisualNode* p = getParent();
    p->unPathUp();
  }

  void
  VisualNode::toggleHidden(void) {
    hidden = !hidden;
    dirtyUp();
  }
  
  void
  VisualNode::hideFailed(void) {
    HideFailedCursor c(this);
    PreorderNodeVisitor<HideFailedCursor> v(c);
    while (v.next()) {}
    dirtyUp();
  }
  
  void
  VisualNode::unhideAll(void) {
    UnhideAllCursor c(this);
    PreorderNodeVisitor<UnhideAllCursor> v(c);
    while (v.next()) {}
    dirtyUp();
  }
  
      
  VisualNode*
  VisualNode::createChild(int alternative) {
    return new VisualNode(alternative, curBest);
  }


  void
  VisualNode::changedStatus() { dirtyUp(); }
  
  bool
  VisualNode::containsCoordinateAtDepth(int x, int depth) {
    if (x < box.left ||
      x > box.right ||
      depth > box.depth) {
      return false;
    }
    Extent theExtent;
     if (shape->getExtentAtDepth(depth, theExtent)) {
      return (theExtent.l <= x && x <= theExtent.r);
    } else {
      return false;
    }    
  }

  VisualNode*
  VisualNode::findNode(int x, int y) {
    VisualNode* cur = this;
    int depth = y / Layout::dist_y;

    while (depth > 0 && cur != NULL) {
     if (cur->isHidden()) {
        break;
      }
      VisualNode* oldCur = cur;
      cur = NULL;
      for (int i=0; i<oldCur->noOfChildren; i++) {
        VisualNode* nextChild = oldCur->getChild(i);
        int newX = x - nextChild->getOffset();
        if (nextChild->containsCoordinateAtDepth(newX, depth - 1)) {
          cur = nextChild;
          x = newX;
          break;
        }
      }
      depth--;
      y -= Layout::dist_y;
    }
   
    if(cur == this && !cur->containsCoordinateAtDepth(x, 0)) {
      return NULL;
    }
    return cur;
  }

  std::string
  VisualNode::toolTip(void) {
    if (getStatus() != BRANCH && getStatus() != DECOMPOSE)
      return "";
    Space* ws = getSpace();
    (void) ws->status();
    assert(ws->status() == SS_BRANCH);
    // const BranchingDesc* d = ws->description();
    Reflection::VarMap vm;
    ws->getVars(vm, false);
    std::string tt;
    try {
      Reflection::BranchingSpec bs = ws->branchingSpec(vm,d);
      delete d;
      tt = "Alternatives:\n";
      for (unsigned int i=0; i<bs.alternatives(); i++) {
        std::ostringstream is; is << i;
        tt += is.str()+": ";
        Reflection::ArrayArg& aa = *bs[i]->toArray();
        for (int j=0; j<aa.size(); j++) {
          if (aa[j]->isString()) {
            tt += aa[j]->toString();
          } else if (aa[j]->isVar()) {
            int v = aa[j]->toVar();
            if (vm.hasName(v))
              tt += vm.name(v).toString();
            else {
              std::ostringstream vs; vs << v;
              tt += "_v"+vs.str();
            }
          } else if (aa[j]->isInt()) {
            std::ostringstream vs; vs << aa[j]->toInt();
            tt += vs.str();
          } else {
            tt += "error in BranchingSpec";
          }
        }
        if (i < bs.alternatives()-1)
          tt += "\n";
      }
    } catch (Reflection::ReflectionException&) {
      tt = "No information available";
    }
    return tt;
  }
  
}}

// STATISTICS: gist-any
