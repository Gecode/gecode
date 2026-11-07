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

#include <iostream>

namespace Gecode::Set {

    class SetVarImp;
    class LUBndSet;
    class GLBndSet;

    /**
     * \brief Finite set delta information for advisors
     *
     */
    class SetDelta : public Delta {
        friend class SetVarImp;
        friend class LUBndSet;
        friend class GLBndSet;

    private:
        int _glb_min; ///< Minimum glb value just pruned
        int _glb_max; ///< Largest glb value just pruned
        int _lub_min; ///< Minimum lub value just pruned
        int _lub_max; ///< Largest lub value just pruned
    public:
        /// Create set delta as providing no information (if \a any is true)
        SetDelta();
        /// Create set delta with \a min and \a max
        SetDelta(int glb_min, int glb_max, int lub_min, int lub_max);
        /// Return glb minimum
        int glb_min() const;
        /// Return glb maximum
        int glb_max() const;
        /// Return lub minimum
        int lub_min() const;
        /// Return lub maximum
        int lub_max() const;
        /// Test whether delta represents any domain change in glb
        bool glb_any() const;
        /// Test whether delta represents any domain change in lub
        bool lub_any() const;
    };

} // namespace Gecode::Set

#include <gecode/set/var-imp/delta.hpp>

namespace Gecode::Set {

    /**
     * \brief Sets of integers
     */
    class BndSet {
    private:
        RangeList* first;
        RangeList* last;

    protected:
        /// The size of this set
        unsigned int _size;
        /// The cardinality this set represents
        unsigned int _card;
        /// Set first range to \a f
        void fst(RangeList* f);
        /// Set last range to \a l
        void lst(RangeList* l);

        /// Return first range
        RangeList* fst() const;
        /// Return last range
        RangeList* lst() const;

    public:
        /// Returned by empty sets when asked for their maximum element
        static const int MAX_OF_EMPTY = Limits::min - 1;
        /// Returned by empty sets when asked for their minimum element
        static const int MIN_OF_EMPTY = Limits::max + 1;

        /// \name Constructors and initialization
        //@{
        /// Default constructor. Creates an empty set.
        BndSet();
        /// Initialize as the set \f$ \{i,\dots,j\}\f$
        BndSet(Space& home, int mn, int mx);
        /// Initialize as the set represented by \a s
        GECODE_SET_EXPORT BndSet(Space& home, const IntSet& s);
        //@}

        /// \name Memory management
        //@{
        /// Free memory used by this set
        void dispose(Space& home);
        //@}

        /// \name Value access
        //@{
        /// Return smallest element
        int min() const;
        /// Return greatest element
        int max() const;
        /// Return \a n -th smallest element
        int min_n(unsigned int n) const;
        /// Return size
        unsigned int size() const;
        /// Return cardinality
        unsigned int card() const;
        /// Set cardinality
        void card(unsigned int c);
        //@}

        /// \name Tests
        //@{
        /// Test whether this set is empty
        bool empty() const;
        /// Test whether \a i is an element of this set
        bool in(int i) const;
        //@}

        /// \name Update operations
        //@{
        /// Make this set equal to \a that
        void become(Space& home, const BndSet& that);
        //@}

        /// \name Range list access for iteration
        //@{
        /// Return range list for iteration
        RangeList* ranges() const;
        //@}

    protected:
        /// Overwrite the ranges with those represented by \a i
        template <class I>
        bool overwrite(Space& home, I& i);

    public:
        /// \name Cloning
        //@{
        /// Update this set to be a clone of set \a d
        void update(Space& home, BndSet& d);
        //@}

        /// Check whether internal invariants hold
        GECODE_SET_EXPORT bool is_consistent() const;
    };

    /**
     * \brief Range iterator for integer sets
     *
     */
    class BndSetRanges : public Iter::Ranges::RangeList {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        BndSetRanges() = default;
        /// Initialize with BndSet \a s
        BndSetRanges(const BndSet& s);
        /// Initialize with BndSet \a s
        void init(const BndSet& s);
        //@}
    };

    /**
     * \brief Growing sets of integers
     *
     * These sets provide operations for monotonically growing the set.
     * Growing sets are used for implementing the greatest lower bound of
     * set variables.
     */
    class GLBndSet : public BndSet {
    private:
        /// Include the set \f$\{i,\dots,j\}\f$ in this set
        GECODE_SET_EXPORT bool include_full(Space& home, int, int, SetDelta&);

    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor. Creates an empty set.
        GLBndSet() = default;
        /// Default constructor. Creates an empty set.
        GLBndSet(Space&);
        /// Initialize as the set \f$ \{i,\dots,j\}\f$
        GLBndSet(Space& home, int i, int ma);
        /// Initialize as the set represented by \a s
        GLBndSet(Space& home, const IntSet& s);
        /// Initialize as the empty set
        void init(Space& home);
        //@}

