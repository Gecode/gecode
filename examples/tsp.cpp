/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  Bugfixes provided by:
 *     Geoffrey Chu
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <algorithm>

using namespace Gecode;

/// Support for %TSP instances
namespace {

  /// This instance is taken from SICStus Prolog
  const int PA_n = 7;
  const int PA_d[PA_n*PA_n] = {
    0,205,677,581,461,878,345,
    205,0,882,427,390,1105,540,
    677,882,0,619,316,201,470,
    581,427,619,0,412,592,570,
    461,390,316,412,0,517,190,
    878,1105,201,592,517,0,691,
    345,540,470,570,190,691,0
  };

  /// This instance is taken from SICStus Prolog
  const int PB_n = 10;
  const int PB_d[PB_n*PB_n] = {
    2,4,4,1,9,2,4,4,1,9,
    2,9,5,5,5,2,9,5,5,5,
    1,5,2,3,3,1,5,2,3,3,
    2,6,8,9,5,2,6,8,9,5,
    3,7,1,6,4,3,7,1,6,4,
    1,2,4,1,7,1,2,4,1,7,
    3,5,2,7,6,3,5,2,7,6,
    2,7,9,5,5,2,7,9,5,5,
    3,9,7,3,4,3,9,7,3,4,
    4,1,5,9,2,4,1,5,9,2
  };

  /// This instance is br17.atsp from TSPLIB
  const int PC_n = 17;
  const int PC_d[PC_n*PC_n] = {
    0,3,5,48,48,8,8,5,5,3,3,0,3,5,8,8,5,
    3,0,3,48,48,8,8,5,5,0,0,3,0,3,8,8,5,
    5,3,0,72,72,48,48,24,24,3,3,5,3,0,48,48,24,
    48,48,74,0,0,6,6,12,12,48,48,48,48,74,6,6,12,
    48,48,74,0,0,6,6,12,12,48,48,48,48,74,6,6,12,
    8,8,50,6,6,0,0,8,8,8,8,8,8,50,0,0,8,
    8,8,50,6,6,0,0,8,8,8,8,8,8,50,0,0,8,
    5,5,26,12,12,8,8,0,0,5,5,5,5,26,8,8,0,
    5,5,26,12,12,8,8,0,0,5,5,5,5,26,8,8,0,
    3,0,3,48,48,8,8,5,5,0,0,3,0,3,8,8,5,
    3,0,3,48,48,8,8,5,5,0,0,3,0,3,8,8,5,
    0,3,5,48,48,8,8,5,5,3,3,0,3,5,8,8,5,
    3,0,3,48,48,8,8,5,5,0,0,3,0,3,8,8,5,
    5,3,0,72,72,48,48,24,24,3,3,5,3,0,48,48,24,
    8,8,50,6,6,0,0,8,8,8,8,8,8,50,0,0,8,
    8,8,50,6,6,0,0,8,8,8,8,8,8,50,0,0,8,
    5,5,26,12,12,8,8,0,0,5,5,5,5,26,8,8,0
  };

