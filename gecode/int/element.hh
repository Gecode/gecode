/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

#ifndef __GECODE_INT_ELEMENT_HH__
#define __GECODE_INT_ELEMENT_HH__

#include "gecode/int.hh"
#include "gecode/int/rel.hh"

/**
 * \namespace Gecode::Int::Element
 * \brief %Element propagators
 */

namespace Gecode { namespace Int { namespace Element {

  /*
   * Element constraint for array of integers
   *
   */

  typedef SharedArray<int> IntSharedArray;

  /**
   * \brief %Element propagator for array of integers
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class ViewA, class ViewB>
  class Int : public Propagator {
  protected:
    /**
     * \brief Linked index-value pairs
     *
     * Data structure linking pairs of index and value (index,value)
     * where pairs are linked in order of both index and
     * value (to allow for easy removal while keeping both index and
     * value sorted).
     */
    class IdxVal  {
    public:
      IdxVal* idx_next; ///< The next pair in index order
      IdxVal* val_next; ///< The next pair in value order
      int idx; ///< The index
      int val; ///< The value
      /// Mark that this pair should be removed
      void mark(void);
      /// Return whether this pair is marked for removal
      bool marked(void) const;
    };
    /**
     * \brief Value iterator for indices in index-value map
     *
     * The iterator also removes marked index-value pairs.
     *
     */
    class IterIdx {
    private:
      IdxVal* l; ///< Current index value pair
    public:
      /// Initialize with pair \a l
      IterIdx(IdxVal& l);
      /// Test whether more pairs to be iterated
      bool operator()(void) const;
      /// Move to next index value pair (next index)
      void operator++(void);
      /// Return index of current index value pair
      int val(void) const;
    };
    /**
     * \brief Value iterator for values in index-value map
     *
     * Note that the iterated value sequence is not strictly
     * increasing (might contain duplicates).
     */
    class IterVal {
    private:
      const IdxVal* l; ///< Current index value pair
    public:
      /// Initialize with pair \a l
      IterVal(const IdxVal& l);
      /// Test whether more pairs to be iterated
      bool operator()(void) const;
      /// Move to next index value pair (next value)
      void operator++(void);
      /// Return value of current index value pair
      int val(void) const;
    };
    /// Sorting pointers to (index,value) pairs in value order
    class ByVal {
    public:
      bool operator()(IdxVal*&, IdxVal*&);
    };

    /// View for index
    ViewA x0;
    /// View for result
    ViewB x1;
    /// Shared array of integer values
    IntSharedArray c;
    /// The index-value data structure
    IdxVal* iv;
    /// Constructor for cloning \a p
    Int(Space& home, bool shared, Int& p);
    /// Constructor for creation
    Int(Space& home, IntSharedArray& i, ViewA x0, ViewB x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Const function (return PC_BINARY_HI)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                       Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space& home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Mangled name of this propagator
    static Support::Symbol ati(void);
    /// Post propagator for \f$i_{x_0}=x_1\f$
    static  ExecStatus post(Space& home, IntSharedArray& i,
                            ViewA x0, ViewB x1);
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };



  /**
   * \brief Class for index-view map
   *
   */
  template <class ViewB> class IdxView;

  /**
   * \brief Base-class for element propagator for array of views
   *
   */
  template <class VA, class VB, class VC, PropCond pc_ac>
  class View : public Propagator {
  protected:
    /// Current index-view map
    IdxView<VA>* iv;
    /// Number of elements in index-view map
    int n;
    /// View for index
    VB x0;
    /// View for result
    VC x1;
    /// Constructor for cloning \a p
    View(Space& home, bool share, View& p);
    /// Constructor for creation
    View(Space& home, IdxView<VA>* iv, int n, VB x0, VC x1);
    /// Specification for this propagator
    Reflection::ActorSpec spec(const Space& home, Reflection::VarMap& m,
                                const Support::Symbol& name) const;
  public:
    // Cost function (defined as dynamic PC_LINEAR_LO)
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Delete propagator and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief Bounds consistent element propagator for array of views
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VA, class VB, class VC>
  class ViewBnd : public View<VA,VB,VC,PC_INT_BND> {
  protected:
    using View<VA,VB,VC,PC_INT_BND>::iv;
    using View<VA,VB,VC,PC_INT_BND>::n;
    using View<VA,VB,VC,PC_INT_BND>::x0;
    using View<VA,VB,VC,PC_INT_BND>::x1;

    /// Constructor for cloning \a p
    ViewBnd(Space& home, bool share, ViewBnd& p);
    /// Constructor for creation
    ViewBnd(Space& home, IdxView<VA>* iv, int n, VB x0, VC x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space& home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
    /// Post propagator for \f$iv_{x_0}=x_1\f$
    static  ExecStatus post(Space& home, IdxView<VA>* iv, int n,
                            VB x0, VC x1);
  };

  /**
   * \brief Domain consistent element propagator for array of views
   *
   * The propagator uses staging: first it uses
   * bounds-propagation on the array of views and the uses
   * domain-propagation on the array of views.
   *
   * Requires \code #include "gecode/int/element.hh" \endcode
   * \ingroup FuncIntProp
   */
  template <class VA, class VB, class VC>
  class ViewDom : public View<VA,VB,VC,PC_INT_DOM> {
  protected:
    using View<VA,VB,VC,PC_INT_DOM>::iv;
    using View<VA,VB,VC,PC_INT_DOM>::n;
    using View<VA,VB,VC,PC_INT_DOM>::x0;
    using View<VA,VB,VC,PC_INT_DOM>::x1;

    /// Constructor for cloning \a p
    ViewDom(Space& home, bool share, ViewDom& p);
    /// Constructor for creation
    ViewDom(Space& home, IdxView<VA>* iv, int n, VB x0, VC x1);
  public:
    /// Perform copying during cloning
    virtual Actor* copy(Space& home, bool share);
    /**
     * \brief Cost function
     *
     * If in stage for bounds-propagation defined as dynamic PC_LINEAR_LO,
     * otherwise as dynamic PC_LINEAR_HI.
     *
     */
    virtual PropCost cost(const Space& home, const ModEventDelta& med) const;
    /// Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
    /// Specification for this propagator
    virtual Reflection::ActorSpec spec(const Space& home,
                                        Reflection::VarMap& m) const;
    /// Post propagator according to specification
    static void post(Space& home, Reflection::VarMap& vars,
                     const Reflection::ActorSpec& spec);
    /// Name of this propagator
    static Support::Symbol ati(void);
    /// Post propagator for \f$iv_{x_0}=x_1\f$
    static  ExecStatus post(Space& home, IdxView<VA>* iv, int n,
                            VB x0, VC x1);
  };

}}}

#include "gecode/int/element/int.icc"
#include "gecode/int/element/view.icc"

#endif


// STATISTICS: int-prop

