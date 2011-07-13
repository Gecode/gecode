/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Test { namespace Int {

  /// Tests for cumulative scheduling constraints
  namespace Cumulative {

    /**
     * \defgroup TaskTestIntCumulative Cumulative scheduling constraints
     * \ingroup TaskTestInt
     */
    //@{
    /// Test for cumulative constraint with mandatory tasks
    class ManFixPCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// The processing times
      Gecode::IntArgs p;
      /// The resource usage
      Gecode::IntArgs u;
      /// Get a reasonable maximal start time
      static int st(int c, 
                    const Gecode::IntArgs& p, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=p.size(); i--; )
          e += static_cast<double>(p[i])*u[i];
        return e / std::max(1,std::abs(c));
      }
      /// Offset
      int o;
    public:
      /// Create and register test
      ManFixPCumulative(int c0, 
                       const Gecode::IntArgs& p0,
                       const Gecode::IntArgs& u0,
                       int o0)
        : Test("Cumulative::Man::Fix::"+str(o0)+"::"+
               str(c0)+"::"+str(p0)+"::"+str(u0),
               (c0 >= 0) ? p0.size():p0.size()+1,0,st(c0,p0,u0)), 
          c(c0), p(p0), u(u0), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomAssignment(arity,dom,500);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int cmax = (c >= 0) ? c : x[x.size()-1];
        int n = (c >= 0) ? x.size() : x.size()-1;
        
        if (c < 0 && x[n] > -c)
          return false;
        
        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,p[i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          for (int t=0; t<p[i]; t++)
            used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++) {
          for (int t=1; t<p[i]; t++) {
            used[x[i]+t] += u[i];
          }
        }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int n = (c >= 0) ? x.size() : x.size()-1;
        Gecode::IntVarArgs xx;
        if (o==0) {
          xx=x.slice(0,1,n);
        } else {
          xx=Gecode::IntVarArgs(n);
          for (int i=n; i--;)
            xx[i]=Gecode::expr(home,x[i]+o,Gecode::ICL_DOM);
        }
        if (c >= 0) {
          Gecode::cumulative(home, c, xx, p, u);
        } else {
          Gecode::rel(home, x[n] <= -c);
          Gecode::cumulative(home, x[n], xx, p, u);
        }
      }
    };


    /// Test for cumulative constraint with optional tasks
    class OptFixPCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// The processing times
      Gecode::IntArgs p;
      /// The resource usage
      Gecode::IntArgs u;
      /// Limit for optional tasks
      int l;
      /// Offset
      int o;
      /// Get a reasonable maximal start time
      static int st(int c, 
                    const Gecode::IntArgs& p, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=p.size(); i--; )
          e += static_cast<double>(p[i])*u[i];
        return e / std::max(1,std::abs(c));
      }
    public:
      /// Create and register test
      OptFixPCumulative(int c0, 
                        const Gecode::IntArgs& p0,
                        const Gecode::IntArgs& u0,
                        int o0)
        : Test("Cumulative::Opt::Fix::"+str(o0)+"::"+
               str(c0)+"::"+str(p0)+"::"+str(u0),
               (c0 >= 0) ? 2*p0.size() : 2*p0.size()+1,0,st(c0,p0,u0)), 
          c(c0), p(p0), u(u0), l(st(c,p,u)/2), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomAssignment(arity,dom,500);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int cmax = (c >= 0) ? c : x[nn];

        if (c < 0 && x[nn] > -c)
          return false;

        int n = nn / 2;
        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,p[i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=0; t<p[i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++) {
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l) {
            for (int t=1; t<p[i]; t++)
              used[x[i]+t] += u[i];
          }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            if (used[x[i]]+u[i] > cmax) {
              delete [] used;
              return false;
            }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn=(c >= 0) ? x.size() : x.size()-1;
        int n=nn / 2;
        Gecode::IntVarArgs s(n);
        Gecode::BoolVarArgs m(n);

        for (int i=0; i<n; i++) {
          s[i]=(c >= 0) ? x[i] : Gecode::expr(home,x[i]+o,Gecode::ICL_DOM);
          m[i]=Gecode::expr(home, x[n+i] > l);
        }

        if (c >= 0) {
          Gecode::cumulative(home, c, s, p, u, m);
        } else {
          Gecode::rel(home, x[nn] <= -c);
          Gecode::cumulative(home, x[nn], s, p, u, m);
        }
      }
    };

    /// Test for cumulative constraint with flexible mandatory tasks
    class ManFlexCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// Minimum processing time
      int _minP;
      /// Maximum processing time
      int _maxP;
      /// The resource usage
      Gecode::IntArgs u;
      /// Get a reasonable maximal start time
      static int st(int c, int maxP, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=u.size(); i--; )
          e += static_cast<double>(maxP)*u[i];
        return e / std::max(1,std::abs(c));
      }
      /// Offset
      int o;
    public:
      /// Create and register test
      ManFlexCumulative(int c0, int minP, int maxP,
                        const Gecode::IntArgs& u0,
                        int o0)
        : Test("Cumulative::Man::Flex::"+str(o0)+"::"+
               str(c0)+"::"+str(minP)+"::"+str(maxP)+"::"+str(u0),
               (c0 >= 0) ? 2*u0.size() : 2*u0.size()+1,
               0,std::max(maxP,st(c0,maxP,u0))), 
          c(c0), _minP(minP), _maxP(maxP), u(u0), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomMixAssignment((c >= 0) ? arity/2 : arity/2+1,
                                       dom,arity/2,
                                       Gecode::IntSet(_minP,_maxP),500);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn/2;
        int cmax = (c >= 0) ? c : x[n];
        int pstart = (c >= 0) ? n : n+1;

        if (c < 0 && cmax > -c)
          return false;

        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++) {
          t = std::max(t,x[i]+std::max(1,x[pstart+i]));
        }
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          for (int t=0; t<x[pstart+i]; t++)
            used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++) {
          for (int t=1; t<x[pstart+i]; t++)
            used[x[i]+t] += u[i];
        }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn/2;
        int pstart = (c >= 0) ? n : n+1;
        Gecode::IntVarArgs s(n);
        Gecode::IntVarArgs px(x.slice(pstart,1,n));
        Gecode::IntVarArgs e(home,n,
                             Gecode::Int::Limits::min,
                             Gecode::Int::Limits::max);
        for (int i=s.size(); i--;) {
          s[i] = expr(home, o+x[i], Gecode::ICL_DOM);
          rel(home, s[i]+px[i] == e[i]);
          rel(home, _minP <= px[i]);
          rel(home, _maxP >= px[i]);
        }
        if (c >= 0) {
          Gecode::cumulative(home, c, s, px, e, u);
        } else {
          rel(home, x[n] <= -c);
          Gecode::cumulative(home, x[n], s, px, e, u);
        }
      }
    };

    /// Test for cumulative constraint with optional flexible tasks
    class OptFlexCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// Minimum processing time
      int _minP;
      /// Maximum processing time
      int _maxP;
      /// The resource usage
      Gecode::IntArgs u;
      /// Limit for optional tasks
      int l;
      /// Offset
      int o;
      /// Get a reasonable maximal start time
      static int st(int c, int maxP, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=u.size(); i--; )
          e += static_cast<double>(maxP)*u[i];
        return e / std::max(1,std::abs(c));
      }
    public:
      /// Create and register test
      OptFlexCumulative(int c0, int minP, int maxP,
                        const Gecode::IntArgs& u0,
                        int o0)
        : Test("Cumulative::Opt::Flex::"+str(o0)+"::"+
               str(c0)+"::"+str(minP)+"::"+str(maxP)+"::"+str(u0),
               (c0 >= 0) ? 3*u0.size() : 3*u0.size()+1,
               0,std::max(maxP,st(c0,maxP,u0))), 
          c(c0), _minP(minP), _maxP(maxP), u(u0), 
          l(std::max(maxP,st(c0,maxP,u0))/2), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomMixAssignment((c >= 0) ? 2*(arity/3) : 2*(arity/3)+1,
                                       dom,arity/3,
                                       Gecode::IntSet(_minP,_maxP),500);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn / 3;
        int cmax = (c >= 0) ? c : x[2*n];
        int pstart = (c >= 0) ? 2*n : 2*n+1;
        
        if (c < 0 && cmax > -c)
          return false;
        
        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,x[pstart+i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=0; t<x[pstart+i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=1; t<x[pstart+i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (x[n+i] > l && used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n=nn / 3;
        int pstart= (c >= 0) ? 2*n : 2*n+1;

        Gecode::IntVarArgs s(n);
        Gecode::IntVarArgs px(n);
        Gecode::IntVarArgs e(home,n,
                             Gecode::Int::Limits::min,
                             Gecode::Int::Limits::max);
        for (int i=n; i--;) {
          s[i] = expr(home, o+x[i]);
          px[i] = x[pstart+i];
          rel(home, s[i]+px[i] == e[i]);
          rel(home, _minP <= px[i]);
          rel(home, _maxP >= px[i]);
        }
        Gecode::BoolVarArgs m(n);
        for (int i=0; i<n; i++)
          m[i]=Gecode::expr(home, (x[n+i] > l));
        if (c >= 0) {
          Gecode::cumulative(home, c, s, px, e, u, m);
        } else {
          Gecode::rel(home, x[2*n] <= -c);
          Gecode::cumulative(home, x[2*n], s, px, e, u, m);
        }
      }
    };

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;
        IntArgs p1(4, 1,1,1,1);
        IntArgs p2(4, 2,2,2,2);
        IntArgs p3(4, 4,3,3,5);
        IntArgs p4(4, 4,0,3,5);

        IntArgs u1(4, 1,1,1,1);
        IntArgs u2(4, 2,2,2,2);
        IntArgs u3(4, 2,3,4,5);

        for (int c=-7; c<8; c++) {
          int off = 0;
          for (int coff=0; coff<2; coff++) {
            (void) new ManFixPCumulative(c,p1,u1,off);
            (void) new ManFixPCumulative(c,p1,u2,off);
            (void) new ManFixPCumulative(c,p1,u3,off);
            (void) new ManFixPCumulative(c,p2,u1,off);
            (void) new ManFixPCumulative(c,p2,u2,off);
            (void) new ManFixPCumulative(c,p2,u3,off);
            (void) new ManFixPCumulative(c,p3,u1,off);
            (void) new ManFixPCumulative(c,p3,u2,off);
            (void) new ManFixPCumulative(c,p3,u3,off);
            (void) new ManFixPCumulative(c,p4,u1,off);
            (void) new ManFixPCumulative(c,p4,u2,off);
            (void) new ManFixPCumulative(c,p4,u3,off);

            (void) new ManFlexCumulative(c,0,1,u1,off);
            (void) new ManFlexCumulative(c,0,1,u2,off);
            (void) new ManFlexCumulative(c,0,1,u3,off);
            (void) new ManFlexCumulative(c,0,2,u1,off);
            (void) new ManFlexCumulative(c,0,2,u2,off);
            (void) new ManFlexCumulative(c,0,2,u3,off);
            (void) new ManFlexCumulative(c,3,5,u1,off);
            (void) new ManFlexCumulative(c,3,5,u2,off);
            (void) new ManFlexCumulative(c,3,5,u3,off);

            (void) new OptFixPCumulative(c,p1,u1,off);
            (void) new OptFixPCumulative(c,p1,u2,off);
            (void) new OptFixPCumulative(c,p1,u3,off);
            (void) new OptFixPCumulative(c,p2,u1,off);
            (void) new OptFixPCumulative(c,p2,u2,off);
            (void) new OptFixPCumulative(c,p2,u3,off);
            (void) new OptFixPCumulative(c,p3,u1,off);
            (void) new OptFixPCumulative(c,p3,u2,off);
            (void) new OptFixPCumulative(c,p3,u3,off);
            (void) new OptFixPCumulative(c,p4,u1,off);
            (void) new OptFixPCumulative(c,p4,u2,off);
            (void) new OptFixPCumulative(c,p4,u3,off);

            (void) new OptFlexCumulative(c,0,1,u1,off);
            (void) new OptFlexCumulative(c,0,1,u2,off);
            (void) new OptFlexCumulative(c,0,1,u3,off);
            (void) new OptFlexCumulative(c,0,2,u1,off);
            (void) new OptFlexCumulative(c,0,2,u2,off);
            (void) new OptFlexCumulative(c,0,2,u3,off);
            (void) new OptFlexCumulative(c,3,5,u1,off);
            (void) new OptFlexCumulative(c,3,5,u2,off);
            (void) new OptFlexCumulative(c,3,5,u3,off);

            off = Gecode::Int::Limits::min;
          }
        }
      }
    };

    Create c;
    //@}

  }
}}

// STATISTICS: test-int
