/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Claude-Guy Quimper
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Claude-Guy Quimper, 2006
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

#include <gecode/int/inter-distance.hh>
#include <gecode/int/distinct.hh>

#include <algorithm>
#include <limits>
#include <map>

namespace Gecode { namespace Int { namespace InterDistance {

  namespace {
    typedef long long int Time;
    typedef std::ptrdiff_t RegionIndex;

    /// Release time, deadline, and position in release-time order
    struct Task {
      Time min, max;
      int minRank, maxRank;
    };

    /// Forbidden starting times (inclusive)
    struct Forbidden {
      Time min, max;
    };

    /**
     * \brief Linear-space feasibility checker of Garey et al. (1981)
     *
     * Algorithm B represents each deadline's critical time as its deadline
     * minus its task load and pseudo-offset. A Fenwick tree maintains loads.
     * A weighted union-find merges deadlines with the same residue modulo p;
     * an ordered map locates residues affected by a forbidden region. Each
     * deadline is inserted once and each merge removes a residue, giving
     * O(n log n) time. Relevant deadlines are kept in deadline order, with
     * dominated predecessors removed permanently (Lemmas 5 and 6).
     */
    class Feasibility {
      const int n;
      int dc;
      Task* tasks;
      Task** release;
      Time* deadlines;
      int* loads;
      int* successor;
      int* previous;
      int* parent;
      int* size;
      Time* offset;
      /// First remaining relevant deadline at or after i
      int find(int i) {
        int root = i;
        while (successor[root] != root)
          root = successor[root];
        while (successor[i] != i) {
          int next = successor[i];
          successor[i] = root;
          i = next;
        }
        return root;
      }
      /// Number of processed tasks with a deadline at or before i
      int load(int i) const {
        int count = 0;
        for (size_t j=static_cast<size_t>(i)+1; j; j-=j & -j)
          count += loads[j];
        return count;
      }
      /// Sum weighted links, including the root's shared offset
      Time displacement(int i) const {
        if (parent[i] == -1)
          return 0;
        Time d = offset[i];
        while (parent[i] != i) {
          i = parent[i];
          d += offset[i];
        }
        return d;
      }
      /// Merge roots while preserving each deadline's displacement
      int merge(int a, int b) {
        if (size[a] < size[b])
          std::swap(a,b);
        parent[b] = a;
        offset[b] -= offset[a];
        size[a] += size[b];
        return a;
      }
      /// Nonnegative remainder, including for negative deadlines
      static Time residue(Time t, Time p) {
        Time q = t % p;
        return (q < 0) ? q+p : q;
      }
      /// Pseudo-critical time for deadline i
      Time critical(int i, Time p) const {
        return deadlines[i]+p-static_cast<Time>(load(i))*p-displacement(i);
      }
    public:
      Feasibility(Region& r, const ViewArray<IntView>& x)
        : n(x.size()), dc(0) {
        tasks = r.alloc<Task>(n);
        release = r.alloc<Task*>(n);
        Task** finish = r.alloc<Task*>(n);
        deadlines = r.alloc<Time>(n);
        for (int i=0; i<n; i++) {
          tasks[i].min = x[i].min();
          tasks[i].max = x[i].max();
          release[i] = finish[i] = tasks+i;
        }
        std::sort(release,release+n,[](const Task* a, const Task* b) {
          return (a->min < b->min) ||
            ((a->min == b->min) && (a->max < b->max));
        });
        std::sort(finish,finish+n,[](const Task* a, const Task* b) {
          return a->max < b->max;
        });
        for (int i=0; i<n; i++) {
          if ((i == 0) || (finish[i]->max != deadlines[dc-1]))
            deadlines[dc++] = finish[i]->max;
          finish[i]->maxRank = dc-1;
        }
        const size_t count = static_cast<size_t>(dc)+1;
        loads = r.alloc<int>(count);
        successor = r.alloc<int>(count);
        previous = r.alloc<int>(dc);
        parent = r.alloc<int>(dc);
        size = r.alloc<int>(dc);
        offset = r.alloc<Time>(dc);
      }
      /// Test interval feasibility, reusing the sorted bounds and scratch
      bool check(int p) {
        if (p == 0)
          return true;
        std::fill(loads,loads+dc+1,0);
        for (int i=0; i<dc; i++) {
          successor[i] = i;
          previous[i] = i-1;
          parent[i] = -1;
        }
        successor[dc] = dc;
        std::map<Time,int> fractions;
        int active = dc, pending = dc-1;
        for (int i=n; i--;) {
          int rank = release[i]->maxRank;
          for (size_t j=static_cast<size_t>(rank)+1;
               j<=static_cast<size_t>(dc); j+=j & -j)
            loads[j]++;
          int d = find(rank);
          active = std::min(active,d);
          Time c = critical(d,p);
          while ((previous[d] >= 0) && (critical(previous[d],p) > c)) {
            int obsolete = previous[d];
            successor[obsolete] = d;
            previous[d] = previous[obsolete];
            if (active == obsolete)
              active = d;
          }
          if ((i > 0) && (release[i-1]->min == release[i]->min))
            continue;
          c = critical(active,p);
          Time r = release[i]->min;
          if (c < r)
            return false;
          if (c-p+1 >= r)
            continue;
          // Only deadlines whose back-schedule can reach this region
          // are activated. Earlier deadlines must retain zero offsets.
          while ((pending >= 0) && (deadlines[pending]+p >= c)) {
            parent[pending] = pending;
            size[pending] = 1;
            offset[pending] = 0;
            Time q = residue(deadlines[pending]+p,p);
            auto entry = fractions.emplace(q,pending);
            if (!entry.second)
              entry.first->second = merge(entry.first->second,pending);
            pending--;
          }
          Time a = residue(c-p,p), b = residue(r,p);
          auto target = fractions.find(a);
          int root = (target == fractions.end()) ? -1 : target->second;
          // The forbidden region is open: (c-p,r). At a wraparound,
          // residue zero is included precisely when it is below b.
          auto consume = [&](std::map<Time,int>::iterator it, Time end) {
            while ((it != fractions.end()) && (it->first < end)) {
              int group = it->second;
              offset[group] += residue(it->first-a,p);
              root = (root < 0) ? group : merge(root,group);
              it = fractions.erase(it);
            }
          };
          if (a < b) {
            consume(fractions.upper_bound(a),b);
          } else {
            consume(fractions.upper_bound(a),p);
            consume(fractions.begin(),b);
          }
          if (root >= 0)
            fractions[a] = root;
        }
        return true;
      }
    };

