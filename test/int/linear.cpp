/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

   /// %Tests for linear constraints
   namespace Linear {

     /// Check whether \a has only one coefficients
     bool one(const Gecode::IntArgs& a) {
      for (int i=a.size(); i--; )
        if (a[i] != 1)
          return false;
      return true;
    }

     /**
      * \defgroup TaskTestIntLinear Linear constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test linear relation over integer variables
     class IntInt : public Test {
     protected:
       /// Coefficients
       Gecode::IntArgs a;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Result
       int c;
     public:
       /// Create and register test
       IntInt(const std::string& s, const Gecode::IntSet& d,
              const Gecode::IntArgs& a0, Gecode::IntRelType irt0,
              int c0, Gecode::IntConLevel icl=Gecode::ICL_BND)
         : Test("Linear::Int::Int::"+
                str(irt0)+"::"+str(icl)+"::"+s+"::"+str(c0)+"::"
                +str(a0.size()),
                a0.size(),d,icl != Gecode::ICL_DOM,icl),
         a(a0), irt(irt0), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double e = 0.0;
         for (int i=0; i<x.size(); i++)
           e += a[i]*x[i];
         return cmp(e, irt, static_cast<double>(c));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (one(a))
           Gecode::linear(home, x, irt, c, icl);
         else
           Gecode::linear(home, a, x, irt, c, icl);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         if (one(a))
           Gecode::linear(home, x, irt, c, b, icl);
         else
           Gecode::linear(home, a, x, irt, c, b, icl);
       }
     };

     /// %Test linear relation over integer variables
     class IntVar : public Test {
     protected:
       /// Coefficients
       Gecode::IntArgs a;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVar(const std::string& s, const Gecode::IntSet& d,
              const Gecode::IntArgs& a0, Gecode::IntRelType irt0,
              Gecode::IntConLevel icl=Gecode::ICL_BND)
         : Test("Linear::Int::Var::"+
                str(irt0)+"::"+str(icl)+"::"+s+"::"+str(a0.size()),
                a0.size()+1,d,icl != Gecode::ICL_DOM,icl),
           a(a0), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double e = 0.0;
         for (int i=0; i<a.size(); i++)
           e += a[i]*x[i];
         return cmp(e, irt, static_cast<double>(x[a.size()]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n = a.size();
         Gecode::IntVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         if (one(a))
           Gecode::linear(home, y, irt, x[n], icl);
         else
           Gecode::linear(home, a, y, irt, x[n], icl);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         int n = a.size();
         Gecode::IntVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         if (one(a))
           Gecode::linear(home, y, irt, x[n], b, icl);
         else
           Gecode::linear(home, a, y, irt, x[n], b, icl);
       }
     };

     /// %Test linear relation over Boolean variables equal to constant
     class BoolInt : public Test {
     protected:
       /// Coefficients
       Gecode::IntArgs a;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Righthand-side constant
       int c;
     public:
       /// Create and register test
       BoolInt(const std::string& s, const Gecode::IntArgs& a0,
               Gecode::IntRelType irt0, int c0)
         : Test("Linear::Bool::Int::"+
                str(irt0)+"::"+s+"::"+str(a0.size())+"::"+str(c0),
                a0.size(),0,1,true,Gecode::ICL_DEF),
           a(a0), irt(irt0), c(c0) {
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double e = 0.0;
         for (int i=0; i<x.size(); i++)
           e += a[i]*x[i];
         return cmp(e, irt, static_cast<double>(c));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVarArgs y(x.size());
         for (int i=x.size(); i--; )
           y[i]=Gecode::channel(home,x[i]);
         if (one(a))
           Gecode::linear(home, y, irt, c, Gecode::ICL_DEF);
         else
           Gecode::linear(home, a, y, irt, c, Gecode::ICL_DEF);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         Gecode::BoolVarArgs y(x.size());
         for (int i=x.size(); i--; )
           y[i]=Gecode::channel(home,x[i]);
         if (one(a))
           Gecode::linear(home, y, irt, c, b, Gecode::ICL_DEF);
         else
           Gecode::linear(home, a, y, irt, c, b, Gecode::ICL_DEF);
       }
     };

     /// %Test linear relation over Boolean variables equal to integer variable
     class BoolVar : public Test {
     protected:
       /// Coefficients
       Gecode::IntArgs a;
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolVar(const std::string& s,
               int min, int max, const Gecode::IntArgs& a0,
               Gecode::IntRelType irt0)
         : Test("Linear::Bool::Var::"+str(irt0)+"::"+s,a0.size()+1,
                min,max,true),
           a(a0), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         for (int i=0; i<n; i++)
           if ((x[i] < 0) || (x[i] > 1))
             return false;
         double e = 0.0;
         for (int i=0; i<n; i++)
           e += a[i]*x[i];
         return cmp(e, irt, static_cast<double>(x[n]));
       }
       /// %Test whether \a x is to be ignore
       virtual bool ignore(const Assignment& x) const {
         for (int i=x.size()-1; i--; )
           if ((x[i] < 0) || (x[i] > 1))
             return true;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::BoolVarArgs y(n);
         for (int i=n; i--; )
           y[i]=Gecode::channel(home,x[i]);
         if (one(a))
           Gecode::linear(home, y, irt, x[n]);
         else
           Gecode::linear(home, a, y, irt, x[n]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         int n=x.size()-1;
         Gecode::BoolVarArgs y(n);
         for (int i=n; i--; )
           y[i]=Gecode::channel(home,x[i]);
         if (one(a))
           Gecode::linear(home, y, irt, x[n], b);
         else
           Gecode::linear(home, a, y, irt, x[n], b);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         {
           IntSet d1(-2,2);
           const int dv2[] = {-4,-1,0,1,4};
           IntSet d2(dv2,5);

           const int dv3[] = {0,1500000000};
           IntSet d3(dv3,2);

           IntArgs a1(1, 0);

           for (IntRelTypes irts; irts(); ++irts) {
             (void) new IntInt("11",d1,a1,irts.irt(),0);
             (void) new IntVar("11",d1,a1,irts.irt());
             (void) new IntInt("21",d2,a1,irts.irt(),0);
             (void) new IntVar("21",d2,a1,irts.irt());
             (void) new IntInt("31",d3,a1,irts.irt(),150000000);
           }
           (void) new IntInt("11",d1,a1,IRT_EQ,0,ICL_DOM);
           (void) new IntVar("11",d1,a1,IRT_EQ,ICL_DOM);
           (void) new IntInt("21",d2,a1,IRT_EQ,0,ICL_DOM);
           (void) new IntVar("21",d2,a1,IRT_EQ,ICL_DOM);

           const int av2[5] = {1,1,1,1,1};
           const int av3[5] = {1,-1,-1,1,-1};
           const int av4[5] = {2,3,5,7,11};
           const int av5[5] = {-2,3,-5,7,-11};

           for (int i=1; i<=5; i++) {
             IntArgs a2(i, av2);
             IntArgs a3(i, av3);
             IntArgs a4(i, av4);
             IntArgs a5(i, av5);
             for (IntRelTypes irts; irts(); ++irts) {
               (void) new IntInt("12",d1,a2,irts.irt(),0);
               (void) new IntInt("13",d1,a3,irts.irt(),0);
               (void) new IntInt("14",d1,a4,irts.irt(),0);
               (void) new IntInt("15",d1,a5,irts.irt(),0);
               (void) new IntInt("22",d2,a2,irts.irt(),0);
               (void) new IntInt("23",d2,a3,irts.irt(),0);
               (void) new IntInt("24",d2,a4,irts.irt(),0);
               (void) new IntInt("25",d2,a5,irts.irt(),0);
               (void) new IntInt("32",d3,a2,irts.irt(),1500000000);
               if (i < 5) {
                 (void) new IntVar("12",d1,a2,irts.irt());
                 (void) new IntVar("13",d1,a3,irts.irt());
                 (void) new IntVar("14",d1,a4,irts.irt());
                 (void) new IntVar("15",d1,a5,irts.irt());
                 (void) new IntVar("22",d2,a2,irts.irt());
                 (void) new IntVar("23",d2,a3,irts.irt());
                 (void) new IntVar("24",d2,a4,irts.irt());
                 (void) new IntVar("25",d2,a5,irts.irt());
               }
             }
             (void) new IntInt("12",d1,a2,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("13",d1,a3,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("14",d1,a4,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("15",d1,a5,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("22",d2,a2,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("23",d2,a3,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("24",d2,a4,IRT_EQ,0,ICL_DOM);
             (void) new IntInt("25",d2,a5,IRT_EQ,0,ICL_DOM);
             if (i < 4) {
               (void) new IntVar("12",d1,a2,IRT_EQ,ICL_DOM);
               (void) new IntVar("13",d1,a3,IRT_EQ,ICL_DOM);
               (void) new IntVar("14",d1,a4,IRT_EQ,ICL_DOM);
               (void) new IntVar("15",d1,a5,IRT_EQ,ICL_DOM);
             }
           }
         }
         {
           const int av1[10] = { 
             1, 1, 1, 1, 1, 1, 1, 1, 1, 1
           };
           const int av2[10] = {
             -1,-1,-1,-1,-1,-1,-1,-1,-1,-1
           };

           for (int i=1; i<=10; i += 3) {
             IntArgs a1(i, av1);
             IntArgs a2(i, av2);
             for (int c=0; c<=6; c++)
               for (IntRelTypes irts; irts(); ++irts) {
                 (void) new BoolInt("1",a1,irts.irt(),c);
                 (void) new BoolInt("2",a2,irts.irt(),-c);
               }
           }

           IntArgs a3(5, 1,2,3,4,5);
           IntArgs a4(5, -1,-2,-3,-4,-5);
           IntArgs a5(5, -1,-2,1,2,4);

           for (IntRelTypes irts; irts(); ++irts) {
             for (int c=0; c<=16; c++) {
               (void) new BoolInt("3",a3,irts.irt(),c);
               (void) new BoolInt("4",a4,irts.irt(),-c);
               (void) new BoolInt("5",a5,irts.irt(),c);
               (void) new BoolInt("6",a5,irts.irt(),-c);
             }
           }

           for (int i=1; i<=5; i += 2) {
             IntArgs a1(i, av1);
             IntArgs a2(i, av2);
             for (IntRelTypes irts; irts(); ++irts) {
               (void) new BoolVar("1::"+Test::str(i),0,5,a1,irts.irt());
               (void) new BoolVar("2::"+Test::str(i),-5,0,a2,irts.irt());
             }
           }

           IntArgs a6(4, 1,2,3,4);
           IntArgs a7(4, -1,-2,-3,-4);
           IntArgs a8(4, -1,-2,1,2);
           IntArgs a9(6, -1,-2,1,2,-3,3);

           for (IntRelTypes irts; irts(); ++irts) {
             (void) new BoolVar("6",0,10,a6,irts.irt());
             (void) new BoolVar("7",-10,0,a7,irts.irt());
             (void) new BoolVar("8",-3,3,a8,irts.irt());
             (void) new BoolVar("9",-3,3,a9,irts.irt());
           }

         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
