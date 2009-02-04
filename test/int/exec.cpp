/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  Last modified:
 *     $Date: 2009-01-20 23:44:27 +0100 (Tue, 20 Jan 2009) $ by $Author: schulte $
 *     $Revision: 8082 $
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

   /// Tests for synchronized execution
   namespace Exec {

     /**
      * \defgroup TaskTestIntExec Synchronized execution
      * \ingroup TaskTestInt
      */
     //@{
     /// Simple test for when
     class When : public Test {
     public:
       /// Create and register test
       When(void) : Test("When",1,0,1,false) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return x[0]==0;
       }
       /// Post when on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::when(home, Gecode::channel(home, x[0]), &t, &e);
       }
       /// Then-function to be executed
       static void t(Gecode::Space& home) {
         home.fail();
       }
       /// Else-function to be executed
       static void e(Gecode::Space& home) {
       }
     };

     When w;
     //@}

   }
}}

// STATISTICS: test-int
