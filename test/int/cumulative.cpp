/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <gecode/minimodel.hh>
#include <gecode/int/cumulative.hh>

#include <memory>
#include <vector>

namespace Test { namespace Int {

  /// Tests for cumulative scheduling constraints
  namespace Cumulative {

    /**
     * \defgroup TaskTestIntCumulative Cumulative scheduling constraints
     * \ingroup TaskTestInt
     */
    //@{
    /// Test for cumulative constraint with mandatory tasks
    class ManFixPCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// The processing times
      Gecode::IntArgs p;
      /// The resource usage
      Gecode::IntArgs u;
      /// Get a reasonable maximal start time
      static int st(int c,
                    const Gecode::IntArgs& p, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=p.size(); i--; )
          e += static_cast<double>(p[i])*u[i];
        return e / std::max(1,std::abs(c));
      }
      /// Offset
      int o;
    public:
      /// Create and register test
      ManFixPCumulative(int c0,
                        const Gecode::IntArgs& p0,
                        const Gecode::IntArgs& u0,
                        int o0,
                        Gecode::IntPropLevel ipl0)
        : Test("Cumulative::Man::Fix::"+str(o0)+"::"+
               str(c0)+"::"+str(p0)+"::"+str(u0)+"::"+str(ipl0),
               (c0 >= 0) ? p0.size():p0.size()+1,0,st(c0,p0,u0),false,ipl0),
          c(c0), p(p0), u(u0), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomAssignment(arity, dom, 500, _rand);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int cmax = (c >= 0) ? c : x[x.size()-1];
        int n = (c >= 0) ? x.size() : x.size()-1;

