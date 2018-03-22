/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

  forceinline
  Builder::Builder(const Options& opt0, bool b0)
    : opt(opt0.expand()), b(b0) {}
  forceinline Options&
  Builder::options(void) {
    return opt;
  }
  forceinline const Options&
  Builder::options(void) const {
    return opt;
  }
  forceinline bool
  Builder::best(void) const {
    return b;
  }
  forceinline
  Builder::~Builder(void) {
  }


  template<class T, class E>
  forceinline Engine*
  build(Space* s, const Options& opt) {
    E engine(dynamic_cast<T*>(s),opt);
    Base<T>* eb = &engine;
    Engine* e = eb->e;
    eb->e = NULL;
    return e;
  }

  template<class T, template<class> class E>
  forceinline Engine*
  build(Space* s, const Options& opt) {
    E<T> engine(dynamic_cast<T*>(s),opt);
    Base<T>* eb = &engine;
    Engine* e = eb->e;
    eb->e = NULL;
    return e;
  }

}}

// STATISTICS: search-other
