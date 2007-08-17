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

#include "gecode/limits.hh"
#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/iter.hh"
#include "gecode/support/shared-array.hh"
#include "gecode/set.hh"
#include "gecode/support/buddy/bdd.h"

/*
 * Support for DLLs under Windows
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

#ifdef GCC_HASCLASSVISIBILITY

#define GECODE_CPLTSET_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_CPLTSET_EXPORT
#endif

#endif

#include "gecode/cpltset/exception.icc"
#include "gecode/cpltset/propagator.icc"

namespace Gecode { 
  //@{
  /// Passing external bdd variables
  typedef bdd  GecodeBdd;

  //@}

namespace CpltSet {

  /** 
   * \namespace Gecode::CpltSet
   * \brief Complete representation of finite integer sets using ROBDD's
   * 
   * The Gecode::CpltSet namespace contains all functionality required
   * to program propagators and branchings for finite integer sets.
   * In addition, all propagators and branchings for finite integer
   * sets provided by %Gecode are contained as nested namespaces.
   *
   */
  const GecodeBdd Empty = bdd_false();
  const GecodeBdd Full = bdd_true();

}}

#define BDDTOP  CpltSet::Full
#define BDDBOT  CpltSet::Empty

#include "gecode/cpltset/manager/buddy.icc"
typedef Gecode::BuddyMgr BddMgr;

#include "gecode/cpltset/support.icc"
#include "gecode/set/var.icc"
#include "gecode/cpltset/var.icc"
#include "gecode/cpltset/view.icc"
#include "gecode/cpltset/array.icc"


namespace Gecode {

  enum CpltSetRelType {
    SRT_LE,    ///< Lexicographically less than on bitstring(\f$\prec_{lex}\f$)
    SRT_GR,    ///< Lexicographically greater than on bitstring (\f$\succ_{lex}\f$)
    SRT_LQ,    ///< Lexicographically less than or equal  (\f$\preceq_{lex}\f$)
    SRT_GQ,    ///< Lexicographically greater than or equal  (\f$\preceq_{lex}\f$)
    SRT_LE_REV,    ///< Lexicographically less than on reversed bitstring(\f$\prec_{lex}\f$)
    SRT_GR_REV,    ///< Lexicographically greater than on reversed bitstring (\f$\succ_{lex}\f$)
    SRT_LQ_REV,    ///< Lexicographically reversed less than or equal  (\f$\preceq_{lex}\f$)
    SRT_GQ_REV,    ///< Lexicographically reversed greater than or equal  (\f$\preceq_{lex}\f$)
  };

  enum CpltSetOpType {
    SOT_SYMDIFF,  ///< Symmetric Difference
  };

  /// Which variable to select for branching
  enum CpltSetBvarSel {
    CPLTSET_BVAR_NONE,             ///< First unassigned
    CPLTSET_BVAR_MIN_CARD,         ///< With smallest unknown set
    CPLTSET_BVAR_MAX_CARD,         ///< With largest unknown set
    CPLTSET_BVAR_MIN_UNKNOWN_ELEM, ///< With smallest unknown element
    CPLTSET_BVAR_MAX_UNKNOWN_ELEM  ///< With largest unknown element 
  };
  
  /// Which values to select first for branching. 
  enum CpltSetBvalSel { 
    CPLTSET_BVAL_MIN,                  ///< Select first domain value and try equality
    CPLTSET_BVAL_MAX,                  ///< Select last  domain value and try equality
    CPLTSET_BVAL_MIN_EX_FIRST,         ///< Select first domain value and try disequality
    CPLTSET_BVAL_MAX_EX_FIRST,         ///< Select last  domain value and try disequality
    CPLTSET_BVAL_MIN_UNKNOWN,          ///< Select smallest element in unknown set and try to include it
    CPLTSET_BVAL_MAX_UNKNOWN,          ///< Select largest  element in unknown set and try to include it
    CPLTSET_BVAL_MIN_UNKNOWN_EX_FIRST, ///< Select smallest element in unknown set and try to exclude it
    CPLTSET_BVAL_MAX_UNKNOWN_EX_FIRST  ///< Select largest  element in unknown set and try to exclude it
  };

  ///\name Posting bdd propagators
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
  
  /// Post propagator for \f$ dom(x) \sim_r y\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, IntVar x, CpltSetRelType r, CpltSetVar s);

