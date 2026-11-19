/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <gecode/search/worker-control.hh>
#ifdef GECODE_HAS_THREADS
#include <gecode/search/par/bab.hh>
#endif

#include "test/test.hh"

#include <atomic>
#include <chrono>
#include <cmath>
#include <future>
#include <limits>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(std::is_copy_constructible<Gecode::NoGoods>::value,
              "NoGoods must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::NoGoods>::value,
              "NoGoods must remain copy assignable");
static_assert(std::is_copy_constructible<Gecode::Search::NodeStop>::value,
              "NodeStop must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::Search::NodeStop>::value,
              "NodeStop must remain copy assignable");
static_assert(std::is_copy_constructible<Gecode::Search::FailStop>::value,
              "FailStop must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::Search::FailStop>::value,
              "FailStop must remain copy assignable");
static_assert(std::is_copy_constructible<Gecode::Search::TimeStop>::value,
              "TimeStop must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::Search::TimeStop>::value,
              "TimeStop must remain copy assignable");
static_assert(std::is_copy_constructible<Gecode::Search::RestartStop>::value,
              "RestartStop must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::Search::RestartStop>::value,
              "RestartStop must remain copy assignable");
static_assert(
  std::is_copy_constructible<Gecode::Search::WorkerControl>::value,
  "WorkerControl must remain copy constructible");
static_assert(
  std::is_copy_assignable<Gecode::Search::WorkerControl>::value,
  "WorkerControl must remain copy assignable");

namespace Test {

  /// Tests for search engines
  namespace Search {

    using namespace Gecode;
    using namespace Gecode::Int;

    using Gecode::Search::EndReason;

#ifdef GECODE_HAS_EXPERIMENTAL
    /// Explicit finite tree with prefix costs, admissible bounds, and ranks
    class TreeSpace : public Space {
    public:
      /// Tree node
      struct Node {
        int cost, bound, rank, best;
        std::vector<int> children;
      };
      typedef std::vector<Node> Graph;
      std::shared_ptr<const Graph> graph;
      bool maximize;
      int vertex, ceiling;
      /// Brancher choosing a child of the current node
      class Decisions : public Brancher {
        class Decision : public Choice {
        public:
          int vertex;
          Decision(const Brancher& b, int v, unsigned int n)
            : Choice(b,n), vertex(v) {}
          void archive(Archive& e) const override {
            Choice::archive(e); e << vertex << alternatives();
          }
        };
      public:
        Decisions(Home home) : Brancher(home) {}
        Decisions(Space& home, Decisions& b) : Brancher(home,b) {}
        bool status(const Space& s) const override {
          return !static_cast<const TreeSpace&>(s).node().children.empty();
        }
        const Choice* choice(Space& s) override {
          const TreeSpace& t = static_cast<const TreeSpace&>(s);
          return new Decision(*this,t.vertex,
            static_cast<unsigned int>(t.node().children.size()));
        }
        const Choice* choice(const Space&, Archive& e) override {
          int v; unsigned int n; e >> v >> n;
          return new Decision(*this,v,n);
        }
        ExecStatus commit(Space& s, const Choice& c, unsigned int a) override {
          TreeSpace& t = static_cast<TreeSpace&>(s);
          t.vertex = (*t.graph)[static_cast<const Decision&>(c).vertex].children[a];
          return t.node().cost >= t.ceiling ? ES_FAILED : ES_OK;
        }
        Actor* copy(Space& home) override {
          return new (home) Decisions(home,*this);
        }
        size_t dispose(Space& home) override {
          (void) Brancher::dispose(home);
          return sizeof(*this);
        }
      };
      TreeSpace(std::shared_ptr<const Graph> g, bool max)
        : graph(g), maximize(max), vertex(0),
          ceiling(std::numeric_limits<int>::max()) {
        (void) new (*this) Decisions(*this);
      }
      TreeSpace(TreeSpace& s)
        : Space(s), graph(s.graph), maximize(s.maximize), vertex(s.vertex),
          ceiling(s.ceiling) {}
      Space* copy(void) override { return new TreeSpace(*this); }
      void hub(HubContext&) override {}
      const Node& node(void) const { return (*graph)[vertex]; }
      void constrain(const Space& s) override {
        ceiling = static_cast<const TreeSpace&>(s).node().cost;
        if (node().cost >= ceiling) fail();
      }
      SpaceComparison compare(const Space& s) const override {
        int a = node().cost;
        int b = static_cast<const TreeSpace&>(s).node().cost;
        return (a < b) ? SpaceComparison::Better : ((a > b) ? SpaceComparison::Worse : SpaceComparison::Equivalent);
      }
      /// Guidance minimizing cost, or maximizing negated cost
      static Gecode::Search::HeuristicGuidance
      guidance(bool max, bool ranked, bool dominance, bool keyed) {
        using Gecode::Search::HeuristicGuidance;
        HeuristicGuidance g(max ? Sense::Maximize : Sense::Minimize,
          [](const Space& s) -> std::int64_t {
            const TreeSpace& t = static_cast<const TreeSpace&>(s);
            return t.maximize ? -t.node().bound : t.node().bound;
          });
        if (ranked)
          g.rank = [](const Space& s) -> std::int64_t {
            return static_cast<const TreeSpace&>(s).node().rank;
          };
        if (dominance)
          g.dominates = [](const Space& a, const Space& b) {
            return static_cast<const TreeSpace&>(a).node().best <=
              static_cast<const TreeSpace&>(b).node().best;
          };
        if (keyed)
          g.dominance_key = [](const Space& s) -> std::size_t {
            return static_cast<std::size_t>
              (static_cast<const TreeSpace&>(s).node().best % 3);
          };
        return g;
      }
      /// Return a random tree where all nodes above depth \a d branch
      static std::shared_ptr<const Graph>
      random(Gecode::Support::RandomGenerator& r, int d=1) {
        Graph g;
        g.push_back({0,0,0,0,{}});
        grow(g,0,0,d,r);
        return std::make_shared<const Graph>(std::move(g));
      }
    private:
      static void grow(Graph& g, int v, int depth, int d,
                       Gecode::Support::RandomGenerator& r) {
        if ((depth < d) || ((depth < 5) && (r(4U) != 0U))) {
          unsigned int n = (depth < d ? 2U : 1U) + r(3U);
          for (unsigned int i=0; i<n; i++) {
            int c = static_cast<int>(g.size());
            g.push_back({g[v].cost+static_cast<int>(r(5U)),0,
                         static_cast<int>(r(10U)),0,{}});
            g[v].children.push_back(c);
            grow(g,c,depth+1,d,r);
          }
        }
        if (g[v].children.empty()) {
          g[v].best = g[v].bound = g[v].cost;
        } else {
          g[v].best = std::numeric_limits<int>::max();
          for (int c : g[v].children)
            g[v].best = std::min(g[v].best,g[c].best);
          g[v].bound = g[v].cost +
            static_cast<int>(r(static_cast<unsigned int>(g[v].best-g[v].cost+1)));
        }
      }
    };

    /// Stop object raising its node limit on each stop
    class Increasing : public Gecode::Search::Stop {
      unsigned long long int limit, step;
    public:
      unsigned int stops;
      Increasing(unsigned long long int s) : limit(s), step(s), stops(0) {}
      bool stop(const Gecode::Search::Statistics& s,
                const Gecode::Search::Options&) override {
        if (s.node <= limit)
          return false;
        limit += step;
        stops++;
        return true;
      }
    };

    /// Check that improving solutions reach the optimum unless search stops
    template<class Engine>
    static bool improving(Engine& e, const TreeSpace& root,
                          const Increasing* stop=nullptr) {
      int last = std::numeric_limits<int>::max();
      unsigned int stops = 0;
      while (true) {
        if (TreeSpace* s = e.next()) {
          int cost = s->node().cost;
          delete s;
          if (cost >= last)
            return false;
          last = cost;
        } else if ((stop != nullptr) && (stop->stops != stops)) {
          // Resume after the stop object stopped search
          stops = stop->stops;
        } else {
          break;
        }
      }
      return (e.end_reason() != EndReason::Exhausted) || (last == (*root.graph)[0].best);
    }

    /// Heuristic search engines on random trees
    template<template<class> class E>
    class Heuristic : public Base {
    public:
      Heuristic(const std::string& name)
        : Base("Search::"+name,TestTags::check()) {}
      bool run(void) override {
        for (bool max : {false,true})
          for (bool ranked : {false,true})
            for (bool dominance : {false,true})
              for (unsigned int threads : {1U,2U,4U})
                for (unsigned int step : {0U,1U,7U}) {
#ifndef GECODE_HAS_THREADS
                  if (threads > 1U) continue;
#endif
                  TreeSpace root(TreeSpace::random(_rand),max);
                  Gecode::Search::HeuristicGuidance g =
                    TreeSpace::guidance(max,ranked,dominance,_rand(2U) == 0U);
                  Increasing stop(step);
                  Gecode::Search::Options o;
                  o.threads = threads;
                  o.guidance = &g;
                  if (step > 0U)
                    o.stop = &stop;
                  if (_rand(3U) == 0U)
                    o.max_width = 1UL << _rand(3U);
                  E<TreeSpace> e(&root,o);
                  if (!improving(e,root,step > 0U ? &stop : nullptr))
                    return false;
                  if ((o.max_width == 0UL) && (e.end_reason() != EndReason::Exhausted))
                    return false;
                }
        return true;
      }
    };

    Heuristic<Gecode::AStar> astar_test("AStar");
    Heuristic<Gecode::CABS> cabs_test("CABS");

    /// Portfolio of BAB, A*, and CABS assets
    class HeuristicPortfolio : public Base {
      /// Stop object that never stops but takes some time
      class Slow : public Gecode::Search::Stop {
      public:
        bool stop(const Gecode::Search::Statistics&,
                  const Gecode::Search::Options&) override {
          std::this_thread::sleep_for(std::chrono::microseconds(200));
          return false;
        }
      };
    public:
      HeuristicPortfolio(void)
        : Base("Search::Heuristic::Portfolio",TestTags::check()) {}
      bool run(void) override {
        TreeSpace root(TreeSpace::random(_rand),false);
        Gecode::Search::HeuristicGuidance g =
          TreeSpace::guidance(false,true,false,false);
        Gecode::Search::Options a;
        a.threads = 1;
        a.guidance = &g;
        SEBs assets({bab<TreeSpace>(a),astar<TreeSpace>(a),
                     cabs<TreeSpace>(a)});
        Gecode::Search::Options o;
#ifdef GECODE_HAS_THREADS
        o.threads = 3;
#endif
        PBS<TreeSpace,BAB> e(&root,assets,o);
        if (!improving(e,root))
          return false;
        // A CABS asset stopping at its width limit must not stop the others
        return limited(1) && limited(2);
      }
      /// Run width-limited CABS and BAB on a tree where CABS stops early
      static bool limited(unsigned int threads) {
#ifndef GECODE_HAS_THREADS
        if (threads > 1U)
          return true;
#endif
        // CABS with width one follows the preferred branch to cost 5
        TreeSpace::Graph g = {
          {0,0,0,1,{1,2}}, {0,0,0,5,{3}}, {0,0,1,1,{4}},
          {5,5,0,5,{}}, {1,1,0,1,{}}
        };
        TreeSpace root(std::make_shared<const TreeSpace::Graph>(g),false);
        Gecode::Search::HeuristicGuidance h =
          TreeSpace::guidance(false,true,false,false);
        Gecode::Search::Options c;
        c.threads = 1;
        c.guidance = &h;
        c.max_width = 1;
        // Slow down BAB so that CABS stops first
        Slow slow;
        Gecode::Search::Options b;
        b.threads = 1;
        b.stop = &slow;
        SEBs assets({cabs<TreeSpace>(c),bab<TreeSpace>(b)});
        Gecode::Search::Options o;
        o.threads = threads;
        PBS<TreeSpace,BAB> e(&root,assets,o);
        int last = std::numeric_limits<int>::max();
        while (TreeSpace* s = e.next()) {
          last = s->node().cost;
          delete s;
        }
        return (last == 1) && (e.end_reason() == EndReason::Exhausted);
      }
    } heuristic_portfolio;
#endif


    /// Values for selecting branchers
    enum HowToBranch {
      HTB_NONE,   ///< Do not branch
      HTB_UNARY,  ///< Branch with single alternative
      HTB_BINARY, ///< Branch with two alternatives
      HTB_NARY    ///< Branch with many alternatives
    };

    /// Values for selecting how to constrain
    enum HowToConstrain {
      HTC_NONE,   ///< Do not constrain
      HTC_LEX_LE, ///< Constrain for lexically smallest
      HTC_LEX_GR, ///< Constrain for lexically biggest
      HTC_BAL_LE, ///< Constrain for smallest balance
      HTC_BAL_GR  ///< Constrain for largest balance
    };

    /// Values for selecting models
    enum WhichModel {
      WM_FAIL_IMMEDIATE, ///< Model that fails immediately
      WM_FAIL_SEARCH,    ///< Model without solutions
      WM_SOLUTIONS       ///< Model with solutions
    };

    /// Space with information
    class TestSpace : public Space {
    public:
      /// Constructor for space creation
      TestSpace(void) {}
      /// Constructor for cloning \a s
      TestSpace(TestSpace& s) : Space(s) {}
      /// Return number of solutions
      virtual int solutions(void) const = 0;
      /// Verify that this is best solution
      virtual bool best(void) const = 0;
      /// Hub hook that does not restart after a solution and keeps the branchers of a portfolio
      virtual void hub(HubContext& hc) {
        if (hc.engine() == MetaEngine::Restart) {
          if (hc.last() != nullptr)
            constrain(*hc.last());
          if (hc.event() == MetaEvent::Solution)
            hc.decision(MetaDecision::Continue);
        }
      }
    };

