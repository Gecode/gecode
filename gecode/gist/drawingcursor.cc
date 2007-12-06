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

  void
  DrawingCursor::moveUpwards(void) { 
    x -= node()->getOffset();
    y -= 38;
    NodeCursor<VisualNode>::moveUpwards();
#ifdef GECODE_GIST_EXPERIMENTAL
    while (node()->isStepNode() && node()->isHidden()) {
      x -= node()->getOffset();
      NodeCursor<VisualNode>::moveUpwards();
    }
#endif
  }

  bool
  DrawingCursor::mayMoveDownwards(void) {
    return NodeCursor<VisualNode>::mayMoveDownwards() &&
           !node()->isHidden() &&
           !isClipped();
  }

#ifdef GECODE_GIST_EXPERIMENTAL
  bool
  DrawingCursor::mayMoveSidewards(void) {
    if(node() != NULL && !node()->isStepNode() && node()->getParent() != NULL
        && node()->getParent()->isStepNode() && node()->getParent()->isHidden()) {
          return (node()->getRealParent()->getNumberOfChildren() > node()->getRealAlternative() + 1);
    } else {
      return NodeCursor<VisualNode>::mayMoveSidewards();
    }
  }
#endif

  void
  DrawingCursor::moveDownwards(void) {
    NodeCursor<VisualNode>::moveDownwards();
    x += node()->getOffset();
    y += 38;
#ifdef GECODE_GIST_EXPERIMENTAL
    while (node()->isStepNode() && node()->isHidden()) {
      NodeCursor<VisualNode>::moveDownwards();
      x += node()->getOffset();
    }
#endif
  }

  void
  DrawingCursor::moveSidewards(void) {
#ifdef GECODE_GIST_EXPERIMENTAL
    if(node()->getParent() != NULL && node()->getParent()->isStepNode()
        && node()->getParent()->isHidden()) {
      NodeCursor<VisualNode>::moveUpwards();
      while (node()->isStepNode() && !node()->isFirstStepNode() && node()->isHidden()) {
        x -= node()->getOffset();
        NodeCursor<VisualNode>::moveUpwards();
      }
    }
    x -= node()->getOffset();
    NodeCursor<VisualNode>::moveSidewards();
    x += node()->getOffset();
    while (node()->isStepNode() && node()->isHidden()) {
      NodeCursor<VisualNode>::moveDownwards();
      x += node()->getOffset();
    }
#else
    x -= node()->getOffset();
    NodeCursor<VisualNode>::moveSidewards();
    x += node()->getOffset();
#endif
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
            y + (b.depth+1) * 38 < clippingRect.y());
  }
  
  void
  DrawingCursor::processCurrentNode(void) {
    Gist::VisualNode* n = node();
    int parentX = x - (n->getOffset());
    int parentY = y - 38 + 20;
    
#ifdef GECODE_GIST_EXPERIMENTAL
    VisualNode* p = n->getParent();
    while(p != NULL && p->isStepNode() && p->isHidden()) {
      parentX -= p->getOffset();
      p = p->getParent();
    }
#endif
  
    int myx = x;
    int myy = y;

    // Calculate HSV hue level from heat
    int heat = (240 + (std::abs(n->getHeat()) % 180)) % 360;

#ifdef GECODE_GIST_EXPERIMENTAL

    if(n->isStepNode()) {
      switch (n->getMetaStatus()) {
        case Gist::FAILED:
          painter.setBrush(QBrush(lightRed));
          break;
        case Gist::SOLVED:
          painter.setBrush(QBrush(lightGreen));
          break;
        case Gist::BRANCH:
          painter.setBrush(QBrush(lightBlue));
          break;
        case Gist::STEP:
        case Gist::SPECIAL:
        case Gist::UNDETERMINED:
          break;
      }

      if (heatView)
        painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
      
      int leftX = myx-10;
      int rightX = myx+10;
      int topY = myy;
      int bottomY = myy+20;
      int height = bottomY - topY;
      int width = rightX - leftX;
      
      if (n->getMetaStatus() == Gist::FAILED) {
        leftX = myx-7;
        rightX = myx+7;
        width = 14;
        height = 20;
      }

      if(n->isFirstStepNode()) {
        switch (n->getMetaStatus()) {
        case Gist::SOLVED: {
          QPoint points[4] = {QPoint(myx,topY),
                              QPoint(myx+10,myy+10),
                              QPoint(myx,myy+20),
                              QPoint(myx-10,myy+10)
          };
          painter.setPen(Qt::SolidLine);
          painter.drawConvexPolygon(points, 4);
          break;
        }
        case Gist::BRANCH:
          painter.setPen(Qt::SolidLine);
          painter.drawEllipse(leftX, topY, width, height);
          break;
        case Gist::FAILED:
          painter.setPen(Qt::SolidLine);
          painter.drawRect(leftX, topY, width, height);
          break;
        case Gist::STEP:
        case Gist::SPECIAL:
        case Gist::UNDETERMINED:
          break;
        }
        height = 18;
        topY = myy+10;
        bottomY = myy+28;
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.drawRect(leftX, topY, width, height);
        painter.restore();
        
        if(n->isLastStepNode()) {
          height += 20;
          bottomY += 20;
        }
      } else if(n->isLastStepNode()) {
        height = 58;
        topY = myy-10;
        bottomY = myy+48;
      } else {
        height = 38;
        topY = myy-10;
        bottomY = myy+28;
      }

      painter.setPen(Qt::NoPen);
      painter.drawRect(leftX, topY, width, height);

      painter.setPen(Qt::SolidLine);
      painter.drawLine(leftX, topY, leftX, bottomY);
      painter.drawLine(rightX, topY, rightX, bottomY);
    }

#endif

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
      painter.drawLine(myx,myy,parentX,parentY);
    }

