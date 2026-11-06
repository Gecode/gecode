/*
 *  Main authors:
 *     Gregory Crosswhite <gcross@phys.washington.edu>
 *
 *  Copyright:
 *     Gregory Crosswhite, 2011
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
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

#include <type_traits>
#include <vector>

#include "test/test.hh"

/// Check the test result and handle failed test
#define CHECK_TEST(T, M)                                                                                               \
    do {                                                                                                               \
        if (opt.log)                                                                                                   \
            olog << ind(3) << "Check: " << (M) << '\n';                                                                \
        if (!(T)) {                                                                                                    \
            problem = (M);                                                                                             \
            goto failed;                                                                                               \
        }                                                                                                              \
    } while (false)

/// Start new test
#define START_TEST(T)                                                                                                  \
    do {                                                                                                               \
        if (opt.log) {                                                                                                 \
            olog.str("");                                                                                              \
            olog << ind(2) << "Testing: " << (T) << '\n';                                                              \
        }                                                                                                              \
        test = (T);                                                                                                    \
    } while (false)

namespace Test {

    /*
     * Shallow handles copy a few pointers or ids and must stay trivially
     * copyable, so that a hand-written copy member cannot come back.
     */
    static_assert(std::is_trivially_copyable_v<Gecode::Home>);
    static_assert(std::is_trivially_copyable_v<Gecode::Group>);
    static_assert(std::is_trivially_copyable_v<Gecode::PropagatorGroup>);
    static_assert(std::is_trivially_copyable_v<Gecode::BrancherGroup>);
    static_assert(std::is_trivially_copyable_v<Gecode::Pos>);
    static_assert(std::is_trivially_copyable_v<Gecode::Reify>);
    static_assert(std::is_trivially_copyable_v<Gecode::LocalHandle>);
    static_assert(std::is_trivially_copyable_v<Gecode::AFC>);
    static_assert(std::is_trivially_copyable_v<Gecode::IntVar>);
    static_assert(std::is_trivially_copyable_v<Gecode::BoolVar>);
    static_assert(std::is_trivially_copyable_v<Gecode::IntVarArray>);
    static_assert(std::is_trivially_copyable_v<Gecode::BoolVarArray>);
    static_assert(std::is_trivially_copyable_v<Gecode::IntAFC>);
    static_assert(std::is_trivially_copyable_v<Gecode::BoolAFC>);
    static_assert(std::is_trivially_copyable_v<Gecode::OpenIntVarList>);
    static_assert(std::is_trivially_copyable_v<Gecode::OpenBoolVarList>);
    static_assert(std::is_trivially_copyable_v<Gecode::OpenViewList<Gecode::Int::IntView>>);
    static_assert(std::is_trivially_copyable_v<Gecode::ViewArray<Gecode::Int::IntView>>);
    static_assert(std::is_trivially_copyable_v<Gecode::ViewArray<Gecode::Int::BoolView>>);
#ifdef GECODE_HAS_SET_VARS
    static_assert(std::is_trivially_copyable_v<Gecode::SetVar>);
    static_assert(std::is_trivially_copyable_v<Gecode::SetVarArray>);
    static_assert(std::is_trivially_copyable_v<Gecode::SetAFC>);
    static_assert(std::is_trivially_copyable_v<Gecode::ViewArray<Gecode::Set::SetView>>);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    static_assert(std::is_trivially_copyable_v<Gecode::FloatVar>);
    static_assert(std::is_trivially_copyable_v<Gecode::FloatVarArray>);
    static_assert(std::is_trivially_copyable_v<Gecode::FloatAFC>);
    static_assert(std::is_trivially_copyable_v<Gecode::ViewArray<Gecode::Float::FloatView>>);
#endif

#ifdef GECODE_HAS_FLOAT_VARS
    // A float value is two bounds and stays trivially copyable.
    static_assert(std::is_trivially_copyable_v<Gecode::FloatVal>);
    static_assert(std::is_trivially_copyable_v<Gecode::Float::ScaleView>);
