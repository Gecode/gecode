/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#ifndef __GECODE_FLATZINC_BRANCH_HH__
#define __GECODE_FLATZINC_BRANCH_HH__

#include <gecode/int.hh>
#include <gecode/int/branch.hh>
#include <gecode/flatzinc.hh>

namespace Gecode { namespace FlatZinc {

  /// Which integer or Boolean variable to select for branching
  class IntBoolVarBranch : public VarBranch<IntVar> {
  public:
    /// Which variable selection
    enum Select {
      SEL_AFC_MAX,         ///< With largest accumulated failure count
      SEL_ACTION_MAX,      ///< With highest action
      SEL_CHB_MAX,         ///< With highest CHB Q-score
      SEL_AFC_SIZE_MAX,    ///< With largest accumulated failure count divided by domain size
      SEL_ACTION_SIZE_MAX, ///< With largest action divided by domain size
      SEL_CHB_SIZE_MAX     ///< With largest CHB Q-score divided by domain size
    };
  protected:
    /// Which variable to select
    Select s;
    /// Integer AFC
    IntAFC iafc;
    /// Boolean AFC
    BoolAFC bafc;
    /// Integer action
    IntAction iaction;
    /// Boolean action
    BoolAction baction;
    /// Integer CHB
    IntCHB ichb;
    /// Boolean CHB
    BoolCHB bchb;
  public:
    /// Initialize with selection strategy \a s and decay factor \a d
    IntBoolVarBranch(Select s, double d);
    /// Initialize with selection strategy \a s and AFC \a i and \a b
    IntBoolVarBranch(Select s, IntAFC i, BoolAFC b);
    /// Initialize with selection strategy \a s and action \a i and \a b
    IntBoolVarBranch(Select s, IntAction i, BoolAction b);
    /// Initialize with selection strategy \a s and CHB \a i and \a b
    IntBoolVarBranch(Select s, IntCHB i, BoolCHB b);
    /// Return selection strategy
    Select select(void) const;
    /// Return integer AFC
    IntAFC intafc(void) const;
    /// Return Boolean AFC
    BoolAFC boolafc(void) const;
    /// Return integer action
    IntAction intaction(void) const;
    /// Return Boolean action
    BoolAction boolaction(void) const;
    /// Return integer CHB
    IntCHB intchb(void) const;
    /// Return Boolean AFC
    BoolCHB boolchb(void) const;
    /// Expand AFC, action, and CHB
    void expand(Home home, const IntVarArgs& x, const BoolVarArgs& y);
  };

  /// Variable selection for both integer and Boolean variables
  //@{
  /// Select variable with largest accumulated failure count
  IntBoolVarBranch INTBOOL_VAR_AFC_MAX(double d=1.0);
  /// Select variable with largest accumulated failure count
  IntBoolVarBranch INTBOOL_VAR_AFC_MAX(IntAFC ia, BoolAFC ba);
  /// Select variable with highest action
  IntBoolVarBranch INTBOOL_VAR_ACTION_MAX(double d=1.0);
  /// Select variable with highest action
  IntBoolVarBranch INTBOOL_VAR_ACTION_MAX(IntAction ia, BoolAction ba);
  /// Select variable with largest CHB Q-score
  IntBoolVarBranch INTBOOL_VAR_CHB_MAX(double d=1.0);
  /// Select variable with largest CHB Q-score
  IntBoolVarBranch INTBOOL_VAR_CHB_MAX(IntCHB ic, BoolCHB bc);
  /// Select variable with largest accumulated failure count divided by domain size
  IntBoolVarBranch INTBOOL_VAR_AFC_SIZE_MAX(double d=1.0);
  /// Select variable with largest accumulated failure count divided by domain size
  IntBoolVarBranch INTBOOL_VAR_AFC_SIZE_MAX(IntAFC ia, BoolAFC ba);
  /// Select variable with largest action divided by domain size
  IntBoolVarBranch INTBOOL_VAR_ACTION_SIZE_MAX(double d=1.0);
  /// Select variable with largest action divided by domain size
  IntBoolVarBranch INTBOOL_VAR_ACTION_SIZE_MAX(IntAction ia, BoolAction ba);
  /// Select variable with largest CHB Q-score divided by domain size
  IntBoolVarBranch INTBOOL_VAR_CHB_SIZE_MAX(double d=1.0);
  /// Select variable with largest CHB Q-score divided by domain size
  IntBoolVarBranch INTBOOL_VAR_CHB_SIZE_MAX(IntCHB ic, BoolCHB bc);
  //@}
  
