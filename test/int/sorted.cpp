/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
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

namespace Test { namespace Int {

   /// %Tests for sorted constraints
   namespace Sorted {

     /**
      * \defgroup TaskTestIntSorted Sorted constraints
      * \ingroup TaskTestInt
      */
     //@{

     /// Relation for sorting integers in increasing order
     class SortIntMin {
     public:
       /// %Test whether \a x is less than \a y
       bool operator()(const int x, const int y) {
         return x<y;
       }
     };

     /// %Test sorted without permutation variables
     class NoVar : public Test {
     protected:
       /// Number of variables to be sorted
       static const int n = 3;
     public:
       /// Create and register test
       NoVar(void) : Test("Sorted::NoVar",2*n,0,3) {}
       /// %Test whether \a xy is solution
       virtual bool solution(const Assignment& xy) const {
         int x[n], y[n];
         for (int i=0;i<3; i++) {
           x[i]=xy[i]; y[i]=xy[n+i];
         }

         for (int i=0; i<n-1; i++)
           if (y[i]>y[i+1])
             return false;

         SortIntMin sim;
         Gecode::Support::quicksort<int,SortIntMin>(x,n,sim);

         for (int i=0; i<n; i++)
           if (x[i] != y[i])
             return false;
         return true;
       }
       /// Post constraint on \a xy
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
         Gecode::IntVarArgs x(n), y(n);
         for (int i=0; i<n; i++) {
           x[i]=xy[i]; y[i]=xy[n+i];
         }
         Gecode::sorted(home,x,y);
       }
     };


     /// %Test sorted with permutation variables
     class PermVar : public Test {
     protected:
       /// Number of variables to be sorted
       static const int n = 3;
     public:
       /// Create and register test
       PermVar(void) : Test("Sorted::PermVar",3*n,0,2) {}
       /// %Test whether \a xyz is solution
       virtual bool solution(const Assignment& xyz) const {
         int x[n], y[n], z[n];
         for (int i=0; i<n; i++) {
           x[i]=xyz[i]; y[i]=xyz[n+i]; z[i]=xyz[2*n+i];
         }
         // check for permutation
         for (int i=0; i<n; i++)
           for (int j=i+1; j<n; j++)
             if (z[i]==z[j])
               return false;

         // y must to be sorted
         for (int i=0; i<n-1; i++)
           if (y[i]>y[i+1])
             return false;

         // check whether permutation is in range
         for (int i=0; i<n; i++)
           if ((z[i] < 0) || (z[i] >= n))
             return false;

         // check whether permutation info is correct
         for (int i=0; i<n; i++)
           if (x[i] != y[z[i]])
             return false;

         // check for sorting
         SortIntMin sim;
         Gecode::Support::quicksort<int,SortIntMin>(x,n,sim);
         for (int i=0; i<n; i++)
           if (x[i] != y[i])
             return false;

         return true;
       }
       /// Post constraint on \a xyz
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xyz) {
         Gecode::IntVarArgs x(n), y(n), z(n);
         for (int i=0; i<n; i++) {
           x[i]=xyz[i]; y[i]=xyz[n+i]; z[i]=xyz[2*n+i];
         }
         Gecode::sorted(home,x,y,z);
       }
     };


     NoVar novar;
     PermVar permvar;
     //@}

   }
}}

// STATISTICS: test-int

