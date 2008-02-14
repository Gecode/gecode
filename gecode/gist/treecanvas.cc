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

#include <QtGui/QPainter>

#include <stack>
#include <fstream>

#include "gecode/gist/treecanvas.hh"

#include "gecode/gist/nodevisitor.hh"
#include "gecode/gist/shapelist.hh"
#include "gecode/gist/visualnode.hh"
#include "gecode/gist/postscript.hh"
#include "gecode/gist/drawingcursor.hh"
#include "gecode/gist/analysiscursor.hh"
#include "gecode/gist/addchild.hh"
#include "gecode/gist/addvisualisationdialog.hh"

#ifdef GECODE_GIST_EXPERIMENTAL
#ifdef GECODE_HAS_INT_VARS
#include "gecode/int.hh"
#endif
#ifdef GECODE_HAS_SET_VARS
#include "gecode/set.hh"
#endif
#endif

namespace Gecode { namespace Gist {

  Inspector::~Inspector(void) {}
    
  TreeCanvasImpl::TreeCanvasImpl(Space* rootSpace, Better* b,
                                 QWidget* parent)
    : QWidget(parent)
    , mutex(QMutex::Recursive)
    , layoutMutex(QMutex::Recursive)
    , inspector(NULL), heatView(false)
    , autoHideFailed(true), autoZoom(false)
    , refresh(500), nextPit(0) {
      QMutexLocker locker(&mutex);
      root = new VisualNode(rootSpace, b);
      root->setMarked(true);
      currentNode = root;
      pathHead = root;
      scale = 1.0;

      setAutoFillBackground(true);

      connect(&searcher, SIGNAL(update()), this, SLOT(update()));
      connect(&searcher, SIGNAL(statusChanged(bool)), this, 
              SLOT(statusChanged(bool)));
      connect(&layouter, SIGNAL(done(int,int)),
              this, SLOT(layoutDone(int,int)));
      
      qRegisterMetaType<Gecode::Gist::NodeStatus>("Gecode::Gist::NodeStatus");
  }
  
  TreeCanvasImpl::~TreeCanvasImpl(void) { delete root; }

  void
  TreeCanvasImpl::setInspector(Inspector* i) { inspector = i; }

  void
  TreeCanvasImpl::scaleTree(int scale0) {
    QMutexLocker locker(&mutex);
    BoundingBox bb;
    if (scale0<1)
      scale0 = 1;
    if (scale0>400)
      scale0 = 400;
    scale = (static_cast<double>(scale0)) / 100.0;
    bb = root->getBoundingBox();
    int w = static_cast<int>((bb.right-bb.left+20)*scale);
    int h = static_cast<int>(40+bb.depth*38*scale);
    if (heatView)
      w = std::max(w, 300);
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
    if (heatView)
      w = std::max(w, 300);
    resize(w,h);
    QWidget::update();
  }

