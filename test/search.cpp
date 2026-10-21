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
#ifdef GECODE_HAS_EXPERIMENTAL
#include "examples/tsptw-transition.hh"
#endif

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <limits>
#include <stdexcept>
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

#ifdef GECODE_HAS_EXPERIMENTAL
    /// Successful preparation can end in failure or skip to a solved brancher.
    class PreparedTerminal : public Base {
      class PreparedSpace : public Space {
        class PreparedBrancher : public Brancher {
          bool prepared, failure;
          std::shared_ptr<std::atomic<unsigned int>> choices;
          class PreparedChoice : public Choice {
          public:
            PreparedChoice(const Brancher& b) : Choice(b,1) {}
          };
        public:
          PreparedBrancher(Home home, bool f, std::shared_ptr<std::atomic<unsigned int>> c)
            : Brancher(home), prepared(false), failure(f), choices(std::move(c)) {
            home.notice(*this,AP_DISPOSE,false);
          }
          PreparedBrancher(Space& home, PreparedBrancher& b)
            : Brancher(home,b), prepared(b.prepared), failure(b.failure), choices(b.choices) {}
          bool status(const Space&) const override { return !prepared; }
          void prepare(Space& home, StatusStatistics&) override {
            prepared=true;
            if (failure) home.fail();
          }
          const Choice* choice(Space&) override {
            (*choices)++;
            return new PreparedChoice(*this);
          }
          const Choice* choice(const Space&, Archive&) override { return new PreparedChoice(*this); }
          ExecStatus commit(Space&, const Choice&, unsigned int) override { return ES_OK; }
          Actor* copy(Space& home) override { return new (home) PreparedBrancher(home,*this); }
          size_t dispose(Space& home) override {
            home.ignore(*this,AP_DISPOSE,false);
            choices.~shared_ptr();
            (void) Brancher::dispose(home);
            return sizeof(*this);
          }
        };
      public:
        PreparedSpace(bool failure, std::shared_ptr<std::atomic<unsigned int>> choices) {
          (void) new (*this) PreparedBrancher(*this,false,choices);
          (void) new (*this) PreparedBrancher(*this,failure,choices);
        }
        PreparedSpace(PreparedSpace& s) : Space(s) {}
        Space* copy(void) override { return new PreparedSpace(*this); }
      };
    public:
      PreparedTerminal(void) : Base("Search::PreparedTerminal",TestTags::check()) {}
      bool run(void) override {
        for (bool failure : {false,true})
          for (unsigned int kind=0; kind<5; kind++) {
            auto choices=std::make_shared<std::atomic<unsigned int>>(0);
            Gecode::Search::NodeStop stop(100);
            Gecode::Search::Options options; options.clone=false; options.stop=&stop;
            std::unique_ptr<Gecode::Search::Engine> engine;
            if (kind==0) engine.reset(Gecode::Search::dfsengine(new PreparedSpace(failure,choices),options));
            else if (kind==1) engine.reset(Gecode::Search::ldsengine(new PreparedSpace(failure,choices),options));
            else if (kind==2) {
              options.threads=2;
              engine.reset(Gecode::Search::babengine(new PreparedSpace(failure,choices),options));
            }
            else {
              using Guidance=Gecode::Search::HeuristicGuidance;
              Guidance guidance(Guidance::MINIMIZE,[](const Space&) -> std::int64_t { return 0; });
              if (kind==3) engine.reset(Gecode::Search::astarengine(new PreparedSpace(failure,choices),guidance,options));
              else engine.reset(Gecode::Search::cabsengine(new PreparedSpace(failure,choices),guidance,options));
            }
            std::unique_ptr<Space> result(engine->next());
            auto stat=engine->statistics();
            if (static_cast<bool>(result)==failure || choices->load() || engine->stopped() ||
                stat.node!=1 || stat.fail!=(failure ? 1U : 0U) || stat.depth) return false;
          }
        return true;
      }
    } preparedTerminal;


    /// The example's real n-ary transitions survive cloning and continuation.
    class TSPTWTransitionTest : public Base {
      using Transition=TSPTW::Transition;
      static std::shared_ptr<const TSPTW::Instance> instance(void) {
        std::istringstream input(
          "4\n0 1 2 3\n1 0 1 4\n1 3 0 1\n20 1 3 0\n"
          "0 100\n5 100\n0 100\n0 100\n");
        return std::make_shared<const TSPTW::Instance>(TSPTW::read(input));
      }
      static bool visit(Transition& s, int node) {
        Gecode::Search::Statistics stat;
        if (s.status()!=SS_BRANCH) return false;
        s.prepare(stat);
        const auto actions=s.actions();
        const auto action=std::find(actions.begin(),actions.end(),node);
        if (action==actions.end()) return false;
        std::unique_ptr<const Choice> choice(s.choice());
        s.commit(*choice,static_cast<unsigned int>(action-actions.begin()));
        return !s.failed();
      }
      static bool replay(Transition::ResidualPropagation propagation=Transition::OFF) {
        const auto data=instance();
        Transition root(data,propagation);
        Gecode::Search::Statistics stat;
        if (root.status()!=SS_BRANCH || root.allocated_positions()!=0)
          return false;
        std::unique_ptr<Transition> absent(static_cast<Transition*>(root.clone()));
        root.prepare(stat);
        if (root.allocated_positions()!=1 ||
            root.position(0).assigned() || root.depth || root.location ||
            root.time || root.cost() || root.unvisited!=std::vector<int>({1,2,3}))
          return false;
        std::unique_ptr<const Choice> original(root.choice());
        Archive archive; original->archive(archive);
        std::unique_ptr<const Choice> restored(root.choice(archive));
        if (restored->alternatives()!=3) return false;
        absent->commit(*restored,0);
        if (absent->status()!=SS_BRANCH || absent->depth!=1 || absent->location!=1 ||
            absent->time!=5 || absent->cost()!=1 || absent->returned ||
            absent->unvisited!=std::vector<int>({2,3}) ||
            absent->position(0).val()!=1 || root.position(0).assigned())
          return false;
        std::unique_ptr<Transition> next(static_cast<Transition*>(absent->clone()));
        absent->prepare(stat);
        if (absent->depth!=1 || absent->cost()!=1 ||
            absent->allocated_positions()!=2 || absent->position(1).assigned())
          return false;
        original.reset(absent->choice());
        Archive prefix_archive; original->archive(prefix_archive);
        restored.reset(absent->choice(prefix_archive));
        next->commit(*restored,0);
        if (next->status()!=SS_BRANCH || next->depth!=2 || next->cost()!=2 ||
            next->time!=6 || next->position(1).val()!=2 || absent->position(1).assigned())
          return false;

        Transition optimum(data,propagation);
        for (int node : {2,3,1,0}) if (!visit(optimum,node)) return false;
        if (optimum.status()!=SS_SOLVED || optimum.cost()!=5) return false;
        std::unique_ptr<Transition> cut(static_cast<Transition*>(absent->clone()));
        cut->constrain(optimum);
        if (cut->status()!=SS_BRANCH || cut->actions()!=std::vector<int>({2})) return false;
        // The original second alternative still means customer 3 after a cut
        // shrinks the fresh choice; it cannot silently become another action.
        cut->commit(*restored,1);
        if (cut->status()!=SS_FAILED) return false;
        if (!visit(*next,3) || next->returned || next->cost()!=3 ||
            next->status()!=SS_BRANCH || next->actions()!=std::vector<int>({0}) ||
            !visit(*next,0)) return false;
        return next->status()==SS_SOLVED && next->returned && next->depth==4 &&
          next->cost()==23 && next->time==27 && next->allocated_positions()==4 &&
          next->tour()==std::vector<int>({1,2,3}) && root.depth==0 && absent->depth==1;
      }
      static bool continuation(int kind) {
        const auto data=instance();
        Transition root(data), incumbent(data);
        for (int node : {3,1,2,0}) if (!visit(incumbent,node)) return false;
        if (incumbent.status()!=SS_SOLVED || incumbent.cost()!=6) return false;
        Gecode::Search::NodeStop stop(10000);
        Gecode::Search::Options options;
        options.threads=1; options.stop=&stop;
        // Force BAB to replay edges between stored prefix clones.
        options.c_d=5; options.a_d=2;
        Gecode::Search::WorkerControl control(1U);
#ifdef GECODE_HAS_THREADS
        options.worker_control=control;
#endif
        std::unique_ptr<Gecode::Search::Engine> engine(kind==0 ?
          Gecode::Search::babengine(&root,options) : kind==1 ?
          Gecode::Search::astarengine(&root,Transition::guidance(),options) :
          Gecode::Search::cabsengine(&root,Transition::guidance(),options));
        std::unique_ptr<Transition> first(static_cast<Transition*>(engine->next()));
        if (!first || first->cost()!=23 || first->tour()!=std::vector<int>({1,2,3}))
          return false;
        const auto before=engine->statistics().node;
        stop.limit(before-1);
        if (engine->next() || !engine->stopped() || engine->statistics().node!=before)
          return false;
        stop.limit(10000);
        engine->constrain(incumbent);
        std::unique_ptr<Transition> better;
#ifdef GECODE_HAS_THREADS
        control.request(0U);
        std::atomic<bool> entered(false), done(false);
        std::thread caller([&] {
          entered.store(true,std::memory_order_release);
          better.reset(static_cast<Transition*>(engine->next()));
          done.store(true,std::memory_order_release);
        });
        while (!entered.load(std::memory_order_acquire)) std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        const bool parked=!done.load(std::memory_order_acquire) &&
          engine->statistics().node==before;
        control.request(1U);
        caller.join();
        if (!parked) return false;
#else
        better.reset(static_cast<Transition*>(engine->next()));
#endif
        return better && better->cost()==5 && better->tour()==std::vector<int>({2,3,1}) &&
          !engine->next() && !engine->stopped() && engine->statistics().depth==4;
      }
      struct Suffix {
        std::int64_t cost;
        int first;
      };
      // Numeric permutations use no bound, transition applicability or CP model.
      static std::vector<Suffix> completions(const Transition& state) {
        if (state.returned) return {{state.prefix_cost,-1}};
        std::vector<Suffix> result;
        auto suffix=state.unvisited;
        do {
          std::int64_t time=state.time, cost=state.prefix_cost;
          int location=state.location;
          bool feasible=true;
          for (std::size_t p=0; p<=suffix.size(); p++) {
            const int node=p==suffix.size() ? 0 : suffix[p];
            const int arc=state.data->arc(location,node);
            time=std::max<std::int64_t>(time+arc,state.data->ready[node]);
            cost+=arc;
            if (time>state.data->due[node]) { feasible=false; break; }
            location=node;
          }
          if (feasible) result.push_back({cost,suffix.empty() ? 0 : suffix.front()});
        } while (std::next_permutation(suffix.begin(),suffix.end()));
        return result;
      }
      static bool residuals(Transition& state, const std::vector<int>& prefix,
                            bool& filtered, bool& rejected) {
        const auto suffixes=completions(state);
        const auto bound=state.bound(Transition::MIN_ARCS);
        if (bound<state.prefix_cost) return false;
        if (state.returned && bound!=state.prefix_cost) return false;
        if (!state.returned && state.unvisited.empty() &&
            bound!=state.prefix_cost+state.data->arc(state.location,0)) return false;
        for (const auto& suffix : suffixes) if (bound>suffix.cost) return false;

        for (auto propagation : {Transition::CHEAP,Transition::UNARY}) {
          Transition inferred(state.data,propagation);
          bool survived=inferred.status()!=SS_FAILED;
          for (int node : prefix) {
            if (!survived) break;
            survived=visit(inferred,node);
          }
          if (!survived) {
            if (propagation==Transition::CHEAP) rejected=true;
            if (!suffixes.empty()) return false;
          } else {
            if (inferred.unvisited!=state.unvisited || inferred.location!=state.location ||
                inferred.time!=state.time || inferred.cost()!=state.cost()) return false;
            const auto actions=inferred.actions();
            if (actions.size()<state.actions().size()) filtered=true;
            for (const auto& suffix : suffixes)
              if (suffix.first!=-1 &&
                  std::find(actions.begin(),actions.end(),suffix.first)==actions.end())
                return false;
          }
        }
        if (state.status()!=SS_BRANCH) return true;
        Gecode::Search::Statistics stat;
        state.prepare(stat);
        const auto actions=state.actions();
        std::unique_ptr<const Choice> choice(state.choice());
        for (std::size_t a=0; a<actions.size(); a++) {
          std::unique_ptr<Transition> child(static_cast<Transition*>(state.clone()));
          child->commit(*choice,static_cast<unsigned int>(a));
          auto next=prefix;
          next.push_back(actions[a]);
          // Even a failed committed residual state must have no feasible suffix.
          if (!residuals(*child,next,filtered,rejected)) return false;
        }
        return true;
      }
      static bool dominance(std::shared_ptr<const TSPTW::Instance> data) {
        Transition reference(data);
        const auto suffixes=completions(reference);
        std::int64_t optimum=std::numeric_limits<std::int64_t>::max();
        for (const auto& suffix : suffixes) optimum=std::min(optimum,suffix.cost);
        for (auto propagation : {Transition::OFF,Transition::CHEAP,Transition::UNARY}) {
          if (data->n==5) {
            auto label=[&](std::initializer_list<int> prefix) {
              auto state=std::make_unique<Transition>(data,propagation);
              for (int node : prefix)
                if (!visit(*state,node)) return std::unique_ptr<Transition>{};
              return state;
            };
            auto a=label({1,4,2}), b=label({4,1,2});
            // The earlier arrival costs more: neither label subsumes the other.
            if (!a || !b || a->time!=21 || a->cost()!=9 ||
                b->time!=20 || b->cost()!=11 || Transition::dominates(*a,*b) ||
                Transition::dominates(*b,*a)) return false;
            a=label({1,3,2}); b=label({3,1,2});
            if (!a || !b || !Transition::dominates(*a,*a) ||
                !Transition::dominates(*a,*b) || Transition::dominates(*b,*a) ||
                Transition::dominates(*a,reference) ||
                Transition::dominance_key(*a)!=Transition::dominance_key(*b))
              return false;
          }
          for (auto bound : {Transition::PREFIX,Transition::MIN_ARCS})
            for (bool enabled : {false,true}) {
              Transition root(data,propagation);
              Gecode::Search::CABSOptions options;
              if (enabled) {
                options.dominates=Transition::dominates;
                options.dominance_key=Transition::dominance_key;
              }
              Gecode::CABS<Transition> engine(&root,Transition::guidance(bound),
                Gecode::Search::Options::def,options);
              std::int64_t best=std::numeric_limits<std::int64_t>::max();
              while (std::unique_ptr<Transition> solution{engine.next()})
                best=solution->cost();
              if (engine.stopped() || best!=optimum) return false;
            }
        }
        return true;
      }
      static bool bounds_and_inference(void) {
        bool filtered=false, rejected=false;
        // Waiting/nonmetric and unequal/incomparable (t,g) labels; tight unary
        // filtering and failure; zero-duration intervals; pending/finished return.
        for (const std::string& text : {
          std::string("4\n0 1 2 3\n1 0 1 4\n1 3 0 1\n20 1 3 0\n"
                      "0 100\n5 100\n0 100\n0 100\n"),
          std::string("5\n0 3 1 7 4\n4 0 5 1 3\n2 1 0 4 6\n"
                      "3 4 2 0 1\n1 2 3 5 0\n"
                      "0 100\n15 100\n0 100\n0 100\n5 100\n"),
          std::string("3\n0 2 9\n9 0 2\n1 9 0\n0 6\n5 5\n0 7\n"),
          // Direct 0->2 misses due 6 and, after waiting, direct 1->0
          // misses depot due 8; visiting 1,2 still returns at 8, cost 5.
          std::string("3\n0 2 9\n4 0 1\n2 8 0\n0 8\n5 10\n0 6\n"),
          std::string("3\n0 0 0\n3 0 3\n3 3 0\n0 10\n0 0\n0 4\n"),
          std::string("3\n0 0 0\n3 0 3\n3 3 0\n0 20\n0 2\n0 2\n"),
          std::string("3\n0 0 0\n0 0 0\n0 0 0\n0 0\n0 0\n0 0\n")}) {
          std::istringstream input(text);
          auto data=std::make_shared<const TSPTW::Instance>(TSPTW::read(input));
          Transition root(data);
          if (!residuals(root,{},filtered,rejected) || !dominance(data)) return false;
        }
        return filtered && rejected;
      }
    public:
      TSPTWTransitionTest(void) : Base("Search::TSPTW::Transition", TestTags::check()) {}
      bool run(void) override {
        if (!replay() || !replay(Transition::CHEAP) || !replay(Transition::UNARY))
          return false;
        for (int kind : {0,1,2})
          if (!continuation(kind)) return false;
        return bounds_and_inference();
      }
    } tsptwTransitionTest;

    /// A tiny closed tree with a generated goal and deeper alternatives.
    class AStarSpace : public Space {
    public:
      static std::atomic<int> live;
      IntVar x, y;
      bool maximize;
      AStarSpace(bool max=false, int fixed=-1, bool failed=false)
        : x(*this,0,3), y(*this,0,1), maximize(max) {
        live++;
        TupleSet tuples(2);
        for (int i=0;i<4;i++) {
          tuples.add(IntArgs({i,0}));
          if (i) tuples.add(IntArgs({i,1}));
        }
        tuples.finalize();
        extensional(*this,IntVarArgs({x,y}),tuples);
        if (fixed >= 0) {
          rel(*this,x,IRT_EQ,fixed);
          rel(*this,y,IRT_EQ,0);
        }
        if (failed) fail();
        Gecode::branch(*this,x,INT_VALUES_MIN());
        Gecode::branch(*this,y,INT_VALUES_MIN());
      }
      AStarSpace(AStarSpace& s)
        : Space(s), maximize(s.maximize) {
        live++;
        x.update(*this,s.x); y.update(*this,s.y);
      }
      ~AStarSpace(void) override { live--; }
      Space* copy(void) override { return new AStarSpace(*this); }
      // Preserve this model's branching in each portfolio asset.
      bool master(const Gecode::MetaInfo&) override { return true; }
      std::int64_t score(int value) const {
        const std::int64_t minimum[] = {
          9,-2,std::numeric_limits<std::int64_t>::min(),5};
        const std::int64_t maximum[] = {
          std::numeric_limits<std::int64_t>::min(),2,
          std::numeric_limits<std::int64_t>::max(),-5};
        return maximize ? maximum[value] : minimum[value];
      }
      std::int64_t objective(void) const { return score(x.val()); }
      bool improves(std::int64_t a, std::int64_t b) const {
        return maximize ? a>b : a<b;
      }
      void constrain(const Space& s0) override {
        const auto& s=static_cast<const AStarSpace&>(s0);
        for (int i=0;i<4;i++)
          if (!improves(score(i),s.objective())) rel(*this,x,IRT_NQ,i);
      }
      static Gecode::Search::HeuristicGuidance guidance(bool max,
                                                       bool loose=false) {
        using Guidance=Gecode::Search::HeuristicGuidance;
        return Guidance(max ? Guidance::MAXIMIZE : Guidance::MINIMIZE,
          [loose](const Space& s0) -> std::int64_t {
            const auto& s=static_cast<const AStarSpace&>(s0);
            if (loose && (!s.x.assigned() || !s.y.assigned()))
              return s.maximize ? std::numeric_limits<std::int64_t>::max() :
                std::numeric_limits<std::int64_t>::min();
            std::int64_t bound=s.score(s.x.min());
            for (IntVarValues values(s.x); values(); ++values)
              if (s.improves(s.score(values.val()),bound))
                bound=s.score(values.val());
            return bound;
          },
          [](const Space& s0) -> std::int64_t {
            const auto& s=static_cast<const AStarSpace&>(s0);
            // An arbitrary preference opposes the optimum at alternative 2.
            return s.x.assigned() && s.x.val()==2 ? 1000 : 0;
          });
      }
    };

    std::atomic<int> AStarSpace::live(0);

    /// Throw before a replacement root can be adopted by an engine.
    class ThrowingRoot : public AStarSpace {
    public:
      ThrowingRoot(bool propagation) {
        if (propagation) {
          Gecode::wait(*this,x,[](Space&) {
            throw std::runtime_error("propagation failure");
          });
          rel(*this,x,IRT_EQ,0);
        }
      }
      Space* copy(void) override {
        throw std::runtime_error("copy failure");
      }
    };

    static bool root_exceptions(bool cabs, unsigned int threads=1) {
      Gecode::Search::Options options; options.clone=false;
      options.threads=threads;
      const auto guidance=AStarSpace::guidance(false);
      auto engine=[&](Space* root) {
        return std::unique_ptr<Gecode::Search::Engine>(cabs ?
          Gecode::Search::cabsengine(root,guidance,options) :
          Gecode::Search::astarengine(root,guidance,options));
      };
      for (bool propagation : {false,true}) {
        // AStar with clone=false does not copy a root during construction.
        if (!cabs && !propagation) options.clone=true;
        std::unique_ptr<ThrowingRoot> root(new ThrowingRoot(propagation));
        try { auto invalid=engine(root.get()); return false; }
        catch (const std::runtime_error&) {}
        if (AStarSpace::live.load()!=1) return false;
        root.reset();
        options.clone=false;
        // Reset transfers ownership even when propagation or cloning throws.
        if (cabs || propagation) {
          auto reset=engine(nullptr);
          try { reset->reset(new ThrowingRoot(propagation)); return false; }
          catch (const std::runtime_error&) {}
          reset.reset();
          if (AStarSpace::live.load()!=0) return false;
        }
      }
      auto reset=engine(nullptr);
      reset->reset(new AStarSpace(false,-1,true));
      return !reset->next() && !reset->stopped() &&
        AStarSpace::live.load()==0;
    }
    // The parallel test runner shares this fixture's ownership counter.
    static std::mutex heuristic_ownership_mutex;

    /// An explicit finite tree with bounds and ranks independent of its costs.
    class TreeSpace : public Space {
    public:
      struct Node {
        int cost, bound, rank;
        std::vector<int> children;
      };
      using Graph=std::vector<Node>;
      std::shared_ptr<const Graph> graph;
      bool maximize;
      int vertex=0, ceiling=std::numeric_limits<int>::max();
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
          return !static_cast<const TreeSpace&>(s).value().children.empty();
        }
        const Choice* choice(Space& s) override {
          const auto& state=static_cast<const TreeSpace&>(s);
          return new Decision(*this,state.vertex,
            static_cast<unsigned int>(state.value().children.size()));
        }
        const Choice* choice(const Space&, Archive& e) override {
          int vertex; unsigned int n; e >> vertex >> n;
          return new Decision(*this,vertex,n);
        }
        ExecStatus commit(Space& s0, const Choice& c, unsigned int a) override {
          auto& s=static_cast<TreeSpace&>(s0);
          const auto& decision=static_cast<const Decision&>(c);
          s.vertex=(*s.graph)[decision.vertex].children[a];
          return s.value().cost>=s.ceiling ? ES_FAILED : ES_OK;
        }
        Actor* copy(Space& home) override { return new (home) Decisions(home,*this); }
        size_t dispose(Space& home) override {
          (void) Brancher::dispose(home); return sizeof(*this);
        }
      };
      explicit TreeSpace(Graph nodes, bool max=false)
        : graph(std::make_shared<const Graph>(std::move(nodes))), maximize(max) {
        (void) new (*this) Decisions(*this);
      }
      TreeSpace(TreeSpace& s)
        : Space(s), graph(s.graph), maximize(s.maximize), vertex(s.vertex),
          ceiling(s.ceiling) {}
      Space* copy(void) override { return new TreeSpace(*this); }
      const Node& value(void) const { return (*graph)[vertex]; }
      void constrain(const Space& s0) override {
        ceiling=static_cast<const TreeSpace&>(s0).value().cost;
        if (value().cost>=ceiling) fail();
      }
      static Gecode::Search::HeuristicGuidance guidance(bool max=false) {
        using Guidance=Gecode::Search::HeuristicGuidance;
        // Maximizing the negated cost mirrors the same finite paths and cuts.
        return {max ? Guidance::MAXIMIZE : Guidance::MINIMIZE,
          [](const Space& s)->std::int64_t {
            const auto& state=static_cast<const TreeSpace&>(s);
            const std::int64_t bound=state.value().bound;
            return state.maximize ? -bound : bound;
          },[](const Space& s)->std::int64_t {
            return static_cast<const TreeSpace&>(s).value().rank;
          }};
      }
      static int optimum(const Graph& graph, int vertex=0) {
        const auto& node=graph[vertex];
        if (node.children.empty()) return node.cost;
        int best=std::numeric_limits<int>::max();
        for (int child : node.children) best=std::min(best,optimum(graph,child));
        return best;
      }
    };

    class AStarTest : public Base {
      static bool ordering(void) {
        using namespace Gecode::Search;
        // Every root alternative needs another commit. The first arrival has
        // the preferred rank but the worse bound; arrivals 2 and 3 lead to
        // the same goal.
        const TreeSpace::Graph graph={
          {0,0,0,{1,2,3}}, {0,8,-100,{4}},
          {0,4,100,{5}}, {0,4,100,{5}},
          {10,10,0,{}}, {5,5,0,{}}};
        auto solve=[&](bool max, AStarOrder order, bool ranked) {
          Gecode::Search::Options options; options.clone=false;
          auto guidance=TreeSpace::guidance(max);
          if (!ranked) guidance.rank={};
          Gecode::AStar<TreeSpace> engine(new TreeSpace(graph,max),
                                         guidance,order,options);
          if (!engine.best) return false;
          std::vector<int> costs;
          while (std::unique_ptr<TreeSpace> solution{
                   engine.next()})
            costs.push_back(solution->value().cost);
          const std::vector<int> expected=order==RANK_FIRST && ranked ?
            std::vector<int>{10,5} : std::vector<int>{5};
          return costs==expected && !engine.stopped() &&
            costs.back()==TreeSpace::optimum(graph);
        };
        for (bool max : {false,true})
          if (!solve(max,BOUND_FIRST,true) || !solve(max,RANK_FIRST,true)) return false;
        return solve(false,RANK_FIRST,false);
      }
      static bool solutions(bool max, bool loose, bool interrupt) {
        Gecode::Search::NodeStop stop(1);
        Gecode::Search::Options o; o.clone=false;
        if (interrupt) o.stop=&stop;
        Gecode::AStar<AStarSpace> engine(new AStarSpace(max),
                                        AStarSpace::guidance(max,loose),o);
        std::unique_ptr<AStarSpace> first(engine.next());
        if (!first || first->x.val()!=0) return false;
        auto previous=first->objective();
        if (interrupt) {
          const auto before=engine.statistics();
          if (engine.next() || !engine.stopped() ||
              engine.statistics().node!=before.node) return false;
          stop.limit(10000);
        }
        unsigned int found=1;
        while (std::unique_ptr<AStarSpace> solution{engine.next()}) {
          if (!solution->improves(solution->objective(),previous)) return false;
          previous=solution->objective(); found++;
        }
        // Enumerating the four scores independently identifies alternative 2.
        return previous==first->score(2) && found==(loose ? 3U : 2U) &&
          !engine.stopped() && engine.statistics().depth==2;
      }
      static bool adoption(bool max, bool queued) {
        Gecode::Search::Options o; o.clone=false;
        Gecode::AStar<AStarSpace> engine(new AStarSpace(max),
                                        AStarSpace::guidance(max,true),o);
        std::unique_ptr<Space> first(engine.next());
        if (!first) return false;
        if (queued) {
          std::unique_ptr<AStarSpace> second(
            engine.next());
          if (!second || second->x.val()!=1) return false;
        }
        AStarSpace incumbent(max,queued ? 2 : 1), worse(max,0);
        incumbent.status(); worse.status();
        engine.constrain(incumbent);
        engine.constrain(incumbent);
        engine.constrain(worse);
        unsigned int found=0;
        while (std::unique_ptr<AStarSpace> solution{
                 engine.next()}) {
          if (!solution->improves(solution->objective(),incumbent.objective()) ||
              solution->x.val()!=2) return false;
          found++;
        }
        if (engine.stopped() || found!=(queued ? 0U : 1U)) return false;
        return true;
      }
      static bool helpers(bool max) {
        Gecode::Search::Options o; o.clone=false;
        const auto guidance=AStarSpace::guidance(max,true);
        // These searches first publish x=0; the helpers must keep improving.
        std::unique_ptr<AStarSpace> astar(
          Gecode::astar(new AStarSpace(max),guidance,o));
        std::unique_ptr<AStarSpace> ordered(Gecode::astar(
          new AStarSpace(max),guidance,Gecode::Search::RANK_FIRST,o));
        if (!astar || astar->x.val()!=2 || !ordered || ordered->x.val()!=2)
          return false;
        std::unique_ptr<AStarSpace> failed_astar(
          Gecode::astar(new AStarSpace(max,-1,true),guidance,o));
        std::unique_ptr<AStarSpace> failed_ordered(Gecode::astar(
          new AStarSpace(max,-1,true),guidance,Gecode::Search::RANK_FIRST,o));
        return !failed_astar && !failed_ordered;
      }
      static bool ownership(void) {
        Gecode::Search::Options o;
        for (bool clone : {false,true}) {
          o.clone=clone;
          for (int state : {-2,-1,0}) {
            std::unique_ptr<AStarSpace> root(new AStarSpace(false,state,state==-2));
            {
              Gecode::AStar<AStarSpace> engine(root.get(),
                                              AStarSpace::guidance(false),o);
              if (!clone) root.release();
              std::unique_ptr<Space> result(engine.next());
              if ((state!=-2)!=static_cast<bool>(result)) return false;
              if (state==0 && (engine.next() || engine.stopped())) return false;
              // Destroy after publication with a pending parent and queued work.
              if (state==-1) delete engine.next();
            }
            root.reset();
            if (AStarSpace::live.load()!=0) return false;
          }
        }
        AStarSpace root;
        o.clone=false;
        using Guidance=Gecode::Search::HeuristicGuidance;
        try {
          Gecode::AStar<AStarSpace> invalid(&root,Guidance(Guidance::MINIMIZE,{}),o);
          return false;
        } catch (const Gecode::Search::UninitializedAStarBound&) {}
        o.clone=true;
        {
          Guidance throws(Guidance::MINIMIZE,[](const Space&) -> std::int64_t {
            throw std::runtime_error("guidance failure");
          });
          Gecode::AStar<AStarSpace> engine(&root,throws,o);
          try { delete engine.next(); return false; }
          catch (const std::runtime_error&) {}
        }
        return AStarSpace::live.load()==1;
      }
    public:
      AStarTest(void) : Base("Search::AStar", TestTags::check()) {}
      bool run(void) override {
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (bool max : {false,true})
          if (!solutions(max,false,false) || !solutions(max,true,true) ||
              !adoption(max,false) || !adoption(max,true) || !helpers(max))
            return false;
        if (!ordering() || !root_exceptions(false) || !ownership() ||
            AStarSpace::live.load()!=0)
          return false;
        Gecode::Search::Options o; o.clone=false;
        std::unique_ptr<Gecode::Search::Engine> engine(
          Gecode::Search::astarengine(nullptr,AStarSpace::guidance(false),o));
        if (engine->next() || engine->stopped()) return false;
        engine->reset(new AStarSpace(false,2));
        std::unique_ptr<Space> solution(engine->next());
        if (!solution || engine->statistics().node!=1) return false;
        engine->reset(nullptr);
        return !engine->next() && !engine->stopped() &&
          engine->nogoods().ng()==0;
      }
    } astarTest;

    /// A narrow beam can keep a stable but ultimately infeasible branch.
    class CABSFeasibility : public Space {
    public:
      IntVar x;
      IntVarArray y;
      CABSFeasibility(bool infeasible=false) : x(*this,0,1), y(*this,3,0,1) {
        TupleSet tuples(3);
        for (int branch=0;branch<2;branch++)
          for (int a=0;a<2;a++)
            for (int b=0;b<2;b++)
              if (branch || a!=b) tuples.add(IntArgs({branch,a,b}));
        tuples.finalize();
        for (int a=0;a<3;a++)
          extensional(*this,IntVarArgs({x,y[a],y[(a+1)%3]}),tuples);
        if (infeasible) rel(*this,x,IRT_EQ,0);
        Gecode::branch(*this,x,INT_VALUES_MIN());
        Gecode::branch(*this,y,INT_VAR_NONE(),INT_VALUES_MIN());
      }
      CABSFeasibility(CABSFeasibility& s) : Space(s) {
        x.update(*this,s.x); y.update(*this,s.y);
      }
      Space* copy(void) override { return new CABSFeasibility(*this); }
      void constrain(const Space& s0) override {
        rel(*this,x,IRT_LE,static_cast<const CABSFeasibility&>(s0).x.val());
      }
      static Gecode::Search::HeuristicGuidance guidance(void) {
        return {Gecode::Search::HeuristicGuidance::MINIMIZE,
          [](const Space& s) -> std::int64_t {
            return static_cast<const CABSFeasibility&>(s).x.min();
          }};
      }
    };

    class CABSTest : public Base {
      static bool dominance(unsigned int threads=1, bool keyed=false) {
        using Relation=std::vector<std::pair<int,int>>;
        auto solve=[threads,keyed](const TreeSpace::Graph& graph, const Relation& relation,
                      unsigned long int width, int cost, bool stopped,
                      unsigned long int restarts, bool reset=false,
                      std::vector<std::size_t> keys={}) {
          Gecode::Search::Options options; options.clone=false;
          options.threads=threads;
          Gecode::Search::CABSOptions beam(width,0);
          unsigned int calls=0;
          bool grouped=true;
          beam.dominates=[relation,keys,&calls,&grouped](const Space& a, const Space& b) {
            calls++;
            const auto x=static_cast<const TreeSpace&>(a).vertex;
            const auto y=static_cast<const TreeSpace&>(b).vertex;
            if (!keys.empty() && keys[x]!=keys[y]) grouped=false;
            return std::find(relation.begin(),relation.end(),std::make_pair(x,y))
              !=relation.end();
          };
          if (keyed || !keys.empty()) beam.dominance_key=[keys](const Space& s) {
            return keys.empty() ? std::size_t{0} :
              keys[static_cast<const TreeSpace&>(s).vertex];
          };
          std::unique_ptr<Gecode::Search::Engine> engine(
            Gecode::Search::cabsengine(new TreeSpace(graph),
              TreeSpace::guidance(),options,beam));
          beam.dominates={};
          beam.dominance_key={};
          for (int run=0; run<(reset ? 2 : 1); run++) {
            const auto before=calls;
            if (run) engine->reset(new TreeSpace(graph));
            int best=std::numeric_limits<int>::max();
            while (std::unique_ptr<TreeSpace> solution{
                     static_cast<TreeSpace*>(engine->next())})
              best=solution->value().cost;
            if (best!=cost || engine->stopped()!=stopped || calls==before || !grouped ||
                engine->statistics().restart!=restarts) return false;
          }
          return true;
        };
        const TreeSpace::Graph pair={
          {0,0,0,{1,2}}, {0,0,0,{3}}, {0,0,9,{4}},
          {10,10,0,{}}, {5,5,0,{}}};
        // The worse-ranked dominator takes the only slot without width loss.
        if (!solve(pair,{{2,1}},1,5,false,0)) return false;
        auto equivalent=pair;
        equivalent[3].cost=equivalent[3].bound=5;
        // Equal labels retain the first representative, even with a better rank.
        equivalent[2].rank=-1;
        if (!solve(equivalent,{{1,2},{2,1}},1,5,false,0) ||
            !solve(pair,{},0,5,false,1)) return false;

        const TreeSpace::Graph multiple={
          {0,0,0,{1,2,3,4}}, {0,0,0,{5}}, {0,0,1,{6}},
          {0,0,9,{7}}, {0,0,10,{8}},
          {10,10,0,{}}, {11,11,0,{}}, {5,5,0,{}}, {4,4,0,{}}};
        // Removing both victims frees a slot for the worse-ranked optimum.
        // Reset repeats widening and must retain the copied callback.
        if (!solve(multiple,{{3,1},{3,2}},2,4,false,1,true) ||
            !solve(multiple,{{3,1},{3,2}},2,4,false,1,true,{0,7,7,7,9,0,0,0,0}))
          return false;

        TreeSpace::Graph lifetime={
          {0,0,0,{1,2,3}}, {0,0,1,{4}}, {0,0,0,{5}}, {0,0,-1,{6}},
          {4,4,0,{}}, {10,10,0,{}}, {5,5,0,{}}};
        // Candidate 1 can subsume 3, but cannot do so after beam eviction.
        if (!solve(lifetime,{{1,3}},1,5,true,0)) return false;
        lifetime[0].children={2,1,3};
        // Nor may candidate 1 justify pruning after admission discards it.
        return solve(lifetime,{{1,3}},1,5,true,0);
      }
      static bool discarded_bounds(bool max) {
        Gecode::Search::Options options; options.clone=false;
        auto solve=[&](const TreeSpace::Graph& graph, unsigned long int width,
                      int cost, bool stopped, unsigned long int restarts) {
          Gecode::CABS<TreeSpace> engine(new TreeSpace(graph,max),
            TreeSpace::guidance(max),options,{width,0});
          int previous=std::numeric_limits<int>::max();
          bool found=false;
          while (std::unique_ptr<TreeSpace> solution{engine.next()}) {
            if (solution->value().cost>=previous) return false;
            previous=solution->value().cost;
            found=true;
          }
          return found && previous==cost && engine.stopped()==stopped &&
            engine.statistics().restart==restarts;
        };
        for (bool evict : {false,true}) {
          // Reject a worse-ranked candidate, or evict one arriving first.
          // Equality also excludes strict improvement of the incumbent.
          const int bound=evict ? 10 : 20;
          const TreeSpace::Graph bounded={
            {0,0,0,evict ? std::vector<int>{2,1} : std::vector<int>{1,2}},
            {0,0,0,{3}}, {0,bound,1,{4}},
            {10,10,0,{}}, {bound,bound,0,{}}};
          if (!solve(bounded,1,TreeSpace::optimum(bounded),false,0))
            return false;
          // Ranking keeps bound 5 over bound 0; the selected bound then falls
          // to 1. A later discard at bound 20 must not erase the earlier 0.
          const TreeSpace::Graph improving={
            {0,0,0,evict ? std::vector<int>{2,1} : std::vector<int>{1,2}},
            {0,5,0,{3,4}}, {0,0,1,{5}},
            {0,1,0,{6}}, {0,20,1,{7}},
            {4,4,0,{}}, {10,10,0,{}}, {20,20,0,{}}};
          if (!solve(improving,1,10,true,0) ||
              !solve(improving,0,TreeSpace::optimum(improving),false,1))
            return false;
        }
        return true;
      }
      static bool interrupted_discard(bool external) {
        // Width loss precedes a shallow goal. The original fourth alternative
        // and the pending parent both improve it and survive publication.
        const TreeSpace::Graph graph={
          {0,0,0,{1,2,3,4}}, {0,0,0,{5}}, {0,7,1,{6}},
          {10,10,0,{}}, {8,8,0,{}},
          {6,6,0,{}}, {7,7,0,{}}};
        Gecode::Search::NodeStop stop(10000);
        Gecode::Search::Options options; options.clone=false; options.stop=&stop;
        Gecode::CABS<TreeSpace> engine(new TreeSpace(graph),
          TreeSpace::guidance(),options,{1,0});
        std::unique_ptr<TreeSpace> first(engine.next());
        if (!first || first->value().cost!=10) return false;
        const auto before=engine.statistics().node;
        stop.limit(before-1);
        for (int i=0;i<2;i++)
          if (engine.next() || !engine.stopped() ||
              engine.statistics().node!=before) return false;
        if (external) {
          TreeSpace incumbent(graph);
          incumbent.vertex=5;
          if (incumbent.status()!=SS_SOLVED) return false;
          engine.constrain(incumbent);
        }
        stop.limit(10000);
        std::vector<int> costs;
        while (std::unique_ptr<TreeSpace> solution{engine.next()})
          costs.push_back(solution->value().cost);
        return costs==(external ? std::vector<int>{} : std::vector<int>{8,6}) &&
          !engine.stopped() && !engine.statistics().restart &&
          engine.statistics().node==before+2;
      }
      static bool reset_discard(void) {
        const TreeSpace::Graph bounded={
          {0,0,0,{1,2}}, {0,0,0,{3}}, {0,20,1,{4}},
          {10,10,0,{}}, {20,20,0,{}}};
        auto improving=bounded;
        improving[2].bound=0;
        improving[4].cost=improving[4].bound=4;
        Gecode::Search::Options options; options.clone=false;
        for (unsigned long int width : {1UL,2UL}) {
          std::unique_ptr<Gecode::Search::Engine> engine(
            Gecode::Search::cabsengine(new TreeSpace(bounded),
              TreeSpace::guidance(),options,{width,0}));
          for (int run=0;run<3;run++) {
            // Repeat widening after reset to check that width starts at one.
            const bool unresolved=run==1 || (run==2 && width==2);
            if (run) engine->reset(new TreeSpace(unresolved ? improving : bounded));
            if (engine->stopped() || engine->statistics().node ||
                engine->statistics().restart) return false;
            std::vector<int> costs;
            while (std::unique_ptr<TreeSpace> solution{
                     static_cast<TreeSpace*>(engine->next())})
              costs.push_back(solution->value().cost);
            const bool widen=unresolved && width==2;
            if (costs!=(widen ? std::vector<int>{10,4} : std::vector<int>{10}) ||
                engine->stopped()!=(unresolved && width==1) ||
                engine->statistics().restart!=(widen ? 1UL : 0UL)) return false;
          }
        }
        return true;
      }
      static bool solutions(bool max, unsigned long int width) {
        Gecode::Search::Options o; o.clone=false;
        Gecode::CABS<AStarSpace> engine(new AStarSpace(max),
          AStarSpace::guidance(max,true),o,{width,0});
        std::int64_t previous=0;
        unsigned int found=0;
        while (std::unique_ptr<AStarSpace> s{engine.next()}) {
          if (found && !s->improves(s->objective(),previous)) return false;
          previous=s->objective(); found++;
        }
        // The helper returns the last improvement even when the width cap stops it.
        std::unique_ptr<AStarSpace> best(Gecode::cabs(new AStarSpace(max),
          AStarSpace::guidance(max,true),o,{width,0}));
        if (!best || best->x.val()!=(width==1 ? 1 : 2)) return false;
        AStarSpace reference(max,2);
        if (width==1) {
          // A shallow goal and a deeper improvement survive publication, but
          // the misleading rank drops the optimum from the width-one beam.
          const auto before=engine.statistics().node;
          return found==2 && previous==reference.score(1) && engine.stopped() &&
            !engine.next() && engine.stopped() && engine.statistics().node==before;
        }
        return found==3 && previous==reference.objective() && !engine.stopped() &&
          engine.statistics().restart>0;
      }
      static bool feasibility(void) {
        Gecode::Search::Options o; o.clone=false;
        for (unsigned long int width : {1UL,0UL}) {
          Gecode::CABS<CABSFeasibility> engine(new CABSFeasibility,
            CABSFeasibility::guidance(),o,{width,0});
          std::unique_ptr<CABSFeasibility> s(engine.next());
          if (width==1) {
            if (s || !engine.stopped() || engine.next() || !engine.stopped())
              return false;
          } else {
            if (!s || s->x.val()!=1 || engine.next() || engine.stopped() ||
                !engine.statistics().restart) return false;
          }
        }
        std::unique_ptr<CABSFeasibility> failed_helper(Gecode::cabs(
          new CABSFeasibility(true),CABSFeasibility::guidance(),o));
        if (failed_helper) return false;
        Gecode::CABS<CABSFeasibility> failed(new CABSFeasibility(true),
          CABSFeasibility::guidance(),o);
        Gecode::BAB<CABSFeasibility> reference(new CABSFeasibility(true),o);
        if (failed.next() || failed.stopped() || reference.next() ||
            reference.stopped()) return false;
        Gecode::BAB<AStarSpace> bab(new AStarSpace,o);
        int optimum=-1;
        while (std::unique_ptr<AStarSpace> s{bab.next()}) optimum=s->x.val();
        return optimum==2 && !bab.stopped();
      }
      static bool adoption(bool queued) {
        Gecode::Search::Options o; o.clone=false;
        Gecode::CABS<AStarSpace> engine(new AStarSpace,
          AStarSpace::guidance(false,true),o);
        std::unique_ptr<Space> first(engine.next());
        if (!first) return false;
        if (queued) {
          std::unique_ptr<AStarSpace> second(
            engine.next());
          if (!second || second->x.val()!=1) return false;
        }
        {
          AStarSpace incumbent(false,queued ? 2 : 1), worse(false,0);
          incumbent.status(); worse.status();
          engine.constrain(incumbent);
          engine.constrain(incumbent);
          engine.constrain(worse);
        }
        // The accepted external copy survives the caller's destruction. Its
        // exact INT64_MIN objective also bounds out every discarded candidate;
        // a weaker adoption still lets the original alternatives recover x=2.
        std::unique_ptr<AStarSpace> optimum(
          engine.next());
        return (queued ? !optimum : optimum && optimum->x.val()==2) &&
          !engine.next() && !engine.stopped() &&
          (!queued || engine.statistics().restart==0);
      }
      static bool ownership_and_limits(void) {
        Gecode::Search::Options o;
        for (bool clone : {false,true}) {
          o.clone=clone;
          for (int state : {-2,-1,0}) {
            std::unique_ptr<AStarSpace> root(new AStarSpace(false,state,state==-2));
            std::unique_ptr<Space> result;
            {
              Gecode::CABS<AStarSpace> engine(root.get(),
                AStarSpace::guidance(false),o);
              if (!clone) root.release();
              result.reset(engine.next());
              if ((state!=-2)!=static_cast<bool>(result)) return false;
              if (state==0 && (engine.next() || engine.stopped())) return false;
            }
            // A returned solution remains independent of all engine storage.
            if (result && result->status()!=SS_SOLVED) return false;
            result.reset(); root.reset();
            if (AStarSpace::live.load()!=0) return false;
          }
        }
        o.clone=false;
        for (const auto limits : {Gecode::Search::CABSOptions(1,0),
                                  Gecode::Search::CABSOptions(0,1)}) {
          std::unique_ptr<Gecode::Search::Engine> engine(
            Gecode::Search::cabsengine(new AStarSpace,
              AStarSpace::guidance(false,true),o,limits));
          while (std::unique_ptr<Space> s{engine->next()}) {}
          const auto before=engine->statistics().node;
          if (!engine->stopped() || engine->next() || !engine->stopped() ||
              engine->statistics().node!=before) return false;
          engine->reset(new AStarSpace(false,2));
          std::unique_ptr<Space> s(engine->next());
          if (!s || engine->stopped() || engine->statistics().node!=1 ||
              engine->statistics().restart || engine->next()) return false;
          engine->reset(nullptr);
          if (engine->next() || engine->stopped() || engine->nogoods().ng())
            return false;
        }
        AStarSpace root;
        o.tracer=&Gecode::StdSearchTracer::def;
        try {
          Gecode::CABS<AStarSpace> invalid(&root,AStarSpace::guidance(false),o);
          return false;
        } catch (const Gecode::Search::UnsupportedCABSTracer&) {}
        o.tracer=nullptr;
        using Guidance=Gecode::Search::HeuristicGuidance;
        try {
          Gecode::CABS<AStarSpace> invalid(&root,Guidance(Guidance::MINIMIZE,{}),o);
          return false;
        } catch (const Gecode::Search::UninitializedCABSBound&) {}
        o.clone=true;
        {
          Guidance throws(Guidance::MINIMIZE,[](const Space&) -> std::int64_t {
            throw std::runtime_error("guidance failure");
          });
          Gecode::CABS<AStarSpace> engine(&root,throws,o);
          try { delete engine.next(); return false; }
          catch (const std::runtime_error&) {}
        }
        return AStarSpace::live.load()==1;
      }
    public:
      CABSTest(void) : Base("Search::CABS", TestTags::check()) {}
      bool run(void) override {
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (bool max : {false,true})
          if (!solutions(max,1) || !solutions(max,0) || !discarded_bounds(max))
            return false;
        return interrupted_discard(false) && interrupted_discard(true) &&
          dominance() && dominance(4) && dominance(1,true) && dominance(4,true) &&
          reset_discard() && feasibility() && adoption(false) && adoption(true) &&
          root_exceptions(true) && ownership_and_limits() &&
          AStarSpace::live.load()==0;
      }
    } cabsTest;

    /// Mixed optimization builders share incumbents and resume after a stop.
    class HeuristicPortfolioTest : public Base {
    public:
      HeuristicPortfolioTest(void)
        : Base("Search::Heuristic::Portfolio", TestTags::check()) {}
      bool run(void) override {
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (bool max : {false,true})
          for (unsigned int threads : {1U,3U}) {
            Gecode::Search::NodeStop stop(0);
            Gecode::Search::Options asset;
            asset.threads=1; asset.stop=&stop;
            const auto guidance=AStarSpace::guidance(max,true);
            Gecode::SEBs assets{Gecode::bab<AStarSpace>(asset),
              Gecode::astar<AStarSpace>(guidance,asset),
              Gecode::cabs<AStarSpace>(guidance,asset)};
            Gecode::Search::Options options;
            options.threads=threads;
            AStarSpace root(max), incumbent(max,1);
            incumbent.status();
            Gecode::PBS<AStarSpace,Gecode::BAB> engine(&root,assets,options);
            while (std::unique_ptr<AStarSpace> solution{engine.next()}) {}
            if (!engine.stopped()) return false;
            engine.constrain(incumbent);
            stop.limit(10000);
            auto previous=incumbent.objective();
            bool found=false;
            while (std::unique_ptr<AStarSpace> solution{engine.next()}) {
              if (!solution->improves(solution->objective(),previous))
                return false;
              previous=solution->objective(); found=true;
            }
            if (!found || previous!=incumbent.score(2) || engine.stopped())
              return false;
          }
        return AStarSpace::live.load()==0;
      }
    } heuristicPortfolioTest;

