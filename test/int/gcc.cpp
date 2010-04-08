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

   /// %Tests for counting constraints (global cardinality)
   namespace GCC {

     /**
      * \defgroup TaskTestIntGCC Counting constraints (global cardinality)
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for integer cardinality with min and max for all variables
     class IntAllMinMax : public Test {
     public:
       /// Create and register test
       IntAllMinMax(Gecode::IntConLevel icl)
         : Test("GCC::Int::All::MinMax::"+str(icl),4,-1,3,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n[5];
         for (int i=5; i--; )
           n[i]=0;
         for (int i=x.size(); i--; )
           n[x[i]+1]++;
         if (n[2] > 0)
          return false;
         for (int i=5; i--;)
           if (n[i]>2)
             return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntArgs values(5);
         IntSet fixed(0,2);
         IntSetArgs cards(5);
         for (int i=0; i<5; i++) {
           values[i] = i-1; cards[i] = fixed;
         }
         cards[2] = IntSet(0,0);
         count(home, x, cards, values, icl);
       }
     };

     /// %Test for integer cardinality with min and max for all variables
     class IntAllMinMaxDef : public Test {
     public:
       /// Create and register test
       IntAllMinMaxDef(Gecode::IntConLevel icl)
         : Test("GCC::Int::All::MinMaxDef::"+str(icl),4,0,3,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n[4];
         for (int i=4; i--; )
           n[i]=0;
         for (int i=x.size(); i--; )
           n[x[i]]++;
         if (n[2] > 0)
           return false;
         for (int i=4; i--;)
           if (n[i]>2)
             return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntSet fixed(0,2);
         IntSetArgs cards(4);
         for (int i=0; i<4; i++) {
           cards[i] = fixed;
         }
         cards[2] = IntSet(0,0);
         count(home, x, cards, icl);
       }
     };

     /// %Test for integer cardinality with max cardinality for all variables
     class IntAllMax : public Test {
     public:
       /// Create and register test
       IntAllMax(Gecode::IntConLevel icl)
         : Test("GCC::Int::All::Max::"+str(icl), 4, 1,2, false, icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n[2];
         for (int i=2; i--; )
           n[i] = 0;
         for (int i=x.size(); i--; )
           n[x[i] - 1]++;
         if (n[0] != 2 || n[1] != 2)
           return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntArgs values(2, 1,2);
         Gecode::count(home, x, Gecode::IntSet(2,2), values, icl);
       }
     };


     /// %Test for integer cardinality for some variables
     template<bool hole>
     class IntSome : public Test {
     public:
       /// Create and register test
       IntSome(Gecode::IntConLevel icl)
         : Test(std::string("GCC::Int::Some::")+
                (hole ? "::Hole" : "::Full")+str(icl),4,1,4,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n[4];
         for (int i=4; i--; )
           n[i]=0;
         for (int i=x.size(); i--; )
           n[x[i]-1]++;
         if ((n[0] < 2) || (n[1] < 2) || (n[2] > 0) || (n[3] > 0))
           return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntArgs values(2, 1,2);
         Gecode::IntSet fixed;
         if (!hole) {
           fixed = IntSet(0,2);
         } else {
           int ish[] = {0,2};
           fixed = IntSet(ish,2);
         }
         Gecode::IntSetArgs cards(2);
         cards[0]=fixed; cards[1]=fixed;
         count(home, x, cards, values, icl);
       }
     };

     /// %Test for variable cardinality for all cardinality values
     class VarAll : public Test {
     protected:
       /// Number of non-cardinality variables
       static const int n = 4;
     public:
       /// Create and register test
       VarAll(Gecode::IntConLevel icl)
         : Test("GCC::Var::All::"+str(icl),7,0,2,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // Number of cardinality variables
         int m = x.size()-n;
         for (int i=0; i<n; i++)
           if ((x[i] < 0) || (x[i] > 2))
             return false;
         int* card = new int[m];
         for (int i=0; i<m; i++) {
           card[i] = 0;
           if ((x[n+i] < 0) || (x[n+i] > 2)) {
             delete [] card;
             return false;
           }
         }
         for (int i=0; i<n; i++)
           card[x[i]]++;
         for (int i=0; i<m; i++)
           if (card[i] != x[n+i]) {
             delete [] card;
             return false;
           }
         delete [] card;
         return true;
       }
       /// Post constraint on \a xy
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
         using namespace Gecode;
         // Number of cardinality variables
         int m = xy.size()-n;

         IntVarArgs x(n), y(m);
         for (int i=0; i<n; i++)
           x[i]=xy[i];
         for (int i=0; i<m; i++)
           y[i]=xy[n+i];
         count(home, x, y, icl);
       }
     };

     /// %Test for variable cardinality for some cardinality values
     class VarSome : public Test {
     protected:
       /// Number of non-cardinality variables
       int n;
       /// Arity beyond which to use randomized tests
       static const int randomArity = 7;
     public:
       /// Create and register test
       VarSome(std::string s, int n0, int min, int max,
               Gecode::IntConLevel icl)
         : Test("GCC::Var::Some::"+s+"::"+str(icl),
                n0+(max-min)+1,min,max,false,icl)
         , n(n0)
        {
          contest = CTL_NONE;
          if (arity>randomArity)
            testsearch = false;
        }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // Number of cardinality variables
         int m = x.size()-n;
         int* card = new int[m];
         for (int i=0; i<m; i++) {
           card[i] = 0;
           if ((x[n+i] < 0) || (x[n+i] > n)) {
             delete [] card;
             return false;
           }
         }
         for (int i=0; i<n; i++)
           card[x[i]-dom.min()]++;
         for (int i=0; i<m; i++)
           if (card[i] != x[n+i]) {
             delete [] card;
             return false;
           }
         delete [] card;
         return true;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         if (arity > randomArity)
           return new RandomAssignment(arity,dom,4000);
         else 
           return new CpltAssignment(arity,dom);
       }
       /// Post constraint on \a xy
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& xy) {
         using namespace Gecode;
         // Number of cardinality variables
         int m = xy.size()-n;
         IntVarArgs x(n), y(m);
         for (int i=0; i<n; i++)
           x[i]=xy[i];
         for (int i=0; i<m; i++)
           y[i]=xy[n+i];
         IntArgs values(m);
         for (int i=m; i--;)
           values[i] = i+dom.min();
         count(home,x,y,values,icl);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (IntConLevels icls; icls(); ++icls) {
           (void) new IntAllMinMax(icls.icl());
           (void) new IntAllMinMaxDef(icls.icl());
           (void) new IntAllMax(icls.icl());
           (void) new IntSome<false>(icls.icl());
           (void) new IntSome<true>(icls.icl());
           (void) new VarAll(icls.icl());
           (void) new VarSome("Small",2,-1,3,icls.icl());
           (void) new VarSome("Large",3,-1,4,icls.icl());
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
