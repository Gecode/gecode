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

// #include "gecode/set/rel.hh"
// #include "gecode/set/rel-op.hh"

namespace Gecode { namespace CpltSet { 

  /**
   * \brief Nary bdd propagator
   */
  template <class View>
  class NaryBddProp : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Bdd representation of the constraint
    GecodeBdd d;
    /// Constructor for cloning \a p
    NaryBddProp(Space* home, bool share, NaryBddProp& p);
    /// Constructor for creation
    NaryBddProp(Space* home, ViewArray<View>& x, GecodeBdd& d);
    /// (EEQ) Earliest Existential Quantification for set bounds propagation
    GecodeBdd bnd_phi(BMI* mgr, int j);
    /// (EEQ) Earliest Existential Quantification
    GecodeBdd phi(BMI* mgr, int i, int j);
    /// Apply existential quantification for all variables
    ExecStatus 
    divide_conquer(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
    /// Apply bounds consistent existential quantification for all variables
//     GECODE_CPLTSET_EXPORT ExecStatus 
//     divide_conquer_conv(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply existential quantification for the remainder part
//     GECODE_CPLTSET_EXPORT ExecStatus 
//     divide_conquer_split(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply cardinality bounds existential quantification for the remainder part
//     GECODE_CPLTSET_EXPORT ExecStatus 
//     divide_conquer_split_card(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply lexicographic bounds existential quantification for the remainder part
//     GECODE_CPLTSET_EXPORT ExecStatus 
//     divide_conquer_split_lex(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
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
//     /// Use naive mathematical definition for domain propagation
//     ExecStatus propagate_naive(Space* home);
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
//     /// Perform split propagation
//     ExecStatus propagate_split(Space* home);
//     /// Perform cardinality propagation
//     ExecStatus propagate_card(Space* home);
//     /// Perform lexicographic propagation
//     ExecStatus propagate_lex(Space* home);

    static  ExecStatus post(Space* home, ViewArray<View>& x, GecodeBdd& d);
  };


  /**
   * \brief Binary bdd propagator
   */
  template <class View>
  class BinBddProp : public Propagator {
  protected:
    /// First view
    View x;
    /// Second view
    View y;
    /// Bdd representation of the constraint
    GecodeBdd d;
    /// Constructor for cloning \a p
    BinBddProp(Space* home, bool share, BinBddProp& p);
    /// Constructor for posting
    BinBddProp(Space* home, View& x0, View& y0, GecodeBdd& d);
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
    static  ExecStatus post(Space* home, View& x0, View& y0, GecodeBdd& d);
  };

  /**
   * \brief Unary bdd propagator
   */

  template <class View>
  class UnaryBddProp : public Propagator {
  protected:
    /// View to propagate on
    View x;
    /// Bdd representation of the constraint
    GecodeBdd d;
    /// Constructor for cloning \a p
    UnaryBddProp(Space* home, bool share, UnaryBddProp& p);
    /// Constructor for posting
    UnaryBddProp(Space* home, View& x0, GecodeBdd& d);
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
    static  ExecStatus post(Space* home, View& x0, GecodeBdd& d);
  };


  /**
   * \brief Binary Rel Disjoint Propagator
   */
  template <class View>
  class BinRelDisj : public BinBddProp<View> {
  protected:
    /// Bdd representation of the constraint
    using BinBddProp<View>::d;
    using BinBddProp<View>::x;
    using BinBddProp<View>::y;
    /// Constructor for cloning \a p
    BinRelDisj(Space* home, bool share, BinRelDisj& p);
    /// Constructor for posting
    BinRelDisj(Space* home, View& x0, View& y0, GecodeBdd& d);
  public:
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, View& y0, GecodeBdd& d);
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
   * \brief Binary bdd propagator
   */
  template <class View0, class View1>
  class Bin : public MixBinaryPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixBinaryPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x0;
    using MixBinaryPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x1;
    /// Constructor for cloning \a p
    Bin(Space* home, bool share,Bin& p);
    /// Constructor for posting
    Bin(Space* home, View0&, View1&, GecodeBdd& d);
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
    static  ExecStatus post(Space* home, View0& x, View1& s, GecodeBdd& d);
  }; 

