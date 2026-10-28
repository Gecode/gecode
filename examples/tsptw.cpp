/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <algorithm>

using namespace Gecode;

/// Instances for TSPTW
namespace {

  /// TSPTW instance
  class Instance {
  public:
    /// Number of nodes, node 0 is the depot
    int n;
    /// Travel times
    const int* d;
    /// Ready and due time for each node
    const int* w;
  };

  /// Instance rbg010a (Ascheuer)
  const int PA_n = 11;
  const int PA_d[PA_n*PA_n] = {
    0,0,0,0,0,0,0,0,0,0,0,
    71,0,85,85,85,96,85,85,97,85,77,
    50,65,0,77,77,62,77,77,62,77,74,
    64,74,88,0,88,80,88,88,82,88,87,
    44,54,68,68,0,60,68,68,62,68,67,
    51,75,51,51,51,0,51,51,80,51,65,
    53,64,78,78,78,68,0,78,69,78,76,
    51,67,69,69,69,75,69,0,75,69,63,
    43,61,71,71,71,43,71,71,0,71,68,
    53,64,78,78,78,68,78,78,69,0,80,
    42,66,52,52,52,70,52,52,70,52,0
  };
  const int PA_w[2*PA_n] = {
    0,9396, 0,865, 399,1299, 550,1450, 685,1585, 819,1719,
    1208,2108, 1412,2312, 1701,2330, 2413,3313, 3798,4698
  };

  /// Instance n20w20.001 (Dumas et al.)
  const int PB_n = 21;
  const int PB_d[PB_n*PB_n] = {
    0,19,17,34,7,20,10,17,28,15,23,29,23,29,21,20,9,16,21,13,12,
    19,0,10,41,26,3,27,25,15,17,17,14,18,48,17,6,21,14,17,13,31,
    17,10,0,47,23,13,26,15,25,22,26,24,27,44,7,5,23,21,25,18,29,
    34,41,47,0,36,39,25,51,36,24,27,38,25,44,54,45,25,28,26,28,27,
    7,26,23,36,0,27,11,17,35,22,30,36,30,22,25,26,14,23,28,20,10,
    20,3,13,39,27,0,26,27,12,15,14,11,15,49,20,9,20,11,14,11,30,
    10,27,26,25,11,26,0,26,31,14,23,32,22,25,31,28,6,17,21,15,4,
    17,25,15,51,17,27,26,0,39,31,38,38,38,34,13,20,26,31,36,28,27,
    28,15,25,36,35,12,31,39,0,17,9,2,11,56,32,21,24,13,11,15,35,
    15,17,22,24,22,15,14,31,17,0,9,18,8,39,29,21,8,4,7,4,18,
    23,17,26,27,30,14,23,38,9,9,0,11,2,48,33,23,17,7,2,10,27,
    29,14,24,38,36,11,32,38,2,18,11,0,13,57,31,20,25,14,13,17,36,
    23,18,27,25,30,15,22,38,11,8,2,13,0,47,34,24,16,7,2,10,26,
    29,48,44,44,22,49,25,34,56,39,48,57,47,0,46,48,31,42,46,40,21,
    21,17,7,54,25,20,31,13,32,29,33,31,34,46,0,11,29,28,32,25,33,
    20,6,5,45,26,9,28,20,21,21,23,20,24,48,11,0,23,19,22,17,32,
    9,21,23,25,14,20,6,26,24,8,17,25,16,31,29,23,0,11,15,9,10,
    16,14,21,28,23,11,17,31,13,4,7,14,7,42,28,19,11,0,5,3,21,
    21,17,25,26,28,14,21,36,11,7,2,13,2,46,32,22,15,5,0,8,25,
    13,13,18,28,20,11,15,28,15,4,10,17,10,40,25,17,9,3,8,0,19,
    12,31,29,27,10,30,4,27,35,18,27,36,26,21,33,32,10,21,25,19,0
  };
  const int PB_w[2*PB_n] = {
    0,408, 62,68, 181,205, 306,324, 214,217, 51,61,
    102,129, 175,186, 250,263, 3,23, 21,49, 79,90,
    78,96, 140,154, 354,386, 42,63, 2,13, 24,42,
    20,33, 9,21, 275,300
  };

  /// The instances
  const Instance instances[] = {
    {PA_n, PA_d, PA_w},
    {PB_n, PB_d, PB_w}
  };
  /// Number of instances
  const unsigned int n_instances = sizeof(instances)/sizeof(Instance);

}

/**
 * \brief %Example: Travelling salesperson with time windows
 *
 * Starting and ending at the depot, visit every customer once such that
 * service at each customer starts within its time window (waiting is
 * allowed). Minimize the total travel time. The instances are from
 * <a href="https://lopez-ibanez.eu/tsptw-instances">López-Ibáñez and
 * Blum's collection</a>.
 *
 * The tour is built from the depot one customer at a time. With the
 * experimental heuristic search engines, the minimal total travel time
 * is the bound. CABS also uses dominance: a partial tour dominates
 * another if it has visited the same customers, ends at the same
 * customer, and is neither later nor more expensive.
 *
 * \ingroup Example
 */
