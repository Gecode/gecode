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

namespace Test { namespace Int { namespace Arithmetic {

  /**
   * \defgroup TaskTestIntArithmetic Arithmetic constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Test for multiplication constraint
  class Mult : public IntTest {
  public:
    Mult(const char* t, const IntSet& d)
      : IntTest(t,3,d) {}
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      double d2 = static_cast<double>(x[2]);
      return d0*d1 == d2;
    }
    virtual void post(Space* home, IntVarArray& x) {
      mult(home, x[0], x[1], x[2]);
    }
  };

  /// Test for squaring constraint
  class Square : public IntTest {
  public:
    Square(const char* t, const IntSet& d)
      : IntTest(t,2,d) {}
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      return d0*d0 == d1;
    }
    virtual void post(Space* home, IntVarArray& x) {
      mult(home, x[0], x[0], x[1]);
    }
  };

  /// Test for absolute value constraint
  class Abs : public IntTest {
  public:
    Abs(const char* t, const IntSet& d, IntConLevel icl)
      : IntTest(t,2,d,false,icl) {}
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      return (d0<0 ? -d0 : d0) == d1;
    }
    virtual void post(Space* home, IntVarArray& x) {
      abs(home, x[0], x[1], icl);
    }
  };

  /// Test for binary minimum constraint  
  class Min : public IntTest {
  public:
    Min(const char* t, const IntSet& d)
      : IntTest(t,3,d) {}
    virtual bool solution(const Assignment& x) const {
      return std::min(x[0],x[1]) == x[2];
    }
    virtual void post(Space* home, IntVarArray& x) {
      min(home, x[0], x[1], x[2]);
    }
  };

  /// Test for binary minimum constraint with shared variables
  class MinShared : public IntTest {
  public:
    MinShared(const char* t, const IntSet& d)
      : IntTest(t,2,d) {}
    virtual bool solution(const Assignment& x) const {
      return std::min(x[0],x[1]) == x[0];
    }
    virtual void post(Space* home, IntVarArray& x) {
      min(home, x[0], x[1], x[0]);
    }
  };

  /// Test for binary maximum constraint  
  class Max : public IntTest {
  public:
    Max(const char* t, const IntSet& d)
      : IntTest(t,3,d) {}
    virtual bool solution(const Assignment& x) const {
      return std::max(x[0],x[1]) == x[2];
    }
    virtual void post(Space* home, IntVarArray& x) {
      max(home, x[0], x[1], x[2]);
    }
  };

  /// Test for binary maximum constraint with shared variables
  class MaxShared : public IntTest {
  public:
    MaxShared(const char* t, const IntSet& d)
      : IntTest(t,2,d) {}
    virtual bool solution(const Assignment& x) const {
      return std::max(x[0],x[1]) == x[0];
    }
    virtual void post(Space* home, IntVarArray& x) {
      max(home, x[0], x[1], x[0]);
    }
  };

  /// Test for n-ary minimmum constraint  
  class MinNary : public IntTest {
  public:
    MinNary(void)
      : IntTest("Arithmetic::Min::Nary",4,-4,4) {}
    virtual bool solution(const Assignment& x) const {
      return std::min(std::min(x[0],x[1]), x[2]) == x[3];
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      min(home, m, x[3]);
    }
  };

  /// Test for n-ary minimmum constraint with shared variables  
  class MinNaryShared : public IntTest {
  public:
    MinNaryShared(void)
      : IntTest("Arithmetic::Min::Nary::Shared",3,-4,4) {}
    virtual bool solution(const Assignment& x) const {
      return std::min(std::min(x[0],x[1]), x[2]) == x[1];
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      min(home, m, x[1]);
    }
  };

  /// Test for n-ary maximum constraint  
  class MaxNary : public IntTest {
  public:
    MaxNary(void)
      : IntTest("Arithmetic::Max::Nary",4,-4,4) {}
    virtual bool solution(const Assignment& x) const {
      return std::max(std::max(x[0],x[1]), x[2]) == x[3];
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      max(home, m, x[3]);
    }
  };

  /// Test for n-ary maximum constraint with shared variables
  class MaxNaryShared : public IntTest {
  public:
    MaxNaryShared(void)
      : IntTest("Arithmetic::Max::Nary::Shared",3,-4,4) {}
    virtual bool solution(const Assignment& x) const {
      return std::max(std::max(x[0],x[1]), x[2]) == x[1];
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      max(home, m, x[1]);
    }
  };



  const int v1[7] = {
    Gecode::Limits::Int::int_min, Gecode::Limits::Int::int_min+1,
    -1,0,1,
    Limits::Int::int_max-1, Limits::Int::int_max
  };
  const int v2[9] = {
    static_cast<int>(-sqrt(static_cast<double>
                           (-Gecode::Limits::Int::int_min))),
    -4,-2,-1,0,1,2,4,
    static_cast<int>(sqrt(static_cast<double>
                          (Gecode::Limits::Int::int_max)))
  };
  
  Gecode::IntSet d1(v1,7);
  Gecode::IntSet d2(v2,9);
  Gecode::IntSet d3(-8,8);

  Mult mult_max("Arithmetic::Mult::A",d1);
  Mult mult_med("Arithmetic::Mult::B",d2);
  Mult mult_min("Arithmetic::Mult::C",d3);

  Square square_max("Arithmetic::Square::A",d1);
  Square square_med("Arithmetic::Square::B",d2);
  Square square_min("Arithmetic::Square::C",d3);

  Abs abs_bnd_max("Arithmetic::Abs::Bnd::A",d1,ICL_BND);
  Abs abs_bnd_med("Arithmetic::Abs::Bnd::B",d2,ICL_BND);
  Abs abs_bnd_min("Arithmetic::Abs::Bnd::C",d3,ICL_BND);
  Abs abs_dom_max("Arithmetic::Abs::Dom::A",d1,ICL_DOM);
  Abs abs_dom_med("Arithmetic::Abs::Dom::B",d2,ICL_DOM);
  Abs abs_dom_min("Arithmetic::Abs::Dom::C",d3,ICL_DOM);

  Min min_max("Arithmetic::Min::Bin::A",d1);
  Min min_med("Arithmetic::Min::Bin::B",d2);
  Min min_min("Arithmetic::Min::Bin::C",d3);

  MinShared min_s_max("Arithmetic::Min::Bin::Shared::A",d1);
  MinShared min_s_med("Arithmetic::Min::Bin::Shared::B",d2);
  MinShared min_s_min("Arithmetic::Min::Bin::Shared::C",d3);

  Max max_max("Arithmetic::Max::Bin::A",d1);
  Max max_med("Arithmetic::Max::Bin::B",d2);
  Max max_min("Arithmetic::Max::Bin::C",d3);

  MaxShared max_s_max("Arithmetic::Max::Bin::Shared::A",d1);
  MaxShared max_s_med("Arithmetic::Max::Bin::Shared::B",d2);
  MaxShared max_s_min("Arithmetic::Max::Bin::Shared::C",d3);

  MinNary min_nary;
  MinNaryShared min_s_nary;
  MaxNary max_nary;
  MaxNaryShared max_s_nary;
  //@}

}}}

// STATISTICS: test-int