  /**
   * \brief Bdd propagator with n+1 arguments
   */
  template <class View0, class View1>
  class NaryOneBdd : 
    public MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x;
    using MixNaryOnePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::y;
    /// Constructor for cloning \a p
    NaryOneBdd(Space* home, bool share, NaryOneBdd& p);
    /// Constructor for posting
    NaryOneBdd(Space* home, ViewArray<View0>&, View1&, GecodeBdd&);
    /// Divide and conquer method including additional view \a y
    ExecStatus divide_conquer(Space* home, BMI* m, GecodeBdd& p, 
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
                            GecodeBdd& d);
  };

  template <class View0, class View1>
  class Range : 
    public MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::y;
    using MixNaryTwoPropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::z;
    /// Constructor for cloning \a p
    Range(Space* home, bool share, Range& p);
    /// Constructor for posting
    Range(Space* home, ViewArray<View0>&, View1&, View1&, GecodeBdd&);
    /// Divide and conquer method including additional \a y and \a z views
    ExecStatus divide_conquer(Space* home, BMI* m, GecodeBdd& p, int l, int r, 
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
                            View1& y, View1& z, GecodeBdd& d);
  };


  template <class View0, class View1>
  class RangeTwice : 
    public MixNaryTwicePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryTwicePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::x;
    using MixNaryTwicePropagator<View0, PC_CPLTSET_DOM, View1, PC_CPLTSET_DOM>::y;
    /// Constructor for cloning \a p
    RangeTwice(Space* home, bool share, RangeTwice& p);
    /// Constructor for posting
    RangeTwice(Space* home, ViewArray<View0>&, ViewArray<View1>&, GecodeBdd&);
    /// Divide and conquer method including additional array \a y
    ExecStatus divide_conquer(Space* home, BMI* m, GecodeBdd& p, 
                              int l, int r, int ypos);
  public:
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, ViewArray<View0>& x, 
                            ViewArray<View1>&y, GecodeBdd& d);
  };


  /**
   * \brief Propagator for DisjointGlb
   */
  template <class View>
  class DisjointGlb : public Propagator {
  protected:
    /// Array of views
    ViewArray<View> x;
    /// Index to be kept disjoint
    int idx;
    /// Constructor for cloning \a p
    DisjointGlb(Space* home, bool share, DisjointGlb& p);
    /// Constructor for posting
    DisjointGlb(Space* home, ViewArray<View>& x0, int& index);
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
    static  ExecStatus post(Space* home, ViewArray<View>& x0, int& index);
  };


  /**
   * \brief Propagator for DisjointSudoku
   */
  template <class View>
  class DisjointSudoku : public Propagator {
  protected:
    /// Array of views
    View x;
    /// Order of the sudoku
    int order;
    /// Constructor for cloning \a p
    DisjointSudoku(Space* home, bool share, DisjointSudoku& p);
    /// Constructor for posting
    DisjointSudoku(Space* home, View& x0, int& order);
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
    static  ExecStatus post(Space* home, View& x0, int& order);
  };


}}

#include "gecode/cpltset/propagators/distinct.icc"
#include "gecode/cpltset/propagators/partition.icc"
#include "gecode/cpltset/propagators/atmost.icc"
#include "gecode/cpltset/propagators/rel.icc"
#include "gecode/cpltset/propagators/select.icc"

#include "gecode/cpltset/propagators/common.icc"

#include "gecode/cpltset/propagators/naryrec.icc"
#include "gecode/cpltset/propagators/nary.icc"
#include "gecode/cpltset/propagators/binary.icc"
#include "gecode/cpltset/propagators/unary.icc"
#include "gecode/cpltset/propagators/singleton.icc" 
#include "gecode/cpltset/propagators/bin.icc"
#include "gecode/cpltset/propagators/rangerec.icc"
#include "gecode/cpltset/propagators/rangeroots.icc"

// check whether we need this in other propagators too
#include "gecode/support/dynamic-array.hh"
#endif

// STATISTICS: cpltset-prop
