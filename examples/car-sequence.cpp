/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
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

#include <iomanip>

using namespace Gecode;

// Problems
namespace {
  // Problem data
  extern const int* problems[];
  // Number of problems
  extern const unsigned int n_problems;
}

/**
 * \brief %Example: Car sequencing.
 *
 * See problem 1 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 *
 */
class CarSequencing : public Script {
protected:
  /// Problem number
  const int problem;
  /// Number of marks
  const int ncars, noptions, nclasses;
  const int fill; ///< Color of fills
  /// Number of fills
  IntVar nfill;
  /// Sequence of cars produced
  IntVarArray s;
public:
  /// Search variants
  enum {
    SEARCH_BAB,    ///< Use branch and bound to optimize
    SEARCH_RESTART ///< Use restart to optimize
  };

  /// Actual model
  CarSequencing(const SizeOptions& opt)
    : problem(opt.size()),
      ncars(problems[problem][0]), 
      noptions(problems[problem][1]), 
      nclasses(problems[problem][2]),
      fill(nclasses),
      nfill(*this, 0, ncars),
      s() {
    Gecode::wait(*this, nfill, post_model, ICL_DEF);
    branch(*this, nfill, INT_VAL_MIN);
  }

  static void post_model(Space& _home) {
    CarSequencing& home = static_cast<CarSequencing&>(_home);

    // Initialize variables
    home.s.resize(home, home.ncars + home.nfill.val());
    for (int i = home.s.size(); i--; )
      home.s[i] = IntVar(home, 0, home.nclasses);

    // Read problem
    const int* probit = problems[home.problem] + 3;

    // Sequence requirements for the options.
    IntArgs max(home.noptions), block(home.noptions);
    for (int i = 0; i < home.noptions; ++i ) {
      max[i] = *probit++;
    }
    for (int i = 0; i < home.noptions; ++i ) {
      block[i] = *probit++;
    }
    // Number of cars per class
    IntArgs ncc(home.nclasses);
    // What classes require an option
    IntSetArgs classes(home.noptions);
    int** cdata = new int*[home.noptions]; 
    for (int i = home.noptions; i--; ) cdata[i] = new int[home.nclasses];
    int* n = new int[home.noptions];
    for (int i = home.noptions; i--; ) n[i] = 0;
    // Read data
    for (int c = 0; c < home.nclasses; ++c) {
      *probit++;
      //int car = *probit++;
      //assert(car == c);
      ncc[c] = *probit++;
      for (int o = 0; o < home.noptions; ++o) {
        if (*probit++) {
          cdata[o][n[o]++] = c;
        }
      }
    }
    for (int o = home.noptions; o--; ) {
      classes[o] = IntSet(cdata[o], n[o]);
      //std::cerr << "Option " << o << " used by " << classes[o] << std::endl;
      delete [] cdata[o];
    }
    delete [] cdata;
    delete [] n;

    // Count of cars
    IntSetArgs c(home.nclasses+1);
    for (int i = home.nclasses; i--; ) {
      c[i] = IntSet(ncc[i], ncc[i]);
    }
    c[home.fill] = IntSet(home.nfill.val(), home.nfill.val());
    count(home, home.s, c);

    // Make sure nothing is overloaded
    for (int o = home.noptions; o--; ) {
      //std::cerr << "Option " << o << ": classes=" << classes[o] 
      //          << " block=" << block[o] << " max=" << max[o] 
      //          << std::endl;
      sequence(home, home.s, classes[o], block[o], 0, max[o]);
    }

    branch(home, home.s, INT_VAR_NONE, INT_VAL_MIN);
  }

  /// Return cost
  virtual void constrain(const Space& _best) {
    const CarSequencing& best = static_cast<const CarSequencing&>(_best);
    rel(*this, nfill, IRT_LE, best.nfill.val());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    int width = nclasses > 9 ? 2 : 1;
    const char* space = nclasses > 9 ? " " : "";
    os << "Number of fills = " << nfill << std::endl;
    if (s.size() > 0) {
      for (int i = 0; i < s.size(); ++i) {
        if (s[i].assigned()) {
          int v = s[i].val();
          if (v == fill) os << space << "_ ";
          else           os << std::setw(width) << v << " ";
        } else {
          os << space << "? ";    
        }
        if ((i+1)%20 == 0) os << std::endl;
      }
      if ((s.size())%20)
        os << std::endl;
    }
  }

  /// Constructor for cloning \a s
  CarSequencing(bool share, CarSequencing& cs)
    : Script(share,cs), 
      problem(cs.problem),
      ncars(cs.ncars),
      noptions(cs.noptions),
      nclasses(cs.nclasses),
      fill(cs.fill)
  {
    nfill.update(*this, share, cs.nfill);
    s.update(*this, share, cs.s);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new CarSequencing(share,*this);
  }
};

