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

#ifndef GECODE_GIST_VISUALISATION_VARITEM_HH
#define GECODE_GIST_VISUALISATION_VARITEM_HH

#include <QtGui/QGraphicsRectItem>

#include <gecode/minimodel.hh>

namespace Gecode { namespace Gist { namespace Visualisation {

/// \brief Base class for visualisation of variables
class VarItem : public QGraphicsRectItem {
  
public:
  VarItem(Reflection::VarSpec* spec, QGraphicsItem *parent = 0);

  void display(Reflection::VarSpec* spec);
  void displayOld(int pit); ///< Use to show the variable at point in time \a pit
  void reset(void);

protected:
  virtual void updateGraphic(void) = 0;
  virtual void store(Reflection::VarSpec* spec) = 0; ///< store the information of the variable on the stack

  int current; ///< Number of actually drawn item
  int noOfUpdates; ///< Number of received updates
};

}}}

#endif

// STATISTICS: gist-any
