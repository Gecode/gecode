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
#include <gecode/search/heuristic.hh>
#include <gecode/search/seq/choice-expansion.hh>

#ifdef GECODE_HAS_EXPERIMENTAL
namespace Gecode { namespace Search { namespace Seq {

  /// Sequential A* search
  class AStar : public Worker {
  private:
    /// Search options
    Options opt;
    /// Guidance
    HeuristicGuidance guidance;
    /// Open nodes
    Queue queue;
    /// Number of created entries
    unsigned long long int order;
    /// Node being expanded
    ChoiceExpansion expansion;
    /// Status of the root node before it is counted
    SpaceStatus status;
    /// Depth of the node being expanded
    unsigned long int depth;
    /// Incumbent revision the node being expanded is constrained with
    unsigned long long int revision;
    /// Whether the node being expanded has been counted
    bool counted;
    /// Best solution so far
    Incumbent best;
    /// Add node \a s at depth \a d if it can improve the incumbent
    void enqueue(std::unique_ptr<Space> s, unsigned long int d);
    /// Make \a s the incumbent and return a copy, if it improves
    Space* publish(std::unique_ptr<Space> s);
    /// Start search from root \a s
    void initialize(Space* s, bool clone);
  public:
    /// Initialize for space \a s with options \a o
    AStar(Space* s, const Options& o);
    /// Return next better solution (nullptr if none or stopped)
    Space* next(void);
    /// Return statistics
    Statistics statistics(void) const;
    /// Constrain future solutions to be better than \a b
    void constrain(const Space& b);
    /// Reset engine to restart at space \a s
    void reset(Space* s);
    /// Return no-goods (none)
    NoGoods& nogoods(void);
  };

}}}

#endif

#endif

// STATISTICS: search-seq
