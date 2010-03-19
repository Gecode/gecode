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

#include <QtGui/QPainter>

#include <stack>
#include <fstream>

#include <gecode/gist/treecanvas.hh>

#include <gecode/gist/nodevisitor.hh>
#include <gecode/gist/layoutcursor.hh>
#include <gecode/gist/visualnode.hh>
#include <gecode/gist/drawingcursor.hh>

#include <gecode/search.hh>
#include <gecode/search/support.hh>

namespace Gecode { namespace Gist {

  TreeCanvas::TreeCanvas(Space* rootSpace, bool bab,
                         QWidget* parent, const Options& opt)
    : QWidget(parent)
    , mutex(QMutex::Recursive)
    , layoutMutex(QMutex::Recursive)
    , finishedFlag(false)
    , autoHideFailed(true), autoZoom(false)
    , refresh(500), smoothScrollAndZoom(false), nextPit(0)
    , targetZoom(LayoutConfig::defScale)
    , metaZoomCurrent(static_cast<double>(LayoutConfig::defScale))
    , zoomTimerId(0)
    , targetScrollX(0), targetScrollY(0)
    , metaScrollXCurrent(0.0), metaScrollYCurrent(0.0)
    , scrollTimerId(0)
    , targetW(0), targetH(0), targetScale(0)
    , layoutDoneTimerId(0) {
      QMutexLocker locker(&mutex);
      curBest = (bab ? new BestNode(NULL) : NULL);
      if (rootSpace->status() == SS_FAILED) {
        rootSpace = NULL;
      } else {
        rootSpace = Gecode::Search::snapshot(rootSpace,opt);
      }
      na = new Node::NodeAllocator();
      root = new (*na) VisualNode(rootSpace);
      root->layout();
      root->setMarked(true);
      currentNode = root;
      pathHead = root;
      scale = LayoutConfig::defScale / 100.0;

      setAutoFillBackground(true);

      connect(&searcher, SIGNAL(update(int,int,int)), this,
                         SLOT(layoutDone(int,int,int)));
      connect(&searcher, SIGNAL(statusChanged(bool)), this,
              SLOT(statusChanged(bool)));

      connect(&searcher, SIGNAL(solution(const Space*)),
              this, SIGNAL(solution(const Space*)),
              Qt::BlockingQueuedConnection);
      connect(&searcher, SIGNAL(solution(const Space*)),
              this, SLOT(inspectSolution(const Space*)),
              Qt::BlockingQueuedConnection);

      connect(&searcher, SIGNAL(searchFinished(void)), this, SIGNAL(searchFinished(void)));

      qRegisterMetaType<Statistics>("Statistics");
      update();
  }

  TreeCanvas::~TreeCanvas(void) { delete root; delete na; }

  void
  TreeCanvas::addDoubleClickInspector(Inspector* i) {
    doubleClickInspectors.append(QPair<Inspector*,bool>(i,false));
  }

  void
  TreeCanvas::activateDoubleClickInspector(int i, bool active) {
    assert(i < doubleClickInspectors.size());
    doubleClickInspectors[i].second = active;
  }

  void
  TreeCanvas::addSolutionInspector(Inspector* i) {
    solutionInspectors.append(QPair<Inspector*,bool>(i,false));
  }

  void
  TreeCanvas::activateSolutionInspector(int i, bool active) {
    assert(i < solutionInspectors.size());
    solutionInspectors[i].second = active;
  }

  void
  TreeCanvas::addMoveInspector(Inspector* i) {
    moveInspectors.append(QPair<Inspector*,bool>(i,false));
  }

  void
  TreeCanvas::activateMoveInspector(int i, bool active) {
    assert(i < moveInspectors.size());
    moveInspectors[i].second = active;
  }

