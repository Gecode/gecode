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

namespace Gecode {

  /**
   * \defgroup TaskBranchViewVal Generic brancher based on view and value selection
   *
   * Implements view-based brancher for an array of views and value.
   * \ingroup TaskActor
   */

  //@{
  /// Status returned by member functions of view selection class
  enum ViewSelStatus {
    VSS_BEST,   ///< Current view is a best one
    VSS_BETTER, ///< Current view is better
    VSS_TIE,    ///< Current view is as good as
    VSS_WORSE   ///< Current view is worse
  };

  /// Position information
  class Pos {
  public:
    /// Position of view
    const int pos;
    /// Create position information
    Pos(int p);
  };

  /**
   * \brief Generic brancher by view selection
   *
   * Implements a baseclass for view-based branching for an array of views
   * (of type \a ViewSel::View). The behaviour is
   * defined by the class \a ViewSel (which view is selected for branching).
   *
   */
  template<class ViewSel>
  class ViewBrancher : public Brancher {
  protected:
    /// Views to branch on
    ViewArray<typename ViewSel::View> x;
    /// Unassigned views start at x[start]
    mutable int start;
    /// View selection object
    ViewSel viewsel;
    /// Branch filter function
    BranchFilter bf;
    /// Return position information
    Pos pos(Space& home);
    /// Return view according to position information \a p
    typename ViewSel::View view(const Pos& p) const;
    /// Constructor for cloning \a b
    ViewBrancher(Space& home, bool share, ViewBrancher& b);
    /// Constructor for creation
    ViewBrancher(Home home, ViewArray<typename ViewSel::View>& x,
                 ViewSel& vi_s, BranchFilter bf0=NULL);
  public:
    /// Check status of brancher, return true if alternatives left
    virtual bool status(const Space& home) const;
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
  };


  /**
   * \brief Generic brancher by view and value selection
   *
   * Implements view-based branching for an array of views (of type
   * \a ViewSel::View) and value (of type \a ValSel::Val). The behaviour is
   * defined by the class \a ViewSel (which view is selected for branching)
   * and the class \a ValSel (which value is selected for branching).
   *
   */
  template<class ViewSel, class ValSel>
  class ViewValBrancher : public ViewBrancher<ViewSel> {
  protected:
    using ViewBrancher<ViewSel>::viewsel;
    using ViewBrancher<ViewSel>::x;
    /// Value selection object
    ValSel valsel;
    /// Constructor for cloning \a b
    ViewValBrancher(Space& home, bool share, ViewValBrancher& b);
    /// Constructor for creation
    ViewValBrancher(Home home, ViewArray<typename ViewSel::View>& x,
                    ViewSel& vi_s, ValSel& va_s, BranchFilter bf0);
  public:
    /// Return choice
    virtual const Choice* choice(Space& home);
    /// Return choice
    virtual const Choice* choice(const Space& home, Archive& e);
    /// Perform commit for choice \a c and alternative \a a
    virtual ExecStatus commit(Space& home, const Choice& c, unsigned int a);
    /// Perform cloning
    virtual Actor* copy(Space& home, bool share);
    /// Delete brancher and return its size
    virtual size_t dispose(Space& home);
    /// Brancher post function
    static void post(Home home, ViewArray<typename ViewSel::View>& x,
                     ViewSel& vi_s, ValSel& va_s, BranchFilter bf=NULL);

  };


  /// %Choices storing position
  template<class ViewSel>
  class GECODE_VTABLE_EXPORT PosChoice : public Choice {
  private:
    /// Position information
    const Pos _pos;
    /// View selection choice to be stored
    const typename ViewSel::Choice _viewchoice;
  public:
    /// Initialize choice for brancher \a b, number of alternatives \a a, position \a p, view selection choice \a viewc
    PosChoice(const Brancher& b, unsigned int a, const Pos& p,
              const typename ViewSel::Choice& viewc);
    /// Return position in array
    const Pos& pos(void) const;
    /// Return view selection choice
    const typename ViewSel::Choice& viewchoice(void) const;
    /// Report size occupied
    virtual size_t size(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };

