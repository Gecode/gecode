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

#ifndef GECODE_GIST_VISUALISATION_VARARRAYVIEWT_HH
#define GECODE_GIST_VISUALISATION_VARARRAYVIEWT_HH

#include <gecode/gist/visualisation/vararrayview.hh>

namespace Gecode { namespace Gist { namespace Visualisation {

  /// \brief Termplatized version of VarArrayView
  template<class VarItem>
  class VarArrayViewT : public VarArrayView {

  public:
    VarArrayViewT(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent = 0);

    static QWidget* create(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent = 0);

  protected:
    virtual void initT(QVector<Reflection::VarSpec*> specs);
    virtual void displayT(QVector<Reflection::VarSpec*> specs);
    virtual void displayOldT(int pit);

    VarArrayItem<VarItem> *varArrayItem;
  };

  template<class VarItem>
  VarArrayViewT<VarItem>::VarArrayViewT(Gecode::Reflection::VarMap& vm0, int pit, QStringList vars0, QWidget *parent)
  : VarArrayView(vm0, pit, vars0, parent)
  {
    init();
  }

  template<class VarItem>
  QWidget*
  VarArrayViewT<VarItem>::create(Gecode::Reflection::VarMap& vm, int pit, QStringList vars, QWidget *parent)
  {
    return new VarArrayViewT<VarItem>(vm, pit, vars, parent);
  }

  template<class VarItem>
  void
  VarArrayViewT<VarItem>::initT(QVector<Reflection::VarSpec*> specs) {
    varArrayItem = new VarArrayItem<VarItem>(specs);
    scene->addItem(varArrayItem);
  }

  template<class VarItem>
  void
  VarArrayViewT<VarItem>::displayT(QVector<Reflection::VarSpec*> specs) {
    varArrayItem->setVisible(true);
    varArrayItem->display(specs);
  }

  template<class VarItem>
  void
  VarArrayViewT<VarItem>::displayOldT(int pit) {

    if(pit < 0 || pit >= nextInternalPit) {
      varArrayItem->setVisible(false);
    }
    else {
      varArrayItem->setVisible(true);
      varArrayItem->displayOld(pit+1);
    }
  }

}}}

#endif

// STATISTICS: gist-any
