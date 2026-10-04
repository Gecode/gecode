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
      : Test("InterDistance::"+str(n)+"::"+str(p0)+"::"+str(min)+
             "::"+str(ipl0),n,min,max,false,ipl0), p(p0) {
      contest = Gecode::ba(ipl) == Gecode::IPL_BASIC ? CTL_NONE : CTL_BOUNDS_Z;
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
      : Test("InterDistance::Variable::"+str(n)+"::"+str(ipl0),
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

  /// Small independent oracle for heterogeneous and holey domains
  class Bounds : public Base {
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

    /// Supported bounds and largest separation found by tuple enumeration
    struct Supports {
      bool found;
      std::vector<int> min;
      std::vector<int> max;
      long long int gap;

      Supports(int n)
        : found(false), min(n,Gecode::Int::Limits::max),
          max(n,Gecode::Int::Limits::min), gap(-1) {}

      void include(const std::vector<int>& tuple) {
        found = true;
        long long int distance = Gecode::Int::Limits::max;
        for (int i=0; i<static_cast<int>(tuple.size()); i++) {
          min[i] = std::min(min[i],tuple[i]);
          max[i] = std::max(max[i],tuple[i]);
          for (int j=0; j<i; j++) {
            long long int delta = static_cast<long long int>(tuple[i])-tuple[j];
            distance = std::min(distance,std::abs(delta));
          }
        }
        gap = std::max(gap,distance);
      }
    };

    /// Enumerate using only the defining pairwise inequality
    static void enumerate(const Domains& dom, int p, int i,
                          std::vector<int>& tuple, Supports& result) {
      if (i == static_cast<int>(dom.size())) {
        result.include(tuple);
        return;
      }

      for (int value : dom[i]) {
        bool valid = true;
        for (int j=0; j<i; j++) {
          long long int delta = static_cast<long long int>(value)-tuple[j];
          if ((-p < delta) && (delta < p)) {
            valid = false;
            break;
          }
        }
        if (valid) {
          tuple[i] = value;
          enumerate(dom,p,i+1,tuple,result);
        }
      }
    }

    /// Return independent supports for these domains and separation
    static Supports enumerate(const Domains& dom, int p) {
      Supports result(static_cast<int>(dom.size()));
      std::vector<int> tuple(dom.size());
      enumerate(dom,p,0,tuple,result);
      return result;
    }

    /// Interval supports for bounds(Z) may use holes in other variables
    static Domains interval_hulls(const Gecode::IntVarArray& x) {
      Domains hull(x.size());
      for (int i=0; i<x.size(); i++)
        for (int value=x[i].min(); value<=x[i].max(); value++)
          hull[i].push_back(value);
      return hull;
    }

    /// Check preservation of every feasible tuple's supported bounds
    static bool preserves_supports(const Gecode::IntVarArray& x,
                                   const Supports& expected) {
      for (int i=0; i<x.size(); i++)
        if ((x[i].min() > expected.min[i]) || (x[i].max() < expected.max[i]))
          return false;
      return true;
    }

    /// Check that each remaining bound has an interval support
    static bool matches_supports(const Gecode::IntVarArray& x,
                                 const Supports& expected) {
      if (!expected.found)
        return false;
      for (int i=0; i<x.size(); i++)
        if ((x[i].min() != expected.min[i]) || (x[i].max() != expected.max[i]))
          return false;
      return true;
    }

    static void print_domain(const Domain& domain) {
      olog << '{';
      for (int value : domain)
        olog << value << ',';
      olog << '}';
    }

    /// Record complete inputs and expectations in the test harness log
    static bool mismatch(const char* reason, const Domains& domains,
                         const Domain& distance, Gecode::IntPropLevel ipl,
                         const Supports& expected, const Model& model) {
      olog << reason << ": domains ";
      for (const Domain& domain : domains) {
        print_domain(domain);
        olog << ' ';
      }
      olog << "distance ";
      print_domain(distance);
      olog << ", propagation level " << ipl
           << ", solution exists " << expected.found << ", expected bounds ";
      for (int i=0; i<model.x.size(); i++)
        olog << '[' << expected.min[i] << ',' << expected.max[i] << "] ";
      olog << ", maximum gap " << expected.gap << ", propagated " << model.x
           << ", distance " << model.p << std::endl;
      return false;
    }

    static bool check(const Domains& dom, int p, bool dense) {
      Supports expected = enumerate(dom,p);
      Model model(dom,{p});
      Gecode::inter_distance(model,model.x,p);
      bool failed = model.status() == Gecode::SS_FAILED;

      if ((expected.found && failed) || (!expected.found && dense && !failed))
        return mismatch("Feasibility",dom,{p},Gecode::IPL_DEF,expected,model);
      if (failed)
        return true;
      if (expected.found && !preserves_supports(model.x,expected))
        return mismatch("Lost solution",dom,{p},Gecode::IPL_DEF,expected,model);

      // Check the final hulls after jumps across domain holes as well.
      if (!dense)
        expected = enumerate(interval_hulls(model.x),p);
      if (!matches_supports(model.x,expected))
        return mismatch("Bounds(Z) consistency",dom,{p},Gecode::IPL_DEF,
                        expected,model);
      return true;
    }

    /// Largest domain value supported by the enumerated maximum gap
    static int largest_distance(const Domain& distance, long long int gap) {
      auto end = std::upper_bound(distance.begin(),distance.end(),gap);
      assert(end != distance.begin());
      return *--end;
    }

    /// Check distance supports separately from BASIC's weaker x filtering
    static bool check_variable(const Domains& dom, const Domain& distance,
                               bool dense, Gecode::IntPropLevel ipl) {
      Supports expected = enumerate(dom,distance.front());
      Model model(dom,distance);
      Gecode::inter_distance(model,model.x,model.p,ipl);
      bool failed = model.status() == Gecode::SS_FAILED;

      if ((expected.found && failed) || (!expected.found && dense && !failed))
        return mismatch("Feasibility",dom,distance,ipl,expected,model);
      if (failed)
        return true;
      if (expected.found && !preserves_supports(model.x,expected))
        return mismatch("Lost solution",dom,distance,ipl,expected,model);

      if (!dense)
        expected = enumerate(interval_hulls(model.x),model.p.min());
      if (!expected.found)
        return mismatch("Interval feasibility",dom,distance,ipl,expected,model);
      if (model.p.max() != largest_distance(distance,expected.gap))
        return mismatch("Maximum distance",dom,distance,ipl,expected,model);
      if ((Gecode::ba(ipl) != Gecode::IPL_BASIC) &&
          !matches_supports(model.x,expected))
        return mismatch("Bounds(Z) consistency",dom,distance,ipl,expected,model);
      return true;
    }
    static bool regression_examples(void) {
      // The paper's example, including its reflected form.
      if (!check({{2,3,4,5,6},{10,11,12,13,14},
                  {4,5,6,7,8,9,10,11,12,13,14,15}},6,true))
        return false;
      if (!check({{-6,-5,-4,-3,-2},{-14,-13,-12,-11,-10},
                  {-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4}},6,true))
        return false;
      // Upper-bound pruning across holes can assign an invalid tuple in
      // one pass; it must be checked rather than immediately subsumed.
      if (!check({{0,2,6,7},{-3,2},{10,11,14},{1,2,3,6},
                  {-1,0,3,5,9}},4,false))
        return false;
      // A maximal distance exercises deadlines beyond Int::Limits::max.
      const int limit = Gecode::Int::Limits::max;
      if (!check({{-limit,-limit+1},{0,1},{limit-1,limit}},limit,true))
        return false;
      // A consumed residue can be reactivated by a later deadline.
      if (!check_variable({{2,3,4,5,6,7,8,9,10,11},
                           {-10,-9,-8,-7,-6,-5,-4,-3,-2},{16,17}},
                          {13,14},true,Gecode::IPL_BASIC))
        return false;
      return true;
    }

    static bool exhaustive_intervals(void) {
      // Exhaust all triples of intervals on [-2,3], including ties.
      Domains intervals;
      for (int l=-2; l<=3; l++)
        for (int u=l; u<=3; u++) {
          std::vector<int> d;
          for (int v=l; v<=u; v++)
            d.push_back(v);
          intervals.push_back(d);
        }
      for (int p=2; p<=3; p++)
        for (const auto& a : intervals)
          for (const auto& b : intervals)
            for (const auto& c : intervals)
              if (!check({a,b,c},p,true))
                return false;
      return true;
    }

    /// Generate heterogeneous domains while preserving the fixed RNG sequence
    static Domains random_domains(Gecode::Support::RandomGenerator& random,
                                  int n, int offset, bool dense) {
      Domains domains(n);
      for (Domain& domain : domains) {
        int min = offset+static_cast<int>(random(21))-10;
        int width = random(5);
        for (int j=0; j<=width; j++)
          if (dense || (j == 0) || (j == width) || random(2))
            domain.push_back(min+j);
      }
      return domains;
    }

    static bool random_constant_cases(Gecode::Support::RandomGenerator& random) {
      for (int test=0; test<2000; test++) {
        int n = 2+random(7);
        int p = 2+random(8);
        int offset = (test % 3 == 0) ? Gecode::Int::Limits::min+30 :
          (test % 3 == 1) ? Gecode::Int::Limits::max-30 : 0;
        bool dense = (test % 2 == 0);
        Domains dom = random_domains(random,n,offset,dense);
        if (!check(dom,p,dense))
          return false;
      }
      return true;
    }

    static bool adjacent_regions(void) {
      // Adjacent forbidden regions used to let the feasibility pass miss
      // this overload. The maximum feasible separation is six, not seven.
      Domains adjacent(3);
      for (int v=1; v<=17; v++) adjacent[0].push_back(v);
      for (int v=5; v<=8; v++) adjacent[1].push_back(v);
      for (int v=2; v<=14; v++) adjacent[2].push_back(v);
      if (!check(adjacent,7,true))
        return false;
      for (Gecode::IntPropLevel ipl : {Gecode::IPL_BASIC,Gecode::IPL_ADVANCED,
                                      Gecode::IPL_BASIC_ADVANCED})
        if (!check_variable(adjacent,{0,1,2,3,4,5,6,7,8},true,ipl))
          return false;
      return true;
    }

    static bool random_variable_cases(Gecode::Support::RandomGenerator& random) {
      for (int test=0; test<512; test++) {
        int n = 2+random(4);
        int offset = test%3 == 0 ? Gecode::Int::Limits::min+30 :
          test%3 == 1 ? Gecode::Int::Limits::max-30 : 0;
        bool dense = test%2 == 0;
        Domains dom = random_domains(random,n,offset,dense);
        std::vector<int> distance;
        int lower = random(4);
        for (int p=lower; p<=12; p++)
          if (dense || (p == lower) || (p == 12) || random(2))
            distance.push_back(p);
        Gecode::IntPropLevel ipl = test%3 == 0 ? Gecode::IPL_BASIC :
          test%3 == 1 ? Gecode::IPL_ADVANCED : Gecode::IPL_DEF;
        if (!check_variable(dom,distance,dense,ipl))
          return false;
      }
      return true;
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
    Bounds(void) : Base("Int::InterDistance::BoundsOracle") {}
    virtual bool run(void) {
      Gecode::Support::RandomGenerator random(2006);
      if (!regression_examples() || !exhaustive_intervals() ||
          !random_constant_cases(random) || !adjacent_regions() ||
          !random_variable_cases(random))
        return false;
      if (!distance_updates()) {
        olog << "Distance updates, cloning, or rewriting failed" << std::endl;
        return false;
      }
      return true;
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
    Posting(void) : Base("Int::InterDistance::Posting") {}
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
  Bounds bounds;
  Posting posting;

}}}

// STATISTICS: test-int
