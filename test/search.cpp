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
static_assert(std::is_copy_constructible<Gecode::Search::Options>::value,
              "Search::Options must remain copy constructible");
static_assert(std::is_copy_assignable<Gecode::Search::Options>::value,
              "Search::Options must remain copy assignable");

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

    /// Focused tests for external worker control
    class WorkerControl : public Base {
    private:
      enum Scenario {
        VALUES,
        ERRORS,
        BINDING,
        CONCURRENT,
        COMPATIBILITY,
        DFS_RESIZE_ENUMERATION,
        DFS_RESIZE_BOUNDARIES,
        DFS_RESIZE_HANDOFF,
        DFS_RESIZE_LIFECYCLE,
        BAB_RESIZE_OPTIMALITY_DEFAULT,
        BAB_RESIZE_OPTIMALITY_FREQUENT,
        BAB_RESIZE_INCUMBENTS,
        BAB_RESIZE_LIFECYCLE
      } scenario;

      /// Stable finite tree used by the DFS resizing tests
      class ResizeSpace : public Gecode::Space {
      public:
        Gecode::BoolVarArray x;

        ResizeSpace(void)
          : x(*this,12,0,1) {
          Gecode::branch(*this,x,Gecode::BOOL_VAR_NONE(),
                         Gecode::BOOL_VAL_MIN());
        }

        ResizeSpace(ResizeSpace& s)
          : Gecode::Space(s) {
          x.update(*this,s.x);
        }

        virtual Gecode::Space* copy(void) {
          return new ResizeSpace(*this);
        }

        unsigned int id(void) const {
          unsigned int result = 0U;
          for (int i=0; i<x.size(); i++)
            result = (result << 1) | static_cast<unsigned int>(x[i].val());
          return result;
        }
      };

      /// One non-stealable current solution and no stealable path
      class SingleSolutionSpace : public Gecode::Space {
      public:
        SingleSolutionSpace(void) {}
        SingleSolutionSpace(SingleSolutionSpace& s)
          : Gecode::Space(s) {}
        virtual Gecode::Space* copy(void) {
          return new SingleSolutionSpace(*this);
        }
      };

      /// Finite minimization tree with a unique optimum
      class BABResizeSpace : public Gecode::Space {
      public:
        Gecode::IntVar x;

        BABResizeSpace(void)
          : x(*this,0,4095) {
          Gecode::branch(*this,x,Gecode::INT_VAL_MAX());
        }

        explicit BABResizeSpace(int value)
          : x(*this,value,value) {}

        BABResizeSpace(int min, int max)
          : x(*this,min,max) {
          Gecode::branch(*this,x,Gecode::INT_VAL_MAX());
        }

        BABResizeSpace(BABResizeSpace& s)
          : Gecode::Space(s) {
          x.update(*this,s.x);
        }

        virtual Gecode::Space* copy(void) {
          return new BABResizeSpace(*this);
        }

        virtual void constrain(const Gecode::Space& _s) {
          const BABResizeSpace& s =
            static_cast<const BABResizeSpace&>(_s);
          Gecode::rel(*this,x,Gecode::IRT_LE,s.x.val());
        }

        int value(void) const {
          return x.val();
        }
      };

      /// Thread-safe trace lifecycle observations
      class CountingTracer : public Gecode::SearchTracer {
      public:
        std::atomic<unsigned int> init_count;
        std::atomic<unsigned int> node_count;
        std::atomic<unsigned int> done_count;
        std::atomic<unsigned int> nodes_at_done;

        CountingTracer(void)
          : init_count(0U), node_count(0U), done_count(0U),
            nodes_at_done(0U) {}
        virtual void init(void) {
          (void) init_count.fetch_add(1U,std::memory_order_relaxed);
        }
        virtual void round(unsigned int) {}
        virtual void skip(const EdgeInfo&) {}
        virtual void node(const EdgeInfo&, const NodeInfo&) {
          (void) node_count.fetch_add(1U,std::memory_order_relaxed);
        }
        virtual void done(void) {
          nodes_at_done.store(node_count.load(std::memory_order_acquire),
                              std::memory_order_release);
          (void) done_count.fetch_add(1U,std::memory_order_release);
        }
      };

      static bool converged(const Gecode::Search::WorkerControl& control,
                            unsigned long long int generation,
                            unsigned int leases) {
        for (unsigned int spin=0U; spin<10000000U; spin++) {
          if ((Gecode::Search::WorkerControlAccess::
               observed_generation(control) == generation) &&
              (Gecode::Search::WorkerControlAccess::leases(control) ==
               leases))
            return true;
          std::this_thread::yield();
        }
        return false;
      }

      static bool settled(const Gecode::Search::WorkerControl& control,
                          unsigned long long int generation,
                          unsigned int leases) {
        for (unsigned int spin=0U; spin<10000000U; spin++) {
          if ((Gecode::Search::WorkerControlAccess::
               observed_generation(control) == generation) &&
              (Gecode::Search::WorkerControlAccess::leases(control) ==
               leases) &&
              (Gecode::Search::WorkerControlAccess::admitted(control) <=
               leases))
            return true;
          std::this_thread::yield();
        }
        return false;
      }

      template<class Exception, class Function>
      static bool throws(Function function) {
        try {
          function();
        } catch (const Exception&) {
          return true;
        } catch (...) {
          return false;
        }
        return false;
      }

      static unsigned int capacity(void) {
#ifdef GECODE_HAS_THREADS
        return 4U;
#else
        return 1U;
#endif
      }

      static Gecode::Search::Engine*
      dfs_engine(Gecode::Search::Options& o) {
        HasSolutions* m =
          new HasSolutions(HTB_BINARY,HTB_BINARY,HTB_BINARY);
        Gecode::Search::Engine* e;
        try {
          e = Gecode::Search::dfsengine(m,o);
        } catch (...) {
          delete m;
          throw;
        }
        delete m;
        return e;
      }

      static Gecode::Search::Engine*
      bab_resize_engine(Gecode::Search::Options& o) {
        BABResizeSpace* m = new BABResizeSpace;
        Gecode::Search::Engine* e;
        try {
          e = Gecode::Search::babengine(m,o);
        } catch (...) {
          delete m;
          throw;
        }
        delete m;
        return e;
      }

      static int bab_resize_best(Gecode::Search::Options o) {
        Gecode::Search::Engine* bab = bab_resize_engine(o);
        int best = 4096;
        while (BABResizeSpace* solution =
               static_cast<BABResizeSpace*>(bab->next())) {
          best = solution->value();
          delete solution;
        }
        bool ok = !bab->stopped();
        delete bab;
        return ok ? best : 4096;
      }

      static bool values(void) {
        Gecode::Search::WorkerControl empty;
        Gecode::Search::Options defaults;
        if (empty || (empty.requested() != 0U) ||
            (empty.capacity() != 0U) || defaults.worker_control)
          return false;

        Gecode::Search::WorkerControl control(3U);
        Gecode::Search::WorkerControl copy(control);
        Gecode::Search::WorkerControl assigned;
        assigned = copy;
        Gecode::Search::Options o;
        o.worker_control = assigned;
        Gecode::Search::Options copied_options(o);
        copy.request(2U);
        return control && (control.requested() == 2U) &&
          (assigned.requested() == 2U) &&
          (o.worker_control.requested() == 2U) &&
          (copied_options.worker_control.requested() == 2U) &&
          (control.capacity() == 0U);
      }

      static bool errors(void) {
        if (!throws<Gecode::Search::InvalidWorkerRequest>([] {
              Gecode::Search::WorkerControl invalid(0U);
              (void) invalid;
            }))
          return false;
        Gecode::Search::WorkerControl empty;
        if (!throws<Gecode::Search::UninitializedWorkerControl>(
              [&] { empty.request(1U); }))
          return false;
        Gecode::Search::WorkerControl unbound(1U);
        if (!throws<Gecode::Search::InvalidWorkerRequest>(
              [&] { unbound.request(0U); }))
          return false;

        Gecode::Search::Options too_many;
        too_many.threads = 1.0;
        too_many.worker_control =
          Gecode::Search::WorkerControl(2U);
        if (!throws<Gecode::Search::InvalidWorkerRequest>([&] {
              Gecode::Search::Engine* e = dfs_engine(too_many);
              delete e;
            }))
          return false;

        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = Gecode::Search::WorkerControl(1U);
        Gecode::Search::Engine* e = dfs_engine(o);
        bool exact = throws<Gecode::Search::InvalidWorkerRequest>(
          [&] { o.worker_control.request(capacity()+1U); });
        delete e;
        return exact;
      }

      static bool binding(void) {
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = Gecode::Search::WorkerControl(1U);
        Gecode::Search::Options copied(o);
        Gecode::Search::Engine* first = dfs_engine(o);
        bool ok = (o.threads == 4.0) && (copied.threads == 4.0) &&
          (o.worker_control.capacity() == capacity()) &&
          (copied.worker_control.capacity() == capacity());
        bool while_live = throws<Gecode::Search::WorkerControlInUse>([&] {
          Gecode::Search::Engine* e = dfs_engine(copied);
          delete e;
        });
        delete first;
        bool after_destroy = throws<Gecode::Search::WorkerControlInUse>([&] {
          Gecode::Search::Engine* e = dfs_engine(copied);
          delete e;
        });
        o.worker_control.request(1U);

        Gecode::Search::Options sequential_dfs;
        sequential_dfs.threads = 1.0;
        sequential_dfs.worker_control =
          Gecode::Search::WorkerControl(1U);
        Gecode::Search::Engine* dfs = dfs_engine(sequential_dfs);
        ok = ok && (sequential_dfs.worker_control.capacity() == 1U);
        delete dfs;

        Gecode::Search::Options sequential_bab;
        sequential_bab.threads = 1.0;
        sequential_bab.worker_control =
          Gecode::Search::WorkerControl(1U);
        HasSolutions* m = new HasSolutions(
          HTB_BINARY,HTB_BINARY,HTB_BINARY,HTC_LEX_LE);
        Gecode::Search::Engine* bab =
          Gecode::Search::babengine(m,sequential_bab);
        delete m;
        ok = ok && (sequential_bab.worker_control.capacity() == 1U);
        delete bab;

        return ok && while_live && after_destroy &&
          (copied.worker_control.requested() == 1U);
      }

      static bool concurrent(void) {
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = Gecode::Search::WorkerControl(1U);
        Gecode::Search::Engine* e = dfs_engine(o);
#ifdef GECODE_HAS_THREADS
        std::vector<std::thread> writers;
        for (unsigned int writer=0U; writer<4U; writer++) {
          Gecode::Search::WorkerControl copy(o.worker_control);
          writers.emplace_back([copy,writer] () mutable {
            for (unsigned int i=0U; i<2000U; i++)
              copy.request(1U + ((i+writer) % 4U));
          });
        }
        for (std::thread& writer : writers)
          writer.join();

        std::atomic<bool> started(false);
        Gecode::Search::WorkerControl copy(o.worker_control);
        std::thread during_destruction([copy,&started] () mutable {
          started.store(true,std::memory_order_release);
          for (unsigned int i=0U; i<10000U; i++)
            copy.request(1U + (i % 4U));
        });
        while (!started.load(std::memory_order_acquire)) {}
        delete e;
        during_destruction.join();
#else
        o.worker_control.request(1U);
        delete e;
#endif
        o.worker_control.request(1U);
        return (o.worker_control.capacity() == capacity()) &&
          (o.worker_control.requested() == 1U);
      }

      static int dfs_solutions(const Gecode::Search::WorkerControl& control) {
        HasSolutions* m =
          new HasSolutions(HTB_BINARY,HTB_BINARY,HTB_BINARY);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        Gecode::DFS<HasSolutions> dfs(m,o);
        delete m;
        int solutions = 0;
        while (HasSolutions* solution = dfs.next()) {
          solutions++;
          delete solution;
        }
        return solutions;
      }

      static bool bab_best(const Gecode::Search::WorkerControl& control) {
        HasSolutions* m = new HasSolutions(
          HTB_BINARY,HTB_BINARY,HTB_BINARY,HTC_LEX_LE);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        Gecode::BAB<HasSolutions> bab(m,o);
        delete m;
        HasSolutions* best = nullptr;
        while (HasSolutions* solution = bab.next()) {
          delete best;
          best = solution;
        }
        bool ok = (best != nullptr) && best->best();
        delete best;
        return ok;
      }

      static bool compatibility(void) {
        Gecode::Search::WorkerControl empty;
        Gecode::Search::WorkerControl dfs_control(capacity());
        Gecode::Search::WorkerControl bab_control(capacity());
        return (dfs_solutions(empty) == 8) &&
          (dfs_solutions(dfs_control) == 8) &&
          bab_best(empty) && bab_best(bab_control);
      }

      static bool resized_enumeration(void) {
#ifdef GECODE_HAS_THREADS
        Gecode::Search::WorkerControl control(4U);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        ResizeSpace* root = new ResizeSpace;
        Gecode::DFS<ResizeSpace> dfs(root,o);
        delete root;

        std::atomic<unsigned int> solutions(0U);
        std::atomic<bool> controller_ok(true);
        std::atomic<unsigned int> paused(0U);
        std::atomic<unsigned int> published(0U);
        std::atomic<unsigned int> completed(0U);
        std::atomic<bool> exhausted(false);
        const unsigned int limits[] = {1U,3U,1U,4U};
        const unsigned int milestones[] = {1U,64U,128U,192U};
        // The consumer processes no solution beyond a milestone until the
        // controller has observed convergence for that milestone's request.
        std::thread controller([&] {
          for (unsigned int i=0U; i<4U; i++) {
            while ((paused.load(std::memory_order_acquire) < i+1U) &&
                   !exhausted.load(std::memory_order_acquire))
              std::this_thread::yield();
            if (exhausted.load(std::memory_order_acquire)) {
              controller_ok.store(false,std::memory_order_release);
              published.store(4U,std::memory_order_release);
              completed.store(4U,std::memory_order_release);
              return;
            }
            try {
              control.request(limits[i]);
            } catch (...) {
              controller_ok.store(false,std::memory_order_release);
              published.store(4U,std::memory_order_release);
              completed.store(4U,std::memory_order_release);
              return;
            }
            unsigned long long int generation =
              Gecode::Search::WorkerControlAccess::generation(control);
            published.store(i+1U,std::memory_order_release);
            bool converged = false;
            for (unsigned int spin=0U; spin<10000000U; spin++) {
              if (exhausted.load(std::memory_order_acquire))
                break;
              if ((Gecode::Search::WorkerControlAccess::
                   observed_generation(control) == generation) &&
                  (Gecode::Search::WorkerControlAccess::leases(control) <=
                   limits[i]) &&
                  (Gecode::Search::WorkerControlAccess::parked(control) >=
                   4U-limits[i])) {
                converged = true;
                break;
              }
              std::this_thread::yield();
            }
            if (!converged) {
              controller_ok.store(false,std::memory_order_release);
              completed.store(4U,std::memory_order_release);
              return;
            }
            completed.store(i+1U,std::memory_order_release);
          }
        });

        std::vector<unsigned int> counts(1U << 12,0U);
        std::vector<ResizeSpace*> pending;
        unsigned int next_milestone = 0U;
        bool search_exhausted = false;
        while (!search_exhausted || !pending.empty()) {
          ResizeSpace* solution;
          if (pending.empty()) {
            solution = dfs.next();
            if (solution == nullptr) {
              search_exhausted = true;
              exhausted.store(true,std::memory_order_release);
              continue;
            }
          } else {
            solution = pending.back();
            pending.pop_back();
          }
          counts[solution->id()]++;
          delete solution;
          unsigned int n =
            solutions.fetch_add(1U,std::memory_order_release)+1U;
          if ((next_milestone < 4U) &&
              (n == milestones[next_milestone])) {
            unsigned int phase = next_milestone+1U;
            paused.store(next_milestone+1U,std::memory_order_release);
            while ((published.load(std::memory_order_acquire) < phase) &&
                   controller_ok.load(std::memory_order_acquire))
              std::this_thread::yield();
            while ((completed.load(std::memory_order_acquire) < phase) &&
                   controller_ok.load(std::memory_order_acquire)) {
              ResizeSpace* extra = dfs.next();
              if (extra == nullptr) {
                search_exhausted = true;
                exhausted.store(true,std::memory_order_release);
                break;
              }
              pending.push_back(extra);
            }
            next_milestone++;
          }
        }
        controller.join();
        if (!controller_ok.load(std::memory_order_acquire) ||
            (solutions.load(std::memory_order_acquire) != counts.size()))
          return false;
        for (unsigned int count : counts)
          if (count != 1U)
            return false;
#endif
        return true;
      }

      static bool resize_boundaries(void) {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControlAccess;
        Gecode::Search::WorkerControl control(4U);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        ResizeSpace* root = new ResizeSpace;
        Gecode::DFS<ResizeSpace> dfs(root,o);
        delete root;

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<bool> finish(false);
        std::atomic<unsigned int> solutions(0U);
        std::thread consumer([&] {
          while (!finish.load(std::memory_order_acquire)) {
            ResizeSpace* solution = dfs.next();
            if (solution == nullptr)
              return;
            delete solution;
            (void) solutions.fetch_add(1U,std::memory_order_release);
          }
        });

        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        if (WorkerControlAccess::admitted(control) != 0U) {
          WorkerControlAccess::gate_release_all(
            control,WorkerControlAccess::GATE_ADMISSION);
          finish.store(true,std::memory_order_release);
          consumer.join();
          return false;
        }

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        control.request(1U);
        unsigned long long int generation =
          WorkerControlAccess::generation(control);
        WorkerControlAccess::reset_max_admitted(control);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        bool ok = converged(control,generation,1U) &&
          (WorkerControlAccess::max_admitted(control) <= 1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        for (unsigned int spin=0U;
             (spin<10000000U) &&
             (WorkerControlAccess::max_admitted(control) == 0U); spin++)
          std::this_thread::yield();
        ok = ok && (WorkerControlAccess::max_admitted(control) == 1U);

        // The request is published and signals worker events while the
        // parked workers are still stopped immediately before event wait.
        control.request(3U);
        generation = WorkerControlAccess::generation(control);
        WorkerControlAccess::reset_max_admitted(control);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        ok = ok && converged(control,generation,3U) &&
          (WorkerControlAccess::max_admitted(control) <= 3U);

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        unsigned int leased[3];
        unsigned int n_leased = 0U;
        for (unsigned int i=0U; i<4U; i++)
          if (WorkerControlAccess::leased(control,i)) {
            if (n_leased < 3U)
              leased[n_leased] = i;
            n_leased++;
          }
        if (n_leased != 3U) {
          finish.store(true,std::memory_order_release);
          WorkerControlAccess::gate_release_all(
            control,WorkerControlAccess::GATE_ADMISSION);
          consumer.join();
          return false;
        }
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,2U);
        control.request(1U);
        generation = WorkerControlAccess::generation(control);
        WorkerControlAccess::reset_max_admitted(control);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,leased[0]);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,leased[1]);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        ok = ok && converged(control,generation,1U) &&
          (WorkerControlAccess::max_admitted(control) <= 1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,leased[2]);
        for (unsigned int spin=0U;
             (spin<10000000U) &&
             (WorkerControlAccess::max_admitted(control) == 0U); spin++)
          std::this_thread::yield();
        ok = ok && (WorkerControlAccess::max_admitted(control) == 1U);

        finish.store(true,std::memory_order_release);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        consumer.join();
        return ok && (solutions.load(std::memory_order_acquire) > 0U);