  /// Post propagator for \f$ x \sim_r y\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetRelType r, CpltSetVar y);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, 
      CpltSetRelType r, CpltSetVar z);

  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, CpltSetOpType o, CpltSetVar y, 
      SetRelType r, CpltSetVar z);

  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, 
      CpltSetRelType r, CpltSetVar z);

  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetOpType o, CpltSetVar y, 
      SetRelType r, CpltSetVar z);

  /// Post propagator for \f$ l \leq |x| \leq u \f$ 
  GECODE_CPLTSET_EXPORT void
  cardinality(Space* home, CpltSetVar x, unsigned int l, unsigned int u);
  
  /// Post propagator for \f$ l |x| = c \f$ 
  GECODE_CPLTSET_EXPORT void
  cardinality(Space* home, CpltSetVar x, unsigned int c);
  
  GECODE_CPLTSET_EXPORT void 
  singleton(Space* home, IntVar x, CpltSetVar s);
  
  /// Post propagator for \f$ \bigcup_{i\neq index} glb(x_{i}) \parallel x_{index} \f$ 
  GECODE_CPLTSET_EXPORT void
  disjointsudoku(Space* home,  CpltSetVar x, int order);

  /// Post propagator for \f$ |x \cap is| =  c \f$ 
  GECODE_CPLTSET_EXPORT void
  exactly(Space* home, CpltSetVar x, IntSet& is, int c);

  /// Post propagator for \f$ 0 \leq |x \cap is| \leq c \f$ 
  GECODE_CPLTSET_EXPORT void
  atmost(Space* home, CpltSetVar x, IntSet& is, int c);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \f$ 
  GECODE_CPLTSET_EXPORT void
  atmost(Space* home, CpltSetVar x, CpltSetVar y, int c);

  /// Post propagator for \f$ 0 \leq |x \cap y| \leq c \wedge x \sim_{lex} y\f$ 
  GECODE_CPLTSET_EXPORT void
  atmostLex(Space* home, CpltSetVar x, CpltSetVar y, int c, 
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

  /// Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \f$ 
  GECODE_CPLTSET_EXPORT void
  partition(Space* home, const IntVarArgs& x, const CpltSetVar& y);

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
                   int c);

  /**
   * \brief Post propagator for \f$ |\bigcup_{i=0}^{n-1} x_i = U \wedge 
   *                       \forall i \in \{0, \dots, n-1\}: |x_i| = c \f$ 
   */
  GECODE_CPLTSET_EXPORT void
  partitionCard(Space* home, const CpltSetVarArgs& x, int c);


  /// Post propagator for \f$ t = \bigcup_{i\in s}  x_i \f$
  GECODE_CPLTSET_EXPORT void
  selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar s,
              CpltSetVar t);

  /// Post propagator for \f$ s = \bigcup_{(x_i \subseteq t) \wedge x_i \neq \emptyset } i \f$
  GECODE_CPLTSET_EXPORT void
  findNonEmptySub(Space* home, const CpltSetVarArgs& x,
                  CpltSetVar s, CpltSetVar t);

  /// Post propagator for \f$ s = \{x\} \f$
  GECODE_CPLTSET_EXPORT void
  singleton(Space* home, IntVar x, CpltSetVar s);

  /// Post propagator for \f$ t = \bigcup_{i\in s}  x_i \f$
  GECODE_CPLTSET_EXPORT void
  range(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t);

  /// Post propagator for \f$ s = \bigcup_{x_i \in t} i \f$
  GECODE_CPLTSET_EXPORT void
  roots(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
        const CpltSetVarArgs& allvars);

  /** \brief Post propagator for \f$ x_i\neq x_j\f$ for all \f$0\leq i\neq j<|x| 
   *  \wedge t = \bigcup_{i\in s}  x_i \f$
   */ 
  GECODE_CPLTSET_EXPORT void
  alldifferent(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
               const CpltSetVarArgs& allvars);

  /* \brief Post propagator \f$\wedge r = \bigcup_{i\in s}  x_i \f$
   */ 
