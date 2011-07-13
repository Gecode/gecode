/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

#include "test/int.hh"

#include <gecode/minimodel.hh>
#include <climits>

namespace Test { namespace Int {
  
  /// %Tests for no-overlap constraint
  namespace NoOverlap {

    /**
     * \defgroup TaskTestIntNoOverlap No-overlap constraints
     * \ingroup TaskTestInt
     */
    //@{
    /// %Test for no-overlap with integer dimensions (rectangles)
    class Int2 : public Test {
    protected:
      /// Width
      Gecode::IntArgs w;
      /// Height
      Gecode::IntArgs h;
    public:
      /// Create and register test with maximal coordinate value \a m
      Int2(int m, const Gecode::IntArgs& w0, const Gecode::IntArgs& h0) 
        : Test("NoOverlap::Int::2::"+str(m)+"::"+str(w0)+"::"+str(h0),
               2*w0.size(), 0, m-1), 
          w(w0), h(h0) {
      }
      /// %Test whether \a xy is solution
      virtual bool solution(const Assignment& xy) const {
        int n = xy.size() / 2;
        for (int i=0; i<n; i++) {
          int xi=xy[2*i+0], yi=xy[2*i+1];
          for (int j=i+1; j<n; j++) {
            int xj=xy[2*j+0], yj=xy[2*j+1];
            if (!((xi + w[i] <= xj) || (xj + w[j] <= xi) ||
                  (yi + h[i] <= yj) || (yj + h[j] <= yi)))
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a xy
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
        using namespace Gecode;
        int n = xy.size() / 2;
        IntVarArgs x(n), y(n);
        for (int i=0; i<n; i++) {
          x[i]=xy[2*i+0]; y[i]=xy[2*i+1];
        }
        nooverlap(home, x, w, y, h);
      }
    };
    /// %Test for no-overlap with optional rectangles
    class IntOpt2 : public Test {
    protected:
      /// Width
      Gecode::IntArgs w;
      /// Height
      Gecode::IntArgs h;
    public:
      /// Create and register test with maximal value \a m and \a n rectangles
      IntOpt2(int m, const Gecode::IntArgs& w0, const Gecode::IntArgs& h0) 
        : Test("NoOverlap::Int::Opt::2::"+str(m)+"::"+str(w0)+"::"+str(h0), 
               3*w0.size(), 0, m-1), w(w0), h(h0) {}
      /// %Test whether \a xyo is solution
      virtual bool solution(const Assignment& xyo) const {
        int n = xyo.size() / 3;
        for (int i=0; i<n; i++) {
          int xi=xyo[3*i+0], yi=xyo[3*i+1];
          int oi=xyo[3*i+2];
          for (int j=i+1; j<n; j++) {
            int xj=xyo[3*j+0], yj=xyo[3*j+1];
            int oj=xyo[3*j+2];
            if ((oi > 0) && (oj > 0) &&
                !((xi + w[i] <= xj) || (xj + w[j] <= xi) ||
                  (yi + h[i] <= yj) || (yj + h[j] <= yi)))
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a xwyho
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& xyo) {
        using namespace Gecode;
        int n = xyo.size() / 3;
        IntVarArgs x(n), y(n);
        BoolVarArgs o(n);
        for (int i=0; i<n; i++) {
          x[i]=xyo[3*i+0]; y[i]=xyo[3*i+1];
          o[i]=expr(home, xyo[3*i+2] > 0);
        }
        nooverlap(home, x, w, y, h, o);
      }
    };
    
    /// %Test for no-overlap with variable dimensions (rectangles)
    class Var2 : public Test {
    public:
      /// Create and register test with maximal value \a m and \a n rectangles
      Var2(int m, int n)
        : Test("NoOverlap::Var::2::"+str(m)+"::"+str(n), 4*n, 0, m) {}
      /// %Test whether \a xwyh is solution
      virtual bool solution(const Assignment& xwyh) const {
        int n = xwyh.size() / 4;
        for (int i=0; i<n; i++) {
          int xi=xwyh[4*i+0], yi=xwyh[4*i+2];
          int wi=xwyh[4*i+1], hi=xwyh[4*i+3];
          for (int j=i+1; j<n; j++) {
            int xj=xwyh[4*j+0], yj=xwyh[4*j+2];
            int wj=xwyh[4*j+1], hj=xwyh[4*j+3];
            if (!((xi + wi <= xj) || (xj + wj <= xi) ||
                  (yi + hi <= yj) || (yj + hj <= yi)))
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a xwyh
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& xwyh) {
        using namespace Gecode;
        int n = xwyh.size() / 4;
        IntVarArgs x0(n), w(n), x1(n), y0(n), h(n), y1(n);
        for (int i=0; i<n; i++) {
          x0[i]=xwyh[4*i+0]; w[i]=xwyh[4*i+1];
          x1[i]=expr(home, x0[i] + w[i]);
          y0[i]=xwyh[4*i+2]; h[i]=xwyh[4*i+3];
          y1[i]=expr(home, y0[i] + h[i]);
        }
        nooverlap(home, x0, w, x1, y0, h, y1);
      }
    };
    
    /// %Test for no-overlap with optional rectangles
    class VarOpt2 : public Test {
    public:
      /// Create and register test with maximal value \a m and \a n rectangles
      VarOpt2(int m, int n)
        : Test("NoOverlap::Var::Opt::2::"+str(m)+"::"+str(n), 5*n, 0, m) {
        testfix = false;
      }
      /// %Test whether \a xwyho is solution
      virtual bool solution(const Assignment& xwyho) const {
        int n = xwyho.size() / 5;
        for (int i=0; i<n; i++) {
          int xi=xwyho[5*i+0], yi=xwyho[5*i+2];
          int wi=xwyho[5*i+1], hi=xwyho[5*i+3];
          int oi=xwyho[5*i+4];
          for (int j=i+1; j<n; j++) {
            int xj=xwyho[5*j+0], yj=xwyho[5*j+2];
            int wj=xwyho[5*j+1], hj=xwyho[5*j+3];
            int oj=xwyho[5*j+4];
            if ((oi > 0) && (oj > 0) &&
                !((xi + wi <= xj) || (xj + wj <= xi) ||
                  (yi + hi <= yj) || (yj + hj <= yi)))
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a xwyho
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& xwyho) {
        using namespace Gecode;
        int n = xwyho.size() / 5;
        IntVarArgs x0(n), w(n), x1(n), y0(n), h(n), y1(n);
        BoolVarArgs o(n);
        for (int i=0; i<n; i++) {
          x0[i]=xwyho[5*i+0]; w[i]=xwyho[5*i+1];
          x1[i]=expr(home, x0[i] + w[i]);
          y0[i]=xwyho[5*i+2]; h[i]=xwyho[5*i+3];
          y1[i]=expr(home, y0[i] + h[i]);
          o[i]=expr(home, xwyho[5*i+4] > 0);
        }
        nooverlap(home, x0, w, x1, y0, h, y1, o);
      }
    };
    
    
    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;

        IntArgs s1(3, 2,1,1);
        IntArgs s2(4, 1,2,3,4);
        IntArgs s3(4, 4,3,2,1);
        IntArgs s4(4, 1,1,1,1);

        for (int m=2; m<3; m++) {
          (void) new Int2(m, s1, s1);
          (void) new Int2(m, s2, s2);
          (void) new Int2(m, s3, s3);
          (void) new Int2(m, s2, s3);
          (void) new Int2(m, s4, s4);
          (void) new Int2(m, s4, s2);
          (void) new IntOpt2(m, s2, s3);
          (void) new IntOpt2(m, s4, s3);
        }

        (void) new Var2(2, 2);
        (void) new Var2(3, 2);
        (void) new Var2(1, 3);
        (void) new VarOpt2(2, 2);
        (void) new VarOpt2(3, 2);

      }
    };
    
    Create c;
    
    //@}
    
  }

}}


// STATISTICS: test-int

