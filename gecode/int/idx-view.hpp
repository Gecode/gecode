/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

namespace Gecode { namespace Int {

  /// VarArg type for integer views
  template<>
  class ViewToVarArg<IntView> {
  public:
    typedef IntVarArgs argtype;
  };
  /// VarArg type for minus views
  template<>
  class ViewToVarArg<MinusView> {
  public:
    typedef IntVarArgs argtype;
  };
  /// VarArg type for Boolean views
  template<>
  class ViewToVarArg<BoolView> {
  public:
    typedef BoolVarArgs argtype;
  };
  /// VarArg type for Boolean views
  template<>
  class ViewToVarArg<NegBoolView> {
  public:
    typedef BoolVarArgs argtype;
  };

  template<class View>
  forceinline IdxView<View>*
  IdxView<View>::allocate(Space& home, int n) {
    return home.alloc<IdxView<View> >(n);
  }

  template<class View>
  forceinline
  IdxViewArray<View>::IdxViewArray(void) : xs(NULL), n(0) {}

  template<class View>
  forceinline
  IdxViewArray<View>::IdxViewArray(const IdxViewArray<View>& a) {
    n = a.n; xs = a.xs;
  }

  template<class View>
  forceinline
  IdxViewArray<View>::IdxViewArray(Space& home,
    const typename ViewToVarArg<View>::argtype& xa) : xs(NULL) {
    n = xa.size();
    if (n>0) {
      xs = IdxView<View>::allocate(home, n);
      for (int i=0; i<n; i++) {
        xs[i].idx = i; xs[i].view = xa[i];
      }
    }
  }

  template<class View>
  forceinline
  IdxViewArray<View>::IdxViewArray(Space& home, int n0) : xs(NULL) {
    n = n0;
    if (n>0) {
      xs = IdxView<View>::allocate(home, n);
    }
  }

  template<class View>
  forceinline int
  IdxViewArray<View>::size(void) const {
    return n;
  }

  template<class View>
  forceinline void
  IdxViewArray<View>::size(int n0) {
    n = n0;
  }

  template<class View>
  forceinline IdxView<View>&
  IdxViewArray<View>::operator [](int i) {
    assert((i >= 0) && (i < size()));
    return xs[i];
  }

  template<class View>
  forceinline const IdxView<View>&
  IdxViewArray<View>::operator [](int i) const {
    assert((i >= 0) && (i < size()));
    return xs[i];
  }

  template<class View>
  forceinline void
  IdxViewArray<View>::subscribe(Space& home, Propagator& p, PropCond pc,
                                bool process) {
    for (int i=0; i<n; i++)
      xs[i].view.subscribe(home,p,pc,process);
  }

  template<class View>
  forceinline void
  IdxViewArray<View>::cancel(Space& home, Propagator& p, PropCond pc) {
    for (int i=0; i<n; i++)
      xs[i].view.cancel(home,p,pc);
  }

  template<class View>
  forceinline void
  IdxViewArray<View>::reschedule(Space& home, Propagator& p, PropCond pc) {
    for (int i=0; i<n; i++)
      xs[i].view.reschedule(home,p,pc);
  }

  template<class View>
  forceinline void
  IdxViewArray<View>::update(Space& home, IdxViewArray<View>& a) {
    n = a.size();
    if (n>0) {
      xs = IdxView<View>::allocate(home,n);
      for (int i=0; i<n; i++) {
        xs[i].idx = a[i].idx;
        xs[i].view.update(home,a[i].view);
      }
    }
  }


  template<class Char, class Traits, class View>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os,
              const IdxViewArray<View>& x) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << '{';
    if (x.size() > 0) {
      s << x[0].idx << ':' << x[0].view;
      for (int i=1; i<x.size(); i++)
        s << ", " << x[i].idx << ':' << x[i].view;
    }
    s << '}';
    return os << s.str();
  }

}}

// STATISTICS: int-prop

