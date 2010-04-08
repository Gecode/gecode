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

   /// %Tests for basic setup
   namespace Basic {

     /**
      * \defgroup TaskTestIntBasic Basic setup
      * \ingroup TaskTestInt
      */
     //@{

     /// %Test whether testing infrastructure for integer variables works
     class Basic : public Test {
     public:
       /// Initialize test
       Basic(int n)
         : Test("Basic",3,-n,n,true) {}
       /// Initialize test
       Basic(Gecode::IntArgs& i)
         : Test("Basic",3,Gecode::IntSet(i),true) {}
       /// Check whether \a x is a solution
       virtual bool solution(const Assignment&) const {
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space&, Gecode::IntVarArray&) {
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray&,
                         Gecode::BoolVar b) {
         Gecode::rel(home, b, Gecode::IRT_EQ, 1);
       }
     };

     Gecode::IntArgs i(4, 1,2,3,4);
     Basic b1(3);
     Basic b2(i);
     //@}

   }
}}

// STATISTICS: test-int
