/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#include <gecode/int/rel.hh>
#include <gecode/int/bool.hh>

/**
 * \namespace Gecode::Int::Unshare
 * \brief Unsharing shared variables
 */

namespace Gecode { namespace Int { namespace Unshare {

  /// Sort order for variables
  template<class Var>
  class VarPtrLess {
  public:
    forceinline bool
    operator ()(const Var* a, const Var* b) {
      return a->varimp() < b->varimp();
    }
  };
  

  /// Replace by fresh yet equal integer variables
  forceinline void
  link(Home home, IntVar** x, int n, IntPropLevel ipl) {
    if (home.failed()) {
      for (int i=1; i<n; i++)
        *x[i]=IntVar(home,x[0]->min(),x[0]->min());
    } else if (n > 2) {
      ViewArray<IntView> y(home,n);
      y[0]=*x[0];
      for (int i=1; i<n; i++)
        y[i]=*x[i]=IntVar(home,x[0]->min(),x[0]->max());
      if ((ipl == IPL_DOM) || (ipl == IPL_DEF)) {
        ExecStatus es = Rel::NaryEqDom<IntView>::post(home,y);
        (void) es; assert(es == ES_OK);
      } else {
        ExecStatus es = Rel::NaryEqBnd<IntView>::post(home,y);
        (void) es; assert(es == ES_OK);
      }
    } else if (n == 2) {
      *x[1]=IntVar(home,x[0]->min(),x[0]->max());
      if ((ipl == IPL_DOM) || (ipl == IPL_DEF)) {
        ExecStatus es = Rel::EqDom<IntView,IntView>::post(home,*x[0],*x[1]);
        (void) es; assert(es == ES_OK);
      } else {
        ExecStatus es = Rel::EqBnd<IntView,IntView>::post(home,*x[0],*x[1]);
        (void) es; assert(es == ES_OK);
      }
    }
  }

  /// Replace by fresh yet equal Boolean variables
  forceinline void
  link(Home home, BoolVar** x, int n, IntPropLevel) {
    if (home.failed()) {
      for (int i=1; i<n; i++)
        *x[i]=BoolVar(home,0,0);
    } else if (n > 2) {
      ViewArray<BoolView> y(home,n);
      y[0]=*x[0];
      for (int i=1; i<n; i++)
        y[i]=*x[i]=BoolVar(home,0,1);
      ExecStatus es = Bool::NaryEq<BoolView>::post(home,y);
      (void) es; assert(es == ES_OK);
    } else if (n == 2) {
      *x[1] = BoolVar(home,0,1);
      ExecStatus es = Bool::Eq<BoolView,BoolView>::post(home,*x[0],*x[1]);
      (void) es; assert(es == ES_OK);
    }
  }

  /// Replace unassigned shared variables by fresh, yet equal variables
  template<class Var>
  forceinline void
  unshare(Home home, VarArgArray<Var>& x, IntPropLevel ipl) {
    int n=x.size();
    if (n < 2)
      return;
    
    Region r;
    Var** y = r.alloc<Var*>(n);
    for (int i=0; i<n; i++)
      y[i]=&x[i];

    VarPtrLess<Var> vpl;
    Support::quicksort<Var*,VarPtrLess<Var> >(y,n,vpl);
    
    // Replace all shared variables with new and equal variables
    for (int i=0; i<n;) {
      int j=i++;
      while ((i<n) && (y[j]->varimp() == y[i]->varimp()))
        i++;
      if (!y[j]->assigned())
        link(home,&y[j],i-j,ipl);
    }
  }

}}}

namespace Gecode {

  void
  unshare(Home home, IntVarArgs& x, IntPropLevel ipl) {
    Int::Unshare::unshare<IntVar>(home,x,vbd(ipl));
  }

  void
  unshare(Home home, BoolVarArgs& x, IntPropLevel) {
    Int::Unshare::unshare<BoolVar>(home,x,IPL_DEF);
  }

}

// STATISTICS: int-post
