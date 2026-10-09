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
#include <vector>
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

  /// Regressions for hole jumps, critical regions, and distance updates
  class Regression : public Base {
    typedef std::vector<int> Domain;
    typedef std::vector<Domain> Domains;

    class Model : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      Gecode::IntVar p;
      Model(const Domains& domains, const Domain& distance={0})
        : x(*this,static_cast<int>(domains.size())),
          p(*this,Gecode::IntSet(distance.data(),
                                static_cast<int>(distance.size()))) {
        for (int i=0; i<x.size(); i++)
          x[i] = Gecode::IntVar(*this,Gecode::IntSet(
            domains[i].data(),static_cast<int>(domains[i].size())));
      }
      Model(Model& s) : Gecode::Space(s) {
        x.update(*this,s.x);
        p.update(*this,s.p);
      }
      virtual Gecode::Space* copy(void) {
        return new Model(*this);
      }
    };

    static bool regressions(void) {
      using namespace Gecode;
      // The paper's example and its reflection have unique solutions.
      for (int sign : {1,-1}) {
        Domains domains = {{2,3,4,5,6},{10,11,12,13,14},
          {4,5,6,7,8,9,10,11,12,13,14,15}};
        for (Domain& domain : domains) {
          for (int& value : domain)
            value *= sign;
          std::sort(domain.begin(),domain.end());
        }
        Model model(domains);
        inter_distance(model,model.x,6);
        if ((model.status() == SS_FAILED) || !model.x.assigned() ||
            (model.x[0].val() != sign*2) ||
            (model.x[1].val() != sign*14) ||
            (model.x[2].val() != sign*8))
          return false;
      }

      // A jump across holes must not subsume an invalid assigned tuple.
      Model holes({{0,2,6,7},{-3,2},{10,11,14},{1,2,3,6},
                   {-1,0,3,5,9}});
      inter_distance(holes,holes.x,4);
      if (holes.status() != SS_FAILED)
        return false;

      // A consumed residue can become active at a later deadline.
      Model residue({{2,3,4,5,6,7,8,9,10,11},
                     {-10,-9,-8,-7,-6,-5,-4,-3,-2},{16,17}},{13,14});
      inter_distance(residue,residue.x,residue.p,IPL_BASIC);
      if ((residue.status() == SS_FAILED) || !residue.p.assigned() ||
          (residue.p.val() != 13))
        return false;

      // Adjacent forbidden regions limit separation to six, not seven.
      Domains adjacent(3);
      for (int v=1; v<=17; v++) adjacent[0].push_back(v);
      for (int v=5; v<=8; v++) adjacent[1].push_back(v);
      for (int v=2; v<=14; v++) adjacent[2].push_back(v);
      for (IntPropLevel ipl : {IPL_BASIC,IPL_ADVANCED,IPL_BASIC_ADVANCED}) {
        Model model(adjacent,{0,1,2,3,4,5,6,7,8});
        inter_distance(model,model.x,model.p,ipl);
        if ((model.status() == SS_FAILED) || (model.p.max() != 6))
          return false;
      }

      // Deadlines can extend beyond the representable integer domain.
      const int limit = Gecode::Int::Limits::max;
      Model extremes({{-limit,-limit+1},{0,1},{limit-1,limit}});
      inter_distance(extremes,extremes.x,limit);
      return (extremes.status() != SS_FAILED) && extremes.x.assigned() &&
        (extremes.x[0].val() == -limit) && (extremes.x[1].val() == 0) &&
        (extremes.x[2].val() == limit);
    }

    static bool distance_updates(void) {
      // Raise the minimum distance in a clone after an initial fixpoint.
      // Both p's subscription and staging must survive cloning.
      Domains paper = {{2,3,4,5,6},
        {10,11,12,13,14},{4,5,6,7,8,9,10,11,12,13,14,15}};
      Model m(paper,{0,1,2,3,4,5,6,7,8});
      Gecode::inter_distance(m,m.x,m.p);
      if ((m.status() == Gecode::SS_FAILED) || (m.p.max() != 6))
        return false;
      Model* clone = static_cast<Model*>(m.clone());
      Gecode::rel(*clone,clone->p,Gecode::IRT_GQ,6);
      bool correct = clone->status() != Gecode::SS_FAILED;
      correct &= clone->p.assigned() && (clone->p.val() == 6);
      correct &= clone->x.assigned() && (clone->x[0].val() == 2) &&
        (clone->x[1].val() == 14) && (clone->x[2].val() == 8);
      correct &= (m.p.min() == 0) && !m.x.assigned();
      delete clone;
      if (!correct)
        return false;

      Model basic(paper,{6,7,8});
      Gecode::inter_distance(basic,basic.x,basic.p,Gecode::IPL_BASIC);
      if ((basic.status() == Gecode::SS_FAILED) || !basic.p.assigned() ||
          (basic.p.val() != 6) ||
          (basic.x[0].min() != 2) || (basic.x[0].max() != 6))
        return false;

      // Assigning p to one must immediately rewrite to distinct, so a
      // newly posted constraint cannot prune further at the same fixpoint.
      Model unit({{-2},{-2,-1}},{1,3});
      Gecode::inter_distance(unit,unit.x,unit.p,Gecode::IPL_BASIC);
      return (unit.status() != Gecode::SS_FAILED) && unit.p.assigned() &&
        (unit.p.val() == 1) && unit.x[1].assigned() && (unit.x[1].val() == -1);
    }
  public:
    Regression(void)
      : Base("Int::InterDistance::Regression",TestTags::check()) {}
    virtual bool run(void) {
      return regressions() && distance_updates();
    }
  };

  /// Posting contracts, vacuous constraints, and shared variables
  class Posting : public Base {
    class Model : public Gecode::Space {
    public:
      Model(void) {}
      Model(Model& s) : Gecode::Space(s) {}
      virtual Gecode::Space* copy(void) {
        return new Model(*this);
      }
    };
    static bool constant_contracts(void) {
      using namespace Gecode;
      Model s;
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

      Model t;
      IntVar assigned(t,3,3);
      duplicate[0] = duplicate[1] = assigned;
      inter_distance(t,duplicate,2);
      if (t.status() != SS_FAILED)
        return false;
      return true;
    }

    static bool variable_contracts(void) {
      using namespace Gecode;
      Model empty;
      IntVar distance(empty,-3,9);
      inter_distance(empty,IntVarArgs(),distance);
      if ((empty.status() == SS_FAILED) || (distance.min() != 0))
        return false;

      Model negativeDistance;
      IntVar negativeVar(negativeDistance,-3,-1);
      inter_distance(negativeDistance,IntVarArgs(),negativeVar);
      if (negativeDistance.status() != SS_FAILED)
        return false;

      Model shared;
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

    static bool large_distances(void) {
      using namespace Gecode;
      // A wide search interval exercises binary search and integer limits.
      Model wide;
      const int limit = Gecode::Int::Limits::max;
      IntVarArgs x(3);
      x[0] = IntVar(wide,0,limit/4);
      x[1] = IntVar(wide,0,limit/4);
      x[2] = IntVar(wide,limit-1,limit);
      IntVar separation(wide,0,limit);
      inter_distance(wide,x,separation,IPL_BASIC);
      if ((wide.status() == SS_FAILED) || (separation.max() != limit/4))
        return false;

      // The assigned gap can exceed the representable distance domain.
      Model extremes;
      IntVarArgs pair(2);
      pair[0] = IntVar(extremes,-limit,-limit);
      pair[1] = IntVar(extremes,limit,limit);
      IntVar large(extremes,0,limit);
      inter_distance(extremes,pair,large);
      return (extremes.status() != SS_FAILED) && (large.max() == limit);
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
      if (!large_distances()) {
        olog << "Large-distance arithmetic failed" << std::endl;
        return false;
      }
      return true;
    }
  };

  class Create {
  public:
    Create(void) {
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
  Regression regression;
  Posting posting;

}}}

// STATISTICS: test-int
