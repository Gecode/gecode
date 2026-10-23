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

  /// Create a native CABS engine
  GECODE_SEARCH_EXPORT Engine*
  cabsengine(Space* s, const HeuristicGuidance& guidance, const Options& o,
             const CABSOptions& limits=CABSOptions());

  /// CABS engine builder
  template<class T>
  class CABSBuilder : public Builder {
    HeuristicGuidance guidance;
    CABSOptions limits;
  public:
    CABSBuilder(const HeuristicGuidance& g, const Options& o,
                const CABSOptions& l)
      : Builder(o,true), guidance(g), limits(l) {}
    Engine* operator()(Space* s) const override {
      return cabsengine(dynamic_cast<T*>(s),guidance,opt,limits);
    }
  };

  inline
  CABSOptions::CABSOptions(unsigned long int maximum_width,
                           unsigned long int frontier_nodes)
    : max_width(maximum_width), frontier_limit(frontier_nodes) {}

}}

namespace Gecode {

  template<class T>
  inline
  CABS<T>::CABS(T* s, const Search::HeuristicGuidance& g,
                const Search::Options& o, const Search::CABSOptions& limits)
    : Search::Base<T>(Search::cabsengine(s,g,o,limits)) {}

  template<class T>
  inline void
  CABS<T>::constrain(const Space& incumbent) {
    this->e->constrain(incumbent);
  }

  template<class T>
  inline T*
  cabs(T* s, const Search::HeuristicGuidance& g, const Search::Options& o,
       const Search::CABSOptions& limits) {
    CABS<T> engine(s,g,o,limits);
    std::unique_ptr<T> best;
    while (T* solution = engine.next())
      best.reset(solution);
    return best.release();
  }

  template<class T>
  inline SEB
  cabs(const Search::HeuristicGuidance& g, const Search::Options& o,
       const Search::CABSOptions& limits) {
    return new Search::CABSBuilder<T>(g,o,limits);
  }

}

#endif

// STATISTICS: search-other
