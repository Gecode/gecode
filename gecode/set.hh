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

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/iter.hh>

/*
 * Configure linking
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

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_SET_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_SET_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_SET
#define GECODE_LIBRARY_NAME "Set"
#include <gecode/support/auto-link.hpp>
#endif


/**
 * \namespace Gecode::Set
 * \brief Finite integer sets
 *
 * The Gecode::Set namespace contains all functionality required
 * to program propagators and branchers for finite integer sets.
 * In addition, all propagators and branchers for finite integer
 * sets provided by %Gecode are contained as nested namespaces.
 *
 */

#include <gecode/set/exception.hpp>

namespace Gecode { namespace Set {

  /// Numerical limits for set variables
  namespace Limits {
    /// Largest allowed integer in integer set
    const int max = (Gecode::Int::Limits::max / 2) - 1;
    /// Smallest allowed integer in integer set
    const int min = -max;
    /// Maximum cardinality of an integer set
    const unsigned int card = max-min+1;
    /// Check whether integer \a n is in range, otherwise throw overflow exception with information \a l
    void check(int n, const char* l);
    /// Check whether unsigned int \a n is in range for cardinality, otherwise throw overflow exception with information \a l
    void check(unsigned int n, const char* l);
    /// Check whether minimum and maximum of IntSet \a s is in range, otherwise throw overflow exception with information \a l
    void check(const IntSet& s, const char* l);
  }

}}

#include <gecode/set/limits.hpp>

#include <gecode/set/var-imp.hpp>

namespace Gecode {

  namespace Set {
    class SetView;
  }

  /**
   * \brief %Set variables
   *
   * \ingroup TaskModelSetVars
   */
  class SetVar : public VarImpVar<Set::SetVarImp> {
    friend class SetVarArray;
    friend class SetVarArgs;
    using VarImpVar<Set::SetVarImp>::x;
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    SetVar(void);
    /// Initialize from set variable \a y
    SetVar(const SetVar& y);
    /// Initialize from set view \a y
    SetVar(const Set::SetView& y);

