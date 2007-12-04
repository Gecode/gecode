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

#include "gecode/gist/postscript.hh"
#include "gecode/gist/drawingcursor.hh"
#include "gecode/gist/analysiscursor.hh"

#include <QtGui/QPainter>
#include <stack>

#include "gecode/gist/addchild.hh"
#include <fstream>
namespace Gecode { namespace Gist {

  Inspector::~Inspector(void) {}
    
  TreeCanvasImpl::TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent)
    : QWidget(parent)
    , mutex(QMutex::Recursive)
    , inspect(NULL), heatView(false), autoHideFailed(true), autoZoom(false)
    , refresh(500) {
      QMutexLocker locker(&mutex);
      root = new VisualNode(rootSpace, b);
      root->setMarked(true);
      currentNode = root;
      pathHead = root;
      scale = 1.0;

      setBackgroundRole(QPalette::Base);
      connect(&searcher, SIGNAL(update()), this, SLOT(update()));
      connect(&searcher, SIGNAL(statusChanged(bool)), this, 
              SLOT(statusChanged(bool)));
      connect(&layouter, SIGNAL(done(int,int)),
              this, SLOT(layoutDone(int,int)));
  }
  
  TreeCanvasImpl::~TreeCanvasImpl(void) { delete root; }

  void
  TreeCanvasImpl::setInspector(Inspector* i) { inspect = i; }

  void
  TreeCanvasImpl::scaleTree(int scale0) {
    QMutexLocker locker(&mutex);
    BoundingBox bb;
    if (scale0<1)
      scale0 = 1;
    if (scale0>400)
      scale0 = 400;
    scale = ((double)scale0) / 100.0;
    bb = root->getBoundingBox();
    int w = (int)((bb.right-bb.left+20)*scale);
    int h = (int)((bb.depth+1)*38*scale);
    resize(w,h);
    emit scaleChanged(scale0);
    QWidget::update();
  }

  void
  TreeCanvasImpl::update(void) {
    QMutexLocker locker(&mutex);
    layouter.layout(this);
  }

  void
  TreeCanvasImpl::layoutDone(int w, int h) {
    resize(w,h);
    QWidget::update();
  }

  void
  TreeCanvasImpl::statusChanged(bool finished) {
    QMutexLocker locker(&mutex);
    if (autoHideFailed) {
      root->hideFailed();
      update();
    }
    if (autoZoom)
      zoomToFit();
    
    if (finished)
      centerCurrentNode();
    emit statusChanged(stats, finished);
  }

  void
  SearcherThread::search(VisualNode* n, bool all, TreeCanvasImpl* ti) {
    node = n;
    a = all;
    t = ti;
    start();
  }
    
  void
  SearcherThread::run() {
    {
      t->mutex.lock();
      emit statusChanged(false);
      std::stack<VisualNode*> stck;
      stck.push(node);

      VisualNode* sol = NULL;
      int nodeCount = 0;
      t->stopSearchFlag = false;
      while (!stck.empty() && !t->stopSearchFlag) {
        if (t->refresh > 0 && ++nodeCount > t->refresh) {
          node->dirtyUp();
          t->mutex.unlock();
          emit statusChanged(false);
          emit update();
          t->mutex.lock();
          nodeCount = 0;
        }
        VisualNode* n = stck.top(); stck.pop();
        if (n->isOpen()) {
          int kids = n->getNumberOfChildNodes(t->stats);
          if (!a && n->getStatus() == SOLVED) {
            sol = n;
            break;
          }
          for (int i=kids; i--;) {
            stck.push(static_cast<VisualNode*>(n->getChild(i)));
          }
        }
      }
      node->dirtyUp();
      if (sol != NULL) {
        t->setCurrentNode(sol);
      }
      t->stopSearchFlag = false;
      t->mutex.unlock();
    }
    emit update();
    emit statusChanged(true);
    t->centerCurrentNode();
  }

