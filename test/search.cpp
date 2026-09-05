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

#include "test/test.hh"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>

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
      Test(const std::string& s,
           HowToBranch _htb1, HowToBranch _htb2, HowToBranch _htb3,
           HowToConstrain _htc=HTC_NONE)
        : Base("Search::"+s),
          htb1(_htb1), htb2(_htb2), htb3(_htb3), htc(_htc) {}
    };

    /// Randomly resize a live leaf engine from another thread
    class WorkerResizer {
    private:
      Gecode::Search::WorkerControl control;
      unsigned int capacity;
      std::atomic<bool> done;
      std::thread worker;
      void resize(unsigned int seed) {
        Gecode::Support::RandomGenerator rand(seed);
        while (!done.load(std::memory_order_acquire)) {
          control.request(rand(capacity+1U));
          std::this_thread::yield();
        }
      }
    public:
      WorkerResizer(const Gecode::Search::WorkerControl& control0,
                    unsigned int capacity0, unsigned int seed, bool enabled)
        : control(control0), capacity(capacity0), done(false) {
        if (enabled)
          worker = std::thread(&WorkerResizer::resize,this,seed);
      }
      ~WorkerResizer(void) {
        done.store(true,std::memory_order_release);
        if (worker.joinable())
          worker.join();
      }
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
    public:
      PBSConstructionFailure(void) : Base("Search::PBS::ConstructionFailure") {}
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
        return true;
      }
    };

    PBSConstructionFailure pbs_construction_failure;

    /// Pause before search or after its first node, then resume enumeration
    class WorkerControlSequentialPause : public Base {
      class Pause : public Gecode::Search::Stop {
        Gecode::Search::WorkerControl control;
        unsigned int node;
        bool paused;
      public:
        std::promise<void> reached;
        Pause(const Gecode::Search::WorkerControl& c, unsigned int n)
          : control(c), node(n), paused(false) {}
        bool stop(const Gecode::Search::Statistics& s,
                  const Gecode::Search::Options&) override {
          if (!paused && (s.node >= node)) {
            paused = true;
            control.request(0U);
            reached.set_value();
          }
          return false;
        }
      };
      template<template<class> class E>
      bool check(unsigned int node) {
        Gecode::Search::WorkerControl control(node == 0U ? 0U : 1U);
        Pause pause(control,node);
        Gecode::Search::Options o;
        o.threads = 1.0;
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
        while (HasSolutions* solution = engine.next()) {
          ++solutions;
          delete solution;
        }
        return ok && !engine.stopped() && (solutions == root.solutions());
      }
    public:
      WorkerControlSequentialPause(void)
        : Base("Search::WorkerControl::SequentialPause") {}
      bool run(void) override {
#ifdef GECODE_HAS_THREADS
        return check<Gecode::DFS>(0U) && check<Gecode::DFS>(1U) &&
          check<Gecode::BAB>(0U) && check<Gecode::BAB>(1U);
#else
        return true;
#endif
      }
    };

    WorkerControlSequentialPause worker_control_sequential_pause;

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
      WorkerControlAPI(void) : Base("Search::WorkerControl::API") {}
      bool run(void) override {
        using Gecode::Search::WorkerControl;
        WorkerControl empty;
        bool ok = !empty && (empty.requested() == 0U) &&
          (empty.capacity() == 0U) &&
          throws<Gecode::Search::UninitializedWorkerControl>(
            [&] { empty.request(1U); });

        const unsigned int capacity = worker_capacity(2U);
        WorkerControl control(0U);
        WorkerControl copy(control);
        ok = ok && copy && (copy.requested() == 0U);

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
        return ok;
      }
    };

    WorkerControlAPI worker_control_api;

    /// A paused PBS asset must not hold the portfolio round open
    class WorkerControlPBSPause : public Base {
    public:
      WorkerControlPBSPause(void)
        : Base("Search::WorkerControl::PBS::Pause") {}
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
      DFS(HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int c_d0, unsigned int a_d0, unsigned int t0)
        : Test("DFS::"+Model::name()+"::"+
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
        const unsigned int capacity = worker_capacity(t);
        const bool adjustable = _rand(2U) != 0U;
        Gecode::Search::WorkerControl control;
        if (adjustable) {
          control = Gecode::Search::WorkerControl(1U + _rand(capacity));
          o.worker_control = control;
        }
        Gecode::DFS<Model> dfs(m,o);
        WorkerResizer resize(control,capacity,_rand.next(),
                             adjustable && (capacity > 1U) &&
                             (_rand(64U) == 0U));
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
      BAB(HowToConstrain htc,
          HowToBranch htb1, HowToBranch htb2, HowToBranch htb3,
          unsigned int c_d0, unsigned int a_d0, unsigned int t0)
        : Test("BAB::"+Model::name()+"::"+str(htc)+"::"+
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
        const unsigned int capacity = worker_capacity(t);
        const bool adjustable = _rand(2U) != 0U;
        Gecode::Search::WorkerControl control;
        if (adjustable) {
          control = Gecode::Search::WorkerControl(1U + _rand(capacity));
          o.worker_control = control;
        }
        Gecode::BAB<Model> bab(m,o);
        WorkerResizer resize(control,capacity,_rand.next(),
                             adjustable && (capacity > 1U) &&
                             (_rand(64U) == 0U));
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
        const unsigned int capacity = worker_capacity(t);
        const bool adjustable = _rand(2U) != 0U;
        Gecode::Search::WorkerControl control;
        if (adjustable) {
          control = Gecode::Search::WorkerControl(1U + _rand(capacity));
          o.worker_control = control;
        }
        Gecode::RBS<Model,Engine> rbs(m,o);
        WorkerResizer resize(control,capacity,_rand.next(),
                             adjustable && (capacity > 1U));
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
        Gecode::Search::WorkerControl control;
        if (a == 1U) {
          const unsigned int capacity = worker_capacity(t);
          control = Gecode::Search::WorkerControl(1U + _rand(capacity));
          o.worker_control = control;
        }
        Gecode::PBS<Model,Engine> pbs(m,o);
        WorkerResizer resize(control,worker_capacity(t),_rand.next(),
                             (a == 1U) && (worker_capacity(t) > 1U));
        if (best) {
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
        const unsigned int capacity = worker_capacity(st);
        Gecode::Search::WorkerControl controls[3];
        Gecode::Search::Options leaf[3] = {so,so,so};
        for (unsigned int i=0U; i<3U; i++)
          if ((worker_capacity(mt) > 1U) && (best || (i != 1U))) {
            controls[i] =
              Gecode::Search::WorkerControl(1U + _rand(capacity));
            leaf[i].worker_control = controls[i];
          }
        if (best) {
          SEBs sebs(3);
          sebs[0] = bab<Model>(leaf[0]);
          sebs[1] = bab<Model>(leaf[1]);
          sebs[2] = rbs<Model,Gecode::BAB>(leaf[2]);
          Gecode::PBS<Model,Gecode::BAB> pbs(m, sebs, mo);
          delete m;

          Model* b = nullptr;
          while (true) {
            for (unsigned int i=0U; i<3U; i++)
              if (controls[i])
                controls[i].request(1U + _rand(capacity));
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
          sebs[0] = dfs<Model>(leaf[0]);
          sebs[1] = lds<Model>(leaf[1]);
          sebs[2] = rbs<Model,Gecode::DFS>(leaf[2]);
          Gecode::PBS<Model,Gecode::DFS> pbs(m, sebs, mo);

          int n = 3 * m->solutions();
          delete m;

          while (true) {
            if (controls[0])
              controls[0].request(1U + _rand(capacity));
            if (controls[2])
              controls[2].request(1U + _rand(capacity));
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
                      (htb1.htb(),htb2.htb(),htb3.htb(),c_d, a_d, t);
              new DFS<FailImmediate>(HTB_NONE, HTB_NONE, HTB_NONE,
                                     c_d, a_d, t);
              new DFS<SolveImmediate>(HTB_NONE, HTB_NONE, HTB_NONE,
                                      c_d, a_d, t);
              new DFS<HasSolutions>(HTB_NONE, HTB_NONE, HTB_NONE,
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
                        (htc.htc(),htb1.htb(),htb2.htb(),htb3.htb(),
                         c_d,a_d,t);
                  }
              (void) new BAB<FailImmediate>
                (HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,c_d,a_d,t);
              (void) new BAB<SolveImmediate>
                (HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,c_d,a_d,t);
              (void) new BAB<HasSolutions>
                (HTC_NONE,HTB_NONE,HTB_NONE,HTB_NONE,c_d,a_d,t);
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