  void
  TreeCanvas::scaleTree(int scale0) {
    QMutexLocker locker(&layoutMutex);
    BoundingBox bb;
    scale0 = std::min(std::max(scale0, LayoutConfig::minScale),
                      LayoutConfig::maxScale);
    scale = (static_cast<double>(scale0)) / 100.0;
    bb = root->getBoundingBox();
    int w =
      static_cast<int>((bb.right-bb.left+Layout::extent)*scale);
    int h =
      static_cast<int>(2*Layout::extent+
        root->getShape()->depth()*Layout::dist_y*scale);
    resize(w,h);
    emit scaleChanged(scale0);
    QWidget::update();
  }

  void
  TreeCanvas::update(void) {
    QMutexLocker locker(&mutex);
    layoutMutex.lock();
    if (root != NULL) {
      root->layout();
      BoundingBox bb = root->getBoundingBox();

      int w = static_cast<int>((bb.right-bb.left+Layout::extent)*scale);
      int h =
        static_cast<int>(2*Layout::extent+
          root->getShape()->depth()*Layout::dist_y*scale);
      xtrans = -bb.left+(Layout::extent / 2);
      resize(w,h);
    }
    if (autoZoom)
      zoomToFit();
    layoutMutex.unlock();
    QWidget::update();
  }

  void
  TreeCanvas::layoutDone(int w, int h, int scale0) {
    targetW = w; targetH = h; targetScale = scale0;
    if (layoutDoneTimerId == 0)
      layoutDoneTimerId = startTimer(15);
  }

  void
  TreeCanvas::statusChanged(bool finished) {
    if (finished) {
      update();
      centerCurrentNode();
    }
    emit statusChanged(currentNode, stats, finished);
  }

  void
  SearcherThread::search(VisualNode* n, bool all, TreeCanvas* ti) {
    node = n;
    
    depth = -1;
    for (VisualNode* p = n; p != NULL; p = p->getParent())
      depth++;
    
    a = all;
    t = ti;
    start();
  }

  void
  SearcherThread::updateCanvas(void) {
    t->layoutMutex.lock();
    if (t->root == NULL)
      return;

    if (t->autoHideFailed) {
      t->root->hideFailed();
    }
    for (VisualNode* n = t->currentNode; n != NULL; n = n->getParent()) {
      if (n->isHidden()) {
        t->currentNode->setMarked(false);
        t->currentNode = n;
        t->currentNode->setMarked(true);
        break;
      }
    }
    
    t->root->layout();
    BoundingBox bb = t->root->getBoundingBox();

    int w = static_cast<int>((bb.right-bb.left+Layout::extent)*t->scale);
    int h = static_cast<int>(2*Layout::extent+
                             t->root->getShape()->depth()
                              *Layout::dist_y*t->scale);
    t->xtrans = -bb.left+(Layout::extent / 2);

    int scale0 = static_cast<int>(t->scale*100);
    if (t->autoZoom) {
      QWidget* p = t->parentWidget();
      if (p) {
        double newXScale =
          static_cast<double>(p->width()) / (bb.right - bb.left +
                                             Layout::extent);
        double newYScale =
          static_cast<double>(p->height()) /
          (t->root->getShape()->depth() * Layout::dist_y + 2*Layout::extent);

        scale0 = static_cast<int>(std::min(newXScale, newYScale)*100);
        if (scale0<LayoutConfig::minScale)
          scale0 = LayoutConfig::minScale;
        if (scale0>LayoutConfig::maxAutoZoomScale)
          scale0 = LayoutConfig::maxAutoZoomScale;
        double scale = (static_cast<double>(scale0)) / 100.0;

        w = static_cast<int>((bb.right-bb.left+Layout::extent)*scale);
        h = static_cast<int>(2*Layout::extent+
                             t->root->getShape()->depth()*Layout::dist_y*scale);
      }
    }
    t->layoutMutex.unlock();
    emit update(w,h,scale0);
  }

  class SearchItem {
  public:
    VisualNode* n;
    int i;
    int noOfChildren;
    SearchItem(VisualNode* n0, int noOfChildren0)
      : n(n0), i(-1), noOfChildren(noOfChildren0) {}
  };

