/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

  /// %Tests for minimal modelling constraints (counting)
   namespace MiniModelCount {

     /// Expand relation to abbreviation
     std::string expand(Gecode::IntRelType irt) {
       switch (irt) {
       case Gecode::IRT_EQ: return "Exactly";
       case Gecode::IRT_LQ: return "AtMost";
       case Gecode::IRT_GQ: return "AtLeast";
       default: GECODE_NEVER;
       }
       GECODE_NEVER;
       return "";
     }

     /**
      * \defgroup TaskTestIntMiniModelCount Minimal modelling constraints (counting)
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
         : Test("MiniModel::"+expand(irt0)+"::Int::Int",4,-2,2), irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,x,0,2); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,x,0,2); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,x,0,2); break;
         default: GECODE_NEVER;
         }
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
         : Test("MiniModel::"+expand(irt0)+"::Int::Var",5,-2,2), irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,y,0,x[4]); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,y,0,x[4]); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,y,0,x[4]); break;
         default: GECODE_NEVER;
         }
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
         : Test("MiniModel::"+expand(irt0)+"::Var::Var",5,-2,2), irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,y,x[3],x[4]); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,y,x[3],x[4]); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,y,x[3],x[4]); break;
         default: GECODE_NEVER;
         }
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
         : Test("MiniModel::"+expand(irt0)+"::Var::Int",4,-2,2), irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,y,x[3],2); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,y,x[3],2); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,y,x[3],2); break;
         default: GECODE_NEVER;
         }
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
         : Test("MiniModel::"+expand(irt0)+"::IntArray::Int",5,-2,2),
           irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,y,ints,2); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,y,ints,2); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,y,ints,2); break;
         default: GECODE_NEVER;
         }
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
         : Test("MiniModel::"+expand(irt0)+"::IntArray::Var",5,-2,2),
           irt(irt0) {}
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
         switch (irt) {
         case Gecode::IRT_EQ:
           Gecode::exactly(home,y,ints,x[4]); break;
         case Gecode::IRT_LQ:
           Gecode::atmost(home,y,ints,x[4]); break;
         case Gecode::IRT_GQ:
           Gecode::atleast(home,y,ints,x[4]); break;
         default: GECODE_NEVER;
         }
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (IntRelTypes irts; irts(); ++irts)
           if ((irts.irt() == Gecode::IRT_EQ) ||
               (irts.irt() == Gecode::IRT_LQ) ||
               (irts.irt() == Gecode::IRT_GQ)) {
             (void) new IntInt(irts.irt());
             (void) new IntVar(irts.irt());
             (void) new VarVar(irts.irt());
             (void) new VarInt(irts.irt());
             (void) new IntArrayInt(irts.irt());
             (void) new IntArrayVar(irts.irt());
           }
       }
     };

     Create c;
     //@}

   }

}}

// STATISTICS: test-minimodel
