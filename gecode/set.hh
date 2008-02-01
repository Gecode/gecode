/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

#ifndef __GECODE_SET_HH__
#define __GECODE_SET_HH__

#include "gecode/config.hh"

namespace Gecode { namespace Set {
  /** 
   * \namespace Gecode::Set
   * \brief Finite integer sets
   * 
   * The Gecode::Set namespace contains all functionality required
   * to program propagators and branchings for finite integer sets.
   * In addition, all propagators and branchings for finite integer
   * sets provided by %Gecode are contained as nested namespaces.
   *
   */
}}

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_SET
#define GECODE_SET_EXPORT __declspec( dllexport )
#else
#define GECODE_SET_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_SET_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_SET_EXPORT

#endif
#endif

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/iter.hh"

#include "gecode/set/exception.icc"

namespace Gecode { namespace Set {

  /// Numerical limits for set variables
  namespace Limits {
    /// Largest allowed integer in integer set
    const int int_max = (Gecode::Int::Limits::int_max / 2) - 1;
    /// Smallest allowed integer in integer set
    const int int_min = -int_max;
    /// Maximum cardinality of an integer set
    const unsigned int card_max = int_max-int_min+1;
    /// Check whether integer \a n is in range, otherwise throw overflow exception with information \a l
    void check(int n, const char* l);
    /// Check whether unsigned int \a n is in range, otherwise throw overflow exception with information \a l
    void check(unsigned int n, const char* l);
  }

}}

#include "gecode/set/limits.icc"

#include "gecode/set/var-imp.icc"

namespace Gecode {
  
  namespace Set {
    class SetView;
  }

  /**
   * \brief %Set variables
   * 
   * \ingroup TaskModelSetVars
   */
  class SetVar {
  private:
    /// Finite set variable implementation used
    Set::SetVarImp* x;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVar(void);
    /// Initialize from set variable \a x0
    SetVar(const SetVar& x0);
    /// Initialize from set view \a x0
    SetVar(const Set::SetView& x0);
    /// Initialize from reflection variable \a x0
    SetVar(const Reflection::Var& x0);

    /// Initialize variable with empty greatest lower and full least upper bound
    GECODE_SET_EXPORT SetVar(Space* home);
    /// Initialize variable with empty greatest lower and full least upper bound
    void init(Space* home);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT 
    SetVar(Space* home,int glbMin,int glbMax,int lubMin,int lubMax,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card_max);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    void init(Space* home,int glbMin,int glbMax,int lubMin,int lubMax,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card_max);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT 
    SetVar(Space* home,const IntSet& glbD,int lubMin,int lubMax,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card_max);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    void init(Space* home,const IntSet& glbD,int lubMin,int lubMax,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card_max);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT 
    SetVar(Space* home,int glbMin,int glbMax,const IntSet& lubD,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card_max);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    void init(Space* home,int glbMin,int glbMax,const IntSet& lubD,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card_max);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT 
    SetVar(Space* home,const IntSet& glbD,const IntSet& lubD,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card_max);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::int_min
     *    and Set::Limits::int_max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    void init(Space* home,const IntSet& glbD,const IntSet& lubD,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card_max);
    //@}

    /// \name Variable implementation access
    //@{
    /// Return set variable implementation
    Set::SetVarImp* var(void) const;
    ///@}
    
    /// \name Value access
    //@{
    /// Return number of elements in the greatest lower bound
    unsigned int glbSize(void) const;
    /// Return number of elements in the least upper bound
    unsigned int lubSize(void) const;
    /// Return number of unknown elements (elements in lub but not in glb)
    unsigned int unknownSize(void) const;
    /// Return cardinality minimum
    unsigned int cardMin(void) const;
    /// Return cardinality maximum
    unsigned int cardMax(void) const;
    /// Return minimum element of least upper bound
    int lubMin(void) const;
    /// Return maximum element of least upper bound
    int lubMax(void) const;
    /// Return minimum element of greatest lower bound
    int glbMin(void) const;
    /// Return maximum of greatest lower bound
    int glbMax(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether \a i is in greatest lower bound
    bool contains(int i) const;
    /// Test whether \a i is not in the least upper bound
    bool notContains(int i) const;
    /// Test whether this variable is assigned
    bool assigned(void) const;

    /// \name Cloning
    //@{
    /// Update this variable to be a clone of variable \a x
    void update(Space* home, bool, SetVar& x);
    //@}
  };