  void
  SearcherThread::run() {
    {
      if (!node->isOpen())
        return;
      t->mutex.lock();
      emit statusChanged(false);

      unsigned int kids = 
        node->getNumberOfChildNodes(*t->na, t->curBest, t->stats,
                                    t->c_d, t->a_d);
      if (kids == 0) {
        t->mutex.unlock();
        updateCanvas();
        emit statusChanged(true);
        return;
      }

      std::stack<SearchItem> stck;
      stck.push(SearchItem(node,kids));
      t->stats.maxDepth =
        std::max(static_cast<long unsigned int>(t->stats.maxDepth), 
                 static_cast<long unsigned int>(depth+stck.size()));

      VisualNode* sol = NULL;
      int nodeCount = 0;
      t->stopSearchFlag = false;
      while (!stck.empty() && !t->stopSearchFlag) {
        if (t->refresh > 0 && ++nodeCount > t->refresh) {
          node->dirtyUp();
          updateCanvas();
          emit statusChanged(false);
          nodeCount = 0;
        }
        SearchItem& si = stck.top();
        si.i++;
        if (si.i == si.noOfChildren) {
          stck.pop();
        } else {
          VisualNode* n = si.n->getChild(si.i);
          if (n->isOpen()) {
            kids = n->getNumberOfChildNodes(*t->na, t->curBest, t->stats,
                                            t->c_d, t->a_d);
            if (kids == 0) {
              if (n->getStatus() == SOLVED) {
                assert(n->hasWorkingSpace());
                emit solution(n->getWorkingSpace());
                n->purge();
                sol = n;
                if (!a)
                  break;
              }
            } else {
              stck.push(SearchItem(n,kids));
              t->stats.maxDepth =
                std::max(static_cast<long unsigned int>(t->stats.maxDepth), 
                         static_cast<long unsigned int>(depth+stck.size()));
            }
          }
        }
      }
      node->dirtyUp();
      t->stopSearchFlag = false;
      t->mutex.unlock();
      if (sol != NULL) {
        t->setCurrentNode(sol,false);
      } else {
        t->setCurrentNode(node,false);
      }
    }
    updateCanvas();
    emit statusChanged(true);
    if (t->finishedFlag)
      emit searchFinished();
  }

  void
  TreeCanvas::searchAll(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, true, this);
  }

  void
  TreeCanvas::searchOne(void) {
    QMutexLocker locker(&mutex);
    searcher.search(currentNode, false, this);
  }

  void
  TreeCanvas::toggleHidden(void) {
    QMutexLocker locker(&mutex);
    currentNode->toggleHidden();
    update();
    centerCurrentNode();
    emit statusChanged(currentNode, stats, true);
  }

  void
  TreeCanvas::hideFailed(void) {
    QMutexLocker locker(&mutex);
    currentNode->hideFailed();
    update();
    centerCurrentNode();
    emit statusChanged(currentNode, stats, true);
  }

  void
  TreeCanvas::unhideAll(void) {
    QMutexLocker locker(&mutex);
    QMutexLocker layoutLocker(&layoutMutex);
    currentNode->unhideAll();
    update();
    centerCurrentNode();
    emit statusChanged(currentNode, stats, true);
  }

