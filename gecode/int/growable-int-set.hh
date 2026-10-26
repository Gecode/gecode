/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#ifndef GECODE_INT_GROWABLE_INT_SET_HH
#define GECODE_INT_GROWABLE_INT_SET_HH

#include <gecode/int.hh>
#include <limits>

namespace Gecode { namespace Int {

  class GrowableIntSetRanges;

  /// A Space-owned set of integer values with growable canonical range storage
  class GrowableIntSet {
    friend class GrowableIntSetRanges;
  private:
    struct Range {
      int min;
      int max;
      Range(void) {}
    };
    Range* _ranges;
    unsigned int _count;
    unsigned int _capacity;

    /// Largest capacity supported by both counts and allocation sizes
    static unsigned int max_capacity(void);
    /// Ensure capacity, copying only initialized ranges
    void reserve(Space& home, unsigned int required);
    /// First range whose maximum is at least value
    unsigned int first_max_ge(int value) const;
    /// Add one range, allocating only when a new range is inserted
    void add(Space& home, int min, int max);

    GrowableIntSet(const GrowableIntSet&) = delete;
    GrowableIntSet& operator =(const GrowableIntSet&) = delete;
  public:
    /// Initialize an empty set
    GrowableIntSet(void);
    /// Test membership
    bool in(int value) const;
    /** \brief Add the ranges from a copyable, independent canonical iterator
     *
     * Endpoints must be within Int::Limits. Mutation invalidates borrowed
     * range iterators. The input must not refer to this set's storage.
     */
    template<class I> void add_r(Space& home, I added);
    /// Copy exactly the live ranges into independent storage in home
    void update(Space& home, const GrowableIntSet& source);
    /// Return allocated storage to home
    void dispose(Space& home);
  };

  /// Borrowed forward iterator over a growable set's ranges
  class GrowableIntSetRanges {
  private:
    const GrowableIntSet::Range* current;
    const GrowableIntSet::Range* end;
  public:
    GrowableIntSetRanges(const GrowableIntSet& set);
    bool operator ()(void) const;
    void operator ++(void);
    int min(void) const;
    int max(void) const;
    unsigned int width(void) const;
  };

}}

#include <gecode/int/growable-int-set.hpp>

#endif

// STATISTICS: int-other