    /// Space that immediately fails
    class FailImmediate : public TestSpace {
    public:
      /// Variables used
      IntVarArray x;
      /// Constructor for space creation
      FailImmediate(HowToBranch, HowToBranch, HowToBranch,
                    HowToConstrain=HTC_NONE)
        : x(*this,1,0,0) {
        rel(*this, x[0], IRT_EQ, 1);
      }
      /// Constructor for cloning \a s
      FailImmediate(FailImmediate& s) : TestSpace(s) {
        x.update(*this, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(void) {
        return new FailImmediate(*this);
      }
      /// Add constraint for next better solution
      virtual void constrain(const Space&) {
      }
      /// Return number of solutions
      virtual int solutions(void) const {
        return 0;
      }
      /// Verify that this is best solution
      virtual bool best(void) const {
        return false;
      }
      /// Return name
      static std::string name(void) {
        return "Fail";
      }
    };

    /// Space that is immediately solved
    class SolveImmediate : public TestSpace {
    public:
      /// Variables used
      IntVarArray x;
      /// Constructor for space creation
      SolveImmediate(HowToBranch, HowToBranch, HowToBranch,
                     HowToConstrain=HTC_NONE)
        : x(*this,1,0,0) {}
      /// Constructor for cloning \a s
      SolveImmediate(SolveImmediate& s) : TestSpace(s) {
        x.update(*this, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(void) {
        return new SolveImmediate(*this);
      }
      /// Add constraint for next better solution
      virtual void constrain(const Space&) {
        fail();
      }
      /// Treat the single solution as equivalent across best-search assets
      virtual SpaceComparison compare(const Space&) const {
        return SpaceComparison::Equivalent;
      }
      /// Return number of solutions
      virtual int solutions(void) const {
        return 1;
      }
      /// Verify that this is best solution
      virtual bool best(void) const {
        return true;
      }
      /// Return name
      static std::string name(void) {
        return "Solve";
      }
    };

    /// Space that requires propagation and has solutions
    class HasSolutions : public TestSpace {
    public:
      /// Variables used
      IntVarArray x;
      /// How to branch
      HowToBranch htb1, htb2, htb3;
      /// How to constrain
      HowToConstrain htc;
      /// Branch on \a x according to \a htb
      void branch(const IntVarArgs& x, HowToBranch htb) {
        switch (htb) {
        case HTB_NONE:
          break;
        case HTB_UNARY:
          assign(*this, x, INT_ASSIGN_MIN());
          break;
        case HTB_BINARY:
          Gecode::branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
          break;
        case HTB_NARY:
          Gecode::branch(*this, x, INT_VAR_NONE(), INT_VALUES_MIN());
          break;
        }
      }
      /// Constructor for space creation
      HasSolutions(HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
                   HowToConstrain _htc=HTC_NONE)
        : x(*this,6,0,5), htb1(_htb1), htb2(_htb2), htb3(_htb3), htc(_htc) {
        distinct(*this, x);
        rel(*this, x[2], IRT_LQ, 3); rel(*this, x[3], IRT_LQ, 3);
        rel(*this, x[4], IRT_LQ, 1); rel(*this, x[5], IRT_LQ, 1);
        IntVarArgs x1(2); x1[0]=x[0]; x1[1]=x[1]; branch(x1, htb1);
        IntVarArgs x2(2); x2[0]=x[2]; x2[1]=x[3]; branch(x2, htb2);
        IntVarArgs x3(2); x3[0]=x[4]; x3[1]=x[5]; branch(x3, htb3);
      }
      /// Constructor for cloning \a s
      HasSolutions(HasSolutions& s)
        : TestSpace(s),
          htb1(s.htb1), htb2(s.htb2), htb3(s.htb3), htc(s.htc) {
        x.update(*this, s.x);
      }
      /// Copy during cloning
      virtual Space* copy(void) {
        return new HasSolutions(*this);
      }
      /// Add constraint for next better solution
      virtual void constrain(const Space& _s) {
        const HasSolutions& s = static_cast<const HasSolutions&>(_s);
        switch (htc) {
        case HTC_NONE:
          break;
        case HTC_LEX_LE:
        case HTC_LEX_GR:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i] = IntVar(*this, s.x[i].val(), s.x[i].val());
            lex(*this, x, (htc == HTC_LEX_LE) ? IRT_LE : IRT_GR, y);
            break;
          }
        case HTC_BAL_LE:
        case HTC_BAL_GR:
          {
            IntVarArgs y(6);
            for (int i=0; i<6; i++)
              y[i] = IntVar(*this, s.x[i].val(), s.x[i].val());
            IntVar xs(*this, -18, 18);
            IntVar ys(*this, -18, 18);
            rel(*this, x[0]+x[1]+x[2]-x[3]-x[4]-x[5] == xs);
            rel(*this, y[0]+y[1]+y[2]-y[3]-y[4]-y[5] == ys);
            rel(*this,
                expr(*this,abs(xs)),
                (htc == HTC_BAL_LE) ? IRT_LE : IRT_GR,
                expr(*this,abs(ys)));
            break;
          }
        }
      }
      /// Compare objectives used by best solution search
      virtual SpaceComparison compare(const Space& _s) const {
        const HasSolutions& s = dynamic_cast<const HasSolutions&>(_s);
        if (htc != s.htc)
          throw DynamicCastFailed("HasSolutions::compare");
        int c=0;
        int sc=0;
        switch (htc) {
        case HTC_LEX_LE:
        case HTC_LEX_GR:
          for (int i=0; i<x.size(); i++) {
            if (x[i].val() == s.x[i].val())
              continue;
            bool better = (htc == HTC_LEX_LE) ?
              (x[i].val() < s.x[i].val()) : (x[i].val() > s.x[i].val());
            return better ? SpaceComparison::Better : SpaceComparison::Worse;
          }
          return SpaceComparison::Equivalent;
        case HTC_BAL_LE:
        case HTC_BAL_GR:
          c = std::abs(x[0].val()+x[1].val()+x[2].val()-
                       x[3].val()-x[4].val()-x[5].val());
          sc = std::abs(s.x[0].val()+s.x[1].val()+s.x[2].val()-
                        s.x[3].val()-s.x[4].val()-s.x[5].val());
          if (c == sc)
            return SpaceComparison::Equivalent;
          return ((htc == HTC_BAL_LE) ? (c < sc) : (c > sc)) ?
            SpaceComparison::Better : SpaceComparison::Worse;
        case HTC_NONE:
          return SpaceComparison::Equivalent;
        default:
          GECODE_NEVER;
        }
        return SpaceComparison::Incomparable;
      }
      /// Return number of solutions
      virtual int solutions(void) const {
        if (htb1 == HTB_NONE) {
          assert((htb2 == HTB_NONE) && (htb3 == HTB_NONE));
          return 1;
        }
        if ((htb1 == HTB_UNARY) || (htb2 == HTB_UNARY))
          return 0;
        if (htb3 == HTB_UNARY)
          return 4;
        return 8;
      }
      /// Verify that this is best solution
      virtual bool best(void) const {
        if ((htb1 == HTB_NONE) || (htb2 == HTB_NONE) || (htb3 == HTB_NONE) ||
            (htb1 == HTB_UNARY) || (htb2 == HTB_UNARY) || (htb3 == HTB_UNARY))
          return true;
        switch (htc) {
        case HTC_NONE:
          return true;
        case HTC_LEX_LE:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==2) && (x[3].val()==3) &&
                  (x[4].val()==0) && (x[5].val()==1));
        case HTC_LEX_GR:
          return ((x[0].val()==5) && (x[1].val()==4) &&
                  (x[2].val()==3) && (x[3].val()==2) &&
                  (x[4].val()==1) && (x[5].val()==0));
        case HTC_BAL_LE:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==2) && (x[3].val()==3) &&
                  (x[4].val()==0) && (x[5].val()==1));
        case HTC_BAL_GR:
          return ((x[0].val()==4) && (x[1].val()==5) &&
                  (x[2].val()==3) && (x[3].val()==2) &&
                  (x[4].val()==0) && (x[5].val()==1));
        default: GECODE_NEVER;
        }
        return false;
      }
      /// Return name
      static std::string name(void) {
        return "Sol";
      }
      /// Rule out that solution is found more than once during restarts
      virtual void hub(HubContext& hc) {
        // For portfolio search, keep the branchers
        if (hc.engine() != MetaEngine::Restart)
          return;
        // With an objective, restart from a bound improving on the last solution
        if (htc != HTC_NONE) {
          Space::hub(hc);
          return;
        }
        if (hc.last() != nullptr) {
          const HasSolutions* s
            = static_cast<const HasSolutions*>(hc.last());
          BoolVarArgs b;
          for (int i=0; i<x.size(); i++)
            b << expr(*this, x[i] == s->x[i]);
          rel(*this, BOT_AND, b, 0);
        }
        if (hc.event() == MetaEvent::Solution)
          hc.decision(MetaDecision::Continue);
      }
    };

    /// Return a bit for meta event \a e
    static unsigned int
    event_bit(MetaEvent e) {
      return 1U << static_cast<unsigned int>(e);
    }

    /// Observations shared by meta-search dispatch test spaces
    struct MetaDispatchState {
      std::atomic<unsigned int> hub_calls{0};
      std::atomic<unsigned int> spoke_calls{0};
      std::atomic<unsigned int> hub_events{0};
      std::atomic<unsigned int> spoke_events{0};
      std::atomic<unsigned int> assets{0};
      std::atomic<unsigned int> live_spaces{0};
      std::atomic<unsigned int> copies{0};
      std::atomic<unsigned int> constrain_calls{0};
      std::atomic<unsigned int> constrain_hub_calls{0};
      std::atomic<unsigned int> constrain_spoke_calls{0};
      std::atomic<unsigned int> constrain_asset_mask{0};
      std::atomic<unsigned int> constrain_hub_asset_mask{0};
      std::atomic<unsigned int> constrain_spoke_asset_mask{0};
      std::atomic<unsigned int> nogood_handoffs{0};
      /// Decision of hub() after a solution
      MetaDecision solution_decision = MetaDecision::Restart;
      /// Coverage that spoke() declares after a solution
      SpokeCoverage solution_spoke_coverage = SpokeCoverage::Full;
      bool fail_solution_spoke = false;
      bool nogood_model = false;
      /// Whether the model posts a branching on x
      bool model_branching = true;
      /// Whether a PBS spoke() posts the branching for its asset
      bool spoke_branching = true;
      /// Whether the first RBS spoke reaches the cutoff without a solution
      bool limit_first_spoke = false;
    };

    /// Common implementation for meta-search dispatch test spaces
    class MetaDispatchSpace : public Space {
    protected:
      /// RBS role of a space
      enum class RestartRole { Hub, Spoke };
      /// Variable used to provide two solutions
      IntVar x;
      /// Optional queens model used to force restart no-good extraction
      IntVarArray q;
      /// Optional variables without a solution, used to reach the cutoff
      IntVarArray y;
      /// Shared hook observations and configuration
      MetaDispatchState* state;
      /// RBS role retained across cloning
      RestartRole restart_role;
      /// PBS asset identity, or -1 before portfolio dispatch
      int portfolio_asset;

      /// Record and implement a Hub callback
      void
      observe_hub(HubContext& hc) {
        state->hub_calls.fetch_add(1, std::memory_order_relaxed);
        // Keep the branchers of a portfolio hub
        if (hc.engine() == MetaEngine::Portfolio)
          return;
        restart_role = RestartRole::Hub;
        state->hub_events.fetch_or(event_bit(hc.event()), std::memory_order_relaxed);
        Space::hub(hc);
        if (hc.nogoods().ng() > 0)
          state->nogood_handoffs.fetch_add(1, std::memory_order_relaxed);
        if (hc.event() == MetaEvent::Solution)
          hc.decision(state->solution_decision);
      }
      /// Record and implement a Spoke callback
      void
      observe_spoke(SpokeContext& sc) {
        state->spoke_calls.fetch_add(1, std::memory_order_relaxed);
        if (sc.engine() == MetaEngine::Portfolio) {
          portfolio_asset = static_cast<int>(sc.asset());
          state->assets.fetch_or(1U << sc.asset(), std::memory_order_relaxed);
          // The default hub() removes the model's branching. A hub()
          // override keeps it, and then this branching is redundant.
          if (state->spoke_branching)
            Gecode::branch(*this, x, INT_VAL_MIN());
          return;
        }
        state->spoke_events.fetch_or(event_bit(sc.event()), std::memory_order_relaxed);
        restart_role = RestartRole::Spoke;
        if ((sc.event() == MetaEvent::Start) && state->limit_first_spoke) {
          // Four distinct values from three fail only during search.
          distinct(*this, y, IPL_VAL);
          Gecode::branch(*this, y, INT_VAR_NONE(), INT_VAL_MIN());
        }
        if (sc.event() == MetaEvent::Solution) {
          if (state->fail_solution_spoke)
            fail();
          sc.coverage(state->solution_spoke_coverage);
        }
      }

    public:
      /// Initialize with shared observations \a s
      MetaDispatchSpace(MetaDispatchState& s)
        : x(*this, 0, 1), q(*this, s.nogood_model ? 8 : 0, 0, 7), y(*this, s.limit_first_spoke ? 4 : 0, 0, 2),
          state(&s), restart_role(RestartRole::Hub), portfolio_asset(-1) {
        state->live_spaces.fetch_add(1, std::memory_order_relaxed);
        if (state->nogood_model) {
          distinct(*this, IntArgs::create(q.size(), 0, 1), q, IPL_VAL);
          distinct(*this, IntArgs::create(q.size(), 0, -1), q, IPL_VAL);
          distinct(*this, q, IPL_VAL);
          Gecode::branch(*this, q, INT_VAR_NONE(), INT_VAL_MIN());
        }
        if (state->model_branching)
          Gecode::branch(*this, x, INT_VAL_MIN());
      }
      /// Clone \a s
      MetaDispatchSpace(MetaDispatchSpace& s)
        : Space(s), state(s.state), restart_role(s.restart_role), portfolio_asset(s.portfolio_asset) {
        state->live_spaces.fetch_add(1, std::memory_order_relaxed);
        state->copies.fetch_add(1, std::memory_order_relaxed);
        x.update(*this, s.x);
        q.update(*this, s.q);
        y.update(*this, s.y);
      }
      /// Record destruction for clone-ownership checks
      ~MetaDispatchSpace(void) override {
        state->live_spaces.fetch_sub(1, std::memory_order_relaxed);
      }
      /// Require a larger value than solution \a s
      void
      constrain(const Space& s) override {
        state->constrain_calls.fetch_add(1, std::memory_order_relaxed);
        if (restart_role == RestartRole::Hub)
          state->constrain_hub_calls.fetch_add(1, std::memory_order_relaxed);
        else
          state->constrain_spoke_calls.fetch_add(1, std::memory_order_relaxed);
        if (portfolio_asset >= 0) {
          const unsigned int asset = 1U << static_cast<unsigned int>(portfolio_asset);
          state->constrain_asset_mask.fetch_or(asset, std::memory_order_relaxed);
          if (restart_role == RestartRole::Hub)
            state->constrain_hub_asset_mask.fetch_or(asset, std::memory_order_relaxed);
          else
            state->constrain_spoke_asset_mask.fetch_or(asset, std::memory_order_relaxed);
        }
        rel(*this, x, IRT_GR, static_cast<const MetaDispatchSpace&>(s).x.val());
      }
      /// Compare with solution \a s, larger values are better
      SpaceComparison
      compare(const Space& s) const override {
        return compare_int(x.val(), static_cast<const MetaDispatchSpace&>(s).x.val(), Sense::Maximize);
      }
      /// Return the solution value
      int
      value(void) const {
        return x.val();
      }
      /// Return whether the solution variable is assigned
      bool
      assigned(void) const {
        return x.assigned();
      }
      /// Assign the solution variable to \a v
      void
      assign_value(int v) {
        rel(*this, x, IRT_EQ, v);
      }
    };

    /// Dispatch test space overriding only Hub
    class HubDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      HubDispatchSpace(MetaDispatchState& s) : MetaDispatchSpace(s) {}
      /// Clone \a s
      HubDispatchSpace(HubDispatchSpace& s) : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space*
      copy(void) override {
        return new HubDispatchSpace(*this);
      }
      /// Observe Hub dispatch
      void
      hub(HubContext& hc) override {
        observe_hub(hc);
      }
    };

    /// Dispatch test space overriding only Spoke
    class SpokeDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      SpokeDispatchSpace(MetaDispatchState& s) : MetaDispatchSpace(s) {}
      /// Clone \a s
      SpokeDispatchSpace(SpokeDispatchSpace& s) : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space*
      copy(void) override {
        return new SpokeDispatchSpace(*this);
      }
      /// Observe Spoke dispatch
      void
      spoke(SpokeContext& sc) override {
        observe_spoke(sc);
      }
    };

    /// Dispatch test space overriding both Hub and Spoke
    class HubSpokeDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      HubSpokeDispatchSpace(MetaDispatchState& s) : MetaDispatchSpace(s) {}
      /// Clone \a s
      HubSpokeDispatchSpace(HubSpokeDispatchSpace& s) : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space*
      copy(void) override {
        return new HubSpokeDispatchSpace(*this);
      }
      /// Observe Hub dispatch
      void
      hub(HubContext& hc) override {
        observe_hub(hc);
      }
      /// Observe Spoke dispatch
      void
      spoke(SpokeContext& sc) override {
        observe_spoke(sc);
      }
    };

    /// Hub and Spoke hook dispatch through RBS and PBS
    class MetaDispatch : public Base {
    public:
      /// Dispatch scenario
      enum class Scenario {
        RbsHubOnly,
        RbsSpokeOnly,
        RbsHubContinue,
        RbsHubRestart,
        RbsSpokeFull,
        RbsSpokeReduced,
        RbsSpokePartial,
        RbsLimit,
        PbsHubOnly,
        PbsSpokeOnly,
        PbsSingle,
        PbsSequentialHubOnly,
        PbsSequentialSpokeOnly,
        PbsSequential,
        PbsParallelHubOnly,
        PbsParallelSpokeOnly,
        PbsParallel,
        PbsParallelThreadLimit,
        PbsSpokeWithoutBrancher,
        PbsNoBrancher,
        RbsNoGoodsResetOwnership,
        MetaStopOwnership,
        BabIncumbent
      };

    protected:
      /// Scenario under test
      Scenario scenario;

      /// Return whether \a counter holds \a expected
      static bool
      holds(const std::atomic<unsigned int>& counter, unsigned int expected) {
        return counter.load(std::memory_order_relaxed) == expected;
      }
      /// Check independent callback dispatch for one RBS model
      template <class Model>
      static bool
      rbs_dispatch(unsigned int hub_calls, unsigned int spoke_calls) {
        MetaDispatchState state;
        Model* m = new Model(state);
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<Model, Gecode::DFS> search(m, o);
        delete m;

        Model* first = search.next();
        Model* second = search.next();
        bool ok = (first != nullptr) && (second != nullptr) && holds(state.hub_calls, hub_calls) &&
                  holds(state.spoke_calls, spoke_calls);
        delete first;
        delete second;
        return ok;
      }
      /// Run one RBS dispatch scenario
      static bool
      rbs(MetaDecision solution_decision, bool fail_solution_spoke,
          SpokeCoverage solution_spoke_coverage) {
        MetaDispatchState state;
        state.solution_decision = solution_decision;
        state.fail_solution_spoke = fail_solution_spoke;
        state.solution_spoke_coverage = solution_spoke_coverage;

        HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(state);
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
        delete m;

        HubSpokeDispatchSpace* first = search.next();
        bool ok = first != nullptr;
        delete first;

        const unsigned int start = event_bit(MetaEvent::Start);
        const unsigned int solution = event_bit(MetaEvent::Solution);
        const unsigned int exhausted = event_bit(MetaEvent::Exhausted);
        HubSpokeDispatchSpace* second = search.next();
        if (!fail_solution_spoke) {
          const bool restart = solution_decision == MetaDecision::Restart;
          ok = ok && (second != nullptr) && holds(state.hub_calls, 2U) &&
               holds(state.spoke_calls, restart ? 2U : 1U) &&
               holds(state.hub_events, start | solution) &&
               holds(state.spoke_events, start | (restart ? solution : 0U));
        } else if (solution_spoke_coverage != SpokeCoverage::Partial) {
          // Exhausting a spoke that is not partial is a proof
          ok = ok && (second == nullptr) &&
               (search.end_reason() == EndReason::Exhausted) &&
               holds(state.hub_calls, 2U) && holds(state.spoke_calls, 2U) &&
               holds(state.hub_events, start | solution) &&
               holds(state.spoke_events, start | solution);
        } else {
          ok = ok && (second != nullptr) && holds(state.hub_calls, 3U) && holds(state.spoke_calls, 3U) &&
               holds(state.hub_events, start | solution | exhausted) &&
               holds(state.spoke_events, start | solution | exhausted);
        }
        delete second;
        return ok;
      }
      /// Run one PBS dispatch scenario
      template <class Model>
      static bool
      pbs(unsigned int assets, double threads, unsigned int hub_calls, unsigned int spoke_calls,
        unsigned int asset_mask) {
        MetaDispatchState state;
        Model* m = new Model(state);
        Gecode::Search::Options o;
        o.assets = assets;
        o.threads = threads;
        Gecode::PBS<Model, Gecode::DFS> search(m, o);
        delete m;

        Model* solution = search.next();
        bool ok = (solution != nullptr) && solution->assigned() && holds(state.hub_calls, hub_calls) &&
                  holds(state.spoke_calls, spoke_calls) && holds(state.assets, asset_mask);
        delete solution;
        return ok;
      }
      /// Check that PBS rejects a spoke without branching when the hub had one
      static bool
      pbs_spoke_without_brancher(bool model_branching) {
        MetaDispatchState state;
        state.spoke_branching = false;
        state.model_branching = model_branching;
        SpokeDispatchSpace* m = new SpokeDispatchSpace(state);
        Gecode::Search::Options o;
        o.assets = 3;
        o.threads = 1;
        try {
          Gecode::PBS<SpokeDispatchSpace, Gecode::DFS> search(m, o);
          delete m;
          // A spoke without branching is a solution as soon as it is stable.
          SpokeDispatchSpace* solution = search.next();
          bool ok = !model_branching && (solution != nullptr) && !solution->assigned() &&
                    holds(state.spoke_calls, 3U);
          delete solution;
          return ok;
        } catch (const Gecode::Search::SpokeNoBrancher&) {
          delete m;
          return model_branching && holds(state.live_spaces, 0U);
        }
      }
      /// Check RBS dispatch when the first spoke reaches the cutoff
      static bool
      rbs_limit(void) {
        MetaDispatchState state;
        state.limit_first_spoke = true;
        HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(state);
        Gecode::Search::Options o;
        o.threads = 1;
        // No-goods from the first spoke would refer to its own brancher.
        o.nogoods_limit = 0;
        o.cutoff = Gecode::Search::Cutoff::constant(1);
        Gecode::RBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
        delete m;

        HubSpokeDispatchSpace* solution = search.next();
        const unsigned int events = event_bit(MetaEvent::Start) | event_bit(MetaEvent::Limit);
        bool ok = (solution != nullptr) && solution->assigned() && holds(state.hub_calls, 2U) &&
                  holds(state.spoke_calls, 2U) && holds(state.hub_events, events) &&
                  holds(state.spoke_events, events);
        delete solution;
        return ok;
      }
      /// Exercise RBS no-good posting, reset, and clone ownership
      static bool
      rbs_nogoods_reset_ownership(void) {
        MetaDispatchState state;
        state.nogood_model = true;
        bool ok = true;
        {
          // Without no-goods on the hub, every round repeats the same failed
          // prefix. The node limit turns that into a failure, not a hang.
          Gecode::Search::NodeStop stop(10000);
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.nogoods_limit = 16;
          o.stop = &stop;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          // The engine constrains the hub by each solution
          Gecode::RBS<HubSpokeDispatchSpace, Gecode::BAB> search(m, o);
          delete m;

          unsigned int solutions = 0;
          while (HubSpokeDispatchSpace* solution = search.next()) {
            solutions++;
            delete solution;
          }
          ok = (search.end_reason() == EndReason::Exhausted) && (solutions == 2U) &&
               (state.spoke_calls.load(std::memory_order_relaxed) >= 2U) &&
               (state.constrain_calls.load(std::memory_order_relaxed) >= 1U) &&
               (state.nogood_handoffs.load(std::memory_order_relaxed) >= 1U);
        }
        return ok && holds(state.live_spaces, 0U) && (state.copies.load(std::memory_order_relaxed) > 0U);
      }
      /// Stop RBS and PBS, then verify orderly clone destruction
      static bool
      meta_stop_ownership(void) {
        MetaDispatchState rbs_state;
        // Continue after a solution, so that the spoke engine prunes with it
        rbs_state.solution_decision = MetaDecision::Continue;
        bool ok = true;
        {
          Gecode::Search::NodeStop stop(0);
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(rbs_state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.stop = &stop;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::RBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
          delete m;
          HubSpokeDispatchSpace* solution = search.next();
          ok = (solution == nullptr) && (search.end_reason() == EndReason::Limit);
          delete solution;
        }
        ok = ok && holds(rbs_state.live_spaces, 0U);

        MetaDispatchState pbs_state;
        {
          Gecode::Search::NodeStop stop(0);
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(pbs_state);
          Gecode::Search::Options o;
          o.assets = 3;
#ifdef GECODE_HAS_THREADS
          o.threads = 3;
#else
          o.threads = 1;
#endif
          o.stop = &stop;
          Gecode::PBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
          delete m;
          HubSpokeDispatchSpace* solution = search.next();
          ok = ok && (solution == nullptr) && (search.end_reason() == EndReason::Limit);
          delete solution;
        }
        return ok && holds(pbs_state.live_spaces, 0U);
      }
      /// Exercise incumbent propagation through RBS and PBS wrappers
      static bool
      bab_incumbent(void) {
        MetaDispatchState rbs_state;
        // Continue after a solution, so that the spoke engine prunes with it
        rbs_state.solution_decision = MetaDecision::Continue;
        bool ok = true;
        {
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(rbs_state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.cutoff = Gecode::Search::Cutoff::constant(100);
          SEB builder = Gecode::rbs<HubSpokeDispatchSpace, Gecode::BAB>(o);
          Gecode::Search::Engine* search = (*builder)(m);
          delete builder;
          delete m;
          // Before the first solution, neither the default hub() nor BAB's
          // own pruning has an incumbent. Only RBS::constrain() can then
          // remove the solution x = 0.
          HubSpokeDispatchSpace* incumbent = new HubSpokeDispatchSpace(rbs_state);
          incumbent->assign_value(0);
          ok = (incumbent->status() == SS_SOLVED) && holds(rbs_state.constrain_calls, 0U);
          search->constrain(*incumbent);
          delete incumbent;
          HubSpokeDispatchSpace* best = static_cast<HubSpokeDispatchSpace*>(search->next());
          ok = ok && (best != nullptr) && (best->value() == 1) &&
               (rbs_state.constrain_hub_calls.load(std::memory_order_relaxed) > 0U) &&
               (rbs_state.constrain_spoke_calls.load(std::memory_order_relaxed) > 0U);
          ok = ok && (search->next() == nullptr);
          delete best;
          delete search;
        }
        ok = ok && holds(rbs_state.live_spaces, 0U);

        MetaDispatchState pbs_state;
        {
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(pbs_state);
          Gecode::Search::Options o;
          o.assets = 3;
          o.threads = 1;
          Gecode::PBS<HubSpokeDispatchSpace, Gecode::BAB> search(m, o);
          delete m;
          HubSpokeDispatchSpace* best = nullptr;
          while (HubSpokeDispatchSpace* solution = search.next()) {
            delete best;
            best = solution;
          }
          ok = ok && (best != nullptr) && (best->value() == 1) && holds(pbs_state.constrain_asset_mask, 7U);
          delete best;
        }
        ok = ok && holds(pbs_state.live_spaces, 0U);

        MetaDispatchState nested_state;
        nested_state.solution_decision = MetaDecision::Continue;
        {
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(nested_state);
          Gecode::Search::Options outer;
          outer.assets = 3;
          outer.threads = 1;
          Gecode::Search::Options inner[3];
          SEBs assets(3);
          for (int i = 0; i < 3; i++) {
            inner[i].threads = 1;
            // Each RBS engine owns and deletes its cutoff.
            inner[i].cutoff = Gecode::Search::Cutoff::constant(100);
            assets[i] = Gecode::rbs<HubSpokeDispatchSpace, Gecode::BAB>(inner[i]);
          }
          Gecode::PBS<HubSpokeDispatchSpace, Gecode::BAB> search(m, assets, outer);
          delete m;
          HubSpokeDispatchSpace* best = nullptr;
          while (HubSpokeDispatchSpace* solution = search.next()) {
            delete best;
            best = solution;
          }
          ok = ok && (best != nullptr) && (best->value() == 1) &&
               holds(nested_state.constrain_hub_asset_mask, 7U) &&
               holds(nested_state.constrain_spoke_asset_mask, 7U);
          delete best;
        }
        return ok && holds(nested_state.live_spaces, 0U);
      }

    public:
      /// Initialize scenario \a s with name \a n
      MetaDispatch(const std::string& n, Scenario s)
        : Base("Search::MetaDispatch::" + n, TestTags::check()), scenario(s) {}
      /// Run test
      bool
      run(void) override {
        switch (scenario) {
        case Scenario::RbsHubOnly:
          return rbs_dispatch<HubDispatchSpace>(2, 0);
        case Scenario::RbsSpokeOnly:
          return rbs_dispatch<SpokeDispatchSpace>(0, 2);
        case Scenario::RbsHubContinue:
          return rbs(MetaDecision::Continue, false, SpokeCoverage::Full);
        case Scenario::RbsHubRestart:
          return rbs(MetaDecision::Restart, false, SpokeCoverage::Full);
        case Scenario::RbsSpokeFull:
          return rbs(MetaDecision::Restart, true, SpokeCoverage::Full);
        case Scenario::RbsSpokeReduced:
          return rbs(MetaDecision::Restart, true, SpokeCoverage::Reduced);
        case Scenario::RbsSpokePartial:
          return rbs(MetaDecision::Restart, true, SpokeCoverage::Partial);
        case Scenario::RbsLimit:
          return rbs_limit();
        case Scenario::PbsHubOnly:
          return pbs<HubDispatchSpace>(1, 1, 1, 0, 0);
        case Scenario::PbsSpokeOnly:
          return pbs<SpokeDispatchSpace>(1, 1, 0, 1, 1);
        case Scenario::PbsSingle:
          return pbs<HubSpokeDispatchSpace>(1, 1, 1, 1, 1);
        case Scenario::PbsSequentialHubOnly:
          return pbs<HubDispatchSpace>(3, 1, 1, 0, 0);
        case Scenario::PbsSequentialSpokeOnly:
          return pbs<SpokeDispatchSpace>(3, 1, 0, 3, 7);
        case Scenario::PbsSequential:
          return pbs<HubSpokeDispatchSpace>(3, 1, 1, 3, 7);
        case Scenario::PbsParallelHubOnly:
          return pbs<HubDispatchSpace>(3, 3, 1, 0, 0);
        case Scenario::PbsParallelSpokeOnly:
          return pbs<SpokeDispatchSpace>(3, 3, 0, 3, 7);
        case Scenario::PbsParallel:
          return pbs<HubSpokeDispatchSpace>(3, 3, 1, 3, 7);
        case Scenario::PbsParallelThreadLimit:
          // Parallel PBS runs all assets, also with fewer threads.
          return pbs<HubSpokeDispatchSpace>(3, 2, 1, 3, 7);
        case Scenario::PbsSpokeWithoutBrancher:
          return pbs_spoke_without_brancher(true);
        case Scenario::PbsNoBrancher:
          return pbs_spoke_without_brancher(false);
        case Scenario::RbsNoGoodsResetOwnership:
          return rbs_nogoods_reset_ownership();
        case Scenario::MetaStopOwnership:
          return meta_stop_ownership();
        case Scenario::BabIncumbent:
          return bab_incumbent();
        default:
          GECODE_NEVER;
        }
        return false;
      }
    };

    /// %Base class for search tests
    class Test : public Base {
    public:
      /// How to branch
      HowToBranch htb1, htb2, htb3;
      /// How to constrain
      HowToConstrain htc;
      /// Map unsigned integer to string
      static std::string str(unsigned int i) {
        std::stringstream s;
        s << i;
        return s.str();
      }
      /// Map branching to string
      static std::string str(HowToBranch htb) {
        switch (htb) {
        case HTB_NONE:   return "None";
        case HTB_UNARY:  return "Unary";
        case HTB_BINARY: return "Binary";
        case HTB_NARY:   return "Nary";
        default: GECODE_NEVER;
        }
        GECODE_NEVER;
        return "";
      }
      /// Map constrain to string
      static std::string str(HowToConstrain htc) {
        switch (htc) {
        case HTC_NONE:   return "None";
        case HTC_LEX_LE: return "LexLe";
        case HTC_LEX_GR: return "LexGr";
        case HTC_BAL_LE: return "BalLe";
        case HTC_BAL_GR: return "BalGr";
        default: GECODE_NEVER;
        }
        GECODE_NEVER;
        return "";
      }
      /// Initialize test
      Test(TestTags tags, const std::string& s,
           HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
           HowToConstrain _htc=HTC_NONE)
        : Base("Search::"+s,tags),
          htb1(_htb1), htb2(_htb2), htb3(_htb3), htc(_htc) {}
      /// Initialize a normal test
      Test(const std::string& s,
           HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
           HowToConstrain _htc=HTC_NONE)
        : Test(TestTags::standard(),s,_htb1,_htb2,_htb3,_htc) {}
    };

    /// Return the effective leaf capacity for this build
    static unsigned int
    worker_capacity(unsigned int threads) {
#ifdef GECODE_HAS_THREADS
      return threads;
#else
      (void) threads;
      return 1U;
#endif
    }

    /// Track seeds and engine clones through nested construction failures
    class PortfolioSeed : public SolveImmediate {
      int& live;
    public:
      PortfolioSeed(int& count)
        : SolveImmediate(HTB_NONE,HTB_NONE,HTB_NONE), live(count) { ++live; }
      PortfolioSeed(PortfolioSeed& s) : SolveImmediate(s), live(s.live) {
        ++live;
      }
      Space* copy(void) override { return new PortfolioSeed(*this); }
      ~PortfolioSeed(void) override { --live; }
    };

    template<class T>
    using RestartPortfolio = Gecode::PBS<T,Gecode::RBS>;

    /// Nested failures must preserve the exception and dispose owned spaces
    class PBSConstructionFailure : public Base {
      bool worker_control_failure(unsigned int threads, bool clone,
                                  bool restart, bool in_use) {
        Gecode::Search::WorkerControl control(in_use ? 1U :
                                              worker_capacity(threads)+1U);
        SolveImmediate binding_root(HTB_NONE,HTB_NONE,HTB_NONE);
        std::unique_ptr<Gecode::DFS<SolveImmediate>> binding;
        if (in_use) {
          Gecode::Search::Options bound;
          bound.worker_control = control;
          binding.reset(new Gecode::DFS<SolveImmediate>(&binding_root,bound));
        }
        int live = 0;
        std::unique_ptr<PortfolioSeed> root(new PortfolioSeed(live));
        PortfolioSeed* seed = root.get();
        if (!clone)
          root.release();
        Gecode::Search::Options o;
        o.assets = 1;
        o.threads = threads;
        o.clone = clone;
        o.worker_control = control;
        std::unique_ptr<Gecode::Search::Cutoff>
          cutoff(Gecode::Search::Cutoff::constant(10));
        o.cutoff = cutoff.get();
        bool caught = false;
        try {
          if (restart) {
            Gecode::RBS<PortfolioSeed> engine(seed,o);
            cutoff.release();
          } else {
            Gecode::PBS<PortfolioSeed> engine(seed,o);
          }
        } catch (const Gecode::Search::WorkerControlInUse&) {
          caught = in_use;
        } catch (const Gecode::Search::InvalidWorkerRequest&) {
          caught = !in_use;
        }
        return caught && (live == (clone ? 1 : 0));
      }
    public:
      PBSConstructionFailure(void) : Base("Search::PBS::ConstructionFailure", TestTags::check()) {}
      bool run(void) override {
        for (unsigned int threads : {1U,4U})
          for (bool clone : {false,true})
            for (int failing : {-1,0,1}) {
              int live = 0;
              std::unique_ptr<PortfolioSeed> root(new PortfolioSeed(live));
              PortfolioSeed* seed = root.get();
              if (!clone)
                root.release();
              Gecode::Search::Options o;
              o.assets = 2;
              o.threads = threads;
              o.clone = clone;
              bool caught = false;
              try {
                if (failing == -1) {
                  Gecode::PBS<PortfolioSeed,RestartPortfolio> engine(seed,o);
                } else {
                  Gecode::SEBs sebs(2);
                  sebs[failing] = Gecode::pbs<PortfolioSeed,Gecode::RBS>(o);
                  sebs[1-failing] = Gecode::dfs<PortfolioSeed>();
                  Gecode::PBS<PortfolioSeed> engine(seed,sebs,o);
                }
              } catch (const Gecode::Search::UninitializedCutoff&) {
                caught = true;
              }
              if (!caught || (live != (clone ? 1 : 0)))
                return false;
            }
        for (unsigned int threads : {1U,4U})
          for (bool clone : {false,true})
            for (bool restart : {false,true})
              for (bool in_use : {false,true})
                if (!worker_control_failure(threads,clone,restart,in_use))
                  return false;
        return true;
      }
    };

    PBSConstructionFailure pbs_construction_failure;

    /// Pause before search or after its first node, then resume enumeration
    class WorkerControlSequentialPause : public Base {
      class Pause : public Gecode::Search::Stop {
        Gecode::Search::WorkerControl control;
        unsigned int node;
        std::atomic<bool> paused;
      public:
        std::promise<void> reached;
        Pause(const Gecode::Search::WorkerControl& c, unsigned int n)
          : control(c), node(n), paused(false) {}
        bool stop(const Gecode::Search::Statistics& s,
                  const Gecode::Search::Options&) override {
          if ((s.node >= node) && !paused.exchange(true)) {
            control.request(0U);
            reached.set_value();
          }
          return false;
        }
      };
      template<template<class> class E>
      bool check(unsigned int node, unsigned int threads) {
        Gecode::Search::WorkerControl control(node == 0U ? 0U : threads);
        Pause pause(control,node);
        Gecode::Search::Options o;
        o.threads = threads;
        o.worker_control = control;
        o.stop = &pause;
        // Best-solution search needs an objective. With min-value branching,
        // each solution found in search order is lexicographically greater
        // than the previous one, so BAB still returns several solutions.
        HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY,
                          E<HasSolutions>::best ? HTC_LEX_GR : HTC_NONE);
        E<HasSolutions> engine(&root,o);
        auto result = std::async(std::launch::async,[&] { return engine.next(); });
        auto reached = pause.reached.get_future();
        reached.wait();
        bool ok = result.wait_for(std::chrono::milliseconds(20)) ==
          std::future_status::timeout;
        control.request(1U);
        int solutions = 0;
        std::unique_ptr<HasSolutions> last(result.get());
        if (last)
          ++solutions;
        // Alternate expansion and shrinking at solution boundaries. This
        // exercises each request while preserving complete search.
        while (true) {
          control.request((solutions % 2) ? threads : 1U);
          HasSolutions* solution = engine.next();
          if (!solution)
            break;
          ++solutions;
          last.reset(solution);
        }
        // Resuming from before the first node runs a single worker, which
        // finds the lexicographically smallest solution first. BAB then
        // resizes at least once more before the search is exhausted.
        bool complete = E<HasSolutions>::best ?
          (last && last->best() && ((node != 0U) || (solutions >= 2))) :
          (solutions == root.solutions());
        return ok && (engine.end_reason() == EndReason::Exhausted) && complete;
      }
#ifdef GECODE_HAS_EXPERIMENTAL
      template<template<class> class E>
      bool heuristic(unsigned int node, unsigned int threads) {
        Gecode::Search::WorkerControl control(node == 0U ? 0U : 1U);
        Pause pause(control,node);
        TreeSpace root(TreeSpace::random(_rand,4),false);
        Gecode::Search::HeuristicGuidance g =
          TreeSpace::guidance(false,true,false,false);
        Gecode::Search::Options o;
        o.threads = threads;
        o.worker_control = control;
        o.stop = &pause;
        o.guidance = &g;
        E<TreeSpace> engine(&root,o);
        int last = std::numeric_limits<int>::max();
        auto consume = [&] {
          while (TreeSpace* s = engine.next()) {
            last = s->node().cost;
            delete s;
          }
        };
        auto result = std::async(std::launch::async,consume);
        pause.reached.get_future().wait();
        bool ok = result.wait_for(std::chrono::milliseconds(20)) ==
          std::future_status::timeout;
        control.request(threads);
        result.get();
        return ok && (engine.end_reason() == EndReason::Exhausted) && (last == (*root.graph)[0].best);
      }
#endif

    public:
      WorkerControlSequentialPause(void)
        : Base("Search::WorkerControl::SequentialPause", TestTags::check()) {}
      bool run(void) override {
#ifdef GECODE_HAS_THREADS
#ifdef GECODE_HAS_EXPERIMENTAL
        for (unsigned int node : {0U,1U})
          for (unsigned int threads : {1U,4U})
            if (!heuristic<Gecode::AStar>(node,threads) ||
                !heuristic<Gecode::CABS>(node,threads))
              return false;
#endif
        for (unsigned int threads : {1U,4U})
          if (!check<Gecode::DFS>(0U,threads) ||
              !check<Gecode::DFS>(1U,threads) ||
              !check<Gecode::BAB>(0U,threads) ||
              !check<Gecode::BAB>(1U,threads))
            return false;
        return true;
#else
        return true;
#endif
      }
    };

    WorkerControlSequentialPause worker_control_sequential_pause;

    /// Requests from other threads while workers search must not lose results
    class WorkerControlConcurrentResize : public Base {
    private:
      /// Drain \a engine while one thread per control resizes it randomly
      template<class Engine>
      bool drain(Engine& engine,
                 std::vector<Gecode::Search::WorkerControl> controls,
                 unsigned int capacity, int& solutions, bool& best) {
        std::atomic<bool> done(false);
        std::vector<std::thread> resizers;
        for (Gecode::Search::WorkerControl& control : controls)
          resizers.emplace_back([&done,&control,capacity,seed=_rand.next()] {
            Gecode::Support::RandomGenerator rand(seed);
            while (!done.load(std::memory_order_acquire)) {
              control.request(rand(capacity+1U));
              std::this_thread::yield();
            }
          });
        HasSolutions* last = nullptr;
        solutions = 0;
        while (HasSolutions* s = engine.next()) {
          ++solutions;
          delete last;
          last = s;
        }
        done.store(true,std::memory_order_release);
        for (std::thread& resizer : resizers)
          resizer.join();
        best = (last != nullptr) && last->best();
        delete last;
        return engine.end_reason() == EndReason::Exhausted;
      }
    public:
      WorkerControlConcurrentResize(void)
        : Base("Search::WorkerControl::ConcurrentResize", TestTags::check()) {}
      bool run(void) override {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControl;
        const unsigned int threads = 4U;
        int n;
        bool best;
        {
          HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY);
          WorkerControl control(threads);
          Gecode::Search::Options o;
          o.threads = threads;
          o.worker_control = control;
          Gecode::DFS<HasSolutions> dfs(&root,o);
          if (!drain(dfs,{control},threads,n,best) || (n != root.solutions()))
            return false;
        }
        {
          HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY,HTC_LEX_LE);
          WorkerControl control(threads);
          Gecode::Search::Options o;
          o.threads = threads;
          o.worker_control = control;
          Gecode::BAB<HasSolutions> bab(&root,o);
          if (!drain(bab,{control},threads,n,best) || !best)
            return false;
        }
        {
          // Restarts rebuild the leaf engine under the same control.
          HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY,HTC_LEX_LE);
          WorkerControl control(threads);
          Gecode::Search::Options o;
          o.threads = threads;
          o.worker_control = control;
          o.cutoff = Gecode::Search::Cutoff::geometric(1,2);
          Gecode::RBS<HasSolutions,Gecode::BAB> rbs(&root,o);
          if (!drain(rbs,{control},threads,n,best) || !best ||
              (rbs.statistics().restart == 0))
            return false;
        }
        {
          HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY,HTC_LEX_LE);
          WorkerControl controls[2] = {WorkerControl(threads),
                                       WorkerControl(threads)};
          Gecode::SEBs sebs(2);
          for (int i=0; i<2; i++) {
            Gecode::Search::Options leaf;
            leaf.threads = threads;
            leaf.worker_control = controls[i];
            sebs[i] = Gecode::bab<HasSolutions>(leaf);
          }
          Gecode::Search::Options o;
          o.threads = 2.0;
          Gecode::PBS<HasSolutions,Gecode::BAB> pbs(&root,sebs,o);
          if (!drain(pbs,{controls[0],controls[1]},threads,n,best) || !best)
            return false;
        }
