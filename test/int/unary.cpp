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

   /// %Tests for unary scheduling constraints
   namespace Unary {

     /**
      * \defgroup TaskTestIntUnary Unary scheduling constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for unary constraint
     class ManFixPUnary : public Test {
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
       ManFixPUnary(const Gecode::IntArgs& p0, int o)
         : Test("Unary::Man::Fix::"+str(o)+"::"+str(p0),
                p0.size(),o,o+st(p0)), 
           p(p0) {
         testsearch = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,500);
       }
       /// %Test whether \a x is solution
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

     /// %Test for unary constraint with optional tasks
     class OptFixPUnary : public Test {
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
       OptFixPUnary(const Gecode::IntArgs& p0, int o)
         : Test("Unary::Opt::Fix::"+str(o)+"::"+str(p0),
                2*p0.size(),o,o+st(p0)), p(p0), l(o+st(p)/2) {
         testsearch = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,500);
       }
       /// %Test whether \a x is solution
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
           m[i]=Gecode::expr(home, (x[n+i] > l));
         }
         Gecode::unary(home, s, p, m);
       }
     };


     /// %Test for unary constraint
     class ManFlexUnary : public Test {
     protected:
       /// Minimum processing time
       int _minP;
       /// Maximum processing time
       int _maxP;
       /// Offset for start times
       int off;
     public:
       /// Create and register test
       ManFlexUnary(int n, int minP, int maxP, int o)
         : Test("Unary::Man::Flex::"+str(o)+"::"+str(n)+"::"
                +str(minP)+"::"+str(maxP),
                2*n,0,n*maxP), _minP(minP), _maxP(maxP), off(o) {
         testsearch = false;
         testfix = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomMixAssignment(arity/2,dom,arity/2,
                                        Gecode::IntSet(_minP,_maxP),500);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size()/2;
         for (int i=0; i<n; i++)
           for (int j=i+1; j<n; j++)
             if ((x[i]+x[n+i] > x[j]) && (x[j]+x[n+j] > x[i]))
               return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs s(x.size()/2);
         Gecode::IntVarArgs px(x.slice(x.size()/2));
         Gecode::IntVarArgs e(home,x.size()/2,
                              Gecode::Int::Limits::min,
                              Gecode::Int::Limits::max);
         for (int i=s.size(); i--;) {
           s[i] = expr(home, off+x[i]);
           rel(home, s[i]+px[i] == e[i]);
           rel(home, _minP <= px[i]);
           rel(home, _maxP >= px[i]);
         }
         Gecode::unary(home, s, px, e);
       }
     };

     /// %Test for unary constraint with optional tasks
     class OptFlexUnary : public Test {
     protected:
       /// Minimum processing time
       int _minP;
       /// Maximum processing time
       int _maxP;
       /// Offset for start times
       int off;
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
       OptFlexUnary(int n, int minP, int maxP, int o)
         : Test("Unary::Opt::Flex::"+str(o)+"::"+str(n)+"::"
                +str(minP)+"::"+str(maxP),
                3*n,0,n*maxP), _minP(minP), _maxP(maxP), off(o),
                l(n*maxP/2) {
         testsearch = false;
         testfix = false;
         contest = CTL_NONE;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomMixAssignment(2*(arity/3),dom,arity/3,
                                        Gecode::IntSet(_minP,_maxP),500);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() / 3;
         for (int i=0; i<n; i++)
           if (x[n+i] > l)
             for (int j=i+1; j<n; j++)
               if(x[n+j] > l)
                 if ((x[i]+x[2*n+i] > x[j]) && (x[j]+x[2*n+j] > x[i]))
                   return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size() / 3;

         Gecode::IntVarArgs s(n);
         Gecode::IntVarArgs px(n);
         Gecode::IntVarArgs e(home,n,
                              Gecode::Int::Limits::min,
                              Gecode::Int::Limits::max);
         for (int i=n; i--;) {
           s[i] = expr(home, off+x[i]);
           px[i] = x[2*n+i];
           rel(home, s[i]+px[i] == e[i]);
           rel(home, _minP <= px[i]);
           rel(home, _maxP >= px[i]);
         }
         Gecode::BoolVarArgs m(n);
         for (int i=0; i<n; i++)
           m[i]=Gecode::expr(home, (x[n+i] > l));
         Gecode::unary(home, s, px, e, m);
       }
     };

     Gecode::IntArgs p1(4, 2,2,2,2);
     ManFixPUnary mfu10(p1,0);
     ManFixPUnary mfu1i(p1,Gecode::Int::Limits::min);
     OptFixPUnary ofu10(p1,0);
     OptFixPUnary ofu1i(p1,Gecode::Int::Limits::min);
     ManFlexUnary mflu10(4,0,2,0);
     ManFlexUnary mflu1i(4,0,2,Gecode::Int::Limits::min);
     ManFlexUnary mflu101(4,1,3,0);
     ManFlexUnary mflu1i1(4,1,3,Gecode::Int::Limits::min);
     OptFlexUnary oflu10(4,0,2,0);
     OptFlexUnary oflu1i(4,0,2,Gecode::Int::Limits::min);

     Gecode::IntArgs p10(5, 2,2,0,2,2);
     ManFixPUnary mfu010(p10,0);
     ManFixPUnary mfu01i(p10,Gecode::Int::Limits::min);
     OptFixPUnary ofu010(p10,0);
     OptFixPUnary ofu01i(p10,Gecode::Int::Limits::min);
     ManFlexUnary mflu010(5,0,2,0);
     ManFlexUnary mflu01i(5,0,2,Gecode::Int::Limits::min);
     OptFlexUnary oflu010(5,0,2,0);
     OptFlexUnary oflu01i(5,0,2,Gecode::Int::Limits::min);

     Gecode::IntArgs p2(4, 4,3,3,5);
     ManFixPUnary mfu20(p2,0);
     ManFixPUnary mfu2i(p2,Gecode::Int::Limits::min);
     OptFixPUnary ofu20(p2,0);
     OptFixPUnary ofu2i(p2,Gecode::Int::Limits::min);
     ManFlexUnary mflu20(4,3,5,0);
     ManFlexUnary mflu2i(4,3,5,Gecode::Int::Limits::min);
     OptFlexUnary oflu20(4,3,5,0);
     OptFlexUnary oflu2i(4,3,5,Gecode::Int::Limits::min);

     Gecode::IntArgs p20(6, 4,0,3,3,0,5);
     ManFixPUnary mfu020(p20,0);
     ManFixPUnary mfu02i(p20,Gecode::Int::Limits::min);
     OptFixPUnary ofu020(p20,0);
     OptFixPUnary ofu02i(p20,Gecode::Int::Limits::min);
     ManFlexUnary mflu020(6,0,5,0);
     ManFlexUnary mflu02i(6,0,5,Gecode::Int::Limits::min);
     OptFlexUnary oflu020(6,0,5,0);
     OptFlexUnary oflu02i(6,0,5,Gecode::Int::Limits::min);

     Gecode::IntArgs p3(6, 4,2,9,3,7,5);
     ManFixPUnary mfu30(p3,0);
     ManFixPUnary mfu3i(p3,Gecode::Int::Limits::min);
     OptFixPUnary ofu30(p3,0);
     OptFixPUnary ofu3i(p3,Gecode::Int::Limits::min);
     ManFlexUnary mflu30(6,2,7,0);
     ManFlexUnary mflu3i(6,2,7,Gecode::Int::Limits::min);
     OptFlexUnary oflu30(6,2,7,0);
     OptFlexUnary oflu3i(6,2,7,Gecode::Int::Limits::min);

     Gecode::IntArgs p30(8, 4,0,2,9,3,7,5,0);
     ManFixPUnary mfu030(p30,0);
     ManFixPUnary mfu03i(p30,Gecode::Int::Limits::min);
     OptFixPUnary ofu030(p30,0);
     OptFixPUnary ofu03i(p30,Gecode::Int::Limits::min);
     ManFlexUnary mflu030(8,0,9,0);
     ManFlexUnary mflu03i(8,0,9,Gecode::Int::Limits::min);
     OptFlexUnary oflu030(8,0,9,0);
     OptFlexUnary oflu03i(8,0,9,Gecode::Int::Limits::min);

     //@}

   }
}}

// STATISTICS: test-int
