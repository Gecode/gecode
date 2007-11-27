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

#ifndef GECODE_GIST_ADDCHILD_HH
#define GECODE_GIST_ADDCHILD_HH

#include <QtGui/QDialog>
#include "gecode/gist/ui_addchild.hh"

#include "gecode/kernel.hh"

namespace Gecode { namespace Gist {

  class AddChild : public QDialog
  {
    Q_OBJECT

  public:
    AddChild(Gecode::Reflection::VarMap& vm, QWidget *parent = 0);

    int value(void);
    QString var(void);
    int rel(void);

  private Q_SLOTS:
    void on_varList_itemSelectionChanged(void);

  private:
    Ui::AddChildClass ui;
  };

}}

#endif

// STATISTICS: gist-any
