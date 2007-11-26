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

#include "gecode/gist/treecanvas.hh"

#include "gecode/gist/nodevisitor.hh"
#include "gecode/gist/shapelist.hh"
#include "gecode/gist/visualnode.hh"
#include "gecode/gist/dfs.hh"

#include "gecode/gist/postscript.hh"

#include "gecode/gist/drawingcursor.hh"

#include <QPainter>

namespace Gecode { namespace Gist {

  Inspector::~Inspector(void) {}
    
  TreeCanvasImpl::TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent)
    : QWidget(parent), inspect(NULL) {
      root = new VisualNode(rootSpace, b);
      root->setMarked(true);
      currentNode = root;
      scale = 1.0;
      
      QAction* inspectCN = new QAction("inspect", this);
      inspectCN->setShortcut(QKeySequence("Return"));
      connect(inspectCN, SIGNAL(triggered()), this, 
                         SLOT(inspectCurrentNode()));

      QAction* navUp = new QAction("Up", this);
      navUp->setShortcut(QKeySequence("Up"));
      connect(navUp, SIGNAL(triggered()), this, 
                     SLOT(navUp()));

      QAction* navDown = new QAction("Down", this);
      navDown->setShortcut(QKeySequence("Down"));
      connect(navDown, SIGNAL(triggered()), this, 
                       SLOT(navDown()));
      
      QAction* navLeft = new QAction("Left", this);
      navLeft->setShortcut(QKeySequence("Left"));
      connect(navLeft, SIGNAL(triggered()), this, 
                       SLOT(navLeft()));

      QAction* navRight = new QAction("Left", this);
      navRight->setShortcut(QKeySequence("Right"));
      connect(navRight, SIGNAL(triggered()), this, 
                        SLOT(navRight()));
            

      QAction* searchNext = new QAction("search next", this);
      searchNext->setShortcut(QKeySequence("N"));
      connect(searchNext, SIGNAL(triggered()), this, SLOT(searchOne()));

      QAction* searchAll = new QAction("search all", this);
      searchAll->setShortcut(QKeySequence("A"));
      connect(searchAll, SIGNAL(triggered()), this, SLOT(searchAll()));
      
      QAction* toggleHidden = new QAction("(un)hide", this);
      toggleHidden->setShortcut(QKeySequence("H"));
      connect(toggleHidden, SIGNAL(triggered()), this, SLOT(toggleHidden()));

      QAction* hideFailed = new QAction("hide failed", this);
      hideFailed->setShortcut(QKeySequence("F"));
      connect(hideFailed, SIGNAL(triggered()), this, SLOT(hideFailed()));

      QAction* unhideAll = new QAction("unhide all", this);
      unhideAll->setShortcut(QKeySequence("U"));
      connect(unhideAll, SIGNAL(triggered()), this, SLOT(unhideAll()));

      QAction* zoomToFit = new QAction("zoom to fit", this);
      zoomToFit->setShortcut(QKeySequence("Z"));
      connect(zoomToFit, SIGNAL(triggered()), this, SLOT(zoomToFit()));

      QAction* centerCN = new QAction("center current node", this);
      centerCN->setShortcut(QKeySequence("C"));
      connect(centerCN, SIGNAL(triggered()), this, SLOT(centerCurrentNode()));

      QAction* exportPostscript = new QAction("export postscript", this);
      connect(exportPostscript, SIGNAL(triggered()), this, SLOT(exportPostscript()));

      addAction(inspectCN);
      addAction(navUp);
      addAction(navDown);
      addAction(navLeft);
      addAction(navRight);
      
      addAction(searchNext);
      addAction(searchAll);
      addAction(toggleHidden);
      addAction(hideFailed);
      addAction(unhideAll);
      addAction(zoomToFit);
      addAction(centerCN);
      addAction(exportPostscript);
      
      contextMenu = new QMenu(this);
      contextMenu->addAction(inspectCN);
      contextMenu->addAction(navUp);
      contextMenu->addAction(navDown);
      contextMenu->addAction(navLeft);
      contextMenu->addAction(navRight);
      
      contextMenu->addAction(searchNext);
      contextMenu->addAction(searchAll);      
      contextMenu->addAction(toggleHidden);
      contextMenu->addAction(hideFailed);
      contextMenu->addAction(unhideAll);
      contextMenu->addAction(zoomToFit);
      contextMenu->addAction(centerCN);
      contextMenu->addAction(exportPostscript);
    }
  
  TreeCanvasImpl::~TreeCanvasImpl(void) { delete root; }

  void
  TreeCanvasImpl::setInspector(Inspector* i) { inspect = i; }

  void
  TreeCanvasImpl::scaleTree(int scale0) {
    if (scale0<1)
      scale0 = 1;
    if (scale0>400)
      scale0 = 400;
    scale = ((double)scale0) / 100.0;
    BoundingBox bb = root->getBoundingBox();
    resize((int)((bb.right-bb.left+20)*scale), (int)((bb.depth+1)*38*scale));
    emit scaleChanged(scale0);
    QWidget::update();
  }

  void
  TreeCanvasImpl::update(void) {
    root->layout();
    BoundingBox bb = root->getBoundingBox();
    resize((int)((bb.right-bb.left+20)*scale), (int)((bb.depth+1)*38*scale));
    xtrans = -bb.left;
    QWidget::update();
  }

  void
  TreeCanvasImpl::searchAll(void) {
    dfsAll<VisualNode>(currentNode);
    currentNode->dirtyUp();
    update();
    centerCurrentNode();
  }

