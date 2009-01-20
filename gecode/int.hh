/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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
#include <iostream>

#include <gecode/kernel.hh>
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
 * to program propagators and branchings for finite domain integers.
 * In addition, all propagators and branchings for finite domain
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
    const int max =  INT_MAX - 1;
    /// Smallest allowed integer value
    const int min = -max;
    /// Check whether integer \a n is in range, otherwise throw out of limits with information \a l
    void check(int n, const char* l);
    /// Check whether double \a n is in integer range, otherwise throw out of limits exception with information \a l
    void check(double n, const char* l);
  }

}}

#include <gecode/int/limits.hpp>

namespace Gecode {

  class IntSetRanges;

  /**
   * \brief Integer sets
   *
   * Integer sets are the means to specify arbitrary sets
   * of integers to be used as domains for integer variables.
   * \ingroup TaskModelIntVars TaskModelSetVars
   */
  class IntSet : public SharedHandle {
    friend class IntSetRanges;
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
      /// Return copy of object
      GECODE_INT_EXPORT SharedHandle::Object* copy(void) const;
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
    IntSet(int n, int m);
    /// Initialize with \a n integers from array \a r
    IntSet(const int r[],   int n);
    /** \brief Initialize with \a n ranges from array \a r
     *
     * For position \a i in the array \a r, the minimum is \a r[\a i][0]
     * and the maximum is \a r[\a i][1].
     */
    IntSet(const int r[][2], int n);
    /// Initialize with range iterator \a i
    template <class I>
    explicit IntSet(I& i);
#ifdef __INTEL_COMPILER
    /// Initialize with integer set \a s
    IntSet(const IntSet& s);
    /// Initialize with integer set \a s
    IntSet(IntSet& s);
#endif
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
    /// Return size (cardinality) of set
    unsigned int size(void) const;
    /// Return width of set (distance between maximum and minimum)
    unsigned int width(void) const;
    /// Return minimum of entire set
    int min(void) const;
    /// Return maximum of entire set
    int max(void) const;
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
  class IntVar : public VarBase<Int::IntVarImp> {
    friend class IntVarArray;
  private:
    using VarBase<Int::IntVarImp>::varimp;
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
    /// Initialize from integer variable \a x
    IntVar(const IntVar& x);
    /// Initialize from integer view \a x
    IntVar(const Int::IntView& x);
    /// Initialize from reflection variable \a x
    IntVar(const Reflection::Var& x);
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
    GECODE_INT_EXPORT IntVar(Space& home, int min ,int max);
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
    GECODE_INT_EXPORT void init(Space& home, int min, int max);
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
    GECODE_INT_EXPORT void init(Space& home, const IntSet& d);
    //@}

    /// \name Value access
    //@{
    /// Return minimum of domain
    int min(void) const;
    /// Return maximum of domain
    int max(void) const;
    /// Return median of domain (rounding downwards)
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
    //@}

    /// \name Domain tests
    //@{
    /// Test whether domain is a range
    bool range(void) const;
    /// Test whether view is assigned
    bool assigned(void) const;

    /// Test whether \a n is contained in domain
    bool in(int n) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this variable to be a clone of variable \a x
    void update(Space& home, bool share, IntVar& x);
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
  class BoolVar : public VarBase<Int::BoolVarImp> {
    friend class BoolVarArray;
  private:
    using VarBase<Int::BoolVarImp>::varimp;
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
    /// Initialize from Boolean variable \a x
    BoolVar(const BoolVar& x);
    /// Initialize from Boolean view \a x
    BoolVar(const Int::BoolView& x);
    /// Initialize from reflection variable \a x
    BoolVar(const Reflection::Var& x);
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
    /**
     * \brief Initialize Boolean variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. The following exceptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Int::VariableEmptyDomain is thrown.
     *  - If \a min is less than 0 or \a max is greater than 1,
     *    an exception of type
     *    Gecode::Int::NotZerOne is thrown.
     */
    GECODE_INT_EXPORT void init(Space& home, int min, int max);
    //@}