  /**
   * \defgroup TaskModelSetIter Range and value iterators for set variables
   * \ingroup TaskModelSet
   */
  //@{

  /// Iterator for the greatest lower bound ranges of a set variable
  class SetVarGlbRanges {
  private:
    Set::GlbRanges<Set::SetVarImp*> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarGlbRanges(void);
    /// Initialize to iterate ranges of variable \a x
    SetVarGlbRanges(const SetVar& x);
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

  /// Iterator for the least upper bound ranges of a set variable
  class SetVarLubRanges {
  private:
    Set::LubRanges<Set::SetVarImp*> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarLubRanges(void);
    /// Initialize to iterate ranges of variable \a x
    SetVarLubRanges(const SetVar& x);
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

  /// Iterator for the unknown ranges of a set variable
  class SetVarUnknownRanges {
  private:
    Set::UnknownRanges<Set::SetVarImp*> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarUnknownRanges(void);
    /// Initialize to iterate ranges of variable \a x
    SetVarUnknownRanges(const SetVar& x);
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
  
  /// Iterator for the values in the greatest lower bound of a set variable
  class SetVarGlbValues {
  private:
    Iter::Ranges::ToValues<SetVarGlbRanges> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarGlbValues(void);
    /// Initialize to iterate values of variable \a x
    SetVarGlbValues(const SetVar& x);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a value or done
    bool operator()(void) const;
    /// Move iterator to next value (if possible)
    void operator++(void);
    //@}

    /// \name Value access
    //@{
    /// Return current value
    int  val(void) const;
    //@}
  };

  /// Iterator for the values in the least upper bound of a set variable
  class SetVarLubValues {
  private:
    Iter::Ranges::ToValues<SetVarLubRanges> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarLubValues(void);
    /// Initialize to iterate values of variable \a x
    SetVarLubValues(const SetVar& x);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a value or done
    bool operator()(void) const;
    /// Move iterator to next value (if possible)
    void operator++(void);
    //@}

    /// \name Value access
    //@{
    /// Return current value
    int  val(void) const;
    //@}
  };

  /// Iterator for the values in the unknown set of a set variable
  class SetVarUnknownValues {
  private:
    Iter::Ranges::ToValues<SetVarUnknownRanges> iter;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVarUnknownValues(void);
    /// Initialize to iterate values of variable \a x
    SetVarUnknownValues(const SetVar& x);
    //@}

    /// \name Iteration control
    //@{
    /// Test whether iterator is still at a value or done
    bool operator()(void) const;
    /// Move iterator to next value (if possible)
    void operator++(void);
    //@}

    /// \name Value access
    //@{
    /// Return current value
    int  val(void) const;
    //@}
  };

  //@}

}

/**
 * \brief Print set variable \a x
 * \relates Gecode::SetVar
 */
GECODE_SET_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::SetVar& x);

#include "gecode/set/view.icc"
#include "gecode/set/propagator.icc"

namespace Gecode {
  /**
   * \defgroup TaskModelSetArgs Argument arrays
   * 
   * Argument arrays are just good enough for passing arguments
   * with automatic memory management.
   * \ingroup TaskModelSet
   */

  //@{
  /// Passing set arguments
  typedef PrimArgArray<IntSet> IntSetArgs;
  /// Passing set variables
  typedef VarArgArray<SetVar>  SetVarArgs;
  //@}

