/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>

 *  Copyright:
 *     Christian Schulte, 2012
 *     Samuel Gagnon, 2018
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

#ifndef __GECODE_INT_BRANCH_HH__
#define __GECODE_INT_BRANCH_HH__

#include <gecode/int.hh>

/**
 * \namespace Gecode::Int::Branch
 * \brief Integer branchers
 */

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \defgroup FuncIntViewSel Merit-based integer view selection for branchers
   *
   * Contains merit-based view selection strategies on integer
   * views that can be used together with the generic view/value
   * brancher classes.
   *
   * All merit-based view selection classes require
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup Other
   */

  /**
   * \brief Merit class for mimimum of integer views
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritMin : public MeritBase<View,int> {
  public:
    using typename MeritBase<View,int>::Var;
    /// Constructor for initialization
    MeritMin(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritMin(Space& home, MeritMin& m);
    /// Return minimum as merit for view \a x at position \a i
    int operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for maximum
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritMax : public MeritBase<View,int> {
  public:
    using typename MeritBase<View,int>::Var;
    /// Constructor for initialization
    MeritMax(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritMax(Space& home, MeritMax& m);
    /// Return maximum as merit for view \a x at position \a i
    int operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritSize : public MeritBase<View,unsigned int> {
  public:
    using typename MeritBase<View,unsigned int>::Var;
    /// Constructor for initialization
    MeritSize(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritSize(Space& home, MeritSize& m);
    /// Return size as merit for view \a x at position \a i
    unsigned int operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for degree over size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritDegreeSize : public MeritBase<View,double> {
  public:
    using typename MeritBase<View,double>::Var;
    /// Constructor for initialization
    MeritDegreeSize(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritDegreeSize(Space& home, MeritDegreeSize& m);
    /// Return degree over size as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for AFC over size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritAFCSize : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// AFC information
    AFC afc;
  public:
    /// Constructor for initialization
    MeritAFCSize(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritAFCSize(Space& home, MeritAFCSize& m);
    /// Return AFC over size as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for action over size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritActionSize : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// Action information
    Action action;
  public:
    /// Constructor for initialization
    MeritActionSize(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritActionSize(Space& home, MeritActionSize& m);
    /// Return action over size as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for CHB over size
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritCHBSize : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// CHB information
    CHB chb;
  public:
    /// Constructor for initialization
    MeritCHBSize(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritCHBSize(Space& home, MeritCHBSize& m);
    /// Return size over action as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for minimum regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritRegretMin : public MeritBase<View,unsigned int> {
  public:
    using typename MeritBase<View,unsigned int>::Var;
    /// Constructor for initialization
    MeritRegretMin(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritRegretMin(Space& home, MeritRegretMin& m);
    /// Return minimum regret as merit for view \a x at position \a i
    unsigned int operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for maximum regret
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntViewSel
   */
  template<class View>
  class MeritRegretMax : public MeritBase<View,unsigned int> {
  public:
    using typename MeritBase<View,unsigned int>::Var;
    /// Constructor for initialization
    MeritRegretMax(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritRegretMax(Space& home, MeritRegretMax& m);
    /// Return maximum regret as merit for view \a x at position \a i
    unsigned int operator ()(const Space& home, View x, int i);
  };

}}}

#include <gecode/int/branch/merit.hpp>

namespace Gecode { namespace Int { namespace Branch {

