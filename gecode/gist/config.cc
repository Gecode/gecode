/*
 *  Main authors:
 *     Niko Paltzer <nikopp@ps.uni-sb.de>
 *
 *  Copyright:
 *     Niko Paltzer, 2007
 *
 *  Last modified:
 *     $Date: 2007-11-29 19:46:48 +0100 (Thu, 29 Nov 2007) $ by $Author: schulte $
 *     $Revision: 5506 $
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

#include "gecode/gist/config.hh"

//#include "intvarview.h"
#include "gecode/gist/visualisation/intvararrayview.hh"
//#include "boolvarview.h"
//#include "boolvararrayview.h"

namespace Gecode { namespace Gist {

  Config::Config(void)
    {
      // TODO nikopp: make the visualisations available after the
      //              interface has been implemented (see IntVarArray)
//      visualisationMap.insert("IntVar", &IntVarView::create);
      visualisationMap.insert("IntVarArray", &Visualisation::IntVarArrayView::create);
//      visualisationMap.insert("BoolVar", &BoolVarView::create);
//      visualisationMap.insert("BoolVarArray", &BoolVarArrayView::create);
    }

}}

// STATISTICS: gist-any
