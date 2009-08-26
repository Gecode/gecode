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
     /// Test for unary constraint
     class ManFixCumulative : public Test {
     protected:
       /// The processing times
       Gecode::IntArgs p;
       /// Get a reasonable maximal start time
       static int st(const Gecode::IntArgs& p) {
         int t = 0;
         for (int i=p.size(); i--; )
           t += p[i];
         return t;
       }
     public:
       /// Create and register test
       ManFixCumulative(const Gecode::IntArgs& p0)
         : Test("Scheduling::Cumulative::Man::Fix::"+str(p0),p0.size(),0,st(p0)), 
           p(p0) {
         testsearch = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,500);
       }
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if ((x[i]+p[i] > x[j]) && (x[j]+p[j] > x[i]))
               return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::unary(home, x, p);
       }
     };

     /// Test for unary constraint with optional tasks
     class OptFixCumulative : public Test {
     protected:
       /// The processing times
       Gecode::IntArgs p;
       /// Thereshold for taking a task as optional
       int l;
       /// Get a reasonable maximal start time
       static int st(const Gecode::IntArgs& p) {
         int t = 0;
         for (int i=p.size(); i--; )
           t += p[i];
         return t;
       }
     public:
       /// Create and register test
       OptFixCumulative(const Gecode::IntArgs& p0)
         : Test("Scheduling::Cumulative::Opt::Fix::"+str(p0),
                2*p0.size(),0,st(p0)), p(p0), l(st(p)/2) {
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
         for (int i=0; i<n; i++)
           if (x[n+i] > l)
             for (int j=i+1; j<n; j++)
               if(x[n+j] > l)
                 if ((x[i]+p[i] > x[j]) && (x[j]+p[j] > x[i]))
                   return false;
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
         Gecode::unary(home, s, p, m);
       }
     };

     Gecode::IntArgs p1(4, 2,2,2,2);
     ManFixCumulative mfu1(p1);
     OptFixCumulative ofu1(p1);

     Gecode::IntArgs p2(4, 4,3,3,5);
     ManFixCumulative mfu2(p2);
     OptFixCumulative ofu2(p2);

     Gecode::IntArgs p3(6, 4,2,9,3,7,5);
     ManFixCumulative mfu3(p3);
     OptFixCumulative ofu3(p3);

     //@}

   }
}}

// STATISTICS: test-int
