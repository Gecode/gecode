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

#include "test/test.hh"
#include <gecode/int/branch.hh>

namespace Test {
  namespace Random {
    using namespace Gecode::Support;

    // A user engine with extra state exercises the public engine contract.
    // The counter deliberately affects output, so omitting it breaks replay.
    class CountedSplitMix {
      SplitMix source;
      uint64_t count = 0;
    public:
      using State = std::array<uint64_t,3>;
      explicit CountedSplitMix(uint64_t s=1) : source(s) {}
      static const char* name() { return "counted-splitmix-test-v1"; }
      static constexpr uint64_t min() { return 0; }
      static constexpr uint64_t max() { return UINT64_MAX; }
      void seed(uint64_t s) { source.seed(s); count=0; }
      uint64_t next() { return source.next() ^ count++; }
      State state() const {
        auto s = source.state();
        return {{s[0],s[1],count}};
      }
      void state(const State& s) {
        source.state({{s[0],s[1]}});
        count=s[2];
      }
      CountedSplitMix split(uint32_t a) const {
        auto child = *this;
        child.source = source.split(a);
        return child;
      }
    };

    template<class Engine>
    bool replay() {
      Gecode::Support::Random<Engine> original(UINT64_MAX), restored;
      for (unsigned int i=0; i<17; ++i)
        (void) original(13);
      original = original.split(37);
      (void) original(UINT64_MAX);
      restored.state(original.state_string());
      for (uint32_t a : {0U,1U,17U,UINT32_MAX}) {
        if (original.split(a).state() != restored.split(a).state())
          return false;
        if (original.next() != restored.next())
          return false;
      }
      auto before = restored.state();
      if (restored(0) || restored(1) || restored(-1) ||
          restored.state() != before)
        return false;
      return true;
    }

    class Contract : public Base {
    public:
      Contract() : Base("Random::Contract") {}
      bool run() override {
        // SplitMix64 reference sequence, seed zero and golden-ratio increment.
        Gecode::Support::Random<SplitMix> r(0);
        for (uint64_t expected : {UINT64_C(0xe220a8397b1dcdaf),
                                  UINT64_C(0x6e789e6aa1b965f4),
                                  UINT64_C(0x06c45d188009454f)})
          if (r.next() != expected)
            return false;
        Xorshift64Star xs(1);
        if (xs.next() != UINT64_C(0x47e4ce4b896cdd1d))
          return false;
        if (!replay<SplitMix>() || !replay<Xorshift64Star>() ||
            !replay<CountedSplitMix>())
          return false;
        if (xs.split(0).split(0).state()!=xs.split(1).state())
          return false;
        auto one_step=xs;
        (void) one_step.next();
        if (xs.split(UINT32_MAX).state()!=one_step.state())
          return false;
        auto parent = r.state();
        // Indexing skips pairs of parent words, exactly as sequential splits.
        Gecode::Support::Random<SplitMix> sequential = r;
        for (uint32_t a=0; a<100; ++a) {
          auto child = r.split(a);
          if (child.state()[0] != sequential.next())
            return false;
          (void) sequential.next();
          if ((a>0) && (child.state() == r.split(a-1).state()))
            return false;
        }
        if (r.state() != parent || r.split(0).state()==r.split(UINT32_MAX).state())
          return false;
        if (random_seed("18446744073709551615") != UINT64_MAX ||
            random_seed("0xffffffffffffffff") != UINT64_MAX)
          return false;
        for (const char* invalid : {"", "-1", "+1", "1x", "0x", "18446744073709551616"}) {
          try { (void) random_seed(invalid); return false; }
          catch (const std::invalid_argument&) {}
        }
        const auto saved = r.state();
        for (const char* invalid : {
              "splitmix-v1:0000000000000000:0000000000000000",
              "splitmix-v1:0000000000000000:0000000000000002",
              "splitmix-v1:000000000000000g:9e3779b97f4a7c15",
              "splitmix-v1:0:9e3779b97f4a7c15",
              "other-v1:0000000000000000:9e3779b97f4a7c15"}) {
          try { r.state(std::string(invalid)); return false; }
          catch (const std::invalid_argument&) {}
          if (r.state() != saved)
            return false;
        }
        Gecode::Support::Random<Xorshift64Star> x(0);
        try { x.state(Xorshift64Star::State{{0}}); return false; }
        catch (const std::invalid_argument&) {}
        for (uint64_t bound : {UINT64_C(2),UINT64_C(3),UINT64_C(0x8000000000000001),UINT64_MAX})
          for (unsigned int i=0; i<100; ++i)
            if (r(bound)>=bound || x(bound)>=bound)
              return false;
        return true;
      }
    } contract;

