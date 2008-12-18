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

#include <QtGui>

#include <gecode/kernel.hh>
#include <gecode/gist.hh>

#include <gecode/gist/visualnode.hh>

namespace Gecode {  namespace Gist {

  class LayoutConfig {
  public:
    static const int minScale = 10;
    static const int maxScale = 400;
    static const int defScale = 100;
    static const int maxAutoZoomScale = defScale;    
  };
  
  class TreeCanvas;
  
  class SearcherThread : public QThread {
    Q_OBJECT
  private:
    VisualNode* node;
    bool a;
    TreeCanvas* t;
    void updateCanvas(void);
  public:
    void search(VisualNode* n, bool all, TreeCanvas* ti);
    
  Q_SIGNALS:
    void update(int w, int h, int scale0);
    void statusChanged(bool);
    void scaleChanged(int);
    
  protected:
    void run(void);
  };

  /// \brief Implementation of the TreeCanvas
  class GECODE_GIST_EXPORT TreeCanvas : public QWidget {
    Q_OBJECT

    friend class SearcherThread;
    friend class Gist;

  public:
    /// Constructor
    TreeCanvas(Space* rootSpace, bool bab, QWidget* parent = NULL);
    /// Destructor
    ~TreeCanvas(void);

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
    /// Export pdf of the current subtree
    void exportPDF(void);
    /// Export pdf of the whole tree
    void exportWholeTreePDF(void);
    /// Print the tree
    void print(void);
    /// Zoom the canvas so that the whole tree fits
    void zoomToFit(void);
    /// Center the view on the currently selected node
    void centerCurrentNode(void);
    /// Call the inspector for the currently selected node
    void inspectCurrentNode(void);
        
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
    /// Move selection to next solution (in DFS order)
    void navNextSol(bool back = false);
    /// Move selection to previous solution (in DFS order)
    void navPrevSol(void);
    /// Recall selection of point in time \a pit
    void markCurrentNode(int pit);
    
    /// Set the current node to be the head of the path
    void setPath(void);
    /// Call the inspector for all nodes on the path from root to head of the path
    void inspectPath(void);

    /// Set recomputation distances
    void setRecompDistances(int c_d, int a_d);
    /// Set preference whether to automatically hide failed subtrees
    void setAutoHideFailed(bool b);
    /// Set preference whether to automatically zoom to fit
    void setAutoZoom(bool b);
    /// Return preference whether to automatically hide failed subtrees
    bool getAutoHideFailed(void);
    /// Return preference whether to automatically zoom to fit
    bool getAutoZoom(void);
    /// Set preference whether to show copies in the tree
    void setShowCopies(bool b);
    /// Return preference whether to show copies in the tree
    bool getShowCopies(void);
    /// Set refresh rate
    void setRefresh(int i);
    /// Return preference wheter to use smooth scrolling and zooming
    bool getSmoothScrollAndZoom(void);
    /// Set preference wheter to use smooth scrolling and zooming
    void setSmoothScrollAndZoom(bool b);
    /// Resize to the outer widget size if auto zoom is enabled
    void resizeToOuter(void);

    /// Calls getVars(\a vm) on the space of the root node
    void getRootVars(Gecode::Reflection::VarMap& vm);
    /// Add a visualisation of type \a visType for \a vars
    void addVisualisation(QStringList vars, QString visType, QString windowName = "");
    /// Add a visualisation via a dialog
    void addVisualisation(void);

    /// Stop search and wait for it to finish
    void finish(void);
    
  Q_SIGNALS:
    /// The scale factor has changed
    void scaleChanged(int);
    /// The auto-zoom state was changed
    void autoZoomChanged(bool);
    /// Context menu triggered
    void contextMenu(QContextMenuEvent*);
    /// Status bar update
    void statusChanged(VisualNode*,const Statistics&, bool);
    /// The node with space \a s is inspected at point in time \a pit
    void inspect(Gecode::Reflection::VarMap& vm, int pit);
    /// The point in time changed to \a pit
    void pointInTimeChanged(int pit);
  protected:
    /// Mutex for synchronizing acccess to the tree
    QMutex mutex;
    /// Mutex for synchronizing layout and drawing
    QMutex layoutMutex;
    /// Search engine thread
    SearcherThread searcher;
    /// Flag signalling the search to stop
    bool stopSearchFlag;
    /// Allocator for nodes
    Node::NodeAllocator* na;
    /// The root node of the tree
    VisualNode* root;
    /// The currently best solution (for branch-and-bound)
    BestNode* curBest;
    /// The currently selected node
    VisualNode* currentNode;
    /// The head of the currently selected path
    VisualNode* pathHead;
    /// The history of inspected nodes
    QVector<VisualNode*> nodeMap;
    /// The active inspector
    Inspector* inspector;
    
    /// The scale bar
    QSlider* scaleBar;
    
    /// Statistics about the search tree
    Statistics stats;
    
    /// Current scale factor
    double scale;
    /// Offset on the x axis so that the tree is centered
    int xtrans;

    /// Whether to hide failed subtrees automatically
    bool autoHideFailed;
    /// Whether to zoom automatically
    bool autoZoom;
    /// Whether to show copies in the tree
    bool showCopies;
    /// Refresh rate
    int refresh;
    /// Whether to use smooth scrolling and zooming
    bool smoothScrollAndZoom;

    /// The recomputation distance
    int c_d;
    /// The adaptive recomputation distance
    int a_d;

    /// The next point in time
    int nextPit;

    /// Return the node corresponding to the \a event position
    VisualNode* eventNode(QEvent *event);
    /// General event handler, used for displaying tool tips
    bool event(QEvent *event);
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

    /// Target zoom value for smooth zooming
    int targetZoom;
    /// Meta current zoom value
    double metaZoomCurrent;
    /// Timer id for smooth zooming
    int zoomTimerId;

    /// Target x coordinate for smooth scrolling
    int targetScrollX;
    /// Target y coordinate for smooth scrolling
    int targetScrollY;
    /// Meta current x coordinate
    double metaScrollXCurrent;
    /// Meta current y coordinate
    double metaScrollYCurrent;
    /// Timer id for smooth scrolling
    int scrollTimerId;

    /// Timer invoked for smooth zooming and scrolling
    virtual void timerEvent(QTimerEvent* e);

  public Q_SLOTS:
    /// Update display
    void update(void);
    /// Layout done
    void layoutDone(int w, int h, int scale0);
  private Q_SLOTS:
    /// Search has finished
    void statusChanged(bool);
    /// Export PDF of the subtree of \a n
    void exportNodePDF(VisualNode* n);
  };
    
}}

#endif

// STATISTICS: gist-any
