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

#include <memory>

namespace Test { namespace Int { namespace MinDistance {

  /// Distances with both a zero off-diagonal pair and threshold boundaries
  Gecode::IntArgs matrix(void) {
    return Gecode::IntArgs({0,0,3,4, 0,0,2,1, 3,2,0,4, 4,1,4,0});
  }

  /// Exhaustive assignments and incremental clone/prune checks
  class Minimum : public Test {
  protected:
    int positions;
    bool computed;
    bool requirements;
    int alias;
    Gecode::MinDistancePropKind kind;
  public:
    Minimum(int n, bool f, bool r, int a,
            Gecode::MinDistancePropKind k, Gecode::IntPropLevel level)
      : Test(TestTags::check(),
             "MinDistance::"+str(n)+"::"+str(f)+"::"+str(r)+"::"+
             str(a)+"::"+str(k == Gecode::MDP_SINGLE)+"::"+str(level),
             n+1,0,4,false,level),
        positions(n), computed(f), requirements(r), alias(a), kind(k) {
      contest = CTL_NONE;
    }
    virtual bool solution(const Assignment& a) const {
      int minimum = 4;
      Gecode::IntArgs m = matrix();
      for (int i=0; i<positions; i++) {
        if (a[i] >= 4)
          return false;
        for (int j=i+1; j<positions; j++) {
          if (a[j] >= 4)
            return false;
          int first = a[alias == 1 && i == 1 ? 0 : i];
          int second = a[alias == 1 && j == 1 ? 0 : j];
          int distance = m[first*4+second];
          if (requirements && (i == 0) && (j == positions-1) &&
              (distance < 3))
            return false;
          minimum = std::min(minimum,distance);
        }
      }
      return (a[positions] == minimum) &&
        ((alias != 2) || (a[positions] == a[0]));
    }
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& a) {
      using namespace Gecode;
      IntArgs m = matrix();
      IntDistance d = computed ?
        IntDistance(4,[m](int i, int j) { return m[i*4+j]; }) :
        IntDistance(4,m);
      IntVarArgs x(positions);
      for (int i=0; i<positions; i++) {
        Gecode::dom(home,a[i],0,3);
        x[i] = a[alias == 1 && i == 1 ? 0 : i];
      }
      if (alias == 2)
        rel(home,a[positions],IRT_EQ,a[0]);
      IntVar z = a[alias == 2 ? 0 : positions];
      if (requirements) {
        IntArgs r(positions*positions);
        for (int i=0; i<r.size(); i++)
          r[i] = 0;
        r[positions-1] = r[(positions-1)*positions] = 3;
        minimum_distance(home,x,z,d,r,ipl,kind);
      } else {
        minimum_distance(home,x,z,d,ipl,kind);
      }
    }
  };

  /// Small space for direct propagation checks
  class Model : public Gecode::Space {
  public:
    Gecode::IntVarArray x;
    Gecode::IntVar z;
    Model(int n, int sites, int lower, int upper,
          Gecode::MinDistancePropKind kind, Gecode::IntPropLevel level,
          std::function<int(int,int)> distance)
      : x(*this,n,0,sites-1), z(*this,lower,upper) {
      Gecode::minimum_distance(*this,x,z,
                              Gecode::IntDistance(sites,distance),level,kind);
    }
    Model(Model& s) : Gecode::Space(s) {
      x.update(*this,s.x); z.update(*this,s.z);
    }
    virtual Gecode::Space* copy(void) { return new Model(*this); }
  };

