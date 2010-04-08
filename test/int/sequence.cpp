/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     David Rijsman <David.Rijsman@quintiq.com>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     David Rijsman, 2009
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date$
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
#include <climits>

namespace Test { namespace Int {

   /// %Tests for sequence constraints
   namespace Sequence {

     /**
      * \defgroup TaskTestIntSequence Sequence constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Base test for sequence
     class SequenceTest : public Test {
     protected:
       Gecode::IntSet s;
       int q,l,u;
     public:
       /// Create and register test
       SequenceTest(const std::string& s, 
                    const Gecode::IntSet& s0, int q0, int l0, int u0, 
                    int size, int min, int max)
         : Test("Sequence::"+s,size,min,max), s(s0), q(q0), l(l0), u(u0) {
       }
       /// %Test whether \a x is solutionin
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i< (x.size() - q + 1); i++ ) {
           int total = 0;
           for (int j=i; j < i + q; j++ ) {
             if (s.in(x[j]))
               total++;
             if (total > u)
               return false;
           }
           if ( total < l )
             return false;
         }
         return true;
       }
     };
     
     
     /// %Test for sequence with boolean variables
     class SequenceBoolTest : public SequenceTest {
     public:
       /// Create and register test
       SequenceBoolTest(const std::string& s, const Gecode::IntSet& s0, 
                        int q0, int l0, int u0, int size)
         : SequenceTest("Bool::"+s,s0,q0,l0,u0,size,0,1) {
       }
       
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVarArgs c(x.size());
         
         for (int i=0; i<x.size(); i++) {
           c[i]=Gecode::channel(home,x[i]);
         }
         
         Gecode::sequence(home,c,s,q,l,u);
       }
     };
     
     /// %Test for sequence with boolean variables
     class SequenceIntTest : public SequenceTest {
     public:
       /// Create and register test
       SequenceIntTest(const std::string& s, const Gecode::IntSet& s0,
                       int q0, int l0, int u0, int size, int min, int max)
         : SequenceTest("Int::"+s,s0,q0,l0,u0,size,min,max) {
       }
       
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sequence(home,x,s,q,l,u);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:

       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;

         IntSet a(0,0);
         IntSet b(1,1);
         IntSet c(2,2);
         IntSet d(0,1);
         IntArgs ie(2, 0,2);
         IntSet e(ie);

         (void) new SequenceBoolTest("A",a,3,2,2,6);
         (void) new SequenceBoolTest("B",b,3,2,2,6);
         (void) new SequenceBoolTest("C",b,6,2,2,6);
         (void) new SequenceBoolTest("D",b,6,0,0,6);
         (void) new SequenceBoolTest("E",b,6,6,6,6);


         (void) new SequenceIntTest ("A",c,3,2,2,6,2,3);
         (void) new SequenceIntTest ("B",c,3,2,2,6,2,4);
         (void) new SequenceIntTest ("C",b,3,2,2,6,1,3);
         (void) new SequenceIntTest ("D",c,3,0,0,3,1,3);
         (void) new SequenceIntTest ("E",c,3,3,3,3,1,3);
         (void) new SequenceIntTest ("F",c,3,2,2,10,2,3);

         (void) new SequenceIntTest ("G",d,3,2,2,6,0,3);
         (void) new SequenceIntTest ("H",d,3,2,2,6,0,4);
         (void) new SequenceIntTest ("I",d,3,2,2,6,1,3);
         (void) new SequenceIntTest ("J",e,3,0,0,6,0,3);
         (void) new SequenceIntTest ("K",e,3,3,3,6,0,3);
         (void) new SequenceIntTest ("L",e,3,2,2,6,0,3);

       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
