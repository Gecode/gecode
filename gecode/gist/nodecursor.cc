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

#include "gecode/gist/nodecursor.hh"

namespace Gecode { namespace Gist {
  
  HideFailedCursor::HideFailedCursor(VisualNode* root)
   : NodeCursor<VisualNode>(root) {}
    
  void
  HideFailedCursor::processCurrentNode(void) {
    VisualNode* n = node();
    if ((n->getStatus() == BRANCH ||
         ((n->getStatus() == SPECIAL || n->getStatus() == STEP) && n->hasFailedChildren())) &&
        !n->hasSolvedChildren() &&
        n->getNoOfOpenChildren() == 0) {
      n->setHidden(true);
      n->setChildrenLayoutDone(false);
      n->dirtyUp();
    }
  }

  UnhideAllCursor::UnhideAllCursor(VisualNode* root)
   : NodeCursor<VisualNode>(root) {}
  
  void
  UnhideAllCursor::processCurrentNode(void) {
    VisualNode* n = node();
    if (n->isHidden()) {
      n->setHidden(false);
      n->dirtyUp();
    }
  }

  NextSolCursor::NextSolCursor(VisualNode* theNode, bool backwards)
   : NodeCursor<VisualNode>(theNode), back(backwards) {}

  void
  NextSolCursor::processCurrentNode(void) {}

  bool
  NextSolCursor::notOnSol(void) {
    return node() == startNode() || node()->getStatus() != SOLVED;
  }

  bool
  NextSolCursor::mayMoveUpwards(void) {
    return notOnSol() && !node()->isRoot();
  }

  bool
  NextSolCursor::mayMoveDownwards(void) {
    return notOnSol() && !(back && node() == startNode())
           && node()->hasSolvedChildren()
           && NodeCursor<VisualNode>::mayMoveDownwards();
  }

  void
  NextSolCursor::moveDownwards(void) {
    NodeCursor<VisualNode>::moveDownwards();
    if (back) {
      while (NodeCursor<VisualNode>::mayMoveSidewards())
        NodeCursor<VisualNode>::moveSidewards();
    }
  }

  bool
  NextSolCursor::mayMoveSidewards(void) {
    if (back) {
      return notOnSol() && alternative() > 0;
    } else {
      return notOnSol() && NodeCursor<VisualNode>::mayMoveSidewards();
    }
  }

  void
  NextSolCursor::moveSidewards(void) {
    if (back) {
      alternative(alternative()-1);
      node(node()->getParent()->getChild(alternative()));
    } else {
      NodeCursor<VisualNode>::moveSidewards();      
    }
  }
  
}}

// STATISTICS: gist-any
