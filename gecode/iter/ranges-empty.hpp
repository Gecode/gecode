/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

namespace Gecode::Iter::Ranges {

    /**
     * \brief Range iterator for empty range
     *
     * \ingroup FuncIterRanges
     */

    class Empty {
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        Empty() = default;
        /// Initialize
        void init();
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

    forceinline void
    Empty::init() {}

    forceinline void
    Empty::operator++() {
        GECODE_NEVER;
    }

    forceinline bool
    Empty::operator()() const {
        return false;
    }

    forceinline int
    Empty::min() const {
        GECODE_NEVER;
        return 0;
    }
    forceinline int
    Empty::max() const {
        GECODE_NEVER;
        return 0;
    }
    forceinline unsigned int
    Empty::width() const {
        GECODE_NEVER;
        return 0;
    }

} // namespace Gecode::Iter::Ranges

// STATISTICS: iter-any
