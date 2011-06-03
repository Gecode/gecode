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
          if ((wi > 0) && (hi > 0))
            for (int j=i+1; j<n; j++) {
              int xj=xwyh[4*j+0], yj=xwyh[4*j+2];
              int wj=xwyh[4*j+1], hj=xwyh[4*j+3];
              if ((wj > 0) && (hj > 0))
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
        IntVarArgs x(n), w(n), y(n), h(n);
        for (int i=0; i<n; i++) {
          x[i]=xwyh[4*i+0]; w[i]=xwyh[4*i+1];
          y[i]=xwyh[4*i+2]; h[i]=xwyh[4*i+3];
        }
        nooverlap(home, x, w, y, h);
      }
    };
    
    /// %Test for no-overlap with integer dimensions (rectangle cuboids)
    class Int3 : public Test {
    protected:
      /// Width
      Gecode::IntArgs w;
      /// Height
      Gecode::IntArgs h;
      /// Depth
      Gecode::IntArgs d;
    public:
      /// Create and register test with maximal coordinate value \a m
      Int3(int m, 
           const Gecode::IntArgs& w0, const Gecode::IntArgs& h0,
           const Gecode::IntArgs& d0) 
        : Test("NoOverlap::Int::3::"+str(m)+"::"+str(w0)+"::"+str(h0)+"::"+str(d0),
               3*w0.size(), 0, m-1), 
          w(w0), h(h0), d(d0) {
      }
      /// %Test whether \a xyz is solution
      virtual bool solution(const Assignment& xyz) const {
        int n = xyz.size() / 3;
        for (int i=0; i<n; i++) {
          int xi=xyz[3*i+0], yi=xyz[3*i+1], zi=xyz[3*i+2];
          for (int j=i+1; j<n; j++) {
            int xj=xyz[3*j+0], yj=xyz[3*j+1], zj=xyz[3*j+2];
            if (!((xi + w[i] <= xj) || (xj + w[j] <= xi) ||
                  (yi + h[i] <= yj) || (yj + h[j] <= yi) ||
                  (zi + d[i] <= zj) || (zj + d[j] <= zi)))
              return false;
          }
        }
        return true;
      }
      /// Post constraint on \a xyz
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& xyz) {
        using namespace Gecode;
        int n = xyz.size() / 3;
        IntVarArgs x(n), y(n), z(n);
        for (int i=0; i<n; i++) {
          x[i]=xyz[3*i+0]; y[i]=xyz[3*i+1]; z[i]=xyz[3*i+2];
        }
        nooverlap(home, x, w, y, h, z, d);
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

        for (int m=2; m<4; m++) {
          (void) new Int2(m, s1, s1);
          (void) new Int2(m, s2, s2);
          (void) new Int2(m, s3, s3);
          (void) new Int2(m, s2, s3);
          (void) new Int2(m, s4, s4);
          (void) new Int2(m, s4, s2);
          (void) new Int3(m, s1, s1, s1);
          (void) new Int3(m, s2, s3, s4);
        }

        (void) new Var2(2, 2);
        (void) new Var2(3, 2);
        (void) new Var2(1, 3);

      }
    };
    
    Create c;
    
    //@}
    
  }

}}


// STATISTICS: test-int