        /// \name Update operations
        //@{
        /// Include the set \f$\{i,\dots,j\}\f$ in this set
        bool include(Space& home, int i, int ma, SetDelta& d);
        /// Include the set represented by \a i in this set
        template <class I>
        bool include_i(Space& home, I& i);
        //@}

        GLBndSet(const GLBndSet&) = delete;
        const GLBndSet& operator=(const GLBndSet&) = delete;
    };

    /**
     * \brief Shrinking sets of integers
     *
     * These sets provide operations for monotonically shrinking the set.
     * Shrinking sets are used for implementing the least upper bound of
     * set variables.
     */
    class LUBndSet : public BndSet {
    private:
        GECODE_SET_EXPORT bool exclude_full(Space& home, int, int, SetDelta&);
        GECODE_SET_EXPORT bool intersect_full(Space& home, int i, int j);

    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor. Creates an empty set.
        LUBndSet() = default;
        /// Initialize as the full set including everything between Limits::min and Limits::max
        LUBndSet(Space& home);
        /// Initialize as the set \f$ \{i,\dots,j\}\f$
        LUBndSet(Space& home, int i, int ma);
        /// Initialize as the set represented by \a s
        LUBndSet(Space& home, const IntSet& s);
        /// Initialize as the full set including everything between Limits::min and Limits::max
        void init(Space& home);
        //@}

        /// \name Update operations
        //@{
        /// Exclude the set \f$\{i,\dots,j\}\f$ from this set
        bool exclude(Space& home, int i, int ma, SetDelta& d);
        /// Intersect this set with the set \f$\{i,\dots,j\}\f$
        bool intersect(Space& home, int i, int ma);
        /// Exclude all elements not in the set represented by \a i from this set
        template <class I>
        bool intersect_i(Space& home, I& i);
        /// Exclude all elements in the set represented by \a i from this set
        template <class I>
        bool exclude_i(Space& home, I& i);
        /// Exclude all elements from this set
        void exclude_all(Space& home);
        //@}

        LUBndSet(const LUBndSet&) = delete;
        const LUBndSet& operator=(const LUBndSet&) = delete;
    };

    /*
     * Iterators
     *
     */

    /**
     * \brief A complement iterator spezialized for the %BndSet limits
     *
     * \ingroup TaskActorSet
     */
    template <class I>
    class RangesCompl : public Iter::Ranges::Compl<Limits::min, Limits::max, I> {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        RangesCompl() = default;
        /// Initialize with iterator \a i
        RangesCompl(I& i);
        /// Initialize with iterator \a i
        void init(I& i);
        //@}
    };

    /**
     * \brief Range iterator for the least upper bound
     *
     * This class provides (by specialization) a range iterator
     * for the least upper bounds of all set views.
     *
     * Note that this template class serves only as a specification
     * of the interface of the various specializations.
     *
     * \ingroup TaskActorSet
     */
    template <class T>
    class LubRanges {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        LubRanges();
        /// Initialize with least upper bound ranges for set variable \a x
        LubRanges(const T& x);
        /// Initialize with least upper bound ranges for set variable \a x
        void init(const T& x);
        //@}

        /// \name Iteration control
        //@{
        /// Test whether iterator is still at a range or done
        bool operator()() const;
        /// Move iterator to next range (if possible)
        void operator++();
        //@}

        /// \name Range access
        //@{
        /// Return smallest value of range
        int min() const;
        /// Return largest value of range
        int max() const;
        /// Return width of range (distance between minimum and maximum)
        unsigned int width() const;
        //@}
    };

    /**
     * \brief Range iterator for the greatest lower bound
     *
     * This class provides (by specialization) a range iterator
     * for the greatest lower bounds of all set views.
     *
     * Note that this template class serves only as a specification
     * of the interface of the various specializations.
     *
     * \ingroup TaskActorSet
     */
    template <class T>
    class GlbRanges {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        GlbRanges();
        /// Initialize with greatest lower bound ranges for set variable \a x
        GlbRanges(const T& x);
        /// Initialize with greatest lower bound ranges for set variable \a x
        void init(const T& x);
        //@}

        /// \name Iteration control
        //@{
        /// Test whether iterator is still at a range or done
        bool operator()() const;
        /// Move iterator to next range (if possible)
        void operator++();
        //@}

        /// \name Range access
        //@{
        /// Return smallest value of range
        int min() const;
        /// Return largest value of range
        int max() const;
        /// Return width of range (distance between minimum and maximum)
        unsigned int width() const;
        //@}
    };