  /// This instance is ftv33.atsp from TSPLIB
  const int PD_n = 34;
  const int PD_d[PD_n*PD_n] = {
    0,26,82,65,100,147,134,69,117,42,89,125,38,13,38,31,22,103,
    143,94,104,123,98,58,38,30,67,120,149,100,93,162,62,66,66,0,
    56,39,109,156,140,135,183,108,155,190,104,79,104,97,88,130,176,121,
    131,150,125,85,65,57,94,147,160,80,67,189,128,40,43,57,0,16,
    53,100,84,107,155,85,132,168,81,56,81,74,65,146,186,137,147,166,
    141,101,81,73,110,163,164,102,71,205,105,62,27,41,62,0,97,144,
    131,96,144,69,116,152,65,40,65,58,49,130,170,121,131,150,125,85,
    65,57,94,147,166,86,73,189,89,46,109,135,161,174,0,47,34,54,
    102,67,114,175,97,96,128,135,131,198,193,203,213,232,207,167,147,139,
    176,229,222,204,148,235,60,175,157,171,114,130,60,0,40,114,162,127,
    174,235,157,156,188,188,179,258,253,251,239,258,203,215,195,187,172,207,
    175,157,101,295,120,133,143,169,132,148,34,31,0,88,133,101,148,209,
    131,130,162,169,165,232,227,237,247,266,221,201,181,173,190,225,193,175,
    119,269,94,151,95,121,177,160,54,101,88,0,48,53,100,158,83,82,
    114,121,117,184,179,189,199,218,193,153,133,125,162,215,244,195,188,221,
    46,161,79,105,161,144,91,138,125,37,0,37,53,114,67,66,98,105,
    101,137,132,149,183,202,177,137,117,109,146,199,228,179,172,174,57,145,
    42,68,124,107,67,114,101,27,75,0,47,108,30,29,61,68,64,131,
    126,136,146,165,140,100,80,72,109,162,191,142,135,168,20,108,83,109,
    165,148,108,155,142,68,88,41,0,61,71,70,102,109,105,84,79,96,
    144,163,175,141,121,113,150,203,232,183,176,121,61,149,204,230,286,269,
    216,255,237,162,125,162,123,0,192,191,223,230,226,144,139,156,184,165,
    215,249,242,234,251,282,332,297,297,113,182,270,38,64,120,103,88,135,
    122,57,105,30,77,87,0,25,31,38,47,110,105,122,142,161,136,96,
    76,68,105,158,187,138,131,147,50,104,13,39,95,78,87,134,121,56,
    104,29,76,112,25,0,32,39,35,116,130,107,117,136,111,71,51,43,
    80,133,162,113,106,172,49,79,38,48,104,87,119,166,153,88,136,61,
    108,118,31,32,0,7,16,123,136,114,124,143,118,78,58,50,87,140,
    169,120,115,178,81,88,31,41,97,80,115,162,149,84,132,57,104,114,
    27,28,7,0,9,116,132,107,117,136,111,71,51,43,80,133,162,113,
    108,174,77,81,22,32,88,71,122,169,156,91,139,64,111,123,36,35,
    16,9,0,107,141,98,108,127,102,62,42,34,71,124,153,104,99,166,
    84,72,108,134,190,173,133,180,167,93,113,66,85,60,96,95,127,134,
    130,0,46,63,116,135,147,166,146,138,175,221,257,208,201,120,86,174,
    127,153,209,192,152,199,186,112,132,85,104,79,115,114,146,153,149,19,
    0,17,70,89,101,135,148,157,137,175,219,183,220,85,105,193,153,179,
    235,218,178,225,212,138,158,111,130,105,141,140,172,179,175,45,57,0,
    53,72,84,118,131,183,120,158,202,166,241,68,131,214,179,165,199,204,
    243,290,277,203,223,176,195,165,206,192,199,192,183,110,112,82,0,19,
    31,65,78,149,67,105,149,113,188,95,196,161,212,205,239,244,237,284,
    271,197,217,170,189,146,200,199,231,232,223,104,93,63,40,0,71,105,
    118,189,107,117,167,153,228,76,190,201,148,134,168,173,212,259,246,172,
    192,145,164,139,175,161,168,161,152,79,125,70,36,55,0,34,47,118,
    36,89,118,82,157,131,165,130,153,146,180,185,178,225,212,138,158,111,
    130,105,141,140,172,173,164,45,91,36,46,65,77,0,59,130,48,101,
    130,94,169,104,131,142,173,166,200,205,198,245,232,158,178,131,150,125,
    161,160,192,193,184,65,111,56,66,85,97,20,0,150,68,121,150,114,
    189,124,151,162,30,16,72,55,125,172,156,99,147,72,119,133,68,43,
    50,43,34,73,119,64,74,93,68,28,8,0,37,90,119,70,83,132,
    92,56,112,98,132,137,185,232,216,181,223,154,195,170,150,125,132,125,
    116,110,156,101,67,86,31,65,78,82,0,53,82,46,121,162,174,94,
    144,130,164,169,217,256,225,213,261,186,233,234,182,157,164,157,148,174,
    209,165,131,116,95,129,122,114,93,0,50,78,147,192,206,126,94,80,
    114,119,167,214,198,163,211,136,183,197,132,107,114,107,98,137,183,128,
    110,129,74,92,72,64,43,57,0,28,103,196,156,76,66,52,101,91,
    154,201,185,135,183,108,155,169,104,79,86,79,70,109,155,100,82,101,
    46,64,44,36,15,68,97,0,90,168,128,63,113,108,70,86,84,131,
    115,138,186,151,198,225,151,126,142,135,126,165,211,156,138,157,102,120,
    100,92,71,124,93,56,0,224,144,32,146,172,228,211,171,218,205,131,
    151,104,123,80,134,133,165,172,168,38,27,44,75,76,106,140,153,176,
    142,180,224,188,239,0,124,212,102,128,184,167,61,108,95,7,55,60,
    107,165,90,89,121,128,124,191,186,196,206,225,200,160,140,132,169,222,
    251,202,195,228,0,168,81,95,38,54,91,138,122,145,193,123,170,206,
    119,94,119,112,103,184,224,175,165,184,129,139,119,111,98,151,120,83,
    27,243,143,0
  };

