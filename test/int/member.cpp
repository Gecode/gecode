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

   /// %Tests for membership constraints
   namespace Member {

     /**
      * \defgroup TaskTestIntMember Membership constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test membership for integer variables
     class Int : public Test {
     public:
       /// Create and register test
       Int(int n) : Test("Member::Int::"+str(n),n+1,0,n,true) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 1;
         for (int i=n; i--; )
           if (x[i] == x[n])
             return true;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size() - 1;
         IntVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         member(home, y, x[n]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         using namespace Gecode;
         int n = x.size() - 1;
         IntVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         member(home, y, x[n], b);
       }
     };

     /// %Test membership for Boolean variables
     class Bool : public Test {
     public:
       /// Create and register test
       Bool(int n) : Test("Member::Bool::"+str(n),n+1,0,1,true) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size() - 1;
         for (int i=n; i--; )
           if (x[i] == x[n])
             return true;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n = x.size() - 1;
         BoolVarArgs y(n);
         for (int i=n; i--; )
           y[i] = channel(home,x[i]);
         member(home, y, channel(home,x[n]));
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         using namespace Gecode;
         int n = x.size() - 1;
         BoolVarArgs y(n);
         for (int i=n; i--; )
           y[i] = channel(home,x[i]);
         member(home, y, channel(home,x[n]), b);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (int i=0; i<=4; i++) {
           (void) new Int(i);
           (void) new Bool(i);
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int

