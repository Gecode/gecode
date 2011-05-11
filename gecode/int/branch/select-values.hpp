/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

namespace Gecode { namespace Int { namespace Branch {

  /**
   * \brief %Choice storing position and values
   *
   * The maximal number of alternatives is defined by \a alt.
   */
  template<class ViewSel, class View>
  class PosValuesChoice : public PosChoice<ViewSel> {
  private:
    /// Information about position and minimum
    class PosMin {
    public:
      /// Start position of range
      unsigned int pos;
      /// Minmum of range
      int min;
    };
    /// Number of ranges
    unsigned int n;
    /// Values to assign to
    PosMin* pm;
  public:
    /// Initialize choice for brancher \a b, position \a p, view choice \a viewc, and view \a x
    PosValuesChoice(const Brancher& b, const Pos& p,
                    const typename ViewSel::Choice& viewc, View x);
    /// Initialize choice for brancher \a b from \a e
    PosValuesChoice(const Brancher& b, unsigned int alt, Pos p,
                    const typename ViewSel::Choice& viewc,
                    Archive& e);
    /// Return value to branch with for alternative \a a
    int val(unsigned int a) const;
    /// Report size occupied
    virtual size_t size(void) const;
    /// Deallocate
    virtual ~PosValuesChoice(void);
    /// Archive into \a e
    virtual void archive(Archive& e) const;
  };


  template<class ViewSel, class View>
  forceinline
  PosValuesChoice<ViewSel,View>::
  PosValuesChoice(const Brancher& b, const Pos& p,
                const typename ViewSel::Choice& viewc, View x)
    : PosChoice<ViewSel>(b,x.size(),p,viewc), n(0) {
    for (ViewRanges<View> r(x); r(); ++r)
      n++;
    pm = heap.alloc<PosMin>(n+1);
    unsigned int w=0;
    int i=0;
    for (ViewRanges<View> r(x); r(); ++r) {
      pm[i].min = r.min();
      pm[i].pos = w;
      w += r.width(); i++;
    }
    pm[i].pos = w;
  }

  template<class ViewSel, class View>
  forceinline
  PosValuesChoice<ViewSel,View>::
  PosValuesChoice(const Brancher& b, unsigned int alt, Pos p,
                  const typename ViewSel::Choice& viewc, Archive& e)
    : PosChoice<ViewSel>(b,alt,p,viewc) {
    e >> n;
    pm = heap.alloc<PosMin>(n+1);
    for (unsigned int i=0; i<n+1; i++) {
      e >> pm[i].pos;
      e >> pm[i].min;
    }
  }

  template<class ViewSel, class View>
  forceinline int
  PosValuesChoice<ViewSel,View>::val(unsigned int a) const {
    PosMin* l = &pm[0];
    PosMin* r = &pm[n-1];
    while (true) {
      PosMin* m = l + (r-l)/2;
      if (a < m->pos) {
        r=m-1;
      } else if (a >= (m+1)->pos) {
        l=m+1;
      } else {
        return m->min + static_cast<int>(a - m->pos);
      }
    }
    GECODE_NEVER;
    return 0;
  }

  template<class ViewSel, class View>
  size_t
  PosValuesChoice<ViewSel,View>::size(void) const {
    return sizeof(PosValuesChoice<ViewSel,View>)+(n+1)*sizeof(PosMin);
  }

  template<class ViewSel, class View>
  PosValuesChoice<ViewSel,View>::~PosValuesChoice(void) {
    heap.free<PosMin>(pm,n+1);
  }

  template<class ViewSel, class View>
  forceinline void
  PosValuesChoice<ViewSel,View>::archive(Archive& e) const {
    PosChoice<ViewSel>::archive(e);
    e << this->alternatives() << n;
    for (unsigned int i=0; i<n+1; i++) {
      e << pm[i].pos;
      e << pm[i].min;
    }
  }

  template<class ViewSel, class View>
  forceinline
  ViewValuesBrancher<ViewSel,View>::
  ViewValuesBrancher(Home home, ViewArray<typename ViewSel::View>& x,
                     ViewSel& vi_s, BranchFilter bf)
    : ViewBrancher<ViewSel>(home,x,vi_s,bf) {}

  template<class ViewSel, class View>
  void
  ViewValuesBrancher<ViewSel,View>::
  post(Home home, ViewArray<typename ViewSel::View>& x, ViewSel& vi_s,
       BranchFilter bf) {
    (void) new (home) ViewValuesBrancher<ViewSel,View>(home,x,vi_s,bf);
  }

  template<class ViewSel, class View>
  forceinline
  ViewValuesBrancher<ViewSel,View>::
  ViewValuesBrancher(Space& home, bool share, ViewValuesBrancher& b)
    : ViewBrancher<ViewSel>(home,share,b) {}

  template<class ViewSel, class View>
  Actor*
  ViewValuesBrancher<ViewSel,View>::copy(Space& home, bool share) {
    return new (home)
      ViewValuesBrancher<ViewSel,View>(home,share,*this);
  }

  template<class ViewSel, class View>
  const Choice*
  ViewValuesBrancher<ViewSel,View>::choice(Space& home) {
    Pos p = ViewBrancher<ViewSel>::pos(home);
    View v(ViewBrancher<ViewSel>::view(p).varimp());
    return new PosValuesChoice<ViewSel,View>
      (*this,p,viewsel.choice(home),v);
  }

  template<class ViewSel, class View>
  const Choice*
  ViewValuesBrancher<ViewSel,View>::choice(const Space& home, Archive& e) {
    int p;
    unsigned int alt;
    e >> p >> alt;
    return new PosValuesChoice<ViewSel,View>
      (*this,alt,p,viewsel.choice(home,e),e);
  }

  template<class ViewSel, class View>
  ExecStatus
  ViewValuesBrancher<ViewSel,View>
  ::commit(Space& home, const Choice& c, unsigned int a) {
    const PosValuesChoice<ViewSel,View>& pvc
      = static_cast<const PosValuesChoice<ViewSel,View>&>(c);
    View v(ViewBrancher<ViewSel>::view(pvc.pos()).varimp());
    viewsel.commit(home, pvc.viewchoice(), a);
    return me_failed(v.eq(home,pvc.val(a))) ? ES_FAILED : ES_OK;
  }

}}}

// STATISTICS: int-branch