    /// Adjustment interval in a doubly linked list sorted by minimum
    struct Adjustment {
      Time min, max;
      RegionIndex previous, next;
    };

    /**
     * \brief Scratch data for one direction of bounds filtering
     *
     * The Garey-Johnson-Simons-Tarjan feasibility pass constructs forbidden
     * starting regions. An earliest-completion table and a latest-start
     * vector allow the dominance sweep to generate only quadratically many
     * adjustment intervals. The linked list and union-find propagate changes
     * to bounds without rescanning all variables for each interval.
     */
    class Filter {
    public:
      const int n;
      const Time p;
      Task* tasks;
      Task** minSorted;
      Task** maxSorted;
      Forbidden* forbidden;
      int forbiddenCount;
      Time* ectTable;
      Time* lstTable;
      Adjustment* regions;
      RegionIndex regionCount;
      RegionIndex* firstRegion;
      RegionIndex* nextRegion;
      Time* bounds;
      int* links;
      bool* processed;
      int* leader;
      /// Allocate scratch storage, with no persistent ownership
      Filter(Region& r, int n0, int p0)
        : n(n0), p(p0), forbiddenCount(0), regionCount(0) {
        const size_t count = static_cast<size_t>(n);
        const size_t limit = std::numeric_limits<size_t>::max();
        // Region::alloc multiplies unchecked. Reject impossible byte sizes
        // before allocating the quadratic tables or interval list.
        if ((count > limit / sizeof(Time) / (count+1)) ||
            (count > (limit / sizeof(Adjustment) - 3) / (count+3)))
          throw MemoryExhausted();
        tasks = r.alloc<Task>(n);
        minSorted = r.alloc<Task*>(n);
        maxSorted = r.alloc<Task*>(n);
        forbidden = r.alloc<Forbidden>(n);
        const size_t entries = count * (count+1);
        ectTable = static_cast<Time*>(r.ralloc(entries * sizeof(Time)));
        lstTable = r.alloc<Time>(n+2);
        const size_t capacity = count * (count+3) / 2 + 3;
        regions = static_cast<Adjustment*>(
          r.ralloc(capacity * sizeof(Adjustment)));
        firstRegion = r.alloc<RegionIndex>(n);
        nextRegion = r.alloc<RegionIndex>(n);
        bounds = r.alloc<Time>(n);
        links = r.alloc<int>(n);
        processed = r.alloc<bool>(n);
        leader = r.alloc<int>(n);
      }
      /// Earliest completion time for q tasks at release time i
      Time ect(int i, int q) const {
        return ectTable[static_cast<size_t>(q)*n+i];
      }
      /// Latest start time for q tasks at the current deadline
      Time lst(int q) const {
        return lstTable[q];
      }
      /// Find the representative of a merged group of bounds
      int find(int i) {
        int j = i;
        while (links[j] > j)
          j = links[j];
        while (links[i] > i) {
          int k = links[i];
          links[i] = j;
          i = k;
        }
        return j;
      }
      /// Notify bounds of a new or expanded adjustment interval
      int notify(RegionIndex r, int vi) {
        assert((vi >= 0) && (links[vi] < vi));
        while ((links[vi] >= 0) &&
               (bounds[links[vi]] >= regions[r].min))
          vi = links[vi];
        while (regions[r].min <= bounds[vi]) {
          if (bounds[vi] <= regions[r].max) {
            while ((vi < n-1) &&
                   (regions[r].max+1 >= minSorted[vi+1]->min)) {
              int previous = links[vi];
              links[vi] = vi+1;
              vi = find(vi);
              links[vi] = previous;
            }
            if (regions[r].max >= bounds[vi])
              bounds[vi] = regions[r].max+1;
          }
          if (regions[r].min < regions[nextRegion[vi]].min)
            r = nextRegion[vi];
          else
            nextRegion[vi] = r;
          r = regions[r].next;
        }
        assert((vi >= 0) && (links[vi] < vi));
        return vi;
      }
      /// Sort bounds and compute forbidden regions; detect infeasibility
      bool initialize(Region& r) {
        for (int i=0; i<n; i++)
          minSorted[i] = maxSorted[i] = tasks+i;
        std::sort(minSorted,minSorted+n,[](const Task* a, const Task* b) {
          return (a->min < b->min) ||
            ((a->min == b->min) && (a->max < b->max));
        });
        std::sort(maxSorted,maxSorted+n,[](const Task* a, const Task* b) {
          return (a->max < b->max) ||
            ((a->max == b->max) && (a->min < b->min));
        });
        Time* deadlines = r.alloc<Time>(n);
        int dc = 0;
        for (int i=0; i<n; i++) {
          minSorted[i]->minRank = i;
          if ((i == 0) || (maxSorted[i]->max > deadlines[dc-1]))
            deadlines[dc++] = maxSorted[i]->max;
          maxSorted[i]->maxRank = dc-1;
        }
        Time* times = r.alloc<Time>(dc);
        int* cursor = r.alloc<int>(dc);
        Forbidden* buffer = r.alloc<Forbidden>(n);
        // Indices avoid both the one-past-end write and the before-begin
        // pointer formed by the original 2006 implementation.
        int last = n-1;
        for (int i=0; i<dc; i++) {
          times[i] = deadlines[i];
          cursor[i] = last;
        }
        Time min = std::numeric_limits<Time>::max();
        Time newMin = min;
        forbiddenCount = 0;
        for (int i=n; i--;) {
          for (int j=minSorted[i]->maxRank; j<dc; j++) {
            times[j] -= p;
            while ((cursor[j] > last) &&
                   (times[j] < buffer[cursor[j]].min))
              cursor[j]--;
            if ((cursor[j] > last) &&
                (times[j] <= buffer[cursor[j]].max))
              times[j] = buffer[cursor[j]--].min-1;
            newMin = std::min(newMin,times[j]);
          }
          if ((i == 0) || (minSorted[i-1]->min < minSorted[i]->min))
            min = newMin;
          if (min < minSorted[i]->min)
            return false;
          if (min-p+1 < minSorted[i]->min) {
            if ((forbiddenCount > 0) &&
                (minSorted[i]->min >= buffer[last+1].min)) {
              // Adjacent integer regions must also be merged: skipping one
              // region must not land on a forbidden start in its neighbour.
              buffer[last+1].min = min-p+1;
            } else {
              assert(last >= 0);
              buffer[last--] = {min-p+1,minSorted[i]->min-1};
              forbiddenCount++;
            }
          }
        }
        for (int i=0; i<forbiddenCount; i++)
          forbidden[i] = buffer[last+1+i];
        for (int i=0; i<n; i++) {
          ectTable[i] = minSorted[i]->min;
          int er = 0;
          for (int q=1; q<=n; q++) {
            Time start = ect(i,q-1);
            while ((er < forbiddenCount) && (forbidden[er].max < start))
              er++;
            if ((er < forbiddenCount) && (forbidden[er].min <= start))
              start = forbidden[er].max+1;
            ectTable[static_cast<size_t>(q)*n+i] = start+p;
          }
        }
        return true;
      }
      /// Apply internal adjustments, then external adjustments by deadline
      void prune(void) {
        const Time infinity = std::numeric_limits<Time>::max();
        regions[0] = {-infinity,-infinity,-1,1};
        regions[1] = {infinity,infinity,0,-1};
        regionCount = 2;
        RegionIndex lastRegion = 0;
        for (int i=0; i<n; i++) {
          links[i] = i-1;
          processed[i] = false;
          bounds[i] = minSorted[i]->min;
          nextRegion[i] = 0;
        }
        int minP = n, maxP = 0;
        for (int it=0; it<n; it++) {
          int i = maxSorted[it]->minRank;
          processed[i] = true;
          maxP = std::max(maxP,i);
          minP = std::min(minP,i);
          int l = leader[i] = i;
          int kp = 1;
          while (++l <= maxP)
            if (processed[l])
              kp++;
          l = i;
          int k = kp;
          if (i > minP) {
            while (!processed[--l]) {}
            k++;
            while (l > leader[l]) {
              while (!processed[--l]) {}
              k++;
            }
            if (ect(i,1) > ect(l,k-kp+1)) {
              l = i;
              k = kp;
            }
          }
          kp = k;
          int j = l;
          while (j < maxP) {
            while (!processed[++j]) {}
            if (ect(j,1) <= ect(l,k- --kp+1))
              leader[j] = l;
            else
              break;
          }
          if ((it < n-1) && (maxSorted[it]->max == maxSorted[it+1]->max))
            continue;
          // Only one deadline is needed at a time. Computing this vector
          // per deadline keeps quadratic time but removes a quadratic
          // latest-start table. The extra entry serves external adjustments.
          lstTable[0] = maxSorted[it]->max;
          int lr = forbiddenCount-1;
          for (int t=1; t<=it+2; t++) {
            Time start = lst(t-1)-p;
            while ((lr >= 0) && (forbidden[lr].min > start))
              lr--;
            if ((lr >= 0) && (forbidden[lr].max >= start))
              start = forbidden[lr].min-1;
            lstTable[t] = start;
          }
          int q = 0;
          RegionIndex insertion = 0;
          int vi = n-1;
          while (true) {
            for (int t=q; t<k; t++) {
              regions[regionCount].min = lst(t+1)+1;
              // The final empty region allows external adjustments to
              // expand the preceding intervals by one more task.
              regions[regionCount].max = (l >= 0) ? ect(l,k-t)-1 : -infinity;
              if (t > 0) {
                while (regions[insertion+1].min >= regions[insertion].min) {
                  insertion = regions[insertion].previous;
                  if (insertion >= 0) {
                    if (regions[insertion].min <= regions[regionCount].min) {
                      insertion--;
                      break;
                    }
                  } else {
                    break;
                  }
                }
                insertion++;
                while (regions[regions[insertion].next].min <
                       regions[regionCount].min)
                  insertion = regions[insertion].next;
              } else {
                insertion = lastRegion;
                firstRegion[i] = lastRegion = regionCount;
              }
              assert(regions[insertion].min <= regions[regionCount].min);
              RegionIndex next = regions[insertion].next;
              regions[regionCount].next = next;
              regions[insertion].next = regionCount;
              regions[regionCount].previous = insertion;
              regions[next].previous = regionCount;
              assert(regions[regionCount].min <= regions[next].min);
              vi = notify(regionCount++,vi);
            }
            if (l == -1)
              break;
            q = k++;
            if (l > minP) {
              while (!processed[--l]) {}
              j = leader[l];
              while (j < l) {
                while (!processed[--l]) {}
                k++;
              }
            } else {
              l = -1;
            }
          }
        }
        // A variable is filtered before adding external intervals for its
        // deadline, so it is never excluded by an interval it helped create.
        for (int it=0; it<n; it++) {
          int i = maxSorted[it]->minRank;
          maxSorted[it]->min = bounds[find(i)];
          if ((it == n-1) || (maxSorted[it]->max != maxSorted[it+1]->max)) {
            int vi = n-1;
            Time lastMax = regions[firstRegion[i]].max;
            for (RegionIndex r=firstRegion[i]+1;
                 (r < regionCount) && (regions[r].min < regions[r-1].min);
                 r++) {
              Time oldMax = regions[r].max;
              regions[r].max = lastMax;
              lastMax = oldMax;
              vi = notify(r,vi);
            }
          }
        }
      }
    };