  /// Select by maximal AFC
  class MeritMaxAFC {
  protected:
    /// Integer AFC information
    IntAFC iafc;
    /// Boolean AFC information
    BoolAFC bafc;
  public:
    /// Constructor for initialization
    MeritMaxAFC(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAFC(Space& home, MeritMaxAFC& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// Select by maximal AFC over size
  class MeritMaxAFCSize {
  protected:
    /// Integer AFC information
    IntAFC iafc;
    /// Boolean AFC information
    BoolAFC bafc;
  public:
    /// Constructor for initialization
    MeritMaxAFCSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAFCSize(Space& home, MeritMaxAFCSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// Select by maximal Action
  class MeritMaxAction {
  protected:
    /// Integer Action information
    IntAction iaction;
    /// Boolean Action information
    BoolAction baction;
  public:
    /// Constructor for initialization
    MeritMaxAction(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxAction(Space& home, MeritMaxAction& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// Select by maximal Action over size
  class MeritMaxActionSize {
  protected:
    /// Integer Action information
    IntAction iaction;
    /// Boolean Action information
    BoolAction baction;
  public:
    /// Constructor for initialization
    MeritMaxActionSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxActionSize(Space& home, MeritMaxActionSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// Select by maximal CHB
  class MeritMaxCHB {
  protected:
    /// Integer CHB information
    IntCHB ichb;
    /// Boolean CHB information
    BoolCHB bchb;
  public:
    /// Constructor for initialization
    MeritMaxCHB(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxCHB(Space& home, MeritMaxCHB& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// Select by maximal CHB over size
  class MeritMaxCHBSize {
  protected:
    /// Integer CHB information
    IntCHB ichb;
    /// Boolean CHB information
    BoolCHB bchb;
  public:
    /// Constructor for initialization
    MeritMaxCHBSize(Space& home, const IntBoolVarBranch& ibvb);
    /// Constructor for cloning
    MeritMaxCHBSize(Space& home, MeritMaxCHBSize& m);
    /// Return merit
    double operator()(Int::IntView x, int i) const;
    /// Return merit
    double operator()(Int::BoolView x, int i) const;
    /// Dispose
    void dispose(void);
  };

  /// %Choice storing position and value
  class GECODE_VTABLE_EXPORT PosIntChoice : public Choice {
  private:
    /// Position of view to assign
    int _pos;
    /// Value to assign to
    int _val;
  public:
    /// Initialize choice for brancher \a b, number of alternatives \a a, position \a p, and value \a n
    PosIntChoice(const Brancher& b, unsigned int a, int p, int n);
    /// Return position of view to assign
    int pos(void) const;
    /// Return value to assign to
    int val(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

  /// Base-class for brancher for integer and Boolean views
  class IntBoolBrancherBase : public Brancher {
  protected:
    /// Integer views to branch on
    ViewArray<Int::IntView> x;
    /// Boolean views to branch on
    ViewArray<Int::BoolView> y;
    /// Unassigned views start here (might be in \a x or \a y)
    mutable int start;
    /// Integer value selection and commit object
    ValSelCommitBase<Int::IntView,int>* xvsc;
    /// Boolean value selection and commit object
    ValSelCommitBase<Int::BoolView,int>* yvsc;
    /// Constructor for cloning \a b
    IntBoolBrancherBase(Space& home, IntBoolBrancherBase& b);
    /// Constructor for creation
    IntBoolBrancherBase(Home home,
                        ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                        ValSelCommitBase<Int::IntView,int>* xvsc,
                        ValSelCommitBase<Int::BoolView,int>* yvsc);
  public:
    /// Check status of brancher, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Return choice
    virtual const Choice* choice(Space& home) = 0;
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a b
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int b);
    /// Create no-good literal for choice \a c and alternative \a b
    virtual NGL* ngl(Space& home, const Choice& c, unsigned int b) const;
    /// Print branch for choice \a c and alternative \a b
    virtual void print(const Space& home, const Choice& c, unsigned int b,
                       std::ostream& o) const;
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
  };

  /// Brancher for integer and Boolean views
  template<class Merit>
  class IntBoolBrancher : public IntBoolBrancherBase {
  protected:
    /// Selection by maximal merit
    Merit merit;
    /// Constructor for cloning \a b
    IntBoolBrancher(Space& home, IntBoolBrancher& b);
    /// Constructor for creation
    IntBoolBrancher(Home home,
                    ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                    Merit& m,
                    ValSelCommitBase<Int::IntView,int>* xvsc,
                    ValSelCommitBase<Int::BoolView,int>* yvsc);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Perform cloning
    virtual Actor* copy(Space& home);
    /// Post brancher
    static void post(Home home,
                     ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y,
                     Merit& m,
                     ValSelCommitBase<Int::IntView,int>* xvsc,
                     ValSelCommitBase<Int::BoolView,int>* yvsc);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
  };

  /// Map respective integer value selection to Boolean value selection
  BoolValBranch i2b(const IntValBranch& ivb);

  /// Branch function for integer and Boolean variables
  GECODE_FLATZINC_EXPORT void
  branch(Home home, const IntVarArgs& x, const BoolVarArgs& y,
         IntBoolVarBranch vars, IntValBranch vals);

}}

#include <gecode/flatzinc/branch.hpp>

#endif

// STATISTICS: flatzinc-branch
