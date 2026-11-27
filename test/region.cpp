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
        Region(void) : Test::Base("Region") {}
        /// Perform actual tests
        bool
        run(void) override {
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
        EmptySpace(void) = default;
        /// Constructor for cloning \a s
        // NOLINTNEXTLINE(modernize-use-equals-default)
        EmptySpace(EmptySpace& s) : Gecode::Space(s) {}
        /// Copy space during cloning
        Gecode::Space*
        copy(void) override {
            return new EmptySpace(*this);
        }
    };

    /// %Test for growing space memory from an empty block
    class SpaceReallocEmpty : public Test::Base {
    public:
        /// Initialize test
        SpaceReallocEmpty(void) : Test::Base("Space::Realloc::Empty") {}
        /// Perform actual tests
        bool
        run(void) override {
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

    /// %Test for comparing space and region allocators
    class AllocatorCompare : public Test::Base {
    public:
        /// Initialize test
        AllocatorCompare(void) : Test::Base("Allocator::Compare") {}
        /// Perform actual tests
        bool
        run(void) override {
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
        ArchiveEmpty(void) : Test::Base("Archive::Empty") {}
        /// Perform actual tests
        bool
        run(void) override {
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
