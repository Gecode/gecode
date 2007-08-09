/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004/2005
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

#ifndef __GECODE_INT_GCC_HH__
#define __GECODE_INT_GCC_HH__

#include "gecode/int.hh"
#include "gecode/support/sort.hh"
#include "gecode/support/static-stack.hh"
#include "gecode/int/gcc/gccbndsup.icc"
#include "gecode/int/gcc/graphsup.icc"
#include "gecode/int/gcc/occur.icc"

/**
 * \namespace Gecode::Int::GCC
 * \brief Global cardinality propagators
 * \note The global cardinality propagator with fixed cardinalities does not
 *       not support sharing!
 *
 */

namespace Gecode { namespace Int { namespace GCC {

  /**
   * \brief Bounds-consistent global cardinality propagator
   * \par [Reference]
   *  The algorithm is taken from: \n
     \verbatim
     @PROCEEDINGS{quimper-efficient,
     title     = {An Efficient Bounds Consistency Algorithm
                  for the Global Cardinality Constraint},
     year      = {2003},
     volume    = {2833},
     address   = {Kinsale, Ireland},
     month     = {September},
     author    = {Claude-Guy Quimper and Peter van Beek
                  and Alejandro López-Ortiz
                  and Alexander Golynski and Sayyed Bashir Sadjad},
     booktitle = {Proceedings of the 9th International Conference
                  on Principles and Practice of
                  Constraint Programming},
     pages     = {600--614},
     url       = {http://ai.uwaterloo.ca/~vanbeek/publications},
     }
     @TECHREPORT{quimper-efficientTR,
     author      = {Claude-Guy Quimper and Peter van Beek
                    and Alejandro López-Ortiz
                    and Alexander Golynski and
                    Sayyed Bashir Sadjad},
     title       = {An Efficient Bounds Consistency Algorithm
                    for the Global Cardinality Constraint,
                    Technical Report},
     institution = {School of Computer Science,
                    University of Waterloo, Waterloo, Canada},
     year        = {2003},
     url         = {http://ai.uwaterloo.ca/~vanbeek/publications},
     }
     \endverbatim
   *
   * This implementation uses the code that is provided
   * by Peter Van Beek:\n
   * http://ai.uwaterloo.ca/~vanbeek/software/software.html
   * The code here has only been slightly modified to fit Gecode
   * (taking idempotent/non-idempotent propagation into account)
   * and uses a more efficient layout of datastructures (keeping the
   * number of different arrays small).
   *
   * The Bnd class is used to post the propagator and BndImp
   * is the actual implementation taking shared variables into account.
   *
   * Requires \code #include "gecode/int/gcc.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View, class Card, bool isView>
  class Bnd{
  public:
    /**
     * \brief Post propagator for views \a x and cardinalities \a k
     *
     * \a all denotes whether the propagator uses all value occuring
     * in the domains of the problem vies specified in \a x. Also
     * checks whether \a x and \a k contain shared views.
     */
    static  ExecStatus  post(Space* home,
                             ViewArray<View>& x,
                             ViewArray<Card>& k,
                             bool all);
  };

  /**
   * \brief Implementation of the bounds consistent
   * global cardinality propagator
   */
  template <class View, class Card, bool isView, bool shared>
  class BndImp : public Propagator {
    friend class Bnd<View, Card, isView>;
  protected:
    /// Views on which to perform bounds-propagation
    ViewArray<View> x;
    /// Array containing either fixed cardinalities or CardViews
    ViewArray<Card> k;
    /**
     * \brief  Data structure storing the sum of the views lower bounds
     * Necessary for reasoning about the interval capacities in the
     * propagation algorithm.
     */
    PartialSum<Card>* lps;
    /// Data structure storing the sum of the views upper bounds
    PartialSum<Card>* ups;
    /**
     * \brief Stores whether cardinalities are all assigned
     *
     * If all cardinalities are assigned the propagation algorithm
     * only has to perform propagation for the upper bounds.
     */
    bool card_fixed;
    /**
     * \brief Stores whether all values in domains of the x-views are used
     * for cardinality reasoning or whether we only use a subset of them.
     */
    bool card_all;
    /**
     * \brief Stores whether the minium required occurences of
     *        the cardinalities are all zero. If so, we do not need
     *        to perform lower bounds propagation.
     */
    bool skip_lbc;
    /// Constructor for posting
    BndImp(Space* home, ViewArray<View>&, ViewArray<Card>&, bool, bool, bool);
    /// Constructor for cloning \a p
    BndImp(Space* home, bool share, BndImp<View, Card, isView, shared>& p);

  public:
    /// Destructor
    virtual size_t dispose(Space* home);
    /// Return how much extra memory is allocated by the propagator
    virtual size_t allocated(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Name of this propagator
    virtual const char* name(void) const;    
    /// Cost funtion returning dynamic PC_LINEAR_HI.
    virtual PropCost    cost (void) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
  };

  /**
   * \brief Performs bounds-consistent global cardinality propagation
   *
   * This function implements the propagation algorithm for
   * the bounds-consistent global cardinality propagator implemented
   * in GCC::Bnd. It is available as seperate function in order
   * to allow staging for GCC::Dom and GCC::Bnd though staging is
   * currently not used due to technical difficulties.
   */
  template <class View, class Card, bool isView, bool shared>
  ExecStatus prop_bnd(Space* home, ViewArray<View>&, ViewArray<Card>&,
                      PartialSum<Card>*&, PartialSum<Card>*&,
                      bool, bool, bool);