  /**
   * \defgroup TaskModelSetVarArrays Variable arrays
   * 
   * Variable arrays can store variables. They are typically used
   * for storing the variables being part of a solution. However,
   * they can also be used for temporary purposes (even though
   * memory is not reclaimed until the space it is created for
   * is deleted).
   * \ingroup TaskModelSet
   */

  /**
   * \brief %Set variable array
   * \ingroup TaskModelSetVarArrays
   */
  class SetVarArray : public VarArray<SetVar> {
  public:
    SetVarArray(void);
    SetVarArray(const SetVarArray&);
    /// Create an uninitialized array of size \a n
    GECODE_SET_EXPORT SetVarArray(Space* home,int n);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT 
    SetVarArray(Space* home,int n,int glbMin,int glbMax,int lubMin,int lubMax,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card_max);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT 
    SetVarArray(Space* home,int n,const IntSet& glb, int lubMin, int lubMax,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card_max);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT 
    SetVarArray(Space* home,int n,int glbMin,int glbMax,const IntSet& lub,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card_max);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT 
    SetVarArray(Space* home,int n,
                const IntSet& glb,const IntSet& lub,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card_max);
  };

}

#include "gecode/set/array.icc"

namespace Gecode {

  /**
   * \brief Common relation types for sets
   * \ingroup TaskModelSet
   */
  enum SetRelType {
    SRT_EQ,   ///< Equality (\f$=\f$)
    SRT_NQ,   ///< Disequality (\f$\neq\f$)
    SRT_SUB,  ///< Subset (\f$\subseteq\f$)
    SRT_SUP,  ///< Superset (\f$\supseteq\f$)
    SRT_DISJ, ///< Disjoint (\f$\parallel\f$)
    SRT_CMPL  ///< Complement
  };

  /**
   * \brief Common operations for sets
   * \ingroup TaskModelSet
   */
  enum SetOpType {
    SOT_UNION,  ///< Union
    SOT_DUNION, ///< Disjoint union
    SOT_INTER,  ///< %Intersection
    SOT_MINUS   ///< Difference
  };

  /**
   * \defgroup TaskModelSetDom Domain constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Propagates \f$ x \sim_r \{i\}\f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, int i);

  /// Propagates \f$ x \sim_r \{i,\dots,j\}\f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, int i, int j);

  /// Propagates \f$ x \sim_r s\f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, const IntSet& s);

  /// Post propagator for \f$ (x \sim_r \{i\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, int i, BoolVar b);

  /// Post propagator for \f$ (x \sim_r \{i,\dots,j\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, int i, int j, BoolVar b);

  /// Post propagator for \f$ (x \sim_r s) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Space* home, SetVar x, SetRelType r, const IntSet& s, BoolVar b);

  /// Propagates \f$ i \leq |s| \leq j \f$
  GECODE_SET_EXPORT void
  cardinality(Space* home, SetVar x, unsigned int i, unsigned int j);

  //@}


  /**
   * \defgroup TaskModelSetRel Relation constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetRelType r, SetVar y);

  /// Post propagator for \f$ (x \sim_r y) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetRelType r, SetVar y, BoolVar b);

  /// Post propagator for \f$ (x \sim_r s) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetRelType r, const IntSet& s, BoolVar b);  

  /// Post propagator for \f$ s \sim_r \{x\}\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar s, SetRelType r, IntVar x);

  /// Post propagator for \f$ \{x\} \sim_r s\f$
  GECODE_SET_EXPORT void
  rel(Space* home, IntVar x, SetRelType r, SetVar s);

  /// Post propagator for \f$ (s \sim_r \{x\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar s, SetRelType r, IntVar x, BoolVar b);

  /// Post propagator for \f$ (\{x\} \sim_r s) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Space* home, IntVar x, SetRelType r, SetVar s, BoolVar b);

  /// Post propagator for \f$\forall i\in s:\ i \sim_r x\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar s, IntRelType r, IntVar x);

  /// Post propagator for \f$\forall i\in s:\ x \sim_r i\f$
  GECODE_SET_EXPORT void
  rel(Space* home, IntVar x, IntRelType r, SetVar s);

  //@}

  /**
   * \defgroup TaskModelSetRelOp Set operation/relation constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetOpType op, SetVar y, SetRelType r, SetVar z);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetOpType op, const SetVarArgs& x, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x \diamond_{\mathit{op}} z\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetOpType op, const SetVarArgs& x, const IntSet& z, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x \diamond_{\mathit{op}} z\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetOpType op, const IntVarArgs& x, const IntSet& z, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetOpType op, const IntVarArgs& x, SetVar y);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, const IntSet& x, SetOpType op, SetVar y,
      SetRelType r, SetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetOpType op, const IntSet& y,
      SetRelType r, SetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetOpType op, SetVar y,
      SetRelType r, const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, const IntSet& x, SetOpType op, SetVar y, SetRelType r,
      const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Space* home, SetVar x, SetOpType op, const IntSet& y, SetRelType r,
      const IntSet& z);

  //@}


  /**
   * \defgroup TaskModelSetConvex Convexity constraints
   * \ingroup TaskModelSet
   *
   */
  //@{

