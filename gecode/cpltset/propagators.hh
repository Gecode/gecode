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
   * \brief Nary bdd propagator
   */
  template <class View>
  class NaryCpltSetPropagator : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    NaryCpltSetPropagator(Space* home, bool share, NaryCpltSetPropagator& p);
    /// Constructor for creation
    NaryCpltSetPropagator(Space* home, ViewArray<View>& x, bdd& d);
    /// (EEQ) Earliest Existential Quantification for set bounds propagation
    bdd bnd_phi(int j);
    /// (EEQ) Earliest Existential Quantification
    bdd phi(int i, int j);
    /// Apply existential quantification for all variables
    ExecStatus 
    divide_conquer(Space* home, bdd& p, int i, int j);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Use naive mathematical definition for bounds propagation
    ExecStatus propagate_naive_bnd(Space* home);
    /// Use eeq to perform domain propagation with n^2 and-abstractions
    ExecStatus propagate_eeq(Space* home);
    /// Use eeq to perform bounds propagation 
    ExecStatus propagate_bnd_eeq(Space* home);
    /// Use ddc to perform domain propagation with nlog(n) and-abstractions
    ExecStatus propagate_ddc(Space* home);
    /// Use ddc to perform bounds propagation with nlog(n) and-abstractions
    ExecStatus propagate_bnd_ddc(Space* home);

    static  ExecStatus post(Space* home, ViewArray<View>& x, bdd& d);
  };


  /**
   * \brief Binary bdd propagator
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
    BinaryCpltSetPropagator(Space* home, bool share, BinaryCpltSetPropagator& p);
    /// Constructor for posting
    BinaryCpltSetPropagator(Space* home, View0& x0, View1& y0, bdd& d);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View0& x0, View1& y0, bdd& d);
  };

  /**
   * \brief Unary bdd propagator
   */

  template <class View>
  class UnaryCpltSetPropagator : public Propagator {
  protected:
    /// View to propagate on
    View x;
    /// Bdd representation of the constraint
    bdd d;
    /// Constructor for cloning \a p
    UnaryCpltSetPropagator(Space* home, bool share, UnaryCpltSetPropagator& p);
    /// Constructor for posting
    UnaryCpltSetPropagator(Space* home, View& x0, bdd& d);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, bdd& d);
  };


  /**
   * \brief Binary Rel Disjoint Propagator
   */
  template <class View0, class View1>
  class BinRelDisj : public BinaryCpltSetPropagator<View0,View1> {
  protected:
    /// Bdd representation of the constraint
    using BinaryCpltSetPropagator<View0,View1>::d;
    using BinaryCpltSetPropagator<View0,View1>::x;
    using BinaryCpltSetPropagator<View0,View1>::y;
    /// Constructor for cloning \a p
    BinRelDisj(Space* home, bool share, BinRelDisj<View0,View1>& p);
    /// Constructor for posting
    BinRelDisj(Space* home, View0& x0, View1& y0, bdd& d);
  public:
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View0& x0, View1& y0, bdd& d);
  };

  /**
   * \brief Singleton channel propagator from IntVar to CpltSetVar
   */

  template <class View1, class View2>
  class Singleton : public Propagator {
  protected:
    /// View for the IntVar
    View1 x;
    /// View for the CpltSetVar
    View2 s;
    /// Constructor for cloning \a p
    Singleton(Space* home, bool share, Singleton& p);
    /// Constructor for creation
    Singleton(Space* home, View1& x, View2& s);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View1& x, View2& s);
  }; 


  /*
   * Inhomogenuous bdd propagators
   *
   */

  /**
   * \brief Bdd propagator with n+1 arguments
   */
  template <class View0, class View1>
  class NaryOneCpltSetPropagator : 
    public MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    bdd d;
    using MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x;
    using MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::y;
    /// Constructor for cloning \a p
    NaryOneCpltSetPropagator(Space* home, bool share, NaryOneCpltSetPropagator& p);
    /// Constructor for posting
    NaryOneCpltSetPropagator(Space* home, ViewArray<View0>&, View1&, bdd&);
    /// Divide and conquer method including additional view \a y
    ExecStatus divide_conquer(Space* home, bdd& p, 
                              int l, int r, int ypos);
  public:
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, ViewArray<View0>& x, View1& y, 
                            bdd& d);
  };

  template <class View0, class View1>
  class Range : 
    public MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    bdd d;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::y;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::z;
    /// Constructor for cloning \a p
    Range(Space* home, bool share, Range& p);
    /// Constructor for posting
    Range(Space* home, ViewArray<View0>&, View1&, View1&, bdd&);
    /// Divide and conquer method including additional \a y and \a z views
    ExecStatus divide_conquer(Space* home, bdd& p, int l, int r, 
                        int ypos, int zpos);
  public:
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(Space* home, Reflection::VarMap& m);
    /// Name of this propagator
    static Support::Symbol name(void);
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, ViewArray<View0>& x, 
                            View1& y, View1& z, bdd& d);
  };


}}

#include "gecode/cpltset/propagators/common.icc"

#include "gecode/cpltset/propagators/naryrec.icc"
#include "gecode/cpltset/propagators/nary.icc"
#include "gecode/cpltset/propagators/binary.icc"
#include "gecode/cpltset/propagators/unary.icc"
#include "gecode/cpltset/propagators/singleton.icc" 

#include "gecode/cpltset/constraints/rangeroots.icc"
#include "gecode/cpltset/constraints/distinct.icc"
#include "gecode/cpltset/constraints/partition.icc"
#include "gecode/cpltset/constraints/atmost.icc"
#include "gecode/cpltset/constraints/rel.icc"
#include "gecode/cpltset/constraints/select.icc"


#endif

// STATISTICS: cpltset-prop
