/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Gabor Szokoli <szokoli@gecode.dev>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
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

namespace Gecode { namespace Set {

    /*
     * Creation of new variable implementations
     *
     */

    forceinline
    SetVarImp::SetVarImp(Space& home)
        : SetVarImpBase(home), lub(home), glb(home) {
        lub.card(Limits::card);
        assert(lub.card() == lub.size());
    }

    forceinline
    SetVarImp::SetVarImp(Space& home, int lb_min, int lb_max, int ub_min, int ub_max, unsigned int card_min,
                         unsigned int card_max)
        : SetVarImpBase(home), lub(home, ub_min, ub_max), glb(home, lb_min, lb_max) {
        glb.card(std::max(card_min, glb.size()));
        lub.card(std::min(card_max, lub.size()));
    }

    forceinline
    SetVarImp::SetVarImp(Space& home, const IntSet& the_glb, int ub_min, int ub_max, unsigned int card_min,
                         unsigned int card_max)
        : SetVarImpBase(home), lub(home, ub_min, ub_max), glb(home, the_glb) {
        glb.card(std::max(card_min, glb.size()));
        lub.card(std::min(card_max, lub.size()));
    }

    forceinline
    SetVarImp::SetVarImp(Space& home, int lb_min, int lb_max, const IntSet& the_lub, unsigned int card_min,
                         unsigned int card_max)
        : SetVarImpBase(home), lub(home, the_lub), glb(home, lb_min, lb_max) {
        glb.card(std::max(card_min, glb.size()));
        lub.card(std::min(card_max, lub.size()));
    }

    forceinline
    SetVarImp::SetVarImp(Space& home, const IntSet& the_glb, const IntSet& the_lub, unsigned int card_min,
                         unsigned int card_max)
        : SetVarImpBase(home), lub(home, the_lub), glb(home, the_glb) {
        glb.card(std::max(card_min, glb.size()));
        lub.card(std::min(card_max, lub.size()));
    }

    forceinline bool
    SetVarImp::assigned(void) const {
        return glb.size() == lub.size();
    }

    forceinline unsigned int
    SetVarImp::card_min(void) const {
        return glb.card();
    }

    forceinline unsigned int
    SetVarImp::card_max(void) const {
        return lub.card();
    }

    forceinline bool
    SetVarImp::known_in(int i) const {
        return (glb.in(i));
    }

    forceinline bool
    SetVarImp::known_out(int i) const {
        return !(lub.in(i));
    }

    forceinline int
    SetVarImp::lub_min(void) const {
        return lub.min();
    }

    forceinline int
    SetVarImp::lub_max(void) const {
        return lub.max();
    }

    forceinline int
    SetVarImp::lub_min_n(unsigned int n) const {
        return lub.min_n(n);
    }

    forceinline int
    SetVarImp::glb_min(void) const {
        return glb.min();
    }

    forceinline int
    SetVarImp::glb_max(void) const {
        return glb.max();
    }

    forceinline unsigned int
    SetVarImp::glb_size() const {
        return glb.size();
    }

    forceinline unsigned int
    SetVarImp::lub_size() const {
        return lub.size();
    }

    /*
     * Support for delta information
     *
     */
    forceinline int
    SetVarImp::glb_min(const Delta& d) {
        return static_cast<const SetDelta&>(d).glb_min();
    }
    forceinline int
    SetVarImp::glb_max(const Delta& d) {
        return static_cast<const SetDelta&>(d).glb_max();
    }
    forceinline bool
    SetVarImp::glb_any(const Delta& d) {
        return static_cast<const SetDelta&>(d).glb_any();
    }
    forceinline int
    SetVarImp::lub_min(const Delta& d) {
        return static_cast<const SetDelta&>(d).lub_min();
    }
    forceinline int
    SetVarImp::lub_max(const Delta& d) {
        return static_cast<const SetDelta&>(d).lub_max();
    }
    forceinline bool
    SetVarImp::lub_any(const Delta& d) {
        return static_cast<const SetDelta&>(d).lub_any();
    }

    forceinline ModEvent
    SetVarImp::card_min(Space& home, unsigned int new_min) {
        if (card_min() >= new_min)
            return ME_SET_NONE;
        if (new_min > card_max())
            return fail(home);
        glb.card(new_min);
        return card_min_full(home);
    }

    forceinline ModEvent
    SetVarImp::card_max(Space& home, unsigned int new_max) {
        if (card_max() <= new_max)
            return ME_SET_NONE;
        if (card_min() > new_max)
            return fail(home);
        lub.card(new_max);
        return card_max_full(home);
    }

