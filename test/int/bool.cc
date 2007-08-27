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

namespace Test { namespace Int { namespace Bool {

  /**
   * \defgroup TaskTestIntChannel Boolean constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// Test for Boolean equality
  class Eq : public IntTest {
  public:
    /// Construct and register test
    Eq(void) : IntTest("Bool::Eq",2,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0]==x[1];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, channel(home,x[0]), IRT_EQ, channel(home,x[1]));
    }
  };

  /// Test for Boolean negation
  class Not : public IntTest {
  public:
    /// Construct and register test
    Not(void) : IntTest("Bool::Not",2,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return x[0]!=x[1];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, channel(home,x[0]), IRT_NQ, channel(home,x[1]));
    }
  };
  
  /// Test for Boolean conjunction
  class And : public IntTest {
  public:
    /// Construct and register test
    And(void) : IntTest("Bool::And",3,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (x[0]&x[1])==x[2];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, 
          channel(home,x[0]), BOT_AND, channel(home,x[1]), 
          channel(home,x[2]));
    }
  };
  
  /// Test for Boolean disjunction
  class Or : public IntTest {
  public:
    /// Construct and register test
    Or(void) : IntTest("Bool::Or",3,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (x[0]|x[1])==x[2];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, 
          channel(home,x[0]), BOT_OR, channel(home,x[1]), 
          channel(home,x[2]));
    }
  };
  
  /// Test for Boolean implication
  class Imp : public IntTest {
  public:
    /// Construct and register test
    Imp(void) : IntTest("Bool::Imp",3,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return ((x[0] == 0 ? 1 : 0)|x[1])==x[2];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, 
          channel(home,x[0]), BOT_IMP, channel(home,x[1]), 
          channel(home,x[2]));
    }
  };
  
  /// Test for Boolean equivalence
  class Eqv : public IntTest {
  public:
    /// Construct and register test
    Eqv(void) : IntTest("Bool::Eqv",3,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (x[0] == x[1])==x[2];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, 
          channel(home,x[0]), BOT_EQV, channel(home,x[1]), 
          channel(home,x[2]));
    }
  };
  
  /// Test for Boolean exclusive or
  class Xor : public IntTest {
  public:
    /// Construct and register test
    Xor(void) : IntTest("Bool::Xor",3,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      return (x[0] != x[1])==x[2];
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      rel(home, 
          channel(home,x[0]), BOT_XOR, channel(home,x[1]), 
          channel(home,x[2]));
    }
  };
  
  /// Test for Boolean n-ary conjunction
  class AndNary : public IntTest {
  public:
    /// Construct and register test
    AndNary(void) : IntTest("Bool::And::Nary",14,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i = x.size()-1; i--; )
        if (x[i] == 0)
          return x[x.size()-1] == 0;
      return x[x.size()-1] == 1;
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs b(2*(x.size()-1));
      for (int i=x.size()-1; i--; )
        b[2*i+0]=b[2*i+1]=channel(home,x[i]);
      rel(home, b, BOT_AND, channel(home,x[x.size()-1]));
    }
  };
  
  /// Test for Boolean n-ary disjunction
  class OrNary : public IntTest {
  public:
    /// Construct and register test
    OrNary(void) : IntTest("Bool::Or::Nary",14,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i = x.size()-1; i--; )
        if (x[i] == 1)
          return x[x.size()-1] == 1;
      return x[x.size()-1] == 0;
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs b(2*(x.size()-1));
      for (int i=x.size()-1; i--; )
        b[2*i+0]=b[2*i+1]=channel(home,x[i]);
      rel(home, b, BOT_OR, channel(home,x[x.size()-1]));
    }
  };
  
  /// Test for Boolean n-ary disjunction that is true
  class OrTrueNary : public IntTest {
  public:
    /// Construct and register test
    OrTrueNary(void) : IntTest("Bool::Or::True::Nary",14,0,1) {}
    /// Check whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i = x.size(); i--; )
        if (x[i] == 1)
          return true;
      return false;
    }
    /// Post constraint
    virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
      using namespace Gecode;
      BoolVarArgs b(2*(x.size()));
      for (int i=x.size(); i--; )
        b[2*i+0]=b[2*i+1]=channel(home,x[i]);
      rel(home, b, BOT_OR, 1);
    }
  };


  Eq beq;
  Not bnot;
  And band;
  Or bor;
  Imp bimp;
  Eqv beqv;
  Xor bxor;
  AndNary bandnary;
  OrNary bornary;
  OrTrueNary bortruenary;

  //@}

}}}

// STATISTICS: test-int

