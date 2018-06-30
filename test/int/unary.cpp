/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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
   /// %Tests for unary scheduling constraints
   namespace Unary {}
}}

namespace Test { namespace Int { namespace Unary {

  /**
   * \defgroup TaskTestIntUnary Unary scheduling constraints
   * \ingroup TaskTestInt
   */
  //@{
  /// %Test for unary constraint
  class ManFixPUnary : public Test {
  protected:
    /// The processing times
    Gecode::IntArgs p;
    /// Get a reasonable maximal start time
    static int st(const Gecode::IntArgs& p) {
      int t = 0;
      for (int i=p.size(); i--; )
        t += p[i];
      return t;
    }
  public:
    /// Create and register test
    ManFixPUnary(const Gecode::IntArgs& p0, int o, Gecode::IntPropLevel ipl0)
      : Test("Unary::Man::Fix::"+str(o)+"::"+str(p0)+"::"+str(ipl0),
             p0.size(),o,o+st(p0),false,ipl0),
        p(p0) {
      testsearch = false;
      contest = CTL_NONE;
    }
    /// Create and register initial assignment
    virtual Assignment* assignment(void) const {
      return new RandomAssignment(arity,dom,500);
    }
    /// %Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if ((x[i]+p[i] > x[j]) && (x[j]+p[j] > x[i]))
            return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::unary(home, x, p, ipl);
    }
  };

  /// %Test for unary constraint with optional tasks
  class OptFixPUnary : public Test {
  protected:
    /// The processing times
    Gecode::IntArgs p;
    /// Threshold for taking a task as optional
    int l;
    /// Get a reasonable maximal start time
    static int st(const Gecode::IntArgs& p) {
      int t = 0;
      for (int i=p.size(); i--; )
        t += p[i];
      return t;
    }
  public:
    /// Create and register test
    OptFixPUnary(const Gecode::IntArgs& p0, int o, Gecode::IntPropLevel ipl0)
      : Test("Unary::Opt::Fix::"+str(o)+"::"+str(p0)+"::"+str(ipl0),
             2*p0.size(),o,o+st(p0),false,ipl0), p(p0), l(o+st(p)/2) {
      testsearch = false;
      contest = CTL_NONE;
    }
    /// Create and register initial assignment
    virtual Assignment* assignment(void) const {
      return new RandomAssignment(arity,dom,500);
    }
    /// %Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n = x.size() / 2;
      for (int i=0; i<n; i++)
        if (x[n+i] > l)
          for (int j=i+1; j<n; j++)
            if(x[n+j] > l)
              if ((x[i]+p[i] > x[j]) && (x[j]+p[j] > x[i]))
                return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      int n=x.size() / 2;
      Gecode::IntVarArgs s(n);
      Gecode::BoolVarArgs m(n);
      for (int i=0; i<n; i++) {
        s[i]=x[i];
        m[i]=Gecode::expr(home, (x[n+i] > l));
      }
      Gecode::unary(home, s, p, m, ipl);
    }
  };

