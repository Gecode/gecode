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

namespace Test { namespace Int {

   /// %Tests for count constraints
   namespace Count {

     /**
      * \defgroup TaskTestIntCount Count constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test number of equal integers equal to integer
     class IntInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntInt(Gecode::IntRelType irt0)
         : Test("Count::Int::Int::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=x.size(); i--; )
           if (x[i] == 0)
             m++;
         return cmp(m,irt,2);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::count(home, x, 0, irt, 2);
       }
     };

     /// %Test number of integers contained in an integer set equal to integer
     class SetInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       SetInt(Gecode::IntRelType irt0)
         : Test("Count::Set::Int::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=x.size(); i--; )
           if ((x[i] >= -1) && (x[i] <= 1))
             m++;
         return cmp(m,irt,2);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntSet s(-1,1);
         Gecode::count(home, x, s, irt, 2);
       }
     };

     /// %Test number of equal integers equal to integer with duplicate variables
     class IntIntDup : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntIntDup(Gecode::IntRelType irt0)
         : Test("Count::Int::Int::Dup::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=x.size(); i--; )
           if (x[i] == 0)
             m += 2;
         return cmp(m,irt,4);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(8);
         for (int i=x.size(); i--; )
           y[i]=y[4+i]=x[i];
         Gecode::count(home, y, 0, irt, 4);
       }
     };

     /// %Test number of equal integers equal to integer variable
     class IntVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVar(Gecode::IntRelType irt0)
         : Test("Count::Int::Var::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == 0)
             m++;
         return cmp(m,irt,x[4]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::count(home, y, 0, irt, x[4]);
       }
     };

     /// %Test number of integers contained in an integer set equal to integer variable
     class SetVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       SetVar(Gecode::IntRelType irt0)
         : Test("Count::Set::Var::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if ((x[i] >= -1) && (x[i] <= 1))
             m++;
         return cmp(m,irt,x[4]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::IntSet s(-1,1);
         Gecode::count(home, y, s, irt, x[4]);
       }
     };

     Gecode::IntArgs ints(4, 1,0,3,2);

     /// %Test number of several equal integers equal to integer
     class IntArrayInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntArrayInt(Gecode::IntRelType irt0)
         : Test("Count::IntArray::Int::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == ints[i])
             m++;
         return cmp(m,irt,2);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::count(home, y, ints, irt, 2);
       }
     };

     /// %Test number of several equal integers equal to integer variable
     class IntArrayVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntArrayVar(Gecode::IntRelType irt0)
         : Test("Count::IntArray::Var::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == ints[i])
             m++;
         return cmp(m,irt,x[4]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::count(home, y, ints, irt, x[4]);
       }
     };

     /// %Test number of equal integers equal to integer variable with sharing
     class IntVarShared : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntVarShared(Gecode::IntRelType irt0)
         : Test("Count::Int::Var::Shared::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == 0)
             m++;
         return cmp(m,irt,x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::count(home, x, 0, irt, x[2]);
       }
     };

     /// %Test number of equal variables equal to integer variable
     class VarVar : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       VarVar(Gecode::IntRelType irt0)
         : Test("Count::Var::Var::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<3; i++)
           if (x[i] == x[3])
             m++;
         return cmp(m,irt,x[4]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(3);
         for (int i=0; i<3; i++)
           y[i]=x[i];
         Gecode::count(home, y, x[3], irt, x[4]);
       }
     };

     /// %Test number of equal variables equal to integer
     class VarInt : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       VarInt(Gecode::IntRelType irt0)
         : Test("Count::Var::Int::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<3; i++)
           if (x[i] == x[3])
             m++;
         return cmp(m,irt,2);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(3);
         for (int i=0; i<3; i++)
           y[i]=x[i];
         Gecode::count(home, y, x[3], irt, 2);
       }
     };

     /// %Test number of equal variables equal to integer variable with sharing
     class VarVarSharedA : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       VarVarSharedA(Gecode::IntRelType irt0)
         : Test("Count::Var::Var::Shared::A::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == x[1])
             m++;
         return cmp(m,irt,x[4]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::count(home, y, x[1], irt, x[4]);
       }
     };

     /// %Test number of equal variables equal to integer variable with sharing
     class VarVarSharedB : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       VarVarSharedB(Gecode::IntRelType irt0)
         : Test("Count::Var::Var::Shared::B::"+str(irt0),5,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == x[4])
             m++;
         return cmp(m,irt,x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs y(4);
         for (int i=0; i<4; i++)
           y[i]=x[i];
         Gecode::count(home, y, x[4], irt, x[3]);
       }
     };

     /// %Test number of equal variables equal to integer variable with sharing
     class VarVarSharedC : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       VarVarSharedC(Gecode::IntRelType irt0)
         : Test("Count::Var::Var::Shared::C::"+str(irt0),4,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int m = 0;
         for (int i=0; i<4; i++)
           if (x[i] == x[1])
             m++;
         return cmp(m,irt,x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::count(home, x, x[1], irt, x[3]);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (IntRelTypes irts; irts(); ++irts) {
           (void) new IntInt(irts.irt());
           (void) new SetInt(irts.irt());
           (void) new IntIntDup(irts.irt());
           (void) new IntVar(irts.irt());
           (void) new SetVar(irts.irt());
           (void) new IntArrayInt(irts.irt());
           (void) new IntArrayVar(irts.irt());
           (void) new IntVarShared(irts.irt());
           (void) new VarVar(irts.irt());
           (void) new VarInt(irts.irt());
           (void) new VarVarSharedA(irts.irt());
           (void) new VarVarSharedB(irts.irt());
           (void) new VarVarSharedC(irts.irt());
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int

