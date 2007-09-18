/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __GECODE_ITER_HH__
#define __GECODE_ITER_HH__

namespace Gecode {
  /// Range and value iterators
  namespace Iter {
     /// Range iterators
     namespace Ranges {
       /// Range iterators with virtual member functions
       namespace Virt {}
     }
     /// Value iterators
     namespace Values {}
  }
}

#include "gecode/iter/ranges-operations.icc"
#include "gecode/iter/ranges-minmax.icc"

#include "gecode/iter/ranges-append.icc"
#include "gecode/iter/ranges-array.icc"
#include "gecode/iter/ranges-cache.icc"
#include "gecode/iter/ranges-compl.icc"
#include "gecode/iter/ranges-diff.icc"
#include "gecode/iter/ranges-empty.icc"
#include "gecode/iter/ranges-inter.icc"
#include "gecode/iter/ranges-minus.icc"
#include "gecode/iter/ranges-offset.icc"
#include "gecode/iter/ranges-scale.icc"
#include "gecode/iter/ranges-singleton.icc"
#include "gecode/iter/ranges-union.icc"
#include "gecode/iter/ranges-values.icc"
#include "gecode/iter/ranges-add.icc"
#include "gecode/iter/ranges-singleton-append.icc"

#include "gecode/iter/values-array.icc"
#include "gecode/iter/values-minus.icc"
#include "gecode/iter/values-offset.icc"
#include "gecode/iter/values-ranges.icc"
#include "gecode/iter/values-singleton.icc"
#include "gecode/iter/values-unique.icc"

#include "gecode/iter/ranges-size.icc"

#include "gecode/iter/virtual-ranges.icc"
#include "gecode/iter/virtual-ranges-union.icc"
#include "gecode/iter/virtual-ranges-inter.icc"
#include "gecode/iter/virtual-ranges-compl.icc"

#endif

// STATISTICS: iter-any

