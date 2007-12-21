/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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
 *
 */

#ifndef __GECODE_INT_SORTED_HH__
#define __GECODE_INT_SORTED_HH__

#include "gecode/int.hh"

/**
 * \namespace Gecode::Int::Sorted
 * \brief %Sorted propagators
 */

namespace Gecode { namespace Int { namespace Sorted {

  /**
   * \brief Bounds-consistent sortedness propagator
   *
   * The algorithm is taken from: Sven Thiel, Efficient Algorithms 
   * for Constraint Propagation and for Processing Tree Descriptions,
   * PhD thesis, Universität des Saarlandes, Germany, 2004, pages 39-59.
   *
   * Requires \code #include "gecode/int/sorted.hh" \endcode
   * \ingroup FuncIntProp
   */

  template<class View, class Tuple, bool Perm>
  class Sorted : public Propagator {
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
    Sorted(Space*, ViewArray<Tuple>&, ViewArray<View>&);
    /// Constructor for cloning
    Sorted(Space* home, bool share, Sorted<View, Tuple, Perm>& p);

  public:
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function returning PC_LINEAR_HI
    virtual PropCost cost(PropModEvent pme) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, PropModEvent pme);
    /// Post propagator for the views \a xz and \a y
    static  ExecStatus post(Space*, ViewArray<Tuple>&, ViewArray<View>&);
  };


}}}

#include "gecode/int/sorted/sortsup.icc"
#include "gecode/int/sorted/order.icc"
#include "gecode/int/sorted/matching.icc"
#include "gecode/int/sorted/narrowing.icc"
#include "gecode/int/sorted/propagate.icc"

#endif


// STATISTICS: int-prop

