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

   /// %Tests for Boolean constraints
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
     /// %Test for binary Boolean operation
     class BinXYZ : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinXYZ(Gecode::BoolOpType op0)
         : Test("Bool::Bin::XYZ::"+str(op0),3,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == x[2];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home,
             channel(home,x[0]), op, channel(home,x[1]),
             channel(home,x[2]));
       }
     };

     /// %Test for binary Boolean operation with shared variables
     class BinXXY : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinXXY(Gecode::BoolOpType op0)
         : Test("Bool::Bin::XXY::"+str(op0),2,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[0]) == x[1];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVar b = channel(home,x[0]);
         rel(home, b, op, b, channel(home,x[1]));
       }
     };

     /// %Test for binary Boolean operation with shared variables
     class BinXYX : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinXYX(Gecode::BoolOpType op0)
         : Test("Bool::Bin::XYX::"+str(op0),2,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == x[0];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVar b = channel(home,x[0]);
         rel(home, b, op, channel(home,x[1]), b);
       }
     };

     /// %Test for binary Boolean operation with shared variables
     class BinXYY : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinXYY(Gecode::BoolOpType op0)
         : Test("Bool::Bin::XYY::"+str(op0),2,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == x[1];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVar b = channel(home,x[1]);
         rel(home, channel(home,x[0]), op, b, b);
       }
     };

     /// %Test for binary Boolean operation with shared variables
     class BinXXX : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       BinXXX(Gecode::BoolOpType op0)
         : Test("Bool::Bin::XXX::"+str(op0),1,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[0]) == x[0];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVar b = channel(home,x[0]);
         rel(home, b, op, b, b);
       }
     };

     /// %Test for binary Boolean operation with constant
     class BinConstXY : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       BinConstXY(Gecode::BoolOpType op0, int c0)
         : Test("Bool::Bin::XY::"+str(op0)+"::"+str(c0),2,0,1),
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[1]) == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, channel(home,x[0]), op, channel(home,x[1]), c);
       }
     };

     /// %Test for binary Boolean operation with shared variables and constant
     class BinConstXX : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       BinConstXX(Gecode::BoolOpType op0, int c0)
         : Test("Bool::Bin::XX::"+str(op0)+"::"+str(c0),1,0,1),
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return check(x[0],op,x[0]) == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVar b = channel(home,x[0]);
         rel(home, b, op, b, c);
       }
     };

     /// %Test for Nary Boolean operation
     class Nary : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       Nary(Gecode::BoolOpType op0, int n)
         : Test("Bool::Nary::"+str(op0)+"::"+str(n),n+1,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size()-1;
         int b = check(x[n-2],op,x[n-1]);
         for (int i=0; i<n-2; i++)
           b = check(x[i],op,b);
         return b == x[n];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size()-1);
         for (int i=x.size()-1; i--; )
           b[i]=channel(home,x[i]);
         rel(home, op, b, channel(home,x[x.size()-1]));
       }
     };

     /// %Test for Nary Boolean operation
     class NaryShared : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       NaryShared(Gecode::BoolOpType op0, int n)
         : Test("Bool::Nary::Shared::"+str(op0)+"::"+str(n),n,0,1),
           op(op0) {
         if ((op == Gecode::BOT_EQV) || (op == Gecode::BOT_XOR))
           testfix = false;
       }
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size();
         int b = check(x[n-2],op,x[n-1]);
         for (int i=0; i<n-2; i++)
           b = check(x[i],op,b);
         return b == x[n-1];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size());
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         rel(home, op, b, b[x.size()-1]);
       }
     };

     /// %Test for Nary Boolean operation with constant
     class NaryConst : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       NaryConst(Gecode::BoolOpType op0, int n, int c0)
         : Test("Bool::Nary::"+str(op0)+"::"+str(n)+"::"+str(c0),n,0,1),
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size();
         int b = check(x[n-2],op,x[n-1]);
         for (int i=0; i<n-2; i++)
           b = check(x[i],op,b);
         return b == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(x.size());
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         rel(home, op, b, c);
       }
     };


     /// %Test for Clause Boolean operation
     class ClauseXYZ : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       ClauseXYZ(Gecode::BoolOpType op0, int n)
         : Test("Bool::Clause::XYZ::"+str(op0)+"::"+str(n),n+1,0,1), op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = (x.size()-1) / 2;
         int b;
         if (n == 1) {
           b = check(x[0],op,!x[1]);
         } else {
           b = check(x[0],op,!x[n]);
           for (int i=1; i<n; i++)
             b = check(b,op,check(x[i],op,!x[n+i]));
         }
         return b == x[x.size()-1];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = (x.size()-1) / 2;
         BoolVarArgs a(n), b(n);
         for (int i=n; i--; ) {
           a[i]=channel(home,x[i]);
           b[i]=channel(home,x[i+n]);
         }
         clause(home, op, a, b, channel(home,x[x.size()-1]));
       }
     };

     /// %Test for Clause Boolean operation
     class ClauseXXYYX : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       ClauseXXYYX(Gecode::BoolOpType op0, int n)
         : Test("Bool::Clause::XXYYX::"+str(op0)+"::"+str(n),n,0,1),
           op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() / 2;
         int b;
         if (n == 1) {
           b = check(x[0],op,!x[1]);
         } else {
           b = check(x[0],op,!x[n]);
           for (int i=1; i<n; i++)
             b = check(b,op,check(x[i],op,!x[n+i]));
         }
         return b == x[0];
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size() / 2;
         BoolVarArgs a(2*n), b(2*n);
         for (int i=n; i--; ) {
           a[i]=a[i+n]=channel(home,x[i]);
           b[i]=b[i+n]=channel(home,x[i+n]);
         }
         clause(home, op, a, b, a[0]);
       }
     };

     /// %Test for Clause Boolean operation
     class ClauseXXY : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
     public:
       /// Construct and register test
       ClauseXXY(Gecode::BoolOpType op0, int n)
         : Test("Bool::Clause::XXY::"+str(op0)+"::"+str(n),n,0,1),
           op(op0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0] == 1) == (op == Gecode::BOT_OR);
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size() / 2;
         BoolVarArgs a(2*n), b(2*n);
         for (int i=n; i--; ) {
           a[i]=b[i+n]=channel(home,x[i]);
           b[i]=a[i+n]=channel(home,x[i+n]);
         }
         clause(home, op, a, b, a[0]);
       }
     };

     /// %Test for Clause Boolean operation with constant
     class ClauseConst : public Test {
     protected:
       /// Boolean operation type for test
       Gecode::BoolOpType op;
       /// Integer constant
       int c;
     public:
       /// Construct and register test
       ClauseConst(Gecode::BoolOpType op0, int n, int c0)
         : Test("Bool::Clause::"+str(op0)+"::"+str(n)+"::"+str(c0),n,0,1),
           op(op0), c(c0) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() / 2;
         int b;
         if (n == 1) {
           b = check(x[0],op,!x[1]);
         } else {
           b = check(x[0],op,!x[n]);
           for (int i=1; i<n; i++)
             b = check(b,op,check(x[i],op,!x[n+i]));
         }
         return b == c;
       }
       /// Post constraint
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size() / 2;
         BoolVarArgs a(n), b(n);
         for (int i=n; i--; ) {
           a[i]=channel(home,x[i]);
           b[i]=channel(home,x[i+n]);
         }
         clause(home, op, a, b, c);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         for (BoolOpTypes bots; bots(); ++bots) {
           (void) new BinXYZ(bots.bot());
           (void) new BinXXY(bots.bot());
           (void) new BinXYX(bots.bot());
           (void) new BinXYY(bots.bot());
           (void) new BinXXX(bots.bot());
           (void) new BinConstXY(bots.bot(),0);
           (void) new BinConstXY(bots.bot(),1);
           (void) new BinConstXX(bots.bot(),0);
           (void) new BinConstXX(bots.bot(),1);
           (void) new Nary(bots.bot(),2);
           (void) new Nary(bots.bot(),6);
           (void) new Nary(bots.bot(),10);
           (void) new NaryShared(bots.bot(),2);
           (void) new NaryShared(bots.bot(),6);
           (void) new NaryShared(bots.bot(),10);
           (void) new NaryConst(bots.bot(),2,0);
           (void) new NaryConst(bots.bot(),6,0);
           (void) new NaryConst(bots.bot(),10,0);
           (void) new NaryConst(bots.bot(),2,1);
           (void) new NaryConst(bots.bot(),6,1);
           (void) new NaryConst(bots.bot(),10,1);
           if ((bots.bot() == BOT_AND) || (bots.bot() == BOT_OR)) {
             (void) new ClauseXYZ(bots.bot(),2);
             (void) new ClauseXYZ(bots.bot(),6);
             (void) new ClauseXYZ(bots.bot(),10);
             (void) new ClauseXXYYX(bots.bot(),2);
             (void) new ClauseXXYYX(bots.bot(),6);
             (void) new ClauseXXYYX(bots.bot(),10);
             (void) new ClauseXXY(bots.bot(),2);
             (void) new ClauseXXY(bots.bot(),6);
             (void) new ClauseXXY(bots.bot(),10);
             (void) new ClauseConst(bots.bot(),2,0);
             (void) new ClauseConst(bots.bot(),6,0);
             (void) new ClauseConst(bots.bot(),10,0);
             (void) new ClauseConst(bots.bot(),2,1);
             (void) new ClauseConst(bots.bot(),6,1);
             (void) new ClauseConst(bots.bot(),10,1);
           }
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int