#else
        return true;
#endif
      }

      static bool resize_handoff(void) {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControlAccess;
        Gecode::Search::WorkerControl control(4U);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        SingleSolutionSpace* root = new SingleSolutionSpace;
        Gecode::DFS<SingleSolutionSpace> dfs(root,o);
        delete root;

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<SingleSolutionSpace*> result(nullptr);
        std::thread consumer([&] {
          result.store(dfs.next(),std::memory_order_release);
        });
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_FAILED_SCAN,3U,1U);
        // Let worker 3 declare its initially empty state idle. With no parked
        // target at capacity, its next action completes a failed steal scan.
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_FAILED_SCAN);

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        control.request(1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);

        bool exact_setup =
          WorkerControlAccess::owner(control,0U) &&
          WorkerControlAccess::worker_parked(control,0U) &&
          WorkerControlAccess::leased(control,3U);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);

        unsigned long long int handoffs =
          WorkerControlAccess::handoffs(control);
        bool before_handoff = exact_setup &&
          WorkerControlAccess::leased(control,3U) &&
          WorkerControlAccess::worker_parked(control,0U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_FAILED_SCAN,3U);
        consumer.join();

        SingleSolutionSpace* solution =
          result.load(std::memory_order_acquire);
        bool ok = before_handoff && (solution != nullptr) &&
          (WorkerControlAccess::handoffs(control) >= handoffs+1U) &&
          (WorkerControlAccess::last_solution_worker(control) == 0U);
        ok = ok &&
          (WorkerControlAccess::solution_handoff_from(control) == 3U) &&
          (WorkerControlAccess::solution_handoff_to(control) == 0U);
        delete solution;
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_FAILED_SCAN);
        return ok;