#endif
        return true;
      }
    };

    WorkerControlConcurrentResize worker_control_concurrent_resize;

    /// Focused API and binding checks not covered by randomized search
    class WorkerControlAPI : public Base {
    private:
      template<class Exception, class Function>
      static bool throws(Function function) {
        try {
          function();
        } catch (const Exception&) {
          return true;
        } catch (...) {
        }
        return false;
      }
    public:
      WorkerControlAPI(void) : Base("Search::WorkerControl::API", TestTags::check()) {}
      bool run(void) override {
        using Gecode::Search::WorkerControl;
        WorkerControl empty;
        bool ok = !empty.engaged() && (empty.requested() == 0U) &&
          (empty.capacity() == 0U) &&
          throws<Gecode::Search::UninitializedWorkerControl>(
            [&] { empty.request(1U); });

        const unsigned int capacity = worker_capacity(2U);
        WorkerControl control(0U);
        WorkerControl copy(control);
        Gecode::Search::WorkerControlAccess::prepare_reuse(control);
        ok = ok && copy.engaged() && (copy.requested() == 0U);

        Gecode::Search::Options o;
        o.threads = 2.0;
        o.worker_control = control;
        SolveImmediate* root =
          new SolveImmediate(HTB_NONE,HTB_NONE,HTB_NONE);
#ifndef GECODE_HAS_THREADS
        ok = ok && throws<Gecode::Search::InvalidWorkerRequest>(
          [&] { Gecode::DFS<SolveImmediate> engine(root,o); });
        control.request(1U);
        ok = ok && throws<Gecode::Search::InvalidWorkerRequest>(
          [&] { control.request(0U); });
#endif
        {
          Gecode::DFS<SolveImmediate> engine(root,o);
          delete root;
          root = nullptr;
          ok = ok && (control.capacity() == capacity);
          ok = ok && throws<Gecode::Search::WorkerControlInUse>([&] {
            Gecode::Search::WorkerControlAccess::prepare_reuse(control);
          });
          control.request(1U);
          ok = ok && (copy.requested() == 1U) &&
            throws<Gecode::Search::InvalidWorkerRequest>(
              [&] { control.request(capacity+1U); });
          delete engine.next();
        }
        root = new SolveImmediate(HTB_NONE,HTB_NONE,HTB_NONE);
        ok = ok && throws<Gecode::Search::WorkerControlInUse>(
          [&] { Gecode::DFS<SolveImmediate> engine(root,o); });
        delete root;
        Gecode::Search::WorkerControlAccess::prepare_reuse(control);
        Gecode::Search::WorkerControlAccess::prepare_reuse(copy);
#ifdef GECODE_HAS_THREADS
        copy.request(0U);
        ok = ok && (control.requested() == 0U);
#endif
        ok = ok && (control.capacity() == capacity) &&
          throws<Gecode::Search::InvalidWorkerRequest>([&] {
            Gecode::Search::WorkerControlAccess::attach(control,capacity+1U);
          });
        root = new SolveImmediate(HTB_NONE,HTB_NONE,HTB_NONE);
        {
          Gecode::DFS<SolveImmediate> engine(root,o);
          delete root;
          copy.request(1U);
          delete engine.next();
        }
        return ok;
      }
    };

    WorkerControlAPI worker_control_api;

    /// A paused PBS asset must not hold the portfolio round open
    class WorkerControlPBSPause : public Base {
    public:
      WorkerControlPBSPause(void)
        : Base("Search::WorkerControl::PBS::Pause", TestTags::check()) {}
      bool run(void) override {
        bool ok = true;
        // A sequential portfolio must reject assets that could block its round.
        for (unsigned int requested : {0U,1U}) {
          Gecode::Search::Options leaf;
          leaf.worker_control = Gecode::Search::WorkerControl(requested);
          Gecode::SEBs sebs(2);
          sebs[0] = Gecode::dfs<SolveImmediate>(leaf);
          sebs[1] = Gecode::dfs<SolveImmediate>();
          SolveImmediate root(HTB_NONE,HTB_NONE,HTB_NONE);
          Gecode::Search::Options o;
          o.threads = 1.0;
          bool rejected = false;
          try {
            Gecode::PBS<SolveImmediate> pbs(&root,sebs,o);
          } catch (const Gecode::Search::WorkerControlInUse&) {
            rejected = true;
            delete sebs[0];
            delete sebs[1];
          }
          ok = ok && rejected;
        }
#ifdef GECODE_HAS_THREADS
        for (unsigned int threads : {1U,2U})
          for (bool restart : {false,true}) {
            Gecode::Search::WorkerControl paused(0U);
            Gecode::Search::Options paused_options;
            paused_options.threads = threads;
            paused_options.worker_control = paused;

            Gecode::Search::Options active_options;
            active_options.threads = 2.0;

            Gecode::SEBs sebs(2);
            if (restart) {
              paused_options.cutoff = Gecode::Search::Cutoff::constant(1000);
              sebs[0] = Gecode::rbs<SolveImmediate,Gecode::DFS>(paused_options);
            } else {
              sebs[0] = Gecode::dfs<SolveImmediate>(paused_options);
            }
            sebs[1] = Gecode::dfs<SolveImmediate>(active_options);

            Gecode::Search::Options portfolio_options;
            portfolio_options.threads = 2.0;
            SolveImmediate* root =
              new SolveImmediate(HTB_NONE,HTB_NONE,HTB_NONE);
            Gecode::PBS<SolveImmediate,Gecode::DFS>
              pbs(root,sebs,portfolio_options);
            delete root;

            SolveImmediate* solution = pbs.next();
            ok = ok && (solution != nullptr) && (paused.requested() == 0U);
            delete solution;
          }
#endif
        return ok;
      }
    };

    WorkerControlPBSPause worker_control_pbs_pause;

    /// Scalar integer objective used for comparison tests
    class MinObjective : public IntMinimizeSpace {
    public:
      IntVar x;
      MinObjective(int l, int u) : x(*this,l,u) {}
      MinObjective(MinObjective& s) : IntMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new MinObjective(*this); }
      virtual IntVar cost(void) const { return x; }
    };

    /// Scalar maximization objective used to check objective families
    class MaxObjective : public IntMaximizeSpace {
    public:
      IntVar x;
      MaxObjective(int v) : x(*this,v,v) {}
      MaxObjective(MaxObjective& s) : IntMaximizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new MaxObjective(*this); }
      virtual IntVar cost(void) const { return x; }
    };

    /// Objective used to test external incumbent updates
    class ExternalObjective : public IntMinimizeSpace {
    public:
      /// Per-test count of posted improvement constraints (optional)
      std::atomic<int>* constraints;
      IntVar x;
      ExternalObjective(std::atomic<int>* c) : constraints(c), x(*this,0,10) {
        Gecode::branch(*this,x,INT_VAL_MIN());
      }
      ExternalObjective(int v) : constraints(nullptr), x(*this,v,v) {}
      ExternalObjective(ExternalObjective& s)
        : IntMinimizeSpace(s), constraints(s.constraints) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new ExternalObjective(*this); }
      virtual IntVar cost(void) const { return x; }
      virtual void constrain(const Space& s) {
        if (constraints != nullptr)
          constraints->fetch_add(1);
        IntMinimizeSpace::constrain(s);
      }
      virtual void hub(HubContext& hc) {
        // Keep the branchers for the assets of a portfolio
        if (hc.engine() == MetaEngine::Restart)
          Space::hub(hc);
      }
    };

    /// Small objective whose solutions arrive from worst to best
    class ParallelObjective : public IntMinimizeSpace {
    public:
      IntVar x;
      ParallelObjective(void) : x(*this,0,10) {
        Gecode::branch(*this,x,INT_VAL_MAX());
      }
      ParallelObjective(int v) : x(*this,v,v) {}
      ParallelObjective(ParallelObjective& s) : IntMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new ParallelObjective(*this); }
      virtual IntVar cost(void) const { return x; }
      virtual void hub(HubContext& hc) {
        // Keep the branchers for the assets of a portfolio
        if (hc.engine() == MetaEngine::Restart)
          Space::hub(hc);
      }
    };

    /// Comparison error raised by a model
    class ComparisonError : public Exception {
    public:
      ComparisonError(void) : Exception("ParallelObjective::compare",
                                        "model comparison failed") {}
    };

    /// Objective used to exercise asynchronous comparison failures
    class FailingParallelObjective : public Space {
    public:
      enum Failure { MISSING, THROWN, INCOMPARABLE };
      /// Per-test count of live spaces
      std::atomic<int>& live;
      IntVar x;
      Failure failure;
      FailingParallelObjective(Failure f, std::atomic<int>& l)
        : live(l), x(*this,0,10), failure(f) {
        live++;
        Gecode::branch(*this,x,INT_VAL_MAX());
      }
      FailingParallelObjective(FailingParallelObjective& s)
        : Space(s), live(s.live), failure(s.failure) {
        live++;
        x.update(*this,s.x);
      }
      virtual ~FailingParallelObjective(void) { live--; }
      virtual Space* copy(void) {
        return new FailingParallelObjective(*this);
      }
      virtual SpaceComparison compare(const Space& s) const {
        if (failure == MISSING)
          return Space::compare(s);
        if (failure == THROWN)
          throw ComparisonError();
        return SpaceComparison::Incomparable;
      }
      virtual void hub(HubContext& hc) {
        // Keep the branchers for the assets of a portfolio
        if (hc.engine() == MetaEngine::Restart)
          Space::hub(hc);
      }
    };


    /// Objective with a genuine partial-order result
    class IncomparableObjective : public ExternalObjective {
    public:
      IncomparableObjective(int v) : ExternalObjective(v) {}
      IncomparableObjective(IncomparableObjective& s) : ExternalObjective(s) {}
      virtual Space* copy(void) { return new IncomparableObjective(*this); }
      virtual SpaceComparison compare(const Space&) const {
        return SpaceComparison::Incomparable;
      }
    };

    /// Search model without objective comparison support
    class UnsupportedObjective : public Space {
    public:
      UnsupportedObjective(void) {}
      UnsupportedObjective(UnsupportedObjective& s) : Space(s) {}
      virtual Space* copy(void) { return new UnsupportedObjective(*this); }
    };

    /// Test sequential BAB and RBS external incumbent arbitration
    class ExternalIncumbent : public Base {
    private:
      template<class Engine>
      static bool updates(Engine& e, const std::atomic<int>& constraints) {
        ExternalObjective five(5), equal(5), worse(7), better(3);
        int n = constraints;
        e.constrain(five);
        int installed = constraints;
        e.constrain(equal);
        e.constrain(worse);
        if ((installed <= n) || (constraints != installed))
          return false;
        e.constrain(better);
        return constraints > installed;
      }
      /// Whether engine \a e reports an incomparable incumbent (deletes \a e)
      static bool rejects_incomparable(Gecode::Search::Engine* e) {
        std::unique_ptr<Gecode::Search::Engine> engine(e);
        IncomparableObjective incomparable(4), incumbent(5);
        engine->constrain(incumbent);
        try {
          engine->constrain(incomparable);
        } catch (const Gecode::Search::Incomparable&) {
          return true;
        }
        return false;
      }
    public:
      ExternalIncumbent(void) : Base("Search::ExternalIncumbent") {}
      virtual bool run(void) {
        Gecode::Search::Options o;
        std::atomic<int> constraints(0);
        ExternalObjective* bm = new ExternalObjective(&constraints);
        Gecode::Search::Engine* bab = Gecode::Search::babengine(bm,o);
        delete bm;
        if (!updates(*bab,constraints)) {
          delete bab;
          return false;
        }
        delete bab;

        o.cutoff = Gecode::Search::Cutoff::constant(10);
        ExternalObjective* rm = new ExternalObjective(&constraints);
        Gecode::Search::Engine* rbs =
          Gecode::Search::build<ExternalObjective,
            Gecode::RBS<ExternalObjective,Gecode::BAB> >(rm,o);
        delete rm;
        if (!updates(*rbs,constraints)) {
          delete rbs;
          return false;
        }
        delete rbs;

        ExternalObjective im(nullptr);
        if (!rejects_incomparable(Gecode::Search::babengine(&im,Gecode::Search::Options())))
          return false;
#ifdef GECODE_HAS_EXPERIMENTAL
        Gecode::Search::HeuristicGuidance g(Gecode::Sense::Minimize,
          [](const Space& s) -> std::int64_t {
            return static_cast<const ExternalObjective&>(s).x.min();
          });
        for (double threads : {1.0,2.0}) {
          Gecode::Search::Options ho;
          ho.threads = threads;
          ho.guidance = &g;
          if (!rejects_incomparable(Gecode::Search::build<ExternalObjective,Gecode::AStar>(&im,ho)) ||
              !rejects_incomparable(Gecode::Search::build<ExternalObjective,Gecode::CABS>(&im,ho)))
            return false;
        }
#endif

        UnsupportedObjective unsupported;
        UnsupportedObjective* um =
          static_cast<UnsupportedObjective*>(unsupported.clone());
        Gecode::Search::Engine* missing = Gecode::Search::babengine(
          um,Gecode::Search::Options());
        delete um;
        missing->constrain(unsupported);
        try {
          missing->constrain(unsupported);
          delete missing;
          return false;
        } catch (const SpaceNoComparison&) {}
        delete missing;
        return true;
      }
    };

    ExternalIncumbent external_incumbent;