  /// Post propagator that propagates that \a x is convex 
  GECODE_SET_EXPORT void
  convex(Space* home, SetVar x);

  /// Post propagator that propagates that \a y is the convex hull of \a x
  GECODE_SET_EXPORT void
  convexHull(Space* home, SetVar x, SetVar y);

  //@}

  /**
   * \defgroup TaskModelSetSequence Sequence constraints
   * \ingroup TaskModelSet
   *
   */
  //@{

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
  GECODE_SET_EXPORT void
  sequence(Space* home, const SetVarArgs& x);

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
  GECODE_SET_EXPORT void
  sequentialUnion(Space* home, const SetVarArgs& y, SetVar x);

  //@}

  /**
   * \defgroup TaskModelSetDistinct Distinctness constraints
   * \ingroup TaskModelSet
   *
   */
  //@{


  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
  GECODE_SET_EXPORT void
  atmostOne(Space* home, const SetVarArgs& x, unsigned int c);

  //@}

  /**
   * \defgroup TaskModelSetConnect Connection constraints to finite domain variables
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Post propagator that propagates that \a x is the minimal element of \a s 
  GECODE_SET_EXPORT void
  min(Space* home, SetVar s, IntVar x);
  
  /// Post propagator that propagates that \a x is the maximal element of \a s 
  GECODE_SET_EXPORT void
  max(Space* home, SetVar s, IntVar x);
  
  /// Post propagator that propagates that \a s contains the \f$x_i\f$, which are sorted in non-descending order 
  GECODE_SET_EXPORT void
  match(Space* home, SetVar s, const IntVarArgs& x);
  
  /// Post propagator for \f$x_i=j \Leftrightarrow i\in y_j\f$
  GECODE_SET_EXPORT void
  channel(Space* home, const IntVarArgs& x,const SetVarArgs& y);

  /// Post propagator for \f$x_i=1 \Leftrightarrow i\in y\f$
  GECODE_SET_EXPORT void
  channel(Space* home, const BoolVarArgs& x, SetVar y);
  
  /// Post propagator for \f$ |s|=x \f$ 
  GECODE_SET_EXPORT void
  cardinality(Space* home, SetVar s, IntVar x);


  /**
   * \brief Post propagator for \f$y = \mathrm{weight}(x)\f$
   *
   * The weights are given as pairs of elements and their weight:
   * \f$\mathrm{weight}(\mathrm{elements}_i) = \mathrm{weights}_i\f$
   *
   * The upper bound of \a x is constrained to contain only elements from
   * \a elements. The weight of a set is the sum of the weights of its
   * elements.
   */
  GECODE_SET_EXPORT void
  weights(Space* home, const IntArgs& elements, const IntArgs& weights,
          SetVar x, IntVar y);

