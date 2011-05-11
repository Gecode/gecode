/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2009
 *     Mikael Lagerkvist, 2009
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

using namespace Gecode;

/// Instance data for radio therapy problem
class RadiotherapyData {
private:
  /// Compute incremental sum
  int incr_sum(int row) {
    int sum = intensity[row*n];
    for (int i=1; i<n; i++)
      sum += std::max(intensity[row*n+i]-intensity[row*n+i-1],0);
    return sum;
  }
public:
  const int  m; ///< Height of intensity matrix
  const int  n; ///< Width of intensity matrix
  const int* intensity; ///< Intensity matrix

  int btMin;   ///< Minimal beam time (computed from other parameters)
  int btMax;   ///< Maximal beam time (computed from other parameters)
  int intsSum; ///< Sum of all intensities

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
  extern RadiotherapyData rds[];
  extern const unsigned int rds_n;
}

/**
 * \brief %Example: %Radiotherapy
 *
 * Implementation of the model for cancer radiation treatment planning
 * from the paper
 *
 *   Davaatseren Baatar, Natashia Boland, Sebastian Brand, Peter J. Stuckey.
 *   Minimum Cardinality Matrix Decomposition into Consecutive-Ones Matrices:
 *   CP and IP Approaches. CPAIOR 2007
 *
 * \ingroup Example
 *
 */

class Radiotherapy : public MinimizeScript {
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
    rel(*this, _cost == beamtime*(rd.m*rd.n+1)+K);

    // First branch over beamtime and N
    IntVarArgs ba(1); ba[0] = beamtime;
    branch(*this, ba, INT_VAR_NONE, INT_VAL_MIN);
    branch(*this, N, INT_VAR_NONE, INT_VAL_SPLIT_MIN);

