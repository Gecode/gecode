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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_SET_CONVEX_HH__
#define __GECODE_SET_CONVEX_HH__

#include "gecode/set.hh"

namespace Gecode { namespace Set { namespace Convex {

  /**
   * \namespace Gecode::Set::Convex
   * \brief Propagators for convexity
   */

  /**
   * \brief %Propagator for the convex constraint
   *
   * Requires \code #include "gecode/set/convex.hh" \endcode
   * \ingroup FuncSetProp
   */

  class Convex : public UnaryPropagator<SetView,PC_SET_ANY> {
  protected:
    /// Constructor for cloning \a p
    Convex(Space* home, bool share, Convex& p);
    /// Constructor for posting
    Convex(Space* home, SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    /// Post propagator that propagates that \a x is convex 
    static  ExecStatus  post(Space* home,SetView x);
    /// Name of this propagator
    GECODE_SET_EXPORT virtual const char* name(void) const;
  };

  /**
   * \brief %Propagator for the convex hull constraint
   *
   * Requires \code #include "gecode/set/convex.hh" \endcode
   * \ingroup FuncSetProp
   */

  class ConvexHull : public BinaryPropagator<SetView,PC_SET_ANY> {
  protected:
    /// Constructor for cloning \a p
    ConvexHull(Space* home, bool share, ConvexHull&);
    /// Constructor for posting
    ConvexHull(Space* home, SetView, SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*  copy(Space* home,bool);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    /// Post propagator that propagates that \a y is the convex hull of \a x
    static  ExecStatus  post(Space* home,SetView x,SetView y);
    /// Name of this propagator
    GECODE_SET_EXPORT virtual const char* name(void) const;
  };


}}}

#include "gecode/set/convex/conv.icc"
#include "gecode/set/convex/hull.icc"

#endif

// STATISTICS: set-prop