    /// Minimum separation in an assigned tuple, without quadratic tables
    Time assigned_distance(Region& r, const ViewArray<IntView>& x) {
      Time* values = r.alloc<Time>(x.size());
      for (int i=0; i<x.size(); i++)
        values[i] = x[i].val();
      std::sort(values,values+x.size());
      Time gap = std::numeric_limits<Time>::max();
      for (int i=1; i<x.size(); i++)
        gap = std::min(gap,values[i]-values[i-1]);
      return gap;
    }
  }

  template<class PView>
  Bnd<PView>::Bnd(Home home, ViewArray<IntView>& x0, PView p0, bool a)
    : MixNaryOnePropagator<IntView,PC_INT_BND,PView,PC_INT_BND>(home,x0,p0),
      advanced(a) {}

  template<class PView>
  Bnd<PView>::Bnd(Space& home, Bnd& b)
    : MixNaryOnePropagator<IntView,PC_INT_BND,PView,PC_INT_BND>(home,b),
      advanced(b.advanced) {}

  template<class PView>
  Propagator*
  Bnd<PView>::copy(Space& home) {
    return new (home) Bnd(home,*this);
  }

  template<class PView>
  PropCost
  Bnd<PView>::cost(const Space&, const ModEventDelta& med) const {
    // ME_INT_DOM is a synthetic event: subscriptions only generate bound
    // or assignment events. It schedules the quadratic stage separately.
    if (IntView::me(med) == ME_INT_DOM)
      return PropCost::quadratic(PropCost::HI,x.size());
    return PropCost::linear(PropCost::HI,x.size());
  }