    // The model's RNG is an ordinary value, independent of selector copies.
    template<class Random>
    class ReplaySpace : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      Random own;
      Gecode::ViewSel<Gecode::Int::IntView>* variable = nullptr;
      Gecode::ValSelCommitBase<Gecode::Int::IntView,int>* value = nullptr;
      void post(const Gecode::IntVarArgs& vars, bool multi, bool different) {
        using namespace Gecode;
        using Int::IntView;
        ViewArray<IntView> views(*this,vars);
        ViewSel<IntView>* selectors[] = {
          new (*this) ViewSelRnd<IntView,Random>(*this,own)
        };
        if (multi) {
          Int::Branch::postviewvaluesbrancher<1,true>(*this,views,selectors,nullptr,nullptr);
        } else {
          using Values = ValSelCommit<Int::Branch::ValSelRnd<IntView,Random>,
                                      Int::Branch::ValCommitEq<IntView>>;
          value = new (*this) Values(*this,INT_VAL_MIN(),different ? own.split(99) : own);
          postviewvalbrancher<IntView,1,int,2>(*this,views,selectors,value,nullptr,nullptr);
        }
        variable = selectors[0];
      }
      ReplaySpace(const Random& source, bool different, bool multi=false,
                  bool callback=false) : x(*this,4,0,2), own(source) {
        using namespace Gecode;
        branch(*this,x[0],INT_VAL_MIN());
        IntVarArgs first(2); first[0]=x[1]; first[1]=x[2];
        post(first,multi,different);
        if (callback)
          branch(*this,[](Space& home) {
            auto& model=static_cast<ReplaySpace&>(home);
            // Explicit model-owned state transition in a one-alternative commit.
            model.own=model.own.split(0);
          });
        branch(*this,x[3],INT_VAL_MIN());
      }
      ReplaySpace(ReplaySpace& s) : Space(s), own(s.own) { x.update(*this,s.x); }
      Space* copy() override { return new ReplaySpace(*this); }
    };

    bool same_archive(const Gecode::Choice& a, const Gecode::Choice& b) {
      Gecode::Archive x,y;
      a.archive(x); b.archive(y);
      if (x.size()!=y.size()) return false;
      for (int i=0; i<x.size(); ++i)
        if (x[i]!=y[i]) return false;
      return true;
    }

    template<class Random>
    bool choice_replay(const Random& source, bool different, bool multi, bool callback) {
      using namespace Gecode;
      using Model=ReplaySpace<Random>;
      std::unique_ptr<Model> root(new Model(source,different,multi,callback));
      while (root->status()==SS_BRANCH) {
        std::unique_ptr<Model> before(static_cast<Model*>(root->clone()));
        const auto owner_state=root->own.state();
        std::unique_ptr<const Choice> choice(root->choice());
        if (root->own.state()!=owner_state) return false;
        for (unsigned int a=choice->alternatives(); a--;) {
          std::unique_ptr<Model> direct(static_cast<Model*>(root->clone()));
          std::unique_ptr<Model> replay(static_cast<Model*>(before->clone()));
          Archive archive; choice->archive(archive);
          std::unique_ptr<const Choice> restored(replay->choice(archive));
          if (!same_archive(*choice,*restored)) return false;
          direct->commit(*choice,a);
          replay->commit(*restored,a);
          auto status=direct->status();
          if (status!=replay->status() || direct->own.state()!=replay->own.state())
            return false;
          if (status==SS_BRANCH) {
            std::unique_ptr<const Choice> next(direct->choice());
            std::unique_ptr<const Choice> next_replay(replay->choice());
            if (!same_archive(*next,*next_replay)) return false;
          }
        }
        root->commit(*choice,0);
      }
      return true;
    }

    template<class Random>
    std::vector<std::string> solutions(const Random& source, unsigned int distance,
                                       bool different, bool multi, bool callback,
                                       unsigned int threads=1) {
      using namespace Gecode;
      using Model=ReplaySpace<Random>;
      Model root(source,different,multi,callback);
      Search::Options options;
      options.c_d=distance; options.a_d=distance; options.threads=threads;
      DFS<Model> search(&root,options);
      std::vector<std::string> result;
      while (std::unique_ptr<Model> s{search.next()}) {
        std::ostringstream item;
        item << s->x << ':' << s->own.state();
        result.push_back(item.str());
      }
      return result;
    }

    template<class Random>
    bool branch_replay(const Random& source) {
      for (bool different : {false,true})
        for (bool multi : {false,true})
          for (bool callback : {false,true}) {
            if (!choice_replay(source,different,multi,callback)) return false;
            auto cloned=solutions(source,1,different,multi,callback);
            auto recomputed=solutions(source,100,different,multi,callback);
            if (cloned.size()!=81 || cloned!=recomputed) return false;
            auto parallel=solutions(source,100,different,multi,callback,2);
            std::sort(cloned.begin(),cloned.end());
            std::sort(parallel.begin(),parallel.end());
            if (cloned!=parallel) return false;
          }
      return true;
    }

