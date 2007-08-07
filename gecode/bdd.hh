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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_BDD_HH__
#define __GECODE_BDD_HH__

#include "gecode/limits.hh"
#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/iter.hh"
#include "gecode/support/shared-array.hh"

#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

#include "gecode/support/buddy/bdd.h"

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_BDD
#define GECODE_BDD_EXPORT __declspec( dllexport )
#else
#define GECODE_BDD_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_BDD_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_BDD_EXPORT
#endif

#endif

#include "gecode/bdd/exception.icc"
#include "gecode/bdd/propagator.icc"

namespace Gecode { 
  //@{
  /// Passing external bdd variables
  typedef bdd  GecodeBdd;

  //@}

namespace Bdd {

  /** 
   * \namespace Gecode::Bdd
   * \brief Complete representation of finite integer sets using ROBDD's
   * 
   * The Gecode::Bdd namespace contains all functionality required
   * to program propagators and branchings for finite integer sets.
   * In addition, all propagators and branchings for finite integer
   * sets provided by %Gecode are contained as nested namespaces.
   *
   */
  const GecodeBdd Empty = bdd_false();
  const GecodeBdd Full = bdd_true();

}}

#define BDDTOP  Bdd::Full
#define BDDBOT  Bdd::Empty


namespace Gecode { 
  enum BddOutput {
    BDD_DOM,    ///< Full domain output 
    BDD_BND,    ///< Only greatest lower and least upper bound
    BDD_BND_DOM ///< Output full domain and bounds
  };
}

#include "gecode/bdd/manager/buddy.icc"
typedef Gecode::BuddyMgr BddMgr;

#include "gecode/bdd/bddsupport.icc"
#include "gecode/set/var.icc"
#include "gecode/bdd/var.icc"
#include "gecode/bdd/view.icc"
#include "gecode/bdd/array.icc"


namespace Gecode {

  enum BddSetRelType {
    SRT_LE,    ///< Lexicographically less than on bitstring(\f$\prec_{lex}\f$)
    SRT_GR,    ///< Lexicographically greater than on bitstring (\f$\succ_{lex}\f$)
    SRT_LQ,    ///< Lexicographically less than or equal  (\f$\preceq_{lex}\f$)
    SRT_GQ,    ///< Lexicographically greater than or equal  (\f$\preceq_{lex}\f$)
    SRT_LE_REV,    ///< Lexicographically less than on reversed bitstring(\f$\prec_{lex}\f$)
    SRT_GR_REV,    ///< Lexicographically greater than on reversed bitstring (\f$\succ_{lex}\f$)
    SRT_LQ_REV,    ///< Lexicographically reversed less than or equal  (\f$\preceq_{lex}\f$)
    SRT_GQ_REV,    ///< Lexicographically reversed greater than or equal  (\f$\preceq_{lex}\f$)
  };

  enum BddSetOpType {
    SOT_SYMDIFF,  ///< Symmetric Difference
  };

  enum SetConLevel {
    SCL_BND_SBR, ///< Bounds consistency (using set bounds representation)
    SCL_BND_BDD, ///< Bounds consistency (using bdd bounds representation)
    SCL_DOM, ///< Domain consistency (using complete bdd representation)
    SCL_SPL, ///< Split bounds  consistency
    SCL_CRD, ///< Cardinality bounds consistency
    SCL_LEX, ///< Lexicographic bounds consistency
    SCL_DEF  ///< The default consistency for a constraint
  };

//   /**
//    * \brief Consistency levels for bdd propagators
//    *
//    * If in the description of a constraint below no consistency level
//    * is mentioned, the propagator for the constraint implements
//    * domain-consistency.
//    */
//   enum BddConLevel {
//     BCL_BND, ///< Bounds consistency
//     BCL_SPL, ///< Split bounds  consistency
//     BCL_CRD, ///< Cardinality bounds consistency
//     BCL_LEX, ///< Lexicographic bounds consistency
//     BCL_DOM, ///< Domain consistency
//     BCL_DEF  ///< The default consistency for a constraint
//   };

  /// Which variable to select for branching
  enum BddBvarSel {
    BDD_BVAR_NONE,             ///< First unassigned
    BDD_BVAR_MIN_CARD,         ///< With smallest unknown set
    BDD_BVAR_MAX_CARD,         ///< With largest unknown set
    BDD_BVAR_MIN_UNKNOWN_ELEM, ///< With smallest unknown element
    BDD_BVAR_MAX_UNKNOWN_ELEM  ///< With largest unknown element 
  };
  