  /// Matching certificates, zero witnesses, and callback lifetime
  class Propagation : public Base {
  public:
    Propagation(void) : Base("Int::MinDistance::Propagation",
                            TestTags::check()) {}
    virtual bool run(void) {
      using namespace Gecode;
      const MinDistancePropKind kinds[] = {MDP_DECOMPOSED,MDP_SINGLE};
      const IntPropLevel levels[] = {IPL_BASIC,IPL_ADVANCED};
      for (MinDistancePropKind kind : kinds)
        for (IntPropLevel level : levels) {
          // Forward filtering includes equality at the threshold.
          Model forward(2,4,2,4,kind,level,
                        [](int a, int b) { return std::abs(a-b); });
          rel(forward,forward.x[0],IRT_EQ,0);
          if ((forward.status() == SS_FAILED) ||
              (forward.x[1].min() != 2) || (forward.z.max() != 3))
            return false;
          std::unique_ptr<Model> clone(static_cast<Model*>(forward.clone()));
          rel(*clone,clone->x[1],IRT_EQ,2);
          if ((clone->status() == SS_FAILED) || !clone->z.assigned() ||
              (clone->z.val() != 2) || forward.z.assigned())
            return false;

          // With only one pair, zero requires equal selections here.
          Model zero(2,4,0,0,kind,level,
                     [](int a, int b) { return std::abs(a-b); });
          dom(zero,zero.x[0],IntSet({0,1}));
          dom(zero,zero.x[1],IntSet({1,2}));
          if ((zero.status() == SS_FAILED) ||
              !zero.x[0].assigned() || (zero.x[0].val() != 1) ||
              !zero.x[1].assigned() || (zero.x[1].val() != 1))
            return false;

          // Assigned objectives still need exact equality, not just >= z.
          Model invalid(2,4,1,1,kind,level,
                        [](int a, int b) { return std::abs(a-b); });
          rel(invalid,invalid.x[0],IRT_EQ,0);
          rel(invalid,invalid.x[1],IRT_EQ,3);
          if (invalid.status() != SS_FAILED)
            return false;

          // One active site can be repeated at objective zero.
          Model repeated(3,1,0,3,kind,level,
                         [](int, int) { return 0; });
          if ((repeated.status() == SS_FAILED) || !repeated.z.assigned() ||
              (repeated.z.val() != 0))
            return false;
        }

      // A perfect conflict matching rules out selecting three sites at 3.
      auto conflicts = [](int a, int b) {
        return a == b ? 0 : (a/2 == b/2 ? 1 : 3);
      };
      for (MinDistancePropKind kind : kinds) {
        Model basic(3,4,1,3,kind,IPL_BASIC,conflicts);
        Model matching(3,4,1,3,kind,IPL_ADVANCED,conflicts);
        if ((basic.status() == SS_FAILED) || (basic.z.max() != 3) ||
            (matching.status() == SS_FAILED) || (matching.z.max() != 1))
          return false;
      }

      // Share a captured object across stable clones and release it with
      // the last Space. Computed distances must not build an n*n matrix.
      std::weak_ptr<int> lifetime;
      std::unique_ptr<Model> clone;
      {
        auto data = std::make_shared<int>(1);
        lifetime = data;
        Model original(2,1000000,1,3,MDP_SINGLE,IPL_BASIC,
                       [data](int a, int b) { return a == b ? 0 : *data; });
        // Restrict domains before propagation to avoid enumerating the
        // million-site universe; construction itself makes no distance calls.
        dom(original,original.x,0,2);
        if (original.status() == SS_FAILED)
          return false;
        clone.reset(static_cast<Model*>(original.clone()));
      }
      if (lifetime.expired() || (clone->status() == SS_FAILED))
        return false;
      clone.reset();
      if (!lifetime.expired())
        return false;
      return true;
    }
  };

  /// Matrix and callback validation
  class Arguments : public Base {
  public:
    Arguments(void) : Base("Int::MinDistance::Arguments",
                            TestTags::check()) {}
    virtual bool run(void) {
      using namespace Gecode;
      try {
        IntDistance invalid(2,IntArgs({0,1,2,0}));
        return false;
      } catch (const Gecode::Int::IllegalOperation&) {}
      try {
        IntDistance invalid(2,IntArgs({0,-1,-1,0}));
        return false;
      } catch (const Gecode::Int::OutOfLimits&) {}
      try {
        IntDistance invalid(2,std::function<int(int,int)>());
        return false;
      } catch (const InvalidFunction&) {}
      try {
        IntDistance invalid(2,[](int, int) { return -1; });
        (void) invalid(0,1);
        return false;
      } catch (const Gecode::Int::OutOfLimits&) {}
      try {
        Model space(2,4,0,4,MDP_DECOMPOSED,IPL_BASIC,
                    [](int a, int b) { return std::abs(a-b); });
        IntVarArgs x;
        IntVar z(space,0,4);
        minimum_distance(space,x,z,IntDistance(4,matrix()));
        return false;
      } catch (const Gecode::Int::TooFewArguments&) {}
      return true;
    }
  };

  /// Register the compact semantic matrix
  class Create {
  public:
    Create(void) {
      const Gecode::MinDistancePropKind kinds[] =
        {Gecode::MDP_DECOMPOSED,Gecode::MDP_SINGLE};
      const Gecode::IntPropLevel levels[] =
        {Gecode::IPL_BASIC,Gecode::IPL_ADVANCED};
      for (int n : {2,3})
        for (bool computed : {false,true})
          for (bool requirements : {false,true})
            for (Gecode::MinDistancePropKind kind : kinds)
              for (Gecode::IntPropLevel level : levels)
                (void) new Minimum(n,computed,requirements,0,kind,level);
      for (Gecode::MinDistancePropKind kind : kinds)
        for (Gecode::IntPropLevel level : levels)
          (void) new Minimum(3,true,true,1,kind,level);
      for (Gecode::MinDistancePropKind kind : kinds)
        for (Gecode::IntPropLevel level : levels)
          (void) new Minimum(3,true,false,2,kind,level);
      (void) new Propagation;
      (void) new Arguments;
    }
  };
  Create create;

}}}

// STATISTICS: test-int
