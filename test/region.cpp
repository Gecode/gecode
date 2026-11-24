/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>

#include <memory>
#include <type_traits>

#include "test/test.hh"

namespace Test {

    /// %Test for %Region memory area
    class Region : public Test::Base {
    protected:
        /// How often to repeat
        static const int n_repeat = 16;
        /// How many blocks to allocate
        static const int n_blocks = 64;
        /// The size of a block
        static const size_t size = Gecode::Kernel::MemoryConfig::region_area_size * 4;

    public:
        /// Initialize test
        Region() : Test::Base("Region") {}
        /// Perform actual tests
        bool
        run() override {
            for (int i = n_repeat; i--;) {
                Gecode::Region r;
                for (int j = n_blocks; j--;)
                    (void)r.alloc<char>(static_cast<unsigned long int>(size));
            }
            return true;
        }
    };

    Region r;

    /// Space without variables
    class EmptySpace : public Gecode::Space {
    public:
        /// Initialize space
        EmptySpace() = default;
        /// Constructor for cloning \a s
        // NOLINTNEXTLINE(modernize-use-equals-default)
        EmptySpace(EmptySpace& s) : Gecode::Space(s) {}
        /// Copy space during cloning
        Gecode::Space*
        copy() override {
            return new EmptySpace(*this);
        }
    };

    /// Space class that reaches Space through a virtual base
    class VirtualBase : public virtual Gecode::Space {
    public:
        /// Initialize space
        VirtualBase() = default;
        /// Constructor for cloning \a s
        // NOLINTNEXTLINE(modernize-use-equals-default)
        VirtualBase(VirtualBase& s) : Gecode::Space(s) {}
    };

    /// Space derived from Space through a virtual base
    class VirtualSpace : public VirtualBase {
    public:
        /// A value to check that cloning copies the space
        int value = 0;
        /// Initialize space
        VirtualSpace() = default;
        /// Constructor for cloning \a s
        VirtualSpace(VirtualSpace& s) : Gecode::Space(s), VirtualBase(s), value(s.value) {}
        /// Copy space during cloning
        Gecode::Space*
        copy() override {
            return new VirtualSpace(*this);
        }
    };

    /// Polymorphic base class without a usable address-of operator
    class NoAddressBase {
    public:
        /// Destructor
        virtual ~NoAddressBase() = default;
        /// Unusable address-of operator
        void operator&() const = delete;
    };

    /// Class derived from a base class without address-of operator
    class NoAddressDerived : public NoAddressBase {
    public:
        /// A value to check the cast
        int value = 3;
    };

    /// %Test that cloning returns the static type of the cloned space
    class SpaceCloneTyped : public Test::Base {
    public:
        /// Initialize test
        SpaceCloneTyped() : Test::Base("Space::Clone::Typed") {}
        /// Cast a reference whose type has no usable address-of operator
        static bool
        no_address_cast() {
            NoAddressDerived derived;
            const NoAddressBase& base = derived;
            return Gecode::down_cast<const NoAddressDerived&>(base).value == 3;
        }
        /// Perform actual tests
        bool
        run() override {
            using namespace Gecode;
            EmptySpace home;
            const Space& erased_home = home;
            static_assert(std::is_same_v<decltype(home.clone()), EmptySpace*>);
            static_assert(std::is_same_v<decltype(erased_home.clone()), Space*>);
            const std::unique_ptr<EmptySpace> typed(home.clone());
            const std::unique_ptr<Space> erased(erased_home.clone());

            VirtualSpace virtual_home;
            virtual_home.value = 7;
            const Space& erased_virtual = virtual_home;
            const std::unique_ptr<VirtualSpace> virtual_typed(virtual_home.clone());
            const std::unique_ptr<Space> virtual_erased(erased_virtual.clone());

            return (dynamic_cast<EmptySpace*>(erased.get()) != nullptr) && (virtual_typed->value == 7) &&
                   (down_cast<const VirtualSpace&>(*virtual_erased).value == 7) &&
                   (down_cast<EmptySpace*>(static_cast<Space*>(nullptr)) == nullptr) && no_address_cast();
        }
    };

    SpaceCloneTyped space_clone_typed;

    /// %Test for growing space memory from an empty block
    class SpaceReallocEmpty : public Test::Base {
    public:
        /// Initialize test
        SpaceReallocEmpty() : Test::Base("Space::Realloc::Empty") {}
        /// Perform actual tests
        bool
        run() override {
            EmptySpace home;
            int* p = home.realloc<int>(static_cast<int*>(nullptr), 0, 4);
            for (int i = 0; i < 4; i++)
                p[i] = i;
            const bool grown = (p[3] == 3);
            home.free<int>(p, 4);
            return grown;
        }
    };

