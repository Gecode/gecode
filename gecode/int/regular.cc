/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include "gecode/int/regular.hh"
#include "gecode/int/rel.hh"
#include "gecode/int/bool.hh"

#include "gecode/support/sort.hh"

namespace Gecode {

  using namespace Int;

  namespace {
    
    /// Store view and position for sorting
    template <class View>
    class ViewPos {
    protected:
      /// Sort order for views
      class ViewLess {
      public:
        bool operator()(const ViewPos<View>&, const ViewPos<View>&);
      };
    public:
      View x; ///< The view
      int  i; ///< Position of view in original array
      /// Sort array according to view
      static void sort(ViewPos<View>* x, int n);
    };


    template <class View>
    forceinline bool
    ViewPos<View>::ViewLess::operator()(const ViewPos<View>& a, 
                                        const ViewPos<View>& b) {
      return before(a.x,b.x);
    }

    template <class View>
    inline void
    ViewPos<View>::sort(ViewPos<View>* x, int n) { 
      ViewLess vl;
      Support::quicksort<ViewPos<View>,ViewLess>(x,n,vl);
    }

    /// Return a fresh yet equal variable for the view
    template <class View>
    ExecStatus link(Space* home, View x, View& y);

    template <>
    forceinline ExecStatus
    link(Space* home, IntView x, IntView& y) {
      if (x.assigned()) {
        y=x;
      } else {
        IntVar z(home,x.min(),x.max());
        GECODE_ES_CHECK((Rel::EqDom<IntView,IntView>::post(home,x,z)));
        y=z;
      }
      return ES_OK;
    }

    template <>
    forceinline ExecStatus
    link(Space* home, BoolView x, BoolView& y) {
      if (x.assigned()) {
        y=x;
      } else {
        BoolVar z(home,0,1);
        GECODE_ES_CHECK((Bool::Eq<BoolView,BoolView>::post(home,x,z)));
        y=z;
      }
      return ES_OK;
    }

    /// Replace unassigned shared views by fresh, yet equal views
    template <class View>
    ExecStatus 
    unshare(Space* home, ViewArray<View>& x) {
      int n=x.size();
      if (n < 2)
        return ES_OK;
      // Initialize array with views and positions
      GECODE_AUTOARRAY(ViewPos<View>,y,n);
      for (int i=n; i--; ) {
        y[i].x=x[i]; y[i].i=i;
      }
      ViewPos<View>::sort(y,n);
      // Replace all shared views with new and equal views
      for (int i=0; i<n;) {
        View z = y[i++].x;
        while ((i<n) && shared(z,y[i].x))
          GECODE_ES_CHECK(link(home,z,x[y[i++].i]));
      }
      return ES_OK;
    }

  }

  void
  regular(Space* home, const IntVarArgs& x, DFA dfa, 
          IntConLevel, PropKind) {
    if (home->failed()) return;
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(home,unshare(home,xv));
    GECODE_ES_FAIL(home,(Regular::Dom<IntView>::post(home,xv,dfa)));
  }

  void
  regular(Space* home, const BoolVarArgs& x, DFA dfa, 
          IntConLevel, PropKind) {
    if (home->failed()) return;
    ViewArray<BoolView> xv(home,x);
    GECODE_ES_FAIL(home,unshare(home,xv));
    GECODE_ES_FAIL(home,(Regular::Dom<BoolView>::post(home,xv,dfa)));
  }

}



// STATISTICS: int-post