  void
  LayoutThread::layout(TreeCanvasImpl* ti) {
    t = ti;
    start();
  }

  void
  LayoutThread::run(void) {
    QMutexLocker locker(&t->mutex);
    t->root->layout();
    BoundingBox bb = t->root->getBoundingBox();
    
    int w = (int)((bb.right-bb.left+20)*t->scale);
    int h = (int)((bb.depth+1)*38*t->scale);
    t->xtrans = -bb.left+10;
    emit done(w,h);
  }

  void
  TreeCanvasImpl::searchAll(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, true, this);
  }

  void
  TreeCanvasImpl::searchOne(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, false, this);
  }

  void
  TreeCanvasImpl::toggleHidden(void) {
    QMutexLocker locker(&mutex);
    currentNode->toggleHidden();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::hideFailed(void) {
    QMutexLocker locker(&mutex);
    currentNode->hideFailed();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::unhideAll(void) {
    QMutexLocker locker(&mutex);
    currentNode->unhideAll();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::zoomToFit(void) {
    QMutexLocker locker(&mutex);
    BoundingBox bb;
    bb = root->getBoundingBox();
    if (root != NULL) {
      QWidget* p = parentWidget();
      if (p) {
        double newXScale =
          static_cast<double>(p->width()) / (bb.right - bb.left + 20);
        double newYScale =
          static_cast<double>(p->height()) / (bb.depth * 38 + 40);

        scaleTree((int)(std::min(newXScale, newYScale)*100));
      }
    }
  }

  void
  TreeCanvasImpl::centerCurrentNode(void) {
    QMutexLocker locker(&mutex);
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
  TreeCanvasImpl::analyzeTree(void) {
    QMutexLocker locker(&mutex);
    int min, max;
    AnalysisCursor ac(root, min, max);
    PostorderNodeVisitor<AnalysisCursor> va(ac);
    while (va.next());
    DistributeCursor dc(root, min, max);
    PreorderNodeVisitor<DistributeCursor> vd(dc);
    while (vd.next());
    heatView = true;
    QWidget::update();    
  }

  void
  TreeCanvasImpl::toggleHeatView(void) {
    heatView = !heatView;
    QWidget::update();
  }

  void
  TreeCanvasImpl::inspectCurrentNode(void) {
    QMutexLocker locker(&mutex);
    if (currentNode->isHidden()) {
      toggleHidden();
      return;
    }
    switch (currentNode->getStatus()) {
    case UNDETERMINED:
        {
          (void) currentNode->getNumberOfChildNodes();
        }
        break;
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
    update();
    centerCurrentNode();
  }

  void
  TreeCanvasImpl::stopSearch(void) {
    stopSearchFlag = true;
  }

  void
  TreeCanvasImpl::reset(void) {
    QMutexLocker locker(&mutex);
    Space* rootSpace = root->getSpace();
    Better* b = root->getBetterWrapper();
    delete root;
    root = new VisualNode(rootSpace, b);
    root->setMarked(true);
    currentNode = root;
    pathHead = root;
    nodeMap.clear();
    scale = 1.0;
    stats = Statistics();
    root->layout();
    update();
  }

  void
  TreeCanvasImpl::setPath(void) {
    QMutexLocker locker(&mutex);
    if(currentNode == pathHead)
      return;

    pathHead->unPathUp();
    pathHead = currentNode;

    currentNode->setPathInfos(true, -1, true);
    currentNode->pathUp();
    currentNode->dirtyUp();
    update();
  }

  void
  TreeCanvasImpl::inspectPath(void) {
    QMutexLocker locker(&mutex);
    setPath();
    setCurrentNode(root);
    if(inspect != NULL)
      while(true) {
        inspectCurrentNode();
        if(!currentNode->isLastOnPath())
          setCurrentNode(currentNode->getChild(currentNode->getPathAlternative()));
        else
          break;
      }
    update();
  }

  void
  TreeCanvasImpl::navUp(void) {
    QMutexLocker locker(&mutex);
    
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    setCurrentNode(p);

    if (p != NULL) {
      centerCurrentNode();
    }
  }

  void
  TreeCanvasImpl::navDown(void) {
    QMutexLocker locker(&mutex);
    if (!currentNode->isHidden()) {
      switch (currentNode->getStatus()) {
      case SPECIAL:
        if (currentNode->getNumberOfChildren() < 1)
          break;
      case BRANCH: 
          {
            VisualNode* n = currentNode->getChild(0);
            setCurrentNode(n);
            centerCurrentNode();
            break;
          }
      default:
        break;
      }
    }
  }

  void
  TreeCanvasImpl::navLeft(void) {
    QMutexLocker locker(&mutex);
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt > 0) {
        VisualNode* n = p->getChild(alt-1);
        setCurrentNode(n);
        centerCurrentNode();
      }
    }
  }

  void
  TreeCanvasImpl::navRight(void) {
    QMutexLocker locker(&mutex);
    VisualNode* p = static_cast<VisualNode*>(currentNode->getParent());
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt + 1 < p->getNumberOfChildNodes()) {
        VisualNode* n = p->getChild(alt+1);
        setCurrentNode(n);
        centerCurrentNode();
      }
    }
  }
  
  void
  TreeCanvasImpl::navRoot(void) {
    QMutexLocker locker(&mutex);
    setCurrentNode(root);
    centerCurrentNode();
  }
    
  void
  TreeCanvasImpl::markCurrentNode(int i) {
    QMutexLocker locker(&mutex);
    if(nodeMap.size() > i && nodeMap[i] != NULL) {
      setCurrentNode(nodeMap[i]);
      centerCurrentNode();
    }
  }

  void
  TreeCanvasImpl::saveCurrentNode(void) {
    QMutexLocker locker(&mutex);
    nodeMap << currentNode;
    emit newPointInTime(nodeMap.size() - 1);
  }

  void
  TreeCanvasImpl::exportPostscript(void) {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save tree as postscript"), "", tr("Postscript (*.ps *.eps)"));
    if (filename != "") {
      std::ofstream outfile(filename.toStdString().c_str());
      QMutexLocker locker(&mutex);
      Postscript::output(outfile, root);      
      outfile.close();
    }
  }

  void
  TreeCanvasImpl::print(void) {
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
      QMutexLocker locker(&mutex);

      BoundingBox bb = root->getBoundingBox();
      QRect pageRect = printer.pageRect();
      double newXScale =
        static_cast<double>(pageRect.width()) / (bb.right - bb.left + 20);
      double newYScale =
        static_cast<double>(pageRect.height()) / (bb.depth * 38 + 40);
      double printScale = std::min(newXScale, newYScale)*100;
      if (printScale<1.0)
        printScale = 1.0;
      if (printScale > 400.0)
        printScale = 400.0;
      printScale = printScale / 100.0;

      QPainter painter(&printer);
      painter.setRenderHint(QPainter::Antialiasing);
      painter.scale(printScale,printScale);
      painter.translate(xtrans, 0);
      QRect clip(0,0,0,0);
      DrawingCursor dc(root, painter, heatView, clip);
      PreorderNodeVisitor<DrawingCursor> v(dc);
      while (v.next());      
    }
  }
  
  void
  TreeCanvasImpl::paintEvent(QPaintEvent* event) {
    QMutexLocker locker(&mutex);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    BoundingBox bb = root->getBoundingBox();
    
    QRect origClip = event->rect();
    if (heatView) {
      painter.setBrush(Qt::black);
      painter.drawRect(origClip.x(),origClip.y(),
                       origClip.width(), origClip.height());
    }
    painter.scale(scale,scale);
    painter.translate(xtrans, 0);
    QRect clip((int)(origClip.x()/scale-xtrans), (int)(origClip.y()/scale),
               (int)(origClip.width()/scale), (int)(origClip.height()/scale));
    DrawingCursor dc(root, painter, heatView, clip);
    PreorderNodeVisitor<DrawingCursor> v(dc);
    while (v.next());    
  }

  void
  TreeCanvasImpl::mouseDoubleClickEvent(QMouseEvent* /*event*/) {
    inspectCurrentNode();
  }
  
  void
  TreeCanvasImpl::contextMenuEvent(QContextMenuEvent* event) {
    QMutexLocker locker(&mutex);
    VisualNode* n;
      n = root->findNode((int)(event->x()/scale-xtrans), 
                         (int)(event->y()/scale-38));
      setCurrentNode(n);
    if (n != NULL) {
      emit contextMenu(event);
      event->accept();
      return;
    }
    event->ignore();
  }

  void
  TreeCanvasImpl::finish(void) {
    stopSearchFlag = true;
    searcher.wait();    
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
    QMutexLocker locker(&mutex);
    if (event->button() == Qt::LeftButton) {
      VisualNode* n;
        n = root->findNode((int)(event->x()/scale-xtrans), 
                           (int)(event->y()/scale-38));
        setCurrentNode(n);
      if (n != NULL) {
        event->accept();
        return;
      }
    }
    event->ignore();
  }
  
  void
  TreeCanvasImpl::setAutoHideFailed(bool b) {
    autoHideFailed = b;
  }
  
  void
  TreeCanvasImpl::setAutoZoom(bool b) {
    autoZoom = b;
  }

  bool
  TreeCanvasImpl::getAutoHideFailed(void) {
    return autoHideFailed;
  }
  
  bool
  TreeCanvasImpl::getAutoZoom(void) {
    return autoZoom;
  }

  void
  TreeCanvasImpl::setRefresh(int i) {
    refresh = i;
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

  void
  TreeCanvasImpl::addChild(void) {
    QMutexLocker locker(&mutex);
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
            update();
          }
          
          delete space;
        }
    }
  }

  void
  TreeCanvasImpl::addFixpoint(void) {
    QMutexLocker locker(&mutex);
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
          update();
          return;
        }
    }
  }

  void
  TreeCanvasImpl::expandCurrentNode(void) {
    QMutexLocker locker(&mutex);

    switch (currentNode->getStatus()) {
    case UNDETERMINED:
    case FAILED:
    case SPECIAL:
      break;
    case BRANCH:
    case SOLVED:
    default:
      if(currentNode != root) {
        Space* inputSpace = currentNode->getInputSpace();
        int alt = currentNode->getAlternative();
        NodeStatus status = currentNode->getStatus();
        
        VisualNode* curNode = currentNode;
        VisualNode* parent = static_cast<VisualNode*>(currentNode->getParent());
        
        setCurrentNode(parent);
        
        VisualNode* newChild = static_cast<VisualNode*>(currentNode->createChild(currentNode->getNumberOfChildren()));

        newChild->setStatus(SPECIAL);
        newChild->setSpecialDesc(new SpecialDesc(alt));
        newChild->setMetaStatus(status);
        newChild->setFirstStepNode(true);
        newChild->setNumberOfChildren(0);
        newChild->setNoOfOpenChildren(0);

        currentNode->setChild(alt, newChild);

        newChild->setDirty(false);
        newChild->dirtyUp();
        setCurrentNode(newChild);

        while(inputSpace->step() != ES_STABLE) {

          VisualNode* newChild = static_cast<VisualNode*>(currentNode->createChild(currentNode->getNumberOfChildren()));

          newChild->setStatus(SPECIAL);
          newChild->setSpecialDesc(new SpecialDesc());
          newChild->setMetaStatus(status);
          newChild->setNumberOfChildren(0);
          newChild->setNoOfOpenChildren(0);

          currentNode->addChild(newChild);

          newChild->setDirty(false);
          newChild->dirtyUp();
          setCurrentNode(newChild);
        }
        
        currentNode->setLastStepNode(true);
        currentNode->addChild(curNode);
        if(curNode->isOpen())
          currentNode->openUp();
        setCurrentNode(curNode);
      }
      break;
    }
    update();
    centerCurrentNode();
  }

