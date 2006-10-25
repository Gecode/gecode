/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#ifndef __GECODE_INT_ELEMENT_HH__
#define __GECODE_INT_ELEMENT_HH__

#include "gecode/int.hh"
#include "gecode/int/rel.hh"

#include "gecode/support/shared-array.hh"
#include "gecode/support/sort.hh"

#include "gecode/iter.hh"

/**
 * \namespace Gecode::Int::Element
 * \brief %Element propagators
 */

namespace Gecode { namespace Int { namespace Element {

  /*
   * Element constraint for array of integers
   *
   */

  typedef Support::SharedArray<int> IntSharedArray;

  /// Class for index-value ma
  class IdxValMap;

  /**
   * \brief %Element propagator for array of integers
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class ViewA, class ViewB>
  class Int : public Propagator {
  protected:
    /// View for index
    ViewA          x0;
    /// View for result
    ViewB          x1;
    /// Shared array of integer values
    IntSharedArray c;
    /// Cache for index-value map
    IdxValMap*     ivm;
    /// Constructor for cloning \a p
    Int(Space* home, bool shared, Int& p);
    /// Constructor for creation
    Int(Space* home, IntSharedArray& i, ViewA x0, ViewB x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Const function (return PC_BINARY_HI)
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$i_{x_0}=x_1\f$
    static  ExecStatus post(Space* home, IntSharedArray& i,
                            ViewA x0, ViewB x1);
    /// Flush cached index-value map
    virtual void flush(void);
    /// Return size of index-value map
    virtual size_t size(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };



  /**
   * \brief Class for index-view map
   *
   */
  template <class ViewB> class IdxView;

  /**
   * \brief Base-class for element propagator for array of views
   *
   */
  template <class ViewA, class ViewB, PropCond pcb>
  class View : public Propagator {
  protected:
    /// View for index
    ViewA    x0;
    /// View for result
    ViewB    x1;
    /// Number of elements in index-view map
    int      n;
    /// Current index-view map
    IdxView<ViewB>* iv;
    /// Constructor for cloning \a p
    View(Space* home, bool share, View& p);
    /// Constructor for creation
    View(Space* home, IdxView<ViewB>* iv, int n, ViewA x0, ViewB x1);
  public:
    // Cost function (defined as dynamic PC_LINEAR_LO)
    virtual PropCost cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };


  /**
   * \brief Bounds-consistent element propagator for array of views
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class ViewA, class ViewB>
  class ViewBnd : public View<ViewA,ViewB,PC_INT_BND> {
  protected:
    using View<ViewA,ViewB,PC_INT_BND>::x0;
    using View<ViewA,ViewB,PC_INT_BND>::x1;
    using View<ViewA,ViewB,PC_INT_BND>::n;
    using View<ViewA,ViewB,PC_INT_BND>::iv;

    /// Constructor for cloning \a p
    ViewBnd(Space* home, bool share, ViewBnd& p);
    /// Constructor for creation
    ViewBnd(Space* home, IdxView<ViewB>* iv, int n, ViewA x0, ViewB x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$iv_{x_0}=x_1\f$
    static  ExecStatus post(Space* home, IdxView<ViewB>* iv, int n,
                            ViewA x0, ViewB x1);
  };

  /**
   * \brief Domain-consistent element propagator for array of views
   *
   * The propagator uses staging: first it uses
   * bounds-propagation on the array of views and the uses
   * domain-propagation on the array of views.
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class ViewA, class ViewB>
  class ViewDom : public View<ViewA,ViewB,PC_INT_DOM> {
  protected:
    using View<ViewA,ViewB,PC_INT_DOM>::x0;
    using View<ViewA,ViewB,PC_INT_DOM>::x1;
    using View<ViewA,ViewB,PC_INT_DOM>::n;
    using View<ViewA,ViewB,PC_INT_DOM>::iv;

    /// Constructor for cloning \a p
    ViewDom(Space* home, bool share, ViewDom& p);
    /// Constructor for creation
    ViewDom(Space* home, IdxView<ViewB>* iv, int n, ViewA x0, ViewB x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space* home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds-propagation defined as dynamic PC_LINEAR_LO,
     * otherwise as dynamic PC_LINEAR_HI.
     *
     */
    virtual PropCost cost(void) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for \f$iv_{x_0}=x_1\f$
    static  ExecStatus post(Space* home, IdxView<ViewB>* iv, int n,
                            ViewA x0, ViewB x1);
  };

}}}

#include "gecode/int/element/int.icc"
#include "gecode/int/element/view.icc"

#endif


// STATISTICS: int-prop