#endif

    // Scoped public enums do not convert to int.
    static_assert(!std::is_convertible_v<Gecode::IntRelType, int>);

    // IntPropLevels keeps levels in bit-fields, so the type must be unsigned.
    static_assert(std::is_unsigned_v<std::underlying_type_t<Gecode::IntPropLevel>>);

    /*
     * A home never changes its space, and a space is copied only by
     * cloning, so neither is assignable.
     */
    static_assert(!std::is_copy_assignable_v<Gecode::Home>);
    static_assert(!std::is_copy_assignable_v<Gecode::Space>);

    /// Tests for arrays
    namespace Array {

        /// Test name prefix
        static const std::string prefix("Array::Iterator::");

        /// %Base class for testing iterators
        class Iterator : public Test::Base {
        protected:
            /// Maximum array size
            static const int n = 16;
            /// Initialize test
            Iterator(const std::string& name) : Test::Base(prefix + name) {}
            /// Perform actual tests
            template <class Array>
            bool
            run_test_for_array(Array& a) {
                // Test/problem information.
                const char* test = "NONE";
                const char* problem = "NONE";
                // Constant reference to the array
                const Array& const_a = a;

                START_TEST("Iteration");
                {
                    typedef typename Array::reference reference;
                    typedef typename Array::pointer pointer;
                    typedef typename Array::iterator iterator;
                    const iterator begin = a.begin(), end = a.end();
                    CHECK_TEST(end - begin == a.size(), "Distance != size");
                    int index = 0;
                    iterator iter = begin;
                    for (; iter != end; ++iter, ++index) {
                        reference ref = *iter;
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &a[index], "Iterator points to the wrong element (going forward)");
                    }
                    CHECK_TEST(index == a.size(), "Iteration covered the wrong number of elements (going forward)");
                    for (; iter != begin; --iter, --index) {
                        reference ref = *(iter - 1);
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &a[index - 1], "Iterator points to the wrong element (going backwards)");
                    }
                    CHECK_TEST(index == 0, "Iteration covered the wrong number of elements (going backward)");
                }
                START_TEST("Read-only iteration");
                {
                    typedef typename Array::const_reference reference;
                    typedef typename Array::const_pointer pointer;
                    typedef typename Array::const_iterator iterator;
                    const iterator begin = const_a.begin(), end = const_a.end();
                    CHECK_TEST(end - begin == const_a.size(), "Distance != size");
                    int index = 0;
                    iterator iter = begin;
                    for (; iter != end; ++iter, ++index) {
                        reference ref = *iter;
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &const_a[index], "Iterator points to the wrong element (going forward)");
                    }
                    CHECK_TEST(index == const_a.size(),
                               "Iteration covered the wrong number of elements (going forward)");
                    for (; iter != begin; --iter, --index) {
                        reference ref = *(iter - 1);
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &const_a[index - 1],
                                   "Iterator points to the wrong element (going backwards)");
                    }
                    CHECK_TEST(index == 0, "Iteration covered the wrong number of elements (going backward)");
                }

                START_TEST("Reverse iteration");
                {
                    typedef typename Array::reference reference;
                    typedef typename Array::pointer pointer;
                    typedef typename Array::reverse_iterator iterator;
                    const iterator begin = a.rbegin(), end = a.rend();
                    CHECK_TEST(end - begin == a.size(), "Distance != size");
                    int index = a.size();
                    iterator iter = begin;
                    for (; iter != end; ++iter, --index) {
                        reference ref = *iter;
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &a[index - 1], "Iterator points to the wrong element (going forward)");
                    }
                    CHECK_TEST(index == 0, "Iteration covered the wrong number of elements (going forward)");
                    for (; iter != begin; --iter, ++index) {
                        reference ref = *(iter - 1);
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &a[index], "Iterator points to the wrong element (going backwards)");
                    }
                    CHECK_TEST(index == a.size(), "Iteration covered the wrong number of elements (going backward)");
                }

                START_TEST("Reverse read-only iteration");
                {
                    typedef typename Array::const_reference reference;
                    typedef typename Array::const_pointer pointer;
                    typedef typename Array::const_reverse_iterator iterator;
                    const iterator begin = const_a.rbegin(), end = const_a.rend();
                    CHECK_TEST(end - begin == const_a.size(), "Distance != size");
                    int index = a.size();
                    iterator iter = begin;
                    for (; iter != end; ++iter, --index) {
                        reference ref = *iter;
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &const_a[index - 1], "Iterator points to the wrong element (going forward)");
                    }
                    CHECK_TEST(index == 0, "Iteration covered the wrong number of elements (going forward)");
                    for (; iter != begin; --iter, ++index) {
                        reference ref = *(iter - 1);
                        const pointer ptr = &ref;
                        CHECK_TEST(ptr == &const_a[index], "Iterator points to the wrong element (going backwards)");
                    }
                    CHECK_TEST(index == a.size(), "Iteration covered the wrong number of elements (going backward)");
                }

                return true;
            failed:
                if (opt.log)
                    olog << "FAILURE" << '\n'
                         << ind(1) << "Test:       " << test << '\n'
                         << ind(1) << "Problem:    " << problem << '\n';
                return false;
            }
        };

        /// Test space
        class TestSpace : public Gecode::Space {
        public:
            TestSpace() {}
            // NOLINTNEXTLINE(modernize-use-equals-default)
            TestSpace(TestSpace& s) : Space(s) {}
            Space*
            copy() override {
                return new TestSpace(*this);
            }
        };

        /// Test name prefix for vector construction
        static const std::string vector_prefix("Array::Vector::");

        /// %Class for testing construction from empty vectors
        class EmptyVector : public Test::Base {
        public:
            /// Initialize test
            EmptyVector() : Test::Base(vector_prefix + "Empty") {}
            /// Perform actual tests
            bool
            run() override {
                std::vector<int> ints;
                Gecode::IntArgs ia(ints);
                if (ia.size() != 0)
                    return false;

                std::vector<Gecode::IntVar> int_vars;
                Gecode::IntVarArgs iva(int_vars);
                if (iva.size() != 0)
                    return false;

                std::vector<Gecode::BoolVar> bool_vars;
                Gecode::BoolVarArgs bva(bool_vars);
                return bva.size() == 0;
            }
        } empty_vector_test;

        /// %Class for testing the VarArray iterator
        class VarArrayIterator : public Iterator {
        protected:
            /// Maximum array size
            static const int n = 16;
            /// Array type being tested
            typedef Gecode::VarArray<Gecode::IntVar> Array;

        public:
            /// Initialize test
            VarArrayIterator() : Iterator("VarArray") {}
            /// Perform actual tests
            bool
            run() override {
                // Space for the test
                TestSpace s;
                // VarArray for the test
                Array a(s, _rand(n));
                // Run the iterator test
                return run_test_for_array(a);
            }
        } var_array_iterator_test;

        /// %Class for testing the VarArgs iterator
        class VarArgsIterator : public Iterator {
        protected:
            /// Maximum array size
            static const int n = 16;
            /// Array type being tested
            typedef Gecode::ArgArrayBase<int> Array;

        public:
            /// Initialize test
            VarArgsIterator() : Iterator("VarArgs") {}
            /// Perform actual tests
            bool
            run() override {
                // Space for the test
                TestSpace s;
                // VarArray for the test
                Array a(_rand(n));
                // Run the iterator test
                return run_test_for_array(a);
            }
        } var_args_iterator_test;

        /// %Class for testing the ViewArray iterator
        class ViewArrayIterator : public Iterator {
        protected:
            /// Maximum array size
            static const int n = 16;
            /// Array type being tested
            typedef Gecode::ViewArray<Gecode::IntVar> Array;

        public:
            /// Initialize test
            ViewArrayIterator() : Iterator("ViewArray") {}
            /// Perform actual tests
            bool
            run() override {
                // Space for the test
                TestSpace s;
                // VarArray for the test
                Array a(s, _rand(n));
                // Run the iterator test
                return run_test_for_array(a);
            }
        } view_array_iterator_test;

        /// %Class for testing the SharedArray iterator
        class SharedArrayIterator : public Iterator {
        protected:
            /// Maximum array size
            static const int n = 16;
            /// Array type being tested
            typedef Gecode::SharedArray<int> Array;

        public:
            /// Initialize test
            SharedArrayIterator() : Iterator("SharedArray") {}
            /// Perform actual tests
            bool
            run() override {
                // SharedArray for the test
                Array a(_rand(n));
                // Run the iterator test
                return run_test_for_array(a);
            }
        } shared_array_iterator_test;

        /// Copyable data whose move constructor is deleted
        class CopyOnlyData {
        public:
            /// The stored value
            int value;
            /// Initialize with \a v
            CopyOnlyData(int v) : value(v) {}
            /// Copy constructor
            CopyOnlyData(const CopyOnlyData&) = default;
            /// Deleted move constructor
            CopyOnlyData(CopyOnlyData&&) = delete;
        };

        /// %Class for testing SharedData with copyable data that cannot move
        class SharedDataCopyOnly : public Test::Base {
        public:
            /// Initialize test
            SharedDataCopyOnly() : Test::Base("Array::SharedData::CopyOnly") {}
            /// Perform actual tests
            bool
            run() override {
                // A temporary must bind to the copying constructor
                Gecode::SharedData<CopyOnlyData> d(CopyOnlyData(3));
                return d().value == 3;
            }
        } shared_data_copy_only_test;

    } // namespace Array
} // namespace Test

// STATISTICS: test-core
