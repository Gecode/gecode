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

#ifndef __GECODE_SUPPORT_SEARCH_HH__
#define __GECODE_SUPPORT_SEARCH_HH__

#include "gecode/support.hh"

#include <algorithm>

namespace Gecode { namespace Support {
  const bool search_debug = true;
#define derr if (search_debug) std::cerr
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
  binarysearch(Type* x, int n, const Key& key, LessThan &lt, int pos = -1) {
#if 0
    int low = 0;
    int high = n - 1;

    derr << "Searching for " << key << std::endl;

    while (low < high) {
      int mid = low + ((high - low) / 2);
      if (search_debug) {
        for (int i = 0; i < n; ++i) {
          derr << (i==low ? "," : " ");
          derr << lt(x[i], key);
          derr << (i==high ? "'" : " ");
        }
        derr << std::endl << " ";
        for (int i = 0; i < mid; ++i) derr << "   ";
        derr << "M" << std::endl;
      }

      if (lt(x[mid], key)) {
        // x[mid] < key
        derr << "x[mid] < key" << std::endl;
        high = mid - 1;
      } else if (lt(key, x[mid])) {
        // key < x[mid] 
        derr << "key < x[mid]" << std::endl;
        low = mid + 1;
      } else {
        derr << "key <> x[mid]" << std::endl;
        if (mid == 0)
          return x+mid; // first occurence of key found at pos 0
        else if (lt(x[mid-1], x[mid]))
          return x+mid; // first occurence of key found
        high = mid;
      }
    }
    

    return NULL;  // key not found.
#else
    for (int i = 0; i < n; ++i) {
      if (lt(x[i], key)) continue;
      if (!lt(key, x[i])) {
        return x+i;
      } else
        return NULL;
    }
    return NULL;
#endif
  }
#undef derr
}}

#endif

// STATISTICS: support-any