  /// Return view selectors for integer views
  GECODE_INT_EXPORT
  ViewSel<IntView>* viewsel(Space& home, const IntVarBranch& ivb);
  /// Return view selectors for Boolean views
  GECODE_INT_EXPORT
  ViewSel<BoolView>* viewsel(Space& home, const BoolVarBranch& bvb);

}}}

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \defgroup FuncIntValSel Integer value selection for brancher
   *
   * Contains a description of value selection strategies on integer
   * views that can be used together with the generic view/value
   * branchers.
   *
   * All value selection classes require
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup Other
   */

  /**
   * \brief Value selection class for mimimum of view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  template<class View>
  class ValSelMin : public ValSel<View,int> {
  public:
    using typename ValSel<View,int>::Var;
    /// Constructor for initialization
    ValSelMin(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelMin(Space& home, ValSelMin& vs);
    /// Return value of view \a x at position \a i
    int val(const Space& home, View x, int i);
  };

  /**
   * \brief Value selection class for maximum of view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  template<class View>
  class ValSelMax : public ValSel<View,int> {
  public:
    using typename ValSel<View,int>::Var;
    /// Constructor for initialization
    ValSelMax(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelMax(Space& home, ValSelMax& vs);
    /// Return value of view \a x at position \a i
    int val(const Space& home, View x, int i);
  };

  /**
   * \brief Value selection class for median of view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  template<class View>
  class ValSelMed : public ValSel<View,int> {
  public:
    using typename ValSel<View,int>::Var;
    /// Constructor for initialization
    ValSelMed(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelMed(Space& home, ValSelMed& vs);
    /// Return value of view \a x at position  i
    int val(const Space& home, View x, int i);
  };

  /**
   * \brief Value selection class for average of view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  template<class View>
  class ValSelAvg : public ValSel<View,int> {
  public:
    using typename ValSel<View,int>::Var;
    /// Constructor for initialization
    ValSelAvg(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelAvg(Space& home, ValSelAvg& vs);
    /// Return value of view \a x at position \a i
    int val(const Space& home, View x, int i);
  };

  /**
   * \brief Value selection class for random value of view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  template<class View>
  class ValSelRnd : public ValSel<View,int> {
    using typename ValSel<View,int>::Var;
  protected:
    /// The used random number generator
    Rnd r;
  public:
    /// Constructor for initialization
    ValSelRnd(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelRnd(Space& home, ValSelRnd& vs);
    /// Return value of view \a x at position \a i
    int val(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete value selection
    void dispose(Space& home);
  };

  /**
   * \brief Value selection class for minimum range of integer view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  class ValSelRangeMin : public ValSel<IntView,int> {
  public:
    /// Constructor for initialization
    ValSelRangeMin(Space& home, const ValBranch<IntVar>& vb);
    /// Constructor for cloning
    ValSelRangeMin(Space& home, ValSelRangeMin& vs);
    /// Return value of integer view \a x at position \a i
    int val(const Space& home, IntView x, int i);
  };

  /**
   * \brief Value selection class for maximum range of integer view
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValSel
   */
  class ValSelRangeMax : public ValSel<IntView,int> {
  public:
    /// Constructor for initialization
    ValSelRangeMax(Space& home, const ValBranch<IntVar>& vb);
    /// Constructor for cloning
    ValSelRangeMax(Space& home, ValSelRangeMax& vs);
    /// Return value of integer view \a x at position \a i
    int val(const Space& home, IntView x, int i);
  };

}}}

#include <gecode/int/branch/val-sel.hpp>

namespace Gecode { namespace Int { namespace Branch {

  /// No-good literal for equality
  template<class View>
  class EqNGL : public ViewValNGL<View,int,PC_INT_VAL> {
    using ViewValNGL<View,int,PC_INT_VAL>::x;
    using ViewValNGL<View,int,PC_INT_VAL>::n;
  public:
    /// Constructor for creation
    EqNGL(Space& home, View x, int n);
    /// Constructor for cloning \a ngl
    EqNGL(Space& home, EqNGL& ngl);
    /// Test the status of the no-good literal
    virtual NGL::Status status(const Space& home) const;
    /// Propagate the negation of the no-good literal
    virtual ExecStatus prune(Space& home);
    /// Create copy
    virtual NGL* copy(Space& home);
  };

  /// No-good literal for disequality
  template<class View>
  class NqNGL : public ViewValNGL<View,int,PC_INT_DOM> {
    using ViewValNGL<View,int,PC_INT_DOM>::x;
    using ViewValNGL<View,int,PC_INT_DOM>::n;
  public:
    /// Constructor for creation
    NqNGL(Space& home, View x, int n);
    /// Constructor for cloning \a ngl
    NqNGL(Space& home, NqNGL& ngl);
    /// Test the status of the no-good literal
    virtual NGL::Status status(const Space& home) const;
    /// Propagate the negation of the no-good literal
    virtual ExecStatus prune(Space& home);
    /// Create copy
    virtual NGL* copy(Space& home);
  };

  /// No-good literal for less or equal
  template<class View>
  class LqNGL : public ViewValNGL<View,int,PC_INT_BND> {
    using ViewValNGL<View,int,PC_INT_BND>::x;
    using ViewValNGL<View,int,PC_INT_BND>::n;
  public:
    /// Constructor for creation
    LqNGL(Space& home, View x, int n);
    /// Constructor for cloning \a ngl
    LqNGL(Space& home, LqNGL& ngl);
    /// Test the status of the no-good literal
    virtual NGL::Status status(const Space& home) const;
    /// Propagate the negation of the no-good literal
    virtual ExecStatus prune(Space& home);
    /// Create copy
    virtual NGL* copy(Space& home);
  };

  /// No-good literal for greater or equal
  template<class View>
  class GqNGL : public ViewValNGL<View,int,PC_INT_BND> {
    using ViewValNGL<View,int,PC_INT_BND>::x;
    using ViewValNGL<View,int,PC_INT_BND>::n;
  public:
    /// Constructor for creation
    GqNGL(Space& home, View x, int n);
    /// Constructor for cloning \a ngl
    GqNGL(Space& home, GqNGL& ngl);
    /// Test the status of the no-good literal
    virtual NGL::Status status(const Space& home) const;
    /// Propagate the negation of the no-good literal
    virtual ExecStatus prune(Space& home);
    /// Create copy
    virtual NGL* copy(Space& home);
  };

}}}