  template<class PView>
  size_t
  Bnd<PView>::dispose(Space& home) {
    (void) MixNaryOnePropagator<IntView,PC_INT_BND,PView,PC_INT_BND>::dispose(home);
    return sizeof(*this);
  }

  template<class PView>
  ExecStatus
  Bnd<PView>::post(Home home, ViewArray<IntView>& x, PView p, bool advanced) {
    if (x.size() > 1)
      (void) new (home) Bnd(home,x,p,advanced);
    return ES_OK;
  }

  template<class PView>
  ExecStatus
  Bnd<PView>::propagate(Space& home, const ModEventDelta& med) {
    Region region;
    bool assigned = true;
    int min = x[0].min(), max = x[0].max();
    for (int i=0; i<x.size(); i++) {
      min = std::min(min,x[i].min());
      max = std::max(max,x[i].max());
      assigned &= x[i].assigned();
    }
    if (assigned) {
      GECODE_ME_CHECK(y.lq(home,assigned_distance(region,x)));
      return home.ES_SUBSUMED(*this);
    }
    if (y.max() == 0)
      return home.ES_SUBSUMED(*this);
    if (y.assigned() && (y.val() == 1))
      GECODE_REWRITE(*this,Distinct::Bnd<IntView>::post(home(*this),x));
    const int p = y.min();
    if (IntView::me(med) != ME_INT_DOM) {
      Time limit = (static_cast<Time>(max)-min)/(x.size()-1);
      int upper = static_cast<int>(std::min(limit,static_cast<Time>(y.max())));
      if (upper < p)
        return ES_FAILED;
      Feasibility f(region,x);
      if (!f.check(p))
        return ES_FAILED;
      if ((upper > p) && !f.check(upper)) {
        // Feasibility is monotone in p. Keep the lower endpoint feasible
        // and the upper endpoint infeasible, without enumerating distances.
        int lower = p;
        while (upper-lower > 1) {
          int middle = lower+(upper-lower)/2;
          if (f.check(middle))
            lower = middle;
          else
            upper = middle;
        }
        upper = lower;
      }
      GECODE_ME_CHECK(y.lq(home,upper));
      if (y.max() == 0)
        return home.ES_SUBSUMED(*this);
      if (y.assigned() && (y.val() == 1))
        GECODE_REWRITE(*this,Distinct::Bnd<IntView>::post(home(*this),x));
      if (!advanced || (p == 0))
        return ES_FIX;
      return home.ES_FIX_PARTIAL(*this,IntView::med(ME_INT_DOM));
    }
    // A zero minimum gives every x bound a support; only the basic stage
    // needs to filter the distance. A later increase wakes that stage again.
    if (p == 0)
      return ES_FIX;
    Filter f(region,x.size(),p);
    bool nofix = false;
    for (int direction=0; direction<2; direction++) {
      for (int i=0; i<x.size(); i++) {
        // Reverse task time: [r,d] becomes [-d,-r]. Variables denote
        // starting times, while the algorithm's maxima are deadlines.
        f.tasks[i].min = direction ? -static_cast<Time>(x[i].max())-p : x[i].min();
        f.tasks[i].max = direction ? -static_cast<Time>(x[i].min()) :
          static_cast<Time>(x[i].max())+p;
      }
      if (!f.initialize(region))
        return ES_FAILED;
      f.prune();
      for (int i=0; i<x.size(); i++) {
        Time bound = direction ? -f.tasks[i].min-p : f.tasks[i].min;
        if (direction ? (bound < x[i].min()) : (bound > x[i].max()))
          return ES_FAILED;
        ModEvent me = direction ? x[i].lq(home,static_cast<int>(bound)) :
          x[i].gq(home,static_cast<int>(bound));
        GECODE_ME_CHECK(me);
        nofix |= (direction ? x[i].max() : x[i].min()) != bound;
      }
    }
    assigned = true;
    for (int i=0; i<x.size(); i++)
      assigned &= x[i].assigned();
    if (assigned) {
      GECODE_ME_CHECK(y.lq(home,assigned_distance(region,x)));
      return home.ES_SUBSUMED(*this);
    }
    // Holes can move actual bounds beyond the computed bounds, invalidating
    // an interval support for x or the distance. Restart the basic stage.
    return nofix ? ES_NOFIX : ES_FIX;
  }

}}}

