/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
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

namespace Test {

    /*
     * Test tags
     *
     */
    inline TestTags::TestTags() : _mask(0) {}
    inline TestTags::TestTags(TestTag t) : _mask(static_cast<unsigned int>(t)) {}
    inline TestTags::TestTags(TestTag t0, TestTag t1)
        : _mask(static_cast<unsigned int>(t0) | static_cast<unsigned int>(t1)) {}
    inline TestTags
    TestTags::check() {
        return TestTags(TestTag::STANDARD, TestTag::CHECK);
    }
    inline TestTags
    TestTags::standard() {
        return TestTags(TestTag::STANDARD);
    }
    inline TestTags
    TestTags::sweep() {
        return TestTags(TestTag::SWEEP);
    }
    inline bool
    TestTags::empty() const {
        return _mask == 0;
    }
    inline bool
    TestTags::overlaps(TestTags t) const {
        return (_mask & t._mask) != 0;
    }
    inline void
    TestTags::add(TestTags t) {
        _mask |= t._mask;
    }

    /*
     * Commandline options
     *
     */
    inline Options::Options()
        : threads(1), seed(0), replay(false), iter(default_iterations),
          fixpoint_probability_denominator(default_fixpoint_probability_denominator), stop(true), log(false),
          test_patterns(), test_tags(), start_from(nullptr), list(false), list_tags(false), list_with_tags(false) {}

    /*
     * Base class for tests
     *
     */
    inline const std::string&
    Base::name() const {
        return _name;
    }
    inline TestTags
    Base::tags() const {
        return _tags;
    }
    inline Base*
    Base::tests() {
        return _tests;
    }
    inline Base*
    Base::next() const {
        return _next;
    }
    inline void
    Base::next(Base* n) {
        _next = n;
    }
    inline bool
    Base::fixpoint() {
        return fixpoint(_rand);
    }

    inline bool
    Base::fixpoint(Gecode::Support::RandomGenerator& rand) {
        return rand(opt.fixpoint_probability_denominator) == 0;
    }

    inline std::string
    Base::str(bool b) {
        std::stringstream s;
        if (b)
            s << "+";
        else
            s << "-";
        return s.str();
    }

    inline std::string
    Base::str(int i) {
        std::stringstream s;
        s << i;
        return s.str();
    }

    inline std::string
    Base::str(const Gecode::IntArgs& x) {
        std::string s = "";
        for (int i = 0; i < x.size() - 1; i++)
            s += str(x[i]) + ",";
        return "[" + s + str(x[x.size() - 1]) + "]";
    }

    template <class T1, class T2>
    inline bool
    Base::compares_equal(const T1& x, const T2& y) {
        return (x == y) && (y == x) && !(x != y) && !(y != x);
    }

    template <class T1, class T2>
    inline bool
    Base::compares_unequal(const T1& x, const T2& y) {
        return !(x == y) && !(y == x) && (x != y) && (y != x);
    }

} // namespace Test

// STATISTICS: test-core
