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

// TODO nikopp: doxygen comments

#ifndef GECODE_GIST_ADDVISUALISATIONDIALOG_HH
#define GECODE_GIST_ADDVISUALISATIONDIALOG_HH

#include <QtGui/QDialog>

#include <gecode/kernel.hh>
#include <gecode/gist/ui_addvisualisationdialog.hh>
#include <gecode/gist/config.hh>

namespace Gecode { namespace Gist {

/// \brief Choose a visualisation for the currently explored Problem
class AddVisualisationDialog : public QDialog
{
  Q_OBJECT

public:
  AddVisualisationDialog(Gecode::Gist::Config conf,
                         Gecode::Reflection::VarMap& vm,
                         QWidget *parent = 0);

  QStringList vars(void);
  QString vis(void);
  QString name(void);

private:
  Ui::AddVisualisationDialogClass ui;

  void refresh(void);
  void updateName(void);

private Q_SLOTS:
  void on_variablesListWidget_itemSelectionChanged(void);
  void on_visualisationsListWidget_itemSelectionChanged(void);
};

#endif

}}

// STATISTICS: gist-any