#ifdef GECODE_HAS_EXPERIMENTAL
    /// Test that a throwing bound in an external update leaks no clone
    class ThrowingBound : public Base {
    private:
      /// Objective that counts its live spaces
      class CountedObjective : public IntMinimizeSpace {
      public:
        std::atomic<int>& live;
        IntVar x;
        CountedObjective(int v, std::atomic<int>& l) : live(l), x(*this,v,v) {
          live++;
        }
        CountedObjective(CountedObjective& s)
          : IntMinimizeSpace(s), live(s.live) {
          live++;
          x.update(*this,s.x);
        }
        virtual ~CountedObjective(void) { live--; }
        virtual Space* copy(void) { return new CountedObjective(*this); }
        virtual IntVar cost(void) const { return x; }
      };
      /// Whether a throwing bound in engine \a E's constrain leaks nothing
      template<template<class> class E>
      static bool check(void) {
        std::atomic<int> live(0);
        Gecode::Search::HeuristicGuidance g(Gecode::Sense::Minimize,
          [](const Space& s) -> std::int64_t {
            int v = static_cast<const CountedObjective&>(s).x.val();
            if (v == 3)
              throw Gecode::Exception("ThrowingBound","bound");
            return v;
          });
        Gecode::Search::Options o;
        o.guidance = &g;
        bool thrown = false;
        {
          CountedObjective root(5,live);
          std::unique_ptr<Gecode::Search::Engine> engine(
            Gecode::Search::build<CountedObjective,E>(&root,o));
          CountedObjective incumbent(3,live);
          try {
            engine->constrain(incumbent);
          } catch (const Gecode::Exception&) {
            thrown = true;
          }
        }
        return thrown && (live == 0);
      }
    public:
      ThrowingBound(void) : Base("Search::ThrowingBound") {}
      virtual bool run(void) {
        return check<Gecode::AStar>() && check<Gecode::CABS>();
      }
    };

    ThrowingBound throwing_bound;
#endif

#ifdef GECODE_HAS_THREADS
    /// Test parallel BAB solution arbitration and failure delivery
    class ParallelBABComparison : public Base {
    private:
      static Gecode::Search::Options options(void) {
        Gecode::Search::Options o;
        o.threads = 2;
        return o;
      }
      static bool resetSearch(Gecode::Search::Engine* e) {
        e->reset(new ParallelObjective);
        int previous = 11;
        ParallelObjective* s;
        while ((s = static_cast<ParallelObjective*>(e->next())) != nullptr) {
          int value = s->x.val();
          delete s;
          if (value >= previous)
            return false;
          previous = value;
        }
        return previous == 0;
      }
      static bool pending_bound(int bound) {
        Gecode::Search::Par::BAB<Gecode::Search::NoTraceRecorder>
          bab(nullptr,options());
        for (int v=10; v>=8; v--)
          bab.solution(new ParallelObjective(v));
        ParallelObjective incumbent(bound);
        bab.constrain(incumbent);
        for (int v=10; v>=8; v--) {
          if (v >= bound)
            continue;
          ParallelObjective* s =
            static_cast<ParallelObjective*>(bab.next());
          bool correct = (s != nullptr) && (s->x.val() == v);
          delete s;
          if (!correct) {
            olog << "Pending solution violates bound " << bound
                 << "; expected " << v << std::endl;
            return false;
          }
        }
        Space* s = bab.next();
        bool exhausted = (s == nullptr);
        delete s;
        if (!exhausted)
          olog << "Unexpected pending solution after bound " << bound
               << std::endl;
        return exhausted;
      }
      static bool pending_failure(void) {
        std::atomic<int> live(0);
        bool reported = false;
        bool preserved = false;
        {
          Gecode::Search::Par::BAB<Gecode::Search::NoTraceRecorder>
            bab(nullptr,options());
          bab.solution(new FailingParallelObjective(
            FailingParallelObjective::THROWN,live));
          delete bab.next();
          bab.solution(new FailingParallelObjective(
            FailingParallelObjective::THROWN,live));
          try { (void) bab.nogoods(); }
          catch (const ComparisonError&) { reported = true; }
          // An external update must report the original worker failure
          // before invoking this incompatible incumbent's comparison.
          UnsupportedObjective incumbent;
          try { bab.constrain(incumbent); }
          catch (const ComparisonError&) { preserved = true; }
          catch (const SpaceNoComparison&) {}
        }
        if (!reported || !preserved)
          olog << "Pending failure: nogoods reported=" << reported
               << ", constrain preserved=" << preserved << std::endl;
        return reported && preserved && (live == 0);
      }
      static bool missingFailure(void) {
        std::atomic<int> live(0);
        Gecode::Search::TimeStop stop(5000);
        Gecode::Search::Options o = options();
        o.stop = &stop;
        bool recovered = false;
        {
          // Declared after stop and live, so the engine is deleted first
          FailingParallelObjective* m =
            new FailingParallelObjective(FailingParallelObjective::MISSING,live);
          std::unique_ptr<Gecode::Search::Engine>
            e(Gecode::Search::babengine(m,o));
          delete m;
          bool repeated = false;
          try {
            while (Space* s = e->next()) delete s;
          } catch (const SpaceNoComparison&) {
            try { (void) e->next(); }
            catch (const SpaceNoComparison&) { repeated = true; }
          }
          if (!repeated)
            return false;
          recovered = resetSearch(e.get());
        }
        return recovered && (live == 0);
      }
      static bool thrownFailure(void) {
        std::atomic<int> live(0);
        Gecode::Search::TimeStop stop(5000);
        Gecode::Search::Options o = options();
        o.stop = &stop;
        bool recovered = false;
        {
          // Declared after stop and live, so the engine is deleted first
          FailingParallelObjective* m =
            new FailingParallelObjective(FailingParallelObjective::THROWN,live);
          std::unique_ptr<Gecode::Search::Engine>
            e(Gecode::Search::babengine(m,o));
          delete m;
          try {
            while (Space* s = e->next()) delete s;
            return false;
          } catch (const ComparisonError&) {}
          recovered = resetSearch(e.get());
        }
        return recovered && (live == 0);
      }
      static bool incomparableFailure(void) {
        std::atomic<int> live(0);
        Gecode::Search::TimeStop stop(5000);
        Gecode::Search::Options o = options();
        o.stop = &stop;
        bool recovered = false;
        {
          // Declared after stop and live, so the engine is deleted first
          FailingParallelObjective* m = new FailingParallelObjective(
            FailingParallelObjective::INCOMPARABLE,live);
          std::unique_ptr<Gecode::Search::Engine>
            e(Gecode::Search::babengine(m,o));
          delete m;
          try {
            while (Space* s = e->next()) delete s;
            return false;
          } catch (const Gecode::Search::Incomparable&) {}
          recovered = resetSearch(e.get());
        }
        return recovered && (live == 0);
      }
    public:
      ParallelBABComparison(void) : Base("Search::ParallelBABComparison") {}
      virtual bool run(void) {
        bool pending = true;
        for (int bound=11; bound>=7; bound--)
          if (!pending_bound(bound))
            pending = false;
        if (!pending_failure() || !pending)
          return false;
        Gecode::Search::TimeStop stop(5000);
        Gecode::Search::Options o = options();
        o.stop = &stop;
        ParallelObjective* m = new ParallelObjective;
        Gecode::BAB<ParallelObjective> bab(m,o);
        delete m;
        int previous = 11;
        ParallelObjective* s;
        while ((s = bab.next()) != nullptr) {
          int value = s->x.val();
          delete s;
          if (value >= previous)
            return false;
          previous = value;
        }
        if (previous != 0)
          return false;

        // A fresh deadline, so that a stop cannot pass for exhaustion
        Gecode::Search::TimeStop bounded_stop(5000);
        o.stop = &bounded_stop;
        m = new ParallelObjective;
        std::unique_ptr<Gecode::Search::Engine>
          bounded(Gecode::Search::babengine(m,o));
        delete m;
        Space* first = bounded->next();
        if (first == nullptr)
          return false;
        delete first;
        // This is stronger than every solution in the model, so it also
        // supersedes any results queued while the first result was returned.
        ParallelObjective bound(-1);
        bounded->constrain(bound);
        while ((s = static_cast<ParallelObjective*>(bounded->next())) !=
               nullptr) {
          int value = s->x.val();
          delete s;
          if (value >= -1)
            return false;
        }
        if (bounded->end_reason() != EndReason::Exhausted)
          return false;
        bounded.reset();
        return missingFailure() && thrownFailure() && incomparableFailure();
      }
    };

    ParallelBABComparison parallel_bab_comparison;
