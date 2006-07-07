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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_ITER_HH__
#define __GECODE_ITER_HH__

namespace Gecode {
  /// Range and value iterators
  namespace Iter {
     /// Range iterators
     namespace Ranges {}
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

#include "gecode/iter/ranges-size.icc"

#include "gecode/iter/values-ranges.icc"

#include "gecode/iter/virtual-ranges.icc"
#include "gecode/iter/virtual-ranges-union.icc"
#include "gecode/iter/virtual-ranges-inter.icc"
#include "gecode/iter/virtual-ranges-compl.icc"

#endif

// STATISTICS: iter-any

