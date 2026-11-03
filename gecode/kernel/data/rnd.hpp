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
   * Copying preserves complete state in independent storage, and split()
   * derives a different stream:
   * \code
   * Rnd parent(42);
   * Rnd same = parent;
   * Rnd child = parent.split(1);
   * \endcode
   *
   * Branching descriptions and selectors own their copies. A brancher keys
   * each selector's copy by the brancher's id and the selector's role, so
   * passing one generator to several selectors or branchers gives them
   * independent streams. Selectors draw from a copy of their state and
   * replace the state by its split for the alternative on commit. The state
   * at a node is therefore determined by the posted state and the path, and
   * cloning and recomputation reproduce it. Restart and portfolio engines
   * split the state for each slave; see Space::random_split(). Replay does
   * not promise identical parallel scheduling, solution order, or search
   * trees under weakly monotonic propagation.
   *
   * Model-owned or callback-owned generators change only as their owner
   * changes them; a custom brancher using one must make its state a
   * function of the path, for example by splitting it on commit. Space
   * cloning does not copy mutable state in shared callback captures. Engine
   * must meet the Support::Random contract, including splitting. See
   * examples/random-engine.cpp for a user-defined engine and selectors.
   * \ingroup TaskModel
   */
  template<class Engine>
  class RndGenerator {
    Support::Random<Engine> r;
  public:
    using State = typename Engine::State;
    /// Generator seeded with 1
    RndGenerator(void) : r(1) {}
    /// Seed explicitly, so an integer never converts to a generator
    explicit RndGenerator(uint64_t seed) : r(seed) {}
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
    template<class Type>
    Type operator ()(Type bound) { return r(bound); }
  };

  /// Build-configured default, with exactly the engine's inline state size.
  using Rnd = RndGenerator<Support::RandomGenerator::EngineType>;

}

// STATISTICS: kernel-other
