/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Contributing authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Zayenz Lagerkvist, 2007
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

#include <bit>
#include <climits>

namespace Gecode { namespace Support {

    class RawBitSetBase;

    /// Date item for bitsets
    class BitSetData {
        friend class RawBitSetBase;

    protected:
#if defined(_MSC_VER) && defined(_M_IX86)
        /// Basetype for bits
        using Base = unsigned long int;
#else
        /// Basetype for bits
        using Base = unsigned long long int;
#endif
        /// The bits
        Base bits;

    public:
        /// Bits per base
        static const unsigned int bpb = static_cast<unsigned int>(CHAR_BIT * sizeof(Base));
        /// Initialize with all bits set if \a set_bits
        void init(bool set_bits = false);
        /// Get number of data elements for \a s bits
        static unsigned int data(unsigned int s);
        /// Test whether any bit with position greater or equal to \a i is set
        bool operator()(unsigned int i = 0U) const;
        /// Access value at bit \a i
        bool get(unsigned int i) const;
        /// Set bit \a i
        void set(unsigned int i);
        /// Clear bit \a i
        void clear(unsigned int i);
        /// Return next set bit with position greater or equal to \a i (there must be a bit)
        unsigned int next(unsigned int i = 0U) const;
        /// Whether all bits are set
        bool all() const;
        /// Whether all bits from bit 0 to bit \a i are set
        bool all(unsigned int i) const;
        /// Whether no bits are set
        bool none() const;
        /// Whether no bits from bit 0 to bit \a i are set
        bool none(unsigned int i) const;
        /// Return the number of bits set
        unsigned int ones() const;
        /// Return the number of bits not set
        unsigned int zeroes() const;
        /// Check whether exactly one bit is set
        bool one() const;
        /// Perform "and" with \a a
        void a(BitSetData a);
        /// Perform "and" with \a a for bits 0 to \a i
        void a(BitSetData a, unsigned int i);
        /// Return "and" of \a a and \a b
        static BitSetData a(BitSetData a, BitSetData b);
        /// Perform "or" with \a a
        void o(BitSetData a);
        /// Perform "or" with \a a for bits 0 to \a i
        void o(BitSetData a, unsigned int i);
        /// Return "or" of \a a and \a b
        static BitSetData o(BitSetData a, BitSetData b);
        /// Whether both hold the same bits
        bool operator==(const BitSetData&) const = default;
        /// Invert all bits in \a b
        BitSetData operator~() const;
    };

    /// Status of a bitset
    enum class BitSetStatus {
        BSS_NONE, ///< No bits set
        BSS_ALL,  ///< All bits set
        BSS_SOME  ///< Some but not all bits set
    };
    using enum BitSetStatus;

    /// Basic bitset support  (without stored size information)
    class RawBitSetBase {
    protected:
        /// Bits per base
        static const unsigned int bpb = BitSetData::bpb;
        /// Stored bits
        BitSetData* data;

    public:
        /// Copy constructor (disabled)
        RawBitSetBase(const RawBitSetBase&) = delete;
        /// Assignment operator (disabled)
        RawBitSetBase& operator=(const RawBitSetBase&) = delete;
        /// Default constructor (yields empty set)
        RawBitSetBase();
        /// Initialize for \a sz bits and allocator \a a
        template <class A>
        RawBitSetBase(A& a, unsigned int sz, bool set_bits = false);
        /// Copy from bitset \a bs with allocator \a a
        template <class A>
        RawBitSetBase(A& a, unsigned int sz, const RawBitSetBase& bs);
        /// Allocate for \a sz bits and allocator \a a (only after default constructor)
        template <class A>
        void allocate(A& a, unsigned int sz);
        /// Initialize for \a sz bits and allocator \a a (only after default constructor)
        template <class A>
        void init(A& a, unsigned int sz, bool set_bits = false);
        /// Clear \a sz bits
        void fill_bits(unsigned int sz, bool set_bits = false);
        /// Copy \a sz bits from \a bs
        void copy(unsigned int sz, const RawBitSetBase& bs);
        /// Access value at bit \a i
        bool get(unsigned int i) const;
        /// Set bit \a i
        void set(unsigned int i);
        /// Clear bit \a i
        void clear(unsigned int i);
        /// Return position greater or equal \a i of next set bit (\a i is allowed to be equal to size)
        unsigned int next(unsigned int i) const;
        /// Return status of bitset
        BitSetStatus status(unsigned int sz) const;
        /// Test whether all bits are set
        bool all(unsigned int sz) const;
        /// Test whether no bits are set
        bool none(unsigned int sz) const;
        /// Resize bitset from \a sz to \a n elememts
        template <class A>
        void resize(A& a, unsigned int sz, unsigned int n, bool set_bits = false);
        /// Dispose memory for bit set
        template <class A>
        void dispose(A& a, unsigned int sz);
    };