  //@}

  /**
   * \defgroup TaskModelSetSelection Selection constraints
   * \ingroup TaskModelSet
   *
   * A selection constraint selects zero, one or more elements out of a 
   * sequence. We write \f$ \langle x_0,\dots, x_{n-1} \rangle \f$ for the
   * sequence, and \f$ [y] \f$ for the selector variable.
   *
   * Set selection constraints are closely related to the ::element constraint
   * on finite domain variables.
   */

  //@{

  /**
   * \brief Post propagator for \f$ z=\bigcup\langle x_0,\dots,x_{n-1}\rangle[y+\mathit{offset}] \f$
   *
   * If \a y is the empty set, \a z will also be constrained to be empty
   * (as an empty union is empty).
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectUnion(Space* home, const SetVarArgs& x, SetVar y, SetVar z);

  /**
   * \brief Post propagator for \f$ z=\bigcup\langle s_0,\dots,s_{n-1}\rangle[y+\mathit{offset}] \f$
   *
   * If \a y is the empty set, \a z will also be constrained to be empty
   * (as an empty union is empty).
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectUnion(Space* home, const IntSetArgs& s, SetVar y, SetVar z);

  /** 
   * \brief Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y+\mathit{offset}] \f$ using \f$ \mathcal{U} \f$ as universe
   *
   * If \a y is empty, \a z will be constrained to be the universe
   * \f$ \mathcal{U} \f$ (as an empty intersection is the universe).
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectInter(Space* home, const SetVarArgs& x, SetVar y, SetVar z);

  /** 
   * \brief Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y+\mathit{offset}] \f$ using \a u as universe
   *
   * If \a y is empty, \a z will be constrained to be the given universe
   * \a u (as an empty intersection is the universe).
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectInterIn(Space* home, const SetVarArgs& x, SetVar y, SetVar z,
                const IntSet& u);

  /// Post propagator for \f$ \parallel\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
  GECODE_SET_EXPORT void
  selectDisjoint(Space* home, const SetVarArgs& x, SetVar y);

  /**
   * \brief Post propagator for \f$ z=\langle x_0,\dots,x_{n-1}\rangle[y+\mathit{offset}] \f$ 
   *
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectSet(Space* home, const SetVarArgs& x, IntVar y, SetVar z);

  /**
   * \brief Post propagator for \f$ z=\langle s_0,\dots,s_{n-1}\rangle[y+\mathit{offset}] \f$ 
   *
   * The indices for \a s start at 0, unless an \a offset is specified.
   */
  GECODE_SET_EXPORT void
  selectSet(Space* home, const IntSetArgs& s, IntVar y, SetVar z);

  //@}

  /**
   * \defgroup TaskModelSetBranch Branching
   * \ingroup TaskModelSet
   */

  //@{
  
  /// Which variable to select for branching
  enum SetVarBranch {
    SET_VAR_NONE,               ///< First unassigned
    SET_VAR_MIN_CARD,           ///< With smallest unknown set
    SET_VAR_MAX_CARD,           ///< With largest unknown set
    SET_VAR_MIN_UNKNOWN_ELEM,   ///< With smallest unknown element
    SET_VAR_MAX_UNKNOWN_ELEM,   ///< With largest unknown element
  };
  
  /// Which values to select first for branching
  enum SetValBranch {
    SET_VAL_MIN,                ///< Select smallest value in unknown set
    SET_VAL_MAX,                ///< Select largest value in unknown set
  };

  /// Branch over \a x with variable selection \a vars and value selection \a vals
  GECODE_SET_EXPORT void
  branch(Space* home, const SetVarArgs& x, 
         SetVarBranch vars, SetValBranch vals);
  //@}

}

#endif

// IFDEF: GECODE_HAVE_SET_VARS
// STATISTICS: set-post
