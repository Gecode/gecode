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

#ifndef GECODE_GIST_NODECURSOR_HH
#define GECODE_GIST_NODECURSOR_HH

#include "gecode/gist/visualnode.hh"

namespace Gecode { namespace Gist {
  
  template <class Node>
  class NodeCursor {
  private:
    Node* _startNode;
    Node* _node;
  public:
    NodeCursor(Node* theNode);
    Node* node(void);
    bool mayMoveUpwards(void);
    void moveUpwards(void);
    bool mayMoveDownwards(void);
    void moveDownwards(void);
    bool mayMoveSidewards(void);
    void moveSidewards(void);    
  };

  class PrintCursor : public NodeCursor<VisualNode> {
  public:
    PrintCursor(VisualNode* theNode);
    
    void processCurrentNode(void);
  };
  
  class LayoutCursor : public NodeCursor<VisualNode> {
  public:
    LayoutCursor(VisualNode* theNode);

    // bool mayMoveUpwards(void);
    // void moveUpwards(void);
    bool mayMoveDownwards(void);
    // void moveDownwards(void);
    // bool mayMoveSidewards(void);
    // void moveSidewards(void);    
    
    void processCurrentNode(void);
  };

  class HideFailedCursor : public NodeCursor<VisualNode> {
  public:
    HideFailedCursor(VisualNode* theNode);
    bool mayMoveDownwards(void);
    void processCurrentNode(void);
  };
  
  class UnhideAllCursor : public NodeCursor<VisualNode> {
  public:
    UnhideAllCursor(VisualNode* theNode);
    void processCurrentNode(void);
  };
  
}}

#include "gecode/gist/nodecursor.icc"

#endif
