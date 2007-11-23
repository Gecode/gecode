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

#include "qtgist/gist.hh"
#include "qtgist/treecanvas.hh"
#include <QtGui>
#include "gecode/kernel.hh"

namespace Gecode {

  namespace Gist {

  	QWidget*
  	exploreWidget(QWidget* parent, Space* root, Better* b,
  	              Gist::Inspector* gi) {
      if (root->status() == SS_FAILED)
        root = NULL;
      else
        root = root->clone();
      Gist::TreeCanvas *c = new Gist::TreeCanvas(root, b, parent);
  		if (gi)
  		  c->setInspector(gi);
  		c->show();
      return c;
  	}

  	int
  	explore(Space* root, Better* b, Gist::Inspector* gi) {
  		char* argv = ""; int argc=0;
  		QApplication app(argc, &argv);
      if (root->status() == SS_FAILED)
        root = NULL;
      else
        root = root->clone();
      Gist::TreeCanvas c(root, b);
  		if (gi)
  		  c.setInspector(gi);
  		c.show();
  		return app.exec();
  	}
    
  }
	
	void
	exploreWidget(QWidget* parent, Space* root, Gist::Inspector* gi) {
    (void) Gist::exploreWidget(parent, root, NULL, gi);
	}

	int
	explore(Space* root, Gist::Inspector* gi) {
    return Gist::explore(root, NULL, gi);
	}
	
}
