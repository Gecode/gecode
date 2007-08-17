/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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

#include "gecode/int/rel.hh"
#include "gecode/int/bool.hh"

#include "gecode/support/sort.hh"

namespace Gecode { 

  namespace Int { namespace Unshare {

    /// Check order for variables
    template <class Var>
    forceinline bool
    varbefore(Var x, Var y) {
      typename VarViewTraits<Var>::View vx;
      typename VarViewTraits<Var>::View vy;
      return before(vx,vy);
    }

    /// Check sharing for variables
    template <class Var>
    forceinline bool
    varshared(Var x, Var y) {
      typename VarViewTraits<Var>::View vx;
      typename VarViewTraits<Var>::View vy;
      return shared(vx,vy);
    }


    /// Store variable and its position for sorting
    template <class Var>
    class VarPos {
    protected:
      /// Sort order for views
      class VarLess {
      public:
        bool operator()(const VarPos<Var>&, const VarPos<Var>&);
      };
    public:
      Var x; ///< The variable
      int i; ///< Position of variable in original array
      /// Sort array according to variable
      static void sort(VarPos<Var>* x, int n);
    };
    
    
    template <class Var>
    forceinline bool
    VarPos<Var>::VarLess::operator()(const VarPos<Var>& a, 
                                     const VarPos<Var>& b) {
      return varbefore(a.x,b.x);
    }
    
    template <class Var>
    inline void
    VarPos<Var>::sort(VarPos<Var>* x, int n) { 
      VarLess vl;
      Support::quicksort<VarPos<Var>,VarLess>(x,n,vl);
    }
    
    /// Return a fresh yet equal integer variable
    forceinline ExecStatus
    link(Space* home, IntVar x, IntVar& y, IntConLevel icl) {
      if (x.assigned()) {
        y=x;
      } else {
        IntVar z(home,x.min(),x.max());
        if ((icl == ICL_DOM) || (icl == ICL_DEF)) {
          GECODE_ES_CHECK((Rel::EqDom<IntView,IntView>::post(home,x,z)));
        } else {
          GECODE_ES_CHECK((Rel::EqBnd<IntView,IntView>::post(home,x,z)));
        }
        y=z;
      }
      return ES_OK;
    }
    
    /// Return a fresh yet equal Boolean variable
    forceinline ExecStatus
    link(Space* home, BoolVar x, BoolVar& y, IntConLevel) {
      if (x.assigned()) {
        y=x;
      } else {
        BoolVar z(home,0,1);
        GECODE_ES_CHECK((Bool::Eq<BoolView,BoolView>::post(home,x,z)));
        y=z;
      }
      return ES_OK;
    }
    
    /// Replace unassigned shared variables by fresh, yet equal variables
    template <class Var>
    forceinline ExecStatus 
    unshare(Space* home, VarArgArray<Var>& x, IntConLevel icl) {
      int n=x.size();
      if (n < 2)
        return ES_OK;
      // Initialize array with views and positions
      GECODE_AUTOARRAY(VarPos<Var>,y,n);
      for (int i=n; i--; ) {
        y[i].x=x[i]; y[i].i=i;
      }
      VarPos<Var>::sort(y,n);
      // Replace all shared variables with new and equal variables
      for (int i=0; i<n;) {
        Var z = y[i++].x;
        while ((i<n) && varshared(z,y[i].x))
          GECODE_ES_CHECK(link(home,z,x[y[i++].i],icl));
      }
      return ES_OK;
    }
    
  }}

  void 
  unshare(Space* home, IntVarArgs& x, IntConLevel icl, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Int::Unshare::unshare<IntVar>(home,x,icl));
  }

  void 
  unshare(Space* home, BoolVarArgs& x, IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_ES_FAIL(home,Int::Unshare::unshare<BoolVar>(home,x,ICL_DEF));
  }

}

// STATISTICS: int-post
