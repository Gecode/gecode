/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael lagerkvist, 2007
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

#ifndef __GECODE_INT_EXTENSIONAL_HH__
#define __GECODE_INT_EXTENSIONAL_HH__

#include "gecode/int.hh"

#include "gecode/int/rel.hh"

#include "gecode/support/bitset.hh"

#include "gecode/int/extensional/table.icc"

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */

namespace Gecode { namespace Int { namespace Extensional {
  typedef Table::tuple tuple;

  /**
   * \brief Naive value extensional propagator
   *
   * Eliminates values of assigned views of type \a View.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Val : public NaryPropagator<View,PC_INT_VAL> {
  protected:
    using NaryPropagator<View,PC_INT_VAL>::x;
    Table& table;

    /// Constructor for posting
    Val(Space* home, ViewArray<View>& x, const Table& t);
    /// Constructor for cloning \a p
    Val(Space* home, bool share, Val<View>& p);
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<View>& x, const Table& t);
  };

  /**
   * \brief Eliminate singletons by naive value propagation
   *
   * This is actually the propagation algorithm for Extensional::Val.
   * It is available as separate function as it is reused for
   * the domain-consistent extensional propagators.
   */
  template <class View>
  ExecStatus prop_val(Space* home, ViewArray<View>& x, const Table& t);


  /**
   * \brief Domain-consistent extensional propagator
   *
   * This propagator implements a basic extensional propagation
   * algorithm. It is based on GAC2001, and as such it does not fully
   * take into account multidirectionality.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Basic : public NaryPropagator<View,PC_INT_DOM> {
  protected:
    using NaryPropagator<View,PC_INT_DOM>::x;
    Table table;
    tuple*** last_data;

    /// Constructor for cloning \a p
    Basic(Space* home, bool share, Basic<View>& p);
    /// Constructor for posting
    Basic(Space* home, ViewArray<View>& x, const Table& t);

    tuple last(int var, int val);
    tuple last_next(int var, int val);
    bool  valid(tuple t, Support::BitSet<unsigned long>* dom);
  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is
     * PC_QUADRATIC_HI. Otherwise it is dynamic PC_CUBIC_HI.
     */
    virtual PropCost cost(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static ExecStatus post(Space* home, ViewArray<View>& x, const Table& t);
  };
}}}

#include "gecode/int/extensional/val.icc"
#include "gecode/int/extensional/basic.icc"

#if GECODE_USE_ADVISORS

namespace Gecode { namespace Int { namespace Extensional {
  struct SupportEntry {
    tuple t;
    SupportEntry* next;
  };

  /**
   * \brief Domain-consistent extensional propagator
   *
   * This propagator implements an incremental propagation algorithm
   * where supports are maintained explicitly.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class Incremental : public NaryPropagator<View,PC_INT_VAL> {
  protected:
    using NaryPropagator<View,PC_INT_VAL>::x;
    Table table;
    tuple*** last_data;
    SupportEntry** support_data;

    /// Constructor for cloning \a p
    Incremental(Space* home, bool share, Incremental<View>& p);
    /// Constructor for posting
    Incremental(Space* home, ViewArray<View>& x, const Table& t);

    tuple last(int var, int val);
    tuple last_next(int var, int val);
    bool  valid(tuple t, Support::BitSet<unsigned long>* dom);
    SupportEntry* support(int var, int val);
  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is
     * PC_QUADRATIC_HI. Otherwise it is dynamic PC_CUBIC_HI.
     */
    virtual PropCost cost(void) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static ExecStatus post(Space* home, ViewArray<View>& x, const Table& t);
    // Dispose propagator
    size_t dispose(Space* home);


    //
    // Advisor proper
    //
  private:
    class SupportAdvisor : public IntUnaryViewAdvisor<View> {
      using IntUnaryViewAdvisor<View>::x;
      unsigned int pos;
    public:
      SupportAdvisor(Space* home, Propagator* p, View v, unsigned int position) 
        : IntUnaryViewAdvisor<View>(home,p,v), pos(position) {
        if (x.assigned()) {
          IntViewAdvisor<View>::schedule(home, Int::ME_INT_VAL);
        }
      }
      SupportAdvisor(Space* home, bool share, SupportAdvisor& d) 
        : IntUnaryViewAdvisor<View>(home, share, d), pos(d.pos) {}
      size_t dispose(Space* home) {
        (void) IntUnaryViewAdvisor<View>::dispose(home);
        return sizeof(*this);
      }
      ExecStatus advise(Space *home, ModEvent me, int lo, int hi);
    };
    
  public:
    typedef Council<SupportAdvisor> DC;
  private:
    DC dc;
  };
}}}

#include "gecode/int/extensional/incremental.icc"
#endif

#endif

// STATISTICS: int-prop