#include <gecode/int/branch/ngl.hpp>

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \defgroup FuncIntValCommit Integer value commit classes
   *
   * Contains the value commit classes for integer and Boolean
   * views that can be used together with the generic view/value
   * branchers.
   *
   * All value commit classes require
   * \code #include <gecode/int/branch.hh> \endcode
   * \ingroup Other
   */

  /**
   * \brief Value commit class for equality
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValCommit
   */
  template<class View>
  class ValCommitEq : public ValCommit<View,int> {
  public:
    using typename ValCommit<View,int>::Var;
    /// Constructor for initialization
    ValCommitEq(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValCommitEq(Space& home, ValCommitEq& vc);
    /// Commit view \a x at position \a i to value \a n for alternative \a a
    ModEvent commit(Space& home, unsigned int a, View x, int i, int n);
    /// Create no-good literal for alternative \a a
    NGL* ngl(Space& home, unsigned int a, View x, int n) const;
    /// Print on \a o the alternative \a with view \a x at position \a i and value \a n
    void print(const Space& home, unsigned int a, View x, int i, int n,
               std::ostream& o) const;
  };

  /**
   * \brief Value commit class for less or equal
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValCommit
   */
  template<class View>
  class ValCommitLq : public ValCommit<View,int> {
  public:
    using typename ValCommit<View,int>::Var;
    /// Constructor for initialization
    ValCommitLq(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValCommitLq(Space& home, ValCommitLq& vc);
    /// Commit view \a x at position \a i to value \a n for alternative \a a
    ModEvent commit(Space& home, unsigned int a, View x, int i, int n);
    /// Create no-good literal for alternative \a a
    NGL* ngl(Space& home, unsigned int a, View x, int n) const;
    /// Print on \a o the alternative \a with view \a x at position \a i and value \a n
    void print(const Space& home, unsigned int a, View x, int i, int n,
               std::ostream& o) const;
  };

  /**
   * \brief Value commit class for greater or equal
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValCommit
   */
  template<class View>
  class ValCommitGq : public ValCommit<View,int> {
  public:
    using typename ValCommit<View,int>::Var;
    /// Constructor for initialization
    ValCommitGq(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValCommitGq(Space& home, ValCommitGq& vc);
    /// Commit view \a x at position \a i to value \a n for alternative \a a
    ModEvent commit(Space& home, unsigned int a, View x, int i, int n);
    /// Create no-good literal for alternative \a a
    NGL* ngl(Space& home, unsigned int a, View x, int n) const;
    /// Print on \a o the alternative \a with view \a x at position \a i and value \a n
    void print(const Space& home, unsigned int a, View x, int i, int n,
               std::ostream& o) const;
  };

  /**
   * \brief Value commit class for greater
   *
   * Requires \code #include <gecode/int/branch.hh> \endcode
   * \ingroup FuncIntValCommit
   */
  template<class View>
  class ValCommitGr : public ValCommit<View,int> {
  public:
    using typename ValCommit<View,int>::Var;
    /// Constructor for initialization
    ValCommitGr(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValCommitGr(Space& home, ValCommitGr& vc);
    /// Commit view \a x at position \a i to value \a n for alternative \a a
    ModEvent commit(Space& home, unsigned int a, View x, int i, int n);
    /// Create no-good literal for alternative \a a
    NGL* ngl(Space& home, unsigned int a, View x, int n) const;
    /// Print on \a o the alternative \a with view \a x at position \a i and value \a n
    void print(const Space& home, unsigned int a, View x, int i, int n,
               std::ostream& o) const;
  };

}}}

#include <gecode/int/branch/val-commit.hpp>

namespace Gecode { namespace Int { namespace Branch {

  /// Return value and commit for integer views
  GECODE_INT_EXPORT
  ValSelCommitBase<IntView,int>*
  valselcommit(Space& home, const IntValBranch& ivb);

  /// Return value and commit for Boolean views
  GECODE_INT_EXPORT
  ValSelCommitBase<BoolView,int>*
  valselcommit(Space& home, const BoolValBranch& bvb);

  /// Return value and commit for integer views
  GECODE_INT_EXPORT
  ValSelCommitBase<IntView,int>*
  valselcommit(Space& home, const IntAssign& ia);

  /// Return value and commit for Boolean views
  GECODE_INT_EXPORT
  ValSelCommitBase<BoolView,int>*
  valselcommit(Space& home, const BoolAssign& ba);

}}}

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \brief %Brancher by view and values selection
   *
   */
  template<int n, bool min, class Filter, class Print>
  class ViewValuesBrancher : public ViewBrancher<IntView,Filter,n> {
  protected:
    using ViewBrancher<IntView,Filter,n>::x;
    using ViewBrancher<IntView,Filter,n>::f;
    /// Print function
    Print p;
    /// Constructor for cloning \a b
    ViewValuesBrancher(Space& home, ViewValuesBrancher& b);
    /// Constructor for creation
    ViewValuesBrancher(Home home, ViewArray<IntView>& x,
                       ViewSel<IntView>* vs[n],
                       IntBranchFilter bf,
                       IntVarValPrint vvp);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a a
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int a);
    /// Create no-good literal for choice \a c and alternative \a a
    virtual NGL* ngl(Space& home, const Choice& c, unsigned int a) const;
    /**
     * \brief Print branch for choice \a c and alternative \a a
     *
     * Prints an explanation of the alternative \a a of choice \a c
     * on the stream \a o.
     *
     */
    virtual void print(const Space& home, const Choice& c, unsigned int a,
                       std::ostream& o) const;
    /// Perform cloning
    virtual Actor* copy(Space& home);
    /// Post function for creation
    static void post(Home home, ViewArray<IntView>& x,
                     ViewSel<IntView>* vs[n],
                     IntBranchFilter bf,
                     IntVarValPrint vvp);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
  };

  /// Post brancher for view and values
  template<int n, bool min>
  void postviewvaluesbrancher(Home home, ViewArray<IntView>& x,
                              ViewSel<IntView>* vs[n],
                              IntBranchFilter bf,
                              IntVarValPrint vvp);

}}}