  /// Problem instance
  class Problem {
  private:
    const int  _n; ///< Size
    const int* _d; ///< Distances
  public:
    /// Initialize problem instance
    Problem(const int n, const int* d);
    /// Return size of instance
    int size(void) const;
    /// Return distance between node \a i and \a j
    int d(int i, int j) const;
    /// Return distances
    const int* d(void) const;
    /// Return estimate for maximal cost of a path
    int max(void) const;
  };

  inline
  Problem::Problem(const int n, const int* d)
    : _n(n), _d(d) {}
  inline int
  Problem::size(void) const {
    return _n;
  }
  inline int
  Problem::d(int i, int j) const {
    return _d[i*_n+j];
  }
  inline const int*
  Problem::d(void) const {
    return _d;
  }
  inline int
  Problem::max(void) const {
    int m=0;
    for (int i=_n*_n; i--; )
      m = std::max(m,_d[i]);
    return m*_n;
  }

  Problem PA(PA_n,PA_d);
  Problem PB(PB_n,PB_d);
  Problem PC(PC_n,PC_d);
  Problem PD(PD_n,PD_d);

  Problem ps[] = {PA,PB,PC,PD};
  const unsigned int ps_n = sizeof(ps) / sizeof(Problem);

}

/**
 * \brief %Example: Travelling salesman problem (%TSP)
 *
 * Simple travelling salesman problem instances. Just meant
 * as a test for circuit.
 *
 * \ingroup Example
 *
 */
class TSP : public MinimizeScript {
protected:
  /// Problem instance to be solved
  Problem     p;
  /// Successor edges
  IntVarArray succ;
  /// Total cost of travel
  IntVar      total;
public:
  /// Actual model
  TSP(const SizeOptions& opt)
    : p(ps[opt.size()]),
      succ(*this, p.size(), 0, p.size()-1),
      total(*this, 0, p.max()) {
    int n = p.size();

    // Cost matrix
    IntArgs c(n*n, p.d());

    for (int i=n; i--; )
      for (int j=n; j--; )
        if (p.d(i,j) == 0)
          rel(*this, succ[i], IRT_NQ, j);

    // Cost of each edge
    IntVarArgs costs(*this, n, Int::Limits::min, Int::Limits::max);

    // Enforce that the succesors yield a tour with appropriate costs
    circuit(*this, c, succ, costs, total, opt.icl());

    // Just assume that the circle starts forwards
    {
      IntVar p0(*this, 0, n-1);
      element(*this, succ, p0, 0);
      rel(*this, p0, IRT_LE, succ[0]);
    }

    // First enumerate cost values, prefer those that maximize cost reduction
    branch(*this, costs, INT_VAR_REGRET_MAX_MAX, INT_VAL_SPLIT_MIN);

    // Then fix the remaining successors
    branch(*this, succ,  INT_VAR_MIN_MIN, INT_VAL_MIN);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return total;
  }
  /// Constructor for cloning \a s
  TSP(bool share, TSP& s) : MinimizeScript(share,s), p(s.p) {
    succ.update(*this, share, s.succ);
    total.update(*this, share, s.total);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new TSP(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    bool assigned = true;
    for (int i=0; i<succ.size(); i++) {
      if (!succ[i].assigned()) {
        assigned = false;
        break;
      }
    }
    if (assigned) {
      os << "\tTour: ";
      int i=0;
      do {
        os << i << " -> ";
        i=succ[i].val();
      } while (i != 0);
      os << 0 << std::endl;
      os << "\tCost: " << total << std::endl;
    } else {
      os << "\tTour: " << std::endl;
      for (int i=0; i<succ.size(); i++) {
        os << "\t" << i << " -> " << succ[i] << std::endl;
      }
      os << "\tCost: " << total << std::endl;
    }
  }
};



/** \brief Main-function
 *  \relates TSP
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("TSP");
  opt.solutions(0);
  opt.icl(ICL_DOM);
  opt.parse(argc,argv);

  if (opt.size() >= ps_n) {
    std::cerr << "Error: size must be between 0 and "
              << ps_n-1 << std::endl;
    return 1;
  }

  MinimizeScript::run<TSP,BAB,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
