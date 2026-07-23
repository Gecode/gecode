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
      /// Origin configuration function that does not restart
      bool origin(const MetaInfo& mi) override {
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
      bool origin(const MetaInfo& mi) override {
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
      /// Use the default completeness contract for each derived exploration
      bool variant(const MetaInfo&) override {
        return true;
      }
    };

    /// Observations shared by meta-search dispatch test spaces
    struct MetaDispatchState {
      std::atomic<unsigned int> origin_calls;
      std::atomic<unsigned int> variant_calls;
      std::atomic<unsigned int> origin_reasons;
      std::atomic<unsigned int> variant_reasons;
      std::atomic<unsigned int> assets;
      std::atomic<unsigned int> live_spaces;
      std::atomic<unsigned int> copies;
      std::atomic<unsigned int> constrain_calls;
      std::atomic<unsigned int> constrain_origin_calls;
      std::atomic<unsigned int> constrain_variant_calls;
      std::atomic<unsigned int> constrain_asset_mask;
      std::atomic<unsigned int> constrain_origin_asset_mask;
      std::atomic<unsigned int> constrain_variant_asset_mask;
      std::atomic<unsigned int> nogood_handoffs;
      bool restart_after_solution;
      bool solution_variant_complete;
      bool fail_solution_variant;
      bool nogood_model;

      /// Initialize without observations
      MetaDispatchState(void)
        : origin_calls(0), variant_calls(0),
          origin_reasons(0), variant_reasons(0), assets(0),
          live_spaces(0), copies(0), constrain_calls(0),
          constrain_origin_calls(0), constrain_variant_calls(0),
          constrain_asset_mask(0), constrain_origin_asset_mask(0),
          constrain_variant_asset_mask(0),
          nogood_handoffs(0),
          restart_after_solution(true),
          solution_variant_complete(true),
          fail_solution_variant(false), nogood_model(false) {}
    };

    /// Common implementation for meta-search dispatch test spaces
    class MetaDispatchSpace : public Space {
    protected:
      /// Variable used to provide two solutions
      IntVar x;
      /// Optional queens model used to force restart no-good extraction
      IntVarArray q;
      /// Shared hook observations and configuration
      MetaDispatchState* state;
      /// RBS role retained across cloning
      enum RestartRole {
        RESTART_ROLE_ORIGIN,
        RESTART_ROLE_VARIANT
      } restart_role;
      /// PBS asset identity, or -1 before portfolio dispatch
      int portfolio_asset;

      /// Record and implement an Origin callback
      bool observe_origin(const MetaInfo& mi) {
        state->origin_calls.fetch_add(1, std::memory_order_relaxed);
        if (mi.type() == MetaInfo::PORTFOLIO)
          // PBS must ignore the callback result.
          return false;
        restart_role = RESTART_ROLE_ORIGIN;
        state->origin_reasons.fetch_or(
          1U << static_cast<unsigned int>(mi.reason()),
          std::memory_order_relaxed);
        bool default_result = Space::origin(mi);
        if ((mi.type() == MetaInfo::RESTART) && (mi.nogoods().ng() > 0))
          state->nogood_handoffs.fetch_add(1,std::memory_order_relaxed);
        return (mi.reason() == MetaInfo::RR_SOL) ?
          state->restart_after_solution : default_result;
      }

      /// Record and implement a Variant callback
      bool observe_variant(const MetaInfo& mi) {
        state->variant_calls.fetch_add(1, std::memory_order_relaxed);
        if (mi.type() == MetaInfo::PORTFOLIO) {
          portfolio_asset = static_cast<int>(mi.asset());
          state->assets.fetch_or(1U << mi.asset(),
                                 std::memory_order_relaxed);
          // PBS must ignore the callback result.
          return false;
        }
        state->variant_reasons.fetch_or(
          1U << static_cast<unsigned int>(mi.reason()),
          std::memory_order_relaxed);
        restart_role = RESTART_ROLE_VARIANT;
        if ((mi.reason() == MetaInfo::RR_SOL) &&
            state->fail_solution_variant)
          fail();
        return (mi.reason() == MetaInfo::RR_SOL) ?
          state->solution_variant_complete : true;
      }

    public:
      /// Initialize with shared observations \a s
      MetaDispatchSpace(MetaDispatchState& s)
        : x(*this,0,1), q(*this,s.nogood_model ? 8 : 0,0,7), state(&s),
          restart_role(RESTART_ROLE_ORIGIN), portfolio_asset(-1) {
        state->live_spaces.fetch_add(1,std::memory_order_relaxed);
        if (state->nogood_model) {
          distinct(*this,IntArgs::create(q.size(),0,1),q,IPL_VAL);
          distinct(*this,IntArgs::create(q.size(),0,-1),q,IPL_VAL);
          distinct(*this,q,IPL_VAL);
          Gecode::branch(*this,q,INT_VAR_NONE(),INT_VAL_MIN());
        }
        Gecode::branch(*this,x,INT_VAL_MIN());
      }
      /// Clone \a s
      MetaDispatchSpace(MetaDispatchSpace& s)
        : Space(s), state(s.state), restart_role(s.restart_role),
          portfolio_asset(s.portfolio_asset) {
        state->live_spaces.fetch_add(1,std::memory_order_relaxed);
        state->copies.fetch_add(1,std::memory_order_relaxed);
        x.update(*this,s.x);
        q.update(*this,s.q);
      }
      /// Record destruction for clone-ownership checks
      ~MetaDispatchSpace(void) override {
        state->live_spaces.fetch_sub(1,std::memory_order_relaxed);
      }
      /// Exclude the preceding solution during an RBS restart
      void constrain(const Space& _s) override {
        state->constrain_calls.fetch_add(1,std::memory_order_relaxed);
        if (restart_role == RESTART_ROLE_ORIGIN)
          state->constrain_origin_calls.fetch_add(1,
                                                 std::memory_order_relaxed);
        else if (restart_role == RESTART_ROLE_VARIANT)
          state->constrain_variant_calls.fetch_add(1,
                                                  std::memory_order_relaxed);
        if (portfolio_asset >= 0) {
          const unsigned int asset = 1U <<
            static_cast<unsigned int>(portfolio_asset);
          state->constrain_asset_mask.fetch_or(asset,
                                               std::memory_order_relaxed);
          if (restart_role == RESTART_ROLE_ORIGIN)
            state->constrain_origin_asset_mask.fetch_or(
              asset,std::memory_order_relaxed);
          else if (restart_role == RESTART_ROLE_VARIANT)
            state->constrain_variant_asset_mask.fetch_or(
              asset,std::memory_order_relaxed);
        }
        const MetaDispatchSpace& s =
          static_cast<const MetaDispatchSpace&>(_s);
        rel(*this,x,IRT_NQ,s.x.val());
      }
      /// Return the solution value
      int value(void) const {
        return x.val();
      }
    };

    /// Dispatch test space overriding only Origin
    class OriginDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      OriginDispatchSpace(MetaDispatchState& s)
        : MetaDispatchSpace(s) {}
      /// Clone \a s
      OriginDispatchSpace(OriginDispatchSpace& s)
        : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space* copy(void) override {
        return new OriginDispatchSpace(*this);
      }
      /// Observe Origin dispatch
      bool origin(const MetaInfo& mi) override {
        return observe_origin(mi);
      }
    };

    /// Dispatch test space overriding only Variant
    class VariantDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      VariantDispatchSpace(MetaDispatchState& s)
        : MetaDispatchSpace(s) {}
      /// Clone \a s
      VariantDispatchSpace(VariantDispatchSpace& s)
        : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space* copy(void) override {
        return new VariantDispatchSpace(*this);
      }
      /// Observe Variant dispatch
      bool variant(const MetaInfo& mi) override {
        return observe_variant(mi);
      }
    };

    /// Dispatch test space overriding both Origin and Variant
    class OriginVariantDispatchSpace : public MetaDispatchSpace {
    public:
      /// Initialize with shared observations \a s
      OriginVariantDispatchSpace(MetaDispatchState& s)
        : MetaDispatchSpace(s) {}
      /// Clone \a s
      OriginVariantDispatchSpace(OriginVariantDispatchSpace& s)
        : MetaDispatchSpace(s) {}
      /// Copy during cloning
      Space* copy(void) override {
        return new OriginVariantDispatchSpace(*this);
      }
      /// Observe Origin dispatch
      bool origin(const MetaInfo& mi) override {
        return observe_origin(mi);
      }
      /// Observe Variant dispatch
      bool variant(const MetaInfo& mi) override {
        return observe_variant(mi);
      }
    };

    /// Focused meta-search hook dispatch test
    class MetaDispatch : public Base {
    public:
      /// Dispatch scenario
      enum Scenario {
        RBS_ORIGIN_ONLY,
        RBS_VARIANT_ONLY,
        RBS_ORIGIN_CONTINUE,
        RBS_ORIGIN_RESTART,
        RBS_VARIANT_COMPLETE,
        RBS_VARIANT_INCOMPLETE,
        PBS_ORIGIN_ONLY,
        PBS_VARIANT_ONLY,
        PBS_SINGLE,
        PBS_SEQUENTIAL_ORIGIN_ONLY,
        PBS_SEQUENTIAL_VARIANT_ONLY,
        PBS_SEQUENTIAL,
#ifdef GECODE_HAS_THREADS
        PBS_PARALLEL_ORIGIN_ONLY,
        PBS_PARALLEL_VARIANT_ONLY,
#endif
        PBS_PARALLEL,
        RBS_NOGOODS_RESET_OWNERSHIP,
        META_STOP_OWNERSHIP,
        BAB_INCUMBENT
      };
    protected:
      /// Scenario under test
      Scenario scenario;

      /// Return a bit for restart reason \a r
      static unsigned int reason(MetaInfo::RestartReason r) {
        return 1U << static_cast<unsigned int>(r);
      }

      /// Check independent callback dispatch for one RBS model
      template<class Model>
      bool rbs_dispatch(unsigned int origin_calls,
                        unsigned int variant_calls) {
        MetaDispatchState state;
        Model* m = new Model(state);
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<Model,Gecode::DFS> search(m,o);
        delete m;

        Model* first = search.next();
        Model* second = search.next();
        bool ok = (first != nullptr) && (second != nullptr) &&
          (state.origin_calls.load(std::memory_order_relaxed) ==
           origin_calls) &&
          (state.variant_calls.load(std::memory_order_relaxed) ==
           variant_calls);
        delete first;
        delete second;
        return ok;
      }

      /// Run one RBS dispatch scenario
      bool rbs(bool restart_after_solution,
               bool fail_solution_variant,
               bool solution_variant_complete) {
        MetaDispatchState state;
        state.restart_after_solution = restart_after_solution;
        state.fail_solution_variant = fail_solution_variant;
        state.solution_variant_complete = solution_variant_complete;

        OriginVariantDispatchSpace* m =
          new OriginVariantDispatchSpace(state);
        Gecode::Search::Options o;
        o.threads = 1;
        o.cutoff = Gecode::Search::Cutoff::constant(100);
        Gecode::RBS<OriginVariantDispatchSpace,Gecode::DFS> search(m,o);
        delete m;

        OriginVariantDispatchSpace* first = search.next();
        bool ok = first != nullptr;
        delete first;

        OriginVariantDispatchSpace* second = search.next();
        if (!fail_solution_variant) {
          ok = ok && (second != nullptr) &&
            (state.origin_calls.load(std::memory_order_relaxed) == 1U) &&
            (state.variant_calls.load(std::memory_order_relaxed) ==
             (restart_after_solution ? 2U : 1U)) &&
            (state.origin_reasons.load(std::memory_order_relaxed) ==
             reason(MetaInfo::RR_SOL)) &&
            (state.variant_reasons.load(std::memory_order_relaxed) ==
             (reason(MetaInfo::RR_INIT) |
              (restart_after_solution ? reason(MetaInfo::RR_SOL) : 0U)));
        } else if (solution_variant_complete) {
          ok = ok && (second == nullptr) &&
            (state.origin_calls.load(std::memory_order_relaxed) == 1U) &&
            (state.variant_calls.load(std::memory_order_relaxed) == 2U) &&
            (state.origin_reasons.load(std::memory_order_relaxed) ==
             reason(MetaInfo::RR_SOL)) &&
            (state.variant_reasons.load(std::memory_order_relaxed) ==
             (reason(MetaInfo::RR_INIT) | reason(MetaInfo::RR_SOL)));
        } else {
          ok = ok && (second != nullptr) &&
            (state.origin_calls.load(std::memory_order_relaxed) == 2U) &&
            (state.variant_calls.load(std::memory_order_relaxed) == 3U) &&
            (state.origin_reasons.load(std::memory_order_relaxed) ==
             (reason(MetaInfo::RR_SOL) | reason(MetaInfo::RR_CMPL))) &&
            (state.variant_reasons.load(std::memory_order_relaxed) ==
             (reason(MetaInfo::RR_INIT) | reason(MetaInfo::RR_SOL) |
              reason(MetaInfo::RR_CMPL)));
        }
        delete second;
        return ok;
      }

      /// Run one PBS dispatch scenario
      template<class Model>
      bool pbs(unsigned int assets, double threads,
               unsigned int origin_calls, unsigned int variant_calls,
               unsigned int asset_mask) {
        MetaDispatchState state;
        Model* m = new Model(state);
        Gecode::Search::Options o;
        o.assets = assets;
        o.threads = threads;
        Gecode::PBS<Model,Gecode::DFS> search(m,o);
        delete m;

        Model* solution = search.next();
        bool ok =
          (solution != nullptr) &&
          (state.origin_calls.load(std::memory_order_relaxed) ==
           origin_calls) &&
          (state.variant_calls.load(std::memory_order_relaxed) ==
           variant_calls) &&
          (state.assets.load(std::memory_order_relaxed) == asset_mask);
        delete solution;
        return ok;
      }

      /// Exercise RBS no-good posting, reset, and clone ownership
      bool rbs_nogoods_reset_ownership(void) {
        MetaDispatchState state;
        state.nogood_model = true;
        bool ok = true;
        {
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.nogoods_limit = 16;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::RBS<OriginVariantDispatchSpace,Gecode::DFS> search(m,o);
          delete m;

          unsigned int solutions = 0;
          while (OriginVariantDispatchSpace* solution = search.next()) {
            solutions++;
            delete solution;
          }
          ok = (solutions == 2U) &&
            (state.variant_calls.load(std::memory_order_relaxed) >= 2U) &&
            (state.constrain_calls.load(std::memory_order_relaxed) >= 1U) &&
            (state.nogood_handoffs.load(std::memory_order_relaxed) >= 1U);
        }
        return ok &&
          (state.live_spaces.load(std::memory_order_relaxed) == 0U) &&
          (state.copies.load(std::memory_order_relaxed) > 0U);
      }

      /// Stop RBS and PBS, then verify orderly clone destruction
      bool meta_stop_ownership(void) {
        MetaDispatchState rbs_state;
        bool ok = true;
        {
          Gecode::Search::NodeStop stop(0);
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(rbs_state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.stop = &stop;
          o.cutoff = Gecode::Search::Cutoff::constant(1);
          Gecode::RBS<OriginVariantDispatchSpace,Gecode::DFS> search(m,o);
          delete m;
          OriginVariantDispatchSpace* solution = search.next();
          ok = (solution == nullptr) && search.stopped();
          delete solution;
        }
        ok = ok &&
          (rbs_state.live_spaces.load(std::memory_order_relaxed) == 0U);

        MetaDispatchState pbs_state;
        {
          Gecode::Search::NodeStop stop(0);
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(pbs_state);
          Gecode::Search::Options o;
          o.assets = 3;
#ifdef GECODE_HAS_THREADS
          o.threads = 3;
#else
          o.threads = 1;
#endif
          o.stop = &stop;
          Gecode::PBS<OriginVariantDispatchSpace,Gecode::DFS> search(m,o);
          delete m;
          OriginVariantDispatchSpace* solution = search.next();
          ok = ok && (solution == nullptr) && search.stopped();
          delete solution;
        }
        return ok &&
          (pbs_state.live_spaces.load(std::memory_order_relaxed) == 0U);
      }

      /// Exercise incumbent propagation through RBS and PBS wrappers
      bool bab_incumbent(void) {
        MetaDispatchState rbs_state;
        bool ok = true;
        {
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(rbs_state);
          Gecode::Search::Options o;
          o.threads = 1;
          o.cutoff = Gecode::Search::Cutoff::constant(100);
          Gecode::SEB builder =
            Gecode::rbs<OriginVariantDispatchSpace,Gecode::BAB>(o);
          Gecode::Search::Engine* search = (*builder)(m);
          delete builder;
          delete m;
          OriginVariantDispatchSpace* best = nullptr;
          while (OriginVariantDispatchSpace* solution =
                   static_cast<OriginVariantDispatchSpace*>(search->next())) {
            search->constrain(*solution);
            delete best;
            best = solution;
          }
          ok = (best != nullptr) && (best->value() == 1) &&
            (rbs_state.constrain_origin_calls.load(
              std::memory_order_relaxed) > 0U) &&
            (rbs_state.constrain_variant_calls.load(
              std::memory_order_relaxed) > 0U);
          delete best;
          delete search;
        }
        ok = ok &&
          (rbs_state.live_spaces.load(std::memory_order_relaxed) == 0U);

        MetaDispatchState pbs_state;
        {
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(pbs_state);
          Gecode::Search::Options o;
          o.assets = 3;
          o.threads = 1;
          Gecode::PBS<OriginVariantDispatchSpace,Gecode::BAB> search(m,o);
          delete m;
          OriginVariantDispatchSpace* best = nullptr;
          while (OriginVariantDispatchSpace* solution = search.next()) {
            delete best;
            best = solution;
          }
          ok = ok && (best != nullptr) && (best->value() == 1) &&
            (pbs_state.constrain_asset_mask.load(
              std::memory_order_relaxed) == 7U);
          delete best;
        }
        ok = ok &&
          (pbs_state.live_spaces.load(std::memory_order_relaxed) == 0U);

        MetaDispatchState nested_state;
        {
          OriginVariantDispatchSpace* m =
            new OriginVariantDispatchSpace(nested_state);
          Gecode::Search::Options outer;
          outer.assets = 3;
          outer.threads = 1;
          Gecode::Search::Options inner[3];
          for (unsigned int i=0; i<3; i++) {
            inner[i].threads = 1;
            // Each RBS engine owns and deletes its cutoff.
            inner[i].cutoff = Gecode::Search::Cutoff::constant(100);
          }
          Gecode::SEBs variants(3);
          variants[0] =
            Gecode::rbs<OriginVariantDispatchSpace,Gecode::BAB>(inner[0]);
          variants[1] =
            Gecode::rbs<OriginVariantDispatchSpace,Gecode::BAB>(inner[1]);
          variants[2] =
            Gecode::rbs<OriginVariantDispatchSpace,Gecode::BAB>(inner[2]);
          Gecode::PBS<OriginVariantDispatchSpace,Gecode::BAB>
            search(m,variants,outer);
          delete m;
          OriginVariantDispatchSpace* best = nullptr;
          while (OriginVariantDispatchSpace* solution = search.next()) {
            delete best;
            best = solution;
          }
          ok = ok && (best != nullptr) && (best->value() == 1) &&
            (nested_state.constrain_origin_asset_mask.load(
              std::memory_order_relaxed) == 7U) &&
            (nested_state.constrain_variant_asset_mask.load(
              std::memory_order_relaxed) == 7U);
          delete best;
        }
        return ok &&
          (nested_state.live_spaces.load(std::memory_order_relaxed) == 0U);
      }

    public:
      /// Initialize scenario \a s with name \a n
      MetaDispatch(const std::string& n, Scenario s)
        : Base("Search::MetaDispatch::"+n), scenario(s) {}
      /// Run test
      bool run(void) override {
        switch (scenario) {
        case RBS_ORIGIN_ONLY:
          return rbs_dispatch<OriginDispatchSpace>(1,0);
        case RBS_VARIANT_ONLY:
          return rbs_dispatch<VariantDispatchSpace>(0,2);
        case RBS_ORIGIN_CONTINUE:
          return rbs(false,false,true);
        case RBS_ORIGIN_RESTART:
          return rbs(true,false,true);
        case RBS_VARIANT_COMPLETE:
          return rbs(true,true,true);
        case RBS_VARIANT_INCOMPLETE:
          return rbs(true,true,false);
        case PBS_ORIGIN_ONLY:
          return pbs<OriginDispatchSpace>(1,1,1,0,0);
        case PBS_VARIANT_ONLY:
          return pbs<VariantDispatchSpace>(1,1,0,1,1);
        case PBS_SINGLE:
          return pbs<OriginVariantDispatchSpace>(1,1,1,1,1);
        case PBS_SEQUENTIAL_ORIGIN_ONLY:
          return pbs<OriginDispatchSpace>(3,1,1,0,0);
        case PBS_SEQUENTIAL_VARIANT_ONLY:
          return pbs<VariantDispatchSpace>(3,1,0,3,7);
        case PBS_SEQUENTIAL:
          return pbs<OriginVariantDispatchSpace>(3,1,1,3,7);
#ifdef GECODE_HAS_THREADS
        case PBS_PARALLEL_ORIGIN_ONLY:
          return pbs<OriginDispatchSpace>(3,3,1,0,0);
        case PBS_PARALLEL_VARIANT_ONLY:
          return pbs<VariantDispatchSpace>(3,3,0,3,7);
#endif
        case PBS_PARALLEL:
          return pbs<OriginVariantDispatchSpace>(3,3,1,3,7);
        case RBS_NOGOODS_RESET_OWNERSHIP:
          return rbs_nogoods_reset_ownership();
        case META_STOP_OWNERSHIP:
          return meta_stop_ownership();
        case BAB_INCUMBENT:
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
      /// Number of portfolio threads
      unsigned int mt;
      /// Number of variant threads
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
        (void) new MetaDispatch("RBS::OriginOnly",
                                MetaDispatch::RBS_ORIGIN_ONLY);
        (void) new MetaDispatch("RBS::VariantOnly",
                                MetaDispatch::RBS_VARIANT_ONLY);
        (void) new MetaDispatch("RBS::OriginContinue",
                                MetaDispatch::RBS_ORIGIN_CONTINUE);
        (void) new MetaDispatch("RBS::OriginRestart",
                                MetaDispatch::RBS_ORIGIN_RESTART);
        (void) new MetaDispatch("RBS::VariantComplete",
                                MetaDispatch::RBS_VARIANT_COMPLETE);
        (void) new MetaDispatch("RBS::VariantIncomplete",
                                MetaDispatch::RBS_VARIANT_INCOMPLETE);
        (void) new MetaDispatch("RBS::NoGoodsResetOwnership",
                                MetaDispatch::RBS_NOGOODS_RESET_OWNERSHIP);
        (void) new MetaDispatch("Lifecycle::StopOwnership",
                                MetaDispatch::META_STOP_OWNERSHIP);
        (void) new MetaDispatch("BAB::Incumbent",
                                MetaDispatch::BAB_INCUMBENT);
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
        (void) new MetaDispatch("PBS::OriginOnly",
                                MetaDispatch::PBS_ORIGIN_ONLY);
        (void) new MetaDispatch("PBS::VariantOnly",
                                MetaDispatch::PBS_VARIANT_ONLY);
        (void) new MetaDispatch("PBS::Single",
                                MetaDispatch::PBS_SINGLE);
        (void) new MetaDispatch("PBS::SequentialOriginOnly",
                                MetaDispatch::PBS_SEQUENTIAL_ORIGIN_ONLY);
        (void) new MetaDispatch("PBS::SequentialVariantOnly",
                                MetaDispatch::PBS_SEQUENTIAL_VARIANT_ONLY);
        (void) new MetaDispatch("PBS::Sequential",
                                MetaDispatch::PBS_SEQUENTIAL);
#ifdef GECODE_HAS_THREADS
        (void) new MetaDispatch("PBS::ParallelOriginOnly",
                                MetaDispatch::PBS_PARALLEL_ORIGIN_ONLY);
        (void) new MetaDispatch("PBS::ParallelVariantOnly",
                                MetaDispatch::PBS_PARALLEL_VARIANT_ONLY);
        (void) new MetaDispatch("PBS::Parallel",
                                MetaDispatch::PBS_PARALLEL);
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
