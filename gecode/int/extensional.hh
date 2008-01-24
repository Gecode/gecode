/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
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

#ifndef __GECODE_INT_EXTENSIONAL_HH__
#define __GECODE_INT_EXTENSIONAL_HH__

#include "gecode/int.hh"

#include "gecode/int/rel.hh"

/**
 * \namespace Gecode::Int::Extensional
 * \brief %Extensional propagators
 */

namespace Gecode { namespace Int { namespace Extensional {

  class Layer;
  class State;

  /**
   * \brief Domain-consistent layered graph (regular) propagator
   *
   * The algorithm for the regular propagator is based on:
   *   Gilles Pesant, A Regular Language Membership Constraint
   *   for Finite Sequences of Variables, CP 2004.
   *   Pages 482-495, LNCS 3258, Springer-Verlag, 2004.
   *
   * The propagator is not capable of multiple occurences of the same
   * view.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View>
  class LayeredGraph : public Propagator {
  protected:
    /// Advisors for views (by position in array)
    class Index : public Advisor {
    public:
      /// The position of the view in the view array
      int i;
      /// Create index advisor
      Index(Space* home, Propagator* p, Council<Index>& c, int i);
      /// Clone index advisor \a a
      Index(Space* home, bool share, Index& a);
      /// Dispose advisor
      void dispose(Space* home, Council<Index>& c);
    };
    /// Range approximation of which positions have changed
    class IndexRange {
    private:
      int _fst; ///< First index
      int _lst; ///< Last index
    public:
      /// Initialize range as empty
      IndexRange(void);
      /// Reset range to be empty
      void reset(void);
      /// Add index \a i to range
      void add(int i);
      /// Return first position
      int fst(void) const;
      /// Return last position
      int lst(void) const;
    };
    /// The advisor council
    Council<Index> c;
    /// The views
    ViewArray<View> x;
    /// The %DFA describing the language
    DFA dfa;
    /// The start state for graph construction
    int start;
    /// The layers of the graph
    Layer* layers;
    /// The states used in the graph
    State* states;
    /// Index range with in-degree modifications
    IndexRange i_ch;
    /// Index range with out-degree modifications
    IndexRange o_ch;
    
    /// Test whether layered graph has already been constructed
    bool constructed(void) const;
    /// Eliminate assigned prefix, return how many layers removed
    void eliminate(void);
    /// Construct layered graph
    ExecStatus construct(Space* home);
    /// Prune incrementally
    ExecStatus prune(Space* home);

    /// Constructor for cloning \a p
    LayeredGraph(Space* home, bool share, LayeredGraph<View>& p);
  public:
    /// Constructor for posting
    LayeredGraph(Space* home, ViewArray<View>& x, DFA& d);
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Cost function (defined as dynamic PC_LINEAR_HI)
    virtual PropCost cost(ModEventDelta med) const;
    /// Give advice to propagator
    virtual ExecStatus advise(Space* home, Advisor* a, const Delta* d);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Mangled name of this propagator
    static Gecode::Support::Symbol ati(void);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
    /// Post propagator on views \a x and DFA \a d
    static ExecStatus post(Space* home, ViewArray<View>& x, DFA& d);
  };

}}}

#include "gecode/int/extensional/layered-graph.icc"


#include "gecode/int/extensional/bitset.icc"

namespace Gecode { namespace Int { namespace Extensional {

  typedef TupleSet::Tuple Tuple;
  typedef BitSet* Domain;

  /**
   * \brief Base for domain-consistent extensional propagation
   *
   * This class contains support for implementing domain consistent
   * extensional propagation algorithms that use positive tuple sets and
   * a \a last data structure.
   *
   * Requires \code #include "gecode/int/extensional.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class View, bool subscribe = true>
  class Base : public Propagator {
  protected:
    ViewArray<View> x; ///< Variables
    TupleSet tupleSet; ///< Definition of constraint
    Tuple** last_data; ///< Last tuple looked at
    /// Access real tuple-set
    TupleSet::TupleSetI* ts(void);

    /// Constructor for cloning \a p
    Base(Space* home, bool share, Base<View,subscribe>& p);
    /// Constructor for posting
    Base(Space* home, ViewArray<View>& x, const TupleSet& t);

    void  init_last(Space* home, Tuple** source);
    Tuple last(int var, int val);
    Tuple last_next(int var, int val);
    void  init_dom(Space* home, Domain dom);
    bool  valid(Tuple t, Domain dom);
    Tuple find_support(Domain dom, int var, int val);

  public:
    virtual PropCost cost(ModEventDelta med) const;

    // Dispose propagator
    virtual size_t dispose(Space* home);
  };
}}}

#include "gecode/int/extensional/base.icc"


namespace Gecode { namespace Int { namespace Extensional {

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
    using Base<View>::tupleSet;
    using Base<View>::ts;
    using Base<View>::last;
    using Base<View>::last_next;
    using Base<View>::init_last;
    using Base<View>::init_dom;
    using Base<View>::find_support;

    /// Constructor for cloning \a p
    Basic(Space* home, bool share, Basic<View>& p);
    /// Constructor for posting
    Basic(Space* home, ViewArray<View>& x, const TupleSet& t);

  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is
     * PC_QUADRATIC_HI. Otherwise it is dynamic PC_CUBIC_HI.
     */
    virtual PropCost cost(ModEventDelta med) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static ExecStatus post(Space* home, ViewArray<View>& x, const TupleSet& t);

    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Mangled name of this propagator
    static Gecode::Support::Symbol ati(void);
  };
}}}