  void
  TreeCanvas::timerEvent(QTimerEvent* e) {
    if (e->timerId() == zoomTimerId) {
      double offset = static_cast<double>(targetZoom - metaZoomCurrent) / 6.0;
      metaZoomCurrent += offset;
      scaleBar->setValue(static_cast<int>(metaZoomCurrent));
      if (static_cast<int>(metaZoomCurrent+.5) == targetZoom) {
        killTimer(zoomTimerId);
        zoomTimerId = 0;
      }
    } else if (e->timerId() == scrollTimerId) {
      QScrollArea* sa =
        static_cast<QScrollArea*>(parentWidget()->parentWidget());

      double xoffset =
        static_cast<double>(targetScrollX - metaScrollXCurrent) / 6.0;
      metaScrollXCurrent += xoffset;
      sa->horizontalScrollBar()
        ->setValue(static_cast<int>(metaScrollXCurrent));
      double yoffset =
        static_cast<double>(targetScrollY - metaScrollYCurrent) / 6.0;
      metaScrollYCurrent += yoffset;
      sa->verticalScrollBar()
        ->setValue(static_cast<int>(metaScrollYCurrent));

      if (static_cast<int>(metaScrollXCurrent+.5) == targetScrollX &&
          static_cast<int>(metaScrollYCurrent+.5) == targetScrollY) {
        killTimer(scrollTimerId);
        scrollTimerId = 0;
      }
    } else if (e->timerId() == layoutDoneTimerId) {
      if (!smoothScrollAndZoom) {
        scaleTree(targetScale);
      } else {
        metaZoomCurrent = static_cast<int>(scale*100);
        targetZoom = targetScale;
        targetZoom = std::min(std::max(targetZoom, LayoutConfig::minScale),
                              LayoutConfig::maxAutoZoomScale);
        zoomTimerId = startTimer(15);
      }
      resize(targetW,targetH);
      QWidget::update();
      killTimer(layoutDoneTimerId);
      layoutDoneTimerId = 0;
    }
  }

  void
  TreeCanvas::zoomToFit(void) {
    QMutexLocker locker(&layoutMutex);
    if (root != NULL) {
      BoundingBox bb;
      bb = root->getBoundingBox();
      QWidget* p = parentWidget();
      if (p) {
        double newXScale =
          static_cast<double>(p->width()) / (bb.right - bb.left +
                                             Layout::extent);
        double newYScale =
          static_cast<double>(p->height()) / (root->getShape()->depth() * 
                                              Layout::dist_y +
                                              2*Layout::extent);
        int scale0 = static_cast<int>(std::min(newXScale, newYScale)*100);
        if (scale0<LayoutConfig::minScale)
          scale0 = LayoutConfig::minScale;
        if (scale0>LayoutConfig::maxAutoZoomScale)
          scale0 = LayoutConfig::maxAutoZoomScale;

        if (!smoothScrollAndZoom) {
          scaleTree(scale0);
        } else {
          metaZoomCurrent = static_cast<int>(scale*100);
          targetZoom = scale0;
          targetZoom = std::min(std::max(targetZoom, LayoutConfig::minScale),
                                LayoutConfig::maxAutoZoomScale);
          zoomTimerId = startTimer(15);
        }
      }
    }
  }

  void
  TreeCanvas::centerCurrentNode(void) {
    QMutexLocker locker(&mutex);
    int x=0;
    int y=0;

    VisualNode* c = currentNode;
    while (c != NULL) {
      x += c->getOffset();
      y += Layout::dist_y;
      c = c->getParent();
    }

    x = static_cast<int>((xtrans+x)*scale); y = static_cast<int>(y*scale);

    QScrollArea* sa =
      static_cast<QScrollArea*>(parentWidget()->parentWidget());

    if (!smoothScrollAndZoom) {
      sa->ensureVisible(x,y);
    } else {
      x -= sa->viewport()->width() / 2;
      y -= sa->viewport()->height() / 2;

      metaScrollXCurrent = sa->horizontalScrollBar()->value();
      metaScrollYCurrent = sa->verticalScrollBar()->value();
      targetScrollX = std::max(sa->horizontalScrollBar()->minimum(), x);
      targetScrollX = std::min(sa->horizontalScrollBar()->maximum(),
                               targetScrollX);
      targetScrollY = std::max(sa->verticalScrollBar()->minimum(), y);
      targetScrollY = std::min(sa->verticalScrollBar()->maximum(),
                               targetScrollY);
      scrollTimerId = startTimer(15);
    }
  }

