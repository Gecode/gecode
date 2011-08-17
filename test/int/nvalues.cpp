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

#include <gecode/minimodel.hh>
#include "test/int.hh"

namespace Test { namespace Int {

   /// %Tests for number of values constraints
   namespace NValues {

     /**
      * \defgroup TaskTestIntNValues Number of values constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test number of values of integer variables equal to integer
     class IntInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Number of values
       int m;
     public:
       /// Create and register test
       IntInt(int n, int m0, Gecode::IntRelType irt0)
         : Test("NValues::Int::Int::"+str(irt0)+"::"+str(n)+"::"+str(m0),
                n,0,n), 
           irt(irt0), m(m0) {
         testfix = false;
         if (arity > 5)
           testsearch = false;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         if (arity > 5)
           return new RandomAssignment(arity,dom,500);
         else
           return new CpltAssignment(arity,dom);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size();
         bool* v = new bool[n+1];
         for (int i=n+1; i--; )
           v[i] = false;
         int k = 0;
         for (int i=n; i--; )
           if (!v[x[i]]) {
             k++;
             v[x[i]] = true;
           }
         delete [] v;
         return cmp(k,irt,m);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::nvalues(home, x, irt, m);
       }
     };

     /// %Test number of values of integer variables equal to integer variable
     class IntVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVar(int n, Gecode::IntRelType irt0)
         : Test("NValues::Int::Var::"+str(irt0)+"::"+str(n),n+1,0,n), 
           irt(irt0) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 1;
         bool* v = new bool[n+1];
         for (int i=n+1; i--; )
           v[i] = false;
         int k = 0;
         for (int i=n; i--; )
           if (!v[x[i]]) {
             k++;
             v[x[i]] = true;
           }
         delete [] v;
         return cmp(k,irt,x[n]);
       }
       /// Post constraint on \a xy
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
         int n = xy.size() - 1;
         Gecode::IntVarArgs x(n);
         for (int i=n; i--; )
           x[i] = xy[i];
         Gecode::nvalues(home, x, irt, xy[n]);
       }
     };

     /// %Test number of values of Boolean variables equal to integer
     class BoolInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Number of values
       int m;
     public:
       /// Create and register test
       BoolInt(int n, int m0, Gecode::IntRelType irt0)
         : Test("NValues::Bool::Int::"+str(irt0)+"::"+str(n)+"::"+str(m0),
                n,0,2), 
           irt(irt0), m(m0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size();
         for (int i=n; i--; )
           if (x[i] > 1)
             return false;
         bool* v = new bool[n+1];
         for (int i=n+1; i--; )
           v[i] = false;
         int k = 0;
         for (int i=n; i--; )
           if (!v[x[i]]) {
             k++;
             v[x[i]] = true;
           }
         delete [] v;
         return cmp(k,irt,m);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(x.size());
         for (int i=x.size(); i--; )
           y[i] = channel(home, x[i]);
         nvalues(home, y, irt, m);
       }
     };

     /// %Test number of values of Boolean variables equal to integer variable
     class BoolVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolVar(int n, Gecode::IntRelType irt0)
         : Test("NValues::Bool::Var::"+str(irt0)+"::"+str(n),n+1,0,2), 
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 1;
         for (int i=n; i--; )
           if (x[i] > 1)
             return false;
         bool* v = new bool[n+1];
         for (int i=n+1; i--; )
           v[i] = false;
         int k = 0;
         for (int i=n; i--; )
           if (!v[x[i]]) {
             k++;
             v[x[i]] = true;
           }
         delete [] v;
         return cmp(k,irt,x[n]);
       }
       /// Post constraint on \a xy
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
         using namespace Gecode;
         int n = xy.size() - 1;
         BoolVarArgs x(n);
         for (int i=n; i--; )
           x[i] = channel(home, xy[i]);
         nvalues(home, x, irt, xy[n]);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (IntRelTypes irts; irts(); ++irts) {
           for (int i=1; i<=7; i += 3) {
             for (int m=0; m<=3; m++)
               (void) new BoolInt(i, m, irts.irt());
             (void) new BoolVar(i, irts.irt());
           }
           for (int i=1; i<=7; i += 2) {
             for (int m=0; m<=i+1; m++)
               (void) new IntInt(i, m, irts.irt());
             if (i <= 5)
               (void) new IntVar(i, irts.irt());
           }
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int