    /// Basic bitset support
    class BitSetBase : public RawBitSetBase {
    protected:
        /// Size of bitset (number of bits)
        unsigned int sz;

    public:
        /// Copy constructor (disabled)
        BitSetBase(const BitSetBase&) = delete;
        /// Assignment operator (disabled)
        BitSetBase& operator=(const BitSetBase&) = delete;
        /// Default constructor (yields empty set)
        BitSetBase();
        /// Initialize for \a s bits and allocator \a a
        template <class A>
        BitSetBase(A& a, unsigned int s, bool set_bits = false);
        /// Copy from bitset \a bs with allocator \a a
        template <class A>
        BitSetBase(A& a, const BitSetBase& bs);
        /// Initialize for \a s bits and allocator \a a (only after default constructor)
        template <class A>
        void init(A& a, unsigned int s, bool set_bits = false);
        /// Clear \a sz bits
        void fill_bits(bool set_bits = false);
        /// Copy \a sz bits from \a bs
        void copy(const BitSetBase& bs);
        /// Return size of bitset (number of bits)
        unsigned int size() const;
        /// Access value at bit \a i
        bool get(unsigned int i) const;
        /// Set bit \a i
        void set(unsigned int i);
        /// Clear bit \a i
        void clear(unsigned int i);
        /// Return position greater or equal \a i of next set bit (\a i is allowed to be equal to size)
        unsigned int next(unsigned int i) const;
        /// Return status of bitset
        BitSetStatus status() const;
        /// Test whether all bits are set
        bool all() const;
        /// Test whether no bits are set
        bool none() const;
        /// Resize bitset to \a n elememts
        template <class A>
        void resize(A& a, unsigned int n, bool set_bits = false);
        /// Dispose memory for bit set
        template <class A>
        void dispose(A& a);
    };

    /*
     * Bitset data
     *
     */

    forceinline void
    BitSetData::init(bool set_bits) {
        bits = set_bits ? ~static_cast<Base>(0) : static_cast<Base>(0);
    }
    forceinline unsigned int
    BitSetData::data(unsigned int s) {
        return s == 0 ? 0 : ((s - 1) / bpb + 1);
    }
    forceinline bool
    BitSetData::operator()(unsigned int i) const {
        return (bits >> i) != static_cast<Base>(0U);
    }
    forceinline bool
    BitSetData::get(unsigned int i) const {
        return (bits & (static_cast<Base>(1U) << i)) != static_cast<Base>(0U);
    }
    forceinline void
    BitSetData::set(unsigned int i) {
        bits |= (static_cast<Base>(1U) << i);
    }
    forceinline void
    BitSetData::clear(unsigned int i) {
        bits &= ~(static_cast<Base>(1U) << i);
    }
    forceinline unsigned int
    BitSetData::next(unsigned int i) const {
        assert((bits >> i) != static_cast<Base>(0));
        return static_cast<unsigned int>(std::countr_zero(bits >> i)) + i;
    }
    forceinline bool
    BitSetData::all() const {
        return bits == ~static_cast<Base>(0U);
    }
    forceinline bool
    BitSetData::all(unsigned int i) const {
        const Base mask = (static_cast<Base>(1U) << i) - static_cast<Base>(1U);
        return (bits & mask) == mask;
    }
    forceinline bool
    BitSetData::none() const {
        return bits == static_cast<Base>(0U);
    }
    forceinline bool
    BitSetData::none(unsigned int i) const {
        const Base mask = (static_cast<Base>(1U) << i) - static_cast<Base>(1U);
        return (bits & mask) == static_cast<Base>(0U);
    }