#endif

    /// Test portfolio comparison, external bounds, and nested failures
    class PortfolioComparison : public Base {
    private:
      static Gecode::Search::Options options(void) {
        Gecode::Search::Options o;
        o.assets = 2;
        o.threads = 2;
        return o;
      }
      static bool failure(FailingParallelObjective::Failure f,
                          double threads=2.0) {
        std::atomic<int> live(0);
        bool caught = false;
        {
          FailingParallelObjective* m = new FailingParallelObjective(f,live);
          Gecode::Search::Options o = options();
          o.threads = threads;
          Gecode::PBS<FailingParallelObjective,Gecode::BAB> pbs(m,o);
          delete m;
          try {
            while (Space* s = pbs.next()) delete s;
          } catch (const SpaceNoComparison&) {
            if (f != FailingParallelObjective::MISSING)
              return false;
            caught = true;
          } catch (const ComparisonError&) {
            caught = (f == FailingParallelObjective::THROWN);
          } catch (const Gecode::Search::Incomparable&) {
            caught = (f == FailingParallelObjective::INCOMPARABLE);
          }
        }
        return caught && (live == 0);
      }
      /// Check that an external bound reports a stored worker failure
      static bool preserved_failure(void) {
        std::atomic<int> live(0);
        bool reported = false;
        bool preserved = false;
        {
          FailingParallelObjective* m =
            new FailingParallelObjective(FailingParallelObjective::THROWN,live);
          std::unique_ptr<Gecode::Search::Engine> pbs(
            Gecode::Search::build<FailingParallelObjective,
              Gecode::PBS<FailingParallelObjective,Gecode::BAB> >(m,options()));
          delete m;
          try {
            while (Space* s = pbs->next()) delete s;
          } catch (const ComparisonError&) {
            reported = true;
          }
          // This incumbent cannot be compared, so a constrain that compares
          // before reporting the stored failure throws SpaceNoComparison.
          UnsupportedObjective incumbent;
          try { pbs->constrain(incumbent); }
          catch (const ComparisonError&) { preserved = true; }
          catch (const SpaceNoComparison&) {}
        }
        if (!reported || !preserved)
          olog << "Pending failure: next reported=" << reported
               << ", constrain preserved=" << preserved << std::endl;
        return reported && preserved && (live == 0);
      }
    public:
      PortfolioComparison(void) : Base("Search::PortfolioComparison") {}
      virtual bool run(void) {
        Gecode::Search::Options o = options();
        ParallelObjective* m = new ParallelObjective;
        Gecode::PBS<ParallelObjective,Gecode::BAB> pbs(m,o);
        delete m;
        int previous = 11;
        ParallelObjective* s;
        while ((s = pbs.next()) != nullptr) {
          int value = s->x.val();
          delete s;
          if (value >= previous)
            return false;
          previous = value;
        }
        if (previous != 0)
          return false;

        std::atomic<int> constraints(0);
        ExternalObjective* em = new ExternalObjective(&constraints);
        Gecode::Search::Engine* external =
          Gecode::Search::build<ExternalObjective,
            Gecode::PBS<ExternalObjective,Gecode::BAB> >(em,o);
        delete em;
        ExternalObjective five(5), equal(5), worse(7), better(3);
        int n = constraints;
        external->constrain(five);
        int installed = constraints;
        external->constrain(equal);
        external->constrain(worse);
        if ((installed <= n) ||
            (constraints != installed)) {
          delete external;
          return false;
        }
        external->constrain(better);
        if (constraints <= installed) {
          delete external;
          return false;
        }
        delete external;

        using namespace Gecode;
        Gecode::Search::Options so;
        so.threads = 1;
        so.cutoff = Gecode::Search::Cutoff::constant(1000000);
        SEBs sebs(2);
        sebs[0] = bab<ParallelObjective>(so);
        sebs[1] = rbs<ParallelObjective,Gecode::BAB>(so);
        m = new ParallelObjective;
        Gecode::PBS<ParallelObjective,Gecode::BAB> mixed(m,sebs,o);
        delete m;
        previous = 11;
        while ((s = mixed.next()) != nullptr) {
          int value = s->x.val();
          delete s;
          if (value >= previous)
            return false;
          previous = value;
        }
        return (previous == 0) &&
          failure(FailingParallelObjective::MISSING,1) &&
          failure(FailingParallelObjective::MISSING) &&
          failure(FailingParallelObjective::THROWN) &&
          failure(FailingParallelObjective::INCOMPARABLE) &&
          preserved_failure();
      }
    };

    PortfolioComparison portfolio_comparison;

#ifdef GECODE_HAS_FLOAT_VARS
    /// Stepped float objective for parallel BAB admission
    class ParallelFloatObjective : public FloatMinimizeSpace {
    public:
      FloatVar x;
      ParallelFloatObjective(void)
        : FloatMinimizeSpace(1.0), x(*this,8.0,10.0) {
        Gecode::branch(*this,x,FLOAT_VAL_SPLIT_MAX());
      }
      ParallelFloatObjective(FloatNum v)
        : FloatMinimizeSpace(1.0), x(*this,v,v) {}
      ParallelFloatObjective(ParallelFloatObjective& s)
        : FloatMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new ParallelFloatObjective(*this); }
      virtual FloatVar cost(void) const { return x; }
    };

    /// Scalar float minimization objective used for comparison and cut tests
    class FloatMinObjective : public FloatMinimizeSpace {
    public:
      FloatVar x;
      FloatMinObjective(FloatVal v, FloatNum s=0.0)
        : FloatMinimizeSpace(s), x(*this,v.min(),v.max()) {}
      FloatMinObjective(FloatMinObjective& s) : FloatMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new FloatMinObjective(*this); }
      virtual FloatVar cost(void) const { return x; }
    };

    /// Scalar float maximization objective used for comparison and cut tests
    class FloatMaxObjective : public FloatMaximizeSpace {
    public:
      FloatVar x;
      FloatMaxObjective(FloatVal v, FloatNum s=0.0)
        : FloatMaximizeSpace(s), x(*this,v.min(),v.max()) {}
      FloatMaxObjective(FloatMaxObjective& s) : FloatMaximizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new FloatMaxObjective(*this); }
      virtual FloatVar cost(void) const { return x; }
    };
#endif

    /// Lexicographic integer objective used for comparison tests
    class LexObjective : public IntLexMinimizeSpace {
    public:
      IntVarArray x;
      LexObjective(int a, int l, int u, int n=2) : x(*this,n,l,u) {
        rel(*this,x[0],IRT_EQ,a);
        (void) status();
      }
      LexObjective(LexObjective& s) : IntLexMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new LexObjective(*this); }
      virtual IntVarArgs cost(void) const { return x; }
    };

#ifdef GECODE_HAS_FLOAT_VARS
    /// Test float objective ranking and compatibility with stepped cuts
    class FloatObjectiveComparison : public Base {
    private:
      template<class Objective>
      static bool admitted(FloatVal candidate, FloatNum step,
                           const Objective& incumbent) {
        Objective c(candidate,step);
        c.constrain(incumbent);
        return c.status() != SS_FAILED;
      }
      /// Whether constructing \a Objective with step \a step is rejected
      template<class Objective>
      static bool rejects_step(FloatNum step) {
        try {
          Objective o(10.0,step);
        } catch (const MiniModel::ArgumentOutOfRange&) {
          return true;
        }
        return false;
      }
    public:
      FloatObjectiveComparison(void)
        : Base("Search::FloatObjectiveComparison") {}
      virtual bool run(void) {
        // A negative step would admit candidates that are not better
        const FloatNum nan = std::numeric_limits<FloatNum>::quiet_NaN();
        if (!rejects_step<FloatMinObjective>(-1.0) ||
            !rejects_step<FloatMaxObjective>(-1.0) ||
            !rejects_step<FloatMinObjective>(nan) ||
            !rejects_step<FloatMaxObjective>(nan))
          return false;
        const FloatNum next = std::nextafter(1.0,2.0);
        FloatMinObjective m9(9.5,1.0), m10(10.0,1.0),
          m10b(10.0,1.0), mzero(10.0), madj(FloatVal(1.0,next));
        FloatMaxObjective x11(10.5,1.0), x10(10.0,1.0),
          x10b(10.0,1.0), xadj(FloatVal(1.0,next));
        if ((m9.compare(m10) != SpaceComparison::Better) ||
            (m10.compare(m9) != SpaceComparison::Worse) ||
            (m10.compare(m10b) != SpaceComparison::Equivalent) ||
            (x11.compare(x10) != SpaceComparison::Better) ||
            (x10.compare(x11) != SpaceComparison::Worse) ||
            (x10.compare(x10b) != SpaceComparison::Equivalent) ||
            (madj.compare(FloatMinObjective(next)) != SpaceComparison::Equivalent) ||
            (xadj.compare(FloatMaxObjective(1.0)) != SpaceComparison::Equivalent))
          return false;

        // Equal keys give identical cuts; better keys only tighten them.
        const FloatVal probes[] = {FloatVal(8.4), FloatVal(8.5),
                                   FloatVal(8.9), FloatVal(9.0)};
        for (unsigned int i=0; i<sizeof(probes)/sizeof(probes[0]); i++) {
          if (admitted<FloatMinObjective>(probes[i],1.0,m10) !=
              admitted<FloatMinObjective>(probes[i],1.0,m10b))
            return false;
          if (admitted<FloatMinObjective>(probes[i],1.0,m9) &&
              !admitted<FloatMinObjective>(probes[i],1.0,m10))
            return false;
        }
        if (admitted<FloatMinObjective>(FloatVal(10.0),0.0,mzero) ||
            !admitted<FloatMinObjective>(FloatVal(9.0),0.0,mzero) ||
            admitted<FloatMaxObjective>(FloatVal(11.0),1.0,x10) ||
            !admitted<FloatMaxObjective>(FloatVal(11.1),1.0,x10))
          return false;

        FloatVal before_m=m9.cost().val();
        FloatVal before_m_other=m10.cost().val();
        FloatVal before_x=x11.cost().val();
        FloatVal before_x_other=x10.cost().val();
        (void) m9.compare(m10); (void) x11.compare(x10);
        if ((m9.cost().val().min() != before_m.min()) ||
            (m9.cost().val().max() != before_m.max()) ||
            (m10.cost().val().min() != before_m_other.min()) ||
            (m10.cost().val().max() != before_m_other.max()) ||
            (x11.cost().val().min() != before_x.min()) ||
            (x11.cost().val().max() != before_x.max()) ||
            (x10.cost().val().min() != before_x_other.min()) ||
            (x10.cost().val().max() != before_x_other.max()))
          return false;
        try { (void) mzero.compare(m10); return false; }
        catch (const SpaceIncompatibleObjectives&) {}
        try { (void) m10.compare(x10); return false; }
        catch (const DynamicCastFailed&) {}

#ifdef GECODE_HAS_THREADS
        // A bound better by less than the step still replaces the incumbent.
        Gecode::Search::TimeStop stop(5000);
        Gecode::Search::Options o;
        o.threads = 2;
        o.stop = &stop;
        ParallelFloatObjective* root = new ParallelFloatObjective;
        std::unique_ptr<Gecode::Search::Engine>
          bab(Gecode::Search::babengine(root,o));
        delete root;
        bab->constrain(ParallelFloatObjective(10.0));
        bab->constrain(ParallelFloatObjective(9.5));
        int found = 0;
        bool substep = true;
        while (Space* s = bab->next()) {
          found++;
          substep = substep &&
            (static_cast<ParallelFloatObjective*>(s)->x.val().max() < 8.5);
          delete s;
        }
        substep = substep && (found == 1) &&
          (bab->end_reason() == EndReason::Exhausted);
        if (!substep)
          return false;
#endif
        return true;
      }
    };

    FloatObjectiveComparison float_objective_comparison;