#ifdef GECODE_GIST_EXPERIMENTAL
    
    // the more visible 'shadow'
    int shadowOffset = 4;
    if (n->isMarked()) {
      if (heatView) {
        painter.setBrush(Qt::white);
      } else {
        painter.setBrush(Qt::black);
      }
      painter.setPen(Qt::NoPen);
      if (n->isHidden()) {
        QPoint points[3] = {QPoint(myx,myy-2*shadowOffset),
                            QPoint(myx+20+shadowOffset,myy+60+shadowOffset),
                            QPoint(myx-20-shadowOffset,myy+60+shadowOffset),
                           };
        painter.drawConvexPolygon(points, 3);
        
      } else {
        switch (n->getStatus()) {
        case Gist::STEP:
        case Gist::SPECIAL:
                painter.drawEllipse(myx-5-shadowOffset, myy+10-shadowOffset, 10+2*shadowOffset, 10+2*shadowOffset);
                break;
        case Gist::SOLVED:
          {
            QPoint points[4] = {QPoint(myx,(int)(myy-1.4*shadowOffset)),
                                QPoint((int)(myx+10+1.4*shadowOffset),myy+10),
                                QPoint(myx,(int)(myy+20+1.4*shadowOffset)),
                                QPoint((int)(myx-10-1.4*shadowOffset),myy+10)
                               };
            painter.drawConvexPolygon(points, 4);
          }
          break;
        case Gist::FAILED:
          painter.drawRect(myx-7-shadowOffset, myy-shadowOffset, 14+2*shadowOffset, 14+2*shadowOffset);
          break;
        case Gist::BRANCH:
          painter.drawEllipse(myx-10-shadowOffset, myy-shadowOffset, 20+2*shadowOffset, 20+2*shadowOffset);
          break;
        case Gist::UNDETERMINED:
          painter.drawEllipse(myx-10-shadowOffset, myy-shadowOffset, 20+2*shadowOffset, 20+2*shadowOffset);
          break;
        }
      }        
    }

#else
    // original shadow
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
        }
      }        
    }
#endif

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
#ifdef GECODE_GIST_EXPERIMENTAL
        if(!n->isFirstStepNode()) {
#endif
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
#ifdef GECODE_GIST_EXPERIMENTAL
        }
#endif
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
      case Gist::BRANCH:
        if (heatView)
          painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        else
          painter.setBrush(QBrush(blue));
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
      case Gist::UNDETERMINED:
        painter.setBrush(Qt::white);
        painter.drawEllipse(myx-10, myy, 20, 20);
        break;
      }
    	
    }

#ifdef GECODE_GIST_EXPERIMENTAL
    
    if (n->hasCopy()) {
    	painter.setBrush(Qt::darkRed);
    	painter.drawEllipse(myx, myy, 10, 10);
    }
    
    if (n->hasWorkingSpace()) {
    	painter.setBrush(Qt::darkYellow);
    	painter.drawEllipse(myx, myy + 10, 10, 10);
    }
    
    if (n->isCollapsed()) {
        painter.setBrush(Qt::yellow);
        painter.drawEllipse(myx-10, myy, 10, 10);
    }
    
#endif
    
  }
  
}}

// STATISTICS: gist-any
