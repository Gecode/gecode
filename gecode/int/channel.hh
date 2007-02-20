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

#include "gecode/support/sentinel-stack.hh"

/**
 * \namespace Gecode::Int::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Int { namespace Channel {

  /// Processing stack
  typedef Support::SentinelStack<int> ProcessStack;

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
   * Only propagates if views become assigned. If \a shared is true,
   * the same views can be contained in both \a x and \a y.
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, bool shared>
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
    /// Post propagator for channeling
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
   * If \a shared is true, the same views can be contained in both 
   * \a x and \a y.   
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, bool shared>
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
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  /**
   * \brief Link propagator for a single Boolean view
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  class LinkSingle :
    public MixBinaryPropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_VAL> {
  private:
    using MixBinaryPropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_VAL>::x0;
    using MixBinaryPropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_VAL>::x1;

    /// Constructor for cloning \a p
    LinkSingle(Space* home, bool share, LinkSingle& p);
    /// Constructor for posting
    LinkSingle(Space* home, BoolView x0, IntView x1);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ x_0 = x_1\f$
    static  ExecStatus post(Space* home, BoolView x0, IntView x1);
  };

  /**
   * \brief Link propagator for multiple Boolean views
   *
   * Requires \code #include "gecode/int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  class LinkMulti :
    public MixNaryOnePropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_DOM> {
  private:
    using MixNaryOnePropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_DOM>::x;
    using MixNaryOnePropagator<BoolView,PC_BOOL_VAL,IntView,PC_INT_DOM>::y;
    /// Offset value
    int o;
    /// Constructor for cloning \a p
    LinkMulti(Space* home, bool share, LinkMulti& p);
    /// Constructor for posting
    LinkMulti(Space* home, ViewArray<BoolView>& x, IntView y, int o0);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function 
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$ x_i = 1\leftrightarrow y=i+o\f$
    static  ExecStatus post(Space* home, 
                            ViewArray<BoolView>& x, IntView y, int o);
  };

}}}

#include "gecode/int/channel/base.icc"
#include "gecode/int/channel/val.icc"
#include "gecode/int/channel/dom.icc"

#endif

// STATISTICS: int-prop

