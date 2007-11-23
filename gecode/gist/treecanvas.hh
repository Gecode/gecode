/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-08-04 16:06:52 +0200 (Fri, 04 Aug 2006) $ by $Author: schulte $
 *     $Revision: 3517 $
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
	
	class TreeCanvasImpl : public QWidget {
		Q_OBJECT
		
	public:
		TreeCanvasImpl(Space* rootSpace, Better* b, QWidget* parent = NULL);
		~TreeCanvasImpl(void);

    void setInspector(Inspector* i);
  public slots:
    void scaleTree(int scale0);
    
    void searchAll(void);
    void searchOne(void);
    void toggleHidden(void);
    void hideFailed(void);
    void unhideAll(void);
    void exportPostscript(void);
    void zoomToFit(void);
    void centerCurrentNode(void);

    void inspectCurrentNode(void);
    void navUp(void);
    void navDown(void);
    void navLeft(void);
    void navRight(void);
    
  signals:
    void scaleChanged(int);
    
  protected:
    VisualNode* root;
    VisualNode* currentNode;
    Inspector* inspect;

    QMenu* contextMenu;
    
    double scale;
    int xtrans;

    void update(void);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    
    void setCurrentNode(VisualNode* n);
  };
  
  class TreeCanvas : public QWidget {
  private:
    TreeCanvasImpl* canvas;
  public:
    TreeCanvas(Space* root, Better* b = NULL, QWidget* parent = NULL);
    ~TreeCanvas(void);
    
    void setInspector(Inspector* i0);
  };
  
}}

#endif
