/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

   /// %Tests for distinct constraints
   namespace Distinct {

     /**
      * \defgroup TaskTestIntDistinct Distinct constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// Simple test for distinct constraint
     template<bool useCount>
     class Distinct : public Test {
     private:
       Gecode::IntSet d;
     public:
       /// Create and register test
       Distinct(const Gecode::IntSet& d0, Gecode::IntConLevel icl)
         : Test(std::string(useCount ? "Count::Distinct::" : "Distinct::")+
                str(icl)+"::Sparse",6,d0,false,icl), d(d0) {}
       /// Create and register test
       Distinct(int min, int max, Gecode::IntConLevel icl)
         : Test(std::string(useCount ? "Count::Distinct::" : "Distinct::")+
                str(icl)+"::Dense",6,min,max,false,icl), d(min,max) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if (x[i]==x[j])
               return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (!useCount) {
           Gecode::distinct(home, x, icl);
         } else {
           Gecode::IntSetRanges dr(d);
           int i = 0;
           Gecode::IntArgs ia(Gecode::Iter::Ranges::size(dr));
           for (Gecode::IntSetValues dr2(d); dr2(); ++dr2)
             ia[i++] = dr2.val();
           Gecode::count(home, x, Gecode::IntSet(0,1), ia, icl);
         }
       }
     };

     /// Simple test for distinct constraint with offsets
     class Offset : public Test {
     public:
       /// Create and register test
       Offset(const Gecode::IntSet& d, Gecode::IntConLevel icl)
         : Test("Distinct::Offset::Sparse::"+str(icl),6,d,false,icl) {}
       /// Create and register test
       Offset(int min, int max, Gecode::IntConLevel icl)
         : Test("Distinct::Offset::Dense::"+str(icl),6,min,max,false,icl) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if (x[i]+i==x[j]+j)
               return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntArgs c(x.size());
         for (int i=0; i<x.size(); i++)
           c[i]=i;
         Gecode::distinct(home, c, x, icl);
       }
     };

     /// Randomized test for distinct constraint
     class Random : public Test {
     public:
       /// Create and register test
       Random(int n, int min, int max, Gecode::IntConLevel icl)
         : Test("Distinct::Random::"+str(icl),n,min,max,false,icl) {
         testsearch = false;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         return new RandomAssignment(arity,dom,100);
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if (x[i]==x[j])
               return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::distinct(home, x, icl);
       }
     };

     /// %Testing pathological cases
     class Pathological : public Base {
     protected:
       /// Number of variables
       int n;
       /// Consistency level
       Gecode::IntConLevel icl;
       /// %Test space
       class TestSpace : public Gecode::Space {
       public:
         /// Constructor
         TestSpace(void) {}
         /// Constructor for cloning \a s
         TestSpace(bool share, TestSpace& s)
           : Gecode::Space(share,s) {}
         /// Copy space during cloning
         virtual Gecode::Space* copy(bool share) {
           return new TestSpace(share,*this);
         }
       };
     public:
       /// Create and register test
       Pathological(int n0, Gecode::IntConLevel icl0)
         : Base("Int::Distinct::Pathological::"+
                Test::str(n0)+"::"+Test::str(icl0)), n(n0), icl(icl0) {}
       /// Perform test
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace* s = new TestSpace;
           IntVarArgs x(n);
           for (int i=0; i<n; i++)
             x[i] = IntVar(*s,0,i);
           distinct(*s,x,icl);
           if (s->status() == SS_FAILED) {
             delete s; return false;
           }
           for (int i=0; i<n; i++)
             if (!x[i].assigned() || (x[i].val() != i)) {
               delete s; return false;
             }
           delete s;
         }
         {
           TestSpace* s = new TestSpace;
           IntVarArgs x(2*n);
           for (int i=0; i<n; i++) {
             int v[] = {0,i};
             IntSet d(v,2);
             x[i] = IntVar(*s,d);
           }
           for (int i=n; i<2*n; i++)
             x[i] = IntVar(*s,n-1,i);
           distinct(*s,x,icl);
           if (s->status() == SS_FAILED) {
             delete s; return false;
           }
           for (int i=0; i<n; i++)
             if (!x[i].assigned() || (x[i].val() != i)) {
               delete s; return false;
             }
           delete s;
         }
         return true;
       }
     };

     const int v[7] = {-1001,-1000,-10,0,10,1000,1001};
     Gecode::IntSet d(v,7);

     Distinct<false> dom_d(-3,3,Gecode::ICL_DOM);
     Distinct<false> bnd_d(-3,3,Gecode::ICL_BND);
     Distinct<false> val_d(-3,3,Gecode::ICL_VAL);
     Distinct<false> dom_s(d,Gecode::ICL_DOM);
     Distinct<false> bnd_s(d,Gecode::ICL_BND);
     Distinct<false> val_s(d,Gecode::ICL_VAL);

     Distinct<true> count_dom_d(-3,3,Gecode::ICL_DOM);
     Distinct<true> count_bnd_d(-3,3,Gecode::ICL_BND);
     Distinct<true> count_val_d(-3,3,Gecode::ICL_VAL);
     Distinct<true> count_dom_s(d,Gecode::ICL_DOM);
     Distinct<true> count_bnd_s(d,Gecode::ICL_BND);
     Distinct<true> count_val_s(d,Gecode::ICL_VAL);

     Offset dom_od(-3,3,Gecode::ICL_DOM);
     Offset bnd_od(-3,3,Gecode::ICL_BND);
     Offset val_od(-3,3,Gecode::ICL_VAL);
     Offset dom_os(d,Gecode::ICL_DOM);
     Offset bnd_os(d,Gecode::ICL_BND);
     Offset val_os(d,Gecode::ICL_VAL);

     Random dom_r(20,-50,50,Gecode::ICL_DOM);
     Random bnd_r(50,-500,500,Gecode::ICL_BND);
     Random val_r(50,-500,500,Gecode::ICL_VAL);

     Pathological p_16_v(16,Gecode::ICL_VAL);
     Pathological p_16_b(16,Gecode::ICL_BND);
     Pathological p_16_d(16,Gecode::ICL_DOM);

     Pathological p_32_v(32,Gecode::ICL_VAL);
     Pathological p_32_b(32,Gecode::ICL_BND);
     Pathological p_32_d(32,Gecode::ICL_DOM);
     //@}

   }
}}

// STATISTICS: test-int