    /// \name Value access
    //@{
    /// Return minimum of domain
    int min(void) const;
    /// Return maximum of domain
    int max(void) const;
    /// Return median of domain
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
    //@}

    /// \name Domain tests
    //@{
    /// Test whether domain is a range
    bool range(void) const;
    /// Test whether view is assigned
    bool assigned(void) const;

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

    /// \name Cloning
    //@{
    /// Update this variable to be a clone of variable \a x
    void update(Space& home, bool share, BoolVar& x);
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
  /// Passing integer arguments
  typedef PrimArgArray<int>    IntArgs;
  /// Passing integer variables
  typedef VarArgArray<IntVar>  IntVarArgs;
  /// Passing Boolean variables
  typedef VarArgArray<BoolVar> BoolVarArgs;
  /// Passing set arguments
  typedef PrimArgArray<IntSet> IntSetArgs;
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
   * \brief Consistency levels for integer propagators
   *
   * The descriptions are meant to be suggestions. It is not
   * required that a propagator achieves full domain consistency or
   * full bounds consistency. It is more like: which level
   * of consistency comes closest.
   *
   * If in the description of a constraint below no consistency level
   * is mentioned, the propagator for the constraint implements
   * domain consistency.
   * \ingroup TaskModelInt
   */
  enum IntConLevel {
    ICL_VAL, ///< Value consistency (naive)
    ICL_BND, ///< Bounds consistency
    ICL_DOM, ///< Domain consistency
    ICL_DEF  ///< The default consistency for a constraint
  };



  /**
   * \defgroup TaskModelIntDomain Domain constraints
   * \ingroup TaskModelInt
   *
   */

  //@{
  /// Propagates \f$ x=n\f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, int n,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x_i=n\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Space& home, const IntVarArgs& x, int n,
      IntConLevel icl=ICL_DEF);

  /// Propagates \f$ l\leq x\leq m\f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, int l, int m,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ l\leq x_i\leq m\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Space& home, const IntVarArgs& x, int l, int m,
      IntConLevel icl=ICL_DEF);

  /// Propagates \f$ x\in s \f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, const IntSet& s,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x_i\in s\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Space& home, const IntVarArgs& x, const IntSet& s,
      IntConLevel icl=ICL_DEF);