#else
        return true;
#endif
      }

      static bool resize_lifecycle(void) {
#ifdef GECODE_HAS_THREADS
        Gecode::Search::WorkerControl control(1U);
        Gecode::Search::NodeStop stop(10U);
        CountingTracer tracer;
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.nogoods_limit = 16U;
        o.stop = &stop;
        o.tracer = &tracer;
        o.worker_control = control;
        ResizeSpace* root = new ResizeSpace;
        Gecode::Search::Engine* dfs =
          Gecode::Search::dfsengine(root,o);
        delete root;

        using Gecode::Search::WorkerControlAccess;
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<ResizeSpace*> stopped_result(nullptr);
        std::thread stopped_next([&] {
          stopped_result.store(static_cast<ResizeSpace*>(dfs->next()),
                               std::memory_order_release);
        });
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        stopped_next.join();
        ResizeSpace* solution =
          stopped_result.load(std::memory_order_acquire);
        bool ok = (solution == nullptr) && dfs->stopped() &&
          (dfs->statistics().node > 0U) &&
          (WorkerControlAccess::parked(control) >= 3U);
        delete solution;
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        (void) dfs->nogoods();

        control.request(3U);
        root = new ResizeSpace;
        dfs->reset(root);
        control.request(1U);
        stop.limit(100000U);
        solution = static_cast<ResizeSpace*>(dfs->next());
        ok = ok && (solution != nullptr);
        delete solution;
        delete dfs;
        return ok &&
          (tracer.init_count.load(std::memory_order_acquire) == 1U) &&
          (tracer.node_count.load(std::memory_order_acquire) > 0U) &&
          (tracer.done_count.load(std::memory_order_acquire) == 1U) &&
          (tracer.nodes_at_done.load(std::memory_order_acquire) ==
           tracer.node_count.load(std::memory_order_acquire));
#else
        return true;
#endif
      }

      static bool bab_resize_optimality(bool frequent) {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControlAccess;
        Gecode::Search::Options baseline_options;
        baseline_options.threads = 4.0;
        if (frequent) {
          baseline_options.c_d = 1U;
          baseline_options.a_d = 1U;
        }
        int baseline = bab_resize_best(baseline_options);

        Gecode::Search::WorkerControl control(4U);
        Gecode::Search::Options o(baseline_options);
        o.worker_control = control;
        Gecode::Search::Engine* bab = bab_resize_engine(o);
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<int> best(4096);
        std::atomic<unsigned int> allowed(0U);
        std::atomic<unsigned int> completed(0U);
        std::atomic<unsigned long long int> phase_generation(0U);
        std::atomic<bool> complete(false);
        std::atomic<bool> consumer_ok(true);
        std::thread consumer([&] {
          for (unsigned int phase=0U; phase<4U; phase++) {
            while (allowed.load(std::memory_order_acquire) <= phase)
              std::this_thread::yield();
            do {
              BABResizeSpace* solution =
                static_cast<BABResizeSpace*>(bab->next());
              if (solution == nullptr) {
                consumer_ok.store(false,std::memory_order_release);
                completed.store(4U,std::memory_order_release);
                return;
              }
              best.store(solution->value(),std::memory_order_release);
              delete solution;
            } while (WorkerControlAccess::completed_generation(control) <
                     phase_generation.load(std::memory_order_acquire));
            while (WorkerControlAccess::admitted(control) != 0U)
              std::this_thread::yield();
            completed.store(phase+1U,std::memory_order_release);
          }
          while (BABResizeSpace* solution =
                 static_cast<BABResizeSpace*>(bab->next())) {
            best.store(solution->value(),std::memory_order_release);
            delete solution;
          }
          complete.store(true,std::memory_order_release);
        });

        allowed.store(1U,std::memory_order_release);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        control.request(1U);
        unsigned long long int generation =
          WorkerControlAccess::generation(control);
        phase_generation.store(generation,std::memory_order_release);
        WorkerControlAccess::reset_max_admitted(control,generation);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        bool ok = settled(control,generation,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        while (completed.load(std::memory_order_acquire) < 1U)
          std::this_thread::yield();
        ok = ok && consumer_ok.load(std::memory_order_acquire) &&
          (WorkerControlAccess::max_admitted(control) <= 1U);

        const unsigned int limits[] = {3U,1U,4U};
        for (unsigned int i=0U; i<3U; i++) {
          unsigned int limit = limits[i];
          control.request(limit);
          generation = WorkerControlAccess::generation(control);
          phase_generation.store(generation,std::memory_order_release);
          WorkerControlAccess::reset_max_admitted(control,generation);
          if (limit > 1U)
            WorkerControlAccess::gate_release_all(
              control,WorkerControlAccess::GATE_EVENT_WAIT);
          allowed.store(i+2U,std::memory_order_release);
          while (completed.load(std::memory_order_acquire) < i+2U)
            std::this_thread::yield();
          ok = ok && consumer_ok.load(std::memory_order_acquire) &&
            settled(control,generation,limit) &&
            (WorkerControlAccess::max_admitted(control) <= limit);
        }

        consumer.join();
        ok = ok && complete.load(std::memory_order_acquire) &&
          !bab->stopped() && (baseline == 0) &&
          (best.load(std::memory_order_acquire) == baseline);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        delete bab;
        return ok;
#else
        Gecode::Search::Options o;
        o.threads = 1.0;
        if (frequent) {
          o.c_d = 1U;
          o.a_d = 1U;
        }
        return bab_resize_best(o) == 0;
#endif
      }

      static bool bab_resize_incumbents(void) {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControlAccess;
        Gecode::Search::WorkerControl control(4U);
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.worker_control = control;
        Gecode::Search::Engine* bab = bab_resize_engine(o);

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<BABResizeSpace*> first(nullptr);
        std::atomic<bool> first_done(false);
        std::atomic<bool> proceed(false);
        std::atomic<bool> solutions_better(true);
        std::atomic<unsigned int> solutions(0U);
        std::atomic<int> best(4096);
        std::thread consumer([&] {
          BABResizeSpace* solution =
            static_cast<BABResizeSpace*>(bab->next());
          first.store(solution,std::memory_order_release);
          first_done.store(true,std::memory_order_release);
          while (!proceed.load(std::memory_order_acquire))
            std::this_thread::yield();
          while (solution != nullptr) {
            if (solution->value() >= 3000)
              solutions_better.store(false,std::memory_order_release);
            best.store(solution->value(),std::memory_order_release);
            (void) solutions.fetch_add(1U,std::memory_order_release);
            delete solution;
            solution = static_cast<BABResizeSpace*>(bab->next());
          }
        });

        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        control.request(1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);

        unsigned long long int before_external[4];
        for (unsigned int i=0U; i<4U; i++)
          before_external[i] =
            WorkerControlAccess::incumbent_deliveries(control,i);
        BABResizeSpace external(3000);
        (void) external.status();
        bab->constrain(external);
        bool ok = true;
        for (unsigned int i=0U; i<4U; i++)
          ok = ok &&
            (WorkerControlAccess::incumbent_deliveries(control,i) ==
             before_external[i]+1U);
        ok = ok && WorkerControlAccess::worker_parked(control,1U) &&
          WorkerControlAccess::worker_parked(control,2U) &&
          WorkerControlAccess::worker_parked(control,3U);

        unsigned long long int before_internal[4];
        for (unsigned int i=0U; i<4U; i++)
          before_internal[i] =
            WorkerControlAccess::incumbent_deliveries(control,i);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        while (!first_done.load(std::memory_order_acquire))
          std::this_thread::yield();
        BABResizeSpace* first_solution =
          first.load(std::memory_order_acquire);
        ok = ok && (first_solution != nullptr) &&
          (first_solution->value() < external.value());
        for (unsigned int i=0U; i<4U; i++)
          ok = ok &&
            (WorkerControlAccess::incumbent_deliveries(control,i) >=
             before_internal[i]+1U);

        // Exercise the equal-to-capacity transition from the exact parked
        // state above. The pending request is reconciled when next resumes.
        control.request(4U);
        unsigned long long int generation =
          WorkerControlAccess::generation(control);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        proceed.store(true,std::memory_order_release);
        ok = ok && converged(control,generation,4U) &&
          (control.requested() == control.capacity());

        consumer.join();
        ok = ok && !bab->stopped() &&
          solutions_better.load(std::memory_order_acquire) &&
          (solutions.load(std::memory_order_acquire) > 0U) &&
          (best.load(std::memory_order_acquire) == 0);
        delete bab;

        // Repeat external constrain while a resize generation is pending
        // and worker 0 remains stopped at the admission boundary.
        Gecode::Search::WorkerControl transition_control(4U);
        Gecode::Search::Options transition_options;
        transition_options.threads = 4.0;
        transition_options.worker_control = transition_control;
        bab = bab_resize_engine(transition_options);
        WorkerControlAccess::gate_install(
          transition_control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<bool> transition_ok(true);
        std::atomic<int> transition_best(4096);
        std::thread transition_consumer([&] {
          while (BABResizeSpace* solution =
                 static_cast<BABResizeSpace*>(bab->next())) {
            if (solution->value() >= 2000)
              transition_ok.store(false,std::memory_order_release);
            transition_best.store(solution->value(),
                                  std::memory_order_release);
            delete solution;
          }
        });
        WorkerControlAccess::gate_wait(
          transition_control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_install(
          transition_control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        transition_control.request(1U);
        WorkerControlAccess::gate_release(
          transition_control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          transition_control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_release(
          transition_control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          transition_control,WorkerControlAccess::GATE_EVENT_WAIT);

        unsigned long long int transition_before[4];
        for (unsigned int i=0U; i<4U; i++)
          transition_before[i] =
            WorkerControlAccess::incumbent_deliveries(
              transition_control,i);
        transition_control.request(3U);
        generation =
          WorkerControlAccess::generation(transition_control);
        BABResizeSpace transition_external(2000);
        (void) transition_external.status();
        bab->constrain(transition_external);
        ok = ok &&
          WorkerControlAccess::gate_waiting(
            transition_control,WorkerControlAccess::GATE_ADMISSION,0U) &&
          (WorkerControlAccess::observed_generation(transition_control) !=
           generation);
        for (unsigned int i=0U; i<4U; i++)
          ok = ok &&
            (WorkerControlAccess::incumbent_deliveries(
               transition_control,i) == transition_before[i]+1U);
        ok = ok &&
          WorkerControlAccess::worker_parked(transition_control,1U) &&
          WorkerControlAccess::worker_parked(transition_control,2U) &&
          WorkerControlAccess::worker_parked(transition_control,3U);

        // Worker 1 reconciles the pending generation before any original
        // active worker leaves admission.
        WorkerControlAccess::gate_release(
          transition_control,WorkerControlAccess::GATE_EVENT_WAIT,1U);
        ok = ok && settled(transition_control,generation,3U);
        WorkerControlAccess::gate_release_all(
          transition_control,WorkerControlAccess::GATE_EVENT_WAIT);
        WorkerControlAccess::gate_release_all(
          transition_control,WorkerControlAccess::GATE_ADMISSION);
        transition_consumer.join();
        ok = ok && transition_ok.load(std::memory_order_acquire) &&
          !bab->stopped() &&
          (transition_best.load(std::memory_order_acquire) == 0);
        delete bab;
        return ok;
#else
        Gecode::Search::WorkerControl control(1U);
        Gecode::Search::Options o;
        o.threads = 1.0;
        o.worker_control = control;
        Gecode::Search::Engine* bab = bab_resize_engine(o);
        BABResizeSpace external(3000);
        (void) external.status();
        bab->constrain(external);
        int best = 4096;
        while (BABResizeSpace* solution =
               static_cast<BABResizeSpace*>(bab->next())) {
          best = solution->value();
          delete solution;
        }
        bool ok = (control.capacity() == 1U) &&
          (control.requested() == 1U) && !bab->stopped() && (best == 0);
        delete bab;
        return ok;
#endif
      }

      static bool bab_resize_lifecycle(void) {
#ifdef GECODE_HAS_THREADS
        using Gecode::Search::WorkerControlAccess;
        Gecode::Search::WorkerControl control(capacity());
        Gecode::Search::NodeStop stop(0U);
        CountingTracer tracer;
        Gecode::Search::Options o;
        o.threads = 4.0;
        o.nogoods_limit = 16U;
        o.stop = &stop;
        o.tracer = &tracer;
        o.worker_control = control;
        Gecode::Search::Engine* bab = bab_resize_engine(o);

        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_ADMISSION,
          WorkerControlAccess::ALL_WORKERS,4U);
        std::atomic<BABResizeSpace*> stopped_result(nullptr);
        std::thread stopped_next([&] {
          stopped_result.store(
            static_cast<BABResizeSpace*>(bab->next()),
            std::memory_order_release);
        });
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_ADMISSION);
        WorkerControlAccess::gate_install(
          control,WorkerControlAccess::GATE_EVENT_WAIT,
          WorkerControlAccess::ALL_WORKERS,3U);
        control.request(1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,1U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,2U);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,3U);
        WorkerControlAccess::gate_wait(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        BABResizeSpace old_external(1000);
        (void) old_external.status();
        bab->constrain(old_external);
        bool ok = (WorkerControlAccess::parked(control) == 3U);
        WorkerControlAccess::gate_release_all(
          control,WorkerControlAccess::GATE_EVENT_WAIT);
        WorkerControlAccess::gate_release(
          control,WorkerControlAccess::GATE_ADMISSION,0U);
        stopped_next.join();
        BABResizeSpace* solution =
          stopped_result.load(std::memory_order_acquire);
        ok = ok && (solution == nullptr) && bab->stopped() &&
          (bab->statistics().node > 0U) &&
          (WorkerControlAccess::parked(control) >= 3U);
        delete solution;
        (void) bab->nogoods();
        ok = ok && (WorkerControlAccess::leases(control) == 1U) &&
          (WorkerControlAccess::parked(control) >= 3U);

        control.request(3U);
        unsigned long long int generation =
          WorkerControlAccess::generation(control);
        BABResizeSpace* root = new BABResizeSpace(2000,4095);
        bab->reset(root);
        ok = ok && settled(control,generation,3U);
        stop.limit(100000U);
        BABResizeSpace external(3000);
        (void) external.status();
        bab->constrain(external);
        int best = 4096;
        while ((solution = static_cast<BABResizeSpace*>(bab->next()))
               != nullptr) {
          best = solution->value();
          delete solution;
        }
        // Reaching 2000 proves reset discarded the prior bound of 1000.
        ok = ok && !bab->stopped() && (best == 2000) &&
          (control.requested() == 3U);
        delete bab;
        bool trace_ok =
          (tracer.init_count.load(std::memory_order_acquire) == 1U) &&
          (tracer.node_count.load(std::memory_order_acquire) > 0U) &&
          (tracer.done_count.load(std::memory_order_acquire) == 1U) &&
          (tracer.nodes_at_done.load(std::memory_order_acquire) ==
           tracer.node_count.load(std::memory_order_acquire));
        return ok && trace_ok;
#else
        Gecode::Search::WorkerControl control(1U);
        Gecode::Search::Options o;
        o.threads = 1.0;
        o.worker_control = control;
        Gecode::Search::Engine* bab = bab_resize_engine(o);
        control.request(1U);
        BABResizeSpace old_external(1000);
        (void) old_external.status();
        bab->constrain(old_external);
        BABResizeSpace* root = new BABResizeSpace(2000,4095);
        bab->reset(root);
        BABResizeSpace external(3000);
        (void) external.status();
        bab->constrain(external);
        BABResizeSpace* solution;
        int best = 4096;
        while ((solution = static_cast<BABResizeSpace*>(bab->next()))
               != nullptr) {
          best = solution->value();
          delete solution;
        }
        bool ok = !bab->stopped() && (best == 2000) &&
          (control.capacity() == 1U) && (control.requested() == 1U);
        delete bab;
        return ok;
#endif
      }

    public:
      WorkerControl(const std::string& name, Scenario s)
        : Base("Search::WorkerControl::"+name), scenario(s) {}

      bool run(void) override {
        switch (scenario) {
        case VALUES:        return values();
        case ERRORS:        return errors();
        case BINDING:       return binding();
        case CONCURRENT:    return concurrent();
        case COMPATIBILITY: return compatibility();
        case DFS_RESIZE_ENUMERATION: return resized_enumeration();
        case DFS_RESIZE_BOUNDARIES:  return resize_boundaries();
        case DFS_RESIZE_HANDOFF:     return resize_handoff();
        case DFS_RESIZE_LIFECYCLE:   return resize_lifecycle();
        case BAB_RESIZE_OPTIMALITY_DEFAULT:
          return bab_resize_optimality(false);
        case BAB_RESIZE_OPTIMALITY_FREQUENT:
          return bab_resize_optimality(true);
        case BAB_RESIZE_INCUMBENTS:  return bab_resize_incumbents();
        case BAB_RESIZE_LIFECYCLE:   return bab_resize_lifecycle();
        default:            GECODE_NEVER;
        }
        return false;
      }

      static void create(void) {
        (void) new WorkerControl("Values",VALUES);
        (void) new WorkerControl("Errors",ERRORS);
        (void) new WorkerControl("Binding",BINDING);
        (void) new WorkerControl("Concurrent",CONCURRENT);
        (void) new WorkerControl("Compatibility",COMPATIBILITY);
        (void) new WorkerControl("DFSResize::Enumeration",
                                 DFS_RESIZE_ENUMERATION);
        (void) new WorkerControl("DFSResize::Boundaries",
                                 DFS_RESIZE_BOUNDARIES);
        (void) new WorkerControl("DFSResize::Handoff",
                                 DFS_RESIZE_HANDOFF);
        (void) new WorkerControl("DFSResize::Lifecycle",
                                 DFS_RESIZE_LIFECYCLE);
        (void) new WorkerControl("BABResize::Optimality::Default",
                                 BAB_RESIZE_OPTIMALITY_DEFAULT);
        (void) new WorkerControl("BABResize::Optimality::Frequent",
                                 BAB_RESIZE_OPTIMALITY_FREQUENT);
        (void) new WorkerControl("BABResize::Incumbents",
                                 BAB_RESIZE_INCUMBENTS);
        (void) new WorkerControl("BABResize::Lifecycle",
                                 BAB_RESIZE_LIFECYCLE);
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
