/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2012
 *
 *  Last modified:
 *     $Date: 2012-08-29 12:29:14 +0200 (Wed, 29 Aug 2012) $ by $Author: schulte $
 *     $Revision: 13015 $
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

#include <gecode/search/sequential/restart.hh>

namespace Gecode {

  namespace Search {
    GECODE_SEARCH_EXPORT Engine* restart(Space* s, size_t sz,
                                         MetaStop* stop,
                                         Engine* e,
                                         const Options& o);
  }

  template<template<class> class E, class T>
  forceinline
  Restart<E,T>::Restart(T* s, const Search::Options& o) {
    Search::Options o_single;
    o_single.clone = true;
    o_single.threads = 1.0;
    o_single.c_d = o.c_d;
    o_single.a_d = o.a_d;
    Search::MetaStop* rs = new Search::MetaStop(o.stop);
    o_single.stop = rs;
    E<T> engine(s,o_single);
    EngineBase* eb = &engine;
    Search::Engine* ee = eb->e;
    eb->e = NULL;
    e = Search::restart(s,sizeof(T),rs,ee,o);
  }

  template<template<class> class E, class T>
  forceinline T*
  Restart<E,T>::next(void) {
    return dynamic_cast<T*>(e->next());
  }

  template<template<class> class E, class T>
  forceinline Search::Statistics
  Restart<E,T>::statistics(void) const {
    return e->statistics();
  }

  template<template<class> class E, class T>
  forceinline bool
  Restart<E,T>::stopped(void) const {
    return e->stopped();
  }

}

// STATISTICS: search-other