  /**
   * \brief Domain-consistent global cardinality propagator
   * \par [Reference]
   *  The algorithm is taken from: \n
   * \anchor CardVarNPCompl
   \verbatim
     @PROCEEDINGS{improvedgcc,
     title     = {Improved Algorithms for the
                  Global Cardinality Constraint},
     year      = {2004},
     volume    = {3528},
     address   = {Toronto, Canada},
     month     = {September},
     author    = {Claude-Guy Quimper and Peter van Beek and
                  Alejandro López-Ortiz and Alexander Golynski},
     booktitle = {Proceedings of the 10th International
                  Conference on Principles and Practice of
                  Constraint Programming},
     url       = {http://ai.uwaterloo.ca/~vanbeek/publications},
     }
     \endverbatim
   *
   * Requires \code #include "gecode/int/gcc.hh" \endcode
   * \ingroup FuncIntProp
   */

  template <class View, class Card, bool isView>
  class Dom : public Propagator {
  protected:
    /// Views on which to perform domain-propagation
    ViewArray<View> x;
    /**
     * \brief Views used to channel information between \c x and \c k
     * (\f$ x \subseteq y \f$).
     */
    ViewArray<View> y;
    /// Array containing either fixed cardinalities or CardViews
    ViewArray<Card> k;
    /// Propagation is performed on a variable-value graph (used as cache)
    VarValGraph<View, Card, isView>* vvg;
    /**
     * \brief Stores whether cardinalities are all assigned
     *
     * If all cardinalities are assigned the propagation algorithm
     * only has to perform propagation for the upper bounds.
     */
    bool card_fixed;
    /**
     * \brief Stores whether all values in domains of the x-views are used
     * for cardinality reasoning or whether we only use a subset of them.
     */
    bool card_all;
    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom<View, Card, isView>& p);
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>&, ViewArray<Card>&, bool, bool);

  public:
    /// Destructor including deallocation of variable-value graph
    virtual size_t dispose(Space* home);
    /// Return how much extra memory is allocated by the propagator
    virtual size_t allocated(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * As the propagation stronlgy depends on the domain size of the
     * views on which propagation is performed, the propagation costs
     * are computed as follows, where \c d denotes the size of the
     * largest domain of a view in \c x:
     * - dynamic PC_LINEAR_LO ( \f$ d < 6\f$ )
     * - dynamic PC_LINEAR_HI ( \f$ 6 \leq d < \frac{n}{2} \f$ )
     * - dynamic PC_QUADRATIC_LO ( \f$ \frac{n}{2} \leq d < n^2 \f$)
     * - dynamic PC_CUBIC_LO  ( \f$ n^2 \leq d \f$)
     */
    virtual PropCost    cost (void) const;
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Name of this propagator
    virtual const char* name(void) const;    
    /**
     * \brief Post propagator for views \a x and cardinalities \a k
     *
     * \a all denotes whether the propagator uses all value occuring
     * in the domains of the problem vies specified in \a x.
     */
    static  ExecStatus  post(Space* home,
                             ViewArray<View>& x, ViewArray<Card>& k,
                             bool all);
  };

  /**
   * \brief Value consistent global cardinality propagator
   *
   * Requires \code #include "gecode/int/gcc.hh" \endcode
   * \ingroup FuncIntProp
   */


  template <class View, class Card, bool isView>
  class Val : public Propagator {
  protected:
    /// Views on which to perform value-propagation
    ViewArray<View> x;
    /// Array containing either fixed cardinalities or CardViews
    ViewArray<Card> k;
    /**
     * \brief Stores whether all values in domains of the x-views are used
     * for cardinality reasoning or whether we only use a subset of them.
     */
    bool card_all;
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val<View, Card, isView>& p );
    /// Constructor for posting
    Val(Space* home, ViewArray<View>&, ViewArray<Card>&, bool);

  public:
    /// Destructor
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost funtion returning dynamic PC_LINEAR_HI.
    virtual PropCost    cost (void) const;
        /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Name of this propagator
    virtual const char* name(void) const;    
    /**
     * \brief Post propagator for views \a x and cardinalities \a k
     *
     * \a all denotes whether the propagator uses all value occuring
     * in the domains of the problem vies specified in \a x.
     */
    static  ExecStatus  post(Space* home,
                             ViewArray<View>& x, ViewArray<Card>& k,
                             bool all);
  };

  /**
   * \brief Performs value-consistent global cardinality propagation
   *
   * This function implements the propagation algorithm for
   * the value-consistent global cardinality propagator implemented
   * in GCC::Val.
   */

  template <class View, class Card, bool isView>
  ExecStatus prop_val(Space* home, ViewArray<View>&, ViewArray<Card>&,
                      bool&);

}}}

#include "gecode/int/gcc/ubc.icc"
#include "gecode/int/gcc/lbc.icc"
#include "gecode/int/gcc/val.icc"
#include "gecode/int/gcc/bnd.icc"
#include "gecode/int/gcc/dom.icc"

#endif


// STATISTICS: int-prop

