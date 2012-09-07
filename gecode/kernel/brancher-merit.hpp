/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

  /**
   * \defgroup TaskBranchMerit Generic merit for branchers based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /**
   * \brief Base-class for merit class
   */
  template<class _View>
  class MeritBase {
  public:
    /// View type
    typedef _View View;
    /// Default constructor
    MeritBase(void);
    /// Constructor for initialization
    MeritBase(Space& home, const VarBranch& vb);
    /// Updating during cloning
    void update(Space& home, bool share, MeritBase& mb);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit class
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for user-defined merit function
   */
  template<class View>
  class MeritFunction : public MeritBase<View> {
  public:
    /// Corresponding variable type
    typedef typename View::VarType Var;
    /// Corresponding merit function type
    typedef typename BranchTraits<Var>::Merit Function;
  protected:
    /// The user-defined merit function
    Function f;
  public:
    /// Default constructor
    MeritFunction(void);
    /// Constructor for initialization
    MeritFunction(Space& home, const VarBranch& vb);
    /// Return degree as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritFunction& mf);
  };

  /**
   * \brief Merit class for degree
   */
  template<class View>
  class MeritDegree : public MeritBase<View> {
  public:
    /// Default constructor
    MeritDegree(void);
    /// Constructor for initialization
    MeritDegree(Space& home, const VarBranch& vb);
    /// Return degree as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for AFC
   */
  template<class View>
  class MeritAfc : public MeritBase<View> {
  public:
    /// Default constructor
    MeritAfc(void);
    /// Constructor for initialization
    MeritAfc(Space& home, const VarBranch& vb);
    /// Return AFC as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for activity
   */
  template<class View>
  class MeritActivity : public MeritBase<View> {
  protected:
    /// Activity information
    Activity activity;
  public:
    /// Default constructor
    MeritActivity(void);
    /// Constructor for initialization
    MeritActivity(Space& home, const VarBranch& vb);
    /// Return activity as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritActivity& ma);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };
  //@}

  // Merit base class
  template<class View>
  forceinline
  MeritBase<View>::MeritBase(void) {}
  template<class View>
  forceinline
  MeritBase<View>::MeritBase(Space&, const VarBranch&) {}
  template<class View>
  forceinline void
  MeritBase<View>::update(Space&, bool, MeritBase&) {}
  template<class View>
  forceinline bool
  MeritBase<View>::notice(void) const {
    return false;
  }
  template<class View>
  forceinline void
  MeritBase<View>::dispose(Space& home) {
  }

  // User-defined function merit
  template<class View>
  forceinline
  MeritFunction<View>::MeritFunction(void) {}
  template<class View>
  forceinline
  MeritFunction<View>::MeritFunction(Space& home, const VarBranch& vb)
    : MeritBase<View>(home,vb),
      f(static_cast<Function>(vb.merit())) {}
  template<class View>
  forceinline double
  MeritFunction<View>::operator ()(const Space& home, View x, int i) {
    return f(home,x,i);
  }
  template<class View>
  forceinline void
  MeritFunction<View>::update(Space&, bool, MeritFunction& mf) {
    f = mf.f;
  }

  // Degree merit
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree(void) {}
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree(Space& home, const VarBranch& vb)
    : MeritBase<View>(home,vb) {}
  template<class View>
  forceinline double
  MeritDegree<View>::operator ()(const Space&, View x, int) {
    return static_cast<double>(x.degree());
  }

  // AFC merit
  template<class View>
  forceinline
  MeritAfc<View>::MeritAfc(void) {}
  template<class View>
  forceinline
  MeritAfc<View>::MeritAfc(Space& home, const VarBranch& vb)
    : MeritBase<View>(home,vb) {}
  template<class View>
  forceinline double
  MeritAfc<View>::operator ()(const Space&, View x, int) {
    return x.afc();
  }


  // Acitivity merit
  template<class View>
  forceinline
  MeritActivity<View>::MeritActivity(void) {}
  template<class View>
  forceinline
  MeritActivity<View>::MeritActivity(Space& home, const VarBranch& vb)
    : MeritBase<View>(home,vb), activity(vb.activity()) {}
  template<class View>
  forceinline double
  MeritActivity<View>::operator ()(const Space&, View, int i) {
    return activity[i];
  }
  template<class View>
  forceinline void
  MeritActivity<View>::update(Space& home, bool share, 
                              MeritActivity<View>& ma) {
    activity.update(home, share, ma.activity);
  }
  template<class View>
  forceinline bool
  MeritActivity<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  MeritActivity<View>::dispose(Space&) {
    activity.~Activity();
  }

}

// STATISTICS: kernel-branch
