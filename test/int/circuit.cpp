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

#include "test/int.hh"
#include <gecode/minimodel.hh>

namespace Test { namespace Int {

   /// %Tests for circuit constraints
   namespace Circuit {

     /**
      * \defgroup TaskTestIntCircuit Circuit constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// Simple test for circuit constraint
     class Circuit : public Test {
     private:
       /// Offset
       int offset;
     public:
       /// Create and register test
       Circuit(int n, int min, int max, int off, Gecode::IntConLevel icl)
         : Test("Circuit::" + str(icl) + "::" + str(n) + "::" + str(off),
                   n,min,max,false,icl), offset(off) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if ((x[i] < 0) || (x[i] > x.size()-1))
             return false;
         int reachable = 0;
         {
           int j=0;
           for (int i=x.size(); i--; ) {
             j=x[j]; reachable |= (1 << j);
           }
         }
         for (int i=x.size(); i--; )
           if (!(reachable & (1 << i)))
             return false;
         return true;
       }
       /// Post circuit constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (offset > 0) {
           Gecode::IntVarArgs xx(x.size());
           for (int i=x.size(); i--;)
             xx[i] = Gecode::expr(home, x[i]+offset);
           Gecode::circuit(home, offset, xx, icl);
         } else {
           Gecode::circuit(home, x, icl);
         }
       }
     };

     /// Simple test for Hamiltonian path constraint
     class Path : public Test {
     private:
       /// Offset
       int offset;
     public:
       /// Create and register test
       Path(int n, int min, int max, int off, Gecode::IntConLevel icl)
         : Test("Path::" + str(icl) + "::" + str(n) + "::" + str(off),
                n+2,min,max,false,icl), offset(off) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 2;
         int s = x[n];
         int e = x[n+1];
         if ((s < 0) || (s > n) || (e < 0) || (e > n) || (x[e] != n))
           return false;
         for (int i=n; i--; )
           if ((i != e) && ((x[i] < 0) || (x[i] > n)))
             return false;
         int reachable = (1 << s);
         {
           int j=s;
           for (int i=n; i--; ) {
             j=x[j]; reachable |= (1 << j);
           }
         }
         for (int i=n; i--; )
           if (!(reachable & (1 << i)))
             return false;
         return true;
       }
       /// Post path constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n = x.size() - 2;
         Gecode::IntVarArgs xx(n);
         if (offset > 0) {
           for (int i=n; i--;)
             xx[i] = Gecode::expr(home, x[i]+offset);
           Gecode::path(home, offset, xx,
                        Gecode::expr(home, x[n]+offset),
                        Gecode::expr(home, x[n+1]+offset),icl);
         } else {
           for (int i=n; i--;)
             xx[i] = x[i];
           Gecode::path(home, xx, x[n], x[n+1], icl);
         }
       }
     };

     /// Simple test for circuit constraint with total cost
     class CircuitCost : public Test {
     private:
       /// Offset
       int offset;
     public:
       /// Create and register test
       CircuitCost(int n, int min, int max, int off, Gecode::IntConLevel icl)
         : Test("Circuit::Cost::"+str(icl)+"::"+str(n)+"::"+str(off),
                n+1,min,max,false,icl), offset(off) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         for (int i=n; i--; )
           if ((x[i] < 0) || (x[i] > n-1))
             return false;
         int reachable = 0;
         {
           int j=0;
           for (int i=n; i--; ) {
             j=x[j]; reachable |= (1 << j);
           }
         }
         for (int i=n; i--; )
           if (!(reachable & (1 << i)))
             return false;
         int c=0;
         for (int i=n; i--; )
           c += x[i];
         return c == x[n];
       }
       /// Post circuit constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size()-1;
         IntArgs c(n*n);
         for (int i=0; i<n; i++)
           for (int j=0; j<n; j++)
             c[i*n+j]=j;
         IntVarArgs y(n);
         if (offset > 0) {
           for (int i=n; i--;)
             y[i] = Gecode::expr(home, x[i]+offset);
           Gecode::circuit(home, c, offset, y, x[n], icl);
         } else {
           for (int i=0; i<n; i++)
             y[i]=x[i];
           circuit(home, c, y, x[n], icl);
         }
       }
     };

     /// Simple test for path constraint with total cost
     class PathCost : public Test {
     private:
       /// Offset
       int offset;
     public:
       /// Create and register test
       PathCost(int n, int min, int max, int off, Gecode::IntConLevel icl)
         : Test("Path::Cost::"+str(icl)+"::"+str(n)+"::"+str(off),
                n+3,min,max,false,icl), offset(off) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 3;
         int s = x[n];
         int e = x[n+1];
         int c = x[n+2] + n;
         if ((s < 0) || (s > n) || (e < 0) || (e > n) || (x[e] != n))
           return false;
         for (int i=n; i--; )
           if ((i != e) && ((x[i] < 0) || (x[i] > n)))
             return false;
         int reachable = (1 << s);
         {
           int j=s;
           for (int i=n; i--; ) {
             j=x[j]; reachable |= (1 << j);
           }
         }
         for (int i=n; i--; )
           if (!(reachable & (1 << i)))
             return false;
         for (int i=n; i--; )
           c -= x[i];
         return c == 0;
       }
       /// Post circuit constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size()-3;
         IntArgs c(n*n);
         for (int i=0; i<n; i++)
           for (int j=0; j<n; j++)
             c[i*n+j]=j;
         IntVarArgs y(n);
         if (offset > 0) {
           for (int i=n; i--;)
             y[i] = Gecode::expr(home, x[i]+offset);
           path(home, c, offset, y, 
                expr(home, x[n]+offset),
                expr(home, x[n+1]+offset),
                x[n+2], icl);
         } else {
           for (int i=0; i<n; i++)
             y[i]=x[i];
           path(home, c, y, x[n], x[n+1], x[n+2], icl);
         }
       }
     };

     /// Simple test for circuit constraint with full cost information
     class CircuitFullCost : public Test {
     private:
       /// Offset
       int offset;
     public:
       /// Create and register test
       CircuitFullCost(int n, int min, int max, int off,
                       Gecode::IntConLevel icl)
         : Test("Circuit::FullCost::" + str(icl)+"::"+str(n)+"::"+str(off),
                2*n+1,min,max,false,icl), offset(off) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=(x.size()-1) / 2;
         for (int i=n; i--; )
           if ((x[i] < 0) || (x[i] > n-1))
             return false;
         int reachable = 0;
         {
           int j=0;
           for (int i=n; i--; ) {
             j=x[j]; reachable |= (1 << j);
           }
         }
         for (int i=n; i--; )
           if (!(reachable & (1 << i)))
             return false;
         for (int i=n; i--; )
           if ((x[i]/2) != x[n+i])
             return false;
         int c=0;
         for (int i=n; i--; )
           c += x[n+i];
         return c == x[2*n];
       }
       /// Post circuit constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=(x.size()-1)/2;
         IntArgs c(n*n);
         for (int i=0; i<n; i++)
           for (int j=0; j<n; j++)
             c[i*n+j]=(j/2);
         IntVarArgs y(n), z(n);
         for (int i=0; i<n; i++) {
           z[i]=x[n+i];
         }
         if (offset > 0) {
           for (int i=n; i--;)
             y[i] = Gecode::expr(home, x[i]+offset);
           Gecode::circuit(home, c, offset, y, z, x[2*n], icl);
         } else {
           for (int i=0; i<n; i++)
             y[i]=x[i];
           circuit(home, c, y, z, x[2*n], icl);
         }
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (int i=1; i<=6; i++) {
           (void) new Circuit(i,0,i-1,0,Gecode::ICL_VAL);
           (void) new Circuit(i,0,i-1,0,Gecode::ICL_DOM);
           (void) new Circuit(i,0,i-1,5,Gecode::ICL_VAL);
           (void) new Circuit(i,0,i-1,5,Gecode::ICL_DOM);
         }
         for (int i=1; i<=4; i++) {
           (void) new Path(i,0,i-1,0,Gecode::ICL_VAL);
           (void) new Path(i,0,i-1,0,Gecode::ICL_DOM);
           (void) new Path(i,0,i-1,5,Gecode::ICL_VAL);
           (void) new Path(i,0,i-1,5,Gecode::ICL_DOM);
         }
         (void) new CircuitCost(4,0,9,0,Gecode::ICL_VAL);
         (void) new CircuitCost(4,0,9,0,Gecode::ICL_DOM);
         (void) new CircuitFullCost(3,0,3,0,Gecode::ICL_VAL);
         (void) new CircuitFullCost(3,0,3,0,Gecode::ICL_DOM);
         (void) new CircuitCost(4,0,9,5,Gecode::ICL_VAL);
         (void) new CircuitCost(4,0,9,5,Gecode::ICL_DOM);
         (void) new CircuitFullCost(3,0,3,5,Gecode::ICL_VAL);
         (void) new CircuitFullCost(3,0,3,5,Gecode::ICL_DOM);
         (void) new PathCost(3,0,5,0,Gecode::ICL_VAL);
         (void) new PathCost(3,0,5,0,Gecode::ICL_DOM);
         (void) new PathCost(3,0,5,5,Gecode::ICL_VAL);
         (void) new PathCost(3,0,5,5,Gecode::ICL_DOM);
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
