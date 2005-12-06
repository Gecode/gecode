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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SET_CONVEX_HH__
#define __GECODE_SET_CONVEX_HH__

#include "set.hh"

namespace Gecode { namespace Set { namespace Convex {

  /**
   * \namespace Gecode::Set::Convex
   * \brief Propagators for convexity
   */

  /**
   * \brief %Propagator for the convex constraint
   *
   * Requires \code #include "set/convex.hh" \endcode
   * \ingroup FuncSetProp
   */

  class Convex : public UnaryPropagator<SetView,PC_SET_ANY> {
  protected:
    /// Constructor for cloning \a p
    Convex(Space* home, bool share,Convex& p);
    /// Constructor for posting
    Convex(Space* home,SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    /// Post propagator that propagates that \a x is convex 
    static  ExecStatus  post(Space* home,SetView x);
  };

  /**
   * \brief %Propagator for the convex hull constraint
   *
   * Requires \code #include "set/convex.hh" \endcode
   * \ingroup FuncSetProp
   */

  class ConvexHull : public Propagator {
  protected:
    SetView x0;
    SetView x1;
    /// Constructor for cloning \a p
    ConvexHull(Space* home, bool share,ConvexHull&);
    /// Constructor for posting
    ConvexHull(Space* home,SetView,SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*  copy(Space* home,bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    GECODE_SET_EXPORT virtual PropCost    cost(void) const;
    /// Destructor
    GECODE_SET_EXPORT virtual ~ConvexHull(void);
    /// Post propagator that propagates that \a y is the convex hull of \a x
    static  ExecStatus  post(Space* home,SetView x,SetView y);
  };


}}}

#include "set/convex/conv.icc"
#include "set/convex/hull.icc"

#endif

// STATISTICS: set-prop