class TSPTW : public IntMinimizeScript {
protected:
  /// The instance
  const Instance& p;
  /// Customer at each position of the tour
  IntVarArray x;
  /// Position of each customer
  IntVarArray pos;
  /// Travel time to each position, and back to the depot
  IntVarArray c;
  /// Start of service at each position
  IntVarArray t;
  /// Total travel time
  IntVar total;
public:
  /// Search engines
  enum {
    SEARCH_BAB, SEARCH_ASTAR, SEARCH_CABS
  };
  /// The actual problem
  TSPTW(const SizeOptions& opt)
    : IntMinimizeScript(opt), p(instances[opt.size()]),
      x(*this,p.n-1,1,p.n-1), pos(*this,p.n-1,0,p.n-2),
      c(*this,p.n,0,Int::Limits::max),
      t(*this,p.n-1,0,p.w[1]), total(*this,0,Int::Limits::max) {
    int n = p.n;
    IntArgs d(n*n,p.d), from(n), to(n), ready(n), due(n);
    for (int i=0; i<n; i++) {
      from[i] = p.d[i]; to[i] = p.d[i*n];
      ready[i] = p.w[2*i]; due[i] = p.w[2*i+1];
    }
    channel(*this, x, 1, pos, 0, opt.ipl());
    // Shortest travel times between customers (not via the depot), as
    // the travel times need not be metric
    IntArgs s(d);
    for (int k=1; k<n; k++)
      for (int i=0; i<n; i++)
        for (int j=0; j<n; j++)
          s[i*n+j] = std::min(s[i*n+j], s[i*n+k]+s[k*n+j]);
    // Customer j precedes k if j cannot be served after k
    for (int j=1; j<n; j++)
      for (int k=1; k<n; k++)
        if ((j != k) && (ready[k] + s[k*n+j] > due[j]))
          rel(*this, pos[j-1] < pos[k-1]);
    element(*this, from, x[0], c[0]);
    rel(*this, t[0] == max(element(ready,x[0]), c[0]));
    for (int i=1; i<n-1; i++) {
      element(*this, d, x[i], n, x[i-1], n, c[i]);
      rel(*this, t[i] == max(element(ready,x[i]), t[i-1]+c[i]));
    }
    for (int i=0; i<n-1; i++)
      rel(*this, t[i] <= element(due,x[i]));
    element(*this, to, x[n-2], c[n-1]);
    rel(*this, t[n-2] + c[n-1] <= due[0]);
    linear(*this, c, IRT_EQ, total);
    // Each branch picks the next customer, as dominance compares prefixes
    branch(*this, x, INT_VAR_NONE(), INT_VALUES_MIN());
  }
  /// Return cost
  virtual IntVar cost(void) const {
    return total;
  }
  /// Return the number of positions assigned from the start
  int prefix(void) const {
    int k = 0;
    while ((k < x.size()) && x[k].assigned())
      k++;
    return k;
  }
  /// Return the set of customers in the first \a k positions
  unsigned long long int visited(int k) const {
    unsigned long long int v = 0;
    for (int i=0; i<k; i++)
      v |= 1ULL << x[i].val();
    return v;
  }
  /// Return the travel time of the first \a k (assigned) positions
  int prefix_cost(int k) const {
    int s = 0;
    for (int i=0; i<k; i++)
      s += c[i].val();
    return s;
  }
  /// Whether this partial tour dominates \a s
  bool dominates(const TSPTW& s) const {
    int k = prefix();
    return (k > 0) && (k == s.prefix()) &&
      (x[k-1].val() == s.x[k-1].val()) && (visited(k) == s.visited(k)) &&
      (t[k-1].val() <= s.t[k-1].val()) &&
      (prefix_cost(k) <= s.prefix_cost(k));
  }
  /// Return a key for dominance
  std::size_t key(void) const {
    int k = prefix();
    std::size_t h = static_cast<std::size_t>(visited(k));
    cmb_hash(h, k > 0 ? x[k-1].val() : 0);
    return h;
  }
  /// Constructor for cloning \a s
  TSPTW(TSPTW& s) : IntMinimizeScript(s), p(s.p) {
    x.update(*this, s.x);
    pos.update(*this, s.pos);
    c.update(*this, s.c);
    t.update(*this, s.t);
    total.update(*this, s.total);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new TSPTW(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tTour: 0";
    for (int i=0; i<x.size(); i++)
      os << " -> " << x[i];
    os << " -> 0" << std::endl
       << "\tTravel time: " << total << std::endl;
  }
};

/** \brief Main-function
 *  \relates TSPTW
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("TSPTW");
  opt.size(0);
  opt.solutions(0);
  opt.search(TSPTW::SEARCH_BAB);
  opt.search(TSPTW::SEARCH_BAB, "bab", "branch and bound");
#ifdef GECODE_HAS_EXPERIMENTAL
  opt.search(TSPTW::SEARCH_ASTAR, "astar", "A*");
  opt.search(TSPTW::SEARCH_CABS, "cabs", "complete anytime beam search");
#endif
  opt.parse(argc,argv);
  if (opt.size() >= n_instances) {
    std::cerr << "Error: size must be between 0 and "
              << n_instances-1 << std::endl;
    return 1;
  }
  switch (opt.search()) {
  case TSPTW::SEARCH_BAB:
    IntMinimizeScript::run<TSPTW,BAB,SizeOptions>(opt);
    break;
#ifdef GECODE_HAS_EXPERIMENTAL
  case TSPTW::SEARCH_ASTAR:
  case TSPTW::SEARCH_CABS:
    {
      Search::HeuristicGuidance g(Search::HeuristicGuidance::MINIMIZE,
        [](const Space& s) -> std::int64_t {
          return static_cast<const TSPTW&>(s).cost().min();
        });
      g.dominates = [](const Space& a, const Space& b) {
        return static_cast<const TSPTW&>(a)
          .dominates(static_cast<const TSPTW&>(b));
      };
      g.dominance_key = [](const Space& s) {
        return static_cast<const TSPTW&>(s).key();
      };
      opt.guidance(&g);
      if (opt.search() == TSPTW::SEARCH_ASTAR)
        IntMinimizeScript::run<TSPTW,AStar,SizeOptions>(opt);
      else
        IntMinimizeScript::run<TSPTW,CABS,SizeOptions>(opt);
    }
    break;
#endif
  }
  return 0;
}

// STATISTICS: example-any
