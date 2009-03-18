/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2009
 *
 *  Last modified:
 *     $Date: 2009-02-05 21:48:53 +1100 (Do, 05 Feb 2009) $ by $Author: schulte $
 *     $Revision: 8155 $
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

#include "examples/support.hh"
#include <gecode/minimodel.hh>

/// Instance data for radio therapy problem
class RadiotherapyData {
  /// Compute incremental sum
  int incr_sum(int row) {
    int sum = intensity[row*n];
    for (int i=1; i<n; i++)
      sum += std::max(intensity[row*n+i]-intensity[row*n+i-1],0);
    return sum;
  }
public:
  const int  m; //< Height of intensity matrix
  const int  n; //< Width of intensity matrix
  const int* intensity; //< Intensity matrix

  int btMin;   //< Minimal beam time (computed from other parameters)
  int btMax;   //< Maximal beam time (computed from other parameters)
  int intsSum; //< Sum of all intensities

  /// Construct instance data
  RadiotherapyData(int m0, int n0, const int* intensity0)
  : m(m0), n(n0), intensity(intensity0) {
    btMax = 0;
    intsSum = 0;
    for (int i=0; i<m*n; i++) {
      btMax = std::max(btMax, intensity[i]);
      intsSum += intensity[i];
    }

    btMin = 0;
    for (int i=0; i<m; i++)
      btMin = std::max(btMin, incr_sum(i));
  }
};

namespace {

  // Small instance
  static const int intensity0[] = {
     7,  2, 14,  8,  9,
    13,  4,  1,  2,  9,
     5, 12,  2, 11,  9,
    10,  2,  4,  9,  7,
    10,  2,  8, 11,  1
  };
  RadiotherapyData rd0(5,5,intensity0);

  // Larger instance
  static const int intensity1[] = {
   6, 10,  6,  8, 10,  0,  4, 10,  0,  6,  2,  8,  0,  2,  0 ,
   1,  8,  3,  1,  0,  8,  0,  3,  6, 10,  9,  8,  9,  6,  9 ,
   8,  5,  6,  7,  7,  0,  6,  8,  2,  7,  5,  2,  0,  9,  2 ,
   9,  2, 10,  5,  7,  1,  3,  7,  5,  1,  8,  2,  3, 10,  4 ,
   8,  7,  4,  1,  6,  3,  0,  1,  2,  6,  4,  4,  0,  5,  0 ,
   9,  0,  7,  4,  9,  7,  4,  1,  4,  1,  1,  9,  2,  9,  9 ,
   3,  6, 10,  0,  6,  6, 10, 10,  7,  0, 10,  2, 10,  2,  4 ,
   8,  9,  5,  2,  6,  1,  9,  0,  4,  2,  4,  1,  5,  1,  4 ,
   6, 10,  0,  0,  7,  0,  0,  5,  8,  5, 10,  3,  2,  2, 10 ,
   4,  3,  0,  6, 10,  7,  2,  7,  2,  9,  2,  8,  9,  7,  9 ,
  10,  2,  0,  5,  5,  1,  3,  7,  1,  6,  5,  4,  2,  8,  1 ,
   3,  6,  4,  3,  7, 10,  6,  7,  7,  6,  5,  9, 10,  8,  3 ,
   9,  9,  5,  2,  4,  2,  3,  3,  1,  2,  9,  2,  5,  6,  3 ,
   7,  5,  2,  6,  4,  8,  1,  0,  2,  4,  7,  9,  3,  3,  0 ,
   5,  3,  8,  7, 10,  6,  7,  7,  6, 10,  4,  4,  5,  8,  0
  };
  RadiotherapyData rd1(15,15,intensity1);

  RadiotherapyData rds[] = {rd0, rd1};
  const unsigned int rds_n = sizeof(rds) / sizeof(RadiotherapyData);
}

/**
 * \brief %Example: Radiotherapy
 *
 * Implementation of the model for cancer radiation treatment planning
 * from the paper
 *
 *   Davaatseren Baatar, Natashia Boland, Sebastian Brand, Peter J. Stuckey.
 *   Minimum Cardinality Matrix Decomposition into Consecutive-Ones Matrices:
 *   CP and IP Approaches. CPAIOR 2007
 *
 * \ingroup ExProblem
 *
 */

class Radiotherapy : public MinimizeExample {
private:
  /// Instance data
  const RadiotherapyData rd;
  
  /// Total beam-on time
  IntVar beamtime;
  /// Number of shape matrices
  IntVar K;
  /// n[b] is the number of shape matrices with associated beam-on time b
  IntVarArray N;
  /// Q[i,j,b] is the number of shape matrices with associated beam-on time b that expose cell (i,j)
  IntVarArray q;

