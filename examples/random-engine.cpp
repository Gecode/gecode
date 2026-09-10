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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <iostream>
#include <memory>

/// A user engine that counts raw draws along each path.
/// Generation and indexed splitting retain SplitMix's algorithms and guarantees.
class CountedSplitMix {
  Gecode::Support::SplitMix engine;
  uint64_t draws = 0;
public:
  using State = std::array<uint64_t,3>;
  explicit CountedSplitMix(uint64_t seed=1) : engine(seed) {}
  static const char* name(void) { return "counted-splitmix-v1"; }
  static constexpr uint64_t min(void) { return 0; }
  static constexpr uint64_t max(void) { return UINT64_MAX; }
  void seed(uint64_t value) { engine.seed(value); draws = 0; }
  uint64_t next(void) { ++draws; return engine.next(); }
  State state(void) const {
    auto s = engine.state();
    return {{s[0],s[1],draws}};
  }
  void state(const State& s) {
    engine.state({{s[0],s[1]}}); // Validate before changing the counter.
    draws = s[2];
  }
  CountedSplitMix split(uint32_t alternative) const {
    auto child = *this;
    child.engine = engine.split(alternative);
    return child;
  }
};

/// Enumerate permutations using one user stream for both random selectors.
class Permutations : public Gecode::Space {
  Gecode::IntVarArray x;
  Gecode::Rnd random;
public:
  explicit Permutations(const Gecode::Rnd& source)
    : x(*this,4,0,3), random(*this,source) {
    Gecode::distinct(*this,x);
    Gecode::branch(*this,x,Gecode::INT_VAR_RND(random),
                   Gecode::INT_VAL_RND(random));
  }
  Permutations(Permutations& s)
    : Space(s), random(*this,s.random) { x.update(*this,s.x); }
  Gecode::Space* copy(void) override { return new Permutations(*this); }
  void print(void) const {
    std::cout << x << "  " << random.state() << '\n';
  }
};

int main(int argc, char* argv[]) {
  try {
    if (argc > 2)
      throw std::invalid_argument("Usage: random-engine [complete-state]");
    Gecode::Support::Random<CountedSplitMix> engine(42);
    if (argc == 2)
      engine.state(std::string(argv[1]));
    std::cout << "Initial state: " << engine.state_string() << '\n';
    Gecode::Rnd source(engine);
    auto root = std::make_unique<Permutations>(source);
    Gecode::DFS<Permutations> search(root.get());
    root.reset();
    while (auto solution = std::unique_ptr<Permutations>(search.next()))
      solution->print();
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
