/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
 *
 *  Last modified:
 *     $Date: 2007-11-29 19:46:48 +0100 (Thu, 29 Nov 2007) $ by $Author: schulte $
 *     $Revision: 5506 $
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

#ifndef GECODE_GIST_VISUALISATION_INTVARARRAYVIEW_HH
#define GECODE_GIST_VISUALISATION_INTVARARRAYVIEW_HH

#include <QtGui/QGraphicsView>
#include <QtCore/QString>

#include <gecode/minimodel.hh>

#include "intvararrayitem.hh"

namespace Gecode { namespace Gist { namespace Visualisation {

class IntVarArrayView : public QGraphicsView {

  Q_OBJECT

public:
  IntVarArrayView(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent = 0);
  
  static QWidget* create(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent = 0);

public Q_SLOTS:
  void display(Gecode::Reflection::VarMap&, int pit);
  void displayOld(int pit); ///< Use to show the variable at point in time pit
  void reset(void); ///< Use to reset the view

private:
  void init(void);

  QGraphicsScene *scene;
  IntVarArrayItem *intVarArrayItem;
  Gecode::Reflection::VarMap& vm;
  int firstPointInTime;
  QStringList vars;
};

}}}

#endif

// STATISTICS: gist-any