#ifdef GECODE_HAS_THREADS
    class ParallelHeuristic : public Base {
      static bool check(bool beam, bool max, unsigned int threads) {
        using namespace Gecode::Search;
        NodeStop stop(1);
        Gecode::Search::Options options; options.clone=false; options.threads=threads; options.stop=&stop;
        const auto guidance=AStarSpace::guidance(max,true);
        auto make=[&](Space* root, CABSOptions limits=CABSOptions()) {
          return std::unique_ptr<Engine>(beam ? cabsengine(root,guidance,options,limits) :
            bestfirstengine(root,guidance,RANK_FIRST,options));
        };
        auto engine=make(new AStarSpace(max));
        std::optional<std::int64_t> previous;
        auto consume=[&]() {
          while (std::unique_ptr<AStarSpace> s{
                   static_cast<AStarSpace*>(engine->next())}) {
            if (previous && !s->improves(s->objective(),*previous)) return false;
            previous=s->objective();
          }
          return true;
        };
        if (!consume() || !engine->stopped()) return false;
        const auto before=engine->statistics().node;
        if (engine->next() || !engine->stopped() || engine->statistics().node!=before)
          return false;
        stop.limit(100000);
        if (!consume() || engine->stopped()) return false;
        AStarSpace optimum(max,2);
        optimum.status();
        if (!previous || *previous!=optimum.objective()) return false;
        for (int repeat=0; repeat<3; repeat++) {
          engine->reset(new AStarSpace(max));
          if (engine->statistics().node || engine->statistics().restart) return false;
          AStarSpace incumbent(max,1); incumbent.status();
          engine->constrain(incumbent);
          previous=incumbent.objective();
          if (!consume() || engine->stopped() || *previous!=optimum.objective()) return false;
          engine->reset(new AStarSpace(max));
          engine->constrain(optimum);
          if (engine->next() || engine->stopped()) return false;
          engine->reset(new AStarSpace(max,-1,true));
          if (engine->next() || engine->stopped()) return false;
        }
        engine.reset();
        if (beam) for (const auto limits : {CABSOptions(1,0),CABSOptions(0,1)}) {
          engine=make(new AStarSpace(max),limits);
          previous.reset();
          if (!consume() || !engine->stopped()) return false;
          const auto nodes=engine->statistics().node;
          if (engine->next() || !engine->stopped() || engine->statistics().node!=nodes)
            return false;
          engine->reset(new AStarSpace(max,2));
          std::unique_ptr<Space> s(engine->next());
          if (!s || engine->stopped() || engine->statistics().node!=1 || engine->next())
            return false;
        }
        return true;
      }
      static bool exceptions(bool beam, unsigned int threads=4) {
        using namespace Gecode::Search;
        Gecode::Search::Options options; options.threads=threads;
        AStarSpace root;
        for (bool rank : {false,true}) {
          auto guidance=AStarSpace::guidance(false);
          auto fail=[](const Space&)->std::int64_t {
            throw std::runtime_error("parallel guidance failure");
          };
          if (rank) guidance.rank=fail; else guidance.bound=fail;
          std::unique_ptr<Engine> engine(beam ? cabsengine(&root,guidance,options) :
            astarengine(&root,guidance,options));
          bool caught=false;
          try { while (std::unique_ptr<Space> s{engine->next()}) {} }
          catch (const std::runtime_error&) { caught=true; }
          if (!caught) return false;
        }
        if (beam) for (bool key : {false,true}) {
          CABSOptions limits;
          limits.dominates=[key](const Space&,const Space&)->bool {
            if (!key) throw std::runtime_error("dominance failure");
            return false;
          };
          if (key) limits.dominance_key=[](const Space&)->std::size_t {
            throw std::runtime_error("dominance key failure");
          };
          std::unique_ptr<Engine> engine(cabsengine(&root,
            AStarSpace::guidance(false,true),options,limits));
          bool caught=false;
          try { while (std::unique_ptr<Space> s{engine->next()}) {} }
          catch (const std::runtime_error&) { caught=true; }
          if (!caught) return false;
          // A key alone does not enable comparisons or evaluate the callback.
          if (key) {
            limits.dominates={};
            engine.reset(cabsengine(&root,AStarSpace::guidance(false,true),options,limits));
            while (std::unique_ptr<Space> s{engine->next()}) {}
            if (engine->stopped()) return false;
          }
        }
        return AStarSpace::live.load()==1;
      }
      static bool failed_alternatives(bool beam, bool bounded) {
        using namespace Gecode::Search;
        TreeSpace::Graph graph(131,{2,2,0,{}});
        graph[0]={0,0,0,{}};
        for (int i=1; i<=129; i++) graph[0].children.push_back(i);
        if (bounded)
          for (int i=1; i<=128; i++) graph[i]={0,2,0,{130}};
        graph[129]={0,0,0,{}};
        TreeSpace incumbent(graph);
        incumbent.vertex=130;
        incumbent.status();
        NodeStop stop(7);
        Gecode::Search::Options options;
        options.clone=false; options.threads=4; options.stop=&stop;
        std::unique_ptr<Engine> engine(beam ?
          cabsengine(new TreeSpace(graph),TreeSpace::guidance(),options) :
          astarengine(new TreeSpace(graph),TreeSpace::guidance(),options));
        engine->constrain(incumbent);
        // Interrupt a long original choice, then reach its final improving goal.
        std::unique_ptr<Space> early(engine->next());
        if (early || !engine->stopped() || engine->statistics().node>=130)
          return false;
        stop.limit(1000);
        std::unique_ptr<TreeSpace> solution(static_cast<TreeSpace*>(engine->next()));
        if (!solution || solution->value().cost!=0) return false;
        std::unique_ptr<Space> extra(engine->next());
        return !extra && !engine->stopped() &&
          engine->statistics().node==130 && engine->statistics().fail==128;
      }
      static bool queued_failures(bool beam) {
        using namespace Gecode::Search;
        class Boundary : public Stop {
        public:
          bool queued=false, resumed=false;
          bool stop(const Statistics& s, const Gecode::Search::Options&) override {
            return !resumed && (queued ? s.fail>0 : s.node>=129);
          }
        } stop;
        TreeSpace::Graph graph(130,{0,2,0,{129}});
        graph[0]={0,0,0,{}};
        for (int i=1; i<=128; i++) graph[0].children.push_back(i);
        graph[129]={2,2,0,{}};
        Gecode::Search::Options options;
        options.clone=false; options.threads=2; options.stop=&stop;
        options.worker_control=WorkerControl(1);
        std::unique_ptr<Engine> engine(beam ?
          cabsengine(new TreeSpace(graph),TreeSpace::guidance(),options) :
          astarengine(new TreeSpace(graph),TreeSpace::guidance(),options));
        std::unique_ptr<Space> early(engine->next());
        if (early || !engine->stopped() || engine->statistics().fail) return false;
        TreeSpace incumbent(graph);
        incumbent.vertex=129;
        incumbent.status();
        engine->constrain(incumbent);
        stop.queued=true;
        // Rejecting queued nodes must still observe the first failure boundary.
        early.reset(engine->next());
        if (early || !engine->stopped() || engine->statistics().fail!=1) return false;
        stop.resumed=true;
        early.reset(engine->next());
        return !early && !engine->stopped() &&
          engine->statistics().node==129 &&
          engine->statistics().fail==(beam ? 1U : 128U);
      }
      static bool overlap(bool beam) {
        using namespace Gecode::Search;
        // All shallow leaves are costly; widening must reach both parents
        // before the better goal at the final original alternative.
        const TreeSpace::Graph graph={
          {0,0,0,{1,2,3,4}}, {0,0,0,{5,6}}, {0,0,1,{7,8}},
          {0,0,2,{9,10}}, {0,0,3,{11,12}},
          {20,20,0,{}}, {19,19,0,{}}, {18,18,0,{}}, {17,17,0,{}},
          {16,16,0,{}}, {15,15,0,{}}, {14,14,0,{}}, {1,1,0,{}}};
        Gecode::Search::Options options; options.clone=false; options.threads=4;
        auto guidance=TreeSpace::guidance();
        auto bound=guidance.bound;
        std::atomic<int> inside{0}, peak{0};
        guidance.bound=[&](const Space& s) {
          const int count=++inside;
          int old=peak.load();
          while (old<count && !peak.compare_exchange_weak(old,count)) {}
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          const auto value=bound(s);
          --inside;
          return value;
        };
        std::unique_ptr<Engine> engine(beam ?
          cabsengine(new TreeSpace(graph),guidance,options) :
          astarengine(new TreeSpace(graph),guidance,options));
        int best=100;
        while (std::unique_ptr<TreeSpace> s{static_cast<TreeSpace*>(engine->next())}) {
          if (s->value().cost>=best) return false;
          best=s->value().cost;
          if (inside.load()) return false;
        }
        return best==1 && !engine->stopped() && peak.load()>1;
      }
    public:
      ParallelHeuristic(void) : Base("Search::ParallelHeuristic",TestTags::check()) {}
      bool run(void) override {
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (bool beam : {false,true}) {
          for (bool max : {false,true})
            for (unsigned int threads : {2U,4U})
              if (!check(beam,max,threads)) return false;
          if (!root_exceptions(beam,4) || !exceptions(beam) ||
              (beam && !exceptions(beam,1)) || !overlap(beam) ||
              !failed_alternatives(beam,false) ||
              !failed_alternatives(beam,true) || !queued_failures(beam)) return false;
          if (AStarSpace::live.load()!=0) return false;
        }
        return true;
      }
    } parallelHeuristic;
