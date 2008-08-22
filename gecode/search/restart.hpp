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

  /*
   * Control for restart best solution search engine
   *
   */

  template <class T>
  Restart<T>::Restart(T* s, const Search::Options& o) :
    Gecode::DFS<T>(s,o),
    root(s->status() == SS_FAILED ? NULL : s->clone()), best(NULL) {
    // Failure and propagation must not be counted, has happened already
    // in the DFS constructor
  }

  template <class T>
  forceinline
  Restart<T>::~Restart(void) {
    delete best;
    delete root;
  }

  template <class T>
  forceinline T*
  Restart<T>::next(void) {
    if (best != NULL) {
      root->constrain(*best);
      this->e.clone++;
      if (root->status(this->e.propagate) == SS_FAILED) {
        this->e.reset();
      } else {
        this->e.reset(root->clone());
      }
    }
    Space* b = this->e.explore();
    delete best;
    best = (b != NULL) ? b->clone() : NULL;
    if (b == NULL)
      return NULL;
    T* dcb = dynamic_cast<T*>(b);
    if (dcb == NULL)
      throw DynamicCastFailed("Restart");
    return dcb;
  }




  /*
   * Restart convenience
   *
   */

  template <class T>
  T*
  restart(T* s, const Search::Options& o) {
    Restart<T> b(s,o);
    T* l = NULL;
    while (T* n = b.next()) {
      delete l; l = n;
    }
    return l;
  }

}

// STATISTICS: search-any
