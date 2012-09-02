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
  class MeritBase {
  public:
    /// Default constructor
    MeritBase(void);
    /// Constructor for initialization
    MeritBase(Space& home, const VarBranchOptions& vbo);
    /// Updating during cloning
    void update(Space& home, bool share, MeritBase& mb);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit class
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for degree
   */
  template<class View>
  class MeritDegree : public MeritBase {
  public:
    /// Default constructor
    MeritDegree(void);
    /// Constructor for initialization
    MeritDegree(Space& home, const VarBranchOptions& vbo);
    /// Return degree as merit for view \a x at position \a i
    double operator ()(Space& home, View x, int i);
  };

  /**
   * \brief Merit class for AFC
   */
  template<class View>
  class MeritAfc : public MeritBase {
  public:
    /// Default constructor
    MeritAfc(void);
    /// Constructor for initialization
    MeritAfc(Space& home, const VarBranchOptions& vbo);
    /// Return AFC as merit for view \a x at position \a i
    double operator ()(Space& home, View x, int i);
  };

  /**
   * \brief Merit class for activity
   */
  template<class View>
  class MeritActivity : public MeritBase {
  protected:
    /// Activity information
    Activity activity;
  public:
    /// Default constructor
    MeritActivity(void);
    /// Constructor for initialization
    MeritActivity(Space& home, const VarBranchOptions& vbo);
    /// Return activity as merit for view \a x at position \a i
    double operator ()(Space& home, View x, int i);
    /// Updating during cloning
    void update(Space& home, bool share, MeritActivity& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };
  //@}

  // Merit base class
  forceinline
  MeritBase::MeritBase(void) {}
  forceinline
  MeritBase::MeritBase(Space&, const VarBranchOptions&) {}
  forceinline void
  MeritBase::update(Space&, bool, MeritBase&) {}
  forceinline bool
  MeritBase::notice(void) const {
    return false;
  }
  forceinline void
  MeritBase::dispose(Space& home) {
  }

  // Degree merit
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree(void) {}
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree(Space& home,
                                 const VarBranchOptions& vbo)
    : MeritBase(home,vbo) {}
  template<class View>
  forceinline double
  MeritDegree<View>::operator ()(Space&, View x, int) {
    return static_cast<double>(x.degree());
  }

  // AFC merit
  template<class View>
  forceinline
  MeritAfc<View>::MeritAfc(void) {}
  template<class View>
  forceinline
  MeritAfc<View>::MeritAfc(Space& home,
                           const VarBranchOptions& vbo)
    : MeritBase(home,vbo) {}
  template<class View>
  forceinline double
  MeritAfc<View>::operator ()(Space&, View x, int) {
    return x.afc();
  }


  // Acitivity merit
  template<class View>
  forceinline
  MeritActivity<View>::MeritActivity(void) {}
  template<class View>
  forceinline
  MeritActivity<View>::MeritActivity(Space& home,
                                     const VarBranchOptions& vbo)
    : MeritBase(home,vbo), activity(vbo.activity) {
    if (!activity.initialized())
      throw MissingActivity("MeritActivity (VAR_ACTIVITY_MIN)");
  }
  template<class View>
  forceinline double
  MeritActivity<View>::operator ()(Space&, View, int i) {
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
