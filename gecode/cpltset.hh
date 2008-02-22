/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#ifndef __GECODE_CPLTSET_HH__
#define __GECODE_CPLTSET_HH__

#include "gecode/kernel.hh"
#include "gecode/iter.hh"
#include "gecode/int.hh"
#include "gecode/set.hh"

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_CPLTSET
#define GECODE_CPLTSET_EXPORT __declspec( dllexport )
#else
#define GECODE_CPLTSET_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_CPLTSET_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_CPLTSET_EXPORT
#endif

#endif

#include "gecode/cpltset/exception.icc"

/** 
 * \namespace Gecode::CpltSet
 * \brief Complete representation of finite integer sets using ROBDD's
 * 
 * The Gecode::CpltSet namespace contains all functionality required
 * to program propagators and branchings for finite integer set variables,
 * where the variables' domains are represented completely (without 
 * approximation) using reduced ordered binary decision diagrams (ROBDDs).
 *
 * In addition, all propagators and branchings for these variables
 * provided by %Gecode are contained as nested namespaces.
 *
 */

#include "gecode/cpltset/bddmanager.icc"

#include "gecode/cpltset/support.icc"
#include "gecode/cpltset/var-imp.icc"

namespace Gecode { 
  /**
   * \brief Bdd variable as complete domain representation for finite integer sets
   *
   */  
  class CpltSetVar : public VarBase<CpltSet::CpltSetVarImp> {
  private:
    using VarBase<CpltSet::CpltSetVarImp>::varimp;  
  public:
    /// Default constructor
    CpltSetVar(void);

