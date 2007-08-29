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
  /// Test for simple relation involving two integer variables
  class IntBinVar : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    IntBinVar(Gecode::IntRelType irt0, 
              Gecode::IntConLevel icl=Gecode::ICL_DEF)
      : IntTest("Rel::Int::Var::"+str(irt0)+"::"+str(icl),
                2,-3,3,true,icl), 
        irt(irt0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return cmp(x[0],irt,x[1]);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], irt, x[1], icl);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::rel(home, x[0], irt, x[1], b, icl);
    }
  };

  /// Test for simple relation involving two Boolean variables
  class BoolBinVar : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    BoolBinVar(Gecode::IntRelType irt0) 
      : IntTest("Rel::Bool::Var::"+str(irt0),2,0,1), irt(irt0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return cmp(x[0],irt,x[1]);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, channel(home,x[0]), irt, channel(home,x[1]));
    }
  };

  /// Test for simple relation involving integer variable and integer constant
  class IntBinInt : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
    /// Integer constant
    int c;
  public:
    /// Create and register test
    IntBinInt(Gecode::IntRelType irt0, int c0) 
      : IntTest("Rel::Int::Int::"+str(irt0)+"::"+str(c0),1,-3,3,true), 
        irt(irt0), c(c0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return cmp(x[0],irt,c);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x[0], irt, c);
    }
    /// Post reified constraint on \a x for \a b
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                      Gecode::BoolVar b) {
      Gecode::rel(home, x[0], irt, c, b);
    }
  };

  /// Test for simple relation involving Boolean variable and integer constant
  class BoolBinInt : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
    /// Integer constant
    int c;
  public:
    /// Create and register test
    BoolBinInt(Gecode::IntRelType irt0, int c0) 
      : IntTest("Rel::Bool::Int::"+str(irt0)+"::"+str(c0),1,0,1), 
        irt(irt0), c(c0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return cmp(x[0],irt,c);
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, Gecode::channel(home,x[0]), irt, c);
    }
  };

  /// Test for pairwise relation between integer variables
  class IntPairwise : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    IntPairwise(Gecode::IntRelType irt0, Gecode::IntConLevel icl)
      : IntTest("Rel::Int::Pairwise::"+str(irt0)+"::"+str(icl),
                4,-3,3,false,icl), 
        irt(irt0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (!cmp(x[i],irt,x[j]))
            return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      Gecode::rel(home, x, irt, icl);
    }
  };

  /// Test for pairwise relation between Boolean variables
  class BoolPairwise : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    BoolPairwise(Gecode::IntRelType irt0)
      : IntTest("Rel::Bool::Pairwise::"+str(irt0),8,0,1), 
        irt(irt0) {}
    /// Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (!cmp(x[i],irt,x[j]))
            return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs b(x.size());
      for (int i=x.size(); i--; )
        b[i]=channel(home,x[i]);
      rel(home, b, irt);
    }
  };

  /// Test for relation between arrays of integer variables
  class IntArray : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    IntArray(Gecode::IntRelType irt0)
      : IntTest("Rel::Int::Array::"+str(irt0),6,-2,2), irt(irt0) {}
    /// Test whether \a x is solution
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
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      int n=x.size() >> 1;
      IntVarArgs y(n); IntVarArgs z(n);
      for (int i=0; i<n; i++) {
        y[i]=x[i]; z[i]=x[n+i];
      }
      rel(home, y, irt, z);
    }
  };

  /// Test for relation between arrays of Boolean variables
  class BoolArray : public IntTest {
  protected:
    /// Integer relation type to propagate
    Gecode::IntRelType irt;
  public:
    /// Create and register test
    BoolArray(Gecode::IntRelType irt0)
      : IntTest("Rel::Bool::Array::"+str(irt0),10,0,1), irt(irt0) {}
    /// Test whether \a x is solution
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
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      int n=x.size() >> 1;
      BoolVarArgs y(n); BoolVarArgs z(n);
      for (int i=0; i<n; i++) {
        y[i]=channel(home,x[i]); z[i]=channel(home,x[n+i]);
      }
      rel(home, y, irt, z);
    }
  };

  /// Help class to create and register tests
  class Create {
  public:
    /// Perform creation and registration
    Create(void) {
      using namespace Gecode;
      for (IntRelTypes irts; irts(); ++irts) {
        for (IntConLevels icls; icls(); ++icls) {
          (void) new IntBinVar(irts.irt(),icls.icl());
          (void) new IntPairwise(irts.irt(),icls.icl());
        }
        (void) new BoolBinVar(irts.irt());
        (void) new BoolPairwise(irts.irt());
        for (int c=-4; c<=4; c++)
          (void) new IntBinInt(irts.irt(),c);
        for (int c=0; c<=1; c++)
          (void) new BoolBinInt(irts.irt(),c);
        (void) new IntArray(irts.irt());
        (void) new BoolArray(irts.irt());
      }      
    }
  };

  Create c;
  //@}

}}}

// STATISTICS: test-int
