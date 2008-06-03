/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

  /*
   * BAB search engine
   *
   */

  namespace Search {

    forceinline
    BabEngine::BabEngine(unsigned int c_d0, unsigned int a_d,
                         Stop* st, size_t sz)
      : EngineCtrl(st,sz), rcs(a_d), cur(NULL),
        mark(0), best(NULL),
        c_d(c_d0), d(0) {}


    forceinline void
    BabEngine::init(Space* s) {
      cur = s;
    }

    forceinline size_t
    BabEngine::stacksize(void) const {
      return rcs.stacksize();
    }

    forceinline
    BabEngine::~BabEngine(void) {
      rcs.reset();
      delete best;
      delete cur;
    }
  }

  /*
   * Control for bab search engine
   *
   */

  template <class T>
  forceinline
  BAB<T>::BAB(T* s, const Search::Options& o)
    : Search::BAB(s,o.c_d,o.a_d,o.stop,sizeof(T)) {}

  template <class T>
  forceinline T*
  BAB<T>::next(void) {
    Space* s = e.explore();
    if (s == NULL)
      return NULL;
    T* t = dynamic_cast<T*>(s);
    if (t == NULL)
      throw DynamicCastFailed("BAB");
    return t;
  }




  /*
   * BAB convenience
   *
   */

  template <class T>
  T*
  bab(T* s, const Search::Options& o) {
    BAB<T> b(s,o);
    T* l = NULL;
    while (T* n = b.next()) {
      delete l; l = n;
    }
    return l;
  }

}

// STATISTICS: search-any
