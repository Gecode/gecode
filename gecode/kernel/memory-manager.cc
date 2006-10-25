/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *      <duchier@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Denys Duchier, 2002
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

#include "gecode/kernel.hh"

namespace Gecode {

  void
  MemoryManager::alloc_refill(size_t sz) {
    // Try to reuse the not used memory
    reuse(start,lsz);
    alloc_fill(sz);
  }

}

// STATISTICS: kernel-core


