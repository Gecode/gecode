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

#include <cmath>
#include <algorithm>

namespace Test { namespace Int {

   /// Tests for arithmetic constraints
   namespace Arithmetic {
   
     /**
      * \defgroup TaskTestIntArithmetic Arithmetic constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// Test for multiplication constraint
     class Mult : public Test {
     public:
       /// Create and register test
       Mult(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Mult::"+s,3,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[2]);
       }
     };
   
     /// Test for squaring constraint
     class Sqr : public Test {
     public:
       /// Create and register test
       Sqr(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Sqr::"+s,2,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return d0*d0 == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::sqr(home, x[0], x[1]);
       }
     };
   
     /// Test for square root constraint
     class Sqrt : public Test {
     public:
       /// Create and register test
       Sqrt(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Sqrt::"+s,2,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0 >= d1*d1) && (d0 < (d1+1)*(d1+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::sqrt(home, x[0], x[1]);
       }
     };
   
     /// Test for absolute value constraint
     class Abs : public Test {
     public:
       /// Create and register test
       Abs(const std::string& s, const Gecode::IntSet& d, Gecode::IntConLevel icl)
         : Test("Arithmetic::Abs::"+str(icl)+"::"+s,
                   2,d,false,icl) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0<0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::abs(home, x[0], x[1], icl);
       }
     };
   
     /// Test for binary minimum constraint  
     class Min : public Test {
     public:
       /// Create and register test
       Min(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Min::Bin::"+s,3,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[2]);
       }
     };
   
     /// Test for binary minimum constraint with shared variables
     class MinShared : public Test {
     public:
       /// Create and register test
       MinShared(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Min::Bin::Shared::"+s,2,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[0]);
       }
     };
   
     /// Test for binary maximum constraint  
     class Max : public Test {
     public:
       /// Create and register test
       Max(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Max::Bin::"+s,3,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[2]);
       }
     };
   
     /// Test for binary maximum constraint with shared variables
     class MaxShared : public Test {
     public:
       /// Create and register test
       MaxShared(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Max::Bin::Shared::"+s,2,d) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[0]);
       }
     };
   
     /// Test for n-ary minimmum constraint  
     class MinNary : public Test {
     public:
       /// Create and register test
       MinNary(void) : Test("Arithmetic::Min::Nary",4,-4,4) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[3]);
       }
     };
   
     /// Test for n-ary minimmum constraint with shared variables  
     class MinNaryShared : public Test {
     public:
       /// Create and register test
       MinNaryShared(void) : Test("Arithmetic::Min::Nary::Shared",3,-4,4) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[1]);
       }
     };
   
     /// Test for n-ary maximum constraint  
     class MaxNary : public Test {
     public:
       /// Create and register test
       MaxNary(void) : Test("Arithmetic::Max::Nary",4,-4,4) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[3]);
       }
     };
   
     /// Test for n-ary maximum constraint with shared variables
     class MaxNaryShared : public Test {
     public:
       /// Create and register test
       MaxNaryShared(void) : Test("Arithmetic::Max::Nary::Shared",3,-4,4) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[1]);
       }
     };
   
   
   
     const int v1[7] = {
       Gecode::Int::Limits::min, Gecode::Int::Limits::min+1,
       -1,0,1,
       Gecode::Int::Limits::max-1, Gecode::Int::Limits::max
     };
     const int v2[9] = {
       static_cast<int>(-sqrt(static_cast<double>
                              (-Gecode::Int::Limits::min))),
       -4,-2,-1,0,1,2,4,
       static_cast<int>(sqrt(static_cast<double>
                             (Gecode::Int::Limits::max)))
     };
     
     Gecode::IntSet d1(v1,7);
     Gecode::IntSet d2(v2,9);
     Gecode::IntSet d3(-8,8);
   
     Mult mult_max("A",d1);
     Mult mult_med("B",d2);
     Mult mult_min("C",d3);
   
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
   
     MinShared min_s_max("A",d1);
     MinShared min_s_med("B",d2);
     MinShared min_s_min("C",d3);
   
     Max max_max("A",d1);
     Max max_med("B",d2);
     Max max_min("C",d3);
   
     MaxShared max_s_max("A",d1);
     MaxShared max_s_med("B",d2);
     MaxShared max_s_min("C",d3);
   
     MinNary min_nary;
     MinNaryShared min_s_nary;
     MaxNary max_nary;
     MaxNaryShared max_s_nary;
     //@}
   
   }
}}

// STATISTICS: test-int
