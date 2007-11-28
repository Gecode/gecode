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

#include <QtGui/QPainter>

#include "gecode/gist/addchild.hh"

namespace Gecode { namespace Gist {

  Inspector::~Inspector(void) {}
    
  TreeCanvasImpl::TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent)
    : QWidget(parent), inspect(NULL) {
      QMutexLocker locker(&mutex);
      root = new VisualNode(rootSpace, b);
      root->setMarked(true);
      currentNode = root;
      pathHead = root;
      scale = 1.0;
      
      QAction* inspectCN = new QAction("inspect", this);
      inspectCN->setShortcut(QKeySequence("Return"));
      connect(inspectCN, SIGNAL(triggered()), this, 
                         SLOT(inspectCurrentNode()));

      QAction* stopCN = new QAction("stop", this);
      stopCN->setShortcut(QKeySequence("Esc"));
      connect(stopCN, SIGNAL(triggered()), this, 
                      SLOT(stopSearch()));

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
      exportPostscript->setShortcut(QKeySequence("Ctrl+P"));
      connect(exportPostscript, SIGNAL(triggered()), this, SLOT(exportPostscript()));
      
      QAction* setPath = new QAction("set Path", this);
      setPath->setShortcut(QKeySequence("Shift+P"));
      connect(setPath, SIGNAL(triggered()), this, SLOT(setPath()));

      QAction* inspectPath = new QAction("inspect Path", this);
      inspectPath->setShortcut(QKeySequence("Shift+I"));
      connect(inspectPath, SIGNAL(triggered()), this, SLOT(inspectPath()));

#ifdef GECODE_GIST_EXPERIMENTAL

      QAction* addChild = new QAction("add child node", this);
      addChild->setShortcut(QKeySequence("Shift+C"));
      connect(addChild, SIGNAL(triggered()), this, SLOT(addChild()));
      
      QAction* addFixpoint = new QAction("add fixpoint node", this);
      addFixpoint->setShortcut(QKeySequence("Shift+F"));
      connect(addFixpoint, SIGNAL(triggered()), this, SLOT(addFixpoint()));

#endif

      addAction(inspectCN);
      addAction(stopCN);
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
      
      addAction(setPath);
      addAction(inspectPath);

#ifdef GECODE_GIST_EXPERIMENTAL

      addAction(addChild);
      addAction(addFixpoint);

#endif

      contextMenu = new QMenu(this);
      contextMenu->addAction(inspectCN);

      contextMenu->addSeparator();
      
      contextMenu->addAction(navUp);
      contextMenu->addAction(navDown);
      contextMenu->addAction(navLeft);
      contextMenu->addAction(navRight);
      
      contextMenu->addAction(searchNext);
      contextMenu->addAction(searchAll);      

      contextMenu->addSeparator();
      
      contextMenu->addAction(toggleHidden);
      contextMenu->addAction(hideFailed);
      contextMenu->addAction(unhideAll);

      contextMenu->addSeparator();
      
      contextMenu->addAction(zoomToFit);
      contextMenu->addAction(centerCN);

      contextMenu->addSeparator();
      
      contextMenu->addAction(exportPostscript);
      
      contextMenu->addSeparator();
      
      contextMenu->addAction(setPath);
      contextMenu->addAction(inspectPath);

#ifdef GECODE_GIST_EXPERIMENTAL

      contextMenu->addAction(addChild);
      contextMenu->addAction(addFixpoint);

#endif

  }
  
  TreeCanvasImpl::~TreeCanvasImpl(void) { delete root; }

  void
  TreeCanvasImpl::setInspector(Inspector* i) { inspect = i; }

  void
  TreeCanvasImpl::scaleTree(int scale0) {
    BoundingBox bb;
    {
      QMutexLocker locker(&mutex);
      if (scale0<1)
        scale0 = 1;
      if (scale0>400)
        scale0 = 400;
      scale = ((double)scale0) / 100.0;
      bb = root->getBoundingBox();
    }
    resize((int)((bb.right-bb.left+20)*scale), (int)((bb.depth+1)*38*scale));
    emit scaleChanged(scale0);
    QWidget::update();
  }

