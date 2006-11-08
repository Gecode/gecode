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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_INT_HH__
#define __GECODE_INT_HH__

namespace Gecode { namespace Int {

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

}}

#include "gecode/limits.hh"

#include "gecode/kernel.hh"

/*
 * Support for DLLs under Windows
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

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_INT_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_INT_EXPORT

#endif
#endif

#include <iostream>

#include "gecode/iter.hh"
#include "gecode/support/shared-array.hh"

#include "gecode/int/exception.icc"

namespace Gecode {

  class IntSetRanges;

  /**
   * \brief Integer sets
   *
   * Integer sets are the means to specify arbitrary sets
   * of integers to be used as domains for integer variables.
   * \ingroup TaskIntIntVars TaskSetSetVars
   */
  class IntSet {
    friend class IntSetRanges;
  private:
    /// %Range (intervals) of integers
    class Range {
    public:
      int min, max;
    };
    /// Shared array that stores the ranges of the domain
    Support::SharedArray<Range> sar;
    /// Sort ranges according to increasing minimum
    class MinInc;
    /// Normalize the first \a n elements
    GECODE_INT_EXPORT void normalize(int n);
    GECODE_INT_EXPORT void init(int n, int m);
    GECODE_INT_EXPORT void init(const int r[], int n);
    GECODE_INT_EXPORT void init(const int r[][2], int n);
  public:
    /// \name Constructors and initialization
    //@{
    /// Initialize as empty set
    IntSet(void);
    /// Copy constructor
    IntSet(const IntSet& is);
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
    IntSet(I& i);
    //@}

    /// \name Range length
    //@{
    /// Return number of ranges of the specification
    int size(void) const;
    //@}

    /// \name Range access
    //@{
    /// Return minimum of range at position \a i
    int min(int i) const;
    /// Return maximum of range at position \a i
    int max(int i) const;
    /// Return width of range at position \a i
    unsigned int width(int i) const;
    //@}

    /// \name Entire set access
    //@{
    /// Return minimum of entire set
    int min(void) const;
    /// Return maximum of entire set
    int max(void) const;
    //@}

    /// \name Cloning
    //@{
    /** \brief Update this set to be a copy of \a s
     *
     * If \a share is true, the copy is identical. Otherwise an independent
     * copy is created.
     */
    void update(bool share, IntSet& s);
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
   * \ingroup TaskIntIntVars TaskSetSetVars
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
    bool operator()(void) const;
    /// Move iterator to next range (if possible)
    void operator++(void);
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
   * \ingroup TaskIntIntVars TaskSetSetVars
   */
  class IntSetValues
    : public Iter::Ranges::ToValues<IntSetRanges> {
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

}

/**
 * \brief Print integer set \a s
 * \relates Gecode::IntSet
 */
GECODE_INT_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::IntSet& s);

#include "gecode/int/int-set.icc"


#include "gecode/int/var.icc"
#include "gecode/int/view.icc"
#include "gecode/int/propagator.icc"
#include "gecode/int/array.icc"


namespace Gecode {

  /**
   * \brief Common relation types for integers
   * \ingroup TaskIntInt
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
   * \brief Consistency levels for integer propagators
   *
   * The descriptions are meant to be suggestions. It is not
   * required that a propagator achieves full domain-consistency or
   * full bounds-consistency. It is more like: which level
   * of consistency comes closest.
   *
   * If in the description of a constraint below no consistency level
   * is mentioned, the propagator for the constraint implements
   * domain-consistency.
   * \ingroup TaskIntInt
   */
  enum IntConLevel {
    ICL_VAL, ///< Value consistency (naive)
    ICL_BND, ///< Bounds consistency
    ICL_DOM, ///< Domain consistency
    ICL_DEF  ///< The default consistency for a constraint
  };



  /**
   * \defgroup TaskIntIntDomain Domain constraints
   * \ingroup TaskIntInt
   *
   */

