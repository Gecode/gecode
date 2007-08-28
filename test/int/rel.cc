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

namespace Test { namespace Int { namespace Rel {

  /**
   * \defgroup TaskTestIntRel Relation constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Test for equality constraint
  class EqBin : public IntTest {
  public:
    /// Create and register test
    EqBin(Gecode::IntConLevel icl)
      : IntTest("Rel::Eq::Bin::"+str(icl),2,-2,2,true,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0]==x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_EQ, x[1], icl);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_EQ, x[1], b, icl);
    }
  };

  /// Test for equal to integer value constraint
  class EqBinInt : public IntTest {
  public:
    /// Create and register test
    EqBinInt(Gecode::IntConLevel icl)
      : IntTest("Rel::Eq::Bin::Int::"+str(icl),1,-2,2,true,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0]==0;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_EQ, 0, icl);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_EQ, 0, b, icl);
    }
  };

  /// Test for n-ary equality constraint
  class EqNary : public IntTest {
  public:
    /// Create and register test
    EqNary(Gecode::IntConLevel icl)
      : IntTest("Rel::Eq::Nary::"+str(icl),4,-2,2,false,icl) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (x[0]==x[1]) && (x[1]==x[2]) && (x[2]==x[3]);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x, Gecode::IRT_EQ, icl);
    }
  };
  
  class Nq : public IntTest {
  private:
    Gecode::IntConLevel icl;
  public:
    /// Create and register test
    Nq(const char* t, Gecode::IntConLevel icl0)
      : IntTest(t,2,-2,2,true), icl(icl0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0]!=x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_NQ, x[1], icl);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_NQ, x[1], b, icl);
    }
  };
  Nq nqbnd("Rel::NqBnd",Gecode::ICL_BND);
  Nq nqdom("Rel::NqDom",Gecode::ICL_DOM);


  class Lq : public IntTest {
  public:
    /// Create and register test
    Lq(void)
      : IntTest("Rel::Lq",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] <= x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_LQ, x[1]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_LQ, x[1], b);
    }
  };
  Lq lq;

  class LqInt : public IntTest {
  public:
    /// Create and register test
    LqInt(void)
      : IntTest("Rel::LqInt",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] <= 0;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_LQ, 0);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_LQ, 0, b);
    }
  };
  LqInt lqint;


  class Le : public IntTest {
  public:
    /// Create and register test
    Le(void)
      : IntTest("Rel::Le",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] < x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_LE, x[1]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_LE, x[1], b);
    }
  };
  Le le;

  class LeInt : public IntTest {
  public:
    /// Create and register test
    LeInt(void)
      : IntTest("Rel::LeInt",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] < 0;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_LE, 0);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_LE, 0, b);
    }
  };
  LeInt leint;

  class Gq : public IntTest {
  public:
    /// Create and register test
    Gq(void)
      : IntTest("Rel::Gq",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] >= x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_GQ, x[1]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_GQ, x[1], b);
    }
  };
  Gq gq;

  class GqInt : public IntTest {
  public:
    /// Create and register test
    GqInt(void)
      : IntTest("Rel::GqInt",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] >= 0;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_GQ, 0);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_GQ, 0, b);
    }
  };
  GqInt gqint;


  class Gr : public IntTest {
  public:
    /// Create and register test
    Gr(void)
      : IntTest("Rel::Gr",2,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] > x[1];
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_GR, x[1]);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_GR, x[1], b);
    }
  };
  Gr gr;

  class GrInt : public IntTest {
  public:
    /// Create and register test
    GrInt(void)
      : IntTest("Rel::GrInt",1,-2,2,true) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0] > 0;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], Gecode::IRT_GR, 0);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
      Gecode::rel(home, x[0], Gecode::IRT_GR, 0, b);
    }
  };
  GrInt grint;

  class LexInt : public IntTest {
  private:
    int  n;
    bool strict;
  public:
    /// Create and register test
    LexInt(const char* t, int m, bool _strict)
      : IntTest(t,m*2,-2,2), n(m), strict(_strict) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<n; i++)
        if (x[i]<x[n+i]) {
          return true;
        } else if (x[i]>x[n+i]) {
          return false;
        }
      return !strict;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs y(n); IntVarArgs z(n);
      for (int i=0; i<n; i++) {
        y[i]=x[i]; z[i]=x[n+i];
      }
      rel(home, y, strict ? IRT_LE : IRT_LQ, z);
    }
  };
  LexInt lexlqi("Lex::Lq::Int",3,false);
  LexInt lexlei("Lex::Le::Int",3,true);

  class LexBool : public IntTest {
  private:
    int  n;
    bool strict;
  public:
    /// Create and register test
    LexBool(const char* t, int m, bool _strict)
      : IntTest(t,m*2,0,1), n(m), strict(_strict) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<n; i++)
        if (x[i]<x[n+i]) {
          return true;
        } else if (x[i]>x[n+i]) {
          return false;
        }
      return !strict;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs y(n); BoolVarArgs z(n);
      for (int i=0; i<n; i++) {
        y[i]=channel(home,x[i]);; z[i]=channel(home,x[n+i]);
      }
      rel(home, y, strict ? IRT_LE : IRT_LQ, z);
    }
  };
  LexBool lexlq("Lex::Lq::Bool",3,false);
  LexBool lexle("Lex::Le::Bool",3,true);


  class NaryNq : public IntTest {
  private:
    int  n;
  public:
    /// Create and register test
    NaryNq(const char* t, int m)
      : IntTest(t,m*2,-2,2), n(m) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<n; i++)
        if (x[i] != x[n+i])
          return true;
      return false;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      IntVarArgs y(n); IntVarArgs z(n);
      for (int i=0; i<n; i++) {
        y[i]=x[i]; z[i]=x[n+i];
      }
      rel(home, y, IRT_NQ, z);
    }
  };
  NaryNq _nnq("NaryNq",3);

  EqBin eqbinbnd(Gecode::ICL_BND);
  EqBin eqbindom(Gecode::ICL_DOM);

  EqBinInt eqbinbndint(Gecode::ICL_BND);
  EqBinInt eqbindomint(Gecode::ICL_DOM);

  EqNary eqnarybnd(Gecode::ICL_BND);
  EqNary eqnarydom(Gecode::ICL_DOM);
  //@}

}}}

// STATISTICS: test-int