  void
  TreeCanvasImpl::statusChanged(bool finished) {
    QMutexLocker locker(&mutex);
    QMutexLocker layoutLocker(&layoutMutex);
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
            stck.push(n->getChild(i));
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
    QMutexLocker layoutLocker(&t->layoutMutex);
    t->root->layout();
    BoundingBox bb = t->root->getBoundingBox();
    
    int w = static_cast<int>((bb.right-bb.left+20)*t->scale);
    int h = static_cast<int>(40+bb.depth*38*t->scale);
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
#ifdef GECODE_GIST_EXPERIMENTAL
    if(currentNode->isStepNode()) {
      VisualNode* curNode = currentNode;
      while(curNode->getParent()->isStepNode()) {
        curNode = curNode->getParent();
      }
      do {
        curNode->toggleHidden();
        curNode = curNode->getChild(0);
      } while(curNode->isStepNode());
      curNode->setCollapsed(!curNode->isCollapsed());
      setCurrentNode(curNode);
      update();
      centerCurrentNode();
      return;
    }
#endif
    currentNode->toggleHidden();
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::hideFailed(void) {
    QMutexLocker locker(&mutex);
#ifdef GECODE_GIST_EXPERIMENTAL
    if(currentNode->isStepNode()) {
    // TODO nikopp: ugly things happen in this case that I do not fully understand
      return;
    }
#endif
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
    if (root != NULL) {
      BoundingBox bb;
      bb = root->getBoundingBox();
      QWidget* p = parentWidget();
      if (p) {
        double newXScale =
          static_cast<double>(p->width()) / (bb.right - bb.left + 20);
        double newYScale =
          static_cast<double>(p->height()) / (bb.depth * 38 + 40);

        scaleTree(static_cast<int>(std::min(newXScale, newYScale)*100));
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
#ifdef GECODE_GIST_EXPERIMENTAL
      if(c->isStepNode() && c->isHidden()) {
        y -= 38;
      }
#endif
      c = c->getParent();
    }
    
    x = static_cast<int>((xtrans+x)*scale); y = static_cast<int>(y*scale);
    
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
    if (!heatView)
      toggleHeatView();
    QWidget::update();    
  }

  void
  TreeCanvasImpl::toggleHeatView(void) {
    heatView = !heatView;
    QPalette pal(palette());
    QScrollArea* sa = 
      static_cast<QScrollArea*>(parentWidget()->parentWidget());
    pal.setColor(QPalette::Window, heatView ? Qt::black : Qt::white);
    setPalette(pal);
    sa->setPalette(pal);
    if (heatView) {
      setMinimumWidth(300);
    }
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
          (void) currentNode->getNumberOfChildNodes(stats);
          emit statusChanged(stats,true);
          emit currentNodeChanged(currentNode->getSpace(), currentNode->getStatus());
        }
        break;
    case FAILED:
    case STEP:
    case SPECIAL:
    case BRANCH:
    case SOLVED:
      Space* curSpace = currentNode->getSpace();
      Reflection::VarMap vm;
      curSpace->getVars(vm, false);
      emit inspect(vm, nextPit);
      saveCurrentNode();
      if (inspector != NULL) {
        inspector->inspect(curSpace);
      }
      break;
    }
    
    currentNode->dirtyUp();
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
    nextPit = 0;
    scale = 1.0;
    stats = Statistics();
    root->layout();
    
    emit statusChanged(stats, true);
    emit currentNodeChanged(NULL, UNDETERMINED);
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
    setCurrentNode(root);
    while(currentNode->isOnPath()) {
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
    
    VisualNode* p = currentNode->getParent();
    
#ifdef GECODE_GIST_EXPERIMENTAL
    while (p != NULL && p->isStepNode() && p->isHidden()) {
      p = p->getParent();
    }
#endif
    
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
      case STEP:
      case SPECIAL:
        if (currentNode->getNumberOfChildren() < 1)
          break;
      case BRANCH: 
          {
            int alt = 0;
            if(currentNode->isOnPath() && !currentNode->isLastOnPath()) {
              alt = currentNode->getPathAlternative();
            }
            VisualNode* n = currentNode->getChild(alt);
#ifdef GECODE_GIST_EXPERIMENTAL
            while (n->isStepNode() && n->isHidden()) {
              n = n->getChild(0);
            }
#endif
            setCurrentNode(n);
            centerCurrentNode();
            break;
          }
      case SOLVED:
      case FAILED:
      case UNDETERMINED:
        break;
      }
    }
  }

