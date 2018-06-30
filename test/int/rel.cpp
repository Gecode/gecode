/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

   /// %Tests for relation constraints
   namespace Rel {

     /**
      * \defgroup TaskTestIntRel Relation constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for simple relation involving integer variables
     class IntVarXY : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVarXY(Gecode::IntRelType irt0, int n, Gecode::IntPropLevel ipl)
         : Test("Rel::Int::Var::XY::"+str(irt0)+"::"+str(ipl)+"::"+str(n),
                n+1,-3,3,n==1,ipl),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x.size() == 2) {
           return cmp(x[0],irt,x[1]);
         } else {
           return cmp(x[0],irt,x[2]) && cmp(x[1],irt,x[2]);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (x.size() == 2) {
           rel(home, x[0], irt, x[1], ipl);
         } else {
           IntVarArgs y(2);
           y[0]=x[0]; y[1]=x[1];
           rel(home, y, irt, x[2], ipl);
         }
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 2);
         Gecode::rel(home, x[0], irt, x[1], r, ipl);
       }
     };

     /// %Test for simple relation involving shared integer variables
     class IntVarXX : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVarXX(Gecode::IntRelType irt0, Gecode::IntPropLevel ipl)
         : Test("Rel::Int::Var::XX::"+str(irt0)+"::"+str(ipl),
                1,-3,3,true,ipl),
           irt(irt0) {
         contest = ((irt != Gecode::IRT_LE) &&
                    (irt != Gecode::IRT_GR) &&
                    (irt != Gecode::IRT_NQ))
           ? CTL_DOMAIN : CTL_NONE;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return cmp(x[0],irt,x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::rel(home, x[0], irt, x[0], ipl);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::rel(home, x[0], irt, x[0], r, ipl);
       }
     };

     /// %Test for simple relation involving Boolean variables
     class BoolVarXY : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolVarXY(Gecode::IntRelType irt0, int n)
         : Test("Rel::Bool::Var::XY::"+str(irt0)+"::"+str(n),n+1,0,1,
                n==1),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x.size() == 2) {
           return cmp(x[0],irt,x[1]);
         } else {
           return cmp(x[0],irt,x[2]) && cmp(x[1],irt,x[2]);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (x.size() == 2) {
           rel(home, channel(home,x[0]), irt, channel(home,x[1]));
         } else {
           BoolVarArgs y(2);
           y[0]=channel(home,x[0]); y[1]=channel(home,x[1]);
           rel(home, y, irt, channel(home,x[2]));
         }
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 2);
         using namespace Gecode;
         rel(home,
             channel(home,x[0]), irt, channel(home,x[1]),
             r);
       }
     };

     /// %Test for simple relation involving shared Boolean variables
     class BoolVarXX : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolVarXX(Gecode::IntRelType irt0)
         : Test("Rel::Bool::Var::XX::"+str(irt0),1,0,1),
           irt(irt0) {
         contest = ((irt != Gecode::IRT_LE) &&
                    (irt != Gecode::IRT_GR) &&
                    (irt != Gecode::IRT_NQ))
           ? CTL_DOMAIN : CTL_NONE;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return cmp(x[0],irt,x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVar b = Gecode::channel(home,x[0]);
         Gecode::rel(home, b, irt, b);
       }
     };

     /// %Test for simple relation involving integer variable and integer constant
     class IntInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Integer constant
       int c;
     public:
       /// Create and register test
       IntInt(Gecode::IntRelType irt0, int n, int c0)
         : Test("Rel::Int::Int::"+str(irt0)+"::"+str(n)+"::"+str(c0),
                n,-3,3,n==1),
           irt(irt0), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x.size() == 1)
           return cmp(x[0],irt,c);
         else
           return cmp(x[0],irt,c) && cmp(x[1],irt,c);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (x.size() == 1)
           rel(home, x[0], irt, c);
         else
           rel(home, x, irt, c);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 1);
         Gecode::rel(home, x[0], irt, c, r);
       }
     };

     /// %Test for simple relation involving Boolean variable and integer constant
     class BoolInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// Integer constant
       int c;
     public:
       /// Create and register test
       BoolInt(Gecode::IntRelType irt0, int n, int c0)
         : Test("Rel::Bool::Int::"+str(irt0)+"::"+str(n)+"::"+str(c0),n,0,1,
                n==1),
           irt(irt0), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x.size() == 1)
           return cmp(x[0],irt,c);
         else
           return cmp(x[0],irt,c) && cmp(x[1],irt,c);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (x.size() == 1) {
           rel(home, channel(home,x[0]), irt, c);
         } else {
           BoolVarArgs y(2);
           y[0]=channel(home,x[0]); y[1]=channel(home,x[1]);
           rel(home, y, irt, c);
         }
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 1);
         using namespace Gecode;
         rel(home, channel(home,x[0]), irt, c, r);
       }
     };

     /// %Test for sequence of relations between integer variables
     class IntSeq : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntSeq(int n, Gecode::IntRelType irt0, Gecode::IntPropLevel ipl)
         : Test("Rel::Int::Seq::"+str(n)+"::"+str(irt0)+"::"+str(ipl),
                n,-3,3,false,ipl),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (irt == Gecode::IRT_NQ) {
           if (x.size() < 2)
             return false;
           for (int i=0; i<x.size()-1; i++)
             if (x[i] != x[i+1])
               return true;
           return false;
         } else {
           for (int i=0; i<x.size()-1; i++)
             if (!cmp(x[i],irt,x[i+1]))
               return false;
           return true;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::rel(home, x, irt, ipl);
       }
     };

     /// %Test for sequence of relations between shared integer variables
     class IntSharedSeq : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntSharedSeq(int n, Gecode::IntRelType irt0, Gecode::IntPropLevel ipl)
         : Test("Rel::Int::Seq::Shared::"+str(n)+"::"+str(irt0)+"::"+str(ipl),
                n,-3,3,false,ipl),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (irt == Gecode::IRT_NQ) {
           if (x.size() < 2)
             return false;
           for (int i=0; i<x.size()-1; i++)
             if (x[i] != x[i+1])
               return true;
           return false;
         } else {
           int n = x.size();
           for (int i=0; i<2*n-1; i++)
             if (!cmp(x[i % n],irt,x[(i+1) % n]))
               return false;
           return true;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size();
         IntVarArgs y(2*n);
         for (int i=n; i--; )
           y[i] = y[n+i] = x[i];
         rel(home, y, irt, ipl);
       }
     };

     /// %Test for sequence of relations between Boolean variables
     class BoolSeq : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolSeq(int n, Gecode::IntRelType irt0)
         : Test("Rel::Bool::Seq::"+str(n)+"::"+str(irt0),n,0,1),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (irt == Gecode::IRT_NQ) {
           if (x.size() < 2)
             return false;
           for (int i=0; i<x.size()-1; i++)
             if (x[i] != x[i+1])
               return true;
           return false;
         } else {
           for (int i=0; i<x.size()-1; i++)
             if (!cmp(x[i],irt,x[i+1]))
               return false;
           return true;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size());
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         rel(home, b, irt);
       }
     };

     /// %Test for sequence of relations between shared Boolean variables
     class BoolSharedSeq : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolSharedSeq(int n, Gecode::IntRelType irt0)
         : Test("Rel::Bool::Seq::Shared::"+str(n)+"::"+str(irt0),n,0,1),
           irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (irt == Gecode::IRT_NQ) {
           if (x.size() < 2)
             return false;
           for (int i=0; i<x.size()-1; i++)
             if (x[i] != x[i+1])
               return true;
           return false;
         } else {
           int n = x.size();
           for (int i=0; i<2*n-1; i++)
             if (!cmp(x[i % n],irt,x[(i+1) % n]))
               return false;
           return true;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size();
         BoolVarArgs b(2*n);
         for (int i=n; i--; )
           b[i]=b[n+i]=channel(home,x[i]);
         rel(home, b, irt);
       }
     };

     /// %Test for relation between same sized arrays of integer variables
     class IntArrayVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntArrayVar(Gecode::IntRelType irt0)
         : Test("Rel::Int::Array::Var::"+str(irt0),6,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size() >> 1;
         for (int i=0; i<n; i++)
           if (x[i] != x[n+i])
             return cmp(x[i],irt,x[n+i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size() >> 1;
         IntVarArgs y(n); IntVarArgs z(n);
         for (int i=0; i<n; i++) {
           y[i]=x[i]; z[i]=x[n+i];
         }
         rel(home, y, irt, z);
       }
     };

     /// %Test for relation between same sized arrays of integer variables and integers
     class IntArrayInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntArrayInt(Gecode::IntRelType irt0)
         : Test("Rel::Int::Array::Int::"+str(irt0),3,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         Gecode::IntArgs y({0,0,0});
         int n=x.size();
         for (int i=0; i<n; i++)
           if (x[i] != y[i])
             return cmp(x[i],irt,y[i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntArgs y({0,0,0});
         rel(home, x, irt, y);
       }
     };

     /// %Test for relation between differently sized arrays of integer variables
     class IntArrayDiff : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
       /// How many variables in total
       static const int n = 4;
       /// How big is the first array
       int n_fst;
     public:
       /// Create and register test
       IntArrayDiff(Gecode::IntRelType irt0, int m)
         : Test("Rel::Int::Array::"+str(irt0)+"::"+str(m)+"::"+str(n-m),
                n,-2,2),
           irt(irt0), n_fst(m) {
         assert(n_fst <= n);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n_snd = n - n_fst;
         for (int i=0; i<std::min(n_fst,n_snd); i++)
           if (x[i] != x[n_fst+i])
             return cmp(x[i],irt,x[n_fst+i]);
         return cmp(n_fst,irt,n_snd);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n_snd = n - n_fst;
         IntVarArgs y(n_fst); IntVarArgs z(n_snd);
         for (int i=0; i<n_fst; i++) {
           y[i]=x[i];
         }
         for (int i=0; i<n_snd; i++) {
           z[i]=x[n_fst + i];
         }
         rel(home, y, irt, z);
       }
     };

     /// %Test for relation between arrays of Boolean variables
     class BoolArrayVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolArrayVar(Gecode::IntRelType irt0)
         : Test("Rel::Bool::Array::Var::"+str(irt0),10,0,1), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size() >> 1;
         for (int i=0; i<n; i++)
           if (x[i] != x[n+i])
             return cmp(x[i],irt,x[n+i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size() >> 1;
         BoolVarArgs y(n); BoolVarArgs z(n);
         for (int i=0; i<n; i++) {
           y[i]=channel(home,x[i]); z[i]=channel(home,x[n+i]);
         }
         rel(home, y, irt, z);
       }
     };

     /// %Test for relation between arrays of Boolean variables and integers
     class BoolArrayInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolArrayInt(Gecode::IntRelType irt0)
         : Test("Rel::Bool::Array::Int::"+str(irt0),5,0,1), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         Gecode::IntArgs y({0,0,1,0,0});
         for (int i=0; i<5; i++)
           if (x[i] != y[i])
             return cmp(x[i],irt,y[i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::IntArgs z({0,0,1,0,0});
         int n=x.size();
         BoolVarArgs y(n);
         for (int i=0; i<n; i++)
           y[i]=channel(home,x[i]);
         rel(home, y, irt, z);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         for (IntRelTypes irts; irts(); ++irts) {
           for (IntPropLevels ipls; ipls(); ++ipls) {
             (void) new IntVarXY(irts.irt(),1,ipls.ipl());
             (void) new IntVarXY(irts.irt(),2,ipls.ipl());
             (void) new IntVarXX(irts.irt(),ipls.ipl());
             (void) new IntSeq(1,irts.irt(),ipls.ipl());
             (void) new IntSeq(2,irts.irt(),ipls.ipl());
             (void) new IntSeq(3,irts.irt(),ipls.ipl());
             (void) new IntSeq(5,irts.irt(),ipls.ipl());
             (void) new IntSharedSeq(1,irts.irt(),ipls.ipl());
             (void) new IntSharedSeq(2,irts.irt(),ipls.ipl());
             (void) new IntSharedSeq(3,irts.irt(),ipls.ipl());
             (void) new IntSharedSeq(4,irts.irt(),ipls.ipl());
           }
           (void) new BoolVarXY(irts.irt(),1);
           (void) new BoolVarXY(irts.irt(),2);
           (void) new BoolVarXX(irts.irt());
           (void) new BoolSeq(1,irts.irt());
           (void) new BoolSeq(2,irts.irt());
           (void) new BoolSeq(3,irts.irt());
           (void) new BoolSeq(10,irts.irt());
           (void) new BoolSharedSeq(1,irts.irt());
           (void) new BoolSharedSeq(2,irts.irt());
           (void) new BoolSharedSeq(3,irts.irt());
           (void) new BoolSharedSeq(4,irts.irt());
           (void) new BoolSharedSeq(8,irts.irt());
           for (int c=-4; c<=4; c++) {
             (void) new IntInt(irts.irt(),1,c);
             (void) new IntInt(irts.irt(),2,c);
           }
           for (int c=0; c<=1; c++) {
             (void) new BoolInt(irts.irt(),1,c);
             (void) new BoolInt(irts.irt(),2,c);
           }
           (void) new IntArrayVar(irts.irt());
           (void) new IntArrayInt(irts.irt());
           for (int n_fst=0; n_fst<=4; n_fst++)
             (void) new IntArrayDiff(irts.irt(),n_fst);
           (void) new BoolArrayVar(irts.irt());
           (void) new BoolArrayInt(irts.irt());
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
