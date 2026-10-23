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

#ifndef GECODE_SEARCH_PAR_FRONTIER_HH
#define GECODE_SEARCH_PAR_FRONTIER_HH

#include <gecode/search.hh>
#include <gecode/search/seq/choice-expansion.hh>
#include <gecode/search/beam-heap.hh>
#include <deque>
#include <exception>
#include <memory>
#include <vector>

#if defined(GECODE_HAS_EXPERIMENTAL) && defined(GECODE_HAS_THREADS)
namespace Gecode { namespace Search { namespace Par {

  /// Shared frontier and worker lifecycle for native A* and CABS
  class Frontier : public Search::Engine {
    using Entry=BeamHeap::Entry;
    struct Earlier {
      HeuristicGuidance::Sense sense;
      AStarOrder ordering;
      bool beam;
      bool ranked;
      bool operator()(const Entry& a, const Entry& b) const;
    };
    struct ExpansionTask {
      Seq::ChoiceExpansion expansion;
      unsigned long int depth=0;
      unsigned long long int revision=0;
      bool counted=false;
    };
    class Runner : public Support::Runnable, public Support::Terminator {
      Frontier& engine;
      unsigned int id;
    public:
      Support::Event wake;
      Support::Event done;
      Runner(Frontier& e, unsigned int i);
      void run(void) override;
      Support::Terminator* terminator(void) const override;
      void terminated(void) override;
    };
    Options opt;
    HeuristicGuidance guidance;
    AStarOrder ordering;
    bool beam;
    CABSOptions limits;
    unsigned long int width=1;
    unsigned long long int order=0;
    unsigned long long int revision=0;
    std::unique_ptr<Space> root;
    std::unique_ptr<Space> best;
    std::unique_ptr<Space> solution;
    std::int64_t best_bound=0;
    std::optional<std::int64_t> discarded_bound;
    std::vector<Entry> frontier;
    BeamHeap candidates;
    std::deque<std::unique_ptr<ExpansionTask>> pending;
    std::vector<std::unique_ptr<Runner>> runners;
    mutable Support::Mutex mutex;
    Support::Event changed;
    Search::Statistics stat;
    /// Workers with selected parents or model state still awaiting disposal
    unsigned int active=0;
    bool running=false;
    bool shutdown=false;
    bool stopped_=false;
    bool limited=false;
    std::exception_ptr error;

    void initialize(Space* s, bool clone);
    void launch(void);
    void wake(void);
    void work(unsigned int id);
    /// Select work or advance a beam layer/pass while holding mutex
    std::unique_ptr<ExpansionTask> select_task(unsigned int id);
    bool improves(std::int64_t bound) const;
    bool advance(void);
    /// Return a rejected or replaced space for disposal by its worker.
    std::unique_ptr<Space> admit(Entry entry, bool parent);
    void publish(std::unique_ptr<Space> s, std::int64_t bound);
    Earlier earlier(void) const;
  public:
    Frontier(Space* s, const HeuristicGuidance& g, AStarOrder ordering,
             const Options& o, bool beam, const CABSOptions& limits=CABSOptions());
    ~Frontier(void) override;
    Space* next(void) override;
    Search::Statistics statistics(void) const override;
    bool stopped(void) const override;
    void constrain(const Space& s) override;
    void reset(Space* s) override;
  };

}}}
#endif
#endif

// STATISTICS: search-par
