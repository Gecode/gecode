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

#include "test/test.hh"

#include <atomic>
#include <chrono>
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
      bool master(const MetaInfo&) override { return true; }
      const Node& node(void) const { return (*graph)[vertex]; }
      void constrain(const Space& s) override {
        ceiling = static_cast<const TreeSpace&>(s).node().cost;
        if (node().cost >= ceiling) fail();
      }
      /// Guidance minimizing cost, or maximizing negated cost
      static Gecode::Search::HeuristicGuidance
      guidance(bool max, bool ranked, bool dominance, bool keyed) {
        using Gecode::Search::HeuristicGuidance;
        HeuristicGuidance g(max ? HeuristicGuidance::MAXIMIZE :
                                  HeuristicGuidance::MINIMIZE,
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
      /// Master configuration function that does not restart
      virtual bool master(const MetaInfo& mi) {
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
      virtual bool master(const MetaInfo& mi) {
        switch (mi.type()) {
        case MetaInfo::RESTART:
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
        HasSolutions root(HTB_BINARY,HTB_BINARY,HTB_BINARY);
        E<HasSolutions> engine(&root,o);
        auto result = std::async(std::launch::async,[&] { return engine.next(); });
        auto reached = pause.reached.get_future();
        reached.wait();
        bool ok = result.wait_for(std::chrono::milliseconds(20)) ==
          std::future_status::timeout;
        control.request(1U);
        int solutions = 0;
        if (HasSolutions* solution = result.get()) {
          ++solutions;
          delete solution;
        }
        // Alternate expansion and shrinking at solution boundaries. This
        // exercises each request while preserving complete enumeration.
        while (true) {
          control.request((solutions % 2) ? threads : 1U);
          HasSolutions* solution = engine.next();
          if (!solution)
            break;
          ++solutions;
          delete solution;
        }
        return ok && !engine.stopped() && (solutions == root.solutions());
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
      /// Initialize test
      RBS(const std::string& e, unsigned int t0)
        : Test("RBS::"+e+"::"+Model::name()+"::"+str(t0),
               HTB_BINARY,HTB_BINARY,HTB_BINARY), t(t0) {}
      /// Run test
      virtual bool run(void) {
        Model* m = new Model(htb1,htb2,htb3);
        Gecode::Search::FailStop f(2);
        Gecode::Search::Options o;
        o.threads = t;
        o.stop = &f;
        o.d_l = 100;
        o.cutoff = Gecode::Search::Cutoff::geometric(1,2);
        Gecode::RBS<Model,Engine> rbs(m,o);
        int n = m->solutions();
        delete m;
        while (true) {
          Model* s = rbs.next();
          if (s != nullptr) {
            n--; delete s;
          }
          if ((s == nullptr) && !rbs.stopped())
            break;
          f.limit(f.limit()+2);
        }
        return n == 0;
      }
    };

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
      /// Number of master threads
      unsigned int mt;
      /// Number of slave threads
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
        for (unsigned int t=1; t<=4; t++) {
          (void) new RBS<HasSolutions,Gecode::DFS>("DFS",t);
          (void) new RBS<HasSolutions,Gecode::LDS>("LDS",t);
          (void) new RBS<HasSolutions,Gecode::BAB>("BAB",t);
          (void) new RBS<FailImmediate,Gecode::DFS>("DFS",t);
          (void) new RBS<FailImmediate,Gecode::LDS>("LDS",t);
          (void) new RBS<FailImmediate,Gecode::BAB>("BAB",t);
          (void) new RBS<SolveImmediate,Gecode::DFS>("DFS",t);
          (void) new RBS<SolveImmediate,Gecode::LDS>("LDS",t);
          (void) new RBS<SolveImmediate,Gecode::BAB>("BAB",t);
        }
        // Portfolio-based search
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
