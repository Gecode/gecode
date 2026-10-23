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

  /// Create a native A* engine
  GECODE_SEARCH_EXPORT Engine*
  astarengine(Space* s, const HeuristicGuidance& guidance, const Options& o);

  /// Create a native best-first engine with fixed exploration order
  GECODE_SEARCH_EXPORT Engine*
  bestfirstengine(Space* s, const HeuristicGuidance& guidance,
                  AStarOrder ordering, const Options& o);

  /// A* engine builder for PBS portfolios
  template<class T>
  class AStarBuilder : public Builder {
    HeuristicGuidance guidance;
    AStarOrder ordering;
  public:
    AStarBuilder(const HeuristicGuidance& g, const Options& o,
                 AStarOrder order=AStarOrder::BOUND_FIRST)
      : Builder(o,true), guidance(g), ordering(order) {}
    Engine* operator()(Space* s) const override {
      return bestfirstengine(dynamic_cast<T*>(s),guidance,ordering,opt);
    }
  };

  inline
  HeuristicGuidance::HeuristicGuidance(Sense s, Evaluate b, Evaluate r)
    : sense(s), bound(std::move(b)), rank(std::move(r)) {}

}}

namespace Gecode {

  template<class T>
  inline
  AStar<T>::AStar(T* s, const Search::HeuristicGuidance& g,
                  const Search::Options& o)
    : Search::Base<T>(Search::astarengine(s,g,o)) {}

  template<class T>
  inline void
  AStar<T>::constrain(const Space& incumbent) {
    this->e->constrain(incumbent);
  }

  template<class T>
  inline T*
  astar(T* s, const Search::HeuristicGuidance& g, const Search::Options& o) {
    AStar<T> engine(s,g,o);
    std::unique_ptr<T> best;
    while (T* solution = engine.next())
      best.reset(solution);
    return best.release();
  }

  template<class T>
  inline
  AStar<T>::AStar(T* s, const Search::HeuristicGuidance& g,
                  Search::AStarOrder ordering, const Search::Options& o)
    : Search::Base<T>(Search::bestfirstengine(s,g,ordering,o)) {}

  template<class T>
  inline T*
  astar(T* s, const Search::HeuristicGuidance& g,
        Search::AStarOrder ordering, const Search::Options& o) {
    AStar<T> engine(s,g,ordering,o);
    std::unique_ptr<T> best;
    while (T* solution = engine.next())
      best.reset(solution);
    return best.release();
  }

  template<class T>
  inline SEB
  astar(const Search::HeuristicGuidance& g, const Search::Options& o) {
    return new Search::AStarBuilder<T>(g,o);
  }

  template<class T>
  inline SEB
  astar(const Search::HeuristicGuidance& g, Search::AStarOrder ordering,
        const Search::Options& o) {
    return new Search::AStarBuilder<T>(g,o,ordering);
  }

}

#endif

// STATISTICS: search-other