  /// %Choice storing position and value
  template<class ViewSel, class ValSel>
  class GECODE_VTABLE_EXPORT PosValChoice : public PosChoice<ViewSel> {
  private:
    /// Value choice to be stored
    const typename ValSel::Choice _valchoice;
    /// Value to assign to
    const typename ValSel::Val _val;
  public:
    /// Initialize choice for brancher \a b, position \a p, view choice \a viewc, value choice \a valc, and value \a n
    PosValChoice(const Brancher& b, const Pos& p,
                 const typename ViewSel::Choice& viewc,
                 const typename ValSel::Choice& valc,
                 const typename ValSel::Val& n);
    /// Return stored choice
    const typename ValSel::Choice& valchoice(void) const;
    /// Return value to branch with
    const typename ValSel::Val& val(void) const;
    /// Report size occupied
    virtual size_t size(void) const;
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };
  //@}




  /*
   * Position information
   *
   */
  forceinline
  Pos::Pos(int p) : pos(p) {}


  /*
   * Choice with position
   *
   */
  template<class ViewSel>
  forceinline
  PosChoice<ViewSel>::PosChoice(const Brancher& b, unsigned int a, 
                                const Pos& p,
                                const typename ViewSel::Choice& viewc)
    : Choice(b,a), _pos(p), _viewchoice(viewc) {}
  template<class ViewSel>
  forceinline const Pos&
  PosChoice<ViewSel>::pos(void) const {
    return _pos;
  }
  template<class ViewSel>
  forceinline const typename ViewSel::Choice&
  PosChoice<ViewSel>::viewchoice(void) const {
    return _viewchoice;
  }
  template<class ViewSel>
  forceinline size_t
  PosChoice<ViewSel>::size(void) const {
    return sizeof(PosChoice<ViewSel>) + _viewchoice.size();
  }
  template<class ViewSel>
  forceinline void
  PosChoice<ViewSel>::archive(Archive& e) const {
    Choice::archive(e);
    e << _pos.pos;
    _viewchoice.archive(e);
  }

  /*
   * %Choice with position and value
   *
   */
  template<class ViewSel, class ValSel>
  forceinline
  PosValChoice<ViewSel,ValSel>
  ::PosValChoice(const Brancher& b, const Pos& p,
                 const typename ViewSel::Choice& viewc,
                 const typename ValSel::Choice& valc,
                 const typename ValSel::Val& n)
    : PosChoice<ViewSel>(b,ValSel::alternatives,p,viewc),
      _valchoice(valc), _val(n) {}

  template<class ViewSel, class ValSel>
  forceinline const typename ValSel::Choice&
  PosValChoice<ViewSel,ValSel>::valchoice(void) const {
    return _valchoice;
  }

  template<class ViewSel, class ValSel>
  forceinline const typename ValSel::Val&
  PosValChoice<ViewSel,ValSel>::val(void) const {
    return _val;
  }

  template<class ViewSel, class ValSel>
  forceinline size_t
  PosValChoice<ViewSel, ValSel>::size(void) const {
    return sizeof(PosValChoice<ViewSel,ValSel>) + _valchoice.size();
  }

  template<class ViewSel, class ValSel>
  forceinline void
  PosValChoice<ViewSel, ValSel>::archive(Archive& e) const {
    PosChoice<ViewSel>::archive(e);
    _valchoice.archive(e);
    e << _val;
  }

  /*
   * Generic brancher based on view selection
   *
   */

  template<class ViewSel>
  forceinline
  ViewBrancher<ViewSel>::ViewBrancher(Home home,
                                      ViewArray<typename ViewSel::View>& x0,
                                      ViewSel& vi_s, BranchFilter bf0)
    : Brancher(home), x(x0), start(0), viewsel(vi_s), bf(bf0) {}


  template<class ViewSel>
  forceinline
  ViewBrancher<ViewSel>::ViewBrancher(Space& home, bool share,
                                      ViewBrancher& b)
    : Brancher(home,share,b), start(b.start), bf(b.bf) {
    x.update(home,share,b.x);
    viewsel.update(home,share,b.viewsel);
  }

  template<class ViewSel>
  bool
  ViewBrancher<ViewSel>::status(const Space& home) const {
    if (bf == NULL) {
      for (int i=start; i < x.size(); i++)
        if (!x[i].assigned()) {
          start = i;
          return true;
        }
    } else {
      for (int i=start; i < x.size(); i++) {
        typename ViewSel::View::VarType y(x[i].varimp());
        if (!x[i].assigned() && bf(home,i,y)) {
          start = i;
          return true;
        }
      }
    }
    return false;
  }

