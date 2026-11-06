/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *     Christian Schulte, 2009
 *     Guido Tack, 2009
 *
 *  This file is part of Gecode, the generic constrain
 *  development environment:
 *     http://www.gecode.dev
 *
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
 */

namespace Gecode { namespace Int {
    namespace GCC {

        /// Return index of \a v in array \a a
        template <class T>
        forceinline bool
        lookup_value(T& a, int v, int& i) {
            int l = 0;
            int r = a.size() - 1;

            while (l <= r) {
                int m = l + (r - l) / 2;
                if (v == a[m].card()) {
                    i = m;
                    return true;
                }
                if (l == r) {
                    return false;
                }
                if (v < a[m].card()) {
                    r = m - 1;
                } else {
                    l = m + 1;
                }
            }
            return false;
        }

        /// Constant view containing lower and upper cardinality bounds
        class CardConst {
        private:
            /// Lower bound
            int _min;
            /// Upper bound
            int _max;
            /// Cardinality information
            int _card;
            /// Counting information
            int _counter;

        public:
            /// This view does not require propagation
            static const bool propagate = false;

            /// \name Initialization
            //@{
            /// Default constructor
            CardConst();
            /// Initialize with \a min, \a max, and cardinality \a c
            void init(Space& home, int min, int max, int c);
            //@}

            /// \name Value access
            //@{
            /// Return minimum of domain
            int min() const;
            /// Return maximum of domain
            int max() const;
            /// Return cardinality
            int card() const;
            /// Return the number of times the value occurs
            int counter() const;
            //@}

            /// \name Domain tests
            ///@{
            /// Test whether view is assigned
            bool assigned() const;
            //@}

            /// \name Domain update by value
            ///@{
            /// Set counter to \a n
            void counter(int n);
            /// Increment counter
            ModEvent inc();
            /// Restrict domain values to be less or equal than \a n
            ModEvent lq(Space& home, int n);
            /// Restrict domain values to be greater or equal than \a n
            ModEvent gq(Space& home, int n);
            /// Restrict domain values to be equal to \a n
            ModEvent eq(Space& home, int n);
            ///@}

            /// \name Dependencies
            ///@{
            /// Subscribe propagator \a p with propagation condition \a pc to view
            void subscribe(Space& home, Propagator& p, PropCond pc, bool process = true);
            /// Cancel subscription of propagator \a p with propagation condition \a pc to view
            void cancel(Space& home, Propagator& p, PropCond pc);
            /// Schedule propagator \a p
            void reschedule(Space& home, Propagator& p, PropCond pc);
            ///@}

            /// \name Cloning
            ///@{
            /// Update this view to be a clone of view \a x
            void update(Space& home, CardConst& x);
            ///@}

            /// Return used IntView (cannot be used)
            IntView base() const;
        };

        /// Cardinality integer view
        class CardView : public DerivedView<IntView> {
        protected:
            using DerivedView<IntView>::x;
            /// Cardinality
            int _card;
            /// Counter
            int _counter;

        public:
            /// This view does require propagation
            static const bool propagate = true;
            /// \name Initialization
            //@{
            /// Default constructor
            CardView();
            /// Initialize with integer view \a y and value \a c
            void init(const IntView& y, int c);
            /// Initialize for set \a s and cardinality \a c
            void init(Space& home, const IntSet& s, int c);
            //@}

            /// \name Value access
            //@{
            /// Return minimum of domain
            int min() const;
            /// Return maximum of domain
            int max() const;
            /// Return size (cardinality) of domain
            unsigned int size() const;
            /// Return the number of times the value occurs
            int counter() const;
            /// Return cardinality
            int card() const;
            ///@}

            /// \name Domain update by value
            ///@{
            /// Set the counter to the number of times value \a n occurs
            void counter(int n);
            /// Increment counter
            ModEvent inc();
            /// Restrict domain values to be less or equal than \a n
            ModEvent lq(Space& home, int n);
            /// Restrict domain values to be greater or equal than \a n
            ModEvent gq(Space& home, int n);
            /// Restrict domain values to be equal to \a n
            ModEvent eq(Space& home, int n);
            ///@}

            /// \name Domain update by iterator
            //@{
            /// Replace domain by values described by \a i
            template <class I>
            ModEvent narrow_v(Space& home, I& i, bool depends = true);
            /// Intersect domain with values described by \a i
            template <class I>
            ModEvent inter_v(Space& home, I& i, bool depends = true);
            /// Remove from domain the values described by \a i
            template <class I>
            ModEvent minus_v(Space& home, I& i, bool depends = true);
            //@}

            /// \name Cloning
            ///@{
            /// Update this view to be a clone of view \a y
            void update(Space& home, CardView& y);
            ///@}
        };

