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
  }
}