  void
  TreeCanvas::inspectCurrentNode(void) {
    QMutexLocker locker(&mutex);

    if (currentNode->isHidden()) {
      toggleHidden();
      return;
    }

    switch (currentNode->getStatus()) {
    case UNDETERMINED:
        {
          unsigned int kids =  
            currentNode->getNumberOfChildNodes(*na,curBest,stats,c_d,a_d);
          int depth = -1;
          for (VisualNode* p = currentNode; p != NULL; p = p->getParent())
            depth++;
          if (kids > 0)
            depth++;
          stats.maxDepth =
            std::max(stats.maxDepth, depth);
          if (currentNode->getStatus() == SOLVED) {
            assert(currentNode->hasWorkingSpace());
            emit solution(currentNode->getWorkingSpace());
            currentNode->purge();
          }
          emit statusChanged(currentNode,stats,true);
          Space* curSpace = currentNode->getSpace(curBest,c_d,a_d);
          for (int i=0; i<moveInspectors.size(); i++) {
            if (moveInspectors[i].second) {
              moveInspectors[i].first->inspect(*curSpace);
            }
          }
        }
        break;
    case FAILED:
    case STEP:
    case SPECIAL:
    case BRANCH:
    case SOLVED:
      {
        // SizeCursor sc(currentNode);
        // PreorderNodeVisitor<SizeCursor> pnv(sc);
        // int nodes = 1;
        // while (pnv.next()) { nodes++; }
        // std::cout << "sizeof(VisualNode): " << sizeof(VisualNode)
        //           << std::endl;
        // std::cout << "Size: " << (pnv.getCursor().s)/1024 << std::endl;
        // std::cout << "Nodes: " << nodes << std::endl;
        // std::cout << "Size / node: " << (pnv.getCursor().s)/nodes
        //           << std::endl;

        if (currentNode->isRoot() && currentNode->getStatus() == FAILED)
          break;
        Space* curSpace = currentNode->getSpace(curBest,c_d,a_d);
        if (currentNode->getStatus() == SOLVED &&
            curSpace->status() != SS_SOLVED) {
          // in the presence of weakly monotonic propagators, we may have to
          // use search to find the solution here
          Space* dfsSpace = Gecode::dfs(curSpace);
          delete curSpace;
          curSpace = dfsSpace;
        }
        saveCurrentNode();

        for (int i=0; i<doubleClickInspectors.size(); i++) {
          if (doubleClickInspectors[i].second) {
            doubleClickInspectors[i].first->inspect(*curSpace);
          }
        }
        delete curSpace;
      }
      break;
    }

    currentNode->dirtyUp();
    update();
    centerCurrentNode();
  }

  void
  TreeCanvas::inspectSolution(const Space* s) {
    Space* c = NULL;
    for (int i=0; i<solutionInspectors.size(); i++) {
      if (solutionInspectors[i].second) {
        if (c == NULL)
          c = s->clone();
        solutionInspectors[i].first->inspect(*c);
      }
    }
    for (int i=0; i<moveInspectors.size(); i++) {
      if (moveInspectors[i].second) {
        if (c == NULL)
          c = s->clone();
        moveInspectors[i].first->inspect(*c);
      }
    }
    delete c;
  }

  void
  TreeCanvas::stopSearch(void) {
    stopSearchFlag = true;
    layoutDoneTimerId = startTimer(15);
  }

  void
  TreeCanvas::reset(void) {
    QMutexLocker locker(&mutex);
    Space* rootSpace =
      root->getStatus() == FAILED ? NULL : root->getSpace(curBest,c_d,a_d);
    if (curBest != NULL) {
      delete curBest;
      curBest = new BestNode(NULL);
    }
    delete root;
    delete na;
    na = new Node::NodeAllocator();
    root = new (*na) VisualNode(rootSpace);
    root->setMarked(true);
    currentNode = root;
    pathHead = root;
    nodeMap.clear();
    nextPit = 0;
    scale = 1.0;
    stats = Statistics();
    for (int i=bookmarks.size(); i--;)
      emit removedBookmark(i);
    bookmarks.clear();
    root->layout();

    emit statusChanged(currentNode, stats, true);
    update();
  }

