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
#include "gecode/support/dynamic-stack.hh"

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */

namespace Gecode { namespace Int { namespace Extensional {
  typedef Table::tuple tuple;
  typedef Support::BitSet<unsigned long>* Domain;

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
   * \brief Base for domain-consistent extensional proipagation.
   *
   * This class contains support for implementing domain consistent
   * extensional propagation algorithms that use positive tables and
   * the \a last data structure.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, bool subscribe = true>
  class Base : public Propagator {
  protected:
    ViewArray<View> x;
    Table table;
    tuple** last_data;

    /// Constructor for cloning \a p
    Base(Space* home, bool share, Base<View,subscribe>& p);
    /// Constructor for posting
    Base(Space* home, ViewArray<View>& x, const Table& t);

    void  init_last(Space* home, tuple** source);
    tuple last(int var, int val);
    tuple last_next(int var, int val);
    void  init_dom(Space* home, Domain dom);
    bool  valid(tuple t, Domain dom);
    tuple find_support(Domain dom, int var, int val);

  public:
    virtual PropCost cost() const;

    // Dispose propagator
    virtual size_t dispose(Space* home);
  };


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
  class Basic : public Base<View> {
  protected:
    using Base<View>::x;
    using Base<View>::table;
    using Base<View>::last;
    using Base<View>::last_next;
    using Base<View>::init_last;
    using Base<View>::init_dom;
    using Base<View>::find_support;

    /// Constructor for cloning \a p
    Basic(Space* home, bool share, Basic<View>& p);
    /// Constructor for posting
    Basic(Space* home, ViewArray<View>& x, const Table& t);

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
#include "gecode/int/extensional/base.icc"
#include "gecode/int/extensional/basic.icc"


#ifdef GECODE_USE_ADVISORS

namespace Gecode { namespace Int { namespace Extensional {
  /*
  struct SupportEntry {
    tuple t;
    SupportEntry* next;
    SupportEntry(tuple t0=NULL, SupportEntry* n0=NULL)
      : t(t0), next(n0) {}
  };
  */

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
  class Incremental : public Base<View, false> {
  protected:
    using Base<View, false>::x;
    using Base<View, false>::table;
    using Base<View, false>::last;
    using Base<View, false>::last_next;
    using Base<View, false>::init_last;
    using Base<View, false>::init_dom;
    struct SupportEntry : public FreeList {
    public:
      /// Supporting tuple
      tuple t;
      /// Next support
      SupportEntry* next;
      
      /// \name Constructors
      //@{
      /// Default constructor (noop)
      SupportEntry(void);
      /// Initialize with tuple and next (defaulting to NULL)
      SupportEntry(tuple t0, SupportEntry* n0 = NULL);
      //@}
      
      /// \name Memory management
      //@{
      /**
       * \brief Free memory for all elements between this and \a l (inclusive)
       *
       * This routine assumes that the list has already been fixed.
       */
      void dispose(Space* home, SupportEntry* l);
      /// Free memory for this element
      void dispose(Space* home);
      
      /// Allocate memory from space
      static void* operator new(size_t s, Space* home);
      /// No-op (for exceptions)
      static void  operator delete(void*);
      /// No-op (use dispose instead)
      static void  operator delete(void*, Space*);
      //@}
    };
    enum WorkType {
      WT_FIND_SUPPORT,
      WT_REMOVE_VALUE
    };
    struct Work {
      WorkType work;
      int var;
      int val;
      Work(WorkType w0, int var0, int val0)
        : work(w0), var(var0), val(val0) {}
    };



    typedef Support::DynamicStack<Work> WorkStack;
    SupportEntry** support_data;
    WorkStack work;
    int unassigned;

    /// Constructor for cloning \a p
    Incremental(Space* home, bool share, Incremental<View>& p);
    /// Constructor for posting
    Incremental(Space* home, ViewArray<View>& x, const Table& t);

    void   find_support(Space* home, Domain dom, int var, int val);
    void   init_support(Space* home);
    void    add_support(Space* home, tuple l);
    void remove_support(Space* home, tuple l, int var, int val
#if defined(SYSTEM_ADVISOR_IMPROVE_EXPENSIVE)
                        , Domain dom
#endif
                        );
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
    class SupportAdvisor : public ViewAdvisor<View> {
    public:
      using ViewAdvisor<View>::x;
      unsigned int pos;
      SupportAdvisor(Space* home, Propagator* p, CouncilBase& c,
                     View v, unsigned int position) 
        : ViewAdvisor<View>(home,p,c,v), pos(position) {}
      SupportAdvisor(Space* home, bool share, SupportAdvisor& a) 
        : ViewAdvisor<View>(home, share, a), pos(a.pos) {
      }
    };
    Council<SupportAdvisor> ac;
  public:
    virtual ExecStatus advise(Space* home, Advisor& a, const Delta& d);
  };
  
}}}

#include "gecode/int/extensional/incremental.icc"
#endif

#endif

// STATISTICS: int-prop

