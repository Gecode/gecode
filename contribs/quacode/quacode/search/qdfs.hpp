/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/search/dfs.hpp
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

  namespace Search {
    /// Create depth-first engine for quantified variables
    QUACODE_EXPORT Engine* qdfs(Space* s, const Options& o);
  }

  template<class T>
  forceinline
  QDFS<T>::QDFS(T* s, const Search::Options& o)
    : e(Search::qdfs(s,o)) {}

  template<class T>
  forceinline T*
  QDFS<T>::next(void) {
    return dynamic_cast<T*>(e->next());
  }

  template<class T>
  forceinline Search::Statistics
  QDFS<T>::statistics(void) const {
    return e->statistics();
  }

  template<class T>
  forceinline bool
  QDFS<T>::stopped(void) const {
    return e->stopped();
  }

  template<class T>
  forceinline
  QDFS<T>::~QDFS(void) {
    delete e;
  }


  template<class T>
  forceinline T*
  qdfs(T* s, const Search::Options& o) {
    QDFS<T> d(s,o);
    return d.next();
  }

}