  void
  TreeCanvas::bookmarkNode(void) {
    QMutexLocker locker(&mutex);
    if (!currentNode->isBookmarked()) {
      bool ok;
      QString text =
        QInputDialog::getText(this, "Add bookmark", "Name:", 
                              QLineEdit::Normal,"",&ok);
      if (ok) {
        currentNode->setBookmarked(true);
        bookmarks.append(currentNode);
        if (text == "")
          text = QString("Node ")+QString().setNum(bookmarks.size());
        emit addedBookmark(text);
      }
    } else {
      currentNode->setBookmarked(false);
      int idx = bookmarks.indexOf(currentNode);
      bookmarks.remove(idx);
      emit removedBookmark(idx);
    }
    currentNode->dirtyUp();
    update();
  }
  
  void
  TreeCanvas::setPath(void) {
    QMutexLocker locker(&mutex);
    if(currentNode == pathHead)
      return;

    pathHead->unPathUp();
    pathHead = currentNode;

    currentNode->pathUp();
    currentNode->dirtyUp();
    update();
  }

  void
  TreeCanvas::inspectPath(void) {
    QMutexLocker locker(&mutex);
    setCurrentNode(root);
    if (currentNode->isOnPath()) {
      inspectCurrentNode();
      int nextAlt = currentNode->getPathAlternative();
      while (nextAlt >= 0) {
        setCurrentNode(currentNode->getChild(nextAlt));
        inspectCurrentNode();
        nextAlt = currentNode->getPathAlternative();
      }
    }
    update();
  }

  void
  TreeCanvas::emitStatusChanged(void) {
    emit statusChanged(currentNode, stats, true);
  }

  void
  TreeCanvas::navUp(void) {
    QMutexLocker locker(&mutex);

    VisualNode* p = currentNode->getParent();

    setCurrentNode(p);

    if (p != NULL) {
      centerCurrentNode();
    }
  }

