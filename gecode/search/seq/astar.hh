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

#ifndef GECODE_SEARCH_SEQ_ASTAR_HH
#define GECODE_SEARCH_SEQ_ASTAR_HH

#include <gecode/search.hh>
#include <gecode/search/seq/choice-expansion.hh>
#include <memory>
#include <vector>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search { namespace Seq {

  /// Sequential tree search with owned stable frontier clones
  class AStar : public Worker {
  public:
    struct Configuration {
      HeuristicGuidance guidance;
      AStarOrder ordering;
    };
  private:
    struct Entry {
      std::unique_ptr<Space> space;
      std::int64_t bound;
      std::int64_t rank;
      unsigned long int depth;
      unsigned long long int order;
      unsigned long long int incumbent_revision;
    };
    /// Heap comparison with fixed exploration order
    struct Later {
      HeuristicGuidance::Sense sense;
      AStarOrder ordering;
      bool operator()(const Entry& a, const Entry& b) const;
    };
    Options opt;
    HeuristicGuidance guidance;
    AStarOrder ordering;
    std::vector<Entry> frontier;
    unsigned long long int order;
    /// Revision of the currently accepted incumbent
    unsigned long long int incumbent_revision;
    /// Parent and next alternative, retained across returned solutions and stops
    ChoiceExpansion expansion;
    SpaceStatus current_status;
    unsigned long int current_depth;
    /// Incumbent revision whose constraints are already present in the parent
    unsigned long long int current_revision;
    bool counted;
    std::unique_ptr<Space> best;
    std::int64_t best_bound;

    bool improves(std::int64_t bound) const;
    void enqueue(std::unique_ptr<Space> s, unsigned long int depth,
                 unsigned long long int revision);
    Space* publish(std::unique_ptr<Space> s);
    void initialize(Space* s, bool clone);
  public:
    AStar(Space* s, const Options& o, const Configuration& c);
    Space* next(void);
    Statistics statistics(void) const;
    void constrain(const Space& b);
    void reset(Space* s);
    NoGoods& nogoods(void);
  };

}}}

#endif

#endif

// STATISTICS: search-seq
