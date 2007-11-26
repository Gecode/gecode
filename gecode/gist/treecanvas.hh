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

#ifndef GECODE_GIST_QT_TREECANVAS_HH
#define GECODE_GIST_QT_TREECANVAS_HH

#include "gecode/gist/gist.hh"
#include <QtGui>

namespace Gecode {
  
  class Space;
  
  namespace Gist {
  
  class VisualNode;
  
  /// \brief Implementation of the TreeCanvas
  class TreeCanvasImpl : public QWidget {
    Q_OBJECT
    
  public:
    /// Constructor
    TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent = NULL);
    /// Destructor
    ~TreeCanvasImpl(void);

    /// Set Inspector to \a i0
    void setInspector(Inspector* i);
  public slots:
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
    /// Export postscript for selected node
    void exportPostscript(void);
    /// Zoom the canvas so that the whole tree fits
    void zoomToFit(void);
    /// Center the view on the currently selected node
    void centerCurrentNode(void);
    /// Call the inspector for the currently selected node
    void inspectCurrentNode(void);
    
    /// Move selection to the parent of the selected node
    void navUp(void);
    /// Move selection to the first child of the selected node
    void navDown(void);
    /// Move selection to the left sibling of the selected node
    void navLeft(void);
    /// Move selection to the right sibling of the selected node
    void navRight(void);
    
  signals:
    /// The scale factor has changed
    void scaleChanged(int);
    
  protected:
    /// The root node of the tree
    VisualNode* root;
    /// The currently selected node
    VisualNode* currentNode;
    /// The active inspector
    Inspector* inspect;

    /// Context menu
    QMenu* contextMenu;
    
    /// Current scale factor
    double scale;
    /// Offset on the x axis so that the tree is centered
    int xtrans;

    /// Update display
    void update(void);
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
  };
  
  /// Tree canvas widget
  class TreeCanvas : public QWidget {
  private:
    /// The canvas implementation
    TreeCanvasImpl* canvas;
  public:
    /// Constructor
    TreeCanvas(Space* root, Better* b = NULL, QWidget* parent = NULL);
    /// Destructor
    ~TreeCanvas(void);
    /// Set Inspector to \a i0
    void setInspector(Inspector* i0);
  };
  
}}

#endif

// STATISTICS: gist-any