  /// Which values to select first for branching. 
  enum BddBvalSel { 
    BDD_BVAL_MIN,                  ///< Select first domain value and try equality
    BDD_BVAL_MAX,                  ///< Select last  domain value and try equality
    BDD_BVAL_MIN_EX_FIRST,         ///< Select first domain value and try disequality
    BDD_BVAL_MAX_EX_FIRST,         ///< Select last  domain value and try disequality
    BDD_BVAL_MIN_UNKNOWN,          ///< Select smallest element in unknown set and try to include it
    BDD_BVAL_MAX_UNKNOWN,          ///< Select largest  element in unknown set and try to include it
    BDD_BVAL_MIN_UNKNOWN_EX_FIRST, ///< Select smallest element in unknown set and try to exclude it
    BDD_BVAL_MAX_UNKNOWN_EX_FIRST  ///< Select largest  element in unknown set and try to exclude it
  };

  ///\name Posting bdd propagators
  //@{

  /// Propagates \f$ x \sim_r \{i\}\f$
  GECODE_BDD_EXPORT void
  dom(Space* home, BddVar x, SetRelType r, int i, 
      SetConLevel scl = SCL_DEF);

  /// Propagates \f$ x \sim_r \{i,\dots,j\}\f$
  GECODE_BDD_EXPORT void
  dom(Space* home, BddVar x, SetRelType r, int i, int j, 
      SetConLevel scl = SCL_DEF);

  /// Propagates \f$ x \sim_r s\f$
  GECODE_BDD_EXPORT void
  dom(Space* home, BddVar x, SetRelType r, const IntSet& s, 
      SetConLevel scl = SCL_DEF);
  
