/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

namespace Test { namespace Float {

   /// %Tests for basic setup
   namespace Basic {

     /**
      * \defgroup TaskTestFloatBasic Basic setup
      * \ingroup TaskTestFloat
      */
     //@{

     /// %Test whether testing infrastructure for float variables works
     class Basic : public Test {
     public:
       /// Initialize test
       Basic(Gecode::FloatNum n, Gecode::FloatNum s)
         : Test("Basic",3,-n,n,s,CPLT_ASSIGNMENT,true) {}
       /// Initialize test
       Basic(Gecode::FloatVal v, Gecode::FloatNum s)
          : Test("Basic",3,v,s,CPLT_ASSIGNMENT,true) {}
       /// Check whether \a x is a solution
       virtual MaybeType solution(const Assignment&) const {
         return MT_TRUE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space&, Gecode::FloatVarArray&) {
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray&,
                         Gecode::Reify r) {
         Gecode::rel(home, r.var(), Gecode::IRT_EQ, 1);
       }
     };

     namespace {
       Basic b1(3,1.5);
       Basic b2(Gecode::FloatVal(-2,10),1.5);
     }
     //@}

   }
}}

// STATISTICS: test-float
