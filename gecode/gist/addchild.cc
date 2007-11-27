/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
 *
 *  Last modified:
 *     $Date: 2007-11-26 17:01:39 +0100 (Mon, 26 Nov 2007) $ by $Author: nikopp $
 *     $Revision: 5439 $
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

namespace Gecode { namespace Gist {

  AddChild::AddChild(Gecode::Reflection::VarMap& vm, QWidget *parent)
  : QDialog(parent)
  {
    ui.setupUi(this);

    ui.relList->insertItem(0, "==");
    ui.relList->insertItem(1, "!=");
    ui.relList->insertItem(2, "<=");
    ui.relList->insertItem(3, "<");
    ui.relList->insertItem(4, ">=");
    ui.relList->insertItem(5, ">");

    Gecode::Reflection::VarMapIter vmi(vm);

    for(int i = 0; vmi(); ++vmi, ++i) {
      ui.varList->insertItem(i, vmi.spec().name().toString().c_str());
    }
  }

  int
  AddChild::value(void) {
    return ui.valueSpinBox->value();
  }

  QString
  AddChild::var(void) {
    return ui.varList->currentItem()->text();
  }

  int
  AddChild::rel(void) {
    return ui.relList->currentRow();
  }

  void
  AddChild::on_varList_itemSelectionChanged(void) {
    if(ui.varList->selectedItems().isEmpty())
      ui.okPushButton->setEnabled(false);
    else
      ui.okPushButton->setEnabled(true);
  }

}}

// STATISTICS: gist-any
