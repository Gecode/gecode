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

#include "gecode/gist/visualisation/vararrayview.hh"

#include <QGraphicsTextItem>

namespace Gecode { namespace Gist { namespace Visualisation {

  VarArrayView::VarArrayView(Gecode::Reflection::VarMap& vm0, int pit, QStringList vars0, QWidget *parent)
  : QWidget(parent)
  , vm(vm0)
  , firstPointInTime(pit)
  , vars(vars0)
  {
    scene = new QGraphicsScene(this);
    
    view = new QGraphicsView(this);
    view->setScene(scene);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    timeBar = new QSlider(Qt::Horizontal, this);
    timeBar->setMinimum(firstPointInTime);
    timeBar->setMaximum(firstPointInTime);
    timeBar->setObjectName("visTimeBar");

    muteButton = new QPushButton("Mute", this);
    muteButton->setObjectName("muteButton");
    
    grid = new QGridLayout(this);
    grid->addWidget(view);
    grid->addWidget(timeBar);
    grid->addWidget(muteButton);
    
    setLayout(grid);
    
    connect(timeBar, SIGNAL(valueChanged(int)), this, SLOT(displayOld(int)));
    
    QMetaObject::connectSlotsByName(this);
  }

  void
  VarArrayView::init() {

    QVector<Reflection::VarSpec*> specs;
    QStringList _vars = vars;
    while(!_vars.empty()) {
      specs.push_back(new Reflection::VarSpec(vm.spec(_vars.takeFirst().toStdString().c_str())));
    }
    
   initT(specs);
  }
  
  // TODO nikopp: this ist not used yet and I am not sure if it is a good idea at all
  //              maybe one should just create a new visualisation instead of resetting
  //              the old one
  void
  VarArrayView::reset(void) {
    resetT();
  }

  void
  VarArrayView::displayOld(int pit) {
    displayOldT(pit);
    updateTimeBar(pit);
  }
  
  void
  VarArrayView::display(Gecode::Reflection::VarMap& _vm, int pit) {

    // TODO nikopp: use pit to implement 'gaps' when view is muted
    
    QVector<Reflection::VarSpec*> specs;
    QStringList _vars = vars;
    while(!_vars.empty()) {
      specs.push_back(new Reflection::VarSpec(_vm.spec(_vars.takeFirst().toStdString().c_str())));
    }
    
    displayT(specs);
    extendTimeBar(pit);
  }

  void
  VarArrayView::extendTimeBar(int pit) {
    timeBar->setMaximum(pit);
    timeBar->setValue(pit);
  }

  void
  VarArrayView::updateTimeBar(int pit) {
    timeBar->setValue(pit);
  }

  void
  VarArrayView::on_muteButton_clicked(void) {
    // TODO nikopp: implement functionality of mute button
  }
}}}

// STATISTICS: gist-any
