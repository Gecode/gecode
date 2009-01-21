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

#include <gecode/gist/visualisation/intvaritem.hh>

#include <QtGui/QBrush>

namespace Gecode { namespace Gist { namespace Visualisation {

  IntVarItem::ItemData
  IntVarItem::initData(QVector<Reflection::VarSpec*> specs) {
    ItemData data(Int::Limits::max);
    for (int i=0; i<specs.size(); i++) {
      if (! (specs[i]->vti() == Int::IntVarImpConf::vti))
        return ItemData(0);
      Reflection::IntArrayArg* dom = specs[i]->dom()->toIntArray();
      data = std::min(data, (*dom)[0]);
    }
    return data;
  }

  IntVarItem::IntVarItem(Reflection::VarSpec* spec, const ItemData& data,
                         QGraphicsItem *parent)
  : VarItem(spec, parent)
  {
    // this item is supposed to display an IntVar
    if(spec->vti() == Int::IntVarImpConf::vti) {
      Reflection::IntArrayArg* dom = spec->dom()->toIntArray();

      int domSize = dom->size();

      initMin = (*dom)[0];
      initMax = (*dom)[domSize-1];

      offset = initMin - data;

      arraylength = 2*(initMax - initMin + 1);

      QBitArray array(arraylength, false);
      updates.push(array);

      initGraphic();
    }
    else {
      arraylength = -1;
      new QGraphicsTextItem("not an IntVar", this);
      setRect(childrenBoundingRect());
    }
  }

  void
  IntVarItem::display(Reflection::VarSpec* spec) {
    if(arraylength < 0) {
      return;
    }
    VarItem::display(spec);
  }

  void
  IntVarItem::displayOld(int pit) {
    if(arraylength < 0) {
      return;
    }
    VarItem::displayOld(pit);
  }

  void
  IntVarItem::initGraphic(void) {

    int vectorSize = initMax - initMin +1;

    QGraphicsTextItem largest(QString::number(initMax));
    QRectF largestBound = largest.boundingRect();

    // make it a square
    if(largestBound.height() > largestBound.width())
      largestBound.setWidth(largestBound.height());
    else
      largestBound.setHeight(largestBound.width());

    domainItems.resize(vectorSize);

    for (int i = initMin, j=0; i <= initMax; ++i, ++j) {

      QGraphicsRectItem* item = new QGraphicsRectItem(largestBound, this);
      QGraphicsTextItem* number = new QGraphicsTextItem (QString::number(i),item);

      //centre the number in the square
      number->moveBy((item->boundingRect().width() - number->boundingRect().width())/2,
                     (item->boundingRect().height() - number->boundingRect().height())/2);
      if (j == 0)
        item->moveBy(offset*largestBound.width(),0);
      else
        item->moveBy(childrenBoundingRect().width(),0);
      domainItems[j] = item;
    }

    setRect(childrenBoundingRect());
  }

  void
  IntVarItem::store(Reflection::VarSpec* spec) {

    Reflection::IntArrayArg* dom = spec->dom()->toIntArray();

    int domSize = dom->size();
    bool assigned = (*dom)[0] == (*dom)[domSize-1];

    QBitArray array(arraylength, false);

    int i = initMin;
    int k = 0;
    int j = 0;

    while(k < domSize) {
      while(i < (*dom)[k]) {
        array.setBit(2*j,true);
        ++i;
        ++j;
      }
      ++k;
      i = (*dom)[k] + 1;
      j = i - initMin;
      ++k;
    }

    if(assigned) {
      array.setBit(2*(j-1),true);
      array.setBit(2*(j-1)+1,true);
    }

    while(i <= initMax) {
      array.setBit(2*j,true);
      ++i;
      ++j;
    }

    updates.push(array);
  }

  void
  IntVarItem::updateGraphic(void) {

    QBitArray array = updates[current];

    for (int i = 0; i <= initMax - initMin; ++i) {
      if(array.testBit(2*i))
        if(array.testBit(2*i + 1))
          domainItems[i]->setBrush(Qt::green);
        else
          domainItems[i]->setBrush(Qt::red);
      else
        domainItems[i]->setBrush(Qt::white);
    }
  }

}}}

// STATISTICS: gist-any
