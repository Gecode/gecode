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
  
  const int nodeWidth = 20;
  const int halfNodeWidth = nodeWidth / 2;
  const int quarterNodeWidth = halfNodeWidth / 2;
  const int failedWidth = 14;
  const int halfFailedWidth = failedWidth / 2;
  const int shadowOffset = 3;
  const int dSolvedOffset = nodeWidth / 6;
  const int dSolvedHalfWidth = (nodeWidth-2*dSolvedOffset) / 2;
  
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
            y + (b.depth+1) * Layout::dist_y < clippingRect.y());
  }
  
  void
  DrawingCursor::processCurrentNode(void) {
    Gist::VisualNode* n = node();
    int parentX = x - (n->getOffset());
    int parentY = y - Layout::dist_y + nodeWidth;

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
    if (n->isMarked()) {
      painter.setBrush(Qt::gray);
      painter.setPen(Qt::NoPen);
      if (n->isHidden()) {
        QPoint points[3] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                            QPoint(myx+nodeWidth+shadowOffset,
                                   myy+3*nodeWidth+shadowOffset),
                            QPoint(myx-nodeWidth+shadowOffset,
                                   myy+3*nodeWidth+shadowOffset),
                           };
        painter.drawConvexPolygon(points, 3);
        
      } else {
        switch (n->getStatus()) {
        case Gist::STEP:
        case Gist::SPECIAL:
                painter.drawEllipse(myx-quarterNodeWidth+shadowOffset, 
                                    myy+shadowOffset, halfNodeWidth, 
                                    nodeWidth);
                break;
        case Gist::SINGLETON:
          {
            QPoint points[3] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                                QPoint(myx+halfNodeWidth+shadowOffset,
                                       myy+nodeWidth+shadowOffset),
                                QPoint(myx-halfNodeWidth+shadowOffset,
                                       myy+nodeWidth+shadowOffset),
                               };
            painter.drawConvexPolygon(points, 3);
          }
          break;
        case Gist::SOLVED:
          {
            QPoint points[4] = {QPoint(myx+shadowOffset,myy+shadowOffset),
                                QPoint(myx+halfNodeWidth+shadowOffset,
                                       myy+halfNodeWidth+shadowOffset),
                                QPoint(myx+shadowOffset,
                                       myy+nodeWidth+shadowOffset),
                                QPoint(myx-halfNodeWidth+shadowOffset,
                                       myy+halfNodeWidth+shadowOffset)
                               };
            painter.drawConvexPolygon(points, 4);
          }
          break;
        case Gist::FAILED:
          painter.drawRect(myx-halfFailedWidth+shadowOffset,
                           myy+shadowOffset, failedWidth, failedWidth);
          break;
        case Gist::BRANCH:
        case Gist::DECOMPOSE:
          painter.drawEllipse(myx-halfNodeWidth+shadowOffset, 
                              myy+shadowOffset, nodeWidth, nodeWidth);
          break;
        case Gist::COMPONENT_IGNORED:
        case Gist::UNDETERMINED:
          painter.drawEllipse(myx-halfNodeWidth+shadowOffset, 
                              myy+shadowOffset, nodeWidth, nodeWidth);
          break;
        }
      }        
    }

    painter.setPen(Qt::SolidLine);
    if (n->isHidden()) {
      painter.setBrush(QBrush(red));
      QPoint points[3] = {QPoint(myx,myy),
                          QPoint(myx+nodeWidth,myy+3*nodeWidth),
                          QPoint(myx-nodeWidth,myy+3*nodeWidth),
                         };
      painter.drawConvexPolygon(points, 3);
    } else {
      switch (n->getStatus()) {
      case Gist::STEP:
      case Gist::SPECIAL:
        painter.setBrush(Qt::yellow);
        painter.drawEllipse(myx-quarterNodeWidth, myy+halfNodeWidth,
                           halfNodeWidth, halfNodeWidth);
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
        painter.drawLine(myx, myy, myx, myy+halfNodeWidth);
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
                              QPoint(myx+halfNodeWidth,myy+halfNodeWidth),
                              QPoint(myx,myy+nodeWidth),
                              QPoint(myx-halfNodeWidth,myy+halfNodeWidth)
                             };
          painter.drawConvexPolygon(points, 4);
        }
        break;
      case Gist::FAILED:
        if (heatView)
          painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        else
          painter.setBrush(QBrush(red));
        painter.drawRect(myx-halfFailedWidth, myy, failedWidth, failedWidth);
        break;
      case Gist::DECOMPOSE:
        painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        if (n->isOpen()) {
          painter.setBrush(Qt::white);
          painter.drawRect(myx-quarterNodeWidth, myy+quarterNodeWidth,
                           halfNodeWidth, halfNodeWidth);
        }
        else if (n->hasSolvedChildren()) {
          painter.setBrush(QBrush(green));
          QPoint points[4] = {QPoint(myx,myy+dSolvedOffset),
                              QPoint(myx+dSolvedHalfWidth,
                                     myy+dSolvedOffset+dSolvedHalfWidth),
                              QPoint(myx,myy+nodeWidth-dSolvedOffset),
                              QPoint(myx-dSolvedHalfWidth,
                                     myy+dSolvedOffset+dSolvedHalfWidth)
                             };
          painter.drawConvexPolygon(points, 4);
        }
        else {
          painter.setBrush(QBrush(red));
          painter.drawRect(myx-quarterNodeWidth, myy+quarterNodeWidth,
                           halfNodeWidth, halfNodeWidth);
        }
        break;
      case Gist::BRANCH:
        if (heatView)
          painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        else
          painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        break;
      case Gist::SINGLETON:
        {
          painter.setBrush(QBrush(green));
          QPoint points[3] = {QPoint(myx,myy),
                              QPoint(myx+halfNodeWidth,myy+nodeWidth),
                              QPoint(myx-halfNodeWidth,myy+nodeWidth),
                             };
          painter.drawConvexPolygon(points, 3);
        }
        break;
      case Gist::COMPONENT_IGNORED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        painter.setBrush(QBrush(red));
        painter.drawRect(myx-quarterNodeWidth, myy+quarterNodeWidth,
                         halfNodeWidth, halfNodeWidth);
        break;
      case Gist::UNDETERMINED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        break;
      }
    	
    }
    
  }
  
}}

// STATISTICS: gist-any
