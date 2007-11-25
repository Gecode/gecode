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

#include "gecode/gist/drawingcursor.hh"

namespace Gecode { namespace Gist {
  
  const QColor DrawingCursor::red(218, 37, 29);
  const QColor DrawingCursor::green(11, 118, 70);
  const QColor DrawingCursor::blue(0, 92, 161);
  
  DrawingCursor::DrawingCursor(Gist::VisualNode* root, QPainter& painter0,
                               const QRect& clippingRect0)
    : NodeCursor<VisualNode>(root), painter(painter0), clippingRect(clippingRect0), x(0), y(0) {}

  void
  DrawingCursor::moveUpwards(void) { 
    x -= node()->getOffset();
    y -= 38;
    NodeCursor<VisualNode>::moveUpwards();
  }

  bool
  DrawingCursor::mayMoveDownwards(void) {
    return NodeCursor<VisualNode>::mayMoveDownwards() &&
           !node()->isHidden() &&
           !isClipped();
  }

  void
  DrawingCursor::moveDownwards(void) {
    NodeCursor<VisualNode>::moveDownwards();
    x += node()->getOffset();
    y += 38;
  }

  void
  DrawingCursor::moveSidewards(void) {
    x -= node()->getOffset();
    NodeCursor<VisualNode>::moveSidewards();
    x += node()->getOffset();
  }

  bool
  DrawingCursor::isClipped(void) {
    BoundingBox b = node()->getBoundingBox();
    return (x + b.left > clippingRect.x() + clippingRect.width() ||
            x + b.right < clippingRect.x() || 
            y > clippingRect.y() + clippingRect.height() ||
            y + (b.depth+1) * 38 < clippingRect.y());
  }
  
  void
  DrawingCursor::processCurrentNode(void) {
    Gist::VisualNode* n = node();
    int parentX = x - (n->getOffset());
    int parentY = 38 + y - 38 + 20;
  
    int myx = x;
    int myy = 38 + y;

    if (! n->isRoot()) {
      painter.setBrush(Qt::black);
      painter.drawLine(myx,myy,parentX,parentY);
    }

    int shadowOffset = 3;
    if (n->isMarked()) {
      painter.setBrush(Qt::gray);
      painter.setPen(Qt::NoPen);
      if (n->isHidden()) {
        QPoint points[3] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                            QPoint(myx+20+shadowOffset,myy+60+shadowOffset),
                            QPoint(myx-20+shadowOffset,myy+60+shadowOffset),
                           };
        painter.drawConvexPolygon(points, 3);
        
      } else {
        switch (n->getStatus()) {
        case Gist::SOLVED:
          {
            QPoint points[4] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                                QPoint(myx+10+shadowOffset,myy+10+shadowOffset),
                                QPoint(myx+shadowOffset,myy+20+shadowOffset),
                                QPoint(myx-10+shadowOffset,myy+10+shadowOffset)
                               };
            painter.drawConvexPolygon(points, 4);
          }
          break;
        case Gist::FAILED:
          painter.drawRect(myx-7+shadowOffset, myy+shadowOffset, 14, 14);
          break;
        case Gist::BRANCH:
          painter.drawEllipse(myx-10+shadowOffset, myy+shadowOffset, 20, 20);
          break;
        case Gist::UNDETERMINED:
          painter.drawEllipse(myx-10+shadowOffset, myy+shadowOffset, 20, 20);
          break;
          break;
        default: assert(false);
        }
      }        
    }

    painter.setPen(Qt::SolidLine);
    if (n->isHidden()) {
      painter.setBrush(QBrush(red));
      QPoint points[3] = {QPoint(myx,myy),
                          QPoint(myx+20,myy+60),
                          QPoint(myx-20,myy+60),
                         };
      painter.drawConvexPolygon(points, 3);
    } else {
      switch (n->getStatus()) {
      case Gist::SOLVED:
        {
          painter.setBrush(QBrush(green));
          QPoint points[4] = {QPoint(myx,myy),
                              QPoint(myx+10,myy+10),
                              QPoint(myx,myy+20),
                              QPoint(myx-10,myy+10)
                             };
          painter.drawConvexPolygon(points, 4);
        }
        break;
      case Gist::FAILED:
        painter.setBrush(QBrush(red));
        painter.drawRect(myx-7, myy, 14, 14);
        break;
      case Gist::BRANCH:
        painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
      case Gist::UNDETERMINED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
        break;
      default: assert(false);
      }
    }
  }
  
}}

// STATISTICS: gist-any
