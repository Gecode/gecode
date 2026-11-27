/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#include <cassert>
#include <concepts>
#include <memory>
#include <type_traits>

namespace Gecode {

    /// Cast \a p into pointer of type \a T
    template <class T>
    T ptr_cast(void* p);

    /**
     * \brief Cast \a from to a derived type that a Gecode protocol guarantees
     *
     * Use down_cast where Gecode guarantees the dynamic type of an object
     * passed through a base-class pointer or reference, so that the cast
     * cannot fail:
     *  - the space passed to Space::constrain or Space::compare has the same
     *    dynamic type as the space the function is called on, because search
     *    passes only clones of one root space;
     *  - the solution returned by MetaContext::last() has the same dynamic
     *    type as the space the meta-search hook runs on;
     *  - the space passed to a branching function, such as a merit, value,
     *    commit, filter, or print function, is the space the brancher was
     *    posted in, or a clone of it;
     *  - the choice passed to Brancher::commit, Brancher::ngl, or
     *    Brancher::print was created by that brancher or by a brancher it
     *    was cloned from;
     *  - the advisor passed to Propagator::advise belongs to that
     *    propagator.
     *
     * These are guarantees, not assumptions to be checked. A failing cast is
     * a bug in Gecode or in a Space::copy that does not create an object of
     * its own class, never bad input, so code calling down_cast needs no
     * check or error path of its own. Use \c dynamic_cast only where the
     * dynamic type is genuinely unknown.
     *
     * With assertions enabled and a polymorphic source type, the cast is
     * checked by an assertion using \c dynamic_cast. Otherwise it is a plain
     * \c static_cast, which UBSan can verify with \c -fsanitize=vptr. A cast
     * through a virtual base, where \c static_cast is ill-formed, uses
     * \c dynamic_cast.
     *
     * \a To is a pointer type for a pointer argument and an lvalue reference
     * type for a reference argument, and it must keep the argument's const
     * qualification, as in <code>down_cast<const Model&>(other)</code>. A null
     * pointer is returned unchanged.
     *
     * \ingroup FuncSupport
     */
    template <class To, class From>
    [[nodiscard]] To down_cast(From* from) noexcept;
    /**
     * \brief Cast \a from to a derived type that a Gecode protocol guarantees
     *
     * The reference form of down_cast(From*).
     *
     * \ingroup FuncSupport
     */
    template <class To, class From>
    [[nodiscard]] To down_cast(From& from) noexcept;

    template <class T>
    forceinline T
    ptr_cast(void* p) {
        return static_cast<T>(p);
    }

    template <class To, class From>
    forceinline To
    down_cast(From* from) noexcept {
        static_assert(std::is_pointer_v<To>, "down_cast of a pointer needs a pointer target type");
        static_assert(std::derived_from<std::remove_pointer_t<To>, From>,
                      "down_cast target type must be publicly derived from the source type");
        if constexpr (requires(From* f) { static_cast<To>(f); }) {
            if constexpr (std::is_polymorphic_v<From>)
                assert((from == nullptr) || (dynamic_cast<To>(from) == from));
            return static_cast<To>(from);
        } else {
            // Only dynamic_cast can cast through a virtual base
            To to = dynamic_cast<To>(from);
            assert((from == nullptr) || (to != nullptr));
            return to;
        }
    }

    template <class To, class From>
    forceinline To
    down_cast(From& from) noexcept {
        static_assert(std::is_lvalue_reference_v<To>, "down_cast of a reference needs a reference target type");
        return *down_cast<std::remove_reference_t<To>*>(std::addressof(from));
    }

} // namespace Gecode

// STATISTICS: support-any
