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

namespace Gecode { namespace Search {

  /// Create a native sequential A* engine
  GECODE_SEARCH_EXPORT Engine*
  astarengine(Space* s, const HeuristicGuidance& guidance, const Options& o,
              const StatePruning& pruning=StatePruning());

  /// Create a native sequential best-first engine with fixed exploration order
  GECODE_SEARCH_EXPORT Engine*
  bestfirstengine(Space* s, const HeuristicGuidance& guidance,
                  BestFirstOrder ordering, const Options& o,
                  const StatePruning& pruning=StatePruning());

  /// A* and best-first engine builder
  template<class T>
  class AStarBuilder : public Builder {
    HeuristicGuidance guidance;
    StatePruning pruning;
    BestFirstOrder ordering;
  public:
    AStarBuilder(const HeuristicGuidance& g, const Options& o,
                 const StatePruning& p=StatePruning(),
                 BestFirstOrder order=BOUND_FIRST)
      : Builder(o,true), guidance(g), pruning(p), ordering(order) {}
    Engine* operator()(Space* s) const override {
      return bestfirstengine(dynamic_cast<T*>(s),guidance,ordering,opt,pruning);
    }
  };

  inline
  HeuristicGuidance::HeuristicGuidance(Sense s, Evaluate b, Evaluate r)
    : sense(s), bound(std::move(b)), rank(std::move(r)) {}

  inline
  StatePruning::StatePruning(Extract e, Compare c, unsigned long int limit)
    : extract(std::move(e)), compare(std::move(c)), descriptor_limit(limit) {}

}}

namespace Gecode {

  template<class T>
  inline
  AStar<T>::AStar(T* s, const Search::HeuristicGuidance& g,
                  const Search::Options& o, const Search::StatePruning& p)
    : Search::Base<T>(Search::astarengine(s,g,o,p)) {}

  template<class T>
  inline void
  AStar<T>::constrain(const Space& incumbent) {
    this->e->constrain(incumbent);
  }

  template<class T>
  inline SEB
  astar(const Search::HeuristicGuidance& g, const Search::Options& o,
        const Search::StatePruning& p) {
    return new Search::AStarBuilder<T>(g,o,p);
  }

  template<class T>
  inline SEB
  bestfirst(const Search::HeuristicGuidance& g, Search::BestFirstOrder ordering,
            const Search::Options& o, const Search::StatePruning& p) {
    return new Search::AStarBuilder<T>(g,o,p,ordering);
  }

}

// STATISTICS: search-other
