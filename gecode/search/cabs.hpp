/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search {

  /// Create CABS engine
  GECODE_SEARCH_EXPORT Engine*
  cabsengine(Space* s, const Options& o);

  /// A CABS engine builder
  template<class T>
  class CABSBuilder : public Builder {
    using Builder::opt;
  public:
    /// The constructor
    CABSBuilder(const Options& opt);
    /// The actual build function
    virtual Engine* operator() (Space* s) const;
  };

  template<class T>
  inline
  CABSBuilder<T>::CABSBuilder(const Options& opt)
    : Builder(opt,CABS<T>::best) {}

  template<class T>
  Engine*
  CABSBuilder<T>::operator() (Space* s) const {
    return build<T,CABS>(s,opt);
  }

}}

namespace Gecode {

  template<class T>
  inline
  CABS<T>::CABS(T* s, const Search::Options& o)
    : Search::Base<T>(Search::cabsengine(s,o)) {}

  template<class T>
  inline T*
  cabs(T* s, const Search::Options& o) {
    CABS<T> c(s,o);
    T* l = nullptr;
    while (T* n = c.next()) {
      delete l; l = n;
    }
    return l;
  }

  template<class T>
  SEB
  cabs(const Search::Options& o) {
    return new Search::CABSBuilder<T>(o);
  }

}

#endif

// STATISTICS: search-other
