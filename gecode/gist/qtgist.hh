/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2008-09-03 14:14:11 +0200 (Mi, 03 Sep 2008) $ by $Author: tack $
 *     $Revision: 7787 $
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

#ifndef GECODE_GIST_QTGIST_HH
#define GECODE_GIST_QTGIST_HH

#include <gecode/gist/treecanvas.hh>

namespace Gecode {  namespace Gist {

  /// Tree canvas widget
  class Gist : public QWidget {
    Q_OBJECT
  private:
    /// The canvas implementation
    TreeCanvas* canvas;
    /// The time slider
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
    QAction* navNextSol;
    QAction* navPrevSol;

    QAction* searchNext;
    QAction* searchAll;
    QAction* toggleHidden;
    QAction* hideFailed;
    QAction* unhideAll;
    QAction* zoomToFit;
    QAction* centerCN;
    QAction* exportPDF;
    QAction* exportWholeTreePDF;
    QAction* print;

    QAction* setPath;
    QAction* inspectPath;
    QAction* addVisualisation;

  public:
    /// Constructor
    Gist(Space* root, bool bab = false, QWidget* parent = NULL);
    /// Destructor
    ~Gist(void);

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

    /// Set recomputation parameters \a c_d and \a a_d
    void setRecompDistances(int c_d, int a_d);
    /// Return recomputation distance
    int getCd(void);
    /// Return adaptive recomputation distance
    int getAd(void);

    /// Stop search and wait until finished
    void finish(void);

    /// Handle resize event
    void resizeEvent(QResizeEvent*);

  Q_SIGNALS:
    void statusChanged(const Statistics&, bool);

  private Q_SLOTS:
    void on_canvas_contextMenu(QContextMenuEvent*);
    void on_canvas_statusChanged(VisualNode*, const Statistics&, bool);
  protected:
    /// Close the widget
    void closeEvent(QCloseEvent* event);
  };

}}

#endif

// STATISTICS: gist-any
