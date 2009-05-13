/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2009-02-23 20:25:04 +1100 (Mo, 23 Feb 2009) $ by $Author: schulte $
 *     $Revision: 8255 $
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

#include <gecode/gist/nodestats.hh>
#include <gecode/gist/nodewidget.hh>
#include <gecode/gist/nodecursor.hh>
#include <gecode/gist/nodevisitor.hh>

namespace Gecode { namespace Gist {

  NodeStatInspector::NodeStatInspector(void) {
    QVBoxLayout* b = new QVBoxLayout();

    QWidget* w = new QWidget();
    QHBoxLayout* hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new QLabel("Node depth:"));
    nodeDepthLabel = new QLabel("0");
    hbl->addWidget(nodeDepthLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    w = new QWidget();
    hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new QLabel("Subtree depth:"));
    subtreeDepthLabel = new QLabel("0");
    hbl->addWidget(subtreeDepthLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    w = new QWidget();
    hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new NodeWidget(SOLVED));
    solvedLabel = new QLabel("0");
    hbl->addWidget(solvedLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    w = new QWidget();
    hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new NodeWidget(FAILED));
    failedLabel = new QLabel("0");
    hbl->addWidget(failedLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    w = new QWidget();
    hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new NodeWidget(BRANCH));
    choicesLabel = new QLabel("0");
    hbl->addWidget(choicesLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    w = new QWidget();
    hbl = new QHBoxLayout();
    hbl->setContentsMargins(0,0,0,0);
    hbl->addWidget(new NodeWidget(UNDETERMINED));
    openLabel = new QLabel("0");
    hbl->addWidget(openLabel);
    w->setLayout(hbl);
    b->addWidget(w);

    setLayout(b);

    setWindowTitle("Gist Node info");
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, false);
  }

  void
  NodeStatInspector::node(VisualNode* n,const Statistics&, bool) {
    if (isVisible()) {
      int nd = -1;
      for (VisualNode* p = n; p != NULL; p = p->getParent())
        nd++;
      nodeDepthLabel->setNum(nd);
      StatCursor sc(n);
      PreorderNodeVisitor<StatCursor> pnv(sc);
      while (pnv.next()) {}
      subtreeDepthLabel->setNum(pnv.getCursor().depth);
      solvedLabel->setNum(pnv.getCursor().solved);
      failedLabel->setNum(pnv.getCursor().failed);
      choicesLabel->setNum(pnv.getCursor().choice);
      openLabel->setNum(pnv.getCursor().open);
    }
  }
  
  void
  NodeStatInspector::showStats(void) {
    show();
    activateWindow();
  }
  
}}

// STATISTICS: gist-any