#endif

    /// Lexicographic maximization objective used for comparison tests
    class LexMaxObjective : public IntLexMaximizeSpace {
    public:
      IntVarArray x;
      LexMaxObjective(int a) : x(*this,2,0,2) {
        rel(*this,x[0],IRT_EQ,a);
        (void) status();
      }
      LexMaxObjective(LexMaxObjective& s) : IntLexMaximizeSpace(s) {
        x.update(*this,s.x);
      }
      virtual Space* copy(void) { return new LexMaxObjective(*this); }
      virtual IntVarArgs cost(void) const { return x; }
    };

    /// Space without objective comparison support
    class PlainSpace : public Space {
    public:
      PlainSpace(void) {}
      PlainSpace(PlainSpace& s) : Space(s) {}
      virtual Space* copy(void) { return new PlainSpace(*this); }
    };

    /// Test objective comparison independently of search arbitration
    class Comparison : public Base {
    public:
      Comparison(void) : Base("Search::Comparison") {}
      virtual bool run(void) {
        MinObjective one(1,1), two(2,2), one_again(1,1), open(0,2);
        if ((one.compare(two) != SpaceComparison::Better) ||
            (two.compare(one) != SpaceComparison::Worse) ||
            (one.compare(one_again) != SpaceComparison::Equivalent))
          return false;
        MaxObjective high(2), low(1);
        if ((high.compare(low) != SpaceComparison::Better) ||
            (low.compare(high) != SpaceComparison::Worse))
          return false;
        if ((compare_int(1,2,Sense::Minimize) != SpaceComparison::Better) ||
            (compare_int(2,1,Sense::Minimize) != SpaceComparison::Worse) ||
            (compare_int(1,1,Sense::Minimize) != SpaceComparison::Equivalent) ||
            (compare_int(2,1,Sense::Maximize) != SpaceComparison::Better) ||
            (compare_int(1,2,Sense::Maximize) != SpaceComparison::Worse) ||
            (compare_int(1,1,Sense::Maximize) != SpaceComparison::Equivalent))
          return false;
#ifdef GECODE_HAS_FLOAT_VARS
        if ((compare_float(0.5,1.0,Sense::Minimize) !=
             SpaceComparison::Better) ||
            (compare_float(1.0,0.5,Sense::Minimize) !=
             SpaceComparison::Worse) ||
            (compare_float(1.0,0.5,Sense::Maximize) !=
             SpaceComparison::Better) ||
            (compare_float(0.5,0.5,Sense::Maximize) !=
             SpaceComparison::Equivalent))
          return false;
#endif
        LexObjective lp(1,0,2), lq(2,0,2);
        if ((lp.compare(lq) != SpaceComparison::Better) ||
            (lq.compare(lp) != SpaceComparison::Worse))
          return false;
        LexMaxObjective lmp(2), lmq(1);
        if ((lmp.compare(lmq) != SpaceComparison::Better) ||
            (lmq.compare(lmp) != SpaceComparison::Worse))
          return false;
        try {
          (void) open.compare(one);
          return false;
        } catch (const Int::ValOfUnassignedVar&) {}
        try {
          (void) one.compare(high);
          return false;
        } catch (const DynamicCastFailed&) {}
        LexObjective short_cost(1,0,2,1);
        try {
          (void) lp.compare(short_cost);
          return false;
        } catch (const SpaceIncompatibleObjectives&) {}
        {
          IntVarArgs a;
          IntVarArgs b;
          a << IntVar(lp,1,1) << IntVar(lp,3,3);
          b << IntVar(lp,1,1) << IntVar(lp,2,2);
          if ((compare_lex(a,b,Sense::Minimize) != SpaceComparison::Worse) ||
              (compare_lex(a,b,Sense::Maximize) != SpaceComparison::Better) ||
              (compare_lex(a,a,Sense::Minimize) !=
               SpaceComparison::Equivalent))
            return false;
        }
        PlainSpace plain;
        try {
          (void) plain.compare(plain);
          return false;
        } catch (const SpaceNoComparison&) {}
        return true;
      }
    };

    /// %Test for depth-first search
    template<class Model>
    class DFS : public Test {
    private:
      /// Minimal recomputation distance
      unsigned int c_d;
      /// Adaptive recomputation distance
      unsigned int a_d;
      /// Number of threads
      unsigned int t;
    public:
      /// Initialize test
      DFS(TestTags tags,
          HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int c_d0, unsigned int a_d0, unsigned int t0)
        : Test(tags,"DFS::"+Model::name()+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3)+"::"+
               str(c_d0)+"::"+str(a_d0)+"::"+str(t0),
               htb1,htb2,htb3), c_d(c_d0), a_d(a_d0), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.c_d = c_d;
        o.a_d = a_d;
        o.threads = t;
        o.stop = &f;
        Gecode::DFS<Model> dfs(m,o);
        int n = m->solutions();
        delete m;
        while (true) {
          Model* s = dfs.next();
          if (s != nullptr) {
            n--; delete s;
          }
          if ((s == nullptr) && (dfs.end_reason() != EndReason::Limit))
            break;
          f.limit(f.limit()+2);
        }
        return (n == 0) && (dfs.end_reason() == EndReason::Exhausted);
      }
    };

    /// %Test for limited discrepancy search
    template<class Model>
    class LDS : public Test {
    private:
      /// Number of threads
      unsigned int t;
    public:
      /// Initialize test
      LDS(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int t0)
        : Test("LDS::"+Model::name()+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3)+"::"+str(t0),
               htb1,htb2,htb3), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.threads = t;
        o.d_l = 50;
        o.stop = &f;
        Gecode::LDS<Model> lds(m,o);
        int n = m->solutions();
        delete m;
        while (true) {
          Model* s = lds.next();
          if (s != nullptr) {
            n--; delete s;
          }
          if ((s == nullptr) && (lds.end_reason() != EndReason::Limit))
            break;
          f.limit(f.limit()+2);
        }
        return (n == 0) && (lds.end_reason() == EndReason::Exhausted);
      }
    };

    /// %Test for best solution search
    template<class Model>
    class BAB : public Test {
    private:
      /// Minimal recomputation distance
      unsigned int c_d;
      /// Adaptive recomputation distance
      unsigned int a_d;
      /// Number of threads
      unsigned int t;
    public:
      /// Initialize test
      BAB(TestTags tags, HowToConstrain htc,
          HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int c_d0, unsigned int a_d0, unsigned int t0)
        : Test(tags,"BAB::"+Model::name()+"::"+str(htc)+"::"+
               str(htb1)+"::"+str(htb2)+"::"+str(htb3)+"::"+
               str(c_d0)+"::"+str(a_d0)+"::"+str(t0),
               htb1,htb2,htb3,htc), c_d(c_d0), a_d(a_d0), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3,htc);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.c_d = c_d;
        o.a_d = a_d;
        o.threads = t;
        o.stop = &f;
        Gecode::BAB<Model> bab(m,o);
        delete m;
        Model* b = nullptr;
        while (true) {
          Model* s = bab.next();
          if (s != nullptr) {
            delete b; b=s;
          }
          if ((s == nullptr) && (bab.end_reason() != EndReason::Limit))
            break;
          f.limit(f.limit()+2);
        }
        bool ok = ((b == nullptr) || b->best()) && (bab.end_reason() == EndReason::Exhausted);
        delete b;
        return ok;
      }
    };

    /// %Test for restart-based search
    template<class Model, template<class> class Engine>
    class RBS : public Test {
    private:
      /// Number of threads
      unsigned int t;
    public:
      /// Initialize test, enumerating all solutions without an objective
      RBS(const std::string& e, unsigned int t0, HowToConstrain htc0=HTC_NONE)
        : Test("RBS::"+e+"::"+Model::name()+"::"+str(t0),
               HTB_BINARY,HTB_BINARY,HTB_BINARY,htc0), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3,htc);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.threads = t;
        o.stop = &f;
        o.d_l = 100;
        o.cutoff = Gecode::Search::Cutoff::geometric(1,2);
        Gecode::RBS<Model,Engine> rbs(m,o);
        int n = m->solutions();
        delete m;
        // With an objective, every solution must improve on the previous one
        Model* b = nullptr;
        bool improving = true;
        while (true) {
          Model* s = rbs.next();
          if (s != nullptr) {
            n--;
            if (htc == HTC_NONE) {
              delete s;
            } else {
              if ((b != nullptr) && (s->compare(*b) != SpaceComparison::Better))
                improving = false;
              delete b; b = s;
            }
          }
          if ((s == nullptr) && (rbs.end_reason() != EndReason::Limit))
            break;
          f.limit(f.limit()+2);
        }
        if (htc == HTC_NONE)
          return (n == 0) && (rbs.end_reason() == EndReason::Exhausted);
        bool ok = improving && (b != nullptr) && b->best();
        delete b;
        return ok;
      }
    };

    /// Space whose spokes add constraints that keep some solution
    class SatPreservingSpoke : public Gecode::Space {
    public:
      /// Coverage that spoke() declares
      Gecode::SpokeCoverage coverage;
      /// Variable with four values
      Gecode::IntVar x;
      /// Four variables with three values
      Gecode::IntVarArray y;
      /// Initialize with declared coverage \a c
      SatPreservingSpoke(Gecode::SpokeCoverage c)
        : coverage(c), x(*this,0,3), y(*this,4,0,2) {
        Gecode::branch(*this,x,Gecode::INT_VAL_MIN());
        Gecode::branch(*this,y,Gecode::INT_VAR_NONE(),Gecode::INT_VAL_MIN());
      }
      /// Clone \a s
      SatPreservingSpoke(SatPreservingSpoke& s)
        : Gecode::Space(s), coverage(s.coverage) {
        x.update(*this,s.x);
        y.update(*this,s.y);
      }
      Gecode::Space* copy(void) override {
        return new SatPreservingSpoke(*this);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        // The y must be distinct, which is impossible, for x < 3 in the
        // first episode and for x > 0 later: either keeps a solution
        Gecode::BoolVar b = (sc.episode() == 0) ?
          Gecode::expr(*this,x < 3) : Gecode::expr(*this,x > 0);
        for (int i=0; i<y.size(); i++)
          for (int j=i+1; j<y.size(); j++)
            Gecode::rel(*this,y[i],Gecode::IRT_NQ,y[j],
                        Gecode::Reify(b,Gecode::RM_IMP));
        sc.coverage(coverage);
      }
    };

    /// No-goods and proofs of spokes follow their declared coverage
    class RBSSpokeCoverage : public Base {
      /// Run RBS with spokes declaring \a c, return the solution or nullptr
      static SatPreservingSpoke* solve(Gecode::SpokeCoverage c,
                                       EndReason& r) {
        SatPreservingSpoke* m = new SatPreservingSpoke(c);
        Gecode::Search::FailStop stop(10000);
        Gecode::Search::Options o;
        o.threads = 1;
        o.nogoods_limit = 128;
        o.stop = &stop;
        o.cutoff = Gecode::Search::Cutoff::constant(8);
        Gecode::RBS<SatPreservingSpoke,Gecode::DFS> rbs(m,o);
        delete m;
        SatPreservingSpoke* s = rbs.next();
        r = rbs.end_reason();
        return s;
      }
    public:
      RBSSpokeCoverage(void) : Base("Search::RBS::SpokeCoverage") {}
      bool run(void) override {
        EndReason r;
        // Declared Reduced: no-goods stay in their spoke, x = 0 remains.
        // These spokes are not Full, declaring them so is undefined.
        SatPreservingSpoke* s = solve(Gecode::SpokeCoverage::Reduced,r);
        const bool reduced = (s != nullptr) && (s->x.val() == 0);
        delete s;
        return reduced;
      }
    };

    RBSSpokeCoverage rbs_spoke_coverage;

    /// Space whose first spoke search is restricted to x2, x3 in {0,1}
    class RestrictedFirstSpoke : public Gecode::IntMinimizeSpace {
    public:
      /// Distinct variables, the cost is x0
      Gecode::IntVarArray x;
      RestrictedFirstSpoke(void) : x(*this,4,0,3) {
        Gecode::distinct(*this,x);
        Gecode::branch(*this,x,Gecode::INT_VAR_NONE(),Gecode::INT_VAL_MIN());
      }
      RestrictedFirstSpoke(RestrictedFirstSpoke& s)
        : Gecode::IntMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      Gecode::Space* copy(void) override {
        return new RestrictedFirstSpoke(*this);
      }
      Gecode::IntVar cost(void) const override {
        return x[0];
      }
      void spoke(Gecode::SpokeContext& sc) override {
        if ((sc.engine() == Gecode::MetaEngine::Restart) &&
            (sc.event() == Gecode::MetaEvent::Start)) {
          Gecode::dom(*this,x[2],0,1);
          Gecode::dom(*this,x[3],0,1);
          sc.coverage(Gecode::SpokeCoverage::Partial);
        }
      }
    };

    /// No-goods from a partial spoke search are not posted in the hub
    class RBSPartialNoGoods : public Base {
    public:
      RBSPartialNoGoods(void) : Base("Search::RBS::PartialNoGoods") {}
      bool run(void) override {
        RestrictedFirstSpoke* m = new RestrictedFirstSpoke;
        Gecode::Search::Options o;
        o.nogoods_limit = 128U;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<RestrictedFirstSpoke,Gecode::BAB> rbs(m,o);
        delete m;
        // The restricted search finds x0 = 2 and rules out x0 in {0,1}
        int best = -1;
        while (RestrictedFirstSpoke* s = rbs.next()) {
          best = s->x[0].val();
          delete s;
        }
        return (rbs.end_reason() == EndReason::Exhausted) && (best == 0);
      }
    };

    RBSPartialNoGoods rbs_partial_nogoods;

    /// Space whose hub posts a brancher after a spoke posted one
    class LateHubBrancher : public Gecode::Space {
    public:
      /// Variables branched on by the spokes
      Gecode::IntVarArray y;
      /// Variables branched on by the hub after the first episode
      Gecode::IntVarArray z;
      /// Whether the hub branches on z
      bool z_branching;
      LateHubBrancher(void)
        : y(*this,5,0,2), z(*this,2,0,1), z_branching(false) {
        // For y0 = 0 the other y must be distinct, which is impossible
        Gecode::BoolVar b = Gecode::expr(*this,y[0] == 0);
        for (int i=1; i<y.size(); i++)
          for (int j=i+1; j<y.size(); j++)
            Gecode::rel(*this,y[i],Gecode::IRT_NQ,y[j],
                        Gecode::Reify(b,Gecode::RM_IMP));
      }
      LateHubBrancher(LateHubBrancher& s)
        : Gecode::Space(s), z_branching(s.z_branching) {
        y.update(*this,s.y);
        z.update(*this,s.z);
      }
      Gecode::Space* copy(void) override {
        return new LateHubBrancher(*this);
      }
      void hub(Gecode::HubContext& hc) override {
        if ((hc.event() == Gecode::MetaEvent::Limit) && !z_branching) {
          Gecode::branch(*this,z,Gecode::INT_VAR_NONE(),
                         Gecode::INT_VAL_MIN());
          z_branching = true;
        }
        // The no-goods refer to the brancher of the first spoke
        Gecode::Space::hub(hc);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        // The first episode searches y0 = 0 first and reaches the cutoff
        Gecode::branch(*this,y,Gecode::INT_VAR_NONE(),
                       (sc.episode() == 0) ? Gecode::INT_VAL_MIN() :
                       Gecode::INT_VAL_MAX());
      }
    };

    /// A brancher posted by hub() does not take the identity of a spoke brancher
    class RBSLateHubBrancher : public Base {
    public:
      RBSLateHubBrancher(void) : Base("Search::RBS::LateHubBrancher") {}
      bool run(void) override {
        LateHubBrancher* m = new LateHubBrancher;
        Gecode::Search::Options o;
        o.threads = 1;
        o.nogoods_limit = 128U;
        o.cutoff = Gecode::Search::Cutoff::constant(3);
        Gecode::RBS<LateHubBrancher,Gecode::DFS> rbs(m,o);
        delete m;
        // A no-good taken for one on z would rule out z = (0,0)
        LateHubBrancher* s = rbs.next();
        const bool ok = (s != nullptr) &&
          (s->z[0].val() == 0) && (s->z[1].val() == 0);
        delete s;
        return ok;
      }
    };

    RBSLateHubBrancher rbs_late_hub_brancher;

    /// Space without solutions whose search tree has several failures
    class NoSolution : public Gecode::IntMinimizeSpace {
    public:
      /// Four variables that must be distinct with three values
      Gecode::IntVarArray x;
      NoSolution(void) : x(*this,4,0,2) {
        for (int i=0; i<x.size(); i++)
          for (int j=i+1; j<x.size(); j++)
            Gecode::rel(*this,x[i],Gecode::IRT_NQ,x[j]);
        Gecode::branch(*this,x,Gecode::INT_VAR_NONE(),Gecode::INT_VAL_MIN());
      }
      NoSolution(NoSolution& s) : Gecode::IntMinimizeSpace(s) {
        x.update(*this,s.x);
      }
      Gecode::Space* copy(void) override {
        return new NoSolution(*this);
      }
      Gecode::IntVar cost(void) const override {
        return x[0];
      }
      void hub(Gecode::HubContext& hc) override {
        // Keep the branchers for the assets of a portfolio
        if (hc.engine() == Gecode::MetaEngine::Restart)
          Gecode::IntMinimizeSpace::hub(hc);
      }
    };

    /// Space without solutions whose hub fails at a restart after a limit
    class FailingHub : public NoSolution {
    public:
      FailingHub(void) = default;
      FailingHub(FailingHub& s) : NoSolution(s) {}
      Gecode::Space* copy(void) override {
        return new FailingHub(*this);
      }
      void hub(Gecode::HubContext& hc) override {
        if (hc.event() == Gecode::MetaEvent::Limit)
          fail();
      }
    };

    /// A hub failing at a restart after a limit proves exhaustion
    class RBSFailedHub : public Base {
    public:
      RBSFailedHub(void) : Base("Search::RBS::FailedHub") {}
      bool run(void) override {
        for (unsigned int t : {1U,2U}) {
          FailingHub* m = new FailingHub;
          Gecode::Search::Options o;
          o.threads = t;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::RBS<FailingHub,Gecode::DFS> rbs(m,o);
          delete m;
          if (FailingHub* s = rbs.next()) {
            delete s;
            return false;
          }
          Gecode::Search::Statistics s = rbs.statistics();
          // The failures of the last spoke search are counted once
          if ((rbs.end_reason() != EndReason::Exhausted) ||
              (s.restart != 0U) || (s.fail < 2U) ||
              ((t == 1U) && (s.fail != 2U)))
            return false;
          if ((rbs.next() != nullptr) ||
              (rbs.end_reason() != EndReason::Exhausted) ||
              (rbs.statistics().fail != s.fail) ||
              (rbs.statistics().node != s.node))
            return false;
        }
        return true;
      }
    };

    RBSFailedHub rbs_failed_hub;

    /// Space without solutions whose restricted spokes fail at the root
    class FailingSpoke : public NoSolution {
    public:
      FailingSpoke(void) = default;
      FailingSpoke(FailingSpoke& s) : NoSolution(s) {}
      Gecode::Space* copy(void) override {
        return new FailingSpoke(*this);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        if ((sc.engine() == Gecode::MetaEngine::Restart) &&
            (sc.episode() > 0)) {
          fail();
          sc.coverage(Gecode::SpokeCoverage::Partial);
        }
      }
    };

    /// Stop after five failures, or after many checks as a safeguard
    class FiveFailures : public Gecode::Search::Stop {
    public:
      /// Number of checks
      unsigned long int checks = 0;
      bool stop(const Gecode::Search::Statistics& s,
                const Gecode::Search::Options&) override {
        return (s.fail >= 5) || (++checks > 1000);
      }
    };

    /// Spoke searches failing at the root count as failures
    class RBSRootFailure : public Base {
      template<template<class> class Engine>
      static bool test(unsigned int threads) {
        FailingSpoke* m = new FailingSpoke;
        FiveFailures f;
        Gecode::Search::Options o;
        o.threads = threads;
        o.stop = &f;
        o.cutoff = Gecode::Search::Cutoff::constant(1);
        Gecode::RBS<FailingSpoke,Engine> rbs(m,o);
        delete m;
        if (FailingSpoke* s = rbs.next()) {
          delete s;
          return false;
        }
        return (rbs.end_reason() == EndReason::Limit) && (f.checks <= 1000) &&
          (rbs.statistics().fail >= 5);
      }
    public:
      RBSRootFailure(void) : Base("Search::RBS::RootFailure") {}
      bool run(void) override {
        return test<Gecode::DFS>(1U) && test<Gecode::DFS>(2U) &&
          test<Gecode::BAB>(1U) && test<Gecode::BAB>(2U) &&
          test<Gecode::LDS>(1U);
      }
    };

    RBSRootFailure rbs_root_failure;

    /// Space with solutions whose first, incomplete spoke fails at the root
    class FailingFirstSpoke : public RestrictedFirstSpoke {
    public:
      FailingFirstSpoke(void) = default;
      FailingFirstSpoke(FailingFirstSpoke& s) : RestrictedFirstSpoke(s) {}
      Gecode::Space* copy(void) override {
        return new FailingFirstSpoke(*this);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        if ((sc.engine() == Gecode::MetaEngine::Restart) &&
            (sc.event() == Gecode::MetaEvent::Start)) {
          fail();
          sc.coverage(Gecode::SpokeCoverage::Partial);
        }
      }
    };

    /// A failed first spoke search that is incomplete is not a proof
    class RBSFailedFirstSpoke : public Base {
      template<template<class> class Engine>
      static bool test(unsigned int threads) {
        FailingFirstSpoke* m = new FailingFirstSpoke;
        Gecode::Search::Options o;
        o.threads = threads;
        o.cutoff = Gecode::Search::Cutoff::constant(1);
        Gecode::RBS<FailingFirstSpoke,Engine> rbs(m,o);
        delete m;
        FailingFirstSpoke* s = rbs.next();
        if (s == nullptr)
          return false;
        delete s;
        return rbs.statistics().restart >= 1U;
      }
    public:
      RBSFailedFirstSpoke(void) : Base("Search::RBS::FailedFirstSpoke") {}
      bool run(void) override {
        return test<Gecode::DFS>(1U) && test<Gecode::DFS>(2U) &&
          test<Gecode::BAB>(1U) && test<Gecode::BAB>(2U) &&
          test<Gecode::LDS>(1U);
      }
    };

    RBSFailedFirstSpoke rbs_failed_first_spoke;

    /// Stop after a number of failures, or after many checks as a safeguard
    class FailureLimit : public Gecode::Search::Stop {
    public:
      /// Failure limit
      unsigned long long int limit;
      /// Number of checks, parallel engines can check concurrently
      std::atomic<unsigned long int> checks{0};
      FailureLimit(unsigned long long int l) : limit(l) {}
      bool stop(const Gecode::Search::Statistics& s,
                const Gecode::Search::Options&) override {
        return (s.fail > limit) || (++checks > 1000);
      }
    };

    /// A search exhausted when it reaches a failure limit is exhausted
    class ExhaustedAtLimit : public Base {
      template<template<class> class Engine>
      static bool test(void) {
        NoSolution root;
        unsigned long long int f;
        {
          Engine<NoSolution> e(&root);
          if (NoSolution* s = e.next()) {
            delete s;
            return false;
          }
          f = e.statistics().fail;
        }
        if (f < 2)
          return false;
        // The search is exhausted at the failure that exceeds the limit
        {
          FailureLimit l(f-1);
          Gecode::Search::Options o;
          o.stop = &l;
          Engine<NoSolution> e(&root,o);
          if ((e.next() != nullptr) || (e.end_reason() != EndReason::Exhausted))
            return false;
        }
        // Restarts at that limit post no no-goods and would repeat forever
        {
          FailureLimit l(1000);
          Gecode::Search::Options o;
          o.stop = &l;
          o.nogoods_limit = 128U;
          o.cutoff = Gecode::Search::Cutoff::constant(f-1);
          Gecode::RBS<NoSolution,Engine> rbs(&root,o);
          if ((rbs.next() != nullptr) ||
              (rbs.end_reason() != EndReason::Exhausted))
            return false;
        }
        return true;
      }
    public:
      ExhaustedAtLimit(void) : Base("Search::ExhaustedAtLimit") {}
      bool run(void) override {
        return test<Gecode::DFS>() && test<Gecode::BAB>();
      }
    };

    ExhaustedAtLimit exhausted_at_limit;

    /// LDS reports an incomplete search at its discrepancy limit
    class LDSIncomplete : public Base {
      static EndReason end_reason(unsigned int d_l) {
        NoSolution root;
        Gecode::Search::Options o;
        o.d_l = d_l;
        Gecode::LDS<NoSolution> lds(&root,o);
        if (NoSolution* s = lds.next()) {
          delete s;
          return EndReason::NotEnded;
        }
        return lds.end_reason();
      }
    public:
      LDSIncomplete(void) : Base("Search::LDS::Incomplete") {}
      bool run(void) override {
        return (end_reason(0) == EndReason::Incomplete) &&
          (end_reason(1) == EndReason::Incomplete) &&
          (end_reason(100) == EndReason::Exhausted);
      }
    };

    LDSIncomplete lds_incomplete;

    /// Space without solutions recording the default decisions after incomplete episodes
    class IncompleteEpisodes : public NoSolution {
    public:
      /// Coverage that spoke() declares
      Gecode::SpokeCoverage coverage;
      /// Default decisions of hub() after an incomplete episode
      std::vector<Gecode::MetaDecision>* defaults;
      IncompleteEpisodes(Gecode::SpokeCoverage c,
                         std::vector<Gecode::MetaDecision>& d)
        : coverage(c), defaults(&d) {}
      IncompleteEpisodes(IncompleteEpisodes& s)
        : NoSolution(s), coverage(s.coverage), defaults(s.defaults) {}
      Gecode::Space* copy(void) override {
        return new IncompleteEpisodes(*this);
      }
      void hub(Gecode::HubContext& hc) override {
        if (hc.event() == Gecode::MetaEvent::Incomplete) {
          defaults->push_back(hc.decision());
          // Finish after the second incomplete episode at the latest
          if (defaults->size() == 2U)
            hc.decision(Gecode::MetaDecision::Finish);
        }
        NoSolution::hub(hc);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        sc.coverage(coverage);
      }
    };

    /// RBS finishes after an incomplete episode unless its spoke is partial
    class RBSIncompleteDefault : public Base {
      /// Return the default decisions when spokes of coverage \a c end incomplete
      static std::vector<Gecode::MetaDecision>
      defaults(Gecode::SpokeCoverage c) {
        std::vector<Gecode::MetaDecision> d;
        IncompleteEpisodes root(c,d);
        Gecode::Search::Options o;
        o.d_l = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(1000);
        Gecode::RBS<IncompleteEpisodes,Gecode::LDS> rbs(&root,o);
        for (int i=0; i<2; i++)
          if (IncompleteEpisodes* s = rbs.next()) {
            delete s;
            d.clear();
          } else if (rbs.end_reason() != EndReason::Incomplete) {
            d.clear();
          }
        return d;
      }
    public:
      RBSIncompleteDefault(void) : Base("Search::RBS::IncompleteDefault") {}
      bool run(void) override {
        using Gecode::MetaDecision;
        using Gecode::SpokeCoverage;
        using Decisions = std::vector<MetaDecision>;
        return (defaults(SpokeCoverage::Full) ==
                Decisions{MetaDecision::Finish}) &&
          (defaults(SpokeCoverage::Reduced) ==
           Decisions{MetaDecision::Finish}) &&
          (defaults(SpokeCoverage::Partial) ==
           Decisions{MetaDecision::Restart,MetaDecision::Restart});
      }
    };

    RBSIncompleteDefault rbs_incomplete_default;

    /// Space for relaxing
    class RelaxSpace : public Gecode::Space {
    public:
      RelaxSpace(void) = default;
      RelaxSpace(RelaxSpace& s) : Gecode::Space(s) {}
      Gecode::Space* copy(void) override {
        return new RelaxSpace(*this);
      }
    };

    /// Relaxing fails the space when a value cannot be kept
    class RelaxFailed : public Base {
    public:
      RelaxFailed(void) : Base("Search::Relax::Failed") {}
      bool run(void) override {
        RelaxSpace s;
        // The kept value lies in a hole of the domain
        Gecode::IntVarArgs x(s,2,Gecode::IntSet({0,2}));
        Gecode::IntVarArgs sx(s,2,1,1);
        // Probability zero keeps one of the two values
        Gecode::relax(s,x,sx,Gecode::Rnd(1U),0.0);
        return s.failed();
      }
    };

    RelaxFailed relax_failed;

    /// %Test for portfolio-based search
    template<class Model, template<class> class Engine>
    class PBS : public Test {
    private:
      /// Whether best solution search is used
      bool best;
      /// Number of assets
      unsigned int a;
      /// Number of threads
      unsigned int t;
    public:
      /// Initialize test
      PBS(const std::string& e, bool b, unsigned int a0, unsigned int t0)
        : Test("PBS::"+e+"::"+Model::name()+"::"+str(a0)+"::"+str(t0),
               HTB_BINARY,HTB_BINARY,HTB_BINARY), best(b), a(a0), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.assets = a;
        o.threads = t;
        o.d_l = 100;
        o.stop = &f;
        Gecode::PBS<Model,Engine> pbs(m,o);
        if (best) {
          delete m;
          Model* b = nullptr;
          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              delete b; b=s;
            }
            if ((s == nullptr) && (pbs.end_reason() != EndReason::Limit))
              break;
            f.limit(f.limit()+2);
          }
          bool ok = (b == nullptr) || b->best();
          delete b;
          return ok;
        } else {
          int n = a * m->solutions();
          delete m;
          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              n--; delete s;
            }
            if ((s == nullptr) && (pbs.end_reason() != EndReason::Limit))
              break;
            f.limit(f.limit()+2);
          }
          return n >= 0;
        }
      }
    };

    /// %Test for portfolio-based search using SEBs
    template<class Model>
    class SEBPBS : public Test {
    private:
      /// Whether best solution search is used
      bool best;
      /// Number of portfolio threads
      unsigned int portfolio_threads;
      /// Number of threads of each asset
      unsigned int asset_threads;
    public:
      /// Initialize test
      SEBPBS(const std::string& e, bool b, unsigned int portfolio_threads0, unsigned int asset_threads0)
        : Test("PBS::SEB::"+e+"::"+Model::name()+"::"+str(portfolio_threads0)+"::"+str(asset_threads0),
               HTB_BINARY,HTB_BINARY,HTB_BINARY), best(b),
          portfolio_threads(portfolio_threads0), asset_threads(asset_threads0) {}
      /// Run test
      virtual bool run(void) {
        using namespace Gecode;
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);

        Gecode::Search::Options portfolio_options;
        portfolio_options.threads = portfolio_threads;
        portfolio_options.d_l = 100;
        portfolio_options.stop = &f;

        Gecode::Search::Options asset_options;
        asset_options.threads = asset_threads;
        asset_options.d_l = 100;
        asset_options.cutoff = Gecode::Search::Cutoff::constant(1000000);
        if (best) {
          SEBs sebs(3);
          sebs[0] = bab<Model>(asset_options);
          sebs[1] = bab<Model>(asset_options);
          sebs[2] = rbs<Model,Gecode::BAB>(asset_options);
          Gecode::PBS<Model,Gecode::BAB> pbs(m, sebs, portfolio_options);
          delete m;

          Model* b = nullptr;
          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              delete b; b=s;
            }
            if ((s == nullptr) && (pbs.end_reason() != EndReason::Limit))
              break;
            f.limit(f.limit()+2);
          }
          bool ok = (b == nullptr) || b->best();
          delete b;
          return ok;
        } else {
          SEBs sebs(3);
          sebs[0] = dfs<Model>(asset_options);
          sebs[1] = lds<Model>(asset_options);
          sebs[2] = rbs<Model,Gecode::DFS>(asset_options);
          Gecode::PBS<Model,Gecode::DFS> pbs(m, sebs, portfolio_options);

          int n = 3 * m->solutions();
          delete m;

          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              n--; delete s;
            }
            if ((s == nullptr) && (pbs.end_reason() != EndReason::Limit))
              break;
            f.limit(f.limit()+2);
          }
          return n >= 0;
        }
      }
    };

    /// The stop object of a portfolio built from builders stops the builders
    class PBSBuilderStop : public Base {
      /// Function returning an engine builder
      typedef Gecode::SEB (*Builder)(const Gecode::Search::Options&);
      template<template<class> class Engine>
      static bool test(unsigned int threads, Builder builder) {
        NoSolution root;
        FailureLimit l(1);
        Gecode::Search::Options o;
        o.threads = threads;
        o.stop = &l;
        Gecode::SEBs sebs(2);
        sebs[0] = builder(Gecode::Search::Options::def);
        sebs[1] = builder(Gecode::Search::Options::def);
        Gecode::PBS<NoSolution,Engine> pbs(&root,sebs,o);
        return (pbs.next() == nullptr) &&
          (pbs.end_reason() == EndReason::Limit) && (l.checks <= 1000);
      }
    public:
      PBSBuilderStop(void) : Base("Search::PBS::SEB::Stop") {}
      bool run(void) override {
        for (unsigned int t : {1U,2U})
          if (!test<Gecode::DFS>(t,&Gecode::dfs<NoSolution>) ||
              !test<Gecode::BAB>(t,&Gecode::bab<NoSolution>))
            return false;
        return true;
      }
    };

    PBSBuilderStop pbs_builder_stop;

    /// An exhausted asset ends a portfolio whose other asset never ends
    class PBSExhaustedAsset : public Base {
      /// Stop after many checks, in case the portfolio does not end
      class ManyChecks : public Gecode::Search::Stop {
      public:
        /// Number of checks
        std::atomic<unsigned long int> checks{0UL};
        bool stop(const Gecode::Search::Statistics&,
                  const Gecode::Search::Options&) override {
          return ++checks > 10000000UL;
        }
      };
      static bool test(unsigned int threads) {
        FailingSpoke root;
        ManyChecks l;
        Gecode::Search::Options a;
        a.cutoff = Gecode::Search::Cutoff::constant(1);
        Gecode::SEBs sebs(2);
        sebs[0] = Gecode::rbs<FailingSpoke,Gecode::DFS>(a);
        sebs[1] = Gecode::dfs<FailingSpoke>();
        Gecode::Search::Options o;
        o.threads = threads;
        o.stop = &l;
        Gecode::PBS<FailingSpoke,Gecode::DFS> pbs(&root,sebs,o);
        return (pbs.next() == nullptr) &&
          (pbs.end_reason() == EndReason::Exhausted) &&
          (pbs.next() == nullptr) &&
          (pbs.end_reason() == EndReason::Exhausted);
      }
    public:
      PBSExhaustedAsset(void) : Base("Search::PBS::ExhaustedAsset") {}
      bool run(void) override {
        return test(1U) && test(2U);
      }
    };

    PBSExhaustedAsset pbs_exhausted_asset;

    /// The stop object of a portfolio sees the failures of all assets
    class PBSPortfolioStop : public Base {
      /// Whether restart-based assets stop at a portfolio-wide failure limit
      static bool test(unsigned int threads, bool builders) {
        FailingSpoke root;
        const unsigned long long int limit = 40U;
        Gecode::Search::FailStop f(limit);
        Gecode::Search::Options o;
        o.threads = threads;
        o.stop = &f;
        o.slice = 2;
        Gecode::Search::Statistics s;
        if (builders) {
          // Each restart-based engine takes ownership of its cutoff
          Gecode::Search::Options a, b;
          a.cutoff = Gecode::Search::Cutoff::constant(1);
          b.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::SEBs sebs(2);
          sebs[0] = Gecode::rbs<FailingSpoke,Gecode::DFS>(a);
          sebs[1] = Gecode::rbs<FailingSpoke,Gecode::DFS>(b);
          Gecode::PBS<FailingSpoke,Gecode::DFS> pbs(&root,sebs,o);
          if ((pbs.next() != nullptr) ||
              (pbs.end_reason() != EndReason::Limit))
            return false;
          s = pbs.statistics();
        } else {
          o.assets = 2;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::PBS<FailingSpoke,Gecode::RBS> pbs(&root,o);
          if ((pbs.next() != nullptr) ||
              (pbs.end_reason() != EndReason::Limit))
            return false;
          s = pbs.statistics();
        }
        // Checked for each asset, the limit would allow twice the failures
        return (s.fail > limit) && (s.fail < limit + limit / 2U);
      }
    public:
      PBSPortfolioStop(void) : Base("Search::PBS::PortfolioStop") {}
      bool run(void) override {
        for (unsigned int t : {1U,2U})
          for (bool builders : {false,true})
            if (!test(t,builders))
              return false;
        return true;
      }
    };

    PBSPortfolioStop pbs_portfolio_stop;

    /// An asset stopped by its own stop object is parked while the others continue
    class PBSParkedAsset : public Base {
      /// Stop object that always stops
      class AlwaysStop : public Gecode::Search::Stop {
      public:
        bool stop(const Gecode::Search::Statistics&,
                  const Gecode::Search::Options&) override {
          return true;
        }
      };
      /// Whether the other asset continues and exhausts the search space
      static bool others_continue(unsigned int threads) {
        NoSolution root;
        AlwaysStop always;
        Gecode::Search::Options a;
        a.stop = &always;
        Gecode::SEBs sebs(2);
        sebs[0] = Gecode::dfs<NoSolution>(a);
        sebs[1] = Gecode::dfs<NoSolution>();
        Gecode::Search::Options o;
        o.threads = threads;
        Gecode::PBS<NoSolution,Gecode::DFS> pbs(&root,sebs,o);
        return (pbs.next() == nullptr) &&
          (pbs.end_reason() == EndReason::Exhausted);
      }
      /// Whether parked assets resume with the next call
      static bool resume(unsigned int threads) {
        NoSolution root;
        Gecode::Search::FailStop f0(0), f1(0);
        Gecode::Search::Options a0, a1;
        a0.stop = &f0;
        a1.stop = &f1;
        Gecode::SEBs sebs(2);
        sebs[0] = Gecode::dfs<NoSolution>(a0);
        sebs[1] = Gecode::lds<NoSolution>(a1);
        Gecode::Search::Options o;
        o.threads = threads;
        Gecode::PBS<NoSolution,Gecode::DFS> pbs(&root,sebs,o);
        unsigned int limits = 0;
        while ((pbs.next() == nullptr) &&
               (pbs.end_reason() == EndReason::Limit)) {
          if (++limits > 1000)
            return false;
          f0.limit(f0.limit() + 1);
          f1.limit(f1.limit() + 1);
        }
        return (limits > 0) && (pbs.end_reason() == EndReason::Exhausted);
      }
    public:
      PBSParkedAsset(void) : Base("Search::PBS::ParkedAsset") {}
      bool run(void) override {
        for (unsigned int t : {1U,2U})
          if (!others_continue(t) || !resume(t))
            return false;
        return true;
      }
    };

    PBSParkedAsset pbs_parked_asset;

    /// A copy of a cutoff generator continues its sequence independently
    class CutoffCopy : public Base {
      /// Check that a copy of \a c continues as \a c, then delete \a c
      static bool same(Gecode::Search::Cutoff* c) {
        std::unique_ptr<Gecode::Search::Cutoff> o(c);
        for (int i=0; i<5; i++)
          (void) ++(*o);
        std::unique_ptr<Gecode::Search::Cutoff> d(o->copy());
        if ((*d)() != (*o)())
          return false;
        for (int i=0; i<20; i++)
          if (++(*d) != ++(*o))
            return false;
        return true;
      }
    public:
      CutoffCopy(void) : Base("Search::Cutoff::Copy") {}
      bool run(void) override {
        using Gecode::Search::Cutoff;
        return same(Cutoff::constant(3)) && same(Cutoff::linear(2)) &&
          same(Cutoff::geometric(2,1.5)) && same(Cutoff::luby(2)) &&
          same(Cutoff::append(Cutoff::linear(1),8,Cutoff::luby(3))) &&
          same(Cutoff::merge(Cutoff::linear(1),Cutoff::geometric(1,2.0))) &&
          same(Cutoff::repeat(Cutoff::linear(4),3));
      }
    };

    CutoffCopy cutoff_copy;

    /// Copies of a random cutoff generator draw distinct, reproducible sequences
    class CutoffRandomCopy : public Base {
      /// Return the first values of \a c, which is deleted
      static std::vector<unsigned long long int>
      values(Gecode::Search::Cutoff* c) {
        std::unique_ptr<Gecode::Search::Cutoff> d(c);
        std::vector<unsigned long long int> v{(*d)()};
        for (int i=0; i<20; i++) {
          v.push_back(++(*d));
          if ((v.back() < 1U) || (v.back() > 1000U))
            return {};
        }
        return v;
      }
      /// Return the values of two copies of a generator with seed \a seed
      static std::vector<std::vector<unsigned long long int> >
      copies(unsigned int seed) {
        std::unique_ptr<Gecode::Search::Cutoff>
          o(Gecode::Search::Cutoff::rnd(seed,1,1000,1000));
        (void) ++(*o);
        std::vector<std::vector<unsigned long long int> > v;
        v.push_back(values(o->copy()));
        v.push_back(values(o->copy()));
        v.push_back(values(o.release()));
        return v;
      }
    public:
      CutoffRandomCopy(void) : Base("Search::Cutoff::RandomCopy") {}
      bool run(void) override {
        std::vector<std::vector<unsigned long long int> > v = copies(7);
        return !v[0].empty() && !v[1].empty() && !v[2].empty() &&
          (v[0] != v[1]) && (v[0] != v[2]) && (v[1] != v[2]) &&
          (copies(7) == v);
      }
    };

    CutoffRandomCopy cutoff_random_copy;

    /// Each restart-based asset of a portfolio owns a copy of the cutoff
    class PBSRestartCutoff : public Base {
      template<template<class> class Engine>
      static bool test(unsigned int threads) {
        NoSolution root;
        Gecode::Search::Options o;
        o.assets = 2;
        o.threads = threads;
        o.cutoff = Gecode::Search::Cutoff::geometric(1,2);
        Gecode::PBS<NoSolution,Engine> pbs(&root,o);
        return (pbs.next() == nullptr) &&
          (pbs.end_reason() == EndReason::Exhausted);
      }
    public:
      PBSRestartCutoff(void) : Base("Search::PBS::RBS::Cutoff") {}
      bool run(void) override {
        for (unsigned int t : {1U,2U,4U})
          if (!test<Gecode::RBS>(t) || !test<RestartPortfolio>(t))
            return false;
        return true;
      }
    };

    PBSRestartCutoff pbs_restart_cutoff;

    /// Hook behaviour of ProtocolSpace
    enum class Protocol {
      Default,         ///< Default hooks
      ContinueAtStart, ///< hub() continues at Start, which is illegal
      FinishAtStart,   ///< hub() finishes at Start
      FinishAtLimit,   ///< hub() finishes at Limit
      FailAtStart,     ///< hub() fails at Start
      FailAtSolution,  ///< hub() fails at Solution
      ContinueAtLimit, ///< hub() continues at Limit
      EmptyHub,        ///< hub() does nothing
      BoundInHub,      ///< hub() posts c <= 1 at Start and keeps the branchers
      Alternate,       ///< Episodes alternate between a solution and the cutoff
    };

    /// Calls of the hooks of ProtocolSpace
    struct ProtocolLog {
      /// Event and episode of each hub() call
      std::vector<std::pair<MetaEvent,unsigned long int> > hub;
      /// Event and episode of each spoke() call
      std::vector<std::pair<MetaEvent,unsigned long int> > spoke;
      /// Number of spaces alive
      std::atomic<int> live{0};
    };

    /// Space whose hooks follow a Protocol
    class ProtocolSpace : public IntMinimizeSpace {
    public:
      /// Hook behaviour
      Protocol protocol;
      /// Log of hook calls
      ProtocolLog* log;
      /// Cost
      IntVar c;
      /// Whether the y must be distinct, which is impossible
      BoolVar p;
      /// Four variables with three values
      IntVarArray y;
      /// Initialize with hook behaviour \a pr and log \a l
      ProtocolSpace(Protocol pr, ProtocolLog& l)
        : protocol(pr), log(&l), c(*this,0,3), p(*this,0,1), y(*this,4,0,2) {
        log->live++;
        for (int i=0; i<y.size(); i++)
          for (int j=i+1; j<y.size(); j++)
            rel(*this,y[i],IRT_NQ,y[j],Reify(p,RM_IMP));
        // Search fails six times for p = 1, then finds c = 3, 2, 1, 0
        Gecode::branch(*this,p,BOOL_VAL_MAX());
        Gecode::branch(*this,y,INT_VAR_NONE(),INT_VAL_MIN());
        Gecode::branch(*this,c,INT_VAL_MAX());
      }
      /// Clone \a s
      ProtocolSpace(ProtocolSpace& s)
        : IntMinimizeSpace(s), protocol(s.protocol), log(s.log) {
        log->live++;
        c.update(*this,s.c);
        p.update(*this,s.p);
        y.update(*this,s.y);
      }
      /// Record destruction
      ~ProtocolSpace(void) override {
        log->live--;
      }
      Space* copy(void) override {
        return new ProtocolSpace(*this);
      }
      IntVar cost(void) const override {
        return c;
      }
      void hub(HubContext& hc) override {
        log->hub.emplace_back(hc.event(),hc.episode());
        switch (protocol) {
        case Protocol::ContinueAtStart:
          if (hc.event() == MetaEvent::Start)
            hc.decision(MetaDecision::Continue);
          break;
        case Protocol::FinishAtStart:
          if (hc.event() == MetaEvent::Start) {
            hc.decision(MetaDecision::Finish);
            return;
          }
          break;
        case Protocol::FinishAtLimit:
          if (hc.event() == MetaEvent::Limit)
            hc.decision(MetaDecision::Finish);
          break;
        case Protocol::FailAtStart:
          if (hc.event() == MetaEvent::Start) {
            fail();
            return;
          }
          break;
        case Protocol::FailAtSolution:
          if (hc.event() == MetaEvent::Solution) {
            fail();
            return;
          }
          break;
        case Protocol::ContinueAtLimit:
          if (hc.event() == MetaEvent::Limit)
            hc.decision(MetaDecision::Continue);
          break;
        case Protocol::EmptyHub:
          return;
        case Protocol::BoundInHub:
          if (hc.event() == MetaEvent::Start) {
            rel(*this,c,IRT_LQ,1);
            return;
          }
          break;
        case Protocol::Default:
        case Protocol::Alternate:
          break;
        }
        IntMinimizeSpace::hub(hc);
      }
      void spoke(SpokeContext& sc) override {
        log->spoke.emplace_back(sc.event(),sc.episode());
        if ((protocol == Protocol::Alternate) &&
            (sc.engine() == MetaEngine::Restart)) {
          // Odd episodes search only the part without solutions
          rel(*this,p,IRT_EQ,((sc.episode() % 2U) == 1U) ? 1 : 0);
          sc.coverage(SpokeCoverage::Partial);
        }
      }
    };

    /// Decisions, events, and ends of the meta search protocol
    class MetaProtocol : public Base {
    public:
      /// Scenario
      enum class Scenario {
        DecisionLegality,
        IllegalDecisionInEngine,
        FinishAtStart,
        FinishAtLimit,
        FailAtStart,
        FailAtSolution,
        ContinueAtLimit,
        EngineConstrainsHub,
        PortfolioHubConstraint,
        Episodes,
      };
    protected:
      /// Scenario under test
      Scenario scenario;
      /// Return options for RBS with constant cutoff \a c
      static Gecode::Search::Options rbs_options(unsigned long int c) {
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(c);
        return o;
      }
      /// Return whether engine \a e has no further solution and ends with \a r
      template<class Engine>
      static bool ends(Engine& e, EndReason r) {
        for (int i=0; i<2; i++) {
          if (ProtocolSpace* s = e.next()) {
            delete s;
            return false;
          }
          if (e.end_reason() != r)
            return false;
        }
        return true;
      }
      /// Return whether \a d is legal after event \a e
      static bool legal(MetaEvent e, MetaDecision d) {
        return (d != MetaDecision::Continue) ||
          (e == MetaEvent::Solution) || (e == MetaEvent::Limit);
      }
      /// Check the legality of each decision for each event
      static bool decision_legality(void) {
        const Gecode::Search::Statistics stat;
        for (MetaEvent e : {MetaEvent::Start,MetaEvent::Solution,
                            MetaEvent::Exhausted,MetaEvent::Incomplete,
                            MetaEvent::Limit})
          for (MetaDecision d : {MetaDecision::Restart,MetaDecision::Continue,
                                 MetaDecision::Finish}) {
            HubContext hc = Gecode::Search::MetaContextAccess::hub
              (MetaEngine::Restart,e,MetaDecision::Restart,0,0,1,nullptr,
               stat,NoGoods::eng);
            if (hc.decision() != MetaDecision::Restart)
              return false;
            bool thrown = false;
            try {
              hc.decision(d);
            } catch (const IllegalMetaDecision&) {
              thrown = true;
            }
            if ((thrown == legal(e,d)) ||
                (hc.decision() != (thrown ? MetaDecision::Restart : d)))
              return false;
          }
        // The engine sets the default decision
        HubContext fc = Gecode::Search::MetaContextAccess::hub
          (MetaEngine::Restart,MetaEvent::Incomplete,MetaDecision::Finish,
           0,0,1,nullptr,stat,NoGoods::eng);
        if (fc.decision() != MetaDecision::Finish)
          return false;
        SpokeContext sc = Gecode::Search::MetaContextAccess::spoke
          (MetaEngine::Restart,MetaEvent::Start,0,0,1,nullptr,stat,
           NoGoods::eng);
        if (sc.coverage() != SpokeCoverage::Full)
          return false;
        sc.coverage(SpokeCoverage::Partial);
        sc.coverage(SpokeCoverage::Reduced);
        return sc.coverage() == SpokeCoverage::Reduced;
      }
      /// Check that an illegal decision throws from the engine
      static bool illegal_decision_in_engine(void) {
        ProtocolLog log;
        ProtocolSpace* m = new ProtocolSpace(Protocol::ContinueAtStart,log);
        Gecode::Search::Options o = rbs_options(10);
        bool thrown = false;
        try {
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,o);
        } catch (const IllegalMetaDecision&) {
          // The engine owns the cutoff only once it is constructed
          delete o.cutoff;
          thrown = true;
        }
        delete m;
        return thrown && log.spoke.empty() && (log.live == 0);
      }
      /// Check that RBS and PBS end with \a r when hub() at Start follows \a pr
      static bool ends_at_start(Protocol pr, EndReason r) {
        ProtocolLog log;
        {
          ProtocolSpace* m = new ProtocolSpace(pr,log);
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,rbs_options(10));
          delete m;
          if (!ends(rbs,r))
            return false;
        }
        for (unsigned int t : {1U,2U}) {
          ProtocolSpace* m = new ProtocolSpace(pr,log);
          Gecode::Search::Options o;
          o.assets = 2;
          o.threads = t;
          Gecode::PBS<ProtocolSpace,Gecode::DFS> pbs(m,o);
          delete m;
          if (!ends(pbs,r))
            return false;
        }
        {
          ProtocolSpace* m = new ProtocolSpace(pr,log);
          Gecode::Search::Options o;
          o.threads = 1;
          SEBs assets(2);
          for (int i=0; i<2; i++)
            assets[i] = Gecode::dfs<ProtocolSpace>(o);
          Gecode::PBS<ProtocolSpace,Gecode::DFS> pbs(m,assets,o);
          delete m;
          if (!ends(pbs,r))
            return false;
        }
        return log.spoke.empty() && (log.hub.size() == 4U) && (log.live == 0);
      }
      /// Check that Finish at Limit ends RBS without a proof
      static bool finish_at_limit(void) {
        ProtocolLog log;
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::FinishAtLimit,log);
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,rbs_options(1));
          delete m;
          if (!ends(rbs,EndReason::Incomplete) ||
              (rbs.statistics().restart != 0U))
            return false;
        }
        using Call = std::pair<MetaEvent,unsigned long int>;
        return (log.hub == std::vector<Call>{{MetaEvent::Start,0},
                                             {MetaEvent::Limit,0}}) &&
          (log.spoke == std::vector<Call>{{MetaEvent::Start,0}}) &&
          (log.live == 0);
      }
      /// Check that a hub failing after a solution proves exhaustion
      static bool fail_at_solution(void) {
        ProtocolLog log;
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::FailAtSolution,log);
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,rbs_options(100));
          delete m;
          ProtocolSpace* s = rbs.next();
          if (s == nullptr)
            return false;
          delete s;
          if (!ends(rbs,EndReason::Exhausted))
            return false;
        }
        return (log.spoke.size() == 1U) && (log.live == 0);
      }
      /// Check that Continue at Limit resumes the same spoke
      static bool continue_at_limit(void) {
        ProtocolLog log;
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::ContinueAtLimit,log);
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,rbs_options(1));
          delete m;
          ProtocolSpace* s = rbs.next();
          if (s == nullptr)
            return false;
          const bool assigned = s->p.assigned() && (s->p.val() == 0);
          delete s;
          if (!assigned || (rbs.statistics().restart != 0U))
            return false;
        }
        if ((log.spoke.size() != 1U) || (log.hub.size() < 3U))
          return false;
        for (std::size_t i=1; i<log.hub.size(); i++)
          if (log.hub[i] != std::make_pair(MetaEvent::Limit,0UL))
            return false;
        return log.live == 0;
      }
      /// Check that RBS<BAB> constrains the hub when hub() does nothing
      static bool engine_constrains_hub(void) {
        ProtocolLog log;
        std::vector<int> costs;
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::EmptyHub,log);
          Gecode::RBS<ProtocolSpace,Gecode::BAB> rbs(m,rbs_options(100));
          delete m;
          // Without the bound, every spoke finds c = 3 again
          while (ProtocolSpace* s = rbs.next()) {
            costs.push_back(s->c.val());
            delete s;
            if (costs.size() > 10U)
              return false;
          }
          if (rbs.end_reason() != EndReason::Exhausted)
            return false;
        }
        return (costs == std::vector<int>{3,2,1,0}) && (log.live == 0);
      }
      /// Check that a constraint posted by hub() under PBS takes effect
      static bool portfolio_hub_constraint(void) {
        ProtocolLog log;
        for (unsigned int t : {1U,2U}) {
          ProtocolSpace* m = new ProtocolSpace(Protocol::BoundInHub,log);
          Gecode::Search::Options o;
          o.assets = 2;
          o.threads = t;
          Gecode::PBS<ProtocolSpace,Gecode::DFS> pbs(m,o);
          delete m;
          ProtocolSpace* s = pbs.next();
          const bool ok = (s != nullptr) && (s->c.val() == 1);
          delete s;
          if (!ok)
            return false;
        }
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::BoundInHub,log);
          Gecode::Search::Options o;
          o.threads = 1;
          SEBs assets(2);
          for (int i=0; i<2; i++)
            assets[i] = Gecode::dfs<ProtocolSpace>(o);
          Gecode::PBS<ProtocolSpace,Gecode::DFS> pbs(m,assets,o);
          delete m;
          ProtocolSpace* s = pbs.next();
          const bool ok = (s != nullptr) && (s->c.val() == 1);
          delete s;
          if (!ok)
            return false;
        }
        return log.live == 0;
      }
      /// Check episode numbering across solution and limit restarts
      static bool episodes(void) {
        ProtocolLog log;
        {
          ProtocolSpace* m = new ProtocolSpace(Protocol::Alternate,log);
          Gecode::RBS<ProtocolSpace,Gecode::DFS> rbs(m,rbs_options(2));
          delete m;
          for (int i=0; i<3; i++) {
            ProtocolSpace* s = rbs.next();
            if (s == nullptr)
              return false;
            delete s;
          }
          if (rbs.statistics().restart != 4U)
            return false;
        }
        using Call = std::pair<MetaEvent,unsigned long int>;
        const std::vector<Call> hub{{MetaEvent::Start,0},
                                    {MetaEvent::Solution,0},
                                    {MetaEvent::Limit,1},
                                    {MetaEvent::Solution,2},
                                    {MetaEvent::Limit,3}};
        const std::vector<Call> spoke{{MetaEvent::Start,0},
                                      {MetaEvent::Solution,1},
                                      {MetaEvent::Limit,2},
                                      {MetaEvent::Solution,3},
                                      {MetaEvent::Limit,4}};
        return (log.hub == hub) && (log.spoke == spoke) && (log.live == 0);
      }
    public:
      /// Initialize scenario \a s with name \a n
      MetaProtocol(const std::string& n, Scenario s)
        : Base("Search::Meta::"+n), scenario(s) {}
      bool run(void) override {
        switch (scenario) {
        case Scenario::DecisionLegality:
          return decision_legality();
        case Scenario::IllegalDecisionInEngine:
          return illegal_decision_in_engine();
        case Scenario::FinishAtStart:
          return ends_at_start(Protocol::FinishAtStart,EndReason::Incomplete);
        case Scenario::FinishAtLimit:
          return finish_at_limit();
        case Scenario::FailAtStart:
          return ends_at_start(Protocol::FailAtStart,EndReason::Exhausted);
        case Scenario::FailAtSolution:
          return fail_at_solution();
        case Scenario::ContinueAtLimit:
          return continue_at_limit();
        case Scenario::EngineConstrainsHub:
          return engine_constrains_hub();
        case Scenario::PortfolioHubConstraint:
          return portfolio_hub_constraint();
        case Scenario::Episodes:
          return episodes();
        default:
          GECODE_NEVER;
        }
        return false;
      }
    };

    MetaProtocol meta_decision_legality
    ("DecisionLegality",MetaProtocol::Scenario::DecisionLegality);
    MetaProtocol meta_illegal_decision_in_engine
    ("IllegalDecisionInEngine",
     MetaProtocol::Scenario::IllegalDecisionInEngine);
    MetaProtocol meta_finish_at_start
    ("FinishAtStart",MetaProtocol::Scenario::FinishAtStart);
    MetaProtocol meta_finish_at_limit
    ("FinishAtLimit",MetaProtocol::Scenario::FinishAtLimit);
    MetaProtocol meta_fail_at_start
    ("FailAtStart",MetaProtocol::Scenario::FailAtStart);
    MetaProtocol meta_fail_at_solution
    ("FailAtSolution",MetaProtocol::Scenario::FailAtSolution);
    MetaProtocol meta_continue_at_limit
    ("ContinueAtLimit",MetaProtocol::Scenario::ContinueAtLimit);
    MetaProtocol meta_engine_constrains_hub
    ("EngineConstrainsHub",MetaProtocol::Scenario::EngineConstrainsHub);
    MetaProtocol meta_portfolio_hub_constraint
    ("PortfolioHubConstraint",MetaProtocol::Scenario::PortfolioHubConstraint);
    MetaProtocol meta_episodes("Episodes",MetaProtocol::Scenario::Episodes);

    /**
     * \brief Constant cutoff counting its deletions
     *
     * The cutoff exceeds the failures of NoSolution, so a restart-based
     * asset finishes without restarting. Without no-goods, restarts with a
     * smaller constant cutoff would repeat the same search forever.
     */
    class CountingCutoff : public Gecode::Search::CutoffConstant {
    public:
      /// Number of deletions, owned by the test
      std::atomic<unsigned int>& deleted;
      CountingCutoff(std::atomic<unsigned int>& d)
        : Gecode::Search::CutoffConstant(1000), deleted(d) {}
      ~CountingCutoff(void) override {
        deleted++;
      }
    };

    /// Space without solutions whose first portfolio asset fails
    class FailingAsset : public NoSolution {
    public:
      FailingAsset(void) = default;
      FailingAsset(FailingAsset& s) : NoSolution(s) {}
      Gecode::Space* copy(void) override {
        return new FailingAsset(*this);
      }
      void spoke(Gecode::SpokeContext& sc) override {
        if ((sc.engine() == Gecode::MetaEngine::Portfolio) &&
            (sc.asset() == 0U))
          fail();
      }
    };

    /// A portfolio of restart-based assets deletes the cutoff exactly once
    class PBSRestartCutoffDeleted : public Base {
      template<class Root, template<class> class Engine>
      static bool test(Root& root, unsigned int threads) {
        std::atomic<unsigned int> deleted{0U};
        {
          Gecode::Search::Options o;
          o.assets = 2;
          o.threads = threads;
          o.cutoff = new CountingCutoff(deleted);
          Gecode::PBS<Root,Engine> pbs(&root,o);
          if (Root* s = pbs.next()) {
            delete s;
            return false;
          }
          if (pbs.end_reason() != EndReason::Exhausted)
            return false;
        }
        return deleted == 1U;
      }
    public:
      PBSRestartCutoffDeleted(void)
        : Base("Search::PBS::RBS::CutoffDeleted") {}
      bool run(void) override {
        // A failed root, also for a portfolio of portfolios
        NoSolution failed;
        Gecode::rel(failed,failed.x[0],Gecode::IRT_LE,0);
        // An asset of a portfolio of portfolios fails at its root
        FailingAsset failing;
        for (unsigned int t : {1U,2U,4U})
          if (!test<NoSolution,Gecode::RBS>(failed,t) ||
              !test<NoSolution,RestartPortfolio>(failed,t) ||
              !test<FailingAsset,Gecode::RBS>(failing,t) ||
              !test<FailingAsset,RestartPortfolio>(failing,t))
            return false;
        return true;
      }
    };

    PBSRestartCutoffDeleted pbs_restart_cutoff_deleted;

    /// Iterator for branching types
    class BranchTypes {
    private:
      /// Array of branching types
      static const HowToBranch htbs[3];
      /// Current position in branching type array
      int i;
    public:
      /// Initialize iterator
      BranchTypes(void) : i(0) {}
      /// Test whether iterator is done
      bool operator()(void) const {
        return i<3;
      }
      /// Increment to next branching type
      void operator++(void) {
        i++;
      }
      /// Return current branching type
      HowToBranch htb(void) const {
        return htbs[i];
      }
    };

    const HowToBranch BranchTypes::htbs[3] = {HTB_UNARY, HTB_BINARY, HTB_NARY};

    /// Iterator for constrain types
    class ConstrainTypes {
    private:
      /// Array of constrain types
      static const HowToConstrain htcs[4];
      /// Current position in constrain type array
      int i;
    public:
      /// Initialize iterator
      ConstrainTypes(void) : i(0) {}
      /// Test whether iterator is done
      bool operator()(void) const {
        return i<4;
      }
      /// Increment to next constrain type
      void operator++(void) {
        i++;
      }
      /// Return current constrain type
      HowToConstrain htc(void) const {
        return htcs[i];
      }
    };

    const HowToConstrain ConstrainTypes::htcs[4] =
      {HTC_LEX_LE, HTC_LEX_GR, HTC_BAL_LE, HTC_BAL_GR};


    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        (void) new Comparison;
        // Depth-first search
        for (unsigned int t = 1; t<=4; t++)
          for (unsigned int c_d = 1; c_d<10; c_d++)
            for (unsigned int a_d = 1; a_d<=c_d; a_d++) {
              for (BranchTypes htb1; htb1(); ++htb1)
                for (BranchTypes htb2; htb2(); ++htb2)
                  for (BranchTypes htb3; htb3(); ++htb3)
                    (void) new DFS<HasSolutions>
                      ((htb1.htb() == HTB_BINARY) &&
                       (htb2.htb() == HTB_NARY) &&
                       (htb3.htb() == HTB_BINARY) &&
                       (c_d == 1) && (a_d == 1) && (t == 1)
                       ? TestTags::check()
                       : TestTags(TestTags::sweep()),
                       htb1.htb(),htb2.htb(),htb3.htb(),c_d,a_d,t);
              new DFS<FailImmediate>(TestTags::standard(),
                                     HTB_NONE, HTB_NONE, HTB_NONE,
                                     c_d, a_d, t);
              new DFS<SolveImmediate>(TestTags::sweep(),
                                      HTB_NONE, HTB_NONE, HTB_NONE,
                                      c_d, a_d, t);
              new DFS<HasSolutions>(TestTags::sweep(),
                                    HTB_NONE, HTB_NONE, HTB_NONE,
                                    c_d, a_d, t);
            }

        // Limited discrepancy search
        for (unsigned int t = 1; t<=4; t++) {
          for (BranchTypes htb1; htb1(); ++htb1)
            for (BranchTypes htb2; htb2(); ++htb2)
              for (BranchTypes htb3; htb3(); ++htb3)
                (void) new LDS<HasSolutions>(htb1.htb(),htb2.htb(),htb3.htb()
                                             ,t);
          new LDS<FailImmediate>(HTB_NONE, HTB_NONE, HTB_NONE, t);
          new LDS<HasSolutions>(HTB_NONE, HTB_NONE, HTB_NONE, t);
        }

        // Best solution search
        for (unsigned int t = 1; t<=4; t++)
          for (unsigned int c_d = 1; c_d<10; c_d++)
            for (unsigned int a_d = 1; a_d<=c_d; a_d++) {
              for (ConstrainTypes htc; htc(); ++htc)
                for (BranchTypes htb1; htb1(); ++htb1)
                  for (BranchTypes htb2; htb2(); ++htb2)
                    for (BranchTypes htb3; htb3(); ++htb3) {
                      (void) new BAB<HasSolutions>
                        ((htc.htc() == HTC_BAL_GR) &&
                         (htb1.htb() == HTB_BINARY) &&
                         (htb2.htb() == HTB_BINARY) &&
                         (htb3.htb() == HTB_BINARY) &&
                         (c_d == 1) && (a_d == 1) && (t == 1)
                         ? TestTags(TestTags::standard())
                         : TestTags(TestTags::sweep()),
                         htc.htc(),htb1.htb(),htb2.htb(),htb3.htb(),
                         c_d,a_d,t);
                  }
              (void) new BAB<FailImmediate>
                (TestTags::standard(),HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,
                 c_d,a_d,t);
              (void) new BAB<SolveImmediate>
                (TestTags::sweep(),HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,
                 c_d,a_d,t);
              (void) new BAB<HasSolutions>
                (TestTags::sweep(),HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,
                 c_d,a_d,t);
            }
        // Restart-based search
        (void)new MetaDispatch("RBS::HubOnly", MetaDispatch::Scenario::RbsHubOnly);
        (void)new MetaDispatch("RBS::SpokeOnly", MetaDispatch::Scenario::RbsSpokeOnly);
        (void)new MetaDispatch("RBS::HubContinue", MetaDispatch::Scenario::RbsHubContinue);
        (void)new MetaDispatch("RBS::HubRestart", MetaDispatch::Scenario::RbsHubRestart);
        (void)new MetaDispatch("RBS::SpokeFull", MetaDispatch::Scenario::RbsSpokeFull);
        (void)new MetaDispatch("RBS::SpokeReduced", MetaDispatch::Scenario::RbsSpokeReduced);
        (void)new MetaDispatch("RBS::SpokePartial", MetaDispatch::Scenario::RbsSpokePartial);
        (void)new MetaDispatch("RBS::Limit", MetaDispatch::Scenario::RbsLimit);
        (void)new MetaDispatch("RBS::NoGoodsResetOwnership",
                               MetaDispatch::Scenario::RbsNoGoodsResetOwnership);
        (void)new MetaDispatch("Lifecycle::StopOwnership", MetaDispatch::Scenario::MetaStopOwnership);
        (void)new MetaDispatch("BAB::Incumbent", MetaDispatch::Scenario::BabIncumbent);
        for (unsigned int t=1; t<=4; t++) {
          (void) new RBS<HasSolutions,Gecode::DFS>("DFS",t);
          (void) new RBS<HasSolutions,Gecode::LDS>("LDS",t);
          (void) new RBS<HasSolutions,Gecode::BAB>("BAB",t,HTC_LEX_LE);
          (void) new RBS<FailImmediate,Gecode::DFS>("DFS",t);
          (void) new RBS<FailImmediate,Gecode::LDS>("LDS",t);
          (void) new RBS<FailImmediate,Gecode::BAB>("BAB",t);
          (void) new RBS<SolveImmediate,Gecode::DFS>("DFS",t);
          (void) new RBS<SolveImmediate,Gecode::LDS>("LDS",t);
          (void) new RBS<SolveImmediate,Gecode::BAB>("BAB",t);
        }
        // Portfolio-based search
        (void)new MetaDispatch("PBS::HubOnly", MetaDispatch::Scenario::PbsHubOnly);
        (void)new MetaDispatch("PBS::SpokeOnly", MetaDispatch::Scenario::PbsSpokeOnly);
        (void)new MetaDispatch("PBS::Single", MetaDispatch::Scenario::PbsSingle);
        (void)new MetaDispatch("PBS::SequentialHubOnly", MetaDispatch::Scenario::PbsSequentialHubOnly);
        (void)new MetaDispatch("PBS::SequentialSpokeOnly", MetaDispatch::Scenario::PbsSequentialSpokeOnly);
        (void)new MetaDispatch("PBS::Sequential", MetaDispatch::Scenario::PbsSequential);
        (void)new MetaDispatch("PBS::SpokeWithoutBrancher", MetaDispatch::Scenario::PbsSpokeWithoutBrancher);
        (void)new MetaDispatch("PBS::NoBrancher", MetaDispatch::Scenario::PbsNoBrancher);