#endif

  TreeCanvas::TreeCanvas(Space* root, Better* b,
                         QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout(this);    

    QScrollArea* scrollArea = new QScrollArea(this);
    canvas = new TreeCanvasImpl(root, b, this);
    canvas->setObjectName("canvas");
    
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setAlignment(Qt::AlignHCenter);
    scrollArea->setBackgroundRole(QPalette::Base);
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

    inspectCN = new QAction("Inspect", this);
    inspectCN->setShortcut(QKeySequence("Return"));
    connect(inspectCN, SIGNAL(triggered()), canvas, 
                       SLOT(inspectCurrentNode()));

    stopCN = new QAction("Stop search", this);
    stopCN->setShortcut(QKeySequence("Esc"));
    connect(stopCN, SIGNAL(triggered()), canvas, 
                    SLOT(stopSearch()));

    reset = new QAction("Reset", this);
    reset->setShortcut(QKeySequence("Ctrl+R"));
    connect(reset, SIGNAL(triggered()), canvas, 
            SLOT(reset()));

    navUp = new QAction("Up", this);
    navUp->setShortcut(QKeySequence("Up"));
    connect(navUp, SIGNAL(triggered()), canvas, 
                   SLOT(navUp()));

    navDown = new QAction("Down", this);
    navDown->setShortcut(QKeySequence("Down"));
    connect(navDown, SIGNAL(triggered()), canvas, 
                     SLOT(navDown()));

    navLeft = new QAction("Left", this);
    navLeft->setShortcut(QKeySequence("Left"));
    connect(navLeft, SIGNAL(triggered()), canvas, 
                     SLOT(navLeft()));

    navRight = new QAction("Right", this);
    navRight->setShortcut(QKeySequence("Right"));
    connect(navRight, SIGNAL(triggered()), canvas, 
                      SLOT(navRight()));

    navRoot = new QAction("Root", this);
    navRoot->setShortcut(QKeySequence("R"));
    connect(navRoot, SIGNAL(triggered()), canvas, 
                      SLOT(navRoot()));

    searchNext = new QAction("Next solution", this);
    searchNext->setShortcut(QKeySequence("N"));
    connect(searchNext, SIGNAL(triggered()), canvas, SLOT(searchOne()));

    searchAll = new QAction("All solutions", this);
    searchAll->setShortcut(QKeySequence("A"));
    connect(searchAll, SIGNAL(triggered()), canvas, SLOT(searchAll()));

    toggleHidden = new QAction("Hide/unhide", this);
    toggleHidden->setShortcut(QKeySequence("H"));
    connect(toggleHidden, SIGNAL(triggered()), canvas, SLOT(toggleHidden()));

    hideFailed = new QAction("Hide failed subtrees", this);
    hideFailed->setShortcut(QKeySequence("F"));
    connect(hideFailed, SIGNAL(triggered()), canvas, SLOT(hideFailed()));

    unhideAll = new QAction("Unhide all", this);
    unhideAll->setShortcut(QKeySequence("U"));
    connect(unhideAll, SIGNAL(triggered()), canvas, SLOT(unhideAll()));

    zoomToFit = new QAction("Zoom to fit", this);
    zoomToFit->setShortcut(QKeySequence("Z"));
    connect(zoomToFit, SIGNAL(triggered()), canvas, SLOT(zoomToFit()));

    centerCN = new QAction("Center current node", this);
    centerCN->setShortcut(QKeySequence("C"));
    connect(centerCN, SIGNAL(triggered()), canvas, SLOT(centerCurrentNode()));

    exportPostscript = new QAction("Export postscript...", this);
    exportPostscript->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(exportPostscript, SIGNAL(triggered()), canvas, 
            SLOT(exportPostscript()));

    print = new QAction("Print...", this);
    print->setShortcut(QKeySequence("Ctrl+P"));
    connect(print, SIGNAL(triggered()), canvas, 
            SLOT(print()));

    setPath = new QAction("Set path", this);
    setPath->setShortcut(QKeySequence("Shift+P"));
    connect(setPath, SIGNAL(triggered()), canvas, SLOT(setPath()));

    inspectPath = new QAction("Inspect path", this);
    inspectPath->setShortcut(QKeySequence("Shift+I"));
    connect(inspectPath, SIGNAL(triggered()), canvas, SLOT(inspectPath()));

    toggleHeatView = new QAction("Toggle heat view", this);
    toggleHeatView->setShortcut(QKeySequence("Y"));
    connect(toggleHeatView, SIGNAL(triggered()), canvas, 
            SLOT(toggleHeatView()));

    analyzeTree = new QAction("Analyze tree", this);
    analyzeTree->setShortcut(QKeySequence("Shift+Y"));
    connect(analyzeTree, SIGNAL(triggered()), canvas, 
            SLOT(analyzeTree()));

#ifdef GECODE_GIST_EXPERIMENTAL

    addChild = new QAction("Add child node", this);
    addChild->setShortcut(QKeySequence("Shift+C"));
    connect(addChild, SIGNAL(triggered()), canvas, SLOT(addChild()));

    addFixpoint = new QAction("Add fixpoint node", this);
    addFixpoint->setShortcut(QKeySequence("Shift+F"));
    connect(addFixpoint, SIGNAL(triggered()), canvas, SLOT(addFixpoint()));

    expandCurrentNode = new QAction("Expand/Collapse", this);
    expandCurrentNode->setShortcut(QKeySequence("Shift+E"));
    connect(expandCurrentNode, SIGNAL(triggered()), canvas, SLOT(expandCurrentNode()));

#endif

    addAction(inspectCN);
    addAction(stopCN);
    addAction(reset);
    addAction(navUp);
    addAction(navDown);
    addAction(navLeft);
    addAction(navRight);
    addAction(navRoot);

    addAction(searchNext);
    addAction(searchAll);
    addAction(toggleHidden);
    addAction(hideFailed);
    addAction(unhideAll);
    addAction(zoomToFit);
    addAction(centerCN);
    addAction(exportPostscript);
    addAction(print);

    addAction(toggleHeatView);
    addAction(analyzeTree);
    
    addAction(setPath);
    addAction(inspectPath);

#ifdef GECODE_GIST_EXPERIMENTAL

    addAction(addChild);
    addAction(addFixpoint);
    addAction(expandCurrentNode);

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
    contextMenu->addAction(expandCurrentNode);

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
  
  void
  TreeCanvas::on_canvas_contextMenu(QContextMenuEvent* event) {
    contextMenu->popup(event->globalPos());    
  }

  void
  TreeCanvas::on_canvas_statusChanged(const Statistics& stats,
                                      bool finished) {
    emit statusChanged(stats,finished);
  }

  void
  TreeCanvas::finish(void) {
    canvas->finish();
  }

  void
  TreeCanvas::closeEvent(QCloseEvent* event) {
    canvas->finish();
    event->accept();
  }

  void
  TreeCanvas::setAutoHideFailed(bool b) { canvas->setAutoHideFailed(b); }
  void
  TreeCanvas::setAutoZoom(bool b) { canvas->setAutoZoom(b); }
  bool
  TreeCanvas::getAutoHideFailed(void) { return canvas->getAutoHideFailed(); }
  bool
  TreeCanvas::getAutoZoom(void) { return canvas->getAutoZoom(); }
  void
  TreeCanvas::setRefresh(int i) { canvas->setRefresh(i); }
  
#ifdef GECODE_GIST_EXPERIMENTAL

  void
  TreeCanvas::on_canvas_newPointInTime(int i) {

    timeBar->setMaximum(i);
    timeBar->setValue(i);

  }

#endif
    
}}

// STATISTICS: gist-any

