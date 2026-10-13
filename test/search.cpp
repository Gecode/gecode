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
#include <gecode/search/completion.hh>
#include <gecode/search/branch-feedback.hh>

#include "test/test.hh"
#include "examples/tsptw-transition.hh"

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

    /// A real nested existential search using the production completion owner.
    class CompletionSpace;
    class TestCompletion : public Gecode::Search::Completion {
      int first, count;
    public:
      TestCompletion(Home home, int f, int n)
        : Completion(home), first(f), count(n) {
        home.notice(*this,AP_DISPOSE,false);
      }
      TestCompletion(Space& home, TestCompletion& b)
        : Completion(home,b), first(b.first), count(b.count) {}
      bool status(const Space&) const override { return !done && !suppressed; }
      Space* trial(Space& home) override;
      Actor* copy(Space& home) override {
        return new (home) TestCompletion(home,*this);
      }
      size_t dispose(Space& home) override {
        home.ignore(*this,AP_DISPOSE,false);
        (void) Completion::dispose(home);
        return sizeof(*this);
      }
    };

    class CompletionSpace : public Space {
    protected:
      std::shared_ptr<std::atomic<unsigned int>> constraints;
    public:
      IntVarArray x;
      CompletionSpace(bool sat, bool nested, bool outer=false,
                      std::shared_ptr<std::atomic<unsigned int>> constraints0={},
                      bool completion=true)
        : constraints(std::move(constraints0)), x(*this,sat ? 4 : 5,1,4) {
        for (int i=0;i<x.size();i++)
          for (int j=0;j<i;j++) rel(*this,x[i],IRT_NQ,x[j]);
        if (outer) {
          IntVar y(*this,0,2);
          branch(*this,y,INT_VAL_MIN());
        }
        if (!completion) {
          branch(*this,x,INT_VAR_NONE(),INT_VAL_MIN());
        } else if (nested) {
          (void) new (*this) TestCompletion(*this,0,1);
          (void) new (*this) TestCompletion(*this,1,x.size()-1);
        } else {
          (void) new (*this) TestCompletion(*this,0,x.size());
        }
      }
      CompletionSpace(CompletionSpace& s)
        : Space(s), constraints(s.constraints) { x.update(*this,s.x); }
      Space* copy(void) override { return new CompletionSpace(*this); }
      void constrain(const Space&) override {
        if (constraints)
          constraints->fetch_add(1,std::memory_order_relaxed);
        // Direct view restriction deliberately bypasses posting Home.
        for (int i=0;i<x.size();i++)
          if (me_failed(IntView(x[i]).lq(*this,3))) {
            fail(); return;
          }
      }
    };

    Space* TestCompletion::trial(Space& home) {
      std::unique_ptr<CompletionSpace> s(
        static_cast<CompletionSpace*>(home.clone()));
      IntVarArgs args(count);
      for (int i=0;i<count;i++) args[i]=s->x[first+i];
      branch(*s,args,INT_VAR_NONE(),INT_VAL_MIN());
      return s.release();
    }

    class CompletionNative : public Base {
      static bool same(const Gecode::Search::Statistics& a,
                       const Gecode::Search::Statistics& b) {
        return a.node==b.node && a.fail==b.fail && a.propagate==b.propagate &&
          a.depth==b.depth && a.restart==b.restart && a.nogood==b.nogood;
      }
      class Slice : public Gecode::Search::Stop {
      public:
        std::atomic<unsigned long long> limit{2};
        bool failures=false;
        std::atomic<bool> valid{true};
        std::atomic<unsigned int> calls{0};
        bool stop(const Gecode::Search::Statistics& s,
                  const Gecode::Search::Options& o) override {
          calls.fetch_add(1,std::memory_order_relaxed);
          if (o.c_d!=5 || o.a_d!=2 || o.stop!=this)
            valid.store(false,std::memory_order_relaxed);
          return (failures ? s.fail : s.node)>=
            limit.load(std::memory_order_relaxed);
        }
      };
      class ObjectiveSpace : public CompletionSpace {
      public:
        IntVar cost;
        ObjectiveSpace(bool completion, int fixed=-1,
                       std::shared_ptr<std::atomic<unsigned int>> constraints={})
          : CompletionSpace(true,false,false,std::move(constraints),completion),
            cost(*this,0,2) {
          if (fixed >= 0)
            rel(*this,cost,IRT_EQ,fixed);
          else
            branch(*this,cost,INT_VAL_MAX());
        }
        ObjectiveSpace(ObjectiveSpace& s) : CompletionSpace(s) {
          cost.update(*this,s.cost);
        }
        Space* copy(void) override { return new ObjectiveSpace(*this); }
        SpaceComparison compare(const Space& s0) const override {
          const ObjectiveSpace& s=dynamic_cast<const ObjectiveSpace&>(s0);
          if (cost.val() < s.cost.val())
            return SC_BETTER;
          if (cost.val() > s.cost.val())
            return SC_WORSE;
          return SC_EQUIVALENT;
        }
        void constrain(const Space& s0) override {
          if (constraints)
            constraints->fetch_add(1,std::memory_order_relaxed);
          const ObjectiveSpace& s=static_cast<const ObjectiveSpace&>(s0);
          if (s.cost.val() > 0) {
            rel(*this,cost,IRT_LE,s.cost.val()-1);
          } else {
            // The strict-better alternative is deliberately infeasible, but
            // direct restriction leaves that hidden completion unresolved.
            for (int i=0;i<x.size();i++)
              if (me_failed(IntView(x[i]).lq(*this,3))) {
                fail(); return;
              }
          }
        }
      };
      template<int Kind>
      static bool heuristic(void) {
        using Guidance=Gecode::Search::HeuristicGuidance;
        Guidance guidance(Guidance::MINIMIZE,[](const Space& s) -> std::int64_t {
          return static_cast<const ObjectiveSpace&>(s).cost.min();
        });
        Gecode::Search::Options o; o.clone=false; o.c_d=5; o.a_d=2;
        std::unique_ptr<Gecode::Search::Engine> full(Kind==2 ?
          Gecode::Search::columnengine(new ObjectiveSpace(true),guidance,o) : Kind==1 ?
          Gecode::Search::cabsengine(new ObjectiveSpace(true),guidance,o) :
          Gecode::Search::astarengine(new ObjectiveSpace(true),guidance,o));
        int optimum=3;
        while (std::unique_ptr<ObjectiveSpace> s{
                 static_cast<ObjectiveSpace*>(full->next())}) optimum=s->cost.val();
        if (optimum!=0 || full->stopped()) return false;
        const auto total=full->statistics();
        optimum=3;
        Slice stop; o.stop=&stop;
        std::unique_ptr<Gecode::Search::Engine> sliced(Kind==2 ?
          Gecode::Search::columnengine(new ObjectiveSpace(true),guidance,o) : Kind==1 ?
          Gecode::Search::cabsengine(new ObjectiveSpace(true),guidance,o) :
          Gecode::Search::astarengine(new ObjectiveSpace(true),guidance,o));
        unsigned int interruptions=0;
        for (unsigned int i=0;i<400;i++) {
          if (std::unique_ptr<ObjectiveSpace> s{
                   static_cast<ObjectiveSpace*>(sliced->next())}) {
            optimum=s->cost.val(); continue;
          }
          if (!sliced->stopped()) {
            if (optimum!=0 || !interruptions || !same(total,sliced->statistics()))
              return false;
            // Adoption during retained preparation cuts committed clones,
            // including hidden completion state that must be revalidated.
            stop.limit.store(2,std::memory_order_relaxed);
            std::unique_ptr<Gecode::Search::Engine> adopted(
              Kind==2 ? Gecode::Search::columnengine(new ObjectiveSpace(true),guidance,o) :
              Kind==1 ? Gecode::Search::cabsengine(new ObjectiveSpace(true),guidance,o) :
              Gecode::Search::astarengine(new ObjectiveSpace(true),guidance,o));
            if (adopted->next() || !adopted->stopped()) return false;
            Gecode::Search::Options witness_options; witness_options.clone=false;
            Gecode::DFS<ObjectiveSpace> dfs(new ObjectiveSpace(true,0),witness_options);
            std::unique_ptr<ObjectiveSpace> witness(dfs.next());
            if (!witness) return false;
            adopted->constrain(*witness);
            const auto before=adopted->statistics();
            if (adopted->next() || !adopted->stopped() ||
                !same(before,adopted->statistics())) return false;
            stop.limit.store(10000,std::memory_order_relaxed);
            return !adopted->next() && !adopted->stopped() &&
              adopted->statistics().fail>before.fail;
          }
          interruptions++;
          const auto before=sliced->statistics();
          if (sliced->next() || !sliced->stopped() ||
              !same(before,sliced->statistics())) return false;
          stop.limit.fetch_add(2,std::memory_order_relaxed);
        }
        return false;
      }
      static bool replay(void) {
        CompletionSpace root(true,false);
        Gecode::Search::Options o;
        o.c_d=5; o.a_d=2;
        Gecode::Search::Statistics stat;
        Slice stop; o.stop=&stop;
        if (root.status()!=SS_BRANCH || root.prepare(stat,o)) return false;
        std::unique_ptr<CompletionSpace> clone(
          static_cast<CompletionSpace*>(root.clone()));
        // Restricting a pending source must discard its obsolete trial.
        root.invalidate_branchers();
        root.constrain(root);
        if (root.status()!=SS_BRANCH) return false;
        stop.limit.store(10000,std::memory_order_relaxed);
        if (!clone->prepare(stat,o)) return false;
        std::unique_ptr<const Choice> ch(clone->choice());
        Archive archive; ch->archive(archive);
        std::unique_ptr<const Choice> archived(clone->choice(archive));
        std::unique_ptr<CompletionSpace> replayed(
          static_cast<CompletionSpace*>(clone->clone()));
        replayed->commit(*archived,0);
        if (replayed->status()!=SS_SOLVED) return false;
        (void) replayed->choice();
        // Retention through solved cleanup and strengthening without failure.
        replayed->invalidate_branchers();
        replayed->constrain(root);
        if (replayed->status()!=SS_BRANCH) return false;
        replayed->commit(*ch,0);
        if (replayed->status()!=SS_BRANCH) return false;
        Slice small; o.stop=&small;
        Gecode::Search::Statistics revalidation;
        if (replayed->prepare(revalidation,o)) return false;
        small.limit.store(10000,std::memory_order_relaxed);
        if (!replayed->prepare(revalidation,o)) return false;
        std::unique_ptr<const Choice> failed(replayed->choice());
        replayed->commit(*failed,0);
        if (replayed->status()!=SS_FAILED) return false;
        // The independent clone's unchanged established choice needs no DFS.
        clone->commit(*ch,0);
        return clone->status()==SS_SOLVED;
      }
    public:
      CompletionNative(void) : Base("Search::Completion::Native", TestTags::check()) {}
      bool run(void) override {
        return replay() && heuristic<0>() && heuristic<1>() && heuristic<2>();
      }
    } completionNative;

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
        Gecode::Search::Options options;
        Gecode::Search::Statistics stat;
        if (s.status()!=SS_BRANCH || !s.prepare(stat,options)) return false;
        const auto actions=s.actions();
        const auto action=std::find(actions.begin(),actions.end(),node);
        if (action==actions.end()) return false;
        std::unique_ptr<const Choice> choice(s.choice());
        s.commit(*choice,static_cast<unsigned int>(action-actions.begin()));
        return !s.failed();
      }
      static bool replay(Transition::Representation representation,
                         Transition::ResidualPropagation propagation=Transition::OFF) {
        const auto data=instance();
        Transition root(data,representation,propagation);
        Gecode::Search::Options options;
        Gecode::Search::Statistics stat;
        const bool lazy=representation==Transition::LAZY;
        if (root.status()!=SS_BRANCH || root.allocated_positions()!=(lazy ? 0 : 4))
          return false;
        std::unique_ptr<Transition> absent(static_cast<Transition*>(root.clone()));
        if (!root.prepare(stat,options) || root.allocated_positions()!=(lazy ? 1 : 4) ||
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
        if (!absent->prepare(stat,options) || absent->depth!=1 || absent->cost()!=1 ||
            absent->allocated_positions()!=(lazy ? 2 : 4) || absent->position(1).assigned())
          return false;
        original.reset(absent->choice());
        Archive prefix_archive; original->archive(prefix_archive);
        restored.reset(absent->choice(prefix_archive));
        next->commit(*restored,0);
        if (next->status()!=SS_BRANCH || next->depth!=2 || next->cost()!=2 ||
            next->time!=6 || next->position(1).val()!=2 || absent->position(1).assigned())
          return false;

        Transition optimum(data,representation,propagation);
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
      static bool continuation(Transition::Representation representation, int kind) {
        const auto data=instance();
        Transition root(data,representation), incumbent(data,representation);
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
          Gecode::Search::astarengine(&root,Transition::guidance(),options) : kind==3 ?
          Gecode::Search::columnengine(&root,Transition::guidance(),options) :
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
          Transition inferred(state.data,Transition::LAZY,propagation);
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
        Gecode::Search::Options options;
        if (!state.prepare(stat,options)) return false;
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
          if (!residuals(root,{},filtered,rejected)) return false;
        }
        return filtered && rejected;
      }
    public:
      TSPTWTransitionTest(void) : Base("Search::TSPTW::Transition", TestTags::check()) {}
      bool run(void) override {
        for (auto representation : {Transition::LAZY,Transition::EAGER}) {
          if (!replay(representation) || !replay(representation,Transition::CHEAP) ||
              !replay(representation,Transition::UNARY))
            return false;
          for (int kind : {1,2,3})
            if (!continuation(representation,kind)) return false;
        }
        return bounds_and_inference();
      }
    } tsptwTransitionTest;

    /// A tiny closed tree with a generated goal and deeper alternatives.
    class AStarSpace : public Space {
    public:
      static std::atomic<int> live;
      IntVar x, y;
      bool maximize, incomparable;
      AStarSpace(bool max=false, int fixed=-1, bool failed=false)
        : x(*this,0,3), y(*this,0,1), maximize(max), incomparable(false) {
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
        : Space(s), maximize(s.maximize), incomparable(s.incomparable) {
        live++;
        x.update(*this,s.x); y.update(*this,s.y);
      }
      ~AStarSpace(void) override { live--; }
      Space* copy(void) override { return new AStarSpace(*this); }
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
      SpaceComparison compare(const Space& s0) const override {
        const auto& s=dynamic_cast<const AStarSpace&>(s0);
        if (incomparable || maximize!=s.maximize) return SC_INCOMPARABLE;
        if (objective()==s.objective()) return SC_EQUIVALENT;
        return improves(objective(),s.objective()) ? SC_BETTER : SC_WORSE;
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
    // The parallel test runner shares this fixture's ownership counter.
    static std::mutex heuristic_ownership_mutex;

    /// Explicit finite paths with model-authored residual keys and labels.
    class PruningSpace : public Space {
    public:
      struct Node {
        int key, time, cost, bound, rank;
        std::vector<int> children;
        bool history=false;
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
          return !static_cast<const PruningSpace&>(s).value().children.empty();
        }
        const Choice* choice(Space& s) override {
          const auto& state=static_cast<const PruningSpace&>(s);
          return new Decision(*this,state.vertex,
            static_cast<unsigned int>(state.value().children.size()));
        }
        const Choice* choice(const Space&, Archive& e) override {
          int vertex; unsigned int n; e >> vertex >> n;
          return new Decision(*this,vertex,n);
        }
        ExecStatus commit(Space& s0, const Choice& c, unsigned int a) override {
          auto& s=static_cast<PruningSpace&>(s0);
          const auto& decision=static_cast<const Decision&>(c);
          s.vertex=(*s.graph)[decision.vertex].children[a];
          return s.value().cost>=s.ceiling ? ES_FAILED : ES_OK;
        }
        Actor* copy(Space& home) override { return new (home) Decisions(home,*this); }
        size_t dispose(Space& home) override {
          (void) Brancher::dispose(home); return sizeof(*this);
        }
      };
      explicit PruningSpace(Graph nodes, bool max=false)
        : graph(std::make_shared<const Graph>(std::move(nodes))), maximize(max) {
        (void) new (*this) Decisions(*this);
      }
      PruningSpace(PruningSpace& s)
        : Space(s), graph(s.graph), maximize(s.maximize), vertex(s.vertex),
          ceiling(s.ceiling) {}
      Space* copy(void) override { return new PruningSpace(*this); }
      const Node& value(void) const { return (*graph)[vertex]; }
      SpaceComparison compare(const Space& s0) const override {
        const int other=static_cast<const PruningSpace&>(s0).value().cost;
        return value().cost==other ? SC_EQUIVALENT :
          value().cost<other ? SC_BETTER : SC_WORSE;
      }
      void constrain(const Space& s0) override {
        ceiling=static_cast<const PruningSpace&>(s0).value().cost;
        if (value().cost>=ceiling) fail();
      }
      static Gecode::Search::HeuristicGuidance guidance(bool max=false) {
        using Guidance=Gecode::Search::HeuristicGuidance;
        // Maximizing the negated cost mirrors the same finite paths and cuts.
        return {max ? Guidance::MAXIMIZE : Guidance::MINIMIZE,
          [](const Space& s)->std::int64_t {
            const auto& state=static_cast<const PruningSpace&>(s);
            const std::int64_t bound=state.value().bound;
            return state.maximize ? -bound : bound;
          },[](const Space& s)->std::int64_t {
            return static_cast<const PruningSpace&>(s).value().rank;
          }};
      }
      static Gecode::Search::StatePruning pruning(unsigned long int limit=0) {
        using Pruning=Gecode::Search::StatePruning;
        return {[](const Space& s)->std::optional<Pruning::Descriptor> {
          const auto& node=static_cast<const PruningSpace&>(s).value();
          // The proposed key omits a path restriction in these nodes. Decline
          // instead of asserting that equal resource labels imply equal futures.
          if (node.history) return std::nullopt;
          return Pruning::Descriptor{0,{node.key},{node.time,node.cost}};
        },[](const Pruning::Descriptor& a, const Pruning::Descriptor& b) {
          if (a.key!=b.key) throw std::runtime_error("hash collision compared");
          if (a.label==b.label) return Pruning::EQUIVALENT;
          if (a.label[0]<=b.label[0] && a.label[1]<=b.label[1])
            return Pruning::DOMINATES;
          if (a.label[0]>=b.label[0] && a.label[1]>=b.label[1])
            return Pruning::DOMINATED;
          return Pruning::INCOMPARABLE;
        },limit};
      }
    };

    class StatePruningChecks {
      using Graph=PruningSpace::Graph;
      using Pruning=Gecode::Search::StatePruning;
      using Statistics=Gecode::Search::Statistics;
    public:
      static int reference(const Graph& graph, int vertex=0) {
        const auto& node=graph[vertex];
        if (node.children.empty()) return node.cost;
        int best=std::numeric_limits<int>::max();
        for (int child : node.children) best=std::min(best,reference(graph,child));
        return best;
      }
    private:
      static bool solve(const Graph& graph, int kind, Pruning pruning,
                        int optimum, bool stopped, Statistics& statistics,
                        unsigned long int width=0, std::vector<int>* costs=nullptr) {
        Gecode::Search::Options options; options.clone=false;
        std::vector<std::unique_ptr<PruningSpace>> retained;
        {
          // Builders copy the policy, including its forgetting ceiling.
          std::unique_ptr<Gecode::Search::Builder> builder(kind==2 ?
            Gecode::column<PruningSpace>(PruningSpace::guidance(),options,{},pruning) : kind==1 ?
            Gecode::cabs<PruningSpace>(PruningSpace::guidance(),options,{width,0},pruning) :
            Gecode::astar<PruningSpace>(PruningSpace::guidance(),options,pruning));
          std::unique_ptr<Gecode::Search::Engine> engine(
            (*builder)(new PruningSpace(graph)));
          while (std::unique_ptr<PruningSpace> s{
                   static_cast<PruningSpace*>(engine->next())}) {
            if (costs) costs->push_back(s->value().cost);
            retained.push_back(std::move(s));
          }
          statistics=engine->statistics();
          if (retained.empty() || retained.back()->value().cost!=optimum ||
              engine->stopped()!=stopped) return false;
          engine->reset(nullptr);
          const auto reset=engine->statistics();
          if (reset.state_pruned || reset.state_descriptors || reset.state_bytes ||
              engine->next() || engine->stopped()) return false;
        }
        // Each publication still owns its graph and Space after engine cleanup.
        for (const auto& solution : retained)
          if (solution->status()!=SS_SOLVED) return false;
        return true;
      }
    public:
      static bool semantics(int kind) {
        const Graph collision={
          {0,0,0,0,0,{1,2}}, {1,5,5,5,0,{3}}, {2,1,1,1,0,{4}},
          {3,15,15,15,0,{}}, {4,3,3,3,0,{}}};
        const Graph pareto={
          {0,0,0,0,0,{1,2,3,4}}, {1,5,5,5,0,{5}}, {1,5,5,5,0,{5}},
          {1,10,1,1,0,{6}}, {1,1,10,10,0,{7}},
          {2,15,15,15,0,{}}, {2,20,11,11,0,{}}, {2,11,20,20,0,{}}};
        const Graph history={
          {0,0,0,0,0,{1,2}}, {1,1,1,1,0,{3},true},
          {1,1,1,1,0,{4},true}, {2,50,50,50,0,{}}, {2,10,10,10,0,{}}};
        for (const auto& graph : {collision,pareto,history}) {
          Statistics off, on, forgotten;
          if (!solve(graph,kind,{},reference(graph),false,off) ||
              off.state_pruned || off.state_descriptors || off.state_bytes ||
              !solve(graph,kind,PruningSpace::pruning(),reference(graph),false,on) ||
              !solve(graph,kind,PruningSpace::pruning(1),reference(graph),false,forgotten) ||
              forgotten.state_descriptors>1) return false;
          if (graph.size()==pareto.size() && !on.state_pruned) return false;
          if (graph[1].history && on.state_descriptors) return false;
          if (kind==1) {
            const auto bytes=Pruning::Descriptor{0,{1},{0,0}}.storage();
            const unsigned long int peak=graph[1].history ? 0 :
              graph.size()==pareto.size() ? 3 : 2;
            if (on.state_descriptors!=peak || on.state_bytes!=peak*bytes ||
                forgotten.state_descriptors!=(peak ? 1UL : 0UL) ||
                forgotten.state_bytes!=(peak ? bytes : 0)) return false;
          }
        }
        // Misconfigured declarations fail before clone=false transfers a root.
        PruningSpace root(collision);
        Gecode::Search::Options options; options.clone=false;
        auto missing=PruningSpace::pruning(); missing.compare={};
        try {
          std::unique_ptr<Gecode::Search::Engine> invalid(kind==2 ?
            Gecode::Search::columnengine(&root,PruningSpace::guidance(),options,{},missing) : kind==1 ?
            Gecode::Search::cabsengine(&root,PruningSpace::guidance(),options,{},missing) :
            Gecode::Search::astarengine(&root,PruningSpace::guidance(),options,missing));
          return false;
        } catch (const Gecode::Search::UninitializedStatePruning&) {}
        return root.status()==SS_BRANCH;
      }
      static bool reopen(int kind=0) {
        // f falls from 5 to 2 along the detour: admissible but inconsistent.
        // The first label has already expanded and published 20 before 12.
        const Graph graph={
          {0,0,0,0,0,{1,2}}, {7,10,10,0,0,{3}}, {8,0,0,5,0,{4}},
          {9,20,20,20,0,{}}, {7,2,2,2,0,{5}}, {9,12,12,12,0,{}}};
        for (unsigned long int limit : {0UL,1UL}) {
          Statistics statistics; std::vector<int> costs;
          if (!solve(graph,kind,PruningSpace::pruning(limit),reference(graph),
                     false,statistics,0,&costs) || costs!=std::vector<int>({20,12}))
            return false;
        }
        return true;
      }
      static bool candidate_lifetime(void) {
        // Arrival 1 is evicted by 2; arrival 3 is beam-discarded. Neither may
        // suppress later arrival 4 with the same exact key and a worse label.
        const Graph discarded={
          {0,0,0,0,0,{1,2,3,4}}, {1,1,1,1,0,{5}}, {2,0,0,0,-1,{6}},
          {1,0,0,0,10,{7}}, {1,2,2,2,-2,{8}},
          {3,11,11,11,0,{}}, {4,100,100,100,0,{}},
          {3,10,10,10,0,{}}, {3,12,12,12,0,{}}};
        Statistics capped, complete;
        if (!solve(discarded,true,PruningSpace::pruning(),12,true,capped,1) ||
            !solve(discarded,true,PruningSpace::pruning(),reference(discarded),
                   false,complete) || !complete.restart) return false;
        // A worse-ranked dominator must take the dominated live candidate's
        // slot. Sound removal causes no beam loss, so width one proves 10.
        const Graph replacement={
          {0,0,0,0,0,{1,2}}, {1,1,1,1,0,{3}}, {1,0,0,0,10,{4}},
          {2,11,11,11,0,{}}, {2,10,10,10,0,{}}};
        const auto bytes=Pruning::Descriptor{0,{1},{0,0}}.storage();
        if (!solve(replacement,true,PruningSpace::pruning(),reference(replacement),
                   false,complete,1) || complete.state_pruned!=1 || complete.restart ||
            complete.state_descriptors!=1 || complete.state_bytes!=bytes) return false;
        // Widening retains two, then three incomparable labels. One dominator
        // removes them together; accounting must release every victim's bytes.
        const Graph multiple={
          {0,0,0,0,0,{1,2,3,4}}, {1,5,5,0,0,{5}}, {1,10,1,0,0,{6}},
          {1,1,9,0,0,{7}}, {1,0,0,0,10,{8}},
          {2,15,15,15,0,{}}, {2,20,11,11,0,{}},
          {2,11,20,20,0,{}}, {2,10,10,10,0,{}}};
        return solve(multiple,true,PruningSpace::pruning(),reference(multiple),
                     false,complete) && complete.state_pruned==6 && complete.restart==2 &&
          complete.state_descriptors==3 && complete.state_bytes==3*bytes;
      }
    };

    class AStarTest : public Base {
      static bool ordering(void) {
        using namespace Gecode::Search;
        // Every root alternative needs another commit. The first arrival has
        // the preferred rank but the worse bound; arrivals 2 and 3 have the
        // same residual problem, label, and remaining transition count.
        const PruningSpace::Graph graph={
          {0,0,0,0,0,{1,2,3}}, {1,0,0,8,-100,{4}},
          {2,0,0,4,100,{5}}, {2,0,0,4,100,{5}},
          {3,0,10,10,0,{}}, {3,0,5,5,0,{}}};
        auto solve=[&](bool max, BestFirstOrder order, bool ranked, bool prune) {
          Gecode::Search::Options options; options.clone=false;
          auto guidance=PruningSpace::guidance(max);
          if (!ranked) guidance.rank={};
          std::unique_ptr<Builder> builder(Gecode::bestfirst<PruningSpace>(
            guidance,order,options,prune ? PruningSpace::pruning() : StatePruning()));
          if (!builder->best()) return false;
          std::unique_ptr<Engine> engine((*builder)(new PruningSpace(graph,max)));
          std::vector<int> costs;
          while (std::unique_ptr<PruningSpace> solution{
                   static_cast<PruningSpace*>(engine->next())})
            costs.push_back(solution->value().cost);
          const std::vector<int> expected=order==RANK_FIRST && ranked ?
            std::vector<int>{10,5} : std::vector<int>{5};
          return costs==expected && !engine->stopped() &&
            costs.back()==StatePruningChecks::reference(graph) &&
            engine->statistics().state_pruned==(prune ? 1UL : 0UL);
        };
        for (bool max : {false,true})
          if (!solve(max,BOUND_FIRST,true,false) ||
              !solve(max,RANK_FIRST,true,false) ||
              !solve(max,RANK_FIRST,true,true)) return false;
        return solve(false,RANK_FIRST,false,false);
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
        std::unique_ptr<Gecode::Search::Builder> builder(
          Gecode::astar<AStarSpace>(AStarSpace::guidance(max,true),o));
        if (!builder->best()) return false;
        std::unique_ptr<Gecode::Search::Engine> engine(
          (*builder)(new AStarSpace(max)));
        std::unique_ptr<Space> first(engine->next());
        if (!first) return false;
        if (queued) {
          std::unique_ptr<AStarSpace> second(
            static_cast<AStarSpace*>(engine->next()));
          if (!second || second->x.val()!=1) return false;
        }
        AStarSpace incumbent(max,queued ? 2 : 1), worse(max,0);
        incumbent.status(); worse.status();
        engine->constrain(incumbent);
        engine->constrain(incumbent);
        engine->constrain(worse);
        unsigned int found=0;
        while (std::unique_ptr<AStarSpace> solution{
                 static_cast<AStarSpace*>(engine->next())}) {
          if (!solution->improves(solution->objective(),incumbent.objective()) ||
              solution->x.val()!=2) return false;
          found++;
        }
        if (engine->stopped() || found!=(queued ? 0U : 1U)) return false;
        worse.incomparable=true;
        try {
          engine->constrain(worse);
          return false;
        } catch (const Gecode::Search::Incomparable&) {}
        return true;
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
        o.clone=false; o.threads=2;
        try {
          Gecode::AStar<AStarSpace> invalid(&root,AStarSpace::guidance(false),o);
          return false;
        } catch (const Gecode::Search::UnsupportedAStarThreads&) {}
        o.threads=1;
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
              !adoption(max,false) || !adoption(max,true)) return false;
        if (!ordering() || !ownership() || AStarSpace::live.load()!=0 ||
            !StatePruningChecks::semantics(false) || !StatePruningChecks::reopen())
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
      SpaceComparison compare(const Space& s0) const override {
        const auto& s=dynamic_cast<const CABSFeasibility&>(s0);
        if (x.val()==s.x.val()) return SC_EQUIVALENT;
        return x.val()<s.x.val() ? SC_BETTER : SC_WORSE;
      }
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
      static bool discarded_bounds(bool max) {
        Gecode::Search::Options options; options.clone=false;
        auto solve=[&](const PruningSpace::Graph& graph, unsigned long int width,
                      int cost, bool stopped, unsigned long int restarts) {
          Gecode::CABS<PruningSpace> engine(new PruningSpace(graph,max),
            PruningSpace::guidance(max),options,{width,0});
          int previous=std::numeric_limits<int>::max();
          bool found=false;
          while (std::unique_ptr<PruningSpace> solution{engine.next()}) {
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
          const PruningSpace::Graph bounded={
            {0,0,0,0,0,evict ? std::vector<int>{2,1} : std::vector<int>{1,2}},
            {1,0,0,0,0,{3}}, {2,0,0,bound,1,{4}},
            {3,10,10,10,0,{}}, {4,bound,bound,bound,0,{}}};
          if (!solve(bounded,1,StatePruningChecks::reference(bounded),false,0))
            return false;
          // Ranking keeps bound 5 over bound 0; the retained bound then falls
          // to 1. A later discard at bound 20 must not erase the earlier 0.
          const PruningSpace::Graph improving={
            {0,0,0,0,0,evict ? std::vector<int>{2,1} : std::vector<int>{1,2}},
            {1,0,0,5,0,{3,4}}, {2,0,0,0,1,{5}},
            {3,0,0,1,0,{6}}, {4,0,0,20,1,{7}},
            {5,4,4,4,0,{}}, {6,10,10,10,0,{}}, {7,20,20,20,0,{}}};
          if (!solve(improving,1,10,true,0) ||
              !solve(improving,0,StatePruningChecks::reference(improving),false,1))
            return false;
        }
        return true;
      }
      static bool interrupted_discard(bool external) {
        // Width loss precedes a shallow goal. The original fourth alternative
        // and the retained parent both improve it and survive publication.
        const PruningSpace::Graph graph={
          {0,0,0,0,0,{1,2,3,4}}, {1,0,0,0,0,{5}}, {2,0,0,7,1,{6}},
          {3,10,10,10,0,{}}, {4,8,8,8,0,{}},
          {5,6,6,6,0,{}}, {6,7,7,7,0,{}}};
        Gecode::Search::NodeStop stop(10000);
        Gecode::Search::Options options; options.clone=false; options.stop=&stop;
        Gecode::CABS<PruningSpace> engine(new PruningSpace(graph),
          PruningSpace::guidance(),options,{1,0});
        std::unique_ptr<PruningSpace> first(engine.next());
        if (!first || first->value().cost!=10) return false;
        const auto before=engine.statistics().node;
        stop.limit(before-1);
        for (int i=0;i<2;i++)
          if (engine.next() || !engine.stopped() ||
              engine.statistics().node!=before) return false;
        if (external) {
          PruningSpace incumbent(graph);
          incumbent.vertex=5;
          if (incumbent.status()!=SS_SOLVED) return false;
          engine.constrain(incumbent);
        }
        stop.limit(10000);
        std::vector<int> costs;
        while (std::unique_ptr<PruningSpace> solution{engine.next()})
          costs.push_back(solution->value().cost);
        return costs==(external ? std::vector<int>{} : std::vector<int>{8,6}) &&
          !engine.stopped() && !engine.statistics().restart &&
          engine.statistics().node==before+2;
      }
      static bool reset_discard(void) {
        const PruningSpace::Graph bounded={
          {0,0,0,0,0,{1,2}}, {1,0,0,0,0,{3}}, {2,0,0,20,1,{4}},
          {3,10,10,10,0,{}}, {4,20,20,20,0,{}}};
        auto improving=bounded;
        improving[2].bound=0;
        improving[4].cost=improving[4].bound=4;
        Gecode::Search::Options options; options.clone=false;
        for (unsigned long int width : {1UL,2UL}) {
          std::unique_ptr<Gecode::Search::Engine> engine(
            Gecode::Search::cabsengine(new PruningSpace(bounded),
              PruningSpace::guidance(),options,{width,0}));
          for (int run=0;run<3;run++) {
            // Repeat widening after reset to check that width starts at one.
            const bool unresolved=run==1 || (run==2 && width==2);
            if (run) engine->reset(new PruningSpace(unresolved ? improving : bounded));
            if (engine->stopped() || engine->statistics().node ||
                engine->statistics().restart) return false;
            std::vector<int> costs;
            while (std::unique_ptr<PruningSpace> solution{
                     static_cast<PruningSpace*>(engine->next())})
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
        std::unique_ptr<Gecode::Search::Builder> builder(
          Gecode::cabs<AStarSpace>(AStarSpace::guidance(false,true),o));
        if (!builder->best()) return false;
        std::unique_ptr<Gecode::Search::Engine> engine((*builder)(new AStarSpace));
        std::unique_ptr<Space> first(engine->next());
        if (!first) return false;
        if (queued) {
          std::unique_ptr<AStarSpace> second(
            static_cast<AStarSpace*>(engine->next()));
          if (!second || second->x.val()!=1) return false;
        }
        {
          AStarSpace incumbent(false,queued ? 2 : 1), worse(false,0);
          incumbent.status(); worse.status();
          engine->constrain(incumbent);
          engine->constrain(incumbent);
          engine->constrain(worse);
          worse.incomparable=true;
          try { engine->constrain(worse); return false; }
          catch (const Gecode::Search::Incomparable&) {}
        }
        // The accepted external copy survives the caller's destruction. Its
        // exact INT64_MIN objective also bounds out every discarded candidate;
        // a weaker adoption still lets the original alternatives recover x=2.
        std::unique_ptr<AStarSpace> optimum(
          static_cast<AStarSpace*>(engine->next()));
        return (queued ? !optimum : optimum && optimum->x.val()==2) &&
          !engine->next() && !engine->stopped() &&
          (!queued || engine->statistics().restart==0);
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
        o.threads=2;
        try {
          Gecode::CABS<AStarSpace> invalid(&root,AStarSpace::guidance(false),o);
          return false;
        } catch (const Gecode::Search::UnsupportedCABSThreads&) {}
        o.threads=1;
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
          reset_discard() && feasibility() && adoption(false) && adoption(true) &&
          ownership_and_limits() && AStarSpace::live.load()==0 &&
          StatePruningChecks::semantics(true) && StatePruningChecks::candidate_lifetime();
      }
    } cabsTest;

    class ColumnTest : public Base {
      static bool solutions(bool max, bool ranked, bool interrupt) {
        Gecode::Search::NodeStop stop(10000);
        Gecode::Search::Options options; options.clone=false; options.stop=&stop;
        auto guidance=AStarSpace::guidance(max,false);
        if (!ranked) guidance.rank={};
        Gecode::Column<AStarSpace> engine(new AStarSpace(max),guidance,options);
        std::vector<int> values;
        std::int64_t previous=0;
        while (std::unique_ptr<AStarSpace> s{engine.next()}) {
          if (!values.empty() && !s->improves(s->objective(),previous)) return false;
          previous=s->objective(); values.push_back(s->x.val());
          if (interrupt) {
            const auto before=engine.statistics().node;
            stop.limit(before-1);
            if (engine.next() || !engine.stopped() ||
                engine.statistics().node!=before) return false;
            stop.limit(10000);
          }
        }
        const std::vector<int> expected=ranked ? std::vector<int>{0,1,2} :
          std::vector<int>{0,2};
        return values==expected && !engine.stopped() &&
          engine.statistics().depth==2;
      }
      static bool cycles(unsigned long int growth, bool interrupt) {
        // Each depth-one parent publishes then creates a depth-two parent.
        // A full cycle must visit that new depth before revisiting depth one.
        const PruningSpace::Graph graph={
          {0,0,0,0,0,{1,2,3,4}},
          {1,0,0,0,0,{5,9}}, {2,0,0,0,0,{6,10}},
          {3,0,0,0,0,{7,11}}, {4,0,0,0,0,{8,12}},
          {5,100,100,100,0,{}}, {6,80,80,80,0,{}},
          {7,70,70,70,0,{}}, {8,40,40,40,0,{}},
          {9,0,0,0,0,{13}}, {10,0,0,0,0,{14}},
          {11,0,0,0,0,{15}}, {12,0,0,0,0,{16}},
          {13,90,90,90,0,{}}, {14,60,60,60,0,{}},
          {15,50,50,50,0,{}}, {16,30,30,30,0,{}}};
        Gecode::Search::NodeStop stop(10000);
        Gecode::Search::Options options; options.clone=false; options.stop=&stop;
        Gecode::Column<PruningSpace> engine(new PruningSpace(graph),
          PruningSpace::guidance(),options,{1,growth,0});
        std::vector<int> costs;
        while (std::unique_ptr<PruningSpace> s{engine.next()}) {
          costs.push_back(s->value().cost);
          if (interrupt) {
            const auto before=engine.statistics().node;
            stop.limit(before-1);
            if (engine.next() || !engine.stopped() ||
                engine.statistics().node!=before) return false;
            stop.limit(10000);
          }
        }
        const std::vector<int> expected=growth==0 ?
          std::vector<int>{100,90,80,60,50,40,30} : growth==1 ?
          std::vector<int>{100,90,80,70,60,50,40,30} :
          std::vector<int>{100,90,80,70,40,30};
        return costs==expected && !engine.stopped() && !engine.statistics().restart;
      }
      static bool stale_frontier(void) {
        // The root queues four obsolete parents and one improving parent
        // before publishing. Ranking puts the obsolete entries first.
        const PruningSpace::Graph graph={
          {0,0,0,0,0,{1,2,3,4,5,6}},
          {1,10,10,10,0,{8}}, {2,10,10,10,0,{8}},
          {3,10,10,10,0,{8}}, {4,10,10,10,0,{8}},
          {5,0,0,0,1,{9}}, {6,20,20,20,0,{}},
          {7,7,7,7,0,{}}, {8,10,10,10,0,{}}, {9,5,5,5,0,{}}};
        Gecode::Search::Statistics uninterrupted;
        for (bool interrupt : {false,true}) {
          Gecode::Search::FailStop stop(10000);
          Gecode::Search::Options options; options.clone=false; options.stop=&stop;
          Gecode::Column<PruningSpace> engine(new PruningSpace(graph),
            PruningSpace::guidance(),options);
          std::unique_ptr<PruningSpace> first(engine.next());
          if (!first || first->value().cost!=20) return false;
          PruningSpace incumbent(graph);
          incumbent.vertex=7;
          if (incumbent.status()!=SS_SOLVED) return false;
          engine.constrain(incumbent);
          if (interrupt) {
            const auto before=engine.statistics();
            stop.limit(before.fail+1);
            if (engine.next() || !engine.stopped() ||
                engine.statistics().fail!=before.fail+2 ||
                engine.statistics().node!=before.node) return false;
            stop.limit(10000);
          }
          std::unique_ptr<PruningSpace> optimum(engine.next());
          if (!optimum || optimum->value().cost!=5 || engine.next() ||
              engine.stopped()) return false;
          const auto after=engine.statistics();
          if (interrupt) {
            if (after.fail!=uninterrupted.fail ||
                after.node!=uninterrupted.node ||
                after.depth!=uninterrupted.depth) return false;
          } else {
            uninterrupted=after;
          }
        }
        return true;
      }
      static bool adoption(bool max) {
        Gecode::Search::Options options; options.clone=false;
        std::unique_ptr<Gecode::Search::Builder> builder(
          Gecode::column<AStarSpace>(AStarSpace::guidance(max,true),options));
        if (!builder->best()) return false;
        std::unique_ptr<Gecode::Search::Engine> engine((*builder)(new AStarSpace(max)));
        std::unique_ptr<Space> first(engine->next());
        if (!first) return false;
        {
          AStarSpace incumbent(max,1), worse(max,0);
          incumbent.status(); worse.status();
          engine->constrain(incumbent); engine->constrain(incumbent);
          engine->constrain(worse);
          worse.incomparable=true;
          try { engine->constrain(worse); return false; }
          catch (const Gecode::Search::Incomparable&) {}
        }
        std::unique_ptr<AStarSpace> optimum(static_cast<AStarSpace*>(engine->next()));
        return optimum && optimum->x.val()==2 && !engine->next() && !engine->stopped();
      }
      static bool ownership_and_limits(void) {
        using namespace Gecode::Search;
        Gecode::Search::Options options;
        for (bool clone : {false,true}) {
          options.clone=clone;
          for (int state : {-2,-1,0}) {
            std::unique_ptr<AStarSpace> root(new AStarSpace(false,state,state==-2));
            std::unique_ptr<Space> result;
            {
              Gecode::Column<AStarSpace> engine(root.get(),
                AStarSpace::guidance(false,true),options);
              if (!clone) root.release();
              result.reset(engine.next());
              if ((state!=-2)!=static_cast<bool>(result)) return false;
              if (state==0 && (engine.next() || engine.stopped())) return false;
            }
            if (result && result->status()!=SS_SOLVED) return false;
            result.reset(); root.reset();
            if (AStarSpace::live.load()) return false;
          }
        }
        options.clone=false;
        std::unique_ptr<Gecode::Search::Engine> capped(columnengine(new AStarSpace,
          AStarSpace::guidance(false,true),options,{1,1,1}));
        std::unique_ptr<Space> first(capped->next());
        if (!first || capped->next() || !capped->stopped()) return false;
        const auto before=capped->statistics().node;
        if (capped->next() || !capped->stopped() ||
            capped->statistics().node!=before) return false;
        capped->reset(new AStarSpace(false,2));
        std::unique_ptr<Space> optimum(capped->next());
        if (!optimum || capped->stopped() || capped->statistics().node!=1 ||
            capped->next()) return false;
        capped->reset(nullptr);
        if (capped->next() || capped->stopped() || capped->nogoods().ng()) return false;
        // The last child replaces its fully expanded parent in the cap count.
        const PruningSpace::Graph chain={
          {0,0,0,0,0,{1}}, {1,0,0,0,0,{2}}, {2,0,0,0,0,{3}},
          {3,3,3,3,0,{}}};
        Gecode::Column<PruningSpace> unary(new PruningSpace(chain),
          PruningSpace::guidance(),options,{1,1,1});
        std::unique_ptr<PruningSpace> solution(unary.next());
        if (!solution || solution->value().cost!=3 || unary.next() || unary.stopped())
          return false;
        Gecode::Column<CABSFeasibility> failed(new CABSFeasibility(true),
          CABSFeasibility::guidance(),options);
        if (failed.next() || failed.stopped()) return false;
        Gecode::Column<CABSFeasibility> feasible(new CABSFeasibility,
          CABSFeasibility::guidance(),options);
        std::unique_ptr<CABSFeasibility> witness(feasible.next());
        return witness && witness->x.val()==1 && !feasible.next() && !feasible.stopped();
      }
      static bool diagnostics(void) {
        using namespace Gecode::Search;
        AStarSpace root;
        Gecode::Search::Options options; options.clone=false; options.threads=2;
        auto guidance=AStarSpace::guidance(false);
        try { Gecode::Column<AStarSpace> invalid(&root,guidance,options); return false; }
        catch (const UnsupportedColumnThreads&) {}
        options.threads=1; options.tracer=&Gecode::StdSearchTracer::def;
        try { Gecode::Column<AStarSpace> invalid(&root,guidance,options); return false; }
        catch (const UnsupportedColumnTracer&) {}
        options.tracer=nullptr;
        try { Gecode::Column<AStarSpace> invalid(&root,guidance,options,{0,1,0}); return false; }
        catch (const InvalidColumnQuota&) {}
        guidance.bound={};
        try { Gecode::Column<AStarSpace> invalid(&root,guidance,options); return false; }
        catch (const UninitializedColumnBound&) {}
        options.clone=true;
        guidance.bound=[](const Space&)->std::int64_t {
          throw std::runtime_error("guidance failure");
        };
        {
          Gecode::Column<AStarSpace> engine(&root,guidance,options);
          try { delete engine.next(); return false; }
          catch (const std::runtime_error&) {}
        }
        return AStarSpace::live.load()==1 && root.status()==SS_BRANCH;
      }
    public:
      ColumnTest(void) : Base("Search::Column", TestTags::check()) {}
      bool run(void) override {
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (bool max : {false,true})
          if (!solutions(max,true,true) || !solutions(max,false,false) ||
              !adoption(max)) return false;
        for (bool interrupt : {false,true})
          for (unsigned long int growth : {0UL,1UL,
                 std::numeric_limits<unsigned long int>::max()})
            if (!cycles(growth,interrupt)) return false;
        return stale_frontier() && ownership_and_limits() &&
          AStarSpace::live.load()==0 &&
          diagnostics() && AStarSpace::live.load()==0 &&
          StatePruningChecks::semantics(2) && StatePruningChecks::reopen(2);
      }
    } columnTest;

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
      Comparison(void) : Base("Search::Comparison", TestTags::check()) {}
      virtual bool run(void) {
        MinObjective one(1,1), two(2,2), one_again(1,1), open(0,2);
        if ((one.compare(two) != SC_BETTER) ||
            (two.compare(one) != SC_WORSE) ||
            (one.compare(one_again) != SC_EQUIVALENT))
          return false;
        MaxObjective high(2), low(1);
        if ((high.compare(low) != SC_BETTER) ||
            (low.compare(high) != SC_WORSE))
          return false;
        try {
          (void) open.compare(one);
          return false;
        } catch (const Int::ValOfUnassignedVar&) {}
        try {
          (void) one.compare(high);
          return false;
        } catch (const DynamicCastFailed&) {}
        PlainSpace plain;
        try {
          (void) plain.compare(plain);
          return false;
        } catch (const SpaceNoComparison&) {}
        return true;
      }
    };

    Comparison comparison;

    /// Brancher used to exercise internal post-propagation feedback
    class FeedbackBrancher : public Brancher {
    private:
      class FeedbackChoice : public Gecode::Search::BranchFeedbackChoice {
      public:
        int pos;
        FeedbackChoice(const Brancher& brancher,
                       const Gecode::Search::BranchFeedback& feedback,
                       int pos0)
          : BranchFeedbackChoice(brancher,2U,feedback,
                                 static_cast<unsigned int>(pos0)),
            pos(pos0) {}
        void archive(Archive& archive) const override {
          Choice::archive(archive);
          archive << pos;
        }
      };
      ViewArray<IntView> x;
      Gecode::Search::BranchFeedback feedback;
      mutable int start;
      int throwing_alternative;

      FeedbackBrancher(Home home, ViewArray<IntView>& x0,
                       const Gecode::Search::BranchFeedback& feedback0,
                       int throwing_alternative0)
        : Brancher(home), x(x0), feedback(feedback0), start(0),
          throwing_alternative(throwing_alternative0) {
        home.notice(*this,AP_DISPOSE,true);
      }
      FeedbackBrancher(Space& home, FeedbackBrancher& brancher)
        : Brancher(home,brancher), feedback(brancher.feedback),
          start(brancher.start),
          throwing_alternative(brancher.throwing_alternative) {
        x.update(home,brancher.x);
      }
    public:
      static void post(Home home, const IntVarArgs& x,
                       const Gecode::Search::BranchFeedback& feedback,
                       int throwing_alternative) {
        ViewArray<IntView> views(home,x);
        (void) new (home)
          FeedbackBrancher(home,views,feedback,throwing_alternative);
      }
      bool status(const Space&) const override {
        while ((start < x.size()) && x[start].assigned())
          start++;
        return start < x.size();
      }
      const Choice* choice(Space&) override {
        return new FeedbackChoice(*this,feedback,start);
      }
      const Choice* choice(const Space&, Archive& archive) override {
        int pos;
        archive >> pos;
        return new FeedbackChoice(*this,feedback,pos);
      }
      ExecStatus commit(Space& home, const Choice& choice0,
                        unsigned int alternative) override {
        const FeedbackChoice& choice =
          static_cast<const FeedbackChoice&>(choice0);
        if (throwing_alternative == static_cast<int>(alternative))
          throw std::runtime_error("branch feedback fixture commit");
        if ((choice.pos == x.size()-1) && (alternative == 1U))
          return ES_FAILED;
        return me_failed(x[choice.pos].eq(home,static_cast<int>(alternative)))
          ? ES_FAILED : ES_OK;
      }
      Actor* copy(Space& home) override {
        return new (home) FeedbackBrancher(home,*this);
      }
      size_t dispose(Space& home) override {
        home.ignore(*this,AP_DISPOSE,true);
        feedback.~BranchFeedback();
        (void) Brancher::dispose(home);
        return sizeof(*this);
      }
    };

    /// Small complete tree for branch-feedback fixtures
    class FeedbackSpace : public Space {
    public:
      IntVarArray x;
      FeedbackSpace(unsigned int size,
                    const Gecode::Search::BranchFeedback& feedback,
                    int throwing_alternative=-1)
        : x(*this,static_cast<int>(size),0,1) {
        FeedbackBrancher::post(*this,x,feedback,throwing_alternative);
      }
      FeedbackSpace(FeedbackSpace& space) : Space(space) {
        x.update(*this,space.x);
      }
      Space* copy(void) override {
        return new FeedbackSpace(*this);
      }
      void constrain(const Space&) override {}
      // This fixture enumerates a complete tree without objective pruning.
      SpaceComparison compare(const Space&) const override {
        return SC_BETTER;
      }
      bool master(const MetaInfo& info) override {
        if ((info.type() == MetaInfo::RESTART) && (info.last() != nullptr)) {
          const FeedbackSpace& last =
            static_cast<const FeedbackSpace&>(*info.last());
          BoolVarArgs same;
          for (int i=0; i<x.size(); i++)
            same << expr(*this,x[i] == last.x[i].val());
          rel(*this,BOT_AND,same,0);
        }
        return false;
      }
      unsigned int assigned(void) const {
        unsigned int count = 0U;
        for (int i=0; i<x.size(); i++)
          if (x[i].assigned())
            count++;
        return count;
      }
    };

    /// Native publication, final-alternative, archive and ownership feedback checks
    class BranchFeedback : public Base {
    private:
      class Recorder : public Gecode::Search::BranchFeedback::Receiver {
      private:
        std::mutex mutex;
        unsigned int count[8][2][3];
        bool invalid;
        bool* destroyed;
      public:
        explicit Recorder(bool* destroyed0=nullptr)
          : invalid(false), destroyed(destroyed0) {
          for (unsigned int i=0U; i<8U; i++)
            for (unsigned int j=0U; j<2U; j++)
              for (unsigned int k=0U; k<3U; k++)
                count[i][j][k] = 0U;
        }
        ~Recorder(void) override {
          if (destroyed != nullptr)
            *destroyed = true;
        }
        void observe(unsigned int id, unsigned int alternative,
                     const Space& space,
                     Gecode::Search::BranchOutcome outcome) noexcept override {
          const FeedbackSpace* feedback_space =
            dynamic_cast<const FeedbackSpace*>(&space);
          std::lock_guard<std::mutex> lock(mutex);
          const unsigned int expected_assigned =
            (outcome == Gecode::Search::BranchOutcome::FAILED) ? id : id+1U;
          if ((feedback_space == nullptr) || (id >= 8U) ||
              (alternative >= 2U) ||
              (feedback_space->assigned() < expected_assigned)) {
            invalid = true;
            return;
          }
          unsigned int outcome_index;
          switch (outcome) {
          case Gecode::Search::BranchOutcome::FAILED:
            outcome_index = 0U; break;
          case Gecode::Search::BranchOutcome::SOLVED:
            outcome_index = 1U; break;
          case Gecode::Search::BranchOutcome::BRANCH:
            outcome_index = 2U; break;
          default:
            invalid = true; return;
          }
          count[id][alternative][outcome_index]++;
        }
        bool expected(unsigned int size, unsigned int repetitions=1U) {
          std::lock_guard<std::mutex> lock(mutex);
          if (invalid)
            return false;
          for (unsigned int id=0U; id<size; id++) {
            const unsigned int branches = repetitions * (1U << id);
            if (id+1U == size) {
              if ((count[id][0][0] != 0U) ||
                  (count[id][0][1] != branches) ||
                  (count[id][0][2] != 0U) ||
                  (count[id][1][0] != branches) ||
                  (count[id][1][1] != 0U) ||
                  (count[id][1][2] != 0U))
                return false;
            } else {
              for (unsigned int alternative=0U; alternative<2U;
                   alternative++)
                if ((count[id][alternative][0] != 0U) ||
                    (count[id][alternative][1] != 0U) ||
                    (count[id][alternative][2] != branches))
                  return false;
            }
          }
          return true;
        }
        unsigned int observations(void) {
          std::lock_guard<std::mutex> lock(mutex);
          unsigned int total = 0U;
          for (unsigned int i=0U; i<8U; i++)
            for (unsigned int j=0U; j<2U; j++)
              for (unsigned int k=0U; k<3U; k++)
                total += count[i][j][k];
          return total;
        }
        bool valid(void) {
          std::lock_guard<std::mutex> lock(mutex);
          return !invalid;
        }
      };

      // Exercise the final alternative after publication, including a commit
      // exception after the selected parent has transferred to that child.
      static bool native(unsigned int kind, bool throwing) {
        bool destroyed = false;
        Recorder* recorder = new Recorder(&destroyed);
        Gecode::Search::BranchFeedback feedback(recorder);
        bool threw = false;
        unsigned int solutions = 0U;
        {
          FeedbackSpace root(1U,feedback,throwing ? 1 : -1);
          Gecode::Search::Options options;
          options.threads = 1.0;
          Gecode::Search::HeuristicGuidance guidance(
            Gecode::Search::HeuristicGuidance::MINIMIZE,
            [](const Space& space) -> std::int64_t {
              return static_cast<const FeedbackSpace&>(space).assigned() ? 0 : -1;
            });
          std::unique_ptr<Gecode::Search::Engine> search;
          switch (kind) {
          case 0U:
            search.reset(Gecode::Search::astarengine(&root,guidance,options));
            break;
          case 1U:
            search.reset(Gecode::Search::cabsengine(&root,guidance,options));
            break;
          case 2U:
            search.reset(Gecode::Search::columnengine(&root,guidance,options));
            break;
          default:
            GECODE_NEVER;
          }
          try {
            while (std::unique_ptr<Space> solution{search->next()})
              solutions++;
          } catch (const std::runtime_error&) {
            threw = true;
          }
          if ((solutions != 1U) || (threw != throwing) ||
              (throwing ? recorder->observations() != 1U :
               !recorder->expected(1U)) || search->stopped())
            return false;
        }
        feedback = Gecode::Search::BranchFeedback();
        return destroyed;
      }

      static bool archive_compatible(void) {
        Recorder* recorder = new Recorder;
        Gecode::Search::BranchFeedback feedback(recorder);
        FeedbackSpace root(1U,feedback);
        if (root.status() != SS_BRANCH)
          return false;
        FeedbackSpace* clone = static_cast<FeedbackSpace*>(root.clone());
        const Choice* choice = root.choice();
        Archive archive;
        choice->archive(archive);
        const Choice* restored = clone->choice(archive);
        clone->commit(*restored,0U);
        delete restored;
        delete choice;
        const bool compatible = (clone->status() == SS_SOLVED) &&
          (recorder->observations() == 0U);
        delete clone;
        return compatible;
      }
    public:
      BranchFeedback(void) : Base("Search::BranchFeedback", TestTags::check()) {}
      bool run(void) override {
        return archive_compatible() &&
          native(0U,false) && native(0U,true) &&
          native(1U,false) && native(1U,true) &&
          native(2U,false) && native(2U,true);
      }
    };

    BranchFeedback branch_feedback;
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
      bool native(unsigned int kind, unsigned int node, bool stopping) {
        Gecode::Search::WorkerControl control(node == 0U ? 0U : 1U);
        Pause pause(control,node);
        Gecode::Search::Options options;
        options.threads=1; options.worker_control=control; options.stop=&pause;
        AStarSpace root;
        const auto guidance=AStarSpace::guidance(false);
        std::unique_ptr<Gecode::Search::Engine> engine(kind==0U ?
          Gecode::Search::astarengine(&root,guidance,options) : kind==1U ?
          Gecode::Search::cabsengine(&root,guidance,options) :
          Gecode::Search::columnengine(&root,guidance,options));
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
          control.request(1U);
          consume();
        } else {
          control.request(1U);
          result.get();
        }
        return paused && found && optimum==root.score(2) && !engine->stopped();
      }
    public:
      WorkerControlSequentialPause(void)
        : Base("Search::WorkerControl::SequentialPause", TestTags::check()) {}
      bool run(void) override {
#ifdef GECODE_HAS_THREADS
        std::lock_guard<std::mutex> lock(heuristic_ownership_mutex);
        for (unsigned int kind : {0U,1U,2U})
          if (!native(kind,0U,false) || !native(kind,1U,false) ||
              !native(kind,0U,true)) return false;
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