    // Then perform a nested search over q
    NestedSearch::post(*this);

  }

  /// Post incremental sum constraint 
  void incr_sum(IntVar& x, IntVarArgs& y, int mn) {
    IntVarArgs s(*this, y.size()-1, 0, mn);
    IntVarArgs t(y.size());
    t[0] = y[0];
    for (int i=1; i<y.size(); i++) {
      rel(*this, s[i-1] >= y[i]-y[i-1]);
      t[i] = s[i-1];
    }
    linear(*this, t, IRT_LQ, x);
  }

  /// Constructor for cloning \a s
  Radiotherapy(bool share, Radiotherapy& s)
  : MinimizeScript(share,s), rd(s.rd) {
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
  class NestedSearch : public Brancher {
  private:
    /// Flag that the brancher is done after one commit
    bool done;
    /// Mapping of index to weight
    struct Idx { 
      int idx;    ///< Index
      int weight; ///< Weight
      /// Sort order (higher weights go first)
      bool operator<(const Idx& rhs) const { return weight > rhs.weight; }
    };
    /// Weighted ordering of rows
    SharedArray<Idx> index;
    /// Choice that only signals failure or success
    class Choice : public Gecode::Choice {
    public:
      /// Whether brancher should fail
      bool fail;
      /// Initialize choice for brancher \a b
      Choice(const Brancher& b, bool fail0)
      : Gecode::Choice(b,1), fail(fail0) {}
      /// Report size occupied
      virtual size_t size(void) const {
        return sizeof(Choice);
      }
      /// Archive into \a e
      virtual void archive(Archive& e) const {
        Gecode::Choice::archive(e);
        e.put(fail);
      }
    };
    /// Construct brancher
    NestedSearch(Space& home) : Brancher(home), done(false) {
      Radiotherapy& rt = static_cast<Radiotherapy&>(home);
      // Set up ordering of rows.  As a heuristic, pre-order the rows
      // with the potentially cheapest ones first.
      index.init(rt.rd.m+1);
      for (int i = rt.rd.m; i--; ) {
        index[i].idx    = i;
        index[i].weight = 0;
        for (int j = rt.rd.n; j--; )
          index[i].weight += rt.rd.intensity[i*rt.rd.n + j] == 0;
      }
      Support::quicksort(&(index[0]), rt.rd.m);
      for (int i = rt.rd.m; i--; )
        index[i].weight = 0;
      index[rt.rd.m].idx = 10;
      // A shared object must be disposed properly
      home.notice(*this, AP_DISPOSE);
    }
    /// Copy constructor
    NestedSearch(Space& home, bool share, NestedSearch& b)
      : Brancher(home, share, b), done(b.done) {
      index.update(home, share, b.index);
    }
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

    virtual Gecode::Choice* choice(Space& home) {
      done = true;
      Radiotherapy& rt = static_cast<Radiotherapy&>(home);

      std::cout << "*";
      
      // Perform nested search for each row
      bool fail = false;
      for (int i=0; i<rt.rd.m; i++) {
        // Create fresh clone for row i
        Radiotherapy* row = static_cast<Radiotherapy*>(rt.clone());

        // Branch over row i
        branch(*row, getRow(row, index[i].idx), 
               INT_VAR_NONE, INT_VAL_SPLIT_MIN);
        Search::Options o; o.clone = false;
        if ( Radiotherapy* newSol = dfs(row, o) ) {
          // Found a solution for row i, so try to find one for i+1
          delete newSol;
          std::cerr << index[i].idx;
        } else {
          // Found no solution for row i, so back to search the N variables
          fail = true;
          index[i].weight += 1;
          if (i && index[i] < index[i-1])
            std::swap(index[i], index[i-1]);
          break;
        }      
      }

      return new Choice(*this, fail);
    }
    virtual Choice* choice(const Space&, Archive& e) {
      bool fail; e >> fail;
      return new Choice(*this, fail);
    }
    virtual ExecStatus commit(Space&, const Gecode::Choice& _c, unsigned int) {
      return static_cast<const Choice&>(_c).fail ? ES_FAILED : ES_OK;
    }
    /// Copy brancher
    virtual Actor* copy(Space& home, bool share) {
      return new (home) NestedSearch(home, share, *this);
    }
    /// Post brancher
    static void post(Home home) {
      (void) new (home) NestedSearch(home);
    }
    /// Dispose member function
    size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      // Periodic scaling of weights
      if (!--index[index.size()-1].idx) {
        index[index.size()-1].idx = 10;
        for (int i = index.size()-1; i--; )
          index[i].weight *= 0.9;
      }
      (void) Brancher::dispose(home);
      (void) index.~SharedArray<Idx>();
      return sizeof(*this);
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

  MinimizeScript::run<Radiotherapy,BAB,SizeOptions>(opt);
  return 0;
}

namespace {
  /** \brief Radiotherapy specifications.
   *  
   *  \relates Radiotherapy
   */
  //@{
  
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

  /*
   * The following 25 clinical instances were provided by 
   *   - James F. Dempsey, ViewRay, Inc.
   *   - H. Edwin Romeijn, Department of Industrial and Operations
   *     Engineering, The University of Michigan
   *   - J. Cole Smith, Department of Industrial and Systems
   *     Engineering, University of Florida
   *   - Z. Caner Taskin, Department of Industrial and Systems
   *     Engineering, University of Florida
   *   - Chunhua Men, Department of Industrial and Systems Engineering,
   *     University of Florida
   * They are from the articles
   *   - "Mixed-Integer Programming Techniques for Decomposing IMRT
   *     Fluence Maps Using Rectangular Apertures", Z. Caner Taskin,
   *     J. Cole Smith, H. Edwin Romeijn
   *   - "Optimal Multileaf Collimator Leaf Sequencing in IMRT Treatment
   *     Planning", Z. Caner Tasin, J. Cole Smith, H. Edwin Romeijn, James
   *     F. Dempsey
   */
  static const int case1_beam1_matrix[] = {
    2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 5, 0, 3, 0, 2,
    3, 1, 0, 0, 0, 0, 0, 0, 18, 0, 0, 4, 6, 0,
    2, 0, 0, 3, 11, 8, 15, 1, 11, 0, 0, 0, 10, 0,
    0, 0, 0, 9, 11, 14, 6, 2, 7, 0, 0, 0, 7, 0,
    0, 8, 2, 7, 10, 11, 7, 2, 0, 7, 0, 0, 0, 1,
    0, 0, 4, 1, 6, 7, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 3, 1, 0, 4, 6, 0, 0, 0, 1, 0, 0, 0, 1,
    0, 1, 5, 6, 8, 8, 5, 0, 2, 0, 0, 0, 7, 0,
    0, 5, 2, 8, 10, 11, 5, 3, 7, 0, 2, 4, 11, 0,
    0, 0, 0, 1, 12, 13, 9, 7, 11, 1, 2, 3, 6, 0,
    0, 0, 0, 0, 0, 0, 0, 4, 20, 0, 0, 8, 5, 0,
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 0, 2,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 1
  };
  RadiotherapyData case1_beam1(15, 14, case1_beam1_matrix);

  static const int case1_beam2_matrix[] = {
    2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 4, 3, 4, 0, 2,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 2, 0, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 1, 0, 0, 3,
    0, 0, 0, 5, 5, 3, 0, 0, 3, 2, 8, 6, 0, 0, 3,
    0, 0, 7, 11, 10, 11, 5, 8, 4, 11, 13, 20, 0, 0, 3,
    0, 10, 10, 9, 7, 7, 7, 2, 9, 0, 0, 0, 9, 0, 2,
    0, 4, 7, 7, 5, 6, 2, 0, 4, 0, 0, 0, 3, 0, 2,
    0, 10, 2, 7, 1, 2, 0, 0, 0, 0, 0, 0, 0, 3, 1,
    0, 0, 5, 6, 3, 1, 0, 6, 8, 0, 0, 0, 0, 1, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2
  };
  RadiotherapyData case1_beam2(11, 15, case1_beam2_matrix);

  static const int case1_beam3_matrix[] = {
    2, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1,
    1, 2, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 1, 2, 2,
    1, 3, 0, 0, 0, 0, 0, 0, 0, 6, 4, 1, 12, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 11, 6, 1, 9, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 2, 11, 0, 0, 0, 2, 1,
    0, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 1, 1,
    0, 3, 0, 2, 6, 7, 6, 6, 4, 0, 0, 0, 0, 0, 2,
    0, 0, 10, 12, 11, 10, 13, 13, 12, 5, 0, 0, 0, 0, 2,
    0, 0, 11, 12, 10, 10, 14, 15, 15, 5, 2, 7, 12, 0, 2,
    0, 9, 5, 9, 7, 6, 12, 16, 13, 8, 5, 7, 7, 0, 2,
    2, 0, 0, 0, 0, 0, 4, 20, 12, 8, 1, 6, 8, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 7, 0, 2,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 1,
    0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 1
  };
  RadiotherapyData case1_beam3(15, 15, case1_beam3_matrix);

  static const int case1_beam4_matrix[] = {
    3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 2,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 13, 0, 4, 0, 2,
    0, 6, 5, 5, 8, 9, 11, 20, 8, 9, 18, 10, 7, 0, 2,
    0, 3, 10, 9, 12, 11, 15, 15, 11, 11, 16, 15, 3, 0, 3,
    0, 5, 7, 12, 14, 11, 15, 15, 13, 10, 15, 10, 5, 0, 3,
    0, 5, 1, 9, 11, 9, 13, 9, 12, 6, 3, 0, 0, 0, 2,
    0, 0, 0, 0, 4, 2, 4, 0, 7, 0, 0, 0, 0, 0, 2,
    2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 1, 7, 4, 0, 0, 0, 10, 10, 4, 0, 1, 1,
    2, 2, 0, 0, 0, 0, 0, 4, 0, 14, 14, 9, 0, 0, 1,
    2, 2, 0, 0, 0, 0, 0, 0, 0, 12, 16, 5, 1, 0, 2,
    2, 2, 0, 0, 0, 0, 0, 0, 1, 10, 12, 6, 3, 0, 2,
    1, 3, 0, 0, 0, 0, 0, 0, 2, 12, 15, 3, 0, 1, 2
  };
  RadiotherapyData case1_beam4(15, 15, case1_beam4_matrix);

  static const int case1_beam5_matrix[] = {
    3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    0, 0, 6, 4, 3, 1, 0, 0, 7, 0, 0, 0, 0, 3, 2,
    0, 13, 6, 1, 1, 1, 5, 0, 2, 0, 0, 0, 0, 1, 2,
    0, 2, 12, 5, 4, 2, 2, 0, 1, 20, 11, 11, 5, 0, 2,
    0, 9, 12, 7, 3, 2, 7, 3, 5, 14, 12, 13, 11, 0, 2,
    0, 5, 11, 13, 6, 6, 5, 5, 5, 15, 11, 13, 12, 0, 2,
    0, 0, 0, 1, 4, 5, 0, 0, 0, 7, 9, 9, 8, 0, 2,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 4, 5, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 9, 0, 3
  };
  RadiotherapyData case1_beam5(11, 15, case1_beam5_matrix);

  static const int case2_beam1_matrix[] = {
    1, 1, 1, 4, 1, 0, 1, 5, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 2, 7, 2, 1,
    1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 12, 12, 5, 8, 5, 3, 0,
    2, 1, 3, 0, 0, 0, 0, 0, 4, 20, 10, 1, 0, 8, 7, 8, 6, 7, 2, 1,
    1, 3, 1, 0, 3, 1, 13, 18, 14, 10, 5, 0, 3, 7, 7, 7, 6, 7, 2, 0,
    2, 0, 0, 0, 3, 1, 9, 8, 8, 6, 2, 3, 4, 5, 11, 10, 9, 10, 3, 0,
    2, 0, 0, 0, 1, 1, 7, 8, 5, 4, 1, 1, 0, 4, 3, 1, 0, 0, 0, 2,
    2, 0, 0, 1, 0, 0, 4, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 4, 2, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    3, 0, 5, 6, 3, 1, 2, 5, 3, 1, 3, 0, 1, 0, 4, 5, 5, 9, 0, 1,
    1, 0, 2, 8, 3, 2, 3, 6, 5, 3, 4, 6, 4, 5, 10, 11, 8, 10, 4, 0,
    0, 0, 0, 8, 5, 4, 5, 8, 5, 7, 6, 5, 3, 5, 8, 7, 7, 10, 2, 0,
    3, 0, 9, 11, 5, 5, 6, 11, 7, 6, 6, 6, 4, 6, 8, 7, 7, 9, 2, 0,
    2, 0, 11, 11, 5, 6, 7, 9, 9, 6, 8, 5, 4, 6, 10, 6, 7, 7, 2, 0,
    2, 0, 6, 11, 4, 3, 1, 0, 0, 0, 0, 4, 7, 6, 2, 0, 0, 3, 0, 2,
    1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 2
  };
  RadiotherapyData case2_beam1(18, 20, case2_beam1_matrix);

  static const int case2_beam2_matrix[] = {
    2, 3, 2, 1, 5, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 3,
    3, 3, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 2,
    3, 3, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 8, 5, 1,
    2, 3, 1, 0, 0, 0, 0, 1, 0, 0, 5, 6, 5, 5, 1, 2, 6, 2, 1,
    2, 2, 2, 0, 0, 0, 0, 8, 0, 4, 2, 5, 2, 7, 5, 1, 4, 2, 1,
    2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 3, 4, 7, 4, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 6, 6, 7, 5, 7, 8, 7, 0,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 10, 7, 2, 3, 5, 12, 6, 0,
    4, 0, 0, 0, 0, 6, 5, 6, 4, 8, 10, 12, 9, 7, 1, 6, 6, 6, 0,
    0, 0, 0, 18, 18, 3, 3, 4, 6, 9, 12, 12, 7, 5, 0, 0, 0, 0, 2,
    0, 0, 0, 20, 11, 0, 1, 4, 5, 10, 10, 8, 6, 1, 6, 3, 4, 1, 3,
    0, 0, 0, 16, 11, 0, 3, 2, 7, 11, 10, 13, 7, 2, 2, 0, 0, 0, 2,
    3, 0, 0, 14, 10, 1, 5, 2, 8, 15, 9, 9, 13, 5, 0, 0, 0, 0, 3,
    2, 0, 0, 16, 9, 5, 5, 4, 7, 18, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    2, 0, 0, 15, 10, 7, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
    2, 0, 0, 0, 18, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2
  };
  RadiotherapyData case2_beam2(17, 19, case2_beam2_matrix);

  static const int case2_beam3_matrix[] = {
    1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 3, 2, 6, 2, 2,
    1, 0, 0, 0, 0, 0, 0, 4, 2, 10, 11, 12, 7, 7, 4, 0, 4, 1,
    2, 0, 0, 0, 0, 0, 0, 4, 6, 9, 10, 12, 6, 5, 4, 4, 3, 2,
    2, 0, 0, 0, 0, 14, 0, 7, 2, 9, 8, 8, 3, 6, 4, 4, 2, 2,
    3, 0, 0, 0, 10, 11, 0, 0, 1, 7, 4, 2, 2, 0, 0, 0, 2, 2,
    0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 4, 1, 1, 0, 2, 2, 0, 0, 2, 0, 3, 0, 1, 2,
    0, 0, 0, 0, 5, 5, 7, 7, 8, 9, 5, 8, 1, 1, 0, 0, 0, 3,
    0, 0, 8, 0, 10, 10, 12, 15, 16, 10, 7, 6, 0, 3, 0, 0, 0, 3,
    0, 0, 20, 4, 12, 12, 11, 19, 17, 17, 11, 9, 12, 12, 11, 13, 3, 1,
    0, 0, 0, 11, 8, 10, 11, 15, 18, 12, 5, 3, 6, 8, 11, 12, 9, 0,
    1, 0, 0, 6, 10, 1, 3, 17, 17, 13, 5, 1, 4, 16, 8, 15, 3, 1,
    2, 0, 0, 8, 0, 0, 0, 0, 0, 11, 6, 0, 6, 0, 0, 0, 0, 3,
    2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
    2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2
  };
  RadiotherapyData case2_beam3(18, 18, case2_beam3_matrix);

  static const int case2_beam4_matrix[] = {
    3, 0, 5, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 2, 2,
    0, 0, 5, 2, 2, 0, 7, 3, 3, 0, 0, 0, 0, 0, 0, 3, 1, 2,
    0, 0, 0, 4, 3, 0, 8, 11, 9, 4, 0, 2, 0, 0, 0, 0, 4, 1,
    0, 0, 9, 5, 5, 2, 12, 13, 10, 7, 3, 1, 4, 0, 0, 0, 0, 3,
    0, 16, 9, 4, 10, 7, 15, 16, 8, 5, 6, 4, 7, 10, 0, 11, 0, 2,
    0, 0, 12, 6, 12, 12, 18, 18, 14, 9, 7, 7, 8, 12, 13, 12, 10, 0,
    0, 0, 0, 8, 13, 15, 18, 20, 12, 13, 12, 12, 12, 13, 11, 10, 8, 0,
    0, 0, 0, 3, 5, 14, 17, 16, 11, 8, 4, 10, 12, 11, 14, 9, 1, 3,
    0, 0, 0, 0, 0, 3, 14, 8, 5, 4, 5, 9, 4, 0, 0, 0, 0, 3,
    4, 3, 0, 0, 1, 0, 8, 3, 3, 0, 0, 0, 0, 0, 2, 0, 0, 3,
    1, 7, 0, 0, 1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 5, 1, 4, 1, 4, 0, 0, 2,
    2, 5, 4, 0, 0, 0, 0, 0, 0, 0, 8, 10, 7, 0, 6, 1, 4, 1,
    2, 4, 4, 0, 0, 0, 0, 0, 0, 4, 5, 5, 6, 1, 6, 6, 2, 2,
    2, 4, 3, 2, 0, 0, 0, 0, 4, 3, 12, 2, 1, 7, 3, 4, 2, 2,
    2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 7, 3, 12, 5, 5, 1,
    2, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 2, 2,
    3, 3, 5, 0, 3, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 3
  };
  RadiotherapyData case2_beam4(18, 18, case2_beam4_matrix);

  static const int case2_beam5_matrix[] = {
    0, 0, 0, 15, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    0, 0, 2, 10, 16, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2,
    0, 0, 6, 9, 15, 3, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 3,
    2, 4, 9, 12, 15, 3, 4, 0, 3, 0, 2, 17, 13, 0, 0, 0, 2, 3,
    0, 5, 12, 14, 17, 5, 2, 0, 0, 8, 17, 16, 13, 4, 0, 0, 0, 3,
    0, 6, 13, 16, 17, 5, 2, 2, 4, 5, 12, 10, 10, 13, 6, 0, 0, 3,
    0, 0, 20, 17, 18, 8, 4, 5, 6, 10, 14, 13, 11, 2, 1, 4, 0, 3,
    0, 0, 0, 14, 18, 11, 8, 9, 9, 10, 13, 12, 8, 8, 5, 6, 5, 0,
    0, 0, 0, 2, 11, 10, 6, 3, 1, 6, 10, 11, 5, 8, 9, 8, 9, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 3, 10, 4, 5, 3, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 6, 6, 2, 2, 2, 2, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 3, 3, 4, 3, 4, 1, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 0, 0, 0, 0, 4, 1, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 8, 3, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 2,
    1, 3, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    1, 2, 1, 4, 8, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2
  };
  RadiotherapyData case2_beam5(17, 18, case2_beam5_matrix);

  static const int case3_beam1_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 13, 8, 8, 1, 0, 0, 0,
    1, 2, 0, 0, 0, 0, 0, 0, 11, 9, 5, 5, 4, 5, 0, 2, 0,
    1, 0, 0, 0, 0, 0, 8, 13, 9, 6, 4, 4, 4, 8, 0, 2, 0,
    0, 2, 17, 10, 13, 14, 10, 8, 7, 6, 4, 4, 5, 8, 0, 2, 0,
    0, 12, 20, 9, 14, 15, 7, 2, 5, 5, 5, 3, 4, 9, 0, 1, 1,
    0, 17, 13, 10, 15, 16, 5, 1, 5, 7, 5, 6, 4, 8, 0, 2, 1,
    1, 0, 15, 9, 15, 20, 6, 1, 4, 7, 7, 6, 5, 9, 7, 1, 1,
    0, 0, 2, 7, 16, 9, 5, 0, 3, 7, 5, 5, 4, 7, 4, 5, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 4, 4, 2, 6, 5, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 6, 3, 3, 3, 6, 5, 4, 0,
    0, 0, 0, 5, 7, 5, 8, 0, 2, 7, 5, 4, 5, 7, 4, 5, 0,
    0, 4, 9, 8, 16, 19, 5, 1, 3, 7, 6, 5, 6, 9, 6, 1, 1,
    0, 13, 12, 8, 14, 14, 4, 2, 0, 8, 4, 5, 5, 8, 2, 0, 2,
    0, 20, 11, 7, 14, 15, 3, 0, 0, 6, 3, 3, 5, 9, 0, 1, 1,
    0, 6, 17, 4, 14, 14, 6, 1, 1, 5, 2, 3, 5, 7, 0, 1, 0,
    0, 0, 0, 11, 6, 13, 7, 2, 2, 5, 2, 4, 3, 6, 0, 2, 0,
    1, 0, 0, 0, 6, 0, 8, 2, 3, 5, 3, 7, 4, 7, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 6, 0, 4, 4, 7, 10, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 2, 8, 5, 0, 10, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
  };
  RadiotherapyData case3_beam1(22, 17, case3_beam1_matrix);

  static const int case3_beam2_matrix[] = {
    0, 0, 1, 1, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 8, 0, 1, 4, 5, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 3, 2, 2, 1, 1, 0, 0, 1, 4, 7, 11, 9, 0, 0, 0,
    2, 0, 0, 0, 3, 2, 2, 0, 2, 4, 1, 4, 7, 11, 10, 20, 1, 0, 0,
    3, 0, 2, 0, 2, 2, 2, 1, 7, 8, 5, 9, 13, 16, 13, 14, 12, 0, 0,
    2, 0, 1, 0, 3, 2, 4, 5, 15, 16, 12, 11, 15, 17, 15, 14, 9, 0, 3,
    2, 0, 11, 0, 6, 3, 0, 5, 17, 16, 10, 10, 13, 17, 13, 14, 7, 1, 3,
    2, 0, 5, 0, 8, 1, 0, 2, 16, 16, 9, 8, 10, 14, 12, 13, 12, 0, 3,
    0, 0, 0, 2, 8, 1, 0, 7, 15, 17, 7, 8, 10, 12, 9, 12, 5, 1, 0,
    0, 0, 0, 0, 5, 0, 2, 7, 15, 13, 5, 4, 9, 7, 4, 0, 0, 2, 0,
    0, 0, 0, 0, 4, 0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  RadiotherapyData case3_beam2(15, 19, case3_beam2_matrix);

  static const int case3_beam3_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 15, 8, 10, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 15, 10, 7, 10, 7, 18, 0, 0, 0, 0,
    0, 3, 5, 5, 3, 0, 7, 8, 12, 9, 12, 11, 20, 0, 0, 0, 0,
    0, 0, 0, 4, 5, 2, 6, 5, 12, 9, 12, 12, 14, 3, 0, 1, 0,
    0, 0, 0, 7, 2, 4, 7, 9, 11, 9, 10, 10, 7, 5, 0, 0, 0,
    0, 0, 1, 7, 1, 2, 7, 8, 10, 4, 5, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 3, 0, 3, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0,
    3, 2, 4, 0, 0, 0, 0, 0, 0, 0, 2, 4, 4, 7, 10, 8, 0,
    0, 0, 4, 4, 0, 0, 0, 9, 6, 7, 7, 10, 6, 13, 8, 10, 0,
    0, 6, 12, 12, 0, 0, 0, 15, 9, 11, 15, 16, 15, 17, 4, 17, 0,
    0, 5, 14, 12, 5, 0, 9, 18, 15, 18, 19, 18, 16, 17, 6, 14, 0,
    0, 14, 7, 13, 3, 2, 16, 17, 13, 17, 17, 16, 17, 12, 8, 12, 0,
    0, 4, 14, 8, 5, 1, 10, 12, 7, 19, 17, 18, 15, 13, 0, 0, 3,
    0, 0, 6, 10, 0, 0, 0, 4, 5, 16, 17, 16, 13, 15, 2, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 5, 17, 15, 16, 12, 15, 2, 2, 0,
    1, 0, 0, 0, 0, 0, 2, 2, 0, 7, 15, 9, 11, 13, 7, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 15, 0, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 1, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0
  };
  RadiotherapyData case3_beam3(20, 17, case3_beam3_matrix);

  static const int case3_beam4_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 12, 0, 3, 0,
    2, 0, 0, 0, 0, 0, 2, 0, 10, 9, 20, 0, 0, 15, 0, 3, 0,
    0, 0, 0, 14, 0, 0, 0, 3, 7, 11, 16, 16, 6, 16, 2, 2, 0,
    0, 0, 10, 9, 5, 0, 0, 16, 7, 10, 17, 16, 13, 11, 12, 0, 0,
    0, 9, 10, 10, 5, 2, 11, 9, 9, 12, 16, 18, 12, 13, 3, 0, 3,
    0, 5, 11, 10, 6, 4, 10, 15, 10, 13, 17, 18, 16, 5, 11, 0, 2,
    0, 1, 13, 11, 7, 2, 19, 12, 14, 12, 16, 18, 16, 12, 7, 11, 0,
    0, 0, 14, 6, 7, 0, 0, 11, 13, 13, 17, 16, 16, 11, 7, 11, 0,
    0, 0, 5, 0, 0, 0, 0, 7, 4, 8, 11, 12, 12, 10, 7, 9, 0,
    2, 0, 0, 0, 0, 0, 0, 1, 2, 2, 5, 5, 7, 7, 8, 1, 1,
    3, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 6, 5, 11, 0, 2,
    0, 6, 3, 2, 2, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 4, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 4, 6, 2, 9, 12, 6, 5, 5, 5, 0, 0, 0, 1,
    0, 0, 0, 4, 2, 4, 0, 7, 10, 8, 10, 10, 5, 0, 0, 1, 0,
    1, 0, 0, 3, 0, 0, 0, 0, 6, 5, 11, 9, 11, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 10, 13, 13, 0, 0, 0, 0
  };
  RadiotherapyData case3_beam4(19, 17, case3_beam4_matrix);

  static const int case3_beam5_matrix[] = {
    0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 1, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 15, 0, 0, 0, 0, 4, 5, 0, 0, 0, 3, 0,
    0, 0, 0, 0, 1, 0, 0, 13, 2, 0, 5, 9, 9, 9, 1, 7, 0, 3, 0,
    1, 0, 1, 2, 5, 0, 0, 3, 5, 0, 8, 10, 9, 12, 10, 17, 4, 2, 0,
    3, 0, 0, 0, 5, 1, 0, 8, 9, 2, 10, 13, 12, 14, 12, 14, 10, 1, 3,
    3, 0, 0, 0, 3, 2, 2, 11, 11, 8, 14, 15, 16, 17, 15, 15, 5, 2, 3,
    3, 0, 2, 2, 2, 1, 3, 9, 8, 7, 7, 15, 13, 19, 18, 13, 15, 1, 0,
    3, 0, 0, 2, 0, 2, 2, 6, 1, 3, 1, 7, 9, 12, 11, 19, 0, 0, 0,
    3, 0, 0, 4, 0, 2, 3, 2, 1, 1, 0, 3, 4, 7, 20, 0, 0, 0, 0,
    3, 0, 16, 0, 3, 0, 3, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 0, 16, 3, 4, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  RadiotherapyData case3_beam5(15, 19, case3_beam5_matrix);

  static const int case4_beam1_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 5, 8, 10, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 2, 9, 2, 0, 2, 1, 2,
    0, 0, 0, 0, 0, 0, 0, 10, 17, 12, 0, 7, 5, 0, 0, 1, 6, 7, 4, 4, 3, 1,
    2, 0, 0, 0, 20, 0, 0, 0, 0, 0, 2, 1, 3, 1, 1, 1, 6, 7, 6, 4, 0, 1,
    2, 1, 0, 8, 6, 0, 0, 0, 0, 0, 0, 2, 3, 2, 2, 1, 6, 7, 7, 7, 0, 0,
    2, 0, 11, 5, 2, 4, 6, 0, 0, 3, 4, 2, 6, 1, 2, 1, 8, 5, 8, 8, 2, 0,
    1, 0, 1, 1, 0, 0, 2, 4, 7, 2, 0, 1, 3, 1, 5, 0, 11, 4, 7, 9, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 6, 1, 10, 3, 7, 8, 3, 0,
    1, 0, 0, 0, 0, 0, 3, 5, 8, 0, 1, 1, 2, 1, 7, 1, 11, 3, 6, 8, 4, 0,
    1, 0, 7, 4, 2, 6, 6, 0, 6, 3, 2, 4, 7, 6, 10, 2, 11, 3, 6, 7, 4, 0,
    0, 8, 16, 13, 0, 0, 0, 0, 0, 0, 2, 3, 6, 6, 7, 3, 10, 3, 5, 7, 4, 0,
    2, 0, 0, 0, 16, 0, 0, 0, 0, 0, 3, 2, 4, 7, 9, 4, 9, 3, 5, 6, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 12, 6, 8, 5, 4, 5, 8, 6, 3, 8, 3, 5, 6, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 14, 15, 10, 0, 3, 9, 8, 4, 2, 7, 3, 4, 5, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 4, 2, 7, 2, 4, 5, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 4, 1, 8, 2, 3, 2, 2, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 12, 5, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 1, 0, 0, 0, 0, 0, 0, 0
  };
  RadiotherapyData case4_beam1(19, 22, case4_beam1_matrix);

  static const int case4_beam2_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 3, 2, 9, 17, 10, 11, 6, 0, 0, 5, 0,
    0, 0, 0, 0, 0, 0, 7, 6, 0, 0, 0, 0, 1, 2, 7, 14, 16, 14, 16, 7, 5, 5, 0, 4,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5, 10, 16, 20, 12, 17, 8, 13, 13, 0, 4,
    0, 3, 7, 2, 0, 5, 5, 0, 6, 2, 0, 0, 3, 12, 16, 17, 17, 10, 19, 9, 11, 13, 0, 4,
    3, 0, 19, 9, 11, 10, 3, 2, 0, 7, 7, 13, 20, 14, 17, 15, 18, 7, 18, 11, 11, 15, 0, 4,
    0, 3, 4, 9, 10, 9, 0, 2, 0, 2, 0, 13, 13, 13, 14, 14, 17, 4, 17, 11, 11, 16, 0, 4,
    0, 1, 0, 0, 0, 6, 0, 0, 1, 1, 0, 5, 13, 9, 11, 9, 12, 5, 14, 11, 10, 18, 0, 4,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 10, 9, 8, 7, 12, 2, 13, 10, 11, 17, 0, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 8, 2, 0, 4, 13, 8, 12, 19, 0, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 3, 0, 0, 0, 0, 5, 8, 15, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 5, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 3, 0, 0
  };
  RadiotherapyData case4_beam2(13, 24, case4_beam2_matrix);

  static const int case4_beam3_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 5, 4, 3, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 4, 4, 6, 3, 1, 0, 0, 5, 5, 0, 0, 1, 0,
    1, 0, 0, 0, 11, 0, 9, 8, 5, 6, 3, 5, 6, 2, 0, 0, 1, 3, 4, 7, 0, 0, 0,
    2, 2, 0, 0, 7, 11, 0, 6, 8, 5, 6, 2, 3, 0, 0, 0, 2, 1, 5, 3, 3, 0, 0,
    1, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 3, 1, 3, 2, 2, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 5, 7, 2, 0, 3, 1, 2, 0, 0,
    1, 0, 6, 2, 4, 7, 3, 0, 0, 0, 3, 4, 6, 8, 6, 6, 4, 0, 2, 1, 3, 0, 2,
    0, 7, 6, 7, 7, 13, 14, 9, 10, 6, 6, 8, 8, 9, 8, 6, 5, 0, 2, 0, 2, 0, 2,
    0, 7, 8, 8, 8, 12, 12, 14, 10, 8, 7, 8, 7, 7, 7, 5, 6, 0, 1, 0, 2, 0, 2,
    0, 0, 0, 1, 7, 20, 13, 8, 17, 11, 6, 6, 5, 6, 9, 7, 7, 0, 1, 0, 3, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 6, 7, 8, 8, 0, 1, 1, 4, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 8, 0, 3, 2, 4, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 4, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0
  };
  RadiotherapyData case4_beam3(18, 23, case4_beam3_matrix);

  static const int case4_beam4_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 7, 1, 2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 6, 0, 5, 0, 3, 1, 3, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 7, 8, 10, 0, 2, 1, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 8, 9, 8, 1, 2, 1, 4, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 6, 4, 9, 9, 7, 0, 2, 2, 3, 0, 0,
    0, 0, 0, 0, 3, 17, 8, 9, 15, 14, 6, 10, 5, 5, 7, 5, 5, 1, 2, 3, 3, 0, 2,
    0, 0, 2, 7, 12, 20, 19, 16, 12, 12, 12, 9, 8, 8, 5, 2, 5, 0, 2, 3, 4, 0, 3,
    0, 12, 10, 13, 9, 15, 15, 11, 11, 14, 8, 10, 10, 10, 5, 4, 3, 0, 2, 4, 4, 0, 0,
    0, 2, 13, 4, 6, 12, 10, 6, 4, 7, 5, 6, 8, 8, 5, 4, 2, 0, 3, 3, 4, 0, 0,
    1, 0, 4, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 3, 1, 0, 3, 3, 2, 0, 0,
    2, 1, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 0, 4, 4, 0, 0, 0,
    2, 3, 0, 0, 13, 0, 2, 9, 0, 8, 7, 8, 2, 0, 0, 0, 4, 2, 6, 4, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 4, 7, 1, 3, 7, 9, 7, 4, 0, 0, 1, 7, 5, 5, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 9, 7, 8, 7, 1, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 11, 2, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 9, 6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 7, 0, 0, 0, 0, 0, 0, 0
  };
  RadiotherapyData case4_beam4(17, 23, case4_beam4_matrix);

  static const int case4_beam5_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 3, 0, 0, 0, 1, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 8, 6, 0, 0, 0, 6, 2, 10, 0, 3, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 7, 9, 5, 6, 0, 12, 10, 7, 15, 0, 3, 0,
    4, 0, 0, 0, 12, 0, 0, 0, 0, 9, 7, 10, 4, 4, 0, 8, 0, 15, 12, 10, 11, 0, 3, 0,
    2, 0, 5, 12, 8, 0, 0, 9, 6, 14, 14, 8, 10, 8, 5, 5, 3, 18, 13, 12, 15, 0, 4, 0,
    0, 19, 19, 15, 19, 1, 0, 17, 10, 14, 15, 13, 12, 9, 5, 8, 5, 20, 13, 13, 13, 0, 4, 1,
    3, 3, 14, 0, 10, 0, 15, 8, 5, 9, 2, 5, 10, 11, 5, 9, 7, 20, 15, 11, 11, 0, 4, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 8, 14, 9, 18, 11, 10, 11, 0, 4, 0,
    0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 1, 0, 3, 4, 11, 12, 12, 13, 8, 11, 0, 4, 0,
    0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 4, 0, 2, 0, 2, 10, 9, 13, 6, 0, 0, 2, 3, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 6, 4, 0, 0, 0, 0, 4, 0, 0
  };
  RadiotherapyData case4_beam5(12, 24, case4_beam5_matrix);

  static const int case5_beam1_matrix[] = {
    1, 2, 1, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
    1, 2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 11, 0, 0, 0, 1,
    1, 2, 0, 0, 0, 0, 0, 0, 1, 9, 8, 1, 8, 4, 5, 0,
    1, 2, 0, 0, 5, 0, 4, 4, 1, 7, 4, 5, 5, 5, 4, 0,
    0, 1, 0, 8, 2, 2, 1, 1, 0, 0, 0, 0, 2, 5, 1, 1,
    0, 0, 2, 2, 4, 4, 4, 2, 0, 0, 0, 0, 0, 6, 3, 1,
    0, 1, 3, 2, 3, 5, 4, 1, 2, 2, 4, 2, 2, 6, 4, 1,
    0, 0, 0, 0, 1, 0, 0, 1, 0, 2, 4, 2, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 0, 0, 0, 0, 1,
    0, 0, 1, 3, 3, 0, 2, 2, 1, 3, 6, 0, 0, 0, 0, 1,
    0, 3, 2, 4, 7, 5, 2, 4, 4, 8, 0, 0, 2, 10, 3, 1,
    0, 3, 3, 7, 9, 7, 4, 3, 0, 0, 0, 0, 0, 6, 4, 0,
    2, 0, 1, 7, 0, 0, 0, 4, 0, 0, 0, 5, 0, 8, 3, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 6, 0, 2, 1, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2
  };
  RadiotherapyData case5_beam1(15, 16, case5_beam1_matrix);

  static const int case5_beam2_matrix[] = {
    2, 2, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 1, 2,
    2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 3, 4, 0, 0, 0, 0, 5, 5, 5, 0, 0, 5, 0, 1, 0, 1,
    2, 2, 4, 0, 0, 0, 0, 0, 2, 2, 3, 0, 3, 0, 1, 5, 0,
    2, 2, 2, 0, 0, 0, 0, 0, 0, 8, 4, 0, 2, 2, 3, 8, 0,
    3, 1, 1, 0, 0, 0, 3, 1, 2, 13, 14, 13, 4, 10, 2, 16, 0,
    3, 2, 0, 0, 0, 0, 0, 0, 9, 19, 16, 6, 8, 18, 2, 9, 0,
    3, 0, 0, 8, 8, 1, 6, 7, 6, 20, 8, 0, 0, 0, 0, 1, 2,
    4, 2, 2, 17, 2, 0, 0, 0, 3, 13, 0, 1, 0, 1, 4, 0, 2,
    2, 6, 0, 8, 0, 0, 3, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 5, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2
  };
  RadiotherapyData case5_beam2(13, 17, case5_beam2_matrix);

  static const int case5_beam3_matrix[] = {
    1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 5, 4, 1,
    1, 2, 0, 0, 0, 0, 0, 0, 0, 10, 11, 5, 10, 3, 4, 1,
    1, 2, 1, 2, 0, 0, 0, 3, 0, 0, 11, 5, 4, 0, 2, 0,
    2, 1, 0, 9, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 3, 3, 0, 0, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    3, 0, 0, 4, 8, 6, 2, 7, 6, 9, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 12, 13, 11, 9, 12, 10, 7, 9, 5, 3, 10, 4, 0,
    0, 0, 10, 14, 13, 10, 12, 15, 9, 11, 12, 8, 7, 8, 9, 0,
    2, 0, 7, 13, 12, 12, 11, 14, 10, 10, 10, 1, 6, 7, 8, 0,
    0, 1, 0, 9, 19, 11, 18, 14, 8, 0, 0, 0, 0, 7, 0, 0,
    0, 0, 0, 0, 8, 20, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 2, 2, 1, 2
  };
  RadiotherapyData case5_beam3(14, 16, case5_beam3_matrix);

  static const int case5_beam4_matrix[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 11, 5, 3, 3, 12, 10, 20, 1, 0, 4, 6, 2, 6, 0,
    1, 0, 9, 7, 7, 10, 11, 8, 8, 18, 12, 8, 6, 4, 8, 0,
    0, 0, 9, 10, 9, 10, 12, 7, 9, 7, 6, 9, 5, 5, 6, 0,
    0, 0, 0, 6, 11, 7, 8, 7, 4, 10, 6, 9, 1, 0, 5, 1,
    3, 1, 0, 0, 5, 1, 3, 2, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 2, 0, 0, 2, 0, 2, 0, 0, 3, 0, 1, 3, 0, 0, 1,
    1, 2, 0, 0, 0, 0, 0, 0, 11, 1, 6, 6, 4, 0, 3, 0,
    1, 2, 0, 0, 0, 0, 0, 2, 9, 6, 3, 8, 6, 0, 6, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 6, 2, 0, 4, 1, 1, 3, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 3, 0, 0, 6, 0, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 10, 1, 1, 0, 1
  };
  RadiotherapyData case5_beam4(14, 16, case5_beam4_matrix);

  static const int case5_beam5_matrix[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 7, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 9, 12, 3, 0, 1, 0, 0, 10, 0, 0, 0, 0, 0, 0, 1,
    3, 0, 10, 11, 11, 1, 9, 3, 5, 0, 6, 3, 14, 12, 0, 0, 3,
    2, 0, 5, 7, 12, 5, 9, 10, 4, 0, 0, 5, 20, 2, 5, 0, 0,
    1, 4, 0, 2, 4, 7, 3, 5, 9, 0, 0, 15, 15, 17, 4, 1, 0,
    2, 4, 0, 0, 0, 0, 0, 0, 6, 0, 5, 12, 9, 14, 6, 8, 0,
    2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 6, 3, 0,
    2, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0,
    2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
    2, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
  };
  RadiotherapyData case5_beam5(12, 17, case5_beam5_matrix);
  //@}

  /// Radiotherapy instances
  RadiotherapyData rds[] = {rd0, rd1, 
                                   case1_beam1,
                                   case1_beam2,
                                   case1_beam3,
                                   case1_beam4,
                                   case1_beam5,
                                   case2_beam1,
                                   case2_beam2,
                                   case2_beam3,
                                   case2_beam4,
                                   case2_beam5,
                                   case3_beam1,
                                   case3_beam2,
                                   case3_beam3,
                                   case3_beam4,
                                   case3_beam5,
                                   case4_beam1,
                                   case4_beam2,
                                   case4_beam3,
                                   case4_beam4,
                                   case4_beam5,
                                   case5_beam1,
                                   case5_beam2,
                                   case5_beam3,
                                   case5_beam4,
                                   case5_beam5
  };
  /// Number of Radiotherapy instances
  const unsigned int rds_n = sizeof(rds) / sizeof(RadiotherapyData);
}
// STATISTICS: example-any