#include "gecode/int/extensional/basic.icc"


namespace Gecode { namespace Int { namespace Extensional {
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
    using Base<View, false>::tupleSet;
    using Base<View, false>::ts;
    using Base<View, false>::last;
    using Base<View, false>::last_next;
    using Base<View, false>::init_last;
    using Base<View, false>::init_dom;
    /// Entry for storing support
    class SupportEntry : public FreeList {
    public:
      /// Supporting Tuple
      Tuple t;
      /// Next support
      SupportEntry* next;
      
      /// \name Constructors
      //@{
      /// Default constructor (noop)
      SupportEntry(void);
      /// Initialize with Tuple and next (defaulting to NULL)
      SupportEntry(Tuple t0, SupportEntry* n0 = NULL);
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
    /// What is to be done
    enum WorkType {
      WT_FIND_SUPPORT,
      WT_REMOVE_VALUE
    };
    /// Description of work to be done
    class Work {
    public:
      WorkType work;
      int var;
      int val;
      Work(WorkType w0, int var0, int val0)
        : work(w0), var(var0), val(val0) {}
    };



    typedef ::Gecode::Support::DynamicStack<Work> WorkStack;
    SupportEntry** support_data;
    WorkStack work;
    int unassigned;

    /// Constructor for cloning \a p
    Incremental(Space* home, bool share, Incremental<View>& p);
    /// Constructor for posting
    Incremental(Space* home, ViewArray<View>& x, const TupleSet& t);

    void   find_support(Space* home, Domain dom, int var, int val);
    void   init_support(Space* home);
    void    add_support(Space* home, Tuple l);
    void remove_support(Space* home, Tuple l, int var, int val);
    SupportEntry* support(int var, int val);
  public:
    /// Perform propagation
    virtual ExecStatus propagate(Space* home, ModEventDelta med);
    /**
     * \brief Cost function
     *
     * If in stage for naive value propagation, the cost is
     * PC_QUADRATIC_HI. Otherwise it is dynamic PC_CUBIC_HI.
     */
    virtual PropCost cost(ModEventDelta med) const;
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Post propagator for views \a x
    static ExecStatus post(Space* home, ViewArray<View>& x, const TupleSet& t);
    // Dispose propagator
    size_t dispose(Space* home);


    /// Specification for this propagator
    virtual Reflection::ActorSpec& spec(const Space* home,
                                        Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space* home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Mangled name of this propagator
    static Gecode::Support::Symbol ati(void);


    //
    // Advisor proper
    //
  private:
    class SupportAdvisor : public ViewAdvisor<View> {
    public:
      using ViewAdvisor<View>::x;
      unsigned int pos;
      SupportAdvisor(Space* home, Propagator* p, Council<SupportAdvisor>& c,
                     View v, unsigned int position) 
        : ViewAdvisor<View>(home,p,c,v), pos(position) {}
      SupportAdvisor(Space* home, bool share, SupportAdvisor& a) 
        : ViewAdvisor<View>(home, share, a), pos(a.pos) {
      }
      /// Dispose advisor
      void dispose(Space* home, Council<SupportAdvisor>& c) {
            ViewAdvisor<View>::dispose(home,c);
      }
    };
    Council<SupportAdvisor> ac;
  public:
    virtual ExecStatus advise(Space* home, Advisor* a, const Delta* d);
  };
  
}}}

#include "gecode/int/extensional/incremental.icc"


#endif

// STATISTICS: int-prop

