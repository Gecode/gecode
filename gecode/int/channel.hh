/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#ifndef __GECODE_INT_CHANNEL_HH__
#define __GECODE_INT_CHANNEL_HH__

#include "gecode/int.hh"
#include "gecode/int/distinct.hh"

/**
 * \namespace Gecode::Int::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Int { namespace Channel {

  /**
   * \brief Base-class for channel propagators
   *
   */
  template <class Info, PropCond pc>
  class Base : public Propagator {
  protected:
    /// Number of views (actually twice as many for both x and y)
    int n;
    /// Total number of not assigned views (not known to be assigned)
    int n_na;
    /// View and information for both \a x and \a y
    Info* xy;
    /// Constructor for cloning \a p
    Base(Space* home, bool share, Base<Info,pc>& p);
    /// Constructor for posting
    Base(Space* home, int n, Info* xy);
  public:
    /// Propagation cost
    virtual PropCost cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };


  /**
   * \brief Combine view with information for value propagation
   *
   */
  template <class View> class ValInfo;

  /**
   * \brief Naive channel propagator
   *
   * Only propagates if views become assigned.
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Val : public Base<ValInfo<View>,PC_INT_VAL> {
 protected:
    using Base<ValInfo<View>,PC_INT_VAL>::n;
    using Base<ValInfo<View>,PC_INT_VAL>::n_na;
    using Base<ValInfo<View>,PC_INT_VAL>::xy;
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val& p);
    /// Constructor for posting
    Val(Space* home, int n, ValInfo<View>* xy);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for channeling on \a xy
    static  ExecStatus post(Space* home, int n, ValInfo<View>* xy);
  };

  /**
   * \brief Combine view with information for domain propagation
   *
   */
  template <class View> class DomInfo;

  /**
   * \brief Domain-consistent channel propagator
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public Base<DomInfo<View>,PC_INT_DOM> {
  protected:
    using Base<DomInfo<View>,PC_INT_DOM>::n;
    using Base<DomInfo<View>,PC_INT_DOM>::n_na;
    using Base<DomInfo<View>,PC_INT_DOM>::xy;
    /// Propagation controller for propagating distinct
    Distinct::DomCtrl<View> dc;
    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom& p);
    /// Constructor for posting
    Dom(Space* home, int n, DomInfo<View>* xy);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Propagation cost
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for channeling on \a xy
    static  ExecStatus post(Space* home, int n, DomInfo<View>* xy);
    /// Returns size of propagation controller
    virtual size_t allocated(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

}}}

#include "gecode/int/channel/base.icc"
#include "gecode/int/channel/stack.icc"
#include "gecode/int/channel/val.icc"
#include "gecode/int/channel/dom.icc"

#endif

// STATISTICS: int-prop

