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
    Gecode::IntPropLevel ipl;
  public:
    Distance(int n, int p0, int min, int max,
             Gecode::IntPropLevel ipl0=Gecode::IPL_DEF)
      : Test("InterDistance::"+str(n)+"::"+str(p0)+"::"+str(min)+
             "::"+str(ipl0),n,min,max), p(p0), ipl(ipl0) {
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
    Gecode::IntPropLevel ipl;
  public:
    Variable(int n, Gecode::IntPropLevel ipl0)
      : Test("InterDistance::Variable::"+str(n)+"::"+str(ipl0),
             n+1,-2,3), ipl(ipl0) {
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
    class Model : public Gecode::Space {
    public:
      Gecode::IntVarArray x;
      Gecode::IntVar p;
      Model(const std::vector<std::vector<int> >& domains,
            const std::vector<int>& distance={0})
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
    /// Enumerate tuples using only the defining pairwise inequality
    static void supports(const std::vector<std::vector<int> >& dom, int p,
                         int i, std::vector<int>& tuple,
                         std::vector<int>& lo, std::vector<int>& hi,
                         bool& found, long long int* gap=nullptr) {
      if (i == static_cast<int>(dom.size())) {
        found = true;
        for (int j=0; j<i; j++) {
          lo[j] = std::min(lo[j],tuple[j]);
          hi[j] = std::max(hi[j],tuple[j]);
        }
        if (gap != nullptr) {
          long long int d = Gecode::Int::Limits::max;
          for (int j=0; j<i; j++)
            for (int k=0; k<j; k++) {
              long long int delta = static_cast<long long int>(tuple[j])-tuple[k];
              d = std::min(d,std::abs(delta));
            }
          *gap = std::max(*gap,d);
        }
        return;
      }
      for (int value : dom[i]) {
        bool valid = true;
        for (int j=0; j<i; j++) {
          long long int d = static_cast<long long int>(value)-tuple[j];
          if ((-p < d) && (d < p)) {
            valid = false;
            break;
          }
        }
        if (valid) {
          tuple[i] = value;
          supports(dom,p,i+1,tuple,lo,hi,found,gap);
        }
      }
    }
    static bool check(const std::vector<std::vector<int> >& dom, int p,
                      bool dense) {
      const int n = static_cast<int>(dom.size());
      std::vector<int> tuple(n), lo(n,Gecode::Int::Limits::max),
        hi(n,Gecode::Int::Limits::min);
      bool found = false;
      supports(dom,p,0,tuple,lo,hi,found);
      Model m(dom);
      Gecode::inter_distance(m,m.x,p);
      bool failed = m.status() == Gecode::SS_FAILED;
      bool correct = found ? !failed : (!dense || failed);
      if (found && !failed)
        for (int i=0; i<n; i++)
          correct &= dense ? ((m.x[i].min() == lo[i]) &&
                              (m.x[i].max() == hi[i])) :
            ((m.x[i].min() <= lo[i]) && (m.x[i].max() >= hi[i]));
      if (!dense && !failed) {
        // Bounds(Z) supports may use holes in other variables. Enumerate
        // the final interval hulls to check that every remaining bound
        // has such a support, including bounds that jumped across holes.
        std::vector<std::vector<int> > hull(n);
        for (int i=0; i<n; i++)
          for (int v=m.x[i].min(); v<=m.x[i].max(); v++)
            hull[i].push_back(v);
        std::fill(lo.begin(),lo.end(),Gecode::Int::Limits::max);
        std::fill(hi.begin(),hi.end(),Gecode::Int::Limits::min);
        found = false;
        supports(hull,p,0,tuple,lo,hi,found);
        correct &= found;
        for (int i=0; i<n; i++)
          correct &= (m.x[i].min() == lo[i]) && (m.x[i].max() == hi[i]);
      }
      if (!correct) {
        olog << "Distance " << p << ", domains:";
        for (const auto& d : dom) {
          olog << " {";
          for (int v : d)
            olog << v << ',';
          olog << '}';
        }
        olog << " propagated " << m.x << std::endl;
      }
      return correct;
    }
    /// Check the largest distance and x supports against tuple enumeration
    static bool check_variable(const std::vector<std::vector<int> >& dom,
                               const std::vector<int>& distance, bool dense,
                               Gecode::IntPropLevel ipl) {
      const int n = static_cast<int>(dom.size());
      std::vector<int> tuple(n), lo(n,Gecode::Int::Limits::max),
        hi(n,Gecode::Int::Limits::min);
      bool found = false;
      long long int gap = -1;
      supports(dom,distance.front(),0,tuple,lo,hi,found,&gap);
      Model m(dom,distance);
      Gecode::inter_distance(m,m.x,m.p,ipl);
      bool failed = m.status() == Gecode::SS_FAILED;
      bool correct = found ? !failed : (!dense || failed);
      if (found && !failed) {
        for (int i=0; i<n; i++)
          correct &= dense && (Gecode::ba(ipl) != Gecode::IPL_BASIC) ?
            ((m.x[i].min() == lo[i]) && (m.x[i].max() == hi[i])) :
            ((m.x[i].min() <= lo[i]) && (m.x[i].max() >= hi[i]));
        if (dense) {
          auto largest = std::upper_bound(distance.begin(),distance.end(),gap);
          correct &= (m.p.max() == *--largest);
        }
      }
      if (!dense && !failed) {
        // Bounds(Z) supports can use holes in the other x domains.
        std::vector<std::vector<int> > hull(n);
        for (int i=0; i<n; i++)
          for (int v=m.x[i].min(); v<=m.x[i].max(); v++)
            hull[i].push_back(v);
        std::fill(lo.begin(),lo.end(),Gecode::Int::Limits::max);
        std::fill(hi.begin(),hi.end(),Gecode::Int::Limits::min);
        found = false;
        gap = -1;
        supports(hull,m.p.min(),0,tuple,lo,hi,found,&gap);
        correct &= found;
        if (found) {
          auto largest = std::upper_bound(distance.begin(),distance.end(),gap);
          correct &= (m.p.max() == *--largest);
          if (Gecode::ba(ipl) != Gecode::IPL_BASIC)
            for (int i=0; i<n; i++)
              correct &= (m.x[i].min() == lo[i]) && (m.x[i].max() == hi[i]);
        }
      }
      if (!correct)
        olog << "Variable distance " << distance.front() << ".."
             << distance.back() << ", propagated " << m.x << ", " << m.p
             << std::endl;
      return correct;
    }
  public:
    Bounds(void) : Base("Int::InterDistance::BoundsOracle") {}
    virtual bool run(void) {
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
      // Exhaust all triples of intervals on [-2,3], including ties.
      std::vector<std::vector<int> > intervals;
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
      // Larger sets cover dominance, external adjustments, holes, and
      // arithmetic close to both integer limits. Use a fixed local seed.
      Gecode::Support::RandomGenerator random(2006);
      for (int test=0; test<2000; test++) {
        int n = 2+random(7), p = 2+random(8);
        int offset = (test % 3 == 0) ? Gecode::Int::Limits::min+30 :
          (test % 3 == 1) ? Gecode::Int::Limits::max-30 : 0;
        bool dense = (test % 2 == 0);
        std::vector<std::vector<int> > dom(n);
        for (int i=0; i<n; i++) {
          int l = offset+static_cast<int>(random(21))-10;
          int width = random(5);
          for (int j=0; j<=width; j++)
            if (dense || (j == 0) || (j == width) || random(2))
              dom[i].push_back(l+j);
        }
        if (!check(dom,p,dense))
          return false;
      }
      // A maximal distance exercises deadlines beyond Int::Limits::max.
      const int limit = Gecode::Int::Limits::max;
      if (!check({{-limit,-limit+1},{0,1},{limit-1,limit}},limit,true))
        return false;
      // Adjacent forbidden regions used to let the feasibility pass miss
      // this overload. The maximum feasible separation is six, not seven.
      std::vector<std::vector<int> > adjacent(3);
      for (int v=1; v<=17; v++) adjacent[0].push_back(v);
      for (int v=5; v<=8; v++) adjacent[1].push_back(v);
      for (int v=2; v<=14; v++) adjacent[2].push_back(v);
      if (!check(adjacent,7,true))
        return false;
      for (Gecode::IntPropLevel ipl : {Gecode::IPL_BASIC,Gecode::IPL_ADVANCED,
                                      Gecode::IPL_BASIC_ADVANCED})
        if (!check_variable(adjacent,{0,1,2,3,4,5,6,7,8},true,ipl))
          return false;
      for (int test=0; test<512; test++) {
        int n = 2+random(4);
        int offset = test%3 == 0 ? Gecode::Int::Limits::min+30 :
          test%3 == 1 ? Gecode::Int::Limits::max-30 : 0;
        bool dense = test%2 == 0;
        std::vector<std::vector<int> > dom(n);
        for (int i=0; i<n; i++) {
          int l = offset+static_cast<int>(random(21))-10;
          int width = random(5);
          for (int j=0; j<=width; j++)
            if (dense || (j == 0) || (j == width) || random(2))
              dom[i].push_back(l+j);
        }
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
      // Raise the minimum distance in a clone after an initial fixpoint.
      // Both p's subscription and staging must survive cloning.
      std::vector<std::vector<int> > paper = {{2,3,4,5,6},
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
  public:
    Posting(void) : Base("Int::InterDistance::Posting") {}
    virtual bool run(void) {
      using namespace Gecode;
      Model s;
      IntVar v(s,-2,2);
      IntVarArgs duplicate(2);
      duplicate[0] = duplicate[1] = v;
      inter_distance(s,duplicate,0);
      bool same = false, negative = false, tooLarge = false;
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
      IntVar a(shared,0,9), b(shared,0,9), p(shared,0,9);
      IntVarArgs pair(2);
      pair[0] = a;
      pair[1] = b;
      bool alias = false, repeated = false;
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
      pair[0] = IntVar(extremes,-limit,-limit);
      pair[1] = IntVar(extremes,limit,limit);
      IntVar large(extremes,0,limit);
      inter_distance(extremes,pair,large);
      return (extremes.status() != SS_FAILED) && (large.max() == limit);
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
