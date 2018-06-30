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
     public:
       /// Create and register test
       Distinct(const Gecode::IntSet& d0, Gecode::IntPropLevel ipl,
                int n=6)
         : Test(std::string(useCount ? "Count::Distinct::" : "Distinct::")+
                str(ipl)+"::Sparse::"+str(n),n,d0,false,ipl) {}
       /// Create and register test
       Distinct(int min, int max, Gecode::IntPropLevel ipl)
         : Test(std::string(useCount ? "Count::Distinct::" : "Distinct::")+
                str(ipl)+"::Dense",6,min,max,false,ipl) {}
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
           Gecode::distinct(home, x, ipl);
         } else {
           Gecode::IntSetRanges dr(dom);
           int i = 0;
           Gecode::IntArgs ia(Gecode::Iter::Ranges::size(dr));
           for (Gecode::IntSetValues dr2(dom); dr2(); ++dr2)
             ia[i++] = dr2.val();
           Gecode::count(home, x, Gecode::IntSet(0,1), ia, ipl);
         }
       }
     };

     /// Simple test for distinct constraint with offsets
     class Offset : public Test {
     public:
       /// Create and register test
       Offset(const Gecode::IntSet& d, Gecode::IntPropLevel ipl)
         : Test("Distinct::Offset::Sparse::"+str(ipl),6,d,false,ipl) {}
       /// Create and register test
       Offset(int min, int max, Gecode::IntPropLevel ipl)
         : Test("Distinct::Offset::Dense::"+str(ipl),6,min,max,false,ipl) {}
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
         Gecode::distinct(home, c, x, ipl);
       }
     };

     /// Simple test for optional distinct constraint
     class Optional : public Test {
     public:
       /// Create and register test
       Optional(const Gecode::IntArgs& d, Gecode::IntPropLevel ipl)
         : Test("Distinct::Optional::"+str(ipl)+"::"+str(d),
                6,Gecode::IntSet(d),false,ipl) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() / 2;
         for (int i=0; i<n; i++)
           if ((x[i] < 0) || (x[i] > 1))
             return false;
         for (int i=0; i<n; i++)
           for (int j=i+1; j<n; j++)
             if ((x[i] == 1) && (x[j] == 1) && (x[n+i] == x[n+j]))
               return false;
         return true;
       }
       /// Post constraint on \a bx
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& bx) {
         int n = bx.size() / 2;
         Gecode::BoolVarArgs b(n);
         Gecode::IntVarArgs x(n);
         for (int i=0; i<n; i++) {
           b[i] = Gecode::channel(home, bx[i]);
           x[i] = bx[n+i];
         }
         Gecode::distinct(home, b, x, ipl);
       }
     };

     /// Simple test for distinct except constant constraint
     class Except : public Test {
     public:
       /// Create and register test
       Except(const Gecode::IntArgs& d, Gecode::IntPropLevel ipl)
         : Test("Distinct::Except::"+str(ipl)+"::"+str(d),
                3,Gecode::IntSet(d),false,ipl) {
         contest = CTL_NONE;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if ((x[i] != 0) && (x[j] != 0) && (x[i] == x[j]))
               return false;
         return true;
       }
       /// Post constraint on \a bx
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::distinct(home, x, 0, ipl);
       }
     };

     /// Randomized test for distinct constraint
     class Random : public Test {
     public:
       /// Create and register test
       Random(int n, int min, int max, Gecode::IntPropLevel ipl)
         : Test("Distinct::Random::"+str(ipl),n,min,max,false,ipl) {
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
         Gecode::distinct(home, x, ipl);
       }
     };

     /// %Testing pathological cases
     class Pathological : public Base {
     protected:
       /// Number of variables
       int n;
       /// Consistency level
       Gecode::IntPropLevel ipl;
       /// %Test space
       class TestSpace : public Gecode::Space {
       public:
         /// Constructor
         TestSpace(void) {}
         /// Constructor for cloning \a s
         TestSpace(TestSpace& s)
           : Gecode::Space(s) {}
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new TestSpace(*this);
         }
       };
     public:
       /// Create and register test
       Pathological(int n0, Gecode::IntPropLevel ipl0)
         : Base("Int::Distinct::Pathological::"+
                Test::str(n0)+"::"+Test::str(ipl0)), n(n0), ipl(ipl0) {}
       /// Perform test
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace* s = new TestSpace;
           IntVarArgs x(n);
           for (int i=0; i<n; i++)
             x[i] = IntVar(*s,0,i);
           distinct(*s,x,ipl);
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
           distinct(*s,x,ipl);
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
     const int vl[6] = {Gecode::Int::Limits::min+0,
                        Gecode::Int::Limits::min+1,
                        Gecode::Int::Limits::min+2,
                        Gecode::Int::Limits::max-2,
                        Gecode::Int::Limits::max-1,
                        Gecode::Int::Limits::max-0};
     Gecode::IntSet dl(vl,6);

     Distinct<false> dom_d(-3,3,Gecode::IPL_DOM);
     Distinct<false> bnd_d(-3,3,Gecode::IPL_BND);
     Distinct<false> val_d(-3,3,Gecode::IPL_VAL);
     Distinct<false> dom_s(d,Gecode::IPL_DOM);
     Distinct<false> bnd_s(d,Gecode::IPL_BND);
     Distinct<false> val_s(d,Gecode::IPL_VAL);

     Distinct<false> dom_l(dl,Gecode::IPL_DOM,5);
     Distinct<false> bnd_l(dl,Gecode::IPL_BND,5);
     Distinct<false> val_l(dl,Gecode::IPL_VAL,5);

     Distinct<true> count_dom_d(-3,3,Gecode::IPL_DOM);
     Distinct<true> count_bnd_d(-3,3,Gecode::IPL_BND);
     Distinct<true> count_val_d(-3,3,Gecode::IPL_VAL);
     Distinct<true> count_dom_s(d,Gecode::IPL_DOM);
     Distinct<true> count_bnd_s(d,Gecode::IPL_BND);
     Distinct<true> count_val_s(d,Gecode::IPL_VAL);

     Offset dom_od(-3,3,Gecode::IPL_DOM);
     Offset bnd_od(-3,3,Gecode::IPL_BND);
     Offset val_od(-3,3,Gecode::IPL_VAL);
     Offset dom_os(d,Gecode::IPL_DOM);
     Offset bnd_os(d,Gecode::IPL_BND);
     Offset val_os(d,Gecode::IPL_VAL);

     Gecode::IntArgs v1({Gecode::Int::Limits::min+4,
                         0,1,
                         Gecode::Int::Limits::max});
     Gecode::IntArgs v2({Gecode::Int::Limits::min,
                         0,1,
                         Gecode::Int::Limits::max-4});
     Gecode::IntArgs v3({0,1,2,3});
     Gecode::IntArgs v4({0,1,2});
     Gecode::IntArgs v5({0,1});

     Optional od1(v1,Gecode::IPL_DOM);
     Optional ob1(v1,Gecode::IPL_BND);
     Optional ov1(v1,Gecode::IPL_VAL);
     Optional od2(v2,Gecode::IPL_DOM);
     Optional ob2(v2,Gecode::IPL_BND);
     Optional ov2(v2,Gecode::IPL_VAL);
     Optional od3(v3,Gecode::IPL_DOM);
     Optional ob3(v3,Gecode::IPL_BND);
     Optional ov3(v3,Gecode::IPL_VAL);
     Optional od4(v4,Gecode::IPL_DOM);
     Optional ob4(v4,Gecode::IPL_BND);
     Optional ov4(v4,Gecode::IPL_VAL);
     Optional od5(v5,Gecode::IPL_DOM);
     Optional ob5(v5,Gecode::IPL_BND);
     Optional ov5(v5,Gecode::IPL_VAL);

     Except ed1(v1,Gecode::IPL_DOM);
     Except eb1(v1,Gecode::IPL_BND);
     Except ev1(v1,Gecode::IPL_VAL);
     Except ed2(v2,Gecode::IPL_DOM);
     Except eb2(v2,Gecode::IPL_BND);
     Except ev2(v2,Gecode::IPL_VAL);
     Except ed5(v5,Gecode::IPL_DOM);
     Except eb5(v5,Gecode::IPL_BND);
     Except ev5(v5,Gecode::IPL_VAL);

     Random dom_r(20,-50,50,Gecode::IPL_DOM);
     Random bnd_r(50,-500,500,Gecode::IPL_BND);
     Random val_r(50,-500,500,Gecode::IPL_VAL);

     Pathological p_16_v(16,Gecode::IPL_VAL);
     Pathological p_16_b(16,Gecode::IPL_BND);
     Pathological p_16_d(16,Gecode::IPL_DOM);

     Pathological p_32_v(32,Gecode::IPL_VAL);
     Pathological p_32_b(32,Gecode::IPL_BND);
     Pathological p_32_d(32,Gecode::IPL_DOM);
     //@}

   }
}}

// STATISTICS: test-int