    /// Deallocate memory
    void dispose(Space* home);

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
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVar(Space* home,
               int glbMin,int glbMax,int lubMin,int lubMax,
               unsigned int cardMin = 0,
               unsigned int cardMax = Set::Limits::card);
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
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    void init(Space* home,
              int glbMin,int glbMax,int lubMin,int lubMax,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card);

    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVar(Space* home,
               const IntSet& glbD,int lubMin,int lubMax,
               unsigned int cardMin = 0,
               unsigned int cardMax = Set::Limits::card);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with
     * greatest lower bound \a glbD,
     * least upper bound \f$\{\mathit{lubMin},\dots,\mathit{lubMax}\}\f$, and
     * cardinality minimum \a cardMin and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    void init(Space* home,
              const IntSet& glbD,int lubMin,int lubMax,
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
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVar(Space* home,
               int glbMin,int glbMax,const IntSet& lubD,
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
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    void init(Space* home,
              int glbMin,int glbMax,const IntSet& lubD,
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
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVar(Space* home,
               const IntSet& glbD,const IntSet& lubD,
               unsigned int cardMin = 0,
               unsigned int cardMax = Set::Limits::card);
    /**
     * \brief Initialize variable with given bounds and cardinality
     *
     * The variable is created with greatest lower bound \a glbD,
     * least upper bound \a lubD, and cardinality minimum \a cardMin 
     * and maximum \a cardMax.
     * The following exceptions might be thrown:
     *  - If the bounds are no legal set bounds (between Set::Limits::min
     *    and Set::Limits::max), an exception of type
     *    Gecode::Set::VariableOutOfRangeDomain is thrown.
     *  - If the cardinality is greater than Set::Limits::max_set_size, an
     *    exception of type Gecode::Set::VariableOutOfRangeCardinality is
     *    thrown.
     *  - If \a minCard > \a maxCard, an exception of type
     *    Gecode::Set::VariableFailedDomain is thrown.
     */
    void init(Space* home,
              const IntSet& glbD,const IntSet& lubD,
              unsigned int cardMin = 0,
              unsigned int cardMax = Set::Limits::card);
    //@}

    /// \name Value access
    //@{
    /// Return the size of the greatest lower bound
    unsigned int glbSize(void) const;
    /// Return the size of the least upper bound
    unsigned int lubSize(void) const;
    /// Return the size of lub-glb
    unsigned int unknownSize(void) const;
    /// Return current cardinality minimum
    unsigned int cardMin(void) const;
    /// Return current cardinality maximum
    unsigned int cardMax(void) const;
    /// Return minimum of the least upper bound
    int lubMin(void) const;
    /// Return maximum of the least upper bound
    int lubMax(void) const;
    /// Return minimum of the greatest lower bound
    int glbMin(void) const;
    /// Return maximum of the greatest lower bound
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
    //@}
    
    /// \name Cloning
    //@{
    /// Update this variable to be a clone of variable \a x
    void update(Space* home, bool share, CpltSetVar& x);
    //@}

  };

  /// Range iterator for the greatest lower bound of a bdd variable
  class CpltSetVarGlbRanges {
  private:
    Set::GlbRanges<CpltSet::CpltSetVarImp*> iter;
  public:
    CpltSetVarGlbRanges(void);
    CpltSetVarGlbRanges(const CpltSetVar& x);
    bool operator()(void) const;
    void operator++(void);
    int min(void) const;
    int max(void) const;
  };

  /// Value iterator for the greatest lower bound of a bdd variable
  class CpltSetVarGlbValues {
  private:
    CpltSet::GlbValues<CpltSet::CpltSetVarImp*> iter;
  public:
    CpltSetVarGlbValues(void);
    CpltSetVarGlbValues(const CpltSetVar& x);
    bool operator()(void) const;
    void operator++(void);
    int val(void) const;
  };

  /// Range iterator for the least upper bound of a bdd variable
  class CpltSetVarLubRanges {
  private:
    Set::LubRanges<CpltSet::CpltSetVarImp*> iter;
  public:
    CpltSetVarLubRanges(void);
    CpltSetVarLubRanges(const CpltSetVar& x);
    bool operator()(void) const;
    void operator++(void);
    int min(void) const;
    int max(void) const;
  };

  /// Value iterator for the least upper bound of a bdd variable
  class CpltSetVarLubValues {
  private:
    CpltSet::LubValues<CpltSet::CpltSetVarImp*> iter;
  public:
    CpltSetVarLubValues(void);
    CpltSetVarLubValues(const CpltSetVar& x);
    bool operator()(void) const;
    void operator++(void);
    int val(void) const;
  };

  /// Range iterator for the unknown set of a bdd variable
  class CpltSetVarUnknownRanges {
  private:
    Set::UnknownRanges<CpltSet::CpltSetVarImp*> iter;
  public:
    CpltSetVarUnknownRanges(void);
    CpltSetVarUnknownRanges(const CpltSetVar& x);
    bool operator()(void) const;
    void operator++(void);
    int min(void) const;
    int max(void) const;
  };

}

/**
 * \brief Print bdd variable \a x
 * \relates Gecode::CpltSetVar
 */
GECODE_CPLTSET_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::CpltSetVar& x);

#include "gecode/cpltset/view.icc"

namespace Gecode {
  /**
   * \defgroup TaskModelCpltSetArgs Argument arrays
   * 
   * Argument arrays are just good enough for passing arguments
   * with automatic memory management.
   * \ingroup TaskModelCpltSet
   */

  //@{
  /// Passing set variables
  typedef VarArgArray<CpltSetVar>  CpltSetVarArgs;
  //@}

  /**
   * \defgroup TaskModelCpltSetVarArrays Variable arrays
   * 
   * Variable arrays can store variables. They are typically used
   * for storing the variables being part of a solution. However,
   * they can also be used for temporary purposes (even though
   * memory is not reclaimed until the space it is created for
   * is deleted).
   * \ingroup TaskModelCpltSet
   */