  void
  TreeCanvasImpl::update(void) {
    BoundingBox bb;
    {
      QMutexLocker locker(&mutex);
      root->layout();
      bb = root->getBoundingBox();
    }
    resize((int)((bb.right-bb.left+20)*scale), (int)((bb.depth+1)*38*scale));
    xtrans = -bb.left;
    QWidget::update();
  }

  void
  Searcher::search(VisualNode* n, bool all, QMutex* m, TreeCanvasImpl* ti) {
    node = n;
    a = all;
    mutex = m;
    t = ti;
    start();
  }
    
  void
  Searcher::run() {
    {
      VisualNode* sol = NULL;
      QMutexLocker lock(mutex);
      t->stopSearchFlag = false;
      if (a) {
        dfsAll<VisualNode>(node, &t->stopSearchFlag);
      } else {
        sol = dfsOne<VisualNode>(node, &t->stopSearchFlag);
      }
      node->dirtyUp();
      if (sol != NULL) {
        t->setCurrentNode(sol);
      }
    }
    t->update();
    t->centerCurrentNode();
  }

  void
  TreeCanvasImpl::searchAll(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, true, &mutex, this);
  }

  void
  TreeCanvasImpl::searchOne(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, false, &mutex, this);
  }

  void
  TreeCanvasImpl::toggleHidden(void) {
    {
      QMutexLocker locker(&mutex);
      currentNode->toggleHidden();
    }
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::hideFailed(void) {
    {
      QMutexLocker locker(&mutex);
      currentNode->hideFailed();
    }
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::unhideAll(void) {
    {
      QMutexLocker locker(&mutex);
      currentNode->unhideAll();
    }
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::exportPostscript(void) {
    QMutexLocker locker(&mutex);
    Postscript::output(std::cout, root);
  }

  void
  TreeCanvasImpl::zoomToFit(void) {
    BoundingBox bb;
    {
      QMutexLocker locker(&mutex);
      bb = root->getBoundingBox();      
    }
    if (root != NULL) {
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

    {
      QMutexLocker locker(&mutex);
      VisualNode* c = currentNode;
      while (c != NULL) {
        x += c->getOffset();
        y += 38;
        c = static_cast<VisualNode*>(c->getParent());
      }
    }
    
    x = (int)((xtrans+x)*scale); y = (int)(y*scale);
    
    QScrollArea* sa = 
      static_cast<QScrollArea*>(parentWidget()->parentWidget());
    sa->ensureVisible(x, y);
    
  }

#ifdef GECODE_GIST_EXPERIMENTAL

  void
  TreeCanvasImpl::addChild(void) {
    mutex.lock();
    //	  special child nodes may only be added to other special nodes
    switch (currentNode->getStatus()) {
    case FAILED:
    case SOLVED:
    case UNDETERMINED:
      break;
    case BRANCH:
    case SPECIAL:
        {
          Reflection::VarMap vm;
          Space* space = currentNode->getSpace();
          space->getVars(vm);
          delete space;

          AddChild dialog(vm, this);

          if(dialog.exec()) {

            std::string var = dialog.var().toStdString();
            int value = dialog.value();
            IntRelType rel = static_cast<IntRelType> (dialog.rel());

            VisualNode* newChild = static_cast<VisualNode*>(currentNode->createChild(currentNode->getNumberOfChildren()));

            newChild->setStatus(SPECIAL);
            newChild->setSpecialDesc(new SpecialDesc(var, rel, value));
            newChild->setNumberOfChildren(0);
            newChild->setNoOfOpenChildren(0);

            currentNode->addChild(newChild);

            newChild->setDirty(false);
            newChild->dirtyUp();
            setCurrentNode(newChild);
            mutex.unlock();
            update();
            return;
          }
        }
    }
    mutex.unlock();
  }

  void
  TreeCanvasImpl::addFixpoint(void) {
    mutex.lock();
    //	  new fixpoints may only be added to special nodes
    switch (currentNode->getStatus()) {
    case FAILED:
    case SOLVED:
    case UNDETERMINED:
    case BRANCH:
      break;
    case SPECIAL:
        {
          VisualNode* newChild = static_cast<VisualNode*>(currentNode->createChild(currentNode->getNumberOfChildren()));

          newChild->setStatus(UNDETERMINED);

          currentNode->addChild(newChild);
          currentNode->openUp();

          newChild->setDirty(false);
          newChild->dirtyUp();
          mutex.unlock();
          update();
          return;
        }
    }
  }

#endif

  void
  TreeCanvasImpl::_inspectCurrentNode(void) {
    if (currentNode->isHidden()) {
      toggleHidden();
      return;
    }
    switch (currentNode->getStatus()) {
    case UNDETERMINED:
      {
        (void) currentNode->getNumberOfChildNodes();
      }
    case FAILED:
    case SPECIAL:
    case BRANCH:
    case SOLVED:
    default:
      if (inspect != NULL) {
        inspect->inspect(currentNode->getSpace());
      }
      break;
    }
    currentNode->dirtyUp();
    saveCurrentNode();
  }

  void
  TreeCanvasImpl::inspectCurrentNode(void) {
    {
      QMutexLocker locker(&mutex);
      _inspectCurrentNode();
    }
    centerCurrentNode();
    update();
  }

  void
  TreeCanvasImpl::stopSearch(void) {
    stopSearchFlag = true;
  }

  void
  TreeCanvasImpl::_setPath(void) {
    if(currentNode == pathHead)
      return;

    pathHead->unPathUp();
    pathHead = currentNode;

    currentNode->setPathInfos(true, -1, true);
    currentNode->pathUp();
    currentNode->dirtyUp();
  }

  void
  TreeCanvasImpl::setPath(void) {
    {
      QMutexLocker locker(&mutex);
      _setPath();
    }
    update();
  }

  void
  TreeCanvasImpl::inspectPath(void) {
    {
      QMutexLocker locker(&mutex);
      _setPath();
      setCurrentNode(root);
      if(inspect != NULL)
        while(true) {
          _inspectCurrentNode();
          if(!currentNode->isLastOnPath())
            setCurrentNode(
              currentNode->getChild(currentNode->getPathAlternative()));
          else
            break;
        }
    }
    update();
  }

  void
  TreeCanvasImpl::navUp(void) {
    VisualNode* p;
    {
      QMutexLocker locker(&mutex);
      p = static_cast<VisualNode*>(currentNode->getParent());
      setCurrentNode(p);
    }
    if (p != NULL) {
      centerCurrentNode();
    }
  }

  void
  TreeCanvasImpl::navDown(void) {
    mutex.lock();
    if (!currentNode->isHidden()) {
      switch (currentNode->getStatus()) {
      case SPECIAL:
        if (currentNode->getNumberOfChildren() < 1)
          break;
      case BRANCH:
        VisualNode* n = currentNode->getChild(0);
        setCurrentNode(n);
        mutex.unlock();
        centerCurrentNode();
        break;
      default:
        mutex.unlock();
        break;
      }
    } else {
      mutex.unlock();
    }
  }

  void
  TreeCanvasImpl::navLeft(void) {
    mutex.lock();
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt > 0) {
        VisualNode* n = p->getChild(alt-1);
        setCurrentNode(n);
        mutex.unlock();
        centerCurrentNode();
        return;
      }
    }
    mutex.unlock();
  }

  void
  TreeCanvasImpl::navRight(void) {
    mutex.lock();
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt + 1 < p->getNumberOfChildNodes()) {
        VisualNode* n = p->getChild(alt+1);
        setCurrentNode(n);
        mutex.unlock();
        centerCurrentNode();
        return;
      }
    }
    mutex.unlock();
  }
  
  void
  TreeCanvasImpl::markCurrentNode(int i) {
    mutex.lock();
    if(nodeMap.size() > i && nodeMap[i] != NULL) {
      setCurrentNode(nodeMap[i]);
      mutex.unlock();
      centerCurrentNode();
      return;
    }
    mutex.unlock();
  }

  void
  TreeCanvasImpl::saveCurrentNode(void) {
    nodeMap << currentNode;
    emit newPointInTime(nodeMap.size() - 1);
  }
  
