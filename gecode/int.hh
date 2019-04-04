/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Stefano Gualandi <stefano.gualandi@gmail.com>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     David Rijsman <David.Rijsman@quintiq.com>
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Stefano Gualandi, 2013
 *     Mikael Lagerkvist, 2006
 *     David Rijsman, 2009
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Samuel Gagnon, 2018
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#ifndef __GECODE_INT_HH__
#define __GECODE_INT_HH__

#include <climits>
#include <cfloat>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <utility>
#include <initializer_list>

#include <gecode/kernel.hh>
#include <gecode/search.hh>
#include <gecode/iter.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_INT
#define GECODE_INT_EXPORT __declspec( dllexport )
#else
#define GECODE_INT_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_INT_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_INT_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_INT
#define GECODE_LIBRARY_NAME "Int"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode::Int
 * \brief Finite domain integers
 *
 * The Gecode::Int namespace contains all functionality required
 * to program propagators and branchers for finite domain integers.
 * In addition, all propagators and branchers for finite domain
 * integers provided by %Gecode are contained as nested namespaces.
 *
 */

#include <gecode/int/exception.hpp>

namespace Gecode { namespace Int {

  /**
   * \brief Numerical limits for integer variables
   *
   * The integer limits are chosen such changing the sign is always possible
   * without overflow.
   * \ingroup TaskModelIntVars
   */
  namespace Limits {
    /// Largest allowed integer value
    const int max = INT_MAX - 1;
    /// Smallest allowed integer value
    const int min = -max;
    /// Infinity for integers
    const int infinity = max + 1;
    /// Largest allowed long long integer value
    const long long int llmax =  LLONG_MAX - 1;
    /// Smallest allowed long long integer value
    const long long int llmin = -llmax;
    /// Infinity for long long integers
    const long long int llinfinity = llmax + 1;
    /// Return whether \a n is in range
    bool valid(int n);
    /// Return whether \a n is in range
    bool valid(long long int n);
    /// Check whether \a n is in range, otherwise throw out of limits with information \a l
    void check(int n, const char* l);
    /// Check whether \a n is in range, otherwise throw out of limits with information \a l
    void check(long long int n, const char* l);
    /// Check whether \a n is in range and strictly positive, otherwise throw out of limits with information \a l
    void positive(int n, const char* l);
    /// Check whether \a n is in range and strictly positive, otherwise throw out of limits with information \a l
    void positive(long long int n, const char* l);
    /// Check whether \a n is in range and nonnegative, otherwise throw out of limits with information \a l
    void nonnegative(int n, const char* l);
    /// Check whether \a n is in integer range and nonnegative, otherwise throw out of limits exception with information \a l
    void nonnegative(long long int n, const char* l);
    /// Check whether adding \a n and \a m would overflow
    bool overflow_add(int n, int m);
    /// Check whether adding \a n and \a m would overflow
    bool overflow_add(long long int n, long long int m);
    /// Check whether subtracting \a m from \a n would overflow
    bool overflow_sub(int n, int m);
    /// Check whether subtracting \a m from \a n would overflow
    bool overflow_sub(long long int n, long long int m);
    /// Check whether multiplying \a n and \a m would overflow
    bool overflow_mul(int n, int m);
    /// Check whether multiplying \a n and \a m would overflow
    bool overflow_mul(long long int n, long long int m);
  }

}}

#include <gecode/int/limits.hpp>

namespace Gecode {

  class IntSetRanges;

  template<class I> class IntSetInit;

  /**
   * \brief Integer sets
   *
   * Integer sets are the means to specify arbitrary sets
   * of integers to be used as domains for integer variables.
   * \ingroup TaskModelIntVars TaskModelSetVars
   */
  class IntSet : public SharedHandle {
    friend class IntSetRanges;
    template<class I> friend class IntSetInit;
  private:
    /// %Range (intervals) of integers
    class Range {
    public:
      int min, max;
    };
    class IntSetObject : public SharedHandle::Object {
    public:
      /// Size of set
      unsigned int size;
      /// Number of ranges
      int n;
      /// Array of ranges
      Range* r;
      /// Allocate object with \a m elements
      GECODE_INT_EXPORT static IntSetObject* allocate(int m);
      /// Check whether \a n is included in the set
      GECODE_INT_EXPORT bool in(int n) const;
      /// Perform equality test on ranges
      GECODE_INT_EXPORT bool equal(const IntSetObject& so) const; 
      /// Delete object
      GECODE_INT_EXPORT virtual ~IntSetObject(void);
    };
    /// Sort ranges according to increasing minimum
    class MinInc;
    /// Normalize the first \a n elements of \a r
    GECODE_INT_EXPORT void normalize(Range* r, int n);
    /// Initialize as range with minimum \a n and maximum \a m
    GECODE_INT_EXPORT void init(int n, int m);
    /// Initialize with \a n integers from array \a r
    GECODE_INT_EXPORT void init(const int r[], int n);
    /// Initialize with \a n ranges from array \a r
    GECODE_INT_EXPORT void init(const int r[][2], int n);
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize as empty set
    IntSet(void);
    /** \brief Initialize as range with minimum \a n and maximum \a m
     *
     * Note that the set is empty if \a n is larger than \a m
     */
    explicit IntSet(int n, int m);
    /// Initialize with \a n integers from array \a r
    explicit IntSet(const int r[],   int n);
    /** \brief Initialize with \a n ranges from array \a r
     *
     * For position \a i in the array \a r, the minimum is \a r[\a i][0]
     * and the maximum is \a r[\a i][1].
     */
    explicit IntSet(const int r[][2], int n);
    /// Initialize with range iterator \a i
    template<class I>
    explicit IntSet(I& i);
    /// Initialize with range iterator \a i
    template<class I>
    explicit IntSet(const I& i);
    /// Initialize with integers from list \a r
    GECODE_INT_EXPORT 
    explicit IntSet(std::initializer_list<int> r);
    /** \brief Initialize with ranges from vector \a r
     *
     * The minimum is the first element and the maximum is the
     * second element.
     */
    GECODE_INT_EXPORT
    explicit IntSet(std::initializer_list<std::pair<int,int>> r);
    //@}

    /// \name Range access
    //@{
    /// Return number of ranges of the specification
    int ranges(void) const;
    /// Return minimum of range at position \a i
    int min(int i) const;
    /// Return maximum of range at position \a i
    int max(int i) const;
    /// Return width of range at position \a i
    unsigned int width(int i) const;
    //@}

    /// \name Entire set access
    //@{
    /// Return whether \a n is included in the set
    bool in(int n) const;
    /// Return size (cardinality) of set
    unsigned int size(void) const;
    /// Return width of set (distance between maximum and minimum)
    unsigned int width(void) const;
    /// Return minimum of entire set
    int min(void) const;
    /// Return maximum of entire set
    int max(void) const;
    //@}

    /// \name Equality tests
    //@{
    /// Return whether \a s is equal
    bool operator ==(const IntSet& s) const;
    /// Return whether \a s is not equal
    bool operator !=(const IntSet& s) const;
    //@}

    /// \name Predefined value
    //@{
    /// Empty set
    GECODE_INT_EXPORT static const IntSet empty;
    //@}
  };

  /**
   * \brief Range iterator for integer sets
   *
   * \ingroup TaskModelIntVars TaskModelSetVars
   */
  class IntSetRanges {
  private:
    /// Current range
    const IntSet::Range* i;
    /// End range
    const IntSet::Range* e;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    IntSetRanges(void);
    /// Initialize with ranges for set \a s
    IntSetRanges(const IntSet& s);
    /// Initialize with ranges for set \a s
    void init(const IntSet& s);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a range or done
    bool operator ()(void) const;
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    int min(void) const;
    /// Return largest value of range
    int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    unsigned int width(void) const;
    //@}
  };

  /**
   * \brief Value iterator for integer sets
   *
   * \ingroup TaskModelIntVars TaskModelSetVars
   */
  class IntSetValues : public Iter::Ranges::ToValues<IntSetRanges> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    IntSetValues(void);
    /// Initialize with values for \a s
    IntSetValues(const IntSet& s);
    /// Initialize with values for \a s
    void init(const IntSet& s);
    //@}
  };

  /**
   * \brief Print integer set \a s
   * \relates Gecode::IntSet
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const IntSet& s);

}

#include <gecode/int/int-set-1.hpp>

#include <gecode/int/var-imp.hpp>

namespace Gecode {

  namespace Int {
    class IntView;
  }

  /**
   * \brief Integer variables
   *
   * \ingroup TaskModelIntVars
   */
  class IntVar : public VarImpVar<Int::IntVarImp> {
    friend class IntVarArray;
    friend class IntVarArgs;
  private:
    using VarImpVar<Int::IntVarImp>::x;
    /**
     * \brief Initialize variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. No exceptions are thrown.
     */
    void _init(Space& home, int min, int max);
    /**
     * \brief Initialize variable with arbitrary domain
     *
     * The variable is created with a domain described by \a d.
     * No exceptions are thrown.
     */
    void _init(Space& home, const IntSet& d);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    IntVar(void);
    /// Initialize from integer variable \a y
    IntVar(const IntVar& y);
    /// Initialize from integer view \a y
    IntVar(const Int::IntView& y);
    /**
     * \brief Initialize variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. The following exceptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for integers as defined
     *    in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT IntVar(Space& home, int min, int max);
    /**
     * \brief Initialize variable with arbitrary domain
     *
     * The variable is created with a domain described by \a d.
     * The following exceptions might be thrown:
     *  - If \a d is empty, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a d contains values that exceed the limits for integers
     *    as defined in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT IntVar(Space& home, const IntSet& d);
    //@}

    /// \name Value access
    //@{
    /// Return minimum of domain
    int min(void) const;
    /// Return maximum of domain
    int max(void) const;
    /// Return median of domain (greatest element not greater than the median)
    int med(void) const;
    /**
     * \brief Return assigned value
     *
     * Throws an exception of type Int::ValOfUnassignedVar if variable
     * is not yet assigned.
     *
     */
    int val(void) const;

    /// Return size (cardinality) of domain
    unsigned int size(void) const;
    /// Return width of domain (distance between maximum and minimum)
    unsigned int width(void) const;
    /// Return regret of domain minimum (distance to next larger value)
    unsigned int regret_min(void) const;
    /// Return regret of domain maximum (distance to next smaller value)
    unsigned int regret_max(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether domain is a range
    bool range(void) const;
    /// Test whether \a n is contained in domain
    bool in(int n) const;
    //@}
  };

  /**
   * \brief Print integer variable \a x
   * \relates Gecode::IntVar
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const IntVar& x);

  /**
   * \brief %Range iterator for integer variables
   * \ingroup TaskModelIntVars
   */
  class IntVarRanges : public Int::IntVarImpFwd {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    IntVarRanges(void);
    /// Initialize with ranges for integer variable \a x
    IntVarRanges(const IntVar& x);
    /// Initialize with ranges for integer variable \a x
    void init(const IntVar& x);
    //@}
  };

  /**
   * \brief Value iterator for integer variables
   * \ingroup TaskModelIntVars
   */
  class IntVarValues : public Iter::Ranges::ToValues<IntVarRanges> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    IntVarValues(void);
    /// Initialize with values for \a x
    IntVarValues(const IntVar& x);
    /// Initialize with values \a x
    void init(const IntVar& x);
    //@}
  };

  namespace Int {
    class BoolView;
  }

  /**
   * \brief Boolean integer variables
   *
   * \ingroup TaskModelIntVars
   */
  class BoolVar : public VarImpVar<Int::BoolVarImp> {
    friend class BoolVarArray;
    friend class BoolVarArgs;
  private:
    using VarImpVar<Int::BoolVarImp>::x;
    /**
     * \brief Initialize Boolean variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. No exceptions are thrown.
     */
    void _init(Space& home, int min, int max);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    BoolVar(void);
    /// Initialize from Boolean variable \a y
    BoolVar(const BoolVar& y);
    /// Initialize from Boolean view \a y
    BoolVar(const Int::BoolView& y);
    /**
     * \brief Initialize Boolean variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. The following exceptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min is less than 0 or \a max is greater than 1,
     *    an exception of type
     *    Gecode::Int::NotZeroOne is thrown.
     */
    GECODE_INT_EXPORT BoolVar(Space& home, int min, int max);
    //@}

    /// \name Value access
    //@{
    /// Return minimum of domain
    int min(void) const;
    /// Return maximum of domain
    int max(void) const;
    /// Return median of domain (greatest element not greater than the median)
    int med(void) const;
    /**
     * \brief Return assigned value
     *
     * Throws an exception of type Int::ValOfUnassignedVar if variable
     * is not yet assigned.
     *
     */
    int val(void) const;

    /// Return size (cardinality) of domain
    unsigned int size(void) const;
    /// Return width of domain (distance between maximum and minimum)
    unsigned int width(void) const;
    /// Return regret of domain minimum (distance to next larger value)
    unsigned int regret_min(void) const;
    /// Return regret of domain maximum (distance to next smaller value)
    unsigned int regret_max(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether domain is a range
    bool range(void) const;
    /// Test whether \a n is contained in domain
    bool in(int n) const;
    //@}

    /// \name Boolean domain tests
    //@{
    /// Test whether domain is zero
    bool zero(void) const;
    /// Test whether domain is one
    bool one(void) const;
    /// Test whether domain is neither zero nor one
    bool none(void) const;
    //@}
  };

  /**
   * \brief Print Boolean variable \a x
   * \relates Gecode::BoolVar
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const BoolVar& x);

}


#include <gecode/int/view.hpp>
#include <gecode/int/propagator.hpp>

namespace Gecode {

  /**
   * \defgroup TaskModelIntArgs Argument arrays
   *
   * Argument arrays are just good enough for passing arguments
   * with automatic memory management.
   * \ingroup TaskModelInt
   */

  //@{
  /// Passing set arguments
  typedef ArgArray<IntSet> IntSetArgs;

}

#include <gecode/int/array-traits.hpp>

namespace Gecode {

  /// Passing integer arguments
  class IntArgs : public ArgArray<int> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    IntArgs(void);
    /// Allocate array with \a n elements
    explicit IntArgs(int n);
    /// Allocate array and copy elements from \a x
    IntArgs(const SharedArray<int>& x);
    /// Allocate array and copy elements from \a x
    IntArgs(const std::vector<int>& x);
    /// Allocate array and copy elements from \a x
    IntArgs(std::initializer_list<int> x);
    /// Allocate array and copy elements from \a first to \a last
    template<class InputIterator>
    IntArgs(InputIterator first, InputIterator last);
    /// Allocate array with \a n elements and initialize with elements from array \a e
    IntArgs(int n, const int* e);
    /// Initialize from primitive argument array \a a (copy elements)
    IntArgs(const ArgArray<int>& a);

    /// Allocate array with \a n elements such that for all \f$0\leq i<n: x_i=\text{start}+i\cdot\text{inc}\f$
    static IntArgs create(int n, int start, int inc=1);
    //@}
  };

  /// \brief Passing integer variables
  class IntVarArgs : public VarArgArray<IntVar> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    IntVarArgs(void);
    /// Allocate array with \a n elements
    explicit IntVarArgs(int n);
    /// Initialize from variable argument array \a a (copy elements)
    IntVarArgs(const IntVarArgs& a);
    /// Initialize from variable array \a a (copy elements)
    IntVarArgs(const VarArray<IntVar>& a);
    /// Initialize from \a a
    IntVarArgs(const std::vector<IntVar>& a);
    /// Initialize from \a a
    IntVarArgs(std::initializer_list<IntVar> a);
    /// Initialize from InputIterator \a first and \a last
    template<class InputIterator>
    IntVarArgs(InputIterator first, InputIterator last);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for integers as defined
     *    in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT
    IntVarArgs(Space& home, int n, int min, int max);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain described by \a s.
     * The following execptions might be thrown:
     *  - If \a s is empty, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a s contains values that exceed the limits for integers
     *    as defined in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT
    IntVarArgs(Space& home, int n, const IntSet& s);
    //@}
  };

  /** \brief Passing Boolean variables
   *
   * We could have used a simple typedef instead, but doxygen cannot
   * resolve some overloading then, leading to unusable documentation for
   * important parts of the library. As long as there is no fix for this,
   * we will keep this workaround.
   *
   */
  class BoolVarArgs : public VarArgArray<BoolVar> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    BoolVarArgs(void);
    /// Allocate array with \a n elements
    explicit BoolVarArgs(int n);
    /// Initialize from variable argument array \a a (copy elements)
    BoolVarArgs(const BoolVarArgs& a);
    /// Initialize from variable array \a a (copy elements)
    BoolVarArgs(const VarArray<BoolVar>& a);
    /// Initialize from \a a
    BoolVarArgs(const std::vector<BoolVar>& a);
    /// Initialize from \a a
    BoolVarArgs(std::initializer_list<BoolVar> a);
    /// Initialize from InputIterator \a first and \a last
    template<class InputIterator>
    BoolVarArgs(InputIterator first, InputIterator last);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min is less than 0 or \a max is greater than 1,
     *    an exception of type
     *    Gecode::Int::NotZeroOne is thrown.
     */
    GECODE_INT_EXPORT
    BoolVarArgs(Space& home, int n, int min, int max);
    //@}
  };
  //@}

  /**
   * \defgroup TaskModelIntVarArrays Variable arrays
   *
   * Variable arrays can store variables. They are typically used
   * for storing the variables being part of a solution (script). However,
   * they can also be used for temporary purposes (even though
   * memory is not reclaimed until the space it is created for
   * is deleted).
   * \ingroup TaskModelInt
   */

  /**
   * \brief Integer variable array
   * \ingroup TaskModelIntVarArrays
   */
  class IntVarArray : public VarArray<IntVar> {
  public:
    /// \name Creation and initialization
    //@{
    /// Default constructor (array of size 0)
    IntVarArray(void);
    /// Allocate array for \a n integer variables (variables are uninitialized)
    IntVarArray(Space& home, int n);
    /// Initialize from integer variable array \a a (share elements)
    IntVarArray(const IntVarArray& a);
    /// Initialize from integer variable argument array \a a (copy elements)
    IntVarArray(Space& home, const IntVarArgs& a);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for integers as defined
     *    in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT
    IntVarArray(Space& home, int n, int min, int max);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain described by \a s.
     * The following execptions might be thrown:
     *  - If \a s is empty, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a s contains values that exceed the limits for integers
     *    as defined in Gecode::Int::Limits, an exception of type
     *    Gecode::Int::OutOfLimits is thrown.
     */
    GECODE_INT_EXPORT
    IntVarArray(Space& home, int n, const IntSet& s);
    //@}
  };

  /**
   * \brief Boolean variable array
   * \ingroup TaskModelIntVarArrays
   */
  class BoolVarArray : public VarArray<BoolVar> {
  public:
    /// \name Creation and initialization
    //@{
    /// Default constructor (array of size 0)
    BoolVarArray(void);
    /// Allocate array for \a n Boolean variables (variables are uninitialized)
    BoolVarArray(Space& home, int n);
    /// Initialize from Boolean variable array \a a (share elements)
    BoolVarArray(const BoolVarArray& a);
    /// Initialize from Boolean variable argument array \a a (copy elements)
    BoolVarArray(Space& home, const BoolVarArgs& a);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min is less than 0 or \a max is greater than 1,
     *    an exception of type
     *    Gecode::Int::NotZeroOne is thrown.
     */
    GECODE_INT_EXPORT
    BoolVarArray(Space& home, int n, int min, int max);
    //@}
  };

}