  /// %Test for unary constraint
  class ManFlexUnary : public Test {
  protected:
    /// Minimum processing time
    int _minP;
    /// Maximum processing time
    int _maxP;
    /// Offset for start times
    int off;
  public:
    /// Create and register test
    ManFlexUnary(int n, int minP, int maxP, int o, Gecode::IntPropLevel ipl0)
      : Test("Unary::Man::Flex::"+str(o)+"::"+str(n)+"::"
             +str(minP)+"::"+str(maxP)+"::"+str(ipl0),
             2*n,0,n*maxP,false,ipl0), _minP(minP), _maxP(maxP), off(o) {
      testsearch = false;
      testfix = false;
      contest = CTL_NONE;
    }
    /// Create and register initial assignment
    virtual Assignment* assignment(void) const {
      return new RandomMixAssignment(arity/2,dom,arity/2,
                                     Gecode::IntSet(_minP,_maxP),500);
    }
    /// %Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n = x.size()/2;
      for (int i=0; i<n; i++)
        for (int j=i+1; j<n; j++)
          if ((x[i]+x[n+i] > x[j]) && (x[j]+x[n+j] > x[i]))
            return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::IntVarArgs s(x.size()/2);
      Gecode::IntVarArgs px(x.slice(x.size()/2));
      Gecode::IntVarArgs e(home,x.size()/2,
                           Gecode::Int::Limits::min,
                           Gecode::Int::Limits::max);
      for (int i=s.size(); i--;) {
        s[i] = expr(home, off+x[i]);
        rel(home, s[i]+px[i] == e[i]);
        rel(home, _minP <= px[i]);
        rel(home, _maxP >= px[i]);
      }
      Gecode::unary(home, s, px, e, ipl);
    }
  };

  /// %Test for unary constraint with optional tasks
  class OptFlexUnary : public Test {
  protected:
    /// Minimum processing time
    int _minP;
    /// Maximum processing time
    int _maxP;
    /// Offset for start times
    int off;
    /// Threshold for taking a task as optional
    int l;
    /// Get a reasonable maximal start time
    static int st(const Gecode::IntArgs& p) {
      int t = 0;
      for (int i=p.size(); i--; )
        t += p[i];
      return t;
    }
  public:
    /// Create and register test
    OptFlexUnary(int n, int minP, int maxP, int o, Gecode::IntPropLevel ipl0)
      : Test("Unary::Opt::Flex::"+str(o)+"::"+str(n)+"::"
             +str(minP)+"::"+str(maxP)+"::"+str(ipl0),
             3*n,0,n*maxP,false,ipl0), _minP(minP), _maxP(maxP), off(o),
        l(n*maxP/2) {
      testsearch = false;
      testfix = false;
      contest = CTL_NONE;
    }
    /// Create and register initial assignment
    virtual Assignment* assignment(void) const {
      return new RandomMixAssignment(2*(arity/3),dom,arity/3,
                                     Gecode::IntSet(_minP,_maxP),500);
    }
    /// %Test whether \a x is solution
    virtual bool solution(const Assignment& x) const {
      int n = x.size() / 3;
      for (int i=0; i<n; i++)
        if (x[n+i] > l)
          for (int j=i+1; j<n; j++)
            if(x[n+j] > l)
              if ((x[i]+x[2*n+i] > x[j]) && (x[j]+x[2*n+j] > x[i]))
                return false;
      return true;
    }
    /// Post constraint on \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      int n=x.size() / 3;

      Gecode::IntVarArgs s(n);
      Gecode::IntVarArgs px(n);
      Gecode::IntVarArgs e(home,n,
                           Gecode::Int::Limits::min,
                           Gecode::Int::Limits::max);
      for (int i=n; i--;) {
        s[i] = expr(home, off+x[i]);
        px[i] = x[2*n+i];
        rel(home, s[i]+px[i] == e[i]);
        rel(home, _minP <= px[i]);
        rel(home, _maxP >= px[i]);
      }
      Gecode::BoolVarArgs m(n);
      for (int i=0; i<n; i++)
        m[i]=Gecode::expr(home, (x[n+i] > l));
      Gecode::unary(home, s, px, e, m, ipl);
    }
  };

  /// Help class to create and register tests
  class Create {
  public:
    /// Perform creation and registration
    Create(void) {
      using namespace Gecode;
      IntArgs p1({2,2,2,2});
      IntArgs p10({2,2,0,2,2});
      IntArgs p2({4,3,3,5});
      IntArgs p20({4,0,3,3,0,5});
      IntArgs p3({4,2,9,3,7,5});
      IntArgs p30({4,0,2,9,3,7,5,0});

      for (IntPropBasicAdvanced ipba; ipba(); ++ipba) {
        (void) new ManFixPUnary(p1,0,ipba.ipl());
        (void) new ManFixPUnary(p1,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p1,0,ipba.ipl());
        (void) new OptFixPUnary(p1,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(4,0,2,0,ipba.ipl());
        (void) new ManFlexUnary(4,0,2,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(4,1,3,0,ipba.ipl());
        (void) new ManFlexUnary(4,1,3,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(4,0,2,0,ipba.ipl());
        (void) new OptFlexUnary(4,0,2,Gecode::Int::Limits::min,ipba.ipl());

        (void) new ManFixPUnary(p10,0,ipba.ipl());
        (void) new ManFixPUnary(p10,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p10,0,ipba.ipl());
        (void) new OptFixPUnary(p10,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(5,0,2,0,ipba.ipl());
        (void) new ManFlexUnary(5,0,2,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(5,0,2,0,ipba.ipl());
        (void) new OptFlexUnary(5,0,2,Gecode::Int::Limits::min,ipba.ipl());

        (void) new ManFixPUnary(p2,0,ipba.ipl());
        (void) new ManFixPUnary(p2,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p2,0,ipba.ipl());
        (void) new OptFixPUnary(p2,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(4,3,5,0,ipba.ipl());
        (void) new ManFlexUnary(4,3,5,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(4,3,5,0,ipba.ipl());
        (void) new OptFlexUnary(4,3,5,Gecode::Int::Limits::min,ipba.ipl());

        (void) new ManFixPUnary(p20,0,ipba.ipl());
        (void) new ManFixPUnary(p20,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p20,0,ipba.ipl());
        (void) new OptFixPUnary(p20,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(6,0,5,0,ipba.ipl());
        (void) new ManFlexUnary(6,0,5,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(6,0,5,0,ipba.ipl());
        (void) new OptFlexUnary(6,0,5,Gecode::Int::Limits::min,ipba.ipl());

        (void) new ManFixPUnary(p3,0,ipba.ipl());
        (void) new ManFixPUnary(p3,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p3,0,ipba.ipl());
        (void) new OptFixPUnary(p3,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(6,2,7,0,ipba.ipl());
        (void) new ManFlexUnary(6,2,7,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(6,2,7,0,ipba.ipl());
        (void) new OptFlexUnary(6,2,7,Gecode::Int::Limits::min,ipba.ipl());

        (void) new ManFixPUnary(p30,0,ipba.ipl());
        (void) new ManFixPUnary(p30,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFixPUnary(p30,0,ipba.ipl());
        (void) new OptFixPUnary(p30,Gecode::Int::Limits::min,ipba.ipl());
        (void) new ManFlexUnary(8,0,9,0,ipba.ipl());
        (void) new ManFlexUnary(8,0,9,Gecode::Int::Limits::min,ipba.ipl());
        (void) new OptFlexUnary(8,0,9,0,ipba.ipl());
        (void) new OptFlexUnary(8,0,9,Gecode::Int::Limits::min,ipba.ipl());
      }
    }
  };

  Create c;
  //@}


}}}

// STATISTICS: test-int
