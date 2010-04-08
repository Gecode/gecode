/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include "test/set.hh"

namespace Test { namespace Set {

   /// %Tests for synchronized execution
   namespace Exec {

     /**
      * \defgroup TaskTestSetExec Synchronized execution
      * \ingroup TaskTestSet
      */
     //@{
     /// Simple test for wait (set variables)
     class Wait : public SetTest {
     public:
       /// Create and register test
       Wait(int n) 
         : SetTest("Wait::"+str(n),n,Gecode::IntSet(0,n),false) {}
       /// Check whether \a x is solution
       virtual bool solution(const SetAssignment& x) const {
         (void) x;
         return true;
       }
       /// Post wait on \a x
       virtual void post(Gecode::Space& home, Gecode::SetVarArray& x,
                         Gecode::IntVarArray&) {
         if (x.size() > 1)
           Gecode::wait(home, x, &c);
         else
           Gecode::wait(home, x[0], &c);
       }
       /// Continuation to be executed
       static void c(Gecode::Space& _home) {
         SetTestSpace& home = static_cast<SetTestSpace&>(_home);
         for (int i=0; i<home.x.size(); i++)
           if (!home.x[i].assigned())
             home.fail();
       }
     };

     Wait w1(1), w2(2);

     //@}

   }

}}

// STATISTICS: test-int