    /// Initialize variable with empty greatest lower and full least upper bound
    GECODE_SET_EXPORT SetVar(Space& home);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a cardMin > \a cardMax, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT
    SetVar(Space& home,int glbMin,int glbMax,int lubMin,int lubMax,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with greatest lower bound \a glbD,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a cardMin > \a cardMax, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT
    SetVar(Space& home,const IntSet& glbD,int lubMin,int lubMax,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \f$\{\mathit{glbMin},\dots,\mathit{glbMax}\}\f$,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT
    SetVar(Space& home,int glbMin,int glbMax,const IntSet& lubD,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \a lubD, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::OutOfLimits is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::OutOfLimits is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableEmptyDomain is thrown.
     */
    GECODE_SET_EXPORT
    SetVar(Space& home,const IntSet& glbD,const IntSet& lubD,
           unsigned int cardMin = 0,
           unsigned int cardMax = Set::Limits::card);
    //@}

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
    bool operator ()(void) const;
    /// Move iterator to next value (if possible)
    void operator ++(void);
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
    bool operator ()(void) const;
    /// Move iterator to next value (if possible)
    void operator ++(void);
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
    bool operator ()(void) const;
    /// Move iterator to next value (if possible)
    void operator ++(void);
    //@}

    /// \name Value access
    //@{
    /// Return current value
    int  val(void) const;
    //@}
  };

  //@}

  /**
   * \brief Print set variable \a x
   * \relates Gecode::SetVar
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const SetVar& x);

}

#include <gecode/set/view.hpp>

namespace Gecode {
  /**
   * \defgroup TaskModelSetArgs Argument arrays
   *
   * Argument arrays are just good enough for passing arguments
   * with automatic memory management.
   * \ingroup TaskModelSet
   */
  
  //@{

}

#include <gecode/set/array-traits.hpp>

namespace Gecode {

  /** \brief Passing set variables
   *
   * We could have used a simple typedef instead, but doxygen cannot
   * resolve some overloading then, leading to unusable documentation for
   * important parts of the library. As long as there is no fix for this,
   * we will keep this workaround.
   *
   */
  class SetVarArgs : public VarArgArray<SetVar> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    SetVarArgs(void) {}
    /// Allocate array with \a n elements
    explicit SetVarArgs(int n) : VarArgArray<SetVar>(n) {}
    /// Initialize from variable argument array \a a (copy elements)
    SetVarArgs(const SetVarArgs& a) : VarArgArray<SetVar>(a) {}
    /// Initialize from variable array \a a (copy elements)
    SetVarArgs(const VarArray<SetVar>& a) : VarArgArray<SetVar>(a) {}
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArgs(Space& home,int n,int glbMin,int glbMax,
               int lubMin,int lubMax,
               unsigned int minCard = 0,
               unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArgs(Space& home,int n,const IntSet& glb,
               int lubMin, int lubMax,
               unsigned int minCard = 0,
               unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArgs(Space& home,int n,int glbMin,int glbMax,
               const IntSet& lub,
               unsigned int minCard = 0,
               unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArgs(Space& home,int n,
               const IntSet& glb,const IntSet& lub,
               unsigned int minCard = 0,
               unsigned int maxCard = Set::Limits::card);
    //@}
  };
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
    /// \name Creation and initialization
    //@{
    /// Default constructor (array of size 0)
    SetVarArray(void);
    /// Initialize from set variable array \a a (share elements)    
    SetVarArray(const SetVarArray&);
    /// Initialize from set variable argument array \a a (copy elements)
    SetVarArray(Space& home, const SetVarArgs&);
    /// Allocate array for \a n set variables (variables are uninitialized)
    GECODE_SET_EXPORT SetVarArray(Space& home, int n);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArray(Space& home,int n,int glbMin,int glbMax,int lubMin,int lubMax,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArray(Space& home,int n,const IntSet& glb, int lubMin, int lubMax,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArray(Space& home,int n,int glbMin,int glbMax,const IntSet& lub,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_SET_EXPORT
    SetVarArray(Space& home,int n,
                const IntSet& glb,const IntSet& lub,
                unsigned int minCard = 0,
                unsigned int maxCard = Set::Limits::card);
    //@}
  };

}

#include <gecode/set/array.hpp>

namespace Gecode {

  /**
   * \brief Common relation types for sets
   *
   * The total order on sets is defined as the lexicographic
   * order on their characteristic functions, e.g.,
   * \f$x\leq y\f$ means that either \f$x\f$ is empty or
   * the minimal element of the symmetric difference
   * \f$x\ominus y\f$ is in \f$y\f$.
   *
   * \ingroup TaskModelSet
   */
  enum SetRelType {
    SRT_EQ,   ///< Equality (\f$=\f$)
    SRT_NQ,   ///< Disequality (\f$\neq\f$)
    SRT_SUB,  ///< Subset (\f$\subseteq\f$)
    SRT_SUP,  ///< Superset (\f$\supseteq\f$)
    SRT_DISJ, ///< Disjoint (\f$\parallel\f$)
    SRT_CMPL, ///< Complement
    SRT_LQ,   ///< Less or equal (\f$\leq\f$)
    SRT_LE,   ///< Less (\f$<\f$)
    SRT_GQ,   ///< Greater or equal (\f$\geq\f$)
    SRT_GR    ///< Greater (\f$>\f$)
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
  dom(Home home, SetVar x, SetRelType r, int i);

  /// Propagates \f$ x \sim_r \{i,\dots,j\}\f$
  GECODE_SET_EXPORT void
  dom(Home home, SetVar x, SetRelType r, int i, int j);

  /// Propagates \f$ x \sim_r s\f$
  GECODE_SET_EXPORT void
  dom(Home home, SetVar x, SetRelType r, const IntSet& s);

  /// Post propagator for \f$ (x \sim_r \{i\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Home home, SetVar x, SetRelType r, int i, BoolVar b);

  /// Post propagator for \f$ (x \sim_r \{i,\dots,j\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Home home, SetVar x, SetRelType r, int i, int j, BoolVar b);

  /// Post propagator for \f$ (x \sim_r s) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  dom(Home home, SetVar x, SetRelType r, const IntSet& s, BoolVar b);

  /// Propagates \f$ i \leq |s| \leq j \f$
  GECODE_SET_EXPORT void
  cardinality(Home home, SetVar x, unsigned int i, unsigned int j);

  //@}


  /**
   * \defgroup TaskModelSetRel Relation constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetRelType r, SetVar y);

  /// Post propagator for \f$ (x \sim_r y) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetRelType r, SetVar y, BoolVar b);

  /// Post propagator for \f$ s \sim_r \{x\}\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar s, SetRelType r, IntVar x);

  /// Post propagator for \f$ \{x\} \sim_r s\f$
  GECODE_SET_EXPORT void
  rel(Home home, IntVar x, SetRelType r, SetVar s);

  /// Post propagator for \f$ (s \sim_r \{x\}) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar s, SetRelType r, IntVar x, BoolVar b);

  /// Post propagator for \f$ (\{x\} \sim_r s) \Leftrightarrow b \f$
  GECODE_SET_EXPORT void
  rel(Home home, IntVar x, SetRelType r, SetVar s, BoolVar b);

  /// Post propagator for \f$|s|\geq 1 \land \forall i\in s:\ i \sim_r x\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar s, IntRelType r, IntVar x);

  /// Post propagator for \f$|s|\geq 1 \land \forall i\in s:\ x \sim_r i\f$
  GECODE_SET_EXPORT void
  rel(Home home, IntVar x, IntRelType r, SetVar s);

  //@}

  /**
   * \defgroup TaskModelSetRelOp Set operation/relation constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetOpType op, SetVar y, SetRelType r, SetVar z);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetOpType op, const SetVarArgs& x, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x \diamond_{\mathit{op}} z\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetOpType op, const SetVarArgs& x, const IntSet& z, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x \diamond_{\mathit{op}} z\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetOpType op, const IntVarArgs& x, const IntSet& z, SetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_SET_EXPORT void
  rel(Home home, SetOpType op, const IntVarArgs& x, SetVar y);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, const IntSet& x, SetOpType op, SetVar y,
      SetRelType r, SetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetOpType op, const IntSet& y,
      SetRelType r, SetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetOpType op, SetVar y,
      SetRelType r, const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, const IntSet& x, SetOpType op, SetVar y, SetRelType r,
      const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_SET_EXPORT void
  rel(Home home, SetVar x, SetOpType op, const IntSet& y, SetRelType r,
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
  convex(Home home, SetVar x);

  /// Post propagator that propagates that \a y is the convex hull of \a x
  GECODE_SET_EXPORT void
  convex(Home home, SetVar x, SetVar y);

  //@}

  /**
   * \defgroup TaskModelSetSequence Sequence constraints
   * \ingroup TaskModelSet
   *
   */
  //@{

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
  GECODE_SET_EXPORT void
  sequence(Home home, const SetVarArgs& x);

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$
  GECODE_SET_EXPORT void
  sequence(Home home, const SetVarArgs& y, SetVar x);

  //@}

  /**
   * \defgroup TaskModelSetDistinct Distinctness constraints
   * \ingroup TaskModelSet
   *
   */
  //@{


  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
  GECODE_SET_EXPORT void
  atmostOne(Home home, const SetVarArgs& x, unsigned int c);

  //@}

  /**
   * \defgroup TaskModelSetConnect Connection constraints to integer variables
   * \ingroup TaskModelSet
   *
   */

  //@{

  /** \brief Post propagator that propagates that \a x is the
   *  minimal element of \a s, and that \a s is not empty */
  GECODE_SET_EXPORT void
  min(Home home, SetVar s, IntVar x);

  /** \brief Post propagator that propagates that \a x is not the
   *  minimal element of \a s */
  GECODE_SET_EXPORT void
  notMin(Home home, SetVar s, IntVar x);

  /** \brief Post reified propagator for \a b iff \a x is the
   *  minimal element of \a s */
  GECODE_SET_EXPORT void
  min(Home home, SetVar s, IntVar x, BoolVar b);

  /** \brief Post propagator that propagates that \a x is the
   *  maximal element of \a s, and that \a s is not empty */
  GECODE_SET_EXPORT void
  max(Home home, SetVar s, IntVar x);

  /** \brief Post propagator that propagates that \a x is not the
   *  maximal element of \a s */
  GECODE_SET_EXPORT void
  notMax(Home home, SetVar s, IntVar x);

  /** \brief Post reified propagator for \a b iff \a x is the
   *  maximal element of \a s */
  GECODE_SET_EXPORT void
  max(Home home, SetVar s, IntVar x, BoolVar b);

  /// Post propagator for \f$\{x_0,\dots,x_{n-1}\}=y\f$ and \f$x_i<x_{i+1}\f$
  GECODE_SET_EXPORT void
  channelSorted(Home home, const IntVarArgs& x, SetVar y);

  /// Post propagator for \f$x_i=j \Leftrightarrow i\in y_j\f$
  GECODE_SET_EXPORT void
  channel(Home home, const IntVarArgs& x,const SetVarArgs& y);

  /// Post propagator for \f$x_i=1 \Leftrightarrow i\in y\f$
  GECODE_SET_EXPORT void
  channel(Home home, const BoolVarArgs& x, SetVar y);

  /// Post propagator for \f$ |s|=x \f$
  GECODE_SET_EXPORT void
  cardinality(Home home, SetVar s, IntVar x);


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
  weights(Home home, IntSharedArray elements, IntSharedArray weights,
          SetVar x, IntVar y);

  //@}

  /**
   * \defgroup TaskModelSetPrecede Value precedence constraints over set variables
   * \ingroup TaskModelSet
   */
  /** \brief Post propagator that \a s precedes \a t in \a x
   *
   * This constraint enforces that if there exists \f$j\f$ such that
   * \f$s\notin x_j\land t\in x_j\f$, then there exists \f$i<j\f$ such that
   * \f$s\in x_i\land t\notin x_i\f$.
   * \ingroup TaskModelSetPrecede
   */
  GECODE_SET_EXPORT void
  precede(Home home, const SetVarArgs& x, int s, int t);
  /** \brief Post propagator that successive values in \a c precede each other in \a x
   * \ingroup TaskModelSetPrecede
   */
  GECODE_SET_EXPORT void
  precede(Home home, const SetVarArgs& x, const IntArgs& c);

  /**
   * \defgroup TaskModelSetElement Element constraints
   * \ingroup TaskModelSet
   *
   * An element constraint selects zero, one or more elements out of a
   * sequence. We write \f$ \langle x_0,\dots, x_{n-1} \rangle \f$ for the
   * sequence, and \f$ [y] \f$ for the index variable.
   *
   * Set element constraints are closely related to the ::element constraint
   * on integer variables.
   */

  //@{

  /**
   * \brief Post propagator for \f$ z=\diamond_{\mathit{op}}\langle x_0,\dots,x_{n-1}\rangle[y] \f$
   *
   * If \a y is the empty set, the usual conventions for set operations apply:
   * an empty union is empty, while an empty intersection is the universe,
   * which can be given as the optional parameter \a u.
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, SetOpType op, const SetVarArgs& x, SetVar y, SetVar z,
    const IntSet& u = IntSet(Set::Limits::min,Set::Limits::max));

  /**
   * \brief Post propagator for \f$ z=\diamond_{\mathit{op}}\langle \{x_0\},\dots,\{x_{n-1}\}\rangle[y] \f$
   *
   * If \a y is the empty set, the usual conventions for set operations apply:
   * an empty union is empty, while an empty intersection is the universe,
   * which can be given as the optional parameter \a u.
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, SetOpType op, const IntVarArgs& x, SetVar y, SetVar z,
          const IntSet& u = IntSet(Set::Limits::min,Set::Limits::max));

  /**
   * \brief Post propagator for \f$ z=\diamond_{\mathit{op}}\langle x_0,\dots,x_{n-1}\rangle[y] \f$
   *
   * If \a y is the empty set, the usual conventions for set operations apply:
   * an empty union is empty, while an empty intersection is the universe,
   * which can be given as the optional parameter \a u.
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, SetOpType op, const IntSetArgs& x, SetVar y, SetVar z,
          const IntSet& u = IntSet(Set::Limits::min,Set::Limits::max));

  /**
   * \brief Post propagator for \f$ z=\diamond_{\mathit{op}}\langle \{x_0\},\dots,\{x_{n-1}\}\rangle[y] \f$
   *
   * If \a y is the empty set, the usual conventions for set operations apply:
   * an empty union is empty, while an empty intersection is the universe,
   * which can be given as the optional parameter \a u.
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, SetOpType op, const IntArgs& x, SetVar y, SetVar z,
          const IntSet& u = IntSet(Set::Limits::min,Set::Limits::max));

  /**
   * \brief Post propagator for \f$ z=\langle x_0,\dots,x_{n-1}\rangle[y] \f$
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, const SetVarArgs& x, IntVar y, SetVar z);

  /**
   * \brief Post propagator for \f$ z=\langle s_0,\dots,s_{n-1}\rangle[y] \f$
   *
   * The indices for \a y start at 0.
   */
  GECODE_SET_EXPORT void
  element(Home home, const IntSetArgs& s, IntVar y, SetVar z);

  /** \brief Post propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * Throws an exception of type Set::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_SET_EXPORT void
  element(Home home, const IntSetArgs& a, 
          IntVar x, int w, IntVar y, int h, SetVar z);
  /** \brief Post propagator for \f$ a_{x+w\cdot y}=z\f$
   *
   * Throws an exception of type Set::ArgumentSizeMismatch, if
   * \f$ w\cdot h\neq|a|\f$.
   */
  GECODE_SET_EXPORT void
  element(Home home, const SetVarArgs& a, 
          IntVar x, int w, IntVar y, int h, SetVar z);
  //@}

  /**
   * \defgroup TaskModelSetExec Synchronized execution
   * \ingroup TaskModelSet
   *
   * Synchronized execution executes a function or a static member function
   * when a certain event happends.
   *
   * \ingroup TaskModelSet
   */
  //@{
  /// Execute \a c when \a x becomes assigned
  GECODE_SET_EXPORT void
  wait(Home home, SetVar x, void (*c)(Space& home));
  /// Execute \a c when all variables in \a x become assigned
  GECODE_SET_EXPORT void
  wait(Home home, const SetVarArgs& x, void (*c)(Space& home));
  //@}

  /**
   * \defgroup TaskModelSetBranch Branching
   * \ingroup TaskModelSet
   */

  //@{
  /// Which variable to select for branching
  enum SetVarBranch {
    SET_VAR_NONE = 0,   ///< First unassigned
    SET_VAR_RND,        ///< Random (uniform, for tie breaking)
    SET_VAR_DEGREE_MIN, ///< With smallest degree
    SET_VAR_DEGREE_MAX, ///< With largest degree
    SET_VAR_AFC_MIN,    ///< With smallest accumulated failure count
    SET_VAR_AFC_MAX,    ///< With largest accumulated failure count
    SET_VAR_MIN_MIN,    ///< With smallest minimum unknown element
    SET_VAR_MIN_MAX,    ///< With largest minimum unknown element
    SET_VAR_MAX_MIN,    ///< With smallest maximum unknown element
    SET_VAR_MAX_MAX,    ///< With largest maximum unknown element
    SET_VAR_SIZE_MIN,   ///< With smallest unknown set
    SET_VAR_SIZE_MAX,   ///< With largest unknown set
    SET_VAR_SIZE_DEGREE_MIN, ///< With smallest domain size divided by degree
    SET_VAR_SIZE_DEGREE_MAX, ///< With largest domain size divided by degree
    SET_VAR_SIZE_AFC_MIN, ///< With smallest domain size divided by accumulated failure count
    SET_VAR_SIZE_AFC_MAX  ///< With largest domain size divided by accumulated failure count
  };

  /// Which values to select first for branching
  enum SetValBranch {
    SET_VAL_MIN_INC, ///< Include smallest element
    SET_VAL_MIN_EXC, ///< Exclude smallest element
    SET_VAL_MED_INC, ///< Include median element (rounding downwards)
    SET_VAL_MED_EXC, ///< Exclude median element (rounding downwards)
    SET_VAL_MAX_INC, ///< Include largest element
    SET_VAL_MAX_EXC, ///< Exclude largest element
    SET_VAL_RND_INC, ///< Include random element
    SET_VAL_RND_EXC  ///< Exclude random element
  };

  /// Branch over \a x with variable selection \a vars and value selection \a vals
  GECODE_SET_EXPORT void
  branch(Home home, const SetVarArgs& x,
         SetVarBranch vars, SetValBranch vals,
         const VarBranchOptions& o_vars = VarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
  GECODE_SET_EXPORT void
  branch(Home home, const SetVarArgs& x,
         const TieBreakVarBranch<SetVarBranch>& vars, SetValBranch vals,
         const TieBreakVarBranchOptions& o_vars = TieBreakVarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with value selection \a vals
  GECODE_SET_EXPORT void
  branch(Home home, SetVar x, SetValBranch vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  //@}

  /**
   * \defgroup TaskModelSetAssign Assigning
   * \ingroup TaskModelSet
   */
  //@{
  /// Which value to select for assignment
  enum SetAssign {
    SET_ASSIGN_MIN_INC, ///< Include smallest element
    SET_ASSIGN_MIN_EXC, ///< Exclude smallest element
    SET_ASSIGN_MED_INC, ///< Include median element (rounding downwards)
    SET_ASSIGN_MED_EXC, ///< Exclude median element (rounding downwards)
    SET_ASSIGN_MAX_INC, ///< Include largest element
    SET_ASSIGN_MAX_EXC, ///< Exclude largest element
    SET_ASSIGN_RND_INC, ///< Include random element
    SET_ASSIGN_RND_EXC  ///< Exclude random element
  };

  /// Assign all \a x with value selection \a vals
  GECODE_SET_EXPORT void
  assign(Home home, const SetVarArgs& x, SetAssign vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Assign \a x with value selection \a vals
  GECODE_SET_EXPORT void
  assign(Home home, SetVar x, SetAssign vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);

  //@}

}

#endif

// IFDEF: GECODE_HAS_SET_VARS
// STATISTICS: set-post