    SpaceReallocEmpty space_realloc_empty;

    /// %Test for growing and shrinking a block of pointers in a space
    class SpaceReallocPointers : public Test::Base {
    public:
        /// Initialize test
        SpaceReallocPointers() : Test::Base("Space::Realloc::Pointers") {}
        /// Perform actual tests
        bool
        run() override {
            EmptySpace home;
            int values[2] = {0, 1};
            int** p = home.alloc<int*>(2);
            p[0] = &values[0];
            p[1] = &values[1];
            // Unsigned long sizes select the overload for pointers
            p = home.realloc(p, 2UL, 4UL);
            const bool grown = (p[0] == &values[0]) && (p[1] == &values[1]);
            p = home.realloc(p, 4UL, 2UL);
            const bool reused = (home.fl_alloc<2 * sizeof(int*)>() == p + 2);
            return grown && reused && (p[1] == &values[1]);
        }
    };

    SpaceReallocPointers space_realloc_pointers;

    /// Object that counts its destructions
    class CountedObject {
    public:
        /// Where to count destructions
        int* destroyed = nullptr;
        /// Count destruction
        ~CountedObject() {
            if (destroyed != nullptr)
                (*destroyed)++;
        }
    };

    /// Test whether shrinking in \a a destroys exactly the removed objects
    template <class Allocator>
    static bool
    shrink_destroys_tail(Allocator& a) {
        int destroyed = 0;
        CountedObject* b = a.template alloc<CountedObject>(4);
        for (int i = 0; i < 4; i++)
            b[i].destroyed = &destroyed;
        b = a.template realloc<CountedObject>(b, 4, 1);
        const bool shrunk = (destroyed == 3);
        a.template free<CountedObject>(b, 1);
        return shrunk && (destroyed == 4);
    }

    /// %Test for shrinking blocks in a space
    class SpaceReallocShrink : public Test::Base {
    public:
        /// Initialize test
        SpaceReallocShrink() : Test::Base("Space::Realloc::Shrink") {}
        /// Perform actual tests
        bool
        run() override {
            EmptySpace home;
            int* p = home.alloc<int>(8);
            int* tail = p + 4;
            (void)home.realloc<int>(p, 8, 4);
            return (home.fl_alloc<4 * sizeof(int)>() == tail) && shrink_destroys_tail(home);
        }
    };

    SpaceReallocShrink space_realloc_shrink;

    /// %Test for shrinking blocks in a region
    class RegionReallocShrink : public Test::Base {
    public:
        /// Initialize test
        RegionReallocShrink() : Test::Base("Region::Realloc::Shrink") {}
        /// Perform actual tests
        bool
        run() override {
            Gecode::Region r;
            return shrink_destroys_tail(r);
        }
    };

    RegionReallocShrink region_realloc_shrink;

    /// %Test for comparing space and region allocators
    class AllocatorCompare : public Test::Base {
    public:
        /// Initialize test
        AllocatorCompare() : Test::Base("Allocator::Compare") {}
        /// Perform actual tests
        bool
        run() override {
            using namespace Gecode;
            EmptySpace home, other_home;
            const space_allocator<int> space_alloc(home);
            Gecode::Region region, other_region;
            const region_allocator<int> region_alloc(region);
            return compares_equal(space_alloc, space_allocator<double>(home)) &&
                   compares_unequal(space_alloc, space_allocator<double>(other_home)) &&
                   compares_equal(space_alloc, space_allocator<int>(space_alloc)) &&
                   compares_equal(region_alloc, region_allocator<double>(region)) &&
                   compares_unequal(region_alloc, region_allocator<double>(other_region)) &&
                   compares_equal(region_alloc, region_allocator<int>(region_alloc));
        }
    };

    AllocatorCompare allocator_compare;

    /// %Test for copying and assigning empty archives
    class ArchiveEmpty : public Test::Base {
    public:
        /// Initialize test
        ArchiveEmpty() : Test::Base("Archive::Empty") {}
        /// Perform actual tests
        bool
        run() override {
            Gecode::Archive empty;
            Gecode::Archive copy(empty);
            if (copy.size() != 0)
                return false;
            Gecode::Archive full;
            full << 1U << 2U;
            full = empty;
            if (full.size() != 0)
                return false;
            Gecode::Archive assigned;
            assigned = copy;
            if (assigned.size() != 0)
                return false;
            // The copy and the assigned archive stay usable
            copy << 3U;
            assigned << 4U;
            return (copy.size() == 1) && (copy[0] == 3U) && (assigned.size() == 1) && (assigned[0] == 4U);
        }
    };

    ArchiveEmpty archive_empty;

} // namespace Test

// STATISTICS: test-core
