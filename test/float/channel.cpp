/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *     Vincent Barichard, 2012
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

#include "test/float.hh"

#include <gecode/minimodel.hh>

namespace Test { namespace Float {

   /// %Tests for channel constraints
   namespace Channel {

     /// %Test channel between float and integer
     class Int : public Test {
     public:
       /// Construct and register test
       Int(Gecode::FloatNum st)
         : Test("Channel::Int",2,-1,2,st,CPLT_ASSIGNMENT,false) {}
       /// Check whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         Gecode::FloatNum tmp;
         return (((modf(x[0].min(),&tmp)==0) ||
                  (modf(x[0].max(),&tmp)==0))
                 && (x[0]==x[1])) ? MT_TRUE : MT_FALSE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         using namespace Gecode;
         IntVar iv(home,-1000,1000);
         channel(home, x[0], iv);
         channel(home, iv, x[1]);
       }
     };

     /// %Test channel between float and Boolean
     class Bool : public Test {
     public:
       /// Construct and register test
       Bool(Gecode::FloatNum st)
         : Test("Channel::Bool",2,0,1,st,CPLT_ASSIGNMENT,false) {}
       /// Check whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         Gecode::FloatNum tmp;
         return (((modf(x[0].min(),&tmp)==0) ||
                  (modf(x[0].max(),&tmp)==0))
                 && (x[0]==x[1])) ? MT_TRUE : MT_FALSE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         using namespace Gecode;
         BoolVar bv(home,0,1);
         channel(home, x[0], bv);
         channel(home, bv, x[1]);
       }
     };

     Gecode::FloatNum step1 = 0.7;
     Gecode::FloatNum step2 = 0.1;

     Int ci(step1);
     Bool cb(step2);
     //@}

   }
}}

// STATISTICS: test-float

