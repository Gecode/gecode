/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2005-08-09 17:26:38 +0200 (Tue, 09 Aug 2005) $ by $Author: schulte $
 *     $Revision: 2190 $
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

#include "int.hh"
#include "int/distinct.hh"

/**
 * \namespace Gecode::Int::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Int { namespace Channel {

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
   * Requires \code #include "int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Val : public Propagator {
  protected:
    /// Number of views (actually twice as many for both x and y)
    int n;
    /// Total number of not assigned views (not known to be assigned)
    int n_na;
    /// View and information for \a x
    ValInfo<View>* x;
    /// View and information for \a y
    ValInfo<View>* y;
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val& p);
    /// Constructor for posting
    Val(Space* home, int n, ValInfo<View>* x, ValInfo<View>* y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Propagation cost
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for channeling on between \a x and \ ay
    static  ExecStatus post(Space* home, int n, 
			    ValInfo<View>* x, ValInfo<View>* y);
    /// Destructor
    ~Val(void);
  };

  /**
   * \brief Combine view with information for domain propagation
   *
   */
  template <class View> class DomInfo;

  /**
   * \brief Domain-consistent channel propagator
   *
   * Requires \code #include "int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public Propagator {
  protected:
    /// Number of views (actually twice as many for both x and y)
    int n;
    /// Total number of not assigned views (not known to be assigned)
    int n_na;
    /// View and information for \a x
    DomInfo<View>* x;
    /// View and information for \a y
    DomInfo<View>* y;
    /// Propagation controller for propagating distinct
    Distinct::DomCtrl<View> dc;
    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom& p);
    /// Constructor for posting
    Dom(Space* home, int n, DomInfo<View>* x, DomInfo<View>* y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Propagation cost
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for channeling on between \a x and \ ay
    static  ExecStatus post(Space* home, int n, 
			    DomInfo<View>* x, DomInfo<View>* y);
    /// Flush propagation controller
    virtual void flush(void);
    /// Returns size of propagation controller
    virtual size_t size(void) const;
    /// Destructor
    ~Dom(void);
  };

}}}

#include "int/channel/stack.icc"
#include "int/channel/val.icc"
#include "int/channel/dom.icc"

#endif

// STATISTICS: int-prop