  void
  TreeCanvasImpl::searchOne(void) {
    dfsOne<VisualNode>(currentNode);
    currentNode->dirtyUp();
    update();
    centerCurrentNode();
  }

  void
  TreeCanvasImpl::toggleHidden(void) {
    currentNode->toggleHidden();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::hideFailed(void) {
    currentNode->hideFailed();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::unhideAll(void) {
    currentNode->unhideAll();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::exportPostscript(void) {
    Postscript::output(std::cout, root);
  }

  void
  TreeCanvasImpl::zoomToFit(void) {
    if (root != NULL) {
      BoundingBox bb = root->getBoundingBox();
      QWidget* p = parentWidget();
      if (p) {
        double newXScale =
          static_cast<double>(p->width()) / (bb.right - bb.left);
        double newYScale =
          static_cast<double>(p->height()) / (bb.depth * 38);

        scaleTree((int)(std::min(newXScale, newYScale)*100));
      }
    }
  }

  void
  TreeCanvasImpl::centerCurrentNode(void) {
    int x=0;
    int y=0;
    
    VisualNode* c = currentNode;
    while (c != NULL) {
      x += c->getOffset();
      y += 38;
      c = static_cast<VisualNode*>(c->getParent());
    }
    
    x = (int)((xtrans+x)*scale); y = (int)(y*scale);
    
    QScrollArea* sa = 
      static_cast<QScrollArea*>(parentWidget()->parentWidget());
    sa->ensureVisible(x, y);
    
  }

  void
  TreeCanvasImpl::inspectCurrentNode(void) {
    if (currentNode->isHidden()) {
      toggleHidden();
      return;
    }
    switch (currentNode->getStatus()) {
    case FAILED: break;
    case UNDETERMINED:
      {
        (void) currentNode->getNumberOfChildNodes();
        currentNode->dirtyUp();
        update();
      }
      break;
    default:
      if (inspect != NULL) {
        inspect->inspect(currentNode->getSpace());
      }
      break;
    }
    centerCurrentNode();
  }

  void
  TreeCanvasImpl::navUp(void) {
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      setCurrentNode(p);
      centerCurrentNode();
    }
  }

  void
  TreeCanvasImpl::navDown(void) {
    if (currentNode->getStatus() == BRANCH &&
        !currentNode->isHidden()) {
      setCurrentNode(currentNode->getChild(0));
      centerCurrentNode();
    }
  }

  void
  TreeCanvasImpl::navLeft(void) {
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt > 0) {
        setCurrentNode(p->getChild(alt-1));
        centerCurrentNode();
      }
    }
  }

  void
  TreeCanvasImpl::navRight(void) {
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt + 1 < p->getNumberOfChildNodes()) {
        setCurrentNode(p->getChild(alt+1));
        centerCurrentNode();
      }
    }
  }

  void
  TreeCanvasImpl::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    BoundingBox bb = root->getBoundingBox();
    
    painter.scale(scale,scale);
    painter.translate(xtrans, 0);
    QRect origClip = event->rect();
    QRect clip((int)(origClip.x()/scale-xtrans), (int)(origClip.y()/scale),
               (int)(origClip.width()/scale), (int)(origClip.height()/scale));
    DrawingCursor dc(root, painter, clip);
    PreorderNodeVisitor<DrawingCursor> v(dc);
    while (v.next());    
  }

  void
  TreeCanvasImpl::mouseDoubleClickEvent(QMouseEvent* /*event*/) {
    inspectCurrentNode();
  }
  
  void
  TreeCanvasImpl::contextMenuEvent(QContextMenuEvent* event) {
    VisualNode* n = root->findNode((int)(event->x()/scale-xtrans), (int)(event->y()/scale-38));
    if (n != NULL) {
      setCurrentNode(n);
      contextMenu->popup(event->globalPos());
      event->accept();
      return;
    }
    event->ignore();
  }
  
  void
  TreeCanvasImpl::setCurrentNode(VisualNode* n) {
    if (n != currentNode) {
      currentNode->setMarked(false);
      currentNode = n;
      currentNode->setMarked(true);
      QWidget::update();          
    }    
  }
  
  void
  TreeCanvasImpl::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
      VisualNode* n = root->findNode((int)(event->x()/scale-xtrans), (int)(event->y()/scale-38));
      if (n != NULL) {
        setCurrentNode(n);
        event->accept();
        return;
      }
    }
    event->ignore();
  }
  
  TreeCanvas::TreeCanvas(Space* root, Better* b,
                         QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout;    

    QScrollArea* scrollArea = new QScrollArea;
    canvas = new TreeCanvasImpl(root, b, this);

    scrollArea->setBackgroundRole(QPalette::Window);
    scrollArea->setWidget(canvas);

    QSlider* scaleBar = new QSlider(Qt::Vertical);
    scaleBar->setMinimum(1);
    scaleBar->setMaximum(400);
    scaleBar->setValue(100);
    
    connect(scaleBar, SIGNAL(valueChanged(int)), canvas, SLOT(scaleTree(int)));

    connect(canvas, SIGNAL(scaleChanged(int)), scaleBar, SLOT(setValue(int)));
    
    layout->addWidget(scrollArea, 0,0);
    layout->addWidget(scaleBar, 0,1);

    setLayout(layout);

    canvas->searchOne();
    canvas->show();

    resize(500, 400);

  }

  void
  TreeCanvas::setInspector(Inspector* i0) { canvas->setInspector(i0); }
  
  TreeCanvas::~TreeCanvas(void) { delete canvas; }
  
}}

// STATISTICS: gist-any