  template<class ViewSel>
  forceinline Pos
  ViewBrancher<ViewSel>::pos(Space& home) {
    assert(!x[start].assigned());
    int i = start;
    int b = i++;
    if (viewsel.init(home,x[b]) != VSS_BEST)
      for (; i < x.size(); i++)
        if (!x[i].assigned())
          switch (viewsel.select(home,x[i])) {
          case VSS_BETTER:              b=i; break;
          case VSS_BEST:                b=i; goto create;
          case VSS_TIE: case VSS_WORSE: break;
          default:                      GECODE_NEVER;
          }
  create:
    Pos p(b);
    return p;
  }

  template<class ViewSel>
  forceinline typename ViewSel::View
  ViewBrancher<ViewSel>::view(const Pos& p) const {
    return x[p.pos];
  }

  template<class ViewSel>
  forceinline size_t
  ViewBrancher<ViewSel>::dispose(Space& home) {
    viewsel.dispose(home);
    (void) Brancher::dispose(home);
    return sizeof(ViewBrancher<ViewSel>);
  }



  /*
   * Generic brancher based on variable/value selection
   *
   */

  template<class ViewSel, class ValSel>
  forceinline
  ViewValBrancher<ViewSel,ValSel>::
  ViewValBrancher(Home home, ViewArray<typename ViewSel::View>& x,
                  ViewSel& vi_s, ValSel& va_s, BranchFilter bf)
    : ViewBrancher<ViewSel>(home,x,vi_s,bf), valsel(va_s) {}

  template<class ViewSel, class ValSel>
  void
  ViewValBrancher<ViewSel,ValSel>::
  post(Home home, ViewArray<typename ViewSel::View>& x,
       ViewSel& vi_s, ValSel& va_s, BranchFilter bf) {
    (void) new (home) ViewValBrancher<ViewSel,ValSel>(home,x,vi_s,va_s,bf);
  }

  template<class ViewSel, class ValSel>
  forceinline
  ViewValBrancher<ViewSel,ValSel>::
  ViewValBrancher(Space& home, bool share, ViewValBrancher& b)
    : ViewBrancher<ViewSel>(home,share,b) {
    valsel.update(home,share,b.valsel);
  }

  template<class ViewSel, class ValSel>
  Actor*
  ViewValBrancher<ViewSel,ValSel>::copy(Space& home, bool share) {
    return new (home)
      ViewValBrancher<ViewSel,ValSel>(home,share,*this);
  }

  template<class ViewSel, class ValSel>
  const Choice*
  ViewValBrancher<ViewSel,ValSel>::choice(Space& home) {
    Pos p = ViewBrancher<ViewSel>::pos(home);
    typename ValSel::View v(ViewBrancher<ViewSel>::view(p).varimp());
    typename ValSel::Val val(valsel.val(home,v));
    return new PosValChoice<ViewSel,ValSel>
      (*this,p,
       viewsel.choice(home),valsel.choice(home),val);
  }

  template<class ViewSel, class ValSel>
  const Choice*
  ViewValBrancher<ViewSel,ValSel>::choice(const Space& home, Archive& e) {
    int p; e >> p;
    typename ViewSel::Choice viewsc = viewsel.choice(home,e);
    typename ValSel::Choice valsc = valsel.choice(home,e);
    typename ValSel::Val val; e >> val;
    return new PosValChoice<ViewSel,ValSel>(*this,p,viewsc,valsc,val);
  }

  template<class ViewSel, class ValSel>
  ExecStatus
  ViewValBrancher<ViewSel,ValSel>
  ::commit(Space& home, const Choice& c, unsigned int a) {
    const PosValChoice<ViewSel,ValSel>& pvc
      = static_cast<const PosValChoice<ViewSel,ValSel>&>(c);
    typename ValSel::View
      v(ViewBrancher<ViewSel>::view(pvc.pos()).varimp());
    viewsel.commit(home, pvc.viewchoice(), a);
    valsel.commit(home, pvc.valchoice(), a);
    return me_failed(valsel.tell(home,a,v,pvc.val())) ? ES_FAILED : ES_OK;
  }

  template<class ViewSel, class ValSel>
  forceinline size_t
  ViewValBrancher<ViewSel,ValSel>::dispose(Space& home) {
    valsel.dispose(home);
    (void) ViewBrancher<ViewSel>::dispose(home);
    return sizeof(ViewValBrancher<ViewSel,ValSel>);
  }

}

// STATISTICS: kernel-branch