    /**
     * \brief Range iterator for the unknown set
     *
     * This class provides a range iterator
     * for the unknown set of all set views. The unknown set is the
     * difference between least upper and greatest lower bound, i.e.
     * those elements which still may be in the set, but are not yet
     * known to be in.
     *
     * \ingroup TaskActorSet
     */
    template <class T>
    class UnknownRanges : public Iter::Ranges::Diff<LubRanges<T>, GlbRanges<T>> {
    private:
        LubRanges<T> i1;
        GlbRanges<T> i2;

    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        UnknownRanges() = default;
        /// Initialize with unknown set ranges for set variable \a v
        UnknownRanges(const T& v);
        /// Initialize with unknown set ranges for set variable \a v
        void init(const T& v);
        //@}
    };

} // namespace Gecode::Set

#include <gecode/set/var-imp/integerset.hpp>
#include <gecode/set/var-imp/iter.hpp>

namespace Gecode::Set {

    /**
     * \brief Finite integer set variable implementation
     *
     * \ingroup Other
     */
    class SetVarImp : public SetVarImpBase {
        friend class LubRanges<SetVarImp*>;
        friend class GlbRanges<SetVarImp*>;

    private:
        /// The least upper bound of the domain
        LUBndSet lub;
        /// The greatest lower bound of the domain
        GLBndSet glb;

    protected:
        /// Constructor for cloning \a x
        SetVarImp(Space& home, SetVarImp& x);

    public:
        /// \name Constructors and initialization
        //@{
        /// Initialize with empty lower and full upper bound
        SetVarImp(Space& home);
        /**
         * \brief Initialize with given bounds and cardinality
         *
         * Creates a set variable \f$s\f$ with
         * \f$\mathit{glb}(s)=\{\mathit{glb_min},\dots,\mathit{glb_max}\}\f$,
         * \f$\mathit{lub}(s)=\{\mathit{lub_min},\dots,\mathit{lub_max}\}\f$, and
         * \f$\mathit{card_min}\leq |s|\leq\mathit{card_max}\f$
         */
        SetVarImp(Space& home, int glb_min, int glb_max, int lub_min, int lub_max, unsigned int card_min = 0,
                  unsigned int card_max = Limits::card);
        /**
         * \brief Initialize with given bounds and cardinality
         *
         * Creates a set variable \f$s\f$ with
         * \f$\mathit{glb}(s)=\mathit{glbD}\f$,
         * \f$\mathit{lub}(s)=\{\mathit{lub_min},\dots,\mathit{lub_max}\}\f$, and
         * \f$\mathit{card_min}\leq |s|\leq\mathit{card_max}\f$
         */
        SetVarImp(Space& home, const IntSet& the_glb, int lub_min, int lub_max, unsigned int card_min,
                  unsigned int card_max);
        /**
         * \brief Initialize with given bounds and cardinality
         *
         * Creates a set variable \f$s\f$ with
         * \f$\mathit{glb}(s)=\{\mathit{glb_min},\dots,\mathit{glb_max}\}\f$,
         * \f$\mathit{lub}(s)=\mathit{lubD}\f$, and
         * \f$\mathit{card_min}\leq |s|\leq\mathit{card_max}\f$
         */
        SetVarImp(Space& home, int glb_min, int glb_max, const IntSet& the_lub, unsigned int card_min,
                  unsigned int card_max);
        /**
         * \brief Initialize with given bounds and cardinality
         *
         * Creates a set variable \f$s\f$ with
         * \f$\mathit{glb}(s)=\mathit{glbD}\f$,
         * \f$\mathit{lub}(s)=\mathit{lubD}\f$, and
         * \f$\mathit{card_min}\leq |s|\leq\mathit{card_max}\f$
         */
        SetVarImp(Space& home, const IntSet& the_glb, const IntSet& the_lub, unsigned int card_min,
                  unsigned int card_max);
        //@}

        /// \name Value access
        //@{
        /// Return current cardinality minimum
        unsigned int card_min() const;
        /// Return current cardinality maximum
        unsigned int card_max() const;
        /// Return minimum of the least upper bound
        int lub_min() const;
        /// Return maximum of the least upper bound
        int lub_max() const;
        /// Return \a n -th smallest element in the least upper bound
        int lub_min_n(unsigned int n) const;
        /// Return minimum of the greatest lower bound
        int glb_min() const;
        /// Return maximum of the greatest lower bound
        int glb_max() const;
        /// Return the size of the greatest lower bound
        unsigned int glb_size() const;
        /// Return the size of the least upper bound
        unsigned int lub_size() const;
        //@}