  /// Post propagator for \f$ (x=n) \Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, int n, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ (l\leq x \leq m) \Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, int l, int m, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ (x \in s) \Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  dom(Space& home, IntVar x, const IntSet& s, BoolVar b,
      IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntRelInt Simple relation constraints over integer variables
   * \ingroup TaskModelInt
   */
  //@{
  /** \brief Post propagator for \f$ x_0 \sim_r x_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space& home, IntVar x0, IntRelType r, IntVar x1,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for \f$ x_i \sim_r y \f$ for all \f$0\leq i<|x|\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const IntVarArgs& x, IntRelType r, IntVar y,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x \sim_r c\f$
  GECODE_INT_EXPORT void
  rel(Space& home, IntVar x, IntRelType r, int c,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x_i \sim_r c \f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  rel(Space& home, const IntVarArgs& x, IntRelType r, int c,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ (x_0 \sim_r x_1)\Leftrightarrow b\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space& home, IntVar x0, IntRelType r, IntVar x1, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$(x \sim_r c)\Leftrightarrow b\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space& home, IntVar x, IntRelType r, int c, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for pairwise relation on \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal.
   *    Supports both bounds (\a icl = ICL_BND) and
   *    domain consistency (\a icl = ICL_DOM, default).
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ,
   *    then the elements of \a x are ordered with respt to \a r.
   *    Supports domain consistency (\a icl = ICL_DOM, default).
   *  - if \a r = IRT_NQ, then all elements of \a x must be pairwise
   *    distinct (corresponds to the distinct constraint).
   *    Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *    and domain consistency (\a icl = ICL_DOM).
   *    Throws an exception of type Int::ArgumentSame, if \a x contains
   *    the same unassigned variable multiply.
   *
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const IntVarArgs& x, IntRelType r,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntRelBool Simple relation constraints over Boolean variables
   * \ingroup TaskModelInt
   */
  //@{
  /// Post propagator for \f$ x_0 \sim_r x_1\f$
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x0, IntRelType r, BoolVar x1,
      IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$(x_0 \sim_r x_1)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x0, IntRelType r, BoolVar x1, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_i \sim_r y \f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  rel(Space& home, const BoolVarArgs& x, IntRelType r, BoolVar y,
      IntConLevel icl=ICL_DEF);
  /**
   * \brief Propagates \f$ x \sim_r n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x, IntRelType r, int n,
      IntConLevel icl=ICL_DEF);
  /**
   * \brief Propagates \f$(x \sim_r n)\Leftrightarrow b\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x, IntRelType r, int n, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /**
   * \brief Propagates \f$ x_i \sim_r n \f$ for all \f$0\leq i<|x|\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const BoolVarArgs& x, IntRelType r, int n,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for pairwise relation on \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal.
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ,
   *    then the elements of \a x are ordered with respt to \a r.
   *  - if \a r = IRT_NQ, then all elements of \a x must be pairwise
   *    distinct (corresponds to the distinct constraint).
   *
   */
  GECODE_INT_EXPORT void
  rel(Space& home, const BoolVarArgs& x, IntRelType r,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean operation on \a x0 and \a x1
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} x_1 = x_2\f$
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean operation on \a x0 and \a x1
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} x_1 = n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolVar x0, BoolOpType o, BoolVar x1, int n,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean operation on \a x
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1}= y\f$
   *
   * Throws an exception of type Int::TooFewArguments, if \f$|x|<2\f$
   * and \a o is BOT_IMP, BOT_EQV, or BOT_XOR.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolOpType o, const BoolVarArgs& x, BoolVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean operation on \a x
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1}= n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   *
   * Throws an exception of type Int::TooFewArguments, if \f$|x|<2\f$
   * and \a o is BOT_IMP, BOT_EQV, or BOT_XOR.
   */
  GECODE_INT_EXPORT void
  rel(Space& home, BoolOpType o, const BoolVarArgs& x, int n,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= z\f$
   *
   * Throws an exception of type Int::IllegalBoolOp, if \a o is different
   * from BOT_AND or BOT_OR.
   */
  GECODE_INT_EXPORT void
  clause(Space& home, BoolOpType o, const BoolVarArgs& x, const BoolVarArgs& y,
         BoolVar z, IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   *
   * Throws an exception of type Int::IllegalBoolOp, if \a o is different
   * from BOT_AND or BOT_OR.
   */
  GECODE_INT_EXPORT void
  clause(Space& home, BoolOpType o, const BoolVarArgs& x, const BoolVarArgs& y,
         int n, IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntElement Element constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Space& home, const IntArgs& n, IntVar x0, IntVar x1,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Space& home, const IntArgs& n, IntVar x0, BoolVar x1,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::OutOfLimits, if
   *  the integers in \a n exceed the limits in Int::Limits.
   */
  GECODE_INT_EXPORT void
  element(Space& home, const IntArgs& n, IntVar x0, int x1,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Space& home, const IntVarArgs& x, IntVar y0, IntVar y1,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Space& home, const IntVarArgs& x, IntVar y0, int y1,
          IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Space& home, const BoolVarArgs& x, IntVar y0, BoolVar y1,
          IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Space& home, const BoolVarArgs& x, IntVar y0, int y1,
          IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntDistinct Distinct constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   * and domain consistency (\a icl = ICL_DOM).
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Space& home, const IntVarArgs& x,
           IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ x_i+n_i\neq x_j+n_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * \li Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *     and domain consistency (\a icl = ICL_DOM).
   * \li Throws an exception of type Int::OutOfLimits, if
   *     the integers in \a n exceed the limits in Int::Limits
   *     or if the sum of \a n and \a x exceed the limits.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a n are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x contains
   *     the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Space& home, const IntArgs& n, const IntVarArgs& x,
           IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntChannel Channel constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i = j\leftrightarrow y_j=i\f$ for all \f$0\leq i<|x|\f$
   *
   * \li Supports domain (\a icl = ICL_DOM) and value propagation (all other
   *     values for \a icl).
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a y are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x or
   *     \a y contain the same unassigned variable multiply. Note that a
   *     variable can occur in both \a x and \a y, but not more than
   *     once in either \a x or \a y.
   */
  GECODE_INT_EXPORT void
  channel(Space& home, const IntVarArgs& x, const IntVarArgs& y,
          IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$ x_i - \mathit{xoff} = j\leftrightarrow y_j - \mathit{yoff} = i\f$ for all \f$0\leq i<|x|\f$
   *
   * \li Supports domain (\a icl = ICL_DOM) and value propagation (all other
   *     values for \a icl).
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a y are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x or
   *     \a y contain the same unassigned variable multiply. Note that a
   *     variable can occur in both \a x and \a y, but not more than
   *     once in either \a x or \a y.
   */
  GECODE_INT_EXPORT void
  channel(Space& home, const IntVarArgs& x, unsigned int xoff,
          const IntVarArgs& y, unsigned int yoff,
          IntConLevel icl=ICL_DEF);

  /// Post propagator for channeling a Boolean and an integer variable \f$ x_0 = x_1\f$
  GECODE_INT_EXPORT void
  channel(Space& home, BoolVar x0, IntVar x1,
          IntConLevel icl=ICL_DEF);
  /// Post propagator for channeling an integer and a Boolean variable \f$ x_0 = x_1\f$
  forceinline void
  channel(Space& home, IntVar x0, BoolVar x1,
          IntConLevel icl=ICL_DEF) {
    channel(home,x1,x0,icl);
  }
  /** \brief Post propagator for channeling Boolean and integer variables \f$ x_i = 1\leftrightarrow y=i+o\f$
   *
   * Throws an exception of type Int::ArgumentSame, if \a x
   * contains the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  channel(Space& home, const BoolVarArgs& x, IntVar y, int o=0,
          IntConLevel icl=ICL_DEF);
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
   * Supports domain (\a icl = ICL_DOM) and value propagation (all
   * other values for \a icl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws an exception of type Int::ArgumentSame, if \a x
   * contains the same unassigned variable multiply.
   */
  GECODE_INT_EXPORT void
  circuit(Space& home, const IntVarArgs& x,
          IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntScheduling Scheduling constraints
   * \ingroup TaskModelInt
   */
  //@{
  /**
   * \brief Post propagators for the cumulatives constraint.
   *
   * This function creates propagators for the cumulatives constraint
   * presented in <em> "A new multi-resource cumulatives constraint
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
   * The limit for a machine is either the maximum amount available at
   * any given time (\a at_most = true), or else the least amount to
   * be used (\a at_most = false).
   *
   * \param machine \f$ machine_i \f$ is the machine assigned to task \f$ i \f$
   * \param start \f$ start_i \f$ is the start date assigned to task \f$ i \f$
   * \param duration \f$ duration_i \f$ is the duration of task \f$ i \f$
   * \param end \f$ end_i \f$ is the end date assigned to task \f$ i \f$
   * \param height \f$ height_i \f$ is the height is the amount of
   *               resources consumed by task \f$ i \f$
   * \param limit \f$ limit_r \f$ is the amount of resource available
   *              for machine \f$ r \f$
   * \param at_most \a at_most tells if the amount of resources used
   *                for a machine should be less than the limit (\a at_most
   *                = true) or greater than the limit (\a at_most = false)
   * \param icl Supports value-consistency only (\a icl = ICL_VAL, default).
   *
   * \exception Int::ArgumentSizeMismatch thrown if the sizes of the arguments
   *            representing tasks does not match.
   * \exception Int::OutOfLimits thrown if any numerical argument is
   *            larger than Int::Limits::max or less than
   *            Int::Limits::min.
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space& home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most,
              IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntSorted Sorted constraints
   *
   * All sorted constraints support bounds consistency.
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
  sorted(Space& home, const IntVarArgs& x, const IntVarArgs& y,
         IntConLevel icl=ICL_DEF);

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
  sorted(Space&, const IntVarArgs& x, const IntVarArgs& y,
         const IntVarArgs& z,
         IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntCard Cardinality constraints
   * \ingroup TaskModelInt
   *
   *  \note
   *    Domain consistency on the extended cardinality variables of
   *    the Global Cardinality Propagator is only obtained if they are bounds
   *    consistent, otherwise the problem of enforcing domain consistency
   *    on the cardinality variables is NP-complete as proved by
   *    \ref CardVarNPCompl "Qumiper et. al. in
   *    Improved Algorithms for the Global Cardinality Constraint"
   */

  //@{
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\sim_r m\f$
   *
   * Supports domain consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, int n, IntRelType r, int m,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_r m\f$
   *
   * Supports domain consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, IntVar y, IntRelType r, int m,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\sim_r m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, const IntArgs& y, IntRelType r, int m,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\sim_r z\f$
   *
   * Supports domain consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, int n, IntRelType r, IntVar z,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_r z\f$
   *
   * Supports domain consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, IntVar y, IntRelType r, IntVar z,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\sim_r z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, const IntArgs& y, IntRelType r, IntVar z,
        IntConLevel icl=ICL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=j\}=c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \{0,\ldots,|c|-1\}\f$
    * (no other value occurs).
    *
    * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
    * and domain consistency (\a icl = ICL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, const IntVarArgs& c,
        IntConLevel icl=ICL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=j\}\in c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \{0,\ldots,|c|-1\}\f$
    * (no other value occurs).
    *
    * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
    * and domain consistency (\a icl = ICL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x, const IntSetArgs& c,
        IntConLevel icl=ICL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}=c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
    * and domain consistency (\a icl = ICL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a cards and \a values are of different size.
    */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x,
        const IntVarArgs& c, const IntArgs& v,
        IntConLevel icl=ICL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}\in c_j\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
    * and domain consistency (\a icl = ICL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a cards and \a values are of different size.
    */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x,
        const IntSetArgs& c, const IntArgs& v,
        IntConLevel icl=ICL_DEF);

  /** \brief Posts a global count (cardinality) constraint
    *
    * Posts the constraint that
    * \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=v_j\}\in c\f$ and
    * \f$ \bigcup_i \{x_i\} \subseteq \bigcup_j \{v_j\}\f$
    * (no other value occurs).
    *
    * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
    * and domain consistency (\a icl = ICL_DOM).
    *
    * Throws an exception of type Int::ArgumentSame, if \a x contains
    * the same unassigned variable multiply.
    *
    * Throws an exception of type Int::ArgumentSizeMismatch, if
    *  \a cards and \a values are of different size.
    */
  GECODE_INT_EXPORT void
  count(Space& home, const IntVarArgs& x,
        const IntSet& c, const IntArgs& v,
        IntConLevel icl=ICL_DEF);

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
  //@{

  /**
   * \brief Deterministic finite automaton (%DFA)
   *
   * After initialization, the start state is always zero.
   * The final states are contiguous ranging from the first to the
   * last final state.
   */
  class DFA : public SharedHandle {
  private:
    /// Implementation of DFA
    class DFAI;
  public:
    /// Specification of a %DFA transition
    class Transition {
    public:
      int i_state; ///< input state
      int symbol;  ///< symbol
      int o_state; ///< output state
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
    /// Initialize by DFA \a d (DFA is shared)
    DFA(const DFA& d);
    /// Return the number of states
    unsigned int n_states(void) const;
    /// Return the number of symbols
    unsigned int n_symbols(void) const;
    /// Return the number of transitions
    unsigned int n_transitions(void) const;
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

    /// Initialize DFA from reflection specification
    GECODE_INT_EXPORT
    DFA(Space& home, Reflection::VarMap& vm, Reflection::Arg* arg);
    /// Create reflection specification for the DFA
    GECODE_INT_EXPORT
    Reflection::Arg* spec(const Space& home,Reflection::VarMap& vm) const;
  };


  /**
   * \brief Extensional propagation kind
   *
   * Signals that a particular kind is used in propagation for
   * the implementation of a extensional constraint.
   *
   */
  enum ExtensionalPropKind {
    EPK_DEF,    ///< Make a default decision
    EPK_SPEED,  ///< Prefer speed over memory consumption
    EPK_MEMORY  ///< Prefer little memory over speed
  };

  /**
   * \brief Post propagator for extensional constraint described by a DFA
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply. If shared occurences of variables
   * are required, unshare should be used.
   */
  GECODE_INT_EXPORT void
  extensional(Space& home, const IntVarArgs& x, DFA d,
              IntConLevel icl=ICL_DEF);

  /**
   * \brief Post propagator for extensional constraint described by a DFA
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same unassigned variable multiply. If shared occurences of variables
   * are required, unshare should be used.
   */
  GECODE_INT_EXPORT void
  extensional(Space& home, const BoolVarArgs& x, DFA d,
              IntConLevel icl=ICL_DEF);

  /** \brief Class represeting a set of tuples.
   *
   * A TupleSet is used for storing an extensional representation of a
   * constraint. After a TupleSet is finalized, no more tuples may be
   * added to it.
   */
  class TupleSet : public SharedHandle {
  public:
    /** \brief Type of a tuple
     *
     * The arity of the tuple is left implicit.
     */
    typedef int* Tuple;

    /// Class for table data
    class TupleSetI;
    /// Get implementation
    TupleSetI* implementation(void);

    /// Construct empty tuple
    TupleSet(void);
    /// Initialize by TupleSet \a d (tuple set is shared)
    TupleSet(const TupleSet& d);

    /// Add tuple to tuple set
    void add(const IntArgs& tuple);
    /// Finalize tuple set
    void finalize(void);
    /// Is tuple set finalized
    bool finalized(void) const;
    /// Arity of tuple set
    int arity(void) const;
    /// Number of tuples
    int tuples(void) const;
    /// Get tuple i
    Tuple operator [](int i) const;
    /// Minimum domain element
    int min(void) const;
    /// Maximum domain element
    int max(void) const;

    /// Initialize TupleSet from reflection specification
    GECODE_INT_EXPORT TupleSet(Reflection::VarMap& vm, Reflection::Arg* arg);
    /// Create reflection specification for the TupleSet
    GECODE_INT_EXPORT Reflection::Arg* spec(Reflection::VarMap& vm) const;
  };

  /** \brief Post propagator for \f$x\in T\f$.
   *
   * \li Supports implementations optimized for memory (\a epk = \a
   *     EPK_MEMORY, default) and speed (\a epk = \a EPK_SPEED).
   * \li Supports domain consistency (\a icl = ICL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   *
   * \warning If the domains for the \f$x_i\f$ are not dense and
   * have similar bounds, lots of memory will be wasted (memory
   * consumption is in \f$
   * O\left(|x|\cdot\min_i(\underline{x_i})\cdot\max_i(\overline{x_i})\right)\f$
   * for the basic algorithm (\a epk = \a EPK_MEMORY) and additionally \f$
   * O\left(|x|^2\cdot\min_i(\underline{x_i})\cdot\max_i(\overline{x_i})\right)\f$
   * for the incremental algorithm (\a epk = \a EPK_SPEED).
   */
  GECODE_INT_EXPORT void
  extensional(Space& home, const IntVarArgs& x, const TupleSet& t,
              ExtensionalPropKind epk=EPK_DEF, IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x\in T\f$.
   *
   * \li Supports implementations optimized for memory (\a epk = \a
   *     EPK_MEMORY, default) and speed (\a epk = \a EPK_SPEED).
   * \li Supports domain consistency (\a icl = ICL_DOM, default) only.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a t are of different size.
   */
  GECODE_INT_EXPORT void
  extensional(Space& home, const BoolVarArgs& x, const TupleSet& t,
              ExtensionalPropKind epk=EPK_DEF, IntConLevel icl=ICL_DEF);
 //@}
}

#include <gecode/int/extensional/dfa.hpp>
#include <gecode/int/extensional/tuple-set.hpp>

namespace Gecode {

  /**
   * \defgroup TaskModelIntArith Arithmetic constraints
   * \ingroup TaskModelInt
   */

  //@{
  /** \brief Post propagator for \f$ \min\{x_0,x_1\}=x_2\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  min(Space& home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ \min x=y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   */
  GECODE_INT_EXPORT void
  min(Space& home, const IntVarArgs& x, IntVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ \max\{x_0,x_1\}=x_2\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  max(Space& home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ \max x=y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   *
   * If \a x is empty, an exception of type Int::TooFewArguments is thrown.
   */
  GECODE_INT_EXPORT void
  max(Space& home, const IntVarArgs& x, IntVar y,
      IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$ |x_0|=x_1\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  abs(Space& home, IntVar x0, IntVar x1,
      IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_1=x_2\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  mult(Space& home, IntVar x0, IntVar x1, IntVar x2,
       IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_0=x_1\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  sqr(Space& home, IntVar x0, IntVar x1,
      IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$\lfloor\sqrt{x_0}\rfloor=x_1\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  GECODE_INT_EXPORT void
  sqrt(Space& home, IntVar x0, IntVar x1,
       IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2 \land x_0\ \mathrm{mod}\ x_1 = x_3\f$
   *
   * Supports bounds consistency (\a icl = ICL_BND, default).
   */
  GECODE_INT_EXPORT void
  divmod(Space& home, IntVar x0, IntVar x1, IntVar x2, IntVar x3,
         IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2\f$
   *
   * Supports bounds consistency (\a icl = ICL_BND, default).
   */
  GECODE_INT_EXPORT void
  div(Space& home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel icl=ICL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{mod}\ x_1=x_2\f$
   *
   * Supports bounds consistency (\a icl = ICL_BND, default).
   */
  GECODE_INT_EXPORT void
  mod(Space& home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel icl=ICL_DEF);
  //@}

  /**
   * \defgroup TaskModelIntLinearInt Linear constraints over integer variables
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
   *  - Variables occurring multiply in the argument arrays are replaced
   *    by a single occurrence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Int::Limits, an exception of type
   *    Int::OutOfLimits is thrown.
   *  - Assume the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the maximal
   *    available precision (at least \f$2^{48}\f$), an exception of
   *    type Int::OutOfLimits is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   */
  //@{
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r c\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const IntVarArgs& x,
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r y\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r c\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r y\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntLinearBool Linear constraints over Boolean variables
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
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for integers as defined in Int::Limits, an exception of
   *    type Int::OutOfLimits is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   */
  //@{
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r c\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const BoolVarArgs& x,
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r c\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const BoolVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r y\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const BoolVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r y\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space& home, const BoolVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, int c,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space& home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel icl=ICL_DEF);
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
   * \ingroup TaskModelInt
   */
  //@{
  /**
   * \brief Replace multiple variable occurences in \a x by fresh variables
   *
   * Supports domain consistency (\a icl = ICL_DOM, default) and
   * bounds consistency (\a icl = ICL_BND).
   *
   */
  GECODE_INT_EXPORT void
  unshare(Space& home, IntVarArgs& x,
          IntConLevel icl=ICL_DEF);
  /// Replace multiple variable occurences in \a x by fresh variables
  GECODE_INT_EXPORT void
  unshare(Space& home, BoolVarArgs& x,
          IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskModelIntBranch Branching
   * \ingroup TaskModelInt
   */
  //@{
  /// Which variable to select for branching
  enum IntVarBranch {
    INT_VAR_NONE = 0,        ///< First unassigned
    INT_VAR_RND,             ///< Random (uniform, for tie breaking)
    INT_VAR_DEGREE_MIN,      ///< With smallest degree
    INT_VAR_DEGREE_MAX,      ///< With largest degree
    INT_VAR_MIN_MIN,         ///< With smallest min
    INT_VAR_MIN_MAX,         ///< With largest min
    INT_VAR_MAX_MIN,         ///< With smallest max
    INT_VAR_MAX_MAX,         ///< With largest max
    INT_VAR_SIZE_MIN,        ///< With smallest domain size
    INT_VAR_SIZE_MAX,        ///< With largest domain size
    INT_VAR_SIZE_DEGREE_MIN, ///< With smallest domain size divided by degree
    INT_VAR_SIZE_DEGREE_MAX, ///< With largest domain size divided by degree
    /** \brief With smallest min-regret
     *
     * The min-regret of a variable is the difference between the
     * smallest and second-smallest value still in the domain.
     */
    INT_VAR_REGRET_MIN_MIN,
    /** \brief With largest min-regret
     *
     * The min-regret of a variable is the difference between the
     * smallest and second-smallest value still in the domain.
     */
    INT_VAR_REGRET_MIN_MAX,
    /** \brief With smallest max-regret
     *
     * The max-regret of a variable is the difference between the
     * largest and second-largest value still in the domain.
     */
    INT_VAR_REGRET_MAX_MIN,
    /** \brief With largest max-regret
     *
     * The max-regret of a variable is the difference between the
     * largest and second-largest value still in the domain.
     */
    INT_VAR_REGRET_MAX_MAX
  };

  /// Which values to select first for branching
  enum IntValBranch {
    INT_VAL_MIN,       ///< Select smallest value
    INT_VAL_MED,       ///< Select median value (rounding downwards)
    INT_VAL_MAX,       ///< Select largest value
    INT_VAL_RND,       ///< Select random value
    INT_VAL_SPLIT_MIN, ///< Select lower half of domain
    INT_VAL_SPLIT_MAX, ///< Select upper half of domain
    INT_VALUES_MIN,    ///< Try all values starting from smallest
    INT_VALUES_MAX     ///< Try all values starting from largest
  };

  /// Branch over \a x with variable selection \a vars and value selection \a vals
  GECODE_INT_EXPORT void
  branch(Space& home, const IntVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const VarBranchOptions& o_vars = VarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
  GECODE_INT_EXPORT void
  branch(Space& home, const IntVarArgs& x,
         const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
         const TieBreakVarBranchOptions& o_vars = TieBreakVarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with variable selection \a vars and value selection \a vals
  GECODE_INT_EXPORT void
  branch(Space& home, const BoolVarArgs& x,
         IntVarBranch vars, IntValBranch vals,
         const VarBranchOptions& o_vars = VarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
  GECODE_INT_EXPORT void
  branch(Space& home, const BoolVarArgs& x,
         const TieBreakVarBranch<IntVarBranch>& vars, IntValBranch vals,
         const TieBreakVarBranchOptions& o_vars = TieBreakVarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  //@}

  /**
   * \defgroup TaskModelIntAssign Assigning
   * \ingroup TaskModelInt
   */
  //@{
  /// Which value to select for assignment
  enum IntAssign {
    INT_ASSIGN_MIN, ///< Select smallest value
    INT_ASSIGN_MED, ///< Select median value (rounding downwards)
    INT_ASSIGN_MAX  ///< Select maximum value
  };

  /// Assign all \a x with value selection \a vals
  GECODE_INT_EXPORT void
  assign(Space& home, const IntVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Assign all \a x with value selection \a vals
  GECODE_INT_EXPORT void
  assign(Space& home, const BoolVarArgs& x, IntAssign vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);

  //@}

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

#endif

// IFDEF: GECODE_HAS_INT_VARS
// STATISTICS: int-post

