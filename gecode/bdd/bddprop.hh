/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date: 2007-05-18 14:13:59 +0000 (Fri, 18 May 2007) $ by $Author: pekczynski $
 *     $Revision: 4608 $
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

#ifndef __GECODE_BDDPROP_HH
#define __GECODE_BDDPROP_HH

#include "gecode/bdd.hh"

// #include "gecode/set/rel.hh"
// #include "gecode/set/rel-op.hh"

namespace Gecode { namespace Bdd { 

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
    /// Consistency level determining which propagation function is used
    SetConLevel scl;
    /// Constructor for cloning \a p
    NaryBddProp(Space* home, bool share, NaryBddProp& p);
    /// Constructor for creation
    NaryBddProp(Space* home, ViewArray<View>& x, GecodeBdd& d, SetConLevel scl);
    /// (EEQ) Earliest Existential Quantification for set bounds propagation
    GecodeBdd bnd_phi(BMI* mgr, int j);
    /// (EEQ) Earliest Existential Quantification
    GecodeBdd phi(BMI* mgr, int i, int j);
    /// Apply existential quantification for all variables
    ExecStatus 
    divide_conquer(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
    /// Apply bounds consistent existential quantification for all variables
//     GECODE_BDD_EXPORT ExecStatus 
//     divide_conquer_conv(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply existential quantification for the remainder part
//     GECODE_BDD_EXPORT ExecStatus 
//     divide_conquer_split(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply cardinality bounds existential quantification for the remainder part
//     GECODE_BDD_EXPORT ExecStatus 
//     divide_conquer_split_card(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
//     /// Apply lexicographic bounds existential quantification for the remainder part
//     GECODE_BDD_EXPORT ExecStatus 
//     divide_conquer_split_lex(Space* home, BMI* mgr, GecodeBdd& p, int i, int j);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Name of this propagator
    virtual const char* name(void) const;

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

    static  ExecStatus post(Space* home, ViewArray<View>& x, GecodeBdd& d, 
			    SetConLevel scl);
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
    /// Consistency level determining which propagation function is used
    SetConLevel scl;
    /// Constructor for cloning \a p
    BinBddProp(Space* home, bool share, BinBddProp& p);
    /// Constructor for posting
    BinBddProp(Space* home, View& x0, View& y0, GecodeBdd& d, SetConLevel scl);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Name of this propagator
    virtual const char* name(void) const;

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, View& y0, GecodeBdd& d, SetConLevel scl);
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
    /// Consistency level determining which propagation function is used
    SetConLevel scl;
    /// Constructor for cloning \a p
    UnaryBddProp(Space* home, bool share, UnaryBddProp& p);
    /// Constructor for posting
    UnaryBddProp(Space* home, View& x0, GecodeBdd& d, SetConLevel scl);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Name of this propagator
    virtual const char* name(void) const;

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, GecodeBdd& d, SetConLevel scl);
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
    using BinBddProp<View>::scl;
    /// Constructor for cloning \a p
    BinRelDisj(Space* home, bool share, BinRelDisj& p);
    /// Constructor for posting
    BinRelDisj(Space* home, View& x0, View& y0, GecodeBdd& d, SetConLevel scl);
  public:
    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, View& y0, GecodeBdd& d, SetConLevel scl);
  };

  /**
   * \brief Singleton channel propagator from IntVar to BddVar
   */

  template <class View1, class View2>
  class Singleton : public Propagator {
  protected:
    /// View for the IntVar
    View1 x;
    /// View for the BddVar
    View2 s;
    /// Constructor for cloning \a p
    Singleton(Space* home, bool share, Singleton& p);
    /// Constructor for creation
    Singleton(Space* home, View1& x, View2& s);
  public:
    /// Cost function
    virtual PropCost cost(void) const;
    /// Name of this propagator
    virtual const char* name(void) const;

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
  class Bin : public MixBinaryPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixBinaryPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::x0;
    using MixBinaryPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::x1;
    /// Constructor for cloning \a p
    Bin(Space* home, bool share,Bin& p);
    /// Constructor for posting
    Bin(Space* home, View0&, View1&, GecodeBdd& d);
  public:
    /// Name of this propagator
    virtual const char* name(void) const;
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
    public MixNaryOnePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryOnePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::x;
    using MixNaryOnePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::y;
    /// Constructor for cloning \a p
    NaryOneBdd(Space* home, bool share, NaryOneBdd& p);
    /// Constructor for posting
    NaryOneBdd(Space* home, ViewArray<View0>&, View1&, GecodeBdd&);
    /// Divide and conquer method including additional view \a y
    ExecStatus divide_conquer(Space* home, BMI* m, GecodeBdd& p, 
                              int l, int r, int ypos);
  public:
    /// Name of this propagator
    virtual const char* name(void) const;
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
    public MixNaryTwoPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryTwoPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::x;
    using MixNaryTwoPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::y;
    using MixNaryTwoPropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::z;
    /// Constructor for cloning \a p
    Range(Space* home, bool share, Range& p);
    /// Constructor for posting
    Range(Space* home, ViewArray<View0>&, View1&, View1&, GecodeBdd&);
    /// Divide and conquer method including additional \a y and \a z views
    ExecStatus divide_conquer(Space* home, BMI* m, GecodeBdd& p, int l, int r, 
			int ypos, int zpos);
  public:
    /// Name of this propagator
    virtual const char* name(void) const;
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
    public MixNaryTwicePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM> {
  protected:
    /// Bdd representation of the constraint
    GecodeBdd d;
    using MixNaryTwicePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::x;
    using MixNaryTwicePropagator<View0, PC_BDD_DOM, View1, PC_BDD_DOM>::y;
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
    /// Name of this propagator
    virtual const char* name(void) const;

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
    /// Name of this propagator
    virtual const char* name(void) const;

    /// Delete propagator
    virtual size_t dispose(Space* home);
    /// Copy propagator during cloning
    virtual Actor*      copy(Space* home,bool);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    static  ExecStatus post(Space* home, View& x0, int& order);
  };


}}

#include "gecode/bdd/bddprop/distinct.icc"
#include "gecode/bdd/bddprop/partition.icc"
#include "gecode/bdd/bddprop/atmost.icc"
#include "gecode/bdd/bddprop/rel.icc"
#include "gecode/bdd/bddprop/select.icc"

#include "gecode/bdd/bddprop/common.icc"

#include "gecode/bdd/bddprop/naryrec.icc"
#include "gecode/bdd/bddprop/nary.icc"
#include "gecode/bdd/bddprop/binary.icc"
#include "gecode/bdd/bddprop/unary.icc"
#include "gecode/bdd/bddprop/singleton.icc" 
#include "gecode/bdd/bddprop/bin.icc"
#include "gecode/bdd/bddprop/rangerec.icc"
#include "gecode/bdd/bddprop/rangeroots.icc"

// check whether we need this in other propagators too
#include "gecode/support/dynamic-array.hh"
#include "gecode/bdd/bddprop/disjointglb.icc"
#endif

// STATISTICS: bdd-prop
