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

#ifndef GECODE_GIST_VISUALNODE_HH
#define GECODE_GIST_VISUALNODE_HH

#include "gecode/gist/spacenode.hh"
#include "gecode/gist/shapelist.hh"
#include "gecode/kernel.hh"
#include <vector>

namespace Gecode { namespace Gist {

  class VisualNode : public SpaceNode {
  protected:
    int offset;
    
    bool dirty;
    bool hidden;
    bool marked;
    
    Shape* shape;
    BoundingBox box;
    
    bool containsCoordinateAtDepth(int x, int depth);
  public:
    VisualNode(int alternative, BestSpace* b);
    VisualNode(Space* root, Better* b);
    virtual ~VisualNode(void);
    
    static const Shape singletonShape;
    static const Shape unitShape;

    bool isHidden(void);
    void setHidden(bool h);
    void dirtyUp(void);
    void layout(void);
    int getOffset(void);
    void setOffset(int n);
    bool isDirty(void);
    void setDirty(bool d);
    bool isMarked(void);
    void setMarked(bool m);
    
    void toggleHidden(void);
    void hideFailed(void);
    void unhideAll(void);
    
    Shape* getShape(void);
    void setShape(Shape* s);
    void setBoundingBox(BoundingBox b);
    BoundingBox getBoundingBox(void);
    
    virtual SpaceNode* createChild(int alternative);
    virtual void changedStatus();
    
    VisualNode* getChild(int i);
    
    VisualNode* findNode(int x, int y);    
  };

}}

#endif

// STATISTICS: gist-any