    forceinline unsigned int
    BitSetData::ones() const {
        return static_cast<unsigned int>(std::popcount(bits));
    }
    forceinline unsigned int
    BitSetData::zeroes() const {
        return bpb - ones();
    }
    forceinline bool
    BitSetData::one() const {
        return (bits & (bits - static_cast<Base>(1U))) == static_cast<Base>(0U);
    }

    forceinline void
    BitSetData::a(BitSetData a) {
        bits &= a.bits;
    }
    forceinline void
    BitSetData::a(BitSetData a, unsigned int i) {
        const Base mask = (static_cast<Base>(1U) << i) - static_cast<Base>(1U);
        bits &= (a.bits & mask);
    }
    forceinline BitSetData
    BitSetData::a(BitSetData a, BitSetData b) {
        BitSetData ab;
        ab.bits = a.bits & b.bits;
        return ab;
    }

    forceinline void
    BitSetData::o(BitSetData a) {
        bits |= a.bits;
    }
    forceinline void
    BitSetData::o(BitSetData a, unsigned int i) {
        const Base mask = (static_cast<Base>(1U) << i) - static_cast<Base>(1U);
        bits |= (a.bits & mask);
    }
    forceinline BitSetData
    BitSetData::o(BitSetData a, BitSetData b) {
        BitSetData ab;
        ab.bits = a.bits | b.bits;
        return ab;
    }

    forceinline BitSetData
    BitSetData::operator~() const {
        BitSetData iv;
        iv.bits = ~bits;
        return iv;
    }

    /*
     * Basic bit sets
     *
     */

    forceinline bool
    RawBitSetBase::get(unsigned int i) const {
        return data[i / bpb].get(i % bpb);
    }
    forceinline void
    RawBitSetBase::set(unsigned int i) {
        data[i / bpb].set(i % bpb);
    }
    forceinline void
    RawBitSetBase::clear(unsigned int i) {
        data[i / bpb].clear(i % bpb);
    }

    template <class A>
    void
    RawBitSetBase::resize(A& a, unsigned int sz, unsigned int n, bool set_bits) {
        if (n > sz) {
            data = a.template realloc<BitSetData>(data, BitSetData::data(sz + 1), BitSetData::data(n + 1));
            for (unsigned int i = BitSetData::data(sz) + 1; i < BitSetData::data(n + 1); i++) {
                data[i].init(set_bits);
            }
            for (unsigned int i = (sz % bpb); i < bpb; i++)
                if (set_bits)
                    data[sz / bpb].set(i);
                else
                    data[sz / bpb].clear(i);
        }
        set(n);
    }

    template <class A>
    forceinline void
    RawBitSetBase::dispose(A& a, unsigned int sz) {
        a.template free<BitSetData>(data, BitSetData::data(sz + 1));
    }

    forceinline
    RawBitSetBase::RawBitSetBase()
        : data(nullptr) {}

    template <class A>
    forceinline
    RawBitSetBase::RawBitSetBase(A& a, unsigned int sz, bool set_bits)
        : data(a.template alloc<BitSetData>(BitSetData::data(sz + 1))) {
        for (unsigned int i = 0U; i < BitSetData::data(sz + 1); i++)
            data[i].init(set_bits);
        // Set a bit at position sz as sentinel (for efficient next)
        set(sz);
    }

    template <class A>
    forceinline
    RawBitSetBase::RawBitSetBase(A& a, unsigned int sz, const RawBitSetBase& bs)
        : data(a.template alloc<BitSetData>(BitSetData::data(sz + 1))) {
        for (unsigned int i = 0U; i < BitSetData::data(sz + 1); i++)
            data[i] = bs.data[i];
        // Set a bit at position sz as sentinel (for efficient next)
        set(sz);
    }

    template <class A>
    forceinline void
    RawBitSetBase::allocate(A& a, unsigned int sz) {
        assert(data == nullptr);
        data = a.template alloc<BitSetData>(BitSetData::data(sz + 1));
    }

    template <class A>
    forceinline void
    RawBitSetBase::init(A& a, unsigned int sz, bool set_bits) {
        assert(data == nullptr);
        data = a.template alloc<BitSetData>(BitSetData::data(sz + 1));
        for (unsigned int i = 0U; i < BitSetData::data(sz + 1); i++)
            data[i].init(set_bits);
        // Set a bit at position sz as sentinel (for efficient next)
        set(sz);
    }

