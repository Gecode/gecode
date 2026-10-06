/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
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

#include <ctime>

namespace Gecode {

  /**
   * \brief Small value-type random generator owned by its consumer
   *
   * Copying copies complete state independently. Branching descriptions
   * and selectors own their copies; cloning preserves selector state
   * without drawing or splitting. Split explicitly to start different streams:
   * \code
   * Rnd parent(42);
   * Rnd same = parent;
   * Rnd child = parent.split(1);
   * \endcode
   *
   * Only the active randomized brancher's selectors are recorded in a choice.
   * Committing an alternative restores their recorded states and splits by
   * the public alternative index. Replay reproduces random state along that
   * path, but does not promise identical parallel scheduling, solution order,
   * or search trees under weakly monotonic propagation.
   *
   * Model-owned or callback-owned generators are not recorded automatically.
   * A custom brancher must preserve their state in its choices when replay
   * needs it. Space cloning does not copy mutable state in shared callback
   * captures. Engine supplies the Support::Random contract, including splitting.
   * See examples/random-engine.cpp for a user-defined engine and selectors.
   * \ingroup TaskModel
   */
  template<class Engine>
  class RndGenerator {
    Support::Random<Engine> r;
  public:
    using State = typename Engine::State;
    explicit RndGenerator(uint64_t seed=1) : r(seed) {}
    explicit RndGenerator(const Support::Random<Engine>& source) : r(source) {}
    RndGenerator copy(void) const { return *this; }
    RndGenerator split(uint32_t a) const { return RndGenerator(r.split(a)); }
    void seed(uint64_t value) { r.seed(value); }
    void time(void) { seed(static_cast<uint64_t>(::time(nullptr))); }
    void hw(void) {
      seed((uint64_t(Support::hwrnd()) << 32) | Support::hwrnd());
    }
    std::string state(void) const { return r.state_string(); }
    void state(const std::string& text) { r.state(text); }
    State state_words(void) const { return r.state(); }
    void state(const State& words) { r.state(words); }
    static const char* name(void) { return Engine::name(); }
    static constexpr unsigned int words(void) { return std::tuple_size<State>::value; }
    template<class Type>
    Type operator ()(Type bound) { return r(bound); }
    /// Save compact choice data and advance the output pointer.
    uint64_t* save(uint64_t* out) const {
      auto s = r.state();
      return std::copy(s.begin(),s.end(),out);
    }
    /// Derive this consumer's next state from its recorded choice data.
    const uint64_t* restore_split(const uint64_t* in, uint32_t a) {
      State s;
      std::copy(in,in+s.size(),s.begin());
      auto parent = r;
      parent.state(s);
      r = parent.split(a);
      return in+s.size();
    }
  };

  /// Build-configured default, with exactly the engine's inline state size.
  using Rnd = RndGenerator<Support::RandomGenerator::EngineType>;

}

// STATISTICS: kernel-other
