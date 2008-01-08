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

#include "gecode/gist/visualisation/intvararrayview.hh"

#include <QGraphicsTextItem>

namespace Gecode { namespace Gist { namespace Visualisation {

  IntVarArrayView::IntVarArrayView(Gecode::Reflection::VarMap& vm0, int pit, QStringList vars0, QWidget *parent)
  : QGraphicsView(parent)
  , intVarArrayItem(NULL)
  , vm(vm0)
  , firstPointInTime(pit)
  , vars(vars0)
  {
    scene = new QGraphicsScene(this);
    setScene(scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    init();
  }

  QWidget*
  IntVarArrayView::create(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent) {
    return new IntVarArrayView(vm, pit, vars, parent);
  }

  void
  IntVarArrayView::init() {

    QVector<Reflection::VarSpec*> specs;
    QStringList _vars = vars;
    while(!_vars.empty()) {
      specs.push_back(new Reflection::VarSpec(vm.spec(_vars.takeFirst().toStdString().c_str())));
    }
    
   intVarArrayItem = new IntVarArrayItem(specs);
   scene->addItem(intVarArrayItem);
   
   setFixedSize(sceneRect().size().toSize() + QSize(5,5));
  }

  void IntVarArrayView::reset(void) {

    scene->removeItem(intVarArrayItem);
    delete intVarArrayItem;
    
    init();
  }

  void IntVarArrayView::displayOld(int pit) {

    if(pit < firstPointInTime)
      intVarArrayItem->setVisible(false);
    else
      intVarArrayItem->setVisible(true);
    
    intVarArrayItem->displayOld(pit - firstPointInTime + 1);
  }

  void IntVarArrayView::display(Gecode::Reflection::VarMap& _vm, int pit) {

    // TODO nikopp: use pit to implement 'gaps' when view is muted
    
    QVector<Reflection::VarSpec*> specs;
    QStringList _vars = vars;
    while(!_vars.empty()) {
      specs.push_back(new Reflection::VarSpec(_vm.spec(_vars.takeFirst().toStdString().c_str())));
    }
    
    intVarArrayItem->display(specs);
  }

}}}

// STATISTICS: gist-any
