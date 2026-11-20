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
     * split the selectors' state for each spoke; see Space::random_split().
     * Replay does not promise identical parallel scheduling, solution order,
     * or search trees under weakly monotonic propagation.
     *
     * A generator that the model owns is copied with the space, so every
     * spoke starts from the hub's state. A model drawing from it in
     * Space::spoke(), for example to relax a solution, must split it with
     * split(const MetaContext&), or every spoke draws the same numbers. A
     * custom brancher using a model-owned generator must make its state a
     * function of the path, for example by splitting it on commit. Space
     * cloning does not copy mutable state in shared callback captures.
     *
     * Rnd uses the SplitMix engine. Any engine that meets the Support::Random
     * contract, including splitting, can be used as RndGenerator<Engine>.
     * \ingroup TaskModel
     */
    template <class Engine>
    class RndGenerator {
        Support::Random<Engine> r;

    public:
        /// Generator seeded with 1
        RndGenerator() : r(1) {}
        /// Seed explicitly, so an integer never converts to a generator
        explicit RndGenerator(uint64_t seed) : r(seed) {}
        /// Use the state of \a source
        explicit RndGenerator(const Support::Random<Engine>& source) : r(source) {}
        /// Return a generator for alternative \a a, leaving this one unchanged
        RndGenerator
        split(uint32_t a) const {
            return RndGenerator(r.split(a));
        }
        /**
         * \brief Return a generator for the spoke of meta search context \a c
         *
         * Splits as Space::random_split() splits the selectors of branchers.
         * A model that draws from a generator it owns in Space::spoke(), for
         * example to relax a solution, uses this so that every spoke draws
         * differently.
         */
        RndGenerator
        split(const MetaContext& c) const {
            const bool restart = c.engine() == MetaEngine::Restart;
            const uint64_t i = restart ? c.episode() : c.asset();
            return split(restart ? UINT32_MAX : UINT32_MAX - 1)
                .split(static_cast<uint32_t>(i >> 32))
                .split(static_cast<uint32_t>(i));
        }
        void
        seed(uint64_t value) {
            r.seed(value);
        }
        void
        time() {
            seed(static_cast<uint64_t>(::time(nullptr)));
        }
        void
        hw() {
            seed((static_cast<uint64_t>(Support::hwrnd()) << 32) | Support::hwrnd());
        }
        std::string
        state() const {
            return r.state_string();
        }
        void
        state(const std::string& text) {
            r.state(text);
        }
        static const char*
        name() {
            return Engine::name();
        }
        template <class Type>
        Type
        operator()(Type bound) {
            return r(bound);
        }
    };

    /// Default generator, using the SplitMix engine
    using Rnd = RndGenerator<Support::SplitMix>;

} // namespace Gecode

// STATISTICS: kernel-other