  /// Cost to be minimized
  IntVar _cost;

public:
  /// The actual problem
  Radiotherapy(const SizeOptions& opt)
  : rd(rds[opt.size()]) {

    // Initialize variables
    beamtime = IntVar(*this, rd.btMin, rd.intsSum);
    K        = IntVar(*this, 0, rd.m*rd.n);
    N        = IntVarArray(*this, rd.btMax, 0, rd.m*rd.n);
    q        = IntVarArray(*this, rd.m*rd.n*rd.btMax, 0, rd.m*rd.n);

    IntArgs coeffs(rd.btMax);
    for (int i=0; i<rd.btMax; i++)
      coeffs[i] = i+1;
    linear(*this, coeffs, N, IRT_EQ, beamtime);
    linear(*this, N, IRT_EQ, K);
    
    for (int i=0; i<rd.m; i++) {
      for (int j=0; j<rd.n; j++) {
        IntVarArgs qs(rd.btMax);
        for (int b=0; b<rd.btMax; b++)
          qs[b] = q[i*rd.n*rd.btMax+j*rd.btMax+b];
        linear(*this, coeffs, qs, IRT_EQ, rd.intensity[i*rd.n+j], ICL_DOM);
      }
    }
    
    for (int i=0; i<rd.m; i++) {
      for (int b=0; b<rd.btMax; b++) {
        IntVarArgs qs(rd.n);
        for (int j=0; j<rd.n; j++)
          qs[j] = q[i*rd.n*rd.btMax+j*rd.btMax+b];
        incr_sum(N[b], qs, rd.m*rd.n);
      }
    }

    _cost = IntVar(*this, 0, (rd.m*rd.n+1)*(rd.intsSum+1));
    post(*this, _cost == beamtime*(rd.m*rd.n+1)+K);

    // First branch over beamtime and N
    IntVarArgs ba(1); ba[0] = beamtime;
    branch(*this, ba, INT_VAR_NONE, INT_VAL_MIN);
    branch(*this, N, INT_VAR_NONE, INT_VAL_SPLIT_MIN);

    // Then perform a nested search over q
    NestedSearch::post(*this);

  }

  /// Post incremental sum constraint 
  void incr_sum(IntVar& x, IntVarArgs& y, int mn) {
    IntVarArray s(*this, y.size()-1, 0, mn);
    IntVarArgs t(y.size());
    t[0] = y[0];
    for (int i=1; i<y.size(); i++) {
      post(*this, s[i-1] >= y[i]-y[i-1]);
      t[i] = s[i-1];
    }
    linear(*this, t, IRT_LQ, x);
  }

  /// Constructor for cloning \a s
  Radiotherapy(bool share, Radiotherapy& s)
  : MinimizeExample(share,s), rd(s.rd) {
    beamtime.update(*this, share, s.beamtime);
    N.update(*this, share, s.N);
    K.update(*this, share, s.K);
    _cost.update(*this, share, s._cost);      
    q.update(*this, share, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Radiotherapy(share,*this);
  }

  /// Cost to be minimized
  virtual IntVar
  cost(void) const { return _cost; }
  
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << std::endl 
       << "B / K = " << beamtime << " / " << K << ",\nN = " << N << std::endl;
  }

  /// Nested search on the q variables
  class NestedSearch : public Branching {
  private:
    /// Flag that the branching is done after one commit
    bool done;
    /// Description that only signals failure or success
    class Description : public BranchingDesc {
    public:
      /// Whether branching should fail
      bool fail;
      /// Initialize description for branching \a b
      Description(const Branching& b, bool fail0)
      : BranchingDesc(b,1), fail(fail0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Description);
      }
    };
    /// Construct branching
    NestedSearch(Space& home) : Branching(home), done(false) {}
    /// Copy constructor
    NestedSearch(Space& home, bool share, NestedSearch& b)
      : Branching(home, share, b), done(b.done) {}
  public:
    virtual bool status(const Space&) const {
      return !done;
    }

    IntVarArgs getRow(Radiotherapy* row, int i) {
      IntVarArgs ri(row->rd.n*row->rd.btMax);
      for (int j=0; j<row->rd.n; j++) {
        for (int b=0; b<row->rd.btMax; b++) {
          ri[j*row->rd.btMax+b] = 
            row->q[i*row->rd.n*row->rd.btMax+j*row->rd.btMax+b];
        }
      }
      return ri;
    }

    virtual BranchingDesc* description(Space& home) {
      done = true;
      Radiotherapy& rt = static_cast<Radiotherapy&>(home);

      std::cerr << "*";
      
      // Perform nested search for each row
      bool fail = false;
      for (int i=0; i<rt.rd.m; i++) {
        // Create fresh clone for row i
        Radiotherapy* row = static_cast<Radiotherapy*>(rt.clone());

        // Branch over row i
        branch(*row, getRow(row, i), INT_VAR_NONE, INT_VAL_SPLIT_MIN);
        Search::Options o; o.clone = false;
        if ( Radiotherapy* newSol = dfs(row, o) ) {
          // Found a solution for row i, so try to find one for i+1
          delete newSol;
          std::cerr << i+1;
        } else {
          // Found no solution for row i, so back to search the N variables
          fail = true;
          break;
        }      
      }
      // delete nest;
      return new Description(*this, fail);
    }
    virtual ExecStatus commit(Space&, const BranchingDesc& d, unsigned int) {
      return static_cast<const Description&>(d).fail ? ES_FAILED : ES_OK;
    }
    /// Copy branching
    virtual Actor* copy(Space& home, bool share) {
      return new (home) NestedSearch(home, share, *this);
    }
    /// Post branching
    static void post(Space& home) {
      (void) new (home) NestedSearch(home);
    }
  };

};

/** \brief Main-function
 *  \relates Radiotherapy
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Radiotherapy");
  opt.solutions(0);
  opt.size(0);
  opt.parse(argc,argv);

  if (opt.size() >= rds_n) {
    std::cerr << "Error: size must be between 0 and "
              << rds_n-1 << std::endl;
    return 1;
  }

  MinimizeExample::run<Radiotherapy,BAB,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
