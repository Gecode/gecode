/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2014
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

  // Instance data
  extern const int* jsd[];
  // Instance names
  extern const char* name[];

  /// A wrapper class for job-shop instance data
  class Spec {
  protected:
    /// Raw instance data
    const int* data;
    /// Find instance by name \a s
    static const int* find(const char* s) {
      for (int i=0; name[i] != NULL; i++)
        if (!strcmp(s,name[i]))
          return jsd[i];
      return NULL;
    }
  public:
    /// Whether a valid specification has been found
    bool valid(void) const {
      return data != NULL;
    }
    /// Return number of jobs
    int jobs(void) const {
      return data[0];
    }
    /// Return number of machines
    int machines(void) const {
      return data[1];
    }
    /// Return machine of job \a j at step \a s
    int machine(int j, int s) const {
      return data[2+j*machines()+2*s+0];
    }
    /// Return duration of job \a j at step \a s
    int duration(int j, int s) const {
      return data[2+j*machines()+2*s+1];
    }
    /// Initialize
    Spec(const char* s) : data(find(s)) {}
  };

}


/**
 * \brief %Example: job-shop scheduling
 *
 * \ingroup Example
 *
 */
class JobShop : public IntMinimizeScript {
protected:
  /// The instance specification
  const Spec& spec;
  /// Precedences
  BoolVarArray b;
  /// Makespan
  IntVar makespan;
  /// Start times
  IntVarArray start;

public:
  /// The actual problem
  JobShop(const SizeOptions& opt)
    : spec(examples[opt.size()]),
      b(*this, (spec.n+spec.m-2)*spec.n*spec.m/2, 0,1),
      makespan(*this, 0, Int::Limits::max),
      _start(*this, spec.m*spec.n, 0, Int::Limits::max) {

    Matrix<IntVarArray> start(_start, spec.m, spec.n);
    IntArgs _dur(spec.m*spec.n, spec.p);
    Matrix<IntArgs> dur(_dur, spec.m, spec.n);

    int minmakespan;
    int maxmakespan;
    crosh(dur, minmakespan, maxmakespan);
    rel(*this, makespan <= maxmakespan);
    rel(*this, makespan >= minmakespan);

    int k=0;
    for (int m=0; m<spec.m; m++)
      for (int j0=0; j0<spec.n-1; j0++)
        for (int j1=j0+1; j1<spec.n; j1++) {
          // The tasks on machine m of jobs j0 and j1 must be disjoint
          rel(*this,
              b[k] == (start(m,j0) + dur(m,j0) <= start(m,j1)));
          rel(*this,
              b[k++] == (start(m,j1) + dur(m,j1) > start(m,j0)));
        }
    
    for (int j=0; j<spec.n; j++)
      for (int m0=0; m0<spec.m-1; m0++)
        for (int m1=m0+1; m1<spec.m; m1++) {
          // The tasks in job j on machine m0 and m1 must be disjoint
          rel(*this,
              b[k] == (start(m0,j) + dur(m0,j) <= start(m1,j)));
          rel(*this,
              b[k++] == (start(m1,j) + dur(m1,j) > start(m0,j)));
        }

    // The makespan is greater than the end time of the latest job
    for (int m=0; m<spec.m; m++) {
      for (int j=0; j<spec.n; j++) {
        rel(*this, start(m,j) + dur(m,j) <= makespan);
      }
    }

    // First branch over the precedences
    branch(*this, b, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_MAX());
    // When the precedences are fixed, simply assign the start times
    assign(*this, _start, INT_ASSIGN_MIN());
    // When the start times are fixed, use the tightest makespan
    assign(*this, makespan, INT_ASSIGN_MIN());
  }

  /// Constructor for cloning \a s
  OpenShop(bool share, OpenShop& s) : IntMinimizeScript(share,s), spec(s.spec) {
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

  /// Helper class for representing tasks when printing a solution
  class PrintTask {
  public:
    int start; //< Start time
    int job;   //< Job number
    int p;     //< Processing time
    /// Comparison of tasks based on start time, used for sorting
    bool operator()(const PrintTask& t1, const PrintTask& t2) {
      return t1.start < t2.start;
    }
  };

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Region re(*this);
    PrintTask* m = re.alloc<PrintTask>(spec.n);
    for (int i=0; i<spec.m; i++) {
      int k=0;
      for (int j=0; j<spec.n; j++) {
        m[k].start = _start[i*spec.n+j].val();
        m[k].job = j;
        m[k].p = spec.p[i*spec.n+j];
        k++;
      }
      Support::quicksort(m, spec.n, m[0]);
      os << "Machine " << i << ": ";
      for (int j=0; j<spec.n; j++)
        os << "\t" << m[j].job << "("<<m[j].p<<")";
      os << " = " << m[spec.n-1].start+m[spec.n-1].p << std::endl;
    }
    os << "Makespan: " << makespan << std::endl;
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
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  IntMinimizeScript::run<OpenShop,BAB,SizeOptions>(opt);
  return 0;
}

namespace {

  /*
   * These instances are taken from the OR-Library by John E. Beasly.
   *
   * These instances are contributed to the OR-Library by
   * Dirk C. Mattfeld (email dirk@uni-bremen.de) and 
   * Rob J.M. Vaessens (email robv@win.tue.nl).
   *
   * abz5-abz9 are from
   *   J. Adams, E. Balas and D. Zawack (1988),
   *   The shifting bottleneck procedure for job shop scheduling,
   *   Management Science 34, 391-401.
   *
   * ft06, ft10, and ft20 are from 
   *   H. Fisher, G.L. Thompson (1963), 
   *   Probabilistic learning combinations of local job-shop scheduling rules, 
   *   J.F. Muth, G.L. Thompson (eds.), Industrial Scheduling, 
   *   Prentice Hall, Englewood Cliffs, New Jersey, 225-251.
   *
   * la01-la40 are from 
   *   S. Lawrence (1984),
   *   Resource constrained project scheduling: an experimental
   *   investigation of heuristic scheduling techniques (Supplement), 
   *   Graduate School of Industrial Administration,
   *   Carnegie-Mellon University, Pittsburgh, Pennsylvania.
   *
   * orb01-orb10 are from 
   *   D. Applegate, W. Cook (1991),
   *   A computational study of the job-shop scheduling instance,
   *   ORSA Journal on Computing 3, 149-156. 
   *   (they were generated in Bonn in 1986)
   *
   * swv01-swv20 are from 
   *   R.H. Storer, S.D. Wu, R. Vaccari (1992),
   *   New search spaces for sequencing instances with application to job shop 
   *   scheduling, Management Science 38, 1495-1509.
   *
   * yn1-yn4 are from 
   *   T. Yamada, R. Nakano (1992),
   *   A genetic algorithm applicable to large-scale job-shop instances,
   *   R. Manner, B. Manderick (eds.), Parallel instance solving from nature 2,
   *   North-Holland, Amsterdam, 281-290.
   *
   */

  /// The instances
  OpenShopSpec examples[] = { ex0, ex1, ex2, ex3, ex4 };
  /// The number of instances
  const unsigned int n_examples = sizeof(examples) / sizeof(OpenShopSpec);

  //@}
}

// STATISTICS: example-any