namespace Gecode {

  void
  inter_distance(Home home, const IntVarArgs& x, int p, IntPropLevel ipl) {
    using namespace Int;
    Limits::nonnegative(p,"Int::inter_distance");
    if ((p > 0) && same(x))
      throw ArgumentSame("Int::inter_distance");
    GECODE_POST;
    if ((p == 0) || (x.size() < 2))
      return;
    ViewArray<IntView> xv(home,x);
    if (p == 1) {
      GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home,xv));
    } else {
      GECODE_ES_FAIL(InterDistance::Bnd<ConstIntView>::post(
        home,xv,ConstIntView(p),ba(ipl) != IPL_BASIC));
    }
  }

  void
  inter_distance(Home home, const IntVarArgs& x, IntVar p, IntPropLevel ipl) {
    using namespace Int;
    if ((x.size() > 1) &&
        (same(x,p) || ((p.max() > 0) && same(x))))
      throw ArgumentSame("Int::inter_distance");
    GECODE_POST;
    IntView pv(p);
    GECODE_ME_FAIL(pv.gq(home,0));
    if ((pv.max() == 0) || (x.size() < 2))
      return;
    if (pv.assigned()) {
      inter_distance(home,x,pv.val(),ipl);
      return;
    }
    ViewArray<IntView> xv(home,x);
    GECODE_ES_FAIL(InterDistance::Bnd<IntView>::post(
      home,xv,pv,ba(ipl) != IPL_BASIC));
  }

}

// STATISTICS: int-post