  /**
   * \brief %CpltSet variable array
   * \ingroup TaskModelCpltSetVarArrays
   */
  class CpltSetVarArray : public VarArray<CpltSetVar> {
  public:
    CpltSetVarArray(void);
    CpltSetVarArray(const CpltSetVarArray&);
    /// Create an uninitialized array of size \a n
    GECODE_CPLTSET_EXPORT CpltSetVarArray(Space* home, int n);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVarArray(Space* home,
                    int n,int glbMin,int glbMax,int lubMin,int lubMax,
                    unsigned int minCard = 0,
                    unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVarArray(Space* home,
                    int n,const IntSet& glb, int lubMin, int lubMax,
                    unsigned int minCard = 0,
                    unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVarArray(Space* home,
                    int n,int glbMin,int glbMax,const IntSet& lub,
                    unsigned int minCard = 0,
                    unsigned int maxCard = Set::Limits::card);
    /**
     * \brief Create an array of size \a n.
     *
     * Each variable is initialized with the bounds and cardinality as
     * given by the arguments.
     */
    GECODE_CPLTSET_EXPORT 
    CpltSetVarArray(Space* home, int n,
                    const IntSet& glb,const IntSet& lub,
                    unsigned int minCard = 0,
                    unsigned int maxCard = Set::Limits::card);
  };
}

#include "gecode/cpltset/array.icc"


namespace Gecode {

  /**
   * \brief Common relation types for sets
   * \ingroup TaskModelCpltSet
   */
  enum CpltSetRelType {
    /// Lexicographically less than on bitstring(\f$\prec_{lex}\f$)
    SRT_LE,    
    /// Lexicographically less than on bitstring(\f$\prec_{lex}\f$)
    SRT_GR,
    /// Lexicographically less than or equal  (\f$\preceq_{lex}\f$)
    SRT_LQ,
    /// Lexicographically greater than or equal  (\f$\preceq_{lex}\f$)    
    SRT_GQ,
    /// Lexicographically less than on reversed bitstring(\f$\prec_{lex}\f$)
    SRT_LE_REV,
    /// Lexicographically greater than on reversed bitstring (\f$\succ_{lex}\f$)
    SRT_GR_REV,
    /// Lexicographically reversed less than or equal (\f$\preceq_{lex}\f$)
    SRT_LQ_REV,
    /// Lexicographically reversed greater than or equal  (\f$\preceq_{lex}\f$)
    SRT_GQ_REV
  };

  /**
   * \brief Common operations for sets
   * \ingroup TaskModelCpltSet
   */
  enum CpltSetOpType {
    SOT_SYMDIFF,  ///< Symmetric Difference
  };

  /**
   * \defgroup TaskModelCpltSetDom Domain constraints
   * \ingroup TaskModelCpltSet
   *
   */
  //@{

  /// Propagates \f$ x \sim_r \{i\}\f$
  GECODE_CPLTSET_EXPORT void
  dom(Space* home, CpltSetVar x, SetRelType r, int i);

  /// Propagates \f$ x \sim_r \{i,\dots,j\}\f$
  GECODE_CPLTSET_EXPORT void
  dom(Space* home, CpltSetVar x, SetRelType r, int i, int j);

  /// Propagates \f$ x \sim_r s\f$
  GECODE_CPLTSET_EXPORT void
  dom(Space* home, CpltSetVar x, SetRelType r, const IntSet& s);

  /// Post propagator for \f$ l \leq |x| \leq u \f$ 
  GECODE_CPLTSET_EXPORT void
  cardinality(Space* home, CpltSetVar x, unsigned int l, unsigned int u);

  /// Post propagator for \f$ l |x| = c \f$ 
  GECODE_CPLTSET_EXPORT void
  cardinality(Space* home, CpltSetVar x, unsigned int c);
  
  //@}

  /**
   * \defgroup TaskModelCpltSetRel Relation constraints
   * \ingroup TaskModelCpltSet
   *
   */

  //@{
  
  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetRelType r, CpltSetVar y);

  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetRelType r, CpltSetVar y);

  //@}

  /**
   * \defgroup TaskModelCpltSetRelOp CpltSet operation/relation constraints
   * \ingroup TaskModelCpltSet
   *
   */

