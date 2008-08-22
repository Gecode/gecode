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

#include "gecode/gist/addvisualisationdialog.hh"

namespace Gecode { namespace Gist {

  AddVisualisationDialog::AddVisualisationDialog(Gecode::Gist::Config conf, 
                                                 Gecode::Reflection::VarMap& vm,
                                                 QWidget *parent)
  : QDialog(parent)
  {
    ui.setupUi(this);

    QStringList visNamesList = conf.visualisationMap.keys();
    ui.visualisationsListWidget->insertItems(0, visNamesList);

    Gecode::Reflection::VarMapIter vmi(vm);

    for(int i = 0; vmi(); ++vmi, ++i) {
      QString varName = vmi.spec().name().toString().c_str();
      QString itemName(varName);
      itemName.append(" (");
      itemName.append(vmi.spec().vti().toString().c_str());
      itemName.append(")");
      QListWidgetItem* item = new QListWidgetItem(itemName, ui.variablesListWidget);
      item->setData(Qt::UserRole, QVariant(varName));
    }
  }

  QStringList
  AddVisualisationDialog::vars(void) {
    QList<QListWidgetItem*> itemList = ui.variablesListWidget->selectedItems();
    QStringList varList;
    while(!itemList.isEmpty()) {
      varList << itemList.takeFirst()->data(Qt::UserRole).toString();
    }
    return varList;
  }

  QString
  AddVisualisationDialog::vis(void) {
    return ui.visualisationsListWidget->currentItem()->text();
  }

  QString
  AddVisualisationDialog::name(void) {
    return ui.nameEdit->text();
  }

  void
  AddVisualisationDialog::on_variablesListWidget_itemSelectionChanged(void) {
    updateName();
    refresh();
  }

  void
  AddVisualisationDialog::on_visualisationsListWidget_itemSelectionChanged(void) {
    refresh();
  }

  void
  AddVisualisationDialog::refresh(void) {
    if(ui.variablesListWidget->selectedItems().isEmpty() ||
        ui.visualisationsListWidget->selectedItems().isEmpty())
      ui.OKpushButton->setEnabled(false);
    else
      ui.OKpushButton->setEnabled(true);
  }

  void
  AddVisualisationDialog::updateName(void) {

    QList<QListWidgetItem*> items = ui.variablesListWidget->selectedItems();

    if(items.count() == 0) {
      ui.nameEdit->setText("");
    }

    if(items.count() == 1) {
      ui.nameEdit->setText(items.first()->text());
    }

    else if(items.count() > 1) {

      QStringList varNames;

      for (int i = 0; i < items.count(); ++i) {
        varNames << items[i]->text();
      }

      varNames.sort();

      QString first = varNames.first();
      QString last = varNames.last();

      while(!last.startsWith(first))
        first.truncate(first.count() - 1);

      ui.nameEdit->setText(first);
    }
  }

}}

// STATISTICS: gist-any
