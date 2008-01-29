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

#ifndef __GECODE_SET_REL_HH__
#define __GECODE_SET_REL_HH__

#include "gecode/set.hh"

namespace Gecode { namespace Set { namespace Rel {

  /**
   * \namespace Gecode::Set::Rel
   * \brief Standard set relation propagators
   */

  /**
   * \brief %Propagator for the subset constraint
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp
   */

  template <class View0, class View1>
  class SubSet :
    public MixBinaryPropagator<View0,PC_SET_CGLB,View1,PC_SET_CLUB> {
  protected:
    using MixBinaryPropagator<View0,PC_SET_CGLB,View1,PC_SET_CLUB>::x0;
    using MixBinaryPropagator<View0,PC_SET_CGLB,View1,PC_SET_CLUB>::x1;
    /// Constructor for cloning \a p
    SubSet(Space* home, bool share,SubSet& p);
    /// Constructor for posting
    SubSet(Space* home,View0, View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ x\subseteq y\f$
    static  ExecStatus post(Space* home,View0 x,View1 y);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Propagator for the negated subset constraint
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp
   */

  template <class View0, class View1>
  class NoSubSet :
    public MixBinaryPropagator<View0,PC_SET_CLUB,View1,PC_SET_CGLB> {
  protected:
    using MixBinaryPropagator<View0,PC_SET_CLUB,View1,PC_SET_CGLB>::x0;
    using MixBinaryPropagator<View0,PC_SET_CLUB,View1,PC_SET_CGLB>::x1;
    /// Constructor for cloning \a p
    NoSubSet(Space* home, bool share,NoSubSet& p);
    /// Constructor for posting
    NoSubSet(Space* home,View0,View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ x\subseteq y\f$
    static  ExecStatus post(Space* home,View0 x,View1 y);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Reified subset propagator
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1>
  class ReSubset :
    public Propagator {
  protected:
    View0 x0;
    View1 x1;
    Gecode::Int::BoolView b;

    /// Constructor for cloning \a p
    ReSubset(Space* home, bool share,ReSubset&);
    /// Constructor for posting
    ReSubset(Space* home,View0, View1, Gecode::Int::BoolView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Cost function (defined as PC_TERNARY_LO)
    virtual PropCost cost(ModEventDelta med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator for \f$ (x\subseteq y) \Leftrightarrow b \f$ 
    static ExecStatus post(Space* home,View0 x, View1 y,
                           Gecode::Int::BoolView b);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Propagator for set equality
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp
   */

  template <class View0, class View1>
  class Eq : 
    public MixBinaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY> {
  protected:
    using MixBinaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::x0;
    using MixBinaryPropagator<View0,PC_SET_ANY,View1,PC_SET_ANY>::x1;
    /// Constructor for cloning \a p
    Eq(Space* home, bool share,Eq& p);
    /// Constructor for posting
    Eq(Space* home,View0, View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ x=y \f$ 
    static  ExecStatus  post(Space* home,View0,View1);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Reified equality propagator
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp
   */
  template <class View0, class View1>
  class ReEq :
    public Propagator {
  protected:
    View0 x0;
    View1 x1;
    Gecode::Int::BoolView b;

    /// Constructor for cloning \a p
    ReEq(Space* home, bool share,ReEq&);
    /// Constructor for posting
    ReEq(Space* home,View0, View1, Gecode::Int::BoolView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Cost function (defined as PC_TERNARY_LO)
    virtual PropCost cost(ModEventDelta med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator for \f$ (x=y) \Leftrightarrow b\f$ 
    static ExecStatus post(Space* home,View0 x, View1 y,
                           Gecode::Int::BoolView b);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Propagator for negated equality
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp   
   */

  template <class View0, class View1>
  class Distinct :
    public MixBinaryPropagator<View0,PC_SET_VAL,View1,PC_SET_VAL> {
  protected:
    using MixBinaryPropagator<View0,PC_SET_VAL,View1,PC_SET_VAL>::x0;
    using MixBinaryPropagator<View0,PC_SET_VAL,View1,PC_SET_VAL>::x1;
    /// Constructor for cloning \a p
    Distinct(Space* home, bool share,Distinct& p);
    /// Constructor for posting
    Distinct(Space* home,View0,View1);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ x\neq y \f$ 
    static  ExecStatus  post(Space* home,View0,View1);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

  /**
   * \brief %Propagator for negated equality
   *
   * This propagator actually propagates the distinctness, after the
   * Distinct propagator waited for one variable to become
   * assigned.
   *
   * Requires \code #include "gecode/set/rel.hh" \endcode
   * \ingroup FuncSetProp   
   */
  template <class View0>
  class DistinctDoit : public UnaryPropagator<View0,PC_SET_ANY> {
  protected:
    using UnaryPropagator<View0,PC_SET_ANY>::x0;
    /// The view that is already assigned
    ConstantView y;
    /// Constructor for cloning \a p
    DistinctDoit(Space* home, bool share,DistinctDoit&);
    /// Constructor for posting
    DistinctDoit(Space* home, View0, ConstantView);
  public:
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home, bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Post propagator \f$ x\neq y \f$ 
    static ExecStatus post(Space* home, View0, ConstantView);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post using specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
  };

}}}

#include "gecode/set/rel/common.icc"
#include "gecode/set/rel/subset.icc"
#include "gecode/set/rel/nosubset.icc"
#include "gecode/set/rel/re-subset.icc"
#include "gecode/set/rel/eq.icc"
#include "gecode/set/rel/re-eq.icc"
#include "gecode/set/rel/nq.icc"

#endif

// STATISTICS: set-prop