  //@{

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, 
      CpltSetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, 
      SetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, 
      CpltSetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, 
      SetRelType r, CpltSetVar z);

  //@}

  /**
   * \defgroup TaskModelCpltSetConnect Connection constraints to finite domain variables
   * \ingroup TaskModelCpltSet
   *
   */

  //@{
    
  /// Post propagator for \f$ s = \{x\} \f$
  GECODE_CPLTSET_EXPORT void
  singleton(Space* home, IntVar x, CpltSetVar s);
  
  //@}

  /**
   * \defgroup TaskModelCpltSetInter Intersection and partition constraints
   * \ingroup TaskModelCpltSet
   *
   */

  //@{
  
  /// Post propagator for \f$ |x \cap is| =  c \f$ 
  GECODE_CPLTSET_EXPORT void
  exactly(Space* home, CpltSetVar x, IntSet& is, unsigned int c);

  /// Post propagator for \f$ 0 \leq |x \cap is| \leq c \f$ 
  GECODE_CPLTSET_EXPORT void
  atmost(Space* home, CpltSetVar x, IntSet& is, unsigned int c);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \f$ 
  GECODE_CPLTSET_EXPORT void
  atmost(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \wedge x \sim_{lex} y\f$ 
  GECODE_CPLTSET_EXPORT void
  atmostLex(Space* home, CpltSetVar x, CpltSetVar y, unsigned int c, 
            CpltSetRelType lex);

  /**
   * \brief  Post propagator for \f$ 0 \leq |x \cap y| \leq c 
   *         \wedge x \sim_{lex} y \wedge |x| = |y| = d\f$ 
   */
  GECODE_CPLTSET_EXPORT void
  atmostLexCard(Space* home, CpltSetVar x, CpltSetVar y, int c, 
                CpltSetRelType lex, int d);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \wedge |x| = |y| = d\f$ 
  GECODE_CPLTSET_EXPORT void
  atmostCard(Space* home, CpltSetVar x, CpltSetVar y, int c, int d);

  /// Post propagator for \f$ x \cap y = z \wedge 0 \leq |z| \leq c \f$ 
  GECODE_CPLTSET_EXPORT void
  atmost(Space* home, CpltSetVar x, CpltSetVar y, CpltSetVar z, int c);

  /** \brief Post propagator for \f$ |x_i \cap x_j| \leq 1 \f$ for all \f$0\leq i\neq j<|x| 
   *  \wedge \forall i \in \{0, \dots, |x|-1\}: |x_i| = c \f$
   *  \note The bdd for this constraint becomes exponential with increasing universe
   *        and increasing |x|
   */ 
  GECODE_CPLTSET_EXPORT void
  atmostOne(Space* home, const CpltSetVarArgs& x, int c);

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_CPLTSET_EXPORT void
  partition(Space* home, const CpltSetVarArgs& x);

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_CPLTSET_EXPORT void
  partition(Space* home, const CpltSetVarArgs& x, const CpltSetVar& y);

  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i,j\in \{0, \dots, n-1\}, i\neq j: 
   *                       x_i \sim_{lex} x_j\f$ 
   *
   */
  GECODE_CPLTSET_EXPORT void
  partitionLex(Space* home, const CpltSetVarArgs& x, CpltSetRelType lex);
  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i,j\in \{0, \dots, n-1\}, i\neq j: 
   *                       x_i \sim_{lex} x_j  \wedge 
   *                       \forall i \in \{0, \dots, n-1\}: |x_i| = c \f$ 
   */
  GECODE_CPLTSET_EXPORT void
  partitionLexCard(Space* home, const CpltSetVarArgs& x, CpltSetRelType lex, 
                   unsigned int c);

  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i \in \{0, \dots, n-1\}: |x_i| = c \f$ 
   */
  GECODE_CPLTSET_EXPORT void
  partitionCard(Space* home, const CpltSetVarArgs& x, unsigned int c);

  //@}
  
  /**
   * \defgroup TaskModelCpltSetSelect Selection constraints
   * \ingroup TaskModelCpltSet
   *
   */

  //@{
  
  /// Post propagator for \f$ t = \bigcup_{i\in s}  x_i \f$
  GECODE_CPLTSET_EXPORT void
  selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar s,
              CpltSetVar t);