#endif


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
        bool paused;
      public:
        std::promise<void> reached;
        std::atomic<bool> terminate{false};
        Pause(const Gecode::Search::WorkerControl& c, unsigned int n)
          : control(c), node(n), paused(false) {}
        bool stop(const Gecode::Search::Statistics& s,
                  const Gecode::Search::Options&) override {
          if (!paused && (s.node >= node)) {
            paused = true;
            control.request(0U);
            reached.set_value();
          }
          return terminate.load(std::memory_order_acquire);
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
#ifdef GECODE_HAS_EXPERIMENTAL
      bool native(unsigned int kind, unsigned int node, bool stopping,
                  unsigned int threads=1) {
        Gecode::Search::WorkerControl control(node == 0U ? 0U : 1U);
        Pause pause(control,node);
        Gecode::Search::Options options;
        options.threads=threads; options.worker_control=control; options.stop=&pause;
        AStarSpace root;
        const auto guidance=AStarSpace::guidance(false);
        std::unique_ptr<Gecode::Search::Engine> engine(kind==0U ?
          Gecode::Search::astarengine(&root,guidance,options) :
          Gecode::Search::cabsengine(&root,guidance,options));
        std::int64_t optimum=0;
        bool found=false;
        auto consume=[&] {
          while (std::unique_ptr<AStarSpace> solution{
                   static_cast<AStarSpace*>(engine->next())}) {
            optimum=solution->objective(); found=true;
          }
        };
        // A shallow generated goal may return before the pause threshold.
        // Keep consuming publications until the engine reaches that boundary.
        auto result=std::async(std::launch::async,consume);
        pause.reached.get_future().wait();
        bool paused=result.wait_for(std::chrono::milliseconds(20))==
          std::future_status::timeout;
        const auto before=engine->statistics().node;
        if (stopping) {
          pause.terminate.store(true,std::memory_order_release);
          Gecode::Search::WorkerControlAccess::signal_all(control);
          result.get();
          if (!engine->stopped() || engine->statistics().node!=before)
            return false;
          pause.terminate.store(false,std::memory_order_release);
          control.request(threads);
          consume();
        } else {
          control.request(threads);
          result.get();
        }
        const bool complete=paused && found && optimum==root.score(2) && !engine->stopped();
        // Destruction must wake parked workers before waiting for completion.
        control.request(0U);
        engine.reset();
        return complete;
      }
#endif

    public:
      WorkerControlSequentialPause(void)
        : Base("Search::WorkerControl::SequentialPause", TestTags::check()) {}
      bool run(void) override {
#ifdef GECODE_HAS_THREADS
#ifdef GECODE_HAS_EXPERIMENTAL
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (unsigned int kind : {0U,1U})
          if (!native(kind,0U,false) || !native(kind,1U,false) ||
              !native(kind,0U,true) || !native(kind,0U,false,4) ||
              !native(kind,1U,false,4) || !native(kind,0U,true,4)) return false;
#endif
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
              if (controls[i].engaged())
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
            if (controls[0].engaged())
              controls[0].request(1U + _rand(capacity));
            if (controls[2].engaged())
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
