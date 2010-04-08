/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

   /// %Tests for minimal modelling constraints (simple relations)
   namespace MiniModel {

     /**
      * \defgroup TaskTestIntMiniModelRel Minimal modelling constraints (relation)
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for relation between arrays of integer variables
     class IntLex : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       IntLex(Gecode::IntRelType irt0)
         : Test("MiniModel::Lex::Int::"+str(irt0),6,-2,2), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size() >> 1;
         for (int i=0; i<n; i++)
           if (x[i] != x[n+i])
             return cmp(x[i],irt,x[n+i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size() >> 1;
         IntVarArgs y(n); IntVarArgs z(n);
         for (int i=0; i<n; i++) {
           y[i]=x[i]; z[i]=x[n+i];
         }
         lex(home, y, irt, z);
       }
     };

     /// %Test for relation between arrays of Boolean variables
     class BoolLex : public Test {
     protected:
       /// Integer relation type to propagate
       Gecode::IntRelType irt;
     public:
       /// Create and register test
       BoolLex(Gecode::IntRelType irt0)
         : Test("MiniModel::Lex::Bool::"+str(irt0),10,0,1), irt(irt0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size() >> 1;
         for (int i=0; i<n; i++)
           if (x[i] != x[n+i])
             return cmp(x[i],irt,x[n+i]);
         return ((irt == Gecode::IRT_LQ) || (irt == Gecode::IRT_GQ) ||
                 (irt == Gecode::IRT_EQ));
         GECODE_NEVER;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         int n=x.size() >> 1;
         BoolVarArgs y(n); BoolVarArgs z(n);
         for (int i=0; i<n; i++) {
           y[i]=channel(home,x[i]); z[i]=channel(home,x[n+i]);
         }
         lex(home, y, irt, z);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         for (IntRelTypes irts; irts(); ++irts) {
           (void) new IntLex(irts.irt());
           (void) new BoolLex(irts.irt());
         }
       }
     };

     Create c;
     //@}

   }

}}

// STATISTICS: test-minimodel
