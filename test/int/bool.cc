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

#include "gecode/minimodel.hh"

namespace Test { namespace Int {

   /// Tests for Boolean constraints
   namespace Bool {

     inline int
     check(int x0, Gecode::BoolOpType op, int x1) {
       switch (op) {
       case Gecode::BOT_AND: return x0 & x1;
       case Gecode::BOT_OR:  return x0 | x1;
       case Gecode::BOT_IMP: return !x0 | x1;
       case Gecode::BOT_EQV: return x0 == x1;
       case Gecode::BOT_XOR: return x0 != x1;
       default: GECODE_NEVER;
       }
       GECODE_NEVER;
       return 0;
     }

     /**
      * \defgroup TaskTestIntBool Boolean constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// Test for binary Boolean operation
     class BinOp : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinOp(Gecode::BoolOpType op0) 
         : Test("Bool::Bin::"+str(op0),3,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == x[2];
       }
       /// Post constraint
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, 
             channel(home,x[0]), op, channel(home,x[1]), 
             channel(home,x[2]));
       }
     };
     
     /// Test for binary Boolean operation with constant
     class BinOpConst : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       BinOpConst(Gecode::BoolOpType op0, int c0) 
         : Test("Bool::Bin::"+str(op0)+"::"+str(c0),2,0,1), 
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, channel(home,x[0]), op, channel(home,x[1]), c);
       }
     };
     
     /// Test for Nary Boolean operation
     class NaryOp : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       NaryOp(Gecode::BoolOpType op0, int n) 
         : Test("Bool::Nary::"+str(op0)+"::"+str(n),n+1,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int b = check(x[0],op,x[1]);
         for (int i=2; i<x.size()-1; i++)
           b = check(b,op,x[i]);
         return b == x[x.size()-1];
       }
       /// Post constraint
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size()-1);
         for (int i=x.size()-1; i--; )
           b[i]=channel(home,x[i]);
         rel(home, op, b, channel(home,x[x.size()-1]));
       }
     };
     
     /// Test for Nary Boolean operation with constant
     class NaryOpConst : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       NaryOpConst(Gecode::BoolOpType op0, int n, int c0) 
         : Test("Bool::Nary::"+str(op0)+"::"+str(n)+"::"+str(c0),n,0,1), 
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int b = check(x[0],op,x[1]);
         for (int i=2; i<x.size(); i++)
           b = check(b,op,x[i]);
         return b == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size());
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         rel(home, op, b, c);
       }
     };
     
     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         for (BoolOpTypes bots; bots(); ++bots) {
           (void) new BinOp(bots.bot());
           (void) new BinOpConst(bots.bot(),0);
           (void) new BinOpConst(bots.bot(),1);
           (void) new NaryOp(bots.bot(),2);
           (void) new NaryOp(bots.bot(),6);
           (void) new NaryOp(bots.bot(),10);
           (void) new NaryOpConst(bots.bot(),2,0);
           (void) new NaryOpConst(bots.bot(),6,0);
           (void) new NaryOpConst(bots.bot(),10,0);
           (void) new NaryOpConst(bots.bot(),2,1);
           (void) new NaryOpConst(bots.bot(),6,1);
           (void) new NaryOpConst(bots.bot(),10,1);
         }
       }
     };
   
     Create c;
     //@}
   
   }
}}

// STATISTICS: test-int

