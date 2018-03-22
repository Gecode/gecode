/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2009
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
 * \ingroup Example
 *
 */
class OpenShop : public IntMinimizeScript {
protected:
  /// The instance specification
  const OpenShopSpec& spec;
  /// Precedences
  BoolVarArray b;
  /// Makespan
  IntVar makespan;
  /// Start times
  IntVarArray _start;

  /// %Task representation for CROSH heuristic
  class Task {
  public:
    int j; //< Job
    int m; //< Machine
    int p; //< Processing time
    /// Default constructor
    Task(void) {}
    /// Constructor
    Task(int j0, int m0, int p0) : j(j0), m(m0), p(p0) {}
  };

  /** \brief Use Constructive Randomized Open-Shop Heuristics
   *         to compute lower and upper bounds
   *
   * This heuristic is taken from the paper
   * A. Malapert, H. Cambazard, C. Gueret, N. Jussien, A. Langevin,
   * L.-M. Rousseau: An Optimal Constraint Programming Approach to the
   * Open-Shop Problem. Technical report, CIRRELT-2009-25.
   *
   */
  void crosh(Matrix<IntArgs>& dur, int& minmakespan, int& maxmakespan) {
    Support::RandomGenerator rnd;

    // Compute maximum makespan as the sum of all production times.
    maxmakespan = 0;
    for (int i=0; i<spec.m; i++)
      for (int j=0; j<spec.n; j++)
        maxmakespan += dur(i,j);

    // Compute minimum makespan as the maximum of individual jobs and machines
    minmakespan = 0;
    for (int i=0; i<spec.m; i++) {
      int ms = 0;
      for (int j=0; j<spec.n; j++) {
        ms += dur(i,j);
      }
      minmakespan = std::max(minmakespan, ms);
    }
    for (int j=0; j<spec.n; j++) {
      int ms = 0;
      for (int i=0; i<spec.m; i++) {
        ms += dur(i,j);
      }
      minmakespan = std::max(minmakespan, ms);
    }

    Region re;
    int* ct_j = re.alloc<int>(spec.n); // Job completion time
    int* ct_m = re.alloc<int>(spec.m); // Machine completion time
    Task* tasks = re.alloc<Task>(spec.n*spec.m); // Tasks
    int k=0;
    for (int i=spec.m; i--;)
      for (int j=spec.n; j--;)
        new (&tasks[k++]) Task(j,i,dur(i,j));
    int* t0_tasks = re.alloc<int>(spec.n*spec.m); // Earliest possible tasks

    bool stopCROSH = false;

    int maxIterations;
    switch (spec.n) {
    case 3: maxIterations = 5; break;
    case 4: maxIterations = 25; break;
    case 5: maxIterations = 50; break;
    case 6: maxIterations = 1000; break;
    case 7: maxIterations = 10000; break;
    case 8: maxIterations = 10000; break;
    case 9: maxIterations = 10000; break;
    default: maxIterations = 25000; break;
    }
    int iteration = 0;
    while (!stopCROSH && maxmakespan > minmakespan) {
      for (int i=spec.n; i--;) ct_j[i] = 0;
      for (int i=spec.m; i--;) ct_m[i] = 0;
      int cmax = 0; // Current makespan
      int u = spec.n*spec.m; // Consider all tasks
      while (u > 0) {
        int u_t0 = 0; // Set of selectable tasks
        int t0 = maxmakespan; // Minimal head of unscheduled tasks
        for (int i=0; i<u; i++) {
          const Task& t = tasks[i];
          int est = std::max(ct_j[t.j], ct_m[t.m]); // Head of T_jm
          if (est < t0) {
            t0 = est;
            t0_tasks[0] = i;
            u_t0 = 1;
          } else if (est == t0) {
            t0_tasks[u_t0++] = i;
          }
        }
        int t_j0m0;
        if (iteration == 0) {
          // In the first iteration, select tasks with longest processing time
          t_j0m0 = 0;
          for (int i=1; i<u_t0; i++)
            if (tasks[t0_tasks[i]].p > tasks[t0_tasks[t_j0m0]].p)
              t_j0m0 = i;
        } else {
          t_j0m0 = rnd(u_t0); // Select random task
        }
        const Task& t = tasks[t0_tasks[t_j0m0]];
        int ect = t0 + t.p;
        ct_j[t.j] = ect;
        ct_m[t.m] = ect;
        std::swap(tasks[--u],tasks[t0_tasks[t_j0m0]]); // Remove task from u
        cmax = std::max(cmax, ect);
        if (cmax > maxmakespan)
          break;
      }

      maxmakespan = std::min(maxmakespan,cmax);
      if (iteration++ > maxIterations)
        stopCROSH = true; // Iterate a couple of times
    }
  }
public:
  /// The actual problem
  OpenShop(const SizeOptions& opt)
    : IntMinimizeScript(opt),
      spec(examples[opt.size()]),
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
    branch(*this, b, BOOL_VAR_AFC_MAX(opt.decay()), BOOL_VAL_MAX());
    // When the precedences are fixed, simply assign the start times
    assign(*this, _start, INT_ASSIGN_MIN());
    // When the start times are fixed, use the tightest makespan
    assign(*this, makespan, INT_ASSIGN_MIN());
  }

  /// Constructor for cloning \a s
  OpenShop(OpenShop& s) : IntMinimizeScript(s), spec(s.spec) {
    b.update(*this, s.b);
    makespan.update(*this, s.makespan);
    _start.update(*this, s._start);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new OpenShop(*this);
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
    Region re;
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

  const int ex4_p[] = {
   49, 58, 37, 79, 16, 64, 71, 65, 6, 44, 17, 85, 99, 57, 89, 4, 16, 8, 40, 66,
   43, 65, 42, 35, 57, 3, 8, 65, 79, 76, 82, 80, 96, 82, 98, 57, 73, 43, 6, 20,
   82, 49, 7, 18, 94, 76, 41, 17, 43, 15, 53, 10, 83, 24, 79, 62, 53, 77, 23, 70,
   18, 30, 80, 7, 97, 84, 10, 27, 7, 91, 14, 12, 7, 31, 24, 97, 16, 33, 99, 15,
   31, 65, 51, 95, 45, 70, 57, 10, 84, 52, 28, 43, 54, 40, 83, 9, 21, 57, 45, 67,
   70, 45, 48, 39, 10, 37, 22, 53, 48, 50, 76, 48, 57, 6, 43, 13, 45, 93, 42, 11,
   80, 5, 53, 97, 75, 22, 10, 70, 79, 92, 96, 18, 57, 3, 82, 52, 1, 21, 23, 38,
   43, 79, 67, 57, 33, 52, 1, 44, 82, 10, 27, 23, 89, 9, 62, 6, 38, 33, 37, 22,
   68, 20, 5, 25, 16, 80, 13, 73, 35, 36, 13, 53, 97, 50, 17, 54, 35, 86, 24, 56,
   60, 83, 8, 81, 3, 4, 48, 14, 77, 10, 71, 57, 86, 94, 49, 36, 62, 62, 41, 56,
   31, 77, 5, 97, 19, 19, 31, 19, 26, 41, 77, 64, 74, 11, 98, 30, 22, 22, 33, 61,
   7, 89, 46, 13, 33, 55, 84, 16, 21, 45, 15, 71, 57, 42, 82, 13, 62, 98, 36, 45,
   84, 90, 20, 61, 24, 59, 8, 49, 53, 53, 83, 76, 28, 62, 59, 11, 41, 2, 58, 46,
   32, 23, 53, 5, 8, 91, 97, 53, 90, 90, 28, 16, 61, 27, 32, 74, 23, 11, 57, 20,
   62, 85, 79, 96, 62, 85, 43, 53, 12, 36, 95, 37, 2, 48, 46, 81, 97, 54, 5, 77,
   57, 35, 41, 55, 72, 98, 22, 81, 6, 8, 70, 64, 55, 53, 7, 38, 58, 30, 83, 81,
   15, 11, 24, 63, 27, 90, 35, 22, 53, 22, 66, 75, 59, 80, 31, 91, 63, 82, 99, 62,
   4, 18, 99, 6, 65, 21, 28, 93, 16, 26, 1, 16, 46, 59, 45, 90, 69, 76, 25, 53,
   50, 24, 66, 2, 17, 85, 5, 86, 4, 88, 44, 5, 29, 19, 27, 14, 36, 57, 59, 15,
   71, 79, 7, 61, 45, 72, 61, 45, 61, 54, 90, 33, 81, 5, 45, 64, 87, 82, 61, 8
  };
  OpenShopSpec ex4(20,20,ex4_p);

  /// The instances
  OpenShopSpec examples[] = { ex0, ex1, ex2, ex3, ex4 };
  /// The number of instances
  const unsigned int n_examples = sizeof(examples) / sizeof(OpenShopSpec);

  //@}
}

// STATISTICS: example-any
