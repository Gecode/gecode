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

   /// %Tests for domain constraints
   namespace Dom {

     /**
      * \defgroup TaskTestIntDom Domain constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for domain constraint (integer)
     class DomInt : public Test {
     public:
       /// Create and register test
       DomInt(int n) : Test("Dom::Int::"+str(n),n,-4,4,n == 1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if (x[i] != -2)
             return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (x.size() == 1)
           Gecode::dom(home, x[0], -2);
         else
           Gecode::dom(home, x, -2);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         assert(x.size() == 1);
         Gecode::dom(home, x[0], -2, b);
       }
     };


     /// %Test for domain constraint (range)
     class DomRange : public Test {
     public:
       /// Create and register test
       DomRange(int n) : Test("Dom::Range::"+str(n),n,-4,4,n == 1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if ((x[i] < -2) || (x[i] > 2))
             return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (x.size() == 1)
           Gecode::dom(home, x[0], -2, 2);
         else
           Gecode::dom(home, x, -2, 2);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         assert(x.size() == 1);
         Gecode::dom(home, x[0], -2, 2, b);
       }
     };

     /// %Test for domain constraint (empty range)
     class DomRangeEmpty : public Test {
     public:
       /// Create and register test
       DomRangeEmpty(void) : Test("Dom::Range::Empty",1,-4,4,true) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment&) const {
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray&) {
         home.fail();
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         Gecode::dom(home, x[0], 3, 2, b);
       }
     };


     const int r[4][2] = {
       {-4,-3},{-1,-1},{1,1},{3,5}
     };
     Gecode::IntSet d(r,4);

     /// %Test for domain constraint (full integer set)
     class DomDom : public Test {
     public:
       /// Create and register test
       DomDom(int n) : Test("Dom::Dom::"+str(n),n,-6,6,n == 1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if (!(((x[i] >= -4) && (x[i] <= -3)) ||
                 ((x[i] >= -1) && (x[i] <= -1)) ||
                 ((x[i] >=  1) && (x[i] <=  1)) ||
                 ((x[i] >=  3) && (x[i] <=  5))))
             return false;
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (x.size() == 1)
           Gecode::dom(home, x[0], d);
         else
           Gecode::dom(home, x, d);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::BoolVar b) {
         assert(x.size() == 1);
         Gecode::dom(home, x[0], d, b);
       }
     };

     DomInt di1(1);
     DomInt di3(3);
     DomRange dr1(1);
     DomRange dr3(3);
     DomDom dd1(1);
     DomDom dd3(3);
     DomRangeEmpty dre;
     //@}

   }
}}

// STATISTICS: test-int

