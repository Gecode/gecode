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
  
  /// Red color for failed nodes
  const QColor DrawingCursor::red(218, 37, 29);
  /// Green color for solved nodes
  const QColor DrawingCursor::green(11, 118, 70);
  /// Blue color for choice nodes
  const QColor DrawingCursor::blue(0, 92, 161);
  /// Orange color for best solutions
  const QColor DrawingCursor::orange(235, 137, 27);

  /// Red color for expanded failed nodes
  const QColor DrawingCursor::lightRed(218, 37, 29, 120);
  /// Green color for expanded solved nodes
  const QColor DrawingCursor::lightGreen(11, 118, 70, 120);
  /// Blue color for expanded choice nodes
  const QColor DrawingCursor::lightBlue(0, 92, 161, 120);
  
  DrawingCursor::DrawingCursor(Gist::VisualNode* root, QPainter& painter0,
                               bool heat,
                               const QRect& clippingRect0)
    : NodeCursor<VisualNode>(root), painter(painter0), 
      clippingRect(clippingRect0), x(0), y(0), heatView(heat) {}

  bool
  DrawingCursor::isClipped(void) {
    if (clippingRect.width() == 0 && clippingRect.x() == 0
        && clippingRect.height() == 0 && clippingRect.y() == 0)
      return false;
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
    int parentY = y - 38 + 20;

    int myx = x;
    int myy = y;

    // Calculate HSV hue level from heat
    int heat = (240 + static_cast<int>(((n->getHeat() / 256.0) * 180.0))) % 360;

    if (! n->isRoot()) {
      if (heatView) {
        if (n->isOnPath())
          painter.setPen(Qt::green);
        else
          painter.setPen(Qt::gray);        
      } else {
        if (n->isOnPath())
          painter.setPen(Qt::red);
        else
          painter.setPen(Qt::black);
      }
      QPainterPath path;
      path.moveTo(myx,myy);
      path.lineTo(parentX,parentY);
      painter.drawPath(path);
    }

    // draw shadow
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
        case Gist::STEP:
        case Gist::SPECIAL:
                painter.drawEllipse(myx-5+shadowOffset, myy+shadowOffset, 10, 20);
                break;
        case Gist::SINGLETON:
          {
            QPoint points[3] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                                QPoint(myx+10+shadowOffset,myy+20+shadowOffset),
                            QPoint(myx-10+shadowOffset,myy+20+shadowOffset),
                           };
            painter.drawConvexPolygon(points, 3);
          }
          break;
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
        case Gist::DECOMPOSE:
          painter.drawEllipse(myx-10+shadowOffset, myy+shadowOffset, 20, 20);
          break;
        case Gist::COMPONENT_IGNORED:
        case Gist::UNDETERMINED:
          painter.drawEllipse(myx-10+shadowOffset, myy+shadowOffset, 20, 20);
          break;
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
      case Gist::STEP:
      case Gist::SPECIAL:
        painter.setBrush(Qt::yellow);
        painter.drawEllipse(myx-5, myy+10, 10, 10);
        if (heatView) {
          if (n->isOnPath())
            painter.setPen(Qt::green);
          else
            painter.setPen(Qt::gray);        
        } else {
          if (n->isOnPath())
            painter.setPen(Qt::red);
          else
            painter.setPen(Qt::black);
        }
        painter.drawLine(myx, myy, myx, myy+10);
        break;
      case Gist::SOLVED:
        {
          if (heatView) {
            painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
          } else if (n->isCurrentBest()) {
            painter.setBrush(QBrush(orange));
          } else {
            painter.setBrush(QBrush(green));
          }
          QPoint points[4] = {QPoint(myx,myy),
                              QPoint(myx+10,myy+10),
                              QPoint(myx,myy+20),
                              QPoint(myx-10,myy+10)
                             };
          painter.drawConvexPolygon(points, 4);
        }
        break;
      case Gist::FAILED:
        if (heatView)
          painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        else
          painter.setBrush(QBrush(red));
        painter.drawRect(myx-7, myy, 14, 14);
        break;
      case Gist::DECOMPOSE:
        painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-10, myy, 20, 20);
        if (n->isOpen()) {
          painter.setBrush(Qt::white);
          painter.drawRect(myx-5, myy+5, 10, 10);
        }
        else if (n->hasSolvedChildren()) {
          painter.setBrush(QBrush(green));
          QPoint points[4] = {QPoint(myx,myy+3),
                              QPoint(myx+7,myy+10),
                              QPoint(myx,myy+17),
                              QPoint(myx-7,myy+10)
                             };
          painter.drawConvexPolygon(points, 4);
        }
        else {
          painter.setBrush(QBrush(red));
          painter.drawRect(myx-5, myy+5, 10, 10);
        }
        break;
      case Gist::BRANCH:
        if (heatView)
          painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        else
          painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
      case Gist::SINGLETON:
        {
          painter.setBrush(QBrush(green));
          QPoint points[3] = {QPoint(myx,myy),
                              QPoint(myx+10,myy+20),
                          QPoint(myx-10,myy+20),
                         };
          painter.drawConvexPolygon(points, 3);
        }
        break;
      case Gist::COMPONENT_IGNORED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-10, myy, 20, 20);
        painter.setBrush(QBrush(red));
        painter.drawRect(myx-5, myy+5, 10, 10);			  
        break;
      case Gist::UNDETERMINED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
      }
    	
    }
    
  }
  
}}

// STATISTICS: gist-any
