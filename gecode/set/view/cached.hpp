/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2011
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

#include <sstream>

namespace Gecode { namespace Set {

    template <class View>
    forceinline
    CachedView<View>::CachedView(const View& y)
        : DerivedView<View>(y) {}

    template <class View>
    forceinline unsigned int
    CachedView<View>::glb_size() const {
        return x.glb_size();
    }

    template <class View>
    forceinline unsigned int
    CachedView<View>::lub_size() const {
        return x.lub_size();
    }

    template <class View>
    forceinline unsigned int
    CachedView<View>::unknown_size() const {
        return x.unknown_size();
    }

    template <class View>
    forceinline bool
    CachedView<View>::contains(int n) const {
        return x.contains(n);
    }

    template <class View>
    forceinline bool
    CachedView<View>::not_contains(int n) const {
        return x.not_contains(n);
    }

    template <class View>
    forceinline unsigned int
    CachedView<View>::card_min() const {
        return x.card_min();
    }

    template <class View>
    forceinline unsigned int
    CachedView<View>::card_max() const {
        return x.card_max();
    }

    template <class View>
    forceinline int
    CachedView<View>::lub_min() const {
        return x.lub_min();
    }

    template <class View>
    forceinline int
    CachedView<View>::lub_max() const {
        return x.lub_max();
    }

    template <class View>
    forceinline int
    CachedView<View>::glb_min() const {
        return x.glb_min();
    }