    class LDSBSpace : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      LDSBSpace() : x(*this,4,0,3) {
        using namespace Gecode;
        Symmetries syms;
        syms << VariableSymmetry(x);
        distinct(*this,x);
        branch(*this,x,INT_VAR_RND(Rnd(42)),INT_VAL_RND(Rnd(7)),syms);
      }
      LDSBSpace(LDSBSpace& s) : Space(s) { x.update(*this,s.x); }
      Space* copy() override { return new LDSBSpace(*this); }
    };

    bool ldsb_replay() {
      using namespace Gecode;
      LDSBSpace root;
      while (root.status()==SS_BRANCH) {
        std::unique_ptr<Space> before(root.clone());
        std::unique_ptr<const Choice> choice(root.choice());
        for (unsigned int a=choice->alternatives(); a--;) {
          std::unique_ptr<Space> direct(root.clone()), replay(before->clone());
          Archive archive; choice->archive(archive);
          std::unique_ptr<const Choice> restored(replay->choice(archive));
          if (!same_archive(*choice,*restored)) return false;
          direct->commit(*choice,a); replay->commit(*restored,a);
          auto status=direct->status();
          if (status!=replay->status()) return false;
          if (status==SS_BRANCH) {
            std::unique_ptr<const Choice> next(direct->choice());
            std::unique_ptr<const Choice> next_replay(replay->choice());
            if (!same_archive(*next,*next_replay)) return false;
          }
        }
        root.commit(*choice,0);
      }
      return true;
    }

    class BranchReplay : public Base {
    public:
      BranchReplay() : Base("Random::BranchReplay") {}
      bool run() override {
        return ldsb_replay() && branch_replay(Gecode::Rnd(42)) &&
          branch_replay(Gecode::RndGenerator<Xorshift64Star>(42)) &&
          branch_replay(Gecode::RndGenerator<CountedSplitMix>(42));
      }
    } branch_replay_test;

    template<class Random>
    bool consumer_states(const Random& source, bool multi) {
      using namespace Gecode;
      // Inspect original-space selectors only; clones own separate copies.
      for (unsigned int a=0; a<(multi ? 3U : 2U); ++a) {
        ReplaySpace<Random> root(source,false,multi);
        if (root.status()!=SS_BRANCH) return false;
        auto original=source.state_words();
        std::vector<uint64_t> observed(Random::words());
        root.variable->random_save(observed.data());
        if (!std::equal(original.begin(),original.end(),observed.begin())) return false;
        std::unique_ptr<const Choice> first(root.choice());
        const auto& deterministic=static_cast<const PosChoice&>(*first);
        if (deterministic.random_data()!=nullptr) return false;
        Archive plain; first->archive(plain);
        if (plain.size()!=3) return false; // No global RNG pointer/count/archive data.
        root.commit(*first,0);
        root.variable->random_save(observed.data());
        if (!std::equal(original.begin(),original.end(),observed.begin())) return false;
        if (root.status()!=SS_BRANCH) return false;
        std::unique_ptr<const Choice> choice(root.choice());
        const uint64_t* recorded=static_cast<const PosChoice&>(*choice).random_data();
        if (!recorded) return false;
        Random expected=source;
        expected.restore_split(recorded,a);
        // Perturb the selector after taking the choice. Commit must restore it.
        root.variable->random_commit(recorded,123);
        root.commit(*choice,a);
        root.variable->random_save(observed.data());
        auto expected_words=expected.state_words();
        if (!std::equal(expected_words.begin(),expected_words.end(),observed.begin()))
          return false;
        if (!multi) {
          expected.restore_split(recorded+Random::words(),a);
          root.value->random_save(observed.data());
          expected_words=expected.state_words();
          if (!std::equal(expected_words.begin(),expected_words.end(),observed.begin()))
            return false;
        }
        if (root.own.state()!=source.state()) return false;
      }
      return true;
    }

    class CommitBoundary : public Base {
    public:
      CommitBoundary() : Base("Random::CommitBoundary") {}
      bool run() override {
        using namespace Gecode;
        static_assert(sizeof(Rnd)==sizeof(Support::RandomGenerator),
                      "Rnd must contain only inline engine state");
        Rnd r(7), copy=r;
        auto state=r.state();
        (void) copy(UINT64_MAX);
        if (r.state()!=state || copy.state()==state) return false;
        return consumer_states(r,false) && consumer_states(r,true) &&
          consumer_states(RndGenerator<CountedSplitMix>(7),false) &&
          consumer_states(RndGenerator<CountedSplitMix>(7),true);
      }
    } commit_boundary;
  }
}
