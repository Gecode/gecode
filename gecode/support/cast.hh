/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#ifndef __GECODE_SUPPORT_CAST_HH__
#define __GECODE_SUPPORT_CAST_HH__

namespace Gecode { namespace Support {

  /// Cast \a p into pointer of type \a T
  template <class T>
  T ptr_cast(void* p);

  template <class T>
  forceinline T
  ptr_cast(void* p) {
    return static_cast<T>(p);
  }

}}

#endif

// STATISTICS: support-any