  void
  TreeCanvasImpl::navLeft(void) {
    QMutexLocker locker(&mutex);
#ifdef GECODE_GIST_EXPERIMENTAL
    VisualNode* p = currentNode;
    while(p->getParent() != NULL && p->getParent()->isStepNode()
        && p->getParent()->isHidden()) {
      p = p->getParent();
    }
    if (p != NULL) {
      int alt = p->getAlternative();
      p = p->getParent();
      if (p != NULL) {
        if (alt > 0) {
          VisualNode* n = p->getChild(alt-1);
          while(n->isStepNode() && n->isHidden()) {
            n = n->getChild(0);
          }
          setCurrentNode(n);
          centerCurrentNode();
        }
      }
    }
#else
    VisualNode* p = currentNode->getParent();
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt > 0) {
        VisualNode* n = p->getChild(alt-1);
        setCurrentNode(n);
        centerCurrentNode();
      }
    }
#endif
  }

  void
  TreeCanvasImpl::navRight(void) {
    QMutexLocker locker(&mutex);
#ifdef GECODE_GIST_EXPERIMENTAL
    VisualNode* p = currentNode;
    while(p->getParent() != NULL && p->getParent()->isStepNode()
        && p->getParent()->isHidden()) {
      p = p->getParent();
    }
    if (p != NULL) {
      int alt = p->getAlternative();
      p = p->getParent();
      if (p != NULL) {
        if (alt + 1 < p->getNumberOfChildNodes()) {
          VisualNode* n = p->getChild(alt+1);
          while(n->isStepNode() && n->isHidden()) {
            n = n->getChild(0);
          }
          setCurrentNode(n);
          centerCurrentNode();
        }
      }
    }
#else
    VisualNode* p = currentNode->getParent();
    if (p != NULL) {
      int alt = currentNode->getAlternative();
      if (alt + 1 < p->getNumberOfChildNodes()) {
        VisualNode* n = p->getChild(alt+1);
        setCurrentNode(n);
        centerCurrentNode();
      }
    }
#endif
  }
  
  void
  TreeCanvasImpl::navRoot(void) {
    QMutexLocker locker(&mutex);
    setCurrentNode(root);
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::markCurrentNode(int pit) {
    QMutexLocker locker(&mutex);
    if(nodeMap.size() > pit && nodeMap[pit] != NULL) {
      setCurrentNode(nodeMap[pit]);
      centerCurrentNode();
      emit pointInTimeChanged(pit);
    }
  }

  void
  TreeCanvasImpl::saveCurrentNode(void) {
    QMutexLocker locker(&mutex);
    assert(nextPit == nodeMap.size());
    nodeMap << currentNode;
    nextPit++;
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

  VisualNode*
  TreeCanvasImpl::eventNode(QEvent* event) {
    int x = 0;
    int y = 0;
    switch (event->type()) {
    case QEvent::ToolTip:
        {
          QHelpEvent* he = static_cast<QHelpEvent*>(event);
          x = he->x();
          y = he->y();
          break;
        }
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        {
          QMouseEvent* me = static_cast<QMouseEvent*>(event);
          x = me->x();
          y = me->y();
          break;
        }
    case QEvent::ContextMenu:
        {
          QContextMenuEvent* ce = static_cast<QContextMenuEvent*>(event);
          x = ce->x();
          y = ce->y();
          break;
        }
    default:
      return NULL;
    }
    VisualNode* n;
    n = root->findNode(static_cast<int>(x/scale-xtrans),
                       static_cast<int>((y-30)/scale));
    return n;  
  }
  
  bool
  TreeCanvasImpl::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
      VisualNode* n = eventNode(event);
      if (n != NULL && !n->isHidden() && n->getStatus() == BRANCH) {
        QHelpEvent* he = static_cast<QHelpEvent*>(event);
        QToolTip::showText(he->globalPos(), QString(n->toolTip().c_str()));
      } else {
        QToolTip::hideText();
      }
    }
    return QWidget::event(event);
  }
  
  void
  TreeCanvasImpl::resizeToOuter(void) {
    if (autoZoom)
      zoomToFit();
  }
  
  void
  TreeCanvasImpl::paintEvent(QPaintEvent* event) {
    QMutexLocker locker(&layoutMutex);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (heatView) {
      painter.setPen(Qt::white);
      painter.drawText(QRect(0,5,40,15), Qt::AlignRight, tr("cold"));
      painter.setPen(Qt::black);
      // Draw legend
      for (int i=0; i<180; i+=10) {
        int heat = (240 + i) % 360;
        painter.setBrush(QBrush(QColor::fromHsv(heat,255,255)));
        painter.drawRect(45+i, 5, 10, 15);
      }
      painter.setPen(Qt::white);
      painter.drawText(QRect(230,5,40,15), Qt::AlignLeft, tr("hot"));
      painter.setPen(Qt::black);
    }

    BoundingBox bb = root->getBoundingBox();
    QRect origClip = event->rect();
    painter.translate(0, 30);
    painter.scale(scale,scale);
    painter.translate(xtrans, 0);
    QRect clip(static_cast<int>(origClip.x()/scale-xtrans), 
               static_cast<int>(origClip.y()/scale),
               static_cast<int>(origClip.width()/scale), 
               static_cast<int>(origClip.height()/scale));
    DrawingCursor dc(root, painter, heatView, clip);
    PreorderNodeVisitor<DrawingCursor> v(dc);
    while (v.next());    
  }

  void
  TreeCanvasImpl::mouseDoubleClickEvent(QMouseEvent* event) {
    if(event->button() == Qt::LeftButton) {
      VisualNode* n = eventNode(event);
      if(n == currentNode) {      
        inspectCurrentNode();
        event->accept();
        return;
      }
    }
    event->ignore();
  }
  
  void
  TreeCanvasImpl::contextMenuEvent(QContextMenuEvent* event) {
    QMutexLocker locker(&mutex);
    VisualNode* n = eventNode(event);
    if (n != NULL) {
      setCurrentNode(n);
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
    QMutexLocker locker(&mutex);
    if (n != NULL && n != currentNode) {
      currentNode->setMarked(false);
      currentNode = n;
      currentNode->setMarked(true);
      NodeStatus status = n->getStatus();
      if(status != UNDETERMINED) {
        emit currentNodeChanged(n->getSpace(), status);
      }
      else {
        emit currentNodeChanged(NULL, status);
      }
      QWidget::update();
    }
  }
  
  void
  TreeCanvasImpl::mousePressEvent(QMouseEvent* event) {
    QMutexLocker locker(&mutex);
    if (event->button() == Qt::LeftButton) {
      VisualNode* n = eventNode(event);
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
  
  void
  TreeCanvasImpl::getRootVars(Gecode::Reflection::VarMap& vm) {
    QMutexLocker locker(&mutex);
    if(root != NULL) {
      Space* space = root->getSpace();
      space->getVars(vm, false);
      delete space;
    }
  }

  void
  TreeCanvasImpl::addVisualisation(QStringList vars, QString visType, QString windowName) {
    Config conf;
    
    pt2createView cv = conf.visualisationMap.value(visType);
    
    if(cv != NULL) {
      Reflection::VarMap vm;
      Space* rootSpace = root->getSpace();
      rootSpace->getVars(vm, false);
      delete rootSpace;

      QWidget* varView = cv(vm, nextPit, vars, this);

      varView->setWindowTitle(windowName);

      varView->setWindowFlags(Qt::Tool);

      varView->show();

      connect(this, SIGNAL(inspect(Gecode::Reflection::VarMap&, int)),
              varView, SLOT(display(Gecode::Reflection::VarMap&, int)));
      connect(this, SIGNAL(pointInTimeChanged(int)),
              varView, SLOT(displayOld(int)));
      connect(varView, SIGNAL(pointInTimeChanged(int)),
              this, SLOT(markCurrentNode(int)));
    }
  }
  
  void
  TreeCanvasImpl::addVisualisation(void) {

    Config conf;
    
    Reflection::VarMap rootVm;
    getRootVars(rootVm);

    AddVisualisationDialog* addVisDialog = new AddVisualisationDialog(conf, rootVm, this);

    if(addVisDialog->exec()) {

      QStringList itemList = addVisDialog->vars();
      QString visualisation = addVisDialog->vis();
      QString name = addVisDialog->name();

      addVisualisation(itemList, visualisation, name);
    }
  }

#ifdef GECODE_GIST_EXPERIMENTAL
  void
  TreeCanvasImpl::toggleDebug(void) {
    if(currentNode->isStepNode()) {
      currentNode->getStepDesc()->toggleDebug();
      update();
      centerCurrentNode();
    }
  }
  
  void
  TreeCanvasImpl::addChild(const QString& var, int rel0, int value) {
    QMutexLocker locker(&mutex);
    switch (currentNode->getStatus()) {
    case UNDETERMINED:
    case FAILED:
    case SOLVED:
      return;
      break;
    case STEP:
    case BRANCH:
    case SPECIAL:
      break;
    }

    Reflection::VarMap vm;
    Space* space = currentNode->getSpace();
    space->getVars(vm, false);

    VisualNode* newChild = currentNode->createChild(currentNode->getNumberOfChildren());

    newChild->setStatus(SPECIAL);
    
    if(false) {
#ifdef GECODE_HAS_INT_VARS
    } else if(vm.spec(var.toStdString().c_str()).vti() == Int::IntVarImpConf::vti) {
      IntRelType rel = static_cast<IntRelType>(rel0);
      newChild->setSpecialDesc(new SpecialDesc(var.toStdString(), rel, value));
#endif
#ifdef GECODE_HAS_SET_VARS
    } else if(vm.spec(var.toStdString().c_str()).vti() == Set::SetVarImpConf::vti) {
      SetRelType rel = static_cast<SetRelType>(rel0);
      newChild->setSpecialDesc(new SpecialDesc(var.toStdString(), rel, value));
#endif
    } else {
      // TODO nikopp: implement other options
    }
    
    newChild->setNumberOfChildren(0);
    newChild->setNoOfOpenChildren(0);

    currentNode->addChild(newChild);

    newChild->setDirty(false);
    newChild->dirtyUp();
    setCurrentNode(newChild);
    update();
  }
  
  void
  TreeCanvasImpl::addChild(void) {
    QMutexLocker locker(&mutex);
    switch (currentNode->getStatus()) {
    case UNDETERMINED:
    case FAILED:
    case SOLVED:
      return;
      break;
    case STEP:
    case BRANCH:
    case SPECIAL:
      break;
    }

    Reflection::VarMap vm;
    Space* space = currentNode->getSpace();
    space->getVars(vm, false);

    AddChild dialog(vm, this);

    if(dialog.exec()) {

      QString var = dialog.var();
      int value = dialog.value();
      int rel = dialog.rel();

      addChild(var, rel, value);
    }

    delete space;
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
    case STEP:
    case SPECIAL:
        {
          VisualNode* newChild = currentNode->createChild(currentNode->getNumberOfChildren());

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

    if(currentNode->isExpanded()) {
      setCurrentNode(currentNode->getParent());
      toggleHidden();
      return;
    }
    
    switch (currentNode->getStatus()) {
    case UNDETERMINED:
    case STEP:
    case SPECIAL:
      break;
    case FAILED:
    case BRANCH:
    case SOLVED:
      if(currentNode != root) {
        Space* inputSpace = currentNode->getInputSpace();
        int alt = currentNode->getAlternative();
        
        VisualNode* curNode = currentNode;
        VisualNode* parent = currentNode->getParent();
        curNode->setRealParent(parent);
        curNode->setRealAlternative(alt);
        
        setCurrentNode(parent);

        VisualNode* newChild =
          curNode->createStepChild(currentNode->getNumberOfChildren(),new StepDesc(0));
        
        newChild->setFirstStepNode(true);

        currentNode->setChild(alt, newChild);

        newChild->setDirty(false);
        newChild->dirtyUp();
        setCurrentNode(newChild);

        while(!inputSpace->stable()) {
	  inputSpace->step();
	  if (inputSpace->failed())
	    break;

          VisualNode* newChild =
            curNode->createStepChild(currentNode->getNumberOfChildren(),
				     new StepDesc(1));

          currentNode->addChild(newChild);

          newChild->setDirty(false);
          newChild->dirtyUp();
          setCurrentNode(newChild);
        } 
        
        currentNode->setLastStepNode(true);
        currentNode->addChild(curNode);
        if(curNode->isOpen()) {
          currentNode->openUp();
        }
        setCurrentNode(curNode);
        currentNode->setExpanded(true);
      }
      break;
    }
    update();
    centerCurrentNode();
  }
  
  void
  TreeCanvasImpl::executeJavaScript(const QString& model) {
    root->executeJavaScript(model.toStdString());
  }

  void
  TreeCanvasImpl::replaceRootCopy(Space* s) {
    root->replaceCopy(s);
  }
#endif

  TreeCanvas::TreeCanvas(Space* root, Better* b,
                         QWidget* parent) : QWidget(parent) {
    QGridLayout* layout = new QGridLayout(this);    

    QScrollArea* scrollArea = new QScrollArea(this);
    
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setAlignment(Qt::AlignHCenter);
    scrollArea->setAutoFillBackground(true);
    QPalette myPalette(scrollArea->palette());
    myPalette.setColor(QPalette::Window, Qt::white);
    scrollArea->setPalette(myPalette);
    canvas = new TreeCanvasImpl(root, b, this);
    canvas->setPalette(myPalette);
    canvas->setObjectName("canvas");

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

    addVisualisation = new QAction("Add visualisation", this);
    addVisualisation->setShortcut(QKeySequence("Shift+V"));
    connect(addVisualisation, SIGNAL(triggered()), canvas, SLOT(addVisualisation()));

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
    
    forwardTimeStep = new QAction("Step forward", this);
    forwardTimeStep->setShortcut(QKeySequence("+"));
    forwardTimeStep->setObjectName("forwardTimeStep");
    
    backwardTimeStep = new QAction("Step backward", this);
    backwardTimeStep->setShortcut(QKeySequence("-"));
    backwardTimeStep->setObjectName("backwardTimeStep");
    
    toggleDebug = new QAction("Toggle debug", this);
    toggleDebug->setShortcut(QKeySequence("Shift+D"));
    connect(toggleDebug, SIGNAL(triggered()), canvas, SLOT(toggleDebug()));
    
    executeJavaScript = new QAction("Execute JavaScript", this);
    executeJavaScript->setShortcut(QKeySequence("Shift+J"));
    connect(executeJavaScript, SIGNAL(triggered()), canvas, SLOT(executeJavaScript()));
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

    addAction(addVisualisation);
    addAction(toggleHeatView);
    addAction(analyzeTree);
    
    addAction(setPath);
    addAction(inspectPath);

#ifdef GECODE_GIST_EXPERIMENTAL
    addAction(addChild);
    addAction(addFixpoint);
    addAction(expandCurrentNode);
    addAction(forwardTimeStep);
    addAction(backwardTimeStep);
    addAction(toggleDebug);
    addAction(executeJavaScript);
#endif

    contextMenu = new QMenu(this);
    contextMenu->addAction(inspectCN);
    contextMenu->addAction(centerCN);

    contextMenu->addSeparator();

    contextMenu->addAction(searchNext);
    contextMenu->addAction(searchAll);      

    contextMenu->addSeparator();

    contextMenu->addAction(toggleHidden);
    contextMenu->addAction(hideFailed);
    contextMenu->addAction(unhideAll);

    contextMenu->addSeparator();

    contextMenu->addAction(setPath);
    contextMenu->addAction(inspectPath);

    contextMenu->addSeparator();

#ifdef GECODE_GIST_EXPERIMENTAL
    contextMenu->addAction(addChild);
    contextMenu->addAction(addFixpoint);

    contextMenu->addSeparator();

    contextMenu->addAction(expandCurrentNode);
    contextMenu->addAction(toggleDebug);
#endif
    
    connect(scaleBar, SIGNAL(valueChanged(int)), canvas, SLOT(scaleTree(int)));

    connect(canvas, SIGNAL(scaleChanged(int)), scaleBar, SLOT(setValue(int)));
    
#ifdef GECODE_GIST_EXPERIMENTAL
    connect(timeBar, SIGNAL(valueChanged(int)), canvas, SLOT(markCurrentNode(int)));
    connect(canvas, SIGNAL(pointInTimeChanged(int)), timeBar, SLOT(setValue(int)));
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
  TreeCanvas::resizeEvent(QResizeEvent*) {
    canvas->resizeToOuter();
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
  TreeCanvas::on_canvas_currentNodeChanged(Gecode::Space*, Gecode::Gist::NodeStatus status) {
    switch (status) {
        case Gecode::Gist::SOLVED:
        case Gecode::Gist::FAILED:
          navDown->setEnabled(false);

          searchNext->setEnabled(false);
          searchAll->setEnabled(false);
          toggleHidden->setEnabled(false);
          hideFailed->setEnabled(false);
          unhideAll->setEnabled(false);

#ifdef GECODE_GIST_EXPERIMENTAL
          addChild->setEnabled(false);
          addFixpoint->setEnabled(false);
          expandCurrentNode->setEnabled(true);
          toggleDebug->setEnabled(false);
#endif
          break;
        case Gecode::Gist::UNDETERMINED:
          navDown->setEnabled(false);

          searchNext->setEnabled(true);
          searchAll->setEnabled(true);
          toggleHidden->setEnabled(false);
          hideFailed->setEnabled(false);
          unhideAll->setEnabled(false);

#ifdef GECODE_GIST_EXPERIMENTAL
          addChild->setEnabled(false);
          addFixpoint->setEnabled(false);
          expandCurrentNode->setEnabled(true);
          toggleDebug->setEnabled(false);
#endif
          break;
        case Gecode::Gist::BRANCH:
        case Gecode::Gist::SPECIAL:
          navDown->setEnabled(true);

          searchNext->setEnabled(true);
          searchAll->setEnabled(true);
          toggleHidden->setEnabled(true);
          hideFailed->setEnabled(true);
          unhideAll->setEnabled(true);

#ifdef GECODE_GIST_EXPERIMENTAL
          addChild->setEnabled(true);
          addFixpoint->setEnabled(true);
          expandCurrentNode->setEnabled(true);
          toggleDebug->setEnabled(false);
#endif
          break;
        case Gecode::Gist::STEP:
          navDown->setEnabled(true);

          searchNext->setEnabled(true);
          searchAll->setEnabled(true);
          toggleHidden->setEnabled(true);
          hideFailed->setEnabled(false);
          unhideAll->setEnabled(true);

#ifdef GECODE_GIST_EXPERIMENTAL
          addChild->setEnabled(true);
          addFixpoint->setEnabled(true);
          expandCurrentNode->setEnabled(false);
          toggleDebug->setEnabled(true);
#endif
          break;
    }
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
  TreeCanvas::on_canvas_inspect(Gecode::Reflection::VarMap&, int pit) {
    timeBar->setMaximum(pit);
    timeBar->setValue(pit);
  }

  void
  TreeCanvas::on_forwardTimeStep_triggered(void) {
    timeBar->setValue(timeBar->value() + 1);
  }

  void
  TreeCanvas::on_backwardTimeStep_triggered(void) {
    timeBar->setValue(timeBar->value() - 1);
  }
#endif
    
}}

// STATISTICS: gist-any

