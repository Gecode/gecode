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

#include <gecode/gist/drawingcursor.hh>

namespace Gecode { namespace Gist {

  /// Red color for failed nodes
  const QColor DrawingCursor::red(218, 37, 29);
  /// Green color for solved nodes
  const QColor DrawingCursor::green(11, 118, 70);
  /// Blue color for choice nodes
  const QColor DrawingCursor::blue(0, 92, 161);
  /// Orange color for best solutions
  const QColor DrawingCursor::orange(235, 137, 27);
  /// White color
  const QColor DrawingCursor::white(255,255,255);

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
  const int hiddenDepth = Layout::dist_y + failedWidth;

  DrawingCursor::DrawingCursor(Gist::VisualNode* root, BestNode* curBest0,
                               QPainter& painter0,
                               const QRect& clippingRect0, bool showCopies)
    : NodeCursor<VisualNode>(root), painter(painter0),
      clippingRect(clippingRect0), curBest(curBest0),
      x(0), y(0), copies(showCopies) {
    QPen pen = painter.pen();
      pen.setWidth(1);
      painter.setPen(pen);
}

  bool
  DrawingCursor::isClipped(void) {
    if (clippingRect.width() == 0 && clippingRect.x() == 0
        && clippingRect.height() == 0 && clippingRect.y() == 0)
      return false;
    BoundingBox b = node()->getBoundingBox();
    return (x + b.left > clippingRect.x() + clippingRect.width() ||
            x + b.right < clippingRect.x() ||
            y > clippingRect.y() + clippingRect.height() ||
            y + (node()->getShape()->depth()+1) * Layout::dist_y < 
            clippingRect.y());
  }

  void
  DrawingCursor::processCurrentNode(void) {
    Gist::VisualNode* n = node();
    int parentX = x - (n->getOffset());
    int parentY = y - Layout::dist_y + nodeWidth;

    int myx = x;
    int myy = y;

    if (n != startNode()) {
      if (n->isOnPath())
        painter.setPen(Qt::red);
      else
        painter.setPen(Qt::black);
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
                                   myy+hiddenDepth+shadowOffset),
                            QPoint(myx-nodeWidth+shadowOffset,
                                   myy+hiddenDepth+shadowOffset),
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
          painter.drawEllipse(myx-halfNodeWidth+shadowOffset,
                              myy+shadowOffset, nodeWidth, nodeWidth);
          break;
        case Gist::UNDETERMINED:
          painter.drawEllipse(myx-halfNodeWidth+shadowOffset,
                              myy+shadowOffset, nodeWidth, nodeWidth);
          break;
        }
      }
    }

    painter.setPen(Qt::SolidLine);
    if (n->isHidden()) {
      if (n->hasOpenChildren()) {
        QLinearGradient gradient(myx-nodeWidth,myy,myx+nodeWidth*1.3,myy+hiddenDepth*1.3);
        if (n->hasSolvedChildren()) {
          gradient.setColorAt(0, white);
          gradient.setColorAt(1, green);
        } else if (n->hasFailedChildren()) {
          gradient.setColorAt(0, white);
          gradient.setColorAt(1, red);          
        } else {
          gradient.setColorAt(0, white);
          gradient.setColorAt(1, QColor(0,0,0));
        }
        painter.setBrush(gradient);
      } else {
        if (n->hasSolvedChildren())
          painter.setBrush(QBrush(green));
        else
          painter.setBrush(QBrush(red));
      }
      
      QPoint points[3] = {QPoint(myx,myy),
                          QPoint(myx+nodeWidth,myy+hiddenDepth),
                          QPoint(myx-nodeWidth,myy+hiddenDepth),
                         };
      painter.drawConvexPolygon(points, 3);
    } else {
      switch (n->getStatus()) {
      case Gist::STEP:
      case Gist::SPECIAL:
        painter.setBrush(Qt::yellow);
        painter.drawEllipse(myx-quarterNodeWidth, myy+halfNodeWidth,
                           halfNodeWidth, halfNodeWidth);
        if (n->isOnPath())
          painter.setPen(Qt::red);
        else
          painter.setPen(Qt::black);
        painter.drawLine(myx, myy, myx, myy+halfNodeWidth);
        break;
      case Gist::SOLVED:
        {
          if (n->isCurrentBest(curBest)) {
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
        painter.setBrush(QBrush(red));
        painter.drawRect(myx-halfFailedWidth, myy, failedWidth, failedWidth);
        break;
      case Gist::BRANCH:
        painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        break;
      case Gist::UNDETERMINED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-halfNodeWidth, myy, nodeWidth, nodeWidth);
        break;
      }
    	
    }

    if (copies && n->hasCopy()) {
     painter.setBrush(Qt::darkRed);
     painter.drawEllipse(myx, myy, 10, 10);
    }

    if (copies && n->hasWorkingSpace()) {
     painter.setBrush(Qt::darkYellow);
     painter.drawEllipse(myx, myy + 10, 10, 10);
    }

  }

}}

// STATISTICS: gist-any