        /// \name Domain tests
        //@{
        /// Test whether variable is assigned
        bool assigned() const;
        /// Test whether \a i is contained in greatest lower bound
        bool known_in(int i) const;
        /// Test whether \a n is not contained in least upper bound
        bool known_out(int) const;
        //@}

    private:
        /// \name Domain update by range iterator, implementations
        //@{
        /// Include set described by \a i in the greatest lower bound
        template <class I>
        ModEvent include_i_full(Space& home, int mi, int ma, I& i);
        /// Exclude set described by \a i from the least upper bound
        template <class I>
        ModEvent exclude_i_full(Space& home, int mi, int ma, I& i);
        /// Exclude everything but set described by \a i from the least upper bound
        template <class I>
        ModEvent intersect_i_full(Space& home, int mi, int ma, I& i);
        //@}

        GECODE_SET_EXPORT ModEvent process_lub_change(Space& home, SetDelta& d);
        GECODE_SET_EXPORT ModEvent process_glb_change(Space& home, SetDelta& d);

        /// \name Cardinality update implementation
        //@{
        /// Restrict cardinality to be at least n
        GECODE_SET_EXPORT ModEvent card_min_full(Space& home);
        /// Restrict cardinality to be at most n
        GECODE_SET_EXPORT ModEvent card_max_full(Space& home);
        //@}

    public:
        /// \name Domain update by value
        //@{
        /// Include \a i in the greatest lower bound
        ModEvent include(Space& home, int i);
        /// Include the range \f$\{i,\dots,j\}\f$ in the greatest lower bound
        ModEvent include(Space& home, int i, int j);
        /// Exclude \a i from the least upper bound
        ModEvent exclude(Space& home, int i);
        /// Exclude the range \f$\{i,\dots,j\}\f$ from the least upper bound
        ModEvent exclude(Space& home, int i, int j);
        /// Exclude everything but \a i from the least upper bound
        ModEvent intersect(Space& home, int i);
        /// Exclude everything but the range \f$\{i,\dots,j\}\f$ from the least upper bound
        ModEvent intersect(Space& home, int i, int j);
        /// Restrict cardinality to be at least n
        ModEvent card_min(Space& home, unsigned int n);
        /// Restrict cardinality to be at most n
        ModEvent card_max(Space& home, unsigned int n);
        //@}

        /// \name Domain update by range iterator
        //@{
        /// Include set described by \a i in the greatest lower bound
        template <class I>
        ModEvent include_i(Space& home, I& i);
        /// Exclude set described by \a i from the least upper bound
        template <class I>
        ModEvent exclude_i(Space& home, I& i);
        /// Exclude everything but set described by \a i from the least upper bound
        template <class I>
        ModEvent intersect_i(Space& home, I& i);
        //@}

        /// \name Dependencies
        //@{
        /**
         * \brief Subscribe propagator \a p with propagation condition \a pc to variable
         *
         * In case \a schedule is false, the propagator is just subscribed but
         * not scheduled for execution (this must be used when creating
         * subscriptions during propagation).
         */
        GECODE_SET_EXPORT void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule = true);
        /// Re-schedule propagator \a p with propagation condition \a pc
        GECODE_SET_EXPORT void reschedule(Space& home, Propagator& p, PropCond pc);
        /** \brief Subscribe advisor \a a to variable
         *
         * The advisor \a a is only subscribed if \a assigned is false.
         *
         * If \a fail is true, the advisor \a a is also run when a variable
         * operation triggers failure. This feature is undocumented.
         *
         */
        GECODE_SET_EXPORT void subscribe(Space& home, Advisor& a, bool fail);
        //@}

    private:
        /// Return copy of not-yet copied variable
        GECODE_SET_EXPORT SetVarImp* perform_copy(Space& home);

    public:
        /// \name Cloning
        //@{
        /// Return copy of this variable
        SetVarImp* copy(Space& home);
        //@}

        /// \name Delta information for advisors
        //@{
        /// Return minimum value just pruned from glb
        static int glb_min(const Delta& d);
        /// Return maximum value just pruned from glb
        static int glb_max(const Delta& d);
        /// Test whether arbitrary values got pruned from glb
        static bool glb_any(const Delta& d);
        /// Return minimum value just pruned from lub
        static int lub_min(const Delta& d);
        /// Return maximum value just pruned from lub
        static int lub_max(const Delta& d);
        /// Test whether arbitrary values got pruned from lub
        static bool lub_any(const Delta& d);
        //@}
    };

    class SetView;

} // namespace Gecode::Set

#include <gecode/set/var-imp/set.hpp>

// STATISTICS: set-var
