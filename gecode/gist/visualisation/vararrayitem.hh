/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#ifndef GECODE_GIST_VISUALISATION_VARARRAYITEM_HH
#define GECODE_GIST_VISUALISATION_VARARRAYITEM_HH

#include <QtGui/QGraphicsRectItem>
#include <QtCore/QVector>

#include <gecode/minimodel.hh>
#include <gecode/gist/visualisation/varitem.hh>

namespace Gecode { namespace Gist { namespace Visualisation {

  /// \brief Visualisation item for variable arrays
  template<class VarItem>
  class VarArrayItem : public QGraphicsRectItem {

  public:
    VarArrayItem(QVector<Reflection::VarSpec*> specs, QGraphicsItem *parent = 0);

    void display(QVector<Reflection::VarSpec*> specs); ///< Use to commit an update
    void displayOld(int pit); ///< Use to show the variable at point in time pit

  protected:
    void initGraphic(QVector<Reflection::VarSpec*> specs);

    QVector<VarItem*> varItems;
    int numberOfVariables;
    int numberOfUpdates;
    int current; ///< Number of actually drawn item
  };

  template<class VarItem>
  VarArrayItem<VarItem>::VarArrayItem(QVector<Reflection::VarSpec*> specs, QGraphicsItem *parent)
  : QGraphicsRectItem(parent)
  , numberOfUpdates(0)
  {
    numberOfVariables = specs.size();
    initGraphic(specs);
  }

  template<class VarItem>
  void
  VarArrayItem<VarItem>::display(QVector<Reflection::VarSpec*> specs) {

    for (int i = 0; i < numberOfVariables; ++i) {
      varItems[i]->display(specs[i]);
    }
    current = ++numberOfUpdates;
  }

  template<class VarItem>
  void
  VarArrayItem<VarItem>::displayOld(int pit) {

    if(pit > numberOfUpdates || pit < 0) {
      return;
    }

    for (int j = 0; j < numberOfVariables; ++j) {
      varItems[j]->displayOld(pit);
    }

    current = pit;
  }

  template<class VarItem>
  void
  VarArrayItem<VarItem>::initGraphic(QVector<Reflection::VarSpec*> specs) {

    varItems.resize(numberOfVariables);

    typename VarItem::ItemData data = VarItem::initData(specs);

    for (int i = 0; i < numberOfVariables; ++i) {

      VarItem* item = new VarItem(specs[i], data, this);

      if(i>0)
        item->moveBy(0, childrenBoundingRect().height());

      varItems[i] = item;
    }

    setRect(childrenBoundingRect());
  }

}}}

#endif

// STATISTICS: gist-any