        if (c < 0 && x[n] > -c)
          return false;

        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,p[i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          for (int t=0; t<p[i]; t++)
            used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++) {
          for (int t=1; t<p[i]; t++) {
            used[x[i]+t] += u[i];
          }
        }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int n = (c >= 0) ? x.size() : x.size()-1;
        Gecode::IntVarArgs xx;
        if (o==0) {
          xx=x.slice(0,1,n);
        } else {
          xx=Gecode::IntVarArgs(n);
          for (int i=n; i--;)
            xx[i]=Gecode::expr(home,x[i]+o,Gecode::IPL_DOM);
        }
        if (c >= 0) {
          Gecode::cumulative(home, c, xx, p, u, ipl);
        } else {
          Gecode::rel(home, x[n] <= -c);
          Gecode::cumulative(home, x[n], xx, p, u, ipl);
        }
      }
    };


    /// Test for cumulative constraint with optional tasks
    class OptFixPCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// The processing times
      Gecode::IntArgs p;
      /// The resource usage
      Gecode::IntArgs u;
      /// Limit for optional tasks
      int l;
      /// Offset
      int o;
      /// Get a reasonable maximal start time
      static int st(int c,
                    const Gecode::IntArgs& p, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=p.size(); i--; )
          e += static_cast<double>(p[i])*u[i];
        return e / std::max(1,std::abs(c));
      }
    public:
      /// Create and register test
      OptFixPCumulative(int c0,
                        const Gecode::IntArgs& p0,
                        const Gecode::IntArgs& u0,
                        int o0,
                        Gecode::IntPropLevel ipl0)
        : Test("Cumulative::Opt::Fix::"+str(o0)+"::"+
               str(c0)+"::"+str(p0)+"::"+str(u0)+"::"+str(ipl0),
               (c0 >= 0) ? 2*p0.size() : 2*p0.size()+1,0,st(c0,p0,u0),
               false,ipl0),
          c(c0), p(p0), u(u0), l(st(c,p,u)/2), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomAssignment(arity, dom, 500, _rand);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int cmax = (c >= 0) ? c : x[nn];

        if (c < 0 && x[nn] > -c)
          return false;

        int n = nn / 2;
        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,p[i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=0; t<p[i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++) {
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l) {
            for (int t=1; t<p[i]; t++)
              used[x[i]+t] += u[i];
          }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            if (used[x[i]]+u[i] > cmax) {
              delete [] used;
              return false;
            }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn=(c >= 0) ? x.size() : x.size()-1;
        int n=nn / 2;
        Gecode::IntVarArgs s(n);
        Gecode::BoolVarArgs m(n);

        for (int i=0; i<n; i++) {
          s[i]=(c >= 0) ? x[i] : Gecode::expr(home,x[i]+o,Gecode::IPL_DOM);
          m[i]=Gecode::expr(home, x[n+i] > l);
        }

        if (c >= 0) {
          Gecode::cumulative(home, c, s, p, u, m, ipl);
        } else {
          Gecode::rel(home, x[nn] <= -c);
          Gecode::cumulative(home, x[nn], s, p, u, m, ipl);
        }
      }
    };

    /// Test for cumulative constraint with flexible mandatory tasks
    class ManFlexCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// Minimum processing time
      int _minP;
      /// Maximum processing time
      int _maxP;
      /// The resource usage
      Gecode::IntArgs u;
      /// Get a reasonable maximal start time
      static int st(int c, int maxP, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=u.size(); i--; )
          e += static_cast<double>(maxP)*u[i];
        return e / std::max(1,std::abs(c));
      }
      /// Offset
      int o;
    public:
      /// Create and register test
      ManFlexCumulative(int c0, int minP, int maxP,
                        const Gecode::IntArgs& u0,
                        int o0,
                        Gecode::IntPropLevel ipl0)
        : Test("Cumulative::Man::Flex::"+str(o0)+"::"+
               str(c0)+"::"+str(minP)+"::"+str(maxP)+"::"+str(u0)+
               "::"+str(ipl0),
               (c0 >= 0) ? 2*u0.size() : 2*u0.size()+1,
               0,std::max(maxP,st(c0,maxP,u0)),false,ipl0),
          c(c0), _minP(minP), _maxP(maxP), u(u0), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomMixAssignment((c >= 0) ? arity / 2 : arity / 2 + 1,
                                       dom, arity / 2,
                                       Gecode::IntSet(_minP, _maxP), 500, _rand);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn/2;
        int cmax = (c >= 0) ? c : x[n];
        int pstart = (c >= 0) ? n : n+1;

        if (c < 0 && cmax > -c)
          return false;

        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++) {
          t = std::max(t,x[i]+std::max(1,x[pstart+i]));
        }
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          for (int t=0; t<x[pstart+i]; t++)
            used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++) {
          for (int t=1; t<x[pstart+i]; t++)
            used[x[i]+t] += u[i];
        }
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn/2;
        int pstart = (c >= 0) ? n : n+1;
        Gecode::IntVarArgs s(n);
        Gecode::IntVarArgs px(x.slice(pstart,1,n));
        Gecode::IntVarArgs e(home,n,
                             Gecode::Int::Limits::min,
                             Gecode::Int::Limits::max);
        for (int i=s.size(); i--;) {
          s[i] = expr(home, o+x[i], Gecode::IPL_DOM);
          rel(home, s[i]+px[i] == e[i]);
          rel(home, _minP <= px[i]);
          rel(home, _maxP >= px[i]);
        }
        if (c >= 0) {
          Gecode::cumulative(home, c, s, px, e, u, ipl);
        } else {
          rel(home, x[n] <= -c);
          Gecode::cumulative(home, x[n], s, px, e, u, ipl);
        }
      }
    };

    /// Test for cumulative constraint with optional flexible tasks
    class OptFlexCumulative : public Test {
    protected:
      /// Capacity of resource
      int c;
      /// Minimum processing time
      int _minP;
      /// Maximum processing time
      int _maxP;
      /// The resource usage
      Gecode::IntArgs u;
      /// Limit for optional tasks
      int l;
      /// Offset
      int o;
      /// Get a reasonable maximal start time
      static int st(int c, int maxP, const Gecode::IntArgs& u) {
        double e = 0;
        for (int i=u.size(); i--; )
          e += static_cast<double>(maxP)*u[i];
        return e / std::max(1,std::abs(c));
      }
    public:
      /// Create and register test
      OptFlexCumulative(int c0, int minP, int maxP,
                        const Gecode::IntArgs& u0,
                        int o0,
                        Gecode::IntPropLevel ipl0)
        : Test("Cumulative::Opt::Flex::"+str(o0)+"::"+
               str(c0)+"::"+str(minP)+"::"+str(maxP)+"::"+str(u0)+
               "::"+str(ipl0),
               (c0 >= 0) ? 3*u0.size() : 3*u0.size()+1,
               0,std::max(maxP,st(c0,maxP,u0)), false,ipl0),
          c(c0), _minP(minP), _maxP(maxP), u(u0),
          l(std::max(maxP,st(c0,maxP,u0))/2), o(o0) {
        testsearch = false;
        testfix = false;
        contest = CTL_NONE;
      }
      /// Create and register initial assignment
      virtual Assignment* assignment(void) const {
        return new RandomMixAssignment((c >= 0) ? 2 * (arity / 3) : 2 * (arity / 3) + 1,
                                       dom, arity / 3,
                                       Gecode::IntSet(_minP, _maxP), 500, _rand);
      }
      /// Test whether \a x is solution
      virtual bool solution(const Assignment& x) const {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n = nn / 3;
        int cmax = (c >= 0) ? c : x[2*n];
        int pstart = (c >= 0) ? 2*n : 2*n+1;

        if (c < 0 && cmax > -c)
          return false;

        // Compute maximal time
        int t = 0;
        for (int i=0; i<n; i++)
          t = std::max(t,x[i]+std::max(1,x[pstart+i]));
        // Compute resource usage (including at start times)
        int* used = new int[t];
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=0; t<x[pstart+i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage
        for (int i=0; i<t; i++)
          if (used[i] > cmax) {
            delete [] used;
            return false;
          }
        // Compute resource usage (only internal)
        for (int i=0; i<t; i++)
          used[i] = 0;
        for (int i=0; i<n; i++)
          if (x[n+i] > l)
            for (int t=1; t<x[pstart+i]; t++)
              used[x[i]+t] += u[i];
        // Check resource usage at start times
        for (int i=0; i<n; i++)
          if (x[n+i] > l && used[x[i]]+u[i] > cmax) {
            delete [] used;
            return false;
          }
        delete [] used;
        return true;
      }
      /// Post constraint on \a x
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
        int nn = (c >= 0) ? x.size() : x.size()-1;
        int n=nn / 3;
        int pstart= (c >= 0) ? 2*n : 2*n+1;

        Gecode::IntVarArgs s(n);
        Gecode::IntVarArgs px(n);
        Gecode::IntVarArgs e(home,n,
                             Gecode::Int::Limits::min,
                             Gecode::Int::Limits::max);
        for (int i=n; i--;) {
          s[i] = expr(home, o+x[i]);
          px[i] = x[pstart+i];
          rel(home, s[i]+px[i] == e[i]);
          rel(home, _minP <= px[i]);
          rel(home, _maxP >= px[i]);
        }
        Gecode::BoolVarArgs m(n);
        for (int i=0; i<n; i++)
          m[i]=Gecode::expr(home, (x[n+i] > l));
        if (c >= 0) {
          Gecode::cumulative(home, c, s, px, e, u, m, ipl);
        } else {
          Gecode::rel(home, x[2*n] <= -c);
          Gecode::cumulative(home, x[2*n], s, px, e, u, m, ipl);
        }
      }
    };

    /// Focused test for bounded knapsack-augmented overload checking
    class KnapsackAugmentedOverload : public Test::Base {
    private:
      struct Task {
        int e, l, p, d;
        int est(void) const { return e; }
        int lst(void) const { return l; }
        int ect(void) const { return e+p; }
        int lct(void) const { return l+p; }
        int c(void) const { return d; }
      };

      static int scalar_available(const std::vector<int>& demands,
                                  int capacity) {
        std::vector<bool> reachable(static_cast<std::size_t>(capacity+1),
                                    false);
        reachable[0] = true;
        for (int demand : demands)
          for (int value=capacity; value>=demand; value--)
            reachable[static_cast<std::size_t>(value)] =
              reachable[static_cast<std::size_t>(value)] ||
              reachable[static_cast<std::size_t>(value-demand)];
        for (int value=capacity; value>=0; value--)
          if (reachable[static_cast<std::size_t>(value)])
            return value;
        return 0;
      }

      static bool scalar_conflict(const std::vector<Task>& tasks,
                                  int capacity) {
        std::vector<int> order(tasks.size());
        std::vector<int> times;
        for (std::size_t i=0; i<tasks.size(); i++) {
          order[i] = static_cast<int>(i);
          times.push_back(tasks[i].est());
          times.push_back(tasks[i].ect());
          times.push_back(tasks[i].lst());
          times.push_back(tasks[i].lct());
        }
        std::sort(order.begin(),order.end(),[&](int a, int b) {
          return tasks[static_cast<std::size_t>(a)].lct() !=
                 tasks[static_cast<std::size_t>(b)].lct() ?
            tasks[static_cast<std::size_t>(a)].lct() <
              tasks[static_cast<std::size_t>(b)].lct() : a < b;
        });
        std::sort(times.begin(),times.end());
        times.erase(std::unique(times.begin(),times.end()),times.end());
        int earliest = tasks[static_cast<std::size_t>(order[0])].est();
        std::vector<int> prefix;
        for (int index : order) {
          prefix.push_back(index);
          earliest = (std::min)(earliest,
            tasks[static_cast<std::size_t>(index)].est());
          const int finish = tasks[static_cast<std::size_t>(index)].lct();
          long long overflow = 0;
          for (std::size_t point=0; point+1<times.size(); point++) {
            const int left = times[point];
            if ((left < earliest) || (left >= finish))
              continue;
            int fixed = 0;
            int required = 0;
            std::vector<int> free;
            for (const Task& task : tasks)
              if ((task.lst() <= left) && (left < task.ect()))
                fixed += task.c();
            if (fixed > capacity)
              return true;
            for (int member : prefix) {
              const Task& task = tasks[static_cast<std::size_t>(member)];
              const bool compulsory =
                (task.lst() <= left) && (left < task.ect());
              if ((task.est() <= left) && (left < task.lct()) && !compulsory)
                free.push_back(task.c());
              if ((task.est() <= left) &&
                  (left < (std::min)(task.ect(),task.lst())))
                required += task.c();
            }
            const int right = (std::min)(times[point+1],finish);
            overflow = (std::max)(0LL,overflow+
              static_cast<long long>(right-left) *
              (required-scalar_available(free,capacity-fixed)));
          }
          if (overflow > 0)
            return true;
        }
        return false;
      }

      static bool reference_parity(void) {
        std::vector<Task> shapes;
        for (int est=0; est<=1; est++)
          for (int lst=est; lst<=2; lst++)
            for (int duration=1; duration<=2; duration++)
              for (int demand=1; demand<=3; demand++)
                shapes.push_back({est,lst,duration,demand});
        for (std::size_t a=0; a<shapes.size(); a++)
          for (std::size_t b=0; b<shapes.size(); b+=3U)
            for (std::size_t c=0; c<shapes.size(); c+=5U)
              for (std::size_t d=0; d<shapes.size(); d+=7U) {
                std::vector<Task> tasks =
                  {shapes[a],shapes[b],shapes[c],shapes[d]};
                for (int capacity=2; capacity<=3; capacity++) {
                  bool supported = true;
                  bool non_unit = false;
                  for (const Task& task : tasks) {
                    supported = supported && (task.c() <= capacity);
                    non_unit = non_unit || (task.c() != 1);
                  }
                  if (supported && non_unit &&
                      (Gecode::Int::Cumulative::Kaoc::conflict(
                         tasks,capacity) != scalar_conflict(tasks,capacity)))
                    return false;
                }
              }
        return true;
      }

      class Fixture : public Gecode::Space {
      public:
        Gecode::IntVarArray starts;
        Fixture(Gecode::IntPropLevel level)
          : starts(*this,5,0,6) {
          starts[0] = Gecode::IntVar(*this,1,4);
          starts[1] = Gecode::IntVar(*this,0,6);
          starts[2] = Gecode::IntVar(*this,2,6);
          starts[3] = Gecode::IntVar(*this,0,0);
          starts[4] = Gecode::IntVar(*this,0,1);
          Gecode::cumulative(*this,3,starts,
            Gecode::IntArgs({3,1,1,2,4}),
            Gecode::IntArgs({3,3,1,2,1}),level);
        }
        Fixture(Fixture& fixture) : Gecode::Space(fixture) {
          starts.update(*this,fixture.starts);
        }
        virtual Gecode::Space* copy(void) {
          return new Fixture(*this);
        }
      };

      class VariableCapacityFixture : public Gecode::Space {
      public:
        Gecode::IntVarArray starts;
        Gecode::IntVar capacity;
        VariableCapacityFixture(void)
          : starts(*this,5,0,6), capacity(*this,3,4) {
          starts[0] = Gecode::IntVar(*this,1,4);
          starts[1] = Gecode::IntVar(*this,0,6);
          starts[2] = Gecode::IntVar(*this,2,6);
          starts[3] = Gecode::IntVar(*this,0,0);
          starts[4] = Gecode::IntVar(*this,0,1);
          Gecode::cumulative(*this,capacity,starts,
            Gecode::IntArgs({3,1,1,2,4}),
            Gecode::IntArgs({3,3,1,2,1}),Gecode::IPL_ADVANCED);
        }
        VariableCapacityFixture(VariableCapacityFixture& fixture)
          : Gecode::Space(fixture) {
          starts.update(*this,fixture.starts);
          capacity.update(*this,fixture.capacity);
        }
        virtual Gecode::Space* copy(void) {
          return new VariableCapacityFixture(*this);
        }
      };

      static bool variable_capacity_rechecks(void) {
        VariableCapacityFixture root;
        if (root.status() == Gecode::SS_FAILED)
          return false;
        std::unique_ptr<VariableCapacityFixture> clone(
          static_cast<VariableCapacityFixture*>(root.clone()));
        Gecode::rel(*clone,clone->capacity,Gecode::IRT_EQ,3);
        return clone->status() == Gecode::SS_FAILED;
      }

      static bool fixed_end_feasible(void) {
        class FixedEndFixture : public Gecode::Space {
        public:
          Gecode::IntVarArray starts;
          FixedEndFixture(void) : starts(*this,4) {
            Gecode::IntArgs ends({10,20,30,40});
            Gecode::TaskTypeArgs types(4);
            for (int i=0; i<4; i++) {
              starts[i] = Gecode::IntVar(*this,0,ends[i]-1);
              types[i] = Gecode::TT_FIXE;
            }
            Gecode::cumulative(*this,4,types,starts,ends,
              Gecode::IntArgs({2,2,2,2}),Gecode::IPL_ADVANCED);
          }
          FixedEndFixture(FixedEndFixture& fixture) : Gecode::Space(fixture) {
            starts.update(*this,fixture.starts);
          }
          virtual Gecode::Space* copy(void) {
            return new FixedEndFixture(*this);
          }
        };

        FixedEndFixture fixture;
        if (fixture.status() == Gecode::SS_FAILED)
          return false;
        // Each task can run for one unit immediately before its fixed end.
        for (int i=0; i<4; i++)
          Gecode::rel(fixture,fixture.starts[i],Gecode::IRT_EQ,10*(i+1)-1);
        return fixture.status() != Gecode::SS_FAILED;
      }

      static bool bit_boundaries(void) {
        using Gecode::Int::Cumulative::Kaoc::Bits;
        using Gecode::Int::Cumulative::Kaoc::add;
        using Gecode::Int::Cumulative::Kaoc::max_reachable;
        const int capacities[] = {63,64,65,127};
        for (int capacity : capacities) {
          for (int first=1; first<=capacity; first+=7) {
            Bits bits = {1ULL,0ULL};
            std::vector<int> demands;
            for (int demand=first; demand<=capacity; demand+=13) {
              add(bits,demand,capacity);
              demands.push_back(demand);
              for (int residual=0; residual<=capacity; residual++)
                if (max_reachable(bits,residual) !=
                    scalar_available(demands,residual))
                  return false;
            }
          }
        }
        Bits gap = {1ULL,0ULL};
        add(gap,4,9);
        add(gap,6,9);
        return max_reachable(gap,9) == 6;
      }

      static bool gates_and_optional_subset(void) {
        std::vector<Task> all_unit(4,Task{0,1,1,1});
        std::vector<Task> parity(4,Task{0,5,2,2});
        std::vector<Task> too_few(3,Task{0,1,1,2});
        std::vector<Task> too_many(65,Task{0,1,1,2});
        std::vector<Task> bad_demand(4,Task{0,1,1,1});
        bad_demand[0].d = 4;
        if (Gecode::Int::Cumulative::Kaoc::conflict(all_unit,3) ||
            !Gecode::Int::Cumulative::Kaoc::conflict(parity,3) ||
            Gecode::Int::Cumulative::Kaoc::conflict(parity,1) ||
            Gecode::Int::Cumulative::Kaoc::conflict(parity,128) ||
            Gecode::Int::Cumulative::Kaoc::conflict(too_few,3) ||
            Gecode::Int::Cumulative::Kaoc::conflict(too_many,3) ||
            Gecode::Int::Cumulative::Kaoc::conflict(bad_demand,3))
          return false;

        class OptionalSubsetFixture : public Gecode::Space {
        public:
          OptionalSubsetFixture(Gecode::IntPropLevel level) {
            const int n = 69;
            Gecode::IntVarArgs starts(n);
            Gecode::BoolVarArgs mandatory(n);
            Gecode::IntArgs processing(n);
            Gecode::IntArgs demand(n);
            for (int i=0; i<n; i++) {
              starts[i] = Gecode::IntVar(*this,2*i,2*i);
              mandatory[i] = Gecode::BoolVar(*this,i < 5 ? 1 : 0,1);
              processing[i] = 1;
              demand[i] = 2+(i & 1);
            }
            const int earliest[] = {1,0,2,0,0};
            const int latest[] = {4,6,6,0,1};
            const int durations[] = {3,1,1,2,4};
            const int demands[] = {3,3,1,2,1};
            for (int i=0; i<5; i++) {
              starts[i] = Gecode::IntVar(*this,earliest[i],latest[i]);
              processing[i] = durations[i];
              demand[i] = demands[i];
            }
            Gecode::cumulative(*this,3,starts,processing,demand,mandatory,
                               level);
          }
          OptionalSubsetFixture(OptionalSubsetFixture& fixture)
            : Gecode::Space(fixture) {}
          virtual Gecode::Space* copy(void) {
            return new OptionalSubsetFixture(*this);
          }
        };

        OptionalSubsetFixture basic(Gecode::IPL_BASIC);
        OptionalSubsetFixture advanced(Gecode::IPL_ADVANCED);
        return (basic.status() != Gecode::SS_FAILED) &&
               (advanced.status() == Gecode::SS_FAILED);
      }

    public:
      KnapsackAugmentedOverload(void)
        : Test::Base("Cumulative::KnapsackAugmentedOverload") {}

      virtual bool run(void) {
        Fixture basic(Gecode::IPL_BASIC);
        Fixture advanced(Gecode::IPL_ADVANCED);
        const bool boundaries = bit_boundaries();
        const bool parity = reference_parity();
        const bool variable = variable_capacity_rechecks();
        const bool fixed_end = fixed_end_feasible();
        const bool gates = gates_and_optional_subset();
        const bool basic_ok = basic.status() != Gecode::SS_FAILED;
        const bool advanced_ok = advanced.status() == Gecode::SS_FAILED;
        if (!(boundaries && parity && variable && fixed_end && gates &&
              basic_ok && advanced_ok))
          std::cerr << "kaoc boundaries=" << boundaries
                    << " parity=" << parity << " variable=" << variable
                    << " fixed_end=" << fixed_end
                    << " gates=" << gates
                    << " basic=" << basic_ok
                    << " advanced=" << advanced_ok << "\n";
        return boundaries && parity && variable && fixed_end && gates &&
          basic_ok && advanced_ok;
      }
    };

    KnapsackAugmentedOverload knapsackAugmentedOverload;

    /// Help class to create and register tests
    class Create {
    public:
      /// Perform creation and registration
      Create(void) {
        using namespace Gecode;
        IntArgs p1({1,1,1,1});
        IntArgs p2({2,2,2,2});
        IntArgs p3({4,3,3,5});
        IntArgs p4({4,0,3,5});
        IntArgs p5({1,1,1});

        IntArgs u1({1,1,1,1});
        IntArgs u2({2,2,2,2});
        IntArgs u3({2,3,4,5});
        IntArgs u4({2,3,0,5});
        IntArgs u5({1,3,2});

        for (IntPropBasicAdvanced ipba; ipba(); ++ipba) {
          // Regression test: check correct detection of disjunctive case
          (void) new ManFixPCumulative(3,p5,u5,0,ipba.ipl());

          for (int c=-7; c<8; c++) {
            int off = 0;
            for (int coff=0; coff<2; coff++) {
              (void) new ManFixPCumulative(c,p1,u1,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p1,u2,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p1,u3,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p1,u4,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p2,u1,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p2,u2,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p2,u3,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p2,u4,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p3,u1,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p3,u2,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p3,u3,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p3,u4,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p4,u1,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p4,u2,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p4,u3,off,ipba.ipl());
              (void) new ManFixPCumulative(c,p4,u4,off,ipba.ipl());

              (void) new ManFlexCumulative(c,0,1,u1,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,1,u2,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,1,u3,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,1,u4,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,2,u1,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,2,u2,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,2,u3,off,ipba.ipl());
              (void) new ManFlexCumulative(c,0,2,u4,off,ipba.ipl());
              (void) new ManFlexCumulative(c,3,5,u1,off,ipba.ipl());
              (void) new ManFlexCumulative(c,3,5,u2,off,ipba.ipl());
              (void) new ManFlexCumulative(c,3,5,u3,off,ipba.ipl());
              (void) new ManFlexCumulative(c,3,5,u4,off,ipba.ipl());

              (void) new OptFixPCumulative(c,p1,u1,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p1,u2,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p1,u3,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p1,u4,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p2,u1,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p2,u2,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p2,u3,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p2,u4,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p3,u1,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p3,u2,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p3,u3,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p3,u4,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p4,u1,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p4,u2,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p4,u3,off,ipba.ipl());
              (void) new OptFixPCumulative(c,p4,u4,off,ipba.ipl());

              (void) new OptFlexCumulative(c,0,1,u1,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,1,u2,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,1,u3,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,1,u4,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,2,u1,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,2,u2,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,2,u3,off,ipba.ipl());
              (void) new OptFlexCumulative(c,0,2,u4,off,ipba.ipl());
              (void) new OptFlexCumulative(c,3,5,u1,off,ipba.ipl());
              (void) new OptFlexCumulative(c,3,5,u2,off,ipba.ipl());
              (void) new OptFlexCumulative(c,3,5,u3,off,ipba.ipl());
              (void) new OptFlexCumulative(c,3,5,u4,off,ipba.ipl());

              off = Gecode::Int::Limits::min;
            }
          }
        }
      }
    };

    Create c;
    //@}

  }
}}

// STATISTICS: test-int