    forceinline ModEvent
    SetVarImp::intersect(Space& home, int i, int j) {
        BndSetRanges lb(glb);
        Iter::Ranges::Singleton s(i, j);
        Iter::Ranges::Diff<BndSetRanges, Iter::Ranges::Singleton> probe(lb, s);
        if (probe())
            return fail(home);
        if (assigned())
            return ME_SET_NONE;
        int old_min = lub.min();
        int old_max = lub.max();
        if (lub.intersect(home, i, j)) {
            SetDelta d;
            if (i == old_min) {
                d._lub_min = lub.max() + 1;
                d._lub_max = old_max;
            } else if (j == old_max) {
                d._lub_min = old_min;
                d._lub_max = lub.min() - 1;
            }
            return process_lub_change(home, d);
        }
        return ME_SET_NONE;
    }

    forceinline ModEvent
    SetVarImp::intersect(Space& home, int i) {
        return intersect(home, i, i);
    }

    template <class I>
    inline ModEvent
    SetVarImp::intersect_i(Space& home, I& iterator) {
        if (assigned()) {
            BndSetRanges lbi(glb);
            Iter::Ranges::Diff<BndSetRanges, I> probe(lbi, iterator);
            if (probe())
                return fail(home);
            return ME_SET_NONE;
        }
        if (!iterator()) {
            if (card_min() > 0)
                return fail(home);
            lub.card(0);
            SetDelta d(1, 0, lub.min(), lub.max());
            lub.exclude_all(home);
            return notify(home, ME_SET_VAL, d);
        }
        int mi = iterator.min();
        int ma = iterator.max();
        ++iterator;
        if (iterator())
            return intersect_i_full(home, mi, ma, iterator);
        else
            return intersect(home, mi, ma);
    }

    template <class I>
    ModEvent
    SetVarImp::intersect_i_full(Space& home, int mi, int ma, I& iterator) {
        Iter::Ranges::SingletonAppend<I> si(mi, ma, iterator);
        if (lub.intersect_i(home, si)) {
            BndSetRanges ub(lub);
            BndSetRanges lb(glb);
            if (!Iter::Ranges::subset(lb, ub)) {
                glb.become(home, lub);
                glb.card(glb.size());
                lub.card(glb.size());
                return fail(home);
            }
            ModEvent me = ME_SET_LUB;
            if (card_max() > lub.size()) {
                lub.card(lub.size());
                if (card_min() > card_max()) {
                    glb.become(home, lub);
                    glb.card(glb.size());
                    lub.card(glb.size());
                    return fail(home);
                }
                me = ME_SET_CLUB;
            }
            if (card_max() == lub.size() && card_min() == card_max()) {
                glb.become(home, lub);
                me = ME_SET_VAL;
            }
            SetDelta d;
            return notify(home, me, d);
        }
        return ME_SET_NONE;
    }

    forceinline ModEvent
    SetVarImp::include(Space& home, int i, int j) {
        if (j < i)
            return ME_SET_NONE;
        BndSetRanges ub(lub);
        Iter::Ranges::Singleton sij(i, j);
        if (!Iter::Ranges::subset(sij, ub)) {
            return fail(home);
        }
        SetDelta d;
        if (glb.include(home, i, j, d))
            return process_glb_change(home, d);
        return ME_SET_NONE;
    }

    forceinline ModEvent
    SetVarImp::include(Space& home, int i) {
        return include(home, i, i);
    }

    template <class I>
    forceinline ModEvent
    SetVarImp::include_i(Space& home, I& iterator) {
        if (!iterator()) {
            return ME_SET_NONE;
        }
        if (assigned()) {
            BndSetRanges lbi(glb);
            Iter::Ranges::Diff<I, BndSetRanges> probe(iterator, lbi);
            return probe() ? fail(home) : ME_SET_NONE;
        }
        int mi = iterator.min();
        int ma = iterator.max();
        ++iterator;
        if (iterator())
            return include_i_full(home, mi, ma, iterator);
        else
            return include(home, mi, ma);
    }

    template <class I>
    ModEvent
    SetVarImp::include_i_full(Space& home, int mi, int ma, I& iterator) {
        Iter::Ranges::SingletonAppend<I> si(mi, ma, iterator);
        if (glb.include_i(home, si)) {
            BndSetRanges ub(lub);
            BndSetRanges lb(glb);
            if (!Iter::Ranges::subset(lb, ub)) {
                glb.become(home, lub);
                glb.card(glb.size());
                lub.card(glb.size());
                return fail(home);
            }
            ModEvent me = ME_SET_GLB;
            if (card_min() < glb.size()) {
                glb.card(glb.size());
                if (card_min() > card_max()) {
                    glb.become(home, lub);
                    glb.card(glb.size());
                    lub.card(glb.size());
                    return fail(home);
                }
                me = ME_SET_CGLB;
            }
            if (card_min() == glb.size() && card_min() == card_max()) {
                lub.become(home, glb);
                me = ME_SET_VAL;
            }
            SetDelta d;
            return notify(home, me, d);
        }
        return ME_SET_NONE;
    }