  /// Post propagator for \f$ dom(x) \sim_r y\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, IntVar x, BddSetRelType r, BddVar s, 
      SetConLevel scl = SCL_DEF);


  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, BddSetRelType r, BddVar y, 
      SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, BddSetOpType o, BddVar y, 
      BddSetRelType r, BddVar z,  SetConLevel scl = SCL_DEF);

  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, BddSetOpType o, BddVar y, 
      SetRelType r, BddVar z,  SetConLevel scl = SCL_DEF);

  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType o, BddVar y, 
      BddSetRelType r, BddVar z,  SetConLevel scl = SCL_DEF);

  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType o, BddVar y, 
      SetRelType r, BddVar z,  SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ l \leq |x| \leq u \f$ 
  GECODE_BDD_EXPORT void
  cardinality(Space* home, BddVar x, unsigned int l, unsigned int u, 
	      SetConLevel scl = SCL_DEF);
  
  /// Post propagator for \f$ l |x| = c \f$ 
  GECODE_BDD_EXPORT void
  cardinality(Space* home, BddVar x, unsigned int c, 
	      SetConLevel scl = SCL_DEF);
  
  GECODE_BDD_EXPORT void 
  singleton(Space* home, IntVar x, BddVar s, SetConLevel scl);
  
  /// Post propagator for \f$ \bigcup_{i\neq index} glb(x_{i}) \parallel x_{index} \f$ 
  GECODE_BDD_EXPORT void
  disjointglb(Space* home, const BddVarArgs& x, int index);

  /// Post propagator for \f$ \bigcup_{i\neq index} glb(x_{i}) \parallel x_{index} \f$ 
  GECODE_BDD_EXPORT void
  disjointsudoku(Space* home,  BddVar x, int order);

  /// Post propagator for \f$ |x \cap is| =  c \f$ 
  GECODE_BDD_EXPORT void
  exactly(Space* home, BddVar x, IntSet& is, int c, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ 0 \leq |x \cap is| \leq c \f$ 
  GECODE_BDD_EXPORT void
  atmost(Space* home, BddVar x, IntSet& is, int c, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \f$ 
  GECODE_BDD_EXPORT void
  atmost(Space* home, BddVar x, BddVar y, int c, 
	 SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \wedge x \sim_{lex} y\f$ 
  GECODE_BDD_EXPORT void
  atmostLex(Space* home, BddVar x, BddVar y, int c, 
	    BddSetRelType lex, SetConLevel scl = SCL_DEF);

  /**
   * \brief  Post propagator for \f$ 0 \leq |x \cap y| \leq c 
   *         \wedge x \sim_{lex} y \wedge |x| = |y| = d\f$ 
   */
  GECODE_BDD_EXPORT void
  atmostLexCard(Space* home, BddVar x, BddVar y, int c, 
		BddSetRelType lex, int d, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \wedge |x| = |y| = d\f$ 
  GECODE_BDD_EXPORT void
  atmostCard(Space* home, BddVar x, BddVar y, int c, 
	     int d, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ x \cap y = z \wedge 0 \leq |z| \leq c \f$ 
  GECODE_BDD_EXPORT void
  atmost(Space* home, BddVar x, BddVar y, BddVar z, int c, 
	 SetConLevel scl = SCL_DEF);

  /** \brief Post propagator for \f$ |x_i \cap x_j| \leq 1 \f$ for all \f$0\leq i\neq j<|x| 
   *  \wedge \forall i \in \{0, \dots, |x|-1\}: |x_i| = c \f$
   *  \note The bdd for this constraint becomes exponential with increasing universe
   *        and increasing |x|
   */ 
  GECODE_BDD_EXPORT void
  atmostOne(Space* home, const BddVarArgs& x, int c, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_BDD_EXPORT void
  partition(Space* home, const BddVarArgs& x, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_BDD_EXPORT void
  partition(Space* home, const BddVarArgs& x, const BddVar& y, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_BDD_EXPORT void
  partition(Space* home, const IntVarArgs& x, const BddVar& y, SetConLevel scl = SCL_DEF);

  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i,j\in \{0, \dots, n-1\}, i\neq j: 
   *                       x_i \sim_{lex} x_j\f$ 
   *
   */
  GECODE_BDD_EXPORT void
  partitionLex(Space* home, const BddVarArgs& x, BddSetRelType lex, 
	       SetConLevel scl = SCL_DEF);
  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i,j\in \{0, \dots, n-1\}, i\neq j: 
   *                       x_i \sim_{lex} x_j  \wedge 
   *                       \forall i \in \{0, \dots, n-1\}: |x_i| = c \f$ 
   */
  GECODE_BDD_EXPORT void
  partitionLexCard(Space* home, const BddVarArgs& x, BddSetRelType lex, int c, 
		   SetConLevel scl = SCL_DEF);

  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i \in \{0, \dots, n-1\}: |x_i| = c \f$ 
   */
  GECODE_BDD_EXPORT void
  partitionCard(Space* home, const BddVarArgs& x, int c, 
		   SetConLevel scl = SCL_DEF);


  /// Post propagator for \f$ t = \bigcup_{i\in s}  x_i \f$
  GECODE_BDD_EXPORT void
  selectUnion(Space* home, const BddVarArgs& x, BddVar s, BddVar t,
	      SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ s = \bigcup_{(x_i \subseteq t) \wedge x_i \neq \emptyset } i \f$
  GECODE_BDD_EXPORT void
  findNonEmptySub(Space* home, const BddVarArgs& x, BddVar s, BddVar t,
		  SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ s = \{x\} \f$
  GECODE_BDD_EXPORT void
  singleton(Space* home, IntVar x, BddVar s, SetConLevel scl = SCL_DOM);

  /// Post propagator for \f$ t = \bigcup_{i\in s}  x_i \f$
  GECODE_BDD_EXPORT void
  range(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	SetConLevel scl = SCL_DOM);

  /// Post propagator for \f$ s = \bigcup_{x_i \in t} i \f$
  GECODE_BDD_EXPORT void
  roots(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	const BddVarArgs& allvars, 
	SetConLevel scl = SCL_DOM);

  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x| 
   *  \wedge t = \bigcup_{i\in s}  x_i \f$
   */ 
  GECODE_BDD_EXPORT void
  alldifferent(Space* home, const IntVarArgs& x, BddVar s, BddVar t, 
	       const BddVarArgs& allvars, 
	       SetConLevel scl = SCL_DOM);

  /* \brief Post propagator \f$\wedge r = \bigcup_{i\in s}  x_i \f$
   */ 
/*   GECODE_BDD_EXPORT void
     permutation(Space* home, const IntVarArgs& x, const IntSet& r, 
   	      BddVar s, BddVar t, SetConLevel scl = SCL_DOM);
  // for permutation we need a CONST VIEW and DONT NEED Card = N THAT IS GOOD
*/

  /** \brief Post propagator for \f$ |\displaystyle\bigcup_{i = 0}^{|x| - 1} x_i| = n 
   *  \wedge t = \bigcup_{i\in s}  x_i \f$
   * 
   */
  GECODE_BDD_EXPORT void
  nvalue(Space* home, const IntVarArgs& x, BddVar s, BddVar t, int n,
	 const BddVarArgs& allvars, 
	 SetConLevel scl = SCL_DOM);

  /** \brief Post propagator for \f$ t = \bigcup_{i\in s} x_i \wedge v = \bigcup_{i\in u} y_i
   *  \wedge v \subseteq t \f$
   * 
   *  |x| and |y| may be different
   */
  GECODE_BDD_EXPORT void
  uses(Space* home, const IntVarArgs& x, BddVar s, BddVar t,
       const IntVarArgs& y, BddVar u, BddVar v,
       SetConLevel scl = SCL_DOM);
  
  //@}


  /**
   * \brief Ordering all declared bdd variables \f$ x_0, \dots, x_{n-1}\f$ such that 
   * \f$ \forall i\in \{0, \dots, n - 1\}: x_{0_{1}} \prec x_{{n-1}_{1}}, \dots, x_{0_{k-1}} \prec x_{{n-1}_{k-1}}\f$
   */
  
  GECODE_BDD_EXPORT void
  hls_order(Space* home, const BddVarArray& x);

  /**
   * \brief Ordering all declared bdd variables \f$ x_0, \dots, x_{n-1}\f$ and \f$ y_0, \dots, y_{n-1}\f$such that 
   * \f$ \forall i\in \{0, \dots, n - 1\}: x_{0_{1}} \prec \dots \prec x_{{n-1}_{1}} , \dots, x_{0_{k-1}} \prec x_{{n-1}_{k-1}}\f$
   */
  
  GECODE_BDD_EXPORT void
  hls_order(Space* home, const BddVarArray& x, const BddVarArray& y);

  /**
   * \brief Ordering all declared bdd variables \f$ x_0, \dots, x_{n-1}\f$ and \f$ y_0, \dots, y_{n-1}\f$such that 
   * \f$ \forall i\in \{0, \dots, n - 1\}: x_{0_{1}} \prec \dots \prec x_{{n-1}_{1}} , \dots, x_{0_{k-1}} \prec x_{{n-1}_{k-1}}\f$
   */
  
  GECODE_BDD_EXPORT void
  hls_order(Space* home, BddVarArray** x, int xs);

  /// Branch over all \a x with variable selection \a vars and value selection \a vals
  GECODE_BDD_EXPORT void
  branch(Space* home, const BddVarArgs& x, BddBvarSel vars, BddBvalSel vals, 
	 SetConLevel scl = SCL_DOM);


  ///\name Use bdd variables with set propagators
  //@{
  /**
   * \defgroup TaskIntSetRel Relation constraints
   * \ingroup TaskIntSet
   *
   */

  //@{

   /// Post propagator for \f$ x \sim_r y\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetRelType r, BddVar y, 
      SetConLevel scl = SCL_BND_SBR);

  /// Post propagator for \f$ (x \sim_r y) \Leftrightarrow b \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetRelType r, BddVar y, BoolVar b, SetConLevel scl = SCL_DEF);
  
  /// Post propagator for \f$ s \sim_r \{x\}\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar s, SetRelType r, IntVar x, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ \{x\} \sim_r s\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, IntVar x, SetRelType r, BddVar s, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ (s \sim_r \{x\}) \Leftrightarrow b \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar s, SetRelType r, IntVar x, BoolVar b, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$ (\{x\} \sim_r s) \Leftrightarrow b \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, IntVar x, SetRelType r, BddVar s, BoolVar b, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$\forall i\in s:\ i \sim_r x\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar s, IntRelType r, IntVar x, SetConLevel scl = SCL_DEF);

  /// Post propagator for \f$\forall i\in s:\ x \sim_r i\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, IntVar x, IntRelType r, BddVar s, SetConLevel scl = SCL_DEF);

  //@}

  /**
   * \defgroup TaskIntSetRelOp Set operation/relation constraints
   * \ingroup TaskIntSet
   *
   */

  //@{

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType op, BddVar y, SetRelType r, BddVar z);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, SetOpType op, const BddVarArgs& x, BddVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  GECODE_BDD_EXPORT void
  rel(Space* home, SetOpType op, const IntVarArgs& x, BddVar y);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, const IntSet& x, SetOpType op, BddVar y,
      SetRelType r, BddVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType op, const IntSet& y,
      SetRelType r, BddVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType op, BddVar y,
      SetRelType r, const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, const IntSet& x, SetOpType op, const IntSet& y,
      SetRelType r, BddVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, const IntSet& x, SetOpType op, BddVar y, SetRelType r,
      const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_BDD_EXPORT void
  rel(Space* home, BddVar x, SetOpType op, const IntSet& y, SetRelType r,
      const IntSet& z);

  //@}

  /**
   * \defgroup TaskIntSetConvex Convexity constraints
   * \ingroup TaskIntSet
   *
   */
  //@{

  /// Post propagator that propagates that \a x is convex 
  GECODE_BDD_EXPORT void
  convex(Space* home, BddVar x);

  /// Post propagator that propagates that \a y is the convex hull of \a x
  GECODE_BDD_EXPORT void
  convexHull(Space* home, BddVar x, BddVar y);

  //@}

  /**
   * \defgroup TaskIntSetSequence Sequence constraints
   * \ingroup TaskIntSet
   *
   */
  //@{

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
  GECODE_BDD_EXPORT void
  sequence(Space* home, const BddVarArgs& x);

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
  GECODE_BDD_EXPORT void
  sequentialUnion(Space* home, const BddVarArgs& y, BddVar x);

  //@}

  /**
   * \defgroup TaskIntSetDistinct Distinctness constraints
   * \ingroup TaskIntSet
   *
   */
  //@{


  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
  GECODE_BDD_EXPORT void
  atmostOne(Space* home, const BddVarArgs& x, unsigned int c);

  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : x_i\neq x_j\f$
  GECODE_BDD_EXPORT void
  distinct(Space* home, const BddVarArgs& x, unsigned int c);

  /// Post propagator for \f$\forall 0\leq i\leq |x| : \forall 0\leq i<j\leq |x| : x_i\neq x_j\f$
  GECODE_BDD_EXPORT void
  distinct(Space* home, const BddVarArgs& x, SetConLevel scl = SCL_DEF);

  //@}

  /**
   * \defgroup TaskIntSetConnect Connection constraints to finite domain variables
   * \ingroup TaskIntSet
   *
   */

  //@{

  /// Post propagator that propagates that \a x is the minimal element of \a s 
  GECODE_BDD_EXPORT void
  min(Space* home, BddVar s, IntVar x);
  
  /// Post propagator that propagates that \a x is the maximal element of \a s 
  GECODE_BDD_EXPORT void
  max(Space* home, BddVar s, IntVar x);
  
  /// Post propagator that propagates that \a s contains the \f$x_i\f$, which are sorted in non-descending order 
  GECODE_BDD_EXPORT void
  match(Space* home, BddVar s, const IntVarArgs& x);
  
  /// Post propagator for \f$x_i=j \Leftrightarrow i\in y_j\f$
  GECODE_BDD_EXPORT void
  channel(Space* home, const IntVarArgs& x,const BddVarArgs& y);
  
  /// Post propagator for \f$ |s|=x \f$ 
  GECODE_BDD_EXPORT void
  cardinality(Space* home, BddVar s, IntVar x);


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
  GECODE_BDD_EXPORT void
  weights(Space* home, const IntArgs& elements, const IntArgs& weights,
	  BddVar x, IntVar y);

  //@}

 /**
   * \brief Post propagator for \f$ z=\bigcup\langle x_0,\dots,x_{n-1}\rangle[y] \f$
   * If \a y is the empty set, \a z will also be constrained to be empty
   * (as an empty union is empty).
   */
  GECODE_BDD_EXPORT void
  selectUnion(Space* home, const BddVarArgs& x, BddVar y, BddVar z);

  /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \f$ \mathcal{U} \f$ as universe
   *
   * If \a y is empty, \a z will be constrained to be the universe
   * \f$ \mathcal{U} \f$ (as an empty intersection is the universe).
   */
  GECODE_BDD_EXPORT void
  selectInter(Space* home, const BddVarArgs& x, BddVar y, BddVar z);

  /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \a u as universe
   *
   * If \a y is empty, \a z will be constrained to be the given universe
   * \a u (as an empty intersection is the universe).
   */
  GECODE_BDD_EXPORT void
  selectInterIn(Space* home, const BddVarArgs& x, BddVar y, BddVar z,
		const IntSet& u);

  /// Post propagator for \f$ \parallel\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
  GECODE_BDD_EXPORT void
  selectDisjoint(Space* home, const BddVarArgs& x, BddVar y);

  /// Post propagator for \f$ z=\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
  GECODE_BDD_EXPORT void
  selectSet(Space* home, const BddVarArgs& x, IntVar y, BddVar z);

  //@}
  //}@ 
}

#endif


// STATISTICS: set-post