    forceinline void
    RawBitSetBase::copy(unsigned int sz, const RawBitSetBase& bs) {
        for (unsigned int i = 0U; i < BitSetData::data(sz + 1); i++)
            data[i] = bs.data[i];
    }

    forceinline void
    RawBitSetBase::fill_bits(unsigned int sz, bool set_bits) {
        for (unsigned int i = 0U; i < BitSetData::data(sz + 1); i++)
            data[i].init(set_bits);
    }

    forceinline unsigned int
    RawBitSetBase::next(unsigned int i) const {
        unsigned int pos = i / bpb;
        unsigned int bit = i % bpb;
        if (data[pos](bit))
            return pos * bpb + data[pos].next(bit);
        // The sentinel bit guarantees that this loop always terminates
        do {
            pos++;
        } while (!data[pos]());
        return pos * bpb + data[pos].next();
    }

    forceinline BitSetStatus
    RawBitSetBase::status(unsigned int sz) const {
        unsigned int pos = sz / bpb;
        unsigned int bits = sz % bpb;
        if (pos > 0) {
            if (data[0].all()) {
                for (unsigned int i = 1; i < pos; i++)
                    if (!data[i].all())
                        return BSS_SOME;
                return data[pos].all(bits) ? BSS_ALL : BSS_SOME;
            } else if (data[0].none()) {
                for (unsigned int i = 1; i < pos; i++)
                    if (!data[i].none())
                        return BSS_SOME;
                return data[pos].none(bits) ? BSS_NONE : BSS_SOME;
            } else {
                return BSS_SOME;
            }
        }
        if (data[0].all(bits))
            return BSS_ALL;
        if (data[0].none(bits))
            return BSS_NONE;
        return BSS_SOME;
    }

    forceinline bool
    RawBitSetBase::all(unsigned int sz) const {
        return status(sz) == BSS_ALL;
    }

    forceinline bool
    RawBitSetBase::none(unsigned int sz) const {
        return status(sz) == BSS_NONE;
    }

    template <class A>
    void
    BitSetBase::resize(A& a, unsigned int n, bool set_bits) {
        RawBitSetBase::resize(a, sz, n, set_bits);
        sz = n;
    }

    template <class A>
    forceinline void
    BitSetBase::dispose(A& a) {
        RawBitSetBase::dispose(a, sz);
    }

    forceinline
    BitSetBase::BitSetBase()
        : sz(0U) {}

    template <class A>
    forceinline
    BitSetBase::BitSetBase(A& a, unsigned int s, bool set_bits)
        : RawBitSetBase(a, s, set_bits), sz(s) {}

    template <class A>
    forceinline
    BitSetBase::BitSetBase(A& a, const BitSetBase& bs)
        : RawBitSetBase(a, bs.sz, bs), sz(bs.sz) {}

    template <class A>
    forceinline void
    BitSetBase::init(A& a, unsigned int s, bool set_bits) {
        assert(sz == 0);
        RawBitSetBase::init(a, s, set_bits);
        sz = s;
    }

    forceinline void
    BitSetBase::copy(const BitSetBase& bs) {
        assert(sz == bs.sz);
        RawBitSetBase::copy(sz, bs);
    }

    forceinline void
    BitSetBase::fill_bits(bool set_bits) {
        RawBitSetBase::fill_bits(sz, set_bits);
    }

    forceinline unsigned int
    BitSetBase::size() const {
        return sz;
    }

    forceinline bool
    BitSetBase::get(unsigned int i) const {
        assert(i < sz);
        return RawBitSetBase::get(i);
    }
    forceinline void
    BitSetBase::set(unsigned int i) {
        assert(i < sz);
        RawBitSetBase::set(i);
    }
    forceinline void
    BitSetBase::clear(unsigned int i) {
        assert(i < sz);
        RawBitSetBase::clear(i);
    }

    forceinline unsigned int
    BitSetBase::next(unsigned int i) const {
        assert(i <= sz);
        return RawBitSetBase::next(i);
    }

    forceinline BitSetStatus
    BitSetBase::status() const {
        return RawBitSetBase::status(sz);
    }

    forceinline bool
    BitSetBase::all() const {
        return RawBitSetBase::all(sz);
    }

    forceinline bool
    BitSetBase::none() const {
        return RawBitSetBase::none(sz);
    }

}} // namespace Gecode::Support

// STATISTICS: support-any
