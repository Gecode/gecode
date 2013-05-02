/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
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
    /// Return value to branch with
    const Val& val(void) const;
    /// Report size occupied
    virtual size_t size(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

  /**
   * \brief Generic brancher by view and value selection
   *
   * Implements view-based branching for an array of views (of type
   * \a View) and value (of type \a Val).
   *
   */
  template<class View, int n, class Val, unsigned int a>
  class ViewValBrancher : public ViewBrancher<View,n> {
    typedef typename ViewBrancher<View,n>::BranchFilter BranchFilter;
  protected:
    using ViewBrancher<View,n>::vs;
    using ViewBrancher<View,n>::x;
    /// Value selection and commit object
    ValSelCommitBase<View,Val>* vsc;
    /// Function type for print variable and value selection
    typedef void (*VarValPrint)(const Space& home, unsigned int a,
                                typename View::VarType x, int i,
                                const Val& n,
                                std::ostream& o);
    /// Print function
    VarValPrint vvp;
    /// Constructor for cloning \a b
    ViewValBrancher(Space& home, bool share, ViewValBrancher& b);
    /// Constructor for creation
    ViewValBrancher(Home home, 
                    ViewArray<View>& x,
                    ViewSel<View>* vs[n], 
                    ValSelCommitBase<View,Val>* vsc,
                    BranchFilter bf,
                    VarValPrint vvp);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a b
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int b);
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
    virtual Actor* copy(Space& home, bool share);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
    /// Brancher post function
    static BrancherHandle post(Home home, 
                               ViewArray<View>& x,
                               ViewSel<View>* vs[n], 
                               ValSelCommitBase<View,Val>* vsc, 
                               BranchFilter bf,
                               VarValPrint vvp);
  };
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
  forceinline size_t
  PosValChoice<Val>::size(void) const {
    return sizeof(PosValChoice<Val>);
  }

  template<class Val>
  forceinline void
  PosValChoice<Val>::archive(Archive& e) const {
    PosChoice::archive(e);
    e << _val;
  }

  /*
   * Generic brancher based on variable/value selection
   *
   */



  template<class View, int n, class Val, unsigned int a>
  forceinline
  ViewValBrancher<View,n,Val,a>::
  ViewValBrancher(Home home, 
                  ViewArray<View>& x,
                  ViewSel<View>* vs[n], 
                  ValSelCommitBase<View,Val>* vsc0,
                  BranchFilter bf,
                  VarValPrint vvp0)
    : ViewBrancher<View,n>(home,x,vs,bf), vsc(vsc0), vvp(vvp0) {
    if (vsc->notice()) {
      for (int i=0; i<n; i++)
        if (vs[i]->notice())
          goto not_needed;
      home.notice(*this,AP_DISPOSE);      
    not_needed: ;
    }
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline BrancherHandle
  ViewValBrancher<View,n,Val,a>::
  post(Home home, ViewArray<View>& x,
       ViewSel<View>* vs[n], ValSelCommitBase<View,Val>* vsc,
       BranchFilter bf,
       VarValPrint vvp) {
    return *new (home) ViewValBrancher<View,n,Val,a>(home,x,vs,vsc,bf,vvp);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline
  ViewValBrancher<View,n,Val,a>::
  ViewValBrancher(Space& home, bool shared, ViewValBrancher<View,n,Val,a>& b)
    : ViewBrancher<View,n>(home,shared,b), 
      vsc(b.vsc->copy(home,shared)), vvp(b.vvp) {}
  
  template<class View, int n, class Val, unsigned int a>
  Actor*
  ViewValBrancher<View,n,Val,a>::copy(Space& home, bool shared) {
    return new (home) ViewValBrancher<View,n,Val,a>(home,shared,*this);
  }

  template<class View, int n, class Val, unsigned int a>
  const Choice*
  ViewValBrancher<View,n,Val,a>::choice(Space& home) {
    Pos p = ViewBrancher<View,n>::pos(home);
    View v = ViewBrancher<View,n>::view(p);
    return new PosValChoice<Val>(*this,a,p,vsc->val(home,v,p.pos));
  }

  template<class View, int n, class Val, unsigned int a>
  const Choice*
  ViewValBrancher<View,n,Val,a>::choice(const Space& home, Archive& e) {
    (void) home;
    int p; e >> p;
    Val v; e >> v;
    return new PosValChoice<Val>(*this,a,p,v);
  }

  template<class View, int n, class Val, unsigned int a>
  ExecStatus
  ViewValBrancher<View,n,Val,a>
  ::commit(Space& home, const Choice& c, unsigned int b) {
    const PosValChoice<Val>& pvc
      = static_cast<const PosValChoice<Val>&>(c);
    return me_failed(vsc->commit(home,b,
                                 ViewBrancher<View,n>::view(pvc.pos()),
                                 pvc.pos().pos,
                                 pvc.val())) 
      ? ES_FAILED : ES_OK;
  }

  template<class View, int n, class Val, unsigned int a>
  void
  ViewValBrancher<View,n,Val,a>
  ::print(const Space& home, const Choice& c, unsigned int b,
          std::ostream& o) const {
    const PosValChoice<Val>& pvc
      = static_cast<const PosValChoice<Val>&>(c);
    View xi = ViewBrancher<View,n>::view(pvc.pos());
    typename View::VarType y(ViewBrancher<View,n>::view(pvc.pos()).varimp());
    if (vvp != NULL)
      vvp(home,b,y,pvc.pos().pos,pvc.val(),o);
    else
      vsc->print(home,b,xi,pvc.pos().pos,pvc.val(),o);
  }

  template<class View, int n, class Val, unsigned int a>
  forceinline size_t
  ViewValBrancher<View,n,Val,a>::dispose(Space& home) {
    if (vsc->notice()) {
      for (int i=0; i<n; i++)
        if (vs[i]->notice())
          goto not_needed;
      home.ignore(*this,AP_DISPOSE);
    not_needed: ;
    }
    vsc->dispose(home);
    (void) ViewBrancher<View,n>::dispose(home);
    return sizeof(ViewValBrancher<View,n,Val,a>);
  }

}

// STATISTICS: kernel-branch
