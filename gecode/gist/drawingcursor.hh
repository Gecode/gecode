/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-08-04 16:06:52 +0200 (Fri, 04 Aug 2006) $ by $Author: schulte $
 *     $Revision: 3517 $
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

#ifndef GECODE_GIST_QT_DRAWINGCURSOR_HH
#define GECODE_GIST_QT_DRAWINGCURSOR_HH

#include "gecode/gist/nodecursor.hh"
#include <QtGui>

namespace Gecode { namespace Gist {

  class DrawingCursor : public Gist::NodeCursor<VisualNode> {
  private:
    QPainter& painter;
    QRect clippingRect;
    int x, y;
    
    static const QColor red;
    static const QColor green;
    static const QColor blue;
    
    bool isClipped(void);
  public:
    DrawingCursor(Gist::VisualNode* root, QPainter& painter0,
                   const QRect& clippingRect0);

    void moveUpwards(void);
    bool mayMoveDownwards(void);
    void moveDownwards(void);
    void moveSidewards(void);
    void processCurrentNode(void);
  };

}}

#endif
