/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
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

#include "gecode/gist/visualisation/vararrayview.hh"

#include <QGraphicsTextItem>

namespace Gecode { namespace Gist { namespace Visualisation {

  VarArrayView::VarArrayView(Gecode::Reflection::VarMap& vm0, int pit, QStringList vars0, QWidget *parent)
  : QWidget(parent)
  , vm(vm0)
  , muted(false)
  , nextInternalPit(0)
  , vars(vars0)
  , pitMap(2*pit+1, -1)
  {
    scene = new QGraphicsScene(this);

    view = new QGraphicsView(this);
    view->setScene(scene);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    
    timeBar = new QSlider(Qt::Horizontal, this);
    timeBar->setMinimum(pit);
    timeBar->setMaximum(pit);
    timeBar->setObjectName("visTimeBar");

    muteButton = new QPushButton("Mute", this);
    muteButton->setObjectName("muteButton");
    
    grid = new QGridLayout(this);
    grid->addWidget(view, 0, 0, 1, 2);
    grid->addWidget(timeBar, 1, 0);
    grid->addWidget(muteButton, 1, 1);
    
    setLayout(grid);
    
    connect(timeBar, SIGNAL(valueChanged(int)), this, SIGNAL(pointInTimeChanged(int)));
    
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
  
  void
  VarArrayView::displayOld(int pit) {
    if(!muted) {
      displayOldT(pitMap[pit]);
      updateTimeBar(pit);
    }
  }
  
  void
  VarArrayView::display(Gecode::Reflection::VarMap& _vm, int pit) {
    if(pitMap.size() <= pit) {
      pitMap.resize(pitMap.size() + pit + 1);
    }
    // TODO nikopp: use pit to implement 'gaps' when view is muted
    if(muted) {
      pitMap[pit] = -1;
    }
    if(!muted) {
      pitMap[pit] = nextInternalPit ++;
      
      QVector<Reflection::VarSpec*> specs;
      QStringList _vars = vars;
      while(!_vars.empty()) {
        specs.push_back(new Reflection::VarSpec(_vm.spec(_vars.takeFirst().toStdString().c_str())));
      }

      displayT(specs);
      extendTimeBar(pit);
    }
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
    muted = !muted;
    if(muted) {
      muteButton->setText("Unmute");
      timeBar->setEnabled(false);
    }
    else {
      muteButton->setText("Mute");
      timeBar->setEnabled(true);
    }
  }

}}}

// STATISTICS: gist-any
