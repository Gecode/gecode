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

#include "gecode/minimodel.hh"

namespace Test { namespace Int { namespace MiniModel {

  /**
   * \defgroup TaskTestIntMiniModel Minimal modelling constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Test for equality
  class LinEqA : public IntTest {
  public:
    /// Create and register test
    LinEqA(void) : IntTest("MiniModel::Lin::Eq::A",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (2*x[0]+1) == (x[1]-1);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, 2*x[0]+1 == x[1]-1);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(2*x[0]+1 == x[1]-1,b)));
    }
  };

  /// Test for linear equality
  class LinEqB : public IntTest {
  public:
    /// Create and register test
    LinEqB(void) : IntTest("MiniModel::Lin::Eq::B",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (2*x[0]+1) == (x[1]-1);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, 2*x[0]+1-x[1] == -1);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(2*x[0]+1-x[1] == -1,b)));
    }
  };

  /// Test for linear equality
  class LinEqC : public IntTest {
  public:
    /// Create and register test
    LinEqC(void) : IntTest("MiniModel::Lin::Eq::C",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return -(1-(1-x[1])) == x[0];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, -(1-(1-x[1])) == x[0]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(-(1-(1-x[1])) == x[0],b)));
    }
  };

  /// Test for linear equality
  class LinEqD : public IntTest {
  public:
    /// Create and register test
    LinEqD(void) : IntTest("MiniModel::Lin::Eq::D",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return 2*(1-x[1]) == x[0];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, 2*(1-x[1]) == x[0]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(2*(1-x[1]) == x[0],b)));
    }
  };

  /// Test for linear equality
  class LinEqE : public IntTest {
  public:
    /// Create and register test
    LinEqE(void) : IntTest("MiniModel::Lin::Eq::E",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return -1==x[0];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, -1==x[0]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(-1==x[0],b)));
    }
  };

  /// Test for linear inequality
  class LinGr : public IntTest {
  public:
    /// Create and register test
    LinGr(void) : IntTest("MiniModel::Lin::Gr",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return 1>x[0];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, 1>x[0]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(1>x[0],b)));
    }
  };

  /// Test for linear inequality
  class LinLe : public IntTest {
  public:
    /// Create and register test
    LinLe(void) : IntTest("MiniModel::Lin::Le",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return 1<x[0];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::post(home, 1<x[0]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::post(home, Gecode::tt(eqv(1<x[0],b)));
    }
  };

  /// Test for linear equality over Boolean variables
  class LinEqBool : public IntTest {
  public:
    /// Create and register test
    LinEqBool(void) : IntTest("MiniModel::Lin::Eq::Bool",3,-2,2,false) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=x.size(); i--; )
        if ((x[i] < 0) || (x[i] > 1))
          return false;
      return (2*x[0]+1) == (x[1]+x[2]-1);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs y(x.size());
      for (int i=x.size(); i--; )
        y[i] = channel(home,x[i]);
      Gecode::post(home, 2*y[0]+1 == y[1]+y[2]-1);
    }
  };
  
  /// Test for posting linear expressions
  class LinExprInt : public IntTest {
  public:
    /// Create and register test
    LinExprInt(void) : IntTest("MiniModel::Lin::Expr::Int",4,-2,2,false) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return 2*x[0]+3*x[1]-x[2] == x[3];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVar y = Gecode::post(home, 2*x[0] + 3*x[1] - x[2]);
      rel(home,y,IRT_EQ,x[3],ICL_DOM);
    }
  };

  /// Test for posting linear expressions over Boolean variables
  class LinExprBool : public IntTest {
  public:
    /// Create and register test
    LinExprBool(void) : IntTest("MiniModel::Lin::Expr::Bool",4,-2,2,false) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=x.size()-1; i--; )
        if ((x[i] < 0) || (x[i] > 1))
          return false;
      return -2*x[0]+3*x[1]-x[2] == x[3];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs y(x.size()-1);
      for (int i=x.size()-1; i--; )
        y[i] = channel(home,x[i]);
      IntVar z = Gecode::post(home, -2*y[0] + 3*y[1] - y[2]);
      rel(home,z,Gecode::IRT_EQ,x[3]);
    }
  };

  /// Test Boolean expressions
  class BoolA : public IntTest {
  public:
    /// Create and register test
    BoolA(void) : IntTest("MiniModel::Bool::A",4,0,1) {}
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
  class BoolB : public IntTest {
  public:
    /// Create and register test
    BoolB(void) : IntTest("MiniModel::Bool::B",4,0,1) {}
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
  class BoolC : public IntTest {
  public:
    /// Create and register test
    BoolC(void) : IntTest("MiniModel::Bool::C",4,0,1) {}
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
  class BoolD : public IntTest {
  public:
    /// Create and register test
    BoolD(void) : IntTest("MiniModel::Bool::D",4,0,1) {}
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
  class BoolE : public IntTest {
  public:
    /// Create and register test
    BoolE(void) : IntTest("MiniModel::Bool::E",4,0,1) {}
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
  class BoolF : public IntTest {
  public:
    /// Create and register test
    BoolF(void) : IntTest("MiniModel::Bool::F",4,0,1) {}
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
  
  /// Test for multiplication constraint
  class Mult : public IntTest {
  public:
    /// Create and register test
    Mult(const std::string& s, const Gecode::IntSet& d)
      : IntTest("MiniModel::Mult::"+s,3,d) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      double d2 = static_cast<double>(x[2]);
      return d0*d1 == d2;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, mult(home, x[0], x[1]), IRT_EQ, x[2], ICL_DOM);
    }
  };

  /// Test for squaring constraint
  class Square : public IntTest {
  public:
    /// Create and register test
    Square(const std::string& s, const Gecode::IntSet& d)
      : IntTest("MiniModel::Square::"+s,2,d) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      return d0*d0 == d1;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, mult(home, x[0], x[0]), IRT_EQ, x[1], ICL_DOM);
    }
  };

  /// Test for absolute value constraint
  class Abs : public IntTest {
  public:
    /// Create and register test
    Abs(const std::string& s, const Gecode::IntSet& d, Gecode::IntConLevel icl)
      : IntTest("MiniModel::Abs::"+str(icl)+"::"+s,
                2,d,false,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      double d0 = static_cast<double>(x[0]);
      double d1 = static_cast<double>(x[1]);
      return (d0<0 ? -d0 : d0) == d1;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, abs(home, x[0], icl), IRT_EQ, x[1], ICL_DOM);
    }
  };

  /// Test for binary minimum constraint  
  class Min : public IntTest {
  public:
    /// Create and register test
    Min(const std::string& s, const Gecode::IntSet& d)
      : IntTest("MiniModel::Min::Bin::"+s,3,d) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return std::min(x[0],x[1]) == x[2];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, min(home, x[0], x[1]), IRT_EQ, x[2], ICL_DOM);
    }
  };

  /// Test for binary maximum constraint  
  class Max : public IntTest {
  public:
    /// Create and register test
    Max(const std::string& s, const Gecode::IntSet& d) 
      : IntTest("MiniModel::Max::Bin::"+s,3,d) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return std::max(x[0],x[1]) == x[2];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, max(home, x[0], x[1]), IRT_EQ, x[2], ICL_DOM);
    }
  };

  /// Test for n-ary minimmum constraint  
  class MinNary : public IntTest {
  public:
    /// Create and register test
    MinNary(void) : IntTest("MiniModel::Min::Nary",4,-4,4) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return std::min(std::min(x[0],x[1]), x[2]) == x[3];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      rel(home, min(home, m), IRT_EQ, x[3], ICL_DOM);
    }
  };

  /// Test for n-ary maximum constraint  
  class MaxNary : public IntTest {
  public:
    /// Create and register test
    MaxNary(void) : IntTest("MiniModel::Max::Nary",4,-4,4) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return std::max(std::max(x[0],x[1]), x[2]) == x[3];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs m(3);
      m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
      rel(home, max(home, m), IRT_EQ, x[3], ICL_DOM);
    }
  };

  LinEqA lineqa;
  LinEqB lineqb;
  LinEqC lineqc;
  LinEqD lineqd;
  LinEqE lineqe;

  LinGr lingr;
  LinLe linle;

  LinEqBool lineqbool;
  LinExprInt linexprint;
  LinExprBool linexprbool;

  BoolA boola;
  BoolB boolb;
  BoolC boolc;
  BoolD boold;
  BoolE boole;
  BoolF boolf;

  const int v1[7] = {
    Gecode::Limits::Int::int_min, Gecode::Limits::Int::int_min+1,
    -1,0,1,
    Gecode::Limits::Int::int_max-1, Gecode::Limits::Int::int_max
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

  Mult mult_max("A",d1);
  Mult mult_med("B",d2);
  Mult mult_min("C",d3);

  Square square_max("A",d1);
  Square square_med("B",d2);
  Square square_min("C",d3);

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

}}}

// STATISTICS: test-int
