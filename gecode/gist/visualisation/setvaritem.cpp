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

#include <gecode/support.hh>

#ifdef GECODE_HAS_SET_VARS
#include <QtGui/QBrush>

#include <gecode/gist/visualisation/setvaritem.hh>
#include <gecode/set.hh>

namespace Gecode { namespace Gist { namespace Visualisation {

  SetVarItem::ItemData
  SetVarItem::initData(QVector<Reflection::VarSpec*> specs) {
    ItemData data(Set::Limits::max);
    for (int i=0; i<specs.size(); i++) {
      if (! (specs[i]->vti() == Set::SetVarImpConf::vti))
        return ItemData(0);
      Reflection::IntArrayArg* dom =
        specs[i]->dom()->second()->first()->toIntArray();
      data = std::min(data, (*dom)[0]);
    }
    return data;
  }

  SetVarItem::SetVarItem(Reflection::VarSpec* spec, const ItemData& data,
                         QGraphicsItem *parent)
  : VarItem(spec, parent)
{
  // this item is supposed to display a SetVar
  if(spec->vti() == Set::SetVarImpConf::vti) {

    int lb_card = spec->dom()->first()->second()->toInt();
    int ub_card = spec->dom()->second()->second()->toInt();

    Reflection::IntArrayArg* lower_bound_dom = spec->dom()->first()->first()->toIntArray();
    Reflection::IntArrayArg* upper_bound_dom = spec->dom()->second()->first()->toIntArray();

    int lb_domSize = lower_bound_dom->size();
    int ub_domSize = upper_bound_dom->size();

    if(ub_domSize > 0) {
      initMin = (*upper_bound_dom)[0];
      initMax = (*upper_bound_dom)[ub_domSize-1];
    }
    if(lb_domSize > 0) {
      initMin = std::min(initMin, (*lower_bound_dom)[0]);
      initMax = std::max(initMax, (*lower_bound_dom)[lb_domSize-1]);
    }
    arraylength = 2*(initMax - initMin + 1);
    offset = initMin - data;

    QBitArray array(arraylength, false);
    updates.push(array);
    bound_updates.push(lb_card);
    bound_updates.push(ub_card);

    initGraphic();
  }
  else {
    arraylength = -1;
    new QGraphicsTextItem("not a SetVar", this);
    setRect(childrenBoundingRect());
  }
}

  void
  SetVarItem::display(Reflection::VarSpec* spec) {
    if(arraylength < 0) {
      return;
    }
    VarItem::display(spec);
  }

  void
  SetVarItem::displayOld(int pit) {
    if(arraylength < 0) {
      return;
    }
    VarItem::displayOld(pit);
  }

  void
  SetVarItem::initGraphic(void) {
    int vectorSize = initMax - initMin +1;

    QGraphicsTextItem largest (QString::number(initMax));
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

    int maxCard = bound_updates[1];
    int minCard = bound_updates[0];
    QGraphicsRectItem* cardItem = new QGraphicsRectItem(this);
    cardText = new QGraphicsTextItem("# " + QString::number(maxCard) + "-" + QString::number(maxCard), cardItem);

    cardItem->setRect(cardItem->childrenBoundingRect());
    cardItem->moveBy(childrenBoundingRect().width(),0);
    cardText->setPlainText("# " + QString::number(minCard) + "-" + QString::number(maxCard));

    setRect(childrenBoundingRect());
  }

  void
  SetVarItem::store(Reflection::VarSpec* spec) {
    Reflection::IntArrayArg* lower_bound_dom = spec->dom()->first()->first()->toIntArray();
    Reflection::IntArrayArg* upper_bound_dom = spec->dom()->second()->first()->toIntArray();

    int lb_domSize = lower_bound_dom->size();
    int ub_domSize = upper_bound_dom->size();

    QBitArray array(arraylength, false);

    int i = initMin;
    int j = 0;
    int k_lb = 0;
    int k_ub = 0;

    while(k_ub < ub_domSize) {
      while(i < (*upper_bound_dom)[k_ub]) {
        array.setBit(2*j,true);
        ++i;
        ++j;
      }
      ++k_ub;
      while(k_lb < lb_domSize && (*lower_bound_dom)[k_lb] <= (*upper_bound_dom)[k_ub]) {
        i = (*lower_bound_dom)[k_lb];
        j = i - initMin;
        ++k_lb;
        while(i <= (*lower_bound_dom)[k_lb]) {
          array.setBit(2*j,true);
          array.setBit(2*j+1,true);
          ++i;
          ++j;
        }
        ++k_lb;
      }
      i = (*upper_bound_dom)[k_ub] + 1;
      j = i - initMin;
      ++k_ub;
    }

    while(i <= initMax) {
      array.setBit(2*j,true);
      ++i;
      ++j;
    }

    int lb_card = spec->dom()->first()->second()->toInt();
    int ub_card = spec->dom()->second()->second()->toInt();

    updates.push(array);
    bound_updates.push(lb_card);
    bound_updates.push(ub_card);
  }

  void
  SetVarItem::updateGraphic(void) {
    QBitArray array = updates[current];
    int lb_card = bound_updates[2*current];
    int ub_card = bound_updates[2*current+1];

    for (int i = 0; i <= initMax - initMin; ++i) {
      if(array.testBit(2*i))
        if(array.testBit(2*i + 1))
          domainItems[i]->setBrush(Qt::green);
        else
          domainItems[i]->setBrush(Qt::red);
      else
        domainItems[i]->setBrush(Qt::white);
    }

    cardText->setPlainText("# " + QString::number(lb_card) + "-" + QString::number(ub_card));
  }

}}}

#endif

// STATISTICS: gist-any