  /// Post propagator for \f$ s = \bigcup_{(x_i \subseteq t) \wedge x_i \neq \emptyset } i \f$
  GECODE_CPLTSET_EXPORT void
  selectNonEmptySub(Space* home, const CpltSetVarArgs& x,
                    CpltSetVar s, CpltSetVar t);

  /// Post propagator for \f$ t = \bigcup_{i\subseteq s}  x_i \f$
  GECODE_CPLTSET_EXPORT void
  range(Space* home, const CpltSetVarArgs& x, CpltSetVar s, CpltSetVar t);

  /// Post propagator for \f$ s = \bigcup_{x_i \subseteq t} i \f$
  GECODE_CPLTSET_EXPORT void
  roots(Space* home, const CpltSetVarArgs& x, CpltSetVar s, CpltSetVar t, 
        const CpltSetVarArgs& allvars);

  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x| 
   *  \wedge t = \bigcup_{i\subseteq s}  x_i \f$
   */ 
  GECODE_CPLTSET_EXPORT void
  alldifferent(Space* home, const CpltSetVarArgs& x, CpltSetVar s,
               CpltSetVar t, const CpltSetVarArgs& allvars);

  /** \brief Post propagator for \f$ |\displaystyle\bigcup_{i = 0}^{|x| - 1} x_i| = n 
   *  \wedge t = \bigcup_{i\subseteq s}  x_i \f$
   * 
   */
  GECODE_CPLTSET_EXPORT void
  nvalue(Space* home, const CpltSetVarArgs& x, CpltSetVar s, CpltSetVar t,
         unsigned int n, const CpltSetVarArgs& allvars);

  /** \brief Post propagator for \f$ t = \bigcup_{i\in s} x_i \wedge v = \bigcup_{i\in u} y_i
   *  \wedge v \subseteq t \f$
   * 
   *  |x| and |y| may be different
   */
  GECODE_CPLTSET_EXPORT void
  uses(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t,
       const CpltSetVarArgs& y, CpltSetVar u, CpltSetVar v);
  //@}


  /**
   * \defgroup TaskModelCpltSetBranch Branching
   * \ingroup TaskModelCpltSet
   */
  //@{
  
  /// Which variable to select for branching
  enum CpltSetVarBranch {
    CPLTSET_VAR_NONE,             ///< First unassigned
    CPLTSET_VAR_MIN_CARD,         ///< With smallest unknown set
    CPLTSET_VAR_MAX_CARD,         ///< With largest unknown set
    CPLTSET_VAR_MIN_UNKNOWN_ELEM, ///< With smallest unknown element
    CPLTSET_VAR_MAX_UNKNOWN_ELEM  ///< With largest unknown element 
  };
  
  /// Which values to select first for branching 
  enum CpltSetValBranch { 
    CPLTSET_VAL_MIN_UNKNOWN,          ///< Select smallest element in unknown set and try to include it
    CPLTSET_VAL_MAX_UNKNOWN,          ///< Select largest  element in unknown set and try to include it
    CPLTSET_VAL_MIN_UNKNOWN_EX_FIRST, ///< Select smallest element in unknown set and try to exclude it
    CPLTSET_VAL_MAX_UNKNOWN_EX_FIRST  ///< Select largest  element in unknown set and try to exclude it
  };

  /// Branch on \a x with variable selection \a vars and value selection \a vals
  GECODE_CPLTSET_EXPORT void
  branch(Space* home, const CpltSetVarArgs& x, 
         CpltSetVarBranch vars, CpltSetValBranch vals);
  //@}

}

#endif

// IFDEF: GECODE_HAS_CPLTSET_VARS
// STATISTICS: set-post
