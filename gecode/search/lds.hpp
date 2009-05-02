/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

namespace Gecode {

  namespace Search { namespace Sequential {
    /// Create sequential lds engine
    GECODE_SEARCH_EXPORT Engine* lds(Space* s, size_t sz, const Options& o);
  }}

#ifdef GECODE_HAS_THREADS
  namespace Search { namespace Parallel {
    /// Create parallel lds engine
    GECODE_SEARCH_EXPORT Engine* lds(Space* s, size_t sz, const Options& o);
  }}
#endif

  namespace Search {
    /// Create lds engine
    GECODE_SEARCH_EXPORT Engine* lds(Space* s, size_t sz, const Options& o);
  }

  template <class T>
  forceinline
  LDS<T>::LDS(T* s, const Search::Options& o)
    : e(Search::lds(s,sizeof(T),o)) {}

  template <class T>
  forceinline T*
  LDS<T>::next(void) {
    return dynamic_cast<T*>(e->next());
  }

  template <class T>
  forceinline Search::Statistics
  LDS<T>::statistics(void) const {
    return e->statistics();
  }

  template <class T>
  forceinline bool
  LDS<T>::stopped(void) const {
    return e->stopped();
  }

  template <class T>
  forceinline
  LDS<T>::~LDS(void) {
    delete e;
  }

  template <class T>
  T*
  lds(T* s, const Search::Options& o) {
    LDS<T> lds(s,o);
    return lds.next();
  }

}

// STATISTICS: search-any
