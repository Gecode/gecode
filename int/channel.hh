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
   * \brief Combine view with information for domain propagation
   *
   */
  template <class View>
  class DomInfo {
  public:
    View         view;
    unsigned int card;
    int          min;
    int          max;
    /// Allocate memory from space \a home for \a n elements
    static DomInfo<View>* allocate(Space* home, int n);
    /// Initialize
    void init(View x, int n);
    /// Update during cloning
    void update(Space* home, bool share, DomInfo<View>& vcb);
    /// Check whether propagation for assignment is to be done
    bool doval(void) const;
    /// Check whether propagation for domain is to be done
    bool dodom(void) const;
    /// Record that view got assigned
    void assigned(void);
    /// Record that one value got removed
    void removed(int i);
    /// Update the cardinality and bounds information after pruning
    void done(void);
  };

  /**
   * \brief Naive domain-consistent channel propagator
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

#include "int/channel/dom.icc"

#endif

// STATISTICS: int-prop

