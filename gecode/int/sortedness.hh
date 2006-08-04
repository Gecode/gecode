/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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
 *
 */

#ifndef __GECODE_INT_SORTEDNESS_HH__
#define __GECODE_INT_SORTEDNESS_HH__


#include "gecode/int.hh"
#include "gecode/int/rel.hh"

/**
 * \namespace Gecode::Int::Sortedness
 * \brief %Sortedness propagators
 */

namespace Gecode { namespace Int { namespace Sortedness {

  /**
   * \brief Bounds consistent sortedness propagator
   * \par [Reference]
   *  The algorithm is taken from: \n
   *  Sven Thiel: Efficient Algorithms for Constraint Propagation
   *  and for Processing Tree Descriptions (pages 39 to 59)
   *  [http://www.mpi-sb.mpg.de/~sthiel/thesis.pdf]
   *
   * Requires \code #include "gecode/int/sortedness.hh" \endcode
   * and \code #include "gecode/int/distinct.hh" \endcode.
   * The latter is required for the extended version of sortedness
   * including permutation views.
   * \ingroup FuncIntProp
   * \note The sortedness propagator does not support sharing!
   */

  template<class View, class Tuple, bool Perm>
  class Sortedness : public Propagator {
  protected:

    /**
     *  \brief Views to be sorted
     *
     *   If Tuple is instantiated to ViewTuple<View,2>,
     *   \f$xz_i\f$ is a pair \f$(x_i, z_i)\f$, where
     *   \f$x_i\f$ denotes an unsorted view
     *   and \f$z_i\f$ is the explicit
     *   permutation view mapping \f$x_i\f$ to its unsorted counterpart
     *   \f$y_{z_i}\f$.
     *   If Tuple is instantiated to ViewTuple<View,1>,
     *   \f$xz_i\f$ is the unsorted view \f$x_i\f$ lifted to a ViewTuple.
     */
    ViewArray<Tuple> xz;

    /// Views denoting the sorted version of \a x
    ViewArray<View> y;
    /// original y array
    ViewArray<View> w;
    /// connection to dropped view
    int reachable;
    /// Constructor for posting
    Sortedness(Space*, ViewArray<Tuple>&, ViewArray<View>&);
    /// Constructor for cloning
    Sortedness(Space* home, bool share, Sortedness<View, Tuple, Perm>& p);

  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function returning PC_LINEAR_HI
    virtual PropCost cost (void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for the views \a xz and \a y
    static  ExecStatus post(Space*, ViewArray<Tuple>&, ViewArray<View>&);
  };


}}}

#include "gecode/int/sortedness/sortsup.icc"
#include "gecode/int/sortedness/order.icc"
#include "gecode/int/sortedness/matching.icc"
#include "gecode/int/sortedness/narrowing.icc"
#include "gecode/int/sortedness/sortedness.icc"

#endif


// STATISTICS: int-prop

