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

#include "gecode/gist/addchild.hh"
#ifdef GECODE_HAVE_INT_VARS
#include "gecode/int.hh"
#endif
#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

namespace Gecode { namespace Gist {

// TODO nikopp: to make this usable for arbitrary kinds of variables, a variable must be able to
//              return the required values
  AddChild::AddChild(Reflection::VarMap& vm, QWidget *parent)
  : QDialog(parent)
  {
    ui.setupUi(this);

    Reflection::VarMapIter vmi(vm);

    for(int i = 0; vmi(); ++vmi, ++i) {
      QString desc = (vmi.spec().name().toString() + " (" + vmi.spec().vti().toString() + ")").c_str();
      QListWidgetItem* item = new QListWidgetItem(desc, ui.varList);

      QList<QVariant> data;
      data << QVariant(QString(vmi.spec().name().toString().c_str()));
      data << QVariant(QString(vmi.spec().vti().toString().c_str()));
      
      if(false) {
#ifdef GECODE_HAVE_INT_VARS
      } else if(vmi.spec().vti() == Int::IntVarImpConf::vti) {
        Reflection::IntArrayArg* dom = vmi.spec().dom()->toIntArray();
        data << QVariant((*dom)[0]) << QVariant((*dom)[dom->size()-1]);
#endif
#ifdef GECODE_HAVE_SET_VARS
      } else if(vmi.spec().vti() == Set::SetVarImpConf::vti) {
        Reflection::IntArrayArg* ub_dom = vmi.spec().dom()->second()->first()->toIntArray();
        data << QVariant((*ub_dom)[0]) << QVariant((*ub_dom)[ub_dom->size()-1]);
#endif
        } else {
        // TODO nikopp: implement other possibilities
      }

      item->setData(Qt::UserRole, data);
    }
  }

  int
  AddChild::value(void) {
    return ui.valueSpinBox->value();
  }

  QString
  AddChild::var(void) {
    return ui.varList->currentItem()->data(Qt::UserRole).toList().takeFirst().toString();
  }

  int
  AddChild::rel(void) {
    return ui.relList->currentItem()->data(Qt::UserRole).toInt();
  }

  void
  AddChild::on_varList_itemSelectionChanged(void) {
    refresh_relList();
  }

  void
  AddChild::on_relList_itemSelectionChanged(void) {
    refresh();
  }

  void
  AddChild::refresh(void) {
    if(ui.varList->selectedItems().isEmpty() ||
        ui.relList->selectedItems().isEmpty())
      ui.okPushButton->setEnabled(false);
    else {
      ui.okPushButton->setEnabled(true);
      updateValue();
    }
  }

  void
  AddChild::refresh_relList(void) {
    ui.relList->clear();
    
    if(ui.varList->selectedItems().isEmpty()) {
      return;
    }
    QVariantList data = ui.varList->currentItem()->data(Qt::UserRole).toList();
    Support::Symbol vti = data.at(1).toString().toStdString().c_str();
    
    if(false) {
#ifdef GECODE_HAVE_INT_VARS
    } else if(vti == Int::IntVarImpConf::vti) {
      QListWidgetItem* eq = new QListWidgetItem("==", ui.relList);
      eq->setData(Qt::UserRole, QVariant(Gecode::IRT_EQ));
      QListWidgetItem* nq = new QListWidgetItem("!=", ui.relList);
      nq->setData(Qt::UserRole, QVariant(Gecode::IRT_NQ));
      QListWidgetItem* lq = new QListWidgetItem("<=", ui.relList);
      lq->setData(Qt::UserRole, QVariant(Gecode::IRT_LQ));
      QListWidgetItem* le = new QListWidgetItem("<", ui.relList);
      le->setData(Qt::UserRole, QVariant(Gecode::IRT_LE));
      QListWidgetItem* gq = new QListWidgetItem(">=", ui.relList);
      gq->setData(Qt::UserRole, QVariant(Gecode::IRT_GQ));
      QListWidgetItem* gr = new QListWidgetItem(">", ui.relList);
      gr->setData(Qt::UserRole, QVariant(Gecode::IRT_GR));
#endif
#ifdef GECODE_HAVE_SET_VARS
    } else if (vti == Set::SetVarImpConf::vti) {
      QListWidgetItem* in = new QListWidgetItem("contains", ui.relList);
      in->setData(Qt::UserRole, QVariant(Gecode::SRT_SUP));
      QListWidgetItem* out = new QListWidgetItem("does not contain", ui.relList);
      out->setData(Qt::UserRole, QVariant(Gecode::SRT_DISJ));
#endif
    } else {
      // TODO nikopp: implement other options
    }
  }
  
  void
  AddChild::updateValue(void) {
    QVariantList data = ui.varList->selectedItems().first()->data(Qt::UserRole).toList();
    
    int min = data.at(2).toInt();
    int max = data.at(3).toInt();
    
    switch (ui.relList->currentRow()) {
    case 0:
    case 1:
      break;
    case 2:
    case 5:
      max--;
      break;
    case 3:
    case 4:
      min++;
      break;
    }
    
    if(min <= max) {
      ui.valueSpinBox->setMinimum(min);
      ui.valueSpinBox->setMaximum(max);
      ui.valueSpinBox->setEnabled(true);
    }
    else {
      ui.valueSpinBox->setEnabled(false);
      ui.okPushButton->setEnabled(false);
    }
  }

}}

// STATISTICS: gist-any
