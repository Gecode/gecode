/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Gabor Szokoli <szokoli@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode::Set {

    /*
     * Constructors and access
     *
     */

    forceinline
    SetView::SetView(const SetVar& y)
        : VarImpView<SetVar>(y.varimp()) {}
    forceinline
    SetView::SetView(SetVarImp* y)
        : VarImpView<SetVar>(y) {}

    /*
     * Variable information
     *
     */

    forceinline unsigned int
    SetView::glb_size() const {
        return x->glb_size();
    }
    forceinline unsigned int
    SetView::lub_size() const {
        return x->lub_size();
    }
    forceinline unsigned int
    SetView::unknown_size() const {
        return x->lub_size() - x->glb_size();
    }
    forceinline bool
    SetView::contains(int i) const {
        return x->known_in(i);
    }
    forceinline bool
    SetView::not_contains(int i) const {
        return x->known_out(i);
    }
    forceinline unsigned int
    SetView::card_min() const {
        return x->card_min();
    }
    forceinline unsigned int
    SetView::card_max() const {
        return x->card_max();
    }
    forceinline int
    SetView::lub_min() const {
        return x->lub_min();
    }
    forceinline int
    SetView::lub_max() const {
        return x->lub_max();
    }
    forceinline int
    SetView::lub_min_n(unsigned int n) const {
        return x->lub_min_n(n);
    }
    forceinline int
    SetView::glb_min() const {
        return x->glb_min();
    }
    forceinline int
    SetView::glb_max() const {
        return x->glb_max();
    }

    /*
     * Tells
     *
     */

    forceinline ModEvent
    SetView::card_min(Space& home, unsigned int m) {
        return x->card_min(home, m);
    }

    forceinline ModEvent
    SetView::card_max(Space& home, unsigned int m) {
        return x->card_max(home, m);
    }

    forceinline ModEvent
    SetView::include(Space& home, int from, int to) {
        return x->include(home, from, to);
    }

    forceinline ModEvent
    SetView::include(Space& home, int n) {
        return x->include(home, n);
    }

    forceinline ModEvent
    SetView::exclude(Space& home, int n) {
        return x->exclude(home, n);
    }

    forceinline ModEvent
    SetView::intersect(Space& home, int from, int to) {
        return x->intersect(home, from, to);
    }

    forceinline ModEvent
    SetView::intersect(Space& home, int n) {
        return x->intersect(home, n);
    }

    template <class I>
    ModEvent
    SetView::include_i(Space& home, I& iter) {
        return x->include_i(home, iter);
    }

    forceinline ModEvent
    SetView::exclude(Space& home, int from, int to) {
        return x->exclude(home, from, to);
    }
    template <class I>
    ModEvent
    SetView::exclude_i(Space& home, I& iter) {
        return x->exclude_i(home, iter);
    }
    template <class I>
    ModEvent
    SetView::intersect_i(Space& home, I& iter) {
        return x->intersect_i(home, iter);
    }

    /*
     * Delta information for advisors
     *
     */

    forceinline ModEvent
    SetView::modification_event(const Delta& d) {
        return SetVarImp::modification_event(d);
    }

    forceinline int
    SetView::glb_min(const Delta& d) const {
        return SetVarImp::glb_min(d);
    }
    forceinline int
    SetView::glb_max(const Delta& d) const {
        return SetVarImp::glb_max(d);
    }
    forceinline bool
    SetView::glb_any(const Delta& d) const {
        return SetVarImp::glb_any(d);
    }

    forceinline int
    SetView::lub_min(const Delta& d) const {
        return SetVarImp::lub_min(d);
    }
    forceinline int
    SetView::lub_max(const Delta& d) const {
        return SetVarImp::lub_max(d);
    }
    forceinline bool
    SetView::lub_any(const Delta& d) const {
        return SetVarImp::lub_any(d);
    }

    /**
     * \brief %Range iterator for least upper bound of set variable views
     * \ingroup TaskActorSetView
     */
    template <>
    class LubRanges<SetView> : public LubRanges<SetVarImp*> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges();
        /// Initialize with ranges for view \a x
        LubRanges(const SetView& x);
        /// Initialize with ranges for view \a x
        void init(const SetView& x);
        //@}
    };

    forceinline
    // NOLINTNEXTLINE(modernize-use-equals-default)
    LubRanges<SetView>::LubRanges() {}

    forceinline
    LubRanges<SetView>::LubRanges(const SetView& x)
        : LubRanges<SetVarImp*>(x.varimp()) {}

    forceinline void
    LubRanges<SetView>::init(const SetView& x) {
        LubRanges<SetVarImp*>::init(x.varimp());
    }

    /**
     * \brief %Range iterator for greatest lower bound of set variable views
     * \ingroup TaskActorSetView
     */
    template <>
    class GlbRanges<SetView> : public GlbRanges<SetVarImp*> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges();
        /// Initialize with ranges for view \a x
        GlbRanges(const SetView& x);
        /// Initialize with ranges for view \a x
        void init(const SetView& x);
    };

    forceinline
    // NOLINTNEXTLINE(modernize-use-equals-default)
    GlbRanges<SetView>::GlbRanges() {}

    forceinline
    GlbRanges<SetView>::GlbRanges(const SetView& x)
        : GlbRanges<SetVarImp*>(x.varimp()) {}

    forceinline void
    GlbRanges<SetView>::init(const SetView& x) {
        GlbRanges<SetVarImp*>::init(x.varimp());
    }

} // namespace Gecode::Set

// STATISTICS: set-var