    forceinline ModEvent
    SetVarImp::exclude(Space& home, int i, int j) {
        if (j < i)
            return ME_SET_NONE;
        Iter::Ranges::Singleton sij(i, j);
        BndSetRanges lb(glb);
        Iter::Ranges::Inter<Iter::Ranges::Singleton, BndSetRanges> probe(sij, lb);
        if (probe())
            return fail(home);
        SetDelta d;
        if (lub.exclude(home, i, j, d))
            return process_lub_change(home, d);
        return ME_SET_NONE;
    }

    forceinline ModEvent
    SetVarImp::exclude(Space& home, int i) {
        return exclude(home, i, i);
    }

    template <class I>
    inline ModEvent
    SetVarImp::exclude_i(Space& home, I& iterator) {
        if (!iterator())
            return ME_SET_NONE;
        if (assigned()) {
            BndSetRanges ubi(lub);
            Iter::Ranges::Inter<BndSetRanges, I> probe(ubi, iterator);
            return probe() ? fail(home) : ME_SET_NONE;
        }
        int mi = iterator.min();
        int ma = iterator.max();
        ++iterator;
        if (iterator())
            return exclude_i_full(home, mi, ma, iterator);
        else
            return exclude(home, mi, ma);
    }

    template <class I>
    ModEvent
    SetVarImp::exclude_i_full(Space& home, int mi, int ma, I& iterator) {
        Iter::Ranges::SingletonAppend<I> si(mi, ma, iterator);
        if (lub.exclude_i(home, si)) {
            BndSetRanges ub(lub);
            BndSetRanges lb(glb);
            if (!Iter::Ranges::subset(lb, ub)) {
                glb.become(home, lub);
                glb.card(glb.size());
                lub.card(glb.size());
                return fail(home);
            }
            ModEvent me = ME_SET_LUB;
            if (card_max() > lub.size()) {
                lub.card(lub.size());
                if (card_min() > card_max()) {
                    glb.become(home, lub);
                    glb.card(glb.size());
                    lub.card(glb.size());
                    return fail(home);
                }
                me = ME_SET_CLUB;
            }
            if (card_max() == lub.size() && card_min() == card_max()) {
                glb.become(home, lub);
                me = ME_SET_VAL;
            }
            SetDelta d;
            return notify(home, me, d);
        }
        return ME_SET_NONE;
    }

    /*
     * Copying a variable
     *
     */

    forceinline SetVarImp*
    SetVarImp::copy(Space& home) {
        return copied() ? static_cast<SetVarImp*>(forward()) : perform_copy(home);
    }

    /*
     * Iterator specializations
     *
     */

    /**
     * \brief Range iterator for the least upper bound of a set variable implementation
     *
     * This class provides (by specialization) a range iterator
     * for the least upper bounds of set variable implementations.
     *
     * \ingroup TaskActorSet
     */
    template <>
    class LubRanges<SetVarImp*> : public BndSetRanges {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges(void) = default;
        /// Initialize with ranges for variable implementation \a x
        LubRanges(const SetVarImp*);
        /// Initialize with ranges for variable implementation \a x
        void init(const SetVarImp*);
        //@}
    };

    forceinline
    LubRanges<SetVarImp*>::LubRanges(const SetVarImp* x)
        : BndSetRanges(x->lub) {}

    forceinline void
    LubRanges<SetVarImp*>::init(const SetVarImp* x) {
        BndSetRanges::init(x->lub);
    }

    /**
     * \brief Range iterator for the greatest lower bound of a set variable implementation
     *
     * This class provides (by specialization) a range iterator
     * for the greatest lower bounds of set variable implementations.
     *
     * \ingroup TaskActorSet
     */
    template <>
    class GlbRanges<SetVarImp*> : public BndSetRanges {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges(void) = default;
        /// Initialize with ranges for variable implementation \a x
        GlbRanges(const SetVarImp* x);
        /// Initialize with ranges for variable implementation \a x
        void init(const SetVarImp*);
        //@}
    };

    forceinline
    GlbRanges<SetVarImp*>::GlbRanges(const SetVarImp* x)
        : BndSetRanges(x->glb) {}

    forceinline void
    GlbRanges<SetVarImp*>::init(const SetVarImp* x) {
        BndSetRanges::init(x->glb);
    }

}} // namespace Gecode::Set

// STATISTICS: set-var