#ifdef GECODE_GIST_EXPERIMENTAL

  void
  TreeCanvasImpl::getRootVars(Gecode::Reflection::VarMap& vm,
                              int& nextPointInTime) {
    QMutexLocker locker(&mutex);
    if(root != NULL) {
      Space* space = root->getSpace();
      space->getVars(vm);
      delete space;
      nextPointInTime = nodeMap.size();
    }
  }

#endif

  void
  TreeCanvasImpl::paintEvent(QPaintEvent* event) {
    QMutexLocker locker(&mutex);
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
    VisualNode* n;
    {
      QMutexLocker locker(&mutex);
      n = root->findNode((int)(event->x()/scale-xtrans), 
                         (int)(event->y()/scale-38));
      setCurrentNode(n);
    }
    if (n != NULL) {
      contextMenu->popup(event->globalPos());
      event->accept();
      return;
    }
    event->ignore();
  }
  
  void
  TreeCanvasImpl::setCurrentNode(VisualNode* n) {
    if (n != NULL && n != currentNode) {
      currentNode->setMarked(false);
      currentNode = n;
      currentNode->setMarked(true);
      QWidget::update();
    }
  }
  
  void
  TreeCanvasImpl::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
      VisualNode* n;
      {
        QMutexLocker locker(&mutex);
        n = root->findNode((int)(event->x()/scale-xtrans), 
                           (int)(event->y()/scale-38));
        setCurrentNode(n);
      }
      if (n != NULL) {
        event->accept();
        return;
      }
    }
    event->ignore();
  }
  
  TreeCanvas::TreeCanvas(Space* root, Better* b,
                         QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout(this);    

    QScrollArea* scrollArea = new QScrollArea(this);
    canvas = new TreeCanvasImpl(root, b, this);
    canvas->setObjectName("canvas");
    
    scrollArea->setBackgroundRole(QPalette::Window);
    scrollArea->setWidget(canvas);

    QSlider* scaleBar = new QSlider(Qt::Vertical, this);
    scaleBar->setObjectName("scaleBar");
    scaleBar->setMinimum(1);
    scaleBar->setMaximum(400);
    scaleBar->setValue(100);
    
#ifdef GECODE_GIST_EXPERIMENTAL

    timeBar = new QSlider(Qt::Horizontal, this);
    timeBar->setObjectName("timeBar");
    timeBar->setMinimum(0);
    timeBar->setMaximum(0);
    timeBar->setValue(0);
    
#endif
    
    connect(scaleBar, SIGNAL(valueChanged(int)), canvas, SLOT(scaleTree(int)));

    connect(canvas, SIGNAL(scaleChanged(int)), scaleBar, SLOT(setValue(int)));
    
#ifdef GECODE_GIST_EXPERIMENTAL

    connect(timeBar, SIGNAL(valueChanged(int)), canvas, SLOT(markCurrentNode(int)));
    
#endif
    
    layout->addWidget(scrollArea, 0,0);
    layout->addWidget(scaleBar, 0,1);
    
#ifdef GECODE_GIST_EXPERIMENTAL

    layout->addWidget(timeBar, 1,0);

#endif
    
    setLayout(layout);

    canvas->searchOne();
    canvas->show();

    resize(500, 400);

    // enables on_<sender>_<signal>() mechanism
    QMetaObject::connectSlotsByName(this);
  }

  void
  TreeCanvas::setInspector(Inspector* i0) { canvas->setInspector(i0); }
  
  TreeCanvas::~TreeCanvas(void) { delete canvas; }
  
#ifdef GECODE_GIST_EXPERIMENTAL

  void
  TreeCanvas::on_canvas_newPointInTime(int i) {

    timeBar->setMaximum(i);
    timeBar->setValue(i);

  }

#endif
    
}}

// STATISTICS: gist-any
