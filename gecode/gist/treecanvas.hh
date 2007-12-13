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

#ifndef GECODE_GIST_TREECANVAS_HH
#define GECODE_GIST_TREECANVAS_HH

#include "gecode/gist/gist.hh"
#include "gecode/gist/visualnode.hh"
#include <QtGui>
// TODO nikopp: this include is just to get proper syntax highlighting in Eclipse
#include <qobject.h>

#include <gecode/kernel.hh>

namespace Gecode {  namespace Gist {
  
  class TreeCanvasImpl;
  
  class SearcherThread : public QThread {
    Q_OBJECT
  private:
    VisualNode* node;
    bool a;
    TreeCanvasImpl* t;
  public:
    void search(VisualNode* n, bool all, TreeCanvasImpl* ti);
    
  Q_SIGNALS:
    void update(void);
    void statusChanged(bool);
    
  protected:
    void run(void);
  };

  class LayoutThread : public QThread {
    Q_OBJECT
  private:
    TreeCanvasImpl* t;
  public:
    void layout(TreeCanvasImpl* ti);
  Q_SIGNALS:
    void done(int,int);
  protected:
    void run(void);
  };

  /// \brief Implementation of the TreeCanvas
  class GECODE_GIST_EXPORT TreeCanvasImpl : public QWidget {
    Q_OBJECT

    friend class SearcherThread;
    friend class LayoutThread;

  public:
    /// Constructor
    TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent = NULL);
    /// Destructor
    ~TreeCanvasImpl(void);

    /// Set Inspector to \a i0
    void setInspector(Inspector* i);

  public Q_SLOTS:
    /// Set scale factor to \a scale0
    void scaleTree(int scale0);
    
    /// Explore complete subtree of selected node
    void searchAll(void);
    /// Find next solution below selected node
    void searchOne(void);
    /// Toggle hidden state of selected node
    void toggleHidden(void);
    /// Hide failed subtrees of selected node
    void hideFailed(void);
    /// Unhide all nodes below selected node
    void unhideAll(void);
    /// Export postscript of the tree
    void exportPostscript(void);
    /// Print the tree
    void print(void);
    /// Zoom the canvas so that the whole tree fits
    void zoomToFit(void);
    /// Center the view on the currently selected node
    void centerCurrentNode(void);
    /// Call the inspector for the currently selected node
    void inspectCurrentNode(void);
    
    /// Toggle the heat view display
    void toggleHeatView(void);
    
    /// Run the analysis that produces the heat view
    void analyzeTree(void);
    
    /// Stop current search
    void stopSearch(void);
    
    /// Reset
    void reset(void);
    
    /// Move selection to the parent of the selected node
    void navUp(void);
    /// Move selection to the first child of the selected node
    void navDown(void);
    /// Move selection to the left sibling of the selected node
    void navLeft(void);
    /// Move selection to the right sibling of the selected node
    void navRight(void);
    /// Move selection to the root node
    void navRoot(void);
    /// Recall selection of point in time \a i
    void markCurrentNode(int i);
    
    /// Set the current node to be the head of the path
    void setPath(void);
    /// Call the inspector for all nodes on the path from root to head of the path
    void inspectPath(void);

    /// Set preference whether to automatically hide failed subtrees
    void setAutoHideFailed(bool b);
    /// Set preference whether to automatically zoom to fit
    void setAutoZoom(bool b);
    /// Return preference whether to automatically hide failed subtrees
    bool getAutoHideFailed(void);
    /// Return preference whether to automatically zoom to fit
    bool getAutoZoom(void);
    /// Set refresh rate
    void setRefresh(int i);

#ifdef GECODE_GIST_EXPERIMENTAL
    /// Add a new special node as child to the current node via dialog
    void addChild(void);
    /// Add a new special node as child to the current node immediately
    void addChild(const QString&, Gecode::IntRelType, int);
    /// Add a new fixpoint node as child to the current node if it is a special node
    void addFixpoint(void);
    /// Calls getVars(\a vm) on the space of the root node and \i is set to the most recent point in time
    void getRootVars(Gecode::Reflection::VarMap& vm);
    /// Expand or collapse the current node
    void expandCurrentNode(void);
    /// Enable gdb debugging if the current node is a step node
    void toggleDebug(void);
#endif
    
    /// Stop search and wait for it to finish
    void finish(void);
    
