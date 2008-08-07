/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#ifndef __GECODE_CPLTSETPROP_HH
#define __GECODE_CPLTSETPROP_HH

#include "gecode/cpltset.hh"

namespace Gecode { namespace CpltSet { 

  /**
   * \brief Nary propagator for CpltSet variables
   *
   * Propagates a constraint specified as a bdd.
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View>
  class NaryCpltSetPropagator : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    NaryCpltSetPropagator(Space& home, bool share, NaryCpltSetPropagator& p);
    /// Constructor for creation
    NaryCpltSetPropagator(Space& home, ViewArray<View>& x, bdd& d);
    /// (EEQ) Earliest Existential Quantification for set bounds propagation
    bdd bnd_phi(int j);
    /// (EEQ) Earliest Existential Quantification
    bdd phi(int i, int j);
    /// Apply existential quantification for all variables
    ExecStatus 
    divide_conquer(Space& home, bdd& p, int i, int j);
  public:
    /// Cost function
    virtual PropCost cost(const ModEventDelta& med) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);

    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Use eeq to perform domain propagation with n^2 and-abstractions
    ExecStatus propagate_eeq(Space& home);
    /// Use eeq to perform bounds propagation 
    ExecStatus propagate_bnd_eeq(Space& home);
    /// Use ddc to perform domain propagation with nlog(n) and-abstractions
    ExecStatus propagate_ddc(Space& home);
    /// Use ddc to perform bounds propagation with nlog(n) and-abstractions
    ExecStatus propagate_bnd_ddc(Space& home);

    static  ExecStatus post(Space& home, ViewArray<View>& x, bdd& d);
  };


  /**
   * \brief Binary propagator for CpltSet variables
   *
   * Propagates a constraint specified as a bdd.
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View0, class View1>
  class BinaryCpltSetPropagator : public Propagator {
  protected:
    /// First view
    View0 x;
    /// Second view
    View1 y;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    BinaryCpltSetPropagator(Space& home, bool share,
                            BinaryCpltSetPropagator& p);
    /// Constructor for posting
    BinaryCpltSetPropagator(Space& home, View0& x0, View1& y0, bdd& d);
  public:
    /// Cost function
    virtual PropCost cost(const ModEventDelta& med) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);

    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, View0& x0, View1& y0, bdd& d);
  };

  /**
   * \brief Unary propagator for CpltSet variables
   */
  template <class View>
  class UnaryCpltSetPropagator : public Propagator {
  protected:
    /// View to propagate on
    View x;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    UnaryCpltSetPropagator(Space& home, bool share, UnaryCpltSetPropagator& p);
    /// Constructor for posting
    UnaryCpltSetPropagator(Space& home, View& x0, bdd& d);
  public:
    /// Cost function
    virtual PropCost cost(const ModEventDelta& med) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);

    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, View& x0, bdd& d);
  };

  /**
   * \brief Propagator for CpltSet variables with n+1 arguments
   *
   * Propagates a constraint specified as a bdd.
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View0, class View1>
  class NaryOneCpltSetPropagator : 
    public MixNaryOnePropagator<View0,PC_CPLTSET_DOM,View1,PC_CPLTSET_DOM> {
  protected:
    typedef MixNaryOnePropagator<View0, PC_CPLTSET_DOM,
                                 View1, PC_CPLTSET_DOM> Super;
    /// Bdd representation of the constraint
    bdd d;
    using Super::x;
    using Super::y;
    /// Constructor for cloning \a p
    NaryOneCpltSetPropagator(Space& home, bool share, 
                             NaryOneCpltSetPropagator& p);
    /// Constructor for posting
    NaryOneCpltSetPropagator(Space& home, ViewArray<View0>&, View1&, bdd&);
    /// Divide and conquer method including additional view \a y
    ExecStatus divide_conquer(Space& home, bdd& p, 
                              int l, int r, int ypos);
  public:
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);
    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, ViewArray<View0>& x, View1& y, 
                            bdd& d);
  };

  /**
   * \brief Propagator for CpltSet variables with n+2 arguments
   *
   * Propagates a constraint specified as a bdd.
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View0, class View1>
  class NaryTwoCpltSetPropagator : 
    public Propagator {
  protected:
    /// Array of views of type View0
    ViewArray<View0> x;
    /// First view of type View1
    View1 y;
    /// Second view of type View1
    View1 z;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    NaryTwoCpltSetPropagator(Space& home, bool share, 
                             NaryTwoCpltSetPropagator& p);
    /// Constructor for posting
    NaryTwoCpltSetPropagator(Space& home,
                             ViewArray<View0>&, View1&, View1&, bdd&);
    /// Divide and conquer method including additional \a y and \a z views
    ExecStatus divide_conquer(Space& home, bdd& p, int l, int r, 
                              int ypos, int zpos);
  public:
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);
    /// Cost function (defined as dynamic PC_LINEAR_LO)
    virtual PropCost cost(const ModEventDelta& med) const;
    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, ViewArray<View0>& x, 
                            View1& y, View1& z, bdd& d);
  };
  
  /**
   * \brief Binary Rel Disjoint Propagator
   *
   * Propagates a constraint specified as a bdd.
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View0, class View1>
  class BinRelDisj : public BinaryCpltSetPropagator<View0,View1> {
  protected:
    /// Bdd representation of the constraint
    using BinaryCpltSetPropagator<View0,View1>::d;
    using BinaryCpltSetPropagator<View0,View1>::x;
    using BinaryCpltSetPropagator<View0,View1>::y;
    /// Constructor for cloning \a p
    BinRelDisj(Space& home, bool share, BinRelDisj<View0,View1>& p);
    /// Constructor for posting
    BinRelDisj(Space& home, View0& x0, View1& y0, bdd& d);
  public:
    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, View0& x0, View1& y0, bdd& d);
  };

  /**
   * \brief Singleton channel propagator from IntVar to CpltSetVar
   *
   * Requires \code #include "gecode/cpltset/propagators.hh" \endcode
   * \ingroup FuncCpltSetProp
   */
  template <class View1, class View2>
  class Singleton : public Propagator {
  protected:
    /// View for the IntVar
    View1 x;
    /// View for the CpltSetVar
    View2 s;
    /// Constructor for cloning \a p
    Singleton(Space& home, bool share, Singleton& p);
    /// Constructor for creation
    Singleton(Space& home, View1& x, View2& s);
  public:
    /// Cost function
    virtual PropCost cost(const ModEventDelta& med) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Name of this propagator
    static Support::Symbol ati(void);

    /// Delete propagator
    virtual size_t dispose(Space& home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space& home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    static  ExecStatus post(Space& home, View1& x, View2& s);
  }; 

}}

#include "gecode/cpltset/propagators/nary.icc"
#include "gecode/cpltset/propagators/naryone.icc"
#include "gecode/cpltset/propagators/narytwo.icc"
#include "gecode/cpltset/propagators/binary.icc"
#include "gecode/cpltset/propagators/unary.icc"
#include "gecode/cpltset/propagators/singleton.icc" 

#endif

// STATISTICS: cpltset-prop