        /*
         * Constant cardinality view
         *
         */
        forceinline
        // NOLINTNEXTLINE(modernize-use-equals-default)
        CardConst::CardConst() {}
        forceinline void
        CardConst::init(Space&, int min, int max, int c) {
            _min = min;
            _max = max;
            _card = c;
            _counter = 0;
        }

        forceinline int
        CardConst::min() const {
            return _min;
        }
        forceinline int
        CardConst::max() const {
            return _max;
        }
        forceinline int
        CardConst::card() const {
            return _card;
        }
        forceinline int
        CardConst::counter() const {
            return _counter;
        }
        forceinline bool
        CardConst::assigned() const {
            return _min == _max;
        }

        forceinline void
        CardConst::counter(int n) {
            _counter = n;
        }
        forceinline ModEvent
        CardConst::inc() {
            if (++_counter > _max)
                return ME_INT_FAILED;
            return ME_INT_NONE;
        }
        forceinline ModEvent
        CardConst::lq(Space&, int n) {
            if (_min > n)
                return ME_INT_FAILED;
            return ME_INT_NONE;
        }
        forceinline ModEvent
        CardConst::gq(Space&, int n) {
            if (_max < n)
                return ME_INT_FAILED;
            return ME_INT_NONE;
        }
        forceinline ModEvent
        CardConst::eq(Space&, int n) {
            if ((_min > n) || (_max < n))
                return ME_INT_FAILED;
            return ME_INT_NONE;
        }

        forceinline void
        CardConst::subscribe(Space&, Propagator&, PropCond, bool) {}
        forceinline void
        CardConst::cancel(Space&, Propagator&, PropCond) {}
        forceinline void
        CardConst::reschedule(Space&, Propagator&, PropCond) {}

        forceinline void
        CardConst::update(Space&, CardConst& x) {
            _min = x._min;
            _max = x._max;
            _card = x._card;
            _counter = x._counter;
        }

        forceinline IntView
        CardConst::base() const {
            GECODE_NEVER;
            return IntView();
        }

        /*
         * Cardinality integer view
         *
         */
        forceinline
        // NOLINTNEXTLINE(modernize-use-equals-default)
        CardView::CardView() {}
        forceinline void
        CardView::init(const IntView& y, int c) {
            x = y;
            _card = c;
            _counter = 0;
        }
        forceinline void
        CardView::init(Space& home, const IntSet& s, int c) {
            x = IntVar(home, s);
            _card = c;
            _counter = 0;
        }

        forceinline int
        CardView::counter() const {
            return _counter;
        }
        forceinline int
        CardView::card() const {
            return _card;
        }
        forceinline int
        CardView::min() const {
            return x.min();
        }
        forceinline int
        CardView::max() const {
            return x.max();
        }
        forceinline unsigned int
        CardView::size() const {
            return x.size();
        }

        forceinline void
        CardView::counter(int n) {
            _counter = n;
        }
        forceinline ModEvent
        CardView::inc() {
            if (++_counter > this->max())
                return ME_INT_FAILED;
            return ME_GEN_NONE;
        }
        forceinline ModEvent
        CardView::lq(Space& home, int n) {
            return x.lq(home, n);
        }
        forceinline ModEvent
        CardView::gq(Space& home, int n) {
            return x.gq(home, n);
        }
        forceinline ModEvent
        CardView::eq(Space& home, int n) {
            return x.eq(home, n);
        }

        template <class I>
        forceinline ModEvent
        CardView::narrow_v(Space& home, I& i, bool depends) {
            return x.narrow_v(home, i, depends);
        }
        template <class I>
        forceinline ModEvent
        CardView::inter_v(Space& home, I& i, bool depends) {
            return x.inter_v(home, i, depends);
        }
        template <class I>
        forceinline ModEvent
        CardView::minus_v(Space& home, I& i, bool depends) {
            return x.minus_v(home, i, depends);
        }

        forceinline void
        CardView::update(Space& home, CardView& y) {
            x.update(home, y.x);
            _card = y._card;
            _counter = y._counter;
        }

    } // namespace GCC

    /**
     * \brief %Range iterator for indexed problem variables
     */
    template <>
    class ViewRanges<GCC::CardView> : public Gecode::Int::ViewRanges<IntView> {
    public:
        /// \name Constructors and initialization
        ///@{
        /// Default constructor
        ViewRanges();
        /// Initialize with ranges for view \a x
        ViewRanges(const GCC::CardView& x);
        /// Initialize with ranges for view \a x
        void init(const GCC::CardView& x);
        ///@}
    };

    forceinline ViewRanges<GCC::CardView>::ViewRanges() = default;

    forceinline
    ViewRanges<GCC::CardView>::ViewRanges(const GCC::CardView& x)
        : Gecode::Int::ViewRanges<IntView>(x.base()) {}

    forceinline void
    ViewRanges<GCC::CardView>::init(const GCC::CardView& x) {
        Gecode::Int::ViewRanges<IntView> xi(x.base());
    }

}} // namespace Gecode::Int

// STATISTICS: int-prop