#ifdef GECODE_HAS_THREADS
        (void)new MetaDispatch("PBS::ParallelHubOnly", MetaDispatch::Scenario::PbsParallelHubOnly);
        (void)new MetaDispatch("PBS::ParallelSpokeOnly", MetaDispatch::Scenario::PbsParallelSpokeOnly);
        (void)new MetaDispatch("PBS::Parallel", MetaDispatch::Scenario::PbsParallel);
        (void)new MetaDispatch("PBS::ParallelThreadLimit", MetaDispatch::Scenario::PbsParallelThreadLimit);
#endif
        for (unsigned int a=1; a<=4; a++)
          for (unsigned int t=1; t<=2*a; t++) {
            (void) new PBS<HasSolutions,Gecode::DFS>("DFS",false,a,t);
            (void) new PBS<HasSolutions,Gecode::LDS>("LDS",false,a,t);
            (void) new PBS<HasSolutions,Gecode::BAB>("BAB",true,a,t);
            (void) new PBS<FailImmediate,Gecode::DFS>("DFS",false,a,t);
            (void) new PBS<FailImmediate,Gecode::LDS>("LDS",false,a,t);
            (void) new PBS<FailImmediate,Gecode::BAB>("BAB",true,a,t);
            (void) new PBS<SolveImmediate,Gecode::DFS>("DFS",false,a,t);
            (void) new PBS<SolveImmediate,Gecode::LDS>("LDS",false,a,t);
            (void) new PBS<SolveImmediate,Gecode::BAB>("BAB",true,a,t);
          }
        // Portfolio-based search using SEBs
        for (unsigned int pt=1; pt<=3; pt += 2)
          for (unsigned int at=1; at<=8; at++) {
            (void) new SEBPBS<HasSolutions>("BAB",true,pt,at);
            (void) new SEBPBS<FailImmediate>("BAB",true,pt,at);
            (void) new SEBPBS<SolveImmediate>("BAB",true,pt,at);
            (void) new SEBPBS<HasSolutions>("DFS+LDS",false,pt,at);
            (void) new SEBPBS<FailImmediate>("DFS+LDS",false,pt,at);
            (void) new SEBPBS<SolveImmediate>("DFS+LDS",false,pt,at);
          }
      }
    };

    Create c;
  }

}

// STATISTICS: test-search