  //@{
  /// Propagates \f$ l\leq x\leq m\f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVar x, int l, int m,
      IntConLevel=ICL_DEF);
  /// Propagates \f$ l\leq x_i\leq m\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVarArgs& x, int l, int m,
      IntConLevel=ICL_DEF);

  /// Propagates \f$ x\in s \f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVar x, const IntSet& s,
      IntConLevel=ICL_DEF);
  /// Propagates \f$ x_i\in s\f$ for all \f$0\leq i<|x|\f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVarArgs& x, const IntSet& s,
      IntConLevel=ICL_DEF);

  /// Post propagator for \f$ (l\leq x \leq m) \Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVar x, int l, int m, BoolVar b,
      IntConLevel=ICL_DEF);
  /// Post propagator for \f$ (x \in s) \Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  dom(Space* home, IntVar x, const IntSet& s, BoolVar b,
      IntConLevel=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntRel Simple relation constraints
   * \ingroup TaskIntInt
   */

  //@{


  /** \brief Post propagator for \f$ x_0 \sim_r x_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1,
      IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_0 \sim_r x_1\f$
  GECODE_INT_EXPORT void
  rel(Space* home, BoolVar x0, IntRelType r, BoolVar x1,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x \sim_r c\f$
  GECODE_INT_EXPORT void
  rel(Space* home, IntVar x, IntRelType r, int c,
      IntConLevel icl=ICL_DEF);
  /// Propagates \f$ x \sim_r c\f$
  GECODE_INT_EXPORT void
  rel(Space* home, BoolVar x, IntRelType r, int c,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ (x_0 \sim_r x_1)\Leftrightarrow b\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space* home, IntVar x0, IntRelType r, IntVar x1, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$(x \sim_r c)\Leftrightarrow b\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  rel(Space* home, IntVar x, IntRelType r, int c, BoolVar b,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for pairwise relation on \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal. 
   *    Supports both bounds (\a icl = ICL_BND) and
   *    domain-consistency (\a icl = ICL_DOM, default).
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ, 
   *    then the elements of \a x are ordered with respt to \a r.
   *    Supports domain-consistency (\a icl = ICL_DOM, default).
   *  - if \a r = IRT_NQ, then all elements of \a x must be pairwise 
   *    distinct (corresponds to the distinct constraint).
   *    Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *    and domain-consistency (\a icl = ICL_DOM).
   *    Throws an exception of type Int::ArgumentSame, if \a x contains
   *    the same variable multiply.
   *
   */
  GECODE_INT_EXPORT void
  rel(Space* home, const IntVarArgs& x, IntRelType r,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for pairwise relation on \a x.
   *
   * States that the elements of \a x are in the following relation:
   *  - if \a r = IRT_EQ, then all elements of \a x must be equal. 
   *  - if \a r = IRT_LE, \a r = IRT_LQ, \a r = IRT_GR, or \a r = IRT_GQ, 
   *    then the elements of \a x are ordered with respt to \a r.
   *  - if \a r = IRT_NQ, then all elements of \a x must be pairwise 
   *    distinct (corresponds to the distinct constraint).
   */
  GECODE_INT_EXPORT void
  rel(Space* home, const BoolVarArgs& x, IntRelType r,
      IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for relation between \a x and \a y.
   *
   * Note that for the inequality relations this corresponds to
   * the lexical order between \a x and \a y.
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  GECODE_INT_EXPORT void
  rel(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
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
  rel(Space* home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntLink Link constraints between Boolean and Integer variables
   * \ingroup TaskIntInt
   */

  //@{
  /// Post propagator for \f$ x_0 = x_1\f$
  GECODE_INT_EXPORT void
  link(Space* home, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_0 = x_1\f$
  forceinline void
  link(Space* home, BoolVar x0, IntVar x1, IntConLevel icl=ICL_DEF) {
    link(home,x1,x0,icl);
  }
  //@}


  /**
   * \defgroup TaskIntIntElement Element constraints
   * \ingroup TaskIntInt
   */

  //@{
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::NumericalOverflow, if
   *  the integers in \a n exceed the limits in Limits::Int.
   */
  GECODE_INT_EXPORT void
  element(Space* home, const IntArgs& n, IntVar x0, IntVar x1,
          IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::NumericalOverflow, if
   *  the integers in \a n exceed the limits in Limits::Int.
   */
  GECODE_INT_EXPORT void
  element(Space* home, const IntArgs& n, IntVar x0, BoolVar x1,
          IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ n_{x_0}=x_1\f$
   *
   *  Throws an exception of type Int::NumericalOverflow, if
   *  the integers in \a n exceed the limits in Limits::Int.
   */
  GECODE_INT_EXPORT void
  element(Space* home, const IntArgs& n, IntVar x0, int x1,
          IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Space* home, const IntVarArgs& x, IntVar y0, IntVar y1,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ x_{y_0}=y_1\f$
   *
   * Supports both bounds (\a icl = ICL_BND) and
   * domain-consistency (\a icl = ICL_DOM, default).
   */
  GECODE_INT_EXPORT void
  element(Space* home, const IntVarArgs& x, IntVar y0, int y1,
          IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Space* home, const BoolVarArgs& x, IntVar y0, BoolVar y1,
          IntConLevel icl=ICL_DEF);
  /// Post propagator for \f$ x_{y_0}=y_1\f$
  GECODE_INT_EXPORT void
  element(Space* home, const BoolVarArgs& x, IntVar y0, int y1,
          IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntDistinct Distinct constraints
   * \ingroup TaskIntInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   * and domain-consistency (\a icl = ICL_DOM).
   *
   * Throws an exception of type Int::ArgumentSame, if \a x contains
   * the same variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Space* home, const IntVarArgs& x,
           IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$ x_i+n_i\neq x_j+n_j\f$ for all \f$0\leq i\neq j<|x|\f$
   *
   * \li Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *     and domain-consistency (\a icl = ICL_DOM).
   * \li Throws an exception of type Int::NumericalOverflow, if
   *     the integers in \a n exceed the limits in Limits::Int.
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a n are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x contains
   *     the same variable multiply.
   */
  GECODE_INT_EXPORT void
  distinct(Space* home, const IntArgs& n, const IntVarArgs& x,
           IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntChannel Channel constraints
   * \ingroup TaskIntInt
   */

  //@{
  /** \brief Post propagator for \f$ x_i = j\leftrightarrow y_j=i\f$ for all \f$0\leq i<|x|\f$
   *
   * \li Supports value (\a icl = ICL_VAL) and domain propagation (all other
   *     values for \a icl).
   * \li Throws an exception of type Int::ArgumentSizeMismatch, if
   *     \a x and \a n are of different size.
   * \li Throws an exception of type Int::ArgumentSame, if \a x together
   *     with \a y contains the same variable multiply.
   */
  GECODE_INT_EXPORT void
  channel(Space* home, const IntVarArgs& x, const IntVarArgs& y,
           IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntScheduling Scheduling constraints
   * \ingroup TaskIntInt
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
   * \exception Int::NumericalOverflow thrown if any numerical argument is
   *            larger than Limits::Int::int_max or less than
   *            Limits::Int::int_min.
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntVarArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
   GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntVarArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntVarArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  /** \brief Post propagators for the cumulatives constraint.
   *
   * \copydoc cumulatives()
   */
  GECODE_INT_EXPORT void
  cumulatives(Space* home, const IntArgs& machine,
              const IntVarArgs& start, const IntArgs& duration,
              const IntVarArgs& end, const IntArgs& height,
              const IntArgs& limit, bool at_most, IntConLevel icl=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntRegular Regular constraints
   * \ingroup TaskIntInt
   */

  class DFA;

  //@{
  /// %Regular expressions
  class GECODE_INT_EXPORT REG {
    friend class DFA;
  private:
    /// Implementation of the actual expression tree
    class Exp;
    /// The expression tree
    Exp* e;
    /// Initialize with given expression tree \a
    REG(Exp* e);
  public:
    /// Initialize from regular expression \a r
    REG(const REG& r);
    /// Initialize from regular expression \a r
    const REG& operator=(const REG& r);

    /// Initialize as empty sequence
    REG(void);
    /// Initialize as single integer
    REG(int);
    /// Return expression for: this expression at least \a n times, at most \a m times
    REG operator()(unsigned int n, unsigned int m);
    /// Return expression for: this expression at least \a n times
    REG operator()(unsigned int n);
    /// Return expression for: this expression or \a r
    REG operator|(const REG& r);
    /// Return expression for: this expression followed by \a r
    REG operator+(const REG& r);
    /// Repeat expression for this expression arbitrarily often (Kleene star)
    REG operator*(void);
    /** \brief Repeat expression for this expression arbitrarily often, but
     *        at least once
     */
    REG operator+(void);
    /// Print expression
    std::ostream& print(std::ostream&) const;
    /// Destructor
    ~REG(void);
  };

  /**
   * \brief Deterministic finite automaton
   *
   * After initialization, the start state is always zero.
   * The final states are contiguous ranging from the first to the
   * last final state.
   */
  class DFA {
  public:
    /// Specification of a %DFA transition
    class Transition {
    public:
      int i_state; ///< input state
      int symbol;  ///< symbol
      int o_state; ///< output state
    };
    /// Iterator for %DFA transitions
    class Transitions;
  private:
    /// Class for %DFA implementation
    class DFAI;
    /// %DFA implementation
    DFAI* dfai;
  protected:
    GECODE_INT_EXPORT
    /**
     * \brief Initialize automaton
     *
     * Start state \a start, transition specification t_spec,
     * final state specification \a f_spec and a flag \a minimize whether
     * the automaton should be minimized
     */
    void init(int start, Transition t_spec[], int f_spec[],
              bool minimize);
  public:
    friend class Transitions;
    /// Default constructor (empty DFA)
    DFA(void);
    /**
     * \brief Initialize DFA by transitions
     *
     * - Start state is given by \a s.
     * - %Transitions are described by \a t, where the last element
     *   must have -1 as value for \c i_state.
     * - Final states are given by \a f, where the last final element
     *   must be -1.
     * - Minimizes the DFA, if \a minimize is true.
     */
    DFA(int s, Transition t[], int f[], bool minimize=true);
    GECODE_INT_EXPORT
    /// Initialize DFA by regular expression \a r
    DFA(REG& r);
    /// Initialize by DFA \a d (DFA is shared)
    DFA(const DFA& d);
    /// Initialize by DFA \a d (DFA is shared)
    const DFA& operator=(const DFA&);
    /// Destructor
    ~DFA(void);
    /**
     * \brief Update this DFA to \a d
     *
     * If \a share is true, share the same \a d. If not, create
     * an independent copy from \a d.
     */
    void update(bool share, DFA& d);
    /// Return the number of states
    unsigned int n_states(void) const;
    /// Return the number of transitions
    unsigned int n_transitions(void) const;
    /// Return the number of the first final state
    int final_fst(void) const;
    /// Return the number of the last final state
    int final_lst(void) const;
    /// Return smallest symbol in DFA
    int symbol_min(void) const;
    /// Return largest symbol in DFA
    int symbol_max(void) const;
  };

  /**
   * \brief Post propagator for regular constraint
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   */
  GECODE_INT_EXPORT void
  regular(Space* home, const IntVarArgs& x, DFA& d,
          IntConLevel=ICL_DEF);

  /**
   * \brief Post propagator for regular constraint
   *
   * The elements of \a x must be a word of the language described by
   * the DFA \a d.
   */
  GECODE_INT_EXPORT void
  regular(Space* home, const BoolVarArgs& x, DFA& d,
          IntConLevel=ICL_DEF);

  //@}

}

#include "gecode/int/regular/dfa.icc"

namespace Gecode {

  /**
   * \note The Sortedness propagator described in this section only
   *       supports bounds-consistency no matter what value for the
   *       argument \a icl is given!
   *
   * \defgroup TaskIntIntSort Sorting constraints
   * \ingroup TaskIntInt
   */


  //@{
  /**
   *  \brief Post propagator \f$\exists\pi:\forall i\in\{0,\dots,|x|-1\}:
   *         y_0 \leq\dots\leq y_{|x|-1} \wedge  x_i=y_{\pi(i)}\f$
   *
   *  \anchor SortednessSemantics
   *
   *  \exception Int::ArgumentSizeMismatch thrown if \a x and \a y
   *             differ in size.
   *  \exception Int::ArgumentSame thrown if \a x or \a y contain
   *             shared variables.
   *
   *  Taking only two arguments, this interface for Sortedness leaves
   *  the sorting permutation \f$\pi\f$ implicit.
   *
   */
  GECODE_INT_EXPORT void
  sortedness(Space* home, const IntVarArgs& x, const IntVarArgs& y,
             IntConLevel icl=ICL_DEF);

  /**
   *  \brief Post propagator \f$\forall i\in\{0,\dots,|x|-1\}:
   *         y_0 \leq\dots\leq y_{|x|-1} \wedge x_i=y_{z_i} \f$ \n
   *
   *  \exception Int::ArgumentSizeMismatch thrown if \a x, \a y or \a z
   *             differ in size.
   *  \exception Int::ArgumentSame thrown if \a x, \a y or \a z contain
   *             shared variables.
   *
   * Providing a third argument \f$z_0, \dots, z_{|x|-1} \f$, this
   * interface for Sortedness models the sorting permutation
   * \f$\pi\f$ explicitly, such that
   * \f$\forall i\in\{0,\dots,|x|-1\}:\pi(i) = z_i\f$ holds.
   *
   */

  GECODE_INT_EXPORT void
  sortedness(Space*, const IntVarArgs& x, const IntVarArgs& y,
             const IntVarArgs& z,
             IntConLevel icl=ICL_DEF);
  //@}

  /**
   * \defgroup TaskIntIntCard Cardinality constraints
   * \ingroup TaskIntInt
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
   * Supports domain-consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space* home, const IntVarArgs& x, int n, IntRelType r, int m,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_r m\f$
   *
   * Supports domain-consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space* home, const IntVarArgs& x, IntVar y, IntRelType r, int m,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\sim_r z\f$
   *
   * Supports domain-consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space* home, const IntVarArgs& x, int n, IntRelType r, IntVar z,
        IntConLevel icl=ICL_DEF);
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\sim_r z\f$
   *
   * Supports domain-consistent propagation only.
   */
  GECODE_INT_EXPORT void
  count(Space* home, const IntVarArgs& x, IntVar y, IntRelType r, IntVar z,
        IntConLevel icl=ICL_DEF);


  /**
   *  \brief Post propagator for
   *  \f{eqnarray*}
   *       \forall t=(v, lb, ub) \in c: & &
   *       lb \leq \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} \leq ub \\
   *           \forall t=(v, unspec_{low}, unspec_{up}) \not\in c: & &
   *           unspec_{low} \leq \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} \leq unspec_{up}
   *  \f}
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  \exception Int::ArgumentSame thrown if \a x contains shared variables.
   *
   *  \param x variables on which to perform propagation
   *  \param c specifying cardinality information as shown below
   *  \param m denotes the size of c
   *  \param unspec_low denotes the lower bound for those values
   *         not specified in c
   *  \param unspec_up denotes the upper bound for those values
   *         not specified in c
   *  \param min smallest domain value of \a x
   *  \param max largest domain value of \a x
   *  \param icl consistency level
   *
   *  Let \f$ D:=\displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}} dom(x_i)\f$ and
   *  \f$ V \f$ the set of values represented by \a c. Then this progator allows
   *  sets \f$ V\subset D\f$ as well as \f$ V=D\f$.
   *
   *  In this interface values \f$ v\f$ and their cardinality bounds
   *  have to specified such that \a c looks as follows (for example):
   *  \f$ c=[ (1,0,1), (2,1,3), \dots, (10,4,5)]\f$, where the value 1 may occur
   *  zero times or once, the value 2 must occur at least once at most three times
   *  and the value 10 must occur at least 4 times and at most 5 times.
   *
   *  Furthermore, this interface requires that
   *  \f$ \forall {i\in\{0, \dots, |x|-1\}}: dom(x_i)
   *  \subseteq I=[min;max]\f$.
   *
   */

  GECODE_INT_EXPORT void
  gcc(Space* home, const IntVarArgs& x, const IntArgs& c,
      int m, int unspec_low, int unspec_up, int min, int max,
      IntConLevel icl);

  /**
   *  \brief Post propagator for
   *  \f{eqnarray*}
   *      \forall t=(v, lb, ub) \in c: & &
   *      lb \leq \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} \leq ub \\
   *          \forall t=(v, 0, unspec) \not\in c: & &
   *          0 \leq \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} \leq unspec
   *  \f}
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  \exception Int::ArgumentSame thrown if \a x contains shared variables.
   *
   *  \param x variables on which to perform propagation
   *  \param c specifying cardinality information as shown below
   *  \param m denotes the size of \a c
   *  \param unspec denotes the upper bound for those values
   *         not specified in \a c
   *  \param min smallest domain value of \a x
   *  \param max largest domain value of \a x
   *  \param icl consistency level
   *
   *  Let \f$ D:=\displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}} dom(x_i)\f$ and
   *  \f$ V \f$ the set of values represented by \a c. Then this interface allows
   *  to specify sets \f$ V\subset D\f$ as well as \f$ V=D\f$.
   *
   *  In this interface values \f$ v\f$ and their cardinality bounds
   *  have to be specified such that \a c looks as follows (for example):
   *  \f$ c=[ (1,0,1), (2,1,3), \dots, (10,4,5)]\f$, where the value 1 may occur
   *  zero times or once, the value 2 must occur at least once at most three times
   *  and the value 10 must occur at least 4 times and at most 5 times.
   */

  GECODE_INT_EXPORT void
  gcc(Space* home, const IntVarArgs& x, const IntArgs& c,
      int m, int unspec, int min, int max,
      IntConLevel icl);

  /**
   *  \brief Post propagator for \f$ \forall v
   *         \in \displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}} dom(x_i):
   *         lb \leq \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} \leq ub \f$
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  \exception Int::ArgumentSame thrown if \a x contains shared variables.
   *
   *  \param x variables on which to perform propagation
   *  \param lb denotes the lower bound for all values
   *         specified in the array \a c,
   *  \param ub denotes the upper bound for all values
   *         specified in the array \a c,
   *         where this interface allows only value sets
   *         \f$ V=D \displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}}
   *         dom(x_i)\f$
   *  \param icl consistency level
   */

  GECODE_INT_EXPORT void
  gcc(Space* home, const IntVarArgs& x, int lb, int ub, IntConLevel icl);

  /**
   *  \brief Post propagator for \f$ \forall v
   *         \in \displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}} dom(x_i):
   *         lb = \#\{i\in\{0, \dots, |x| - 1\} | x_i = v\} = ub \f$
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  \exception Int::ArgumentSame thrown if \a x contains shared variables.
   *
   *  \param x variables on which to perform propagation
   *  \param ub denotes the upper bound for all values specified
   *            in the array \a c, where this interface
   *            allows only value sets
   *            \f$ V=D \displaystyle \bigcup_{i\in\{0, \dots, |x|-1\}}
   *            dom(x_i)\f$
   *  \param icl consistency level
   *
   */

  GECODE_INT_EXPORT void
  gcc(Space* home, const IntVarArgs& x, int ub, IntConLevel icl);


  /**
   *  \brief Post propagator for
   *  \f{eqnarray*}
   *    v_j \in I=[min;max] & & \\
   *    \forall j \in \{0, \dots, |I| - 1\}: & &
   *    \#\{i\in\{0, \dots, |x| - 1\} | x_i = v_j\} = c_j
   *  \f}
   *
   *  \param x variables on which to perform propagation
   *  \param c cardinality variables
   *  \param min smallest domain value of \a x
   *  \param max largest domain value of \a x
   *  \param icl consistency level
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  This interface requires that
   *  \f$ \forall {i\in\{0, \dots, |x|-1\}}: dom(x_i)
   *  \subseteq I=[min;max]\f$.
   */

  GECODE_INT_EXPORT void
  gcc(Space* home,  const IntVarArgs& x, const IntVarArgs& c,
      int min, int max,
      IntConLevel icl);

  /**
   *  \brief Post propagator for \f$ \forall j \in \{0, \dots, |v| - 1\}:
   *  \#\{i\in\{0, \dots, |x| - 1\} | x_i = v_j\} = c_j \f$
   *
   *  \param x variables on which to perform propagation
   *  \param v containing the values connected to the cardinality
   *         variables as specified below
   *  \param c cardinality variables
   *  \param m denotes the size of \a v and \a c
   *  \param unspec_low denotes the lower bound for those values
   *         not specified in \a v and \a c
   *  \param unspec_up denotes the upper bound for those values
   *         not specified in \a v and \a c
   *  \param all specifies whether the propagator uses all values in
   *         the interval \f$ I=[min;max]\f$
   *  \param min smallest domain value of \a x
   *  \param max largest domain value of \a x
   *  \param icl consistency level
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  This interface requires that
   *  \f$ \forall {i\in\{0, \dots, |x|-1\}}: dom(x_i)
   *  \subseteq I=[min;max]\f$.
   *  If \a all is set to true, every value from the interval
   *  \f$ I=[min;max]\f$ is specified with cardinalities.
   *  Otherwise, only specified values in \a v are used and unspecified values
   *  may occur between \a unspec_low and \a unspec_up times.
   */

  GECODE_INT_EXPORT void
  gcc(Space* home,
      const IntVarArgs& x, const IntArgs& v, const IntVarArgs& c,
      int m, int unspec_low, int unspec_up, bool all, int min, int max,
      IntConLevel icl);

  /**
   *  \brief Post propagator for \f$ \forall j \in \{0, \dots, |v| - 1\}:
   *  \#\{i\in\{0, \dots, |x| - 1\} | x_i = v_j\} = c_j \f$
   *
   *  \param x variables on which to perform propagation
   *  \param v containing the values connected to the cardinality
   *         variables as specified below
   *  \param c cardinality variables
   *  \param m denotes the size of \a v and \a c
   *  \param unspec denotes the upper bound for those values
   *         not specified in \a v and \a c
   *  \param all specifies whether the propagator uses all values in
   *         the interval \f$ I=[min;max]\f$
   *  \param min smallest domain value of \a x
   *  \param max largest domain value of \a x
   *  \param icl consistency level
   *
   *  Supports value (\a icl = ICL_VAL, default), bounds (\a icl = ICL_BND),
   *  and domain-consistency (\a icl = ICL_DOM).
   *
   *  This interface requires that
   *  \f$ \forall {i\in\{0, \dots, |x|-1\}}: dom(x_i)
   *  \subseteq [min, \dots, max]\f$.
   *  If \a all is set to true, every value from the interval
   *  \f$ I=[min;max]\f$ is specified with cardinalities.
   *  Otherwise, only specified values in \a v are used and unspecified values
   *  may occur between 0 and \a unspec times.
   */
  GECODE_INT_EXPORT void
  gcc(Space* home,
      const IntVarArgs& x, const IntArgs& v, const IntVarArgs& c,
      int m, int unspec, bool all, int min, int max,
      IntConLevel icl);
  //@}

  /**
   * \defgroup TaskIntIntBool Boolean constraints
   * \ingroup TaskIntInt
   */

  //@{
  /// Post propagator for \f$ \lnot b_0 = b_1\f$
  GECODE_INT_EXPORT void
  bool_not(Space* home, BoolVar b0, BoolVar b1,
           IntConLevel=ICL_DEF);

  /// Post propagator for \f$ b_0 = b_1\f$
  GECODE_INT_EXPORT void
  bool_eq(Space* home, BoolVar b0, BoolVar b1,
          IntConLevel=ICL_DEF);

  /// Post propagator for \f$ b_0 \land b_1 = b_2 \f$
  GECODE_INT_EXPORT void
  bool_and(Space* home, BoolVar b0, BoolVar b1, BoolVar b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_0 \land b_1 = b_2 \f$
  GECODE_INT_EXPORT void
  bool_and(Space* home, BoolVar b0, BoolVar b1, bool b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ \bigwedge_{i=0}^{|b|-1} b_i = c\f$
  GECODE_INT_EXPORT void
  bool_and(Space* home, const BoolVarArgs& b, BoolVar c,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ \bigwedge_{i=0}^{|b|-1} b_i = c\f$
  GECODE_INT_EXPORT void
  bool_and(Space* home, const BoolVarArgs& b, bool c,
           IntConLevel=ICL_DEF);

  /// Post propagator for \f$ b_0 \lor b_1 = b_2 \f$
  GECODE_INT_EXPORT void
  bool_or(Space* home, BoolVar b0, BoolVar b1, BoolVar b2,
          IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_0 \lor b_1 = b_2 \f$
  GECODE_INT_EXPORT void
  bool_or(Space* home, BoolVar b0, BoolVar b1, bool b2,
          IntConLevel=ICL_DEF);
  /// Post propagator for \f$ \bigvee_{i=0}^{|b|-1} b_i = c\f$
  GECODE_INT_EXPORT void
  bool_or(Space* home, const BoolVarArgs& b, BoolVar c,
          IntConLevel=ICL_DEF);
  /// Post propagator for \f$ \bigvee_{i=0}^{|b|-1} b_i = c\f$
  GECODE_INT_EXPORT void
  bool_or(Space* home, const BoolVarArgs& b, bool c,
          IntConLevel=ICL_DEF);

  /// Post propagator for \f$ b_0 \Rightarrow b_1 = b_2 \f$ (implication)
  GECODE_INT_EXPORT void
  bool_imp(Space* home, BoolVar b0, BoolVar b1, BoolVar b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_0 \Rightarrow b_1 = b_2 \f$ (implication)
  GECODE_INT_EXPORT void
  bool_imp(Space* home, BoolVar b0, BoolVar b1, bool b2,
           IntConLevel=ICL_DEF);

  /// Post propagator for \f$ b_0 \Leftrightarrow b_1 = b_2 \f$ (equivalence)
  GECODE_INT_EXPORT void
  bool_eqv(Space* home, BoolVar b0, BoolVar b1, BoolVar b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_0 \Leftrightarrow b_1 = b_2 \f$ (equivalence)
  GECODE_INT_EXPORT void
  bool_eqv(Space* home, BoolVar b0, BoolVar b1, bool b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_1 \otimes b_2 = b_3 \f$ (exclusive or)
  GECODE_INT_EXPORT void
  bool_xor(Space* home, BoolVar b0, BoolVar b1, BoolVar b2,
           IntConLevel=ICL_DEF);
  /// Post propagator for \f$ b_1 \otimes b_2 = b_3 \f$ (exclusive or)
  GECODE_INT_EXPORT void
  bool_xor(Space* home, BoolVar b0, BoolVar b1, bool b2,
           IntConLevel=ICL_DEF);

  //@}

  /**
   * \defgroup TaskIntIntArith Arithmetic constraints
   * \ingroup TaskIntInt
   */

  //@{
  /** \brief Post propagator for \f$ \min\{x_0,x_1\}=x_2\f$
   *
   * Only bounds-consistency is supported.
   */
  GECODE_INT_EXPORT void
  min(Space* home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ \min x=y\f$
   *
   * Only bounds-consistency is supported.
   */
  GECODE_INT_EXPORT void
  min(Space* home, const IntVarArgs& x, IntVar y,
      IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ \max\{x_0,x_1\}=x_2\f$
   *
   * Only bounds-consistency is supported. If \a x is empty,
   * an exception of type Int::ArgumentEmpty is thrown.
   */
  GECODE_INT_EXPORT void
  max(Space* home, IntVar x0, IntVar x1, IntVar x2,
      IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$ \max x=y\f$
   *
   * Only bounds-consistency is supported. If \a x is empty,
   * an exception of type Int::ArgumentEmpty is thrown.
   */
  GECODE_INT_EXPORT void
  max(Space* home, const IntVarArgs& x, IntVar y,
      IntConLevel=ICL_DEF);

  /** \brief Post propagator for \f$ |x_0|=x_1\f$
   *
   * Only bounds-consistency is supported.
   */
  GECODE_INT_EXPORT void
  abs(Space* home, IntVar x0, IntVar x1,
      IntConLevel=ICL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_1=x_2\f$
   *
   * Only bounds-consistency is supported. The propagator
   * is overflow safe.
   */
  GECODE_INT_EXPORT void
  mult(Space* home, IntVar x0, IntVar x1, IntVar x2,
       IntConLevel=ICL_DEF);
  //@}

  /**
   * \defgroup TaskIntIntLinear Linear constraints
   * \ingroup TaskIntInt
   *
   * All variants for linear constraints share the following properties:
   *  - Bounds-consistency (over the real numbers) is supported for
   *    all constraints (actually, for disequlities always domain-consistency
   *    is used as it is cheaper). Domain-consistency is supported for all
   *    non-reified constraint. As bounds-consistency for inequalities
   *    coincides with domain-consistency, the only
   *    real variation is for linear equations. Domain-consistent
   *    linear equations have exponential complexity, so use with care!
   *  - Variables occurring multiply in the argument arrays are replaced
   *    by a single occurrence: for example, \f$ax+bx\f$ becomes
   *    \f$(a+b)x\f$.
   *  - If in the above simplification the value for \f$(a+b)\f$ (or for
   *    \f$a\f$ and \f$b\f$) exceeds the limits for integers as
   *    defined in Limits::Int, an exception of type
   *    Int::NumericalOverflow is thrown.
   *  - Assume the constraint
   *    \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$.
   *    If  \f$|c|+\sum_{i=0}^{|x|-1}a_i\cdot x_i\f$ exceeds the limits
   *    for doubles as defined in Limits::Int, an exception of
   *    type Int::NumericalOverflow is thrown.
   *  - In all other cases, the created propagators are accurate (that
   *    is, they will not silently overflow during propagation).
   */

  //@{
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r c\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const IntVarArgs& x,
         IntRelType r, int c,
         IntConLevel=ICL_DEF);
  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r y\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r c\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b, IntConLevel=ICL_DEF);
  /// Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_r y\right)\Leftrightarrow b\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b, IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c,
         IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, int c, BoolVar b,
         IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\right)\Leftrightarrow b\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const IntVarArgs& x,
         IntRelType r, IntVar y, BoolVar b,
         IntConLevel=ICL_DEF);

  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r c\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const BoolVarArgs& x,
         IntRelType r, int c,
         IntConLevel=ICL_DEF);

  /// Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_r y\f$
  GECODE_INT_EXPORT void
  linear(Space* home, const BoolVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel=ICL_DEF);

  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r c\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, int c,
         IntConLevel=ICL_DEF);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_r y\f$
   *
   *  Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   */
  GECODE_INT_EXPORT void
  linear(Space* home, const IntArgs& a, const BoolVarArgs& x,
         IntRelType r, IntVar y,
         IntConLevel=ICL_DEF);
  //@}


  /**
   * \defgroup TaskIntIntBranch Branching
   * \ingroup TaskIntInt
   */

  //@{
  /// Which variable to select for branching
  enum BvarSel {
    BVAR_NONE,          ///< First unassigned
    BVAR_MIN_MIN,       ///< With smallest min
    BVAR_MIN_MAX,       ///< With largest min
    BVAR_MAX_MIN,       ///< With smallest max
    BVAR_MAX_MAX,       ///< With largest max
    BVAR_SIZE_MIN,      ///< With smallest domain size
    BVAR_SIZE_MAX,      ///< With largest domain size
    /** \brief With smallest degree
     *
     * The degree of a variable is defined as the number of dependant
     * propagators. In case of ties, choose the variable with smallest
     * domain.
     */
    BVAR_DEGREE_MIN,
    /** \brief With largest degree
     *
     * The degree of a variable is defined as the number of dependant
     * propagators. In case of ties, choose the variable with smallest
     * domain.
     */
    BVAR_DEGREE_MAX,
    /** \brief With smallest min-regret
     *
     * The min-regret of a variable is the difference between the
     * smallest and second-smallest value still in the domain.
     */
    BVAR_REGRET_MIN_MIN,
    /** \brief With largest min-regret
     *
     * The min-regret of a variable is the difference between the
     * smallest and second-smallest value still in the domain.
     */
    BVAR_REGRET_MIN_MAX,
    /** \brief With smallest max-regret
     *
     * The max-regret of a variable is the difference between the
     * largest and second-largest value still in the domain.
     */
    BVAR_REGRET_MAX_MIN,
    /** \brief With largest max-regret
     *
     * The max-regret of a variable is the difference between the
     * largest and second-largest value still in the domain.
     */
    BVAR_REGRET_MAX_MAX
  };

  /// Which values to select first for branching
  enum BvalSel {
    BVAL_MIN,      ///< Select smallest value
    BVAL_MED,      ///< Select median value
    BVAL_MAX,      ///< Select maximal value
    BVAL_SPLIT_MIN, ///< Select lower half of domain
    BVAL_SPLIT_MAX  ///< Select upper half of domain
  };

  /// Branch over all \a x with variable selection \a vars and value selection \a vals
  GECODE_INT_EXPORT void
  branch(Space* home, const IntVarArgs& x, BvarSel vars, BvalSel vals);
  GECODE_INT_EXPORT void
  branch(Space* home, const BoolVarArgs& x, BvarSel vars, BvalSel vals);
  //@}

  /**
   * \defgroup TaskIntIntAssign Assigning
   * \ingroup TaskIntInt
   */
  //@{
  /// Which value to select for assignment
  enum AvalSel {
    AVAL_MIN, ///< Select smallest value
    AVAL_MED, ///< Select median value
    AVAL_MAX  ///< Select maximum value
  };

  /// Assign all \a x with value selection \a vals
  GECODE_INT_EXPORT void
  assign(Space* home, const IntVarArgs& x, AvalSel vals);
  /// Assign all \a x with value selection \a vals
  GECODE_INT_EXPORT void
  assign(Space* home, const BoolVarArgs& x, AvalSel vals);

  //@}

}

/** \relates Gecode::REG
 * Print regular expression \a r
 */
GECODE_INT_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::REG& r);

/** \relates Gecode::DFA
 * Print DFA \a d
 */
GECODE_INT_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::DFA& d);

#endif

// STATISTICS: int-post