  void
  TreeCanvas::navDown(void) {
    QMutexLocker locker(&mutex);
    if (!currentNode->isHidden()) {
      switch (currentNode->getStatus()) {
      case STEP:
      case SPECIAL:
        if (currentNode->getNumberOfChildren() < 1)
          break;
      case BRANCH:
          {
            int alt = std::max(0, currentNode->getPathAlternative());
            VisualNode* n = currentNode->getChild(alt);
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
  TreeCanvas::navLeft(void) {
    QMutexLocker locker(&mutex);
    VisualNode* p = currentNode->getParent();
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
  TreeCanvas::navRight(void) {
    QMutexLocker locker(&mutex);
    VisualNode* p = currentNode->getParent();
    if (p != NULL) {
      unsigned int alt = currentNode->getAlternative();
      if (alt + 1 < p->getNumberOfChildren()) {
        VisualNode* n = p->getChild(alt+1);
        setCurrentNode(n);
        centerCurrentNode();
      }
    }
  }

  void
  TreeCanvas::navRoot(void) {
    QMutexLocker locker(&mutex);
    setCurrentNode(root);
    centerCurrentNode();
  }

  void
  TreeCanvas::navNextSol(bool back) {
    QMutexLocker locker(&mutex);
    NextSolCursor nsc(currentNode, back);
    PreorderNodeVisitor<NextSolCursor> nsv(nsc);
    while (nsv.next()) {}
    if (nsv.getCursor().node() != root) {
      setCurrentNode(nsv.getCursor().node());
      centerCurrentNode();
    }
  }

  void
  TreeCanvas::navPrevSol(void) {
    navNextSol(true);
  }

  void
  TreeCanvas::markCurrentNode(int pit) {
    QMutexLocker locker(&mutex);
    if(nodeMap.size() > pit && nodeMap[pit] != NULL) {
      setCurrentNode(nodeMap[pit]);
      centerCurrentNode();
      emit pointInTimeChanged(pit);
    }
  }

  void
  TreeCanvas::saveCurrentNode(void) {
    QMutexLocker locker(&mutex);
    assert(nextPit == nodeMap.size());
    nodeMap << currentNode;
    nextPit++;
  }

  void
  TreeCanvas::exportNodePDF(VisualNode* n) {
#if QT_VERSION >= 0x040400
    QString filename = QFileDialog::getSaveFileName(this, tr("Export tree as pdf"), "", tr("PDF (*.pdf)"));
    if (filename != "") {
      QPrinter printer(QPrinter::ScreenResolution);
      QMutexLocker locker(&mutex);

      BoundingBox bb = n->getBoundingBox();
      printer.setFullPage(true);
      printer.setPaperSize(QSizeF(bb.right-bb.left+Layout::extent,
                                  n->getShape()->depth() * Layout::dist_y +
                                  Layout::extent), QPrinter::Point);
      printer.setOutputFileName(filename);
      QPainter painter(&printer);

      painter.setRenderHint(QPainter::Antialiasing);

      QRect pageRect = printer.pageRect();
      double newXScale =
        static_cast<double>(pageRect.width()) / (bb.right - bb.left +
                                                 Layout::extent);
      double newYScale =
        static_cast<double>(pageRect.height()) /
                            (n->getShape()->depth() * Layout::dist_y +
                             Layout::extent);
      double printScale = std::min(newXScale, newYScale);
      painter.scale(printScale,printScale);

      int printxtrans = -bb.left+(Layout::extent / 2);

      painter.translate(printxtrans, Layout::dist_y / 2);
      QRect clip(0,0,0,0);
      DrawingCursor dc(n, curBest, painter, clip, showCopies);
      currentNode->setMarked(false);
      PreorderNodeVisitor<DrawingCursor> v(dc);
      while (v.next()) {}
      currentNode->setMarked(true);
    }
#endif
  }

  void
  TreeCanvas::exportWholeTreePDF(void) {
#if QT_VERSION >= 0x040400
    exportNodePDF(root);
#endif
  }

  void
  TreeCanvas::exportPDF(void) {
#if QT_VERSION >= 0x040400
    exportNodePDF(currentNode);
#endif
  }

  void
  TreeCanvas::print(void) {
    QPrinter printer;
    if (QPrintDialog(&printer, this).exec() == QDialog::Accepted) {
      QMutexLocker locker(&mutex);

      BoundingBox bb = root->getBoundingBox();
      QRect pageRect = printer.pageRect();
      double newXScale =
        static_cast<double>(pageRect.width()) / (bb.right - bb.left +
                                                 Layout::extent);
      double newYScale =
        static_cast<double>(pageRect.height()) /
                            (root->getShape()->depth() * Layout::dist_y +
                             2*Layout::extent);
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
      DrawingCursor dc(root, curBest, painter, clip, showCopies);
      PreorderNodeVisitor<DrawingCursor> v(dc);
      while (v.next()) {}
    }
  }

  VisualNode*
  TreeCanvas::eventNode(QEvent* event) {
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
  TreeCanvas::event(QEvent* event) {
    if (mutex.tryLock()) {
      if (event->type() == QEvent::ToolTip) {
        VisualNode* n = eventNode(event);
        if (n != NULL && !n->isHidden() &&
            (n->getStatus() == BRANCH)) {
          QHelpEvent* he = static_cast<QHelpEvent*>(event);
          QToolTip::showText(he->globalPos(),
                             QString(n->toolTip(curBest,c_d,a_d).c_str()));
        } else {
          QToolTip::hideText();
        }
      }
      mutex.unlock();
    }
    return QWidget::event(event);
  }

  void
  TreeCanvas::resizeToOuter(void) {
    if (autoZoom)
      zoomToFit();
  }

  void
  TreeCanvas::paintEvent(QPaintEvent* event) {
    QMutexLocker locker(&layoutMutex);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    BoundingBox bb = root->getBoundingBox();
    QRect origClip = event->rect();
    painter.translate(0, 30);
    painter.scale(scale,scale);
    painter.translate(xtrans, 0);
    QRect clip(static_cast<int>(origClip.x()/scale-xtrans),
               static_cast<int>(origClip.y()/scale),
               static_cast<int>(origClip.width()/scale),
               static_cast<int>(origClip.height()/scale));
    DrawingCursor dc(root, curBest, painter, clip, showCopies);
    PreorderNodeVisitor<DrawingCursor> v(dc);

    while (v.next()) {}

    // int nodesLayouted = 1;
    // clock_t t0 = clock();
    // while (v.next()) { nodesLayouted++; }
    // double t = (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC) * 1000.0;
    // double nps = static_cast<double>(nodesLayouted) /
    //   (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC);
    // std::cout << "Drawing done. " << nodesLayouted << " nodes in "
    //   << t << " ms. " << nps << " nodes/s." << std::endl;

  }

  void
  TreeCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
    if (mutex.tryLock()) {
      if(event->button() == Qt::LeftButton) {
        VisualNode* n = eventNode(event);
        if(n == currentNode) {
          inspectCurrentNode();
          event->accept();
          mutex.unlock();
          return;
        }
      }
      mutex.unlock();
    }
    event->ignore();
  }

  void
  TreeCanvas::contextMenuEvent(QContextMenuEvent* event) {
    if (mutex.tryLock()) {
      VisualNode* n = eventNode(event);
      if (n != NULL) {
        setCurrentNode(n);
        emit contextMenu(event);
        event->accept();
        mutex.unlock();
        return;
      }
      mutex.unlock();
    }
    event->ignore();
  }

  bool
  TreeCanvas::finish(void) {
    if (finishedFlag)
      return true;
    stopSearchFlag = true;
    finishedFlag = true;
    for (int i=0; i<doubleClickInspectors.size(); i++)
      doubleClickInspectors[i].first->finalize();
    for (int i=0; i<solutionInspectors.size(); i++)
      solutionInspectors[i].first->finalize();
    for (int i=0; i<moveInspectors.size(); i++)
      moveInspectors[i].first->finalize();
    return !searcher.isRunning();
  }

  void
  TreeCanvas::setCurrentNode(VisualNode* n, bool update) {
    QMutexLocker locker(&mutex);
    if (update && n != NULL && n != currentNode &&
        n->getStatus() != UNDETERMINED && !n->isHidden()) {
      Space* curSpace = NULL;
      for (int i=0; i<moveInspectors.size(); i++) {
        if (moveInspectors[i].second) {
          if (curSpace == NULL)
            curSpace = n->getSpace(curBest,c_d,a_d);
          moveInspectors[i].first->inspect(*curSpace);
        }
      }
    }
    if (n != NULL) {
      currentNode->setMarked(false);
      currentNode = n;
      currentNode->setMarked(true);
      emit statusChanged(currentNode,stats,true);
      if (update)
        QWidget::update();
    }
  }

  void
  TreeCanvas::mousePressEvent(QMouseEvent* event) {
    if (mutex.tryLock()) {
      if (event->button() == Qt::LeftButton) {
        VisualNode* n = eventNode(event);
        setCurrentNode(n);
        if (n != NULL) {
          event->accept();
          mutex.unlock();
          return;
        }
      }
      mutex.unlock();
    }
    event->ignore();
  }

  void
  TreeCanvas::setRecompDistances(int c_d0, int a_d0) {
    c_d = c_d0; a_d = a_d0;
  }

  void
  TreeCanvas::setAutoHideFailed(bool b) {
    autoHideFailed = b;
  }

  void
  TreeCanvas::setAutoZoom(bool b) {
    autoZoom = b;
    if (autoZoom) {
      zoomToFit();
    }
    emit autoZoomChanged(b);
    scaleBar->setEnabled(!b);
  }

  void
  TreeCanvas::setShowCopies(bool b) {
    showCopies = b;
  }
  bool
  TreeCanvas::getShowCopies(void) {
    return showCopies;
  }

  bool
  TreeCanvas::getAutoHideFailed(void) {
    return autoHideFailed;
  }

  bool
  TreeCanvas::getAutoZoom(void) {
    return autoZoom;
  }

  void
  TreeCanvas::setRefresh(int i) {
    refresh = i;
  }

  bool
  TreeCanvas::getSmoothScrollAndZoom(void) {
    return smoothScrollAndZoom;
  }

  void
  TreeCanvas::setSmoothScrollAndZoom(bool b) {
    smoothScrollAndZoom = b;
  }

}}

// STATISTICS: gist-any
