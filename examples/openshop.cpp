/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2009
 *
 *  Last modified:
 *     $Date: 2009-09-29 12:50:21 +0200 (Di, 29 Sep 2009) $ by $Author: tack $
 *     $Revision: 9767 $
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

namespace {
  /**
   * \brief Specification of an open shop instance
   *
   */
  class OpenShopSpec {
  public:
    const int m;  //< number of machines
    const int n;  //< number of jobs
    const int* p; //< processing times of the tasks
    /// Constructor
    OpenShopSpec(int m0, int n0, const int* p0) : m(m0), n(n0), p(p0) {}
  };

  extern OpenShopSpec examples[];
  extern const unsigned int n_examples;
}

/**
 * \brief %Example: open-shop scheduling
 *
 * \ingroup ExProblem
 *
 */
class OpenShop : public MinimizeScript {
protected:
  /// The instance specification
  const OpenShopSpec& spec;
  /// Precedences
  BoolVarArray b;
  /// Makespan
  IntVar makespan;
  /// Start times
  IntVarArray _start;

public:
  /// Search variants
  enum {
    SEARCH_BAB,    ///< Use branch and bound to optimize
    SEARCH_RESTART ///< Use restart to optimize
  };
  /// The actual problem
  OpenShop(const SizeOptions& opt)
    : spec(examples[opt.size()]),
      b(*this, (spec.n+spec.m-2)*spec.n*spec.m/2, 0,1),
      makespan(*this, 0, Int::Limits::max),
      _start(*this, spec.m*spec.n, 0, Int::Limits::max) {

    Matrix<IntVarArray> start(_start, spec.m, spec.n);
    IntArgs _dur(spec.m*spec.n, spec.p);
    Matrix<IntArgs> dur(_dur, spec.m, spec.n);

    int k=0;
    for (int m=0; m<spec.m; m++)
      for (int j0=0; j0<spec.n-1; j0++)
        for (int j1=j0+1; j1<spec.n; j1++) {
          // The tasks on machine m of jobs j0 and j1 must be disjoint
          post(*this,
            tt(eqv(b[k], ~(start(m,j0) + dur(m,j0) <= start(m,j1)))));
          post(*this,
            tt(eqv(b[k++], ~(start(m,j1) + dur(m,j1) > start(m,j0)))));
        }
    
    for (int j=0; j<spec.n; j++)
      for (int m0=0; m0<spec.m-1; m0++)
        for (int m1=m0+1; m1<spec.m; m1++) {
          // The tasks in job j on machine m0 and m1 must be disjoint
          post(*this,
            tt(eqv(b[k], ~(start(m0,j) + dur(m0,j) <= start(m1,j)))));
          post(*this,
            tt(eqv(b[k++], ~(start(m1,j) + dur(m1,j) > start(m0,j)))));
        }

    // The makespan is greater than the end time of the latest job
    for (int m=0; m<spec.m; m++) {
      for (int j=0; j<spec.n; j++) {
        post(*this, start(m,j) + dur(m,j) <= makespan);
      }
    }

    // First branch over the precedences
    branch(*this, b, INT_VAR_NONE, INT_VAL_MIN);
    // When the precedences are fixed, simply assign the start times
    assign(*this, _start, INT_ASSIGN_MIN);
    // When the start times are fixed, use the tightest makespan
    assign(*this, makespan, INT_ASSIGN_MIN);
  }

  /// Constructor for cloning \a s
  OpenShop(bool share, OpenShop& s) : MinimizeScript(share,s), spec(s.spec) {
    b.update(*this, share, s.b);
    makespan.update(*this, share, s.makespan);
    _start.update(*this, share, s._start);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new OpenShop(share,*this);
  }

  /// Minimize the makespan
  virtual IntVar
  cost(void) const {
    return makespan;
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << b << std::endl;
    os << _start << std::endl;
    os << "Makespan: " << makespan << std::endl;
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates OpenShop
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("OpenShop");
  opt.iterations(500);
  opt.size(0);
  opt.solutions(0);
  opt.search(OpenShop::SEARCH_BAB);
  opt.search(OpenShop::SEARCH_BAB, "bab");
  opt.search(OpenShop::SEARCH_RESTART, "restart");
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  switch (opt.search()) {
  case OpenShop::SEARCH_BAB:
    MinimizeScript::run<OpenShop,BAB,SizeOptions>(opt); break;
  case OpenShop::SEARCH_RESTART:
    MinimizeScript::run<OpenShop,Restart,SizeOptions>(opt); break;
  }
  return 0;
}

namespace {

  /** \name Open shop specifications
   *
   * Each specification gives the processing times of the tasks for each
   * job, as well as the number of jobs and machines.
   *
   * \relates OpenShop
   */
  //@{

  const int ex0_p[] = {
    661,6,333,
    168,489,343,
    171,505,324
  };
  OpenShopSpec ex0(3,3,ex0_p);

  const int ex1_p[] = {
   54, 34, 61,  2,
    9, 15, 89, 70,
   38, 19, 28, 87,
   95, 34,  7, 29
  };
  OpenShopSpec ex1(4,4,ex1_p);

  const int ex2_p[] = {
   5, 70, 45, 83,
   24, 80, 58, 45,
   29, 56, 29, 61,
   43, 64, 45, 74
  };
  OpenShopSpec ex2(4,4,ex2_p);

  const int ex3_p[] = {
   89, 39, 54, 34, 71, 92, 56,
   19, 13, 81, 46, 91, 73, 27,
   66, 95, 48, 24, 96, 18, 14,
   48, 46, 78, 94, 19, 68, 63,
   60, 28, 91, 75, 52,  9,  7,
   33, 98, 37, 11,  2, 30, 38,
   83, 45, 37, 77, 52, 88, 52
  };
  OpenShopSpec ex3(7,7,ex3_p);

  /// The instances
  OpenShopSpec examples[] = { ex0, ex1, ex2, ex3 };
  /// The number of instances
  const unsigned int n_examples = sizeof(examples) / sizeof(OpenShopSpec);

  //@}
}

// STATISTICS: example-any