/*   GECODE_CPLTSET_EXPORT void
     permutation(Space* home, const IntVarArgs& x, const IntSet& r, 
                 CpltSetVar s, CpltSetVar t, SetConLevel scl = SCL_DOM);
  // for permutation we need a CONST VIEW and DONT NEED Card = N THAT IS GOOD
*/

  /** \brief Post propagator for \f$ |\displaystyle\bigcup_{i = 0}^{|x| - 1} x_i| = n 
   *  \wedge t = \bigcup_{i\in s}  x_i \f$
   * 
   */
  GECODE_CPLTSET_EXPORT void
  nvalue(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, int n,
         const CpltSetVarArgs& allvars);

  /** \brief Post propagator for \f$ t = \bigcup_{i\in s} x_i \wedge v = \bigcup_{i\in u} y_i
   *  \wedge v \subseteq t \f$
   * 
   *  |x| and |y| may be different
   */
  GECODE_CPLTSET_EXPORT void
  uses(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t,
       const IntVarArgs& y, CpltSetVar u, CpltSetVar v);
  
  /**
   * \brief Ordering all declared bdd variables \f$ x_0, \dots, x_{n-1}\f$ such that 
   * \f$ \forall i\in \{0, \dots, n - 1\}: x_{0_{1}} \prec x_{{n-1}_{1}}, \dots, x_{0_{k-1}} \prec x_{{n-1}_{k-1}}\f$
   */
  GECODE_CPLTSET_EXPORT void
  hls_order(Space* home, const CpltSetVarArray& x);
  //@}

  /// Branch over all \a x with variable selection \a vars and value selection \a vals
  GECODE_CPLTSET_EXPORT void
  branch(Space* home, const CpltSetVarArgs& x, CpltSetBvarSel vars,     
         CpltSetBvalSel vals);


  ///\name Use bdd variables with set propagators
  //@{
  /**
   * \defgroup TaskIntSetRel Relation constraints
   * \ingroup TaskIntSet
   *
   */

  //@{

   /// Post propagator for \f$ x \sim_r y\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, CpltSetVar x, SetRelType r, CpltSetVar y);

  /// Post propagator for \f$ (x \sim_r y) \Leftrightarrow b \f$
  /// MISSING!
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar x, SetRelType r, CpltSetVar y, BoolVar b);
  
  /// Post propagator for \f$ s \sim_r \{x\}\f$
  /// MISSING
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar s, SetRelType r, IntVar x);

  /// Post propagator for \f$ \{x\} \sim_r s\f$
  GECODE_CPLTSET_EXPORT void
  rel(Space* home, IntVar x, SetRelType r, CpltSetVar s);

  /// Post propagator for \f$ (s \sim_r \{x\}) \Leftrightarrow b \f$
  /// MISSING
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar s, SetRelType r, IntVar x, BoolVar b);

  /// Post propagator for \f$ (\{x\} \sim_r s) \Leftrightarrow b \f$
  /// MISSING
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, IntVar x, SetRelType r, CpltSetVar s, BoolVar b);

  /// Post propagator for \f$\forall i\in s:\ i \sim_r x\f$
  /// MISSING
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar s, IntRelType r, IntVar x);

  /// Post propagator for \f$\forall i\in s:\ x \sim_r i\f$
  /// MISSING
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, IntVar x, IntRelType r, CpltSetVar s);

  //@}

  /**
   * \defgroup TaskIntSetRelOp Set operation/relation constraints
   * \ingroup TaskIntSet
   *
   */

  //@{

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar x, SetOpType op, CpltSetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, SetOpType op, const CpltSetVarArgs& x, CpltSetVar y);

  /// Post propagator for \f$ y = \diamond_{\mathit{op}} x\f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, SetOpType op, const IntVarArgs& x, CpltSetVar y);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, const IntSet& x, SetOpType op, CpltSetVar y,
  //     SetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar x, SetOpType op, const IntSet& y,
  //     SetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar x, SetOpType op, CpltSetVar y,
  //     SetRelType r, const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, const IntSet& x, SetOpType op, const IntSet& y,
  //     SetRelType r, CpltSetVar z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, const IntSet& x, SetOpType op, CpltSetVar y, SetRelType r,
  //     const IntSet& z);

  /// Post propagator for \f$ (x \diamond_{\mathit{op}} y) \sim_r z \f$
  // GECODE_CPLTSET_EXPORT void
  // rel(Space* home, CpltSetVar x, SetOpType op, const IntSet& y, SetRelType r,
  //     const IntSet& z);

  //@}

  /**
   * \defgroup TaskIntSetConvex Convexity constraints
   * \ingroup TaskIntSet
   *
   */
  //@{

  /// Post propagator that propagates that \a x is convex 
  // GECODE_CPLTSET_EXPORT void
  // convex(Space* home, CpltSetVar x);

  /// Post propagator that propagates that \a y is the convex hull of \a x
  // GECODE_CPLTSET_EXPORT void
  // convexHull(Space* home, CpltSetVar x, CpltSetVar y);

  //@}

  /**
   * \defgroup TaskIntSetSequence Sequence constraints
   * \ingroup TaskIntSet
   *
   */
  //@{

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$
  // GECODE_CPLTSET_EXPORT void
  // sequence(Space* home, const CpltSetVarArgs& x);

  /// Post propagator for \f$\forall 0\leq i< |x|-1 : \max(x_i)<\min(x_{i+1})\f$ and \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
  // GECODE_CPLTSET_EXPORT void
  // sequentialUnion(Space* home, const CpltSetVarArgs& y, CpltSetVar x);

  //@}

  /**
   * \defgroup TaskIntSetDistinct Distinctness constraints
   * \ingroup TaskIntSet
   *
   */
  //@{


  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
  // GECODE_CPLTSET_EXPORT void
  // atmostOne(Space* home, const CpltSetVarArgs& x, unsigned int c);

  /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : x_i\neq x_j\f$
  // GECODE_CPLTSET_EXPORT void
  // distinct(Space* home, const CpltSetVarArgs& x, unsigned int c);

  /// Post propagator for \f$\forall 0\leq i\leq |x| : \forall 0\leq i<j\leq |x| : x_i\neq x_j\f$
  // GECODE_CPLTSET_EXPORT void
  // distinct(Space* home, const CpltSetVarArgs& x);

  //@}

  /**
   * \defgroup TaskIntSetConnect Connection constraints to finite domain variables
   * \ingroup TaskIntSet
   *
   */

  //@{

  /// Post propagator that propagates that \a x is the minimal element of \a s 
  // GECODE_CPLTSET_EXPORT void
  // min(Space* home, CpltSetVar s, IntVar x);
  
  /// Post propagator that propagates that \a x is the maximal element of \a s 
  // GECODE_CPLTSET_EXPORT void
  // max(Space* home, CpltSetVar s, IntVar x);
  
  /// Post propagator that propagates that \a s contains the \f$x_i\f$, which are sorted in non-descending order 
  // GECODE_CPLTSET_EXPORT void
  // match(Space* home, CpltSetVar s, const IntVarArgs& x);
  
  /// Post propagator for \f$x_i=j \Leftrightarrow i\in y_j\f$
  // GECODE_CPLTSET_EXPORT void
  // channel(Space* home, const IntVarArgs& x,const CpltSetVarArgs& y);
  
  /// Post propagator for \f$ |s|=x \f$ 
  // GECODE_CPLTSET_EXPORT void
  // cardinality(Space* home, CpltSetVar s, IntVar x);


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
  // GECODE_CPLTSET_EXPORT void
  // weights(Space* home, const IntArgs& elements, const IntArgs& weights,
  //         CpltSetVar x, IntVar y);

  //@}

 /**
   * \brief Post propagator for \f$ z=\bigcup\langle x_0,\dots,x_{n-1}\rangle[y] \f$
   * If \a y is the empty set, \a z will also be constrained to be empty
   * (as an empty union is empty).
   */
  // GECODE_CPLTSET_EXPORT void
  // selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z);


  /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \f$ \mathcal{U} \f$ as universe
   *
   * If \a y is empty, \a z will be constrained to be the universe
   * \f$ \mathcal{U} \f$ (as an empty intersection is the universe).
   */
  // GECODE_CPLTSET_EXPORT void
  // selectInter(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z);

  /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \a u as universe
   *
   * If \a y is empty, \a z will be constrained to be the given universe
   * \a u (as an empty intersection is the universe).
   */
  // GECODE_CPLTSET_EXPORT void
  // selectInterIn(Space* home, const CpltSetVarArgs& x, CpltSetVar y, CpltSetVar z,
  //               const IntSet& u);

  /// Post propagator for \f$ \parallel\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
  // GECODE_CPLTSET_EXPORT void
  // selectDisjoint(Space* home, const CpltSetVarArgs& x, CpltSetVar y);

  /// Post propagator for \f$ z=\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
  // GECODE_CPLTSET_EXPORT void
  // selectSet(Space* home, const CpltSetVarArgs& x, IntVar y, CpltSetVar z);

  //@}
  //}@ 
}

#endif


// STATISTICS: set-post