#include <gecode/int/branch/view-values.hpp>

#ifdef GECODE_HAS_CBS

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \brief %Brancher using counting-based search
   *
   */
  template<class View>
  class CBSBrancher : public Brancher {
  private:
    /// Views to branch on
    ViewArray<View> x;

    /**
     * \brief Map of variable ids to positions in \a x
     *
     * Once created, this mapping doesn't need to be updated. Because of this,
     * we can share a handle of the map across all instances of CBSBrancher in
     * all spaces.
     */
    class VarIdToPos : public SharedHandle {
    protected:
      class VarIdToPosO : public SharedHandle::Object {
      public:
        /// The map we want to share
        std::unordered_map<unsigned int, unsigned int> _varIdToPos;
      public:
        /// Default constructur
        VarIdToPosO(void) = default;
        /// Delete implementation
        virtual ~VarIdToPosO(void) = default;
      };
    public:
      /// Default constructor
      VarIdToPos(void) = default;
      /// Allocation of the shared handle
      void init(void);
      /// Tests if a variable id is in the map
      bool isIn(unsigned int var_id) const;
      /// Returns the position of the variable id in \a x
      int operator[](unsigned int var_id) const;
      /// Inserts a new position for a variable id
      void insert(unsigned int var_id, unsigned int pos);
    } varIdToPos;

    /**
     * \brief Propagator information for counting-based search
     *
     * Keeps the best branching choice for each propagators (i.e. variable and
     * value with highest solution density). We also keep \a domsum to know
     * wether we need to recompute solution densities for the given propagator
     * when computing a new branching choice.
     */
    struct PropInfo {
      /// Sum of variable cardinalities
      unsigned int domsum;
      /// Variable with the highest solution density
      unsigned int var_id;
      /// Value with highest solution density
      int val;
      /// Density of the above <var_id, val> pair
      double dens;
      /// Flag for deleting propagator if unvisited when branching
      bool visited;
    };

    /**
     * \brief Map of propagator ids to PropInfo
     *
     * Any active propagator that:
     *
     *   - shares a variable with \a x
     *   - has a counting algorithm (i.e. overloads solndistrib)
     *
     * will have an entry in this map.
     */
    std::unordered_map<unsigned int, PropInfo,
                       std::hash<unsigned int>,
                       std::equal_to<unsigned int>,
                       space_allocator<std::pair<const unsigned int, PropInfo>>>
    logProp;

  public:
    /// Constructor for creation
    CBSBrancher(Home home, ViewArray<View>& x0);
    /// Constructor for cloning \a b
    CBSBrancher(Space& home, CBSBrancher& b);
    /// Post brancher
    static void post(Home home, ViewArray<View>& x);
    /// Copy brancher during cloning
    virtual Actor* copy(Space& home);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
    /// Check status of brancher, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space&, Archive& e);
    /// Commit choice \a c for alternative \a a
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int a);
    /// Print on \a o the alternative \a with view \a x at position \a i and value \a n
    virtual void print(const Space& home, const Choice& c, unsigned int a,
                       std::ostream& o) const;
  private:
    /// Returns wether a variable is in \a x or not
    bool inbrancher(unsigned int varId) const;
  };

}}}

#include <gecode/int/branch/cbs.hpp>

#endif

#endif

// STATISTICS: int-branch
