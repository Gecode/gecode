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

#include "gecode/gist/mainwindow.hh"

namespace Gecode { namespace Gist {
  
  GistMainWindow::GistMainWindow(Space* root, Better* b,
                                 Gist::Inspector* gi)
  : c(root,b,this) {
    if (gi != NULL)
      c.setInspector(gi);
    setCentralWidget(&c);
    setWindowTitle(tr("Gist"));
    resize(500,500);

    menuBar = new QMenuBar(0);

    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    fileMenu->addAction(c.print);
    fileMenu->addAction(c.exportPostscript);
    
    QMenu* nodeMenu = menuBar->addMenu(tr("&Node"));
    nodeMenu->addAction(c.inspectCN);
    nodeMenu->addAction(c.setPath);
    nodeMenu->addAction(c.inspectPath);
    nodeMenu->addSeparator();
    nodeMenu->addAction(c.navUp);
    nodeMenu->addAction(c.navDown);
    nodeMenu->addAction(c.navLeft);
    nodeMenu->addAction(c.navRight);
    nodeMenu->addAction(c.navRoot);
    nodeMenu->addSeparator();
    nodeMenu->addAction(c.toggleHidden);
    nodeMenu->addAction(c.hideFailed);
    nodeMenu->addAction(c.unhideAll);
    nodeMenu->addSeparator();
    nodeMenu->addAction(c.zoomToFit);
    nodeMenu->addAction(c.centerCN);
    
    QMenu* searchMenu = menuBar->addMenu(tr("&Search"));
    searchMenu->addAction(c.searchNext);
    searchMenu->addAction(c.searchAll);
    searchMenu->addSeparator();
    searchMenu->addAction(c.stopCN);
    searchMenu->addSeparator();
    searchMenu->addAction(c.reset);

    QMenu* toolsMenu = menuBar->addMenu(tr("&Tools"));
    toolsMenu->addAction(c.analyzeTree);
    toolsMenu->addSeparator();
    toolsMenu->addAction(c.toggleHeatView);
    
    // Set up status bar
    statisticsLabel = new QLabel("S: 0 F: 0 C: 0 U: 0");
    statusBar()->addPermanentWidget(statisticsLabel);
    
    isSearching = false;
    
    connect(&c,SIGNAL(statusChanged(const Statistics&,bool)),
            this,SLOT(statusChanged(const Statistics&,bool)));
    
    show();
  }

  GistMainWindow::~GistMainWindow(void) {
    delete menuBar;
  }

  void
  GistMainWindow::closeEvent(QCloseEvent* event) {
    c.finish();
    event->accept();
  }

  void
  GistMainWindow::statusChanged(const Statistics& stats, bool finished) {
    if (isSearching && finished) {
      statusBar()->showMessage("Ready");
      isSearching = false;
    } else if (!isSearching && !finished) {
      statusBar()->showMessage("Searching");
      isSearching = true;
    }
    statisticsLabel->setText(QString("S: %1 F: %2 C: %3 U: %4")
      .arg(stats.solutions, 4)
      .arg(stats.failures, 4)
      .arg(stats.choices, 4)
      .arg(stats.undetermined, 4));
  }

}}

// STATISTICS: gist-any
