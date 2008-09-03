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

#include <QtGui>

#include <gecode/gist.hh>
#include <gecode/gist/treecanvas.hh>
#include <gecode/gist/textoutput.hh>
#include <gecode/gist/mainwindow.hh>

namespace Gecode {

  namespace Gist {

    /// Implementation of the TextInspector
    class TextInspector::TextInspectorImpl {
    public:
      /// The window where the text is output
      TextOutput* to;
      /// Constructor
      TextInspectorImpl(const std::string& name) {
        to = new TextOutput(name);
        to->show();
      }
      /// Destructor
      ~TextInspectorImpl(void) {
        delete to;
      }
    };
    
    TextInspector::TextInspector(const std::string& name)
    : t(NULL), n(name) {}

    TextInspector::~TextInspector(void) {
      delete t;
    }

    void
    TextInspector::init(void) {
      if (t == NULL) {
        t = new TextInspectorImpl(n);
      }
      t->to->setVisible(true);
    }

    std::ostream&
    TextInspector::getStream(void) {
      return t->to->getStream();
    }

    void
    TextInspector::addHtml(const char* s) {
      t->to->insertHtml(s);
    }

    QWidget*
    exploreWidget(QWidget* parent, Space* root, bool bab,
                  Gist::Inspector* gi) {
      if (root->status() == SS_FAILED)
        root = NULL;
      else
        root = root->clone();
      Gist::TreeCanvas *c = new Gist::TreeCanvas(root, bab, parent);
      if (gi)
        c->setInspector(gi);
      c->show();
      return c;
    }

    int
    explore(Space* root, bool bab, Gist::Inspector* gi) {
      char* argv = ""; int argc=0;
      QApplication app(argc, &argv);
      if (root->status() == SS_FAILED)
        root = NULL;
      else
        root = root->clone();
      Gist::GistMainWindow mw(root, bab, gi);
      return app.exec();
    }
    
  }
  
  void
  exploreWidget(QWidget* parent, Space* root, Gist::Inspector* gi) {
    (void) Gist::exploreWidget(parent, root, false, gi);
  }

  int
  explore(Space* root, Gist::Inspector* gi) {
    return Gist::explore(root, false, gi);
  }
  
  int
  exploreBest(Space* root, Gist::Inspector* gi) {
    return Gist::explore(root, true, gi);
  }

  void
  exploreBestWidget(QWidget* parent, Space* root, Gist::Inspector* gi) {
    (void) Gist::exploreWidget(parent, root, true, gi);
  }

}

// STATISTICS: gist-any
