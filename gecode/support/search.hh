/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Miakel Lagerkvist, 2007
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

#ifndef __GECODE_SUPPORT_SEARCH_HH__
#define __GECODE_SUPPORT_SEARCH_HH__

#include <algorithm>

namespace Gecode { namespace Support {
  /**
   * \brief Binary search.
   *
   * Searches in an array \a x (\a n elements long) for the first
   * occurence of the element \a e. The order of the elements are
   * described by the ordering \a lt.
   *
   * If the key is not found in the array, NULL is returned.
   *
   * Requires \code #include "gecode/support/search.hh" \endcode
   * \ingroup FuncSupport
   */
  template <class Type, class Key, class LessThan>
  forceinline Type*
  binarysearch(Type* x, int n, const Key& key, LessThan &lt) {
    int low = 0;
    int high = n - 1;
    
    while (low <= high) {
      int mid = low + ((high - low) / 2);
      
      if (lt(x[mid], key)) {
        low = mid + 1;
      } else if (lt(key, x[mid])) {
        high = mid - 1;
      } else {
        if (lt(x[mid-1], x[mid]))
          return x+mid; // first occurence of key found
        high = mid;
      }
    }

    return NULL;  // key not found.
  }

}}

#endif

// STATISTICS: support-any
