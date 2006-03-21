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

/**
 * \namespace Gecode::Int::Channel
 * \brief %Channel propagators
 */

namespace Gecode { namespace Int { namespace Channel {

  /**
   * \brief Naive domain-consistent channel propagator
   *
   * Requires \code #include "int/channel.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Dom : public Propagator {
  protected:
    ViewArray<View> x;
    ViewArray<View> y;
    class CardInfo {
    public:
      unsigned int x;
      unsigned int y;
    };
    CardInfo* ci;
    /// Constructor for cloning \a p
    Dom(Space* home, bool share, Dom& p);
    /// Constructor for posting
    Dom(Space* home, ViewArray<View>& x, ViewArray<View>& y);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Propagation cost
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for channeling on \a xy
    static  ExecStatus post(Space* home, 
			    ViewArray<View>& x, ViewArray<View>& y);
    /// Destructor
    ~Dom(void);
  };

}}}

#include "int/channel/dom.icc"

#endif

// STATISTICS: int-prop

