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

  /// %Tests for minimal modelling constraints (arithmetic)
  namespace MiniModelArithmetic {

     /**
      * \defgroup TaskTestIntMiniModelArithmetic Minimal modelling constraints (arithmetic)
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for multiplication constraint
     class Mult : public Test {
     public:
       /// Create and register test
       Mult(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Mult::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, x[0] * x[1]), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for division constraint
     class Div : public Test {
     public:
       /// Create and register test
       Div(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Div::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1] != 0) && (x[0] / x[1] == x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, x[0] / x[1]), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for division constraint
     class Mod : public Test {
     public:
       /// Create and register test
       Mod(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Mod::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1] != 0) && (x[0] % x[1] == x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, x[0] % x[1]), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for addition constraint
     class Plus : public Test {
     public:
       /// Create and register test
       Plus(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Plus::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return ((d0+d1 >= Gecode::Int::Limits::min) &&
                 (d0+d1 <= Gecode::Int::Limits::max) &&
                 (d0+d1 == d2));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, x[0] + x[1]), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for subtraction constraint
     class Minus : public Test {
     public:
       /// Create and register test
       Minus(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Minus::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return ((d0-d1 >= Gecode::Int::Limits::min) &&
                 (d0-d1 <= Gecode::Int::Limits::max) &&
                 (d0-d1 == d2));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, x[0] - x[1]), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for sqr constraint
     class Sqr : public Test {
     public:
       /// Create and register test
       Sqr(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Sqr::"+s,2,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return d0*d0 == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, sqr(x[0])), IRT_EQ, x[1], ICL_DOM);
       }
     };

     /// %Test for sqrt constraint
     class Sqrt : public Test {
     public:
       /// Create and register test
       Sqrt(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Sqrt::"+s,2,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0 >= 0) && (d0 >= d1*d1) && (d0 < (d1+1)*(d1+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, sqrt(x[0])), IRT_EQ, x[1], ICL_DOM);
       }
     };

     /// %Test for absolute value constraint
     class Abs : public Test {
     public:
       /// Create and register test
       Abs(const std::string& s, const Gecode::IntSet& d, Gecode::IntConLevel icl)
         : Test("MiniModel::Abs::"+str(icl)+"::"+s,
                   2,d,false,icl) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0<0.0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, abs(x[0]), icl), IRT_EQ, x[1], ICL_DOM);
       }
     };

     /// %Test for binary minimum constraint
     class Min : public Test {
     public:
       /// Create and register test
       Min(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Min::Bin::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, min(x[0], x[1])), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for binary maximum constraint
     class Max : public Test {
     public:
       /// Create and register test
       Max(const std::string& s, const Gecode::IntSet& d)
         : Test("MiniModel::Max::Bin::"+s,3,d) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         rel(home, expr(home, max(x[0], x[1])), IRT_EQ, x[2], ICL_DOM);
       }
     };

     /// %Test for n-ary minimmum constraint
     class MinNary : public Test {
     public:
       /// Create and register test
       MinNary(void) : Test("MiniModel::Min::Nary",4,-4,4) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         rel(home, expr(home, min(m)), IRT_EQ, x[3], ICL_DOM);
       }
     };

     /// %Test for n-ary maximum constraint
     class MaxNary : public Test {
     public:
       /// Create and register test
       MaxNary(void) : Test("MiniModel::Max::Nary",4,-4,4) {
         testfix = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         rel(home, expr(home, max(m)), IRT_EQ, x[3], ICL_DOM);
       }
     };

     const int v1[7] = {
       Gecode::Int::Limits::min, Gecode::Int::Limits::min+1,
       -1,0,1,
       Gecode::Int::Limits::max-1, Gecode::Int::Limits::max
     };
     const int v2[9] = {
       static_cast<int>(-sqrt(static_cast<double>(-Gecode::Int::Limits::min))),
       -4,-2,-1,0,1,2,4,
       static_cast<int>(sqrt(static_cast<double>(Gecode::Int::Limits::max)))
     };

     Gecode::IntSet d1(v1,7);
     Gecode::IntSet d2(v2,9);
     Gecode::IntSet d3(-8,8);

     Mult mult_max("A",d1);
     Mult mult_med("B",d2);
     Mult mult_min("C",d3);

     Div div_max("A",d1);
     Div div_med("B",d2);
     Div div_min("C",d3);

     Mod mod_max("A",d1);
     Mod mod_med("B",d2);
     Mod mod_min("C",d3);

     Plus plus_max("A",d1);
     Plus plus_med("B",d2);
     Plus plus_min("C",d3);

     Minus minus_max("A",d1);
     Minus minus_med("B",d2);
     Minus minus_min("C",d3);

     Sqr sqr_max("A",d1);
     Sqr sqr_med("B",d2);
     Sqr sqr_min("C",d3);

     Sqrt sqrt_max("A",d1);
     Sqrt sqrt_med("B",d2);
     Sqrt sqrt_min("C",d3);

     Abs abs_bnd_max("A",d1,Gecode::ICL_BND);
     Abs abs_bnd_med("B",d2,Gecode::ICL_BND);
     Abs abs_bnd_min("C",d3,Gecode::ICL_BND);
     Abs abs_dom_max("A",d1,Gecode::ICL_DOM);
     Abs abs_dom_med("B",d2,Gecode::ICL_DOM);
     Abs abs_dom_min("C",d3,Gecode::ICL_DOM);

     Min min_max("A",d1);
     Min min_med("B",d2);
     Min min_min("C",d3);

     Max max_max("A",d1);
     Max max_med("B",d2);
     Max max_min("C",d3);

     MinNary min_nary;
     MaxNary max_nary;

     //@}
   }

}}

// STATISTICS: test-minimodel