    template <class View>
    forceinline int
    CachedView<View>::glb_max() const {
        return x.glb_max();
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::card_min(Space& home, unsigned int m) {
        return x.card_min(home, m);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::card_max(Space& home, unsigned int m) {
        return x.card_max(home, m);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::include(Space& home, int i) {
        return x.include(home, i);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::exclude(Space& home, int i) {
        return x.exclude(home, i);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::intersect(Space& home, int i) {
        return x.intersect(home, i);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::intersect(Space& home, int i, int j) {
        return x.intersect(home, i, j);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::include(Space& home, int i, int j) {
        return x.include(home, i, j);
    }

    template <class View>
    forceinline ModEvent
    CachedView<View>::exclude(Space& home, int i, int j) {
        return x.exclude(home, i, j);
    }

    template <class View>
    template <class I>
    ModEvent
    CachedView<View>::exclude_i(Space& home, I& iter) {
        return x.exclude_i(home, iter);
    }

    template <class View>
    template <class I>
    ModEvent
    CachedView<View>::include_i(Space& home, I& iter) {
        return x.include_i(home, iter);
    }

    template <class View>
    template <class I>
    ModEvent
    CachedView<View>::intersect_i(Space& home, I& iter) {
        return x.intersect_i(home, iter);
    }

    template <class View>
    forceinline void
    CachedView<View>::subscribe(Space& home, Propagator& p, PropCond pc, bool schedule) {
        x.subscribe(home, p, pc, schedule);
    }

    template <class View>
    forceinline void
    CachedView<View>::cancel(Space& home, Propagator& p, PropCond pc) {
        x.cancel(home, p, pc);
    }

    template <class View>
    forceinline void
    CachedView<View>::subscribe(Space& home, Advisor& a) {
        x.subscribe(home, a);
    }

    template <class View>
    forceinline void
    CachedView<View>::cancel(Space& home, Advisor& a) {
        x.cancel(home, a);
    }

    template <class View>
    forceinline void
    CachedView<View>::schedule(Space& home, Propagator& p, ModEvent me) {
        return View::schedule(home, p, me);
    }
    template <class View>
    forceinline ModEvent
    CachedView<View>::me(const ModEventDelta& med) {
        return View::me(med);
    }

    template <class View>
    forceinline ModEventDelta
    CachedView<View>::med(ModEvent me) {
        return View::med(me);
    }

    /*
     * Delta information for advisors
     *
     */

    template <class View>
    forceinline ModEvent
    CachedView<View>::modification_event(const Delta& d) {
        return View::modification_event(d);
    }

    template <class View>
    forceinline int
    CachedView<View>::glb_min(const Delta& d) const {
        return x.glb_min(d);
    }

    template <class View>
    forceinline int
    CachedView<View>::glb_max(const Delta& d) const {
        return x.glb_max(d);
    }

    template <class View>
    forceinline bool
    CachedView<View>::glb_any(const Delta& d) const {
        return x.glb_any(d);
    }

    template <class View>
    forceinline int
    CachedView<View>::lub_min(const Delta& d) const {
        return x.lub_min(d);
    }

    template <class View>
    forceinline int
    CachedView<View>::lub_max(const Delta& d) const {
        return x.lub_max(d);
    }

    template <class View>
    forceinline bool
    CachedView<View>::lub_any(const Delta& d) const {
        return x.lub_any(d);
    }

    template <class View>
    forceinline void
    CachedView<View>::update(Space& home, CachedView<View>& y) {
        lub_cache.update(home, y.lub_cache);
        glb_cache.update(home, y.glb_cache);
        DerivedView<View>::update(home, y);
    }

    /*
     * Cache operations
     *
     */
    template <class View>
    void
    CachedView<View>::init_cache(Space& home, const IntSet& glb, const IntSet& lub) {
        glb_cache.init(home);
        IntSetRanges gr(glb);
        glb_cache.include_i(home, gr);
        lub_cache.init(home);
        IntSetRanges lr(lub);
        lub_cache.intersect_i(home, lr);
    }

    template <class View>
    forceinline void
    CachedView<View>::cache_glb(Space& home) {
        GlbRanges<View> gr(DerivedView<View>::base());
        glb_cache.include_i(home, gr);
    }

    template <class View>
    forceinline void
    CachedView<View>::cache_lub(Space& home) {
        LubRanges<View> lr(DerivedView<View>::base());
        lub_cache.intersect_i(home, lr);
    }

    template <class View>
    forceinline bool
    CachedView<View>::glb_modified() const {
        return glb_cache.size() != glb_size();
    }

    template <class View>
    forceinline bool
    CachedView<View>::lub_modified() const {
        return lub_cache.size() != lub_size();
    }

    /**
     * \brief %Range iterator for least upper bound of cached set views
     * \ingroup TaskActorSetView
     */
    template <class View>
    class LubRanges<CachedView<View>> : public LubRanges<View> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges() = default;
        /// Initialize with ranges for view \a s
        LubRanges(const CachedView<View>& s);
        /// Initialize with ranges for view \a s
        void init(const CachedView<View>& s);
        //@}
    };

    template <class View>
    forceinline
    LubRanges<CachedView<View>>::LubRanges(const CachedView<View>& s)
        : LubRanges<View>(s.base()) {}

    template <class View>
    forceinline void
    LubRanges<CachedView<View>>::init(const CachedView<View>& s) {
        LubRanges<View>::init(s.base());
    }

    /**
     * \brief %Range iterator for greatest lower bound of cached set views
     * \ingroup TaskActorSetView
     */
    template <class View>
    class GlbRanges<CachedView<View>> : public GlbRanges<View> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges() = default;
        /// Initialize with ranges for view \a s
        GlbRanges(const CachedView<View>& s);
        /// Initialize with ranges for view \a s
        void init(const CachedView<View>& s);
        //@}
    };

    template <class View>
    forceinline
    GlbRanges<CachedView<View>>::GlbRanges(const CachedView<View>& s)
        : GlbRanges<View>(s.base()) {}

    template <class View>
    forceinline void
    GlbRanges<CachedView<View>>::init(const CachedView<View>& s) {
        GlbRanges<View>::init(s.base());
    }

    template <class Char, class Traits, class View>
    std::basic_ostream<Char, Traits>&
    operator<<(std::basic_ostream<Char, Traits>& os, const CachedView<View>& x) {
        return os << x.base();
    }

    template <class View>
    forceinline bool
    operator==(const CachedView<View>& x, const CachedView<View>& y) {
        return x.base() == y.base();
    }

    template <class View>
    forceinline
    GlbDiffRanges<View>::GlbDiffRanges(const CachedView<View>& x)
        : gr(x.base()), cr(x.glb_cache) {
        Iter::Ranges::Diff<GlbRanges<View>, BndSetRanges>::init(gr, cr);
    }

    template <class View>
    forceinline
    LubDiffRanges<View>::LubDiffRanges(const CachedView<View>& x)
        : cr(x.lub_cache), lr(x.base()) {
        Iter::Ranges::Diff<BndSetRanges, LubRanges<View>>::init(cr, lr);
    }

}} // namespace Gecode::Set

// STATISTICS: set-var
