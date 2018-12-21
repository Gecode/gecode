/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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
  template<class _View, class _Val>
  class MeritBase {
  public:
    /// View type
    typedef _View View;
    /// Corresponding variable type
    typedef typename View::VarType Var;
    /// Type of merit
    typedef _Val Val;
    /// Constructor for initialization
    MeritBase(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritBase(Space& home, MeritBase& mb);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit class
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for user-defined merit function
   */
  template<class View>
  class MeritFunction : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  public:
    /// Corresponding merit function type
    typedef typename BranchTraits<Var>::Merit Function;
  protected:
    /// The user-defined merit function
    SharedData<Function> f;
  public:
    /// Constructor for initialization
    MeritFunction(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritFunction(Space& home, MeritFunction& mf);
    /// Return degree as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete view merit class
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for degree
   */
  template<class View>
  class MeritDegree : public MeritBase<View,unsigned int> {
    using typename MeritBase<View,unsigned int>::Var;
  public:
    /// Constructor for initialization
    MeritDegree(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritDegree(Space& home, MeritDegree& md);
    /// Return degree as merit for view \a x at position \a i
    unsigned int operator ()(const Space& home, View x, int i);
  };

  /**
   * \brief Merit class for AFC
   */
  template<class View>
  class MeritAFC : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// AFC information
    AFC afc;
  public:
    /// Constructor for initialization
    MeritAFC(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritAFC(Space& home, MeritAFC& ma);
    /// Return AFC as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };

  /**
   * \brief Merit class for action
   */
  template<class View>
  class MeritAction : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// Action information
    Action action;
  public:
    /// Constructor for initialization
    MeritAction(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritAction(Space& home, MeritAction& ma);
    /// Return action as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };
  //@}

  /**
   * \brief Merit class for CHB
   */
  template<class View>
  class MeritCHB : public MeritBase<View,double> {
    using typename MeritBase<View,double>::Var;
  protected:
    /// CHB information
    CHB chb;
  public:
    /// Constructor for initialization
    MeritCHB(Space& home, const VarBranch<Var>& vb);
    /// Constructor for cloning
    MeritCHB(Space& home, MeritCHB& ma);
    /// Return action as merit for view \a x at position \a i
    double operator ()(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Dispose view selection
    void dispose(Space& home);
  };
  //@}


  // Merit base class
  template<class View, class Val>
  forceinline
  MeritBase<View,Val>::MeritBase(Space&, const VarBranch<Var>&) {}
  template<class View, class Val>
  forceinline
  MeritBase<View,Val>::MeritBase(Space&, MeritBase&) {}
  template<class View, class Val>
  forceinline bool
  MeritBase<View,Val>::notice(void) const {
    return false;
  }
  template<class View, class Val>
  forceinline void
  MeritBase<View,Val>::dispose(Space&) {}

  // User-defined function merit
  template<class View>
  forceinline
  MeritFunction<View>::MeritFunction
    (Space& home, const VarBranch<MeritFunction<View>::Var>& vb)
    : MeritBase<View,double>(home,vb), f(vb.merit()) {
    if (!f())
      throw InvalidFunction("MeritFunction::MeritFunction");
  }
  template<class View>
  forceinline
  MeritFunction<View>::MeritFunction(Space& home, MeritFunction& mf)
    : MeritBase<View,double>(home,mf), f(mf.f) {
  }
  template<class View>
  forceinline double
  MeritFunction<View>::operator ()(const Space& home, View x, int i) {
    typename View::VarType y(x.varimp());
    GECODE_VALID_FUNCTION(f());
    return f()(home,y,i);
  }
  template<class View>
  forceinline bool
  MeritFunction<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  MeritFunction<View>::dispose(Space&) {
    f.~SharedData<Function>();
  }


  // Degree merit
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree
    (Space& home, const VarBranch<MeritDegree<View>::Var>& vb)
    : MeritBase<View,unsigned int>(home,vb) {}
  template<class View>
  forceinline
  MeritDegree<View>::MeritDegree(Space& home, MeritDegree& md)
    : MeritBase<View,unsigned int>(home,md) {}
  template<class View>
  forceinline unsigned int
  MeritDegree<View>::operator ()(const Space&, View x, int) {
    return x.degree();
  }

  // AFC merit
  template<class View>
  forceinline
  MeritAFC<View>::MeritAFC
    (Space& home, const VarBranch<MeritAFC<View>::Var>& vb)
    : MeritBase<View,double>(home,vb), afc(vb.afc()) {}
  template<class View>
  forceinline
  MeritAFC<View>::MeritAFC(Space& home, MeritAFC& ma)
    : MeritBase<View,double>(home,ma), afc(ma.afc) {}
  template<class View>
  forceinline double
  MeritAFC<View>::operator ()(const Space&, View x, int) {
    return x.afc();
  }
  template<class View>
  forceinline bool
  MeritAFC<View>::notice(void) const {
    // Given that AFC is just a fake, this not really necessary
    return false;
  }
  template<class View>
  forceinline void
  MeritAFC<View>::dispose(Space&) {
    // Given that AFC is just a fake, this not really necessary
    afc.~AFC();
  }


  // Action merit
  template<class View>
  forceinline
  MeritAction<View>::MeritAction
    (Space& home, const VarBranch<MeritAction<View>::Var>& vb)
    : MeritBase<View,double>(home,vb), action(vb.action()) {}
  template<class View>
  forceinline
  MeritAction<View>::MeritAction(Space& home, MeritAction& ma)
    : MeritBase<View,double>(home,ma), action(ma.action) {}
  template<class View>
  forceinline double
  MeritAction<View>::operator ()(const Space&, View, int i) {
    return action[i];
  }
  template<class View>
  forceinline bool
  MeritAction<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  MeritAction<View>::dispose(Space&) {
    action.~Action();
  }

  // CHB merit
  template<class View>
  forceinline
  MeritCHB<View>::MeritCHB
    (Space& home, const VarBranch<MeritCHB<View>::Var>& vb)
    : MeritBase<View,double>(home,vb), chb(vb.chb()) {}
  template<class View>
  forceinline
  MeritCHB<View>::MeritCHB(Space& home, MeritCHB& ma)
    : MeritBase<View,double>(home,ma), chb(ma.chb) {}
  template<class View>
  forceinline double
  MeritCHB<View>::operator ()(const Space&, View, int i) {
    return chb[i];
  }
  template<class View>
  forceinline bool
  MeritCHB<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  MeritCHB<View>::dispose(Space&) {
    chb.~CHB();
  }

}

// STATISTICS: kernel-branch
