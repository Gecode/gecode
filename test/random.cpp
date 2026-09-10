/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
// Copyright (c) 2026 Mikael Zayenz Lagerkvist. MIT license; see LICENSE.

#include "test/test.hh"

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
        RandomGenerator r(0);
        for (uint64_t expected : {UINT64_C(0xe220a8397b1dcdaf),
                                  UINT64_C(0x6e789e6aa1b965f4),
                                  UINT64_C(0x06c45d188009454f)})
          if (r.next() != expected)
            return false;
        Xorshift64Star xs(1);
        if (xs.next() != UINT64_C(0x47e4ce4b896cdd1d))
          return false;
        if (!replay<SplitMix>() || !replay<CountedSplitMix>())
          return false;
        auto parent = r.state();
        // Indexing skips pairs of parent words, exactly as sequential splits.
        RandomGenerator sequential = r;
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

    class ReplaySpace : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      Gecode::Rnd variable, value;
      ReplaySpace(const Gecode::Rnd& source, bool separate, bool multi=false)
        : x(*this,4,0,2), variable(*this,source),
          value(*this,separate ? source.copy() : source) {
        using namespace Gecode;
        // The first branch is deterministic, but later streams must split.
        branch(*this,x[0],multi ? INT_VALUES_MIN() : INT_VAL_MIN());
        IntVarArgs first(2);
        first[0]=x[1]; first[1]=x[2];
        branch(*this,first,INT_VAR_RND(variable),INT_VAL_RND(value));
        branch(*this,x[3],INT_VAL_RND(value));
      }
      ReplaySpace(ReplaySpace& s)
        : Space(s), variable(*this,s.variable), value(*this,s.value) {
        x.update(*this,s.x);
      }
      Space* copy() override { return new ReplaySpace(*this); }
    };

    bool same_archive(const Gecode::Choice& a, const Gecode::Choice& b) {
      Gecode::Archive x,y;
      a.archive(x); b.archive(y);
      if (x.size()!=y.size())
        return false;
      for (int i=0; i<x.size(); ++i)
        if (x[i]!=y[i])
          return false;
      return true;
    }

    bool choice_replay(const Gecode::Rnd& source, bool separate, bool multi) {
      using namespace Gecode;
      std::unique_ptr<ReplaySpace> root(new ReplaySpace(source,separate,multi));
      while (root->status()==SS_BRANCH) {
        std::unique_ptr<ReplaySpace> before(static_cast<ReplaySpace*>(root->clone()));
        const auto source_state = source.state();
        std::unique_ptr<const Choice> choice(root->choice());
        Archive packed;
        choice->archive(packed);
        if (packed[1] != source.words()*(separate ? 2 : 1))
          return false;
        const auto variable = root->variable.copy();
        const auto value = root->value.copy();
        std::vector<std::string> siblings;
        // Explore backwards, exercising late alternatives without earlier draws.
        for (unsigned int a=choice->alternatives(); a--;) {
          std::unique_ptr<ReplaySpace> direct(static_cast<ReplaySpace*>(root->clone()));
          std::unique_ptr<ReplaySpace> replay(static_cast<ReplaySpace*>(before->clone()));
          Archive archive;
          choice->archive(archive);
          std::unique_ptr<const Choice> restored(replay->choice(archive));
          direct->commit(*choice,a);
          // State on the recomputed space is intentionally different before commit.
          (void) replay->variable(13);
          replay->commit(*restored,a);
          if (direct->variable.state()!=variable.split(a).state() ||
              direct->value.state()!=value.split(a).state() ||
              replay->variable.state()!=direct->variable.state() ||
              replay->value.state()!=direct->value.state())
            return false;
          for (const auto& previous : siblings)
            if (previous==direct->variable.state())
              return false;
          siblings.push_back(direct->variable.state());
          auto status = direct->status();
          if (status!=replay->status())
            return false;
          if (status==SS_BRANCH) {
            std::unique_ptr<const Choice> next(direct->choice());
            std::unique_ptr<const Choice> next_replay(replay->choice());
            if (!same_archive(*next,*next_replay))
              return false;
          }
        }
        if (source.state()!=source_state || root->variable.state()!=variable.state())
          return false;
        root->commit(*choice,0);
      }
      return true;
    }

    std::vector<std::string> solutions(const Gecode::Rnd& source,
                                       unsigned int distance, bool separate,
                                       bool multi) {
      using namespace Gecode;
      ReplaySpace root(source,separate,multi);
      Search::Options options;
      options.c_d=distance;
      options.a_d=distance;
      DFS<ReplaySpace> search(&root,options);
      std::vector<std::string> result;
      while (std::unique_ptr<ReplaySpace> s{search.next()}) {
        std::ostringstream item;
        item << s->x << ':' << s->variable.state() << ':' << s->value.state();
        result.push_back(item.str());
      }
      return result;
    }

    class BranchReplay : public Base {
    public:
      BranchReplay() : Base("Random::BranchReplay") {}
      bool run() override {
        Gecode::Rnd engines[] = {
          Gecode::Rnd(42),
          Gecode::Rnd(Gecode::Support::Random<CountedSplitMix>(42))
        };
        for (const auto& engine : engines)
          for (bool separate : {false,true})
            for (bool multi : {false,true}) {
              if (!choice_replay(engine,separate,multi))
                return false;
              auto cloned = solutions(engine,1,separate,multi);
              auto recomputed = solutions(engine,100,separate,multi);
              if (cloned.size()!=81 || cloned!=recomputed)
                return false;
            }
        return true;
      }
    } branch_replay;
  }
}