#include <gecode/int/int-set-2.hpp>

#include <gecode/int/array.hpp>

namespace Gecode {

  /**
   * \brief Mode for reification
   * \ingroup TaskModelInt
   */
  enum ReifyMode {
    /**
     * \brief Equivalence for reification (default)
     *
     * For a constraint \f$c\f$ and a Boolean control variable \f$b\f$
     * defines that \f$b=1\Leftrightarrow c\f$ is propagated.
     */
    RM_EQV,
    /**
     * \brief Implication for reification
     *
     * For a constraint \f$c\f$ and a Boolean control variable \f$b\f$
     * defines that \f$b=1\Leftarrow c\f$ is propagated.
     */
    RM_IMP,
    /**
     * \brief Inverse implication for reification
     *
     * For a constraint \f$c\f$ and a Boolean control variable \f$b\f$
     * defines that \f$b=1\Rightarrow c\f$ is propagated.
     */
    RM_PMI
  };

  /**
   * \brief Reification specification
   * \ingroup TaskModelInt
   */
  class Reify {
  protected:
    /// The Boolean control variable
    BoolVar x;
    /// The reification mode
    ReifyMode rm;
  public:
    /// Default constructor without proper initialization
    Reify(void);
    /// Construct reification specification
    Reify(BoolVar x, ReifyMode rm=RM_EQV);
    /// Return Boolean control variable
    BoolVar var(void) const;
    /// Return reification mode
    ReifyMode mode(void) const;
    /// Set Boolean control variable
    void var(BoolVar x);
    /// Set reification mode
    void mode(ReifyMode rm);
  };

  /**
   * \brief Use equivalence for reification
   * \ingroup TaskModelInt
   */
  Reify eqv(BoolVar x);

  /**
   * \brief Use implication for reification
   * \ingroup TaskModelInt
   */
  Reify imp(BoolVar x);

  /**
   * \brief Use reverse implication for reification
   * \ingroup TaskModelInt
   */
  Reify pmi(BoolVar x);

}

#include <gecode/int/reify.hpp>

namespace Gecode {

  /**
   * \brief Relation types for integers
   * \ingroup TaskModelInt
   */
  enum IntRelType {
    IRT_EQ, ///< Equality (\f$=\f$)
    IRT_NQ, ///< Disequality (\f$\neq\f$)
    IRT_LQ, ///< Less or equal (\f$\leq\f$)
    IRT_LE, ///< Less (\f$<\f$)
    IRT_GQ, ///< Greater or equal (\f$\geq\f$)
    IRT_GR  ///< Greater (\f$>\f$)
  };

  /// Return swapped relation type of \a irt
  IntRelType swap(IntRelType irt);

  /// Return negated relation type of \a irt
  IntRelType neg(IntRelType irt);

}

#include <gecode/int/irt.hpp>

namespace Gecode {

  /**
   * \brief Operation types for Booleans
   * \ingroup TaskModelInt
   */
  enum BoolOpType {
    BOT_AND, ///< Conjunction
    BOT_OR,  ///< Disjunction
    BOT_IMP, ///< Implication
    BOT_EQV, ///< Equivalence
    BOT_XOR  ///< Exclusive or
  };

  /**
   * \brief Propagation levels for integer propagators
   *
   * The descriptions are meant to be approximate. It is not
   * required that a propagator achieves full domain consistency or
   * full bounds consistency. It is more like: which level
   * of consistency comes closest to the level of propagation
   * the propagator implements.
   *
   * If in the description of a constraint below no propagation level
   * is mentioned, the propagation level for the constraint is domain
   * propagation and the implementation in fact enforces domain
   * consistency.
   *
   * \ingroup TaskModelInt
   */
  enum IntPropLevel {
    /// Simple propagation levels
    IPL_DEF = 0, ///< Default level of propagation
    IPL_VAL = 1, ///< Value propagation
    IPL_BND = 2, ///< Bounds propagation
    IPL_DOM = 3, ///< Domain propagation
    /// Options: basic versus advanced propagation
    IPL_BASIC = 4,    ///< Use basic propagation algorithm
    IPL_ADVANCED = 8, ///< Use advanced propagation algorithm
    IPL_BASIC_ADVANCED = IPL_BASIC | IPL_ADVANCED, ///< Use both
    _IPL_BITS = 4 ///< Number of bits required (internal)
  };

  /// Extract value, bounds, or domain propagation from propagation level
  IntPropLevel vbd(IntPropLevel ipl);

  /// Extract basic or advanced from propagation level
  IntPropLevel ba(IntPropLevel ipl);

}

#include <gecode/int/ipl.hpp>

namespace Gecode {

  /**
   * \brief Type of task for scheduling constraints
   *
   * \ingroup TaskModelInt
   */
  enum TaskType {
    TT_FIXP, //< Task with fixed processing time
    TT_FIXS, //< Task with fixed start time
    TT_FIXE  //< Task with fixed end time
  };

  /**
   * \brief Argument arrays for passing task type arguments
   *
   * \ingroup TaskModelInt
   */
  typedef ArgArray<TaskType> TaskTypeArgs;

  /// Traits of %TaskTypeArgs
  template<>
  class ArrayTraits<ArgArray<TaskType> > {
  public:
    typedef TaskTypeArgs StorageType;
    typedef TaskType     ValueType;
    typedef TaskTypeArgs ArgsType;
  };


  /**
   * \defgroup TaskModelIntDomain Domain constraints
   * \ingroup TaskModelInt
   *
   */

