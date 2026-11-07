/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

    /**
     * \brief %Range iterator for a two-dimensional array
     * \ingroup TaskActorSetView
     */
    class ArrayRanges {
    private:
        int* _ranges;
        int _size;
        int _pos;

    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        ArrayRanges() : _ranges(nullptr), _size(0), _pos(0) {}
        /// Initialize with ranges for array \a ranges which is of size \a size
        ArrayRanges(int* ranges, int size) : _ranges(ranges), _size(size), _pos(0) {}
        /// Initialize with ranges for array \a ranges which is of size \a size
        void
        init(int* ranges, int size) {
            _ranges = ranges;
            _size = size;
            _pos = 0;
        }
        //@}

        /// \name Iteration control
        //@{
        /// Test whether iterator is still at a range or done
        bool
        operator()() const {
            return _pos < _size;
        }
        /// Move iterator to next range (if possible)
        void
        operator++() {
            _pos++;
        }
        //@}

        /// \name Range access
        //@{
        /// Return smallest value of range
        int
        min() const {
            return _ranges[_pos * 2];
        }
        /// Return largest value of range
        int
        max() const {
            return _ranges[_pos * 2 + 1];
        }
        /// Return width of range (distance between minimum and maximum)
        unsigned int
        width() const {
            return static_cast<unsigned int>(_ranges[_pos * 2 + 1] - _ranges[_pos * 2] + 1);
        }
        //@}
    };

    forceinline
    ConstSetView::ConstSetView()
        : ranges(nullptr), size(0), dom_size(0) {}

    forceinline
    ConstSetView::ConstSetView(Space& home, const IntSet& dom) {
        size = dom.ranges();
        dom_size = 0;
        if (size > 0) {
            ranges = home.alloc<int>(2 * size);
            IntSetRanges dr(dom);
            for (int i = 0; dr(); ++dr, i += 2) {
                int min = dr.min();
                int max = dr.max();
                ranges[i] = min;
                ranges[i + 1] = max;
                dom_size += static_cast<unsigned int>(max - min + 1);
            }
        } else {
            ranges = nullptr;
        }
    }

    forceinline unsigned int
    ConstSetView::glb_size() const {
        return dom_size;
    }

    forceinline unsigned int
    ConstSetView::lub_size() const {
        return dom_size;
    }

    forceinline unsigned int
    ConstSetView::unknown_size() const {
        return 0;
    }

    forceinline bool
    ConstSetView::contains(int i) const {
        for (int j = size; j--;) {
            if (ranges[2 * j + 1] < i)
                return false;
            if (ranges[2 * j] >= i)
                return true;
        }
        return false;
    }

    forceinline bool
    ConstSetView::not_contains(int i) const {
        return !contains(i);
    }

    forceinline unsigned int
    ConstSetView::card_min() const {
        return dom_size;
    }

    forceinline unsigned int
    ConstSetView::card_max() const {
        return dom_size;
    }

    forceinline int
    ConstSetView::lub_min() const {
        return size == 0 ? BndSet::MIN_OF_EMPTY : ranges[0];
    }

    forceinline int
    ConstSetView::lub_max() const {
        return size == 0 ? BndSet::MAX_OF_EMPTY : ranges[size * 2 - 1];
    }

    forceinline int
    ConstSetView::glb_min() const {
        return lub_min();
    }

    forceinline int
    ConstSetView::glb_max() const {
        return lub_max();
    }

    forceinline ModEvent
    ConstSetView::card_min(Space&, unsigned int c) {
        return c <= dom_size ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::card_max(Space&, unsigned int c) {
        return c >= dom_size ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::include(Space&, int c) {
        return contains(c) ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::exclude(Space&, int c) {
        return contains(c) ? ME_SET_FAILED : ME_SET_NONE;
    }

    forceinline ModEvent
    ConstSetView::intersect(Space&, int c) {
        return (size == 0 || (size == 1 && ranges[0] == ranges[1] && ranges[0] == c)) ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::intersect(Space&, int i, int j) {
        return (glb_min() >= i && glb_max() <= j) ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::include(Space&, int i, int j) {
        Iter::Ranges::Singleton single(i, j);
        ArrayRanges ar(ranges, size);
        return (single() && Iter::Ranges::subset(single, ar)) ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    ConstSetView::exclude(Space&, int i, int j) {
        Iter::Ranges::Singleton single(i, j);
        ArrayRanges ar(ranges, size);
        return (single() && Iter::Ranges::subset(single, ar)) ? ME_SET_FAILED : ME_SET_NONE;
    }

    template <class I>
    ModEvent
    ConstSetView::exclude_i(Space&, I& i) {
        ArrayRanges ar(ranges, size);
        return (i() && Iter::Ranges::subset(i, ar)) ? ME_SET_FAILED : ME_SET_NONE;
    }

    template <class I>
    ModEvent
    ConstSetView::include_i(Space&, I& i) {
        ArrayRanges ar(ranges, size);
        return Iter::Ranges::subset(i, ar) ? ME_SET_NONE : ME_SET_FAILED;
    }

    template <class I>
    ModEvent
    ConstSetView::intersect_i(Space&, I& i) {
        ArrayRanges ar(ranges, size);
        return Iter::Ranges::subset(ar, i) ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline void
    ConstSetView::update(Space& home, ConstSetView& p) {
        ConstView<SetView>::update(home, p);
        // dispose old ranges
        if (size > 0)
            home.free<int>(ranges, 2);

        dom_size = p.dom_size;
        size = p.size;
        if (size == 0) {
            ranges = nullptr;
        } else {
            // copy ranges from p
            ranges = home.alloc<int>(2 * size);
            for (int i = size; i--;) {
                ranges[2 * i] = p.ranges[2 * i];
                ranges[2 * i + 1] = p.ranges[2 * i + 1];
            }
        }
    }

    /*
     * Delta information for advisors
     *
     */
    forceinline int
    ConstSetView::glb_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }
    forceinline int
    ConstSetView::glb_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }
    forceinline bool
    ConstSetView::glb_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }
    forceinline int
    ConstSetView::lub_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }
    forceinline int
    ConstSetView::lub_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }
    forceinline bool
    ConstSetView::lub_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }

    forceinline unsigned int
    EmptyView::glb_size() const {
        return 0;
    }

    forceinline unsigned int
    EmptyView::lub_size() const {
        return 0;
    }

    forceinline unsigned int
    EmptyView::unknown_size() const {
        return 0;
    }

    forceinline bool
    EmptyView::contains(int) const {
        return false;
    }

    forceinline bool
    EmptyView::not_contains(int) const {
        return true;
    }

    forceinline unsigned int
    EmptyView::card_min() const {
        return 0;
    }

    forceinline unsigned int
    EmptyView::card_max() const {
        return 0;
    }

    forceinline int
    EmptyView::lub_min() const {
        return 0;
    }

    forceinline int
    EmptyView::lub_max() const {
        return 0;
    }

    forceinline int
    EmptyView::glb_min() const {
        return 0;
    }

    forceinline int
    EmptyView::glb_max() const {
        return 0;
    }

    forceinline ModEvent
    EmptyView::card_min(Space&, unsigned int c) {
        return c == 0 ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    EmptyView::card_max(Space&, unsigned int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    EmptyView::include(Space&, int) {
        return ME_SET_FAILED;
    }

    forceinline ModEvent
    EmptyView::exclude(Space&, int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    EmptyView::intersect(Space&, int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    EmptyView::intersect(Space&, int, int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    EmptyView::include(Space&, int, int) {
        return ME_SET_FAILED;
    }

    forceinline ModEvent
    EmptyView::exclude(Space&, int, int) {
        return ME_SET_NONE;
    }

    template <class I>
    ModEvent
    EmptyView::exclude_i(Space&, I&) {
        return ME_SET_NONE;
    }

    template <class I>
    ModEvent
    EmptyView::include_i(Space&, I& i) {
        return i() ? ME_SET_FAILED : ME_SET_NONE;
    }

    template <class I>
    ModEvent
    EmptyView::intersect_i(Space&, I&) {
        return ME_SET_NONE;
    }

    /*
     * Delta information for advisors
     *
     */
    forceinline int
    EmptyView::glb_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline int
    EmptyView::glb_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline bool
    EmptyView::glb_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }

    forceinline int
    EmptyView::lub_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline int
    EmptyView::lub_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline bool
    EmptyView::lub_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }

    // Constant universe variable

    forceinline unsigned int
    UniverseView::glb_size() const {
        return Set::Limits::card;
    }

    forceinline unsigned int
    UniverseView::lub_size() const {
        return Set::Limits::card;
    }

    forceinline unsigned int
    UniverseView::unknown_size() const {
        return 0;
    }

    forceinline bool
    UniverseView::contains(int) const {
        return true;
    }

    forceinline bool
    UniverseView::not_contains(int) const {
        return false;
    }

    forceinline unsigned int
    UniverseView::card_min() const {
        return Set::Limits::card;
    }

    forceinline unsigned int
    UniverseView::card_max() const {
        return Limits::card;
    }

    forceinline int
    UniverseView::lub_min() const {
        return Limits::card;
    }

    forceinline int
    UniverseView::lub_max() const {
        return Limits::card;
    }

    forceinline int
    UniverseView::glb_min() const {
        return Limits::card;
    }

    forceinline int
    UniverseView::glb_max() const {
        return Limits::card;
    }

    forceinline ModEvent
    UniverseView::card_min(Space&, unsigned int c) {
        return c > Limits::card ? ME_SET_FAILED : ME_SET_NONE;
    }

    forceinline ModEvent
    UniverseView::card_max(Space&, unsigned int c) {
        return c >= Limits::card ? ME_SET_NONE : ME_SET_FAILED;
    }

    forceinline ModEvent
    UniverseView::include(Space&, int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    UniverseView::exclude(Space&, int) {
        return ME_SET_FAILED;
    }

    forceinline ModEvent
    UniverseView::intersect(Space&, int) {
        return ME_SET_FAILED;
    }

    forceinline ModEvent
    UniverseView::include(Space&, int, int) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    UniverseView::exclude(Space&, int, int) {
        return ME_SET_FAILED;
    }

    template <class I>
    ModEvent
    UniverseView::exclude_i(Space&, I& i) {
        return i() ? ME_SET_FAILED : ME_SET_NONE;
    }

    template <class I>
    forceinline ModEvent
    UniverseView::include_i(Space&, I&) {
        return ME_SET_NONE;
    }

    forceinline ModEvent
    UniverseView::intersect(Space&, int i, int j) {
        return (i > Limits::min || j < Limits::max) ? ME_SET_FAILED : ME_SET_NONE;
    }

    template <class I>
    forceinline ModEvent
    UniverseView::intersect_i(Space&, I& i) {
        return (i() && (i.min() > Limits::min || i.max() < Limits::max)) ? ME_SET_FAILED : ME_SET_NONE;
    }

    /*
     * Delta information for advisors
     *
     */
    forceinline int
    UniverseView::glb_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline int
    UniverseView::glb_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline bool
    UniverseView::glb_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }

    forceinline int
    UniverseView::lub_min(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline int
    UniverseView::lub_max(const Delta&) const {
        GECODE_NEVER;
        return 0;
    }

    forceinline bool
    UniverseView::lub_any(const Delta&) const {
        GECODE_NEVER;
        return false;
    }

    /*
     * Iterators
     *
     */

    /**
     * \brief %Range iterator for least upper bound of constantly empty set view
     * \ingroup TaskActorSetView
     */
    template <>
    class LubRanges<EmptyView> : public Iter::Ranges::Empty {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges() = default;
        /// Initialize with ranges for view \a x
        LubRanges(const EmptyView& x) {
            (void)x;
        }
        /// Initialize with ranges for view \a x
        void
        init(const EmptyView& x) {
            (void)x;
        }
        //@}
    };

    /**
     * \brief %Range iterator for greatest lower bound of constantly empty set view
     * \ingroup TaskActorSetView
     */
    template <>
    class GlbRanges<EmptyView> : public Iter::Ranges::Empty {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges() = default;
        /// Initialize with ranges for view \a x
        GlbRanges(const EmptyView& x) {
            (void)x;
        }
        /// Initialize with ranges for view \a x
        void
        init(const EmptyView& x) {
            (void)x;
        }
        //@}
    };

    /**
     * \brief %Range iterator for least upper bound of constant universe set view
     * \ingroup TaskActorSetView
     */
    template <>
    class LubRanges<UniverseView> : public Iter::Ranges::Singleton {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges() : Iter::Ranges::Singleton(Limits::min, Limits::max) {}
        /// Initialize with ranges for view \a x
        LubRanges(const UniverseView& x) : Iter::Ranges::Singleton(Limits::min, Limits::max) {
            (void)x;
        }
        /// Initialize with ranges for view \a x
        void
        init(const UniverseView& x) {
            (void)x;
        }
        //@}
    };

    /**
     * \brief %Range iterator for greatest lower bound of constant universe set view
     * \ingroup TaskActorSetView
     */
    template <>
    class GlbRanges<UniverseView> : public Iter::Ranges::Singleton {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges() : Iter::Ranges::Singleton(Limits::min, Limits::max) {}
        /// Initialize with ranges for view \a x
        GlbRanges(const UniverseView& x) : Iter::Ranges::Singleton(Limits::min, Limits::max) {
            (void)x;
        }
        /// Initialize with ranges for view \a x
        void
        init(const UniverseView& x) {
            (void)x;
        }
        //@}
    };

    /**
     * \brief %Range iterator for least upper bound of constant set view
     * \ingroup TaskActorSetView
     */
    template <>
    class LubRanges<ConstSetView> {
    private:
        ArrayRanges ar;

    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        // NOLINTNEXTLINE(modernize-use-equals-default)
        LubRanges() {}
        /// Initialize with ranges for view \a x
        LubRanges(const ConstSetView& x) : ar(x.ranges, x.size) {}
        /// Initialize with ranges for view \a x
        void
        init(const ConstSetView& x) {
            ar.init(x.ranges, x.size);
        }
        //@}

        /// \name Iteration control
        //@{
        /// Test whether iterator is still at a value or done
        bool
        operator()() const {
            return ar();
        }
        /// Move iterator to next value (if possible)
        void
        operator++() {
            ++ar;
        }
        //@}

        /// \name Range access
        //@{
        /// Return smallest value of range
        int
        min() const {
            return ar.min();
        }
        /// Return largest value of range
        int
        max() const {
            return ar.max();
        }
        /// Return width of range (distance between minimum and maximum)
        unsigned int
        width() const {
            return ar.width();
        }
        //@}
    };

    /**
     * \brief %Range iterator for greatest lower bound of constant set view
     * \ingroup TaskActorSetView
     */
    template <>
    class GlbRanges<ConstSetView> : public LubRanges<ConstSetView> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        // NOLINTNEXTLINE(modernize-use-equals-default)
        GlbRanges() {}
        /// Initialize with ranges for view \a x
        GlbRanges(const ConstSetView& x) : LubRanges<ConstSetView>(x) {}
        /// Initialize with ranges for view \a x
        void
        init(const ConstSetView& x) {
            LubRanges<ConstSetView>::init(x);
        }
        //@}
    };

    forceinline bool
    ConstSetView::operator<(const ConstSetView& y) const {
        if (size < y.size)
            return true;
        if (size > y.size)
            return false;
        if (dom_size < y.dom_size)
            return true;
        if (dom_size > y.dom_size)
            return false;
        for (int i = size; i--;) {
            if (ranges[2 * i] < y.ranges[2 * i])
                return true;
            if (ranges[2 * i] > y.ranges[2 * i])
                return false;
            if (ranges[2 * i + 1] < y.ranges[2 * i + 1])
                return true;
            if (ranges[2 * i + 1] > y.ranges[2 * i + 1])
                return false;
        }
        return false;
    }

    /*
     * Testing
     *
     */
    forceinline bool
    operator==(const ConstSetView& x, const ConstSetView& y) {
        if ((x.size != y.size) || (x.dom_size != y.dom_size))
            return false;
        for (int i = 0; i < x.size; i++)
            if ((x.ranges[2 * i] != y.ranges[2 * i]) || (x.ranges[2 * i + 1] != y.ranges[2 * i + 1]))
                return false;
        return true;
    }

    forceinline bool
    operator==(const EmptyView&, const EmptyView&) {
        return true;
    }

    forceinline bool
    operator==(const UniverseView&, const UniverseView&) {
        return true;
    }

} // namespace Gecode::Set

// STATISTICS: set-var
