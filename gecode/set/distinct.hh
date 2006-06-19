/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
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

#ifndef __GECODE_SET_ATMOSTONE_HH__
#define __GECODE_SET_ATMOSTONE_HH__

#include "gecode/set.hh"
#include "gecode/set/distinct/binomial.icc"

namespace Gecode { namespace Set { namespace Distinct {

  /**
   * \namespace Gecode::Set::Distinct
   * \brief Propagators for global distinctness constraints
   */

  /**
   * \brief %Propagator for the AtMostOneIntersection constraint
   *
   * Requires \code #include "gecode/set/distinct.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class AtmostOne :
    public NaryPropagator<SetView, PC_SET_ANY> {
  protected:
    /// Cardinality of the sets
    unsigned int c;
    /// Constructor for cloning \a p
    AtmostOne(Space* home, bool share,AtmostOne& p);
    /// Constructor for posting
    AtmostOne(Space* home,ViewArray<SetView>&,unsigned int);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
    static ExecStatus post(Space* home,ViewArray<SetView> x,unsigned int c);
  };

  /**
   * \brief %Propagator for the n-ary distinct constraint with fixed cardinalities
   *
   * Requires \code #include "gecode/set/distinct.hh" \endcode
   * \ingroup FuncSetProp   
   */

  class Distinct :
    public NaryPropagator<SetView, PC_SET_ANY> {
  protected:
    /// Binomial coefficients table
    Binomial bin;
    /// Cardinality of the sets
    unsigned int c;
    /// Constructor for cloning \a p
    Distinct(Space* home, bool share,Distinct& p);
    /// Constructor for posting
    Distinct(Space* home,ViewArray<SetView>&,unsigned int);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home, bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$\forall 0\leq i\leq |x| : |x_i|=c\f$ and \f$\forall 0\leq i<j\leq |x| : |x_i\cap x_j|\leq 1\f$
    static ExecStatus post(Space* home,ViewArray<SetView> x,unsigned int c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };


}}}

#include "gecode/set/distinct/atmostOne.icc"
#include "gecode/set/distinct/distinct.icc"

#endif

// STATISTICS: set-prop