  //@{
  /// Propagates \f$x=n\f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, int n,
      IntPropLevel ipl=IPL_DEF);
  /// Propagates \f$ x_i=n\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Home home, const IntVarArgs& x, int n,
      IntPropLevel ipl=IPL_DEF);

  /// Propagates \f$ l\leq x\leq m\f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, int l, int m,
      IntPropLevel ipl=IPL_DEF);
  /// Propagates \f$ l\leq x_i\leq m\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Home home, const IntVarArgs& x, int l, int m,
      IntPropLevel ipl=IPL_DEF);

  /// Propagates \f$ x\in s \f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, const IntSet& s,
      IntPropLevel ipl=IPL_DEF);
  /// Propagates \f$ x_i\in s\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Home home, const IntVarArgs& x, const IntSet& s,
      IntPropLevel ipl=IPL_DEF);

  /// Post domain consistent propagator for \f$ (x=n) \equiv r\f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, int n, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$ (l\leq x \leq m) \equiv r\f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, int l, int m, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$ (x \in s) \equiv r\f$
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, const IntSet& s, Reify r,
      IntPropLevel ipl=IPL_DEF);

  /// Constrain domain of \a x according to domain of \a d
  GECODE_INT_EXPORT void
  dom(Home home, IntVar x, IntVar d,
      IntPropLevel ipl=IPL_DEF);
  /// Constrain domain of \a x according to domain of \a d
  GECODE_INT_EXPORT void
  dom(Home home, BoolVar x, BoolVar d,
      IntPropLevel ipl=IPL_DEF);
  /// Constrain domain of \f$ x_i \f$ according to domain of \f$ d_i \f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Home home, const IntVarArgs& x, const IntVarArgs& d,
      IntPropLevel ipl=IPL_DEF);
  /// Constrain domain of \f$ x_i \f$ according to domain of \f$ d_i \f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Home home, const BoolVarArgs& x, const BoolVarArgs& d,
      IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntRelInt Simple relation constraints over integer variables
   * \ingroup TaskModelInt
   */
  /** \brief Post propagator for \f$ x_0 \sim_{irt} x_1\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, IntVar x0, IntRelType irt, IntVar x1,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ x_i \sim_{irt} y \f$ for all \f$0\leq i<|x|\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntVarArgs& x, IntRelType irt, IntVar y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Propagates \f$ x \sim_{irt} c\f$
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, IntVar x, IntRelType irt, int c,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Propagates \f$ x_i \sim_{irt} c \f$ for all \f$0\leq i<|x|\f$
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntVarArgs& x, IntRelType irt, int c,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ (x_0 \sim_{irt} x_1)\equiv r\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, IntVar x0, IntRelType irt, IntVar x1, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$(x \sim_{irt} c)\equiv r\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, IntVar x, IntRelType irt, int c, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for relation among elements in \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ,
   *    then the elements of \a x are ordered with respect to \a r.
   *    Supports domain consistency (\a ipl = IPL_DOM, default).
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal.
   *    Supports both bounds (\a ipl = IPL_BND) and
   *    domain consistency (\a ipl = IPL_DOM, default).
   *  - if \a r = IRT_NQ, then not all elements of \a x must be equal.
   *    Supports domain consistency (\a ipl = IPL_DOM, default).
   *
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntVarArgs& x, IntRelType irt,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntVarArgs& x, IntRelType irt, const IntVarArgs& y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Supports domain consistency.
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntVarArgs& x, IntRelType irt, const IntArgs& y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Supports domain consistency.
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   * \ingroup TaskModelIntRelInt
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntArgs& x, IntRelType irt, const IntVarArgs& y,
      IntPropLevel ipl=IPL_DEF);

  /**
   * \defgroup TaskModelIntRelBool Simple relation constraints over Boolean variables
   * \ingroup TaskModelInt
   */
  /** \brief Post domain consistent propagator for \f$ x_0 \sim_{irt} x_1\f$
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x0, IntRelType irt, BoolVar x1,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$(x_0 \sim_{irt} x_1)\equiv r\f$
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x0, IntRelType irt, BoolVar x1, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$ x_i \sim_{irt} y \f$ for all \f$0\leq i<|x|\f$
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const BoolVarArgs& x, IntRelType irt, BoolVar y,
      IntPropLevel ipl=IPL_DEF);
  /**
   * \brief Propagates \f$ x \sim_{irt} n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x, IntRelType irt, int n,
      IntPropLevel ipl=IPL_DEF);
  /**
   * \brief Post domain consistent propagator for \f$(x \sim_{irt} n)\equiv r\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x, IntRelType irt, int n, Reify r,
      IntPropLevel ipl=IPL_DEF);
  /**
   * \brief Propagates \f$ x_i \sim_{irt} n \f$ for all \f$0\leq i<|x|\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const BoolVarArgs& x, IntRelType irt, int n,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const BoolVarArgs& x, IntRelType irt, const BoolVarArgs& y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const BoolVarArgs& x, IntRelType irt, const IntArgs& y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Note that the constraint is also defined if \a x and \a y are of
   * different size. That means that if \a x and \a y are of different
   * size, then if \a r = IRT_EQ the constraint is false and if
   * \a r = IRT_NQ the constraint is subsumed.
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const IntArgs& x, IntRelType irt, const BoolVarArgs& y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for relation between elements in \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ,
   *    then the elements of \a x are ordered with respect to \a r.
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal.
   *  - if \a r = IRT_NQ, then not all elements of \a x must be equal.
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, const BoolVarArgs& x, IntRelType irt,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean operation on \a x0 and \a x1
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} x_1 = x_2\f$
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean operation on \a x0 and \a x1
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} x_1 = n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolVar x0, BoolOpType o, BoolVar x1, int n,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean operation on \a x
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1}= y\f$
   *
   * Throws an exception of type Int::TooFewArguments, if \f$|x|<2\f$
   * and \a o is BOT_IMP, BOT_EQV, or BOT_XOR.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolOpType o, const BoolVarArgs& x, BoolVar y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean operation on \a x
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1}= n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   *
   * Throws an exception of type Int::TooFewArguments, if \f$|x|<2\f$
   * and \a o is BOT_IMP, BOT_EQV, or BOT_XOR.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  rel(Home home, BoolOpType o, const BoolVarArgs& x, int n,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= z\f$
   *
   * Throws an exception of type Int::IllegalOperation, if \a o is different
   * from BOT_AND or BOT_OR.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  clause(Home home, BoolOpType o, const BoolVarArgs& x, const BoolVarArgs& y,
         BoolVar z, IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   *
   * Throws an exception of type Int::IllegalOperation, if \a o is different
   * from BOT_AND or BOT_OR.
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  clause(Home home, BoolOpType o, const BoolVarArgs& x, const BoolVarArgs& y,
         int n, IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for if-then-else constraint
   *
   * Posts propagator for \f$ z = b ? x : y \f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  ite(Home home, BoolVar b, IntVar x, IntVar y, IntVar z,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for if-then-else constraint
   *
   * Posts propagator for \f$ z = b ? x : y \f$
   *
   * \ingroup TaskModelIntRelBool
   */
  GECODE_INT_EXPORT void
  ite(Home home, BoolVar b, BoolVar x, BoolVar y, BoolVar z,
      IntPropLevel ipl=IPL_DEF);


  /**
   * \defgroup TaskModelIntPrecede Value precedence constraints over integer variables
   * \ingroup TaskModelInt
   */
  /** \brief Post propagator that \a s precedes \a t in \a x
   *
   * This constraint enforces that \f$x_0\neq t\f$ and
   * \f$x_j=t \to \bigvee_{0\leq i<j} x_i=s\f$ for \f$0\leq j<|x|\f$.
   * The propagator is domain consistent.
   * \ingroup TaskModelIntPrecede
   */
  GECODE_INT_EXPORT void
  precede(Home home, const IntVarArgs& x, int s, int t,
          IntPropLevel=IPL_DEF);
  /** \brief Post propagator that successive values in \a c precede each other in \a x
   *
   * This constraint enforces that \f$x_0\neq c_k\f$ for \f$0<k<|c|\f$ and
   * \f$x_j=c_{k} \to \bigvee_{0\leq i<j} x_i=c_{k-1}\f$ for \f$0\leq j<|x|\f$
   * and \f$0< k<|c|\f$.
   * \ingroup TaskModelIntPrecede
   */
  GECODE_INT_EXPORT void
  precede(Home home, const IntVarArgs& x, const IntArgs& c,
          IntPropLevel=IPL_DEF);


  /**
   * \defgroup TaskModelIntMember Membership constraints
   * \ingroup TaskModelInt
   */
  //@{
  /// Post domain consistent propagator for \f$y\in \{x_0,\ldots,x_{|x|-1}\}\f$
  GECODE_INT_EXPORT void
  member(Home home, const IntVarArgs& x, IntVar y,
         IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$y\in \{x_0,\ldots,x_{|x|-1}\}\f$
  GECODE_INT_EXPORT void
  member(Home home, const BoolVarArgs& x, BoolVar y,
         IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$\left(y\in \{x_0,\ldots,x_{|x|-1}\}\right)\equiv r\f$
  GECODE_INT_EXPORT void
  member(Home home, const IntVarArgs& x, IntVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$\left(y\in \{x_0,\ldots,x_{|x|-1}\}\right)\equiv r\f$
  GECODE_INT_EXPORT void
  member(Home home, const BoolVarArgs& x, BoolVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntElement Element constraints
   * \ingroup TaskModelInt
   */

  //@{
  /// Arrays of integers that can be shared among several element constraints
  typedef SharedArray<int> IntSharedArray;
  /** \brief Post domain consistent propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Home home, IntSharedArray n, IntVar x0, IntVar x1,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Home home, IntSharedArray n, IntVar x0, BoolVar x1,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Home home, IntSharedArray n, IntVar x0, int x1,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Home home, const IntVarArgs& x, IntVar y0, IntVar y1,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Home home, const IntVarArgs& x, IntVar y0, int y1,
          IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Home home, const BoolVarArgs& x, IntVar y0, BoolVar y1,
          IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Home home, const BoolVarArgs& x, IntVar y0, int y1,
          IntPropLevel ipl=IPL_DEF);

  /** \brief Post domain consistent propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * If \a a is regarded as a two-dimensional array in row-major
   * order of width \a w and height \a h, then \a z is constrained
   * to be the element in column \a x and row \a y.
   *
   * Throws an exception of type Int::OutOfLimits, if
   * the integers in \a n exceed the limits in Int::Limits.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_INT_EXPORT void
  element(Home home, IntSharedArray a,
          IntVar x, int w, IntVar y, int h, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * If \a a is regarded as a two-dimensional array in row-major
   * order of width \a w and height \a h, then \a z is constrained
   * to be the element in column \a x and row \a y.
   *
   * Throws an exception of type Int::OutOfLimits, if
   * the integers in \a n exceed the limits in Int::Limits.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_INT_EXPORT void
  element(Home home, IntSharedArray a,
          IntVar x, int w, IntVar y, int h, BoolVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * If \a a is regarded as a two-dimensional array in row-major
   * order of width \a w and height \a h, then \a z is constrained
   * to be the element in column \a x and row \a y.
   *
   * Supports both bounds (\a ipl = IPL_BND) and
   * domain consistency (\a ipl = IPL_DOM, default).
   *
   * Throws an exception of type Int::OutOfLimits, if
   * the integers in \a n exceed the limits in Int::Limits.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_INT_EXPORT void
  element(Home home, const IntVarArgs& a,
          IntVar x, int w, IntVar y, int h, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * If \a a is regarded as a two-dimensional array in row-major
   * order of width \a w and height \a h, then \a z is constrained
   * to be the element in column \a x and row \a y.
   *
   * Throws an exception of type Int::OutOfLimits, if
   * the integers in \a n exceed the limits in Int::Limits.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_INT_EXPORT void
  element(Home home, const BoolVarArgs& a,
          IntVar x, int w, IntVar y, int h, BoolVar z,
          IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntDistinct Distinct constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
   * and domain consistency (\a ipl = IPL_DOM).
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Home home, const IntVarArgs& x,
           IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ x_i+n_i\neq x_j+n_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * \li Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
   *     and domain consistency (\a ipl = IPL_DOM).
   * \li Throws an exception of type Int::OutOfLimits, if
   *     the integers in \a n exceed the limits in Int::Limits
   *     or if the sum of \a n and \a x exceed the limits.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a n are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x contains
   *     the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Home home, const IntArgs& n, const IntVarArgs& x,
           IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ b_i=1\wedge b_j=1\to x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * \li Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
   *     and domain consistency (\a ipl = IPL_DOM).
   * \li Throws an exception of type Int::OutOfLimits, if
   *     the variable domains in \a x are too large (it must hold that
   *     one of the values \f$(\max_{i=0,\ldots,|x|-1} \max(x_i))+|x|\f$
   *     and \f$(\min_{i=0,\ldots,|x|-1} \min(x_i))-|x|\f$
   *     does not exceed the limits in Int::Limits.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a b and \a x are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x
   *     contains the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Home home, const BoolVarArgs& b, const IntVarArgs& x,
           IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ x_i=c\vee x_j=c\vee x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * \li Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
   *     and domain consistency (\a ipl = IPL_DOM).
   * \li Throws an exception of type Int::OutOfLimits, if
   *     the variable domains in \a x are too large (it must hold that
   *     one of the values \f$(\max_{i=0,\ldots,|x|-1} \max(x_i))+|x|\f$
   *     and \f$(\min_{i=0,\ldots,|x|-1} \min(x_i))-|x|\f$
   *     does not exceed the limits in Int::Limits.
   * \li Throws an exception of type Int::ArgumentSame, if \a x
   *     contains the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Home home, const IntVarArgs& x, int c,
           IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntChannel Channel constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i = j\leftrightarrow y_j=i\f$ for all \f$0\leq i<|x|\f$
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM) and value
   *     propagation (all other values for \a ipl, default).
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a y are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x or
   *     \a y contain the same unassigned variable multiply. Note that a
   *     variable can occur in both \a x and \a y, but not more than
   *     once in either \a x or \a y.
   */
  GECODE_INT_EXPORT void
  channel(Home home, const IntVarArgs& x, const IntVarArgs& y,
          IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$ x_i - \mathit{xoff} = j\leftrightarrow y_j - \mathit{yoff} = i\f$ for all \f$0\leq i<|x|\f$
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM) and value
   *     propagation (all other values for \a ipl, default).
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a y are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x or
   *     \a y contain the same unassigned variable multiply. Note that a
   *     variable can occur in both \a x and \a y, but not more than
   *     once in either \a x or \a y.
   * \li Throws an exception of type Int::OutOfLimits, if \a xoff or
   *     \a yoff are negative.
   */
  GECODE_INT_EXPORT void
  channel(Home home, const IntVarArgs& x, int xoff,
          const IntVarArgs& y, int yoff,
          IntPropLevel ipl=IPL_DEF);

  /// Post domain consistent propagator for channeling a Boolean and an integer variable \f$ x_0 = x_1\f$
  GECODE_INT_EXPORT void
  channel(Home home, BoolVar x0, IntVar x1,
          IntPropLevel ipl=IPL_DEF);
  /// Post domain consistent propagator for channeling an integer and a Boolean variable \f$ x_0 = x_1\f$
  void
  channel(Home home, IntVar x0, BoolVar x1,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post domain consistent propagator for channeling Boolean and integer variables \f$ x_i = 1\leftrightarrow y=i+o\f$
   *
   * Throws an exception of type Int::ArgumentSame, if \a x
   * contains the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  channel(Home home, const BoolVarArgs& x, IntVar y, int o=0,
          IntPropLevel ipl=IPL_DEF);
  //@}

}

#include <gecode/int/channel.hpp>

namespace Gecode {

  /**
   * \defgroup TaskModelIntSorted Sorted constraints
   *
   * All sorted constraints support bounds consistency only.
   *
   * \ingroup TaskModelInt
   */
  //@{
  /**
   * \brief Post propagator that \a y is \a x sorted in increasing order
   *
   * Might throw the following exceptions:
   *  - Int::ArgumentSizeMismatch, if \a x and \a y differ in size.
   *  - Int::ArgumentSame, if \a x or \a y contain
   *             shared unassigned variables.
   */
  GECODE_INT_EXPORT void
  sorted(Home home, const IntVarArgs& x, const IntVarArgs& y,
         IntPropLevel ipl=IPL_DEF);

  /**
   * \brief Post propagator that \a y is \a x sorted in increasing order
   *
   * The values in \a z describe the sorting permutation, that is
   * \f$\forall i\in\{0,\dots,|x|-1\}: x_i=y_{z_i} \f$.
   *
   * Might throw the following exceptions:
   *  - Int::ArgumentSizeMismatch, if \a x and \a y differ in size.
   *  - Int::ArgumentSame, if \a x or \a y contain
   *             shared unassigned variables.
   */
  GECODE_INT_EXPORT void
  sorted(Home home, const IntVarArgs& x, const IntVarArgs& y,
         const IntVarArgs& z,
         IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntCount Counting constraints
   * \ingroup TaskModelInt
   *
   *  \note
   *    Domain consistency on the extended cardinality variables of
   *    the Global Cardinality Propagator is only obtained if they are bounds
   *    consistent, otherwise the problem of enforcing domain consistency
   *    on the cardinality variables is NP-complete as proved by
   *    Qumiper et. al. in
   *    ''Improved Algorithms for the Global Cardinality Constraint''.
   */

  //@{
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\sim_{irt} m\f$
   *
   * Performs domain propagation but is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, int n, IntRelType irt, int m,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i\in y\}\sim_{irt} m\f$
   *
   * Performs domain propagation but is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntSet& y, IntRelType irt, int m,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_{irt} m\f$
   *
   * Performs domain propagation (\a ipl = IPL_DOM, default)
   * and slightly less domain propagation (all other values for \a ipl),
   * where \a y is not pruned. Note that in both cases propagation
   * is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, IntVar y, IntRelType irt, int m,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\sim_{irt} m\f$
   *
   * Performs domain propagation but is not domain consistent.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntArgs& y, IntRelType irt, int m,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\sim_{irt} z\f$
   *
   * Performs domain propagation but is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, int n, IntRelType irt, IntVar z,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i\in y\}\sim_{irt} z\f$
   *
   * Performs domain propagation but is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntSet& y, IntRelType irt, IntVar z,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_{irt} z\f$
   *
   * Performs domain propagation (\a ipl = IPL_DOM, default)
   * and slightly less domain propagation (all other values for \a ipl),
   * where \a y is not pruned. Note that in both cases propagation
   * is not domain consistent.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, IntVar y, IntRelType irt, IntVar z,
        IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\sim_{irt} z\f$
   *
   * Performs domain propagation but is not domain consistent.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntArgs& y, IntRelType irt, IntVar z,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=j\}=c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \{0,\ldots,|c|-1\}\f$
    * (no other value occurs).
    *
    * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
    * and domain consistency (\a ipl = IPL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntVarArgs& c,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=j\}\in c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \{0,\ldots,|c|-1\}\f$
    * (no other value occurs).
    *
    * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
    * and domain consistency (\a ipl = IPL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x, const IntSetArgs& c,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}=c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
    * and domain consistency (\a ipl = IPL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a c and \a v are of different size.
    */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x,
        const IntVarArgs& c, const IntArgs& v,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}\in c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
    * and domain consistency (\a ipl = IPL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a c and \a v are of different size.
    */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x,
        const IntSetArgs& c, const IntArgs& v,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}\in c\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a ipl = IPL_VAL, default), bounds (\a ipl = IPL_BND),
    * and domain consistency (\a ipl = IPL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a c and \a v are of different size.
    */
  GECODE_INT_EXPORT void
  count(Home home, const IntVarArgs& x,
        const IntSet& c, const IntArgs& v,
        IntPropLevel ipl=IPL_DEF);

  //@}

  /**
   * \defgroup TaskModelIntNValues Number of values constraints
   * \ingroup TaskModelInt
   *
   * The number of values constraints perform propagation
   * following: C. Bessiere, E. Hebrard, B. Hnich, Z. Kiziltan,
   * and T. Walsh, Filtering Algorithms for the NValue
   * Constraint, Constraints, 11(4), 271-293, 2006.
   */

  //@{
  /** \brief Post propagator for \f$\#\{x_0,\ldots,x_{|x|-1}\}\sim_{irt} y\f$
   *
   */
  GECODE_INT_EXPORT void
  nvalues(Home home, const IntVarArgs& x, IntRelType irt, int y,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{x_0,\ldots,x_{|x|-1}\}\sim_{irt} y\f$
   *
   */
  GECODE_INT_EXPORT void
  nvalues(Home home, const IntVarArgs& x, IntRelType irt, IntVar y,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{x_0,\ldots,x_{|x|-1}\}\sim_{irt} y\f$
   *
   */
  GECODE_INT_EXPORT void
  nvalues(Home home, const BoolVarArgs& x, IntRelType irt, int y,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\#\{x_0,\ldots,x_{|x|-1}\}\sim_{irt} y\f$
   *
   */
  GECODE_INT_EXPORT void
  nvalues(Home home, const BoolVarArgs& x, IntRelType irt, IntVar y,
          IntPropLevel ipl=IPL_DEF);
  //@}

  /**
   * \defgroup TaskModelIntSequence Sequence constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$\operatorname{sequence}(x,s,q,l,u)\f$
   *
   * Posts a domain consistent propagator for the constraint
   * \f$\bigwedge_{i=0}^{|x|-q}
   *      \operatorname{among}(\langle x_i,\ldots,x_{i+q-1}\rangle,s,l,u)\f$
   * where the among constraint is defined as
   * \f$l\leq\#\{j\in\{i,\ldots,i+q-1\}\;|\;x_j\in s\} \leq u\f$.
   *
   * Throws the following exceptions:
   *  - Of type Int::TooFewArguments, if \f$|x|=0\f$.
   *  - Of type Int::ArgumentSame, if \a x contains
   *    the same unassigned variable multiply.
   *  - Of type Int::OutOfRange, if \f$q < 1 \vee q > |x|\f$.
   */
  GECODE_INT_EXPORT void
  sequence(Home home, const IntVarArgs& x, const IntSet& s,
           int q, int l, int u, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$\operatorname{sequence}(x,s,q,l,u)\f$
   *
   * Posts a domain consistent propagator for the constraint
   * \f$\bigwedge_{i=0}^{|x|-q}
   *      \operatorname{among}(\langle x_i,\ldots,x_{i+q-1}\rangle,s,l,u)\f$
   * where the among constraint is defined as
   * \f$l\leq\#\{j\in\{i,\ldots,i+q-1\}\;|\;x_j\in s\} \leq u\f$.
   *
   * Throws the following exceptions:
   *  - Of type Int::TooFewArguments, if \f$|x|=0\f$.
   *  - Of type Int::ArgumentSame, if \a x contains
   *    the same unassigned variable multiply.
   *  - Of type Int::OutOfRange, if \f$q < 1 \vee q > |x|\f$.
   */
  GECODE_INT_EXPORT void
  sequence(Home home, const BoolVarArgs& x, const IntSet& s,
           int q, int l, int u, IntPropLevel ipl=IPL_DEF);

  //@}

  /**
   * \defgroup TaskModelIntExt Extensional constraints
   * \ingroup TaskModelInt
   *
   * Extensional constraints support different ways of how the
   * extensionally defined relation between the variables is defined.
   * Examples include specification by a %DFA or a table.
   *
   * A %DFA can be defined by a regular expression, for regular expressions
   * see the module MiniModel.
   */

  /**
   * \brief Deterministic finite automaton (%DFA)
   *
   * After initialization, the start state is always zero.
   * The final states are contiguous ranging from the first to the
   * last final state.
   *
   * \ingroup TaskModelIntExt
   */
  class DFA : public SharedHandle {
  private:
    /// Implementation of DFA
    class DFAI;
    /// Test whether DFA is equal to \a d
    GECODE_INT_EXPORT
    bool equal(const DFA& d) const;
  public:
    /// Specification of a %DFA transition
    class Transition {
    public:
      int i_state; ///< input state
      int symbol;  ///< symbol
      int o_state; ///< output state
      /// Default constructor
      Transition(void);
      /// Initialize members
      Transition(int i_state0, int symbol0, int o_state0);
    };
    /// Iterator for %DFA transitions (sorted by symbols)
    class Transitions {
    private:
      /// Current transition
      const Transition* c_trans;
      /// End of transitions
      const Transition* e_trans;
    public:
      /// Initialize to all transitions of DFA \a d
      Transitions(const DFA& d);
      /// Initialize to transitions of DFA \a d for symbol \a n
      Transitions(const DFA& d, int n);
      /// Test whether iterator still at a transition
      bool operator ()(void) const;
      /// Move iterator to next transition
      void operator ++(void);
      /// Return in-state of current transition
      int i_state(void) const;
      /// Return symbol of current transition
      int symbol(void) const;
      /// Return out-state of current transition
      int o_state(void) const;
    };
    /// Iterator for %DFA symbols
    class Symbols {
    private:
      /// Current transition
      const Transition* c_trans;
      /// End of transitions
      const Transition* e_trans;
    public:
      /// Initialize to symbols of DFA \a d
      Symbols(const DFA& d);
      /// Test whether iterator still at a symbol
      bool operator ()(void) const;
      /// Move iterator to next symbol
      void operator ++(void);
      /// Return current symbol
      int val(void) const;
    };
    /**
     * \brief Initialize DFA
     *
     * - Start state is given by \a s.
     * - %Transitions are described by \a t, where the last element
     *   must have -1 as value for \c i_state.
     * - Final states are given by \a f, where the last final element
     *   must be -1.
     * - Minimizes the DFA, if \a minimize is true.
     * - Note that the transitions must be deterministic.
     */
    GECODE_INT_EXPORT
    void init(int s, Transition t[], int f[], bool minimize=true);
  public:
    friend class Transitions;
    /// Initialize for DFA accepting the empty word
    DFA(void);
    /**
     * \brief Initialize DFA
     *
     * - Start state is given by \a s.
     * - %Transitions are described by \a t, where the last element
     *   must have -1 as value for \c i_state.
     * - Final states are given by \a f, where the last final element
     *   must be -1.
     * - Minimizes the DFA, if \a minimize is true.
     * - Note that the transitions must be deterministic.
     */
    GECODE_INT_EXPORT
    DFA(int s, Transition t[], int f[], bool minimize=true);
    /**
     * \brief Initialize DFA
     *
     * - Start state is given by \a s.
     * - %Transitions are described by \a t.
     * - Final states are given by \a f.
     * - Minimizes the DFA, if \a minimize is true.
     * - Note that the transitions must be deterministic.
     */
    GECODE_INT_EXPORT
    DFA(int s, std::initializer_list<Transition> t,
        std::initializer_list<int> f, bool minimize=true);
    /// Initialize by DFA \a d (DFA is shared)
    DFA(const DFA& d);
    /// Test whether DFA is equal to \a d
    GECODE_INT_EXPORT
    bool operator ==(const DFA& d) const;
    /// Test whether DFA is not equal to \a d
    bool operator !=(const DFA& d) const;
    /// Return the number of states
    int n_states(void) const;
    /// Return the number of transitions
    int n_transitions(void) const;
    /// Return the number of symbols
    unsigned int n_symbols(void) const;
    /// Return maximal degree (in-degree and out-degree) of any state
    unsigned int max_degree(void) const;
    /// Return the number of the first final state
    int final_fst(void) const;
    /// Return the number of the last final state
    int final_lst(void) const;
    /// Return smallest symbol in DFA
    int symbol_min(void) const;
    /// Return largest symbol in DFA
    int symbol_max(void) const;
    /// Return hash key
    std::size_t hash(void) const;
  };

}

#include <gecode/int/extensional/dfa.hpp>

namespace Gecode {

  /** \brief Class represeting a set of tuples.
   *
   * A TupleSet is used for storing an extensional representation of a
   * constraint. After a TupleSet is finalized, no more tuples may be
   * added to it.
   *
   * \ingroup TaskModelIntExt
   */
  class TupleSet : public SharedHandle {
  public:
    /** \brief Type of a tuple
     *
     * The arity of the tuple is left implicit.
     */
    typedef int* Tuple;
    /// Import bit set data type
    typedef Gecode::Support::BitSetData BitSetData;
    /// Range information
    class Range {
    public:
      /// Minimum value
      int min;
      /// Maximum value
      int max;
      /// Begin of supports
      BitSetData* s;
      /// Return the width
      unsigned int width(void) const;
      /// Return the supports for value \a n
      const BitSetData* supports(unsigned int n_words, int n) const;
    };
  protected:
    /// Data about values in the table
    class ValueData {
    public:
      /// Number of ranges
      unsigned int n;
      /// Ranges
      Range* r;
      /// Find start range for value \a n
      unsigned int start(int n) const;
    };
    /**
     * \brief Data stored for a Table
     *
     */
    class GECODE_VTABLE_EXPORT Data : public SharedHandle::Object {
    protected:
      /// Initial number of free tuples
      static const int n_initial_free = 1024;
    public:
      /// Arity
      int arity;
      /// Number of words for support
      unsigned int n_words;
      /// Number of Tuples
      int n_tuples;
      /// Number of free tuple entries of arity
      int n_free;
      /// Smallest value
      int min;
      /// Largest value
      int max;
      /// Hash key
      std::size_t key;
      /// Tuple data
      int* td;
      /// Value data
      ValueData* vd;
      /// Pointer to all ranges
      Range* range;
      /// Pointer to all support data
      BitSetData* support;

      /// Return newly added tuple
      Tuple add(void);
      /// Return tuple with number \a i
      Tuple get(int i) const;
      /// Set bit \a n in bitset data \a d
      static void set(BitSetData* d, unsigned int n);
      /// Get bit \a n in bitset data \a d
      static bool get(const BitSetData* d, unsigned int n);
      /// Map tuple address to index
      unsigned int tuple2idx(Tuple t) const;
      /// Return first range for position \a i
      const Range* fst(int i) const;
      /// Return last range for position \a i
      const Range* lst(int i) const;
      /// Finalize datastructure (disallows additions of more Tuples)
      GECODE_INT_EXPORT
      void finalize(void);
      /// Resize tuple data
      GECODE_INT_EXPORT
      void resize(void);
      /// Is datastructure finalized
      bool finalized(void) const;
      /// Initialize as empty tuple set with arity \a a
      Data(int a);
      /// Delete implementation
      GECODE_INT_EXPORT
      virtual ~Data(void);
    };

    /// Get data (must be initialized and finalized)
    Data& data(void) const;
    /// Get raw data (must be initialized)
    Data& raw(void) const;
    /// Add tuple \a t to tuple set
    GECODE_INT_EXPORT
    void _add(const IntArgs& t);
    /// Test whether tuple set is equal to \a t
    GECODE_INT_EXPORT
    bool equal(const TupleSet& t) const;
  public:
    /// \name Initialization
    //@{
    /// Construct an unitialized tuple set
    TupleSet(void);
    /// Initialize for a tuple set with arity \a a
    GECODE_INT_EXPORT
    TupleSet(int a);
    /// Initialize an uninitialized tuple set
    GECODE_INT_EXPORT
    void init(int a);
    /// Initialize by TupleSet \a t (tuple set is shared)
    GECODE_INT_EXPORT
    TupleSet(const TupleSet& t);
    /// Assignment operator
    GECODE_INT_EXPORT
    TupleSet& operator =(const TupleSet& t);
    /// Initialize with DFA \a dfa for arity \a a
    GECODE_INT_EXPORT
    TupleSet(int a, const DFA& dfa);
    /// Test whether tuple set has been initialized
    operator bool(void) const;
    /// Test whether tuple set is equal to \a t
    bool operator ==(const TupleSet& t) const;
    /// Test whether tuple set is different from \a t
    bool operator !=(const TupleSet& t) const;
    //@}

    /// \name Addition and finalization
    //@{
    /// Add tuple \a t to tuple set
    TupleSet& add(const IntArgs& t);
    /// Is tuple set finalized
    bool finalized(void) const;
    /// Finalize tuple set
    void finalize(void);
    //@}

    /// \name Tuple access
    //@{
    /// Arity of tuple set
    int arity(void) const;
    /// Number of tuples
    int tuples(void) const;
    /// Return number of required bit set words
    unsigned int words(void) const;
    /// Get tuple \a i
    Tuple operator [](int i) const;
    /// Return minimal value in all tuples
    int min(void) const;
    /// Return maximal value in all tuples
    int max(void) const;
    /// Return hash key
    std::size_t hash(void) const;
    //@}

    /// \name Range access and iteration
    //@{
    /// Return first range for position \a i
    const Range* fst(int i) const;
    /// Return last range for position \a i
    const Range* lst(int i) const;
    /// Iterator over ranges
    class Ranges {
    protected:
      /// Current range
      const Range* c;
      /// Last range
      const Range* l;
    public:
      /// \name Constructors and initialization
      //@{
      /// Initialize for column \a i
      Ranges(const TupleSet& ts, int i);
      //@}

      /// \name Iteration control
      //@{
      /// Test whether iterator is still at a range
      bool operator ()(void) const;
      /// Move iterator to next range (if possible)
      void operator ++(void);
      //@}

      /// \name %Range access
      //@{
      /// Return smallest value of range
      int min(void) const;
      /// Return largest value of range
      int max(void) const;
      /// Return width of range (distance between minimum and maximum)
      unsigned int width(void) const;
      //@}
    };
    //@}
  };

}

#include <gecode/int/extensional/tuple-set.hpp>

namespace Gecode {

  /**
   * \brief Post domain consistent propagator for extensional constraint described by a DFA
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply. If shared occurences of variables
   * are required, unshare should be used.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const IntVarArgs& x, DFA d,
              IntPropLevel ipl=IPL_DEF);

  /**
   * \brief Post domain consistent propagator for extensional constraint described by a DFA
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply. If shared occurences of variables
   * are required, unshare should be used.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const BoolVarArgs& x, DFA d,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x\in t\f$.
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x\in t\f$ or \f$x\not\in t\f$.
   *
   * \li If \a pos is true, it posts a propagator for \f$x\in t\f$
   *     and otherwise for \f$x\not\in t\f$.
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$(x\in t)\equiv r\f$.
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, Reify r,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$(x\in t)\equiv r\f$ or \f$(x\not\in t)\equiv r\f$.
   *
   * \li If \a pos is true, it posts a propagator for \f$(x\in t)\equiv r\f$
   *     and otherwise for \f$(x\not\in t)\equiv r\f$.
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
              Reify r,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x\in t\f$.
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x\in t\f$ or \f$x\not\in t\f$.
   *
   * \li If \a pos is true, it posts a propagator for \f$x\in t\f$
   *     and otherwise for \f$x\not\in t\f$.
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$(x\in t)\equiv r\f$.
   *
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, Reify r,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$(x\in t)\equiv r\f$ or \f$(x\not\in t)\equiv r\f$.
   *
   * \li If \a pos is true, it posts a propagator for \f$(x\in t)\equiv r\f$
   *     and otherwise for \f$(x\not\in t)\equiv r\f$.
   * \li Supports domain consistency (\a ipl = IPL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   * \li Throws an exception of type Int::NotYetFinalized, if the tuple
   *     set \a t has not been finalized.
   *
   * \ingroup TaskModelIntExt
   */
  GECODE_INT_EXPORT void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              Reify r,
              IntPropLevel ipl=IPL_DEF);

}

#include <gecode/int/extensional.hpp>

namespace Gecode {

  /**
   * \defgroup TaskModelIntArith Arithmetic constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ \min\{x_0,x_1\}=x_2\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  min(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \min x=y\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   */
  GECODE_INT_EXPORT void
  min(Home home, const IntVarArgs& x, IntVar y,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \max\{x_0,x_1\}=x_2\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  max(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \max x=y\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   */
  GECODE_INT_EXPORT void
  max(Home home, const IntVarArgs& x, IntVar y,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$ \operatorname{argmin}(x)=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmin(Home home, const IntVarArgs& x, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmin}(x)+o=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmin(Home home, const IntVarArgs& x, int o, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmax}(x)=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmax(Home home, const IntVarArgs& x, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmax}(x)+o=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmax(Home home, const IntVarArgs& x, int o, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmin}(x)=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmin(Home home, const BoolVarArgs& x, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmin}(x)-o=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmin(Home home, const BoolVarArgs& x, int o, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmax}(x)=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmax(Home home, const BoolVarArgs& x, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$ \operatorname{argmax}(x)-o=y\f$
   *
   * In case of ties, the smallest value for \a y is chosen
   * (provided \a tiebreak is true).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   * If \a y occurs in \a x, an exception of type Int::ArgumentSame
   * is thrown.
   */
  GECODE_INT_EXPORT void
  argmax(Home home, const BoolVarArgs& x, int o, IntVar y, bool tiebreak=true,
         IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$ |x_0|=x_1\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  abs(Home home, IntVar x0, IntVar x1,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_1=x_2\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  mult(Home home, IntVar x0, IntVar x1, IntVar x2,
       IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2 \land x_0\ \mathrm{mod}\ x_1 = x_3\f$
   *
   * Supports bounds consistency (\a ipl = IPL_BND, default).
   */
  GECODE_INT_EXPORT void
  divmod(Home home, IntVar x0, IntVar x1, IntVar x2, IntVar x3,
         IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2\f$
   *
   * Supports bounds consistency (\a ipl = IPL_BND, default).
   */
  GECODE_INT_EXPORT void
  div(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{mod}\ x_1=x_2\f$
   *
   * Supports bounds consistency (\a ipl = IPL_BND, default).
   */
  GECODE_INT_EXPORT void
  mod(Home home, IntVar x0, IntVar x1, IntVar x2,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0^2=x_1\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  sqr(Home home, IntVar x0, IntVar x1,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$\lfloor\sqrt{x_0}\rfloor=x_1\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   */
  GECODE_INT_EXPORT void
  sqrt(Home home, IntVar x0, IntVar x1,
       IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$x_0^n=x_1\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   *
   * Throws an exception of type Int::OutOfLimits, if \a n is
   * negative.
   */
  GECODE_INT_EXPORT void
  pow(Home home, IntVar x0, int n, IntVar x1,
      IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagator for \f$\lfloor\sqrt[n]{x_0}\rfloor=x_1\f$
   *
   * Supports both bounds consistency (\a ipl = IPL_BND, default)
   * and domain consistency (\a ipl = IPL_DOM).
   *
   * Throws an exception of type Int::OutOfLimits, if \a n is
   * not strictly positive.
   */
  GECODE_INT_EXPORT void
  nroot(Home home, IntVar x0, int n, IntVar x1,
       IntPropLevel ipl=IPL_DEF);

  //@}

  /**
   * \defgroup TaskModelIntLI Linear constraints over integer variables
   * \ingroup TaskModelInt
   *
   * All variants for linear constraints over integer variables share
   * the following properties:
   *  - Bounds consistency (over the real numbers) is supported for
   *    all constraints (actually, for disequlities always domain consistency
   *    is used as it is cheaper). Domain consistency is supported for all
   *    non-reified constraint. As bounds consistency for inequalities
   *    coincides with domain consistency, the only
   *    real variation is for linear equations. Domain consistent
   *    linear equations have exponential complexity, so use with care!
   *  - If the integer propagation level IPL_DEF is used as argument
   *    (hence, default propagation) and the linear constraint is sufficiently
   *    simple (two variables with unit coefficients), the domain
   *    consistent propagation is used.
   *  - Variables occurring multiply in the argument arrays are replaced
   *    by a single occurrence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Int::Limits, an exception of type
   *    Int::OutOfLimits is thrown.
   *  - Assume the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the maximal
   *    available precision (at least \f$2^{48}\f$), an exception of
   *    type Int::OutOfLimits is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   */
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{irt} c\f$
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntVarArgs& x,
         IntRelType irt, int c,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{irt} y\f$
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntVarArgs& x,
         IntRelType irt, IntVar y,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{irt} c\right)\equiv r\f$
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntVarArgs& x,
         IntRelType irt, int c, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{irt} y\right)\equiv r\f$
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntVarArgs& x,
         IntRelType irt, IntVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const IntVarArgs& x,
         IntRelType irt, int c,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const IntVarArgs& x,
         IntRelType irt, IntVar y,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\right)\equiv r\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const IntVarArgs& x,
         IntRelType irt, int c, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} y\right)\equiv r\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLI
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const IntVarArgs& x,
         IntRelType irt, IntVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);


  /**
   * \defgroup TaskModelIntLB Linear constraints over Boolean variables
   * \ingroup TaskModelInt
   *
   * All variants for linear constraints over Boolean variables share
   * the following properties:
   *  - Bounds consistency (over the real numbers) is supported for
   *    all constraints (actually, for disequlities always domain consistency
   *    is used as it is cheaper).
   *  - Variables occurring multiply in the argument arrays are replaced
   *    by a single occurrence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Int::Limits, an exception of type
   *    Int::OutOfLimits is thrown.
   *  - Assume the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Int::Limits, an exception of
   *    type Int::OutOfLimits is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   */
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{irt} c\f$
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const BoolVarArgs& x,
         IntRelType irt, int c,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{irt} c\right)\equiv r\f$
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const BoolVarArgs& x,
         IntRelType irt, int c, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{irt} y\f$
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const BoolVarArgs& x,
         IntRelType irt, IntVar y,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{irt} y\right)\equiv r\f$
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const BoolVarArgs& x,
         IntRelType irt, IntVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType irt, int c,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} c\right)\equiv r\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType irt, int c, Reify r,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType irt, IntVar y,
         IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{irt} y\right)\equiv r\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelIntLB
   */
  GECODE_INT_EXPORT void
  linear(Home home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType irt, IntVar y, Reify r,
         IntPropLevel ipl=IPL_DEF);


  /**
   * \defgroup TaskModelIntBinPacking Bin packing constraints
   * \ingroup TaskModelInt
   *
   */
  /** \brief Post propagator for bin packing
   *
   * The variables in \a l are the loads for each bin, whereas the
   * variables in \a b define for each item into which bin it is packed.
   * The integer values \a s define the size of the items.
   *
   * It is propagated that for each \f$j\f$ with \f$0\leq j<|l|\f$ the
   * constraint \f$l_j=\sum_{0\leq i<|b|\wedge b_i=j}s_i\f$ holds and that
   * for each \f$i\f$ with \f$0\leq i<|b|\f$ the constraint
   * \f$0\leq b_i<|l|\f$ holds.
   *
   * The propagation follows: Paul Shaw. A Constraint for Bin Packing. CP 2004.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if \a b and \a s are not of
   *    the same size.
   *  - Of type Int::ArgumentSame if \a l and \a b share unassigned variables.
   *  - Of type Int::OutOfLimits if \a s contains a negative number.
   *
   * \ingroup TaskModelIntBinPacking
   */
  GECODE_INT_EXPORT void
  binpacking(Home home,
             const IntVarArgs& l,
             const IntVarArgs& b, const IntArgs& s,
             IntPropLevel ipl=IPL_DEF);
  /* \brief Post propagator for multi-dimensional bin packing
   *
   * In the following \a n refers to the number of items and \a m
   * refers to the number of bins.
   *
   * The multi-dimensional bin-packing constraint enforces that
   * all items are packed into bins
   * \f$b_i\in\{0,\ldots,m-1\}\f$ for \f$0\leq i<n\f$
   * and that the load of each bin corresponds to the items
   * packed into it for each dimension \f$l_{j\cdot
   * d + k} = \sum_{\{i\in\{0,\ldots,n-1\}|
   * b_{j\cdot d+k}=i}\}s_{i\cdot d+k}\f$
   * for \f$0\leq j<m\f$, \f$0\leq k<d\f$
   * Furthermore, the load variables must satisfy the capacity
   * constraints \f$l_{j\cdot d + k} \leq
   * c_k\f$ for \f$0\leq j<m\f$, \f$0\leq k<d\f$.
   *
   * The constraint is implemented by the decomposition
   * introduced in: Stefano Gualandi and Michele Lombardi. A
   * simple and effective decomposition for the multidimensional
   * binpacking constraint. CP 2013, pages 356--364.
   *
   * Posting the constraint returns a maximal set containing conflicting
   * items that require pairwise different bins.
   *
   * Note that posting the constraint has exponential complexity in the
   * number of items due to the Bron-Kerbosch algorithm used for finding
   * the maximal conflict item sets.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if any of the following properties
   *    is violated: \f$|b|=n\f$, \f$|l|=m\cdot d\f$, \f$|s|=n\cdot d\f$,
   *    and \f$|c|=d\f$.
   *  - Of type Int::ArgumentSame if \a l and \a b share unassigned variables.
   *  - Of type Int::OutOfLimits if \a s or \a c contains a negative number.
   *
   * \ingroup TaskModelIntBinPacking
   */
  GECODE_INT_EXPORT IntSet
  binpacking(Home home, int d,
             const IntVarArgs& l, const IntVarArgs& b,
             const IntArgs& s, const IntArgs& c,
             IntPropLevel ipl=IPL_DEF);


  /**
   * \defgroup TaskModelIntGeoPacking Geometrical packing constraints
   * \ingroup TaskModelInt
   *
   * Constraints for modeling geometrical packing problems.
   */
  /** \brief Post propagator for rectangle packing
   *
   * Propagate that no two rectangles as described by the coordinates
   * \a x, and \a y, widths \a w, and heights \a h overlap.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if \a x, \a w, \a y, or \a h
   *    are not of the same size.
   *  - Of type Int::OutOfLimits if \a w or \a h contain a negative number.
   *
   * \ingroup TaskModelIntGeoPacking
   */
  GECODE_INT_EXPORT void
  nooverlap(Home home,
            const IntVarArgs& x, const IntArgs& w,
            const IntVarArgs& y, const IntArgs& h,
            IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for rectangle packing
   *
   * Propagate that no two rectangles as described by the coordinates
   * \a x, and \a y, widths \a w, and heights \a h overlap. The rectangles
   * can be optional, as described by the Boolean variables \a o.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if \a x, \a w, \a y, \a h, or \a o
   *    are not of the same size.
   *  - Of type Int::OutOfLimits if \a w or \a h contain a negative number.
   *
   * \ingroup TaskModelIntGeoPacking
   */
  GECODE_INT_EXPORT void
  nooverlap(Home home,
            const IntVarArgs& x, const IntArgs& w,
            const IntVarArgs& y, const IntArgs& h,
            const BoolVarArgs& o,
            IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for rectangle packing
   *
   * Propagate that no two rectangles as described by the start coordinates
   * \a x0 and \a y0, widths \a w and heights \a h, and end coordinates
   * \a x1 and \a y1 overlap.
   *
   * Note that the relations \f$x0_i+w_i=x1_i\f$ and \f$y0_i+h_i=y1_i\f$ are
   * not propagated (for \f$0\leq i<|x0|\f$). That is, additional constraints
   * must be posted to enforce that relation.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if \a x0, \a x1, \a w,
   *    \a y0, \a y1, or \a h are not of the same size.
   *
   * \ingroup TaskModelIntGeoPacking
   */
  GECODE_INT_EXPORT void
  nooverlap(Home home,
            const IntVarArgs& x0, const IntVarArgs& w, const IntVarArgs& x1,
            const IntVarArgs& y0, const IntVarArgs& h, const IntVarArgs& y1,
            IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator for rectangle packing
   *
   * Propagate that no two rectangles as described by the start coordinates
   * \a x0 and \a y0, widths \a w and heights \a h, and end coordinates
   * \a x1 and \a y1 overlap. The rectangles can be optional, as described
   * by the Boolean variables \a o.
   *
   * Note that the relations \f$x0_i+w_i=x1_i\f$ and \f$y0_i+h_i=y1_i\f$ are
   * not propagated (for \f$0\leq i<|x0|\f$). That is, additional constraints
   * must be posted to enforce that relation.
   *
   * Throws the following exceptions:
   *  - Of type Int::ArgumentSizeMismatch if \a x0, \a x1, \a w,
   *    \a y0, \a y1, or \a h are not of the same size.
   *
   * \ingroup TaskModelIntGeoPacking
   */
  GECODE_INT_EXPORT void
  nooverlap(Home home,
            const IntVarArgs& x0, const IntVarArgs& w, const IntVarArgs& x1,
            const IntVarArgs& y0, const IntVarArgs& h, const IntVarArgs& y1,
            const BoolVarArgs& o,
            IntPropLevel ipl=IPL_DEF);


  /**
   * \defgroup TaskModelIntScheduling Scheduling constraints
   * \ingroup TaskModelInt
   */
  //@{

  /** \brief Post propagators for ordering two tasks
   *
   * Order two tasks with start times \f$s_0\f$ and \f$s_1\f$ with
   * processing times \f$p_0\f$ and \f$p_1\f$ according to Boolean variable
   * \a b (if \a b is zero \f$s_0\f$ starts before \f$s_1\f$).
   *
   * Throws an exception of Int::OutOfLimits, if the durations or
   * the sum of durations and start times are too large.
   *
   */
  GECODE_INT_EXPORT void
  order(Home home, IntVar s0, int p0, IntVar s1, int p1, BoolVar b,
        IntPropLevel ipl=IPL_DEF);

  /**
   * \brief Post propagators for the cumulatives constraint
   *
   * This function creates propagators for the cumulatives constraint
   * presented in <em>"A new multi-resource cumulatives constraint
   * with negative heights"</em>, Nicolas Beldiceanu and Mats
   * Carlsson, Principles and Practice of Constraint Programming 2002.
   *
   * The constraint models a set of machines and a set of tasks that
   * should be assigned to the machines. The machines have a positive
   * resource limit and the tasks each have a resource usage that can
   * be either positive, negative, or zero. The constraint is enforced
   * over each point in time for a machine where there is at least one
   * task assigned.
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The limit for a machine is either the maximum amount available at
   * any given time (\a at_most = true), or else the least amount to
   * be used (\a at_most = false).
   *
   * \param home current space
   * \param m \f$ m_i \f$ is the machine assigned to task \f$ i \f$
   * \param s \f$ s_i \f$ is the start time assigned to task \f$ i \f$
   * \param p \f$ p_i \f$ is the processing time of task \f$ i \f$
   * \param e \f$ e_i \f$ is the end time assigned to task \f$ i \f$
   * \param u \f$ u_i \f$ is the amount of
   *               resources consumed by task \f$ i \f$
   * \param c \f$ c_r \f$ is the capacity, the amount of resource available
   *              for machine \f$ r \f$
   * \param at_most \a at_most tells if the amount of resources used
   *                for a machine should be less than the limit (\a at_most
   *                = true) or greater than the limit (\a at_most = false)
   * \param ipl Supports value-consistency only (\a ipl = IPL_VAL, default).
   *
   * \exception Int::ArgumentSizeMismatch thrown if the sizes
   *            of the arguments representing tasks does not match.
   * \exception Int::OutOfLimits thrown if any numerical argument is
   *            larger than Int::Limits::max or less than
   *            Int::Limits::min.
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntVarArgs& m,
              const IntVarArgs& s, const IntVarArgs& p,
              const IntVarArgs& e, const IntVarArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntArgs& m,
              const IntVarArgs& s, const IntVarArgs& p,
              const IntVarArgs& e, const IntVarArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntVarArgs& m,
              const IntVarArgs& s, const IntArgs& p,
              const IntVarArgs& e, const IntVarArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntArgs& m,
              const IntVarArgs& s, const IntArgs& p,
              const IntVarArgs& e, const IntVarArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntVarArgs& m,
              const IntVarArgs& s, const IntVarArgs& p,
              const IntVarArgs& e, const IntArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntArgs& m,
              const IntVarArgs& s, const IntVarArgs& p,
              const IntVarArgs& e, const IntArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntVarArgs& m,
              const IntVarArgs& s, const IntArgs& p,
              const IntVarArgs& e, const IntArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Home home, const IntArgs& m,
              const IntVarArgs& s, const IntArgs& p,
              const IntVarArgs& e, const IntArgs& u,
              const IntArgs& c, bool at_most,
              IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with start times \a s and processing times \a p
   * on a unary resource. The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined (default), all the above listed propagation
   *    is performed.
   *
   * Posting the constraint might throw the following exceptions:
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s
   *    and \a p are of different size.
   *  - Throws an exception of type Int::ArgumentSame, if \a s contains
   *    the same unassigned variable multiply.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntArgs& p,
        IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. The propagator uses the
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined (default), all the above listed propagation
   *    is performed.
   *
   * Posting the constraint might throw the following exceptions:
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, or \a m are of different size.
   *  - Throws an exception of type Int::ArgumentSame, if \a s contains
   *    the same unassigned variable multiply.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntArgs& p,
        const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with flexible times \a flex and fixed times \a fix
   * on a unary resource. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined (default), all the above listed propagation
   *    is performed.
   *
   * Posting the constraint might throw the following exceptions:
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s
   *    and \a p are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative for a task with type <code>TT_FIXP</code>
   *    or that could generate an overflow.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& flex, const IntArgs& fix, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with flexible times \a flex, fixed times \a fix,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator uses the
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined (default), all the above listed propagation
   *    is performed.
   *
   * Posting the constraint might throw the following exceptions:
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p contains
   *    an integer that is negative for a task with type <code>TT_FIXP</code>
   *    or that could generate an overflow.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const TaskTypeArgs& t,
        const IntVarArgs& flex, const IntArgs& fix,
        const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on unary resources
   *
   * Schedule tasks with start times \a s, processing times \a p, and
   * end times \a e
   * on a unary resource. The propagator uses the algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined (default), all the above listed propagation
   *    is performed.
   *
   * The processing times are constrained to be non-negative.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a s
   * and \a p are of different size.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on unary resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * end times \a e,
   * and whether a task is mandatory \a m (a task is mandatory if the
   * Boolean variable is 1) on a unary resource. The propagator uses the
   * algorithms from:
   *   Petr Vilím, Global Constraints in Scheduling, PhD thesis,
   *   Charles University, Prague, Czech Republic, 2007.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking,
   *    detectable precendence propagation, not-first-not-last propagation,
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed (default).
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The processing times are constrained to be non-negative.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   * \a p, or \a m are of different size.
   */
  GECODE_INT_EXPORT void
  unary(Home home, const IntVarArgs& s, const IntVarArgs& p,
        const IntVarArgs& e, const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);



  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with flexible times \a flex, fixed times \a fix, and
   * use capacity \a u on a cumulative resource with capacity \a c. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a t, \a s
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& flex, const IntArgs& fix, const IntArgs& u,
             IntPropLevel ipl=IPL_DEF);


  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * \copydoc cumulative(Home,int,const TaskTypeArgs&,const IntVarArgs&,const IntArgs&,const IntArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& flex, const IntArgs& fix, const IntArgs& u,
             IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule tasks with flexible times \a flex, fixed times \a fix,
   * use capacity \a u, and whether a task is mandatory \a m (a task is
   * mandatory if the Boolean variable is 1) on a cumulative resource with
   * capacity \a c. For each
   * task, it depends on \a t how the flexible and fix times are interpreted:
   *  - If <code>t[i]</code> is <code>TT_FIXP</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    processing time.
   *  - If <code>t[i]</code> is <code>TT_FIXS</code>, then
   *    <code>flex[i]</code> is the end time and <code>fix[i]</code> is the
   *    start time.
   *  - If <code>t[i]</code> is <code>TT_FIXE</code>, then
   *    <code>flex[i]</code> is the start time and <code>fix[i]</code> is the
   *    end time.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a t, \a s
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const TaskTypeArgs& t,
             const IntVarArgs& flex, const IntArgs& fix, const IntArgs& u,
             const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   * \copydoc cumulative(Home,int,const TaskTypeArgs&,const IntVarArgs&,const IntArgs&,const IntArgs&,const BoolVarArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const TaskTypeArgs& t,
             const IntVarArgs& flex, const IntArgs& fix, const IntArgs& u,
             const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with start times \a s, processing times \a p, and
   * use capacity \a u on a cumulative resource with capacity \a c.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   * \copydoc cumulative(Home,int,const IntVarArgs&,const IntArgs&,const IntArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * used capacity \a u, and whether a task is mandatory \a m (a task is
   * mandatory if the Boolean variable is 1) on a cumulative resource
   * with capacity \a c.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, \a u, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a p, \a u, or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   * \copydoc cumulative(Home,int,const IntVarArgs&,const IntArgs&,const IntArgs&,const BoolVarArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntArgs& p,
             const IntArgs& u, const BoolVarArgs& m, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   *
   * Schedule tasks with start times \a s, processing times \a p,
   * end times \a e, and
   * use capacity \a u on a cumulative resource with capacity \a c.
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s
   *    \a p, or \a u are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a u or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling tasks on cumulative resources
   * \copydoc cumulative(Home,int,const IntVarArgs&,const IntVarArgs&,const IntVarArgs&,const IntArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   *
   * Schedule optional tasks with start times \a s, processing times \a p,
   * end times \a e,
   * used capacity \a u, and whether a task is mandatory \a m (a task is
   * mandatory if the Boolean variable is 1) on a cumulative resource
   * with capacity \a c.
   *
   * The propagator does not enforce \f$s_i+p_i=e_i\f$, this constraint
   * has to be posted in addition to ensure consistency of the task bounds.
   *
   * The propagator performs propagation that depends on the integer
   * propagation level \a ipl as follows:
   *  - If \a IPL_BASIC is set, the propagator performs overload checking
   *    and time-tabling propagation.
   *  - If \a IPL_ADVANCED is set, the propagator performs overload checking
   *    and edge finding.
   *  - If both flags are combined, all the above listed propagation is
   *    performed.
   *
   * The propagator uses algorithms taken from:
   *
   * Petr Vilím, Max Energy Filtering Algorithm for Discrete Cumulative
   * Resources, in W. J. van Hoeve and J. N. Hooker, editors, CPAIOR, volume
   * 5547 of LNCS, pages 294-308. Springer, 2009.
   *
   * and
   *
   * Petr Vilím, Edge finding filtering algorithm for discrete cumulative
   * resources in O(kn log n). In I. P. Gent, editor, CP, volume 5732 of LNCS,
   * pages 802-816. Springer, 2009.
   *
   *  - Throws an exception of type Int::ArgumentSizeMismatch, if \a s,
   *    \a p, \a u, or \a m are of different size.
   *  - Throws an exception of type Int::OutOfLimits, if \a u or \a c
   *    contain an integer that is not nonnegative, or that could generate
   *    an overflow.
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, int c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m,
             IntPropLevel ipl=IPL_DEF);

  /** \brief Post propagators for scheduling optional tasks on cumulative resources
   * \copydoc cumulative(Home,int,const IntVarArgs&,const IntVarArgs&,const IntVarArgs&,const IntArgs&,const BoolVarArgs&,IntPropLevel)
   */
  GECODE_INT_EXPORT void
  cumulative(Home home, IntVar c, const IntVarArgs& s, const IntVarArgs& p,
             const IntVarArgs& e, const IntArgs& u, const BoolVarArgs& m,
             IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntGraph Graph constraints
   * \ingroup TaskModelInt
   */
  //@{
  /** \brief Post propagator such that \a x forms a circuit
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   */
  GECODE_INT_EXPORT void
  circuit(Home home, const IntVarArgs& x,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a circuit
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_{i-\text{offset}}=j\f$ has a single cycle covering all nodes.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  circuit(Home home, int offset, const IntVarArgs& x,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with costs \a y and \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes.
   * The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire circuit. The variables \a y define the cost
   * of the edge in \a x: that is, if \f$x_i=j\f$ then \f$y_i=c_{i*n+j}\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismatch, if \a x and \a y do not have the same
   *    size or if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_INT_EXPORT void
  circuit(Home home,
          const IntArgs& c,
          const IntVarArgs& x, const IntVarArgs& y, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with costs \a y and \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_{i-\text{offset}}=j\f$ has a single cycle covering all nodes.
   * The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire circuit. The variables \a y define the cost
   * of the edge in \a x: that is, if \f$x_i=j\f$ then \f$y_i=c_{i*n+j}\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismatch, if \a x and \a y do not have the same
   *    size or if \f$|x|\times|x|\neq|c|\f$.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  circuit(Home home,
          const IntArgs& c, int offset,
          const IntVarArgs& x, const IntVarArgs& y, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with cost \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes. The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire circuit.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismatch, if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_INT_EXPORT void
  circuit(Home home,
          const IntArgs& c,
          const IntVarArgs& x, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with cost \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_{i-\text{offset}}=j\f$ has a single cycle covering all nodes.
   * The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire circuit.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismatch, if \f$|x|\times|x|\neq|c|\f$.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  circuit(Home home,
          const IntArgs& c, int offset,
          const IntVarArgs& x, IntVar z,
          IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_i=j\f$ visits all nodes exactly once. The path starts at
   * node \a s and the successor of the last node \a e is equal to \f$|x|\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   */
  GECODE_INT_EXPORT void
  path(Home home, const IntVarArgs& x, IntVar s, IntVar e,
       IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_{i-\text{offset}}=j\f$ visits all nodes exactly once.
   * The path starts at node \a s and the successor of the last node \a e
   * is equal to \f$|x|+\text{offset}\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  path(Home home, int offset, const IntVarArgs& x, IntVar s, IntVar e,
       IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path with costs \a y and \a z
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_i=j\f$ visits all nodes exactly once. The path starts at node
   * \a s and the successor of
   * the last node \a e is equal to \f$|x|\f$. The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire path. The variables \a y define the cost
   * of the edge in \a x: that is, if \f$x_i=j\f$ then \f$y_i=c_{i*n+j}\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \a x and \a y do not have the same
   *    size or if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_INT_EXPORT void
  path(Home home,
       const IntArgs& c,
       const IntVarArgs& x, IntVar s, IntVar e, const IntVarArgs& y, IntVar z,
       IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path with costs \a y and \a z
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_{i-\text{offset}}=j\f$ visits all nodes exactly once.
   * The path starts at node \a s and the successor of
   * the last node \a e is equal to \f$|x|+\text{offset}\f$.
   * The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire path. The variables \a y define the cost
   * of the edge in \a x: that is, if \f$x_i=j\f$ then \f$y_i=c_{i*n+j}\f$.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \a x and \a y do not have the same
   *    size or if \f$|x|\times|x|\neq|c|\f$.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  path(Home home,
       const IntArgs& c, int offset,
       const IntVarArgs& x, IntVar s, IntVar e, const IntVarArgs& y, IntVar z,
       IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path with cost \a z
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_i=j\f$ visits all nodes exactly once. The path starts at node
   * \a s and the successor of
   * the last node \a e is equal to \f$|x|\f$. The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire path.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_INT_EXPORT void
  path(Home home,
       const IntArgs& c,
       const IntVarArgs& x, IntVar s, IntVar e, IntVar z,
       IntPropLevel ipl=IPL_DEF);
  /** \brief Post propagator such that \a x forms a Hamiltonian path with cost \a z
   *
   * \a x forms a Hamiltonian path if the graph with edges \f$i\to j\f$
   * where \f$x_{i-\text{offset}}=j\f$ visits all nodes exactly once.
   * The path starts at node \a s and the successor of
   * the last node \a e is equal to \f$|x|+\text{offset}\f$.
   * The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of
   * the entire circuit.
   *
   * Supports domain (\a ipl = IPL_DOM) and value propagation (all
   * other values for \a ipl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \f$|x|\times|x|\neq|c|\f$.
   *  - Int::OutOfLimits, if \a offset is negative.
   */
  GECODE_INT_EXPORT void
  path(Home home,
       const IntArgs& c, int offset,
       const IntVarArgs& x, IntVar s, IntVar e, IntVar z,
       IntPropLevel ipl=IPL_DEF);
  //@}



  /**
   * \defgroup TaskModelIntExec Synchronized execution
   * \ingroup TaskModelInt
   *
   * Synchronized execution executes a function or a static member function
   * when a certain event happends.
   */
  //@{
  /// Execute \a c when \a x becomes assigned
  GECODE_INT_EXPORT void
  wait(Home home, IntVar x, std::function<void(Space& home)> c,
       IntPropLevel ipl=IPL_DEF);
  /// Execute \a c when \a x becomes assigned
  GECODE_INT_EXPORT void
  wait(Home home, BoolVar x, std::function<void(Space& home)> c,
       IntPropLevel ipl=IPL_DEF);
  /// Execute \a c when all variables in \a x become assigned
  GECODE_INT_EXPORT void
  wait(Home home, const IntVarArgs& x, std::function<void(Space& home)> c,
       IntPropLevel ipl=IPL_DEF);
  /// Execute \a c when all variables in \a x become assigned
  GECODE_INT_EXPORT void
  wait(Home home, const BoolVarArgs& x,
       std::function<void(Space& home)> c,
       IntPropLevel ipl=IPL_DEF);
  /// Execute \a t (then) when \a x is assigned one, and \a e (else) otherwise
  GECODE_INT_EXPORT void
  when(Home home, BoolVar x,
       std::function<void(Space& home)> t,
       std::function<void(Space& home)> e,
       IntPropLevel ipl=IPL_DEF);
  /// Execute \a t (then) when \a x is assigned one
  GECODE_INT_EXPORT void
  when(Home home, BoolVar x,
       std::function<void(Space& home)> t,
       IntPropLevel ipl=IPL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntUnshare Unsharing variables
   * \ingroup TaskModelInt
   *
   * Unsharing replaces multiple occurences of the same variable by
   * fresh yet equal (enforced through propagators for equality)
   * variables: after unsharing a variable appears at most once. Note
   * that this is only done for not yet assigned variables (as all
   * propagators can handle multiple occurences of the same variable
   * provided it is already assigned).
   *
   * Unsharing is useful for constraints that only accept variable
   * arrays without multiple occurences of the same variable, for
   * example extensional.
   *
   */
  //@{
  /**
   * \brief Replace multiple variable occurences in \a x by fresh variables
   *
   * Supports domain consistency (\a ipl = IPL_DOM, default) and
   * bounds consistency (\a ipl = IPL_BND).
   *
   */
  GECODE_INT_EXPORT void
  unshare(Home home, IntVarArgs& x,
          IntPropLevel ipl=IPL_DEF);
  /// Replace multiple variable occurences in \a x by fresh variables
  GECODE_INT_EXPORT void
  unshare(Home home, BoolVarArgs& x,
          IntPropLevel ipl=IPL_DEF);
  //@}

}

namespace Gecode {

  /**
   * \defgroup TaskModelIntBranch Branching
   * \ingroup TaskModelInt
   */

  /**
   * \brief Branch filter function type for integer variables
   *
   * The variable \a x is considered for selection and \a i refers to the
   * variable's position in the original array passed to the brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<bool(const Space& home, IntVar x, int i)>
    IntBranchFilter;
  /**
   * \brief Branch filter function type for Boolean variables
   *
   * The variable \a x is considered for selection and \a i refers to the
   * variable's position in the original array passed to the brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<bool(const Space& home, BoolVar x, int i)>
    BoolBranchFilter;

  /**
   * \brief Branch merit function type for integer variables
   *
   * The function must return a merit value for the variable
   * \a x. The integer \a i refers to the variable's position
   * in the original array passed to the brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<double(const Space& home, IntVar x, int i)> 
    IntBranchMerit;
  /**
   * \brief Branch merit function type for Boolean variables
   *
   * The function must return a merit value for the variable
   * \a x. The integer \a i refers to the variable's position
   * in the original array passed to the brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<double(const Space& home, BoolVar x, int i)>
    BoolBranchMerit;

  /**
   * \brief Branch value function type for integer variables
   *
   * Returns a value for the variable \a x that is to be used in the
   * corresponding branch commit function. The integer \a i refers
   * to the variable's position in the original array passed to the
   * brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<int(const Space& home, IntVar x, int i)>
    IntBranchVal;
  /**
   * \brief Branch value function type for Boolean variables
   *
   * Returns a value for the variable \a x that is to be used in the
   * corresponding branch commit function. The integer \a i refers
   * to the variable's position in the original array passed to the
   * brancher.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<int(const Space& home, BoolVar x, int i)>
    BoolBranchVal;

  /**
   * \brief Branch commit function type for integer variables
   *
   * The function must post a constraint on the variable \a x which
   * corresponds to the alternative \a a. The integer \a i refers
   * to the variable's position in the original array passed to the
   * brancher. The value \a n is the value
   * computed by the corresponding branch value function.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<void(Space& home, unsigned int a,
                             IntVar x, int i, int n)>
    IntBranchCommit;
  /**
   * \brief Branch commit function type for Boolean variables
   *
   * The function must post a constraint on the variable \a x which
   * corresponds to the alternative \a a.  The integer \a i refers
   * to the variable's position in the original array passed to the
   * brancher. The value \a n is the value
   * computed by the corresponding branch value function.
   *
   * \ingroup TaskModelIntBranch
   */
  typedef std::function<void(Space& home, unsigned int a,
                             BoolVar x, int i, int n)>
    BoolBranchCommit;

}

#include <gecode/int/branch/traits.hpp>

namespace Gecode {

  /**
   * \brief Recording AFC information for integer variables
   *
   * \ingroup TaskModelIntBranch
   */
  class IntAFC : public AFC {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized AFC storage is init or the assignment operator.
     *
     */
    IntAFC(void);
    /// Copy constructor
    IntAFC(const IntAFC& a);
    /// Assignment operator
    IntAFC& operator =(const IntAFC& a);
    /**
     * \brief Initialize for integer variables \a x and decay factor \a d
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    IntAFC(Home home, const IntVarArgs& x, double d=1.0, bool share=true);
    /**
     * \brief Initialize for integer variables \a x with decay factor \a d
     *
     * This member function can only be used once and only if the
     * AFC storage has been constructed with the default constructor.
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    void init(Home home, const IntVarArgs& x, double d=1.0, bool share=true);
  };

  /**
   * \brief Recording AFC information for Boolean variables
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolAFC : public AFC {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized AFC storage is init or the assignment operator.
     *
     */
    BoolAFC(void);
    /// Copy constructor
    BoolAFC(const BoolAFC& a);
    /// Assignment operator
    BoolAFC& operator =(const BoolAFC& a);
    /**
     * \brief Initialize for Boolean variables \a x and decay factor \a d
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    BoolAFC(Home home, const BoolVarArgs& x, double d=1.0, bool share=true);
    /**
     * \brief Initialize for Boolean variables \a x with decay factor \a d
     *
     * This member function can only be used once and only if the
     * AFC storage has been constructed with the default constructor.
     *
     * If several AFC objects are created for a space or its clones,
     * the AFC values are shared between spaces. If the values should
     * not be shared, \a share should be false.
     */
    void init(Home home, const BoolVarArgs& x, double d=1.0, bool share=true);
  };

}

#include <gecode/int/branch/afc.hpp>

namespace Gecode {

  /**
   * \brief Recording actions for integer variables
   *
   * \ingroup TaskModelIntBranch
   */
  class IntAction : public Action {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized action storage is init or the assignment operator.
     *
     */
    IntAction(void);
    /// Copy constructor
    IntAction(const IntAction& a);
    /// Assignment operator
    IntAction& operator =(const IntAction& a);
    /**
     * \brief Initialize for integer variables \a x with decay factor \a d
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     */
    GECODE_INT_EXPORT
    IntAction(Home home, const IntVarArgs& x, double d=1.0,
              IntBranchMerit bm=nullptr);
    /**
     * \brief Initialize for integer variables \a x with decay factor \a d
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     * This member function can only be used once and only if the
     * action storage has been constructed with the default constructor.
     *
     */
    GECODE_INT_EXPORT void
    init(Home home, const IntVarArgs& x, double d=1.0,
         IntBranchMerit bm=nullptr);
  };

  /**
   * \brief Recording actions for Boolean variables
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolAction : public Action {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized action storage is init or the assignment operator.
     *
     */
    BoolAction(void);
    /// Copy constructor
    BoolAction(const BoolAction& a);
    /// Assignment operator
    BoolAction& operator =(const BoolAction& a);
    /**
     * \brief Initialize for Boolean variables \a x with decay factor \a d
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     */
    GECODE_INT_EXPORT
    BoolAction(Home home, const BoolVarArgs& x, double d=1.0,
               BoolBranchMerit bm=nullptr);
    /**
     * \brief Initialize for Boolean variables \a x with decay factor \a d
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     * This member function can only be used once and only if the
     * action storage has been constructed with the default constructor.
     *
     */
    GECODE_INT_EXPORT void
    init(Home home, const BoolVarArgs& x, double d=1.0,
         BoolBranchMerit bm=nullptr);
  };

}

#include <gecode/int/branch/action.hpp>

namespace Gecode {

  /**
   * \brief Recording CHB for integer variables
   *
   * \ingroup TaskModelIntBranch
   */
  class IntCHB : public CHB {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized CHB storage is init or the assignment operator.
     *
     */
    IntCHB(void);
    /// Copy constructor
    IntCHB(const IntCHB& chb);
    /// Assignment operator
    IntCHB& operator =(const IntCHB& chb);
   /**
     * \brief Initialize for integer variables \a x
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     */
    GECODE_INT_EXPORT
    IntCHB(Home home, const IntVarArgs& x, IntBranchMerit bm=nullptr);
   /**
     * \brief Initialize for integer variables \a x
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     * This member function can only be used once and only if the
     * action storage has been constructed with the default constructor.
     *
     */
    GECODE_INT_EXPORT void
    init(Home home, const IntVarArgs& x, IntBranchMerit bm=nullptr);
  };

  /**
   * \brief Recording CHB for Boolean variables
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolCHB : public CHB {
  public:
    /**
     * \brief Construct as not yet initialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized action storage is init or the assignment operator.
     *
     */
    BoolCHB(void);
    /// Copy constructor
    BoolCHB(const BoolCHB& chb);
    /// Assignment operator
    BoolCHB& operator =(const BoolCHB& chb);
   /**
     * \brief Initialize for Boolean variables \a x
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     */
    GECODE_INT_EXPORT
    BoolCHB(Home home, const BoolVarArgs& x, BoolBranchMerit bm=nullptr);
   /**
     * \brief Initialize for Boolean variables \a x
     *
     * If the branch merit function \a bm is different from nullptr, the
     * action for each variable is initialized with the merit returned
     * by \a bm.
     *
     * This member function can only be used once and only if the
     * action storage has been constructed with the default constructor.
     *
     */
    GECODE_INT_EXPORT void
    init(Home home, const BoolVarArgs& x, BoolBranchMerit bm=nullptr);
  };

}

#include <gecode/int/branch/chb.hpp>

namespace Gecode {

  /// Function type for printing branching alternatives for integer variables
  typedef std::function<void(const Space &home, const Brancher& b,
                             unsigned int a,
                             IntVar x, int i, const int& n,
                             std::ostream& o)>
    IntVarValPrint;

  /// Function type for printing branching alternatives for Boolean variables
  typedef std::function<void(const Space &home, const Brancher& b,
                             unsigned int a,
                             BoolVar x, int i, const int& n,
                             std::ostream& o)>
    BoolVarValPrint;

}

namespace Gecode {

  /**
   * \brief Which integer variable to select for branching
   *
   * \ingroup TaskModelIntBranch
   */
  class IntVarBranch : public VarBranch<IntVar> {
  public:
    /// Which variable selection
    enum Select {
      SEL_NONE = 0,        ///< First unassigned
      SEL_RND,             ///< Random (uniform, for tie breaking)
      SEL_MERIT_MIN,       ///< With least merit
      SEL_MERIT_MAX,       ///< With highest merit
      SEL_DEGREE_MIN,      ///< With smallest degree
      SEL_DEGREE_MAX,      ///< With largest degree
      SEL_AFC_MIN,         ///< With smallest accumulated failure count
      SEL_AFC_MAX,         ///< With largest accumulated failure count
      SEL_ACTION_MIN,      ///< With lowest action
      SEL_ACTION_MAX,      ///< With highest action
      SEL_CHB_MIN,         ///< With lowest CHB Q-score
      SEL_CHB_MAX,         ///< With highest CHB Q-score
      SEL_MIN_MIN,         ///< With smallest min
      SEL_MIN_MAX,         ///< With largest min
      SEL_MAX_MIN,         ///< With smallest max
      SEL_MAX_MAX,         ///< With largest max
      SEL_SIZE_MIN,        ///< With smallest domain size
      SEL_SIZE_MAX,        ///< With largest domain size
      SEL_DEGREE_SIZE_MIN, ///< With smallest degree divided by domain size
      SEL_DEGREE_SIZE_MAX, ///< With largest degree divided by domain size
      SEL_AFC_SIZE_MIN,    ///< With smallest accumulated failure count divided by domain size
      SEL_AFC_SIZE_MAX,    ///< With largest accumulated failure count divided by domain size
      SEL_ACTION_SIZE_MIN, ///< With smallest action divided by domain size
      SEL_ACTION_SIZE_MAX, ///< With largest action divided by domain size
      SEL_CHB_SIZE_MIN,    ///< With smallest CHB Q-score divided by domain size
      SEL_CHB_SIZE_MAX,    ///< With largest CHB Q-score divided by domain size
      /** \brief With smallest min-regret
       *
       * The min-regret of a variable is the difference between the
       * smallest and second-smallest value still in the domain.
       */
      SEL_REGRET_MIN_MIN,
      /** \brief With largest min-regret
       *
       * The min-regret of a variable is the difference between the
       * smallest and second-smallest value still in the domain.
       */
      SEL_REGRET_MIN_MAX,
      /** \brief With smallest max-regret
       *
       * The max-regret of a variable is the difference between the
       * largest and second-largest value still in the domain.
       */
      SEL_REGRET_MAX_MIN,
      /** \brief With largest max-regret
       *
       * The max-regret of a variable is the difference between the
       * largest and second-largest value still in the domain.
       */
      SEL_REGRET_MAX_MAX
    };
  protected:
    /// Which variable to select
    Select s;
  public:
    /// Initialize with strategy SEL_NONE
    IntVarBranch(void);
    /// Initialize with random number generator \a r
    IntVarBranch(Rnd r);
    /// Initialize with selection strategy \a s and tie-break limit function \a t
    IntVarBranch(Select s, BranchTbl t);
    /// Initialize with selection strategy \a s, decay factor \a d, and tie-break limit function \a t
    IntVarBranch(Select s, double d, BranchTbl t);
    /// Initialize with selection strategy \a s, AFC \a a, and tie-break limit function \a t
    IntVarBranch(Select s, IntAFC a, BranchTbl t);
    /// Initialize with selection strategy \a s, action \a a, and tie-break limit function \a t
    IntVarBranch(Select s, IntAction a, BranchTbl t);
    /// Initialize with selection strategy \a s, CHB \a c, and tie-break limit function \a t
    IntVarBranch(Select s, IntCHB c, BranchTbl t);
    /// Initialize with selection strategy \a s, branch merit function \a mf, and tie-break limit function \a t
    IntVarBranch(Select s, IntBranchMerit mf, BranchTbl t);
    /// Return selection strategy
    Select select(void) const;
    /// Expand AFC, action, and CHB
    void expand(Home home, const IntVarArgs& x);
  };

  /**
   * \brief Which Boolean variable to select for branching
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolVarBranch : public VarBranch<BoolVar> {
  public:
    /// Which variable selection
    enum Select {
      SEL_NONE = 0,        ///< First unassigned
      SEL_RND,             ///< Random (uniform, for tie breaking)
      SEL_MERIT_MIN,       ///< With least merit
      SEL_MERIT_MAX,       ///< With highest merit
      SEL_DEGREE_MIN,      ///< With smallest degree
      SEL_DEGREE_MAX,      ///< With largest degree
      SEL_AFC_MIN,         ///< With smallest accumulated failure count
      SEL_AFC_MAX,         ///< With largest accumulated failure count
      SEL_ACTION_MIN,      ///< With lowest action
      SEL_ACTION_MAX,      ///< With highest action
      SEL_CHB_MIN,         ///< With lowest CHB
      SEL_CHB_MAX          ///< With highest CHB
    };
  protected:
    /// Which variable to select
    Select s;
  public:
    /// Initialize with strategy SEL_NONE
    BoolVarBranch(void);
    /// Initialize with random number generator \a r
    BoolVarBranch(Rnd r);
    /// Initialize with selection strategy \a s and tie-break limit function \a t
    BoolVarBranch(Select s, BranchTbl t);
    /// Initialize with selection strategy \a s, decay factor \a d, and tie-break limit function \a t
    BoolVarBranch(Select s, double d, BranchTbl t);
    /// Initialize with selection strategy \a s, AFC \a a, and tie-break limit function \a t
    BoolVarBranch(Select s, BoolAFC a, BranchTbl t);
    /// Initialize with selection strategy \a s, action \a a, and tie-break limit function \a t
    BoolVarBranch(Select s, BoolAction a, BranchTbl t);
    /// Initialize with selection strategy \a s, CHB \a c, and tie-break limit function \a t
    BoolVarBranch(Select s, BoolCHB c, BranchTbl t);
    /// Initialize with selection strategy \a s, branch merit function \a mf, and tie-break limit function \a t
    BoolVarBranch(Select s, BoolBranchMerit mf, BranchTbl t);
    /// Return selection strategy
    Select select(void) const;
    /// Expand decay factor into AFC or action
    void expand(Home home, const BoolVarArgs& x);
  };

  /**
   * \defgroup TaskModelIntBranchVar Variable selection for integer and Boolean variables
   * \ingroup TaskModelIntBranch
   */
  //@{
  /// Select first unassigned variable
  IntVarBranch INT_VAR_NONE(void);
  /// Select random variable (uniform distribution, for tie breaking)
  IntVarBranch INT_VAR_RND(Rnd r);
  /// Select variable with least merit according to branch merit function \a bm
  IntVarBranch INT_VAR_MERIT_MIN(IntBranchMerit bm, BranchTbl tbl=nullptr);
  /// Select variable with highest merit according to branch merit function \a bm
  IntVarBranch INT_VAR_MERIT_MAX(IntBranchMerit bm, BranchTbl tbl=nullptr);
  /// Select variable with smallest degree
  IntVarBranch INT_VAR_DEGREE_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest degree
  IntVarBranch INT_VAR_DEGREE_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count with decay factor \a d
  IntVarBranch INT_VAR_AFC_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count
  IntVarBranch INT_VAR_AFC_MIN(IntAFC a, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count with decay factor \a d
  IntVarBranch INT_VAR_AFC_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count
  IntVarBranch INT_VAR_AFC_MAX(IntAFC a, BranchTbl tbl=nullptr);
  /// Select variable with lowest action with decay factor \a d
  IntVarBranch INT_VAR_ACTION_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with lowest action
  IntVarBranch INT_VAR_ACTION_MIN(IntAction a, BranchTbl tbl=nullptr);
  /// Select variable with highest action with decay factor \a d
  IntVarBranch INT_VAR_ACTION_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with highest action
  IntVarBranch INT_VAR_ACTION_MAX(IntAction a, BranchTbl tbl=nullptr);
  /// Select variable with lowest CHB Q-score
  IntVarBranch INT_VAR_CHB_MIN(IntCHB c, BranchTbl tbl=nullptr);
  /// Select variable with lowest CHB Q-score
  IntVarBranch INT_VAR_CHB_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score
  IntVarBranch INT_VAR_CHB_MAX(IntCHB c, BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score
  IntVarBranch INT_VAR_CHB_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest min
  IntVarBranch INT_VAR_MIN_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest min
  IntVarBranch INT_VAR_MIN_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest max
  IntVarBranch INT_VAR_MAX_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest max
  IntVarBranch INT_VAR_MAX_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest domain size
  IntVarBranch INT_VAR_SIZE_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest domain size
  IntVarBranch INT_VAR_SIZE_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest degree divided by domain size
  IntVarBranch INT_VAR_DEGREE_SIZE_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest degree divided by domain size
  IntVarBranch INT_VAR_DEGREE_SIZE_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count divided by domain size with decay factor \a d
  IntVarBranch INT_VAR_AFC_SIZE_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count divided by domain size
  IntVarBranch INT_VAR_AFC_SIZE_MIN(IntAFC a, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count divided by domain size with decay factor \a d
  IntVarBranch INT_VAR_AFC_SIZE_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count divided by domain size
  IntVarBranch INT_VAR_AFC_SIZE_MAX(IntAFC a, BranchTbl tbl=nullptr);
  /// Select variable with smallest action divided by domain size with decay factor \a d
  IntVarBranch INT_VAR_ACTION_SIZE_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with smallest action divided by domain size
  IntVarBranch INT_VAR_ACTION_SIZE_MIN(IntAction a, BranchTbl tbl=nullptr);
  /// Select variable with largest action divided by domain size with decay factor \a d
  IntVarBranch INT_VAR_ACTION_SIZE_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with largest action divided by domain size
  IntVarBranch INT_VAR_ACTION_SIZE_MAX(IntAction a, BranchTbl tbl=nullptr);
  /// Select variable with smallest CHB Q-score divided by domain size
  IntVarBranch INT_VAR_CHB_SIZE_MIN(IntCHB c, BranchTbl tbl=nullptr);
  /// Select variable with smallest CHB Q-score divided by domain size
  IntVarBranch INT_VAR_CHB_SIZE_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score divided by domain size
  IntVarBranch INT_VAR_CHB_SIZE_MAX(IntCHB c, BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score divided by domain size
  IntVarBranch INT_VAR_CHB_SIZE_MAX(BranchTbl tbl=nullptr);
  /** \brief Select variable with smallest min-regret
   *
   * The min-regret of a variable is the difference between the
   * smallest and second-smallest value still in the domain.
   */
  IntVarBranch INT_VAR_REGRET_MIN_MIN(BranchTbl tbl=nullptr);
  /** \brief Select variable with largest min-regret
   *
   * The min-regret of a variable is the difference between the
   * smallest and second-smallest value still in the domain.
   */
  IntVarBranch INT_VAR_REGRET_MIN_MAX(BranchTbl tbl=nullptr);
  /** \brief Select variable with smallest max-regret
   *
   * The max-regret of a variable is the difference between the
   * largest and second-largest value still in the domain.
   */
  IntVarBranch INT_VAR_REGRET_MAX_MIN(BranchTbl tbl=nullptr);
  /** \brief Select variable with largest max-regret
   *
   * The max-regret of a variable is the difference between the
   * largest and second-largest value still in the domain.
   */
  IntVarBranch INT_VAR_REGRET_MAX_MAX(BranchTbl tbl=nullptr);

  /// Select first unassigned variable
  BoolVarBranch BOOL_VAR_NONE(void);
  /// Select random variable (uniform distribution, for tie breaking)
  BoolVarBranch BOOL_VAR_RND(Rnd r);
  /// Select variable with least merit according to branch merit function \a bm
  BoolVarBranch BOOL_VAR_MERIT_MIN(BoolBranchMerit bm, BranchTbl tbl=nullptr);
  /// Select variable with highest merit according to branch merit function \a bm
  BoolVarBranch BOOL_VAR_MERIT_MAX(BoolBranchMerit bm, BranchTbl tbl=nullptr);
  /// Select variable with smallest degree
  BoolVarBranch BOOL_VAR_DEGREE_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest degree
  BoolVarBranch BOOL_VAR_DEGREE_MAX(BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count with decay factor \a d
  BoolVarBranch BOOL_VAR_AFC_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with smallest accumulated failure count
  BoolVarBranch BOOL_VAR_AFC_MIN(BoolAFC a, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count with decay factor \a d
  BoolVarBranch BOOL_VAR_AFC_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with largest accumulated failure count
  BoolVarBranch BOOL_VAR_AFC_MAX(BoolAFC a, BranchTbl tbl=nullptr);
  /// Select variable with lowest action with decay factor \a d
  BoolVarBranch BOOL_VAR_ACTION_MIN(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with lowest action
  BoolVarBranch BOOL_VAR_ACTION_MIN(BoolAction a, BranchTbl tbl=nullptr);
  /// Select variable with highest action with decay factor \a d
  BoolVarBranch BOOL_VAR_ACTION_MAX(double d=1.0, BranchTbl tbl=nullptr);
  /// Select variable with highest action
  BoolVarBranch BOOL_VAR_ACTION_MAX(BoolAction a, BranchTbl tbl=nullptr);
  /// Select variable with lowest CHB Q-score
  BoolVarBranch BOOL_VAR_CHB_MIN(BoolCHB c, BranchTbl tbl=nullptr);
  /// Select variable with lowest CHB Q-score
  BoolVarBranch BOOL_VAR_CHB_MIN(BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score
  BoolVarBranch BOOL_VAR_CHB_MAX(BoolCHB c, BranchTbl tbl=nullptr);
  /// Select variable with largest CHB Q-score
  BoolVarBranch BOOL_VAR_CHB_MAX(BranchTbl tbl=nullptr);
  //@}

}

#include <gecode/int/branch/var.hpp>

namespace Gecode {

  /**
   * \brief Which values to select for branching first
   *
   * \ingroup TaskModelIntBranch
   */
  class IntValBranch : public ValBranch<IntVar> {
  public:
    /// Which value selection
    enum Select {
      SEL_MIN,        ///< Select smallest value
      SEL_MED,        ///< Select greatest value not greater than the median
      SEL_MAX,        ///< Select largest value
      SEL_RND,        ///< Select random value
      SEL_SPLIT_MIN,  ///< Select values not greater than mean of smallest and largest value
      SEL_SPLIT_MAX,  ///< Select values greater than mean of smallest and largest value
      SEL_RANGE_MIN,  ///< Select the smallest range of the variable domain if it has several ranges, otherwise select values not greater than mean of smallest and largest value
      SEL_RANGE_MAX,  ///< Select the largest range of the variable domain if it has several ranges, otherwise select values greater than mean of smallest and largest value
      SEL_VAL_COMMIT, ///< Select value according to user-defined functions
      SEL_VALUES_MIN, ///< Select all values starting from smallest
      SEL_VALUES_MAX  ///< Select all values starting from largest
   };
  protected:
    /// Which value to select
    Select s;
  public:
    /// Initialize with selection strategy \a s
    IntValBranch(Select s = SEL_MIN);
    /// Initialize with random number generator \a r
    IntValBranch(Rnd r);
    /// Initialize with value function \a f and commit function \a c
    IntValBranch(IntBranchVal v, IntBranchCommit c);
    /// Return selection strategy
    Select select(void) const;
  };

  /**
   * \brief Which values to select for branching first
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolValBranch : public ValBranch<BoolVar> {
  public:
    /// Which value selection
    enum Select {
      SEL_MIN,       ///< Select smallest value
      SEL_MAX,       ///< Select largest value
      SEL_RND,       ///< Select random value
      SEL_VAL_COMMIT ///< Select value according to user-defined functions
   };
  protected:
    /// Which value to select
    Select s;
  public:
    /// Initialize with selection strategy \a s
    BoolValBranch(Select s = SEL_MIN);
    /// Initialize with random number generator \a r
    BoolValBranch(Rnd r);
    /// Initialize with value function \a f and commit function \a c
    BoolValBranch(BoolBranchVal v, BoolBranchCommit c);
    /// Return selection strategy
    Select select(void) const;
  };

  /**
   * \defgroup TaskModelIntBranchVal Value selection for integer and Boolean variables
   * \ingroup TaskModelIntBranch
   */
  //@{
  /// Select smallest value
  IntValBranch INT_VAL_MIN(void);
  /// Select greatest value not greater than the median
  IntValBranch INT_VAL_MED(void);
  /// Select largest value
  IntValBranch INT_VAL_MAX(void);
  /// Select random value
  IntValBranch INT_VAL_RND(Rnd r);
  /// Select values not greater than mean of smallest and largest value
  IntValBranch INT_VAL_SPLIT_MIN(void);
  /// Select values greater than mean of smallest and largest value
  IntValBranch INT_VAL_SPLIT_MAX(void);
  /// Select the smallest range of the variable domain if it has several ranges, otherwise select values not greater than mean of smallest and largest value
  IntValBranch INT_VAL_RANGE_MIN(void);
  /// Select the largest range of the variable domain if it has several ranges, otherwise select values greater than mean of smallest and largest value
  IntValBranch INT_VAL_RANGE_MAX(void);
  /**
   * \brief Select value as defined by the value function \a v and commit function \a c
   * Uses a commit function as default that posts the constraints that
   * a variable \a x must be equal to a value \a n for the first alternative
   * and that \a x must be different from \a n for the second alternative.
   */
  IntValBranch INT_VAL(IntBranchVal v, IntBranchCommit c=nullptr);
  /// Try all values starting from smallest
  IntValBranch INT_VALUES_MIN(void);
  /// Try all values starting from largest
  IntValBranch INT_VALUES_MAX(void);

  /// Select smallest value
  BoolValBranch BOOL_VAL_MIN(void);
  /// Select largest value
  BoolValBranch BOOL_VAL_MAX(void);
  /// Select random value
  BoolValBranch BOOL_VAL_RND(Rnd r);
  /**
   * \brief Select value as defined by the value function \a v and commit function \a c
   * Uses a commit function as default that posts the constraints that
   * a variable \a x must be equal to a value \a n for the first alternative
   * and that \a x must be different from \a n for the second alternative.
   */
  BoolValBranch BOOL_VAL(BoolBranchVal v, BoolBranchCommit c=nullptr);
  //@}

}

#include <gecode/int/branch/val.hpp>

namespace Gecode {

  /**
   * \brief Which values to select for assignment
   *
   * \ingroup TaskModelIntBranch
   */
  class IntAssign : public ValBranch<IntVar> {
  public:
    /// Which value selection
    enum Select {
      SEL_MIN,       ///< Select smallest value
      SEL_MED,       ///< Select greatest value not greater than the median
      SEL_MAX,       ///< Select largest value
      SEL_RND,       ///< Select random value
      SEL_VAL_COMMIT ///< Select value according to user-defined functions
    };
  protected:
    /// Which value to select
    Select s;
  public:
    /// Initialize with selection strategy \a s
    IntAssign(Select s = SEL_MIN);
    /// Initialize with random number generator \a r
    IntAssign(Rnd r);
    /// Initialize with value function \a f and commit function \a c
    IntAssign(IntBranchVal v, IntBranchCommit c);
    /// Return selection strategy
    Select select(void) const;
  };

  /**
   * \brief Which values to select for assignment
   *
   * \ingroup TaskModelIntBranch
   */
  class BoolAssign : public ValBranch<BoolVar> {
  public:
    /// Which value selection
    enum Select {
      SEL_MIN,       ///< Select smallest value
      SEL_MAX,       ///< Select largest value
      SEL_RND,       ///< Select random value
      SEL_VAL_COMMIT ///< Select value according to user-defined functions
    };
  protected:
    /// Which value to select
    Select s;
  public:
    /// Initialize with selection strategy \a s
    BoolAssign(Select s = SEL_MIN);
    /// Initialize with random number generator \a r
    BoolAssign(Rnd r);
    /// Initialize with value function \a f and commit function \a c
    BoolAssign(BoolBranchVal v, BoolBranchCommit c);
    /// Return selection strategy
    Select select(void) const;
  };

  /**
   * \defgroup TaskModelIntBranchAssign Value selection for assigning integer variables
   * \ingroup TaskModelIntBranch
   */
  //@{
  /// Select smallest value
  IntAssign INT_ASSIGN_MIN(void);
  /// Select greatest value not greater than the median
  IntAssign INT_ASSIGN_MED(void);
  /// Select largest value
  IntAssign INT_ASSIGN_MAX(void);
  /// Select random value
  IntAssign INT_ASSIGN_RND(Rnd r);
  /**
   * \brief Select value as defined by the value function \a v and commit function \a c
   *
   * Uses a commit function as default that posts the constraint that
   * a variable \a x must be equal to the value \a n.
   */
  IntAssign INT_ASSIGN(IntBranchVal v, IntBranchCommit c=nullptr);

  /// Select smallest value
  BoolAssign BOOL_ASSIGN_MIN(void);
  /// Select largest value
  BoolAssign BOOL_ASSIGN_MAX(void);
  /// Select random value
  BoolAssign BOOL_ASSIGN_RND(Rnd r);
  /**
   * \brief Select value as defined by the value function \a v and commit function \a c
   *
   * Uses a commit function as default that posts the constraint that
   * a variable \a x must be equal to the value \a n.
   */
  BoolAssign BOOL_ASSIGN(BoolBranchVal v, BoolBranchCommit c=nullptr);
  //@}

}

#include <gecode/int/branch/assign.hpp>

namespace Gecode {

  /**
   * \brief Branch over \a x with variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const IntVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const IntVarArgs& x,
         TieBreak<IntVarBranch> vars, IntValBranch vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, IntVar x, IntValBranch vals,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const BoolVarArgs& x,
         BoolVarBranch vars, BoolValBranch vals,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const BoolVarArgs& x,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, BoolVar x, BoolValBranch vals,
         BoolVarValPrint vvp=nullptr);

  /**
   * \brief Assign all \a x with variable selection \a vars with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, const IntVarArgs& x,
         IntVarBranch vars, IntAssign vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Assign all \a x with tie-breaking variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, const IntVarArgs& x,
         TieBreak<IntVarBranch> vars, IntAssign vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Assign \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, IntVar x, IntAssign vals,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Assign all \a x with variable selection \a vars with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, const BoolVarArgs& x,
         BoolVarBranch vars, BoolAssign vals,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);
  /**
   * \brief Assign all \a x with tie-breaking variable selection \a vars and value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, const BoolVarArgs& x,
         TieBreak<BoolVarBranch> vars, BoolAssign vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Assign \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  assign(Home home, BoolVar x, BoolAssign vals,
         BoolVarValPrint vvp=nullptr);

}

namespace Gecode {

  /**
   * \brief Branch over \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  void
  branch(Home home, const IntVarArgs& x, IntValBranch vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  void
  branch(Home home, const BoolVarArgs& x, BoolValBranch vals,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);

  /**
   * \brief Assign all \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  void
  assign(Home home, const IntVarArgs& x, IntAssign vals,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Assign all \a x with value selection \a vals
   *
   * \ingroup TaskModelIntBranch
   */
  void
  assign(Home home, const BoolVarArgs& x, BoolAssign vals,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);

}

#include <gecode/int/branch.hpp>

namespace Gecode {

  /** Print DFA \a d
   * \relates Gecode::DFA
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const DFA& d);

  /** Print TupleSet \a ts
   * \relates Gecode::TupleSet
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const TupleSet& ts);

}

// LDSB-related declarations.
namespace Gecode {

  namespace Int { namespace LDSB {
    class SymmetryObject;
  }}

  /**
   * \brief A reference-counted pointer to a SymmetryObject
   *
   * \ingroup TaskModelIntBranch
   */
  class GECODE_INT_EXPORT SymmetryHandle {
  public:
    /// Symmetry object that this handle refers to.
    Int::LDSB::SymmetryObject* ref;
    /// Increment counter
    void increment(void);
    /// Decrement counter
    void decrement(void);
  public:
    /// Default constructor
    SymmetryHandle(void);
    /// Initialies with a SymmetryObject
    SymmetryHandle(Int::LDSB::SymmetryObject* o);
    /// Copy constructor
    SymmetryHandle(const SymmetryHandle& h);
    /// Assignment operator
    const SymmetryHandle& operator=(const SymmetryHandle& h);
    /// Destructor
    ~SymmetryHandle(void);
  };
  class Symmetries;
  /// Traits of %Symmetries
  template<>
  class ArrayTraits<ArgArray<SymmetryHandle> > {
  public:
    typedef Symmetries     StorageType;
    typedef SymmetryHandle ValueType;
    typedef Symmetries     ArgsType;
  };

  /**
   * \defgroup TaskModelIntBranchSymm Symmetry declarations
   *
   * \ingroup TaskModelIntBranch
   */
  //@{
  /// Collection of symmetries
  class Symmetries : public ArgArray<SymmetryHandle> {};
  // If this is instead a typedef, strange things happen with the
  // overloading of the "branch" function.

  /// Variables in \a x are interchangeable
  GECODE_INT_EXPORT SymmetryHandle VariableSymmetry(const IntVarArgs& x);
  /// Variables in \a x are interchangeable
  GECODE_INT_EXPORT SymmetryHandle VariableSymmetry(const BoolVarArgs& x);
  /// Specified variables in \a x are interchangeable
  GECODE_INT_EXPORT SymmetryHandle VariableSymmetry(const IntVarArgs& x,
                                                    const IntArgs& indices);
  /// Values in \a v are interchangeable
  GECODE_INT_EXPORT SymmetryHandle ValueSymmetry(const IntArgs& v);
  /// Values in \a v are interchangeable
  GECODE_INT_EXPORT SymmetryHandle ValueSymmetry(const IntSet& v);
  /// All values in the domain of the given variable are interchangeable
  GECODE_INT_EXPORT SymmetryHandle ValueSymmetry(IntVar vars);
  /**
   * \brief Variable sequences in \a x of size \a ss are interchangeable
   *
   * The size of \a x must be a multiple of \a ss.
   */
  GECODE_INT_EXPORT
  SymmetryHandle VariableSequenceSymmetry(const IntVarArgs& x, int ss);
  /**
   * \brief Variable sequences in \a x of size \a ss are interchangeable
   *
   * The size of \a x must be a multiple of \a ss.
   */
  GECODE_INT_EXPORT
  SymmetryHandle VariableSequenceSymmetry(const BoolVarArgs& x, int ss);
  /**
   * \brief Value sequences in \a v of size \a ss are interchangeable
   *
   * The size of \a v must be a multiple of \a ss.
   */
  GECODE_INT_EXPORT
  SymmetryHandle ValueSequenceSymmetry(const IntArgs& v, int ss);

  /// The values from \a lower to \a upper (inclusive) can be reflected
  GECODE_INT_EXPORT SymmetryHandle values_reflect(int lower, int upper);
  /// The values in the domain of \x can be reflected
  GECODE_INT_EXPORT SymmetryHandle values_reflect(IntVar x);
  //@}

  /**
   * \brief Branch over \a x with variable selection \a vars and value
   * selection \a vals with symmetry breaking
   *
   * Throws LDSBBadValueSelection exception if \a vals is any of
   * SEL_SPLIT_MIN, SEL_SPLIT_MAX, SEL_RANGE_MIN, SEL_RANGE_MAX,
   * SEL_VALUES_MIN, and SEL_VALUES_MAX, or if \a vals is
   * SEL_VAL_COMMIT with a custom commit function.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const IntVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const Symmetries& syms,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with tie-breaking variable selection \a
   * vars and value selection \a vals with symmetry breaking
   *
   * Throws LDSBBadValueSelection exception if \a vals is any of
   * SEL_SPLIT_MIN, SEL_SPLIT_MAX, SEL_RANGE_MIN, SEL_RANGE_MAX,
   * SEL_VALUES_MIN, and SEL_VALUES_MAX, or if \a vals is
   * SEL_VAL_COMMIT with a custom commit function.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const IntVarArgs& x,
         TieBreak<IntVarBranch> vars, IntValBranch vals,
         const Symmetries& syms,
         IntBranchFilter bf=nullptr,
         IntVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with variable selection \a vars and value
   * selection \a vals with symmetry breaking
   *
   * Throws LDSBBadValueSelection exception if \a vals is any of
   * SEL_SPLIT_MIN, SEL_SPLIT_MAX, SEL_RANGE_MIN, SEL_RANGE_MAX,
   * SEL_VALUES_MIN, and SEL_VALUES_MAX, or if \a vals is
   * SEL_VAL_COMMIT with a custom commit function.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const BoolVarArgs& x,
         BoolVarBranch vars, BoolValBranch vals,
         const Symmetries& syms,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);
  /**
   * \brief Branch over \a x with tie-breaking variable selection \a
   * vars and value selection \a vals with symmetry breaking
   *
   * Throws LDSBBadValueSelection exception if \a vals is any of
   * SEL_SPLIT_MIN, SEL_SPLIT_MAX, SEL_RANGE_MIN, SEL_RANGE_MAX,
   * SEL_VALUES_MIN, and SEL_VALUES_MAX, or if \a vals is
   * SEL_VAL_COMMIT with a custom commit function.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  branch(Home home, const BoolVarArgs& x,
         TieBreak<BoolVarBranch> vars, BoolValBranch vals,
         const Symmetries& syms,
         BoolBranchFilter bf=nullptr,
         BoolVarValPrint vvp=nullptr);

#ifdef GECODE_HAS_CBS

  /**
   * \brief Branch over \a x using counting-based search
   *
   * Branches on the <variable, value> pair that has the the highest solution
   * density across all active propagators. Computing solution density is
   * currently supported for the following propagators:
   *
   *   - Gecode::Int::Distinct::Val
   *   - Gecode::Int::Distinct::Bnd
   *   - Gecode::Int::Distinct::Dom
   *   - More to come...
   *
   * If the space does not contain any of the preceding propagators, this
   * brancher won't be able to make any branching choices. For more details,
   * please see the documentation in MPG, section ?.
   *
   * To use this brancher, Gecode needs to be compiled with --enable-cbs.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  cbsbranch(Home home, const IntVarArgs& x);


  /**
   * \brief Branch over \a x using counting-based search
   *
   * Branches on the <variable, value> pair that has the the highest solution
   * density across all active propagators. Computing solution density is
   * currently supported for the following propagators:
   *
   *   - Gecode::Int::Distinct::Val
   *   - Gecode::Int::Distinct::Bnd
   *   - Gecode::Int::Distinct::Dom
   *   - More to come...
   *
   * If the space does not contain any of the preceding propagators, this
   * brancher won't be able to make any branching choices. For more details,
   * please see the documentation in MPG, section ?.
   *
   * To use this brancher, Gecode needs to be compiled with --enable-cbs.
   *
   * \ingroup TaskModelIntBranch
   */
  GECODE_INT_EXPORT void
  cbsbranch(Home home, const BoolVarArgs& x);

#endif

}

namespace Gecode {

  /*
   * \brief Relaxed assignment of variables in \a x from values in \a sx
   *
   * The variables in \a x are assigned values from the assigned variables
   * in the solution \a sx with a relaxation probability \a p. That is,
   * if \$fp=0.1\f$ approximately 10% of the variables in \a x will be
   * assigned a value from \a sx.
   *
   * The random numbers are generated from the generator \a r. At least
   * one variable will not be assigned: in case the relaxation attempt
   * would suggest that all variables should be assigned, a single
   * variable will be selected randomly to remain unassigned.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a x and
   * \a sx are of different size.
   *
   * Throws an exception of type Int::OutOfLimits, if \a p is not between
   * \a 0.0 and \a 1.0.
   *
   * \ingroup TaskModelInt
   */
  GECODE_INT_EXPORT void
  relax(Home home, const IntVarArgs& x, const IntVarArgs& sx,
        Rnd r, double p);

  /*
   * \brief Relaxed assignment of variables in \a x from values in \a sx
   *
   * The variables in \a x are assigned values from the assigned variables
   * in the solution \a sx with a relaxation probability \a p. That is,
   * if \$fp=0.1\f$ approximately 10% of the variables in \a x will be
   * assigned a value from \a sx.
   *
   * The random numbers are generated from the generator \a r. At least
   * one variable will not be assigned: in case the relaxation attempt
   * would suggest that all variables should be assigned, a single
   * variable will be selected randomly to remain unassigned.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if \a x and
   * \a sx are of different size.
   *
   * Throws an exception of type Int::OutOfLimits, if \a p is not between
   * \a 0.0 and \a 1.0.
   *
   * \ingroup TaskModelInt
   */
  GECODE_INT_EXPORT void
  relax(Home home, const BoolVarArgs& x, const BoolVarArgs& sx,
        Rnd r, double p);

}


#include <gecode/int/trace/int-trace-view.hpp>
#include <gecode/int/trace/bool-trace-view.hpp>

namespace Gecode {

  /**
   * \defgroup TaskIntTrace Tracing for integer and Boolean variables
   * \ingroup TaskTrace
   */

  /**
   * \brief Trace delta information for integer variables
   * \ingroup TaskIntTrace
   */
  class IntTraceDelta
    : public Iter::Ranges::Diff<Iter::Ranges::RangeList,
                                Int::ViewRanges<Int::IntView> > {
  protected:
    /// Iterator over the new values
    Int::ViewRanges<Int::IntView> rn;
    /// Iterator over the old values
    Iter::Ranges::RangeList ro;
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize with old trace view \a o, new view \a n, and delta \a d
    IntTraceDelta(Int::IntTraceView o, Int::IntView n, const Delta& d);
    //@}
  };

  /**
   * \brief Trace delta information for Boolean variables
   * \ingroup TaskIntTrace
   */
  class BoolTraceDelta {
  protected:
    /// Delta information
    int delta;
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize with old trace view \a o, new view \a n, and delta \a d
    BoolTraceDelta(Int::BoolTraceView o, Int::BoolView n, const Delta& d);
    //@}
    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a range or done
    bool operator ()(void) const;
    /// Move iterator to next range (if possible)
    void operator ++(void);
    //@}

    /// \name Range access
    //@{
    /// Return smallest value of range
    int min(void) const;
    /// Return largest value of range
    int max(void) const;
    /// Return width of range (distance between minimum and maximum)
    unsigned int width(void) const;
    //@}
  };

}

#include <gecode/int/trace/int-delta.hpp>
#include <gecode/int/trace/bool-delta.hpp>

#include <gecode/int/trace/traits.hpp>

namespace Gecode {

  /**
   * \brief Tracer for integer variables
   * \ingroup TaskIntTrace
   */
  typedef ViewTracer<Int::IntView> IntTracer;
  /**
   * \brief Trace recorder for integer variables
   * \ingroup TaskIntTrace
   */
  typedef ViewTraceRecorder<Int::IntView> IntTraceRecorder;

  /**
   * \brief Standard integer variable tracer
   * \ingroup TaskIntTrace
   */
  class GECODE_INT_EXPORT StdIntTracer : public IntTracer {
  protected:
    /// Output stream to use
    std::ostream& os;
  public:
    /// Initialize with output stream \a os0 and events \ e
    StdIntTracer(std::ostream& os0 = std::cerr);
    /// Print init information
    virtual void init(const Space& home, const IntTraceRecorder& t);
    /// Print prune information
    virtual void prune(const Space& home, const IntTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, IntTraceDelta& d);
    /// Print fixpoint information
    virtual void fix(const Space& home, const IntTraceRecorder& t);
    /// Print failure information
    virtual void fail(const Space& home, const IntTraceRecorder& t);
    /// Print that trace recorder is done
    virtual void done(const Space& home, const IntTraceRecorder& t);
    /// Default tracer (printing to std::cerr)
    static StdIntTracer def;
  };


  /**
   * \brief Tracer for Boolean variables
   * \ingroup TaskIntTrace
   */
  typedef ViewTracer<Int::BoolView> BoolTracer;
  /**
   * \brief Trace recorder for Boolean variables
   * \ingroup TaskIntTrace
   */
  typedef ViewTraceRecorder<Int::BoolView> BoolTraceRecorder;

  /**
   * \brief Standard Boolean variable tracer
   * \ingroup TaskIntTrace
   */
  class GECODE_INT_EXPORT StdBoolTracer : public BoolTracer {
  protected:
    /// Output stream to use
    std::ostream& os;
  public:
    /// Initialize with output stream \a os0
    StdBoolTracer(std::ostream& os0 = std::cerr);
    /// Print init information
    virtual void init(const Space& home, const BoolTraceRecorder& t);
    /// Print prune information
    virtual void prune(const Space& home, const BoolTraceRecorder& t,
                       const ViewTraceInfo& vti, int i, BoolTraceDelta& d);
    /// Print fixpoint information
    virtual void fix(const Space& home, const BoolTraceRecorder& t);
    /// Print failure information
    virtual void fail(const Space& home, const BoolTraceRecorder& t);
    /// Print that trace recorder is done
    virtual void done(const Space& home, const BoolTraceRecorder& t);
    /// Default tracer (printing to std::cerr)
    static StdBoolTracer def;
  };

  /**
   * \brief Create a tracer for integer variables
   * \ingroup TaskIntTrace
   */
  GECODE_INT_EXPORT void
  trace(Home home, const IntVarArgs& x,
        TraceFilter tf,
        int te = (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        IntTracer& t = StdIntTracer::def);
  /**
   * \brief Create a tracer for integer variables
   * \ingroup TaskIntTrace
   */
  void
  trace(Home home, const IntVarArgs& x,
        int te = (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        IntTracer& t = StdIntTracer::def);

  /**
   * \brief Create a tracer for Boolean Variables
   * \ingroup TaskIntTrace
   */
  GECODE_INT_EXPORT void
  trace(Home home, const BoolVarArgs& x,
        TraceFilter tf,
        int te = (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        BoolTracer& t = StdBoolTracer::def);
  /**
   * \brief Create a tracer for Boolean Variables
   * \ingroup TaskIntTrace
   */
  void
  trace(Home home, const BoolVarArgs& x,
        int te = (TE_INIT | TE_PRUNE | TE_FIX | TE_FAIL | TE_DONE),
        BoolTracer& t = StdBoolTracer::def);

}

#include <gecode/int/trace.hpp>

#endif

// IFDEF: GECODE_HAS_INT_VARS
// STATISTICS: int-post

