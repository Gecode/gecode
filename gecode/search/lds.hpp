/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004, 2016
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

namespace Gecode { namespace Search {

  /// Create lds engine
  GECODE_SEARCH_EXPORT Engine*
  ldsengine(Space* s, const Options& o);

  /// A DFS engine builder
  template<class T>
  class LdsBuilder : public Builder {
    using Builder::opt;
  public:
    /// The constructor
    LdsBuilder(const Options& opt);
    /// The actual build function
    virtual Engine* operator() (Space* s) const;
  };

  template<class T>
  inline
  LdsBuilder<T>::LdsBuilder(const Options& opt)
    : Builder(opt,LDS<T>::best) {}

  template<class T>
  Engine*
  LdsBuilder<T>::operator() (Space* s) const {
    return build<T,LDS>(s,opt);
  }


}}

namespace Gecode {

  template<class T>
  forceinline
  LDS<T>::LDS(T* s, const Search::Options& o)
    : Search::Base<T>(Search::ldsengine(s,o)) {}

  template<class T>
  T*
  lds(T* s, const Search::Options& o) {
    LDS<T> lds(s,o);
    return lds.next();
  }

  template<class T>
  SEB
  lds(const Search::Options& o) {
    return new Search::LdsBuilder<T>(o);
  }

}

// STATISTICS: search-other
