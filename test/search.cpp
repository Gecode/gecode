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
      bool hub(const MetaInfo&) override { return true; }
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
      return e.stopped() || (last == (*root.graph)[0].best);
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
                  if ((o.max_width == 0UL) && e.stopped())
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
        return (last == 1) && e.stopped();
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
      /// Hub configuration function that does not restart
      virtual bool hub(const MetaInfo& mi) {
        if (mi.type() == MetaInfo::RESTART) {
          if (mi.last() != nullptr)
            constrain(*mi.last());
          return false;
        } else {
          return false;
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
      virtual bool hub(const MetaInfo& mi) {
        switch (mi.type()) {
        case MetaInfo::RESTART:
          // With an objective, restart from a bound improving on the last solution
          if (htc != HTC_NONE)
            return Space::hub(mi);
          if (mi.last() != nullptr) {
            const HasSolutions* s
              = static_cast<const HasSolutions*>(mi.last());
            BoolVarArgs b;
            for (int i=0; i<x.size(); i++)
              b << expr(*this, x[i] == s->x[i]);
            rel(*this, BOT_AND, b, 0);
          }
          break;
        case MetaInfo::PORTFOLIO:
          // Do not kill the brancher!
          break;
        default:
          break;
        }
        return false;
      }
      /// Use the default completeness contract for each derived exploration
      bool
      spoke(const MetaInfo&) override {
        return true;
      }
    };

    /// Return a bit for restart reason \a r
    static unsigned int
    reason_bit(MetaInfo::RestartReason r) {
      return 1U << static_cast<unsigned int>(r);
    }

    /// Observations shared by meta-search dispatch test spaces
    struct MetaDispatchState {
      std::atomic<unsigned int> hub_calls{0};
      std::atomic<unsigned int> spoke_calls{0};
      std::atomic<unsigned int> hub_reasons{0};
      std::atomic<unsigned int> spoke_reasons{0};
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
      bool restart_after_solution = true;
      bool solution_spoke_complete = true;
      bool fail_solution_spoke = false;
      bool nogood_model = false;
    };

    /// Common implementation for meta-search dispatch test spaces
    class MetaDispatchSpace : public Space {
    protected:
      /// RBS role of a space
      enum class RestartRole { HUB, SPOKE };
      /// Variable used to provide two solutions
      IntVar x;
      /// Optional queens model used to force restart no-good extraction
      IntVarArray q;
      /// Shared hook observations and configuration
      MetaDispatchState* state;
      /// RBS role retained across cloning
      RestartRole restart_role;
      /// PBS asset identity, or -1 before portfolio dispatch
      int portfolio_asset;

      /// Record and implement a Hub callback
      bool
      observe_hub(const MetaInfo& mi) {
        state->hub_calls.fetch_add(1, std::memory_order_relaxed);
        if (mi.type() == MetaInfo::PORTFOLIO)
          // PBS must ignore the callback result.
          return false;
        restart_role = RestartRole::HUB;
        state->hub_reasons.fetch_or(reason_bit(mi.reason()), std::memory_order_relaxed);
        bool default_result = Space::hub(mi);
        if ((mi.type() == MetaInfo::RESTART) && (mi.nogoods().ng() > 0))
          state->nogood_handoffs.fetch_add(1, std::memory_order_relaxed);
        return (mi.reason() == MetaInfo::RR_SOL) ? state->restart_after_solution : default_result;
      }
      /// Record and implement a Spoke callback
      bool
      observe_spoke(const MetaInfo& mi) {
        state->spoke_calls.fetch_add(1, std::memory_order_relaxed);
        if (mi.type() == MetaInfo::PORTFOLIO) {
          portfolio_asset = static_cast<int>(mi.asset());
          state->assets.fetch_or(1U << mi.asset(), std::memory_order_relaxed);
          // PBS must ignore the callback result.
          return false;
        }
        state->spoke_reasons.fetch_or(reason_bit(mi.reason()), std::memory_order_relaxed);
        restart_role = RestartRole::SPOKE;
        if ((mi.reason() == MetaInfo::RR_SOL) && state->fail_solution_spoke)
          fail();
        return (mi.reason() == MetaInfo::RR_SOL) ? state->solution_spoke_complete : true;
      }

    public:
      /// Initialize with shared observations \a s
      MetaDispatchSpace(MetaDispatchState& s)
        : x(*this, 0, 1), q(*this, s.nogood_model ? 8 : 0, 0, 7), state(&s), restart_role(RestartRole::HUB),
          portfolio_asset(-1) {
        state->live_spaces.fetch_add(1, std::memory_order_relaxed);
        if (state->nogood_model) {
          distinct(*this, IntArgs::create(q.size(), 0, 1), q, IPL_VAL);
          distinct(*this, IntArgs::create(q.size(), 0, -1), q, IPL_VAL);
          distinct(*this, q, IPL_VAL);
          Gecode::branch(*this, q, INT_VAR_NONE(), INT_VAL_MIN());
        }
        Gecode::branch(*this, x, INT_VAL_MIN());
      }
      /// Clone \a s
      MetaDispatchSpace(MetaDispatchSpace& s)
        : Space(s), state(s.state), restart_role(s.restart_role), portfolio_asset(s.portfolio_asset) {
        state->live_spaces.fetch_add(1, std::memory_order_relaxed);
        state->copies.fetch_add(1, std::memory_order_relaxed);
        x.update(*this, s.x);
        q.update(*this, s.q);
      }
      /// Record destruction for clone-ownership checks
      ~MetaDispatchSpace(void) override {
        state->live_spaces.fetch_sub(1, std::memory_order_relaxed);
      }
      /// Require a larger value than solution \a s
      void
      constrain(const Space& s) override {
        state->constrain_calls.fetch_add(1, std::memory_order_relaxed);
        if (restart_role == RestartRole::HUB)
          state->constrain_hub_calls.fetch_add(1, std::memory_order_relaxed);
        else
          state->constrain_spoke_calls.fetch_add(1, std::memory_order_relaxed);
        if (portfolio_asset >= 0) {
          const unsigned int asset = 1U << static_cast<unsigned int>(portfolio_asset);
          state->constrain_asset_mask.fetch_or(asset, std::memory_order_relaxed);
          if (restart_role == RestartRole::HUB)
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
      bool
      hub(const MetaInfo& mi) override {
        return observe_hub(mi);
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
      bool
      spoke(const MetaInfo& mi) override {
        return observe_spoke(mi);
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
      bool
      hub(const MetaInfo& mi) override {
        return observe_hub(mi);
      }
      /// Observe Spoke dispatch
      bool
      spoke(const MetaInfo& mi) override {
        return observe_spoke(mi);
      }
    };

    /// Hub and Spoke hook dispatch through RBS and PBS
    class MetaDispatch : public Base {
    public:
      /// Dispatch scenario
      enum class Scenario {
        RBS_HUB_ONLY,
        RBS_SPOKE_ONLY,
        RBS_HUB_CONTINUE,
        RBS_HUB_RESTART,
        RBS_SPOKE_COMPLETE,
        RBS_SPOKE_INCOMPLETE,
        PBS_HUB_ONLY,
        PBS_SPOKE_ONLY,
        PBS_SINGLE,
        PBS_SEQUENTIAL_HUB_ONLY,
        PBS_SEQUENTIAL_SPOKE_ONLY,
        PBS_SEQUENTIAL,
        PBS_PARALLEL_HUB_ONLY,
        PBS_PARALLEL_SPOKE_ONLY,
        PBS_PARALLEL,
        RBS_NOGOODS_RESET_OWNERSHIP,
        META_STOP_OWNERSHIP,
        BAB_INCUMBENT
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
      rbs(bool restart_after_solution, bool fail_solution_spoke, bool solution_spoke_complete) {
        MetaDispatchState state;
        state.restart_after_solution = restart_after_solution;
        state.fail_solution_spoke = fail_solution_spoke;
        state.solution_spoke_complete = solution_spoke_complete;

        HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(state);
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
        delete m;

        HubSpokeDispatchSpace* first = search.next();
        bool ok = first != nullptr;
        delete first;

        HubSpokeDispatchSpace* second = search.next();
        if (!fail_solution_spoke) {
          ok = ok && (second != nullptr) && holds(state.hub_calls, 1U) &&
               holds(state.spoke_calls, restart_after_solution ? 2U : 1U) &&
               holds(state.hub_reasons, reason_bit(MetaInfo::RR_SOL)) &&
               holds(state.spoke_reasons, reason_bit(MetaInfo::RR_INIT) |
                                              (restart_after_solution ? reason_bit(MetaInfo::RR_SOL) : 0U));
        } else if (solution_spoke_complete) {
          ok = ok && (second == nullptr) && holds(state.hub_calls, 1U) && holds(state.spoke_calls, 2U) &&
               holds(state.hub_reasons, reason_bit(MetaInfo::RR_SOL)) &&
               holds(state.spoke_reasons, reason_bit(MetaInfo::RR_INIT) | reason_bit(MetaInfo::RR_SOL));
        } else {
          ok = ok && (second != nullptr) && holds(state.hub_calls, 2U) && holds(state.spoke_calls, 3U) &&
               holds(state.hub_reasons, reason_bit(MetaInfo::RR_SOL) | reason_bit(MetaInfo::RR_CMPL)) &&
               holds(state.spoke_reasons, reason_bit(MetaInfo::RR_INIT) | reason_bit(MetaInfo::RR_SOL) |
                                              reason_bit(MetaInfo::RR_CMPL));
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
        bool ok = (solution != nullptr) && holds(state.hub_calls, hub_calls) &&
                  holds(state.spoke_calls, spoke_calls) && holds(state.assets, asset_mask);
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
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.nogoods_limit = 16;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::RBS<HubSpokeDispatchSpace, Gecode::DFS> search(m, o);
          delete m;

          unsigned int solutions = 0;
          while (HubSpokeDispatchSpace* solution = search.next()) {
            solutions++;
            delete solution;
          }
          ok = (solutions == 2U) && (state.spoke_calls.load(std::memory_order_relaxed) >= 2U) &&
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
        rbs_state.restart_after_solution = false;
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
          ok = (solution == nullptr) && search.stopped();
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
          ok = ok && (solution == nullptr) && search.stopped();
          delete solution;
        }
        return ok && holds(pbs_state.live_spaces, 0U);
      }
      /// Exercise incumbent propagation through RBS and PBS wrappers
      static bool
      bab_incumbent(void) {
        MetaDispatchState rbs_state;
        // Continue after a solution, so that the spoke engine prunes with it
        rbs_state.restart_after_solution = false;
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
          HubSpokeDispatchSpace* best = nullptr;
          while (HubSpokeDispatchSpace* solution = static_cast<HubSpokeDispatchSpace*>(search->next())) {
            search->constrain(*solution);
            delete best;
            best = solution;
          }
          ok = (best != nullptr) && (best->value() == 1) &&
               (rbs_state.constrain_hub_calls.load(std::memory_order_relaxed) > 0U) &&
               (rbs_state.constrain_spoke_calls.load(std::memory_order_relaxed) > 0U);
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
        nested_state.restart_after_solution = false;
        {
          HubSpokeDispatchSpace* m = new HubSpokeDispatchSpace(nested_state);
          Gecode::Search::Options outer;
          outer.assets = 3;
          outer.threads = 1;
          Gecode::Search::Options inner[3];
          SEBs spokes(3);
          for (int i = 0; i < 3; i++) {
            inner[i].threads = 1;
            // Each RBS engine owns and deletes its cutoff.
            inner[i].cutoff = Gecode::Search::Cutoff::constant(100);
            spokes[i] = Gecode::rbs<HubSpokeDispatchSpace, Gecode::BAB>(inner[i]);
          }
          Gecode::PBS<HubSpokeDispatchSpace, Gecode::BAB> search(m, spokes, outer);
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
      MetaDispatch(const std::string& n, Scenario s) : Base("Search::MetaDispatch::" + n), scenario(s) {}
      /// Run test
      bool
      run(void) override {
        switch (scenario) {
        case Scenario::RBS_HUB_ONLY:
          return rbs_dispatch<HubDispatchSpace>(1, 0);
        case Scenario::RBS_SPOKE_ONLY:
          return rbs_dispatch<SpokeDispatchSpace>(0, 2);
        case Scenario::RBS_HUB_CONTINUE:
          return rbs(false, false, true);
        case Scenario::RBS_HUB_RESTART:
          return rbs(true, false, true);
        case Scenario::RBS_SPOKE_COMPLETE:
          return rbs(true, true, true);
        case Scenario::RBS_SPOKE_INCOMPLETE:
          return rbs(true, true, false);
        case Scenario::PBS_HUB_ONLY:
          return pbs<HubDispatchSpace>(1, 1, 1, 0, 0);
        case Scenario::PBS_SPOKE_ONLY:
          return pbs<SpokeDispatchSpace>(1, 1, 0, 1, 1);
        case Scenario::PBS_SINGLE:
          return pbs<HubSpokeDispatchSpace>(1, 1, 1, 1, 1);
        case Scenario::PBS_SEQUENTIAL_HUB_ONLY:
          return pbs<HubDispatchSpace>(3, 1, 1, 0, 0);
        case Scenario::PBS_SEQUENTIAL_SPOKE_ONLY:
          return pbs<SpokeDispatchSpace>(3, 1, 0, 3, 7);
        case Scenario::PBS_SEQUENTIAL:
          return pbs<HubSpokeDispatchSpace>(3, 1, 1, 3, 7);
        case Scenario::PBS_PARALLEL_HUB_ONLY:
          return pbs<HubDispatchSpace>(3, 3, 1, 0, 0);
        case Scenario::PBS_PARALLEL_SPOKE_ONLY:
          return pbs<SpokeDispatchSpace>(3, 3, 0, 3, 7);
        case Scenario::PBS_PARALLEL:
          return pbs<HubSpokeDispatchSpace>(3, 3, 1, 3, 7);
        case Scenario::RBS_NOGOODS_RESET_OWNERSHIP:
          return rbs_nogoods_reset_ownership();
        case Scenario::META_STOP_OWNERSHIP:
          return meta_stop_ownership();
        case Scenario::BAB_INCUMBENT:
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
        return ok && !engine.stopped() && complete;
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
        return ok && !engine.stopped() && (last == (*root.graph)[0].best);
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
        return !engine.stopped();
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
      virtual bool hub(const MetaInfo& mi) {
        if (mi.type() == MetaInfo::PORTFOLIO)
          return false;
        return Space::hub(mi);
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
      virtual bool hub(const MetaInfo& mi) {
        if (mi.type() == MetaInfo::PORTFOLIO)
          return false;
        return Space::hub(mi);
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
        if (bounded->stopped())
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
        substep = substep && (found == 1) && !bab->stopped();
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
          if ((s == nullptr) && !dfs.stopped())
            break;
          f.limit(f.limit()+2);
        }
        return n == 0;
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
          if ((s == nullptr) && !lds.stopped())
            break;
          f.limit(f.limit()+2);
        }
        return n == 0;
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
          if ((s == nullptr) && !bab.stopped())
            break;
          f.limit(f.limit()+2);
        }
        bool ok = (b == nullptr) || b->best();
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
          if ((s == nullptr) && !rbs.stopped())
            break;
          f.limit(f.limit()+2);
        }
        if (htc == HTC_NONE)
          return n == 0;
        bool ok = improving && (b != nullptr) && b->best();
        delete b;
        return ok;
      }
    };

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
      bool spoke(const Gecode::MetaInfo& mi) override {
        if ((mi.type() == Gecode::MetaInfo::RESTART) &&
            (mi.reason() == Gecode::MetaInfo::RR_INIT)) {
          Gecode::dom(*this,x[2],0,1);
          Gecode::dom(*this,x[3],0,1);
          return false;
        }
        return true;
      }
    };

    /// No-goods from an incomplete spoke search are not posted in the hub
    class RBSIncompleteNoGoods : public Base {
    public:
      RBSIncompleteNoGoods(void) : Base("Search::RBS::IncompleteNoGoods") {}
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
        return !rbs.stopped() && (best == 0);
      }
    };

    RBSIncompleteNoGoods rbs_incomplete_nogoods;

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
      bool hub(const Gecode::MetaInfo& mi) override {
        // Keep the branchers for the assets of a portfolio
        if (mi.type() == Gecode::MetaInfo::PORTFOLIO)
          return true;
        return Gecode::IntMinimizeSpace::hub(mi);
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
      bool hub(const Gecode::MetaInfo& mi) override {
        if ((mi.type() == Gecode::MetaInfo::RESTART) &&
            (mi.reason() == Gecode::MetaInfo::RR_LIM))
          fail();
        return true;
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
          if (rbs.stopped() || (s.restart != 1U) || (s.fail < 2U) ||
              ((t == 1U) && (s.fail != 2U)))
            return false;
          if ((rbs.next() != nullptr) || rbs.stopped() ||
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
      bool spoke(const Gecode::MetaInfo& mi) override {
        if ((mi.type() == Gecode::MetaInfo::RESTART) && (mi.restart() > 0)) {
          fail();
          return false;
        }
        return true;
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
        return rbs.stopped() && (f.checks <= 1000) &&
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
      bool spoke(const Gecode::MetaInfo& mi) override {
        if ((mi.type() == Gecode::MetaInfo::RESTART) &&
            (mi.reason() == Gecode::MetaInfo::RR_INIT)) {
          fail();
          return false;
        }
        return true;
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

    /// A search exhausted when it reaches a failure limit is not stopped
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
          if ((e.next() != nullptr) || e.stopped())
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
          if ((rbs.next() != nullptr) || rbs.stopped())
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
            if ((s == nullptr) && !pbs.stopped())
              break;
            f.limit(f.limit()+2);
          }
          bool ok = (b == nullptr) || b->best();
          delete b;
          return ok;
        } else {
          int n = ((t > 1) ? std::min(a,t) : a) * m->solutions();
          delete m;
          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              n--; delete s;
            }
            if ((s == nullptr) && !pbs.stopped())
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
      unsigned int mt;
      /// Number of spoke threads
      unsigned int st;
    public:
      /// Initialize test
      SEBPBS(const std::string& e, bool b, unsigned int mt0, unsigned int st0)
        : Test("PBS::SEB::"+e+"::"+Model::name()+"::"+str(mt0)+"::"+str(st0),
               HTB_BINARY,HTB_BINARY,HTB_BINARY), best(b), mt(mt0), st(st0) {}
      /// Run test
      virtual bool run(void) {
        using namespace Gecode;
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);

        Gecode::Search::Options mo;
        mo.threads = mt;
        mo.d_l = 100;
        mo.stop = &f;

        Gecode::Search::Options so;
        so.threads = st;
        so.d_l = 100;
        so.cutoff = Gecode::Search::Cutoff::constant(1000000);
        if (best) {
          SEBs sebs(3);
          sebs[0] = bab<Model>(so);
          sebs[1] = bab<Model>(so);
          sebs[2] = rbs<Model,Gecode::BAB>(so);
          Gecode::PBS<Model,Gecode::BAB> pbs(m, sebs, mo);
          delete m;

          Model* b = nullptr;
          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              delete b; b=s;
            }
            if ((s == nullptr) && !pbs.stopped())
              break;
            f.limit(f.limit()+2);
          }
          bool ok = (b == nullptr) || b->best();
          delete b;
          return ok;
        } else {
          SEBs sebs(3);
          sebs[0] = dfs<Model>(so);
          sebs[1] = lds<Model>(so);
          sebs[2] = rbs<Model,Gecode::DFS>(so);
          Gecode::PBS<Model,Gecode::DFS> pbs(m, sebs, mo);

          int n = 3 * m->solutions();
          delete m;

          while (true) {
            Model* s = pbs.next();
            if (s != nullptr) {
              n--; delete s;
            }
            if ((s == nullptr) && !pbs.stopped())
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
        return (pbs.next() == nullptr) && pbs.stopped() && (l.checks <= 1000);
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

    /// Restart-based assets of a portfolio share the cutoff sequence
    class PBSRestartCutoff : public Base {
      template<template<class> class Engine>
      static bool test(unsigned int threads) {
        NoSolution root;
        Gecode::Search::Options o;
        o.assets = 2;
        o.threads = threads;
        o.cutoff = Gecode::Search::Cutoff::geometric(1,2);
        Gecode::PBS<NoSolution,Engine> pbs(&root,o);
        return (pbs.next() == nullptr) && !pbs.stopped();
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
      bool spoke(const Gecode::MetaInfo& mi) override {
        if ((mi.type() == Gecode::MetaInfo::PORTFOLIO) && (mi.asset() == 0U))
          fail();
        return true;
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
          if (pbs.stopped())
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
        (void)new MetaDispatch("RBS::HubOnly", MetaDispatch::Scenario::RBS_HUB_ONLY);
        (void)new MetaDispatch("RBS::SpokeOnly", MetaDispatch::Scenario::RBS_SPOKE_ONLY);
        (void)new MetaDispatch("RBS::HubContinue", MetaDispatch::Scenario::RBS_HUB_CONTINUE);
        (void)new MetaDispatch("RBS::HubRestart", MetaDispatch::Scenario::RBS_HUB_RESTART);
        (void)new MetaDispatch("RBS::SpokeComplete", MetaDispatch::Scenario::RBS_SPOKE_COMPLETE);
        (void)new MetaDispatch("RBS::SpokeIncomplete", MetaDispatch::Scenario::RBS_SPOKE_INCOMPLETE);
        (void)new MetaDispatch("RBS::NoGoodsResetOwnership",
                               MetaDispatch::Scenario::RBS_NOGOODS_RESET_OWNERSHIP);
        (void)new MetaDispatch("Lifecycle::StopOwnership", MetaDispatch::Scenario::META_STOP_OWNERSHIP);
        (void)new MetaDispatch("BAB::Incumbent", MetaDispatch::Scenario::BAB_INCUMBENT);
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
        (void)new MetaDispatch("PBS::HubOnly", MetaDispatch::Scenario::PBS_HUB_ONLY);
        (void)new MetaDispatch("PBS::SpokeOnly", MetaDispatch::Scenario::PBS_SPOKE_ONLY);
        (void)new MetaDispatch("PBS::Single", MetaDispatch::Scenario::PBS_SINGLE);
        (void)new MetaDispatch("PBS::SequentialHubOnly", MetaDispatch::Scenario::PBS_SEQUENTIAL_HUB_ONLY);
        (void)new MetaDispatch("PBS::SequentialSpokeOnly", MetaDispatch::Scenario::PBS_SEQUENTIAL_SPOKE_ONLY);
        (void)new MetaDispatch("PBS::Sequential", MetaDispatch::Scenario::PBS_SEQUENTIAL);
#ifdef GECODE_HAS_THREADS
        (void)new MetaDispatch("PBS::ParallelHubOnly", MetaDispatch::Scenario::PBS_PARALLEL_HUB_ONLY);
        (void)new MetaDispatch("PBS::ParallelSpokeOnly", MetaDispatch::Scenario::PBS_PARALLEL_SPOKE_ONLY);
        (void)new MetaDispatch("PBS::Parallel", MetaDispatch::Scenario::PBS_PARALLEL);
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
        for (unsigned int mt=1; mt<=3; mt += 2)
          for (unsigned int st=1; st<=8; st++) {
            (void) new SEBPBS<HasSolutions>("BAB",true,mt,st);
            (void) new SEBPBS<FailImmediate>("BAB",true,mt,st);
            (void) new SEBPBS<SolveImmediate>("BAB",true,mt,st);
            (void) new SEBPBS<HasSolutions>("DFS+LDS",false,mt,st);
            (void) new SEBPBS<FailImmediate>("DFS+LDS",false,mt,st);
            (void) new SEBPBS<SolveImmediate>("DFS+LDS",false,mt,st);
          }
      }
    };

    Create c;
  }

}

// STATISTICS: test-search
