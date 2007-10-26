/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __GECODE_SET_SELECT_HH__
#define __GECODE_SET_SELECT_HH__

#include "gecode/set.hh"

#include "gecode/set/select/idxarray.hh"
#include "gecode/set/rel.hh"
#include "gecode/set/rel-op.hh"

namespace Gecode { namespace Set { namespace Select {

  /**
   * \namespace Gecode::Set::Select
   * \brief %Set selection propagators
   */

  /**
   * \brief %Propagator for selected intersection
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class SView, class RView>
  class SelectIntersection :
    public Propagator {
  protected:
    IntSet universe;
    SView x0;
    IdxViewArray<SView> iv;
    RView x1;

    /// Constructor for cloning \a p
    SelectIntersection(Space* home, bool share,SelectIntersection& p);
    /// Constructor for posting
    SelectIntersection(Space* home,SView,IdxViewArray<SView>&,RView,
                       const IntSet& universe);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol name(void);
    /** Post propagator for \f$ z=\bigcap\langle x_0,\dots,x_{n-1}\rangle[y] \f$ using \a u as universe
     *
     * If \a y is empty, \a z will be constrained to be the given universe
     * \a u (as an empty intersection is the universe).
     */
    static  ExecStatus  post(Space* home,SView z,IdxViewArray<SView>& x,
                             RView y, const IntSet& u);
  };

  /**
   * \brief %Propagator for selected union
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class SView, class RView>
  class SelectUnion :
    public Propagator {
  protected:
    SView x0;
    IdxViewArray<SView> iv;
    RView x1;

    /// Constructor for cloning \a p
    SelectUnion(Space* home, bool share,SelectUnion& p);
    /// Constructor for posting
    SelectUnion(Space* home,SView,IdxViewArray<SView>&,RView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol name(void);
    /** Post propagator for \f$ z=\bigcup\langle x_0,\dots,x_{n-1}\rangle[y] \f$
     *
     * If \a y is empty, \a z will be constrained to be empty
     * (as an empty union is the empty set).
     */
    static  ExecStatus  post(Space* home,SView z,IdxViewArray<SView>& x,
                             RView y);
  };

  /**
   * \brief %Propagator for selected union with constant sets
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class SView, class RView>
  class SelectUnionConst :
    public Propagator {
  protected:
    SView x0;
    SharedArray<IntSet> iv;
    RView x1;

    /// Constructor for cloning \a p
    SelectUnionConst(Space* home, bool share,SelectUnionConst& p);
    /// Constructor for posting
    SelectUnionConst(Space* home,SView,SharedArray<IntSet>&,RView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol name(void);
    /** Post propagator for \f$ z=\bigcup\langle s_0,\dots,s_{n-1}\rangle[y] \f$
     *
     * If \a y is empty, \a z will be constrained to be empty
     * (as an empty union is the empty set).
     */
    static  ExecStatus  post(Space* home,SView z,SharedArray<IntSet>& x,
                             RView y);
  };

  /**
   * \brief %Propagator for selected disjointness
   *
   * Requires \code #include "gecode/set/select.hh" \endcode
   * \ingroup FuncSetProp
   */
  class SelectDisjoint :
    public Propagator {
  protected:
    IdxViewArray<SetView> iv;
    SetView x1;

    /// Constructor for cloning \a p
    SelectDisjoint(Space* home, bool share,SelectDisjoint& p);
    /// Constructor for posting
    SelectDisjoint(Space* home,IdxViewArray<SetView>&,SetView);
  public:
    /// Copy propagator during cloning
    GECODE_SET_EXPORT virtual Actor*      copy(Space* home,bool);
    GECODE_SET_EXPORT virtual PropCost    cost(void) const;
    /// Delete propagator and return its size
    GECODE_SET_EXPORT virtual size_t dispose(Space* home);
    /// Perform propagation
    GECODE_SET_EXPORT virtual ExecStatus  propagate(Space* home);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol name(void);
    /// Post propagator for \f$ \parallel\langle x_0,\dots,x_{n-1}\rangle[y] \f$ 
    static  ExecStatus  post(Space* home,IdxViewArray<SetView>& x,SetView y);
  };

}}}

#include "gecode/set/select/inter.icc"
#include "gecode/set/select/union.icc"
#include "gecode/set/select/unionConst.icc"
#include "gecode/set/select/disjoint.icc"

#endif

// STATISTICS: set-prop