/** \brief Main-function
 *  \relates CarSequencing
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("CarSequencing");
  opt.solutions(0);
  opt.size(0);
  opt.search(CarSequencing::SEARCH_BAB);
  opt.search(CarSequencing::SEARCH_BAB, "bab");
  opt.search(CarSequencing::SEARCH_RESTART, "restart");
  opt.parse(argc,argv);
  if (opt.size() >= n_problems) {
    std::cerr << "Error: size must be between 0 and "
              << n_problems-1 << std::endl;
    return 1;
  }

  switch (opt.search()) {
  case CarSequencing::SEARCH_BAB:
    Script::run<CarSequencing,BAB,SizeOptions>(opt); break;
  case CarSequencing::SEARCH_RESTART:
    Script::run<CarSequencing,Restart,SizeOptions>(opt); break;
  }
  return 0;
}


namespace {
  /// Problems from CSPLib

  /// Simple initial example
  const int p0[] = {
    10, 5, 6,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 1, 1, 0, 1, 1, 0, 
    1, 1, 0, 0, 0, 1, 0, 
    2, 2, 0, 1, 0, 0, 1, 
    3, 2, 0, 1, 0, 1, 0, 
    4, 2, 1, 0, 1, 0, 0, 
    5, 2, 1, 1, 0, 0, 0
  };

  // ---------------------------------
  //  Problem 4/72  (Regin & Puget   // 1)
  // ---------------------------------
  const int p1[] = {
    100, 5, 22,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 6, 1, 0, 0, 1, 0, 
    1, 10, 1, 1, 1, 0, 0, 
    2, 2, 1, 1, 0, 0, 1, 
    3, 2, 0, 1, 1, 0, 0, 
    4, 8, 0, 0, 0, 1, 0, 
    5, 15, 0, 1, 0, 0, 0, 
    6, 1, 0, 1, 1, 1, 0, 
    7, 5, 0, 0, 1, 1, 0, 
    8, 2, 1, 0, 1, 1, 0, 
    9, 3, 0, 0, 1, 0, 0, 
    10, 2, 1, 0, 1, 0, 0, 
    11, 1, 1, 1, 1, 0, 1, 
    12, 8, 0, 1, 0, 1, 0, 
    13, 3, 1, 0, 0, 1, 1, 
    14, 10, 1, 0, 0, 0, 0, 
    15, 4, 0, 1, 0, 0, 1, 
    16, 4, 0, 0, 0, 0, 1, 
    17, 2, 1, 0, 0, 0, 1, 
    18, 4, 1, 1, 0, 0, 0, 
    19, 6, 1, 1, 0, 1, 0, 
    20, 1, 1, 0, 1, 0, 1, 
    21, 1, 1, 1, 1, 1, 1, 
  };

  // --------------------------------
  //  Problem 6/76, (Regin & Puget   // 2)
  // --------------------------------
  const int p2[] = {
    100, 5, 22,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 13, 1, 0, 0, 0, 0, 
    1, 8, 0, 0, 0, 1, 0, 
    2, 7, 0, 1, 0, 0, 0, 
    3, 1, 1, 0, 0, 1, 0, 
    4, 12, 0, 0, 1, 0, 0, 
    5, 5, 0, 1, 0, 1, 0, 
    6, 5, 0, 0, 1, 1, 0, 
    7, 6, 0, 1, 1, 0, 0, 
    8, 3, 1, 0, 0, 0, 1, 
    9, 12, 1, 1, 0, 0, 0, 
    10, 8, 1, 1, 0, 1, 0, 
    11, 2, 1, 0, 0, 1, 1, 
    12, 2, 1, 1, 1, 0, 0, 
    13, 1, 0, 1, 0, 1, 1, 
    14, 4, 1, 0, 1, 0, 0, 
    15, 4, 0, 1, 0, 0, 1, 
    16, 1, 1, 1, 0, 1, 1, 
    17, 2, 1, 0, 1, 1, 0, 
    18, 1, 0, 0, 0, 0, 1, 
    19, 1, 1, 1, 1, 1, 0, 
    20, 1, 1, 1, 0, 0, 1, 
    21, 1, 0, 1, 1, 1, 0, 
  };

  // ---------------------------------
  //  Problem 10/93, (Regin & Puget   // 3)
  // ---------------------------------
  const int p3[] = {
    100, 5, 25,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 7, 1, 0, 0, 1, 0,
    1, 11, 1, 1, 0, 0, 0,
    2, 1, 0, 1, 1, 1, 1,
    3, 3, 1, 0, 1, 0, 0,
    4, 15, 0, 1, 0, 0, 0,
    5, 2, 1, 0, 1, 1, 0,
    6, 8, 0, 1, 0, 1, 0,
    7, 5, 0, 0, 1, 0, 0,
    8, 3, 0, 0, 0, 1, 0,
    9, 4, 0, 1, 1, 1, 0,
    10, 5, 1, 0, 0, 0, 0,
    11, 2, 1, 1, 1, 0, 1,
    12, 6, 0, 1, 1, 0, 0,
    13, 2, 0, 0, 1, 0, 1,
    14, 2, 0, 1, 0, 0, 1,
    15, 4, 1, 1, 1, 1, 0,
    16, 3, 1, 0, 0, 0, 1,
    17, 5, 1, 1, 0, 1, 0,
    18, 2, 1, 1, 1, 0, 0,
    19, 4, 1, 1, 0, 0, 1,
    20, 1, 1, 0, 0, 1, 1,
    21, 1, 1, 1, 0, 1, 1,
    22, 1, 0, 1, 0, 1, 1,
    23, 1, 0, 1, 1, 0, 1,
    24, 2, 0, 0, 0, 0, 1,
  };

  // --------------
  //  Problem 16/81,
  // --------------
  const int p4[] = {
    100, 5, 26,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 10, 1, 0, 0, 0, 0,
    1, 2, 0, 0, 0, 0, 1,
    2, 8, 0, 1, 0, 1, 0,
    3, 8, 0, 0, 0, 1, 0,
    4, 6, 0, 1, 1, 0, 0,
    5, 11, 0, 1, 0, 0, 0,
    6, 3, 0, 0, 1, 0, 0,
    7, 2, 0, 0, 1, 1, 0,
    8, 7, 1, 1, 0, 0, 0,
    9, 2, 1, 0, 0, 1, 1,
    10, 4, 1, 0, 1, 0, 0,
    11, 7, 1, 0, 0, 1, 0,
    12, 1, 1, 1, 1, 0, 1,
    13, 3, 0, 1, 1, 1, 0,
    14, 4, 0, 1, 0, 0, 1,
    15, 5, 1, 1, 1, 0, 0,
    16, 2, 1, 1, 0, 0, 1,
    17, 1, 1, 0, 1, 1, 1,
    18, 2, 1, 0, 1, 1, 0,
    19, 3, 1, 0, 0, 0, 1,
    20, 2, 0, 1, 1, 0, 1,
    21, 1, 0, 1, 0, 1, 1,
    22, 3, 1, 1, 0, 1, 0,
    23, 1, 0, 0, 1, 1, 1,
    24, 1, 1, 1, 1, 1, 1,
    25, 1, 1, 1, 1, 1, 0,
  };

  // ----------------------------------
  //  Problem 19/71,  (Regin & Puget   // 4)
  // ----------------------------------
  const int p5[] = {
    100, 5, 23,
    1, 2, 1, 2, 1,
    2, 3, 3, 5, 5,
    0, 2, 0, 0, 0, 1, 1,
    1, 2, 0, 0, 1, 0, 1,
    2, 5, 0, 1, 1, 1, 0,
    3, 4, 0, 0, 0, 1, 0,
    4, 4, 0, 1, 0, 1, 0,
    5, 1, 1, 1, 0, 0, 1,
    6, 3, 1, 1, 1, 0, 1,
    7, 4, 0, 0, 1, 0, 0,
    8, 19, 0, 1, 0, 0, 0,
    9, 7, 1, 1, 0, 1, 0,
    10, 10, 1, 0, 0, 0, 0,
    11, 1, 0, 0, 1, 1, 0,
    12, 5, 1, 1, 1, 1, 0,
    13, 2, 1, 0, 1, 1, 0,
    14, 6, 1, 1, 0, 0, 0,
    15, 4, 1, 1, 1, 0, 0,
    16, 8, 1, 0, 0, 1, 0,
    17, 1, 1, 0, 0, 0, 1,
    18, 4, 0, 1, 1, 0, 0,
    19, 2, 0, 0, 0, 0, 1,
    20, 4, 0, 1, 0, 0, 1,
    21, 1, 1, 1, 0, 1, 1,
    22, 1, 0, 1, 1, 0, 1,
  };

  const int* problems[] = {
    &p0[0],
    &p1[0],
    &p2[0],
    &p3[0],
    &p4[0],
    &p5[0],
  };

    /// The number of instances
  const unsigned int n_problems = sizeof(problems)/sizeof(int*);
};

// STATISTICS: example-any

