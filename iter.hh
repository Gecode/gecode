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

#include "./iter/ranges-operations.icc"
#include "./iter/ranges-minmax.icc"

#include "./iter/ranges-append.icc"
#include "./iter/ranges-cache.icc"
#include "./iter/ranges-compl.icc"
#include "./iter/ranges-diff.icc"
#include "./iter/ranges-empty.icc"
#include "./iter/ranges-inter.icc"
#include "./iter/ranges-minus.icc"
#include "./iter/ranges-offset.icc"
#include "./iter/ranges-scale.icc"
#include "./iter/ranges-singleton.icc"
#include "./iter/ranges-union.icc"
#include "./iter/ranges-values.icc"
#include "./iter/ranges-add.icc"

#include "./iter/values-ranges.icc"

#endif

// STATISTICS: iter-any

