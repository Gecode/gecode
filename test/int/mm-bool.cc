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

#include "gecode/minimodel.hh"

namespace Test { namespace Int {

   /// Tests for minimal modelling constraints
   namespace MiniModel {
   
     /**
      * \defgroup TaskTestIntMiniModelBool Minimal modelling constraints (Boolean conbstraints)
      * \ingroup TaskTestInt
      */
     //@{
     /// Test Boolean expressions
     class BoolA : public Test {
     public:
       /// Create and register test
       BoolA(void) : Test("MiniModel::Bool::A",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0]==1) && (x[1]==1)) || ((x[2]==1)!=(x[3]==1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, tt((b[0] && b[1]) || !eqv(b[2],b[3])));
       }
     };
   
     /// Test Boolean expressions
     class BoolB : public Test {
     public:
       /// Create and register test
       BoolB(void) : Test("MiniModel::Bool::B",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, tt((b[0] && b[1]) && (b[2] && b[3])));
       }
     };
   
     /// Test Boolean expressions
     class BoolC : public Test {
     public:
       /// Create and register test
       BoolC(void) : Test("MiniModel::Bool::C",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, tt(!!(b[0] && b[1]) && !(!b[2] || !b[3])));
       }
     };
   
     /// Test Boolean expressions
     class BoolD : public Test {
     public:
       /// Create and register test
       BoolD(void) : Test("MiniModel::Bool::D",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0]&x[1])==x[2]) && (x[0]==x[3]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, tt(eqv(b[0] && b[1], b[2]) && eqv(b[0],b[3])));
       }
     };
   
     /// Test Boolean expressions
     class BoolE : public Test {
     public:
       /// Create and register test
       BoolE(void) : Test("MiniModel::Bool::E",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return !(((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, ff(b[0] && b[1] && b[2] && b[3]));
       }
     };
   
     /// Test Boolean expressions
     class BoolF : public Test {
     public:
       /// Create and register test
       BoolF(void) : Test("MiniModel::Bool::F",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0]==1) || (x[1]==1)) || ((x[2]==1) || (x[3]==1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs b(4);
         for (int i=x.size(); i--; )
           b[i]=channel(home,x[i]);
         Gecode::post(home, tt(b[0] || b[1] || b[2] || b[3]));
       }
     };
   
     BoolA boola;
     BoolB boolb;
     BoolC boolc;
     BoolD boold;
     BoolE boole;
     BoolF boolf;
   
     //@}
   
   }
}}

// STATISTICS: test-int
