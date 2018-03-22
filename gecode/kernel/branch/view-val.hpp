/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
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
   * \defgroup TaskBranchViewVal Generic brancher based on view and value selection
   *
   * Implements view-based brancher for an array of views and value.
   * \ingroup TaskActor
   */

  //@{
  /// %Choice storing position and value
  template<class Val>
  class GECODE_VTABLE_EXPORT PosValChoice : public PosChoice {
  private:
    /// Value to assign to
    const Val _val;
  public:
    /// Initialize choice for brancher \a b, number of alternatives \a a, position \a p, and value \a n
    PosValChoice(const Brancher& b, unsigned int a, const Pos& p, const Val& n);
    const Val& val(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };


  /// View-value no-good literal
  template<class View, class Val, PropCond pc>
  class ViewValNGL : public NGL {
  protected:
    /// The stored view
    View x;
    /// The stored value
    Val n;
  public:
    /// Initialize for propagator \a p with view \a x and value \a n
    ViewValNGL(Space& home, View x, Val n);
    /// Constructor for cloning \a ngl
    ViewValNGL(Space& home, ViewValNGL& ngl);
    /// Create subscription for no-good literal
    virtual void subscribe(Space& home, Propagator& p);
    /// Cancel subscription for no-good literal
    virtual void cancel(Space& home, Propagator& p);
    /// Schedule propagator \a p
    virtual void reschedule(Space& home, Propagator& p);
    /// Dispose
    virtual size_t dispose(Space& home);
  };

  /**
   * \brief Generic brancher by view and value selection
   *
   * Implements view-based branching for an array of views (of type
   * \a View) and value (of type \a Val).
   *
   */
  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  class ViewValBrancher : public ViewBrancher<View,Filter,n> {
  protected:
    using ViewBrancher<View,Filter,n>::vs;
    using ViewBrancher<View,Filter,n>::x;
    using ViewBrancher<View,Filter,n>::f;
    /// The corresponding variable
    typedef typename View::VarType Var;
    /// Value selection and commit object
    ValSelCommitBase<View,Val>* vsc;
    /// Print function
    Print p;
    /// Constructor for cloning \a b
    ViewValBrancher(Space& home, ViewValBrancher& b);
    /// Constructor for creation
    ViewValBrancher(Home home,
                    ViewArray<View>& x,
                    ViewSel<View>* vs[n],
                    ValSelCommitBase<View,Val>* vsc,
                    BranchFilter<Var> bf,
                    VarValPrint<Var,Val> vvp);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a b
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int b);
    /// Create no-good literal for choice \a c and alternative \a b
    virtual NGL* ngl(Space& home, const Choice& c, unsigned int b) const;
    /**
     * \brief Print branch for choice \a c and alternative \a b
     *
     * Prints an explanation of the alternative \a b of choice \a c
     * on the stream \a o.
     *
     */
    virtual void print(const Space& home, const Choice& c, unsigned int b,
                       std::ostream& o) const;
    /// Perform cloning
    virtual Actor* copy(Space& home);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
    /// Brancher post function
    static void post(Home home,
                     ViewArray<View>& x,
                     ViewSel<View>* vs[n],
                     ValSelCommitBase<View,Val>* vsc,
                     BranchFilter<Var> bf,
                     VarValPrint<Var,Val> vvp);
  };

  /// Post view value brancher
  template<class View, int n, class Val, unsigned int a>
  void
  postviewvalbrancher(Home home,
                      ViewArray<View>& x,
                      ViewSel<View>* vs[n],
                      ValSelCommitBase<View,Val>* vsc,
                      BranchFilter<typename View::VarType> bf,
                      VarValPrint<typename View::VarType,Val> vvp);
  //@}

  /*
   * %Choice with position and value
   *
   */
  template<class Val>
  forceinline
  PosValChoice<Val>::PosValChoice(const Brancher& b, unsigned int a,
                                  const Pos& p, const Val& n)
    : PosChoice(b,a,p), _val(n) {}

  template<class Val>
  forceinline const Val&
  PosValChoice<Val>::val(void) const {
    return _val;
  }

  template<class Val>
  forceinline void
  PosValChoice<Val>::archive(Archive& e) const {
    PosChoice::archive(e);
    e << _val;
  }


  /*
   * View-value no-good literal
   *
   */
  template<class View, class Val, PropCond pc>
  forceinline
  ViewValNGL<View,Val,pc>::ViewValNGL(Space& home, View x0, Val n0)
    : NGL(home), x(x0), n(n0) {}

  template<class View, class Val, PropCond pc>
  forceinline
  ViewValNGL<View,Val,pc>::ViewValNGL(Space& home, ViewValNGL& ngl)
    : NGL(home,ngl), n(ngl.n) {
    x.update(home,ngl.x);
  }

  template<class View, class Val, PropCond pc>
  void
  ViewValNGL<View,Val,pc>::subscribe(Space& home, Propagator& p) {
    x.subscribe(home,p,pc);
  }

  template<class View, class Val, PropCond pc>
  void
  ViewValNGL<View,Val,pc>::cancel(Space& home, Propagator& p) {
    x.cancel(home,p,pc);
  }

  template<class View, class Val, PropCond pc>
  void
  ViewValNGL<View,Val,pc>::reschedule(Space& home, Propagator& p) {
    x.reschedule(home,p,pc);
  }

  template<class View, class Val, PropCond pc>
  size_t
  ViewValNGL<View,Val,pc>::dispose(Space& home) {
    (void) NGL::dispose(home);
    return sizeof(*this);
  }



  /*
   * Generic brancher based on variable/value selection
   *
   */
  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline
  ViewValBrancher<View,n,Val,a,Filter,Print>::
  ViewValBrancher(Home home,
                  ViewArray<View>& x,
                  ViewSel<View>* vs[n],
                  ValSelCommitBase<View,Val>* vsc0,
                  BranchFilter<Var> bf,
                  VarValPrint<Var,Val> vvp)
    : ViewBrancher<View,Filter,n>(home,x,vs,bf), vsc(vsc0), p(vvp) {
    if (vsc->notice() || f.notice() || p.notice())
      home.notice(*this,AP_DISPOSE,true);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline void
  ViewValBrancher<View,n,Val,a,Filter,Print>::
  post(Home home, ViewArray<View>& x,
       ViewSel<View>* vs[n], ValSelCommitBase<View,Val>* vsc,
       BranchFilter<Var> bf,
       VarValPrint<Var,Val> vvp) {
    (void) new (home) ViewValBrancher<View,n,Val,a,Filter,Print>
      (home,x,vs,vsc,bf,vvp);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline
  ViewValBrancher<View,n,Val,a,Filter,Print>::
  ViewValBrancher(Space& home,
                  ViewValBrancher<View,n,Val,a,Filter,Print>& b)
    : ViewBrancher<View,Filter,n>(home,b),
      vsc(b.vsc->copy(home)), p(b.p) {}

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  Actor*
  ViewValBrancher<View,n,Val,a,Filter,Print>::copy(Space& home) {
    return new (home) ViewValBrancher<View,n,Val,a,Filter,Print>
      (home,*this);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  const Choice*
  ViewValBrancher<View,n,Val,a,Filter,Print>::choice(Space& home) {
    Pos p = ViewBrancher<View,Filter,n>::pos(home);
    View v = ViewBrancher<View,Filter,n>::view(p);
    return new PosValChoice<Val>(*this,a,p,vsc->val(home,v,p.pos));
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  const Choice*
  ViewValBrancher<View,n,Val,a,Filter,Print>::choice(const Space& home,
                                                     Archive& e) {
    (void) home;
    int p; e >> p;
    Val v; e >> v;
    return new PosValChoice<Val>(*this,a,p,v);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  ExecStatus
  ViewValBrancher<View,n,Val,a,Filter,Print>
  ::commit(Space& home, const Choice& c, unsigned int b) {
    const PosValChoice<Val>& pvc
      = static_cast<const PosValChoice<Val>&>(c);
    return me_failed(vsc->commit(home,b,
                                 ViewBrancher<View,Filter,n>::view(pvc.pos()),
                                 pvc.pos().pos,
                                 pvc.val()))
      ? ES_FAILED : ES_OK;
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  NGL*
  ViewValBrancher<View,n,Val,a,Filter,Print>
  ::ngl(Space& home, const Choice& c, unsigned int b) const {
    const PosValChoice<Val>& pvc
      = static_cast<const PosValChoice<Val>&>(c);
    return vsc->ngl(home,b,
                    ViewBrancher<View,Filter,n>::view(pvc.pos()),pvc.val());
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  void
  ViewValBrancher<View,n,Val,a,Filter,Print>
  ::print(const Space& home, const Choice& c, unsigned int b,
          std::ostream& o) const {
    const PosValChoice<Val>& pvc
      = static_cast<const PosValChoice<Val>&>(c);
    View xi = ViewBrancher<View,Filter,n>::view(pvc.pos());
    if (p)
      p(home,*this,b,xi,pvc.pos().pos,pvc.val(),o);
    else
      vsc->print(home,b,xi,pvc.pos().pos,pvc.val(),o);
  }

  template<class View, int n, class Val, unsigned int a,
           class Filter, class Print>
  forceinline size_t
  ViewValBrancher<View,n,Val,a,Filter,Print>::dispose(Space& home) {
    if (vsc->notice() || f.notice() || p.notice())
      home.ignore(*this,AP_DISPOSE,true);
    vsc->dispose(home);
    (void) ViewBrancher<View,Filter,n>::dispose(home);
    return sizeof(ViewValBrancher<View,n,Val,a,Filter,Print>);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline void
  postviewvalbrancher(Home home,
                      ViewArray<View>& x,
                      ViewSel<View>* vs[n],
                      ValSelCommitBase<View,Val>* vsc,
                      BranchFilter<typename View::VarType> bf,
                      VarValPrint<typename View::VarType,Val> vvp) {
    if (bf) {
      if (vvp) {
        ViewValBrancher<View,n,Val,a,
          BrancherFilter<View>,BrancherPrint<View,Val> >
          ::post(home,x,vs,vsc,bf,vvp);
      } else {
        ViewValBrancher<View,n,Val,a,
          BrancherFilter<View>,BrancherNoPrint<View,Val> >
          ::post(home,x,vs,vsc,bf,vvp);
        }
    } else {
      if (vvp)
        ViewValBrancher<View,n,Val,a,
          BrancherNoFilter<View>,BrancherPrint<View,Val> >
          ::post(home,x,vs,vsc,bf,vvp);
      else
        ViewValBrancher<View,n,Val,a,
          BrancherNoFilter<View>,BrancherNoPrint<View,Val> >
          ::post(home,x,vs,vsc,bf,vvp);
    }
  }      

}

// STATISTICS: kernel-branch
