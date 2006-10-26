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

#ifndef __GECODE_SET_RELOP_HH__
#define __GECODE_SET_RELOP_HH__

#include "gecode/set.hh"
#include "gecode/iter.hh"
#include "gecode/set/rel.hh"

namespace Gecode { namespace Set { namespace RelOp {

  /**
   * \namespace Gecode::Set::RelOp
   * \brief Standard set operation propagators
   */

   /**
    * \brief %Propagator for the superset of intersection
    *
    * Requires \code #include "gecode/set/rel-op.hh" \endcode
    * \ingroup FuncSetProp
    */
  
   template <class View0, class View1, class View2>
   class SuperOfInter :
    public MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_CLUB> {
   protected:
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_CLUB>::x0;
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_CLUB>::x1;
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_CLUB>::x2;
     /// Constructor for cloning \a p
     SuperOfInter(Space* home, bool share,SuperOfInter& p);
     /// Constructor for posting
     SuperOfInter(Space* home,View0, View1, View2);
   public:
     /// Copy propagator during cloning
     virtual Actor*      copy(Space* home,bool);
     /// Perform propagation
     virtual ExecStatus  propagate(Space* home);
     /// Post propagator \f$ z \supseteq x \cap y\f$ 
     static  ExecStatus  post(Space* home, View0 x, View1 y, View2 z);
   };

  /**
   * \brief %Propagator for the subset of union
   *
   * Requires \code #include "gecode/set/rel-op.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1, class View2>
  class SubOfUnion :
    public MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY> {
  protected:
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x0;
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x1;
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x2;
    /// Constructor for cloning \a p
    SubOfUnion(Space* home, bool share,SubOfUnion& p);
    /// Constructor for posting
    SubOfUnion(Space* home,View0, View1, View2);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Post propagator \f$ z \subseteq x \cap y\f$ 
    static  ExecStatus  post(Space* home,View0 x,View1 y,View2 z);
  };


   /**
    * \brief %Propagator for ternary intersection
    *
    * Requires \code #include "gecode/set/rel-op.hh" \endcode
    * \ingroup FuncSetProp
    */
   template <class View0, class View1, class View2>
   class Intersection:
    public MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY> {
   protected:
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY>::x0;
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY>::x1;
     using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY>::x2;
     /// Constructor for cloning \a p
     Intersection(Space* home, bool share,Intersection& p);
     /// Constructor for posting
     Intersection(Space* home,View0,View1,View2);
   public:
     /// Copy propagator during cloning
     virtual Actor*      copy(Space* home,bool);
     /// Perform propagation
     virtual ExecStatus  propagate(Space* home);
     /// Post propagator \f$ z=x\cap y\f$ 
     static  ExecStatus  post(Space* home,View0 x,View1 y,View2 z);
   };

  /**
   * \brief %Propagator for ternary union
   *
   * Requires \code #include "gecode/set/rel-op.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1, class View2>
  class Union:
    public MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                  View2,PC_SET_ANY> {
  protected:
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x0;
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x1;
    using MixTernaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY,
                                 View2,PC_SET_ANY>::x2;
    /// Constructor for cloning \a p
    Union(Space* home, bool share,Union& p);
    /// Constructor for posting
    Union(Space* home,View0,View1,View2);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Post propagator \f$ z=x\cup y\f$
    static  ExecStatus  post(Space* home,View0 x,View1 y,View2 z);
  };

   /**
    * \brief %Propagator for nary intersection
    *
    * Requires \code #include "gecode/set/rel-op.hh" \endcode
    * \ingroup FuncSetProp
    */
  template <class View0, class View1>
  class IntersectionN : public MixNaryOnePropagator<View0,PC_SET_ANY,
                                                      View1,PC_SET_ANY> {
  protected:
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::x;
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::y;
    /// Whether the any views share a variable implementation
    bool shared;
    /// Intersection of the determined \f$x_i\f$ (which are dropped)
    LUBndSet intOfDets;
    /// Constructor for cloning \a p
    IntersectionN(Space* home, bool share,IntersectionN& p);
    /// Constructor for posting
    IntersectionN(Space* home,ViewArray<View0>&, View1);
  public:
    virtual PropCost    cost(void) const;
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Post propagator \f$ x = \bigcap_{i\in\{0,\dots,n-1\}} y_i \f$
    static  ExecStatus  post(Space* home,ViewArray<View0>& y,View1 x);
  };

  /**
   * \brief %Propagator for nary union
   *
   * Requires \code #include "gecode/set/rel-op.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1>
  class UnionN : public MixNaryOnePropagator<View0,PC_SET_ANY,
                                               View1,PC_SET_ANY> {
  protected:
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::x;
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::y;
    /// Whether the any views share a variable implementation
    bool shared;
    /// Union of the determined \f$x_i\f$ (which are dropped)
    GLBndSet unionOfDets;
    /// Constructor for cloning \a p
    UnionN(Space* home, bool share,UnionN& p);
    /// Constructor for posting
    UnionN(Space* home,ViewArray<View0>&,View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home, bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    virtual PropCost    cost(void) const;
    /// Post propagator \f$ x = \bigcup_{i\in\{0,\dots,n-1\}} y_i \f$ 
    static  ExecStatus  post(Space* home,ViewArray<View0>& y,View1 x);
  };


  /**
   * \brief %Propagator for nary partition
   *
   * Requires \code #include "gecode/set/rel-op.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1>
  class PartitionN : public MixNaryOnePropagator<View0,PC_SET_ANY,
                                                   View1,PC_SET_ANY> {
  protected:
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::x;
    using MixNaryOnePropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::y;
    /// Whether the any views share a variable implementation
    bool shared;
    /// Union of the determined \f$x_i\f$ (which are dropped)
    GLBndSet unionOfDets;
    /// Constructor for cloning \a p
    PartitionN(Space* home, bool share,PartitionN& p);
    /// Constructor for posting
    PartitionN(Space* home,ViewArray<View0>&, View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    virtual PropCost    cost(void) const;
    /// Post propagator \f$ x = \biguplus_{i\in\{0,\dots,n-1\}} y_i \f$ 
    static  ExecStatus  post(Space* home,ViewArray<View0>& y,View1 x);
  };

}}}

#include "gecode/set/rel-op/common.icc"
#include "gecode/set/rel-op/superofinter.icc"
#include "gecode/set/rel-op/subofunion.icc"
#include "gecode/set/rel-op/inter.icc"
#include "gecode/set/rel-op/union.icc"
#include "gecode/set/rel-op/partition.icc"
#include "gecode/set/rel-op/post.icc"

#endif

// STATISTICS: set-prop