  Q_SIGNALS:
    /// The scale factor has changed
    void scaleChanged(int);
    /// A new point in time was logged
    void newPointInTime(int);
    /// Context menu triggered
    void contextMenu(QContextMenuEvent*);
    /// Status bar update
    void statusChanged(const Statistics&, bool);
    /// The current node has been changed to \a n
    void currentNodeChanged(Gecode::Space*, Gecode::Gist::NodeStatus);
  protected:
    /// Mutex for synchronizing acccess to the tree
    QMutex mutex;
    /// Search engine thread
    SearcherThread searcher;
    /// Layout thread
    LayoutThread layouter;
    /// Flag signalling the search to stop
    bool stopSearchFlag;
    /// The root node of the tree
    VisualNode* root;
    /// The currently selected node
    VisualNode* currentNode;
    /// The head of the currently selected path
    VisualNode* pathHead;
    /// The history of inspected nodes
    QVector<VisualNode*> nodeMap;
    /// The active inspector
    Inspector* inspect;
    
    /// Statistics about the search tree
    Statistics stats;
    
    /// Current scale factor
    double scale;
    /// Offset on the x axis so that the tree is centered
    int xtrans;
    /// Whether to display the heat view
    bool heatView;

    /// Whether to hide failed subtrees automatically
    bool autoHideFailed;
    /// Whether to zoom automatically
    bool autoZoom;
    /// Refresh rate
    int refresh;

    /// Paint the tree
    void paintEvent(QPaintEvent* event);
    /// Handle mouse press event
    void mousePressEvent(QMouseEvent* event);
    /// Handle mouse double click event
    void mouseDoubleClickEvent(QMouseEvent* event);
    /// Handle context menu event
    void contextMenuEvent(QContextMenuEvent* event);
    /// Set the selected node to \a n
    void setCurrentNode(VisualNode* n);
    /// Log the current node as new point in time
    void saveCurrentNode(void);

  public Q_SLOTS:
    /// Update display
    void update(void);
    /// Layout done
    void layoutDone(int w, int h);
  private Q_SLOTS:
    /// Search has finished
    void statusChanged(bool);
  };
  
  /// Tree canvas widget
  class TreeCanvas : public QWidget {
    Q_OBJECT
  private:
    /// The canvas implementation
    TreeCanvasImpl* canvas;
    QSlider* timeBar;
    /// Context menu
    QMenu* contextMenu;

  public:
    QAction* inspectCN;
    QAction* stopCN;
    QAction* reset;
    QAction* navUp;
    QAction* navDown;
    QAction* navLeft;
    QAction* navRight;
    QAction* navRoot;

    QAction* searchNext;
    QAction* searchAll;
    QAction* toggleHidden;
    QAction* hideFailed;
    QAction* unhideAll;
    QAction* zoomToFit;
    QAction* centerCN;
    QAction* exportPostscript;
    QAction* print;

    QAction* setPath;
    QAction* inspectPath;

    QAction* toggleHeatView;
    QAction* analyzeTree;

#ifdef GECODE_GIST_EXPERIMENTAL
    QAction* addChild;
    QAction* addFixpoint;
    QAction* expandCurrentNode;
    QAction* forwardTimeStep;
    QAction* backwardTimeStep;
    QAction* toggleDebug;
#endif

  public:
    /// Constructor
    TreeCanvas(Space* root, Better* b = NULL, QWidget* parent = NULL);
    /// Destructor
    ~TreeCanvas(void);

    /// Set Inspector to \a i0
    void setInspector(Inspector* i0);

    /// Set preference whether to automatically hide failed subtrees
    void setAutoHideFailed(bool b);
    /// Set preference whether to automatically zoom to fit
    void setAutoZoom(bool b);
    /// Return preference whether to automatically hide failed subtrees
    bool getAutoHideFailed(void);
    /// Return preference whether to automatically zoom to fit
    bool getAutoZoom(void);
    /// Set refresh rate
    void setRefresh(int i);

    /// Stop search and wait until finished
    void finish(void);

  Q_SIGNALS:
    void statusChanged(const Statistics&, bool);

#ifdef GECODE_GIST_EXPERIMENTAL
    
  private Q_SLOTS:
    void on_canvas_newPointInTime(int);
    void on_forwardTimeStep_triggered(void);
    void on_backwardTimeStep_triggered(void);
#endif

  private Q_SLOTS:
    void on_canvas_contextMenu(QContextMenuEvent*);
    void on_canvas_statusChanged(const Statistics&, bool);
  protected:
    /// Close the widget
    void closeEvent(QCloseEvent* event);
  };
  
}}

#endif

// STATISTICS: gist-any
