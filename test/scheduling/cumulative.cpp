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
#include <gecode/scheduling.hh>

namespace Test { namespace Int {

   /// Tests for cumulative scheduling constraints
   namespace Cumulative {

     /**
      * \defgroup TaskTestCumulative Cumulative scheduling constraints
      * \ingroup TaskTestScheduling
      */
     //@{
     /// Test for cumulative constraint with mandatory tasks
     class ManFixCumulative : public Test {
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
         return e / c;
       }
     public:
       /// Create and register test
       ManFixCumulative(int c0, 
                        const Gecode::IntArgs& p0,
                        const Gecode::IntArgs& u0)
         : Test("Scheduling::Cumulative::Man::Fix::"+
                str(c0)+"::"+str(p0)+"::"+str(u0),
                p0.size(),0,st(c0,p0,u0)), 
           c(c0), p(p0), u(u0) {
         testsearch = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,500);
       }
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // Compute maximal time
         int t = 0;
         for (int i=0; i<x.size(); i++)
           t = std::max(t,x[i]+p[i]);
         // Compute resource usage
         int* used = new int[t];
         for (int i=0; i<t; i++)
           used[i] = 0;
         for (int i=0; i<x.size(); i++)
           for (int t=0; t<p[i]; t++)
             used[x[i]+t] += u[i];
         // Check resource usage
         for (int i=0; i<t; i++)
           if (used[i] > c) {
             delete [] used;
             return false;
           }
         delete [] used;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::cumulative(home, c, x, p, u);
       }
     };


     /// Test for cumulative constraint with optional tasks
     class OptFixCumulative : public Test {
     protected:
       /// Capacity of resource
       int c;
       /// The processing times
       Gecode::IntArgs p;
       /// The resource usage
       Gecode::IntArgs u;
       /// Limit for optional tasks
       int l;
       /// Get a reasonable maximal start time
       static int st(int c, 
                     const Gecode::IntArgs& p, const Gecode::IntArgs& u) {
         double e = 0;
         for (int i=p.size(); i--; )
           e += static_cast<double>(p[i])*u[i];
         return e / c;
       }
     public:
       /// Create and register test
       OptFixCumulative(int c0, 
                        const Gecode::IntArgs& p0,
                        const Gecode::IntArgs& u0)
         : Test("Scheduling::Cumulative::Opt::Fix::"+
                str(c0)+"::"+str(p0)+"::"+str(u0),
                2*p0.size(),0,st(c0,p0,u0)), 
           c(c0), p(p0), u(u0), l(st(c,p,u)/2) {
         testsearch = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,500);
       }
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() / 2;
         // Compute maximal time
         int t = 0;
         for (int i=0; i<n; i++)
           t = std::max(t,x[i]+p[i]);
         // Compute resource usage
         int* used = new int[t];
         for (int i=0; i<t; i++)
           used[i] = 0;
         for (int i=0; i<n; i++)
           if (x[n+i] > l)
             for (int t=0; t<p[i]; t++)
               used[x[i]+t] += u[i];
         // Check resource usage
         for (int i=0; i<t; i++)
           if (used[i] > c) {
             delete [] used;
             return false;
           }
         delete [] used;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size() / 2;
         Gecode::IntVarArgs s(n);
         Gecode::BoolVarArgs m(n);
         for (int i=0; i<n; i++) {
           s[i]=x[i];
           m[i]=Gecode::post(home, ~(x[n+i] > l));
         }
         Gecode::cumulative(home, c, s, p, u, m);
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
         
         IntArgs u1(4, 1,1,1,1);
         IntArgs u2(4, 2,2,2,2);
         IntArgs u3(4, 2,3,4,5);

         for (int c=1; c<8; c++) {
           (void) new ManFixCumulative(c,p1,u1);
           (void) new ManFixCumulative(c,p1,u2);
           (void) new ManFixCumulative(c,p1,u3);
           (void) new ManFixCumulative(c,p2,u1);
           (void) new ManFixCumulative(c,p2,u2);
           (void) new ManFixCumulative(c,p2,u3);
           (void) new ManFixCumulative(c,p3,u1);
           (void) new ManFixCumulative(c,p3,u2);
           (void) new ManFixCumulative(c,p3,u3);
           /*
           (void) new OptFixCumulative(c,p1,u1);
           (void) new OptFixCumulative(c,p1,u2);
           (void) new OptFixCumulative(c,p1,u3);
           (void) new OptFixCumulative(c,p2,u1);
           (void) new OptFixCumulative(c,p2,u2);
           (void) new OptFixCumulative(c,p2,u3);
           (void) new OptFixCumulative(c,p3,u1);
           (void) new OptFixCumulative(c,p3,u2);
           (void) new OptFixCumulative(c,p3,u3);
           */
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
