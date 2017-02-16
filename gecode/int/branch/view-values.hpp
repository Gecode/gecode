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

  /// %Choice storing position and values for integer views
  class GECODE_VTABLE_EXPORT PosValuesChoice : public PosChoice {
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
    /// Initialize choice for brancher \a b, position \a p, and view \a x
    GECODE_INT_EXPORT
    PosValuesChoice(const Brancher& b, const Pos& p, IntView x);
    /// Initialize choice for brancher \a b from archive \a e
    GECODE_INT_EXPORT
    PosValuesChoice(const Brancher& b, unsigned int alt, Pos p, Archive& e);
    /// Return value to branch with for alternative \a a
    int val(unsigned int a) const;
    /// Report size occupied
    GECODE_INT_EXPORT
    virtual size_t size(void) const;
    /// Deallocate
    GECODE_INT_EXPORT
    virtual ~PosValuesChoice(void);
    /// Archive into \a e
    GECODE_INT_EXPORT
    virtual void archive(Archive& e) const;
  };

  forceinline int
  PosValuesChoice::val(unsigned int a) const {
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


  template<int n, bool min, class Print>
  forceinline
  ViewValuesBrancher<n,min,Print>::
  ViewValuesBrancher(Home home, ViewArray<IntView>& x,
                     ViewSel<IntView>* vs[n],
                     IntVarValPrint vvp)
    : ViewBrancher<IntView,n>(home,x,vs), p(vvp) {}

  template<int n, bool min, class Print>
  forceinline void
  ViewValuesBrancher<n,min,Print>::post(Home home, ViewArray<IntView>& x,
                                        ViewSel<IntView>* vs[n],
                                        IntVarValPrint vvp) {
    (void) new (home) ViewValuesBrancher<n,min,Print>(home,x,vs,vvp);
  }

  template<int n, bool min, class Print>
  forceinline
  ViewValuesBrancher<n,min,Print>::
  ViewValuesBrancher(Space& home, bool shared, ViewValuesBrancher& b)
    : ViewBrancher<IntView,n>(home,shared,b), p(home,shared,b.p) {
    if (p.notice())
      home.notice(*this,AP_DISPOSE,true);
  }

  template<int n, bool min, class Print>
  Actor*
  ViewValuesBrancher<n,min,Print>::copy(Space& home, bool shared) {
    return new (home) ViewValuesBrancher<n,min,Print>(home,shared,*this);
  }

  template<int n, bool min, class Print>
  const Choice*
  ViewValuesBrancher<n,min,Print>::choice(Space& home) {
    Pos p = ViewBrancher<IntView,n>::pos(home);
    return new PosValuesChoice(*this,p,
                               ViewBrancher<IntView,n>::view(p));
  }

  template<int n, bool min, class Print>
  const Choice*
  ViewValuesBrancher<n,min,Print>::choice(const Space& home, Archive& e) {
    (void) home;
    int p;
    unsigned int a;
    e >> p >> a;
    return new PosValuesChoice(*this,a,p,e);
  }

  template<int n, bool min, class Print>
  ExecStatus
  ViewValuesBrancher<n,min,Print>::commit(Space& home, const Choice& c,
                                          unsigned int a) {
    const PosValuesChoice& pvc
      = static_cast<const PosValuesChoice&>(c);
    IntView x(ViewBrancher<IntView,n>::view(pvc.pos()));
    unsigned int b = min ? a : (pvc.alternatives() - 1 - a);
    return me_failed(x.eq(home,pvc.val(b))) ? ES_FAILED : ES_OK;
  }

  template<int n, bool min, class Print>
  NGL*
  ViewValuesBrancher<n,min,Print>::ngl(Space& home, const Choice& c,
                                       unsigned int a) const {
    const PosValuesChoice& pvc
      = static_cast<const PosValuesChoice&>(c);
    IntView x(ViewBrancher<IntView,n>::view(pvc.pos()));
    unsigned int b = min ? a : (pvc.alternatives() - 1 - a);
    return new (home) EqNGL<IntView>(home,x,pvc.val(b));
  }

  template<int n, bool min, class Print>
  void
  ViewValuesBrancher<n,min,Print>::print(const Space& home, const Choice& c,
                                         unsigned int a, 
                                         std::ostream& o) const {
    const PosValuesChoice& pvc
      = static_cast<const PosValuesChoice&>(c);
    IntView x(ViewBrancher<IntView,n>::view(pvc.pos()));
    unsigned int b = min ? a : (pvc.alternatives() - 1 - a);
    int nn = pvc.val(b);
    if (p)
      p(home,*this,a,x,pvc.pos().pos,nn,o);
    else
      o << "var[" << pvc.pos().pos << "] = " << nn;
  }

  template<int n, bool min, class Print>
  forceinline size_t
  ViewValuesBrancher<n,min,Print>::dispose(Space& home) {
    if (p.notice())
      home.ignore(*this,AP_DISPOSE,true);
    (void) ViewBrancher<IntView,n>::dispose(home);
    return sizeof(ViewValuesBrancher<n,min,Print>);
  }

  template<int n, bool min>
  forceinline void
  postviewvaluesbrancher(Home home, ViewArray<IntView>& x,
                         ViewSel<IntView>* vs[n],
                         IntVarValPrint vvp) {
    if (vvp)
      ViewValuesBrancher<n,min,BrancherPrint<IntView,int> >
        ::post(home,x,vs,vvp);
    else
      ViewValuesBrancher<n,min,BrancherNoPrint<IntView,int> >
        ::post(home,x,vs,vvp);
  }


}}}

// STATISTICS: int-branch
