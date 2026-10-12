/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <algorithm>
#include <limits>

namespace Test { namespace Int { namespace InterDistance {

  /// Exercise assignment, cloning, search, and bounds(Z) consistency
  class Distance : public Test {
  protected:
    int p;
  public:
    Distance(int n, int p0, int min, int max,
             Gecode::IntPropLevel ipl0=Gecode::IPL_DEF)
      : Test((n == 3 && p0 == 2 && min == -3)
             ? TestTags::check() : TestTags::standard(),
             "InterDistance::"+str(n)+"::"+str(p0)+"::"+str(min)+
             "::"+str(ipl0),n,min,max,false,ipl0), p(p0) {
      contest = Gecode::ba(ipl) == Gecode::IPL_BASIC ? CTL_NONE : CTL_BOUNDS_Z;
    }
    Distance(int n, int p0, const Gecode::IntSet& domain)
      : Test(TestTags::standard(),"InterDistance::Holes::"+str(n)+"::"+str(p0),
             n,domain,false,Gecode::IPL_DEF), p(p0) {
      contest = CTL_BOUNDS_Z;
    }
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=0; j<i; j++) {
          long long int d = static_cast<long long int>(x[i])-x[j];
          if ((-p < d) && (d < p))
            return false;
        }
      return true;
    }
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::inter_distance(home,x,p,ipl);
    }
  };

  /// Variable distance, including propagation after distance assignments
  class Variable : public Test {
  public:
    Variable(int n, Gecode::IntPropLevel ipl0)
      : Test(n == 3 ? TestTags::check() : TestTags::standard(),
             "InterDistance::Variable::"+str(n)+"::"+str(ipl0),
             n+1,-2,3,false,ipl0) {
      contest = Gecode::ba(ipl) == Gecode::IPL_BASIC ? CTL_NONE : CTL_BOUNDS_Z;
    }
    virtual bool solution(const Assignment& x) const {
      int p = x[x.size()-1];
      if (p < 0)
        return false;
      for (int i=0; i<x.size()-1; i++)
        for (int j=0; j<i; j++)
          if (std::abs(x[i]-x[j]) < p)
            return false;
      return true;
    }
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::IntVarArgs a(x.size()-1);
      for (int i=0; i<a.size(); i++)
        a[i] = x[i];
      Gecode::inter_distance(home,a,x[x.size()-1],ipl);
    }
  };

  /// Mix arbitrary assignments with sorted, separated values and shuffle them
  class DistanceAssignment : public RandomAssignment {
  protected:
    int p;
    bool variable;
  public:
    DistanceAssignment(int n, const Gecode::IntSet& d, int p0, bool v,
                       Gecode::Support::RandomGenerator& rand)
      : RandomAssignment(n,d,100,rand), p(p0), variable(v) {}
    virtual void next(Gecode::Support::RandomGenerator& rand) {
      RandomAssignment::next(rand);
      if (a % 2 == 0) {
        int m = n - static_cast<int>(variable);
        int distance = variable ? static_cast<int>(rand(p+1)) : p;
        int slack = d.max()-d.min()-(m-1)*distance;
        int value = d.min();
        for (int i=0; i<m; i++) {
          int extra = static_cast<int>(rand(slack+1));
          slack -= extra;
          value += extra;
          vals[i] = value;
          if (i+1 < m)
            value += distance;
        }
        for (int i=m-1; i>0; i--) {
          int j = static_cast<int>(rand(i+1));
          std::swap(vals[i],vals[j]);
        }
        if (variable)
          vals[m] = distance;
      }
    }
  };

  /// Larger instances with a bounded total slack
  class Random : public Test {
  protected:
    int p;
    bool variable;
  public:
    Random(int n, int p0, int slack, bool v, Gecode::IntPropLevel level)
      : Test("InterDistance::Random::"+str(n)+"::"+str(p0)+"::"+
             str(slack)+"::"+str(v)+"::"+str(level),
             n+static_cast<int>(v),0,(n-1)*p0+slack,false,level),
        p(p0), variable(v) {
      testsearch = false;
    }
    virtual Assignment* assignment(void) const {
      return new DistanceAssignment(arity,dom,p,variable,_rand);
    }
    virtual bool solution(const Assignment& x) const {
      int n = x.size()-static_cast<int>(variable);
      int distance = variable ? x[n] : p;
      for (int i=0; i<n; i++)
        for (int j=0; j<i; j++)
          if (std::abs(x[i]-x[j]) < distance)
            return false;
      return true;
    }
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::IntVarArgs y(x.size()-static_cast<int>(variable));
      for (int i=0; i<y.size(); i++)
        y[i] = x[i];
      if (variable)
        Gecode::inter_distance(home,y,x[y.size()],ipl);
      else
        Gecode::inter_distance(home,y,p,ipl);
    }
  };

  /// Enumerate assignments within each example's individual variable domains
  class ExampleAssignment : public Assignment {
    Gecode::IntSet domains[6];
    Gecode::IntSetValues values[6];
  public:
    ExampleAssignment(int n, const Gecode::IntSet* d)
      : Assignment(n,Gecode::IntSet()) {
      for (int i=0; i<n; i++) {
        domains[i] = d[i];
        values[i].init(domains[i]);
      }
    }
    virtual bool has_more(void) const {
      return values[0]();
    }
    virtual void next(Gecode::Support::RandomGenerator&) {
      int i = n-1;
      while (true) {
        ++values[i];
        if (values[i]() || (i == 0))
          return;
        values[i--].init(domains[i]);
      }
    }
    virtual int operator[](int i) const {
      return values[i].val();
    }
  };

  /// Hole-jump and residue examples use the same assignment checks
  class Examples : public Test {
  protected:
    Gecode::IntSet domains[6];
  public:
    Examples(const std::string& name, int n, const Gecode::IntSet& domain,
             const Gecode::IntSet* d, Gecode::IntPropLevel level)
      : Test(TestTags::check(),"InterDistance::"+name+"::"+str(level),
             n,domain,false,level) {
      for (int i=0; i<n; i++)
        domains[i] = d[i];
      testsearch = false;
    }
    virtual Assignment* assignment(void) const {
      return new ExampleAssignment(arity,domains);
    }
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        if (!domains[i].in(x[i]))
          return false;
      int p = x[x.size()-1];
      for (int i=0; i<x.size()-1; i++)
        for (int j=0; j<i; j++) {
          long long int d = static_cast<long long int>(x[i])-x[j];
          if ((-p < d) && (d < p))
            return false;
        }
      return true;
    }
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      for (int i=0; i<x.size(); i++)
        Gecode::dom(home,x[i],domains[i]);
      Gecode::IntVarArgs y(x.size()-1);
      for (int i=0; i<y.size(); i++)
        y[i] = x[i];
      Gecode::inter_distance(home,y,x[x.size()-1],ipl);
    }
  };

  /// Posting contracts, vacuous constraints, and shared variables
  class Posting : public Base {
    static bool constant_contracts(void) {
      using namespace Gecode;
      IntSet domain(-2,2);
      TestSpace s(0,domain,nullptr);
      IntVar v(s,-2,2);
      IntVarArgs duplicate(2);
      duplicate[0] = duplicate[1] = v;
      inter_distance(s,duplicate,0);
      bool same = false;
      bool negative = false;
      bool tooLarge = false;
      try {
        inter_distance(s,duplicate,2);
      } catch (const Gecode::Int::ArgumentSame&) {
        same = true;
      }
      try {
        inter_distance(s,IntVarArgs(),-1);
      } catch (const Gecode::Int::OutOfLimits&) {
        negative = true;
      }
      try {
        inter_distance(s,IntVarArgs(),std::numeric_limits<int>::max());
      } catch (const Gecode::Int::OutOfLimits&) {
        tooLarge = true;
      }

      inter_distance(s,IntVarArgs(),2);
      IntVarArgs singleton(1);
      singleton[0] = v;
      inter_distance(s,singleton,2);
      if (!same || !negative || !tooLarge || (s.status() == SS_FAILED))
        return false;

      TestSpace t(0,domain,nullptr);
      IntVar assigned(t,3,3);
      duplicate[0] = duplicate[1] = assigned;
      inter_distance(t,duplicate,2);
      if (t.status() != SS_FAILED)
        return false;
      return true;
    }

    static bool variable_contracts(void) {
      using namespace Gecode;
      IntSet domain(-3,9);
      TestSpace empty(0,domain,nullptr);
      IntVar distance(empty,-3,9);
      inter_distance(empty,IntVarArgs(),distance);
      if ((empty.status() == SS_FAILED) || (distance.min() != 0))
        return false;

      TestSpace negativeDistance(0,domain,nullptr);
      IntVar negativeVar(negativeDistance,-3,-1);
      inter_distance(negativeDistance,IntVarArgs(),negativeVar);
      if (negativeDistance.status() != SS_FAILED)
        return false;

      TestSpace shared(0,domain,nullptr);
      IntVar a(shared,0,9);
      IntVar b(shared,0,9);
      IntVar p(shared,0,9);
      IntVarArgs pair(2);
      pair[0] = a;
      pair[1] = b;
      bool alias = false;
      bool repeated = false;
      try {
        inter_distance(shared,pair,a);
      } catch (const Gecode::Int::ArgumentSame&) {
        alias = true;
      }
      pair[1] = a;
      try {
        inter_distance(shared,pair,p);
      } catch (const Gecode::Int::ArgumentSame&) {
        repeated = true;
      }
      if (!alias || !repeated)
        return false;
      return true;
    }

  public:
    Posting(void) : Base("Int::InterDistance::Posting",TestTags::check()) {}
    virtual bool run(void) {
      if (!constant_contracts()) {
        olog << "Constant posting contracts failed" << std::endl;
        return false;
      }
      if (!variable_contracts()) {
        olog << "Variable posting contracts failed" << std::endl;
        return false;
      }
      return true;
    }
  };

  class Create {
  public:
    Create(void) {
      using namespace Gecode;
      for (IntPropLevel level : {IPL_BASIC,IPL_ADVANCED,IPL_BASIC_ADVANCED}) {
        for (int n : {8,24})
          for (int slack : {0,8})
            for (bool variable : {false,true})
              (void) new Random(n,3,slack,variable,level);
        const IntSet paper[] = {IntSet(2,6),IntSet(10,14),
                                IntSet(4,15),IntSet(0,8)};
        (void) new Examples("Paper",4,IntSet(0,15),paper,level);
        const IntSet adjacent[] = {IntSet(1,17),IntSet(5,8),
                                   IntSet(2,14),IntSet(0,8)};
        (void) new Examples("Adjacent",4,IntSet(0,17),adjacent,level);
        const IntSet holes[] = {IntSet({0,2,6,7}),IntSet({-3,2}),
          IntSet({10,11,14}),IntSet({1,2,3,6}),IntSet({-1,0,3,5,9}),
          IntSet(4,4)};
        (void) new Examples("HoleJump",6,IntSet(-3,14),holes,level);
        const IntSet residue[] = {IntSet(2,11),IntSet(-10,-2),
                                  IntSet(16,17),IntSet(13,14)};
        (void) new Examples("Residue",4,IntSet(-10,17),residue,level);
      }
      const int limit = Gecode::Int::Limits::max;
      const IntSet extremes[] = {IntSet(-limit,-limit+1),IntSet(0,1),
                                 IntSet(limit-1,limit),IntSet(limit-1,limit)};
      (void) new Examples("Limits",4,
                         IntSet({-limit,-limit+1,0,1,limit-1,limit}),
                         extremes,IPL_BASIC);
      (void) new Distance(3,limit,IntSet({-limit,-limit+1,0,1,
                                        limit-1,limit}));
      for (int n=1; n<=4; n++)
        for (int p=0; p<=4; p++)
          (void) new Distance(n,p,-3,3);
      (void) new Distance(3,2,Gecode::IntSet({-3,-1,0,2,3}));
      (void) new Distance(3,2,Gecode::Int::Limits::min,
                             Gecode::Int::Limits::min+5);
      (void) new Distance(3,2,Gecode::Int::Limits::max-5,
                             Gecode::Int::Limits::max);
      for (Gecode::IntPropLevel ipl : {Gecode::IPL_DEF,Gecode::IPL_BASIC,
                                      Gecode::IPL_ADVANCED}) {
        (void) new Variable(2,ipl);
        (void) new Variable(3,ipl);
      }
      (void) new Variable(0,Gecode::IPL_DEF);
      (void) new Variable(1,Gecode::IPL_DEF);
      for (Gecode::IntPropLevel ipl : {Gecode::IPL_BASIC,Gecode::IPL_ADVANCED,
                                      Gecode::IPL_BASIC_ADVANCED})
        (void) new Distance(3,2,-3,3,ipl);
      (void) new Distance(3,2,-3,3,static_cast<Gecode::IntPropLevel>(
        Gecode::IPL_BND | Gecode::IPL_BASIC));
    }
  } create;
  Posting posting;

}}}

// STATISTICS: test-int
