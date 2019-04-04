/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace Gecode;

/*
 * The paper uses ideas from: D. Grimes and E. Hebrard, Solving Variants
 * of the Job Shop Scheduling Problem through Conflict-Directed Search,
 * INFORMS Jounral of Computing, Volume 27, Issue 2, 2015.
 *
 * Warning: this solution is a sketch and not competitive as not all
 * techniques from the paper have been implemented.
 *
 */

/// Default configuration settings
namespace JobShopConfig {
  /// Whether to print verbose information
  static const bool verbose = false;

  /// How many probes to perform
  static const unsigned int probes = 50U;
  /// How many failures maximal per probe
  static const unsigned int fail_probe = 10000U;

  /// How much time to spend on probing
  static const unsigned int time_probe  = 30U * 1000U;
  /// How much time to spend on adjusting
  static const unsigned int time_adjust = 30U * 1000U;
  /// How much time to spend on solving
  static const unsigned int time_solve  = 60U * 1000U;

  /// Restart scale factor
  static const double restart_scale = 5000.0;
  /// Restart base
  static const double restart_base = 1.3;
}

// Instance data
namespace {

  // Instances
  extern const int* js[];
  // Instance names
  extern const char* name[];

  /// A wrapper class for instance data
  class Spec {
  protected:
    /// Raw instance data
    const int* data;
    /// Lower and upper bound
    int l, u;
    /// Name
    const char* n;
  public:
    /// Whether a valid specification has been found
    bool valid(void) const {
      return data != nullptr;
    }
    /// Return number of jobs
    int jobs(void) const {
      return data[0];
    }
    /// Return number of machines
    int machines(void) const {
      return data[1];
    }
    /// Return machine of step \a j in job \a i
    int machine(int i, int j) const {
      return data[2 + i*machines()*2 + j*2];
    }
    /// Return duration of step \a j in job \a i
    int duration(int i, int j) const {
      return data[2 + i*machines()*2 + j*2 + 1];
    }
  protected:
    /// Find instance by name \a s
    static const int* find(const char* s) {
      for (int i=0; ::name[i] != nullptr; i++)
        if (!strcmp(s,::name[i]))
          return js[i];
      return nullptr;
    }
    /// Compute lower bound
    int clower(void) const {
      int l = 0;
      Region r;
      int* mach = r.alloc<int>(machines());
      for (int j=0; j<machines(); j++)
        mach[j]=0;
      for (int i=0; i<jobs(); i++) {
        int job = 0;
        for (int j=0; j<machines(); j++) {
          mach[machine(i,j)] += duration(i,j);
          job += duration(i,j);
        }
        l = std::max(l,job);
      }
      for (int j=0; j<machines(); j++)
        l = std::max(l,mach[j]);
      return l;
    }
    /// Compute naive upper bound
    int cupper(void) const {
      int u = 0;
      for (int i=0; i<jobs(); i++)
        for (int j=0; j<machines(); j++)
          u += duration(i,j);
      return u;
    }
  public:
    /// Initialize
    Spec(const char* s) : data(find(s)), l(0), u(0), n(s) {
      if (valid()) {
        l = clower(); u = cupper();
      }
    }
    /// Return lower bound
    int lower(void) const {
      return l;
    }
    /// Return upper bound
    int upper(void) const {
      return u;
    }
    /// Return name
    const char* name(void) const {
      return n;
    }
  };

}

/**
 * \brief %Options for %JobShop problems
 *
 */
class JobShopOptions : public InstanceOptions {
private:
  /// Whether to print schedule
  Driver::BoolOption _verbose;
  /// How many probes
  Driver::UnsignedIntOption _probes;
  /// How many failures per probe
  Driver::UnsignedIntOption _fail_probe;
  /// Time for probing
  Driver::UnsignedIntOption _time_probe;
  /// Time for adjusting
  Driver::UnsignedIntOption _time_adjust;
  /// Time for solving
  Driver::UnsignedIntOption _time_solve;
  /// Tie-breaking factor
  Driver::DoubleOption _tbf;
public:
  /// Initialize options for example with name \a s
  JobShopOptions(const char* s)
    : InstanceOptions(s),
      _verbose("verbose","whether to print schedule",
               JobShopConfig::verbose),
      _probes("probes","how many probes to perform",
              JobShopConfig::probes),
      _fail_probe("fail-probe","failure limit per probe",
                  JobShopConfig::fail_probe),
      _time_probe("time-probe","time-out for probing (in milliseconds)",
                   JobShopConfig::time_probe),                   
      _time_adjust("time-adjust","time-out for adjusting (in milliseconds)",
                   JobShopConfig::time_adjust),                   
      _time_solve("time-solve","time-out for solving (in milliseconds)",
                  JobShopConfig::time_solve),                   
      _tbf("tbf", "tie-breaking factor", 0.0) {
    add(_verbose);
    add(_probes);
    add(_fail_probe);
    add(_time_probe);
    add(_time_adjust);
    add(_time_solve);
    add(_tbf);
  }
  /// Return whether to print schedule
  bool verbose(void) const {
    return _verbose.value();
  }
  /// Return number of probes
  unsigned int probes(void) const {
    return _probes.value();
  }
  /// Return number of failures per probe
  unsigned int fail_probe(void) const {
    return _fail_probe.value();
  }
  /// Return time-out for probe
  unsigned int time_probe(void) const {
    return _time_probe.value();
  }
  /// Return time-out for adjust
  unsigned int time_adjust(void) const {
    return _time_adjust.value();
  }
  /// Return time-out for solve
  unsigned int time_solve(void) const {
    return _time_solve.value();
  }
  /// Return tie-breaking factor
  double tbf(void) const {
    return _tbf.value();
  }
  /// Print help text for list of instances
  virtual void help(void) {
    InstanceOptions::help();
    std::cerr << "\tAvailable instances:" << std::endl << "\t\t";
    for (int i=1; ::name[i] != nullptr; i++) {
      std::cerr << ::name[i] << ", ";
      if (i % 4 == 0)
        std::cerr << std::endl << "\t\t";
    }
    std::cerr << std::endl;
  }
};


/**
 * \brief %Example: Job Shop Scheduling
 *
 * \ingroup Example
 *
 */
class JobShopBase : public IntMinimizeScript {
protected:
  /// Options
  const JobShopOptions& opt;
  /// Specification
  const Spec spec;
  /// Start times for each step in a job
  IntVarArray start;
  /// Makespan
  IntVar makespan;
public:
  /// Actual model
  JobShopBase(const JobShopOptions& o)
    : IntMinimizeScript(o), opt(o), spec(opt.instance()),
      start(*this, spec.machines() * spec.jobs(), 0, spec.upper()),
      makespan(*this, spec.lower(), spec.upper()) {
    // Number of jobs and machines/steps
    int n = spec.jobs(), m = spec.machines();
    // Endtimes for each job
    IntVarArgs end(*this, n, 0, spec.upper());

    // Precedence constraints and makespan
    for (int i=0; i<n; i++) {
      for (int j=1; j<m; j++)
        rel(*this, start[i*m+j-1] + spec.duration(i,j) <= start[i*m+j]);
      rel(*this, start[i*m+m-1] + spec.duration(i,m-1) <= makespan);
    }
  }
  /// Do not overload machines
  void nooverload(void) {
    // Number of jobs and machines/steps
    int n = spec.jobs(), m = spec.machines();

    IntVarArgs jobs(m*n);
    IntArgs dur(m*n);

    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++) {
        jobs[spec.machine(i,j)*n+i] = start[i*m+j];
        dur[spec.machine(i,j)*n+i] = spec.duration(i,j);
      }
    
    for (int j=0; j<m; j++) {
      IntVarArgs jpm(n);
      IntArgs dpm(n);
      for (int i=0; i<n; i++) {
        jpm[i] = jobs[j*n+i]; dpm[i] = dur[j*n+i];
      }
      unary(*this, jpm, dpm);
    }
  }
  /// Return cost
  virtual IntVar cost(void) const {
    return makespan;
  }
  /// Constructor for cloning \a s
  JobShopBase(JobShopBase& s)
    : IntMinimizeScript(s), opt(s.opt), spec(s.spec) {
    start.update(*this, s.start);
    makespan.update(*this, s.makespan);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t\t" << spec.name()
       << " [makespan: " << makespan << "]" << std::endl;
    if (opt.verbose()) {
      // Number of jobs
      int n = spec.jobs();
      // Number of machines/steps
      int m = spec.machines();
      for (int i=0; i<n; i++) {
        os << "\t\t\t[" << i << "]: ";
        for (int j=0; j<m; j++)
          os << start[i*m+j] << "  ";
        os << std::endl;
      }
    }
  }
};

/// Common command line options
class CommonOptions : public Search::Options {
public:
  /// Initialize
  CommonOptions(const JobShopOptions& opt) {
    clone = false;
    threads = opt.threads();
    c_d = opt.c_d();
    a_d = opt.a_d();
    nogoods_limit = opt.nogoods() ? opt.nogoods_limit() : 0U;
  }
};

/// Model for probing
class JobShopProbe : public JobShopBase {
public:
  /// Actual model
  JobShopProbe(const JobShopOptions& o)
    : JobShopBase(o) {
    nooverload();
  }
  void branch(unsigned int p, Rnd r) {
    switch (p) {
    case 0U:
      Gecode::branch(*this, start, INT_VAR_MIN_MIN(), INT_VAL_MIN());
      break;
    case 1U:
      Gecode::branch(*this, start, INT_VAR_MAX_MIN(), INT_VAL_MIN());
      break;
    case 2U:
      Gecode::branch(*this, start, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      break;
    case 3U:
      Gecode::branch(*this, start, tiebreak(INT_VAR_MIN_MIN(),
                                            INT_VAR_RND(r)), INT_VAL_MIN());
      break;
    case 4U:
      Gecode::branch(*this, start, tiebreak(INT_VAR_MAX_MIN(),
                                            INT_VAR_RND(r)), INT_VAL_MIN());
      break;
    default:
      if (p & 1U)
        Gecode::branch(*this, start, INT_VAR_RND(r), INT_VAL_MIN());
      else
        Gecode::branch(*this, start, INT_VAR_RND(r), INT_VAL_SPLIT_MIN());
      break;
    }
    assign(*this, makespan, INT_ASSIGN_MIN());
  }
  /// Constructor for cloning \a s
  JobShopProbe(JobShopProbe& s)
    : JobShopBase(s) {}
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new JobShopProbe(*this);
  }
};

/// Model for solving
class JobShopSolve : public JobShopBase {
protected:
  /// Step order variables
  BoolVarArray sorder;
  /// Record which step is first in order
  IntSharedArray fst;
  /// Record which step is second in order
  IntSharedArray snd;
  /// AFC information
  IntAFC iafc;
  /// AFC-based cost
  double afc(BoolVar x, int i) const {
    return ((x.afc() + start[fst[i]].afc() + start[snd[i]].afc()) /
            (start[fst[i]].size() + start[fst[i]].size()));
  }
  /// Trampoline function for AFC-based cost
  static double afcmerit(const Space& home, BoolVar x, int i) {
    return static_cast<const JobShopSolve&>(home).afc(x,i);
  }
  /// Action information
  IntAction iaction; BoolAction baction;
  /// Action-based cost
  double action(int i) const {
    return ((baction[i] + iaction[fst[i]] + iaction[snd[i]]) /
            (start[fst[i]].size() + start[fst[i]].size()));
  }
  /// Trampoline function for Action-based cost
  static double actionmerit(const Space& home, BoolVar, int i) {
    return static_cast<const JobShopSolve&>(home).action(i);
  }
  /// CHB information
  IntCHB ichb; BoolCHB bchb;
  /// CHB-based cost
  double chb(int i) const {
    return ((bchb[i] + ichb[fst[i]] + ichb[snd[i]]) /
            (start[fst[i]].size() + start[fst[i]].size()));
  }
  /// Trampoline function for CHB-based cost
  static double chbmerit(const Space& home, BoolVar, int i) {
    return static_cast<const JobShopSolve&>(home).chb(i);
  }
  /// Random number generator for probing and relaxation
  Rnd rnd;
public:
  /// Branching to use
  enum {
    BRANCH_AFC,    ///< Branch using AFC
    BRANCH_ACTION, ///< Branch using action
    BRANCH_CHB     ///< Branch using CHB
  };
  /// Propagation to use
  enum {
    PROP_ORDER, ///< Only propagate order constraints
    PROP_UNARY  ///< Also post unary constraints
  };
  /// Actual model
  JobShopSolve(const JobShopOptions& o)
    : JobShopBase(o),
      sorder(*this, spec.machines()*spec.jobs()*(spec.jobs()-1)/2, 0, 1),
      rnd(o.seed()) {
    if (opt.propagation() == PROP_UNARY)
      nooverload();

    // Number of jobs and machines/steps
    int n = spec.jobs(), m = spec.machines();
   
    fst.init(m*n*(n-1)/2);
    snd.init(m*n*(n-1)/2);

    IntArgs jobs(m*n), dur(m*n);

    for (int i=0; i<n; i++)
      for (int j=0; j<m; j++) {
        jobs[spec.machine(i,j)*n+i] = i*m+j;
        dur[spec.machine(i,j)*n+i] = spec.duration(i,j);
      }

    int l=0;
    for (int j=0; j<m; j++) {
      for (int i1=0; i1<n; i1++)
        for (int i2=i1+1; i2<n; i2++) {
          if (dur[j*n+i1] > dur[j*n+i2]) {
            order(*this,
                  start[jobs[j*n+i1]], dur[j*n+i1],
                  start[jobs[j*n+i2]], dur[j*n+i2],
                  sorder[l]);
            fst[l] = j*n+i1; snd[l] = j*n+i2;
          } else {
            order(*this,
                  start[jobs[j*n+i2]], dur[j*n+i2],
                  start[jobs[j*n+i1]], dur[j*n+i1],
                  sorder[l]);
            fst[l] = j*n+i2; snd[l] = j*n+i1;
          }
          l++;
        }
      assert(l == (j+1)*n*(n-1)/2);
    }

    double tbf = opt.tbf();
    switch (opt.branching()) {
    case BRANCH_AFC:
      iafc.init(*this,start,opt.decay());
      if (tbf > 0.0) {
        auto tbl =
          [tbf] (const Space&, double w, double b) {
          assert(b >= w);
          return b - (b - w) * tbf;
        };
        branch(*this, sorder, tiebreak(BOOL_VAR_MERIT_MAX(&afcmerit,tbl),
                                       BOOL_VAR_RND(rnd)),
               BOOL_VAL_MIN());
      } else {
        branch(*this, sorder, BOOL_VAR_MERIT_MAX(&afcmerit),
               BOOL_VAL_MIN());
      }
      break;
    case BRANCH_ACTION:
      iaction.init(*this,start,opt.decay());
      baction.init(*this,sorder,opt.decay());
      if (tbf > 0.0) {
        auto tbl =
          [tbf] (const Space&, double w, double b) {
          assert(b >= w);
          return b - (b - w) * tbf;
        };
        branch(*this, sorder, tiebreak(BOOL_VAR_MERIT_MAX(&actionmerit,tbl),
                                       BOOL_VAR_RND(rnd)),
               BOOL_VAL_MIN());
      } else {
        branch(*this, sorder, BOOL_VAR_MERIT_MAX(&actionmerit),
               BOOL_VAL_MIN());
      }
      break;
    case BRANCH_CHB:
      ichb.init(*this,start);
      bchb.init(*this,sorder);
      if (tbf > 0.0) {
        auto tbl =
          [tbf] (const Space&, double w, double b) {
          assert(b >= w);
          return b - (b - w) * tbf;
        };
        branch(*this, sorder, tiebreak(BOOL_VAR_MERIT_MAX(&chbmerit,tbl),
                                       BOOL_VAR_RND(rnd)),
               BOOL_VAL_MIN());
      } else {
        branch(*this, sorder, BOOL_VAR_MERIT_MAX(&chbmerit),
               BOOL_VAL_MIN());
      }
      break;
    }
    assign(*this, start, INT_VAR_MIN_MIN(), INT_ASSIGN_MIN());
    assign(*this, makespan, INT_ASSIGN_MIN());
  }
  /// Constructor for cloning \a s
  JobShopSolve(JobShopSolve& s)
    : JobShopBase(s), sorder(s.sorder), fst(s.fst), snd(s.snd), 
      iafc(s.iafc), iaction(s.iaction), baction(s.baction),
      ichb(s.ichb), bchb(s.bchb), rnd(s.rnd) {}
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new JobShopSolve(*this);
  }
};

/// Stop object combining time and failuresa
class FailTimeStop : public Search::Stop {
protected:
  Search::FailStop* fs; ///< Used fail stop object
  Search::TimeStop* ts; ///< Used time stop object
public:
  /// Initialize stop object
  FailTimeStop(unsigned int fail, unsigned int time)
      : fs(new Search::FailStop(fail)),
        ts(new Search::TimeStop(time)) {}
  /// Test whether search must be stopped
  virtual bool stop(const Search::Statistics& s, const Search::Options& o) {
    return fs->stop(s,o) || ts->stop(s,o);
  }
  /// Whether the stop was due to failures
  bool fail(const Search::Statistics& s, const Search::Options& o) const {
    return fs->stop(s,o);
  }
  /// Whether the stop was due to time
  bool time(const Search::Statistics& s, const Search::Options& o) const {
    return ts->stop(s,o);
  }
  /// Destructor
  ~FailTimeStop(void) {
    delete fs; delete ts;
  }
};

/// Print statistics
void
print(const Search::Statistics& stat, bool restart) {
  using namespace std;
  cout << "\t\t\tnodes:      " << stat.node << endl
       << "\t\t\tfailures:   " << stat.fail << endl;
  if (restart)
    cout << "\t\t\trestarts:   " << stat.restart << endl
         << "\t\t\tno-goods:   " << stat.nogood << endl;
  cout << "\t\t\tpeak depth: " << stat.depth << endl;
}

/// Solver
void
solve(const JobShopOptions& opt) {
  Rnd rnd(opt.seed());

  /*
   * Invariant:
   *  - There is a solution with makespan u,
   *  - There is no solution with makespan l
   */

  int l, u;

  {
    Support::Timer t; t.start();
    Search::Statistics stat;
    JobShopProbe* master = new JobShopProbe(opt);

    if (master->status() != SS_SOLVED) {
      delete master;
      std::cerr << "Error: has no solution..." << std::endl;
      return;
    }

    l = master->cost().min()-1;
    u = master->cost().max();

    FailTimeStop fts(opt.fail_probe(),opt.time_probe());
    CommonOptions so(opt);
    so.stop = &fts;
    bool stopped = false;

    std::cout << "\tProbing..." << std::endl;
    
    for (unsigned int p=0; p<opt.probes(); p++) {
      JobShopProbe* jsp = static_cast<JobShopProbe*>(master->clone());
      jsp->branch(p,rnd);
      DFS<JobShopProbe> dfs(jsp,so);
      JobShopProbe* s = dfs.next();
      Search::Statistics statj = dfs.statistics();
      
      if (s != nullptr) {
        if (u > s->cost().val()) {
          u = s->cost().val();
          s->print(std::cout);
        }
        delete s;
      } else if (fts.time(statj,so)) {
        stopped = true;
        break;
      }
      stat += statj;
    }
    delete master;

    print(stat,false);
    std::cout << "\t\t\truntime:    ";
    Driver::stop(t,std::cout);
    std::cout << std::endl;

    if (stopped) {
      std::cout << "\t\t\t\tstopped due to time-out..." << std::endl;
    }
  }

  std::cout << std::endl << "\tAdjusting..." << std::endl;

  // Dictotomic search
  {
    JobShopSolve* master = new JobShopSolve(opt);

    if (master->status() == SS_FAILED) {
      delete master;
      std::cerr << "Error: has no solution..." << std::endl;
      return;
    }

    {
      Support::Timer t; t.start();
      Search::Statistics stat;
      CommonOptions so(opt);
      so.stop = Search::Stop::time(opt.time_adjust());
      bool stopped = false;
      while (l < u-1) {
        std::cout << "\t\tBounds: [" << l << "," << u << "]"
                  << std::endl;
        JobShopSolve* jss = static_cast<JobShopSolve*>(master->clone());
        int m = (l + u) / 2;
        rel(*jss, jss->cost() >= l);
        rel(*jss, jss->cost() <= m);
        so.cutoff = Search::Cutoff::geometric(JobShopConfig::restart_scale,
                                              JobShopConfig::restart_base);
        RBS<JobShopSolve,DFS> rbs(jss,so);
        JobShopSolve* s = rbs.next();

        stat += rbs.statistics();

        if (s != nullptr) {
          s->print(std::cout);
          u = s->cost().val();
          delete s;
        } else if (rbs.stopped()) {
          stopped = true;
          break;
        } else {
          l = m+1;
        }
      }

      print(stat,true);
      std::cout << "\t\t\truntime:    ";
      Driver::stop(t,std::cout);
      std::cout << std::endl;

      if (stopped) {
        std::cout << "\t\t\t\tstopped due to time-out..." << std::endl;
      }

    }

    if (l == u-1) {
      delete master;
      std::cout << std::endl
                << "\tFound best solution and proved optimality."
                << std::endl;
      return;
    }


    { 
      Support::Timer t; t.start();
      std::cout << std::endl << "\tSolving..." << std::endl;

      rel(*master, master->cost() >= l);
      rel(*master, master->cost() < u);

      CommonOptions so(opt);
      so.stop = Search::Stop::time(opt.time_solve());
      so.cutoff = Search::Cutoff::geometric(JobShopConfig::restart_scale,
                                            JobShopConfig::restart_base);
      RBS<JobShopSolve,BAB> rbs(master,so);
      while (JobShopSolve* s = rbs.next()) {
        s->print(std::cout);
        u = s->cost().val();
        delete s;
      }

      print(rbs.statistics(),true);
      std::cout << "\t\t\truntime:    ";
      Driver::stop(t,std::cout);
      std::cout << std::endl;

      if (rbs.stopped()) {
        std::cout << "\t\t\t\tstopped due to time-out..." << std::endl;
        std::cout << std::endl
                  << "\tSolution at most ";
        double a = (static_cast<double>(u-l+1) / u) * 100.0;
        std::cout << std::setprecision(2) << a
                  << "% away from optimum."
                  << std::endl;

      } else {
        std::cout << std::endl
                  << "\tFound best solution and proved optimality."
                  << std::endl;
      }
    }

  }
}





/** \brief Main-function
 *  \relates JobShop
 */
int
main(int argc, char* argv[]) {
  JobShopOptions opt("JobShop");

  opt.branching(JobShopSolve::BRANCH_AFC);
  opt.branching(JobShopSolve::BRANCH_AFC, "afc");
  opt.branching(JobShopSolve::BRANCH_ACTION, "action");
  opt.branching(JobShopSolve::BRANCH_CHB, "chb");

  opt.propagation(JobShopSolve::PROP_UNARY);
  opt.propagation(JobShopSolve::PROP_ORDER,"order");
  opt.propagation(JobShopSolve::PROP_UNARY,"unary");

  opt.instance("ft06");

  opt.restart_base(JobShopConfig::restart_base);
  opt.restart_scale(JobShopConfig::restart_scale);
  opt.nogoods(true);

  opt.parse(argc,argv);
  if (!Spec(opt.instance()).valid()) {
    std::cerr << "Error: unkown instance" << std::endl;
    return 1;
  }
  solve(opt);
  return 0;
}

namespace {

  // Test instance
  const int test[] = {
    4, 6, // Number of jobs and machines
    0,1, 1,2, 2,3, 3,4, 4,2, 5,1,
    5,4, 1,3, 2,2, 4,1, 0,1, 3,2,
    5,1, 4,2, 1,3, 0,4, 2,2, 3,2,
    5,4, 4,3, 1,2, 0,1, 3,4, 2,3
  };
  
  /*
   * These instances are contributed to the OR-Library by
   * Dirk C. Mattfeld (email dirk@uni-bremen.de) and 
   * Rob J.M. Vaessens (email robv@win.tue.nl).
   *
   * o abz5-abz9 are from
   *   J. Adams, E. Balas and D. Zawack (1988),
   *   The shifting bottleneck procedure for job shop scheduling,
   *   Management Science 34, 391-401.
   * o ft06, ft10, and ft20 are from 
   *   H. Fisher, G.L. Thompson (1963), 
   *   Probabilistic learning combinations of local job-shop scheduling rules, 
   *     J.F. Muth, G.L. Thompson (eds.), 
   *     Industrial Scheduling, 
   *     Prentice Hall, Englewood Cliffs, New Jersey, 
   *   225-251.
   * o la01-la40 are from 
   *   S. Lawrence (1984),
   *   Resource constrained project scheduling: an experimental investigation of 
   *     heuristic scheduling techniques (Supplement), 
   *   Graduate School of Industrial Administration,
   *   Carnegie-Mellon University, Pittsburgh, Pennsylvania.
   * o orb01-orb10 are from 
   *   D. Applegate, W. Cook (1991),
   *   A computational study of the job-shop scheduling instance,
   *   ORSA Journal on Computing 3, 149-156. 
   *   (they were generated in Bonn in 1986)
   * o swv01-swv20 are from 
   *   R.H. Storer, S.D. Wu, R. Vaccari (1992),
   *   New search spaces for sequencing instances with application to job shop 
   *     scheduling, 
   *   Management Science 38, 1495-1509.
   * o yn1-yn4 are from 
   *   T. Yamada, R. Nakano (1992),
   *   A genetic algorithm applicable to large-scale job-shop instances,
   *     R. Manner, B. Manderick (eds.),
   *     Parallel instance solving from nature 2,
   *     North-Holland, Amsterdam, 
   *   281-290.
   *
   * The following are from E. Taillard, "Benchmarks for basic scheduling problems", 
   * EJOR 64(2):278-285, 1993.
   * o taillard-n-m-i: n jobs, m machines, i-th instance
   *   the machine numbers are decremented by one to start with machine 0.
   *
   */

  // Adams, Balas, and Zawack 10x10 instance (Table 1, instance 5)
  const int abz5[] = {
    10, 10, // Number of jobs and machines
    4, 88, 8, 68, 6, 94, 5, 99, 1, 67, 2, 89, 9, 77, 7, 99, 0, 86, 3, 92, 
    5, 72, 3, 50, 6, 69, 4, 75, 2, 94, 8, 66, 0, 92, 1, 82, 7, 94, 9, 63, 
    9, 83, 8, 61, 0, 83, 1, 65, 6, 64, 5, 85, 7, 78, 4, 85, 2, 55, 3, 77, 
    7, 94, 2, 68, 1, 61, 4, 99, 3, 54, 6, 75, 5, 66, 0, 76, 9, 63, 8, 67, 
    3, 69, 4, 88, 9, 82, 8, 95, 0, 99, 2, 67, 6, 95, 5, 68, 7, 67, 1, 86, 
    1, 99, 4, 81, 5, 64, 6, 66, 8, 80, 2, 80, 7, 69, 9, 62, 3, 79, 0, 88, 
    7, 50, 1, 86, 4, 97, 3, 96, 0, 95, 8, 97, 2, 66, 5, 99, 6, 52, 9, 71, 
    4, 98, 6, 73, 3, 82, 2, 51, 1, 71, 5, 94, 7, 85, 0, 62, 8, 95, 9, 79, 
    0, 94, 6, 71, 3, 81, 7, 85, 1, 66, 2, 90, 4, 76, 5, 58, 8, 93, 9, 97, 
    3, 50, 0, 59, 1, 82, 8, 67, 7, 56, 9, 96, 6, 58, 4, 81, 5, 59, 2, 96
  };
  // Adams, and Zawack 10x10 instance (Table 1, instance 6)
  const int abz6[] = {
    10, 10, // Number of jobs and machines
    7, 62, 8, 24, 5, 25, 3, 84, 4, 47, 6, 38, 2, 82, 0, 93, 9, 24, 1, 66, 
    5, 47, 2, 97, 8, 92, 9, 22, 1, 93, 4, 29, 7, 56, 3, 80, 0, 78, 6, 67, 
    1, 45, 7, 46, 6, 22, 2, 26, 9, 38, 0, 69, 4, 40, 3, 33, 8, 75, 5, 96, 
    4, 85, 8, 76, 5, 68, 9, 88, 3, 36, 6, 75, 2, 56, 1, 35, 0, 77, 7, 85, 
    8, 60, 9, 20, 7, 25, 3, 63, 4, 81, 0, 52, 1, 30, 5, 98, 6, 54, 2, 86, 
    3, 87, 9, 73, 5, 51, 2, 95, 4, 65, 1, 86, 6, 22, 8, 58, 0, 80, 7, 65, 
    5, 81, 2, 53, 7, 57, 6, 71, 9, 81, 0, 43, 4, 26, 8, 54, 3, 58, 1, 69, 
    4, 20, 6, 86, 5, 21, 8, 79, 9, 62, 2, 34, 0, 27, 1, 81, 7, 30, 3, 46, 
    9, 68, 6, 66, 5, 98, 8, 86, 7, 66, 0, 56, 3, 82, 1, 95, 4, 47, 2, 78, 
    0, 30, 3, 50, 7, 34, 2, 58, 1, 77, 5, 34, 8, 84, 4, 40, 9, 46, 6, 44
  };
  // Adams, Balas, and Zawack 15 x 20 instance (Table 1, instance 7)
  const int abz7[] = {
    20, 15, // Number of jobs and machines
    2, 24, 3, 12, 9, 17, 4, 27, 0, 21, 6, 25, 8, 27, 7, 26, 1, 30, 5, 31, 11, 18, 14, 16, 13, 39, 10, 19, 12, 26, 
    6, 30, 3, 15, 12, 20, 11, 19, 1, 24, 13, 15, 10, 28, 2, 36, 5, 26, 7, 15, 0, 11, 8, 23, 14, 20, 9, 26, 4, 28, 
    6, 35, 0, 22, 13, 23, 7, 32, 2, 20, 3, 12, 12, 19, 10, 23, 9, 17, 1, 14, 5, 16, 11, 29, 8, 16, 4, 22, 14, 22, 
    9, 20, 6, 29, 1, 19, 7, 14, 12, 33, 4, 30, 0, 32, 5, 21, 11, 29, 10, 24, 14, 25, 2, 29, 3, 13, 8, 20, 13, 18, 
    11, 23, 13, 20, 1, 28, 6, 32, 7, 16, 5, 18, 8, 24, 9, 23, 3, 24, 10, 34, 2, 24, 0, 24, 14, 28, 12, 15, 4, 18, 
    8, 24, 11, 19, 14, 21, 1, 33, 7, 34, 6, 35, 5, 40, 10, 36, 3, 23, 2, 26, 4, 15, 9, 28, 13, 38, 12, 13, 0, 25, 
    13, 27, 3, 30, 6, 21, 8, 19, 12, 12, 4, 27, 2, 39, 9, 13, 14, 12, 5, 36, 10, 21, 11, 17, 1, 29, 0, 17, 7, 33, 
    5, 27, 4, 19, 6, 29, 9, 20, 3, 21, 10, 40, 8, 14, 14, 39, 13, 39, 2, 27, 1, 36, 12, 12, 11, 37, 7, 22, 0, 13, 
    13, 32, 11, 29, 8, 24, 3, 27, 5, 40, 4, 21, 9, 26, 0, 27, 14, 27, 6, 16, 2, 21, 10, 13, 7, 28, 12, 28, 1, 32, 
    12, 35, 1, 11, 5, 39, 14, 18, 7, 23, 0, 34, 3, 24, 13, 11, 8, 30, 11, 31, 4, 15, 10, 15, 2, 28, 9, 26, 6, 33, 
    10, 28, 5, 37, 12, 29, 1, 31, 7, 25, 8, 13, 14, 14, 4, 20, 3, 27, 9, 25, 13, 31, 11, 14, 6, 25, 2, 39, 0, 36, 
    0, 22, 11, 25, 5, 28, 13, 35, 4, 31, 8, 21, 9, 20, 14, 19, 2, 29, 7, 32, 10, 18, 1, 18, 3, 11, 12, 17, 6, 15, 
    12, 39, 5, 32, 2, 36, 8, 14, 3, 28, 13, 37, 0, 38, 6, 20, 7, 19, 11, 12, 14, 22, 1, 36, 4, 15, 9, 32, 10, 16, 
    8, 28, 1, 29, 14, 40, 12, 23, 4, 34, 5, 33, 6, 27, 10, 17, 0, 20, 7, 28, 11, 21, 2, 21, 13, 20, 9, 33, 3, 27, 
    9, 21, 14, 34, 3, 30, 12, 38, 0, 11, 11, 16, 2, 14, 5, 14, 1, 34, 8, 33, 4, 23, 13, 40, 10, 12, 6, 23, 7, 27, 
    9, 13, 14, 40, 7, 36, 4, 17, 0, 13, 5, 33, 8, 25, 13, 24, 10, 23, 3, 36, 2, 29, 1, 18, 11, 13, 6, 33, 12, 13, 
    3, 25, 5, 15, 2, 28, 12, 40, 7, 39, 1, 31, 8, 35, 6, 31, 11, 36, 4, 12, 10, 33, 14, 19, 9, 16, 13, 27, 0, 21, 
    12, 22, 10, 14, 0, 12, 2, 20, 5, 12, 1, 18, 11, 17, 8, 39, 14, 31, 3, 31, 7, 32, 9, 20, 13, 29, 4, 13, 6, 26, 
    5, 18, 10, 30, 7, 38, 14, 22, 13, 15, 11, 20, 9, 16, 3, 17, 1, 12, 2, 13, 12, 40, 6, 17, 8, 30, 4, 38, 0, 13, 
    9, 31, 8, 39, 12, 27, 1, 14, 5, 33, 3, 31, 11, 22, 13, 36, 0, 16, 7, 11, 14, 14, 4, 29, 6, 28, 2, 22, 10, 17
  };
  // Adams, Balas, and Zawack 15 x 20 instance (Table 1, instance 8)
  const int abz8[] = {
    20, 15, // Number of jobs and machines
    0, 19, 9, 33, 2, 32, 13, 18, 10, 39, 8, 34, 6, 25, 4, 36, 11, 40, 12, 33, 1, 31, 14, 30, 3, 34, 5, 26, 7, 13, 
    9, 11, 10, 22, 14, 19, 5, 12, 4, 25, 6, 38, 0, 29, 7, 39, 13, 19, 11, 22, 1, 23, 3, 20, 2, 40, 12, 19, 8, 26, 
    3, 25, 8, 17, 11, 24, 13, 40, 10, 32, 14, 16, 5, 39, 9, 19, 0, 24, 1, 39, 4, 17, 2, 35, 7, 38, 6, 20, 12, 31, 
    14, 22, 3, 36, 2, 34, 12, 17, 4, 30, 13, 12, 1, 13, 6, 25, 9, 12, 7, 18, 10, 31, 0, 39, 5, 40, 8, 26, 11, 37, 
    12, 32, 14, 15, 1, 35, 7, 13, 8, 32, 11, 23, 6, 22, 4, 21, 0, 38, 2, 38, 3, 40, 10, 31, 5, 11, 13, 37, 9, 16, 
    10, 23, 12, 38, 8, 11, 14, 27, 9, 11, 6, 25, 5, 14, 4, 12, 2, 27, 11, 26, 7, 29, 3, 28, 13, 21, 0, 20, 1, 30, 
    6, 39, 8, 38, 0, 15, 12, 27, 10, 22, 9, 27, 2, 32, 4, 40, 3, 12, 13, 20, 14, 21, 11, 22, 5, 17, 7, 38, 1, 27, 
    11, 11, 13, 24, 10, 38, 8, 15, 9, 19, 14, 13, 5, 30, 0, 26, 2, 29, 6, 33, 12, 21, 1, 15, 3, 21, 4, 28, 7, 33, 
    8, 20, 6, 17, 5, 26, 3, 34, 9, 23, 0, 16, 2, 18, 4, 35, 12, 24, 10, 16, 11, 26, 7, 12, 14, 13, 13, 27, 1, 19, 
    1, 18, 7, 37, 14, 27, 9, 40, 5, 40, 6, 17, 8, 22, 3, 17, 10, 30, 0, 38, 4, 21, 12, 32, 11, 24, 13, 24, 2, 30, 
    11, 19, 0, 22, 13, 36, 6, 18, 5, 22, 3, 17, 14, 35, 10, 34, 7, 23, 8, 19, 2, 29, 1, 22, 12, 17, 4, 33, 9, 39, 
    6, 32, 3, 22, 12, 24, 5, 13, 4, 13, 1, 11, 0, 11, 13, 25, 8, 13, 2, 15, 10, 33, 11, 17, 14, 16, 9, 38, 7, 24, 
    14, 16, 13, 16, 1, 37, 8, 25, 2, 26, 3, 11, 9, 34, 4, 14, 0, 20, 6, 36, 12, 12, 5, 29, 10, 25, 7, 32, 11, 12, 
    8, 20, 10, 24, 11, 27, 9, 38, 5, 34, 12, 39, 7, 33, 4, 37, 2, 31, 13, 15, 14, 34, 3, 33, 6, 26, 1, 36, 0, 14, 
    8, 31, 0, 17, 9, 13, 1, 21, 10, 17, 7, 19, 13, 14, 3, 40, 5, 32, 11, 25, 2, 34, 14, 23, 6, 13, 12, 40, 4, 26, 
    8, 38, 12, 17, 3, 14, 13, 17, 4, 12, 1, 35, 6, 35, 0, 19, 10, 36, 7, 19, 9, 29, 2, 31, 5, 26, 11, 35, 14, 37, 
    14, 20, 3, 16, 0, 33, 10, 14, 5, 27, 7, 31, 8, 16, 6, 31, 12, 28, 9, 37, 4, 37, 2, 29, 11, 38, 1, 30, 13, 36, 
    11, 18, 3, 37, 14, 16, 6, 15, 8, 14, 12, 11, 13, 32, 5, 12, 1, 11, 10, 29, 7, 19, 4, 12, 9, 18, 2, 26, 0, 39, 
    11, 11, 2, 11, 12, 22, 9, 35, 14, 20, 7, 31, 4, 19, 3, 39, 5, 28, 6, 33, 10, 34, 1, 38, 0, 20, 13, 17, 8, 28, 
    2, 12, 12, 25, 5, 23, 8, 21, 6, 27, 9, 30, 14, 23, 11, 39, 3, 26, 13, 34, 7, 17, 1, 24, 4, 12, 0, 19, 10, 36
  };
  // Adams, Balas, and Zawack 15 x 20 instance (Table 1, instance 9)
  const int abz9[] = {
    20, 15, // Number of jobs and machines
    6, 14, 5, 21, 8, 13, 4, 11, 1, 11, 14, 35, 13, 20, 11, 17, 10, 18, 12, 11, 2, 23, 3, 13, 0, 15, 7, 11, 9, 35, 
    1, 35, 5, 31, 0, 13, 3, 26, 6, 14, 9, 17, 7, 38, 12, 20, 10, 19, 13, 12, 8, 16, 4, 34, 11, 15, 14, 12, 2, 14, 
    0, 30, 4, 35, 2, 40, 10, 35, 6, 30, 14, 23, 8, 29, 13, 37, 7, 38, 3, 40, 9, 26, 12, 11, 1, 40, 11, 36, 5, 17, 
    7, 40, 5, 18, 4, 12, 8, 23, 0, 23, 9, 14, 13, 16, 12, 14, 10, 23, 3, 12, 6, 16, 14, 32, 1, 40, 11, 25, 2, 29, 
    2, 35, 3, 15, 12, 31, 11, 28, 6, 32, 4, 30, 10, 27, 7, 29, 0, 38, 13, 11, 1, 23, 14, 17, 5, 27, 9, 37, 8, 29, 
    5, 33, 3, 33, 6, 19, 12, 40, 10, 19, 0, 33, 13, 26, 2, 31, 11, 28, 7, 36, 4, 38, 1, 21, 14, 25, 9, 40, 8, 35, 
    13, 25, 0, 32, 11, 33, 12, 18, 4, 32, 6, 28, 5, 15, 3, 35, 9, 14, 2, 34, 7, 23, 10, 32, 1, 17, 14, 26, 8, 19, 
    2, 16, 12, 33, 9, 34, 11, 30, 13, 40, 8, 12, 14, 26, 5, 26, 6, 15, 3, 21, 1, 40, 4, 32, 0, 14, 7, 30, 10, 35, 
    2, 17, 10, 16, 14, 20, 6, 24, 8, 26, 3, 36, 12, 22, 0, 14, 13, 11, 9, 20, 7, 23, 1, 29, 11, 23, 4, 15, 5, 40, 
    4, 27, 9, 37, 3, 40, 11, 14, 13, 25, 7, 30, 0, 34, 2, 11, 5, 15, 12, 32, 1, 36, 10, 12, 14, 28, 8, 31, 6, 23, 
    13, 25, 0, 22, 3, 27, 8, 14, 5, 25, 6, 20, 14, 18, 7, 14, 1, 19, 2, 17, 4, 27, 9, 22, 12, 22, 11, 27, 10, 21, 
    14, 34, 10, 15, 0, 22, 3, 29, 13, 34, 6, 40, 7, 17, 2, 32, 12, 20, 5, 39, 4, 31, 11, 16, 1, 37, 8, 33, 9, 13, 
    6, 12, 12, 27, 4, 17, 2, 24, 8, 11, 5, 19, 14, 11, 3, 17, 9, 25, 1, 11, 11, 31, 13, 33, 7, 31, 10, 12, 0, 22, 
    5, 22, 14, 15, 0, 16, 8, 32, 7, 20, 4, 22, 9, 11, 13, 19, 1, 30, 12, 33, 6, 29, 11, 18, 3, 34, 10, 32, 2, 18, 
    5, 27, 3, 26, 10, 28, 6, 37, 4, 18, 12, 12, 11, 11, 13, 26, 7, 27, 9, 40, 14, 19, 1, 24, 2, 18, 0, 12, 8, 34, 
    8, 15, 5, 28, 9, 25, 6, 32, 1, 13, 7, 38, 11, 11, 2, 34, 4, 25, 0, 20, 10, 32, 3, 23, 12, 14, 14, 16, 13, 20, 
    1, 15, 4, 13, 8, 37, 3, 14, 10, 22, 5, 24, 12, 26, 7, 22, 9, 34, 14, 22, 11, 19, 13, 32, 0, 29, 2, 13, 6, 35, 
    7, 36, 5, 33, 13, 28, 9, 20, 10, 30, 4, 33, 14, 29, 0, 34, 3, 22, 11, 12, 6, 30, 8, 12, 1, 35, 2, 13, 12, 35, 
    14, 26, 11, 31, 5, 35, 2, 38, 13, 19, 10, 35, 4, 27, 8, 29, 3, 39, 9, 13, 6, 14, 7, 26, 0, 17, 1, 22, 12, 15, 
    1, 36, 7, 34, 11, 33, 8, 17, 14, 38, 6, 39, 5, 16, 3, 27, 13, 29, 2, 16, 0, 16, 4, 19, 9, 40, 12, 35, 10, 39
  };

  // Fisher and Thompson 6x6 instance, alternate name (mt06)
  const int ft06[] = {
    6, 6, // Number of jobs and machines
    2, 1, 0, 3, 1, 6, 3, 7, 5, 3, 4, 6, 
    1, 8, 2, 5, 4, 10, 5, 10, 0, 10, 3, 4, 
    2, 5, 3, 4, 5, 8, 0, 9, 1, 1, 4, 7, 
    1, 5, 0, 5, 2, 5, 3, 3, 4, 8, 5, 9, 
    2, 9, 1, 3, 4, 5, 5, 4, 0, 3, 3, 1, 
    1, 3, 3, 3, 5, 9, 0, 10, 4, 4, 2, 1
  };
  // Fisher and Thompson 10x10 instance, alternate name (mt10)
  const int ft10[] = {
    10, 10, // Number of jobs and machines
    0, 29, 1, 78, 2, 9, 3, 36, 4, 49, 5, 11, 6, 62, 7, 56, 8, 44, 9, 21, 
    0, 43, 2, 90, 4, 75, 9, 11, 3, 69, 1, 28, 6, 46, 5, 46, 7, 72, 8, 30, 
    1, 91, 0, 85, 3, 39, 2, 74, 8, 90, 5, 10, 7, 12, 6, 89, 9, 45, 4, 33, 
    1, 81, 2, 95, 0, 71, 4, 99, 6, 9, 8, 52, 7, 85, 3, 98, 9, 22, 5, 43, 
    2, 14, 0, 6, 1, 22, 5, 61, 3, 26, 4, 69, 8, 21, 7, 49, 9, 72, 6, 53, 
    2, 84, 1, 2, 5, 52, 3, 95, 8, 48, 9, 72, 0, 47, 6, 65, 4, 6, 7, 25, 
    1, 46, 0, 37, 3, 61, 2, 13, 6, 32, 5, 21, 9, 32, 8, 89, 7, 30, 4, 55, 
    2, 31, 0, 86, 1, 46, 5, 74, 4, 32, 6, 88, 8, 19, 9, 48, 7, 36, 3, 79, 
    0, 76, 1, 69, 3, 76, 5, 51, 2, 85, 9, 11, 6, 40, 7, 89, 4, 26, 8, 74, 
    1, 85, 0, 13, 2, 61, 6, 7, 8, 64, 9, 76, 5, 47, 3, 52, 4, 90, 7, 45
  };
  // Fisher and Thompson 20x5 instance, alternate name (mt20)
  const int ft20[] = {
    20, 5, // Number of jobs and machines
    0, 29, 1, 9, 2, 49, 3, 62, 4, 44, 
    0, 43, 1, 75, 3, 69, 2, 46, 4, 72, 
    1, 91, 0, 39, 2, 90, 4, 12, 3, 45, 
    1, 81, 0, 71, 4, 9, 2, 85, 3, 22, 
    2, 14, 1, 22, 0, 26, 3, 21, 4, 72, 
    2, 84, 1, 52, 4, 48, 0, 47, 3, 6, 
    1, 46, 0, 61, 2, 32, 3, 32, 4, 30, 
    2, 31, 1, 46, 0, 32, 3, 19, 4, 36, 
    0, 76, 3, 76, 2, 85, 1, 40, 4, 26, 
    1, 85, 2, 61, 0, 64, 3, 47, 4, 90, 
    1, 78, 3, 36, 0, 11, 4, 56, 2, 21, 
    2, 90, 0, 11, 1, 28, 3, 46, 4, 30, 
    0, 85, 2, 74, 1, 10, 3, 89, 4, 33, 
    2, 95, 0, 99, 1, 52, 3, 98, 4, 43, 
    0, 6, 1, 61, 4, 69, 2, 49, 3, 53, 
    1, 2, 0, 95, 3, 72, 4, 65, 2, 25, 
    0, 37, 2, 13, 1, 21, 3, 89, 4, 55, 
    0, 86, 1, 74, 4, 88, 2, 48, 3, 79, 
    1, 69, 2, 51, 0, 11, 3, 89, 4, 74, 
    0, 13, 1, 7, 2, 76, 3, 52, 4, 45
  };

  // Lawrence 10x5 instance (Table 3, instance 1); also called (setf1) or (F1)
  const int la01[] = {
    10, 5, // Number of jobs and machines
    1, 21, 0, 53, 4, 95, 3, 55, 2, 34, 
    0, 21, 3, 52, 4, 16, 2, 26, 1, 71, 
    3, 39, 4, 98, 1, 42, 2, 31, 0, 12, 
    1, 77, 0, 55, 4, 79, 2, 66, 3, 77, 
    0, 83, 3, 34, 2, 64, 1, 19, 4, 37, 
    1, 54, 2, 43, 4, 79, 0, 92, 3, 62, 
    3, 69, 4, 77, 1, 87, 2, 87, 0, 93, 
    2, 38, 0, 60, 1, 41, 3, 24, 4, 83, 
    3, 17, 1, 49, 4, 25, 0, 44, 2, 98, 
    4, 77, 3, 79, 2, 43, 1, 75, 0, 96
  };
  // Lawrence 10x5 instance (Table 3, instance 2); also called (setf2) or (F2)
  const int la02[] = {
    10, 5, // Number of jobs and machines
    0, 20, 3, 87, 1, 31, 4, 76, 2, 17, 
    4, 25, 2, 32, 0, 24, 1, 18, 3, 81, 
    1, 72, 2, 23, 4, 28, 0, 58, 3, 99, 
    2, 86, 1, 76, 4, 97, 0, 45, 3, 90, 
    4, 27, 0, 42, 3, 48, 2, 17, 1, 46, 
    1, 67, 0, 98, 4, 48, 3, 27, 2, 62, 
    4, 28, 1, 12, 3, 19, 0, 80, 2, 50, 
    1, 63, 0, 94, 2, 98, 3, 50, 4, 80, 
    4, 14, 0, 75, 2, 50, 1, 41, 3, 55, 
    4, 72, 2, 18, 1, 37, 3, 79, 0, 61
  };
  // Lawrence 10x5 instance (Table 3, instance 3); also called (setf3) or (F3)
  const int la03[] = {
    10, 5, // Number of jobs and machines
    1, 23, 2, 45, 0, 82, 4, 84, 3, 38, 
    2, 21, 1, 29, 0, 18, 4, 41, 3, 50, 
    2, 38, 3, 54, 4, 16, 0, 52, 1, 52, 
    4, 37, 0, 54, 2, 74, 1, 62, 3, 57, 
    4, 57, 0, 81, 1, 61, 3, 68, 2, 30, 
    4, 81, 0, 79, 1, 89, 2, 89, 3, 11, 
    3, 33, 2, 20, 0, 91, 4, 20, 1, 66, 
    4, 24, 1, 84, 0, 32, 2, 55, 3, 8, 
    4, 56, 0, 7, 3, 54, 2, 64, 1, 39, 
    4, 40, 1, 83, 0, 19, 2, 8, 3, 7
  };
  // Lawrence 10x5 instance (Table 3, instance 4); also called (setf4) or (F4)
  const int la04[] = {
    10, 5, // Number of jobs and machines
    0, 12, 2, 94, 3, 92, 4, 91, 1, 7, 
    1, 19, 3, 11, 4, 66, 2, 21, 0, 87, 
    1, 14, 0, 75, 3, 13, 4, 16, 2, 20, 
    2, 95, 4, 66, 0, 7, 3, 7, 1, 77, 
    1, 45, 3, 6, 4, 89, 0, 15, 2, 34, 
    3, 77, 2, 20, 0, 76, 4, 88, 1, 53, 
    2, 74, 1, 88, 0, 52, 3, 27, 4, 9, 
    1, 88, 3, 69, 0, 62, 4, 98, 2, 52, 
    2, 61, 4, 9, 0, 62, 1, 52, 3, 90, 
    2, 54, 4, 5, 3, 59, 1, 15, 0, 88
  };
  // Lawrence 10x5 instance (Table 3, instance 5); also called (setf5) or (F5)
  const int la05[] = {
    10, 5, // Number of jobs and machines
    1, 72, 0, 87, 4, 95, 2, 66, 3, 60, 
    4, 5, 3, 35, 0, 48, 2, 39, 1, 54, 
    1, 46, 3, 20, 2, 21, 0, 97, 4, 55, 
    0, 59, 3, 19, 4, 46, 1, 34, 2, 37, 
    4, 23, 2, 73, 3, 25, 1, 24, 0, 28, 
    3, 28, 0, 45, 4, 5, 1, 78, 2, 83, 
    0, 53, 3, 71, 1, 37, 4, 29, 2, 12, 
    4, 12, 2, 87, 3, 33, 1, 55, 0, 38, 
    2, 49, 3, 83, 1, 40, 0, 48, 4, 7, 
    2, 65, 3, 17, 0, 90, 4, 27, 1, 23
  };
  // Lawrence 15x5 instance (Table 4, instance 1); also called (setg1) or (G1)
  const int la06[] = {
    15, 5, // Number of jobs and machines
    1, 21, 2, 34, 4, 95, 0, 53, 3, 55, 
    3, 52, 4, 16, 1, 71, 2, 26, 0, 21, 
    2, 31, 0, 12, 1, 42, 3, 39, 4, 98, 
    3, 77, 1, 77, 4, 79, 0, 55, 2, 66, 
    4, 37, 3, 34, 2, 64, 1, 19, 0, 83, 
    2, 43, 1, 54, 0, 92, 3, 62, 4, 79, 
    0, 93, 3, 69, 1, 87, 4, 77, 2, 87, 
    0, 60, 1, 41, 2, 38, 4, 83, 3, 24, 
    2, 98, 3, 17, 4, 25, 0, 44, 1, 49, 
    0, 96, 4, 77, 3, 79, 1, 75, 2, 43, 
    4, 28, 2, 35, 0, 95, 3, 76, 1, 7, 
    0, 61, 4, 10, 2, 95, 1, 9, 3, 35, 
    4, 59, 3, 16, 1, 91, 2, 59, 0, 46, 
    4, 43, 1, 52, 0, 28, 2, 27, 3, 50, 
    0, 87, 1, 45, 2, 39, 4, 9, 3, 41
  };
  // Lawrence 15x5 instance (Table 4, instance 2); also called (setg2) or (G2)
  const int la07[] = {
    15, 5, // Number of jobs and machines
    0, 47, 4, 57, 1, 71, 3, 96, 2, 14, 
    0, 75, 1, 60, 4, 22, 3, 79, 2, 65, 
    3, 32, 0, 33, 2, 69, 1, 31, 4, 58, 
    0, 44, 1, 34, 4, 51, 3, 58, 2, 47, 
    3, 29, 1, 44, 0, 62, 2, 17, 4, 8, 
    1, 15, 2, 40, 0, 97, 4, 38, 3, 66, 
    2, 58, 1, 39, 0, 57, 4, 20, 3, 50, 
    2, 57, 3, 32, 4, 87, 0, 63, 1, 21, 
    4, 56, 0, 84, 2, 90, 1, 85, 3, 61, 
    4, 15, 0, 20, 1, 67, 3, 30, 2, 70, 
    4, 84, 0, 82, 1, 23, 2, 45, 3, 38, 
    3, 50, 2, 21, 0, 18, 4, 41, 1, 29, 
    4, 16, 1, 52, 0, 52, 2, 38, 3, 54, 
    4, 37, 0, 54, 3, 57, 2, 74, 1, 62, 
    4, 57, 1, 61, 0, 81, 2, 30, 3, 68
  };
  // Lawrence 15x5 instance (Table 4, instance 3); also called (setg3) or (G3)
  const int la08[] = {
    15, 5, // Number of jobs and machines
    3, 92, 2, 94, 0, 12, 4, 91, 1, 7, 
    2, 21, 1, 19, 0, 87, 3, 11, 4, 66, 
    1, 14, 3, 13, 0, 75, 4, 16, 2, 20, 
    2, 95, 4, 66, 0, 7, 1, 77, 3, 7, 
    2, 34, 4, 89, 3, 6, 1, 45, 0, 15, 
    4, 88, 3, 77, 2, 20, 1, 53, 0, 76, 
    4, 9, 3, 27, 0, 52, 1, 88, 2, 74, 
    3, 69, 2, 52, 0, 62, 1, 88, 4, 98, 
    3, 90, 0, 62, 4, 9, 2, 61, 1, 52, 
    4, 5, 2, 54, 3, 59, 0, 88, 1, 15, 
    0, 41, 1, 50, 4, 78, 3, 53, 2, 23, 
    0, 38, 4, 72, 2, 91, 3, 68, 1, 71, 
    0, 45, 3, 95, 4, 52, 2, 25, 1, 6, 
    3, 30, 1, 66, 0, 23, 4, 36, 2, 17, 
    2, 95, 0, 71, 3, 76, 1, 8, 4, 88
  };
  // Lawrence 15x5 instance (Table 4, instance 4); also called (setg4) or (G4)
  const int la09[] = {
    15, 5, // Number of jobs and machines
    1, 66, 3, 85, 2, 84, 0, 62, 4, 19, 
    3, 59, 1, 64, 2, 46, 4, 13, 0, 25, 
    4, 88, 3, 80, 1, 73, 2, 53, 0, 41, 
    0, 14, 1, 67, 2, 57, 3, 74, 4, 47, 
    0, 84, 4, 64, 2, 41, 3, 84, 1, 78, 
    0, 63, 3, 28, 1, 46, 2, 26, 4, 52, 
    3, 10, 2, 17, 4, 73, 1, 11, 0, 64, 
    2, 67, 1, 97, 3, 95, 4, 38, 0, 85, 
    2, 95, 4, 46, 0, 59, 1, 65, 3, 93, 
    2, 43, 4, 85, 3, 32, 1, 85, 0, 60, 
    4, 49, 3, 41, 2, 61, 0, 66, 1, 90, 
    1, 17, 0, 23, 3, 70, 4, 99, 2, 49, 
    4, 40, 3, 73, 0, 73, 1, 98, 2, 68, 
    3, 57, 1, 9, 2, 7, 0, 13, 4, 98, 
    0, 37, 1, 85, 2, 17, 4, 79, 3, 41
  };
  // Lawrence 15x5 instance (Table 4, instance 5); also called (setg5) or (G5)
  const int la10[] = {
    15, 5, // Number of jobs and machines
    1, 58, 2, 44, 3, 5, 0, 9, 4, 58, 
    1, 89, 0, 97, 4, 96, 3, 77, 2, 84, 
    0, 77, 1, 87, 2, 81, 4, 39, 3, 85, 
    3, 57, 1, 21, 2, 31, 0, 15, 4, 73, 
    2, 48, 0, 40, 1, 49, 3, 70, 4, 71, 
    3, 34, 4, 82, 2, 80, 0, 10, 1, 22, 
    1, 91, 4, 75, 0, 55, 2, 17, 3, 7, 
    2, 62, 3, 47, 1, 72, 4, 35, 0, 11, 
    0, 64, 3, 75, 4, 50, 1, 90, 2, 94, 
    2, 67, 4, 20, 3, 15, 0, 12, 1, 71, 
    0, 52, 4, 93, 3, 68, 2, 29, 1, 57, 
    2, 70, 0, 58, 1, 93, 4, 7, 3, 77, 
    3, 27, 2, 82, 1, 63, 4, 6, 0, 95, 
    1, 87, 2, 56, 4, 36, 0, 26, 3, 48, 
    3, 76, 2, 36, 0, 36, 4, 15, 1, 8
  };
  // Lawrence 20x5 instance (Table 5, instance 1); also called (seth1) or H1
  const int la11[] = {
    20, 5, // Number of jobs and machines
    2, 34, 1, 21, 0, 53, 3, 55, 4, 95, 
    0, 21, 3, 52, 1, 71, 4, 16, 2, 26, 
    0, 12, 1, 42, 2, 31, 4, 98, 3, 39, 
    2, 66, 3, 77, 4, 79, 0, 55, 1, 77, 
    0, 83, 4, 37, 3, 34, 1, 19, 2, 64, 
    4, 79, 2, 43, 0, 92, 3, 62, 1, 54, 
    0, 93, 4, 77, 2, 87, 1, 87, 3, 69, 
    4, 83, 3, 24, 1, 41, 2, 38, 0, 60, 
    4, 25, 1, 49, 0, 44, 2, 98, 3, 17, 
    0, 96, 1, 75, 2, 43, 4, 77, 3, 79, 
    0, 95, 3, 76, 1, 7, 4, 28, 2, 35, 
    4, 10, 2, 95, 0, 61, 1, 9, 3, 35, 
    1, 91, 2, 59, 4, 59, 0, 46, 3, 16, 
    2, 27, 1, 52, 4, 43, 0, 28, 3, 50, 
    4, 9, 0, 87, 3, 41, 2, 39, 1, 45, 
    1, 54, 0, 20, 4, 43, 3, 14, 2, 71, 
    4, 33, 1, 28, 3, 26, 0, 78, 2, 37, 
    1, 89, 0, 33, 2, 8, 3, 66, 4, 42, 
    4, 84, 0, 69, 2, 94, 1, 74, 3, 27, 
    4, 81, 2, 45, 1, 78, 3, 69, 0, 96
  };
  // Lawrence 20x5 instance (Table 5, instance 2); also called (seth2) or H2
  const int la12[] = {
    20, 5, // Number of jobs and machines
    1, 23, 0, 82, 4, 84, 2, 45, 3, 38, 
    3, 50, 4, 41, 1, 29, 0, 18, 2, 21, 
    4, 16, 3, 54, 1, 52, 2, 38, 0, 52, 
    1, 62, 3, 57, 4, 37, 2, 74, 0, 54, 
    3, 68, 1, 61, 2, 30, 0, 81, 4, 57, 
    1, 89, 2, 89, 3, 11, 0, 79, 4, 81, 
    1, 66, 0, 91, 3, 33, 4, 20, 2, 20, 
    3, 8, 4, 24, 2, 55, 0, 32, 1, 84, 
    0, 7, 2, 64, 1, 39, 4, 56, 3, 54, 
    0, 19, 4, 40, 3, 7, 2, 8, 1, 83, 
    0, 63, 2, 64, 3, 91, 4, 40, 1, 6, 
    1, 42, 3, 61, 4, 15, 2, 98, 0, 74, 
    1, 80, 0, 26, 3, 75, 4, 6, 2, 87, 
    2, 39, 4, 22, 0, 75, 3, 24, 1, 44, 
    1, 15, 3, 79, 4, 8, 0, 12, 2, 20, 
    3, 26, 2, 43, 0, 80, 4, 22, 1, 61, 
    2, 62, 1, 36, 0, 63, 3, 96, 4, 40, 
    1, 33, 3, 18, 0, 22, 4, 5, 2, 10, 
    2, 64, 4, 64, 0, 89, 1, 96, 3, 95, 
    2, 18, 4, 23, 3, 15, 1, 38, 0, 8
  };
  // Lawrence 20x5 instance (Table 5, instance 3); also called (seth3) or (H3)
  const int la13[] = {
    20, 5, // Number of jobs and machines
    3, 60, 0, 87, 1, 72, 4, 95, 2, 66, 
    1, 54, 0, 48, 2, 39, 3, 35, 4, 5, 
    3, 20, 1, 46, 0, 97, 2, 21, 4, 55, 
    2, 37, 0, 59, 3, 19, 1, 34, 4, 46, 
    2, 73, 3, 25, 1, 24, 0, 28, 4, 23, 
    1, 78, 3, 28, 2, 83, 0, 45, 4, 5, 
    3, 71, 1, 37, 2, 12, 4, 29, 0, 53, 
    4, 12, 3, 33, 1, 55, 2, 87, 0, 38, 
    0, 48, 1, 40, 2, 49, 3, 83, 4, 7, 
    0, 90, 4, 27, 2, 65, 3, 17, 1, 23, 
    0, 62, 3, 85, 1, 66, 2, 84, 4, 19, 
    3, 59, 2, 46, 4, 13, 1, 64, 0, 25, 
    2, 53, 1, 73, 3, 80, 4, 88, 0, 41, 
    2, 57, 4, 47, 0, 14, 1, 67, 3, 74, 
    2, 41, 4, 64, 3, 84, 1, 78, 0, 84, 
    4, 52, 3, 28, 2, 26, 0, 63, 1, 46, 
    1, 11, 0, 64, 3, 10, 4, 73, 2, 17, 
    4, 38, 3, 95, 0, 85, 1, 97, 2, 67, 
    3, 93, 1, 65, 2, 95, 0, 59, 4, 46, 
    0, 60, 1, 85, 2, 43, 4, 85, 3, 32
  };
  // Lawrence 20x5 instance (Table 5, instance 4); also called (seth4) or (H4)
  const int la14[] = {
    20, 5, // Number of jobs and machines
    3, 5, 4, 58, 2, 44, 0, 9, 1, 58, 
    1, 89, 4, 96, 0, 97, 2, 84, 3, 77, 
    2, 81, 3, 85, 1, 87, 4, 39, 0, 77, 
    0, 15, 3, 57, 4, 73, 1, 21, 2, 31, 
    2, 48, 4, 71, 3, 70, 0, 40, 1, 49, 
    0, 10, 4, 82, 3, 34, 2, 80, 1, 22, 
    2, 17, 0, 55, 1, 91, 4, 75, 3, 7, 
    3, 47, 2, 62, 1, 72, 4, 35, 0, 11, 
    1, 90, 2, 94, 4, 50, 0, 64, 3, 75, 
    3, 15, 2, 67, 0, 12, 4, 20, 1, 71, 
    4, 93, 2, 29, 0, 52, 1, 57, 3, 68, 
    3, 77, 1, 93, 0, 58, 2, 70, 4, 7, 
    1, 63, 3, 27, 0, 95, 4, 6, 2, 82, 
    4, 36, 0, 26, 3, 48, 2, 56, 1, 87, 
    2, 36, 1, 8, 4, 15, 3, 76, 0, 36, 
    4, 78, 1, 84, 3, 41, 0, 30, 2, 76, 
    1, 78, 0, 75, 4, 88, 3, 13, 2, 81, 
    0, 54, 4, 40, 2, 13, 1, 82, 3, 29, 
    1, 26, 4, 82, 0, 52, 3, 6, 2, 6, 
    3, 54, 1, 64, 0, 54, 2, 32, 4, 88
  };
  // Lawrence 20x5 instance (Table 5, instance 5); also called (seth5) or (H5)
  const int la15[] = {
    20, 5, // Number of jobs and machines
    0, 6, 2, 40, 1, 81, 3, 37, 4, 19, 
    2, 40, 3, 32, 0, 55, 4, 81, 1, 9, 
    1, 46, 4, 65, 2, 70, 3, 55, 0, 77, 
    2, 21, 4, 65, 0, 64, 3, 25, 1, 15, 
    2, 85, 0, 40, 1, 44, 3, 24, 4, 37, 
    0, 89, 4, 29, 1, 83, 3, 31, 2, 84, 
    4, 59, 3, 38, 1, 80, 2, 30, 0, 8, 
    0, 80, 2, 56, 1, 77, 4, 41, 3, 97, 
    4, 56, 0, 91, 3, 50, 2, 71, 1, 17, 
    1, 40, 0, 88, 4, 59, 2, 7, 3, 80, 
    0, 45, 1, 29, 2, 8, 4, 77, 3, 58, 
    2, 36, 0, 54, 3, 96, 1, 9, 4, 10, 
    0, 28, 2, 73, 1, 98, 3, 92, 4, 87, 
    0, 70, 3, 86, 2, 27, 1, 99, 4, 96, 
    1, 95, 0, 59, 4, 56, 3, 85, 2, 41, 
    1, 81, 2, 92, 4, 32, 0, 52, 3, 39, 
    1, 7, 4, 22, 2, 12, 0, 88, 3, 60, 
    3, 45, 0, 93, 2, 69, 4, 49, 1, 27, 
    0, 21, 1, 84, 2, 61, 3, 68, 4, 26, 
    1, 82, 2, 33, 4, 71, 0, 99, 3, 44
  };
  // Lawrence 10x10 instance (Table 6, instance 1); also called (seta1) or (A1)
  const int la16[] = {
    10, 10, // Number of jobs and machines
    1, 21, 6, 71, 9, 16, 8, 52, 7, 26, 2, 34, 0, 53, 4, 21, 3, 55, 5, 95, 
    4, 55, 2, 31, 5, 98, 9, 79, 0, 12, 7, 66, 1, 42, 8, 77, 6, 77, 3, 39, 
    3, 34, 2, 64, 8, 62, 1, 19, 4, 92, 9, 79, 7, 43, 6, 54, 0, 83, 5, 37, 
    1, 87, 3, 69, 2, 87, 7, 38, 8, 24, 9, 83, 6, 41, 0, 93, 5, 77, 4, 60, 
    2, 98, 0, 44, 5, 25, 6, 75, 7, 43, 1, 49, 4, 96, 9, 77, 3, 17, 8, 79, 
    2, 35, 3, 76, 5, 28, 9, 10, 4, 61, 6, 9, 0, 95, 8, 35, 1, 7, 7, 95, 
    3, 16, 2, 59, 0, 46, 1, 91, 9, 43, 8, 50, 6, 52, 5, 59, 4, 28, 7, 27, 
    1, 45, 0, 87, 3, 41, 4, 20, 6, 54, 9, 43, 8, 14, 5, 9, 2, 39, 7, 71, 
    4, 33, 2, 37, 8, 66, 5, 33, 3, 26, 7, 8, 1, 28, 6, 89, 9, 42, 0, 78, 
    8, 69, 9, 81, 2, 94, 4, 96, 3, 27, 0, 69, 7, 45, 6, 78, 1, 74, 5, 84
  };
  // Lawrence 10x10 instance (Table 6, instance 2); also called (seta2) or (A2)
  const int la17[] = {
    10, 10, // Number of jobs and machines
    4, 18, 7, 21, 9, 41, 2, 45, 3, 38, 8, 50, 5, 84, 6, 29, 1, 23, 0, 82, 
    8, 57, 5, 16, 1, 52, 7, 74, 2, 38, 3, 54, 6, 62, 9, 37, 4, 54, 0, 52, 
    2, 30, 4, 79, 3, 68, 1, 61, 8, 11, 6, 89, 7, 89, 0, 81, 9, 81, 5, 57, 
    0, 91, 8, 8, 3, 33, 7, 55, 5, 20, 2, 20, 4, 32, 6, 84, 1, 66, 9, 24, 
    9, 40, 0, 7, 4, 19, 8, 7, 6, 83, 2, 64, 5, 56, 3, 54, 7, 8, 1, 39, 
    3, 91, 2, 64, 5, 40, 0, 63, 7, 98, 4, 74, 8, 61, 1, 6, 6, 42, 9, 15, 
    1, 80, 7, 39, 8, 24, 3, 75, 4, 75, 5, 6, 6, 44, 0, 26, 2, 87, 9, 22, 
    1, 15, 7, 43, 2, 20, 0, 12, 8, 26, 6, 61, 3, 79, 9, 22, 5, 8, 4, 80, 
    2, 62, 3, 96, 4, 22, 9, 5, 0, 63, 6, 33, 7, 10, 8, 18, 1, 36, 5, 40, 
    1, 96, 0, 89, 5, 64, 3, 95, 9, 23, 7, 18, 8, 15, 2, 64, 6, 38, 4, 8
  };
  // Lawrence 10x10 instance (Table 6, instance 3); also called (seta3) or (A3)
  const int la18[] = {
    10, 10, // Number of jobs and machines
    6, 54, 0, 87, 4, 48, 3, 60, 7, 39, 8, 35, 1, 72, 5, 95, 2, 66, 9, 5, 
    3, 20, 9, 46, 6, 34, 5, 55, 0, 97, 8, 19, 4, 59, 2, 21, 7, 37, 1, 46, 
    4, 45, 1, 24, 8, 28, 0, 28, 7, 83, 6, 78, 5, 23, 3, 25, 9, 5, 2, 73, 
    9, 12, 1, 37, 4, 38, 3, 71, 8, 33, 2, 12, 6, 55, 0, 53, 7, 87, 5, 29, 
    3, 83, 2, 49, 6, 23, 9, 27, 7, 65, 0, 48, 4, 90, 5, 7, 1, 40, 8, 17, 
    1, 66, 4, 25, 0, 62, 2, 84, 9, 13, 6, 64, 7, 46, 8, 59, 5, 19, 3, 85, 
    1, 73, 3, 80, 0, 41, 2, 53, 9, 47, 7, 57, 8, 74, 4, 14, 6, 67, 5, 88, 
    5, 64, 3, 84, 6, 46, 1, 78, 0, 84, 7, 26, 8, 28, 9, 52, 2, 41, 4, 63, 
    1, 11, 0, 64, 7, 67, 4, 85, 3, 10, 5, 73, 9, 38, 8, 95, 6, 97, 2, 17, 
    4, 60, 8, 32, 2, 95, 3, 93, 1, 65, 6, 85, 7, 43, 9, 85, 5, 46, 0, 59
  };
  // Lawrence 10x10 instance (Table 6, instance 4); also called (seta4) or (A4)
  const int la19[] = {
    10, 10, // Number of jobs and machines
    2, 44, 3, 5, 5, 58, 4, 97, 0, 9, 7, 84, 8, 77, 9, 96, 1, 58, 6, 89, 
    4, 15, 7, 31, 1, 87, 8, 57, 0, 77, 3, 85, 2, 81, 5, 39, 9, 73, 6, 21, 
    9, 82, 6, 22, 4, 10, 3, 70, 1, 49, 0, 40, 8, 34, 2, 48, 7, 80, 5, 71, 
    1, 91, 2, 17, 7, 62, 5, 75, 8, 47, 4, 11, 3, 7, 6, 72, 9, 35, 0, 55, 
    6, 71, 1, 90, 3, 75, 0, 64, 2, 94, 8, 15, 4, 12, 7, 67, 9, 20, 5, 50, 
    7, 70, 5, 93, 8, 77, 2, 29, 4, 58, 6, 93, 3, 68, 1, 57, 9, 7, 0, 52, 
    6, 87, 1, 63, 4, 26, 5, 6, 2, 82, 3, 27, 7, 56, 8, 48, 9, 36, 0, 95, 
    0, 36, 5, 15, 8, 41, 9, 78, 3, 76, 6, 84, 4, 30, 7, 76, 2, 36, 1, 8, 
    5, 88, 2, 81, 3, 13, 6, 82, 4, 54, 7, 13, 8, 29, 9, 40, 1, 78, 0, 75, 
    9, 88, 4, 54, 6, 64, 7, 32, 0, 52, 2, 6, 8, 54, 5, 82, 3, 6, 1, 26
  };
  // Lawrence 10x10 instance (Table 6, instance 5); also called (seta5) or (A5)
  const int la20[] = {
    10, 10, // Number of jobs and machines
    6, 9, 1, 81, 4, 55, 2, 40, 8, 32, 3, 37, 0, 6, 5, 19, 9, 81, 7, 40, 
    7, 21, 2, 70, 9, 65, 4, 64, 1, 46, 5, 65, 8, 25, 0, 77, 3, 55, 6, 15, 
    2, 85, 5, 37, 0, 40, 3, 24, 1, 44, 6, 83, 4, 89, 8, 31, 7, 84, 9, 29, 
    4, 80, 6, 77, 7, 56, 0, 8, 2, 30, 5, 59, 3, 38, 1, 80, 9, 41, 8, 97, 
    0, 91, 6, 40, 4, 88, 1, 17, 2, 71, 3, 50, 9, 59, 8, 80, 5, 56, 7, 7, 
    2, 8, 6, 9, 3, 58, 5, 77, 1, 29, 8, 96, 0, 45, 9, 10, 4, 54, 7, 36, 
    4, 70, 3, 92, 1, 98, 5, 87, 6, 99, 7, 27, 8, 86, 9, 96, 0, 28, 2, 73, 
    1, 95, 7, 92, 3, 85, 4, 52, 6, 81, 9, 32, 8, 39, 0, 59, 2, 41, 5, 56, 
    3, 60, 8, 45, 0, 88, 2, 12, 1, 7, 5, 22, 4, 93, 9, 49, 7, 69, 6, 27, 
    0, 21, 2, 61, 3, 68, 5, 26, 6, 82, 9, 71, 8, 44, 4, 99, 7, 33, 1, 84
  };
  // Lawrence 15x10 instance (Table 7, instance 1); also called (setb1) or (B1)
  const int la21[] = {
    15, 10, // Number of jobs and machines
    2, 34, 3, 55, 5, 95, 9, 16, 4, 21, 6, 71, 0, 53, 8, 52, 1, 21, 7, 26, 
    3, 39, 2, 31, 0, 12, 1, 42, 9, 79, 8, 77, 6, 77, 5, 98, 4, 55, 7, 66, 
    1, 19, 0, 83, 3, 34, 4, 92, 6, 54, 9, 79, 8, 62, 5, 37, 2, 64, 7, 43, 
    4, 60, 2, 87, 8, 24, 5, 77, 3, 69, 7, 38, 1, 87, 6, 41, 9, 83, 0, 93, 
    8, 79, 9, 77, 2, 98, 4, 96, 3, 17, 0, 44, 7, 43, 6, 75, 1, 49, 5, 25, 
    8, 35, 7, 95, 6, 9, 9, 10, 2, 35, 1, 7, 5, 28, 4, 61, 0, 95, 3, 76, 
    4, 28, 5, 59, 3, 16, 9, 43, 0, 46, 8, 50, 6, 52, 7, 27, 2, 59, 1, 91, 
    5, 9, 4, 20, 2, 39, 6, 54, 1, 45, 7, 71, 0, 87, 3, 41, 9, 43, 8, 14, 
    1, 28, 5, 33, 0, 78, 3, 26, 2, 37, 7, 8, 8, 66, 6, 89, 9, 42, 4, 33, 
    2, 94, 5, 84, 6, 78, 9, 81, 1, 74, 3, 27, 8, 69, 0, 69, 7, 45, 4, 96, 
    1, 31, 4, 24, 0, 20, 2, 17, 9, 25, 8, 81, 5, 76, 3, 87, 7, 32, 6, 18, 
    5, 28, 9, 97, 0, 58, 4, 45, 6, 76, 3, 99, 2, 23, 1, 72, 8, 90, 7, 86, 
    5, 27, 9, 48, 8, 27, 7, 62, 4, 98, 6, 67, 3, 48, 0, 42, 1, 46, 2, 17, 
    1, 12, 8, 50, 0, 80, 2, 50, 9, 80, 3, 19, 5, 28, 6, 63, 4, 94, 7, 98, 
    4, 61, 3, 55, 6, 37, 5, 14, 2, 50, 8, 79, 1, 41, 9, 72, 7, 18, 0, 75
  };
  // Lawrence 15x10 instance (Table 7, instance 2); also called (setb2) or (B2)
  const int la22[] = {
    15, 10, // Number of jobs and machines
    9, 66, 5, 91, 4, 87, 2, 94, 7, 21, 3, 92, 1, 7, 0, 12, 8, 11, 6, 19, 
    3, 13, 2, 20, 4, 7, 1, 14, 9, 66, 0, 75, 6, 77, 5, 16, 7, 95, 8, 7, 
    8, 77, 7, 20, 2, 34, 0, 15, 9, 88, 5, 89, 6, 53, 3, 6, 1, 45, 4, 76, 
    3, 27, 2, 74, 6, 88, 4, 62, 7, 52, 8, 69, 5, 9, 9, 98, 0, 52, 1, 88, 
    4, 88, 6, 15, 1, 52, 2, 61, 7, 54, 0, 62, 8, 59, 5, 9, 3, 90, 9, 5, 
    6, 71, 0, 41, 4, 38, 3, 53, 7, 91, 8, 68, 1, 50, 5, 78, 2, 23, 9, 72, 
    3, 95, 9, 36, 6, 66, 5, 52, 0, 45, 8, 30, 4, 23, 2, 25, 7, 17, 1, 6, 
    4, 65, 1, 8, 8, 85, 0, 71, 7, 65, 6, 28, 5, 88, 3, 76, 9, 27, 2, 95, 
    9, 37, 1, 37, 4, 28, 3, 51, 8, 86, 2, 9, 6, 55, 0, 73, 7, 51, 5, 90, 
    3, 39, 2, 15, 6, 83, 9, 44, 7, 53, 0, 16, 4, 46, 5, 24, 1, 25, 8, 82, 
    1, 72, 4, 48, 0, 87, 2, 66, 9, 5, 6, 54, 7, 39, 8, 35, 5, 95, 3, 60, 
    1, 46, 3, 20, 0, 97, 2, 21, 9, 46, 7, 37, 8, 19, 4, 59, 6, 34, 5, 55, 
    5, 23, 3, 25, 6, 78, 1, 24, 0, 28, 7, 83, 8, 28, 9, 5, 2, 73, 4, 45, 
    1, 37, 0, 53, 7, 87, 4, 38, 3, 71, 5, 29, 9, 12, 8, 33, 6, 55, 2, 12, 
    4, 90, 8, 17, 2, 49, 3, 83, 1, 40, 6, 23, 7, 65, 9, 27, 5, 7, 0, 48
  };
  // Lawrence 15x10 instance (Table 7, instance 3); also called (setb3) or (B3)
  const int la23[] = {
    15, 10, // Number of jobs and machines
    7, 84, 5, 58, 8, 77, 2, 44, 4, 97, 6, 89, 3, 5, 1, 58, 9, 96, 0, 9, 
    6, 21, 1, 87, 4, 15, 5, 39, 2, 81, 3, 85, 7, 31, 8, 57, 9, 73, 0, 77, 
    0, 40, 5, 71, 8, 34, 9, 82, 3, 70, 6, 22, 4, 10, 7, 80, 2, 48, 1, 49, 
    5, 75, 2, 17, 3, 7, 6, 72, 4, 11, 7, 62, 8, 47, 9, 35, 1, 91, 0, 55, 
    9, 20, 4, 12, 6, 71, 7, 67, 0, 64, 2, 94, 8, 15, 5, 50, 3, 75, 1, 90, 
    6, 93, 5, 93, 1, 57, 7, 70, 8, 77, 4, 58, 0, 52, 2, 29, 9, 7, 3, 68, 
    7, 56, 0, 95, 8, 48, 4, 26, 2, 82, 1, 63, 9, 36, 3, 27, 6, 87, 5, 6, 
    3, 76, 5, 15, 9, 78, 1, 8, 8, 41, 2, 36, 4, 30, 6, 84, 0, 36, 7, 76, 
    0, 75, 7, 13, 2, 81, 8, 29, 4, 54, 6, 82, 5, 88, 1, 78, 9, 40, 3, 13, 
    2, 6, 1, 26, 7, 32, 6, 64, 4, 54, 0, 52, 5, 82, 3, 6, 9, 88, 8, 54, 
    8, 62, 2, 67, 5, 32, 0, 62, 7, 69, 3, 61, 1, 35, 4, 72, 9, 5, 6, 93, 
    2, 78, 9, 90, 0, 85, 1, 72, 8, 64, 6, 63, 3, 11, 7, 82, 5, 88, 4, 7, 
    4, 28, 9, 11, 7, 50, 6, 88, 0, 44, 5, 31, 2, 27, 1, 66, 8, 49, 3, 35, 
    2, 14, 5, 39, 6, 56, 4, 62, 3, 97, 9, 66, 7, 69, 1, 7, 8, 47, 0, 76, 
    1, 18, 8, 93, 7, 58, 6, 47, 3, 69, 9, 57, 2, 41, 5, 53, 4, 79, 0, 64
  };
  // Lawrence 15x10 instance (Table 7, instance 4); also called (setb4) or (B4)
  const int la24[] = {
    15, 10, // Number of jobs and machines
    7, 8, 9, 75, 0, 72, 6, 74, 4, 30, 8, 43, 2, 38, 5, 98, 1, 26, 3, 19, 
    6, 19, 8, 73, 3, 43, 0, 23, 1, 85, 4, 39, 5, 13, 9, 26, 2, 67, 7, 9, 
    1, 50, 3, 93, 5, 80, 4, 7, 0, 55, 2, 61, 6, 57, 8, 72, 9, 42, 7, 46, 
    1, 68, 7, 43, 4, 99, 6, 60, 5, 68, 0, 91, 8, 11, 3, 96, 9, 11, 2, 72, 
    7, 84, 2, 34, 8, 40, 5, 7, 1, 70, 6, 74, 3, 12, 0, 43, 9, 69, 4, 30, 
    8, 60, 0, 49, 4, 59, 5, 72, 9, 63, 1, 69, 7, 99, 6, 45, 3, 27, 2, 9, 
    6, 71, 2, 91, 8, 65, 1, 90, 9, 98, 4, 8, 7, 50, 0, 75, 5, 37, 3, 17, 
    8, 62, 7, 90, 5, 98, 3, 31, 2, 91, 4, 38, 9, 72, 1, 9, 0, 72, 6, 49, 
    4, 35, 0, 39, 9, 74, 5, 25, 7, 47, 3, 52, 2, 63, 8, 21, 6, 35, 1, 80, 
    9, 58, 0, 5, 3, 50, 8, 52, 1, 88, 6, 20, 2, 68, 5, 24, 4, 53, 7, 57, 
    7, 99, 3, 91, 4, 33, 5, 19, 2, 18, 6, 38, 0, 24, 9, 35, 1, 49, 8, 9, 
    0, 68, 3, 60, 2, 77, 7, 10, 8, 60, 5, 15, 9, 72, 1, 18, 6, 90, 4, 18, 
    9, 79, 1, 60, 3, 56, 6, 91, 2, 40, 8, 86, 7, 72, 0, 80, 5, 89, 4, 51, 
    4, 10, 2, 92, 5, 23, 6, 46, 8, 40, 7, 72, 3, 6, 1, 23, 0, 95, 9, 34, 
    2, 24, 5, 29, 9, 49, 8, 55, 0, 47, 6, 77, 3, 77, 7, 8, 1, 28, 4, 48
  };
  // Lawrence 15x10 instance (Table 7, instance 5); also called (setb5) or (B5)
  const int la25[] = {
    15, 10, // Number of jobs and machines
    8, 14, 4, 75, 3, 12, 2, 38, 0, 76, 5, 97, 9, 12, 1, 29, 7, 44, 6, 66, 
    5, 38, 3, 82, 2, 85, 4, 58, 6, 87, 9, 89, 0, 43, 1, 80, 7, 69, 8, 92, 
    9, 5, 1, 84, 0, 43, 6, 48, 4, 8, 7, 7, 3, 41, 5, 61, 8, 66, 2, 14, 
    2, 42, 1, 8, 0, 96, 5, 19, 4, 59, 7, 97, 9, 73, 8, 43, 3, 74, 6, 41, 
    6, 55, 2, 70, 3, 75, 8, 42, 4, 37, 7, 23, 1, 48, 5, 5, 9, 38, 0, 7, 
    8, 9, 2, 72, 7, 31, 0, 79, 5, 73, 3, 95, 4, 25, 6, 43, 9, 60, 1, 56, 
    0, 97, 2, 64, 3, 78, 5, 21, 4, 94, 9, 31, 8, 53, 6, 16, 7, 86, 1, 7, 
    3, 86, 7, 85, 9, 63, 0, 61, 2, 65, 4, 30, 5, 32, 1, 33, 8, 44, 6, 59, 
    2, 44, 3, 16, 4, 11, 6, 45, 1, 30, 9, 84, 8, 93, 0, 60, 5, 61, 7, 90, 
    7, 36, 8, 31, 4, 47, 6, 52, 0, 32, 5, 11, 2, 28, 9, 35, 3, 20, 1, 49, 
    8, 20, 6, 49, 7, 74, 4, 10, 5, 17, 3, 34, 0, 85, 2, 77, 9, 68, 1, 84, 
    1, 85, 5, 7, 8, 71, 6, 59, 4, 76, 0, 17, 3, 29, 2, 17, 7, 48, 9, 13, 
    2, 15, 6, 87, 7, 11, 1, 39, 4, 39, 8, 43, 0, 19, 3, 32, 9, 16, 5, 64, 
    6, 32, 2, 92, 5, 33, 8, 82, 1, 83, 7, 57, 9, 99, 4, 91, 3, 99, 0, 8, 
    4, 88, 7, 7, 8, 27, 1, 38, 3, 91, 2, 69, 6, 21, 9, 62, 5, 39, 0, 48
  };
  // Lawrence 20x10 instance (Table 8, instance 1); also called (setc1) or (C1)
  const int la26[] = {
    20, 10, // Number of jobs and machines
    8, 52, 7, 26, 6, 71, 9, 16, 2, 34, 1, 21, 5, 95, 4, 21, 0, 53, 3, 55, 
    4, 55, 5, 98, 3, 39, 9, 79, 0, 12, 8, 77, 6, 77, 7, 66, 2, 31, 1, 42, 
    5, 37, 4, 92, 2, 64, 6, 54, 1, 19, 7, 43, 0, 83, 3, 34, 9, 79, 8, 62, 
    1, 87, 5, 77, 0, 93, 3, 69, 2, 87, 7, 38, 8, 24, 6, 41, 9, 83, 4, 60, 
    2, 98, 5, 25, 6, 75, 9, 77, 1, 49, 3, 17, 8, 79, 0, 44, 7, 43, 4, 96, 
    1, 7, 4, 61, 0, 95, 2, 35, 9, 10, 8, 35, 5, 28, 3, 76, 7, 95, 6, 9, 
    5, 59, 9, 43, 0, 46, 4, 28, 6, 52, 3, 16, 2, 59, 1, 91, 8, 50, 7, 27, 
    5, 9, 9, 43, 8, 14, 7, 71, 4, 20, 6, 54, 3, 41, 0, 87, 1, 45, 2, 39, 
    1, 28, 8, 66, 0, 78, 2, 37, 9, 42, 3, 26, 5, 33, 6, 89, 4, 33, 7, 8, 
    4, 96, 3, 27, 6, 78, 5, 84, 2, 94, 8, 69, 1, 74, 9, 81, 7, 45, 0, 69, 
    4, 24, 7, 32, 9, 25, 2, 17, 3, 87, 8, 81, 5, 76, 6, 18, 1, 31, 0, 20, 
    8, 90, 5, 28, 1, 72, 7, 86, 2, 23, 3, 99, 6, 76, 9, 97, 4, 45, 0, 58, 
    2, 17, 4, 98, 3, 48, 1, 46, 8, 27, 6, 67, 7, 62, 0, 42, 9, 48, 5, 27, 
    0, 80, 8, 50, 3, 19, 7, 98, 5, 28, 2, 50, 4, 94, 6, 63, 1, 12, 9, 80, 
    9, 72, 0, 75, 4, 61, 8, 79, 6, 37, 2, 50, 5, 14, 3, 55, 7, 18, 1, 41, 
    3, 96, 2, 14, 5, 57, 0, 47, 7, 65, 4, 75, 8, 79, 1, 71, 6, 60, 9, 22, 
    1, 31, 7, 47, 8, 58, 3, 32, 4, 44, 5, 58, 6, 34, 0, 33, 2, 69, 9, 51, 
    1, 44, 7, 40, 2, 17, 0, 62, 8, 66, 6, 15, 3, 29, 9, 38, 5, 8, 4, 97, 
    2, 58, 3, 50, 4, 63, 9, 87, 0, 57, 6, 21, 7, 57, 8, 32, 1, 39, 5, 20, 
    1, 85, 0, 84, 5, 56, 3, 61, 9, 15, 7, 70, 8, 30, 2, 90, 6, 67, 4, 20
  };
  // Lawrence 20x10 instance (Table 8, instance 2); also called (setc2) or (C2)
  const int la27[] = {
    20, 10, // Number of jobs and machines
    3, 60, 4, 48, 5, 95, 0, 87, 1, 72, 9, 5, 8, 35, 7, 39, 6, 54, 2, 66, 
    7, 37, 6, 34, 0, 97, 5, 55, 2, 21, 3, 20, 4, 59, 9, 46, 8, 19, 1, 46, 
    4, 45, 2, 73, 1, 24, 8, 28, 0, 28, 3, 25, 5, 23, 7, 83, 9, 5, 6, 78, 
    0, 53, 2, 12, 9, 12, 1, 37, 8, 33, 3, 71, 6, 55, 5, 29, 7, 87, 4, 38, 
    4, 90, 2, 49, 9, 27, 7, 65, 5, 7, 6, 23, 0, 48, 3, 83, 8, 17, 1, 40, 
    3, 85, 4, 25, 2, 84, 6, 64, 9, 13, 1, 66, 7, 46, 8, 59, 0, 62, 5, 19, 
    5, 88, 6, 67, 4, 14, 0, 41, 1, 73, 7, 57, 2, 53, 3, 80, 9, 47, 8, 74, 
    1, 78, 5, 64, 4, 63, 6, 46, 3, 84, 0, 84, 8, 28, 9, 52, 7, 26, 2, 41, 
    1, 11, 0, 64, 6, 97, 9, 38, 2, 17, 4, 85, 5, 73, 3, 10, 8, 95, 7, 67, 
    3, 93, 2, 95, 7, 43, 1, 65, 8, 32, 0, 59, 6, 85, 5, 46, 9, 85, 4, 60, 
    2, 61, 3, 41, 5, 49, 4, 23, 0, 66, 7, 49, 8, 70, 9, 99, 1, 90, 6, 17, 
    4, 13, 7, 7, 1, 98, 8, 57, 0, 73, 3, 73, 2, 68, 5, 40, 9, 98, 6, 9, 
    9, 86, 6, 76, 4, 14, 3, 41, 1, 85, 0, 37, 8, 19, 2, 17, 7, 54, 5, 79, 
    1, 40, 2, 53, 7, 97, 5, 87, 8, 96, 4, 84, 3, 16, 6, 66, 9, 52, 0, 95, 
    6, 33, 1, 33, 3, 87, 0, 18, 2, 55, 8, 13, 4, 77, 7, 60, 9, 42, 5, 74, 
    7, 92, 5, 91, 8, 79, 2, 54, 4, 69, 6, 79, 3, 33, 1, 61, 9, 39, 0, 16, 
    6, 82, 1, 41, 4, 28, 5, 64, 2, 78, 3, 76, 7, 6, 8, 49, 9, 47, 0, 58, 
    0, 52, 5, 42, 8, 24, 9, 91, 3, 47, 6, 88, 4, 91, 7, 52, 2, 28, 1, 35, 
    5, 82, 2, 76, 3, 86, 6, 93, 4, 84, 7, 38, 8, 95, 9, 37, 1, 21, 0, 23, 
    9, 77, 4, 8, 6, 42, 7, 64, 0, 70, 2, 45, 8, 45, 5, 28, 3, 67, 1, 86
  };
  // Lawrence 20x10 instance (Table 8, instance 3); also called (setc3) or (C3)
  const int la28[] = {
    20, 10, // Number of jobs and machines
    8, 32, 1, 81, 4, 55, 7, 40, 0, 6, 5, 19, 9, 81, 3, 37, 2, 40, 6, 9, 
    2, 70, 3, 55, 7, 21, 4, 64, 1, 46, 8, 25, 9, 65, 0, 77, 5, 65, 6, 15, 
    7, 84, 4, 89, 3, 24, 1, 44, 2, 85, 8, 31, 9, 29, 6, 83, 5, 37, 0, 40, 
    4, 80, 5, 59, 0, 8, 2, 30, 6, 77, 3, 38, 1, 80, 7, 56, 9, 41, 8, 97, 
    6, 40, 2, 71, 0, 91, 7, 7, 9, 59, 8, 80, 3, 50, 5, 56, 1, 17, 4, 88, 
    7, 36, 9, 10, 0, 45, 6, 9, 4, 54, 8, 96, 2, 8, 5, 77, 1, 29, 3, 58, 
    6, 99, 8, 86, 3, 92, 0, 28, 1, 98, 4, 70, 5, 87, 9, 96, 2, 73, 7, 27, 
    1, 95, 3, 85, 5, 56, 4, 52, 0, 59, 2, 41, 6, 81, 8, 39, 9, 32, 7, 92, 
    1, 7, 7, 69, 4, 93, 6, 27, 5, 22, 0, 88, 8, 45, 3, 60, 9, 49, 2, 12, 
    7, 33, 2, 61, 8, 44, 5, 26, 1, 84, 6, 82, 3, 68, 0, 21, 9, 71, 4, 99, 
    8, 43, 0, 72, 4, 30, 5, 98, 9, 75, 1, 26, 7, 8, 6, 74, 3, 19, 2, 38, 
    6, 19, 2, 67, 8, 73, 1, 85, 9, 26, 4, 39, 7, 9, 0, 23, 5, 13, 3, 43, 
    8, 72, 7, 46, 5, 80, 3, 93, 2, 61, 4, 7, 9, 42, 1, 50, 0, 55, 6, 57, 
    4, 99, 0, 91, 9, 11, 5, 68, 7, 43, 3, 96, 2, 72, 8, 11, 6, 60, 1, 68, 
    9, 69, 0, 43, 3, 12, 8, 40, 1, 70, 6, 74, 2, 34, 5, 7, 4, 30, 7, 84, 
    7, 99, 3, 27, 4, 59, 5, 72, 2, 9, 6, 45, 0, 49, 9, 63, 1, 69, 8, 60, 
    0, 75, 3, 17, 2, 91, 7, 50, 8, 65, 5, 37, 9, 98, 1, 90, 6, 71, 4, 8, 
    9, 72, 1, 9, 3, 31, 6, 49, 2, 91, 8, 62, 7, 90, 0, 72, 5, 98, 4, 38, 
    4, 35, 2, 63, 5, 25, 6, 35, 8, 21, 7, 47, 3, 52, 1, 80, 0, 39, 9, 74, 
    2, 68, 5, 24, 9, 58, 8, 52, 0, 5, 6, 20, 3, 50, 7, 57, 1, 88, 4, 53
  };
  // Lawrence 20x10 instance (Table 8, instance 4); also called (setc4) or (C4)
  const int la29[] = {
    20, 10, // Number of jobs and machines
    8, 14, 2, 38, 7, 44, 0, 76, 5, 97, 3, 12, 4, 75, 6, 66, 9, 12, 1, 29, 
    0, 43, 2, 85, 3, 82, 5, 38, 4, 58, 9, 89, 8, 92, 6, 87, 7, 69, 1, 80, 
    3, 41, 7, 7, 9, 5, 0, 43, 2, 14, 4, 8, 5, 61, 1, 84, 8, 66, 6, 48, 
    2, 42, 3, 74, 4, 59, 6, 41, 1, 8, 9, 73, 8, 43, 0, 96, 5, 19, 7, 97, 
    7, 23, 8, 42, 4, 37, 6, 55, 0, 7, 5, 5, 2, 70, 9, 38, 3, 75, 1, 48, 
    8, 9, 6, 43, 7, 31, 4, 25, 5, 73, 3, 95, 0, 79, 2, 72, 9, 60, 1, 56, 
    1, 7, 5, 21, 8, 53, 6, 16, 4, 94, 0, 97, 3, 78, 2, 64, 7, 86, 9, 31, 
    2, 65, 6, 59, 7, 85, 1, 33, 4, 30, 8, 44, 0, 61, 3, 86, 9, 63, 5, 32, 
    6, 45, 2, 44, 5, 61, 8, 93, 1, 30, 7, 90, 9, 84, 4, 11, 3, 16, 0, 60, 
    4, 47, 7, 36, 8, 31, 1, 49, 3, 20, 2, 28, 6, 52, 9, 35, 5, 11, 0, 32, 
    2, 77, 4, 10, 9, 68, 5, 17, 0, 85, 1, 84, 8, 20, 6, 49, 7, 74, 3, 34, 
    0, 17, 5, 7, 1, 85, 3, 29, 2, 17, 4, 76, 6, 59, 8, 71, 9, 13, 7, 48, 
    6, 87, 4, 39, 8, 43, 7, 11, 2, 15, 3, 32, 5, 64, 0, 19, 1, 39, 9, 16, 
    5, 33, 3, 99, 6, 32, 4, 91, 8, 82, 2, 92, 9, 99, 7, 57, 1, 83, 0, 8, 
    3, 91, 5, 39, 2, 69, 8, 27, 7, 7, 6, 21, 1, 38, 9, 62, 4, 88, 0, 48, 
    2, 67, 7, 80, 3, 24, 0, 88, 4, 18, 1, 44, 8, 45, 9, 64, 5, 80, 6, 38, 
    9, 59, 3, 72, 6, 47, 4, 40, 7, 21, 5, 43, 0, 51, 8, 52, 1, 24, 2, 15, 
    3, 70, 2, 31, 6, 20, 8, 76, 1, 40, 7, 43, 0, 32, 5, 88, 9, 5, 4, 77, 
    4, 47, 5, 64, 9, 85, 3, 49, 7, 58, 1, 26, 0, 32, 8, 80, 2, 14, 6, 94, 
    5, 59, 2, 96, 0, 5, 7, 79, 8, 34, 4, 75, 3, 26, 6, 9, 9, 23, 1, 11
  };
  // Lawrence 20x10 instance (Table 8, instance 5); also called (setc5) or (C5)
  const int la30[] = {
    20, 10, // Number of jobs and machines
    6, 32, 3, 16, 1, 33, 8, 12, 7, 70, 4, 10, 9, 75, 0, 82, 5, 88, 2, 20, 
    8, 39, 4, 81, 3, 91, 5, 56, 9, 69, 1, 45, 6, 59, 0, 86, 2, 36, 7, 68, 
    3, 84, 2, 57, 7, 41, 5, 73, 4, 81, 0, 88, 8, 38, 9, 17, 6, 83, 1, 5, 
    4, 20, 5, 6, 2, 15, 8, 19, 1, 30, 0, 94, 6, 45, 7, 17, 3, 18, 9, 88, 
    9, 24, 6, 49, 5, 16, 4, 11, 3, 60, 7, 5, 8, 63, 1, 25, 2, 15, 0, 45, 
    1, 86, 8, 50, 2, 77, 6, 54, 9, 48, 0, 93, 3, 32, 7, 92, 5, 45, 4, 71, 
    5, 86, 6, 90, 3, 78, 9, 88, 2, 57, 0, 32, 7, 57, 8, 86, 4, 71, 1, 39, 
    2, 59, 3, 18, 9, 31, 4, 41, 7, 20, 5, 83, 8, 65, 0, 54, 6, 94, 1, 69, 
    3, 47, 4, 79, 6, 76, 0, 59, 1, 72, 2, 8, 9, 30, 5, 73, 7, 57, 8, 84, 
    0, 59, 2, 89, 4, 10, 7, 45, 3, 8, 5, 54, 6, 88, 8, 20, 9, 7, 1, 62, 
    5, 63, 6, 9, 4, 77, 3, 37, 2, 5, 8, 13, 9, 79, 1, 24, 7, 10, 0, 82, 
    0, 74, 1, 32, 2, 61, 7, 53, 4, 92, 9, 20, 8, 10, 3, 5, 6, 45, 5, 23, 
    2, 85, 9, 51, 0, 61, 5, 99, 4, 37, 6, 94, 1, 98, 8, 65, 3, 33, 7, 75, 
    0, 51, 3, 24, 5, 8, 6, 30, 7, 12, 8, 23, 2, 7, 4, 17, 9, 35, 1, 81, 
    1, 71, 5, 42, 8, 68, 2, 31, 6, 29, 3, 63, 4, 65, 9, 70, 7, 27, 0, 93, 
    1, 28, 5, 38, 4, 51, 7, 70, 2, 33, 8, 78, 9, 45, 3, 90, 6, 54, 0, 72, 
    0, 18, 2, 90, 4, 25, 6, 92, 8, 85, 5, 35, 7, 29, 1, 81, 9, 80, 3, 59, 
    5, 67, 2, 96, 1, 38, 4, 86, 0, 97, 3, 94, 7, 86, 6, 35, 9, 82, 8, 45, 
    2, 92, 8, 51, 4, 59, 6, 52, 5, 8, 9, 70, 1, 75, 3, 54, 7, 60, 0, 33, 
    3, 98, 7, 80, 5, 78, 0, 82, 2, 7, 9, 89, 1, 69, 4, 51, 8, 79, 6, 62
  };
  // Lawrence 30x10 instance (Table 9, instance 1); also called (setd1) or (D1)
  const int la31[] = {
    30, 10, // Number of jobs and machines
    4, 21, 7, 26, 9, 16, 2, 34, 3, 55, 8, 52, 5, 95, 6, 71, 1, 21, 0, 53, 
    8, 77, 5, 98, 1, 42, 7, 66, 2, 31, 3, 39, 6, 77, 9, 79, 4, 55, 0, 12, 
    2, 64, 4, 92, 3, 34, 1, 19, 8, 62, 6, 54, 7, 43, 0, 83, 9, 79, 5, 37, 
    0, 93, 8, 24, 3, 69, 7, 38, 5, 77, 2, 87, 4, 60, 6, 41, 1, 87, 9, 83, 
    9, 77, 0, 44, 4, 96, 8, 79, 6, 75, 2, 98, 5, 25, 3, 17, 7, 43, 1, 49, 
    3, 76, 2, 35, 5, 28, 0, 95, 7, 95, 4, 61, 8, 35, 1, 7, 6, 9, 9, 10, 
    1, 91, 7, 27, 8, 50, 3, 16, 4, 28, 5, 59, 6, 52, 0, 46, 2, 59, 9, 43, 
    1, 45, 7, 71, 2, 39, 0, 87, 8, 14, 6, 54, 3, 41, 9, 43, 5, 9, 4, 20, 
    2, 37, 3, 26, 4, 33, 9, 42, 0, 78, 6, 89, 7, 8, 8, 66, 1, 28, 5, 33, 
    1, 74, 0, 69, 5, 84, 3, 27, 9, 81, 7, 45, 8, 69, 2, 94, 6, 78, 4, 96, 
    5, 76, 7, 32, 6, 18, 0, 20, 3, 87, 2, 17, 9, 25, 4, 24, 1, 31, 8, 81, 
    9, 97, 8, 90, 5, 28, 7, 86, 0, 58, 1, 72, 2, 23, 6, 76, 3, 99, 4, 45, 
    9, 48, 5, 27, 6, 67, 7, 62, 4, 98, 0, 42, 1, 46, 8, 27, 3, 48, 2, 17, 
    9, 80, 3, 19, 5, 28, 1, 12, 4, 94, 6, 63, 7, 98, 8, 50, 0, 80, 2, 50, 
    2, 50, 1, 41, 4, 61, 8, 79, 5, 14, 9, 72, 7, 18, 3, 55, 6, 37, 0, 75, 
    9, 22, 5, 57, 4, 75, 2, 14, 7, 65, 3, 96, 1, 71, 0, 47, 8, 79, 6, 60, 
    3, 32, 2, 69, 4, 44, 1, 31, 9, 51, 0, 33, 6, 34, 5, 58, 7, 47, 8, 58, 
    8, 66, 7, 40, 2, 17, 0, 62, 9, 38, 5, 8, 6, 15, 3, 29, 1, 44, 4, 97, 
    3, 50, 2, 58, 6, 21, 4, 63, 7, 57, 8, 32, 5, 20, 9, 87, 0, 57, 1, 39, 
    4, 20, 6, 67, 1, 85, 2, 90, 7, 70, 0, 84, 8, 30, 5, 56, 3, 61, 9, 15, 
    6, 29, 0, 82, 4, 18, 3, 38, 7, 21, 8, 50, 1, 23, 5, 84, 2, 45, 9, 41, 
    3, 54, 9, 37, 6, 62, 5, 16, 0, 52, 8, 57, 4, 54, 2, 38, 7, 74, 1, 52, 
    4, 79, 1, 61, 8, 11, 0, 81, 7, 89, 6, 89, 5, 57, 3, 68, 9, 81, 2, 30, 
    9, 24, 1, 66, 4, 32, 3, 33, 8, 8, 2, 20, 6, 84, 0, 91, 7, 55, 5, 20, 
    3, 54, 2, 64, 6, 83, 9, 40, 7, 8, 0, 7, 4, 19, 5, 56, 1, 39, 8, 7, 
    1, 6, 4, 74, 0, 63, 2, 64, 9, 15, 6, 42, 7, 98, 8, 61, 5, 40, 3, 91, 
    1, 80, 3, 75, 0, 26, 2, 87, 9, 22, 7, 39, 8, 24, 4, 75, 6, 44, 5, 6, 
    5, 8, 3, 79, 6, 61, 1, 15, 0, 12, 7, 43, 8, 26, 9, 22, 2, 20, 4, 80, 
    1, 36, 0, 63, 7, 10, 4, 22, 3, 96, 5, 40, 9, 5, 8, 18, 6, 33, 2, 62, 
    4, 8, 8, 15, 2, 64, 3, 95, 1, 96, 6, 38, 7, 18, 9, 23, 5, 64, 0, 89
  };
  // Lawrence 30x10 instance (Table 9, instance 2); also called (setd2) or (D2)
  const int la32[] = {
    30, 10, // Number of jobs and machines
    6, 89, 1, 58, 4, 97, 2, 44, 8, 77, 3, 5, 0, 9, 5, 58, 9, 96, 7, 84, 
    7, 31, 2, 81, 9, 73, 4, 15, 1, 87, 5, 39, 8, 57, 0, 77, 3, 85, 6, 21, 
    2, 48, 5, 71, 0, 40, 3, 70, 1, 49, 6, 22, 4, 10, 8, 34, 7, 80, 9, 82, 
    4, 11, 6, 72, 7, 62, 0, 55, 2, 17, 5, 75, 3, 7, 1, 91, 9, 35, 8, 47, 
    0, 64, 6, 71, 4, 12, 1, 90, 2, 94, 3, 75, 9, 20, 8, 15, 5, 50, 7, 67, 
    2, 29, 6, 93, 3, 68, 5, 93, 1, 57, 8, 77, 0, 52, 9, 7, 4, 58, 7, 70, 
    4, 26, 3, 27, 1, 63, 5, 6, 6, 87, 7, 56, 8, 48, 9, 36, 0, 95, 2, 82, 
    1, 8, 7, 76, 3, 76, 4, 30, 6, 84, 9, 78, 8, 41, 0, 36, 2, 36, 5, 15, 
    3, 13, 8, 29, 0, 75, 2, 81, 1, 78, 5, 88, 4, 54, 9, 40, 7, 13, 6, 82, 
    0, 52, 2, 6, 3, 6, 5, 82, 6, 64, 9, 88, 8, 54, 4, 54, 7, 32, 1, 26, 
    8, 62, 1, 35, 4, 72, 7, 69, 0, 62, 5, 32, 9, 5, 3, 61, 2, 67, 6, 93, 
    2, 78, 3, 11, 7, 82, 4, 7, 1, 72, 8, 64, 9, 90, 0, 85, 5, 88, 6, 63, 
    7, 50, 4, 28, 3, 35, 1, 66, 2, 27, 8, 49, 9, 11, 6, 88, 5, 31, 0, 44, 
    4, 62, 5, 39, 0, 76, 2, 14, 6, 56, 3, 97, 1, 7, 7, 69, 9, 66, 8, 47, 
    6, 47, 2, 41, 0, 64, 7, 58, 9, 57, 8, 93, 3, 69, 5, 53, 1, 18, 4, 79, 
    7, 76, 9, 81, 0, 76, 6, 61, 4, 77, 8, 26, 2, 74, 5, 22, 1, 58, 3, 78, 
    6, 30, 8, 72, 3, 43, 0, 65, 1, 16, 4, 92, 5, 95, 9, 29, 2, 99, 7, 64, 
    1, 35, 3, 74, 5, 16, 4, 85, 0, 7, 2, 81, 6, 86, 8, 61, 9, 35, 7, 34, 
    1, 97, 7, 43, 4, 72, 6, 88, 5, 17, 0, 43, 8, 94, 3, 64, 9, 22, 2, 42, 
    7, 99, 2, 84, 8, 99, 5, 98, 1, 20, 6, 31, 3, 74, 0, 92, 9, 23, 4, 89, 
    8, 32, 0, 6, 4, 55, 5, 19, 9, 81, 1, 81, 7, 40, 6, 9, 3, 37, 2, 40, 
    6, 15, 2, 70, 8, 25, 1, 46, 9, 65, 4, 64, 7, 21, 0, 77, 5, 65, 3, 55, 
    8, 31, 7, 84, 5, 37, 3, 24, 2, 85, 4, 89, 9, 29, 1, 44, 0, 40, 6, 83, 
    4, 80, 0, 8, 9, 41, 5, 59, 7, 56, 3, 38, 2, 30, 8, 97, 6, 77, 1, 80, 
    9, 59, 0, 91, 3, 50, 8, 80, 1, 17, 6, 40, 2, 71, 5, 56, 4, 88, 7, 7, 
    7, 36, 3, 58, 4, 54, 5, 77, 2, 8, 6, 9, 0, 45, 9, 10, 1, 29, 8, 96, 
    0, 28, 3, 92, 2, 73, 7, 27, 8, 86, 5, 87, 9, 96, 1, 98, 6, 99, 4, 70, 
    9, 32, 1, 95, 3, 85, 6, 81, 2, 41, 8, 39, 7, 92, 0, 59, 5, 56, 4, 52, 
    4, 93, 2, 12, 5, 22, 6, 27, 8, 45, 7, 69, 3, 60, 1, 7, 0, 88, 9, 49, 
    2, 61, 5, 26, 9, 71, 8, 44, 0, 21, 6, 82, 3, 68, 7, 33, 1, 84, 4, 99
  };
  // Lawrence 30x10 instance (Table 9, instance 3); also called (setd3) or (D3)
  const int la33[] = {
    30, 10, // Number of jobs and machines
    2, 38, 4, 75, 9, 12, 5, 97, 0, 76, 1, 29, 8, 14, 6, 66, 7, 44, 3, 12, 
    0, 43, 5, 38, 1, 80, 3, 82, 2, 85, 4, 58, 6, 87, 8, 92, 9, 89, 7, 69, 
    6, 48, 4, 8, 8, 66, 7, 7, 2, 14, 3, 41, 5, 61, 0, 43, 1, 84, 9, 5, 
    5, 19, 3, 74, 6, 41, 4, 59, 8, 43, 2, 42, 9, 73, 7, 97, 1, 8, 0, 96, 
    3, 75, 5, 5, 2, 70, 8, 42, 7, 23, 6, 55, 1, 48, 9, 38, 4, 37, 0, 7, 
    2, 72, 7, 31, 3, 95, 0, 79, 4, 25, 1, 56, 8, 9, 9, 60, 5, 73, 6, 43, 
    9, 31, 3, 78, 6, 16, 4, 94, 7, 86, 5, 21, 0, 97, 8, 53, 1, 7, 2, 64, 
    3, 86, 2, 65, 6, 59, 8, 44, 1, 33, 7, 85, 0, 61, 5, 32, 9, 63, 4, 30, 
    4, 11, 5, 61, 9, 84, 3, 16, 7, 90, 1, 30, 0, 60, 8, 93, 2, 44, 6, 45, 
    5, 11, 2, 28, 0, 32, 7, 36, 8, 31, 4, 47, 3, 20, 6, 52, 9, 35, 1, 49, 
    5, 17, 3, 34, 6, 49, 1, 84, 0, 85, 8, 20, 7, 74, 9, 68, 4, 10, 2, 77, 
    8, 71, 5, 7, 3, 29, 1, 85, 4, 76, 6, 59, 2, 17, 0, 17, 9, 13, 7, 48, 
    1, 39, 9, 16, 4, 39, 6, 87, 7, 11, 3, 32, 2, 15, 0, 19, 5, 64, 8, 43, 
    5, 33, 8, 82, 2, 92, 1, 83, 6, 32, 3, 99, 9, 99, 4, 91, 0, 8, 7, 57, 
    7, 7, 0, 48, 9, 62, 4, 88, 6, 21, 5, 39, 8, 27, 3, 91, 1, 38, 2, 69, 
    9, 64, 8, 45, 3, 24, 7, 80, 2, 67, 4, 18, 6, 38, 0, 88, 5, 80, 1, 44, 
    2, 15, 3, 72, 4, 40, 7, 21, 8, 52, 0, 51, 9, 59, 1, 24, 6, 47, 5, 43, 
    4, 77, 7, 43, 1, 40, 2, 31, 8, 76, 6, 20, 5, 88, 3, 70, 9, 5, 0, 32, 
    2, 14, 7, 58, 9, 85, 5, 64, 1, 26, 6, 94, 0, 32, 3, 49, 8, 80, 4, 47, 
    9, 23, 1, 11, 8, 34, 4, 75, 7, 79, 3, 26, 2, 96, 0, 5, 6, 9, 5, 59, 
    0, 75, 2, 20, 8, 10, 3, 66, 6, 43, 7, 37, 1, 9, 9, 83, 4, 68, 5, 52, 
    8, 54, 1, 26, 4, 79, 7, 88, 6, 84, 0, 6, 2, 54, 9, 59, 3, 28, 5, 42, 
    4, 56, 9, 29, 3, 36, 0, 40, 6, 86, 8, 68, 2, 69, 7, 23, 5, 62, 1, 16, 
    7, 53, 1, 5, 6, 17, 9, 59, 2, 59, 8, 78, 3, 64, 0, 82, 4, 13, 5, 12, 
    9, 7, 6, 62, 7, 90, 5, 83, 1, 85, 3, 69, 0, 16, 4, 81, 2, 58, 8, 66, 
    7, 24, 2, 65, 1, 69, 5, 42, 9, 82, 6, 82, 0, 83, 3, 46, 8, 72, 4, 33, 
    1, 10, 8, 27, 7, 43, 5, 20, 4, 71, 9, 65, 2, 73, 6, 99, 0, 24, 3, 64, 
    9, 35, 3, 92, 0, 38, 5, 35, 7, 30, 8, 45, 2, 8, 4, 82, 1, 34, 6, 21, 
    5, 23, 7, 84, 9, 7, 4, 85, 8, 60, 1, 15, 2, 52, 6, 94, 3, 83, 0, 6, 
    2, 70, 6, 29, 8, 27, 9, 80, 4, 6, 7, 39, 1, 79, 0, 28, 3, 66, 5, 66
  };
  // Lawrence 30x10 instance (Table 9, instance 4); also called (setd4) or (D4)
  const int la34[] = {
    30, 10, // Number of jobs and machines
    2, 51, 7, 59, 1, 35, 5, 73, 9, 65, 0, 27, 6, 13, 3, 81, 8, 32, 4, 74, 
    4, 64, 7, 33, 5, 75, 2, 33, 8, 10, 0, 28, 3, 38, 6, 53, 9, 49, 1, 55, 
    6, 83, 1, 23, 2, 72, 3, 7, 9, 72, 0, 6, 4, 39, 5, 52, 8, 90, 7, 21, 
    3, 82, 1, 23, 2, 93, 4, 78, 6, 88, 7, 53, 9, 28, 8, 65, 5, 21, 0, 61, 
    4, 41, 6, 12, 9, 12, 3, 77, 1, 70, 7, 24, 0, 81, 5, 73, 2, 62, 8, 6, 
    4, 98, 3, 28, 6, 42, 9, 72, 0, 15, 8, 15, 5, 94, 2, 33, 1, 51, 7, 99, 
    0, 32, 8, 22, 9, 96, 4, 15, 6, 78, 3, 31, 5, 7, 1, 94, 2, 23, 7, 86, 
    7, 93, 2, 97, 3, 43, 5, 73, 0, 24, 8, 68, 9, 88, 1, 42, 4, 35, 6, 72, 
    2, 14, 0, 44, 8, 13, 5, 67, 1, 63, 3, 49, 7, 5, 4, 17, 6, 85, 9, 66, 
    7, 82, 9, 15, 3, 72, 4, 26, 0, 8, 1, 68, 6, 21, 8, 45, 2, 99, 5, 27, 
    4, 93, 6, 23, 0, 51, 8, 54, 3, 49, 1, 96, 2, 56, 9, 36, 5, 53, 7, 52, 
    8, 60, 0, 14, 4, 70, 9, 55, 1, 23, 5, 83, 3, 38, 2, 24, 7, 37, 6, 48, 
    0, 62, 7, 15, 8, 69, 9, 23, 1, 82, 6, 26, 4, 45, 5, 33, 3, 12, 2, 37, 
    6, 72, 1, 9, 7, 15, 5, 28, 8, 92, 9, 12, 0, 59, 3, 64, 4, 87, 2, 73, 
    0, 50, 1, 14, 7, 90, 5, 46, 3, 71, 4, 48, 2, 80, 9, 61, 8, 24, 6, 44, 
    0, 22, 9, 94, 5, 16, 3, 73, 2, 54, 8, 54, 4, 46, 1, 97, 6, 61, 7, 75, 
    9, 55, 3, 67, 6, 77, 4, 30, 7, 6, 1, 32, 8, 47, 5, 93, 2, 6, 0, 40, 
    1, 30, 3, 98, 7, 79, 0, 22, 6, 79, 2, 7, 8, 36, 9, 36, 5, 9, 4, 92, 
    8, 37, 7, 72, 2, 52, 4, 31, 1, 82, 9, 54, 5, 7, 6, 82, 3, 73, 0, 49, 
    1, 73, 3, 83, 7, 45, 2, 76, 4, 43, 9, 29, 0, 35, 5, 92, 8, 39, 6, 28, 
    2, 58, 0, 26, 1, 48, 8, 52, 7, 34, 6, 96, 5, 70, 4, 98, 3, 80, 9, 94, 
    1, 70, 8, 23, 5, 26, 4, 14, 6, 90, 2, 93, 3, 21, 0, 42, 7, 18, 9, 36, 
    4, 28, 6, 76, 7, 25, 0, 17, 1, 84, 2, 67, 8, 87, 3, 43, 9, 88, 5, 84, 
    7, 30, 3, 91, 8, 52, 4, 80, 0, 21, 5, 8, 9, 37, 2, 15, 6, 12, 1, 92, 
    1, 28, 4, 7, 7, 46, 6, 92, 2, 77, 3, 15, 9, 69, 8, 54, 0, 47, 5, 39, 
    9, 50, 5, 44, 2, 64, 8, 38, 4, 93, 6, 33, 7, 75, 0, 41, 1, 24, 3, 5, 
    7, 94, 0, 17, 6, 87, 2, 21, 8, 92, 9, 28, 1, 61, 4, 63, 3, 34, 5, 77, 
    3, 72, 8, 98, 9, 5, 4, 28, 2, 9, 5, 95, 6, 64, 1, 43, 0, 50, 7, 96, 
    0, 85, 2, 85, 8, 39, 1, 98, 7, 24, 3, 71, 5, 60, 4, 55, 9, 22, 6, 35, 
    3, 78, 6, 49, 2, 46, 1, 11, 0, 90, 5, 20, 9, 34, 7, 6, 4, 70, 8, 74
  };
  // Lawrence 30x10 instance (Table 9, instance 5); also called (setd5) or (D5)
  const int la35[] = {
    30, 10, // Number of jobs and machines
    0, 66, 2, 84, 3, 26, 7, 29, 9, 94, 6, 98, 8, 7, 5, 98, 1, 45, 4, 43, 
    3, 32, 0, 97, 6, 55, 2, 88, 8, 93, 9, 88, 1, 20, 4, 50, 7, 17, 5, 5, 
    4, 43, 3, 68, 8, 47, 9, 68, 1, 57, 6, 20, 5, 81, 2, 60, 7, 94, 0, 62, 
    1, 57, 5, 40, 0, 78, 6, 9, 2, 49, 9, 17, 3, 32, 4, 30, 8, 87, 7, 77, 
    0, 52, 4, 30, 3, 48, 5, 48, 1, 26, 9, 17, 6, 93, 8, 97, 7, 49, 2, 89, 
    7, 95, 0, 33, 1, 5, 6, 17, 5, 70, 3, 57, 4, 34, 2, 61, 8, 62, 9, 39, 
    7, 97, 5, 92, 1, 31, 8, 5, 2, 79, 4, 5, 3, 67, 0, 5, 9, 78, 6, 60, 
    2, 79, 4, 6, 7, 20, 8, 45, 6, 34, 3, 24, 9, 26, 5, 68, 1, 16, 0, 46, 
    7, 58, 9, 50, 2, 19, 8, 93, 6, 49, 3, 25, 5, 85, 4, 50, 0, 93, 1, 26, 
    9, 81, 6, 71, 5, 7, 1, 39, 2, 16, 8, 42, 0, 71, 4, 84, 3, 56, 7, 99, 
    8, 9, 0, 86, 9, 6, 3, 71, 6, 97, 5, 85, 4, 16, 2, 42, 7, 81, 1, 81, 
    4, 72, 3, 24, 0, 30, 8, 56, 2, 43, 1, 61, 7, 82, 6, 40, 5, 59, 9, 43, 
    9, 43, 1, 13, 6, 70, 7, 93, 0, 95, 8, 12, 4, 15, 2, 78, 5, 97, 3, 14, 
    0, 14, 6, 26, 1, 71, 3, 46, 8, 80, 5, 31, 4, 37, 9, 27, 7, 92, 2, 67, 
    2, 12, 0, 43, 5, 96, 6, 7, 3, 45, 7, 20, 1, 13, 9, 29, 4, 60, 8, 33, 
    1, 78, 5, 50, 6, 84, 0, 42, 8, 84, 4, 30, 9, 76, 2, 57, 7, 87, 3, 59, 
    4, 49, 7, 50, 1, 15, 8, 13, 0, 93, 6, 50, 9, 32, 5, 59, 3, 10, 2, 35, 
    1, 25, 0, 47, 7, 60, 8, 33, 4, 53, 5, 37, 9, 73, 2, 22, 3, 87, 6, 79, 
    0, 84, 6, 83, 1, 71, 5, 68, 9, 89, 8, 11, 3, 60, 4, 50, 2, 33, 7, 97, 
    1, 14, 0, 38, 6, 88, 5, 5, 4, 77, 7, 92, 8, 24, 2, 73, 9, 52, 3, 71, 
    7, 62, 9, 19, 6, 38, 3, 15, 8, 64, 2, 64, 4, 8, 1, 61, 0, 19, 5, 33, 
    2, 33, 5, 46, 4, 74, 0, 56, 6, 84, 9, 83, 8, 19, 7, 8, 3, 32, 1, 97, 
    4, 50, 3, 71, 6, 50, 2, 97, 9, 8, 0, 17, 7, 19, 8, 92, 5, 54, 1, 52, 
    8, 32, 1, 79, 3, 97, 5, 38, 9, 49, 4, 76, 6, 76, 0, 56, 2, 78, 7, 54, 
    5, 13, 3, 5, 2, 25, 0, 86, 1, 95, 9, 28, 6, 78, 8, 24, 7, 10, 4, 39, 
    7, 48, 2, 59, 0, 20, 9, 7, 5, 31, 6, 97, 1, 89, 4, 32, 3, 25, 8, 41, 
    5, 87, 0, 18, 9, 48, 2, 43, 1, 30, 6, 97, 7, 47, 8, 65, 3, 69, 4, 27, 
    6, 71, 5, 20, 8, 20, 1, 78, 3, 39, 0, 17, 7, 50, 2, 44, 9, 42, 4, 38, 
    0, 50, 9, 42, 3, 72, 5, 7, 1, 77, 7, 58, 4, 78, 2, 89, 6, 70, 8, 36, 
    3, 32, 9, 95, 2, 13, 0, 73, 6, 97, 8, 24, 4, 49, 5, 57, 1, 68, 7, 94
  };
  // Lawrence 15x15 instance (Table 10, instance 1); also called (seti1) or (I1)
  const int la36[] = {
    15, 15, // Number of jobs and machines
    4, 21, 3, 55, 6, 71, 14, 98, 10, 12, 2, 34, 9, 16, 1, 21, 0, 53, 7, 26, 8, 52, 5, 95, 12, 31, 11, 42, 13, 39, 
    11, 54, 4, 83, 1, 77, 7, 64, 8, 34, 14, 79, 12, 43, 0, 55, 3, 77, 6, 19, 9, 37, 5, 79, 10, 92, 13, 62, 2, 66, 
    9, 83, 5, 77, 2, 87, 7, 38, 4, 60, 12, 98, 0, 93, 13, 17, 6, 41, 10, 44, 3, 69, 11, 49, 8, 24, 1, 87, 14, 25, 
    5, 77, 0, 96, 9, 28, 6, 7, 4, 95, 13, 35, 7, 35, 8, 76, 11, 9, 12, 95, 2, 43, 1, 75, 10, 61, 14, 10, 3, 79, 
    10, 87, 4, 28, 8, 50, 2, 59, 0, 46, 11, 45, 14, 9, 9, 43, 6, 52, 7, 27, 1, 91, 13, 41, 3, 16, 5, 59, 12, 39, 
    0, 20, 2, 71, 4, 78, 13, 66, 3, 14, 12, 8, 14, 42, 6, 28, 1, 54, 9, 33, 11, 89, 8, 26, 7, 37, 10, 33, 5, 43, 
    8, 69, 4, 96, 12, 17, 0, 69, 7, 45, 11, 31, 6, 78, 10, 20, 3, 27, 13, 87, 1, 74, 5, 84, 14, 76, 2, 94, 9, 81, 
    4, 58, 13, 90, 11, 76, 3, 81, 7, 23, 9, 28, 1, 18, 2, 32, 12, 86, 8, 99, 14, 97, 0, 24, 10, 45, 6, 72, 5, 25, 
    5, 27, 1, 46, 6, 67, 8, 27, 13, 19, 10, 80, 2, 17, 3, 48, 7, 62, 11, 12, 14, 28, 4, 98, 0, 42, 9, 48, 12, 50, 
    11, 37, 5, 80, 4, 75, 8, 55, 7, 50, 0, 94, 9, 14, 6, 41, 14, 72, 3, 50, 10, 61, 13, 79, 2, 98, 12, 18, 1, 63, 
    7, 65, 3, 96, 0, 47, 4, 75, 12, 69, 14, 58, 10, 33, 1, 71, 9, 22, 13, 32, 5, 57, 8, 79, 2, 14, 11, 31, 6, 60, 
    1, 34, 2, 47, 3, 58, 5, 51, 4, 62, 6, 44, 9, 8, 7, 17, 10, 97, 8, 29, 11, 15, 13, 66, 12, 40, 0, 44, 14, 38, 
    3, 50, 7, 57, 13, 61, 5, 20, 11, 85, 12, 90, 2, 58, 4, 63, 10, 84, 1, 39, 9, 87, 6, 21, 14, 56, 8, 32, 0, 57, 
    9, 84, 7, 45, 5, 15, 14, 41, 10, 18, 4, 82, 11, 29, 2, 70, 1, 67, 3, 30, 13, 50, 6, 23, 0, 20, 12, 21, 8, 38, 
    9, 37, 10, 81, 11, 61, 14, 57, 8, 57, 0, 52, 7, 74, 6, 62, 12, 30, 1, 52, 2, 38, 13, 68, 4, 54, 3, 54, 5, 16
  };
  // Lawrence 15x15 instance (Table 10, instance 2); also called (seti2) or (I2)
  const int la37[] = {
    15, 15, // Number of jobs and machines
    5, 19, 6, 64, 11, 73, 9, 13, 2, 84, 14, 88, 3, 85, 10, 41, 12, 53, 13, 80, 1, 66, 7, 46, 8, 59, 4, 25, 0, 62, 
    1, 67, 3, 74, 7, 41, 2, 57, 14, 52, 0, 14, 9, 64, 8, 84, 6, 78, 5, 47, 13, 28, 4, 84, 10, 63, 12, 26, 11, 46, 
    6, 97, 8, 95, 0, 64, 9, 38, 10, 59, 12, 95, 2, 17, 11, 65, 13, 93, 3, 10, 5, 73, 1, 11, 4, 85, 14, 46, 7, 67, 
    10, 23, 12, 49, 3, 32, 4, 66, 2, 43, 0, 60, 8, 41, 7, 61, 13, 70, 9, 49, 11, 17, 6, 90, 1, 85, 14, 99, 5, 85, 
    9, 98, 8, 57, 3, 73, 6, 9, 0, 73, 7, 7, 1, 98, 4, 13, 13, 41, 5, 40, 11, 85, 10, 37, 2, 68, 14, 79, 12, 17, 
    11, 66, 7, 53, 5, 86, 6, 40, 0, 14, 3, 19, 13, 96, 4, 95, 2, 54, 10, 84, 12, 97, 8, 16, 14, 52, 1, 76, 9, 87, 
    4, 77, 2, 55, 9, 42, 5, 74, 14, 91, 13, 33, 10, 16, 12, 54, 0, 18, 3, 87, 7, 60, 8, 13, 6, 33, 1, 33, 11, 61, 
    6, 41, 5, 39, 11, 82, 9, 64, 14, 47, 10, 28, 7, 78, 13, 49, 1, 79, 4, 58, 2, 92, 3, 79, 12, 6, 0, 69, 8, 76, 
    11, 21, 5, 42, 9, 91, 2, 28, 0, 52, 6, 88, 12, 76, 13, 86, 10, 23, 1, 35, 7, 52, 4, 91, 3, 47, 14, 82, 8, 24, 
    11, 42, 1, 93, 3, 95, 13, 45, 9, 28, 14, 77, 0, 84, 10, 8, 7, 45, 4, 70, 5, 37, 6, 86, 12, 64, 8, 67, 2, 38, 
    4, 97, 12, 81, 1, 58, 7, 84, 5, 58, 0, 9, 11, 87, 3, 5, 2, 44, 13, 85, 6, 89, 10, 77, 9, 96, 14, 39, 8, 77, 
    12, 80, 1, 21, 10, 10, 5, 73, 8, 70, 6, 49, 2, 31, 13, 34, 4, 40, 11, 22, 0, 15, 14, 82, 3, 57, 9, 71, 7, 48, 
    2, 17, 7, 62, 5, 75, 9, 35, 1, 91, 14, 50, 3, 7, 10, 64, 13, 75, 12, 94, 0, 55, 6, 72, 8, 47, 4, 11, 11, 90, 
    11, 93, 6, 57, 1, 71, 12, 70, 9, 93, 5, 20, 3, 15, 13, 77, 10, 58, 0, 12, 2, 67, 8, 68, 14, 7, 7, 29, 4, 52, 
    13, 76, 3, 27, 4, 26, 9, 36, 11, 8, 10, 36, 0, 95, 8, 48, 2, 82, 6, 87, 5, 6, 1, 63, 7, 56, 12, 36, 14, 15
  };
  // Lawrence 15x15 instance (Table 10, instance 3); also called (seti3) or (I3)
  const int la38[] = {
    15, 15, // Number of jobs and machines
    1, 26, 12, 67, 0, 72, 6, 74, 14, 13, 8, 43, 4, 30, 3, 19, 10, 23, 11, 85, 5, 98, 13, 43, 2, 38, 7, 8, 9, 75, 
    14, 42, 0, 39, 4, 55, 12, 46, 1, 19, 8, 93, 9, 80, 5, 26, 10, 7, 6, 50, 11, 57, 3, 73, 2, 9, 7, 61, 13, 72, 
    3, 96, 4, 99, 12, 34, 6, 60, 7, 43, 14, 7, 13, 12, 8, 11, 11, 70, 10, 43, 0, 91, 1, 68, 9, 11, 5, 68, 2, 72, 
    14, 63, 11, 45, 4, 49, 1, 74, 8, 27, 0, 30, 9, 72, 7, 9, 12, 99, 13, 60, 5, 69, 6, 69, 2, 84, 3, 40, 10, 59, 
    2, 91, 0, 75, 9, 98, 3, 17, 10, 72, 13, 31, 11, 9, 14, 98, 7, 50, 5, 37, 4, 8, 8, 65, 1, 90, 12, 91, 6, 71, 
    11, 35, 6, 80, 4, 39, 3, 62, 14, 74, 5, 72, 10, 35, 9, 25, 1, 49, 8, 52, 7, 63, 2, 90, 13, 21, 12, 47, 0, 38, 
    14, 19, 7, 57, 10, 24, 13, 91, 3, 50, 0, 5, 11, 49, 12, 18, 9, 58, 5, 24, 8, 52, 1, 88, 2, 68, 6, 20, 4, 53, 
    7, 77, 14, 72, 5, 35, 11, 90, 4, 68, 6, 18, 3, 9, 0, 33, 8, 60, 10, 18, 12, 10, 13, 60, 1, 38, 2, 99, 9, 15, 
    13, 6, 8, 86, 2, 40, 9, 79, 12, 92, 11, 23, 5, 89, 10, 95, 6, 91, 7, 72, 0, 80, 1, 60, 3, 56, 4, 51, 14, 23, 
    1, 46, 6, 28, 5, 34, 11, 77, 4, 47, 0, 10, 14, 49, 8, 77, 10, 48, 7, 24, 12, 8, 2, 72, 13, 55, 9, 29, 3, 40, 
    10, 22, 4, 89, 12, 79, 0, 7, 9, 15, 1, 6, 11, 30, 6, 38, 5, 11, 8, 52, 3, 20, 7, 5, 14, 9, 2, 20, 13, 28, 
    5, 73, 14, 56, 2, 37, 3, 22, 13, 25, 6, 58, 1, 8, 7, 93, 4, 88, 8, 17, 12, 9, 11, 69, 10, 71, 9, 85, 0, 55, 
    9, 85, 14, 58, 3, 46, 8, 64, 2, 49, 6, 37, 1, 33, 4, 30, 5, 26, 0, 20, 13, 74, 10, 77, 12, 99, 11, 56, 7, 21, 
    10, 17, 3, 24, 4, 89, 5, 15, 11, 60, 1, 42, 8, 98, 2, 64, 13, 92, 0, 63, 7, 52, 12, 54, 6, 75, 14, 23, 9, 38, 
    3, 8, 5, 17, 11, 56, 7, 93, 14, 26, 9, 62, 6, 7, 10, 88, 0, 97, 1, 7, 2, 43, 8, 29, 13, 35, 12, 87, 4, 57
  };
  // Lawrence 15x15 instance (Table 10, instance 4); also called (seti4) or (I4)
  const int la39[] = {
    15, 15, // Number of jobs and machines
    10, 51, 14, 43, 7, 80, 4, 18, 6, 38, 3, 24, 2, 67, 12, 15, 11, 24, 13, 72, 8, 45, 5, 80, 9, 64, 1, 44, 0, 88, 
    6, 40, 9, 88, 10, 77, 5, 59, 11, 20, 3, 52, 8, 70, 0, 40, 4, 32, 13, 76, 12, 43, 7, 31, 2, 21, 14, 5, 1, 47, 
    0, 32, 3, 49, 10, 5, 5, 64, 7, 58, 8, 80, 6, 94, 11, 11, 1, 26, 13, 26, 14, 59, 9, 85, 4, 47, 12, 96, 2, 14, 
    5, 23, 6, 9, 0, 75, 12, 37, 11, 43, 2, 79, 4, 75, 3, 34, 7, 20, 13, 10, 14, 83, 10, 68, 9, 52, 8, 66, 1, 9, 
    12, 69, 9, 59, 3, 28, 14, 62, 13, 36, 1, 26, 6, 84, 11, 16, 8, 54, 5, 42, 2, 54, 0, 6, 10, 40, 7, 88, 4, 79, 
    13, 78, 12, 53, 11, 17, 5, 29, 4, 82, 2, 23, 9, 12, 8, 64, 1, 86, 7, 59, 6, 5, 3, 68, 14, 59, 10, 13, 0, 56, 
    10, 83, 13, 46, 9, 7, 12, 65, 11, 69, 6, 62, 0, 16, 2, 58, 8, 66, 5, 83, 7, 90, 14, 42, 4, 81, 3, 69, 1, 85, 
    7, 73, 10, 71, 8, 64, 6, 10, 9, 20, 11, 99, 4, 24, 14, 65, 5, 82, 3, 72, 12, 43, 1, 82, 13, 27, 2, 24, 0, 33, 
    4, 82, 1, 34, 3, 92, 2, 8, 0, 38, 8, 45, 6, 21, 5, 35, 12, 52, 9, 35, 11, 15, 14, 23, 10, 6, 13, 83, 7, 30, 
    2, 84, 5, 7, 9, 66, 10, 6, 4, 28, 13, 27, 6, 79, 7, 70, 0, 85, 1, 94, 3, 60, 14, 80, 12, 39, 8, 66, 11, 29, 
    3, 44, 6, 58, 13, 14, 8, 65, 1, 72, 5, 14, 12, 52, 4, 21, 9, 25, 0, 5, 11, 51, 7, 61, 14, 55, 10, 42, 2, 36, 
    14, 43, 10, 72, 5, 78, 11, 12, 12, 17, 0, 46, 9, 27, 6, 51, 2, 63, 1, 79, 8, 79, 7, 91, 4, 49, 13, 26, 3, 93, 
    7, 49, 0, 49, 4, 71, 5, 78, 9, 44, 10, 41, 12, 91, 13, 84, 8, 91, 6, 21, 11, 47, 14, 28, 3, 61, 2, 70, 1, 93, 
    3, 25, 4, 85, 0, 66, 2, 45, 10, 95, 12, 21, 8, 84, 5, 24, 9, 53, 7, 67, 6, 91, 11, 11, 13, 32, 1, 30, 14, 89, 
    3, 92, 7, 93, 0, 99, 1, 40, 10, 37, 12, 69, 5, 66, 6, 57, 14, 22, 9, 44, 8, 73, 13, 97, 11, 18, 2, 69, 4, 41
  };
  // Lawrence 15x15 instance (Table 10, instance 5); also called (seti5) or (I5)
  const int la40[] = {
    15, 15, // Number of jobs and machines
    9, 65, 10, 28, 4, 74, 12, 33, 2, 51, 14, 75, 5, 73, 8, 32, 6, 13, 3, 81, 1, 35, 7, 59, 13, 38, 11, 55, 0, 27, 
    0, 64, 1, 53, 11, 83, 2, 33, 4, 6, 9, 52, 14, 72, 8, 7, 13, 90, 12, 21, 6, 23, 3, 10, 10, 39, 5, 49, 7, 72, 
    14, 73, 3, 82, 1, 23, 12, 62, 6, 88, 5, 21, 8, 65, 11, 70, 7, 53, 10, 81, 2, 93, 13, 77, 0, 61, 9, 28, 4, 78, 
    1, 12, 6, 51, 7, 33, 4, 15, 14, 72, 10, 98, 9, 94, 5, 12, 11, 42, 2, 24, 13, 15, 8, 28, 3, 6, 12, 99, 0, 41, 
    12, 97, 5, 7, 9, 96, 4, 15, 14, 73, 13, 43, 0, 32, 8, 22, 11, 42, 1, 94, 2, 23, 7, 86, 6, 78, 10, 24, 3, 31, 
    1, 72, 5, 88, 2, 93, 13, 13, 4, 44, 14, 66, 6, 63, 7, 14, 9, 67, 10, 17, 11, 85, 0, 35, 3, 68, 12, 5, 8, 49, 
    9, 15, 7, 82, 6, 21, 14, 53, 3, 72, 13, 49, 2, 99, 4, 26, 12, 56, 8, 45, 1, 68, 10, 51, 0, 8, 5, 27, 11, 96, 
    3, 54, 7, 24, 4, 14, 8, 38, 5, 36, 2, 52, 14, 55, 12, 37, 11, 48, 0, 93, 13, 60, 10, 70, 1, 23, 6, 23, 9, 83, 
    3, 12, 8, 69, 6, 26, 9, 23, 14, 28, 1, 82, 5, 33, 4, 45, 13, 64, 7, 15, 11, 9, 12, 73, 10, 59, 2, 37, 0, 62, 
    0, 87, 5, 12, 7, 80, 4, 50, 10, 48, 12, 90, 1, 72, 13, 24, 6, 14, 8, 71, 11, 44, 9, 46, 2, 15, 14, 61, 3, 92, 
    2, 54, 0, 22, 6, 61, 4, 46, 3, 73, 5, 16, 12, 6, 9, 94, 14, 93, 13, 67, 8, 54, 7, 75, 11, 32, 10, 40, 1, 97, 
    10, 92, 14, 36, 4, 22, 9, 9, 3, 47, 1, 77, 12, 79, 13, 36, 6, 30, 8, 98, 11, 79, 7, 7, 5, 55, 2, 6, 0, 30, 
    0, 49, 13, 83, 3, 73, 6, 82, 1, 82, 14, 92, 11, 73, 4, 31, 10, 35, 9, 54, 5, 7, 8, 37, 7, 72, 2, 52, 12, 76, 
    10, 98, 12, 34, 13, 52, 4, 26, 1, 28, 3, 39, 8, 80, 5, 29, 9, 70, 0, 43, 6, 48, 7, 58, 2, 45, 14, 94, 11, 96, 
    1, 70, 10, 17, 6, 90, 12, 67, 4, 14, 8, 23, 3, 21, 7, 18, 13, 43, 11, 84, 5, 26, 9, 36, 2, 93, 14, 84, 0, 42
  };

  // trivial 10x10 instance from Bill Cook (BIC2)
  const int orb01[] = {
    10, 10, // Number of jobs and machines
    0, 72, 1, 64, 2, 55, 3, 31, 4, 53, 5, 95, 6, 11, 7, 52, 8, 6, 9, 84, 
    0, 61, 3, 27, 4, 88, 2, 78, 1, 49, 5, 83, 8, 91, 6, 74, 7, 29, 9, 87, 
    0, 86, 3, 32, 1, 35, 2, 37, 5, 18, 4, 48, 6, 91, 7, 52, 9, 60, 8, 30, 
    0, 8, 1, 82, 4, 27, 3, 99, 6, 74, 5, 9, 2, 33, 9, 20, 7, 59, 8, 98, 
    1, 50, 0, 94, 5, 43, 3, 62, 4, 55, 7, 48, 2, 5, 8, 36, 9, 47, 6, 36, 
    0, 53, 6, 30, 2, 7, 3, 12, 1, 68, 8, 87, 4, 28, 9, 70, 7, 45, 5, 7, 
    2, 29, 3, 96, 0, 99, 1, 14, 4, 34, 7, 14, 5, 7, 6, 76, 8, 57, 9, 76, 
    2, 90, 0, 19, 3, 87, 4, 51, 1, 84, 5, 45, 9, 84, 6, 58, 7, 81, 8, 96, 
    2, 97, 1, 99, 4, 93, 0, 38, 7, 13, 5, 96, 3, 40, 9, 64, 6, 32, 8, 45, 
    2, 44, 0, 60, 8, 29, 3, 5, 6, 74, 1, 85, 4, 34, 7, 95, 9, 51, 5, 47
  };
  // doomed 10x10 instance from Monika (MON2)
  const int orb02[] = {
    10, 10, // Number of jobs and machines
    0, 72, 1, 54, 2, 33, 3, 86, 4, 75, 5, 16, 6, 96, 7, 7, 8, 99, 9, 76, 
    0, 16, 3, 88, 4, 48, 8, 52, 9, 60, 6, 29, 7, 18, 5, 89, 2, 80, 1, 76, 
    0, 47, 7, 11, 3, 14, 2, 56, 6, 16, 4, 83, 1, 10, 5, 61, 8, 24, 9, 58, 
    0, 49, 1, 31, 3, 17, 8, 50, 5, 63, 2, 35, 4, 65, 7, 23, 6, 50, 9, 29, 
    0, 55, 6, 6, 1, 28, 3, 96, 5, 86, 2, 99, 9, 14, 7, 70, 8, 64, 4, 24, 
    4, 46, 0, 23, 6, 70, 8, 19, 2, 54, 3, 22, 9, 85, 7, 87, 5, 79, 1, 93, 
    4, 76, 3, 60, 0, 76, 9, 98, 2, 76, 1, 50, 8, 86, 7, 14, 6, 27, 5, 57, 
    4, 93, 6, 27, 9, 57, 3, 87, 8, 86, 2, 54, 7, 24, 5, 49, 0, 20, 1, 47, 
    2, 28, 6, 11, 8, 78, 7, 85, 4, 63, 9, 81, 3, 10, 1, 9, 5, 46, 0, 32, 
    2, 22, 9, 76, 5, 89, 8, 13, 6, 88, 3, 10, 7, 75, 4, 98, 1, 78, 0, 17
  };
  // deadlier 10x10 instance from Bruce Gamble (BRG1)
  const int orb03[] = {
    10, 10, // Number of jobs and machines
    0, 96, 1, 69, 2, 25, 3, 5, 4, 55, 5, 15, 6, 88, 7, 11, 8, 17, 9, 82, 
    0, 11, 1, 48, 2, 67, 3, 38, 4, 18, 7, 24, 6, 62, 5, 92, 9, 96, 8, 81, 
    2, 67, 1, 63, 0, 93, 4, 85, 3, 25, 5, 72, 6, 51, 7, 81, 8, 58, 9, 15, 
    2, 30, 1, 35, 0, 27, 4, 82, 3, 44, 7, 92, 6, 25, 5, 49, 9, 28, 8, 77, 
    1, 53, 0, 83, 4, 73, 3, 26, 2, 77, 6, 33, 5, 92, 9, 99, 8, 38, 7, 38, 
    1, 20, 0, 44, 4, 81, 3, 88, 2, 66, 6, 70, 5, 91, 9, 37, 8, 55, 7, 96, 
    1, 21, 2, 93, 4, 22, 0, 56, 3, 34, 6, 40, 7, 53, 9, 46, 5, 29, 8, 63, 
    1, 32, 2, 63, 4, 36, 0, 26, 3, 17, 5, 85, 7, 15, 8, 55, 9, 16, 6, 82, 
    0, 73, 2, 46, 3, 89, 4, 24, 1, 99, 6, 92, 7, 7, 9, 51, 5, 19, 8, 14, 
    0, 52, 2, 20, 3, 70, 4, 98, 1, 23, 5, 15, 7, 81, 8, 71, 9, 24, 6, 81
  };
  // deadly 10x10 instance from Bruce Shepherd (BRS1)
  const int orb04[] = {
    10, 10, // Number of jobs and machines
    0, 8, 1, 10, 2, 35, 3, 44, 4, 15, 5, 92, 6, 70, 7, 89, 8, 50, 9, 12, 
    0, 63, 8, 39, 3, 80, 5, 22, 2, 88, 1, 39, 9, 85, 6, 27, 7, 74, 4, 69, 
    0, 52, 6, 22, 1, 33, 3, 68, 8, 27, 2, 68, 5, 25, 4, 34, 7, 24, 9, 84, 
    0, 31, 1, 85, 4, 55, 8, 80, 5, 58, 7, 11, 6, 69, 9, 56, 3, 73, 2, 25, 
    0, 97, 5, 98, 9, 87, 8, 47, 7, 77, 4, 90, 3, 98, 2, 80, 1, 39, 6, 40, 
    1, 97, 5, 68, 0, 44, 9, 67, 2, 44, 8, 85, 3, 78, 6, 90, 7, 33, 4, 81, 
    0, 34, 3, 76, 8, 48, 7, 61, 9, 11, 2, 36, 4, 33, 6, 98, 1, 7, 5, 44, 
    0, 44, 9, 5, 4, 85, 1, 51, 5, 58, 7, 79, 2, 95, 6, 48, 3, 86, 8, 73, 
    0, 24, 1, 63, 9, 48, 7, 77, 8, 73, 6, 74, 4, 63, 5, 17, 2, 93, 3, 84, 
    0, 51, 2, 5, 4, 40, 9, 60, 1, 46, 5, 58, 8, 54, 3, 72, 6, 29, 7, 94
  };
  // 10x10 instance from George Steiner (GES1)
  const int orb05[] = {
    10, 10, // Number of jobs and machines
    9, 11, 8, 93, 0, 48, 7, 76, 6, 13, 5, 71, 3, 59, 2, 90, 4, 10, 1, 65, 
    8, 52, 9, 76, 0, 84, 7, 73, 5, 56, 4, 10, 6, 26, 2, 43, 3, 39, 1, 49, 
    9, 28, 8, 44, 7, 26, 6, 66, 4, 68, 5, 74, 3, 27, 2, 14, 1, 6, 0, 21, 
    0, 18, 1, 58, 3, 62, 2, 46, 6, 25, 4, 6, 5, 60, 7, 28, 8, 80, 9, 30, 
    0, 78, 1, 47, 7, 29, 5, 16, 4, 29, 6, 57, 3, 78, 2, 87, 8, 39, 9, 73, 
    9, 66, 8, 51, 3, 12, 7, 64, 5, 67, 4, 15, 6, 66, 2, 26, 1, 20, 0, 98, 
    8, 23, 9, 76, 6, 45, 7, 75, 5, 24, 3, 18, 4, 83, 2, 15, 1, 88, 0, 17, 
    9, 56, 8, 83, 7, 80, 6, 16, 4, 31, 5, 93, 3, 30, 2, 29, 1, 66, 0, 28, 
    9, 79, 8, 69, 2, 82, 4, 16, 5, 62, 3, 41, 6, 91, 7, 35, 0, 34, 1, 75, 
    0, 5, 1, 19, 2, 20, 3, 12, 4, 94, 5, 60, 6, 99, 7, 31, 8, 96, 9, 63
  };
  // trivial 10X10 instance from Bill Cook (BIC1)
  const int orb06[] = {
    10, 10, // Number of jobs and machines
    0, 99, 1, 74, 2, 49, 3, 67, 4, 17, 5, 7, 6, 9, 7, 39, 8, 35, 9, 49, 
    0, 49, 3, 67, 4, 82, 2, 92, 1, 62, 5, 84, 8, 45, 6, 30, 7, 42, 9, 71, 
    0, 26, 3, 33, 1, 82, 2, 98, 5, 83, 4, 16, 6, 64, 7, 65, 9, 36, 8, 77, 
    0, 41, 1, 62, 4, 73, 3, 94, 6, 51, 5, 46, 2, 55, 9, 31, 7, 64, 8, 46, 
    1, 68, 0, 26, 5, 50, 3, 46, 4, 25, 7, 88, 2, 6, 8, 13, 9, 98, 6, 84, 
    0, 24, 6, 80, 2, 91, 3, 55, 1, 48, 8, 99, 4, 72, 9, 91, 7, 84, 5, 12, 
    2, 16, 3, 13, 0, 9, 1, 58, 4, 23, 7, 85, 5, 36, 6, 89, 8, 71, 9, 41, 
    2, 54, 0, 41, 3, 38, 4, 53, 1, 11, 5, 74, 9, 88, 6, 46, 7, 41, 8, 65, 
    2, 53, 1, 50, 4, 40, 0, 90, 7, 7, 5, 80, 3, 57, 9, 60, 6, 91, 8, 47, 
    2, 45, 0, 59, 8, 81, 3, 99, 6, 71, 1, 19, 4, 75, 7, 77, 9, 94, 5, 95
  };
  // doomed 10x10 instance from Monika (MON1)
  const int orb07[] = {
    10, 10, // Number of jobs and machines
    0, 32, 1, 14, 2, 15, 3, 37, 4, 18, 5, 43, 6, 19, 7, 27, 8, 28, 9, 31, 
    0, 8, 3, 12, 4, 49, 8, 24, 9, 52, 6, 19, 7, 23, 5, 19, 2, 17, 1, 32, 
    0, 25, 7, 19, 3, 27, 2, 45, 6, 21, 4, 15, 1, 13, 5, 16, 8, 43, 9, 19, 
    0, 24, 1, 18, 3, 41, 8, 29, 5, 14, 2, 17, 4, 23, 7, 15, 6, 18, 9, 23, 
    0, 27, 6, 29, 1, 39, 3, 21, 5, 15, 2, 15, 9, 25, 7, 26, 8, 44, 4, 20, 
    4, 17, 0, 15, 6, 51, 8, 17, 2, 46, 3, 16, 9, 33, 7, 25, 5, 30, 1, 25, 
    4, 15, 3, 31, 0, 25, 9, 12, 2, 13, 1, 51, 8, 19, 7, 21, 6, 12, 5, 26, 
    4, 8, 6, 29, 9, 25, 3, 15, 8, 17, 2, 22, 7, 32, 5, 20, 0, 11, 1, 28, 
    2, 41, 6, 10, 8, 32, 7, 5, 4, 21, 9, 59, 3, 26, 1, 10, 5, 16, 0, 29, 
    2, 20, 9, 7, 5, 44, 8, 22, 6, 33, 3, 25, 7, 29, 4, 12, 1, 14, 0, 0
  };
  // deadlier 10x10 instance from Bruce Gamble (BRG2)
  const int orb08[] = {
    10, 10, // Number of jobs and machines
    0, 55, 1, 74, 2, 45, 3, 23, 4, 76, 5, 19, 6, 18, 7, 61, 8, 44, 9, 11, 
    0, 63, 1, 43, 2, 51, 3, 18, 4, 42, 7, 11, 6, 29, 5, 52, 9, 29, 8, 88, 
    2, 88, 1, 31, 0, 47, 4, 10, 3, 62, 5, 60, 6, 58, 7, 29, 8, 52, 9, 92, 
    2, 16, 1, 71, 0, 55, 4, 55, 3, 9, 7, 49, 6, 83, 5, 54, 9, 7, 8, 57, 
    1, 7, 0, 41, 4, 92, 3, 94, 2, 46, 6, 79, 5, 34, 9, 38, 8, 8, 7, 18, 
    1, 25, 0, 5, 4, 89, 3, 94, 2, 14, 6, 94, 5, 20, 9, 23, 8, 44, 7, 39, 
    1, 24, 2, 21, 4, 47, 0, 40, 3, 94, 6, 71, 7, 89, 9, 75, 5, 97, 8, 15, 
    1, 5, 2, 7, 4, 74, 0, 28, 3, 72, 5, 61, 7, 9, 8, 53, 9, 32, 6, 97, 
    0, 34, 2, 52, 3, 37, 4, 6, 1, 94, 6, 6, 7, 56, 9, 41, 5, 5, 8, 16, 
    0, 77, 2, 74, 3, 82, 4, 10, 1, 29, 5, 15, 7, 51, 8, 65, 9, 37, 6, 21
  };
  // deadly 10x10 instance from Bruce Shepherd (BRS2)
  const int orb09[] = {
    10, 10, // Number of jobs and machines
    0, 36, 1, 96, 2, 86, 3, 7, 4, 20, 5, 9, 6, 39, 7, 79, 8, 82, 9, 24, 
    0, 16, 8, 95, 3, 67, 5, 63, 2, 87, 1, 24, 9, 62, 6, 49, 7, 92, 4, 16, 
    0, 65, 6, 71, 1, 9, 3, 67, 8, 70, 2, 48, 5, 49, 4, 66, 7, 5, 9, 96, 
    0, 50, 1, 31, 4, 6, 8, 13, 5, 98, 7, 97, 6, 93, 9, 30, 3, 34, 2, 83, 
    0, 99, 5, 7, 9, 55, 8, 78, 7, 68, 4, 81, 3, 90, 2, 75, 1, 66, 6, 40, 
    1, 42, 5, 11, 0, 5, 9, 39, 2, 10, 8, 30, 3, 39, 6, 50, 7, 20, 4, 51, 
    0, 38, 3, 68, 8, 86, 7, 77, 9, 32, 2, 89, 4, 37, 6, 53, 1, 43, 5, 89, 
    0, 19, 9, 11, 4, 37, 1, 41, 5, 72, 7, 7, 2, 52, 6, 31, 3, 68, 8, 10, 
    0, 83, 1, 21, 9, 23, 7, 87, 8, 58, 6, 89, 4, 74, 5, 29, 2, 74, 3, 23, 
    0, 44, 2, 57, 4, 69, 9, 50, 1, 65, 5, 69, 8, 60, 3, 58, 6, 89, 7, 13
  };
  // 10x10 instance from George Steiner (GES2)
  const int orb10[] = {
    10, 10, // Number of jobs and machines
    9, 66, 8, 13, 0, 93, 7, 91, 6, 14, 5, 70, 3, 99, 2, 53, 4, 86, 1, 16, 
    8, 34, 9, 99, 0, 62, 7, 65, 5, 62, 4, 64, 6, 21, 2, 12, 3, 9, 1, 75, 
    9, 12, 8, 26, 7, 64, 6, 92, 4, 67, 5, 28, 3, 66, 2, 83, 1, 38, 0, 58, 
    0, 77, 1, 73, 3, 82, 2, 75, 6, 84, 4, 19, 5, 18, 7, 89, 8, 8, 9, 73, 
    0, 34, 1, 74, 7, 48, 5, 44, 4, 92, 6, 40, 3, 60, 2, 62, 8, 22, 9, 67, 
    9, 8, 8, 85, 3, 58, 7, 97, 5, 92, 4, 89, 6, 75, 2, 77, 1, 95, 0, 5, 
    8, 52, 9, 43, 6, 5, 7, 78, 5, 12, 3, 62, 4, 21, 2, 80, 1, 60, 0, 31, 
    9, 81, 8, 23, 7, 23, 6, 75, 4, 78, 5, 56, 3, 51, 2, 39, 1, 53, 0, 96, 
    9, 79, 8, 55, 2, 88, 4, 21, 5, 83, 3, 93, 6, 47, 7, 10, 0, 63, 1, 14, 
    0, 43, 1, 63, 2, 83, 3, 29, 4, 52, 5, 98, 6, 54, 7, 39, 8, 33, 9, 23
  };

  // Storer, Wu, and Vaccari hard 20x10 instance (Table 2, instance 1)
  const int swv01[] = {
    20, 10, // Number of jobs and machines
    3, 19, 2, 27, 1, 39, 4, 13, 0, 25, 8, 37, 9, 40, 5, 54, 7, 74, 6, 93, 
    2, 69, 0, 30, 4, 1, 3, 4, 1, 64, 7, 71, 5, 2, 9, 84, 6, 31, 8, 8, 
    4, 79, 3, 80, 0, 86, 2, 55, 1, 54, 8, 81, 6, 72, 7, 86, 5, 59, 9, 75, 
    2, 76, 3, 15, 1, 26, 0, 17, 4, 30, 8, 44, 7, 91, 6, 83, 5, 52, 9, 68, 
    4, 73, 3, 87, 1, 74, 0, 39, 2, 98, 9, 100, 5, 43, 8, 17, 7, 7, 6, 77, 
    1, 63, 0, 49, 2, 16, 3, 55, 4, 9, 9, 73, 5, 61, 8, 34, 6, 82, 7, 46, 
    0, 87, 1, 71, 4, 43, 3, 80, 2, 39, 7, 70, 8, 18, 6, 41, 9, 79, 5, 44, 
    4, 70, 2, 22, 0, 73, 3, 62, 1, 64, 5, 25, 8, 19, 6, 69, 9, 41, 7, 28, 
    3, 16, 0, 84, 1, 58, 4, 7, 2, 9, 5, 8, 6, 10, 7, 17, 8, 42, 9, 65, 
    3, 8, 0, 10, 1, 3, 4, 41, 2, 3, 7, 40, 8, 56, 5, 53, 9, 96, 6, 13, 
    4, 62, 1, 60, 3, 64, 2, 12, 0, 39, 5, 2, 7, 64, 6, 87, 9, 21, 8, 60, 
    2, 66, 1, 71, 3, 23, 4, 75, 0, 78, 7, 74, 6, 35, 9, 24, 8, 23, 5, 50, 
    1, 5, 3, 92, 4, 6, 0, 69, 2, 80, 7, 13, 5, 17, 9, 89, 6, 80, 8, 47, 
    0, 82, 3, 84, 1, 24, 2, 47, 4, 93, 7, 85, 5, 34, 6, 73, 8, 28, 9, 91, 
    4, 55, 0, 57, 3, 63, 2, 24, 1, 40, 7, 30, 6, 37, 5, 99, 8, 88, 9, 41, 
    1, 75, 2, 47, 3, 68, 0, 7, 4, 78, 7, 80, 6, 2, 9, 23, 8, 49, 5, 50, 
    0, 91, 4, 25, 2, 10, 1, 21, 3, 94, 8, 6, 7, 59, 5, 84, 9, 75, 6, 70, 
    2, 85, 1, 31, 0, 94, 4, 94, 3, 11, 5, 21, 9, 7, 6, 61, 8, 50, 7, 93, 
    1, 27, 0, 77, 4, 13, 2, 30, 3, 2, 5, 88, 7, 4, 9, 39, 6, 53, 8, 54, 
    1, 34, 2, 12, 3, 31, 0, 24, 4, 24, 7, 16, 5, 6, 9, 88, 8, 81, 6, 11
  };
  // Storer, Wu, and Vaccari hard 20x10 instance (Table 2, instance 2)
  const int swv02[] = {
    20, 10, // Number of jobs and machines
    2, 16, 1, 58, 0, 22, 4, 24, 3, 53, 8, 9, 9, 57, 7, 63, 5, 92, 6, 43, 
    3, 6, 1, 48, 4, 14, 0, 66, 2, 24, 7, 2, 9, 85, 6, 73, 8, 19, 5, 99, 
    4, 100, 2, 90, 0, 63, 1, 14, 3, 31, 5, 27, 9, 15, 8, 1, 6, 51, 7, 33, 
    2, 98, 3, 84, 4, 52, 0, 12, 1, 96, 9, 60, 6, 74, 8, 93, 5, 45, 7, 49, 
    4, 39, 0, 54, 2, 28, 3, 8, 1, 30, 8, 57, 6, 75, 5, 9, 7, 41, 9, 19, 
    3, 94, 0, 8, 2, 89, 1, 13, 4, 37, 8, 36, 6, 63, 9, 24, 5, 71, 7, 97, 
    3, 90, 2, 69, 1, 25, 4, 15, 0, 65, 7, 52, 6, 56, 9, 91, 8, 83, 5, 86, 
    3, 59, 1, 99, 4, 41, 0, 68, 2, 14, 7, 4, 9, 55, 6, 48, 8, 13, 5, 15, 
    4, 36, 2, 17, 1, 51, 0, 16, 3, 54, 8, 45, 5, 50, 7, 98, 6, 68, 9, 82, 
    1, 75, 0, 11, 4, 55, 2, 93, 3, 51, 6, 61, 9, 40, 7, 19, 8, 24, 5, 55, 
    4, 56, 0, 73, 3, 59, 2, 38, 1, 51, 6, 99, 8, 29, 9, 53, 5, 7, 7, 72, 
    3, 68, 4, 50, 1, 88, 2, 88, 0, 33, 5, 47, 8, 52, 6, 26, 9, 74, 7, 68, 
    2, 3, 3, 42, 0, 45, 1, 57, 4, 28, 5, 14, 8, 22, 9, 31, 6, 44, 7, 38, 
    3, 89, 0, 73, 4, 12, 1, 9, 2, 49, 5, 11, 8, 15, 7, 41, 9, 37, 6, 10, 
    3, 76, 2, 97, 4, 100, 1, 92, 0, 25, 5, 8, 9, 92, 7, 51, 6, 58, 8, 65, 
    4, 50, 0, 54, 3, 85, 1, 47, 2, 45, 6, 99, 9, 39, 5, 32, 8, 87, 7, 56, 
    0, 70, 2, 58, 3, 33, 1, 85, 4, 25, 8, 5, 7, 65, 9, 20, 6, 52, 5, 44, 
    1, 22, 3, 45, 4, 60, 0, 66, 2, 5, 7, 61, 6, 73, 9, 60, 5, 14, 8, 44, 
    4, 64, 0, 97, 2, 31, 1, 4, 3, 43, 9, 47, 7, 93, 6, 100, 5, 10, 8, 51, 
    3, 9, 4, 87, 2, 34, 0, 62, 1, 56, 5, 66, 8, 95, 7, 56, 9, 42, 6, 86
  };
  // Storer, Wu, and Vaccari hard 20x10 instance (Table 2, instance 3)
  const int swv03[] = {
    20, 10, // Number of jobs and machines
    2, 19, 0, 30, 1, 68, 4, 55, 3, 24, 8, 34, 7, 72, 5, 32, 9, 62, 6, 45, 
    2, 63, 1, 11, 4, 65, 3, 16, 0, 67, 9, 95, 8, 23, 7, 82, 6, 52, 5, 53, 
    2, 19, 4, 17, 1, 79, 3, 49, 0, 12, 7, 41, 9, 67, 8, 40, 6, 25, 5, 42, 
    0, 42, 2, 71, 3, 27, 4, 95, 1, 19, 5, 48, 8, 100, 6, 31, 7, 25, 9, 38, 
    3, 1, 1, 100, 4, 68, 0, 94, 2, 89, 5, 86, 7, 35, 9, 29, 8, 56, 6, 55, 
    4, 93, 1, 53, 2, 4, 3, 48, 0, 57, 8, 99, 7, 67, 5, 86, 6, 80, 9, 60, 
    4, 82, 1, 95, 2, 12, 0, 60, 3, 80, 8, 88, 7, 5, 6, 81, 9, 52, 5, 69, 
    3, 79, 1, 31, 4, 63, 0, 28, 2, 64, 8, 63, 5, 29, 7, 75, 9, 18, 6, 33, 
    4, 9, 1, 64, 2, 31, 0, 13, 3, 33, 9, 82, 6, 79, 5, 30, 7, 84, 8, 20, 
    2, 14, 0, 56, 1, 95, 4, 34, 3, 13, 6, 16, 5, 44, 7, 45, 8, 62, 9, 86, 
    4, 66, 3, 9, 2, 66, 1, 46, 0, 12, 5, 10, 7, 58, 6, 6, 8, 62, 9, 17, 
    4, 89, 1, 52, 2, 37, 3, 74, 0, 7, 8, 43, 5, 96, 7, 89, 6, 21, 9, 66, 
    1, 73, 3, 68, 2, 5, 4, 49, 0, 67, 9, 23, 7, 7, 5, 44, 8, 30, 6, 29, 
    2, 21, 0, 68, 1, 88, 4, 75, 3, 64, 6, 6, 8, 72, 7, 66, 9, 66, 5, 56, 
    1, 24, 4, 25, 2, 69, 0, 27, 3, 51, 9, 60, 8, 26, 6, 45, 5, 77, 7, 93, 
    2, 19, 3, 17, 1, 82, 4, 75, 0, 34, 5, 67, 9, 89, 6, 91, 7, 13, 8, 35, 
    4, 2, 0, 21, 3, 83, 1, 19, 2, 65, 6, 65, 8, 8, 9, 68, 7, 60, 5, 7, 
    1, 63, 3, 49, 2, 4, 4, 2, 0, 50, 9, 99, 5, 27, 6, 68, 8, 46, 7, 89, 
    0, 48, 4, 45, 3, 100, 2, 66, 1, 30, 6, 58, 7, 73, 9, 94, 5, 36, 8, 5, 
    2, 36, 0, 53, 4, 56, 3, 57, 1, 77, 9, 7, 6, 59, 8, 8, 5, 15, 7, 23
  };
  // Storer, Wu, and Vaccari hard 20x10 instance (Table 2, instance 4)
  const int swv04[] = {
    20, 10, // Number of jobs and machines
    2, 16, 0, 59, 4, 10, 3, 95, 1, 64, 8, 92, 9, 56, 7, 3, 5, 73, 6, 17, 
    1, 5, 4, 64, 3, 30, 2, 14, 0, 96, 9, 11, 8, 73, 7, 35, 6, 93, 5, 12, 
    3, 35, 4, 75, 0, 54, 1, 30, 2, 83, 9, 20, 8, 29, 7, 38, 6, 90, 5, 39, 
    4, 29, 3, 21, 0, 52, 2, 93, 1, 20, 5, 5, 7, 11, 8, 53, 9, 56, 6, 98, 
    0, 17, 3, 16, 4, 41, 1, 78, 2, 100, 5, 55, 8, 27, 6, 2, 7, 87, 9, 55, 
    3, 97, 1, 32, 4, 84, 2, 71, 0, 38, 9, 64, 7, 16, 5, 5, 6, 41, 8, 41, 
    3, 41, 1, 57, 4, 37, 0, 64, 2, 92, 6, 19, 9, 47, 7, 94, 8, 79, 5, 21, 
    0, 23, 3, 67, 1, 39, 4, 98, 2, 63, 8, 83, 5, 45, 6, 89, 9, 81, 7, 44, 
    1, 88, 0, 59, 3, 39, 2, 63, 4, 91, 8, 36, 5, 44, 6, 45, 9, 43, 7, 12, 
    2, 29, 1, 17, 0, 6, 3, 74, 4, 51, 9, 14, 6, 2, 5, 56, 7, 49, 8, 14, 
    3, 75, 2, 10, 4, 1, 0, 35, 1, 99, 7, 56, 5, 95, 9, 78, 6, 53, 8, 82, 
    0, 75, 2, 96, 1, 21, 3, 90, 4, 55, 6, 23, 7, 40, 9, 76, 8, 55, 5, 45, 
    3, 90, 4, 64, 0, 72, 2, 33, 1, 59, 7, 51, 6, 74, 5, 85, 9, 76, 8, 38, 
    3, 57, 1, 84, 2, 87, 4, 2, 0, 68, 8, 4, 5, 77, 6, 37, 7, 37, 9, 94, 
    1, 16, 3, 46, 4, 34, 2, 23, 0, 77, 7, 68, 8, 14, 9, 54, 5, 37, 6, 99, 
    4, 24, 1, 73, 2, 92, 0, 43, 3, 42, 5, 81, 7, 99, 8, 88, 9, 80, 6, 5, 
    1, 56, 2, 51, 0, 3, 4, 87, 3, 25, 5, 62, 7, 11, 8, 88, 6, 68, 9, 29, 
    2, 85, 3, 3, 4, 21, 0, 49, 1, 79, 8, 38, 5, 37, 9, 72, 7, 18, 6, 18, 
    0, 2, 3, 55, 1, 31, 2, 29, 4, 98, 5, 92, 6, 43, 8, 99, 7, 67, 9, 41, 
    4, 69, 3, 64, 0, 61, 1, 13, 2, 31, 5, 6, 8, 84, 9, 94, 7, 32, 6, 54
  };
  // Storer, Wu, and Vaccari hard 20x10 instance (Table 2, instance 5)
  const int swv05[] = {
    20, 10, // Number of jobs and machines
    2, 19, 1, 30, 3, 80, 0, 84, 4, 14, 8, 51, 5, 73, 6, 91, 7, 81, 9, 71, 
    2, 74, 4, 79, 1, 39, 0, 7, 3, 66, 9, 6, 5, 93, 8, 76, 6, 21, 7, 76, 
    4, 90, 3, 33, 1, 38, 2, 73, 0, 61, 8, 61, 7, 76, 5, 86, 9, 28, 6, 35, 
    4, 1, 3, 22, 2, 1, 0, 77, 1, 33, 6, 98, 5, 4, 9, 27, 8, 8, 7, 68, 
    2, 63, 4, 5, 1, 95, 0, 7, 3, 50, 8, 46, 9, 28, 6, 70, 5, 60, 7, 34, 
    0, 98, 1, 73, 4, 15, 3, 21, 2, 32, 7, 24, 9, 9, 8, 24, 5, 7, 6, 34, 
    3, 51, 4, 47, 2, 30, 1, 16, 0, 51, 5, 41, 6, 79, 7, 79, 9, 3, 8, 72, 
    4, 3, 1, 59, 0, 53, 3, 20, 2, 19, 6, 20, 9, 16, 7, 90, 5, 96, 8, 18, 
    1, 34, 2, 55, 3, 97, 0, 93, 4, 90, 7, 81, 5, 63, 8, 41, 6, 1, 9, 51, 
    4, 77, 3, 87, 1, 92, 2, 83, 0, 45, 7, 75, 9, 60, 6, 75, 5, 93, 8, 33, 
    0, 31, 2, 66, 1, 58, 4, 17, 3, 94, 5, 63, 7, 80, 9, 61, 6, 78, 8, 52, 
    4, 70, 1, 25, 2, 75, 0, 89, 3, 41, 7, 100, 5, 73, 6, 28, 8, 94, 9, 88, 
    1, 67, 4, 62, 3, 12, 2, 55, 0, 62, 5, 58, 8, 66, 7, 73, 6, 55, 9, 1, 
    4, 81, 0, 37, 1, 2, 3, 39, 2, 17, 7, 74, 6, 71, 8, 61, 5, 42, 9, 5, 
    3, 62, 0, 31, 4, 63, 2, 31, 1, 5, 9, 7, 7, 77, 8, 34, 6, 34, 5, 3, 
    0, 5, 2, 55, 3, 62, 1, 82, 4, 80, 6, 6, 8, 7, 7, 29, 5, 80, 9, 89, 
    3, 26, 1, 50, 2, 58, 0, 22, 4, 68, 7, 12, 6, 9, 9, 34, 5, 90, 8, 87, 
    0, 50, 2, 28, 1, 64, 4, 34, 3, 63, 7, 9, 9, 48, 6, 63, 8, 61, 5, 2, 
    0, 47, 2, 23, 1, 23, 4, 82, 3, 98, 7, 66, 6, 78, 8, 100, 9, 79, 5, 32, 
    1, 13, 4, 14, 0, 90, 2, 77, 3, 80, 9, 30, 7, 31, 5, 36, 6, 51, 8, 69
  };
  // Storer, Wu, and Vaccari hard 20x15 instance (Table 2, instance 6)
  const int swv06[] = {
    20, 15, // Number of jobs and machines
    1, 16, 6, 58, 2, 22, 4, 24, 5, 53, 3, 9, 0, 57, 10, 63, 8, 92, 12, 43, 7, 41, 13, 26, 14, 20, 9, 44, 11, 93, 
    2, 89, 1, 94, 0, 86, 3, 13, 6, 54, 4, 41, 5, 55, 7, 98, 13, 38, 14, 80, 9, 1, 11, 100, 12, 90, 10, 63, 8, 14, 
    1, 26, 6, 96, 3, 32, 4, 75, 5, 9, 0, 57, 2, 39, 12, 54, 14, 28, 10, 8, 11, 30, 13, 57, 9, 75, 7, 9, 8, 41, 
    3, 37, 2, 36, 5, 63, 0, 24, 6, 71, 1, 97, 4, 74, 14, 19, 12, 45, 8, 24, 11, 71, 13, 53, 10, 61, 9, 6, 7, 32, 
    3, 57, 0, 55, 1, 21, 5, 84, 2, 23, 6, 79, 4, 90, 11, 8, 14, 59, 10, 99, 9, 41, 12, 68, 8, 14, 13, 4, 7, 55, 
    4, 10, 2, 81, 1, 13, 3, 78, 0, 78, 5, 10, 6, 48, 9, 37, 11, 21, 7, 88, 12, 75, 14, 11, 13, 55, 10, 93, 8, 51, 
    6, 100, 2, 52, 3, 54, 1, 37, 5, 26, 4, 74, 0, 87, 8, 13, 12, 88, 10, 94, 14, 73, 7, 55, 11, 68, 9, 50, 13, 88, 
    4, 47, 5, 70, 6, 7, 2, 72, 0, 62, 3, 30, 1, 95, 10, 18, 9, 65, 7, 69, 13, 89, 8, 89, 14, 64, 12, 81, 11, 25, 
    6, 1, 1, 10, 0, 72, 3, 59, 4, 92, 5, 53, 2, 89, 14, 52, 7, 48, 8, 8, 13, 69, 10, 49, 9, 26, 12, 76, 11, 97, 
    6, 85, 2, 47, 4, 45, 1, 99, 0, 39, 5, 32, 3, 87, 10, 56, 8, 98, 11, 13, 7, 96, 12, 71, 14, 95, 9, 11, 13, 78, 
    0, 17, 2, 21, 3, 87, 6, 41, 5, 41, 4, 31, 1, 96, 8, 17, 11, 95, 13, 29, 14, 3, 10, 71, 7, 64, 9, 97, 12, 31, 
    6, 9, 0, 87, 4, 34, 1, 62, 3, 56, 5, 66, 2, 95, 9, 56, 14, 42, 8, 86, 7, 68, 12, 82, 10, 82, 13, 52, 11, 97, 
    3, 86, 1, 37, 2, 49, 0, 2, 6, 30, 5, 63, 4, 4, 14, 47, 8, 84, 10, 5, 13, 13, 9, 39, 12, 18, 7, 76, 11, 63, 
    0, 29, 6, 34, 1, 53, 3, 7, 5, 19, 4, 26, 2, 63, 12, 22, 10, 98, 13, 77, 14, 11, 7, 87, 9, 5, 11, 44, 8, 42, 
    6, 44, 4, 91, 1, 91, 2, 58, 0, 77, 3, 51, 5, 14, 13, 1, 9, 17, 7, 55, 12, 40, 8, 95, 14, 31, 11, 54, 10, 37, 
    5, 59, 4, 47, 1, 56, 6, 39, 2, 7, 0, 43, 3, 39, 13, 75, 10, 43, 12, 32, 9, 6, 11, 93, 7, 69, 8, 47, 14, 93, 
    4, 24, 1, 30, 3, 97, 6, 17, 0, 7, 2, 55, 5, 8, 7, 70, 10, 87, 8, 29, 12, 20, 13, 29, 11, 51, 9, 14, 14, 32, 
    2, 29, 4, 99, 3, 17, 0, 96, 1, 50, 5, 67, 6, 91, 10, 91, 13, 14, 12, 14, 7, 19, 8, 36, 11, 11, 14, 83, 9, 6, 
    0, 7, 6, 60, 3, 31, 5, 76, 1, 23, 2, 83, 4, 30, 8, 73, 14, 76, 11, 17, 10, 53, 13, 9, 12, 72, 7, 89, 9, 24, 
    3, 63, 0, 89, 2, 2, 1, 46, 6, 86, 5, 74, 4, 1, 7, 34, 9, 30, 12, 19, 13, 48, 11, 75, 8, 72, 14, 47, 10, 58
  };
  // Storer, Wu, and Vaccari hard 20x15 instance (Table 2, instance 7)
  const int swv07[] = {
    20, 15, // Number of jobs and machines
    3, 92, 1, 49, 2, 93, 6, 48, 0, 1, 4, 52, 5, 57, 8, 16, 12, 6, 13, 6, 11, 19, 9, 96, 7, 27, 14, 76, 10, 60, 
    5, 4, 3, 96, 6, 52, 1, 87, 2, 94, 4, 83, 0, 9, 11, 85, 10, 47, 8, 63, 9, 31, 13, 26, 12, 46, 7, 49, 14, 48, 
    1, 34, 6, 34, 4, 37, 2, 82, 0, 25, 5, 43, 3, 11, 9, 71, 14, 55, 7, 34, 11, 77, 12, 20, 8, 89, 10, 23, 13, 32, 
    3, 49, 5, 12, 6, 52, 2, 76, 0, 64, 1, 51, 4, 84, 10, 42, 12, 5, 7, 45, 8, 20, 11, 93, 14, 48, 13, 75, 9, 100, 
    2, 35, 1, 1, 3, 15, 6, 49, 5, 78, 4, 80, 0, 99, 9, 88, 7, 24, 11, 20, 10, 100, 8, 28, 14, 71, 13, 1, 12, 7, 
    3, 69, 6, 24, 5, 21, 4, 3, 1, 28, 2, 8, 0, 42, 10, 33, 11, 40, 9, 50, 8, 8, 13, 5, 12, 13, 7, 42, 14, 73, 
    0, 83, 4, 15, 2, 62, 6, 27, 5, 5, 1, 65, 3, 100, 14, 65, 10, 82, 7, 89, 13, 81, 9, 92, 8, 38, 11, 47, 12, 96, 
    6, 98, 4, 24, 2, 75, 0, 57, 1, 93, 3, 74, 5, 10, 7, 44, 13, 59, 11, 51, 12, 82, 14, 65, 10, 8, 8, 12, 9, 24, 
    4, 55, 0, 44, 3, 47, 5, 75, 2, 81, 6, 30, 1, 42, 10, 100, 8, 81, 7, 29, 13, 31, 9, 47, 11, 34, 12, 77, 14, 92, 
    2, 18, 5, 42, 0, 37, 4, 1, 3, 67, 6, 20, 1, 91, 8, 21, 14, 57, 12, 100, 10, 100, 11, 59, 13, 77, 9, 21, 7, 98, 
    3, 42, 1, 16, 4, 19, 6, 70, 2, 7, 0, 74, 5, 7, 12, 50, 9, 74, 8, 46, 14, 88, 13, 71, 10, 42, 7, 34, 11, 60, 
    6, 12, 4, 45, 2, 7, 0, 15, 1, 22, 3, 31, 5, 70, 13, 88, 9, 46, 8, 44, 14, 45, 12, 87, 11, 5, 7, 99, 10, 70, 
    4, 51, 5, 39, 0, 50, 2, 9, 3, 23, 6, 28, 1, 49, 13, 5, 12, 17, 14, 40, 10, 30, 11, 62, 8, 65, 7, 84, 9, 12, 
    6, 92, 0, 67, 5, 85, 1, 88, 3, 18, 4, 13, 2, 70, 7, 69, 14, 10, 13, 52, 8, 42, 11, 82, 10, 19, 12, 21, 9, 5, 
    4, 34, 0, 60, 1, 52, 5, 70, 2, 51, 6, 2, 3, 43, 10, 75, 11, 45, 8, 53, 12, 96, 13, 1, 14, 44, 7, 66, 9, 19, 
    6, 31, 1, 44, 0, 84, 3, 16, 4, 10, 2, 4, 5, 48, 13, 67, 14, 11, 12, 21, 8, 78, 7, 42, 11, 44, 9, 37, 10, 35, 
    1, 20, 4, 40, 3, 37, 2, 68, 6, 42, 0, 11, 5, 6, 10, 44, 11, 43, 12, 17, 14, 3, 7, 77, 13, 100, 9, 82, 8, 5, 
    5, 14, 0, 5, 3, 40, 1, 70, 4, 63, 2, 59, 6, 42, 9, 74, 13, 32, 7, 50, 10, 21, 14, 29, 12, 83, 11, 64, 8, 45, 
    6, 70, 0, 28, 3, 79, 4, 25, 5, 98, 2, 24, 1, 54, 12, 65, 13, 93, 10, 74, 7, 22, 9, 73, 11, 75, 8, 69, 14, 9, 
    5, 100, 2, 46, 4, 69, 3, 41, 1, 3, 6, 18, 0, 41, 8, 94, 11, 97, 12, 30, 14, 96, 7, 7, 9, 86, 13, 83, 10, 90
  };
  // Storer, Wu, and Vaccari hard 20x15 instance (Table 2, instance 8)
  const int swv08[] = {
    20, 15, // Number of jobs and machines
    3, 8, 4, 73, 2, 49, 5, 24, 6, 81, 1, 68, 0, 23, 12, 69, 8, 74, 10, 45, 11, 4, 14, 59, 9, 25, 7, 70, 13, 68, 
    3, 34, 2, 33, 5, 7, 1, 69, 4, 54, 6, 18, 0, 38, 8, 28, 12, 12, 14, 50, 10, 66, 7, 81, 9, 81, 13, 91, 11, 66, 
    0, 8, 6, 20, 3, 52, 4, 83, 5, 18, 2, 82, 1, 68, 7, 50, 14, 54, 11, 6, 10, 73, 13, 48, 9, 20, 8, 93, 12, 99, 
    2, 41, 0, 72, 1, 91, 4, 52, 5, 30, 3, 1, 6, 92, 13, 52, 8, 41, 9, 45, 14, 43, 12, 97, 10, 64, 11, 71, 7, 76, 
    0, 48, 1, 44, 5, 49, 6, 92, 3, 29, 2, 29, 4, 88, 14, 14, 10, 99, 8, 22, 13, 79, 9, 93, 12, 69, 11, 63, 7, 68, 
    0, 56, 6, 42, 2, 42, 3, 93, 1, 80, 4, 54, 5, 94, 12, 80, 14, 69, 11, 39, 8, 85, 10, 95, 13, 12, 9, 28, 7, 64, 
    0, 90, 4, 75, 6, 9, 1, 46, 2, 91, 3, 93, 5, 93, 14, 77, 9, 63, 11, 50, 12, 82, 13, 74, 8, 67, 7, 72, 10, 76, 
    0, 55, 2, 90, 6, 11, 3, 60, 4, 75, 1, 23, 5, 74, 11, 54, 7, 97, 12, 32, 13, 67, 10, 15, 14, 48, 8, 100, 9, 55, 
    6, 71, 5, 64, 2, 40, 0, 32, 3, 92, 1, 59, 4, 69, 13, 68, 14, 34, 12, 71, 8, 28, 9, 94, 7, 82, 10, 1, 11, 58, 
    6, 36, 4, 46, 1, 50, 5, 87, 3, 33, 2, 94, 0, 3, 14, 60, 11, 45, 13, 84, 9, 1, 8, 38, 10, 22, 12, 39, 7, 50, 
    1, 53, 0, 34, 5, 56, 6, 97, 3, 95, 4, 32, 2, 28, 14, 48, 7, 54, 12, 98, 8, 84, 9, 77, 10, 46, 13, 65, 11, 94, 
    2, 1, 5, 97, 0, 77, 4, 82, 6, 14, 1, 18, 3, 74, 14, 52, 11, 14, 12, 93, 9, 35, 8, 34, 13, 84, 10, 6, 7, 81, 
    1, 62, 0, 86, 2, 57, 6, 80, 5, 37, 3, 94, 4, 77, 7, 72, 9, 26, 11, 41, 10, 7, 8, 56, 13, 98, 14, 67, 12, 47, 
    5, 45, 3, 30, 0, 57, 6, 68, 1, 61, 2, 34, 4, 2, 7, 57, 13, 96, 9, 10, 12, 85, 14, 42, 10, 93, 8, 89, 11, 43, 
    6, 49, 4, 53, 1, 51, 2, 4, 0, 17, 5, 21, 3, 31, 10, 45, 13, 45, 9, 63, 11, 21, 8, 4, 7, 23, 14, 90, 12, 1, 
    6, 68, 5, 18, 0, 87, 3, 6, 4, 13, 2, 9, 1, 40, 8, 83, 7, 95, 12, 27, 10, 94, 14, 68, 11, 22, 13, 28, 9, 66, 
    2, 80, 6, 14, 0, 67, 5, 15, 1, 14, 3, 97, 4, 23, 8, 45, 10, 1, 11, 5, 14, 87, 7, 34, 12, 12, 9, 98, 13, 35, 
    4, 33, 2, 20, 3, 74, 6, 20, 5, 3, 0, 90, 1, 37, 13, 56, 12, 38, 8, 7, 14, 84, 9, 100, 11, 41, 10, 6, 7, 97, 
    6, 47, 4, 63, 3, 1, 0, 28, 2, 99, 1, 41, 5, 45, 14, 60, 13, 2, 7, 25, 8, 59, 9, 39, 10, 76, 11, 89, 12, 5, 
    6, 67, 2, 46, 3, 25, 1, 2, 5, 22, 4, 8, 0, 22, 13, 64, 7, 82, 12, 99, 11, 79, 10, 87, 8, 71, 9, 24, 14, 19
  };
  // Storer, Wu, and Vaccari hard 20x15 instance (Table 2, instance 9)
  const int swv09[] = {
    20, 15, // Number of jobs and machines
    5, 8, 3, 73, 0, 69, 2, 38, 6, 6, 4, 62, 1, 78, 9, 79, 8, 59, 13, 77, 11, 22, 10, 80, 12, 58, 14, 49, 7, 48, 
    3, 34, 4, 29, 2, 69, 0, 5, 5, 63, 1, 82, 6, 94, 14, 17, 11, 94, 9, 29, 10, 5, 13, 75, 7, 15, 8, 61, 12, 61, 
    1, 52, 2, 30, 0, 25, 6, 17, 3, 46, 4, 86, 5, 3, 14, 70, 11, 34, 9, 23, 10, 68, 13, 76, 8, 53, 12, 71, 7, 9, 
    2, 50, 4, 20, 3, 24, 0, 53, 1, 97, 5, 79, 6, 92, 14, 3, 12, 52, 10, 75, 9, 74, 8, 59, 7, 75, 13, 84, 11, 99, 
    2, 15, 0, 61, 3, 47, 4, 38, 6, 49, 5, 21, 1, 6, 11, 8, 8, 71, 14, 83, 13, 24, 12, 18, 9, 33, 7, 70, 10, 100, 
    4, 48, 5, 50, 2, 66, 0, 92, 6, 2, 3, 58, 1, 23, 9, 84, 8, 66, 10, 12, 7, 36, 14, 4, 12, 88, 13, 64, 11, 12, 
    3, 29, 0, 25, 6, 44, 5, 87, 2, 42, 1, 44, 4, 86, 8, 28, 10, 86, 9, 74, 14, 77, 13, 59, 12, 94, 7, 58, 11, 16, 
    4, 31, 3, 58, 0, 94, 5, 69, 2, 44, 1, 93, 6, 92, 9, 80, 8, 63, 12, 47, 13, 3, 7, 79, 11, 39, 14, 80, 10, 75, 
    1, 69, 2, 27, 0, 76, 5, 19, 6, 86, 3, 16, 4, 31, 12, 33, 9, 69, 13, 19, 10, 43, 14, 9, 11, 37, 7, 35, 8, 24, 
    2, 75, 3, 78, 6, 41, 4, 60, 5, 59, 0, 42, 1, 60, 12, 18, 8, 31, 10, 15, 7, 54, 14, 60, 9, 20, 11, 61, 13, 69, 
    4, 89, 6, 20, 1, 27, 5, 78, 3, 2, 2, 21, 0, 55, 13, 79, 11, 77, 10, 99, 9, 70, 12, 30, 7, 97, 8, 41, 14, 98, 
    6, 1, 2, 10, 4, 84, 5, 72, 0, 14, 1, 9, 3, 51, 7, 22, 14, 65, 10, 100, 13, 65, 11, 43, 8, 10, 12, 14, 9, 19, 
    5, 50, 2, 13, 3, 49, 6, 75, 1, 42, 0, 81, 4, 89, 9, 100, 14, 54, 13, 37, 10, 7, 11, 38, 8, 25, 12, 78, 7, 79, 
    2, 44, 3, 77, 5, 26, 1, 42, 4, 9, 6, 73, 0, 60, 9, 61, 10, 85, 12, 14, 11, 92, 7, 100, 14, 49, 8, 46, 13, 12, 
    2, 72, 0, 53, 1, 43, 5, 65, 6, 59, 4, 87, 3, 13, 8, 71, 12, 25, 9, 71, 10, 89, 11, 2, 7, 76, 14, 21, 13, 12, 
    2, 60, 6, 28, 5, 33, 1, 36, 0, 6, 3, 96, 4, 48, 9, 40, 11, 79, 10, 60, 8, 39, 13, 34, 7, 54, 12, 20, 14, 52, 
    5, 82, 2, 12, 3, 11, 4, 61, 1, 21, 0, 21, 6, 34, 12, 86, 14, 53, 8, 7, 9, 4, 7, 95, 10, 62, 13, 54, 11, 82, 
    5, 72, 0, 13, 3, 46, 6, 97, 1, 87, 4, 87, 2, 11, 7, 45, 14, 85, 11, 66, 8, 43, 9, 39, 13, 34, 10, 30, 12, 55, 
    1, 39, 5, 19, 0, 19, 4, 73, 6, 63, 3, 30, 2, 69, 9, 36, 7, 13, 10, 96, 12, 27, 13, 59, 14, 76, 11, 62, 8, 14, 
    1, 7, 4, 14, 3, 79, 2, 27, 6, 43, 0, 96, 5, 24, 11, 30, 7, 27, 12, 2, 8, 69, 14, 75, 13, 34, 10, 79, 9, 96
  };
  // Storer, Wu, and Vaccari hard 20x15 instance (Table 2, instance 10)
  const int swv10[] = {
    20, 15, // Number of jobs and machines
    3, 8, 2, 73, 1, 79, 0, 95, 6, 69, 4, 9, 5, 5, 8, 85, 9, 52, 11, 43, 14, 32, 7, 91, 10, 24, 13, 89, 12, 38, 
    6, 45, 1, 70, 4, 84, 3, 24, 5, 18, 0, 20, 2, 71, 8, 21, 7, 60, 9, 98, 10, 70, 13, 52, 12, 34, 11, 23, 14, 52, 
    6, 16, 4, 68, 1, 85, 0, 39, 5, 40, 2, 98, 3, 61, 10, 77, 7, 60, 11, 73, 9, 66, 14, 84, 8, 16, 13, 43, 12, 88, 
    0, 72, 1, 17, 3, 68, 4, 89, 2, 94, 6, 98, 5, 56, 10, 88, 13, 27, 9, 60, 12, 61, 8, 8, 7, 88, 11, 48, 14, 65, 
    6, 78, 2, 24, 5, 28, 0, 73, 4, 21, 1, 69, 3, 52, 14, 32, 8, 83, 11, 48, 10, 29, 13, 48, 12, 92, 9, 43, 7, 82, 
    4, 54, 6, 31, 5, 14, 3, 47, 0, 82, 1, 75, 2, 4, 8, 31, 12, 72, 7, 58, 9, 45, 13, 91, 14, 31, 11, 61, 10, 27, 
    4, 88, 1, 28, 5, 92, 6, 62, 3, 93, 0, 14, 2, 65, 7, 33, 9, 44, 8, 31, 14, 32, 11, 72, 13, 47, 12, 61, 10, 34, 
    0, 52, 1, 59, 5, 98, 3, 6, 2, 19, 6, 53, 4, 39, 8, 74, 12, 48, 10, 33, 13, 49, 11, 92, 7, 22, 14, 41, 9, 37, 
    0, 2, 6, 85, 3, 34, 2, 51, 4, 97, 5, 95, 1, 73, 14, 61, 9, 28, 12, 73, 8, 21, 11, 85, 7, 75, 13, 42, 10, 7, 
    5, 94, 1, 28, 0, 77, 2, 56, 6, 79, 4, 2, 3, 82, 9, 88, 10, 93, 12, 44, 14, 5, 8, 96, 7, 34, 13, 56, 11, 41, 
    2, 15, 5, 88, 6, 18, 3, 14, 1, 82, 0, 58, 4, 33, 13, 19, 10, 42, 9, 36, 14, 57, 12, 85, 7, 3, 11, 62, 8, 36, 
    3, 30, 6, 33, 0, 13, 4, 4, 2, 74, 1, 37, 5, 78, 14, 2, 13, 56, 9, 21, 10, 61, 11, 81, 7, 18, 8, 59, 12, 62, 
    5, 40, 1, 75, 6, 45, 0, 41, 3, 97, 2, 65, 4, 92, 7, 11, 12, 44, 8, 40, 9, 100, 11, 91, 14, 66, 13, 53, 10, 27, 
    1, 83, 2, 52, 0, 84, 3, 66, 5, 3, 6, 5, 4, 71, 13, 41, 10, 42, 11, 63, 12, 50, 14, 43, 8, 3, 9, 35, 7, 18, 
    4, 44, 0, 26, 1, 59, 6, 81, 2, 84, 5, 81, 3, 91, 13, 41, 7, 42, 11, 53, 8, 63, 14, 89, 9, 15, 10, 64, 12, 40, 
    1, 46, 0, 97, 5, 67, 4, 97, 3, 71, 6, 88, 2, 69, 14, 44, 12, 20, 11, 52, 13, 34, 10, 74, 8, 79, 7, 10, 9, 87, 
    3, 71, 6, 13, 4, 100, 2, 67, 1, 57, 5, 24, 0, 36, 7, 88, 14, 79, 8, 21, 9, 86, 12, 60, 11, 28, 10, 14, 13, 3, 
    0, 97, 6, 24, 2, 41, 4, 40, 1, 51, 5, 73, 3, 19, 9, 27, 12, 70, 13, 98, 10, 11, 11, 83, 7, 76, 8, 60, 14, 12, 
    5, 88, 3, 48, 1, 33, 4, 96, 6, 10, 0, 49, 2, 52, 10, 38, 13, 49, 7, 31, 12, 94, 14, 23, 9, 7, 11, 5, 8, 4, 
    2, 85, 0, 100, 5, 51, 6, 91, 1, 21, 3, 83, 4, 30, 12, 23, 9, 48, 8, 19, 11, 47, 10, 95, 7, 23, 14, 78, 13, 22
  };
  // Storer, Wu, and Vaccari hard 50x10 instance (Table 2, instance 11)
  const int swv11[] = {
    50, 10, // Number of jobs and machines
    0, 92, 4, 47, 3, 56, 2, 91, 1, 49, 5, 39, 9, 63, 7, 12, 6, 1, 8, 37, 
    0, 86, 2, 100, 1, 75, 3, 92, 4, 90, 5, 11, 7, 85, 8, 54, 9, 100, 6, 38, 
    1, 4, 4, 94, 3, 44, 2, 40, 0, 92, 8, 53, 6, 40, 9, 5, 5, 68, 7, 27, 
    4, 87, 0, 48, 1, 59, 2, 92, 3, 35, 6, 99, 7, 46, 9, 27, 8, 83, 5, 91, 
    0, 83, 1, 78, 4, 76, 3, 64, 2, 44, 8, 12, 9, 91, 6, 31, 7, 98, 5, 63, 
    3, 49, 0, 15, 1, 100, 4, 18, 2, 24, 6, 92, 9, 65, 5, 26, 7, 29, 8, 24, 
    0, 28, 3, 53, 4, 84, 2, 47, 1, 85, 7, 100, 5, 34, 6, 35, 8, 90, 9, 88, 
    2, 61, 4, 71, 3, 54, 1, 34, 0, 13, 9, 47, 8, 2, 6, 97, 7, 27, 5, 97, 
    0, 85, 2, 75, 1, 33, 4, 72, 3, 49, 7, 23, 5, 12, 8, 90, 6, 87, 9, 42, 
    2, 24, 3, 20, 1, 65, 4, 33, 0, 75, 9, 47, 6, 84, 8, 44, 7, 74, 5, 29, 
    2, 48, 3, 27, 4, 1, 0, 23, 1, 66, 6, 35, 7, 46, 9, 29, 5, 63, 8, 44, 
    2, 79, 0, 4, 4, 61, 3, 46, 1, 69, 7, 10, 8, 88, 9, 19, 6, 50, 5, 34, 
    0, 16, 4, 31, 3, 77, 2, 3, 1, 25, 8, 88, 7, 97, 9, 49, 6, 79, 5, 22, 
    1, 40, 0, 39, 4, 15, 2, 93, 3, 48, 6, 63, 9, 74, 8, 46, 7, 91, 5, 51, 
    4, 48, 0, 93, 2, 8, 3, 50, 1, 5, 6, 48, 7, 46, 9, 35, 5, 88, 8, 97, 
    3, 70, 1, 8, 2, 65, 0, 32, 4, 84, 8, 9, 6, 43, 7, 10, 5, 72, 9, 60, 
    0, 21, 2, 28, 1, 26, 3, 91, 4, 58, 9, 90, 6, 43, 8, 64, 5, 39, 7, 93, 
    1, 50, 2, 60, 0, 51, 4, 90, 3, 93, 7, 20, 9, 33, 8, 27, 6, 12, 5, 89, 
    1, 21, 3, 3, 2, 47, 4, 34, 0, 53, 9, 67, 8, 8, 5, 68, 7, 1, 6, 71, 
    3, 57, 4, 26, 2, 36, 0, 48, 1, 11, 9, 44, 7, 25, 5, 30, 8, 92, 6, 57, 
    1, 20, 0, 20, 4, 6, 3, 74, 2, 48, 9, 77, 8, 15, 5, 80, 7, 27, 6, 10, 
    3, 71, 1, 40, 0, 86, 2, 23, 4, 29, 7, 99, 8, 56, 6, 100, 9, 77, 5, 28, 
    4, 83, 0, 61, 3, 27, 1, 86, 2, 99, 7, 31, 5, 60, 8, 40, 9, 84, 6, 26, 
    4, 68, 1, 94, 3, 46, 2, 60, 0, 33, 7, 46, 5, 86, 9, 63, 6, 70, 8, 89, 
    4, 33, 1, 13, 2, 91, 3, 27, 0, 38, 8, 82, 7, 31, 6, 23, 9, 27, 5, 87, 
    4, 58, 3, 30, 0, 24, 2, 12, 1, 38, 8, 2, 9, 37, 5, 59, 6, 37, 7, 36, 
    2, 62, 1, 47, 4, 5, 3, 39, 0, 75, 7, 60, 9, 65, 8, 61, 6, 77, 5, 31, 
    4, 100, 0, 21, 1, 53, 3, 74, 2, 3, 8, 34, 6, 6, 7, 91, 9, 80, 5, 28, 
    1, 8, 0, 3, 2, 88, 3, 54, 4, 18, 9, 4, 6, 34, 5, 54, 8, 59, 7, 42, 
    3, 33, 4, 72, 0, 83, 2, 17, 1, 23, 6, 24, 8, 60, 9, 96, 7, 78, 5, 70, 
    4, 63, 2, 36, 3, 70, 0, 97, 1, 99, 6, 71, 9, 92, 5, 41, 8, 73, 7, 97, 
    2, 28, 1, 37, 4, 24, 0, 30, 3, 55, 8, 38, 5, 9, 9, 77, 7, 17, 6, 51, 
    3, 15, 0, 46, 2, 14, 4, 18, 1, 99, 9, 48, 6, 41, 5, 10, 7, 47, 8, 80, 
    4, 89, 3, 78, 2, 51, 1, 63, 0, 29, 7, 70, 9, 7, 5, 14, 8, 84, 6, 32, 
    4, 26, 1, 69, 2, 92, 3, 15, 0, 23, 8, 42, 6, 95, 5, 47, 9, 83, 7, 56, 
    1, 38, 2, 44, 3, 47, 4, 23, 0, 10, 9, 63, 7, 65, 6, 21, 5, 70, 8, 56, 
    3, 42, 4, 85, 1, 29, 0, 35, 2, 66, 9, 46, 8, 25, 5, 90, 7, 85, 6, 75, 
    3, 99, 0, 46, 4, 74, 2, 96, 1, 48, 5, 52, 6, 13, 7, 88, 8, 4, 9, 30, 
    1, 15, 3, 80, 4, 47, 2, 25, 0, 8, 9, 61, 7, 70, 8, 23, 6, 93, 5, 5, 
    0, 90, 2, 51, 3, 66, 4, 5, 1, 86, 5, 59, 6, 97, 9, 28, 7, 85, 8, 9, 
    0, 59, 1, 50, 4, 40, 3, 23, 2, 93, 7, 61, 9, 96, 8, 63, 6, 34, 5, 14, 
    1, 62, 2, 72, 4, 30, 0, 21, 3, 15, 5, 77, 6, 13, 7, 2, 8, 22, 9, 22, 
    2, 20, 4, 14, 3, 85, 1, 4, 0, 2, 9, 33, 7, 90, 5, 48, 8, 90, 6, 62, 
    0, 49, 3, 49, 4, 46, 1, 89, 2, 64, 9, 72, 8, 6, 5, 83, 6, 13, 7, 66, 
    4, 74, 1, 55, 2, 73, 0, 25, 3, 16, 7, 19, 9, 38, 6, 22, 5, 26, 8, 63, 
    3, 13, 2, 96, 1, 8, 0, 15, 4, 97, 6, 95, 7, 2, 5, 66, 8, 57, 9, 46, 
    4, 73, 1, 97, 3, 39, 0, 22, 2, 90, 9, 64, 6, 65, 8, 31, 5, 98, 7, 85, 
    3, 43, 2, 67, 0, 38, 1, 77, 4, 11, 7, 61, 5, 7, 9, 95, 8, 97, 6, 69, 
    0, 35, 2, 68, 1, 5, 3, 46, 4, 4, 7, 51, 6, 44, 5, 58, 9, 69, 8, 98, 
    2, 68, 1, 81, 0, 2, 3, 4, 4, 59, 9, 53, 8, 69, 5, 69, 6, 14, 7, 21
  };
  // Storer, Wu, and Vaccari hard 50x10 instance (Table 2, instance 12)
  const int swv12[] = {
    50, 10, // Number of jobs and machines
    0, 92, 4, 49, 1, 93, 3, 48, 2, 1, 7, 52, 6, 57, 9, 16, 5, 6, 8, 6, 
    4, 82, 3, 25, 2, 69, 1, 86, 0, 54, 6, 15, 5, 31, 9, 5, 7, 6, 8, 18, 
    0, 31, 1, 26, 3, 46, 2, 49, 4, 48, 8, 74, 7, 82, 5, 47, 9, 93, 6, 91, 
    0, 34, 4, 37, 1, 82, 3, 25, 2, 43, 6, 11, 9, 71, 5, 55, 7, 34, 8, 77, 
    4, 22, 0, 91, 3, 54, 2, 49, 1, 97, 9, 2, 7, 46, 5, 98, 6, 27, 8, 89, 
    2, 46, 3, 70, 1, 3, 0, 44, 4, 24, 9, 65, 6, 60, 5, 94, 8, 58, 7, 22, 
    3, 53, 0, 99, 1, 80, 2, 74, 4, 29, 6, 72, 7, 54, 5, 98, 8, 60, 9, 69, 
    3, 96, 1, 87, 0, 36, 2, 57, 4, 7, 8, 36, 9, 26, 5, 94, 6, 47, 7, 70, 
    3, 5, 2, 47, 1, 59, 0, 57, 4, 28, 9, 24, 8, 79, 6, 19, 5, 44, 7, 35, 
    0, 96, 1, 4, 3, 60, 2, 43, 4, 39, 7, 97, 5, 2, 9, 81, 6, 89, 8, 91, 
    2, 23, 4, 74, 3, 98, 0, 24, 1, 75, 9, 57, 8, 93, 6, 74, 5, 10, 7, 44, 
    3, 36, 4, 5, 2, 36, 0, 49, 1, 90, 8, 62, 5, 74, 9, 4, 6, 85, 7, 53, 
    2, 44, 1, 47, 3, 75, 4, 81, 0, 30, 7, 42, 8, 100, 9, 81, 6, 29, 5, 31, 
    1, 2, 0, 18, 3, 88, 2, 27, 4, 5, 5, 36, 7, 30, 6, 51, 8, 51, 9, 31, 
    1, 21, 0, 57, 3, 100, 2, 100, 4, 59, 8, 77, 7, 21, 5, 98, 6, 38, 9, 84, 
    4, 97, 2, 72, 1, 70, 3, 99, 0, 42, 6, 94, 5, 59, 9, 90, 8, 78, 7, 13, 
    3, 16, 2, 19, 1, 70, 0, 7, 4, 74, 6, 7, 5, 50, 9, 74, 8, 46, 7, 88, 
    3, 45, 4, 91, 2, 28, 0, 52, 1, 12, 5, 45, 6, 7, 7, 15, 9, 22, 8, 31, 
    3, 56, 2, 3, 1, 8, 4, 25, 0, 90, 8, 99, 6, 22, 9, 65, 7, 51, 5, 31, 
    0, 23, 3, 28, 1, 49, 2, 5, 4, 17, 7, 40, 9, 30, 5, 62, 8, 65, 6, 84, 
    2, 88, 0, 86, 4, 8, 1, 41, 3, 12, 6, 67, 9, 77, 5, 94, 7, 80, 8, 11, 
    4, 81, 3, 42, 0, 19, 2, 100, 1, 10, 5, 23, 9, 71, 8, 18, 6, 93, 7, 36, 
    4, 74, 2, 73, 3, 63, 1, 9, 0, 51, 8, 39, 7, 7, 6, 96, 5, 81, 9, 22, 
    1, 1, 3, 44, 0, 66, 4, 19, 2, 65, 7, 10, 6, 23, 8, 26, 9, 76, 5, 77, 
    1, 54, 2, 18, 4, 99, 0, 79, 3, 22, 5, 2, 6, 42, 8, 54, 7, 90, 9, 28, 
    3, 16, 4, 1, 1, 28, 0, 54, 2, 97, 5, 71, 6, 53, 8, 32, 7, 26, 9, 28, 
    0, 82, 3, 5, 2, 18, 4, 71, 1, 50, 5, 41, 7, 62, 9, 89, 6, 93, 8, 54, 
    2, 63, 3, 59, 0, 42, 1, 74, 4, 32, 5, 50, 6, 21, 7, 29, 8, 83, 9, 64, 
    4, 29, 2, 76, 1, 6, 3, 44, 0, 4, 9, 81, 5, 29, 7, 95, 8, 66, 6, 89, 
    3, 55, 4, 84, 1, 36, 0, 42, 2, 64, 5, 81, 8, 85, 6, 76, 7, 4, 9, 16, 
    4, 100, 0, 46, 1, 69, 3, 41, 2, 3, 6, 18, 5, 41, 7, 94, 8, 97, 9, 30, 
    3, 34, 4, 35, 2, 18, 1, 58, 0, 98, 9, 78, 8, 17, 5, 53, 6, 85, 7, 86, 
    4, 68, 2, 89, 1, 99, 0, 3, 3, 92, 5, 10, 6, 52, 7, 30, 8, 66, 9, 69, 
    0, 21, 3, 65, 4, 19, 2, 14, 1, 76, 9, 84, 6, 45, 5, 24, 8, 54, 7, 73, 
    4, 47, 0, 68, 2, 87, 3, 92, 1, 96, 6, 29, 5, 90, 8, 29, 7, 39, 9, 100, 
    2, 35, 0, 60, 4, 61, 1, 61, 3, 72, 9, 57, 8, 94, 5, 77, 7, 1, 6, 53, 
    3, 85, 2, 38, 0, 79, 4, 43, 1, 71, 6, 44, 5, 87, 8, 61, 7, 51, 9, 37, 
    1, 100, 2, 33, 3, 94, 0, 59, 4, 25, 5, 88, 9, 50, 6, 19, 8, 4, 7, 66, 
    2, 8, 0, 85, 1, 80, 4, 75, 3, 1, 7, 17, 9, 32, 6, 60, 5, 30, 8, 57, 
    4, 25, 2, 98, 1, 94, 3, 49, 0, 34, 9, 37, 7, 80, 6, 50, 8, 25, 5, 72, 
    3, 51, 4, 49, 1, 53, 2, 7, 0, 73, 6, 96, 7, 19, 9, 41, 5, 55, 8, 42, 
    0, 57, 1, 86, 2, 1, 4, 61, 3, 66, 6, 28, 5, 56, 7, 68, 8, 21, 9, 65, 
    2, 98, 1, 100, 0, 47, 4, 28, 3, 4, 7, 34, 9, 55, 5, 32, 6, 72, 8, 66, 
    4, 2, 0, 74, 2, 20, 1, 39, 3, 63, 5, 88, 9, 3, 7, 22, 6, 8, 8, 73, 
    2, 44, 0, 1, 3, 52, 1, 43, 4, 4, 6, 36, 9, 75, 8, 58, 5, 61, 7, 38, 
    2, 21, 4, 6, 3, 32, 1, 74, 0, 57, 5, 72, 8, 10, 9, 34, 6, 91, 7, 94, 
    4, 26, 0, 59, 3, 53, 1, 45, 2, 23, 5, 55, 8, 12, 7, 34, 6, 98, 9, 43, 
    2, 4, 1, 53, 4, 57, 3, 95, 0, 6, 6, 30, 8, 1, 7, 92, 9, 20, 5, 86, 
    1, 98, 2, 77, 3, 65, 4, 51, 0, 85, 7, 23, 6, 79, 5, 30, 8, 41, 9, 17, 
    4, 58, 2, 43, 3, 14, 0, 74, 1, 64, 7, 37, 8, 78, 6, 33, 9, 42, 5, 80
  };
  // Storer, Wu, and Vaccari hard 50x10 instance (Table 2, instance 13)
  const int swv13[] = {
    50, 10, // Number of jobs and machines
    4, 68, 1, 39, 2, 79, 0, 72, 3, 65, 5, 82, 7, 33, 6, 82, 8, 66, 9, 55, 
    2, 14, 3, 45, 0, 18, 4, 72, 1, 27, 7, 57, 6, 90, 8, 19, 9, 19, 5, 50, 
    4, 25, 1, 77, 0, 64, 3, 18, 2, 19, 8, 27, 6, 97, 9, 81, 7, 65, 5, 11, 
    3, 70, 0, 29, 2, 31, 1, 39, 4, 62, 8, 12, 9, 2, 5, 91, 7, 98, 6, 91, 
    2, 90, 4, 51, 3, 38, 1, 27, 0, 29, 6, 67, 8, 95, 9, 60, 7, 86, 5, 64, 
    4, 90, 0, 55, 3, 69, 1, 76, 2, 97, 7, 94, 5, 57, 8, 65, 9, 80, 6, 24, 
    1, 23, 4, 13, 0, 90, 3, 24, 2, 41, 8, 69, 7, 8, 5, 81, 6, 94, 9, 76, 
    3, 19, 1, 37, 0, 16, 4, 4, 2, 68, 6, 45, 8, 79, 9, 4, 7, 30, 5, 33, 
    2, 36, 0, 76, 3, 97, 4, 71, 1, 19, 9, 87, 6, 97, 8, 64, 5, 84, 7, 43, 
    2, 20, 1, 77, 0, 71, 3, 73, 4, 47, 7, 88, 5, 100, 9, 16, 8, 69, 6, 77, 
    3, 55, 4, 96, 0, 8, 2, 61, 1, 40, 8, 46, 7, 29, 9, 71, 5, 89, 6, 59, 
    2, 21, 0, 18, 3, 37, 4, 97, 1, 59, 7, 79, 6, 2, 5, 80, 8, 85, 9, 59, 
    4, 19, 1, 83, 2, 1, 0, 95, 3, 48, 9, 37, 7, 59, 5, 56, 8, 57, 6, 81, 
    0, 8, 1, 60, 4, 91, 3, 85, 2, 27, 9, 39, 5, 31, 6, 62, 7, 94, 8, 12, 
    4, 2, 3, 10, 0, 17, 1, 38, 2, 96, 6, 21, 9, 81, 8, 64, 5, 76, 7, 46, 
    2, 46, 1, 4, 4, 25, 3, 41, 0, 11, 5, 96, 9, 56, 6, 10, 7, 25, 8, 32, 
    0, 21, 1, 77, 4, 22, 2, 72, 3, 53, 9, 28, 7, 23, 5, 2, 8, 52, 6, 83, 
    3, 9, 4, 37, 0, 2, 2, 74, 1, 15, 8, 26, 5, 83, 6, 90, 7, 51, 9, 80, 
    3, 6, 1, 7, 0, 57, 2, 4, 4, 56, 7, 11, 5, 57, 8, 12, 6, 94, 9, 29, 
    1, 40, 2, 93, 3, 65, 4, 66, 0, 96, 9, 5, 7, 32, 8, 85, 5, 93, 6, 94, 
    1, 38, 2, 19, 4, 22, 0, 73, 3, 7, 5, 63, 8, 28, 6, 23, 9, 11, 7, 84, 
    1, 96, 4, 10, 0, 29, 3, 59, 2, 94, 5, 26, 7, 22, 8, 52, 6, 37, 9, 50, 
    1, 38, 3, 31, 2, 76, 0, 8, 4, 8, 6, 50, 5, 95, 8, 5, 9, 25, 7, 62, 
    0, 15, 2, 84, 4, 100, 3, 76, 1, 66, 7, 56, 5, 95, 8, 94, 6, 56, 9, 85, 
    3, 73, 2, 38, 1, 84, 0, 42, 4, 37, 5, 16, 7, 24, 9, 59, 6, 60, 8, 23, 
    3, 43, 1, 79, 0, 80, 2, 44, 4, 65, 5, 81, 7, 7, 8, 93, 6, 55, 9, 34, 
    2, 8, 4, 2, 0, 12, 3, 55, 1, 60, 9, 91, 6, 6, 5, 83, 8, 31, 7, 91, 
    0, 8, 4, 46, 3, 47, 2, 57, 1, 47, 9, 55, 8, 74, 7, 98, 6, 54, 5, 51, 
    2, 56, 4, 90, 1, 41, 0, 35, 3, 62, 7, 4, 5, 15, 9, 89, 6, 73, 8, 66, 
    0, 2, 4, 39, 3, 44, 1, 68, 2, 54, 7, 7, 8, 76, 9, 29, 5, 90, 6, 53, 
    2, 34, 0, 94, 3, 1, 1, 23, 4, 45, 8, 83, 7, 84, 5, 49, 6, 67, 9, 49, 
    4, 4, 2, 70, 1, 19, 0, 19, 3, 92, 5, 70, 7, 33, 9, 50, 8, 82, 6, 48, 
    4, 64, 2, 76, 0, 70, 3, 83, 1, 91, 7, 98, 8, 37, 5, 3, 9, 75, 6, 92, 
    3, 96, 1, 17, 0, 20, 4, 13, 2, 28, 7, 21, 9, 65, 5, 87, 6, 54, 8, 98, 
    0, 68, 4, 40, 3, 98, 2, 90, 1, 38, 7, 45, 8, 21, 5, 9, 9, 3, 6, 47, 
    0, 58, 4, 19, 2, 16, 3, 74, 1, 32, 9, 32, 5, 58, 6, 93, 7, 1, 8, 80, 
    0, 32, 2, 99, 1, 95, 3, 2, 4, 8, 9, 55, 6, 32, 8, 26, 5, 6, 7, 68, 
    3, 7, 4, 45, 2, 19, 0, 97, 1, 56, 7, 22, 9, 72, 8, 98, 5, 59, 6, 20, 
    2, 97, 4, 98, 3, 43, 0, 28, 1, 23, 5, 3, 8, 75, 9, 43, 7, 58, 6, 71, 
    3, 31, 0, 88, 2, 88, 1, 82, 4, 65, 5, 53, 9, 15, 7, 68, 6, 60, 8, 99, 
    4, 4, 0, 100, 2, 95, 1, 11, 3, 28, 5, 80, 7, 25, 9, 87, 6, 25, 8, 9, 
    0, 75, 3, 10, 4, 59, 2, 80, 1, 60, 5, 75, 8, 87, 6, 33, 9, 10, 7, 31, 
    0, 54, 3, 6, 4, 7, 1, 72, 2, 49, 7, 72, 8, 64, 6, 32, 9, 86, 5, 69, 
    4, 15, 3, 19, 1, 18, 0, 84, 2, 96, 9, 71, 8, 64, 6, 38, 5, 58, 7, 62, 
    1, 32, 4, 80, 2, 83, 3, 83, 0, 50, 5, 81, 7, 82, 9, 33, 8, 10, 6, 55, 
    0, 65, 4, 95, 3, 84, 2, 64, 1, 18, 9, 27, 6, 70, 7, 74, 5, 87, 8, 68, 
    1, 50, 2, 49, 0, 96, 3, 1, 4, 89, 8, 42, 5, 88, 9, 91, 6, 64, 7, 3, 
    3, 44, 0, 91, 1, 5, 2, 100, 4, 77, 6, 20, 5, 13, 7, 25, 9, 71, 8, 71, 
    0, 86, 4, 91, 1, 19, 2, 69, 3, 71, 5, 13, 8, 87, 6, 98, 9, 43, 7, 13, 
    4, 8, 0, 60, 3, 31, 2, 93, 1, 8, 9, 1, 7, 19, 6, 8, 5, 85, 8, 24
  };
  // Storer, Wu, and Vaccari hard 50x10 instance (Table 2, instance 14)
  const int swv14[] = {
    50, 10, // Number of jobs and machines
    4, 69, 0, 37, 3, 64, 1, 1, 2, 65, 9, 34, 5, 67, 8, 43, 7, 72, 6, 79, 
    1, 11, 0, 7, 3, 68, 4, 43, 2, 52, 6, 29, 9, 71, 7, 81, 8, 12, 5, 36, 
    4, 90, 3, 29, 1, 1, 2, 1, 0, 14, 8, 38, 5, 13, 9, 21, 7, 41, 6, 97, 
    1, 46, 0, 26, 4, 83, 2, 36, 3, 20, 9, 4, 8, 23, 7, 65, 5, 56, 6, 42, 
    4, 46, 0, 39, 2, 92, 3, 53, 1, 62, 9, 68, 7, 65, 8, 74, 6, 87, 5, 46, 
    4, 13, 1, 44, 3, 43, 2, 67, 0, 75, 6, 5, 9, 94, 5, 95, 7, 28, 8, 85, 
    1, 1, 2, 99, 4, 36, 3, 86, 0, 65, 8, 32, 5, 17, 7, 71, 6, 15, 9, 61, 
    2, 18, 4, 63, 3, 15, 0, 59, 1, 33, 7, 95, 5, 63, 6, 85, 8, 34, 9, 3, 
    4, 13, 2, 25, 0, 82, 3, 23, 1, 26, 7, 22, 9, 35, 8, 16, 6, 24, 5, 41, 
    3, 1, 1, 7, 0, 21, 2, 73, 4, 39, 6, 32, 7, 77, 5, 29, 8, 89, 9, 21, 
    1, 53, 3, 27, 4, 55, 0, 16, 2, 64, 5, 78, 9, 32, 8, 60, 7, 20, 6, 20, 
    1, 71, 2, 54, 3, 21, 0, 20, 4, 23, 9, 40, 5, 99, 7, 61, 6, 94, 8, 71, 
    2, 76, 4, 72, 3, 91, 0, 75, 1, 7, 6, 53, 8, 32, 7, 71, 5, 63, 9, 53, 
    2, 12, 1, 3, 4, 35, 0, 64, 3, 30, 5, 94, 8, 67, 7, 31, 6, 79, 9, 14, 
    4, 63, 1, 28, 3, 87, 0, 89, 2, 52, 8, 2, 9, 21, 7, 92, 6, 44, 5, 37, 
    0, 79, 1, 65, 4, 35, 3, 78, 2, 17, 8, 90, 5, 54, 9, 91, 7, 57, 6, 23, 
    3, 20, 1, 93, 4, 61, 0, 76, 2, 23, 5, 10, 8, 34, 7, 20, 9, 87, 6, 77, 
    0, 37, 2, 17, 1, 92, 4, 30, 3, 59, 5, 47, 8, 7, 7, 45, 6, 13, 9, 60, 
    4, 90, 3, 74, 0, 46, 2, 36, 1, 2, 6, 9, 5, 83, 8, 90, 7, 88, 9, 39, 
    3, 83, 0, 85, 2, 20, 4, 88, 1, 94, 6, 14, 5, 16, 7, 62, 9, 53, 8, 9, 
    0, 4, 4, 16, 2, 64, 1, 60, 3, 79, 5, 37, 6, 49, 7, 67, 9, 95, 8, 5, 
    3, 32, 0, 86, 1, 5, 4, 66, 2, 77, 7, 15, 5, 68, 9, 40, 8, 1, 6, 4, 
    0, 2, 1, 48, 4, 23, 3, 25, 2, 58, 9, 55, 7, 14, 8, 21, 6, 85, 5, 27, 
    1, 71, 4, 92, 3, 99, 2, 56, 0, 81, 7, 79, 6, 66, 9, 42, 8, 47, 5, 43, 
    1, 77, 4, 85, 3, 72, 2, 19, 0, 71, 5, 34, 7, 9, 9, 14, 6, 62, 8, 58, 
    4, 38, 0, 3, 2, 61, 3, 98, 1, 76, 5, 14, 9, 56, 8, 26, 7, 43, 6, 44, 
    1, 68, 4, 54, 0, 62, 2, 93, 3, 22, 6, 57, 7, 79, 9, 19, 5, 77, 8, 45, 
    2, 62, 1, 96, 4, 56, 0, 68, 3, 24, 5, 41, 6, 19, 7, 2, 8, 73, 9, 50, 
    2, 86, 0, 53, 3, 3, 1, 89, 4, 37, 7, 100, 5, 59, 9, 23, 6, 19, 8, 35, 
    3, 90, 4, 94, 0, 21, 2, 78, 1, 85, 5, 94, 6, 90, 8, 28, 9, 92, 7, 56, 
    4, 85, 2, 97, 0, 8, 3, 27, 1, 86, 9, 26, 7, 5, 8, 96, 5, 68, 6, 57, 
    0, 58, 3, 4, 4, 49, 2, 1, 1, 79, 8, 10, 6, 44, 9, 87, 5, 16, 7, 13, 
    3, 85, 0, 24, 4, 23, 1, 41, 2, 59, 8, 20, 6, 52, 5, 58, 9, 75, 7, 77, 
    0, 47, 1, 89, 2, 68, 4, 88, 3, 17, 6, 48, 8, 84, 9, 100, 5, 92, 7, 47, 
    1, 30, 0, 1, 3, 61, 4, 20, 2, 73, 8, 78, 7, 41, 9, 52, 5, 43, 6, 74, 
    0, 11, 4, 58, 3, 66, 2, 67, 1, 18, 8, 42, 7, 88, 9, 49, 5, 62, 6, 71, 
    4, 5, 2, 51, 3, 67, 1, 20, 0, 11, 7, 37, 6, 42, 8, 25, 9, 57, 5, 1, 
    0, 58, 4, 83, 2, 9, 3, 68, 1, 21, 6, 28, 9, 77, 5, 19, 7, 32, 8, 66, 
    3, 85, 2, 58, 0, 65, 1, 80, 4, 50, 7, 79, 5, 43, 8, 29, 9, 9, 6, 18, 
    3, 74, 2, 29, 0, 11, 1, 23, 4, 34, 7, 84, 8, 57, 5, 77, 6, 83, 9, 82, 
    2, 6, 4, 67, 0, 97, 3, 66, 1, 21, 8, 90, 9, 46, 6, 12, 5, 17, 7, 96, 
    4, 34, 1, 5, 2, 13, 0, 100, 3, 12, 8, 63, 7, 59, 5, 75, 6, 91, 9, 89, 
    1, 30, 2, 66, 0, 33, 3, 70, 4, 16, 6, 80, 5, 58, 8, 8, 7, 86, 9, 66, 
    3, 55, 0, 46, 2, 1, 1, 77, 4, 19, 7, 85, 9, 32, 6, 59, 5, 37, 8, 69, 
    2, 3, 0, 16, 1, 48, 4, 8, 3, 51, 7, 72, 6, 19, 8, 58, 9, 59, 5, 94, 
    3, 30, 4, 23, 1, 92, 0, 18, 2, 19, 9, 32, 6, 57, 5, 50, 7, 64, 8, 27, 
    2, 18, 0, 72, 4, 92, 1, 6, 3, 67, 8, 100, 6, 32, 9, 14, 5, 51, 7, 55, 
    4, 48, 0, 87, 1, 96, 2, 58, 3, 83, 8, 77, 5, 26, 7, 77, 9, 72, 6, 86, 
    1, 80, 4, 5, 0, 50, 3, 65, 2, 85, 7, 88, 5, 47, 6, 33, 8, 50, 9, 75, 
    1, 78, 0, 96, 4, 80, 3, 5, 2, 99, 9, 58, 5, 38, 7, 29, 8, 69, 6, 44
  };
  // Storer, Wu, and Vaccari hard 50x10 instance (Table 2, instance 15)
  const int swv15[] = {
    50, 10, // Number of jobs and machines
    2, 93, 4, 40, 0, 1, 3, 77, 1, 77, 5, 16, 9, 74, 8, 11, 6, 51, 7, 92, 
    0, 92, 4, 80, 1, 76, 3, 59, 2, 70, 5, 86, 9, 17, 6, 78, 7, 30, 8, 93, 
    1, 44, 2, 92, 3, 96, 4, 77, 0, 53, 9, 10, 7, 49, 5, 84, 8, 59, 6, 14, 
    1, 60, 2, 19, 3, 76, 0, 73, 4, 85, 7, 13, 8, 93, 5, 68, 9, 50, 6, 78, 
    2, 20, 0, 24, 3, 41, 1, 2, 4, 4, 9, 44, 7, 79, 8, 81, 5, 16, 6, 39, 
    3, 41, 2, 35, 1, 32, 4, 18, 0, 15, 8, 98, 6, 29, 5, 19, 7, 14, 9, 26, 
    1, 59, 0, 45, 4, 53, 3, 44, 2, 98, 5, 84, 6, 23, 7, 45, 8, 39, 9, 89, 
    1, 30, 4, 51, 3, 25, 0, 51, 2, 84, 6, 60, 5, 45, 7, 89, 8, 25, 9, 97, 
    0, 47, 3, 18, 2, 40, 4, 62, 1, 58, 5, 36, 7, 93, 8, 77, 9, 90, 6, 15, 
    3, 33, 1, 68, 0, 41, 4, 72, 2, 20, 6, 69, 7, 47, 5, 22, 9, 47, 8, 22, 
    2, 28, 1, 100, 4, 20, 0, 35, 3, 26, 5, 24, 9, 41, 6, 42, 7, 100, 8, 32, 
    0, 65, 2, 12, 4, 53, 3, 93, 1, 40, 8, 18, 7, 23, 5, 60, 6, 89, 9, 53, 
    0, 58, 1, 60, 4, 97, 3, 31, 2, 50, 9, 85, 5, 64, 7, 38, 6, 85, 8, 35, 
    3, 64, 0, 58, 1, 49, 2, 45, 4, 9, 8, 49, 6, 22, 5, 99, 9, 15, 7, 7, 
    0, 10, 4, 85, 3, 72, 2, 37, 1, 77, 5, 70, 7, 45, 9, 8, 6, 83, 8, 57, 
    4, 93, 0, 87, 1, 87, 2, 18, 3, 4, 8, 78, 5, 67, 9, 20, 6, 17, 7, 35, 
    4, 72, 0, 56, 3, 57, 2, 15, 1, 45, 6, 41, 5, 40, 9, 85, 8, 32, 7, 81, 
    0, 36, 3, 63, 4, 79, 2, 32, 1, 5, 6, 25, 7, 86, 9, 91, 5, 21, 8, 35, 
    2, 83, 4, 29, 0, 9, 1, 38, 3, 73, 7, 50, 9, 99, 5, 18, 8, 29, 6, 41, 
    0, 100, 3, 29, 2, 60, 4, 63, 1, 64, 8, 71, 6, 35, 5, 26, 9, 9, 7, 22, 
    1, 81, 0, 60, 3, 62, 4, 48, 2, 68, 7, 28, 5, 69, 8, 92, 6, 79, 9, 10, 
    0, 40, 4, 80, 1, 41, 2, 10, 3, 68, 8, 28, 9, 51, 7, 33, 6, 82, 5, 25, 
    4, 30, 2, 12, 0, 35, 3, 17, 1, 70, 9, 29, 7, 18, 8, 93, 6, 94, 5, 37, 
    1, 36, 2, 41, 3, 27, 4, 36, 0, 78, 7, 64, 6, 88, 5, 25, 9, 92, 8, 66, 
    2, 65, 3, 27, 4, 74, 0, 32, 1, 40, 5, 88, 8, 73, 6, 92, 7, 83, 9, 42, 
    0, 48, 1, 85, 2, 92, 4, 95, 3, 61, 8, 72, 9, 76, 5, 58, 7, 11, 6, 89, 
    3, 84, 2, 50, 0, 70, 4, 24, 1, 42, 9, 55, 5, 100, 6, 70, 7, 4, 8, 68, 
    0, 95, 4, 41, 2, 11, 3, 98, 1, 85, 5, 64, 6, 8, 7, 26, 8, 6, 9, 6, 
    0, 84, 2, 49, 1, 17, 3, 69, 4, 55, 8, 75, 6, 45, 9, 38, 7, 59, 5, 28, 
    2, 48, 0, 29, 4, 1, 1, 64, 3, 41, 5, 23, 7, 64, 9, 31, 6, 56, 8, 12, 
    2, 81, 4, 25, 3, 33, 0, 22, 1, 50, 5, 74, 9, 56, 8, 33, 7, 85, 6, 83, 
    1, 62, 4, 25, 0, 21, 2, 20, 3, 8, 6, 36, 9, 9, 5, 91, 8, 90, 7, 49, 
    1, 43, 0, 16, 2, 91, 3, 96, 4, 24, 5, 11, 9, 91, 7, 41, 8, 35, 6, 66, 
    1, 91, 2, 20, 4, 44, 0, 42, 3, 87, 9, 57, 6, 15, 5, 38, 8, 42, 7, 89, 
    0, 33, 3, 95, 4, 68, 2, 22, 1, 80, 7, 53, 8, 13, 9, 70, 5, 22, 6, 69, 
    0, 15, 3, 47, 1, 24, 2, 31, 4, 41, 8, 14, 9, 28, 7, 59, 5, 52, 6, 39, 
    2, 95, 0, 42, 4, 5, 1, 57, 3, 67, 6, 30, 9, 21, 8, 70, 5, 9, 7, 20, 
    2, 54, 0, 15, 1, 20, 3, 64, 4, 83, 9, 40, 7, 6, 5, 89, 6, 91, 8, 48, 
    0, 22, 4, 27, 1, 77, 3, 25, 2, 16, 8, 72, 9, 61, 6, 75, 7, 4, 5, 19, 
    3, 68, 1, 82, 2, 16, 0, 83, 4, 2, 7, 10, 8, 88, 5, 41, 9, 21, 6, 66, 
    1, 64, 0, 76, 2, 85, 3, 71, 4, 97, 5, 97, 7, 8, 6, 40, 8, 70, 9, 35, 
    0, 94, 1, 45, 2, 94, 4, 84, 3, 44, 8, 41, 5, 30, 7, 47, 6, 19, 9, 22, 
    2, 23, 1, 10, 0, 82, 3, 93, 4, 90, 8, 67, 7, 9, 9, 18, 5, 22, 6, 87, 
    0, 75, 2, 27, 4, 97, 3, 9, 1, 57, 9, 14, 5, 50, 7, 31, 8, 62, 6, 23, 
    1, 42, 3, 41, 2, 35, 0, 75, 4, 18, 9, 65, 7, 38, 6, 38, 8, 51, 5, 56, 
    4, 72, 1, 63, 0, 33, 2, 27, 3, 41, 5, 52, 7, 42, 9, 10, 6, 14, 8, 71, 
    2, 91, 1, 89, 0, 44, 4, 91, 3, 26, 6, 49, 5, 22, 8, 31, 9, 69, 7, 5, 
    3, 42, 1, 34, 0, 4, 4, 34, 2, 16, 6, 86, 7, 25, 8, 99, 5, 67, 9, 25, 
    4, 34, 1, 93, 0, 26, 3, 81, 2, 9, 7, 96, 8, 79, 9, 68, 5, 76, 6, 10, 
    3, 19, 1, 47, 4, 13, 2, 98, 0, 32, 7, 12, 9, 45, 6, 52, 8, 49, 5, 34
  };
  // Storer, Wu, and Vaccari easy 50x10 instance (Table 2, instance 16)
  const int swv16[] = {
    50, 10, // Number of jobs and machines
    1, 55, 3, 46, 5, 71, 8, 29, 0, 47, 2, 12, 7, 57, 4, 79, 6, 91, 9, 30, 
    2, 96, 6, 94, 8, 98, 0, 55, 3, 10, 1, 95, 5, 95, 7, 37, 9, 82, 4, 2, 
    6, 43, 3, 93, 8, 30, 2, 41, 0, 23, 1, 60, 7, 14, 4, 15, 5, 42, 9, 56, 
    0, 45, 6, 85, 2, 59, 7, 76, 1, 93, 9, 62, 4, 33, 8, 46, 5, 33, 3, 35, 
    2, 45, 3, 36, 8, 11, 6, 96, 7, 96, 1, 8, 0, 75, 5, 6, 4, 13, 9, 2, 
    9, 51, 7, 75, 0, 4, 3, 13, 5, 12, 1, 4, 2, 38, 6, 30, 4, 42, 8, 28, 
    9, 58, 4, 33, 6, 77, 2, 11, 3, 37, 8, 64, 5, 94, 7, 89, 1, 96, 0, 93, 
    6, 37, 3, 67, 0, 88, 9, 92, 8, 19, 4, 27, 7, 46, 1, 58, 2, 60, 5, 55, 
    4, 60, 2, 88, 0, 23, 5, 69, 8, 60, 1, 32, 7, 4, 6, 56, 9, 25, 3, 14, 
    2, 98, 5, 56, 1, 68, 6, 63, 7, 61, 3, 78, 8, 45, 0, 62, 4, 31, 9, 70, 
    7, 66, 8, 80, 0, 18, 3, 97, 9, 47, 5, 38, 1, 26, 2, 8, 6, 90, 4, 90, 
    0, 16, 7, 6, 4, 53, 6, 86, 5, 81, 8, 49, 3, 90, 2, 57, 1, 34, 9, 56, 
    2, 69, 8, 65, 5, 20, 4, 15, 1, 61, 3, 71, 6, 71, 9, 58, 0, 24, 7, 71, 
    4, 84, 5, 20, 9, 58, 0, 55, 8, 98, 2, 75, 7, 46, 3, 81, 1, 71, 6, 46, 
    5, 6, 6, 58, 7, 90, 1, 54, 9, 73, 0, 92, 4, 39, 3, 23, 2, 100, 8, 18, 
    2, 32, 5, 58, 6, 97, 1, 49, 3, 61, 0, 69, 8, 2, 4, 3, 9, 32, 7, 46, 
    0, 78, 7, 14, 4, 98, 3, 26, 8, 25, 9, 45, 6, 12, 2, 98, 1, 99, 5, 69, 
    2, 50, 1, 95, 4, 82, 9, 25, 0, 68, 8, 83, 5, 36, 7, 78, 3, 35, 6, 27, 
    6, 29, 7, 20, 8, 55, 4, 14, 2, 66, 5, 52, 0, 75, 9, 63, 1, 93, 3, 64, 
    1, 11, 0, 18, 9, 42, 4, 81, 7, 2, 2, 39, 3, 83, 6, 11, 5, 38, 8, 52, 
    4, 11, 8, 99, 9, 2, 7, 10, 3, 91, 5, 83, 6, 61, 0, 21, 2, 69, 1, 8, 
    9, 11, 7, 65, 1, 14, 2, 85, 3, 5, 8, 5, 5, 11, 4, 47, 6, 67, 0, 41, 
    9, 60, 7, 9, 8, 16, 2, 4, 5, 34, 6, 2, 4, 30, 1, 32, 0, 51, 3, 51, 
    9, 31, 2, 41, 1, 13, 6, 28, 5, 97, 3, 8, 7, 42, 4, 95, 8, 46, 0, 93, 
    4, 1, 6, 91, 8, 49, 3, 75, 1, 19, 7, 100, 0, 58, 2, 14, 5, 34, 9, 82, 
    3, 28, 5, 68, 9, 30, 7, 68, 1, 10, 6, 20, 8, 47, 4, 51, 0, 44, 2, 32, 
    9, 86, 3, 9, 1, 80, 0, 89, 5, 93, 4, 12, 8, 13, 7, 10, 6, 18, 2, 4, 
    0, 22, 5, 12, 8, 95, 4, 24, 3, 30, 1, 81, 2, 21, 7, 28, 9, 100, 6, 27, 
    1, 87, 0, 68, 2, 64, 3, 33, 7, 59, 5, 95, 6, 1, 9, 14, 8, 82, 4, 43, 
    2, 14, 6, 98, 0, 86, 1, 85, 8, 85, 5, 12, 4, 99, 7, 8, 3, 21, 9, 7, 
    5, 47, 9, 90, 0, 88, 1, 52, 8, 43, 4, 62, 7, 33, 3, 51, 6, 97, 2, 22, 
    2, 59, 7, 26, 4, 76, 0, 26, 3, 71, 8, 59, 1, 73, 9, 70, 5, 57, 6, 10, 
    6, 92, 2, 10, 9, 45, 0, 11, 1, 53, 3, 35, 8, 76, 4, 83, 7, 55, 5, 79, 
    9, 96, 4, 3, 3, 92, 7, 67, 6, 60, 8, 35, 5, 70, 0, 52, 2, 39, 1, 94, 
    4, 65, 0, 17, 9, 26, 7, 46, 5, 81, 1, 42, 2, 64, 6, 46, 3, 96, 8, 59, 
    9, 6, 3, 21, 8, 46, 0, 82, 2, 74, 5, 56, 7, 94, 6, 83, 4, 63, 1, 21, 
    6, 89, 5, 23, 8, 78, 2, 33, 9, 4, 7, 97, 3, 60, 1, 29, 0, 79, 4, 93, 
    0, 46, 1, 46, 4, 20, 7, 91, 2, 76, 9, 83, 3, 14, 6, 61, 5, 84, 8, 76, 
    7, 82, 8, 43, 6, 76, 1, 36, 0, 27, 9, 93, 5, 71, 4, 81, 2, 45, 3, 62, 
    7, 51, 9, 27, 5, 12, 6, 52, 4, 85, 8, 66, 0, 100, 3, 44, 2, 82, 1, 36, 
    3, 75, 7, 13, 6, 63, 1, 78, 4, 1, 8, 60, 2, 24, 5, 10, 9, 56, 0, 3, 
    5, 48, 4, 32, 2, 82, 0, 1, 1, 2, 7, 35, 3, 16, 9, 67, 8, 74, 6, 39, 
    7, 24, 0, 8, 8, 96, 3, 59, 2, 41, 4, 23, 1, 37, 9, 4, 5, 69, 6, 27, 
    1, 23, 9, 3, 2, 85, 6, 93, 5, 18, 7, 47, 0, 96, 8, 6, 4, 60, 3, 3, 
    6, 99, 2, 14, 9, 16, 3, 81, 8, 89, 1, 53, 7, 86, 4, 39, 5, 3, 0, 87, 
    5, 67, 8, 53, 0, 77, 4, 69, 2, 55, 3, 78, 6, 95, 1, 76, 7, 2, 9, 71, 
    1, 5, 6, 89, 0, 37, 3, 88, 7, 20, 9, 4, 4, 77, 8, 27, 5, 31, 2, 47, 
    1, 66, 2, 55, 4, 15, 7, 35, 3, 76, 9, 91, 6, 35, 5, 37, 8, 54, 0, 33, 
    3, 79, 5, 2, 6, 17, 1, 65, 7, 27, 8, 53, 4, 52, 9, 35, 0, 23, 2, 59, 
    9, 100, 0, 55, 5, 14, 2, 86, 4, 69, 3, 87, 8, 46, 1, 3, 6, 89, 7, 100
  };
  // Storer, Wu, and Vaccari easy 50x10 instance (Table 2, instance 17)
  const int swv17[] = {
    50, 10, // Number of jobs and machines
    7, 9, 2, 57, 9, 62, 5, 34, 6, 83, 0, 33, 1, 80, 4, 46, 3, 21, 8, 89, 
    9, 82, 1, 35, 8, 37, 5, 26, 6, 21, 3, 78, 7, 64, 4, 33, 2, 40, 0, 21, 
    7, 14, 5, 49, 3, 48, 9, 34, 4, 52, 1, 16, 2, 78, 0, 24, 8, 58, 6, 43, 
    2, 94, 3, 86, 8, 41, 5, 27, 7, 29, 6, 53, 9, 5, 0, 36, 4, 98, 1, 37, 
    7, 55, 1, 87, 8, 51, 5, 29, 9, 93, 3, 51, 0, 54, 6, 85, 2, 20, 4, 29, 
    2, 88, 1, 98, 3, 67, 8, 41, 6, 23, 9, 70, 7, 26, 4, 28, 5, 17, 0, 87, 
    2, 78, 0, 18, 4, 43, 3, 86, 9, 78, 6, 43, 7, 62, 8, 42, 1, 44, 5, 9, 
    9, 37, 4, 89, 3, 26, 6, 59, 0, 89, 5, 90, 1, 91, 8, 28, 7, 37, 2, 51, 
    3, 82, 2, 31, 1, 98, 5, 25, 0, 16, 7, 23, 9, 92, 4, 89, 6, 32, 8, 12, 
    6, 66, 1, 58, 5, 14, 3, 42, 0, 62, 8, 66, 4, 46, 7, 88, 2, 89, 9, 97, 
    8, 94, 9, 11, 6, 3, 1, 86, 2, 4, 5, 19, 7, 93, 4, 43, 0, 78, 3, 11, 
    5, 22, 1, 87, 9, 61, 2, 2, 3, 15, 6, 37, 7, 81, 0, 17, 8, 31, 4, 73, 
    6, 28, 0, 86, 3, 54, 2, 68, 4, 63, 1, 33, 8, 22, 5, 35, 9, 84, 7, 15, 
    6, 18, 1, 2, 2, 23, 8, 49, 7, 82, 9, 8, 4, 73, 5, 31, 3, 20, 0, 1, 
    7, 49, 5, 8, 2, 36, 8, 31, 6, 47, 3, 90, 0, 7, 9, 6, 1, 44, 4, 51, 
    4, 43, 1, 95, 0, 18, 9, 99, 7, 98, 3, 26, 8, 99, 5, 90, 2, 24, 6, 91, 
    1, 49, 6, 69, 3, 73, 9, 52, 0, 10, 7, 41, 8, 42, 5, 96, 4, 85, 2, 76, 
    0, 5, 1, 69, 3, 38, 7, 35, 5, 23, 2, 40, 8, 17, 4, 33, 6, 99, 9, 82, 
    3, 42, 1, 93, 4, 90, 6, 88, 2, 70, 8, 11, 9, 54, 7, 76, 5, 40, 0, 94, 
    5, 88, 9, 44, 0, 63, 7, 92, 1, 4, 4, 91, 6, 92, 8, 53, 3, 52, 2, 38, 
    5, 83, 3, 75, 1, 44, 2, 79, 7, 63, 6, 32, 0, 10, 4, 2, 9, 6, 8, 56, 
    7, 71, 0, 23, 5, 93, 3, 44, 6, 36, 4, 27, 2, 96, 1, 23, 9, 35, 8, 21, 
    5, 42, 2, 43, 6, 37, 9, 98, 0, 55, 3, 35, 4, 45, 1, 8, 8, 5, 7, 100, 
    0, 40, 8, 34, 2, 7, 9, 17, 5, 60, 4, 98, 7, 34, 6, 23, 1, 37, 3, 58, 
    9, 87, 2, 39, 3, 23, 8, 48, 6, 83, 7, 50, 5, 9, 1, 49, 0, 37, 4, 42, 
    6, 60, 5, 3, 2, 60, 7, 40, 0, 54, 1, 68, 4, 49, 8, 50, 9, 22, 3, 34, 
    5, 22, 1, 55, 2, 32, 0, 83, 8, 38, 4, 22, 6, 29, 7, 23, 9, 59, 3, 90, 
    9, 51, 2, 27, 6, 81, 8, 87, 0, 79, 7, 1, 3, 14, 5, 73, 4, 25, 1, 14, 
    6, 88, 1, 46, 5, 16, 2, 62, 9, 95, 7, 63, 4, 78, 0, 9, 3, 68, 8, 37, 
    4, 77, 2, 13, 8, 96, 3, 61, 0, 21, 7, 39, 5, 12, 6, 49, 9, 73, 1, 86, 
    7, 91, 5, 14, 3, 37, 0, 17, 9, 49, 4, 27, 1, 68, 2, 60, 6, 42, 8, 15, 
    9, 13, 4, 25, 6, 62, 0, 4, 1, 31, 8, 76, 5, 3, 7, 8, 3, 26, 2, 95, 
    7, 45, 5, 50, 1, 14, 0, 69, 9, 43, 4, 1, 6, 73, 8, 35, 3, 1, 2, 61, 
    4, 57, 1, 1, 0, 74, 8, 1, 6, 96, 2, 92, 7, 85, 5, 42, 3, 12, 9, 38, 
    7, 49, 5, 31, 8, 79, 6, 83, 1, 40, 4, 65, 3, 34, 2, 32, 9, 97, 0, 25, 
    9, 24, 5, 40, 4, 81, 3, 10, 6, 59, 8, 83, 2, 66, 1, 28, 7, 33, 0, 31, 
    5, 33, 4, 39, 3, 50, 1, 96, 7, 62, 2, 72, 8, 42, 6, 86, 9, 66, 0, 80, 
    3, 88, 7, 47, 0, 35, 4, 69, 1, 79, 9, 61, 2, 25, 8, 56, 5, 68, 6, 96, 
    9, 23, 6, 95, 0, 42, 1, 84, 8, 57, 4, 42, 2, 2, 5, 79, 3, 29, 7, 90, 
    9, 96, 8, 21, 4, 17, 7, 12, 1, 25, 2, 9, 6, 7, 5, 26, 0, 81, 3, 51, 
    1, 63, 7, 16, 6, 40, 2, 22, 9, 48, 5, 87, 0, 15, 8, 24, 3, 37, 4, 55, 
    7, 95, 0, 60, 3, 62, 2, 7, 9, 2, 8, 81, 5, 83, 4, 64, 1, 68, 6, 66, 
    3, 24, 7, 60, 6, 35, 2, 77, 1, 85, 8, 57, 9, 29, 5, 59, 4, 53, 0, 14, 
    1, 24, 6, 30, 0, 9, 3, 89, 8, 72, 4, 77, 2, 7, 5, 23, 9, 73, 7, 35, 
    0, 66, 8, 12, 1, 9, 5, 50, 2, 14, 9, 76, 4, 90, 3, 43, 7, 48, 6, 63, 
    3, 97, 1, 29, 0, 59, 4, 64, 9, 17, 2, 77, 5, 60, 7, 16, 6, 61, 8, 40, 
    9, 5, 4, 22, 2, 3, 8, 63, 5, 1, 7, 23, 0, 1, 3, 61, 1, 92, 6, 19, 
    6, 91, 8, 74, 1, 88, 5, 2, 7, 61, 4, 39, 0, 35, 2, 23, 9, 84, 3, 27, 
    8, 87, 5, 58, 7, 44, 1, 6, 6, 22, 3, 57, 9, 78, 4, 19, 2, 74, 0, 6, 
    4, 6, 1, 94, 0, 45, 2, 54, 9, 67, 7, 90, 5, 19, 8, 72, 6, 70, 3, 58
  };
  // Storer, Wu, and Vaccari easy 50x10 instance (Table 2, instance 18)
  const int swv18[] = {
    50, 10, // Number of jobs and machines
    7, 35, 6, 23, 2, 92, 4, 5, 5, 40, 1, 90, 3, 30, 9, 35, 8, 8, 0, 86, 
    2, 60, 3, 97, 8, 21, 9, 70, 7, 82, 0, 12, 4, 3, 5, 45, 1, 75, 6, 69, 
    7, 96, 2, 38, 0, 61, 1, 55, 4, 31, 5, 48, 9, 79, 3, 4, 6, 12, 8, 29, 
    4, 83, 7, 82, 8, 97, 1, 43, 0, 95, 6, 92, 2, 18, 3, 29, 5, 4, 9, 67, 
    3, 46, 9, 80, 8, 66, 2, 38, 4, 95, 1, 40, 7, 89, 0, 32, 6, 64, 5, 1, 
    6, 57, 4, 80, 8, 68, 7, 27, 0, 90, 5, 45, 3, 98, 9, 59, 1, 6, 2, 94, 
    5, 50, 0, 91, 2, 97, 9, 63, 7, 52, 3, 48, 4, 4, 8, 96, 1, 18, 6, 100, 
    7, 23, 6, 43, 3, 25, 8, 83, 2, 76, 9, 41, 1, 88, 0, 31, 5, 44, 4, 13, 
    2, 20, 3, 90, 9, 20, 4, 42, 8, 72, 5, 46, 1, 27, 0, 81, 6, 40, 7, 34, 
    7, 80, 5, 97, 0, 42, 2, 49, 9, 10, 1, 10, 3, 71, 4, 71, 6, 14, 8, 98, 
    2, 79, 3, 29, 0, 96, 7, 66, 1, 58, 8, 31, 4, 47, 5, 76, 6, 59, 9, 88, 
    8, 93, 6, 3, 1, 7, 3, 27, 5, 66, 7, 23, 0, 60, 4, 97, 2, 66, 9, 55, 
    9, 12, 8, 39, 4, 77, 5, 79, 0, 26, 7, 58, 2, 98, 6, 38, 3, 31, 1, 28, 
    6, 8, 9, 48, 4, 4, 1, 87, 3, 38, 2, 28, 8, 10, 0, 19, 7, 82, 5, 83, 
    5, 6, 9, 13, 2, 86, 6, 19, 3, 26, 7, 79, 0, 55, 1, 85, 8, 33, 4, 30, 
    3, 37, 8, 26, 7, 29, 6, 74, 9, 43, 5, 17, 0, 45, 2, 28, 1, 58, 4, 15, 
    7, 15, 3, 37, 6, 21, 5, 47, 2, 90, 0, 37, 9, 33, 1, 42, 4, 7, 8, 62, 
    8, 49, 4, 46, 1, 28, 7, 18, 6, 41, 2, 57, 0, 75, 3, 21, 9, 3, 5, 32, 
    6, 98, 1, 30, 8, 24, 4, 91, 9, 73, 7, 25, 5, 49, 0, 40, 2, 9, 3, 4, 
    6, 33, 3, 94, 1, 21, 2, 90, 9, 86, 7, 85, 5, 29, 0, 17, 4, 94, 8, 90, 
    6, 3, 4, 85, 1, 66, 7, 61, 8, 57, 3, 84, 2, 5, 9, 40, 0, 54, 5, 70, 
    7, 81, 1, 98, 2, 45, 0, 18, 6, 65, 9, 1, 4, 98, 3, 30, 8, 84, 5, 82, 
    6, 40, 7, 77, 3, 72, 1, 97, 5, 39, 4, 21, 0, 59, 8, 42, 9, 90, 2, 26, 
    5, 57, 3, 63, 1, 14, 4, 64, 6, 23, 8, 78, 2, 54, 0, 51, 9, 100, 7, 96, 
    5, 61, 1, 55, 6, 73, 2, 87, 4, 35, 3, 41, 7, 96, 0, 32, 8, 91, 9, 60, 
    9, 19, 5, 90, 8, 91, 0, 45, 3, 66, 2, 84, 1, 61, 7, 3, 6, 84, 4, 100, 
    2, 33, 9, 72, 6, 27, 8, 14, 3, 59, 0, 39, 7, 20, 5, 29, 4, 54, 1, 88, 
    4, 45, 0, 18, 3, 73, 2, 26, 8, 55, 6, 22, 7, 27, 1, 46, 9, 43, 5, 77, 
    2, 57, 9, 16, 1, 71, 8, 25, 7, 50, 3, 41, 6, 58, 5, 71, 4, 9, 0, 32, 
    8, 48, 9, 32, 0, 42, 3, 73, 1, 56, 7, 53, 6, 3, 5, 66, 4, 15, 2, 44, 
    6, 69, 7, 14, 1, 2, 8, 40, 4, 70, 9, 90, 3, 38, 2, 31, 5, 55, 0, 50, 
    9, 100, 8, 14, 0, 55, 2, 5, 5, 12, 4, 79, 1, 68, 3, 83, 6, 89, 7, 78, 
    4, 26, 5, 44, 8, 39, 1, 84, 7, 64, 9, 98, 3, 38, 2, 2, 6, 27, 0, 18, 
    3, 98, 2, 10, 9, 99, 8, 50, 0, 20, 6, 12, 4, 7, 1, 57, 7, 87, 5, 89, 
    0, 64, 8, 63, 7, 98, 5, 31, 1, 30, 6, 62, 3, 11, 4, 89, 9, 31, 2, 34, 
    3, 26, 6, 43, 4, 69, 7, 27, 8, 92, 2, 51, 1, 10, 5, 29, 9, 21, 0, 37, 
    8, 21, 5, 98, 0, 64, 6, 38, 2, 23, 1, 13, 7, 89, 9, 89, 4, 21, 3, 27, 
    4, 39, 7, 32, 1, 67, 0, 33, 5, 16, 2, 43, 6, 62, 3, 42, 9, 70, 8, 90, 
    7, 73, 9, 45, 3, 37, 0, 45, 2, 61, 6, 25, 5, 15, 4, 5, 8, 58, 1, 98, 
    7, 94, 0, 17, 6, 15, 5, 81, 9, 64, 3, 62, 1, 2, 8, 16, 2, 35, 4, 40, 
    5, 32, 6, 37, 9, 11, 0, 25, 1, 37, 8, 21, 2, 76, 7, 52, 4, 56, 3, 87, 
    3, 23, 2, 40, 1, 6, 7, 31, 6, 25, 9, 98, 8, 29, 4, 4, 5, 25, 0, 33, 
    8, 96, 9, 30, 1, 95, 3, 2, 6, 3, 2, 22, 0, 62, 4, 30, 7, 1, 5, 99, 
    9, 54, 5, 3, 0, 78, 2, 43, 6, 90, 7, 88, 4, 1, 8, 97, 1, 30, 3, 96, 
    5, 29, 6, 60, 3, 80, 1, 94, 2, 67, 0, 42, 8, 17, 9, 27, 7, 75, 4, 86, 
    1, 17, 5, 62, 2, 25, 7, 80, 6, 62, 9, 19, 8, 81, 3, 73, 0, 57, 4, 90, 
    9, 31, 3, 54, 5, 28, 1, 19, 4, 4, 2, 34, 8, 64, 6, 46, 7, 60, 0, 27, 
    9, 95, 7, 1, 2, 43, 3, 6, 4, 7, 8, 66, 1, 45, 5, 13, 0, 80, 6, 1, 
    3, 20, 7, 82, 0, 87, 1, 65, 6, 64, 8, 61, 2, 21, 5, 32, 9, 16, 4, 37, 
    0, 49, 3, 54, 2, 31, 8, 69, 1, 21, 5, 2, 6, 73, 9, 35, 4, 66, 7, 82
  };
  // Storer, Wu, and Vaccari easy 50x10 instance (Table 2, instance 19)
  const int swv19[] = {
    50, 10, // Number of jobs and machines
    7, 74, 1, 27, 5, 66, 3, 89, 6, 58, 0, 11, 8, 77, 9, 17, 2, 70, 4, 97, 
    5, 10, 0, 11, 2, 38, 3, 60, 1, 50, 7, 35, 6, 94, 9, 52, 4, 2, 8, 20, 
    7, 17, 0, 65, 6, 93, 8, 62, 9, 91, 5, 2, 1, 51, 2, 4, 3, 19, 4, 10, 
    4, 87, 3, 3, 9, 81, 0, 17, 6, 44, 2, 82, 7, 16, 5, 13, 8, 100, 1, 85, 
    9, 18, 6, 33, 7, 35, 0, 78, 2, 68, 3, 68, 8, 3, 5, 2, 4, 53, 1, 25, 
    2, 36, 8, 41, 6, 60, 9, 43, 0, 66, 5, 34, 3, 24, 7, 11, 1, 5, 4, 55, 
    9, 52, 4, 99, 6, 62, 0, 50, 1, 24, 8, 73, 7, 19, 3, 23, 2, 15, 5, 2, 
    4, 85, 9, 21, 3, 27, 7, 53, 0, 86, 1, 36, 6, 35, 5, 99, 8, 30, 2, 43, 
    6, 43, 5, 31, 9, 99, 2, 12, 0, 6, 7, 79, 3, 81, 1, 18, 8, 73, 4, 55, 
    4, 90, 6, 100, 1, 15, 0, 40, 7, 96, 9, 25, 5, 43, 8, 23, 2, 31, 3, 7, 
    5, 61, 4, 88, 6, 10, 3, 48, 0, 100, 2, 62, 1, 83, 8, 20, 7, 42, 9, 19, 
    9, 35, 7, 41, 6, 16, 3, 58, 0, 86, 2, 69, 5, 58, 1, 93, 4, 47, 8, 77, 
    2, 61, 0, 40, 4, 99, 1, 51, 7, 46, 6, 39, 3, 43, 9, 37, 8, 88, 5, 9, 
    4, 15, 8, 38, 2, 84, 5, 98, 6, 17, 1, 91, 7, 91, 9, 23, 3, 48, 0, 98, 
    3, 26, 2, 42, 8, 55, 4, 24, 0, 43, 1, 83, 9, 27, 7, 38, 6, 37, 5, 58, 
    5, 21, 8, 78, 6, 97, 0, 77, 9, 82, 4, 26, 3, 22, 1, 90, 7, 57, 2, 31, 
    4, 3, 9, 44, 3, 90, 1, 64, 5, 52, 8, 35, 7, 18, 2, 45, 0, 4, 6, 14, 
    8, 60, 6, 59, 3, 67, 2, 85, 0, 43, 7, 93, 5, 44, 4, 22, 1, 68, 9, 38, 
    4, 77, 8, 41, 2, 74, 6, 99, 0, 100, 1, 45, 9, 14, 3, 26, 7, 98, 5, 77, 
    8, 38, 9, 57, 7, 42, 5, 64, 1, 80, 6, 81, 4, 70, 3, 13, 2, 41, 0, 65, 
    9, 36, 4, 22, 8, 39, 0, 76, 1, 78, 2, 27, 5, 55, 3, 10, 6, 5, 7, 71, 
    7, 70, 9, 81, 1, 60, 5, 85, 3, 63, 6, 97, 2, 61, 8, 44, 0, 5, 4, 35, 
    9, 38, 0, 94, 2, 46, 5, 20, 8, 87, 1, 41, 4, 41, 3, 40, 7, 99, 6, 48, 
    7, 30, 6, 9, 5, 13, 2, 79, 8, 81, 0, 25, 9, 93, 4, 85, 3, 78, 1, 76, 
    4, 6, 8, 58, 6, 51, 7, 48, 2, 68, 3, 34, 5, 78, 9, 59, 1, 98, 0, 36, 
    4, 90, 6, 56, 7, 97, 9, 37, 0, 38, 1, 47, 2, 56, 3, 8, 5, 37, 8, 7, 
    0, 66, 8, 15, 1, 39, 5, 89, 7, 3, 9, 54, 3, 24, 2, 14, 6, 99, 4, 73, 
    3, 12, 9, 37, 4, 79, 8, 95, 0, 50, 1, 74, 6, 1, 5, 55, 7, 98, 2, 49, 
    8, 99, 9, 79, 3, 99, 2, 87, 0, 80, 4, 13, 5, 99, 6, 13, 1, 54, 7, 61, 
    1, 51, 9, 21, 3, 32, 6, 20, 0, 80, 7, 58, 2, 91, 5, 84, 8, 62, 4, 91, 
    1, 11, 8, 38, 2, 14, 9, 12, 3, 39, 5, 34, 0, 37, 6, 94, 4, 10, 7, 2, 
    6, 76, 9, 86, 3, 40, 4, 30, 2, 97, 0, 59, 8, 100, 7, 9, 5, 55, 1, 86, 
    3, 33, 1, 49, 0, 94, 2, 17, 6, 17, 8, 70, 5, 17, 7, 42, 4, 26, 9, 24, 
    4, 75, 1, 20, 9, 93, 2, 58, 3, 51, 0, 94, 6, 24, 7, 70, 8, 51, 5, 82, 
    8, 59, 1, 9, 3, 59, 5, 62, 9, 79, 7, 53, 6, 48, 4, 98, 2, 76, 0, 71, 
    6, 90, 2, 35, 5, 89, 0, 59, 9, 28, 7, 51, 4, 69, 3, 36, 1, 32, 8, 27, 
    5, 10, 6, 85, 4, 97, 1, 3, 0, 79, 9, 86, 3, 10, 7, 80, 2, 37, 8, 39, 
    7, 60, 0, 27, 5, 69, 8, 58, 6, 67, 2, 36, 9, 31, 3, 69, 1, 16, 4, 22, 
    2, 27, 5, 16, 6, 15, 4, 40, 8, 16, 1, 92, 9, 60, 7, 43, 3, 2, 0, 7, 
    1, 79, 7, 99, 0, 27, 9, 56, 5, 29, 6, 17, 8, 67, 4, 34, 3, 86, 2, 61, 
    6, 57, 7, 100, 4, 73, 9, 17, 8, 3, 3, 64, 2, 99, 0, 71, 5, 27, 1, 90, 
    2, 80, 5, 23, 4, 54, 6, 39, 9, 77, 3, 65, 7, 59, 0, 7, 1, 63, 8, 32, 
    4, 98, 6, 17, 8, 44, 5, 1, 3, 10, 7, 56, 2, 95, 9, 80, 0, 99, 1, 64, 
    8, 60, 7, 74, 3, 60, 6, 30, 0, 81, 5, 25, 4, 89, 9, 19, 2, 59, 1, 21, 
    1, 67, 0, 42, 8, 93, 2, 47, 5, 34, 7, 11, 6, 100, 9, 15, 4, 99, 3, 2, 
    9, 35, 3, 61, 5, 93, 8, 83, 7, 87, 4, 66, 0, 96, 2, 55, 1, 41, 6, 61, 
    8, 22, 5, 25, 7, 29, 3, 70, 6, 93, 1, 19, 0, 49, 9, 62, 2, 19, 4, 73, 
    8, 11, 4, 93, 5, 97, 1, 28, 2, 14, 0, 75, 7, 41, 3, 40, 9, 62, 6, 66, 
    7, 76, 6, 61, 8, 64, 3, 90, 0, 20, 2, 43, 9, 50, 1, 13, 5, 4, 4, 47, 
    3, 38, 4, 11, 0, 30, 5, 37, 7, 57, 9, 64, 1, 68, 8, 42, 2, 19, 6, 79
  };
  // Storer, Wu, and Vaccari easy 50x10 instance (Table 2, instance 20)
  const int swv20[] = {
    50, 10, // Number of jobs and machines
    8, 100, 7, 30, 4, 42, 9, 11, 2, 31, 1, 71, 5, 41, 0, 1, 3, 55, 6, 94, 
    4, 81, 6, 20, 3, 96, 7, 39, 8, 29, 0, 90, 9, 61, 2, 64, 1, 86, 5, 47, 
    5, 80, 0, 56, 1, 88, 7, 19, 2, 68, 8, 95, 3, 44, 4, 22, 9, 60, 6, 80, 
    4, 86, 6, 70, 0, 88, 2, 15, 7, 50, 1, 54, 9, 88, 3, 25, 8, 89, 5, 33, 
    0, 48, 1, 57, 4, 86, 8, 60, 3, 78, 5, 4, 9, 60, 7, 40, 2, 11, 6, 25, 
    6, 23, 7, 9, 1, 90, 0, 51, 2, 52, 9, 14, 5, 30, 4, 1, 8, 25, 3, 83, 
    1, 30, 4, 75, 5, 76, 9, 100, 7, 54, 2, 41, 6, 50, 8, 75, 0, 1, 3, 28, 
    2, 46, 3, 78, 1, 37, 7, 12, 6, 56, 4, 50, 8, 66, 5, 39, 0, 8, 9, 72, 
    1, 24, 6, 90, 0, 32, 3, 6, 2, 99, 9, 22, 8, 12, 4, 63, 7, 81, 5, 52, 
    6, 62, 3, 9, 8, 59, 0, 66, 4, 41, 1, 32, 5, 29, 7, 79, 9, 84, 2, 4, 
    9, 57, 5, 99, 6, 2, 3, 17, 0, 51, 7, 10, 4, 14, 1, 64, 2, 99, 8, 27, 
    7, 81, 0, 67, 9, 83, 2, 30, 5, 25, 6, 87, 1, 29, 3, 7, 8, 93, 4, 1, 
    5, 65, 8, 53, 9, 48, 4, 28, 7, 74, 0, 60, 6, 77, 2, 22, 1, 5, 3, 98, 
    1, 97, 5, 37, 0, 71, 7, 49, 6, 51, 3, 17, 4, 38, 9, 67, 8, 28, 2, 31, 
    0, 20, 8, 94, 3, 39, 6, 73, 9, 63, 4, 8, 2, 57, 1, 27, 7, 26, 5, 42, 
    8, 77, 1, 68, 9, 20, 7, 100, 4, 1, 5, 77, 6, 17, 3, 35, 2, 65, 0, 86, 
    8, 68, 6, 62, 4, 79, 7, 84, 1, 60, 3, 56, 0, 10, 9, 86, 5, 60, 2, 30, 
    4, 71, 2, 74, 6, 6, 1, 56, 3, 69, 0, 8, 8, 50, 9, 78, 5, 4, 7, 89, 
    8, 29, 5, 5, 1, 59, 3, 96, 0, 46, 4, 91, 2, 48, 7, 53, 6, 21, 9, 82, 
    2, 19, 9, 96, 0, 73, 1, 39, 5, 54, 8, 50, 7, 60, 3, 50, 4, 65, 6, 78, 
    7, 68, 4, 15, 2, 26, 3, 26, 0, 13, 9, 13, 5, 96, 8, 70, 6, 27, 1, 93, 
    6, 41, 8, 18, 4, 66, 7, 9, 1, 31, 2, 92, 0, 3, 3, 78, 5, 41, 9, 53, 
    5, 9, 0, 64, 2, 15, 6, 73, 4, 12, 1, 43, 8, 89, 7, 69, 3, 32, 9, 22, 
    5, 93, 6, 19, 3, 74, 8, 81, 0, 72, 2, 94, 9, 19, 1, 26, 4, 53, 7, 7, 
    3, 48, 2, 29, 5, 51, 8, 72, 7, 35, 6, 32, 1, 38, 0, 98, 4, 58, 9, 54, 
    0, 94, 9, 23, 4, 41, 6, 53, 2, 53, 7, 27, 1, 62, 3, 68, 8, 84, 5, 49, 
    4, 4, 1, 4, 0, 66, 7, 90, 9, 78, 2, 29, 5, 2, 6, 86, 3, 23, 8, 46, 
    3, 78, 5, 61, 2, 97, 7, 68, 8, 92, 0, 15, 4, 12, 6, 77, 1, 12, 9, 22, 
    0, 100, 7, 89, 6, 71, 2, 70, 8, 89, 4, 72, 5, 78, 3, 23, 9, 37, 1, 2, 
    0, 91, 3, 74, 2, 36, 4, 72, 6, 62, 1, 80, 9, 20, 7, 77, 5, 47, 8, 80, 
    1, 44, 0, 67, 4, 66, 8, 99, 6, 59, 5, 5, 7, 15, 2, 38, 3, 40, 9, 19, 
    1, 69, 9, 35, 3, 86, 0, 7, 2, 35, 5, 32, 6, 66, 4, 89, 8, 63, 7, 52, 
    3, 3, 4, 68, 1, 66, 7, 27, 6, 41, 5, 2, 9, 77, 0, 45, 2, 40, 8, 39, 
    4, 66, 3, 42, 7, 79, 0, 55, 6, 98, 9, 44, 5, 6, 8, 73, 1, 55, 2, 1, 
    3, 80, 8, 18, 9, 94, 2, 27, 5, 42, 4, 17, 7, 74, 0, 65, 6, 6, 1, 27, 
    2, 73, 4, 70, 5, 51, 0, 84, 8, 29, 9, 95, 1, 97, 7, 28, 3, 68, 6, 89, 
    9, 85, 6, 56, 5, 54, 3, 76, 2, 50, 0, 43, 1, 8, 7, 93, 4, 17, 8, 65, 
    1, 1, 3, 17, 2, 61, 5, 38, 4, 71, 7, 18, 0, 40, 9, 94, 6, 41, 8, 74, 
    3, 30, 8, 22, 6, 39, 9, 56, 5, 3, 7, 64, 4, 74, 2, 21, 0, 93, 1, 1, 
    0, 17, 8, 8, 9, 20, 5, 38, 3, 85, 7, 5, 2, 63, 1, 18, 4, 89, 6, 88, 
    8, 87, 5, 44, 0, 42, 1, 34, 9, 11, 7, 13, 3, 71, 4, 88, 6, 32, 2, 12, 
    2, 39, 1, 73, 6, 43, 0, 48, 9, 77, 8, 48, 5, 23, 7, 66, 3, 94, 4, 68, 
    1, 98, 7, 19, 3, 69, 6, 5, 8, 85, 9, 19, 0, 30, 2, 43, 5, 87, 4, 70, 
    2, 45, 1, 60, 4, 30, 9, 71, 5, 35, 0, 75, 3, 75, 6, 41, 8, 67, 7, 37, 
    3, 63, 7, 39, 2, 16, 9, 69, 1, 46, 5, 20, 6, 57, 4, 51, 0, 66, 8, 40, 
    2, 7, 7, 73, 6, 17, 1, 21, 0, 24, 8, 2, 5, 68, 4, 22, 9, 36, 3, 60, 
    1, 20, 4, 17, 8, 12, 9, 29, 5, 28, 0, 7, 3, 38, 6, 57, 7, 22, 2, 75, 
    5, 53, 4, 7, 7, 5, 8, 27, 9, 38, 2, 100, 6, 48, 0, 53, 1, 11, 3, 18, 
    1, 49, 7, 47, 4, 81, 8, 9, 0, 20, 2, 63, 3, 15, 6, 1, 9, 10, 5, 5, 
    4, 49, 6, 27, 7, 17, 5, 64, 2, 30, 8, 56, 0, 42, 3, 97, 9, 82, 1, 34
  };

  // Yamada and Nakano 20x20 instance (Table 4, instance 1)
  const int yn1[] = {
    20, 20, // Number of jobs and machines
    17, 13, 2, 26, 11, 35, 4, 45, 12, 29, 13, 21, 7, 40, 0, 45, 3, 16, 15, 10, 18, 49, 10, 43, 14, 25, 8, 25, 1, 40, 6, 16, 19, 43, 5, 48, 9, 36, 16, 11, 
    8, 21, 6, 22, 14, 15, 5, 28, 10, 10, 2, 46, 11, 19, 19, 13, 13, 18, 18, 14, 3, 11, 4, 21, 16, 30, 1, 29, 0, 16, 15, 41, 17, 40, 12, 38, 7, 28, 9, 39, 
    4, 39, 3, 28, 8, 32, 17, 46, 0, 35, 14, 14, 1, 44, 10, 20, 13, 12, 6, 23, 18, 22, 9, 15, 11, 35, 7, 27, 16, 26, 5, 27, 15, 23, 2, 27, 12, 31, 19, 31, 
    4, 31, 10, 24, 3, 34, 6, 44, 18, 43, 12, 32, 2, 35, 15, 34, 19, 21, 7, 46, 13, 15, 5, 10, 9, 24, 14, 37, 17, 38, 1, 41, 8, 34, 0, 32, 16, 11, 11, 36, 
    19, 45, 1, 23, 5, 34, 9, 23, 7, 41, 16, 10, 11, 40, 12, 46, 14, 27, 8, 13, 4, 20, 2, 40, 15, 28, 13, 44, 17, 34, 18, 21, 10, 27, 0, 12, 6, 37, 3, 30, 
    13, 48, 2, 34, 3, 22, 7, 14, 12, 22, 14, 10, 8, 45, 19, 38, 6, 32, 16, 38, 11, 16, 4, 20, 0, 12, 5, 40, 9, 33, 17, 35, 1, 32, 10, 15, 15, 31, 18, 49, 
    9, 19, 5, 33, 18, 32, 16, 37, 12, 28, 3, 16, 2, 40, 10, 37, 4, 10, 11, 20, 1, 17, 17, 48, 6, 44, 13, 29, 14, 44, 15, 48, 8, 21, 0, 31, 7, 36, 19, 43, 
    9, 20, 6, 43, 1, 13, 5, 22, 2, 33, 7, 28, 16, 39, 12, 16, 13, 34, 17, 20, 10, 47, 18, 43, 19, 44, 8, 29, 15, 22, 4, 14, 11, 28, 14, 44, 0, 33, 3, 28, 
    7, 14, 12, 40, 8, 19, 0, 49, 13, 11, 10, 13, 9, 47, 18, 22, 2, 27, 17, 26, 3, 47, 5, 37, 6, 19, 15, 43, 14, 41, 1, 34, 11, 21, 4, 30, 19, 32, 16, 45, 
    16, 32, 7, 22, 15, 30, 6, 18, 18, 41, 19, 34, 9, 22, 11, 11, 17, 29, 10, 37, 4, 30, 2, 25, 1, 27, 0, 31, 14, 16, 13, 20, 3, 26, 12, 14, 5, 24, 8, 43, 
    18, 22, 17, 22, 12, 30, 15, 31, 13, 15, 4, 13, 16, 47, 19, 18, 6, 33, 3, 30, 7, 46, 2, 48, 11, 42, 0, 18, 1, 16, 8, 25, 10, 43, 5, 21, 9, 27, 14, 14, 
    5, 48, 1, 39, 2, 21, 18, 18, 13, 20, 0, 28, 15, 20, 8, 36, 6, 24, 9, 35, 7, 22, 19, 36, 3, 39, 14, 34, 4, 49, 17, 36, 11, 38, 10, 46, 12, 44, 16, 13, 
    14, 26, 1, 32, 2, 11, 15, 10, 9, 41, 13, 10, 6, 26, 19, 26, 12, 13, 11, 35, 5, 22, 0, 11, 7, 24, 17, 33, 8, 11, 10, 34, 16, 11, 3, 22, 4, 12, 18, 17, 
    16, 39, 10, 24, 17, 43, 14, 28, 3, 49, 15, 34, 18, 46, 13, 29, 6, 31, 11, 40, 7, 24, 1, 47, 9, 15, 2, 26, 8, 40, 12, 46, 5, 18, 19, 16, 4, 14, 0, 21, 
    11, 41, 19, 26, 16, 14, 3, 47, 0, 49, 5, 16, 17, 31, 9, 43, 15, 20, 10, 25, 14, 10, 13, 49, 8, 32, 6, 36, 7, 19, 4, 23, 2, 20, 18, 15, 12, 34, 1, 33, 
    11, 37, 5, 48, 10, 31, 7, 42, 2, 24, 1, 13, 9, 30, 15, 24, 0, 19, 13, 34, 19, 35, 8, 42, 3, 10, 14, 40, 4, 39, 6, 42, 12, 38, 16, 12, 18, 27, 17, 40, 
    14, 19, 1, 27, 8, 39, 12, 41, 5, 45, 11, 40, 10, 46, 6, 48, 7, 37, 3, 30, 17, 31, 4, 16, 18, 29, 15, 44, 0, 41, 16, 35, 13, 47, 9, 21, 2, 10, 19, 48, 
    18, 38, 0, 27, 13, 32, 9, 30, 7, 17, 14, 21, 1, 14, 4, 37, 17, 15, 16, 31, 5, 27, 10, 25, 15, 41, 11, 48, 3, 48, 6, 36, 2, 30, 12, 45, 8, 26, 19, 17, 
    1, 17, 10, 40, 9, 16, 5, 36, 4, 34, 16, 47, 19, 14, 0, 24, 18, 10, 6, 14, 13, 14, 3, 30, 12, 23, 2, 37, 17, 11, 11, 23, 8, 40, 15, 15, 14, 10, 7, 46, 
    14, 37, 10, 28, 13, 13, 0, 28, 2, 18, 1, 43, 16, 46, 8, 39, 3, 30, 12, 15, 11, 38, 17, 38, 18, 45, 19, 44, 9, 16, 15, 29, 5, 33, 6, 20, 7, 35, 4, 34
  };
  // Yamada and Nakano 20x20 instance (Table 4, instance 2)
  const int yn2[] = {
    20, 20, // Number of jobs and machines
    17, 15, 2, 28, 11, 10, 4, 46, 12, 19, 13, 13, 7, 18, 0, 14, 3, 11, 15, 21, 18, 30, 10, 29, 14, 16, 8, 41, 1, 40, 6, 38, 19, 28, 5, 39, 9, 39, 16, 28, 
    8, 32, 6, 46, 14, 35, 5, 14, 10, 44, 2, 20, 11, 12, 19, 23, 13, 22, 18, 15, 3, 35, 4, 27, 16, 26, 1, 27, 0, 23, 15, 27, 17, 31, 12, 31, 7, 31, 9, 24, 
    4, 34, 3, 44, 8, 43, 17, 32, 0, 35, 14, 34, 1, 21, 10, 46, 13, 15, 6, 10, 18, 24, 9, 37, 11, 38, 7, 41, 16, 34, 5, 32, 15, 11, 2, 36, 12, 45, 19, 23, 
    4, 34, 10, 23, 3, 41, 6, 10, 18, 40, 12, 46, 2, 27, 15, 13, 19, 20, 7, 40, 13, 28, 5, 44, 9, 34, 14, 21, 17, 27, 1, 12, 8, 37, 0, 30, 16, 48, 11, 34, 
    19, 22, 1, 14, 5, 22, 9, 10, 7, 45, 16, 38, 11, 32, 12, 38, 14, 16, 8, 20, 4, 12, 2, 40, 15, 33, 13, 35, 17, 32, 18, 15, 10, 31, 0, 49, 6, 19, 3, 33, 
    13, 32, 2, 37, 3, 28, 7, 16, 12, 40, 14, 37, 8, 10, 19, 20, 6, 17, 16, 48, 11, 44, 4, 29, 0, 44, 5, 48, 9, 21, 17, 31, 1, 36, 10, 43, 15, 20, 18, 43, 
    9, 13, 5, 22, 18, 33, 16, 28, 12, 39, 3, 16, 2, 34, 10, 20, 4, 47, 11, 43, 1, 44, 17, 29, 6, 22, 13, 14, 14, 28, 15, 44, 8, 33, 0, 28, 7, 14, 19, 40, 
    9, 19, 6, 49, 1, 11, 5, 13, 2, 47, 7, 22, 16, 27, 12, 26, 13, 47, 17, 37, 10, 19, 18, 43, 19, 41, 8, 34, 15, 21, 4, 30, 11, 32, 14, 45, 0, 32, 3, 22, 
    7, 30, 12, 18, 8, 41, 0, 34, 13, 22, 10, 11, 9, 29, 18, 37, 2, 30, 17, 25, 3, 27, 5, 31, 6, 16, 15, 20, 14, 26, 1, 14, 11, 24, 4, 43, 19, 22, 16, 22, 
    16, 30, 7, 31, 15, 15, 6, 13, 18, 47, 19, 18, 9, 33, 11, 30, 17, 46, 4, 48, 10, 42, 2, 18, 1, 16, 0, 25, 14, 43, 13, 21, 3, 27, 12, 14, 5, 48, 8, 39, 
    18, 21, 17, 18, 12, 20, 15, 28, 13, 20, 4, 36, 16, 24, 19, 35, 7, 22, 3, 36, 6, 39, 10, 34, 11, 49, 0, 36, 1, 38, 8, 46, 9, 44, 5, 13, 2, 26, 14, 32, 
    9, 11, 1, 10, 2, 41, 11, 10, 13, 26, 0, 26, 12, 13, 10, 35, 6, 22, 5, 11, 7, 24, 19, 33, 3, 11, 14, 34, 17, 11, 4, 22, 18, 12, 8, 17, 15, 39, 16, 24, 
    1, 43, 15, 28, 2, 49, 14, 34, 4, 46, 12, 29, 18, 31, 19, 40, 13, 24, 11, 47, 5, 15, 0, 26, 7, 40, 17, 46, 8, 18, 10, 16, 16, 14, 3, 21, 9, 41, 6, 26, 
    16, 14, 6, 47, 17, 49, 10, 16, 3, 31, 12, 43, 4, 20, 8, 25, 14, 10, 18, 49, 7, 32, 0, 36, 9, 19, 2, 23, 15, 20, 5, 15, 13, 34, 19, 33, 11, 37, 1, 48, 
    4, 31, 11, 42, 7, 24, 6, 13, 0, 30, 14, 24, 17, 19, 19, 34, 16, 35, 10, 42, 15, 10, 13, 40, 2, 39, 8, 42, 5, 38, 9, 12, 1, 27, 18, 40, 12, 19, 3, 27, 
    6, 39, 5, 41, 13, 45, 15, 40, 2, 46, 9, 48, 7, 37, 0, 30, 1, 31, 12, 16, 19, 29, 14, 44, 3, 41, 8, 35, 10, 47, 11, 21, 4, 10, 16, 48, 18, 38, 17, 27, 
    16, 32, 1, 30, 8, 17, 18, 21, 0, 14, 17, 37, 10, 15, 12, 31, 7, 27, 3, 25, 5, 41, 4, 48, 13, 48, 6, 36, 2, 30, 15, 45, 11, 26, 9, 17, 14, 17, 19, 40, 
    18, 16, 17, 36, 4, 34, 2, 47, 10, 14, 15, 24, 1, 10, 3, 14, 7, 14, 12, 30, 5, 23, 9, 37, 8, 11, 14, 23, 11, 40, 6, 15, 16, 10, 0, 46, 13, 37, 19, 28, 
    17, 13, 13, 28, 11, 18, 16, 43, 7, 46, 8, 39, 3, 30, 5, 15, 4, 38, 2, 38, 14, 45, 0, 44, 10, 16, 6, 29, 12, 33, 1, 20, 19, 35, 15, 34, 9, 16, 18, 40, 
    17, 14, 2, 30, 0, 27, 15, 47, 18, 43, 3, 17, 14, 13, 6, 43, 7, 45, 12, 32, 13, 13, 16, 48, 1, 10, 4, 14, 10, 42, 9, 38, 5, 43, 19, 22, 11, 43, 8, 23
  };
  // Yamada and Nakano 20x20 instance (Table 4, instance 3)
  const int yn3[] = {
    20, 20, // Number of jobs and machines
    13, 47, 16, 21, 17, 27, 8, 46, 1, 27, 14, 39, 19, 24, 4, 34, 7, 27, 3, 36, 6, 11, 5, 32, 0, 13, 9, 40, 2, 40, 15, 20, 18, 45, 10, 23, 12, 36, 11, 31, 
    1, 40, 11, 20, 12, 27, 6, 32, 16, 26, 13, 36, 10, 37, 7, 26, 3, 22, 4, 44, 18, 18, 2, 11, 17, 15, 9, 27, 15, 39, 5, 25, 8, 16, 14, 13, 0, 49, 19, 25, 
    9, 40, 8, 11, 14, 47, 2, 35, 13, 41, 7, 37, 1, 37, 18, 28, 6, 42, 3, 23, 10, 41, 5, 33, 17, 25, 0, 19, 19, 15, 16, 42, 12, 37, 11, 34, 4, 10, 15, 41, 
    2, 28, 4, 18, 11, 42, 5, 26, 13, 27, 6, 24, 12, 41, 0, 25, 1, 27, 7, 40, 17, 40, 14, 49, 10, 33, 3, 30, 15, 34, 16, 17, 8, 49, 9, 21, 18, 35, 19, 42, 
    7, 26, 9, 27, 4, 25, 3, 42, 19, 28, 15, 22, 17, 34, 0, 15, 6, 46, 1, 34, 12, 47, 2, 16, 16, 34, 10, 31, 14, 24, 5, 43, 13, 45, 11, 47, 8, 18, 18, 15, 
    4, 30, 8, 48, 1, 46, 15, 13, 9, 20, 7, 31, 14, 20, 2, 20, 16, 34, 19, 38, 18, 12, 17, 11, 11, 47, 5, 19, 0, 35, 13, 17, 10, 23, 12, 11, 3, 22, 6, 11, 
    3, 27, 2, 11, 5, 17, 0, 43, 1, 25, 15, 24, 18, 36, 8, 12, 9, 21, 13, 44, 10, 17, 17, 41, 16, 34, 11, 14, 12, 45, 7, 45, 14, 27, 6, 47, 4, 47, 19, 11, 
    5, 27, 4, 41, 17, 44, 16, 16, 11, 42, 10, 29, 3, 23, 2, 15, 0, 22, 13, 28, 7, 16, 14, 39, 9, 21, 12, 15, 18, 32, 15, 36, 1, 29, 8, 18, 6, 39, 19, 33, 
    4, 44, 19, 38, 11, 24, 17, 21, 13, 34, 15, 11, 10, 16, 8, 43, 16, 41, 7, 45, 3, 37, 9, 10, 6, 36, 18, 31, 2, 17, 14, 28, 12, 43, 0, 22, 1, 25, 5, 15, 
    7, 40, 15, 23, 4, 37, 2, 12, 8, 28, 12, 19, 10, 30, 17, 40, 13, 20, 18, 11, 5, 23, 16, 46, 3, 40, 1, 37, 14, 17, 0, 16, 11, 31, 6, 15, 9, 10, 19, 22, 
    5, 10, 1, 37, 15, 22, 2, 28, 6, 10, 9, 21, 19, 38, 16, 35, 7, 34, 0, 13, 14, 33, 11, 16, 4, 26, 3, 20, 17, 10, 18, 37, 13, 21, 8, 31, 10, 27, 12, 23, 
    16, 32, 6, 32, 7, 20, 1, 14, 0, 11, 19, 27, 3, 21, 18, 32, 10, 33, 13, 13, 17, 36, 8, 25, 4, 32, 5, 41, 15, 44, 2, 32, 14, 12, 9, 32, 12, 10, 11, 28, 
    7, 28, 9, 33, 11, 35, 17, 44, 4, 43, 16, 35, 12, 31, 2, 14, 6, 48, 8, 40, 15, 28, 0, 31, 3, 22, 5, 30, 13, 27, 10, 24, 18, 47, 14, 38, 1, 46, 19, 22, 
    12, 33, 6, 33, 14, 38, 9, 15, 10, 16, 13, 24, 1, 30, 8, 18, 7, 46, 2, 30, 17, 37, 11, 24, 5, 13, 3, 14, 18, 11, 16, 38, 0, 31, 4, 24, 19, 42, 15, 30, 
    10, 15, 16, 12, 6, 43, 18, 27, 0, 24, 9, 20, 3, 41, 2, 22, 12, 41, 11, 30, 5, 26, 4, 24, 7, 45, 13, 46, 14, 22, 15, 11, 8, 20, 1, 42, 19, 11, 17, 49, 
    4, 14, 19, 30, 17, 15, 7, 17, 8, 34, 2, 48, 3, 45, 14, 16, 12, 23, 16, 29, 13, 28, 6, 28, 18, 24, 10, 21, 5, 37, 1, 38, 11, 31, 0, 29, 9, 42, 15, 22, 
    15, 41, 17, 19, 5, 37, 7, 36, 8, 47, 12, 49, 11, 29, 6, 18, 9, 33, 10, 30, 0, 49, 16, 37, 3, 11, 2, 46, 14, 36, 18, 35, 13, 45, 1, 31, 4, 33, 19, 18, 
    9, 42, 4, 11, 15, 28, 18, 48, 6, 22, 8, 15, 1, 37, 11, 36, 3, 26, 19, 21, 2, 48, 16, 17, 12, 30, 10, 27, 13, 35, 17, 20, 0, 18, 7, 14, 14, 20, 5, 41, 
    19, 35, 17, 19, 16, 20, 15, 36, 1, 15, 3, 46, 4, 13, 8, 42, 18, 19, 5, 37, 2, 10, 13, 44, 10, 30, 11, 20, 14, 42, 6, 35, 0, 26, 9, 29, 7, 21, 12, 42, 
    17, 33, 3, 11, 7, 42, 16, 45, 9, 29, 0, 27, 5, 15, 13, 37, 2, 32, 11, 25, 14, 21, 8, 49, 19, 34, 1, 31, 15, 35, 6, 32, 4, 20, 18, 30, 10, 24, 12, 29
  };
  // Yamada and Nakano 20x20 instance (Table 4, instance 4)
  const int yn4[] = {
    20, 20, // Number of jobs and machines
    16, 34, 17, 38, 0, 21, 6, 15, 15, 42, 8, 17, 7, 41, 18, 10, 10, 26, 11, 24, 1, 31, 19, 25, 14, 31, 13, 33, 4, 35, 9, 30, 3, 16, 12, 16, 5, 30, 2, 13, 
    5, 41, 11, 33, 6, 15, 16, 38, 0, 40, 14, 38, 3, 37, 1, 20, 13, 22, 4, 34, 7, 16, 17, 39, 9, 15, 2, 19, 10, 36, 12, 39, 18, 26, 8, 19, 15, 39, 19, 34, 
    17, 34, 1, 12, 16, 10, 7, 47, 13, 28, 15, 27, 0, 19, 6, 34, 19, 33, 12, 40, 9, 37, 14, 24, 8, 15, 10, 34, 2, 44, 3, 37, 18, 22, 11, 31, 4, 39, 5, 26, 
    5, 48, 7, 46, 16, 47, 10, 45, 14, 15, 8, 25, 0, 34, 3, 24, 12, 35, 18, 15, 2, 48, 13, 19, 11, 10, 1, 48, 17, 16, 15, 28, 4, 18, 6, 17, 9, 44, 19, 41, 
    12, 47, 3, 23, 9, 48, 16, 45, 14, 39, 6, 42, 8, 32, 15, 11, 13, 16, 5, 14, 11, 19, 1, 46, 19, 10, 10, 17, 7, 41, 2, 47, 17, 32, 4, 17, 0, 21, 18, 17, 
    18, 14, 16, 20, 1, 18, 12, 14, 13, 10, 6, 16, 5, 24, 4, 18, 0, 24, 11, 18, 15, 42, 19, 13, 3, 23, 14, 40, 9, 48, 8, 12, 2, 24, 10, 23, 7, 45, 17, 30, 
    0, 27, 12, 15, 4, 26, 13, 19, 17, 14, 5, 49, 7, 16, 18, 28, 16, 16, 8, 20, 9, 36, 2, 21, 14, 30, 3, 36, 1, 17, 15, 22, 6, 43, 11, 32, 10, 23, 19, 17, 
    0, 32, 16, 15, 17, 12, 7, 46, 3, 37, 18, 43, 11, 40, 13, 43, 9, 48, 4, 36, 15, 24, 8, 25, 1, 33, 14, 32, 5, 26, 6, 37, 12, 24, 10, 24, 2, 15, 19, 22, 
    10, 34, 6, 33, 15, 25, 8, 46, 0, 20, 18, 33, 4, 19, 13, 45, 2, 47, 1, 32, 3, 12, 11, 29, 16, 29, 5, 46, 12, 17, 7, 48, 14, 39, 17, 40, 19, 41, 9, 37, 
    13, 26, 3, 47, 5, 44, 6, 49, 1, 22, 17, 12, 10, 28, 19, 36, 9, 27, 4, 25, 14, 48, 7, 11, 16, 49, 12, 24, 11, 48, 2, 19, 0, 47, 18, 49, 8, 46, 15, 36, 
    13, 23, 18, 48, 14, 15, 0, 42, 3, 36, 8, 15, 6, 32, 10, 18, 1, 45, 15, 23, 11, 45, 2, 13, 17, 21, 12, 32, 7, 44, 5, 25, 19, 34, 16, 22, 9, 11, 4, 43, 
    17, 37, 7, 49, 15, 45, 2, 28, 9, 15, 8, 35, 12, 29, 13, 44, 1, 26, 4, 25, 5, 30, 3, 39, 0, 15, 14, 28, 18, 23, 6, 42, 11, 33, 16, 45, 10, 10, 19, 20, 
    0, 10, 6, 37, 3, 15, 13, 13, 10, 11, 2, 49, 1, 28, 14, 28, 15, 13, 8, 29, 12, 21, 16, 32, 11, 21, 4, 48, 5, 11, 17, 26, 9, 33, 18, 22, 7, 21, 19, 49, 
    18, 38, 0, 41, 4, 30, 13, 43, 6, 11, 2, 43, 14, 27, 3, 26, 9, 30, 15, 19, 16, 36, 1, 31, 17, 47, 5, 41, 10, 34, 8, 40, 12, 32, 7, 13, 11, 18, 19, 27, 
    6, 24, 5, 30, 7, 10, 10, 35, 8, 28, 16, 43, 19, 12, 9, 44, 15, 15, 3, 15, 2, 35, 18, 43, 0, 38, 4, 16, 1, 29, 17, 40, 14, 49, 13, 38, 12, 16, 11, 30, 
    3, 48, 6, 35, 13, 43, 2, 37, 17, 18, 5, 27, 9, 27, 7, 41, 1, 22, 15, 28, 16, 18, 10, 37, 18, 48, 4, 10, 8, 14, 11, 18, 14, 43, 0, 48, 12, 12, 19, 49, 
    0, 13, 13, 38, 7, 34, 6, 42, 1, 36, 5, 45, 18, 24, 8, 35, 14, 26, 19, 30, 12, 47, 16, 24, 11, 47, 4, 40, 10, 43, 3, 16, 15, 10, 2, 12, 9, 39, 17, 22, 
    16, 30, 13, 47, 19, 49, 8, 20, 4, 40, 3, 46, 17, 21, 14, 33, 6, 44, 7, 23, 9, 24, 0, 48, 10, 43, 15, 41, 2, 32, 5, 29, 11, 36, 1, 38, 12, 47, 18, 12, 
    13, 10, 5, 36, 12, 18, 16, 48, 0, 27, 14, 43, 10, 46, 6, 27, 7, 46, 19, 35, 11, 31, 2, 18, 8, 24, 3, 23, 17, 29, 18, 14, 9, 19, 1, 40, 15, 38, 4, 13, 
    9, 45, 16, 44, 0, 43, 17, 31, 14, 35, 13, 17, 12, 42, 3, 14, 18, 37, 10, 39, 6, 48, 7, 38, 15, 26, 4, 49, 2, 28, 11, 35, 1, 42, 5, 24, 8, 44, 19, 38
  };

  // Taillard 15x15 instance 0
  const int tai_15_15_0[] = {
    15, 15, // Number of jobs and machines
     6,94, 12,66,  4,10,  7,53,  3,26,  2,15, 10,65, 11,82,  8,10, 14,27,  9,93, 13,92,  5,96,  0,70,  1,83, 
     4,74,  5,31,  7,88, 14,51, 13,57,  8,78, 11, 8,  9, 7,  6,91, 10,79,  0,18,  3,51, 12,18,  1,99,  2,33, 
     1, 4,  8,82,  9,40, 12,86,  6,50, 11,54, 13,21,  5, 6,  0,54,  2,68,  7,82, 10,20,  4,39,  3,35, 14,68, 
     5,73,  2,23,  9,30,  6,30, 10,53,  0,94, 13,58,  4,93,  7,32, 14,91, 11,30,  8,56, 12,27,  1,92,  3, 9, 
     7,78,  8,23,  6,21, 10,60,  4,36,  9,29,  2,95, 14,99, 12,79,  5,76,  1,93, 13,42, 11,52,  0,42,  3,96, 
     5,29,  3,61, 12,88, 13,70, 11,16,  4,31, 14,65,  7,83,  2,78,  1,26, 10,50,  0,87,  9,62,  6,14,  8,30, 
    12,18,  3,75,  7,20,  8, 4, 14,91,  6,68,  1,19, 11,54,  4,85,  5,73,  2,43, 10,24,  0,37, 13,87,  9,66, 
    11,32,  5,52,  0, 9,  7,49, 12,61, 13,35, 14,99,  1,62,  2, 6,  8,62,  4, 7,  3,80,  9, 3,  6,57, 10, 7, 
    10,85, 11,30,  6,96, 14,91,  0,13,  1,87,  2,82,  5,83, 12,78,  4,56,  8,85,  7, 8,  9,66, 13,88,  3,15, 
     6, 5, 11,59,  9,30,  2,60,  8,41,  0,17, 13,66,  3,89, 10,78,  7,88,  1,69, 12,45, 14,82,  4, 6,  5,13, 
     4,90,  7,27, 13, 1,  0, 8,  5,91, 12,80,  6,89,  8,49, 14,32, 10,28,  3,90,  1,93, 11, 6,  9,35,  2,73, 
     2,47, 14,43,  0,75, 12, 8,  6,51, 10, 3,  7,84,  5,34,  8,28,  9,60, 13,69,  1,45,  3,67, 11,58,  4,87, 
     5,65,  8,62, 10,97,  2,20,  3,31,  6,33,  9,33,  0,77, 13,50,  4,80,  1,48, 11,90, 12,75,  7,96, 14,44, 
     8,28, 14,21,  4,51, 13,75,  5,17,  6,89,  9,59,  1,56, 12,63,  7,18, 11,17, 10,30,  3,16,  2, 7,  0,35, 
    10,57,  8,16, 12,42,  6,34,  4,37,  1,26, 13,68, 14,73, 11, 5,  0, 8,  7,12,  3,87,  2,83,  9,20,  5,97
  };
  // Taillard 15x15 instance 1
  const int tai_15_15_1[] = {
    15, 15, // Number of jobs and machines
     9,86, 14,60,  4,10, 13,59, 10,65,  3,94,  7,71,  8,25,  0,98,  5,49,  1,43,  2, 8, 12,90,  6,21, 11,73, 
    10,68,  8,28, 11,38, 14,36,  3,93, 13,35,  9,37,  7,28,  4,62,  2,86,  6,65,  1,11,  5,20, 12,82,  0,23, 
     7,33,  0,67,  6,96,  5,91, 14,83, 13,81,  2,60, 11,88,  4,20, 12,62,  1,22,  9,79,  3,38, 10,40,  8,82, 
     9,13, 11,14, 14,73,  0,88,  1,24,  8,16,  5,78, 10,70, 12,53,  4,68, 13,73,  3,90,  6,58,  7, 7,  2, 4, 
    11,93,  4,52, 13,63,  3,13,  8,19,  1,41, 10,71, 12,59,  2,19, 14,60,  6,85,  7,99,  0,73,  9,95,  5,19, 
     5,62,  2,60,  1,93, 10,16,  0,10,  4,72,  8,88, 14,69,  6,58,  3,41,  9,46,  7,63, 11,76, 12,83, 13,62, 
     5,50, 10,68, 13,90,  0,34,  9,44,  8, 5,  1, 8, 11,25, 14,70,  7,53, 12,78,  2,92,  6,62,  4,85,  3,70, 
    12,60,  0,64,  9,92,  3,44, 13,63,  6,91,  5,21,  7, 1,  2,96, 14,19, 11,59,  8,12, 10,41,  1,11,  4,94, 
    11,93, 10,46,  5,51, 13,37,  1,91,  9,90,  8,63,  7,40,  3,68,  6,13,  0,16,  2,83, 14,49, 12,24,  4,23, 
     2, 5, 14,35,  3,21, 10,14,  6,66,  1, 3,  0, 6, 13,98, 11,63,  4,64,  5,76,  8,94,  7,17, 12,62,  9,37, 
    11,35, 14,42, 13,62,  5,68,  4,73,  9,27,  1,52,  6,39, 12,41,  0,25,  2, 9,  8,34, 10,50,  3,41,  7,98, 
    12,23,  3,32, 10,35,  8,10,  4,29,  7,68, 13,20, 11, 8, 14,58,  1,62,  2,39,  0,32,  5, 8,  6,33,  9,91, 
     8,28, 13,31,  5, 3,  0,28, 11,66,  9,59,  4,24, 12,45,  1,81, 10, 8,  6,44,  2,42,  7, 2, 14,23,  3,53, 
     2,11,  5,93,  4,27,  3,59,  9,62,  1,23, 11,23, 13, 7,  7,77,  6,64, 10,60, 14,97,  0,36,  8,53, 12,72, 
     1,36, 10,98,  4,38,  2,24,  0,84,  7,47,  6,72,  9, 1, 11,91, 12,85,  5,68, 14,42,  3,20, 13,30,  8,30
  };
  // Taillard 15x15 instance 2
  const int tai_15_15_2[] = {
    15, 15, // Number of jobs and machines
     7,69, 11,81,  8,81,  3,62, 12,80,  1, 3, 13,38,  0,62, 14,54,  6,66,  9,88,  4,82,  2, 3, 10,12,  5,88, 
    12,83,  1,51, 11,47,  9,15,  6,89,  3,76,  2,52,  4,18,  5,22,  8,85, 13,26, 14,30, 10, 5,  0,89,  7,22, 
     1,62,  2,47,  9,93,  0,54,  3,38,  5,78,  8,71,  4,96, 14,19, 10,33, 12,44, 13,71,  7,90,  6, 9, 11,21, 
    13,33, 10,82,  6,80,  2,30, 14,96,  7,31,  4,11, 11,26,  0,41,  5,55,  9,12,  3,10,  8,92,  1, 3, 12,75, 
     1,36,  8,49,  4,10, 14,43,  6,69,  5,72,  3,19,  2,65,  9,37, 10,57, 13,32,  7,11, 11,73,  0,89, 12,12, 
     5,83, 14,32,  2, 6, 12,13, 10,87,  1,94, 11,36,  4,76,  6,46,  9,30,  0,56, 13,62,  8,32,  3,52,  7,72, 
     5,29,  2,78,  0,21,  1,27,  8,17, 14,43, 11,14, 10,15,  7,16,  9,49,  6,72, 12,19,  4,99, 13,38,  3,64, 
     4,12,  7,74, 10, 4,  1, 3,  9,15,  8,62,  2,50, 14,38, 11,49,  3,25,  5,18,  6,55, 13, 5, 12,71,  0,27, 
    14,69, 11,13,  0,33,  9,47, 10,86,  5,31,  3,97, 12,48,  8,25, 13,40,  6,94,  1,22,  7,61,  2,59,  4,16, 
     9,27,  0, 4,  3,35, 10,80, 12,49, 13,46,  5,84,  1,46,  6,96, 14,72,  8,18, 11,23,  2,96,  7,74,  4,23, 
     7,36,  2,17,  1,81, 12,67,  3,47, 14, 5,  4,51,  6,23,  5,82,  9,35,  8,96, 13, 7, 10,54,  0,92, 11,38, 
     0,78,  8,58, 14,62, 12,43,  9, 1,  5,56,  6,76, 10,49,  7,80, 11,26,  3,79,  4, 9,  1,24, 13,24,  2,42, 
     8,38, 12,86, 10,38, 11,38, 14,83,  3,36,  6,11,  1,17,  4,99,  5,14,  0,57,  9,64, 13,58,  2,96,  7,17, 
    13,10,  2,86, 11,93,  0,63, 14,61, 10,62,  3,75,  1,90, 12,40,  4,77,  5, 8,  6,27,  7,96,  9,69,  8,64, 
     1,73, 13,12,  0,14, 11,71,  2, 3, 10,47,  4,84,  8,84,  3,53,  5,58,  7,95,  6,87,  9,90, 12,68, 14,75
  };
  // Taillard 15x15 instance 3
  const int tai_15_15_3[] = {
    15, 15, // Number of jobs and machines
     3,72,  7,51,  6,42, 14,31,  9,61,  8,46,  5,88,  4,33, 10,27,  1,85,  0,70, 12,56, 11,70,  2,50, 13,25, 
     1,19, 11,79,  0,79,  5,47,  8,40, 13,67,  3,43, 10,65,  9,84,  2,61, 12,30,  6,56,  4,19,  7,91, 14,68, 
     7,94,  3, 7,  8, 2, 11,95,  0,60,  4,82,  9,76, 13,36,  1, 8, 10,85,  6, 7,  5,44, 14, 2, 12,72,  2,91, 
     6,58, 11,67,  5,84, 13,34,  3,19,  2,19,  1,94,  4,41,  9,98, 12,96,  8,25,  0,40, 14,74,  7,88, 10,74, 
     1,45, 11,60,  6, 8,  5,29,  8,32,  7,42, 12,25,  9, 4,  2,71, 14,79, 13,93,  3,28,  0,30,  4,17, 10,43, 
    11,84,  2,56, 12,46,  1,93, 14,66,  0,84,  6,40,  9, 4,  3,15, 13,15,  7,54, 10,39,  4,77,  5,55,  8,31, 
    14,65,  7,91, 11,17,  0,47, 12,77,  8,68,  9,62, 10,22,  3,72, 13,47,  4,38,  1, 7,  2,11,  6,22,  5,63, 
     7,12,  4,21, 13,60,  5,42, 14,22, 11,84,  2,60, 12,52,  6,25,  1,53,  0,53, 10,56,  9,29,  3,83,  8,32, 
     7,48,  4,28, 14,70, 10,26,  3,68,  0, 4, 13,19,  8,92,  1,24,  2,54,  6,57, 12,47, 11,84,  9,85,  5,95, 
     2,36, 12,34, 10,65,  3,64,  6,30,  5,41, 14,53,  9,74, 13,44,  4,13,  0,41, 11, 6,  7,32,  8,94,  1,37, 
    11,62,  9, 9,  3,89,  7,37,  6,28,  4,23,  0,13,  2,60,  5,46,  1,94, 10,85,  8,72, 13,18, 12,79, 14,11, 
     0,74,  4,61,  1,43,  9,26,  2,97, 11,62, 13,40, 10,60,  7,62,  6,78,  8,42, 12, 8, 14,21,  3,11,  5,70, 
     6, 9,  0,22,  4, 9, 10, 8,  2,54, 14,32,  9,92, 13,76, 11, 2, 12,63,  1,63,  8,98,  5,42,  3,12,  7,41, 
     5,67, 10, 7, 13,91,  2,52,  7,87,  4, 4,  8, 1,  6,56,  9,82,  0,47, 12,35, 11, 8,  1,92, 14,39,  3,11, 
    11,44,  2,24,  0,24,  4,14, 14,34,  5,57,  9,30, 10,64,  6, 4, 12,14,  8,69,  1,95,  7,22,  3,60, 13,61
  };
  // Taillard 15x15 instance 4
  const int tai_15_15_4[] = {
    15, 15, // Number of jobs and machines
    12,40,  1,96,  4,59,  9,95, 13,76,  0,75, 11,23,  8,65,  3,65,  5,16,  6,71, 14,52, 10,84,  2,99,  7,24, 
     5, 2,  1,88, 14,99, 10,52, 13,68,  9,13,  6,38, 12,35,  8,57,  2,37,  7,93,  0,38,  3,68,  4,94, 11,71, 
     6,87,  3,46, 10,14,  7,87, 13,30,  4,79,  5,62,  9,37,  8,54,  2, 1,  0,97,  1,16, 11, 2, 14,51, 12,96, 
    10,19,  7,15,  5,42,  0, 8,  9,72, 13,15,  2,76,  8,25,  1,78, 14,84, 11,62,  3,70, 12,81,  6,16,  4,97, 
     6,68, 12,71, 14, 3,  1,68,  7,91,  5,37, 11,73,  0,21,  2,85,  3,79,  8,51, 13,50,  4,21,  9,30, 10,64, 
     4,14,  7, 1,  6,29,  0,72,  8, 6, 13,31, 12,98, 14,50,  3,83,  2, 2,  5,86,  9,33, 10,33,  1,98, 11,59, 
    10,21, 11,80,  6,99,  9,70,  0,80,  2,71,  1,47,  8,96, 12,56,  7,78,  5,53,  3,10, 13,92, 14, 1,  4,33, 
     3,29, 10,85,  5,89,  6,10,  8,30,  4,38,  0,38, 14,48,  2,16,  7,65,  9,90, 11,73, 12,88,  1,46, 13,47, 
     1,37,  2, 9,  6,49,  7,23, 10, 1, 12,78, 14,39,  8,15,  0, 9,  3,41, 13,35,  5,83,  4, 8,  9,61, 11,60, 
    12, 1,  7,73,  6,47, 14,46,  3,10,  4,37,  0,60, 13,84, 10,26,  8,11, 11,37,  9,79,  5,75,  2,49,  1,51, 
     4,22,  3,49,  8,33, 14, 2,  2,24, 13, 3,  5,73,  6,68, 10,21, 12,61,  7,69, 11,94,  1,43,  9,39,  0,48, 
     6,81, 12,46,  7,21,  5,23,  2,86,  4,19, 13,64, 11,52,  8,22,  0,50, 10,11,  3,73,  1,77,  9,16, 14,75, 
    12,21, 13,80,  8,30,  7,32,  1,22,  6,23,  0,85,  5,92,  9,14, 10,13,  4,68,  2,60, 14,45,  3,32, 11,90, 
    12,29,  1,95,  4,52,  8,59,  6,33, 10,12,  7,73,  3,96,  0,75,  5,12, 13,83,  2, 3,  9,90, 14,57, 11, 6, 
     7,94,  5,18,  0,54, 10,42,  2,70,  3,29,  9,43,  6,50, 11,75,  8,70,  1,40,  4,48, 14, 1, 12,27, 13,12
  };
  // Taillard 15x15 instance 5
  const int tai_15_15_5[] = {
    15, 15, // Number of jobs and machines
     7,96, 12,23,  5,71,  8,26,  3,28, 14,16, 13,27,  9,71,  0,18,  1,57,  4,43,  2, 5,  6,12, 11,91, 10,63, 
     8,32,  0,81,  5,95, 13,79,  6,55,  1,45,  4,60,  2,73,  3,23, 11,44, 10,92, 12,20,  9, 5, 14,72,  7,73, 
     6,63,  8,93,  7,63,  1,79,  9,10,  5,66, 12,27,  4,93,  0,24,  2,26, 14, 8, 11,69,  3,29, 13,66, 10,97, 
    12,80,  0,87, 14,68,  7,23, 13,54,  1,16,  6,68,  8,32, 11,74,  2, 3,  3, 2,  9,71,  5, 4, 10,67,  4,28, 
     8,46,  2,96, 12,11, 11,41, 10,93,  9, 2,  4,98,  3,10, 14,43,  5,65,  0,27,  7,57,  6,75, 13,87,  1,81, 
    13, 5,  8,91,  6,92, 11,87, 14,66,  9,36, 12,67,  7,88,  0,92,  1,27, 10,13,  2, 7,  5,95,  3,66,  4,13, 
     4,90,  1,33,  2,78, 13,76,  7,93,  6,67,  0,82,  8,94, 12,12, 14, 5, 10,85,  9,42,  5, 4,  3, 2, 11,70, 
     4,79, 10,24, 14,41,  3,83,  6,45, 13,29, 11, 3,  9,42,  0, 5,  5,44,  1,83, 12,59,  8,60,  7,78,  2,44, 
     7,19,  1,55,  5,20,  4,74, 14,66,  8,37,  0,55,  9,63, 12,40,  3,73, 10,55,  2,84, 13,54, 11,62,  6, 6, 
     3,27,  6,59, 13, 6,  4,90,  5, 6, 10,37,  2,64,  8,35, 12,25, 11,59,  9,77,  7,30, 14, 1,  1, 7,  0,70, 
     1, 4,  4,53,  7, 6,  2,10, 14,51, 10,89, 12,38, 13,38,  6,35,  5,44,  9,99,  0,88,  3,52,  8,16, 11,99, 
     0,28,  7,11, 14,76, 13,51,  8,35,  3,60,  6,44,  9,39,  5,66, 12,49, 10,40,  4,34,  1,80,  2,38, 11,29, 
    14,31, 10,32,  0,40,  8,25, 12,40, 13,85,  4,39, 11,61,  1,15,  3,41,  6,93,  5,64,  2,16,  9,81,  7,97, 
    13, 9,  4,21,  8, 8,  6,55,  5,79,  2,76,  9,79,  0,61, 11,68, 12,99,  1,24,  3,23, 14,92,  7,91, 10,22, 
    14,80,  2,30,  6,67,  4,58,  3,45, 11,29,  1,48,  5,28,  7,64,  0,63,  8,80, 10,23,  9,93, 12,55, 13,48
  };
  // Taillard 15x15 instance 6
  const int tai_15_15_6[] = {
    15, 15, // Number of jobs and machines
    13,52, 12,19,  7, 6,  0,20,  5, 1, 11,26,  9,90, 10,44,  1,27,  2,18,  3,51, 14,80,  4,10,  6,51,  8,41, 
     2,44,  1,85,  8, 2, 14,78, 13,86,  0,88,  5,61, 11,20,  9,56, 12,12, 10,69,  7,34,  3,55,  4,34,  6,84, 
     0,62,  3,72,  1,74,  7,63,  4,95,  8,29, 14,24,  5,34,  6,89,  2,83, 13,90, 12,26,  9,98, 10,65, 11,31, 
     9,10, 11,15,  0,93,  7,79, 12,77, 14,61,  6, 1,  1,48, 13,22, 10,27,  4,21,  5,17,  3,45,  2,96,  8,11, 
     8,83, 13,52,  5,70, 10,78,  9, 7, 14,28, 12,97,  2,52,  3,29,  0,81,  6,60, 11,91,  7,80,  4,54,  1,35, 
    12, 3, 13,31, 14,98,  9,97,  5,77,  1,39,  6,41,  7,10,  0, 9,  3,93,  8, 7, 10,49,  4,20, 11,45,  2,59, 
     9,28,  0,93,  5, 4,  4,51, 12,67, 14, 5,  6,18,  1,52,  3,47,  8,21, 13,49, 11,63,  2,96, 10,85,  7,90, 
    12,25,  1,82,  9,58, 14,15,  6,67, 10,50,  2,66,  7,92,  4,56, 11,82, 13,57,  5,16,  3,34,  8,99,  0,61, 
     6,82,  5,31, 12,22,  4,16,  1,87, 14,48,  9,59,  0,63,  7,29,  8,99, 10,48, 13,36,  2,91, 11,61,  3,59, 
     8,28,  3,25,  5,69,  4,65,  1,62, 10,57,  7,97,  9,31, 13,15,  2,25,  0,83, 11,98,  6,55, 12,66, 14,31, 
     4,20,  2,99,  1,13,  0,88, 14,25, 10,75,  9,90,  6,84, 11,70, 12,41,  3,17,  8,54,  7,63, 13, 1,  5,95, 
     8,59,  3,22, 13,46, 14,10,  2, 1, 11,21,  5, 3,  4,84,  9,93, 12,59, 10,78,  7,73,  1,59,  0,42,  6,63, 
     7,72,  1,80, 10,12,  4,56,  0,22,  8, 8, 12,93,  6,27, 13,17,  2,38,  3,26,  9,51, 11,43, 14,80,  5,94, 
     8,72, 11,78,  3,29,  7,90,  1,46, 12,46,  6,43,  4,75, 10,90, 13,29,  2, 8,  5,92, 14,16,  9,62,  0, 6, 
     5,89, 13,44, 14,41,  6,32,  9,10, 11,85,  7,16,  2,23, 12,91,  8,46,  3,35,  4,17, 10,93,  1,45,  0,93
  };
  // Taillard 15x15 instance 7
  const int tai_15_15_7[] = {
    15, 15, // Number of jobs and machines
     3,83,  6, 1,  7,96, 13,54,  4,30,  1,80, 12,81, 10, 9,  8,49, 14,32,  0,19,  9,92,  5,65,  2,88, 11,64, 
     2, 4, 12,68,  1,79, 13,21,  3,84,  4,92,  8,66, 14,51,  0,83, 10,96,  7,68,  5,38,  9,38, 11,99,  6,76, 
     8,46, 14,57,  3,66,  0,75, 10,88,  5,58,  9,56, 13,35,  2,59, 11,82,  6,24,  4,96,  7,24,  1,55, 12,80, 
     5,34, 13,69, 11,53,  4,98, 14, 8,  2,81,  9,81,  8,38, 10,39,  3, 3,  6,59,  7,81,  1,30,  0,76, 12,71, 
    13,85,  5,80,  2,36, 14,57,  1,96,  8,34,  4,14,  3, 3, 11,90,  6,99,  7, 9, 12,42,  9,95,  0,27, 10,27, 
    14,28, 11,11,  2,66,  5, 2,  8,35, 10,69,  0,61,  4,84,  9,73,  7,56, 13,98,  3,81,  1,72,  6,92, 12,23, 
     8,21, 12, 5,  5,95, 10, 5,  7,22, 11,16,  2,77, 13,85,  0,76, 14,46,  6,36,  9,89,  1,99,  4,44,  3,37, 
     9,49,  7,80, 12,61,  3,87,  6,41,  2, 6, 10,83, 13,79,  1,44,  8,83, 14, 9,  4,84,  0,99,  5,38, 11,68, 
     0,77,  4,51,  5,68,  1,69, 11, 6,  7,26,  6,99,  2, 6, 14,34, 13,27,  8,51,  9,82,  3, 5, 10,90, 12, 1, 
     8,85, 11,64,  7,55,  1,76, 14,89, 13,68, 12,34,  4,14,  6,52,  9,33, 10,91,  0, 4,  5,18,  2,95,  3,76, 
    12,40,  4, 8,  5,36, 13, 5,  6, 1,  0,51,  1,33,  7,80,  9,90,  8,75, 11,47, 10,65, 14,42,  2,16,  3,11, 
     3,38, 14,83,  4,48, 13,74,  5,15,  9,10,  8,89,  0,41,  7,97, 12,97,  6,16,  1,47, 10,21, 11,95,  2,20, 
    10,89,  8,22,  4,11,  1,15,  2,37,  0,65,  7,28, 14,39, 12,88, 13,14,  9,28,  6, 6,  3,24, 11, 4,  5,23, 
     6,14,  2,66,  0, 4, 14,58,  1, 7,  4, 6, 12, 5,  3,48, 10,54,  7,59,  8, 2,  9, 1,  5, 4, 13,82, 11,75, 
    10,24,  2,66,  9, 4, 13,20,  5,79,  0,50,  6,23,  7,15,  1,14, 14,91, 11,86,  4,96, 12,63,  3,16,  8, 3
  };
  // Taillard 15x15 instance 8
  const int tai_15_15_8[] = {
    15, 15, // Number of jobs and machines
     3,91, 13,15, 11,45,  4,26,  2,90,  9,53, 10, 7,  6,78, 12,94,  7, 8,  5,19, 14,56,  8,69,  1,66,  0,98, 
    13,34,  9, 1, 14,40,  0,74,  4,43,  5,73,  6,96, 11,80,  1,87,  7,78,  8,88, 12,90, 10,49,  3,85,  2, 3, 
     1,88, 14,98, 13,82,  8,46, 10,79,  4,69,  0,95, 12,41,  5,39,  2,12, 11, 1,  6,71,  9,27,  7,77,  3,99, 
    14,50,  5, 1,  4,21,  3,72, 10,46,  7,20,  1,62, 11,33, 13,79,  6,56,  9,67, 12,23,  2,56,  8,44,  0,56, 
    14,15,  1,15, 13,16,  6,79, 12, 8,  4,73,  5,86,  2,52,  9,79,  0,62,  3,93,  7,86, 11,44,  8,80, 10,18, 
     9,79, 11,63, 14,94, 10, 9, 13,86,  7,89,  8,12,  2,66,  1,55,  0,70, 12,35,  5,14,  4, 3,  6,54,  3,62, 
     3,42,  7,39, 11,42,  8, 9,  4,37,  9,25,  1,78, 12,76, 10,16,  6,38,  0,30, 14,80,  5,34,  2,92, 13,29, 
    14,96, 13,25,  5,49,  0,67,  9,53,  8,20,  2,52,  3,29,  1,51, 11,35,  6,38, 10,18,  4,43,  7,46, 12,98, 
     3,73,  2,68,  0, 3, 11,98,  1,68,  4, 8,  5,15, 13,88,  7,72, 12,20,  9,89, 10,59,  8,68,  6,63, 14,41, 
    14,30,  9,43, 11,80, 13,64,  1,14,  5, 6,  3,36,  4,88,  0,71, 12,51,  6,63,  2,32,  8,16, 10,63,  7, 7, 
    14,18, 12,90,  2,55,  5,25,  4,72,  1,92, 13,88,  0,69,  8,89, 10,83,  7,58, 11,35,  3,79,  6,43,  9,86, 
     4,50, 13,64,  3,88,  1,57, 11,25,  8,73,  9,18,  7, 4,  0,69,  2,40,  6,28, 10,37,  5,42, 12,82, 14,83, 
    11, 2,  9,41,  1,13,  8,75, 12,31, 14,66,  2,72, 10,66,  7,96,  0,45, 13,29,  6,49,  3,96,  5,50,  4,38, 
     1,80,  4,90,  5,36,  6,50, 11,76,  3,15,  9,31, 10,89,  8,87,  2,55,  7,49, 13,23, 14,19, 12,38,  0,93, 
    13,75,  2,45,  4,75, 12,72,  7,65,  3, 6,  8,16,  0,24, 11,24, 14,44,  9, 4,  6,22,  1,99,  5,10, 10,85
  };
  // Taillard 15x15 instance 9
  const int tai_15_15_9[] = {
    15, 15, // Number of jobs and machines
     8,35,  2,78,  7,79, 14,65, 12,53,  0,14,  4,93,  3,70, 13,14,  5,90,  9,95, 10,49,  6,36,  1,85, 11, 1, 
     2,83,  8,41,  6,22,  4,29, 11,52,  0,71,  9,16,  7,93,  5,54,  3,63,  1,12, 10,85, 13,62, 14,45, 12,30, 
     5,60, 11,43,  8,71, 14, 2, 12,50,  1,37,  4,86,  9,81, 10,60,  6,57, 13,66,  0,24,  2,98,  3,92,  7,69, 
     9,14, 13,59,  8,35,  6, 6, 12,25,  5,57, 10, 1,  2,44,  1,94,  0,30, 14,95, 11,93,  3,51,  4,52,  7,16, 
     8,96, 10,39,  3,75, 13,98, 14, 2,  5,38,  4,69,  0,32, 12,95, 11,63,  9, 4,  6,11,  2,50,  1,95,  7,78, 
     7,73,  2,28,  9,43,  5,47, 10,57,  4,88,  3,33,  1,13, 11, 7,  8,49,  6,23, 14,38, 13,21,  0,99, 12,72, 
     2, 3, 10,80,  1,67, 12,93, 14,91,  3,31,  0,52,  4,64,  9,83,  5, 2, 11,90, 13,64,  7,16,  8,18,  6,25, 
    13,23,  6,30, 11,22, 12,54,  8,68,  7,63, 14,89,  5,95,  3, 5,  4,37,  0, 5, 10,42,  2,17,  1,54,  9,46, 
     8,44,  4,59,  6,87,  7,62,  0,51, 11,55, 13, 3, 12,40,  2,26,  3,18, 10,15,  1,18,  5,72, 14,35,  9,60, 
     2,27,  6,14, 12,77,  9,24,  8,55, 11,67,  1,59,  3,19, 13,29,  4,33, 10,88,  5,30,  0,91,  7,11, 14,11, 
     0,67,  9,94,  4,50,  8, 2, 10,83,  2,19, 11,29, 12,37, 14,58,  6,32,  3,38,  7,99,  5,88,  1,49, 13,70, 
     8,60,  1, 7, 12,81,  5,82,  7,58, 10,83,  9,16,  4, 1, 11,69,  6, 7, 13, 3,  2,84, 14, 8,  0,12,  3,93, 
    13,92, 14,81,  2, 4,  9,78,  7, 9,  8,78, 12,75,  4, 5, 10,50,  6, 8,  1,44,  0, 4,  5,60, 11,94,  3,74, 
     1,32,  0,88, 12,31,  2,68, 13,31,  8,10,  3,45, 14,75,  4,82,  7,51,  6,55,  9,99, 11,44, 10,84,  5,22, 
     3,12, 13,35,  2,64,  5,17,  9,42,  6,46,  7,65,  4,74,  8,96,  0,28, 12,86, 10,95, 14,93,  1,67, 11,56
  };
  // Taillard 20x15 instance 0
  const int tai_20_15_0[] = {
    20, 15, // Number of jobs and machines
     3,25, 11,75, 14,75,  1,76, 10,38,  2,62,  4,38,  7,59,  0,14, 12,13,  5,46,  9,31,  6,57, 13,92,  8, 3, 
     5,67,  0, 5,  3,11,  8,11,  4,40,  1,34, 12,77, 14,42,  6,35,  7,96, 10,22,  2,55,  9,21, 13,29, 11,16, 
     2,22,  3,98, 14, 8,  0,35,  9,59, 12,31,  5,13,  4,46,  7,52, 10,22,  8,18, 11,19, 13,64,  1,29,  6,70, 
     8,99, 10,42,  1, 2, 13,35,  3,11,  4,92, 14,88,  9,97,  2,21,  5,56, 11,17,  7,43,  0,27,  6,19, 12,23, 
    14,50,  8, 5,  1,59,  2,71, 10,47,  9,39, 12,82,  4,35,  6,12,  5, 2,  0,39, 13,42,  3,52, 11,65,  7,35, 
     3,48, 10,57,  1, 5,  5, 2,  6,60,  0,64,  8,86,  7, 3, 11,51, 13,26,  2,34, 14,39, 12,45,  9,63,  4,54, 
     2,40, 10,43,  1,50, 12,71,  8,46,  0,99,  7,67,  6,34, 14, 6, 13,95,  4,67,  3,54,  5,29,  9,30, 11,60, 
     1,59,  0, 3,  2,85,  4, 6,  7,46, 13,49, 11, 5,  3,82, 12,18,  5,71,  6,48, 14,79,  9,62,  8,65, 10,76, 
     4,65,  5,55,  9,81, 10,15,  7,32,  6,52,  2,97,  1,69, 12,82,  3,89, 13,69,  0,87,  8,22, 14,71, 11,63, 
     1,70,  4,74,  3,52, 10,94, 14,14,  0,81,  6,24, 13,14, 11,32,  8,39,  5,67, 12,59,  7,18,  9,77,  2,50, 
     3,18, 10, 6,  1,96,  0,53,  9,35,  8,99, 14,39,  6,18,  4,14,  7,90,  2,64, 12,81,  5,89, 11,48, 13,80, 
     2,44,  7,75,  6,12,  8,13,  3,74,  5,59, 14,71,  4,75,  1,30,  0,93,  9,26, 10,30, 13,84, 11,91, 12,93, 
     0,39,  7,56, 14,13,  8,29, 12,55, 10,69,  9,26,  3, 7,  6,55,  1,48,  4,22,  2,46, 11,50, 13,96,  5,17, 
    12,57,  3,14,  9, 8,  4,13,  1,95,  0,53, 10,78,  6,24,  5,92,  2,90, 14,68, 13,87,  7,43,  8,75, 11,94, 
     3,93, 14,92,  6,18,  5,28, 13,27,  9,40,  1,56,  0,83, 12,51,  7,15,  2,97,  4,48, 10,53,  8,78, 11,39, 
     5,47, 14,34,  6,42, 12,28,  8,11,  2,11,  4,30,  9,14, 11,10, 13, 4,  3,20,  1,92,  7,19,  0,59, 10,28, 
     3,69,  7,82, 10,64, 14,40,  0,27,  8,82,  1,27, 11,43,  5,56, 13,17,  4,18, 12,20,  6,98,  9,43,  2,68, 
    10,84,  8,26,  2,87, 11,61, 13,95,  6,23, 14,88,  3,89,  9,49,  7,84,  4,12,  5,51, 12, 3,  0,44,  1,20, 
     3,43,  2,54, 12,18, 13,72,  1,70,  6,28, 14,20,  5,22,  4,59,  8,36,  9,85, 11,13,  0,73, 10,29,  7,45, 
    11, 7, 14,97,  5, 4,  6,22, 10,74,  9,45, 13,62,  1,95,  4,66,  8,14,  0,40,  3,23, 12,79,  2,34,  7, 8
  };
  // Taillard 20x15 instance 1
  const int tai_20_15_1[] = {
    20, 15, // Number of jobs and machines
     2,55,  5,66,  1,48,  8,59,  3, 8,  4,21, 14,64,  7, 7, 10,80, 13, 5, 11,59,  9, 8, 12,91,  6,11,  0,81, 
    14,86,  8,76, 12,40,  4,76, 11, 9,  3,23,  6,80,  9,51,  0,46, 10,48,  1,68, 13,51,  2,15,  7, 5,  5,82, 
     7,84, 12,97,  1,26,  8,70,  2,33, 10,31,  3,20, 11,39, 13,42, 14,33,  5,70,  6,84,  0,23,  9,54,  4,55, 
     1,60,  8,82,  5,14, 11,36,  7,22,  6,21,  3, 3,  2,11,  4,82,  9,92, 12,52, 13,85,  0,77, 14, 3, 10,89, 
     8,83,  4,33, 12,15,  1,36,  3,96, 14,99,  2,81,  9,24, 13,59,  6,89,  5,11, 10,13, 11,26,  7,91,  0,87, 
     2,51, 14,20, 10,89,  7,99,  3,95,  0,41,  1, 7, 13,67,  9,77,  6,45, 11,74, 12,91,  4,87,  5, 1,  8,55, 
     0,35,  4,71, 13,47,  1,34,  8,77, 10,68, 11,85,  6,27,  9, 2,  2,99,  5, 9, 12,18,  7,28,  3,33, 14,92, 
    12,76,  2,58, 11,37,  9,28,  8,80, 10,96, 13,97,  4,92,  5,84, 14,68,  6, 1,  3,86,  1,33,  7,66,  0,20, 
     7,17,  3,11,  4,18,  5,90, 13,57,  0,95, 11,17, 10,33,  9,61,  1,49,  8,36,  6,38, 12,62, 14,73,  2,25, 
     7,82, 14,84, 12,87,  6,44,  5,96,  9,64, 10,68,  0,57,  3,65,  4,89,  2,42,  1,77,  8,43, 13,76, 11,38, 
     0,54,  9,66, 13, 8,  7,48,  6,84,  3,15, 11,93,  8,94, 10,57,  4,16,  2,64,  1,13, 14,62, 12,63,  5,53, 
    14,21, 10,70,  3,42, 12,29,  5,83,  7, 5,  4,16,  6,76,  1,67,  2,46,  0,67, 13,83,  9,46, 11,29,  8,26, 
    11,96, 10,42,  5,49,  1,54,  3,58, 13, 8,  9,41,  7,14,  8,35, 14, 9,  0,74,  2,16,  6,50, 12,69,  4,45, 
     7,69,  6,90, 13,17, 10,18,  3,45,  1,48,  5,31, 11,29,  4,27,  8,85,  0,71, 12,92,  9,20,  2,11, 14,86, 
     2,41,  4,24, 14,82,  5,50,  0,24,  7,75, 10,34, 12,80,  1,71,  9,54,  3, 5,  8,42,  6, 8, 13,35, 11,93, 
     5,63, 14, 4, 12,85,  7,53,  3,61, 13,54,  0,16,  9,18, 10, 5,  6,43,  1,24,  2,88,  8,67,  4,79, 11,41, 
    14,17,  8,37,  1,56, 10,70, 11,56, 12,24,  7,95,  0,12,  3,96,  2,27,  9,55, 13,36,  5,41,  6,65,  4,23, 
     5,79,  2, 6, 10,89,  3,69, 12,16,  4,56,  6,81, 11,98,  0,12, 13,19,  8,88, 14, 3,  7,36,  1,67,  9,74, 
     0,38,  9,76, 10,47,  4,21,  2,80,  6,97, 13,35,  1,45,  3,74, 14,92,  7,98,  8,54,  5,91, 12,79, 11,46, 
     2,34, 12,56,  9,26,  8,62,  4,82,  6,38, 10,89, 14,33,  7,50,  5,62,  1,39, 11,63,  3,88,  0,13, 13,42
  };
  // Taillard 20x15 instance 2
  const int tai_20_15_2[] = {
    20, 15, // Number of jobs and machines
    12,91, 11,17,  8, 4,  9,63,  7,67, 13,30,  0,87, 10,80,  2,95,  4,14,  5,17,  6,22,  3, 1, 14,85,  1,41, 
     2,77,  5,77,  0, 9, 10,77,  4,24,  1, 8, 13,64, 11, 6,  3,12, 14,13,  6,71,  8,76,  9,95, 12, 8,  7, 6, 
     3,92,  2, 3,  6,12,  9,27,  5,58, 12,66,  0,99, 13,33,  7, 7, 10,78, 14,96,  1,30,  4,54,  8,23, 11,88, 
     9,19, 14,45,  5,65,  7,24, 13,30, 10,30, 12,49,  2,32,  3,78,  8,31,  1, 3,  6,25, 11, 9,  0, 2,  4,22, 
     3,84,  8,61,  6,35,  1,44, 13,37,  0,16,  9,97,  5,85, 14,51,  2,26, 12,13,  7,76,  4,41, 11, 2, 10,96, 
     3,85,  8,55, 13, 2, 11,65, 12,52,  1,97,  4,81, 14, 8,  6,22,  5,59,  9,95,  0,52,  7,85, 10,64,  2,13, 
    11,64,  2,94,  0, 4,  6,13,  4,98,  1,26,  5,32, 12,20,  9,97,  7,28,  3,63,  8, 2, 13,23, 10,14, 14,62, 
    12,56,  3,98,  4,56, 11,28,  8, 1,  1,96,  5,27,  0,38, 13,41,  6,94, 10,77,  9,63,  2,63, 14,81,  7, 6, 
     5,63,  8,98,  7,64,  0,37, 12,89, 13,96,  1,88,  6,13,  4,72, 14,28, 10,57,  9,99,  3,11,  2, 8, 11,96, 
     6,17,  3,71,  9,80,  8,33,  2,87,  1,82, 12,44,  7,14,  5,85, 14, 2,  0,60,  4,72, 11,27, 13,63, 10,66, 
     6,47,  0,42,  4,61, 14,17,  8,65, 13, 5, 12,96, 11,47, 10, 9,  1,20,  9,10,  7,11,  5,86,  3,90,  2,65, 
     0,66,  6,91,  4, 8,  3,37,  8,99,  2,90, 11,16,  1,89,  9,17,  5,98, 13,87, 12, 8, 10,40, 14,33,  7,37, 
     2,99,  9, 2,  5,22, 12,12,  3,13, 14,62,  4,30, 13,44, 11,25,  7,56,  1,10, 10,44,  8,25,  6,39,  0,65, 
    11,35,  7,62, 10,52,  4,84,  8,30, 14, 2,  6,50,  3,69,  9,64,  5,54,  1,45, 13,38,  2,90, 12,70,  0,37, 
     8,73, 13,40, 10,16, 12,21,  0,50,  9,10,  2,46,  6, 2, 11,48,  1,16,  3,58, 14,37,  5,12,  7,30,  4,82, 
    14,76,  7,40, 11,21,  6,91, 10,48,  9, 6,  2,91,  0,75,  1,79, 12,51,  8,51,  3,81,  4,70, 13,65,  5,19, 
     8,49, 13, 5,  7,59,  9,40, 11,74, 12,70,  5,84,  4,47,  2,25,  3,86, 14,75,  6,26,  0,51, 10,32,  1,15, 
     8,11, 14,18,  5, 6, 11,60,  1,83,  0,64,  7,85,  3,21, 12,52,  2,49,  9,30, 10,56, 13,31,  6,25,  4,31, 
     8,83,  6,42,  1,11, 11,64,  7,44, 12,90, 14, 8, 10,35,  0,72,  9,67,  3,72,  4,55,  5,43, 13,88,  2,35, 
    13,19,  1,53,  4,80,  0,89,  5,21,  7,34,  3,56,  6,89, 12,50,  8,28, 11,15,  2,27, 10,74, 14,83,  9,79
  };
  // Taillard 20x15 instance 3
  const int tai_20_15_3[] = {
    20, 15, // Number of jobs and machines
     8,56, 13,25,  3,17,  7,63, 10, 9,  6,30, 14,75, 12,22,  9,42,  2,83,  0,69,  4,90,  5,88, 11,20,  1,30, 
    11,39,  4,20,  6,35,  3,79,  8,35,  9,66, 12,15,  5,56, 13,60,  0,72,  7,52, 10,14, 14, 2,  2,16,  1,59, 
    14, 5,  9,31,  4,55,  0,70,  2,49, 13,70,  6,92,  7,40, 11,13,  1,14,  3,49,  5,30, 10,50, 12,77,  8,81, 
     0,64, 12,63, 10,21,  1,21, 11,29, 14,10,  8,25,  5,60,  9,93, 13,24,  2,48,  6,52,  3, 8,  4,30,  7,37, 
     2, 4,  6,32,  4,10, 13,77,  8,45, 12,37,  3,89, 10,60,  9,59, 11,42,  0,48,  7,30,  1,22,  5,23, 14,15, 
     1,14,  4,10, 13,68,  7,95, 12,42,  8,29,  3,44,  0,23, 14,61,  5,57, 10,45, 11,98,  9,30,  6,27,  2,13, 
     6,50, 12,55,  3,23, 11,25,  0,51,  1,55,  7, 9, 13,87,  4,21,  8,48,  5,55,  9,22, 14,47, 10,50,  2,86, 
     5,11,  7,43, 14,26, 11,31,  4,18,  3,59, 10,84,  0,33,  1,73,  6,20, 13,34, 12,92,  8,65,  9,87,  2,37, 
     3, 9,  2,20,  6,11,  4,21, 11,11, 10,96,  0,94,  8,91, 14,92,  7,97, 13,28,  5,55,  1,89,  9,34, 12,61, 
     3,10, 11,58, 14,86, 10,86,  2,87,  4,18,  1,74, 13,64,  6,12,  9,22,  5,80,  8, 5, 12,95,  7, 6,  0,35, 
     9,47, 10,68,  6,60,  7,20,  4,14, 12, 6,  5,20, 14, 6, 13,46,  1,79,  8,32,  0,82,  2, 7,  3,74, 11,54, 
     7,20,  5,99, 11,55, 14,78,  8,35,  0,26, 13,23, 12,87,  2,86,  9,25, 10,98,  1, 1,  4,16,  3,33,  6,50, 
    11,35,  0,34,  6,66, 12,47,  5,48, 10,52, 13,33,  3,77,  4,38,  1,65,  9,58, 14,71,  2,14,  8,85,  7,13, 
     2,85,  9,86,  4,15,  6,68,  3,32,  7,83, 13,80,  5,81, 12,10, 10,12,  1,31, 11,38,  8,78, 14,44,  0,18, 
     6,60, 14,58,  5,16,  8,24, 13,57,  4, 8,  1,41,  3,39,  0,28,  7,56, 12,37,  2,34, 10,39, 11,69,  9,52, 
    10,76, 14,87,  8,91,  7,13,  6, 4,  5,32,  1,58, 13,62, 11,83,  2,48,  3,41,  9,36, 12,68,  0,28,  4,12, 
     5, 1, 11,67,  1,98,  4,41,  7,84,  6,34, 14,86, 13,75,  8,93, 12,83,  3,66,  2,93,  0,47,  9,58, 10,64, 
     4,61, 14,49,  2,35,  6,92,  0,84, 11,57,  7,31, 13,50,  9,53,  1,11,  5,74,  3, 8,  8,14, 10,12, 12,50, 
     7,19, 14,89, 10,67,  1,10,  5,75,  9,49,  2,75,  4,66,  0,37, 11,77, 13,94,  3,60,  6,38, 12,52,  8,61, 
    11,29, 13,73,  7,62,  8,19,  6,99,  0,95,  5, 2,  4,39, 10,70,  1,90,  3,10,  2,60,  9,21, 12,40, 14,17
  };
  // Taillard 20x15 instance 4
  const int tai_20_15_4[] = {
    20, 15, // Number of jobs and machines
     7,15, 12,89,  6,49,  3,95,  8,40,  5,79,  0,44,  9,59,  1,87, 10,88,  2,48,  4,44, 13,43, 14,11, 11,75, 
     8, 6,  3,46, 12,18,  5, 4,  6,56,  9,44,  1,15, 14,40,  4,44,  2,79, 11, 1,  0,32, 10, 5, 13,92,  7,76, 
     1,78,  0,45,  8,61,  4,49, 11,26, 14,36, 12,94,  6,80, 13,49,  9,53,  2, 4,  5,51,  7,82, 10,36,  3,76, 
    11,58, 13,34, 10,70,  0,19,  4,85,  7,69, 14,87,  5,38,  3, 5,  2,88,  9,66,  1, 3, 12,10,  8,28,  6,18, 
    14,78,  6, 7,  2,83, 10,75, 11,39,  5,24, 12,10,  9,13,  3,42,  1, 2, 13,61,  4,26,  0,11,  7,89,  8,39, 
     7,80,  5,88, 12,13, 13,92,  9,11, 11,62,  1,42,  4, 3,  3, 6,  6,36,  2,49,  8,98,  0,40, 14,59, 10,15, 
    14,83, 13,12,  8,48,  9, 1,  1,76,  0,32, 11, 1,  4,81,  2,53,  3,70, 10,78,  6,75,  7, 7, 12,82,  5,31, 
     7,75, 13,13,  4, 9, 11,11,  0,49, 10,15,  5,57,  2,84,  6,77,  1,80,  8,41, 12,82,  3,68,  9,64, 14,50, 
     5,39, 11,64,  4,88,  1, 9, 12,97, 14,99, 13,27,  7,48,  8,18,  6,49,  9,50,  2,26,  0,54,  3,80, 10,77, 
     9,66,  4,87,  3,27,  8,47, 14,68,  5,75,  6,31, 13,25, 11,49, 10,85, 12,86,  7,12,  1,26,  2,82,  0,78, 
    11,93, 13,87,  6,74,  5,26, 10,60,  3,76, 12, 3,  8,98,  2,72,  4,52,  9,73,  1,75,  0,28, 14, 1,  7,51, 
    11,79,  1,13,  4,14,  7,27, 12,14,  3, 5,  5,58, 10,32, 14,38,  2,67, 13,70,  0,86,  6,28,  9,94,  8,33, 
     1,83,  5,67,  7,18, 12,20, 13, 4,  6,84, 11,22,  4, 8,  8,91,  3,89, 14,25, 10, 8,  0,69,  9,85,  2,46, 
     4,64, 10,18,  7,12,  8,43,  3,78, 13,65,  0,20, 14,53, 11,32, 12,49,  9,25,  6,10,  1,43,  2,30,  5, 3, 
     8,99,  4,29, 11,50, 13,99,  1,53,  9,65,  0,23,  6,49, 12,91,  5, 1,  2,86, 10, 7,  3,68,  7,71, 14,89, 
    13,13,  8,19, 10,31,  5,94,  6,78, 14,43, 11,16,  7,56, 12,76,  0, 1,  1,11,  4,24,  9,13,  2,62,  3,55, 
     5,43,  8,24, 13,85, 11,20,  9, 6, 14,44, 10,49,  1,41, 12,67,  6,47,  4,25,  7,86,  3, 6,  0, 6,  2,30, 
     2,68,  9,92,  4,15,  0,80,  5,29,  7,72,  8,22, 10,41, 12,49, 11,36,  3,97, 13,80,  6,23,  1,77, 14, 4, 
     1,51,  4,34,  8,10, 10,96,  6,74, 11,80,  9,65,  5,75, 13,14,  0,83,  7,13, 12,78,  3,61, 14,43,  2,58, 
     1,69, 10,56, 13,15, 12,89,  5,22,  4,21,  6,89, 14,16, 11,59,  7,83,  8,20,  2,33,  9,11,  0,67,  3,90
  };
  // Taillard 20x15 instance 5
  const int tai_20_15_5[] = {
    20, 15, // Number of jobs and machines
     2,76, 11,17,  0,58, 13,26,  1,90,  8,77,  9,63,  6,87,  4,74,  7,35, 12,60,  3,90, 14,64,  5,68, 10,28, 
     5, 5,  8,79,  9,71,  3,42,  1,71, 13,20, 11,86,  0,88, 14,47,  7,62,  4,37, 12,87, 10,47,  6,97,  2,24, 
     5, 2,  2,67,  8,28, 14,98, 13,66,  1,42,  6,46,  9,23, 12,94,  4,25, 11,89,  0, 3,  7,38,  3,76, 10,76, 
     8,96, 10,79,  4,19,  0,36,  3,87, 11, 6, 13, 9,  6,18,  1,32,  5,37,  2,55,  9, 3, 12,15, 14,12,  7,45, 
     8,71, 11,73,  1,17,  0,41,  5,71, 13,88,  2,43, 14,59, 10,37, 12,22,  9,21,  3,77,  4,66,  6,46,  7,52, 
    11,19,  3,12, 13,87,  9,22, 14,41,  1,29,  0, 6,  7, 4,  4,79,  5,78, 10,21, 12,27,  6,16,  2,54,  8,60, 
    13,96,  3,39, 11,82, 10,15,  5,22, 12,29,  0,64,  4,92, 14,68,  6,60,  7,37,  8,10,  9,47,  2,68,  1,74, 
     5,28,  8, 3,  6,71,  7,59,  2,94,  3,60, 10,98,  4,77,  1, 9, 14,57, 11,21,  9,74, 13,19,  0,74, 12,19, 
    12, 7,  8,38,  9,63,  7,69,  3,13,  6,56, 11,53, 13,58, 10, 2,  0,93,  2,90,  5, 6,  1,66,  4,76, 14,60, 
     5,85, 14,46, 11,75,  4,34, 10,33,  8,94,  9,50,  2,20, 13, 4, 12,28,  7,60,  1,74,  0,90,  3,51,  6,67, 
    10,88,  9,11,  1,35, 12,87,  2,14,  7,85,  3,12,  4,21,  0,23,  5,37,  6,12,  8,88, 13,98, 14,33, 11,76, 
     9,30,  7,89,  5,91,  6, 3,  0,97, 14,71,  1,73,  3,16,  8,15, 11,98,  2,71, 12,19, 10,65, 13,89,  4, 2, 
     2,61, 13,86, 12,71,  3,76,  9,88,  1,32,  5,31, 10,50, 11,25,  6,84,  0,79,  7,34,  8,59, 14,75,  4,78, 
     3, 9,  0,59,  9,93, 13,69,  8,38,  2,65,  6,96,  1,67, 10,74, 12,41,  7,61, 14,68,  4,11,  5,24, 11,25, 
     6,86,  5,77,  0,21, 13,50,  8,72, 14,68,  2,91,  7,72, 12,65,  4,52, 10,45,  3, 5,  1,71,  9,68, 11,25, 
     6,37,  8,27,  0,23,  7,26,  4, 2,  1,36, 11,20, 10,65, 13,61, 14,27, 12,35,  2,50,  9,45,  3,80,  5,19, 
    13, 5,  2,57,  1,70,  8,95,  3,46, 12,36,  6,88, 14,42, 11,49,  9,23,  5,63,  7,77,  4,47, 10,88,  0, 7, 
    13,29,  7,64,  5,23, 11,42,  4,33,  6,65, 10,92, 14,80,  2,49,  9, 3,  0,83,  3,20,  8,63, 12,78,  1,85, 
     9,67,  2,47, 10,48, 11,57,  7,84,  1,63, 14,48,  3,70,  6,85,  8,93,  5, 1,  4,63, 12,87,  0,29, 13,90, 
    13,80,  4,14, 11,41, 12,73,  7,22,  0,93,  5, 6,  1,81,  8,19, 14,62,  9,62,  2,85,  3,25, 10,70,  6,10
  };
  // Taillard 20x15 instance 6
  const int tai_20_15_6[] = {
    20, 15, // Number of jobs and machines
     6,40,  8,57, 11,95,  0,33, 14,72, 13,31,  9,55,  5,36,  3,92, 12,72, 10,80,  2,39,  7, 3,  4,86,  1,29, 
     0,20,  9,56,  1,68,  6,49,  5,35, 13,58, 11,90,  8,52,  3,97,  4,95, 12,94, 14,32,  7,56,  2,71, 10,83, 
     8,98,  3, 5,  9,97,  1,85,  0,31, 10, 5,  5,16, 14,19, 12,75, 13,50,  6,23,  2,63,  7,89, 11,65,  4,24, 
     5,80,  1,58,  7,41, 10,34,  0,94,  2,63, 13, 8, 11,75, 14,60,  3,42,  6,38,  9, 3,  8,73, 12,79,  4,36, 
     2,71,  9,65,  6,26,  7,59,  0,54,  8,69, 13,86,  5,86, 10,43,  3, 7, 12,35,  4,86, 14,99,  1,94, 11,99, 
     5,82,  3,70,  9,53,  2,74,  1,58,  4,70, 10,50,  8,37, 13,90,  0,22, 11, 9, 12,98,  6,30, 14,94,  7,43, 
     8,71,  7,63, 12,65,  9,15,  6,39, 14,93,  4,97, 11,67,  5, 5,  1,61,  3,64, 13,68, 10, 2,  0,31,  2,17, 
    13, 7,  7,10,  6,65, 11,63,  2,92,  0,90, 14,85,  4,81,  3,32,  1,62, 10, 5,  8,21,  9, 5, 12,49,  5,36, 
     1, 9, 14,31,  0,77, 10,49, 11,24,  8,67,  7,66,  5,37,  9,82,  3,69,  4,63,  6, 4, 12,62,  2,52, 13,66, 
    13,67,  1,73,  3,87, 14,28,  5,43,  9,13,  2,18,  0,73, 11,69,  7,20, 12,97,  4,73, 10,64,  6, 8,  8,13, 
    10,85,  5,30,  9,80, 13,64,  6,18,  8,72, 11,66,  4,72, 14,28,  0,13,  7,17,  2,55, 12,17,  1,42,  3,58, 
    13,87,  9,36, 14,87, 10,27,  3,23,  7,72,  2,49,  5,79,  6,30, 11,17,  8,57,  1,56,  0,82, 12, 4,  4,66, 
     5, 6,  9,62,  7,78,  8,78, 10,62, 13,17,  2,43,  6,18,  1,53,  0,16, 12,66, 11,20,  3,69,  4,49, 14, 4, 
     9,34, 11,89,  6,23,  7,69,  2,12,  8,59, 14,50,  3,57,  0,85, 12,16,  4,55, 13,82,  5,61,  1, 5, 10,36, 
    10,71,  2,19,  1,96,  5, 9,  0,85,  3,88, 14, 3, 11,68,  4,52,  8,29,  9,29,  7,22, 12,10,  6, 9, 13,65, 
    12,23, 14,34,  0,73,  2,34, 10,85,  5,40, 13,73,  3,15,  7,51, 11,91,  1, 1,  9,43,  6, 7,  8,63,  4, 7, 
     8,18,  6,62, 12,97,  4,49, 13, 4, 10,71,  5,68,  7,51,  9,42,  0,40,  1,32, 14,92,  3,11,  2,46, 11,99, 
    14, 1,  5,93, 12,46,  4,12,  1,11, 13,82,  6,56,  2,39,  8,84,  7,43, 10,77, 11,22,  9,23,  3,47,  0,43, 
     7,98,  2,38,  6,92,  3,72,  8,78, 14,70,  5,47,  9,32,  4,84, 13,84,  0,63, 11,95, 12,59,  1,26, 10,14, 
    10,80,  7,53, 13,72,  4, 9,  5,89,  3,30,  6,35,  1,34,  8,52,  0,87, 12,97, 11,34,  9,73, 14,68,  2,31
  };
  // Taillard 20x15 instance 7
  const int tai_20_15_7[] = {
    20, 15, // Number of jobs and machines
     1,70,  6, 6, 10,29, 14,55,  7,14,  5,33, 13,66,  4,14,  3, 7,  8,57,  0,55,  9,18, 12,62,  2,46, 11,92, 
     0,30,  9,53,  3,19,  2, 1,  8,98,  5,81, 14,63, 12,62,  6,10, 11,15, 10,73,  7,75,  4,80,  1,84, 13,97, 
     1,42,  6,61,  2, 6, 11,60,  3,24,  4,70, 10,78,  9,11, 12,35,  8,38,  7,61,  5,90,  0,74, 14, 1, 13,60, 
    12,76, 14,84,  8,72,  9,17,  5,27,  3,86,  0,84,  1,71, 13,90, 10,27,  6,13, 11,98,  4, 3,  7,57,  2,66, 
    14,38, 10,58,  8,80,  6,24,  0,50,  4,76,  5, 6,  9,12, 11,26,  3,14,  7,35, 13,38,  1,55, 12,33,  2,42, 
     8,77,  2,87,  9,59, 12,19,  3,84,  0,85, 13,63, 14,51,  5,18, 11,29,  7, 2,  1,13,  4, 1,  6,25, 10,54, 
     5,19,  3,83,  8,71,  7,22,  0, 4, 12,68,  4,68,  2,88, 14,80,  6,55, 10,11, 11,19,  9,39,  1,68, 13,37, 
     5,38,  8,98, 12,11,  1, 3,  7,33,  4,43,  2,19,  9,90,  6,56, 14,83, 13,76,  0,97,  3, 2, 10,76, 11, 1, 
    14,25,  8,65,  7,88, 12,56,  6,75, 10,48,  0,40,  1,19, 11,39,  9,40,  5,43,  4,99,  2,23,  3,74, 13,39, 
     0,97, 14,66,  2,54,  5,29, 10,23,  4, 9,  9,74,  7,46, 11,85,  6,98,  3,74, 13,12,  1,71, 12,65,  8,25, 
     1, 3,  3,40, 11,81,  0,74,  9,67,  5,93,  2,76, 14,16,  6,12,  7,67,  8,52, 12,20, 13,24, 10,71,  4,90, 
    12,13,  8,59,  3,95, 10,79,  2,46,  0,16,  7,67,  9,67, 14,64,  5,85,  4,85,  1,27, 11,26, 13,56,  6, 1, 
     1,64,  6, 1, 14,29,  2,66, 11,32,  4,35,  8, 8,  3,26, 13,94, 10,94,  0,62,  9,42, 12,60,  7,56,  5, 7, 
     1, 3,  4, 7,  9,40,  3,93, 11,55, 14,75,  8,25,  0,21, 13,30, 10,82,  2, 1,  6,58,  5,53,  7,88, 12,19, 
     2,66,  7,88,  3,48,  1,77,  8,38,  4,78, 14,16, 11,41, 12,93,  5,38, 10,25,  0,51, 13,14,  6,98,  9,61, 
    10,33,  2,23,  4, 7,  6,60,  1,74,  9,54,  7, 2, 12,22,  3,32, 11,15, 13,79, 14,83,  5,69,  8,41,  0,19, 
     2,61,  0,26,  5,66, 14,85,  4,34,  6,15, 11,59, 10,75,  8, 3,  7,80, 13,39,  1,69,  3, 6,  9,73, 12,65, 
     3,96,  0, 6, 11,52, 14,22,  5,35, 10,79,  4,16, 13,72,  9,29,  6,26,  8,52,  1,58,  7,57,  2,31, 12,74, 
     1,42,  0,79,  7,84,  5,25,  9,70,  8,90, 14, 8, 13,60,  2,81, 10,88,  4,11,  3,71, 12,61, 11,49,  6,81, 
     1,52, 12, 3, 14,57,  4,66,  7,88,  9,42,  8,23,  3,72,  6,97,  0,91, 10,50,  2,43, 11,82, 13,62,  5,27
  };
  // Taillard 20x15 instance 8
  const int tai_20_15_8[] = {
    20, 15, // Number of jobs and machines
     8,78,  1,22, 10,89,  7,46,  9,42,  5,59,  6,13,  3,90,  2,41, 13,69,  4,71,  0,13, 14,48, 11,97, 12,62, 
     1,87,  9,56,  4,44,  5, 1,  0,74, 11, 3,  6,89,  7,77, 12,29,  2,17, 14,12,  8,60, 13,92, 10,35,  3,24, 
     6,57,  2, 6,  8,73,  9,36,  3,57,  5,25,  4,94, 12,21, 11,46,  7,89, 10,47, 13, 2,  0,57, 14,67,  1,55, 
     4,74,  9,40,  1, 1,  2,37, 13,52,  3,84,  6,50,  0,39, 12,65,  5,80, 10,44,  7,70,  8,25, 11,27, 14,12, 
     6,15,  7,72,  9,25, 13,69, 11, 8,  1,96,  5,14,  0,13, 12,31,  3,74, 14,13,  2,91,  4,39,  8,57, 10,46, 
     2,95, 12, 2,  6,68, 13,22,  8,40,  4,33,  1,36, 14,32,  5,50,  9,32, 10,10,  0,63,  3,85,  7,16, 11, 1, 
    13,15,  2,98, 12,21,  5,10, 11,35,  6,76,  8,29,  4,64, 10,34,  7,25,  1,88,  0,30,  3,52,  9,43, 14,45, 
     9,14,  8,21, 14,86,  7, 2,  3,19, 10,78,  1,92, 12,85,  4,54, 13,61, 11, 6,  6,13,  5,85,  0,87,  2,43, 
    14, 8,  3,58,  6,67,  8,16, 12,99,  9,33,  4,14,  5,47,  1,21,  7,77,  0,64, 10,29,  2,73, 11,10, 13,47, 
    11,55,  4,84, 14,55,  8,26, 13,83, 10, 6,  9,99,  0,51,  5,28,  6,63,  2,93, 12,52,  3,86,  1,68,  7,46, 
     6,43,  2,19,  8,32,  0,36,  3,18,  5,60,  7,97, 10,13, 14,48,  9,36, 11,79, 13,14,  4,69,  1,15, 12,23, 
     9,12,  1,68, 14,36,  0,72, 13,90,  2,68,  7,28,  8,13,  5,18, 11,68,  4,49, 10,52,  6,50,  3,63, 12,10, 
     8,76, 11,75, 12,73,  3,40,  7,58, 13,23,  6, 6,  1,31, 14, 5,  5,16,  9,73,  2,41,  4,47,  0,67, 10,37, 
     8,93, 12,58,  7,58, 11,93,  3,21, 14,90,  6,13, 10,82,  5, 6,  1,62,  2,52,  9,44,  0, 4,  4,29, 13,20, 
    10,98, 13,66, 12,63,  8,63,  3,71, 11, 9,  5,10, 14,94,  0,93,  4,77,  2,47,  6,40,  1,24,  9,96,  7,56, 
     8,33,  6,18, 10,95, 14,80,  1,87, 11, 3,  3,72,  5,18, 12,30,  9,32,  2,93,  4,10,  0,86,  7,58, 13,45, 
    13,69,  2,83,  8,62, 14,77,  7,41,  0,13,  9, 8,  6,87,  3, 3, 11,65, 10,40,  5,11, 12,32,  1,71,  4,86, 
     0,21, 13,77, 12,76,  6,77,  7,61,  2,82, 11,76, 14,42,  8, 6,  3,88,  4,51, 10,50,  1,29,  5,63,  9,18, 
     9,87,  6,16,  2,98, 12,27,  0,58,  8,59, 11,69,  4,95,  5,85,  1,80,  7,97,  3,88, 14,11, 10, 8, 13,42, 
     2,25,  8,16,  6,20, 12,67, 14,85, 10,74, 13,48,  5,44,  0,95,  7,28, 11,66,  9,34,  3,25,  4,94,  1,19
  };
  // Taillard 20x15 instance 9
  const int tai_20_15_9[] = {
    20, 15, // Number of jobs and machines
     7,84,  0,58, 12,71,  4,26,  1,98,  9,36,  2,12, 11,30, 10,87, 14,95,  5,45,  6,28, 13,73,  3,73,  8,45, 
     4,29,  8,22,  7,47,  3,75,  9,94, 13,15, 12, 4,  0,82, 11,14, 10,35,  1,79,  6,34,  5,57, 14,23,  2,56, 
     1,73,  4,36,  7,48, 13,26,  3,49,  8,60, 10,15,  5,66, 12,90, 14,39,  9, 8,  6,74,  2,63,  0,94, 11,91, 
     5, 1, 11,35,  9,23, 12,93,  7,75,  1,50,  6,40, 13,60,  8,41,  2, 7,  0,57, 14,72,  3,40,  4,75, 10, 7, 
     4,13, 11,15, 12,17,  1,14,  0,67,  9,94,  6,18, 13,52,  2,53, 14,16,  5,33, 10,61,  3,47,  8,65,  7,39, 
     2,54,  6,80,  3,87,  8,36, 14,54,  0,72,  4,17, 10,44, 11,37,  1,88,  7,77, 13,84, 12,17,  5,82,  9,90, 
     4, 4, 14,62,  5,33, 10,62,  8,86,  7,30,  6,39,  1,67,  0,42, 12,31,  9,83, 13,39, 11,67,  3,67,  2,31, 
     7,29, 10,29, 11,69, 14,26,  3,55,  2,46,  4,53,  5,65,  1,97, 12,24,  9,69,  6,22, 13,17,  0,39,  8,13, 
    14,12, 11,73,  0,36, 13,70,  3,12,  2,80,  1,99,  8,70,  5,51,  7,14,  4,71, 12,28,  6,35, 10,58,  9,35, 
     0,61,  5,49, 12,74,  1,90, 13,60, 10,88,  9, 3,  4,60,  2,59,  8,94, 14,91, 11,34,  7,26,  6, 4,  3,26, 
     4,89,  3,90,  8,95, 12,32,  9,18, 11,73,  2, 9, 14,19,  5,97,  7,58, 13,36,  6,62, 10,13,  1,16,  0, 1, 
     9,71,  6,47,  1,95,  0, 7, 14,63,  7,49, 13,24, 12,46,  2,72, 11,73,  5,19,  8,96, 10,41,  3,15,  4,81, 
     4,45,  3, 9,  0,97, 14,62, 13,77,  9,78,  7,70,  2,19, 11,86,  8,15, 10,23,  1,46,  6,32, 12, 6,  5,70, 
    12,74, 10,46,  3,98,  6, 1,  4,53,  5,59,  0,86,  7,98,  2,76,  8,12, 13,91, 11,98, 14,98,  9,11,  1,27, 
    14,73,  7,70,  5,14,  8,32, 11,19,  0,57,  2,17, 13,96, 12,56,  4,73,  6,32,  1, 7, 10,79,  9,10,  3,91, 
     6,39, 14,87, 12,11,  2,81,  7, 7,  5,79,  8,24, 13, 9, 11,58,  9,42,  0,67,  3,27,  4,20,  1,19, 10,67, 
     9,76,  5,89, 14,64, 10,14, 12,11,  1,14,  4,99, 13,85,  0,81, 11, 3,  3,46,  2,47,  7,40,  6,81,  8,27, 
     9,55, 12,71,  4, 5, 14,83, 11,16,  8, 4,  0,20,  7,15,  5,60,  3, 8,  1,93, 10,33,  6,63, 13,71,  2,29, 
    12,92,  2,25,  3, 8, 14,86,  5,22,  1,79,  6,23, 11,96, 13,24,  9,94,  7,97, 10,17,  8,48,  0,67,  4,47, 
     3, 5, 12,77, 10,74,  5,59, 14,13,  0,57,  9,62,  8,37, 13,54,  6,69, 11,80,  1,35,  7,88,  2,47,  4,98
  };
  // Taillard 20x20 instance 0
  const int tai_20_20_0[] = {
    20, 20, // Number of jobs and machines
     6,64,  1,57, 15,81,  2,98, 19,59, 13,87, 16,93, 18,62,  3,20, 12,14, 14,85,  5,45, 10,47,  7, 9,  8,94,  9, 9,  0,15, 17,66, 11, 1,  4,94, 
     8,39,  6,96, 10,88,  9,83, 19,77,  1,58,  0,83, 17, 3,  2,78, 11,68,  7,64, 13,97, 18,33, 15,25,  4,47, 14,44,  5, 7,  3,60, 16,42, 12,91, 
     1,96,  3,66,  4,88, 12,60,  0,22, 14,92,  7,62, 19,14, 10,89, 11,39,  5,94, 18,66, 13,10, 15,53, 16,26,  8,15,  9,65,  2,82, 17,10,  6,27, 
    18,93, 15,92, 10,96,  1,70, 19,83,  9,74,  2,31, 12,88,  4,51,  0,57, 13,78, 16, 8, 14, 7, 11,91,  3,79,  5,18,  8,51, 17,18,  6,99,  7,33, 
     2, 4, 10,82, 12,40, 15,86,  7,50, 14,54, 17,21,  5, 6,  8,54,  1,68, 13,82,  0,20,  3,39, 16,35,  4,68,  9,73, 11,23, 18,30,  6,30, 19,53, 
     0,94, 16,58, 10,93,  9,32, 17,91,  8,30,  5,56,  3,27, 19,92, 11, 9, 15,78,  2,23, 14,21,  4,60, 12,36, 13,29,  1,95,  6,99,  7,79, 18,76, 
    11,93,  0,42,  1,52, 19,42,  9,96, 10,29,  8,61, 17,88, 18,70, 16,16,  4,31,  3,65, 12,83,  7,78,  6,26, 15,50,  5,87, 14,62,  2,14, 13,30, 
    16,18,  3,75,  8,20, 10, 4, 18,91,  7,68,  1,19, 14,54, 19,85,  2,73, 12,43,  6,24,  5,37, 11,87, 17,66,  4,32, 13,52,  9, 9, 15,49,  0,61, 
    17,35, 18,99, 15,62, 11, 6, 10,62, 12, 7, 14,80,  3, 3,  1,57,  0, 7, 16,85,  9,30,  6,96, 19,91,  7,13,  8,87,  4,82,  2,83, 13,78,  5,56, 
     9,85,  5, 8,  3,66,  2,88, 11,15,  4, 5, 16,59, 14,30,  7,60, 13,41,  1,17, 18,66,  8,89, 15,78, 12,88,  6,69, 17,45, 19,82,  0, 6, 10,13, 
     5,90,  9,27, 18, 1,  0, 8,  3,91, 16,80,  6,89,  1,49,  2,32, 11,28, 15,90, 19,93,  7, 6,  8,35,  4,73, 10,47, 12,43, 17,75, 14, 8, 13,51, 
    11, 3,  5,84,  9,34,  4,28,  3,60, 15,69, 17,45,  7,67, 18,58, 19,87, 13,65,  1,62,  6,97, 12,20, 10,31,  0,33, 14,33, 16,77,  2,50,  8,80, 
    15,48,  3,90,  4,75,  7,96, 18,44, 13,28, 19,21,  9,51,  2,75, 10,17, 11,89, 14,59,  6,56, 17,63, 12,18, 16,17,  0,30,  8,16,  1, 7,  5,35, 
    13,57, 10,16, 16,42,  8,34,  4,37,  1,26,  2,68, 19,73, 14, 5, 12, 8, 18,12, 17,87,  0,83,  5,20,  9,97,  7,20,  6,85,  3,61, 11, 9, 15,36, 
     0,63,  5,11,  4,45, 19,10, 14,33,  8, 5,  1,41, 16,47, 15, 9, 10,74, 12,33,  2,35, 11,78,  6,12,  7,22, 18,44, 13, 8,  3,97, 17,10,  9,86, 
    11,33, 16,60, 17,21, 10,96, 15,69, 12,34,  0,94,  5,15,  4,23,  8,84,  3,16,  7,55, 13,50,  1, 5, 18,59,  9,35, 19,12,  6,57, 14,11,  2,51, 
     7,72,  8,42,  5, 4, 13,62, 19,15, 16,27, 11,16,  1,34, 14, 8, 18,50, 10,85,  6,12,  3,48, 15, 5,  4,25,  2,40,  0,81,  9,46, 17,67, 12,25, 
    17,83, 10,92, 15,25,  5,40,  4,21, 14, 4,  8,43, 18,38, 13,60,  6,24, 19, 3,  3,28,  1,86,  9,68, 11,55,  2,91, 12,97, 16,19,  7,73,  0,20, 
    14,28,  3,81, 16,46,  0,98, 10,46, 18,29,  4,96,  1,12,  5,71, 19,32, 12,64,  9,39, 11,16, 15,97, 13,99,  7,49, 17,75,  8, 7,  6,79,  2,80, 
    13,71, 10, 9,  3,11, 14, 8,  1, 4,  8,47,  0,93, 16,82, 19, 6,  4,49,  6, 7,  9,24, 11,92,  5,13,  2,86,  7,80, 17,34, 12,75, 15,35, 18,29
  };
  // Taillard 20x20 instance 1
  const int tai_20_20_1[] = {
    20, 20, // Number of jobs and machines
     3,94,  2,61, 19,12, 11,68, 10,40, 13,84, 16,30, 15,16,  1,34,  0,92, 12,53, 17,55, 18,61,  9,67,  5,30,  7,88, 14,12,  4,20,  8,16,  6,51, 
    14,22, 12,75,  4,29, 17,87,  7,47,  2,48, 10,21, 15,46,  8,77,  9,35,  3,10,  6,92, 16, 9, 13,75, 11,40,  1,89,  0,86,  5,33, 19, 2, 18, 1, 
    18,32, 17, 8,  0,99, 13,14,  7,41,  1,53, 11,97, 19,19,  5,39,  3,20,  2,91, 16,54, 15,97,  4,79,  8,21, 14,22,  9,93, 12,67, 10,17,  6,84, 
    17,13, 10,43,  8,97, 12,41,  5, 4, 16,35, 14, 6, 11,93,  6,32,  1,35,  9, 2,  4,54, 19,77,  0, 9, 18,97, 15,10,  3,45,  2,81, 13,76,  7,37, 
     8,26, 12,70,  1,33, 10,58, 13,38, 15,77,  5,86, 17,53,  9,47,  7,20, 18,71,  3,69,  4,95,  0, 4,  2,23, 14,89, 19,87, 11,20, 16,67,  6,65, 
     6,86,  0,73,  9,93, 16,26, 14,98, 18,37, 17,67, 10,87,  2,33, 13, 6, 11,68,  7,16, 19,12,  1, 5,  8,33,  3,87,  4,96, 15,46, 12,87,  5,89, 
    12, 3,  1,34,  6, 2, 14,96, 11,67, 15,37,  3,30, 10,50,  9,84, 19,27,  5,37, 17,89,  2,92,  4,68,  8,20, 18,80,  7,76,  0,74, 13,11, 16,38, 
     5,60, 12,97, 19,42,  1,73,  8,28, 13,69, 18,90,  9,44, 10,27,  2,54, 17,24,  3,36, 15,82,  6,13, 16,33,  7,80,  0,44,  4,99, 11,80, 14,82, 
     2,79,  8,62,  3,31, 10,27, 16,72, 15,12,  0, 4, 17, 4,  7,11, 13,35,  5,83, 14,57, 12,19,  9,80, 19,20,  6,16, 18,96, 11,24,  4,64,  1,93, 
    17,61,  2,86,  7,46, 19,58, 11, 2, 16,19, 13,46, 10,50, 15,79,  1,84,  5,14,  6,16,  8,76,  9,89,  3,85, 12,86, 18,60, 14,44,  0,28,  4,63, 
     7,10, 12,44,  8,26, 11,61, 16,92, 15,30,  4,19,  3,27, 14,22,  9,86, 19,22,  2,62, 13,75, 17,10,  6,78,  1, 3, 10,97, 18,88,  5,10,  0,46, 
     4,21, 17,51, 11, 3, 16,94, 15,82, 18,26, 14,83, 19,57,  6,86,  7,61,  5,80, 10,81, 13,25,  0, 5,  3,75,  9,38,  2,16,  8,20,  1,50, 12,52, 
     8,17, 16,86, 12, 6,  2,49,  5,74, 15,82, 10,86, 17,26,  7,80, 13,46,  4,94,  1, 7,  0,27,  3,26, 19,97,  9,14, 18,27, 14, 3, 11,12,  6,82, 
     1,46,  8,21, 18, 1, 11,99, 17,83,  9,22, 10, 2, 16,42, 19,61,  5,79, 12,17,  4,67,  3,61,  6,72,  7,49,  2,91, 14,38,  0,28, 13,34, 15,14, 
    13,50,  7,49,  4,40,  0,63, 19, 5,  2,80, 14,70,  3, 3, 11,62, 17,43, 16,58, 10,39,  1,52,  9,68, 12,71,  6,86, 15,61,  8,53,  5, 1, 18,97, 
    15,53,  0,51, 18,25, 16,16,  8,91,  4,93,  5,37, 14,61,  7,41, 17,49,  1,20,  3,24, 10,58, 19, 8,  6,72,  2,30, 13,15, 12,86,  9,31, 11,40, 
     3,72, 11,77,  2,34,  9,45, 15,83,  1,85, 10,19,  7, 5, 19,77,  5,75,  8,61, 13,89, 17,77,  6,44,  0,32, 16,86, 14,40, 12,23, 18,35,  4,57, 
    17,33,  6,16, 11,60,  5,70, 16,67,  4,37, 12,42,  9,24, 18,75,  0, 1, 10,22, 14,32,  7,21,  3, 3, 15,69,  1,77,  8,53, 19,64,  2,34, 13,15, 
     5,58,  6,55,  8,68, 12, 5,  2,20,  9,88, 16,91, 13,79, 18,55,  1,16,  7,53, 17,84,  0, 1, 15,66, 19,14,  3,83, 11, 1,  4,96, 14,54, 10,30, 
     3,80, 15,81, 10, 9,  4,49, 18,32,  1,19, 16,92,  9,65, 12,88,  7,64, 11, 4,  0,68,  2,79, 19,21, 13,84, 14,92, 17,66,  5,51,  6,83,  8,96
  };
  // Taillard 20x20 instance 2
  const int tai_20_20_2[] = {
    20, 20, // Number of jobs and machines
     7,33,  4, 8,  0,81, 10,68, 17,28, 18,91, 16,91, 15,74,  3, 7, 11, 7,  5,19,  6,50,  9,65, 19,53, 12, 9,  8,90, 14,69, 13,50,  2,58,  1,13, 
     3,69, 19,10, 17,58, 14,10, 11,91,  1, 5,  2,37,  7, 9, 16,93, 13,94,  9,46,  5,55,  8,99,  0,28, 18,95, 10,94,  6, 4,  4,51, 15,59, 12,10, 
    15,79,  0,70,  4,35,  8,82,  2,35, 14,84, 18,34, 12,87,  7,91,  5,69,  6,12, 16,31, 17,94, 11,65, 10,13, 13,16, 19,39,  9,46,  1, 4,  3,74, 
    17,50,  7,40, 12,81,  9,47,  0,96, 13,67,  1,94, 14,53,  2,22, 16,17, 10,23, 18,24,  8,66,  4,15,  6,56, 15,84, 19,79,  5,25,  3,13, 11,72, 
    19, 7,  6,81,  5,62, 15,50, 13,91,  4,77,  2,32, 18,10,  3,78, 16,78, 11,21, 14,78,  1,21, 17,10,  8,88,  9,23,  7,92, 10,34, 12,88,  0,48, 
    14,66,  0,71,  5,55, 18,25, 11,43,  2,24, 12,87,  7,59,  6,90, 13,63,  4,90, 19,22, 15, 6,  1,50,  3, 9, 10,18, 16,19, 17,52,  9,83,  8,66, 
     8,66, 14,39, 18,10,  7,80,  6,55,  3,38,  1,29, 11,41, 17,63, 10,32, 15,91, 19,27,  4,72,  2,71, 16,61, 13,35,  9,17,  0,26,  5,42, 12,64, 
    17,11, 18,33, 13,84, 15,12,  5,18,  8,57, 19,43, 16,24,  0,77,  2,85,  7,62,  3,49,  1, 5,  9,46, 11,93, 14,85,  6,92, 10,30,  4,64, 12,77, 
    11,38,  0,30,  9,31, 18,25,  1,90, 19,79, 16, 3, 12,52,  6,87, 10,30,  7,87, 14, 4, 15,57,  8,43,  4,55, 13,21, 17,30,  5, 1,  2,72,  3,75, 
     0, 9,  8,49, 18,91,  4,39, 13,40, 14,59,  1,20,  7,27, 19,67, 15,22, 16, 2, 10,47,  3,91,  6,11, 11,70,  5,97, 12,78, 17,69,  9,17,  2,40, 
    16,57,  6,32,  3,67,  8,26, 10,23,  1,55, 19,14, 17,77,  0,77, 13,82,  2,34,  4, 1,  5,64, 12,90,  9,37, 15,47,  7,27, 18,54, 11, 3, 14,94, 
     4,25,  7,33, 16,12, 10,27,  9,32,  0,49, 18,35,  1, 5, 19,73,  8, 3, 14,28,  2,54, 13,45, 11,32,  6,53,  5,99, 12,85, 15,86,  3,13, 17,99, 
    12,64, 13,77,  2,82, 18,32,  9,75,  8,32,  0,68, 10,16,  7,63,  3,81, 15,31, 17,58,  6,73, 16,12, 14,25,  5,64,  4,98, 11,72,  1,47, 19,84, 
    17,17, 18,98,  5,99,  8,39,  2,73,  4,82,  9, 1, 15,43,  7,48, 10,62,  3,44, 12,50, 13,44, 16,72, 14,89,  1,45,  0,44,  6,21, 11,79, 19,60, 
    12,87,  7,63, 10, 8,  2,20, 18,88,  4,88, 19,77, 14,88, 13,46,  3,30,  6,44,  1,42,  5,84, 16,41, 15,74,  8,52,  9,25,  0,87, 11,43, 17,77, 
     0,39,  3,93, 15,44,  1,23, 16,75, 13, 7, 14,60,  7,45, 18,71, 10,49,  5, 3, 11,68,  6,56, 19,20,  2,35, 17, 8,  8,79,  9,21, 12,48,  4,43, 
     3,75, 15,92,  1,83,  4,48,  2, 7, 11,99, 13,43, 17,94, 18, 6, 14,34, 16,48,  0,60,  6,33,  8,16, 12,34,  5,99,  9,83, 19,11,  7,80, 10,43, 
     2,97, 17,80, 10, 2,  1,37, 19,31, 12,37,  6,58,  8,11, 15,24, 14,84,  5,10,  9,30,  3,97, 18,89, 11,47, 13,37,  0,73,  4,11, 16,90,  7,54, 
    11, 1,  3,97, 19,68,  1, 8, 15, 7, 14,72,  8,38,  6,50,  4,42, 16,32,  0,54,  5,94, 13,31, 10,52, 17,76,  2,20, 18,29, 12,56,  7,36,  9,16, 
     2,29, 17,31,  9,49, 10,91,  3, 7,  5,37,  1,86,  0,75, 16,21, 14,46, 15,47, 12, 1, 11,16,  8,29, 18,47,  4,81, 13,52, 19,44,  6,95,  7,79
  };
  // Taillard 20x20 instance 3
  const int tai_20_20_3[] = {
    20, 20, // Number of jobs and machines
     1,30, 18,80,  4,34, 15,92, 16,29, 19,96,  0,25, 10,49,  5,67, 14,53, 13,20,  7,52,  8,29,  6,51,  2,35, 11,38,  3,18,  9,43, 12,46, 17,98, 
     4,73,  2,68,  3, 3, 14,98,  1,68,  0, 8, 13,15, 17,88, 18,72, 16,20,  5,89, 10,59,  9,68, 15,63, 19,41, 11,30,  8,43,  6,80,  7,64, 12,14, 
     1, 6, 16,36,  8,88, 18,71, 13,51, 14,63,  6,32,  0,16,  9,63, 15, 7, 19,18,  3,90, 12,55,  5,25,  4,72, 11,92, 10,88,  7,69,  2,89, 17,83, 
    19,58,  3,35,  4,79,  9,43,  1,86,  2,50, 18,64,  8,88,  6,57, 16,25, 13,73, 14,18, 12, 4,  5,69,  7,40, 11,28, 15,37, 10,42, 17,82,  0,83, 
    15, 2, 12,41,  1,13, 10,75,  0,31, 19,66, 11,72, 13,66,  2,96, 14,45, 17,29,  6,49, 16,96,  3,50,  7,38,  4,80,  8,90,  9,36, 18,50,  5,76, 
     3,15,  7,31,  9,89,  0,87,  5,55,  6,49, 14,23, 16,19,  4,38,  2,93, 19,75, 12,45, 13,75, 18,72,  1,65, 11, 6, 17,16, 10,24,  8,24, 15,44, 
    17, 4,  3,22, 11,99,  9,10,  4,85, 13,79, 16, 2,  8,54,  0,80, 18, 2,  1,58, 19,33, 10,92,  5,93,  6,94, 15,34, 14,36, 12,48,  7,54,  2,12, 
    17,19, 13, 2, 16, 7,  4,60, 11,36,  7,11, 12,97,  6,57,  8,71, 10,60,  2,20, 18,68,  0,53,  9,54, 15,59,  1,16,  3,60,  5,68, 14,65, 19,42, 
     0,57, 11,16,  1,92, 18,99,  2,82, 13,91,  9,12,  7,19,  8,59, 15,43, 16,20, 12,84, 14,24, 10,80,  6,60, 17,82,  3,62,  5,32,  4,29, 19,20, 
     9,76,  3,78,  6,78, 12,42,  0, 3, 13,30,  2, 7,  1,82, 17,62, 16,13, 18,84, 19,22,  4,78, 11,80, 14,58,  7,53, 15, 6,  5,85,  8,23, 10,99, 
    12,83, 10, 9, 16,72,  9,88, 18,84,  7,87,  8,78,  0,65,  6,23,  3, 7, 15,35,  1,94,  5,33,  4,10, 17, 6, 14,85, 13,88, 19,18, 11,94,  2,92, 
     6,28, 18,33,  8,93,  4,11,  7,25, 17,67,  0,44, 14,28, 11,69, 19,67,  3, 9, 10,82,  9,43, 16,53, 12,48,  5,39,  1,52, 13,75,  2,81, 15,44, 
    17,24,  4,44, 16,58, 12,73, 19,30,  1,25,  2,21, 18,14, 11, 6,  3,41, 10,19,  6,21,  5,36,  7,72,  9,96,  0,32,  8, 5, 13,46, 15,61, 14,82, 
    14,91, 11,42,  1,97,  9,65, 12,78,  7,40, 10,93, 19,64,  2, 8, 17,56, 13,10, 16,93,  8,28, 15,77,  5,87,  3,26, 18,33,  0,17,  6, 2,  4,35, 
     4,15, 10,45, 19,96, 18,11, 16,95, 11,39,  5,22, 15,73,  1,79,  7,64,  2,79, 14,88, 13,65,  3,24, 17,38,  0,17, 12, 3,  6,73,  9,59,  8,92, 
     2, 3,  3,28,  7,17, 18,71, 14,91,  6,17, 12,69,  0,69,  9,51, 13,40,  4,93, 16,82, 10,47,  5,42, 19,59, 11, 7, 17,43, 15,83,  8,45,  1,83, 
     3,67,  2, 9, 15,37,  9,62, 18,82, 11,69,  7,34, 17,39, 16,15,  8,84, 19,32, 13,72,  0,68,  6,95,  4,70, 10,80, 14,78,  1,80,  5,30, 12,44, 
     4,13, 10,96,  8,26,  1, 4, 12,89, 18,98, 17,83, 15, 8, 11,70, 19,68,  3,37, 14,20, 13,35,  7,99,  2,27,  9,12,  5,73,  6,92,  0,98, 16,75, 
    13,75, 15, 1,  5,35, 14,73,  3,35, 18, 6, 17,38, 10,34,  2,70, 19,51,  6,16, 12,78, 11,58, 16, 9,  7,97,  0,55,  4,38,  1,65,  9, 1,  8, 8, 
     3,27, 13,36, 18,50,  8,21, 11,32,  9, 6, 17,34, 12,84, 16,50,  0,39,  2, 4, 14,94, 19,49,  7,20, 10,98,  1,64, 15,41,  4,29,  6, 4,  5,90
  };
  // Taillard 20x20 instance 4
  const int tai_20_20_4[] = {
    20, 20, // Number of jobs and machines
     9,14,  6,62,  2,32, 14,81, 18,65, 13,53,  0,31,  1,98, 10,34, 19,27, 17,60, 16,43, 11,30, 15,24,  7,61,  8,40, 12, 7,  4,15,  5,50,  3,10, 
    10,12,  5,42, 17,69, 19,12,  4,84, 11,24,  9,87, 12,69,  0,45,  7,37,  8,38, 15,72,  1,54,  6,66, 13,45, 14, 4,  2,61,  3,20, 16,49, 18,17, 
     6,60,  8,45, 19,34, 10,74, 14,65,  5,75,  4,92, 17,69,  2,40, 11,26, 15,69,  0,30,  7,18,  3,88,  1,49, 13,68,  9,25, 12, 1, 16,95, 18,25, 
    11,77,  4,61, 17,42, 15,65,  8,99, 13,81,  9,84,  6,33, 16, 8, 19,21,  0,26,  7,58,  1,91, 10, 7, 18,95,  3,91,  2,91,  5,14, 12,46, 14,49, 
    11,62, 19,88, 16, 2,  0,12,  8,68,  5,99,  2,46, 10,35, 18,87, 13,53,  9,60,  3,54, 12,99, 15,59,  7,10,  4,34,  1,67, 17,31,  6,52, 14,53, 
    11,21,  9,92, 16,33,  7, 8,  6, 9, 15,51,  1,44, 19, 1, 10,69, 18,83,  8,17, 17,86,  2,51, 14,95,  4,40, 12,32, 13,84,  3,54,  0, 3,  5,31, 
     3,46, 12,87,  2,45,  7,62, 16,10, 14,19, 15, 3,  9,69, 19,51, 18,56,  1,20,  5,51,  0,41, 10,12, 11, 6,  8,45, 13,17,  6, 2, 17,93,  4,39, 
    18,10, 12,82,  0,44,  9,22,  3, 9,  7,55, 16,29,  8, 3,  4, 3, 17,77, 11,78, 10,43,  5, 9,  2,84, 14, 1, 15,11,  6,59, 19,97,  1,23, 13,83, 
     0, 3, 17,89,  8,34, 12, 4,  6,94, 15,10, 13,90,  1,16,  9,18,  5,55, 11,69,  2,39, 18,99,  3,77,  7,65,  4,55, 16,27, 10,84, 19,94, 14, 2, 
    11,85, 18,98,  5, 6,  6,74,  0,24, 10,54,  2,85,  1, 7, 15,60, 14,49,  3,92, 12,59,  4,26, 19,97, 16,87,  8,28, 13,81,  7,46, 17, 4,  9,82, 
    13,49,  7,99,  6,92, 17,55, 15,38, 10,23,  2,97, 19,42, 12,94, 11,95,  1,93, 16,31,  8,91, 14, 3,  3,30,  0,28,  5,56,  4,21,  9,51, 18,22, 
     6,51, 19,65, 12,71, 13,81,  5,56, 10,45,  9,41, 18,26, 14,52, 16,88,  7,97, 11, 3,  0,32,  2,16,  3, 1, 15,13,  4, 8, 17,50,  1,66,  8, 5, 
     6,93,  9,78, 10,90, 14,25, 12,83,  5,40, 11,83,  7,67,  2,59, 17,90, 19,91,  4,50, 15,22,  0, 9,  1,12, 13,28, 16,28,  3,40, 18,43,  8,29, 
    11,65, 19,30,  5,14,  7,33, 10,50,  9,91, 15,19,  4,50,  2,86, 14,83,  6,13, 12,49,  0,30, 13,43, 16,46, 17,67,  1, 6, 18,77,  3,87,  8,64, 
     9,92, 19,96,  5,76, 10,42,  4,39, 11,17,  0,46,  6,61,  2,17,  3,29,  7,69, 13,58,  1,69, 18,98,  8,60, 15,97, 16,76, 14,41, 17,55, 12,32, 
     7,37,  4,38,  6,77,  2, 4,  3,72, 18,31,  0,32,  5,98,  1,44,  8,65, 13,16, 17,84, 11,60, 14,88, 19,20, 16,60, 12,92, 15,91, 10,72,  9,58, 
    15,15, 12,37,  5,51,  2, 9,  8,15,  7,14, 14,73,  9,93,  4,79,  3,63, 13,21,  6,68, 16, 9, 17,51, 19,25,  1,57, 11,41, 10,51, 18,80,  0,20, 
    19,50, 13,50,  9,19,  5,81,  6, 1,  4, 6,  8,15,  3,30, 15,19, 10,36,  2,64, 12,76,  0,40,  1,32, 18,77, 16,62, 14,52, 11, 7,  7,97, 17,40, 
     7,29, 17,35,  5, 7, 12,59,  4, 1, 14,65, 13,92,  2,39, 18,56,  9,93,  1,29, 11,54,  0,41,  6,54, 16, 7, 10,85, 15,74,  8,79, 19,72,  3,79, 
     8,31, 14, 9, 11,76, 18,54, 15,44, 16,39, 19,48, 13,17, 17, 4,  1,13,  9,87,  5,24,  6,68,  0,84,  3,82,  4, 1, 10, 4,  2,60,  7,56, 12,58
  };
  // Taillard 20x20 instance 5
  const int tai_20_20_5[] = {
    20, 20, // Number of jobs and machines
    10,47,  2,61,  9, 7,  3,13,  7,52,  4,33, 18,83, 12,60,  6,57, 14, 7, 15,74,  0,93,  5,59, 17,46,  8, 7, 13,84, 19,56, 11,58, 16,45,  1, 4, 
     0,97, 14,15,  2,18, 11,73,  8,37,  6,94,  7,20, 15,69, 12,13, 18,26,  1,48, 16,71, 13,96, 19, 5, 10,42,  5,15,  4,64,  3,36, 17, 6,  9,74, 
     8,87,  7,89,  1,28, 12,81,  4,47, 18,53, 14,67,  2,78, 15,14, 10,92, 11,94, 16,26,  0,68, 17,36,  9,79,  5,71, 13,94, 19,28,  6,25,  3, 2, 
    10,99, 15,65,  0, 9,  6,52, 18,10, 16,55,  9,20,  2,67, 19,69,  5,16, 11,10, 13,54,  8,47,  7, 4,  4,66, 17,33, 12, 9,  1,53,  3,30, 14,29, 
    10,33, 18,64, 16,17,  4,81,  5,42,  9,60,  8,14,  3,95, 12,36, 14,95, 17,37,  1,85,  0,48, 11,74,  2,76, 19,68, 15,77, 13,14,  7,91,  6,69, 
     3,86, 12,16,  9,34, 11,83, 19,79, 14,89,  2,22,  1,74, 10,58, 13,71,  5,22,  8,36,  7,53, 17,80, 16,53, 18, 1,  0,57, 15,68,  4,26,  6,26, 
     2, 4,  7,83,  6,26, 19,54,  5,16,  9,88, 11,16, 16,61, 10,41,  8,54,  1,98, 18, 3,  4,84,  0,11, 13,55,  3,18, 15,67, 14,62, 12,17, 17,31, 
    12,16,  7,99, 18,46, 19,40,  5,54, 10,27,  8,71, 14,95, 11, 9, 17,46,  0,57,  9,86, 15, 7,  6,16,  1,70,  4,15,  3,71,  2,41, 16,83, 13,14, 
     8,30, 12,24,  7,95,  4,41,  5,53, 10,84,  3,55,  9,54,  0,42,  2,75, 13,55, 11,57,  1,62, 19,23, 14,28, 16, 3, 17,83, 15,88, 18,11,  6,68, 
    17,78, 14,63,  5,21, 10,64, 13,91, 11,75,  9,53,  4,35,  2,77,  7,29,  0,68,  3,92,  6,89,  8,49, 18,47,  1,33, 15, 4, 16,58, 19,18, 12,33, 
     2,25, 17,86,  9,55, 14,68,  1,56,  7,43, 15,23,  3,15,  4,88,  6,28,  5,41,  0,87, 18,75, 11,77, 10,49, 12,52, 16,80,  8,25, 19,94, 13,55, 
    19,40, 10,29,  9,27,  4,70,  1,76,  7,19, 18,67,  2, 9, 15,10, 11, 8,  3,83,  8,49, 13,70, 16,62,  6,70, 12,38, 17,68,  5,46, 14,77,  0, 9, 
     0,72, 16,82,  1,78, 12,12, 17,98, 13,98,  8,46, 19,79,  4,88,  3,11,  7,36,  9,67, 18,97, 15,22, 10,53,  6,21,  2,22, 11,17,  5,43, 14,60, 
     7,60, 12,77, 13,32,  8,51, 17,31,  9,65, 16,18, 10, 3,  6,31,  3,12, 14,35,  4,54, 11,44, 19,10,  5,43,  1,77, 18,40, 15,98,  0,69,  2,33, 
     5,72, 10,42,  1,20,  4, 2,  2,50,  6,67, 14,81,  7,95, 11,39, 18,45,  9,82, 13,50, 15,89,  3,77,  0,63, 19,44, 17,42, 16,40, 12,86,  8,84, 
     4,62, 13, 6, 18,46,  3,40, 19,75, 10,89, 14,11,  2,13, 12,89,  1,71,  7,69, 11,86,  5,60,  8,92, 15,56,  6,88,  9,80,  0,18, 16,75, 17,66, 
     8,51, 15,16, 16,60, 19,38,  5,43,  6,94, 12, 3,  2,53, 10,80, 14,96,  0,70,  3,66,  9,83, 13,82,  1,83,  4,70, 11,22, 17,94, 18,46,  7,57, 
     5, 6, 19,28, 11,71, 12, 9,  6,27,  3,88,  4,90,  8,72,  1,43, 17,16, 18,36,  2,44, 13,41, 14,37, 15,80, 16,84, 10,86,  7,91,  0,24,  9, 3, 
     7,43,  1,27, 10,46, 18,67, 14,89, 16,10,  5,63, 13,33,  4,14, 19,95,  2,61,  6,66,  0,68, 12,46, 11,27, 15, 5,  3,17, 17,64,  8,10,  9,74, 
     0,83,  9,35, 16,39,  4,97,  2,99,  7,77, 15,98, 13,88,  1,51,  6,31, 19,88,  3,24, 10,34,  8,44, 11,29,  5,37, 18,23, 12,15, 17,50, 14,56
  };
  // Taillard 20x20 instance 6
  const int tai_20_20_6[] = {
    20, 20, // Number of jobs and machines
     7,86, 13,43, 17,61,  6,99,  8, 7, 11,70,  4,21,  3, 2, 12,88, 18,38,  9,65,  2,81,  0,38,  1,51, 10,81, 16,37, 15,72, 14,94,  5,44, 19,99, 
     7,80, 17,66, 16,90, 10,83,  0,89, 15, 7, 12,55,  1,17,  2,13,  9,45, 18,28,  5,73,  3,44,  6,65, 19,50, 14,84,  4,70, 11,71,  8,32, 13,91, 
    11,90, 17,43, 19,37,  0,71,  2,64,  4,88,  8,27, 14,30, 10,34,  3,99,  5,10, 12,44, 18,99, 15,94,  1,96, 16,98,  6,44,  7, 7,  9,33, 13,59, 
     2,34,  7,52, 17, 5, 12, 4,  1,84, 16,54, 13, 3,  6,97, 19,39,  3, 9, 18, 9, 11,91,  5,60,  0, 4,  4,63, 14, 3,  8,35, 10,79, 15,66,  9,97, 
     3,52,  5,51, 12,72,  0,24, 10,96, 17,54, 16,51,  2,61,  7,92,  8,81,  9,74, 18,49, 19,24,  4,59, 13, 4, 11,21,  1,78,  6, 2, 15, 3, 14,49, 
     9,42, 13,47,  6,10,  0,27,  7,38, 18,92, 12,88, 17,16, 19, 3, 10,56,  3,80,  4,10, 16,26,  1,78, 15,69,  8,91,  2,82, 11,77, 14,73,  5,96, 
     7,19,  3,38,  5,83, 17,50, 14,31, 19,87,  8,67,  9,99, 13,69, 11,77,  1, 4, 12,31, 16,96,  0,77,  2,80, 18,68, 15,74,  4,86, 10,30,  6,54, 
    15,25, 16,47, 12,10,  5,16,  4,83,  9,62, 18, 3,  3,38,  8,87, 19,19, 17,98,  6, 2, 13,58, 11,30,  0,22,  7,55, 14,80, 10,69,  1,77,  2,40, 
     2,17,  5,98,  6,25, 19,41,  4,62, 18,28,  1,52,  0, 5,  8,25,  9,37,  7,93, 12,63, 13,23, 14,58, 17,92, 15,70, 16,90,  3,29, 10,26, 11,69, 
    19,41,  0,65, 17,34, 14, 4,  7,73, 11,79,  8,58,  6,14, 12,97,  4,71, 10,97,  2,95, 16,58, 13,12,  3,17,  9,66,  1,78, 15,68, 18,69,  5,53, 
    10,27,  4,83, 14,20,  9,12, 19,86,  6,34, 18,36,  0,28, 13,63, 12,37, 16,23,  5,50,  7,90,  8, 5, 17,17,  3,80,  1,35,  2, 4, 15,41, 11,81, 
     4,85, 18,92, 12,90, 15,95,  3,19,  8,59, 19,94,  9,75,  5,75,  6,47, 16, 9, 14, 6, 10,43,  0,30,  2,88,  1,19, 17,10, 13,76, 11,58,  7,29, 
     4,23,  5,87,  6,50,  7,76, 11,26, 10,28,  0,36, 17,35,  2, 4,  9,32, 14,22, 13,74,  1,52, 16,13,  8,14, 19,61, 15,47, 12,87,  3,73, 18,64, 
     5,80,  0,43, 17,45, 14,92,  8,68, 12,66,  2,60,  9,37, 18,60,  6,51, 19,41,  3,61, 13,98,  7,59,  4,95, 15,38, 11,67,  1,12, 10,95, 16,22, 
    10,57,  1,96, 15,11,  5,25,  8,69,  4,59,  9,45, 13,52, 14,85,  2,26, 17,91,  6,57,  3,30, 16,32, 19,58, 11,40,  7,11, 18,19,  0,19, 12,82, 
     1,81,  5,83,  2,77, 16,45, 11,63, 17,95, 14,25,  8,48,  9,27, 18,56,  4,54, 12,82, 15,32, 19,99,  7,41,  0, 1, 13, 2,  6,61, 10,23,  3,26, 
     1,47, 18, 9,  2,90, 14,28,  5,68,  8,23, 10,66,  6,46, 19,75,  4,96, 13,68,  9,60, 16,46,  7,35, 11, 9, 12,89, 15,96,  0,42,  3, 2, 17,86, 
    11,90,  3,52, 14,10,  7,25,  1,59, 13,55, 18,30,  9,33, 19,18, 12,80,  8,73, 15,41,  4, 9,  6,64, 16,79, 17,31,  2,79, 10,44,  0,14,  5,73, 
    10,59,  2,57, 17,67,  7,47,  8,13, 18,35, 13,72,  6,74,  5,56, 16,85,  0,52, 15,25,  1,92,  3,92, 12,81, 11,76,  9,91, 14,93, 19,36,  4,88, 
     8,28, 16,89,  0, 3,  2,75, 11,31, 10,87, 19,66, 17,67,  7,34, 14,19,  3,30,  4,91, 13,53, 15,81,  6,13,  5,15,  1,59,  9,17, 12,85, 18,11
  };
  // Taillard 20x20 instance 7
  const int tai_20_20_7[] = {
    20, 20, // Number of jobs and machines
    17,24,  7, 8,  9,42, 12,87,  4,95,  5,14, 14,30,  8,89,  0,69,  2,32, 18,59,  3,20, 10,26, 11,35, 16,83, 13,25, 19,48, 15,51,  1,58,  6,99, 
    18, 5, 10,34,  6,32,  3,29,  4,44, 16,79,  1,73, 14,13, 15,25, 13, 8, 11,37, 19, 6, 17, 1,  8,31,  0,97,  9, 7,  2,47,  5,91, 12,74,  7,38, 
    13,67, 16,68,  8,44,  7,31,  4,90,  3,29, 15,21, 19,37, 18,82, 17,27,  5,33,  1, 1, 10,73, 14,35, 12,83, 11,79,  6,79,  9,92,  2,44,  0,78, 
    13,13, 18,85,  4,76,  6,84,  5,77, 14,20,  9,63, 19, 1,  1, 5,  3, 5, 10,50,  7,11, 12, 8,  2,14, 15,34, 11,20,  8,58, 17,32, 16,56,  0,74, 
    17,71, 10,12,  8,79, 12,78,  3,26, 14,38, 16,72, 15,83,  2,51,  0, 9,  7,45,  5,13, 13,31, 19,91, 11,40, 18, 5,  6,91,  4,24,  9,96,  1,72, 
     5,83,  0,98, 19,83, 17,22,  8, 8,  7,28, 12,93,  6, 5,  1,82,  3,65, 13,77, 16,56, 15,66,  2,61, 14,82, 10, 9,  9,82, 11,35, 18,83,  4,51, 
    17,38, 14,78,  6,91,  8,21, 18,88,  2,93, 16,15, 12,10,  5,68,  0,75,  4,53, 15,35,  7,11, 10,68,  3,98, 19,56,  1,37, 11,57,  9,15, 13,83, 
    11, 3,  1,98, 15,19, 16,67,  2,33, 13,78,  9,59, 14, 2,  0,32,  6,78, 18,97, 10,77,  7,72,  5,34,  4,45, 17,26,  3,79, 19,28, 12,88,  8,19, 
    17,46, 10,28,  7,99,  2,95, 16,17, 13,84, 11,68,  0,26,  8,32,  4,85, 19,98,  3,59,  1,67, 12,44,  5,80, 18,70,  9,95, 14,70,  6, 9, 15,49, 
    18,27, 15,17, 14, 6, 12,62,  9,90, 19,17, 10,58, 16,94,  7,11, 17,65,  6,96, 11,76,  0,58,  2,60,  4,51,  8,51, 13,98,  5,26,  3,92,  1,66, 
     1,92,  0, 9,  9,71,  6,66, 19,57, 17,56, 11, 8, 18,80, 15,11, 16,78, 14,50,  8,37,  7,92, 13, 5,  3,13, 10,63, 12,21,  4, 6,  5, 2,  2,27, 
    15,70,  2,55, 10,13, 19,50,  9,23,  3,75, 17,24, 16,69, 13,72, 11,53,  4,94,  6,25,  1,21,  7,57, 14,16, 12,17,  5,70, 18,34,  0,42,  8, 6, 
     6,94,  1,80, 19,74, 14,71, 17, 8, 11,51, 10,87,  3,86, 13,37,  2,93, 12,82, 15, 1,  4,76, 16,49,  8,35, 18,44,  5,50,  0,75,  9,63,  7, 4, 
     2,19, 10,40,  1,30, 19,92,  5,10, 12,60,  8,32, 14,71, 17,73, 13,61, 11,31,  4,94, 16,61, 18,85, 15,91,  0,98,  6,35,  7,55,  9,84,  3,93, 
    14,68,  0,13,  8,30,  3,83,  9,46, 10, 8,  2,41, 17,83,  4,33,  6,19, 19,75,  1,37, 16,17,  5,29, 12, 5, 13,62, 15,96,  7, 7, 18,73, 11,39, 
     8,49, 12,19, 13,10, 10,67,  7, 6,  0,42,  4,87, 16,83,  5, 7, 15,51, 18,55,  3,79,  9,24,  2, 2, 11,88, 19,80, 14,37,  6,58,  1,20, 17,45, 
     8,89, 12,33, 19,27, 11,20, 10, 2,  4,26,  2,88,  7,24,  9,62,  6,68,  0,59, 15,53, 13, 7,  1,85, 16,66, 14,14,  5,22,  3,15, 17, 8, 18,58, 
     9,88,  6,88, 16,91,  4,72,  3, 9,  2,41, 14,76,  8,24, 12,77,  7,60, 17,93, 15,39, 19,93, 18,71,  1,13,  0,73,  5,44, 10,15, 13,19, 11,95, 
     9,93, 18,34, 12,36, 10,82, 14,28, 16,52,  1,22, 17,33,  4,77,  7,27,  0,62,  8,59,  2,52, 15, 1, 19,39,  6,85,  5,62,  3,34, 11,77, 13,74, 
     6,15,  2,38, 17,83,  9,32, 19,12, 11,41, 14,81,  7,79,  4,90,  3,12,  8,18, 12,37, 18, 1, 15,91, 16,73,  0, 5,  5,82,  1,64, 10,37, 13,91
  };
  // Taillard 20x20 instance 8
  const int tai_20_20_8[] = {
    20, 20, // Number of jobs and machines
    10,84,  9,45, 16,53, 18,48, 17, 9,  5, 9, 14,39,  2,79, 13,83, 19,50, 12,24,  3,49,  8,81,  4, 5,  1,70, 15,89,  0,91,  6,25, 11,80,  7,36, 
     4,75, 19,48,  5, 6,  6,32,  2,68,  7,23, 16,44, 12,21,  9,42, 11,13,  8,74, 13,31, 14,62,  3,91,  1,65, 15,66, 18,26, 17,96,  0,97, 10,51, 
    10,49,  3,26, 14,35,  0,64, 16,55,  1,78,  2,72,  5,83,  7,59, 15,16, 17,92, 13,68, 19,64,  4, 4, 11,76, 12,82,  9,70, 18,75,  8,65,  6,31, 
     1,55,  8,58, 17,37, 11,32,  0,31, 10,65, 12,65,  5,85, 13,50,  4,94,  6,37,  3,20, 15,94,  7,20, 16,31, 14,30,  9,49,  2, 8, 19,22, 18,47, 
    13,81, 16,68, 18, 2, 10,69, 15,38,  4, 7, 14,81,  5,79, 19,76,  2,94, 17,65,  0,11,  6,98,  7,38,  3,95,  1,93,  9, 9, 12,21, 11,17,  8,79, 
     5,64,  7, 7, 15,94, 19,29, 17,77,  4,75, 12,50,  2,78,  3,57, 18,29,  8,66, 13,93,  0,74,  9,73, 14,80, 10, 8, 11,26,  6,87,  1,69, 16,85, 
    10,49,  3,59, 12,91,  8,59,  9,25, 16,44, 19,50,  6,41, 13,33, 11,89, 17,79,  0, 3, 14,54, 18,82,  1,63,  4,31, 15,15,  5, 2,  7,67,  2,71, 
     4, 4, 14,20, 18,23,  3,33, 16,65,  7,44,  9,57, 10,20, 17,93, 11,23, 15,18,  0, 8,  5,72,  6,54, 19,18, 13,93, 12,43,  1,18,  8,56,  2,21, 
     8,58,  4,25,  7,34,  5,89,  9,54, 19,89,  3,12,  0,51, 13,74, 14,78, 11, 4, 17,72, 16,81, 10,92,  1,69,  2,35, 12,25,  6,35, 18,10, 15,33, 
    11,33, 19,84, 12,75,  4,66, 18,49,  6,77,  8,87, 13,44, 15,37,  2,67,  0,33,  1,75,  3,65, 17,44, 16,66, 10,45,  5,93,  9,98,  7,22, 14,67, 
     0,17, 17,26,  1,54, 18,25, 13,92, 12,34,  6,47,  7,80,  2,24,  5,92, 15,75, 10,68, 11,84,  9,72, 19,84, 16,94,  4,69, 14,96,  3,34,  8,29, 
     0,66, 10,79,  5,74,  8,67,  2,72,  1,22, 15,50,  7,30, 18,47,  3,75, 12,43, 13,44,  9,71, 17,61, 19,54, 11,99, 16,11,  6,97, 14,75,  4,81, 
    16,35, 13,75,  4,99, 19,72, 18,92,  6,90, 10,26,  5,91,  9,70, 17,82,  1,13, 12,45, 14,82,  8,58, 15,38, 11,19,  3,66,  0,23,  7,49,  2,19, 
    17,82, 13,74,  4,40, 12,33,  7, 9,  0,33, 11,26,  3,44,  2,18,  8,73, 16,41, 18,96,  1,39,  6,91, 14,89, 15,11, 10, 1, 19, 2,  5,69,  9,10, 
    12,25, 10,32, 16,41,  5,14,  9,67,  0,25,  6,94, 18,89,  1,21,  2,98, 15,92, 11,72, 14,57,  3, 4,  8, 1,  7, 2, 13,84, 19,91, 17,42,  4,85, 
     7,29,  1,98,  4,41, 13,87, 16,52, 12, 9,  9,22,  3, 2, 14,79, 18,73,  0,16, 10,22,  5,97, 17,13,  8,19,  2,13, 19,50,  6,43, 15,91, 11,34, 
    14,91,  8,41, 18,47, 17,61, 15,66, 19,31,  3,92, 12,42, 16,19,  0,98,  2,36,  1,29, 13, 8,  5,25, 10, 5, 11,90,  9,62,  7,63,  6,17,  4,23, 
     8,69,  9,78, 15,61,  7,52,  5,40,  1,71,  0,40, 18,61, 14,93,  2,37,  6,32, 12,48, 17, 7,  4,37, 10,69, 19, 4, 11,79, 13,81, 16,10,  3,75, 
    16,90,  4,16, 15,68, 14,32,  8,96,  7, 7, 18,42, 19,52, 12,38,  5,68,  6,72,  9,78,  1,10,  2,61, 13,40,  3,31,  0,81, 11,69, 17,84, 10,27, 
     4,91,  8,17,  2,75, 14, 7, 15,44,  6,10, 13,32, 17,78, 12, 9,  9,69,  7,45,  5,87,  3,90,  1,50,  0,42, 11, 2, 19,21, 10,62, 18,93, 16,88
  };
  // Taillard 20x20 instance 9
  const int tai_20_20_9[] = {
    20, 20, // Number of jobs and machines
    18,84, 15, 9,  6,34,  8,62, 14,11, 12,60, 19,43, 11,52,  7,77,  4,37,  9,15, 16,43, 10, 8, 13, 5,  3,36,  5,56,  2,46,  0,51,  1,86, 17,86, 
    18,61, 13,56,  8,60,  3,78,  1,73, 19,12, 14, 8, 16,16,  0,12,  2,63,  9,31, 10,62, 11,97,  4,53,  5, 1, 12, 3,  6,99,  7,65, 15,63, 17,32, 
    10,86,  7,53,  4,59, 17,12, 14,34,  0,27,  8, 2,  1,86, 16,85,  5,21,  3,58, 13,70,  2,55, 15,77,  6,15, 18,20, 19,32, 12,42, 11,17,  9,38, 
     2, 3, 19,13,  5,67, 16,13, 11,63, 15,88, 13,68,  4,21,  0,21,  8,86, 17, 7,  6,91, 18, 8, 10,56,  9,92,  3,58,  1,94,  7,54, 12,57, 14,87, 
    12,29, 15,74,  2,89,  3,18, 11,38,  7,75, 10,18, 17,15,  9,95, 16,11,  0,24, 13, 4,  4,12, 14,17,  1,34, 19,35,  8,62,  6,90,  5,48, 18,21, 
     6,11, 12,14, 13,90,  8,74, 14,67,  0,91,  5,70, 11, 8,  7, 7,  2,49, 19,13,  3,78,  1,75, 18,80,  4,31, 15,22, 10,99, 16,66,  9,80, 17,66, 
    11,97, 15,63, 17,11,  7,71, 18, 1,  9,63, 12,70,  4,33, 16,74,  1,76,  2,86, 10,87,  8, 9,  5,18,  0,51,  3,27, 14,48,  6,31, 13,45, 19,76, 
    18,19, 17,64,  0,94,  7, 4,  4,81, 12, 5,  1,72, 10,30,  2, 2, 15,16, 13,38, 14,93, 19,15,  5,17,  6,61,  3,71,  9,18,  8,22, 11,17, 16,20, 
    15,61, 11,66, 12,62,  7,70,  1,59,  0,80, 14,82,  9, 2, 18,97,  8,76, 13,72,  5,90, 16,74,  3,95, 19,41,  4, 9, 10,46,  6,20, 17,78,  2,32, 
     7,61, 18,90,  4,37, 19,86, 12,15,  9,19, 13,62,  5,82,  2,86, 15,59, 10,92, 14,89,  0,82, 16,48,  1,13,  6,29,  8,28, 17,45, 11,84,  3,62, 
    17, 7, 13,86, 18,79, 14,67, 19,85,  1,68,  6,94, 11,61, 12,47,  4,49, 15,50,  0,55, 16, 3,  3,18, 10,79,  8,32,  2,43,  9,97,  7,53,  5,44, 
    10,20, 14,50, 12,72,  9,90,  2,25,  0,24, 19,43,  5, 4,  4,26, 13,62,  7,42, 11,77,  3, 9, 15,61, 18,19, 16,69,  1, 9,  6,60, 17, 5,  8,54, 
     7,45, 19,73,  1,50, 10,58, 11,94, 14,90,  8,97,  9,42,  4,36,  5,72, 12,84, 15,33, 17,44, 13,59,  3,47,  6,40,  2,81, 18,85,  0,26, 16,28, 
    17,67, 11,46, 13, 9,  1,40,  2,81,  4,97,  7, 7,  0, 2, 10,69,  8, 9,  3,17, 19,81, 14,81,  6,46,  9,26,  5,30, 18,88, 16,73, 12,44, 15,99, 
    11,16,  3,62,  5, 3, 19,30, 10,16,  1,40, 17,62,  7,96,  6,75, 13,69,  2,86, 18,90,  0,93, 15,15, 12,30,  8,46, 14,50, 16,29,  9, 9,  4,97, 
    12, 5,  8,73, 16,54,  0,81, 18,26, 13,36,  7,35, 19,56,  4,62,  9,31, 17, 2,  5,23, 11,60, 14,12, 10,88,  3,38,  6,95, 15,65,  1,86,  2,64, 
     9, 3,  5,99, 19,81,  7,93, 11,82, 12,17, 16, 1, 14, 1, 13,32,  0,36,  8,30,  4,62, 10,90, 18,20,  1,98,  6, 3,  3,66, 17,75, 15,79,  2,67, 
    19,52,  4,76,  6,79,  1,63, 14,52,  2,23,  0,35, 18,22,  7,58, 10,13,  3,26, 12,68, 15,84,  9,16, 13,28,  5,28,  8,54, 17,76, 16,86, 11,47, 
    19,74,  3,34, 15,68,  4,37,  7,26, 12,48, 13,29, 16,24, 18,60, 14,98, 17,54,  5,97,  8,19,  0,99,  2,62, 11,46,  6,25, 10,53,  9,11,  1, 4, 
    11,15, 17,92, 13,41,  4,63,  3,87, 15,67, 18,77, 12,89,  0,65, 10,17,  5,24, 19,67,  9,10,  2,87,  7,91, 14,58,  6,52,  8,26,  1,33, 16, 3
  };
  // Taillard 30x15 instance 0
  const int tai_30_15_0[] = {
    30, 15, // Number of jobs and machines
     3,99, 10,43, 14, 6,  1,99,  5,23,  8,98,  4,84, 11,24, 13,30, 12,53,  2,34,  9,95,  7,50,  0,48,  6,38, 
     6,19,  4,24,  2,65,  3,16, 14,94,  5, 9,  7,60, 13,32,  9,59,  0,85, 11, 9, 12,36,  8,22, 10,25,  1, 5, 
     4,54,  2,62, 10,93,  5,78, 12,59, 13,71,  3,49, 11,88,  9,40,  7,13,  8,17,  6,88, 14,47,  1,30,  0,56, 
     0,60, 13,16,  4,79,  1,84, 14,84,  9,42, 12,59,  3,14,  8,74,  5,60,  2,98,  6,17, 10,42,  7,31, 11,19, 
     6,49,  1,52, 10,46,  4,50,  8, 1,  5,14,  2, 2,  9,56,  7,64,  0,51, 13,75, 11,28,  3, 9, 14,37, 12, 6, 
     5,59,  6,65, 12,85,  3,40,  0,23,  4,39,  2,99,  8,46,  1,17, 13,94, 11, 6,  9,67, 14,69,  7,86, 10, 8, 
    11,10, 13, 7,  1,22,  6,36,  4,31, 12,75, 14,57,  0,49, 10,44,  8,21,  5,77,  7,70,  2,64,  9,46,  3,69, 
     5,53,  2,74,  1,93,  4,26,  9,54, 13,89,  8,82, 10,66, 11,37,  3,63,  7,71,  0,17,  6,58, 12, 4, 14,46, 
     7,76, 13,72, 12,42,  5,17, 10,27,  9,56,  0,78, 11, 5,  8,72,  4,19,  2,90,  1,46, 14,43,  6,56,  3,17, 
     6,18,  7,79, 14,93,  1,71,  9,48, 11,23,  3,20,  5,90, 10,94,  2,87,  8, 6,  0,36, 13,84, 12,25,  4,83, 
    10,52,  0,61,  3,45,  4,60,  5,15, 11,74,  7,49, 12,26, 13,94, 14,54,  8, 1,  1,58,  6,56,  2,54,  9,72, 
     5,63,  0,73,  9,82, 11,84, 12,15,  6,54, 10,52, 14,52,  8,36,  4,21,  7,45,  3,41, 13,21,  1,97,  2,50, 
     8,90,  0,90,  7,77,  4,33,  3,31,  1,26, 13,14,  5,75,  6,92, 12,70, 11,55,  9,56,  2,39, 14,49, 10,23, 
     4,87,  9,47, 13,58, 12,34,  6,29,  2,83,  7,24,  1,48, 11,97, 10,89,  8,84, 14,82,  0,53,  5,99,  3,10, 
     7,35, 14,32, 11,30,  2,93, 10,58, 12,28,  1,88,  3,16, 13,98,  9, 4,  4,82,  8,98,  5,26,  0,29,  6,77, 
    13,18, 12,92,  4,62, 11,59,  1, 3,  0,94, 10,34,  6,56,  5,24,  9,18,  2,66,  7,53,  3,30, 14,41,  8,10, 
     0, 2,  7,26, 12,17, 14,18,  3,60,  2,39,  8,23, 11,95, 13,81,  9,56,  4,34,  1, 8,  5,47,  6,72, 10,56, 
     5, 6,  6,79,  7,65,  4,58, 12,94,  9,45, 11,80,  3, 3, 10,29,  8,80,  1,27,  0,60,  2,94, 14,14, 13,76, 
    12,31,  4,79,  3,87, 13,79, 11,57,  6,48,  5,33,  0,42, 10,93,  1,86,  2,54,  9,32,  7, 8,  8,16, 14,63, 
     0,96,  7, 1,  3,75, 11,42, 10,45,  1,51,  8,10, 12,58,  5,71,  6,92,  2,23, 14,18,  9,63, 13,27,  4,63, 
    14,84,  5,82,  7,16,  1,61, 10,43,  6,75,  9,28,  3,15, 12,19,  0,93, 11,22, 13, 1,  4,62,  2, 9,  8, 5, 
    12,46, 11,29,  6,50,  8,12, 13,72, 10,18,  1,79,  7,73, 14,23,  9, 1,  4,58,  3, 1,  2,95,  0,25,  5,71, 
     8,10,  9,39, 11,49,  3,56,  4,71, 13,40, 10,90,  2,28,  0,89, 12,42,  5, 9,  6,92,  7,52,  1, 6, 14,20, 
    12,70,  8,63,  6,68,  9,97, 11,86,  5,81,  2,38,  7, 7, 14,53,  0,48,  4,43,  1,59,  3,88, 13,29, 10,87, 
    14,81, 11,97,  8,65,  4,60, 10,15,  5,29,  3, 9,  2,80,  6,78,  9,85, 12,95, 13,85,  0,91,  1,28,  7,92, 
     7,39, 14, 6,  4,59,  0,34, 12,34, 10,32,  8,12,  5, 7,  3,35,  1, 4,  6,53,  9,69,  2,89, 11, 3, 13,40, 
    11,98, 12,85,  4,51, 14, 9, 13,24,  1, 7,  5,59,  8,98,  0,50,  7,98, 10,64,  9,31,  2,31,  6,29,  3, 1, 
    14,59,  8,68,  7, 3,  1, 8,  0, 2,  6, 9,  9,69, 12,14,  5,72, 10,84,  4,69, 13,54, 11,45,  2,59,  3, 7, 
     8,92,  7,21, 10,53, 14,64,  5,59, 12,79,  9,52,  2,14, 11,61,  1,86,  3,82,  0,98, 13,83,  6,24,  4,87, 
     2,51, 12,70, 13,94,  3,80,  0,35, 14,56,  6, 8,  5,94, 10,11, 11, 3,  8,60,  9,73,  1,26,  7,21,  4,45
  };
  // Taillard 30x15 instance 1
  const int tai_30_15_1[] = {
    30, 15, // Number of jobs and machines
    13,79,  0,31, 10,42,  2,88, 12,16,  8,99,  5,82,  6,53,  4,29, 14,49,  1, 9, 11,15,  9,92,  3,73,  7,98, 
     7,76,  3,89,  0,48, 12,15, 13,54, 14,37,  8,53,  1,63, 10,44,  4,91, 11,13,  2,73,  6,42,  9,99,  5,41, 
    12,49, 14,52, 10,25,  1,89, 13, 3,  5, 2,  3,40, 11,44,  6,94,  2, 7,  4,68,  7,73,  8,73,  0,30,  9,14, 
    14,28, 13,49,  1,13, 12,87,  7,62,  0,10,  4,29,  5,62,  3,34,  2, 7,  6,47,  8,40, 11,57, 10,80,  9,86, 
     7,39,  5,12,  1,34,  3,91,  8,48, 12,71,  2,45, 13,98,  0,23,  4,91,  6,90, 10,41, 11,90, 14,54,  9,87, 
    11,30,  5,63,  4,57, 14,36, 10,72,  3,54,  0,69,  9, 9,  7,53, 12,72,  6,68,  2,33, 13,61,  8,12,  1,89, 
     5,65,  8,40, 12,34,  9,37,  3,64,  2,62, 14,14,  4,78,  1, 1, 13,65,  7, 2,  0,67, 11,56,  6,75, 10,26, 
     7,22, 14,98,  5,67,  9,56,  3,41,  1,89,  4,25, 10,94, 12,76,  2,37,  6, 8, 13,84, 11,73,  0,65,  8,74, 
    10,44, 14,33,  6,41,  5,52, 13,86, 12,11,  3,60,  8,87,  0,13,  7,40,  2,62,  9,47,  4,39, 11,65,  1,77, 
     6,88,  5,31,  4,63,  3,49, 12,50, 14,77,  7, 6,  9,80,  1,20, 13,30, 11,11,  8,41,  2,43,  0,74, 10,73, 
     5, 7,  1,69,  0,69, 11,53, 13,52,  6,33, 12,19,  7,84,  2,12,  9,36,  3,85,  4,74, 10, 2,  8,97, 14,52, 
    12,33, 13, 8,  0,74,  3,75,  2,51,  5,64, 10,55, 11, 7,  4,81,  1,82,  7,70, 14,33,  9,84,  6,37,  8,48, 
    11,54,  0,97,  9,79, 12,71,  7,70,  2,84, 13,28,  8,14,  5,20,  4,99,  3, 6,  1,30, 14,51,  6,68, 10,41, 
     5,10,  8,90, 13,14, 10,72, 14,30,  7,77,  9,69,  0,56, 12,78,  2,55,  6,98,  4,91, 11,27,  3,36,  1,86, 
     9,92,  3,97, 10,71,  5,13,  2,93,  6,65,  7,44, 11,46,  4,71,  1,69, 12,26, 13,18, 14,31,  8,10,  0,47, 
     1,47,  7, 5,  2,14,  5,47,  6,81,  9,84, 10,62,  8,91, 14, 5, 11,58,  4,77, 12,55,  0,49,  3, 5, 13, 5, 
    13,46, 11,96,  5,61,  2,67,  0, 2,  6, 9,  8,94,  9,38, 10,66,  4,25,  7,67,  1,57, 12,79, 14,74,  3,47, 
     9,74,  8,52,  1,50, 10,43,  3,93,  2,30,  6,85, 11,75,  0,58,  7,47,  5,70, 12,42, 13,62, 14,58,  4,81, 
     5, 5, 12,42,  3,63,  0,42, 13,28, 10,40,  7,36,  9,49,  6,65, 14, 6,  1,14,  4,20,  2,85,  8,41, 11,70, 
     5, 7, 10,36, 11,54,  7,91,  4,98,  6,31,  9,33,  0,72,  8,21, 12,61,  2, 1,  3,30,  1,85, 13,79, 14,32, 
    10,79, 14,82,  3,49,  1,51, 11,43, 13,16,  9,44, 12,62,  6,20,  8,12,  0, 7,  5, 1,  2,64,  7,21,  4,37, 
    14,94, 12,75,  8,56, 13,25,  6,89,  9,72,  5,84,  2,71,  7,74, 10,83,  3, 6,  1,69,  4,87, 11,19,  0,68, 
    11, 7,  8,29,  7,15,  6, 3,  0,62,  9,53, 13,92, 10, 1, 12,27,  5,21,  2,66, 14,92,  1,19,  4,22,  3,48, 
     1,75,  9,12, 12,46,  7,37,  5,72,  8,35, 13, 6,  2,32,  3,50, 14,33,  6,14,  0,34, 10,93, 11,83,  4,11, 
     2,87,  5,56, 13,70,  0,81,  9,80,  3,58,  8,75,  1,48, 12,55, 14,92,  6, 9,  4,16, 10,41, 11,71,  7,63, 
     1,29,  8,66,  7,18, 14,55, 10,53,  3,81,  5,47,  6,86,  4,33, 12,30, 13,75, 11,73,  0,27,  2,51,  9,67, 
     2,60, 14,17,  3,18, 13,61, 10,82,  5,72, 12, 5,  7,92,  1,75,  8,91,  0,89, 11,35,  4,53,  6,68,  9,85, 
    14,82,  4,54,  3,96,  2,19,  9,20,  0,67,  8,27,  6,77,  7,59, 11,87,  1,40, 13, 7, 12,46,  5,32, 10,84, 
     9,69, 12,52,  5,26,  4,65,  2,89,  3,51,  0,79, 13,51,  8,27,  7,91,  6,23, 10,59,  1,99, 11,51, 14,70, 
    11,62,  6,57,  7,30,  8, 5,  5,30, 14,13, 13,39,  3,31, 10,16,  1,68, 12,32,  0,83,  2, 4,  9,27,  4,27
  };
  // Taillard 30x15 instance 2
  const int tai_30_15_2[] = {
    30, 15, // Number of jobs and machines
    10,36, 11,32,  9,40,  8,47,  1,87,  7,55,  6,77,  2,27,  5,89, 13,72,  4,93, 14,15,  0,98, 12,95,  3,32, 
    12, 7,  8,42, 14,84,  3,76,  1,44,  7,66, 13,47,  6,72, 10,24,  5,68,  4, 5,  2,35,  9,13, 11,55,  0,73, 
    12,59,  0, 7,  3,85, 13,57, 14,98, 10,71,  2,61,  6,98,  9, 3,  1,61, 11,12,  4,44,  5, 7,  7,28,  8, 3, 
    14,16,  3, 3,  7,97,  2,27,  6,97,  5,93, 10,19,  8, 9, 11,70,  1,19,  0,92,  9,27, 13,76, 12, 1,  4, 4, 
     6,71,  5,53,  7,36,  4,63,  9,16,  3,35, 13,27, 10,36,  1,79,  0,92, 14,23, 11,60,  8,49, 12,52,  2,19, 
     7,48, 11,88, 12,68,  9, 6,  8,63,  2,25,  3,28,  5,67,  4,62,  1,53, 10,51,  0,65,  6,97, 13,15, 14,79, 
    11,49, 12,65,  1,26,  7,72, 14,60,  6,15,  3,70,  5,26,  8,71,  9,48, 13,79,  4,54, 10,98,  0,81,  2,43, 
     3,35,  9,28, 11,88, 14,58,  2,87, 13,18, 12, 5, 10,74,  6,43,  7,28,  0,82,  1,34,  4,28,  5,98,  8,73, 
     9,83,  2,93,  3,86, 13,62, 10,18, 14,70, 12,67,  1,66,  7,14,  0,62,  6,47,  8,51, 11,90,  4, 2,  5,98, 
     0,28,  7,94,  3, 6, 14,66,  4,79,  8,71, 13,35,  2,57,  1,69,  9,75, 12,84,  5,47,  6,21, 10,66, 11,66, 
     2,20, 12,62,  0,37,  4,71, 14,19, 10,63,  8,90, 13,79,  7,87,  3,40,  6,92,  9,15,  5, 5,  1,76, 11,45, 
     5,71,  2,59,  8,99,  6,70,  4,27, 12,54,  9,82,  1,62, 10, 7,  0, 5, 14,12,  7,90, 13,92, 11,83,  3,71, 
    14,90,  5,79,  0,48,  3,66,  7,86,  1,87,  4, 3,  8,49,  2,84, 12,98, 11,46, 13,58,  9,74, 10,11,  6,21, 
     5,56,  2,49, 14,93,  6,11, 12, 5,  4,32, 10,19,  9,96,  3, 7,  8,80, 13,96,  7,17,  1,22,  0,45, 11,84, 
     4,93,  9,18,  0,25,  6,69,  1,65, 10,40, 13,85,  3,19,  2,88, 12,78,  5,35, 11,53,  7,46,  8,73, 14,16, 
     2,36,  5,18, 12,36,  4,34,  1,64,  7,80,  6,87, 14,40,  3,39,  8,63, 13,42, 10,74,  9,34,  0,87, 11,49, 
     6,84, 10,79,  1,63,  8,15, 12,73,  2, 1, 11,58,  9,27, 14,67,  5,81,  0,18,  7,35, 13,52,  3,34,  4,95, 
     2,64,  0,48,  4,82, 10, 1,  3,11,  5,19,  7,27, 12,93, 14,42, 13,83, 11,12,  9,37,  8,55,  1,66,  6,42, 
     5,77,  6,13,  8,55,  0,15, 10,72, 11,20, 13,71, 14,45,  7,39,  2,61, 12,73,  1,93,  4,34,  9,62,  3,66, 
    13,68,  8,15, 11,97,  7,85,  9,81, 10,53,  3,49, 14,70,  4,96, 12,65,  1,72,  6,76,  2,71,  5,81,  0,77, 
     1,62,  6,84, 14,58,  3,36, 12,63, 10,69,  7,10, 13,51,  5,34,  0,27,  4,19, 11,98,  9,21,  2,16,  8,23, 
     7,60, 11,17,  0,89, 10,87,  1,52,  2,80,  5,17,  4,30,  3,82,  9,50, 14,53,  8,78,  6,69, 12,77, 13,67, 
     6,56,  7,40, 12,32, 13,37, 11,37,  3,12,  8,11,  1,36, 10,85,  9,89,  0,85,  5,32,  2,66, 14,98,  4,79, 
     4,32,  6,56,  1,22, 10,95, 12,55,  9,20, 14,46,  7, 8, 11,68,  0,49,  5,86,  3,92, 13,25,  8,24,  2,13, 
     7,53,  2, 1, 10,92,  9,65,  5,10,  3,92,  1,92, 13,48, 11,39,  8,53,  4,49, 14,26, 12,75,  6,84,  0, 2, 
     3,14,  4,67, 12,84,  0,31,  7,61, 11,63, 10,24,  2,51, 13,22,  5,33,  8,54,  6, 8, 14,38,  9, 7,  1,67, 
    11,68, 12,10,  0,55, 13,30,  6,26,  2,17,  1, 4,  9,98,  5,55,  7,45,  3,27, 10,76,  4,96, 14,65,  8,60, 
     0, 9,  9,24,  8,22, 12,40,  3,47, 14,73, 10,72,  7,70,  2,66,  1,19,  6, 3, 11,97,  4,98,  5,85, 13,51, 
     4,54,  2,19,  7,72, 11,38,  6,18,  3,84,  1,71, 10,80,  8,46,  9,25, 13,29,  0,57, 12,92, 14,41,  5,75, 
    14,16,  6,79,  2,53,  8,98, 10, 8,  4,20, 11, 2, 13,64, 12,61,  7,78,  3,91,  5,35,  1,55,  0,92,  9,78
  };
  // Taillard 30x15 instance 3
  const int tai_30_15_3[] = {
    30, 15, // Number of jobs and machines
     7, 7,  0,11, 12,34,  5,56, 11,14, 10,33,  8,95,  2,64, 13,12,  6,22,  4,87,  3,32,  1,54, 14, 5,  9,55, 
     7,57, 11,11,  6,33, 10,56,  3, 9,  1,71,  4,99,  2,31, 14,52,  8,33, 12,96, 13,46,  9, 1,  5,48,  0,55, 
     1,90,  2,57,  8,80,  5, 8,  4,36, 10, 7, 12,41, 14,31, 13,31,  6,48,  7,68,  9,19, 11,25,  0,38,  3,88, 
     7,87,  5,24,  2, 1, 10,49,  1,63,  3,27, 13,98,  6,22,  8,35, 12,18,  4, 7, 14,55, 11,55,  9,87,  0,29, 
    13,33, 12,36,  0,75,  9,17,  3, 8,  5,55, 10,53, 14,31,  4,95,  8,31,  1,67,  6,80, 11,87,  7, 5,  2,58, 
     8,75,  6,25,  4,76,  3,72,  5,78, 11,22,  0,81, 12,37,  9,27,  1,85, 13,71,  2,16, 14,86,  7,78, 10,14, 
     3,90,  9,54,  5,98, 13,10,  8,75, 14, 4, 12,24, 10,10,  2, 7,  0,15,  1,43,  6,90,  7,81,  4,49, 11,88, 
     6,96,  8,81, 14,92,  3,31, 10, 9, 11,65, 12,35,  4,98,  7,85,  1,37,  9,43, 13,96,  2,91,  5, 1,  0,36, 
    12,40,  8,45,  3,94,  0,21, 14,87,  6,68, 11,35, 10,63, 13,37,  1,53,  7,98,  5,94,  9, 6,  2,25,  4,72, 
    14,35, 12,55, 10,26, 11,98,  3,23,  5,65,  8,88,  2,71,  9,35, 13,59,  0,84,  4,31,  7,76,  1,13,  6,89, 
    11,77, 13,34,  7,60,  9,76, 12,58, 14,63,  2, 2,  5,44,  8,91,  6,42, 10,53,  3,45,  4,45,  1,59,  0,99, 
     9, 6, 10,56, 11,47,  4,95, 13,36,  8,63,  1,85,  2,47, 14,60,  0,35,  5,82, 12,90,  6,30,  3,76,  7,94, 
     9,58,  3, 2,  6,69,  4,19, 10,64,  8,27, 12,17,  5,33,  1,48, 14,81, 11,86, 13,28,  0,94,  2,71,  7, 3, 
    13,93,  7,40,  5,95, 14,36,  0,38,  6,47, 12,24,  9,97, 11,11,  3,55,  2, 7,  1,68,  4, 3, 10,44,  8,47, 
    10,79,  2,33,  9,65,  5,57,  3,55, 14,78, 13,31,  1,60,  4,79,  0,25,  7,76, 12,96, 11, 5,  8, 5,  6,38, 
     1,75,  4,29,  3,77,  9,50, 14,31,  0,50,  5, 5,  7,25,  2,70, 11,38,  8,91, 10,71, 12,84,  6,80, 13,76, 
    13,64,  3,85,  5,96,  9,11,  7,73,  1,41,  4,50, 10,27,  6,40, 11,54,  0,63, 14,74, 12,84,  2,76,  8,58, 
    14,66,  0,75,  1,54,  9, 4,  7,16, 11, 6, 10,89, 13,29,  6, 3, 12,10,  2,93,  5,53,  4, 8,  3,59,  8,22, 
     7,17,  5,76, 11,84, 14,45,  8,70, 13, 5,  3,55,  6, 7,  9,26,  4,59,  1, 2,  2,18, 10,66,  0,58, 12,99, 
     5,57,  1,84,  8,50,  2,54,  6,92,  9,34, 12,58,  7,51,  4,34, 10,60, 13,42,  3,66, 14,18,  0,11, 11,59, 
     8,85,  5,31,  1,29, 11,18, 10,46,  6,29,  7,49,  4,37, 12,42, 13,18,  0,77,  9,67,  3,61, 14,46,  2,91, 
     9, 2,  2,66,  5,75,  4,83,  0,63,  8,62, 14,71, 10,20,  3,42,  1,59,  6, 4, 11,67, 13,95,  7,76, 12,80, 
    14,46,  1,83,  0, 7, 12,37, 10,60, 13,76,  6, 6,  3,84,  8,82, 11,94,  5,36,  4,79,  2,46,  9,90,  7,94, 
    10, 8, 12,60,  1,99,  7,70,  5,22,  8,91, 11,68,  9,87, 14,11,  3,51,  2,66,  6,19,  4,28,  0,47, 13,66, 
    13,91,  2, 2,  1,39,  5,12,  4,11,  8,17, 14,86,  0,68, 11,88, 10,86,  7,78, 12,75,  6,86,  3, 5,  9,79, 
     1,18, 13,90, 10,91,  0,21, 12,45, 11,31,  4,66,  3,49,  7,95,  9,11,  8,57,  5,31, 14,36,  6,57,  2,88, 
     5,56,  7,18, 10,45, 14, 9, 12, 4, 11, 2,  2,96,  0,60,  9,45, 13,57,  4, 5,  6,49,  1,90,  3,31,  8,97, 
    11,95, 10,96,  9,41,  7,75,  8,61,  1,65,  3,19,  6,38,  2,78, 12,85,  0,29, 14,65, 13,77,  4,67,  5,84, 
     2,64,  8,62,  0,52, 10,21,  1,82, 13,27,  5,93, 12,65,  3,32, 11,47,  4,66,  7,39, 14,45,  6,78,  9,26, 
     1,22,  8,52,  5,36,  3,31,  6,41, 13,92, 11,98,  0,68,  4,57,  9,32, 10,82, 12,39,  2,83,  7,48, 14,85
  };
  // Taillard 30x15 instance 4
  const int tai_30_15_4[] = {
    30, 15, // Number of jobs and machines
     3, 4, 11,27, 14,90,  5,76,  8,76,  6,16, 13,38,  0,72,  1,80,  2,94,  9,97, 10, 5,  7,44, 12, 9,  4,23, 
     0,47,  3,48,  4,69,  8,84,  9,25, 10,34,  5, 8,  1,32, 12,62,  7,90, 13, 2, 14,92,  2,55,  6,25, 11,37, 
    12,87, 10,54,  5,76,  8,80,  7,17, 14, 1, 11,26,  2,36,  1,12,  6,56, 13,71,  4, 6,  9,42,  3,89,  0,96, 
     5,25,  0,11,  9,69,  1,38,  8,98, 14,50, 10,98,  7,50, 11,19, 13,76,  3, 6,  4,95, 12,19,  2,37,  6,34, 
    12,10,  6,32,  3,94, 11,22,  2,55,  8,58, 14, 6,  5,78,  0,36,  9,56, 13,16,  4,22,  7,59,  1,41, 10,63, 
     6,12,  7,59, 14,95, 13,93,  0,93,  2, 7,  8,95,  4,10, 11,23,  9,48,  3,84, 12,64, 10,14,  5,90,  1,69, 
     8,43, 12, 1,  9,27,  7,30, 11,25,  5, 3,  6,94, 14,77,  4, 6, 13,42,  0,17,  1,76, 10,29,  2,63,  3,59, 
    12,14,  7,59, 10,27,  1,59,  5,56,  4, 6,  9,48, 13,43,  2,27,  0,27,  3,43,  6,32, 11,11,  8, 5, 14,25, 
     8,14,  7,47, 13,21, 12,32, 10,29,  6,40,  1,63,  3,25,  4,49,  2, 4,  0,67, 14,27,  5, 9,  9,75, 11,15, 
     8,70,  7,97,  6,52,  3,22, 14,87,  1,87,  0,36,  2,86, 12, 2, 11,93, 10, 1,  5,16,  4,70,  9,99, 13,43, 
     2, 5,  8,28, 13,77, 11,23,  1,63,  5,69, 12,35,  3,22,  6,90, 10,46,  7,67,  4,63,  0,63, 14,24,  9,79, 
     1,69,  6,25,  8,65,  4,29,  7,51,  5,88, 13,70,  0,25, 14,58, 11,20, 10,24,  3,38, 12,34,  2,71,  9,66, 
    14, 4, 13, 4,  9,34,  8,21,  7,60, 12,55,  1,70,  4,68, 10,80,  6,56,  2,29,  3,97,  0,84,  5,66, 11,50, 
     9,88,  2,81,  6,50,  1,38,  0,52,  3, 7, 12,33, 14,46,  8,59,  7,38,  4,14,  5,66, 11,72, 13,80, 10,97, 
    11,35, 10,88,  8,98,  3,78,  5,86,  7,13,  2,94, 14,25, 13,50,  6,76,  9,89,  0,41,  4,53,  1,10, 12,99, 
     0,42, 13,16,  8,44,  6,40,  3,35, 10,71,  7,52, 11,35,  2,98, 14,73,  5,92,  9,44,  1,35,  4,79, 12,17, 
     9,46,  5,55,  1,74, 12,80, 11,89,  2,61, 14,34,  7,75,  4,39,  8,47,  3,70,  6,84, 13,26, 10,44,  0,82, 
    13,77, 12,40,  4,43,  2,76,  8,69, 11,42, 14,25, 10,34,  1, 8,  3,77,  6,57,  5,56,  7,80,  0,12,  9,89, 
    12,96, 14,53, 10, 3,  5,49,  2,76,  7,37, 11,50, 13,73,  8,98,  6,44,  3,89,  9, 2,  1, 1,  0,99,  4,89, 
     7, 7, 12,91,  0,32, 10,44, 13, 2,  8,66,  6,62,  5,22,  9,23,  1,92, 14,70,  3,31, 11,10,  4,94,  2,89, 
     6,46, 12,15,  7,23, 11,70, 13,57, 14,67,  3,58,  9,92, 10,66,  4,55,  1,13,  8,33,  2,64,  5,36,  0,21, 
     9,39,  2,35,  3,90,  1,67, 10,70, 11,94,  5,48,  8,76,  6,93, 14,46,  0,34, 13,58,  7,74, 12,49,  4,80, 
     0,99, 14,10,  1,90, 12,60,  5, 5,  8,17, 11,24, 10,83, 13,37,  6,59,  9,17,  2,99,  4,42,  3,72,  7,36, 
     8,94, 10,69, 13,47,  5,96,  7,30,  9,29,  6,22,  0,26, 11,99, 12,13,  1,59, 14,66,  4,89,  3, 1,  2,24, 
     3,91, 10,21,  9,42,  0,79,  5, 8,  2, 9, 12,66,  4, 1, 14,59,  8,36,  6,54, 11,52,  7,87,  1,82, 13,33, 
    10,31,  7,93, 12,68,  3,72,  0,22,  8,85,  6,40, 14,76,  5,48, 11,83,  9,89,  4,83,  2,43,  1,69, 13,67, 
     0,64, 13,59,  2,63, 12,54,  5,21, 11,79,  6,35,  8,95,  4, 7,  9,67,  3,15,  7,89, 10,54,  1,98, 14,26, 
    14,14,  3,93,  0,87,  7,15,  8,40,  9,20, 12,61, 10, 8,  4, 8, 11,57, 13,14,  5,90,  1,16,  2,36,  6,59, 
    13, 2,  1,87,  5, 8,  6, 2,  2,12, 14,35,  8, 6,  7,73,  9,82, 10,37,  3,19, 11,81,  4,19, 12,12,  0,60, 
    10, 4,  7, 9,  1, 7, 14,59,  8,29,  5,39,  2,55, 13,18,  0,70,  4,14,  9,47, 12,75, 11,78,  3,99,  6, 9
  };
  // Taillard 30x15 instance 5
  const int tai_30_15_5[] = {
    30, 15, // Number of jobs and machines
     2,96, 13,86,  9,75,  1, 3,  4,97,  0,88, 14,88, 12,66,  6,16, 11,63,  5,73,  7, 3,  3,63,  8,91, 10,33, 
     3,21, 11,33, 14,38,  1,94,  5,79,  0,36,  8,50, 10,83,  2, 4,  4,83,  6, 7,  9,26, 12,87, 13,15,  7,90, 
     2,88,  8,37,  6,94,  5,65,  4,24,  7,86,  0,96, 14,94, 11,81,  1, 2, 10,93, 12, 5,  3,23, 13,45,  9,11, 
     6,62,  0,61,  8,37,  7, 1,  4,10,  9,21,  2,88, 14,24, 12,61, 10,42, 11,54,  1,92,  3, 4,  5,38, 13, 9, 
    12,52,  3,87,  5,37,  0,27, 10,76, 11,69, 13,76,  6,74,  7,86,  4,46,  8,84,  2,48,  1,16, 14,91,  9,28, 
    11,35,  1,53,  3,46,  0,99,  5,17,  8,78,  7,84, 10,88,  4,60,  2,53,  6,27,  9,33, 12,88, 14,75, 13,13, 
    10,79,  1,72, 11,25,  5,52,  9,24,  4,98, 13, 4,  3,99,  7,17,  0,52,  8,85,  6,48, 14,85, 12,99,  2,72, 
    12,77,  1,80,  4,44,  5,73, 13,46,  9,60,  2,25, 14,67,  7,18,  3,17, 11, 4,  8,73,  6,32, 10,67,  0, 6, 
    10,51, 14,26, 11,29,  9,57,  4,54,  0,16,  3,41,  2, 2,  6,15,  8,88,  5,47, 13,10,  7, 2,  1,75, 12,16, 
    14,27,  2, 1,  3,88,  1,44,  5,17,  9,20, 10,50,  4,40, 11,40,  0,65,  8,10,  7,50,  6,11, 13,36, 12,81, 
     7, 6,  6,36, 12,67,  1,73,  2,30, 11,97,  3,62, 14,11,  4,80, 13,25,  5,60,  0,42,  8,42,  9,40, 10,55, 
    14,59,  3,86, 13,89,  0, 8,  7, 4, 12,16,  6,67,  1,43, 11,74, 10,97,  2, 3,  8,12,  9,55,  4, 3,  5,29, 
     3,22, 11,88, 13,39,  8,91,  2,25,  1,23, 10,38, 14,14,  4,72,  9,87, 12,78,  5,42,  6, 3,  0,31,  7,83, 
     8,74,  0,69, 13,59,  1,74,  6,61, 14,83, 10,82,  4,43,  5,42, 11,43,  2,20,  9,51,  7, 7, 12, 5,  3, 7, 
     5,75, 12,71,  4,45,  6,92, 14, 9,  8,48,  3,20,  7,28, 13,25,  9,14,  1,61, 10,39,  2,65, 11,28,  0,49, 
     2,94,  5,12, 12,33, 11,35,  1,57,  4,33,  7,22, 14,47,  0,87,  9,47,  8,61,  6,42,  3,84, 13,12, 10,58, 
    10,71,  3,35,  1,70,  4,67,  7,86,  5,42,  2,72,  0,52, 14,73, 12,44,  8,96,  6,42, 13,96,  9, 3, 11,94, 
     2,70,  6,30, 13,48,  4,57,  3,66,  8,95,  7,95,  5,17, 14,64, 10,70,  9, 6, 12,99,  1,63, 11,33,  0,27, 
     2,80,  0,93,  6,15, 11,86,  5,33, 12,65,  7,44, 14,22, 13,86,  4,93,  3,92,  1,88, 10,65,  8,39,  9,14, 
     6,91,  5,42,  2,14,  3,17,  0,50,  7,16, 10, 2,  1,36, 14,47,  4,11,  8,34, 13,29,  9,71, 11,78, 12,55, 
     5,76, 13,67,  2,35, 14,93, 11,13,  0,58,  6,24,  4,10,  1, 6,  9,49,  7,40, 10,61, 12,72,  3,97,  8,17, 
    12,89,  6,86,  0,45, 14,59,  9,16, 10,52,  5,39,  4,83,  7,11,  3,56,  8,30, 13,60, 11,80,  1,43,  2, 4, 
     1,49,  5,44, 10, 3,  4,73, 12,49,  3,63,  0,20,  7,68,  8,40, 14,37,  9,17, 13,66, 11,92,  2,82,  6, 5, 
    13,69,  3,57,  8,34,  2,67, 14,73,  6,60, 10,93,  7, 1,  9,43,  1,67,  4,85,  0,80,  5,80, 11,81, 12,98, 
     8,55,  1,27,  3,50, 12,96,  0,42,  2,76,  4,33,  6,82, 14,82,  9,87, 13,93, 11,42,  5,20,  7,97, 10,15, 
     2, 8,  0,79,  7,24,  5,19, 11,73,  8,82, 13,47,  1,90, 14,97,  9,93,  4,69, 10,47,  6,68, 12,44,  3,54, 
     7,52,  6,11,  9,98, 11,44, 10,14, 13,38, 12,57,  0,50,  5,40,  1,89,  4, 2,  3,73,  8,19,  2,40, 14,96, 
     7, 5,  0,52, 14,45, 10,17,  3,94,  2,44, 11, 9,  5,18, 12,37,  1,84,  6,28,  4,78,  8,28, 13,68,  9,64, 
    10,23, 14,63,  5,57,  1,57,  6,22,  4,71, 13,69,  8,15,  2,19, 11,88,  0,25,  9,83, 12,62,  3,54,  7,68, 
    13,47,  1,96,  2,11,  0,99,  6,28, 12, 8, 10,50,  7,18,  9,97,  5,10,  3,54, 11,50,  4,67,  8,16, 14,79
  };
  // Taillard 30x15 instance 6
  const int tai_30_15_6[] = {
    30, 15, // Number of jobs and machines
     4,96, 12,47,  9,40, 11,68,  5,49, 14,91, 10,57,  0,81,  2,87,  1, 6,  7,82,  6,47,  3,97,  8,94, 13,75, 
     0,42,  9,55, 14,82, 12,61,  1,67,  4,79, 11,39, 10,43,  6,62,  3,41,  2,78,  7,36,  8, 8,  5,21, 13,91, 
     0,52,  5,28,  3,48,  8,25,  7, 2, 14,98, 12,95, 13, 7,  9,68,  4,96, 10,72,  2,50, 11,68,  6,54,  1,37, 
     6,81,  5,59,  3,46,  4,61, 14,43,  8,71, 12,24, 10,71,  7,30,  2,87,  1,86,  9,10, 11,10,  0,93, 13,94, 
     8,30, 13,71, 10,65,  0,13,  6,30,  2,22,  7,46, 12,70, 11,88, 14,78,  1,79,  5,57,  3,71,  9,58,  4,36, 
     5,68,  9,32, 13,32,  2,15, 11,98,  0,95,  7,57, 14,66,  4,43,  6,31, 12,34, 10,60,  8,73,  1,56,  3,40, 
     9,27, 10,42,  7,24,  8,12,  1,22,  4,57,  6,93, 13,58, 14,67, 12,64,  0,30,  3,16,  2,21, 11,19,  5,33, 
     7,43,  9,51, 11,53, 13,62, 10,58,  1,38,  5,84, 14,65,  8,87,  2,26,  4,95,  6,61,  0,77, 12,49,  3,49, 
    10,93,  3,81,  2,55, 14,59, 12,70, 13,68,  8,97,  6,57, 11,48,  0,92,  1,82,  9,89,  7,87,  5,13,  4,54, 
    11,60,  3,33, 10,66,  1,96,  0,23, 14,36, 13,28,  2,53,  5,78,  4,78,  7,94,  6,72,  9,47,  8,28, 12,87, 
     5,98,  2,89,  9,27,  8,34,  3,88,  4,15, 10,64, 12, 5,  1, 4, 11,63, 13,69,  7,82, 14,29,  6,53,  0,69, 
    13,97, 14,87,  1,50,  0,68,  9,76,  2,74,  4,89, 11,15,  3, 5,  5, 2, 12,79,  6,59,  8,93, 10,19,  7,86, 
     1,60, 14,47, 12,13,  9,62,  5,95, 11,67,  3,89, 10,11, 13,29,  4,52,  0,62,  2,31,  8,51,  6,55,  7,39, 
    11,76,  4,92, 13,85, 14,20,  7,61,  6,14,  2,62,  9,52,  1, 5, 12,63,  0,29,  5,85,  3,79,  8,52, 10,51, 
     6,61, 13,45,  0,93,  5,51, 11,97,  9,46,  3,88, 12,28,  2,57,  8,45, 10,23,  4,91, 14,66,  1,73,  7,41, 
     8,49,  4,58,  7,32, 13,30,  6,59, 14,57,  9,14,  1,33, 12,14,  5,59,  2,41,  3,59,  0,50, 10,67, 11,53, 
     7,94, 11,52,  2, 7, 12,51, 10, 8,  1,99,  5,97, 14,66,  3,98,  8,58,  6,52,  0,43,  9,80,  4,23, 13,18, 
     6,97,  8,57,  9,72, 13,97, 10,12,  3,70, 14,33, 12,72,  7,14,  1, 2,  4,99,  0,30,  5,18,  2,95, 11, 2, 
    12,64, 10,75,  7,63,  4,14,  1,55, 11,10,  0,89,  6,89,  3,24,  8,32, 14,70,  5,79,  2,71, 13,42,  9,14, 
     9, 1,  5,82, 12,27, 11,22,  2,44,  7,97,  0,76, 13,16, 14,27,  1,24,  6,98,  8,25,  3,82, 10,75,  4,15, 
     7,26,  3, 4, 13,18,  8,51,  6,47, 11,27,  2, 6, 14,84,  4,72,  9,29, 10,91,  1,76,  5,78,  0,36, 12,93, 
     0,35,  1,39,  2,89,  6,53, 14,85,  5, 7,  3,90, 12,16, 10,70, 13,49,  7,73,  4,13,  9,12,  8,89, 11, 9, 
     1,56,  3,40,  6,51,  5,47,  2,77,  8,65,  0,84,  7,93, 10,54, 14,66, 12, 6, 11,36,  9,87, 13,41,  4, 7, 
     6,56, 11, 1,  9,57, 13,45,  3, 3,  0,14,  1,74, 10,29, 12,65,  2,43, 14,13,  4,42,  7,67,  5,45,  8,78, 
     2,81,  3,72,  0,99,  4,52,  9,69,  1,39, 12,74,  5,47, 10,29,  8,73, 13, 6, 14, 5,  6, 2,  7, 8, 11,25, 
     7,17,  4,64,  9,97, 11,94,  3,99,  5,68,  1,36, 10,21, 14,22, 12,61,  8,43,  2,93, 13,82,  6,91,  0,86, 
     9,20, 12,28,  7,98,  5, 7,  2,18, 10,37,  4,60, 14,47,  8,62,  6,75,  1,42,  0,52,  3,97, 11,46, 13,98, 
    10, 9, 14,15, 11,85,  0,55,  4, 7, 13, 6,  1, 3,  9,27,  7,11,  6,31,  5,90, 12,81,  3, 5,  8,86,  2,30, 
    13,53, 14,87,  5,93,  4,62,  6,19,  1,12,  0,53, 12,73, 11, 4,  3, 1, 10,65,  2,35,  9,65,  7,23,  8,40, 
    14,13,  6,22,  8,34,  2, 5,  7,68,  5,81, 11,53, 10,66,  4,96,  0,50,  3,40, 13,70,  1,92, 12,13,  9,43
  };
  // Taillard 30x15 instance 7
  const int tai_30_15_7[] = {
    30, 15, // Number of jobs and machines
     2,81, 13,59,  1, 8,  9,88,  8,14, 12,18,  4,22, 10,52, 11,75,  3,33,  6,23,  7,69, 14,42,  5,26,  0,54, 
     6, 4,  3,79,  0,76, 10,59, 13,42,  7,28,  4,75, 14,60, 11,41,  5,14,  1,99,  8,58, 12,41,  9,66,  2, 1, 
     4,37,  6,63, 12,46, 11,79,  0,38,  7,44, 14,18, 13,45,  5,55,  1,78,  9,79,  8,27, 10, 6,  2,21,  3,70, 
    11,58,  4,63, 14,56,  0,27,  1,37,  9,51,  2,37,  8,31,  5,24, 10,73, 12, 7,  3,72,  7,34, 13,32,  6,27, 
     0,95,  5,33, 11,81,  1,23,  7,26,  4,12, 14,32, 13,60,  2,89,  6,78,  3,20,  9,35,  8,35, 12,34, 10,17, 
    11,64,  5,11,  3,58,  2,70,  0,31,  1,74,  4,82, 12,31,  7,65,  8,90, 13,63, 14,81,  9,80,  6,70, 10,82, 
    11,99,  9,28,  6,63,  7,81,  2,86, 14,10, 12, 7,  5,17,  1,22,  3,45, 10,92, 13, 1,  4,37,  8,37,  0,43, 
     4,86, 11,92,  8,74,  2,93,  5,42,  6,28, 12,59,  7,77, 13,81,  0,41,  9,11,  1,45, 10,62,  3,22, 14,57, 
     1,12,  2,90, 12,70,  7,89,  5,37, 13,56, 10,21,  9,74,  4,63, 14,39,  0,37,  8,83, 11,78,  6,66,  3, 6, 
    11,38, 12,67,  8,27,  7,11, 10,45,  3,21,  6,73,  9,47,  2,31,  1,24,  0,59, 13,91,  4,46,  5,48, 14,42, 
     3,63,  2,17, 14,59,  7,27,  9,81,  6, 7, 10,19,  0,52, 11,74,  1, 9,  5,50,  8,59, 12,41, 13,64,  4,96, 
     1,81,  5,91, 13,10,  6,46,  8,65, 10,73,  3,59,  2,93, 12,75,  0,47,  9,61,  4,86, 14,65,  7,29, 11,21, 
     2,63,  8, 9, 13,81,  5,37,  9,32,  0,62, 12,93,  1,63,  6,53,  3,99,  7,62, 11,10, 14,85,  4,43, 10,25, 
    11,26, 12,46,  9, 7, 13,50, 10,68,  4,81,  5,88,  7,66,  8,90,  3,51,  2,62,  0,29,  1,87, 14,41,  6, 8, 
    11,90, 13, 8, 12,63,  7,57,  6,23,  5, 5,  0,20,  1, 6,  3,31,  9,42,  4,86,  8,76, 14,98, 10,45,  2,86, 
     0,11,  2,94,  1,42,  6,95,  9,43,  3,51, 12,42,  5,39, 13,82,  8, 1,  7,96,  4,36, 14,74, 10,74, 11,74, 
    14,12,  8,77,  1,13,  3,31, 13, 9,  4,39,  0,57,  2,25, 10,55, 11,60, 12,87,  6,55,  7,85,  9,12,  5,78, 
     3,55,  5, 4, 10,12,  1,42,  4,46,  6,89,  8,44, 14,33,  7,15, 11,73, 12,47,  9,72, 13,81,  2,79,  0, 6, 
     0,77, 12,44,  1,62, 13,17,  3,70,  4,19,  9,69, 14,70, 11,30, 10,97,  8,82,  5,36,  2,19,  7,33,  6,50, 
     1,98,  9,42, 13, 4,  7,26,  0,84, 10,34,  2, 3, 11,59, 14,52, 12,70,  8,49,  6,42,  4, 6,  3, 7,  5, 6, 
     4, 2,  2,84,  3, 1, 12,76, 13,10, 10, 2,  6,75,  7,10,  9,97,  8, 3, 14,18, 11,53,  5,31,  0,84,  1,17, 
     8,63, 11, 6,  3,77,  6,85, 12,20,  2,28, 10,81,  9,76,  7,33,  4,76, 13,27, 14,87,  0,13,  5,37,  1,62, 
     4,20,  1,70,  6,89, 10,60,  8,64,  3,39, 11,67, 12,78,  2, 7,  9,46,  0,25,  5,49, 14,27, 13,76,  7,98, 
    13, 3,  9,22,  5, 9, 12,66,  6,39,  8,51,  3,30,  1,92,  7,94,  2, 8, 11,24, 14,27,  4,88, 10, 9,  0,65, 
     3,79, 10,33, 14,62,  5,85, 11,17,  7,64,  8,66, 13, 2,  1,71,  6,88,  4,64,  9, 3, 12,44,  0,60,  2, 6, 
     0,91, 12,24,  6, 5,  1,31,  4,53,  9,53,  7, 8, 10,15, 14,11,  5,53, 13,22, 11,83,  8,50,  3,81,  2,52, 
    10,87,  2,62,  0,84,  5,91,  4,53,  9,17, 13,72, 14,13, 12,92,  8,92,  7,16,  3,13,  1,13,  6,69, 11,44, 
     9,83,  3,62,  8,61, 13,26,  4,14,  6,69, 14,34,  5,61,  2,12, 12, 2,  0,27, 10,51, 11,64,  1,14,  7,82, 
    14,54,  2,82,  6,68, 10,83,  1,71,  5,81, 11, 6,  7,42,  0,22,  9,22, 13,94,  4,25, 12,53,  8, 5,  3,70, 
     6,67,  7,72, 12,47,  5,35,  4,78,  9,34,  1,67, 13,86,  2,89, 10,69,  8,46, 14,57, 11,87,  0,22,  3,87
  };
  // Taillard 30x15 instance 8
  const int tai_30_15_8[] = {
    30, 15, // Number of jobs and machines
     5,20,  3,56, 12,69,  6,15,  9, 6,  1,75, 14,84,  4,24,  2,64, 13,37, 10,79,  0,95,  8,27, 11,60,  7,35, 
     1,27,  7,58,  0,56,  2,30,  4,30, 11,57, 10,55, 14,63,  5,32, 12,43,  8,44,  3,74,  9,46,  6,11, 13,41, 
     9,43,  1,75,  6, 8,  4,71,  0,50, 10,56,  5,13, 13,14,  7,99, 11,57, 14,86,  2,28,  3,37,  8,83, 12,70, 
     5,68,  8,66,  6,27, 14,33,  1,47,  0,73,  3,87, 11,70, 12,21, 10,22,  7,30, 13,10,  4,11,  2,10,  9,17, 
     0, 6, 11,49,  1,11,  5,69,  9,50, 13,50, 14,12,  2,73,  7,84, 12,92,  3, 1,  8,84,  4,88, 10,52,  6,46, 
    12,14,  1, 7, 10,11,  7,79, 14,45,  9,22,  0,85,  2,67,  6,64, 13,75,  8,22,  3,35, 11,65,  5,30,  4,68, 
     8,67, 12,17,  3,58, 11,20,  6,45,  5,55,  2,53,  7,10, 10, 3, 13,66, 14,63,  1,36,  4,93,  0,40,  9,85, 
     8, 8,  4,27, 10,68,  0,39, 14,24,  2,67, 12,32,  1,42,  6,54,  9,18,  3,58, 13,75,  7,37,  5,63, 11,16, 
    12,83,  5,27,  3, 3,  8,25,  4,68, 11,63,  7,33, 14,86,  2,12,  1,22, 13,75,  6,16,  9,44, 10,80,  0,48, 
     0,93,  9,54,  5,81, 14,14,  1,69, 13,66,  6,20, 11, 2,  7, 6, 10,88,  4, 7,  8,73,  2,66,  3,66, 12,81, 
    13,62,  6,81,  4,36,  7,86,  2,95,  8,92, 12,46, 10,10,  5, 6, 11,18,  0,41,  1,70,  3,29,  9,22, 14,15, 
     6,47,  0,38,  7,59,  8,97,  4,62, 13, 9,  2,21,  1,30,  9, 8, 14,23, 12,74, 10,48,  5,14, 11,68,  3,55, 
     6,67,  0,26,  4, 3,  7,83,  2,73, 12,19, 10,12,  8,81, 13,15, 11,34, 14,88,  9,54,  3,35,  5,58,  1,69, 
     3, 7, 12,42,  5,32,  1,93,  6,97,  4, 4,  7,98,  2,80, 11,90,  9,58, 13,40, 14,15,  0,34,  8,58, 10, 3, 
    11,68,  3, 3,  1,39, 13,51,  7,71,  9,77,  8,44, 14,43, 10, 6,  4,38,  6,10,  2,81, 12,42,  0,28,  5,65, 
     7,29, 13,12,  9,64,  4,55, 10,77,  2,20,  0,78,  6,39,  5,88,  3,47,  8,81,  1,41, 14,18, 11, 7, 12,40, 
     7,61,  5,26, 14,24, 10,60, 13,76,  6,57,  3,67,  2,28,  9,61, 12,60,  4, 3,  8,20,  1,47,  0,26, 11,90, 
    13,33,  7,82, 10,36,  1,51,  5,97, 11,19,  8,63,  0,27, 14,35,  6,28,  4,26,  3,13, 12,66,  2,11,  9,26, 
     5,49, 10,71,  7,99, 11,67, 14,77,  1,20, 12,96,  0, 1, 13,88,  2,21,  4,81,  6,84,  3,49,  9,92,  8, 7, 
    10,83,  7,17,  5,92,  2,87, 11,17, 14,61,  3,31,  9, 1,  0,67,  6,80,  4, 8, 12,16, 13,50,  8, 9,  1,69, 
    11,38,  3,73,  6,86,  2,65,  7,83, 10,25,  1,35, 13,22,  0,81,  4,14,  8,19,  5,42, 14,21,  9,30, 12,83, 
    10,67,  9,61,  0,96,  3,44,  4,13,  5,38,  8,51, 11,90,  1,84,  7,30,  6,13, 14,60, 12,20, 13,14,  2,82, 
    10,87,  9,75,  0,24,  4,67,  7,20,  2,96,  1,76, 13,61,  6,44, 14,51, 12,90,  8,40, 11, 4,  3,16,  5,51, 
     6,22,  0,52,  7,35, 13,58,  5, 1,  3,62, 14, 4,  1,68,  9, 8, 12,39,  8,48,  2,76,  4,51, 11,25, 10,37, 
     7,34,  6,51, 11,27,  5,40, 13,11, 12,96,  8,81,  4,88, 14,90,  9,32,  0,62,  3,52,  1,91,  2,54, 10,96, 
     7,40,  5, 1, 10, 3,  6,71, 11,20, 13,52,  9,92,  0,73, 12,17,  2,87,  8,81, 14,35,  1,24,  3,23,  4,93, 
    13,58,  7,65,  2, 1,  9,81,  1,34, 11,48,  0,82,  5,32,  8,23, 10,44, 12,20,  6,80,  3,85, 14,56,  4,90, 
     7,82,  9,61,  4,68, 14,65, 13,48,  2,88, 12, 2,  3,76,  8,37, 10,72,  0,18,  6,11, 11,33,  5,24,  1,65, 
     9,94, 12,14,  7,17, 11,25,  4,57,  5,81,  0, 8,  1,81, 13,59,  8,97,  3,43, 10,34,  2,55,  6,36, 14, 2, 
    10,23, 11,64, 14, 7, 13,89,  5,13,  1,96, 12,84,  9,91,  3,20,  6, 3,  8,45,  2,50,  0, 1,  4,41,  7,57
  };
  // Taillard 30x15 instance 9
  const int tai_30_15_9[] = {
    30, 15, // Number of jobs and machines
     5,76, 11, 4,  1,98,  9, 1,  6,51,  4,92, 13,75,  3,47,  7,35, 10,70, 12,21, 14,20,  2,79,  8,81,  0,63, 
    11,71,  8,85,  4, 3,  9,45,  1,13,  2,57, 12,26,  5,63, 10,38, 13,91,  6,73,  7,59, 14,74,  0,37,  3,65, 
     9,60,  2,74,  0,35, 13,73, 10,49,  6, 1, 11,92,  3, 5,  8,41, 14, 2, 12,95,  5,28,  7,37,  4,78,  1,86, 
    12,95,  1,73, 13,23,  5,77, 14,47,  0,24,  8,29, 11,88,  7,69,  3,49,  4,28,  6,66,  2, 4, 10,41,  9, 3, 
     5,28, 10,26, 12,71,  8,81,  7,36,  2,96, 14,95,  9,45, 13,10,  4,55, 11,87,  0,65,  6,54,  1,26,  3,60, 
     4,40, 11,36,  2, 5, 14,78, 12,63,  5,96, 10,25,  8,86,  1,85,  6,56,  9,25,  0,30, 13,98,  3,78,  7,41, 
     2,90,  3,15,  0,89,  6,87, 14,52, 11,59,  7,22,  5,42,  4,46, 12,60, 13,54,  8,87,  9,22,  1,97, 10, 1, 
    14,51,  5,75,  6, 2,  8,86,  9,19, 10,88,  1,20,  2,88,  4,24,  7,42, 11,90,  3,20, 13,29, 12,20,  0,50, 
    13,11,  2,96, 10,92,  0,94,  6,78,  9,63,  8,44,  7, 8,  4,68, 14,77,  5,52,  1,74,  3,43, 11,10, 12,87, 
     6,75,  1,78, 14,27, 10,27,  7,82,  2,91,  4,88, 11,76,  0,37, 13,43,  9,52,  8,71,  3,45,  5,99, 12,70, 
     4,47,  6, 8,  7,99, 13,85, 11,11,  8,16, 10,24,  5,10,  3,10,  0,12,  9,37,  2,39, 14,38, 12,76,  1,91, 
     5,62, 13,98,  9,68,  2,14,  0,57, 12, 2, 14,52,  7,36,  1,58, 10,54,  3,99,  6,57, 11,52,  8,90,  4,58, 
    12,53,  6, 6,  2,65,  0,68, 10,53,  3,66, 11,15,  4,83, 14,80,  5,73,  7,86,  9,57, 13,23,  8,88,  1,37, 
     3,73, 12,65,  4,54, 10,95,  8,12,  7,69,  5, 4,  1, 7,  6,12, 11,82,  9, 5, 14,22, 13,15,  2, 2,  0,38, 
    10,59,  6,49, 11,29,  5,69, 13,79,  3,57,  1,27, 12,62,  0,57, 14,22,  8,29,  2,42,  7,59,  9,20,  4,86, 
    10,81, 13,24,  1,55,  4,95, 11, 2,  5,94,  6,38,  7,43, 14,15,  3,52,  8,54,  2,66,  0,64, 12,24,  9,29, 
     2,20,  6,25,  5,70, 13, 6,  4, 3, 10, 5, 14,73,  7,25,  0,58,  8,36,  3,91, 12,22, 11,61,  9,38,  1,33, 
    14,61, 13,20,  2,21,  1,22,  4,22,  9,69, 11,98,  0,12,  3,30, 12,98,  8,28,  7, 8,  6, 7,  5,51, 10,66, 
     7,77,  0, 3,  3,11,  8,23,  2,56,  4,30, 10,77,  1,64, 13,52,  5,70,  6, 3, 12,97, 14,93, 11,54,  9,15, 
     0, 1,  1,22, 11,99, 14,34,  5,48,  2,15,  9, 9, 13,67,  8,85,  4,41,  6,13, 10,48, 12, 7,  3,66,  7,55, 
    12,44, 11,94, 13,33, 10,28, 14,23,  4,31,  8,10,  1,15,  9,50,  3,68,  7, 7,  5,50,  0,79,  6,76,  2,89, 
    10,73,  0, 2,  1,76,  4,26, 14,50, 13,93,  7,93,  9,35,  2,64,  3,42, 11,17, 12,26,  8,60,  5,73,  6,57, 
     0,79, 11,56,  8,22,  5,39,  6,27,  9,38, 14,14,  4,55,  2,64, 13,99, 10,28,  7,97, 12, 7,  1,92,  3,71, 
    12, 2,  5, 3,  1,33,  7,74, 13,69,  4,58,  8,99, 11,79,  0,84, 10,92,  6,98, 14,41,  9,37,  2,12,  3,12, 
     5,46,  8,23,  3,48,  4,69, 13,71,  9, 9, 12,94, 10,44,  1, 1,  7,26,  0,93, 11,54,  2,24,  6,77, 14,44, 
     0,83, 14,86,  3, 6, 12,61,  6,39, 10,72,  8, 1,  7, 8,  9,17, 13,60,  2,41,  1,16,  5,21,  4,21, 11, 6, 
     8,28,  4,59, 14,62,  7,97,  2,52, 13,58, 11,49, 12,83,  0,11,  5,49,  6,24,  3,56,  9,43, 10,34,  1,23, 
    12,75,  2,82,  0,75,  3,94,  5,67, 13,15, 14,23,  1,57, 11, 4,  4,51,  8,23,  7,40,  9,63,  6,97, 10,20, 
    10,14,  3,33,  2,16, 13,14,  8,24,  6, 1, 11,20,  9,96,  0,75,  1,36,  4,92,  7,74,  5,13, 14,79, 12,48, 
     5,33, 10,89, 11,89,  4,49,  0,58,  2,32,  3,95,  9,64, 13,11,  8,13, 12,43,  7,98,  6,32, 14,56,  1,62
  };
  // Taillard 30x20 instance 0
  const int tai_30_20_0[] = {
    30, 20, // Number of jobs and machines
     5,59, 13,72,  3,92, 15,35, 19,17,  9,48, 10,57,  6,18, 12,55,  4,51,  2, 8, 14,68,  8,90,  7,39, 16,79, 11,40,  0,66, 17,55, 18,62,  1,66, 
    13,42,  2,60, 15,52, 12,14,  4,83,  9,95,  3,36,  7,28,  5, 4,  0,31,  8,14, 14,23,  6,71, 19,95,  1,69, 10,17, 18,13, 17,23, 16,78, 11,51, 
     8,93,  5,12,  2,34, 18,32, 17,59, 14,73,  0,46,  4,25, 11,82,  3,90, 16,70,  7,27,  6,82,  1,85, 12,40, 15,35, 10,61, 19,92, 13,98,  9,53, 
    13,31,  8,95,  0,36, 11,61,  3,92, 14,24,  7,70,  4,15, 18,76,  6,50, 19,18,  5,30,  2,93,  9,62, 10,83, 15,16, 17,75,  1,29, 12,35, 16,31, 
     8,27,  6,10,  2,93, 13,26, 16,64, 11,70, 15,16, 12,48,  9,46,  4,37, 19,89, 17,83,  5,71,  3,72,  7,45,  1,73, 10,97, 18,12, 14,57,  0,62, 
     1, 9, 13,16, 10,75,  0,21,  2,49, 12,97, 19, 5,  5,20, 17,26, 16,50, 18,26,  4,20, 15, 8,  8,96,  7,50,  6,84, 14, 7, 11,52,  3,73,  9,43, 
     2,20,  3,76, 16, 3, 17,45, 10,53, 15, 6, 13,83, 12,77,  9,21, 19,52,  8,32,  0,27, 18,93, 11,81, 14,78,  1, 6,  5,93,  7,60,  4,12,  6,35, 
    12,41, 10,59, 17,37,  4,89,  3,87, 11,32,  2,98, 19,24,  8,30,  9,84, 13,49, 14,84,  7,44, 18,16, 15,43,  6,65, 16,44,  1,83,  5,71,  0,71, 
    10,46, 14,36,  3,60,  0,59,  7,53, 17,58, 19, 8,  2,33, 11,70, 16,93,  6,38,  4, 5, 18,75,  1,23, 13,98,  5,90, 12,18, 15,62,  8, 4,  9,56, 
     0,27,  2,31,  3,45,  9,57,  8,79, 12,14, 16,82, 13,96,  4, 4, 10,17,  6,46, 19, 3, 15, 7,  1,42,  5,24, 17,86,  7,67, 14,79, 18,43, 11,17, 
     1,72,  4,54,  0,51, 10,87, 15,52, 11, 4, 16,35,  9,62, 19,15, 13,45, 14,84,  8,65,  6,85, 17,49, 18,98, 12, 5,  2,81,  7, 8,  3,72,  5,33, 
     6,31,  5,86,  9,46, 10, 3,  0,63,  2,58,  3,81, 12, 7, 11,54,  7,39, 13,46,  8,92, 19,96, 14,57, 16,40,  4,49, 17,57,  1,86, 18,20, 15,91, 
    13,35, 19, 5, 11,43,  5,84,  2,77, 15,20, 14,84,  4,70,  3,79,  6,52, 10,92,  1,34, 12,39, 18,30,  9,65,  7,11, 16,88, 17,32,  8,80,  0, 2, 
    10,59,  3,36,  4,72,  9,46, 17,48,  8,72,  0,76, 19,48,  2,69, 11,62, 12,30,  1,48, 18, 7, 15,89,  5,37, 16,49,  6,30, 14,52, 13, 1,  7,56, 
     4,18, 14,35,  8,61, 15,23, 11,46,  9,12, 10,38,  1,59, 17,50, 16,75,  5,60, 18,57,  6,63, 13,89, 19,71, 12,52,  7,83,  3,86,  2,81,  0,98, 
     4,33,  9,14, 18,19, 19,84,  3,69, 17,59, 15, 2, 16,83,  5,12,  8,21, 11,73, 14,83,  0,26,  2,94,  1,65, 12,98,  6,83, 10,45, 13,40,  7,89, 
    17,63, 10,72, 13,80, 19, 2,  0,94, 12,11, 14,25, 18,10,  2,90,  5,73, 11,20, 16,92,  7,11,  4,85,  8,63,  1,97, 15,38,  6,13,  9,42,  3,59, 
    19,95, 13, 4,  9,95,  3, 6,  6,67, 17,30,  7,88, 16,26,  5,57, 14,61, 15, 9, 18,35,  0,23,  2,47,  1,46, 12,96,  8,19, 11,54, 10,75,  4,11, 
     9,64, 11,79, 17,87, 13,91,  3, 2,  1,61, 10,31, 16,85,  0,53,  5,77,  7,25,  6,94,  4,43,  2,13, 18,40, 15,59, 14, 3, 19,80,  8, 7, 12,98, 
    17,56,  8,12, 13,74,  7,42,  4,98,  0,75,  6,18,  9,98, 16,20, 18,72, 11,34,  5,74, 10,10, 12,98,  2,12, 19,95, 15,33, 14,69,  3,93,  1,81, 
    17,73,  2,38,  7,25, 16,92,  6,38, 12,91, 14,95,  9, 2,  3,79, 18,41,  0, 3,  4,99,  5,83,  8,18,  1,12, 19,71, 10, 4, 11,66, 15,20, 13,53, 
     0,61,  7,24, 16,24, 12,22,  3,85,  6,56, 19,98,  2, 5, 10,29,  9,73, 18,27,  8,99, 13, 4,  5,99, 11,63,  4,25, 15,61,  1,51, 17,84, 14,30, 
    10, 5, 18,17,  4,40,  5,88, 12,30,  6, 3,  9, 1,  1,96, 11, 9, 13,94,  8,69,  7,72, 17,90,  0,14,  2,41, 14,50, 19,69, 15,38, 16,12,  3, 1, 
    14,55,  0,19,  5,61, 11,61,  9,97, 19,76, 10,38, 15,69,  2,24,  3,62,  4,24, 18,94, 17, 3, 13, 5, 12,84,  7,43,  6,73,  1,76, 16,47,  8,91, 
    11,85,  6,98, 18,68, 14,57, 16,63,  0,58, 17,74, 10,52,  9,59,  2,47,  7,73,  4,79, 19,48,  5,38,  8,88,  1,85, 12, 4, 15,44, 13,37,  3,75, 
    18,44, 13,32, 10,38, 12,93,  1,40,  9,56,  0,80,  7,90,  2,74,  5,82, 16,59,  6,91, 17,40,  8,26, 14,74,  3, 7,  4,49, 11,88, 19,60, 15,35, 
    14,75,  2,73, 19,13, 10, 4, 11,77, 16, 5,  9,57,  7,98, 15,60,  1,99,  5,12, 13,14,  4,25,  8,86, 18,13,  0,93,  3,41, 12, 1,  6,53, 17,54, 
     0,33, 18,75,  8,97,  9,31, 12,84, 17,49, 16,51, 19,30, 15,62, 11,67,  2,84, 13,45,  3,48, 10,62,  5,64,  7,87,  6,14,  4,76,  1,42, 14,71, 
    18,74, 11,98,  9,11, 14,96,  4,39,  0,31, 16,54,  3,49,  8,51,  6,40, 13,21, 15,19,  7,44,  2,76, 10,64, 12,43, 19, 9, 17,30,  5,66,  1,17, 
    16,31,  3,77,  9,92,  6,27,  2,71, 18,82, 11,36,  1,33,  4,48,  0,91,  5,49,  7,39, 12,91, 15,47,  8,74, 14,17, 13,62, 19,28, 10,91, 17,58
  };
  // Taillard 30x20 instance 1
  const int tai_30_20_1[] = {
    30, 20, // Number of jobs and machines
     4,11, 10,45, 12,34,  5,61,  3, 8,  8,97, 19,59, 15,72,  6,82,  0,57, 11,87, 13,61,  1,90, 16, 4, 17,17, 18,69,  2,95, 14,15,  7,97,  9,93, 
     6,57,  1,64,  2,15, 13,19, 11,14,  3,54, 12,71, 14,29, 10,17, 19,43,  7,81, 18,87, 16,65, 17,62, 15,80,  5,75,  0,19,  4,48,  9,33,  8,68, 
    17,71, 15, 1,  8,90, 16,81,  1,84, 10,19, 13,75, 14,83,  6,25, 12,69,  0, 1,  9,80,  5,35, 18,76,  2,37,  4,23,  3,13, 19, 4, 11,81,  7,20, 
     7,16,  1,91,  2,22, 17,28, 16,89,  8,99,  9,69,  4,22, 15,85, 14,25,  6,60, 18,33, 13,17,  3, 6,  0,94, 11,56, 19, 8, 12,77,  5,54, 10,82, 
    10, 3,  1,51,  7,43,  3,21, 17,66,  0,71,  2,17, 18,98, 12,73, 16,76,  6,93, 13,88, 11,61,  8,79, 15, 9, 19,18,  9,31, 14,80,  5, 4,  4,32, 
     4,44,  8,97,  3, 7,  9,54, 12,12,  2,68,  6,26, 18,42,  5,19, 19,92,  1,57, 11,71, 17,67,  0, 2, 13,49, 16,40, 14,51,  7,27, 15,35, 10,22, 
    17, 5,  4,55, 13, 2, 19, 6, 15,65,  3,42,  2,19,  1,64,  7,51, 16, 4,  8,13, 10,46, 12,52,  6,38, 11,15,  9,87, 14,74, 18,64,  5,80,  0,91, 
    10,92,  1,39,  0,24, 17,71, 18,12,  7,37, 16,64,  6,17, 15,95, 12,52,  2, 9, 11, 3,  4,87,  8,46, 14,71,  5,29, 19,22, 13,62,  3,43,  9,51, 
    17,78,  0,91,  9,26,  7,81,  4,43,  2,43,  5,93, 18,35, 11,36, 12,74, 14,18, 13,30, 19,15,  3,64,  8,90, 10,75,  1,37, 15,35,  6,39, 16,87, 
    11,14,  1,21,  9,74, 16,30, 12,62,  3,70,  6,87, 13,29, 10,86, 14,88,  8,24, 19,54,  4,11, 18,15,  0,21,  5,29, 17,57, 15,75,  2,57,  7,43, 
    19,40, 18,34, 14,46, 10,97, 13,67,  5,59,  3,65, 11,47,  8,20,  7,22, 16,15,  0,66,  6,20,  4,40,  2,72, 12,73,  9,50, 15, 4, 17,88,  1,44, 
     5,33, 18, 9,  6,84,  9,70, 17,56, 12,60,  3,16, 14,84, 15,13,  0,47, 19,65,  4,76,  2,51,  1,34, 10,53,  8,63, 16,14, 11,84, 13,78,  7,92, 
     2,67,  0,18,  5,66, 18,37, 19,65,  9,92,  7,30, 10,57, 11, 1,  4,16,  8,89, 15,36, 17,30,  1,49, 14, 7,  6,73, 12,20, 13,26, 16,29,  3,42, 
    10,99, 12,72,  4,22,  8,79, 13,60, 17,28,  2,59, 19,95, 15,84, 16,49,  3,86,  0,37,  6,10,  5,68,  1,70, 11,20, 18,71, 14,23,  7,71,  9,51, 
     1,39, 16,88, 19,82,  0,41, 17,99, 15,45,  2,11,  9,48,  3, 2,  6, 8, 10,88, 18,95,  7,64, 14, 7, 13,62,  8,19, 12,61, 11,60,  4,45,  5,32, 
     2,81, 14,18,  9,77, 16,33,  3,17,  0, 9,  4, 5, 12,76, 13,75, 19,65, 17,11, 15,69,  8,17, 18,66, 11,36,  7,23,  6,75,  1,64,  5,14, 10,42, 
    17,47,  8,51, 19,60,  7,94,  4,15, 18,13, 12, 8, 13,16,  3,61, 11,72,  6,69,  9,17,  1,44,  5,84, 10,97, 15,93,  0,91, 14,60,  2,99, 16,57, 
    13,28,  2,70,  6,42, 14,96, 18,14,  7,81,  8,57,  0,16, 16,45,  1,44,  3,40, 10,11, 19,70,  4,97, 12,20, 11,80,  5,24, 15,27, 17,55,  9,13, 
    15,92, 19, 4, 11,31,  0,76,  4,91, 10,66,  7,59,  1,97,  8,15,  6,27,  9,15, 18,62, 14,82, 13,94,  3,55, 12,52,  2,77,  5,39, 16,38, 17,53, 
    17,17,  8,99,  4,47, 10,82, 12,14,  0, 2, 11,82,  9,69,  7, 6, 16,89, 18,66, 19,39,  1, 9, 13,90,  3,91,  6,63,  2,13, 14,34, 15,36,  5,81, 
     8,99,  1,68, 11,56, 19,70, 16,72,  5,77, 17,51,  7,64,  2,66, 15,57, 12,74,  0, 9,  9,72,  3,94, 13,63,  6,21,  4,39, 10,23, 14,80, 18, 8, 
    16,67, 13,22,  0,59,  1,37, 10, 6, 15,64, 14,17,  8,50, 17,45, 18,30, 19, 7,  2,78,  4,72, 12,36,  7,23,  6,94,  9,25, 11,74,  3, 6,  5,97, 
     7,42,  6,90,  4,28, 17,19, 15, 7, 14,97, 19,82,  8,41, 10,69,  0,47,  3,76, 12,88, 16,11,  5,68, 13,70,  9,31, 18, 8,  2,81,  1, 3, 11,84, 
     9,62,  1,34,  4,98, 11,65, 17,12, 16,66, 12,32, 14,60,  6,12,  2,85, 15,73,  5,55, 10,97,  0,24,  8, 9, 13,26, 18,92,  3, 3,  7,41, 19,83, 
    12,12,  3,93, 14,74, 10,20,  0,33,  4,89, 11,41,  5,96, 15, 4,  9,99,  2,47,  6,23,  8,12, 13,91, 18,25, 17,83,  1,34, 16,83,  7,70, 19,27, 
    13,99,  8,50,  4,17,  6, 9,  3,72,  7,91, 19,37,  2,39,  5,72,  9,31, 18,72, 11,97, 10,40, 16,43,  0,96,  1,51, 14,29, 12,21, 17,18, 15,50, 
    10,55,  0,41,  1, 4, 13,75, 12,86, 11,59,  5,44,  8,73, 19,66,  2,70,  3,79,  7,85,  4,51,  6, 5, 17,35,  9,17, 15,30, 16,35, 14,34, 18,91, 
    16,97, 19,32, 13,41,  3,33, 15,23,  1,39,  8,74, 14,49, 12,69,  2,28,  5,55, 17,60,  0,61,  9,84, 11, 2, 10,84, 18,17,  4,73,  6,26,  7,91, 
    11,21,  4,51,  8,99, 13,79,  5,21,  3,48, 18,44,  1,67, 17,48, 14,96, 12,19,  9,39,  6,56, 19,76, 10,16,  2,40,  0,69,  7,93, 15,15, 16,52, 
    16,45, 11,22, 14,89,  8,42, 19,43,  5,99, 17,91, 12,34,  3,43, 15,68, 13,76, 18,55,  9, 1,  6,73,  1,56,  0,89,  7,13, 10,99,  4,82,  2,72
  };
  // Taillard 30x20 instance 2
  const int tai_30_20_2[] = {
    30, 20, // Number of jobs and machines
     3,86, 17, 5, 14,21, 10,67,  5,87, 12,90, 18,21,  8,87, 15,82,  4,68,  1,25,  7,10,  9,58,  6,65, 16,20,  0,34, 13,12,  2,35, 11,63, 19,41, 
     8,91,  7,80, 12,38, 14,79,  1,66,  4, 6, 17,21, 19,89,  9,50, 15,93,  0,52,  3,33,  2,82,  6,51, 11,90, 16,55, 10,99,  5,75, 18,22, 13,58, 
    18,59, 14,22,  1,10, 11, 1,  0,75,  6, 1, 12,35,  4,15, 15,39,  5,28, 13,29, 17, 8,  9,65, 16,45,  8, 5,  2,90, 19,18,  3,11,  7,39, 10,70, 
    15,20,  1,39,  4, 2, 11,32,  3,44,  2,85, 17,30, 10,68, 14,67, 13,57, 16,14,  7,75, 18,71,  0,41, 12,36,  5,33,  8,72,  9,32,  6,92, 19,17, 
     5,82, 10,71, 15,55, 13,28, 16,73, 19,12, 18,18,  2,41,  4,78,  7,71,  8,26, 17,97, 11,23,  9,65, 14,54,  6,88,  3,94,  0,28, 12,22,  1,95, 
     9, 5, 16,29,  1,73,  5,69,  6,51, 11,70, 13,24, 14,89,  2,21,  0,89,  4,83, 17,14, 15,61, 19,12, 10,97, 18,57,  8,61,  3,61,  7,19, 12, 3, 
     2,43,  6, 4,  7,32, 14, 4,  4,96, 12,34, 15,21, 16, 2, 13,33,  5,77,  1,62, 10,39,  0,89, 17,90,  9,90, 18,42,  3,16, 19,73,  8,75, 11,57, 
     8,78, 14,63, 13,26, 16,48, 10, 9,  7,26,  2,55, 19,93,  0,15,  6,85,  5,39, 12,87,  4,66, 11,54,  1,68,  3,30, 18, 7, 17,52,  9, 2, 15,31, 
     3,55,  0,87, 14,10,  4, 5, 15,48,  6,78,  8,87,  5, 8, 17,70,  9,69, 13,57, 16,85,  1,58,  2,74, 12,92, 10,77, 18,54, 19,43,  7,28, 11, 6, 
     1,42, 19,71,  0,68,  7,77,  2,19,  6,12, 10,59,  4,74, 17,71,  5,22, 12, 7, 13,53, 14,99, 15,71, 18,88, 16,91,  3,22,  8,46,  9,80, 11,55, 
     9,77,  0,17,  1,79, 13, 1,  3,72, 16,88, 14,42,  2,83,  5,84, 12, 8,  7,40,  8,91, 19,66,  4,85, 18,43, 10,51, 11,94,  6,23, 15,84, 17,15, 
     1, 6, 14,41,  4, 5, 11,87,  2,46,  7,75,  3,49, 12, 6, 16, 1,  9,50, 13,88, 17,65, 18,10,  0,88, 10,46, 15,33,  5,47,  8,72,  6,48, 19,12, 
     0,56,  9,75, 10,53,  4,34,  2,73,  3,83,  7,72, 12,15, 11,28, 17,52, 14,49, 15,15, 13,81,  6,88,  8,11, 19,52, 18,48,  1,88, 16,18,  5,46, 
    17,68, 11,36,  5,34,  3,11, 14,63,  7,31,  1,30, 15,59, 10,85, 12,60, 13,78, 16,82,  2, 6, 18,88, 19,43,  0,66,  9,93,  8,82,  4,39,  6,16, 
     3,58, 19,48,  4,97, 10,67,  2, 3,  1,85, 11,36, 15,24,  0, 2,  5,37, 14,72, 17, 2,  8,25, 13,74, 12,46,  6,43,  7,62,  9,27, 18,77, 16,82, 
     9, 5,  0,93,  6,79, 14,40,  3, 4, 11,82,  8,73, 10,71,  2,61, 13,65, 15,74, 19, 2,  4,57, 18,78, 17,12, 12, 1,  1,83,  5,10,  7,85, 16,48, 
    17,75, 12,63, 18,16,  6,15, 14,42, 16,34, 13,27,  2, 3,  9,83, 19, 7,  7,12,  3,63,  8,94, 11,20,  0,35,  4,75,  1,52,  5,25, 15,98, 10,83, 
     3,39, 15,65,  0,21,  2,34,  4,66,  6,27,  9,81, 10,33, 14,29, 16,95, 11, 1,  5,64, 17,82, 13,61, 18,74, 19,51,  8,48,  1,99, 12,23,  7,57, 
     0,88,  6,93, 17,11,  4,90,  1,27,  2,63, 18,20, 12,51, 11,36, 16,76, 10,26, 15,10,  3,71, 13,74,  8,35, 14,48, 19,12,  7,36,  9,24,  5,10, 
     6,93, 19,56,  3,28, 13,57, 18,21,  4,59,  7,48,  1, 6,  9,16, 16,90,  8, 6, 17,49, 10,32, 14,82,  2, 3, 15, 4, 11,31, 12,25,  5, 8,  0,28, 
     8,68, 18,11,  7,99, 14, 3,  0,78, 11, 1,  4,39,  9,65, 13,19,  5,16,  3,11, 12,26, 17,10, 16,54,  2, 2, 15,69, 19,91,  6,39, 10, 1,  1,91, 
    15,10,  0,24,  8,55,  9,71, 13,99,  5,85,  3,58, 16,18, 10,11, 17,90,  1, 7,  7,88, 14,75,  4,97, 18,75,  2,11, 12, 8,  6, 6, 19,45, 11,78, 
    14,68, 17,57, 19,15,  5,36, 16,27,  6,26,  4,66, 10,38, 15,97, 18,55, 12,73, 13,23,  8,68,  2,19,  7,89,  9,46,  1,34,  0,39, 11,23,  3,60, 
    19,28, 13,20,  2,44, 17,81,  9,62,  1,66,  4,44,  0,52,  3,40, 11,89, 10,92,  8,27, 18, 6, 14,75, 16, 6, 12,96, 15,50,  7,73,  5,60,  6,31, 
     6,90, 14,55, 15,41,  7,20,  4,51, 16,44, 11,67,  0, 6,  1,82, 10, 5, 17,10, 19,63, 13,80, 18,39,  3,22, 12,48,  8,24,  5,66,  2,46,  9,91, 
     8,41,  6, 4,  4,34,  7,68, 15,58, 10,71, 18,57,  3,81, 13,62, 19,84, 14,57,  5,23,  0,31,  2,59, 11,18, 16,74, 17,60, 12,38,  9,70,  1,49, 
     4,53,  0, 6,  5,79,  8,84,  9, 3, 18,41, 14,28, 15,61, 10,43, 12,36, 13,68,  7, 8, 11,35,  3,73, 16,81,  2,93,  6, 1,  1,94, 19,96, 17,73, 
    18,92,  7,94,  3,54,  9,17, 14,11,  6,41,  0,55, 15,15,  8,87, 19,81, 11,62, 12,78, 13,28,  5, 8, 10,77,  4,82,  1, 1, 16,68,  2,84, 17,58, 
     3,82, 15,31, 11,12,  9,78, 16,83,  5,33, 12,39, 19,78, 13,33,  0,11, 17,91,  2,54,  8,26,  1,90,  6,71, 14,12, 18,28,  4,57,  7,99, 10,49, 
     7,37, 18,17, 16, 3, 10,57,  4,71,  3,82, 11, 9,  0,29, 17,17, 14,99,  2,96,  5,97,  9,10,  6,26, 19,36, 15,32, 13,14, 12,35,  8,34,  1, 8
  };
  // Taillard 30x20 instance 3
  const int tai_30_20_3[] = {
    30, 20, // Number of jobs and machines
     5,51,  0,93,  6,49, 16, 1,  2,52, 15,26, 19,74, 10,59, 12,44, 13, 8,  4,81,  3,95,  9,68, 17,57, 18,57,  1,40,  7,17,  8,92, 14,88, 11, 6, 
    17,75, 11,22, 16,11, 13,49,  1,31,  7,32,  4, 5, 14,51, 10,14,  8,43,  2,43, 19,24,  3,83, 12,67,  6, 2,  5,45,  0,75, 15,35,  9,50, 18,95, 
    15,80,  9,13, 18,36, 19,51,  1,63, 16,58, 17,30,  8,75,  0,72, 14,92, 13,13,  2,13,  7,92,  3,12, 12,76, 11,29,  5,64,  6,58, 10,26,  4,21, 
    13,91,  1,95, 19,51,  2,75, 10,89,  6,56,  8,74,  3,60,  0,86, 16,70,  9,97,  5,11, 14,61, 17,68, 15,43, 11, 5, 12,17,  4,18, 18,14,  7,93, 
    13,40,  3, 9,  2,80, 19,82,  1,67,  0,33, 15,84,  6,39, 18,48, 14,89,  5,95, 16,60, 17, 4,  9,99, 11,92, 10,52, 12,79,  7, 9,  4,89,  8,54, 
     6,55, 17,70, 14,95, 19,60,  7, 9, 12,82,  3,52, 18,30,  5, 6, 11,27,  8,57,  0,89,  1,63, 15,29, 16,55, 10,37, 13,66,  4,16,  2,87,  9,63, 
     0,44, 10,47, 15,90, 11,35, 17,79, 14,57,  5,58,  6,98,  8,62,  3, 8,  9,31, 16,94, 12,49,  2,90,  1,11, 19,63, 18,22, 13,44,  7,96,  4,86, 
     9,63, 10,80,  8,72, 13,83, 16,25, 18,55, 15,68,  3,42,  6,70,  5,64,  1,24, 11, 7, 14,45,  0,12,  7,17, 17, 8,  2,41, 12,88,  4, 7, 19,83, 
     4,68,  6,99, 19,37,  5,33,  2,72,  9,98, 16,92, 18,28,  7,14,  8,16, 12,99,  1, 9, 15,93, 13,25, 17, 8, 10,64,  0, 4, 14,74,  3,35, 11,37, 
    17,79,  1,34, 12,36,  5,83,  2,48,  3,23,  8, 2,  7, 5,  6,16,  9,76, 19,10, 15,95, 14,12, 10,94,  4,46, 18,53, 11,35, 16,73, 13,78,  0,55, 
    17,31, 15,75, 11,11,  0,92,  7,46,  6,84,  5,39,  9,17,  4,83, 12,87, 10,86,  2,93, 14,68, 16,67, 19,83,  3, 4, 13,96,  8, 3,  1, 7, 18,51, 
    13, 4, 14,50,  2,20,  5,74,  0,37, 11,95, 16,65, 17,83, 15,98,  9,25, 18,64,  6,90, 19,51,  7,61, 12,97,  3,70,  4,14,  1,13, 10,99,  8,83, 
     3,41,  5,81, 13,93,  0,78, 14,53, 12,66, 11,40, 16, 8,  2,63, 19,66,  8, 2,  9,36, 15,24,  4,61,  7,75, 17,27,  6,71, 18,23, 10,18,  1,60, 
    15,87,  5,29, 18,36, 17, 2,  6,18, 16, 2, 13,11, 12,47,  3,94,  2,92,  9,58,  0,93,  1,47, 14,90,  8,28, 10,54, 11,28, 19,84,  4,68,  7, 4, 
    10,23,  6,74, 19,95, 15,64,  4,21, 17,46,  8,86,  1, 8, 12,58, 18,64,  0,99,  9,29,  3,47,  5,64, 13, 6, 11,25,  2,63, 16,59,  7,96, 14,19, 
     3,75,  9,75, 16,76,  6,83,  7,22, 11,98, 14,85,  8,75, 18,11, 12,64,  5,21, 17,94, 19,46, 10,63,  2,78,  1,35,  0, 9, 15,16, 13,39,  4,28, 
    13,57, 18,66,  5,46,  6,84,  8,16,  0,19,  4, 1, 10,29,  7,65, 12,42,  1,87, 15,38,  3,88, 16,83, 19,86,  2,21, 11,38, 17,61,  9,29, 14,74, 
     5,66, 15,74,  1,43,  2,55, 19,86, 18,69, 12,11, 13,12, 16,61, 17,56, 14,56,  9,77,  0,80, 11,16, 10,13,  3,14,  8,14,  4,96,  6,88,  7,20, 
     8,52, 14, 1,  6,82,  3,57, 11,18,  4,94, 12,44, 15,81,  1,25,  0,75, 17,29,  7,74,  5,10, 10,24, 19,63,  2,42, 18,62, 13,98, 16,67,  9,72, 
    10,81,  0,95, 14,46,  2, 6, 16, 5, 19,18, 17,79,  6,43,  3,28,  8,27, 12,84, 11,83,  7,99, 13,60, 15,86,  9,21,  1,13,  5,28,  4,91, 18,20, 
    17,63, 14,56,  1,24,  4,43,  7,30, 16,22,  0,31, 18,64, 19,56, 13,62, 15,25,  8,85,  2,13, 11,76, 10,63,  6,51,  3,87, 12,21,  9,65,  5, 1, 
    14,54, 16, 1, 17,71,  5,76,  4,23,  8,90, 10,19, 12,97,  3,84, 19,27,  2,70,  9,38, 18,62, 13,94, 15,47,  0,22,  7,52,  1,21,  6,11, 11,97, 
     1,91, 14,67,  2,12,  8,75, 13,42,  6,38, 12,63,  3,92,  5,41, 18,14, 17,28, 11,84, 10,39,  0,49,  7,23, 19,58, 15, 9,  4,19, 16,17,  9,46, 
     7,89, 11,44,  3,61,  9,63,  8,95,  4,70, 18,49,  5,99, 14,44,  0,68, 10,86, 13,86,  1,11,  2,13, 15,17, 17,85, 19,62,  6,80, 12,37, 16, 2, 
    19,39,  6,42,  9,19, 16,81, 17,46,  8,87, 14, 7, 11,58,  7,27, 10,97, 18,53,  2,21,  3,69,  0,97,  1,64, 12,47,  4,11,  5,43, 15,67, 13,11, 
     2, 4, 16,13, 17,44,  6,27,  3,23,  4,25, 11,44,  5,39, 18,38, 14,31, 15,38, 19,95,  0,13,  9,19,  7,29, 12,37, 13,44, 10,77,  1,24,  8,39, 
    13,56, 11,91, 16,37, 14,36,  6,86, 15, 2,  1,39, 17,19,  0,90, 12,43,  5,72,  3,87,  2,37,  4,39,  7,90,  8,33, 10,73,  9,88, 18,34, 19,66, 
    10,56,  2,32,  6,48,  0, 6, 19, 9, 14,57,  7,21, 12,56, 18,37, 16,75,  5,40,  9,93, 15,97,  3, 5, 11,67, 13,24,  1,20, 17,15,  4,16,  8,21, 
    15,31, 19,30, 11,88, 17,45, 18,37, 13,38,  4, 3,  6,97,  8,40, 12,29,  3,24, 14,30,  1,29, 10,45, 16,51,  2,58,  5,82,  0,51,  7,85,  9,37, 
     0,38, 18,77, 19, 8,  6,48, 11,46,  4,89,  5,96, 13,50,  2,21, 12,38, 17,57,  3,26,  1,97, 16,70,  8,23, 10,18,  9,33,  7,34, 14,35, 15,69
  };
  // Taillard 30x20 instance 4
  const int tai_30_20_4[] = {
    30, 20, // Number of jobs and machines
    11,38, 18,65, 19,92,  9,13,  1,61, 10,56,  3,95, 13,77,  7,40,  5,23, 15,87,  0,96,  4,95,  6,51,  8,98, 17,44, 16,10,  2,57, 14,44, 12,28, 
     8,81, 15,37, 18,13, 16,48,  6, 7,  0,87,  2,12, 11,23, 13,83,  7,69, 10,26, 19,61,  9,16, 12,60,  3,79, 17,52, 14,84,  4,93,  5,73,  1,92, 
    17,70, 18, 1,  2,72,  4,36, 14,66,  7,65, 10,62, 12,98, 13,22,  0,65, 19,26, 15,89,  5,12, 16,52,  1,73, 11,52,  9,28,  6,60,  8,11,  3,26, 
     2, 4, 17,27,  4,65, 10,39,  9,93,  6,12, 12,92,  8,86,  5, 9, 18,87,  3,65,  7,79, 16,92, 11,41, 19,97, 14,45, 15,84, 13,89,  1,64,  0,37, 
     4,60, 11,89, 18,16,  9,24,  2,49, 12,93, 16,80,  5,35,  7,61,  6,46,  8,36, 15,68,  3,23, 13,13, 14,51, 10,25,  0,76,  1,46, 19,98, 17,58, 
     4,35,  6,18, 16,72, 18,86, 14,99,  2,52,  7,48,  9,98,  1,58, 15, 7, 10,26, 12,15,  5, 3,  8,37, 19,92, 13, 9, 17,63,  3,20,  0,91, 11,86, 
    19,56, 14,10, 11,62,  3, 8,  6,50,  9,19, 15,53,  4,69,  2,67, 12, 8, 17,10, 10,22, 13,40,  8,85,  0,44, 16,22,  7, 1,  1,90,  5,47, 18,59, 
     6,82,  9,83, 17,75, 12,89, 16,72,  5,39,  0,47,  8,39,  1,15,  3, 1, 14,64, 13,66,  7,17,  2,68,  4,43, 15,63, 18,96, 11,37, 10,64, 19,35, 
    10,58, 16,48, 17,19,  5,17, 15,33, 18,29, 14,46,  2,81,  0,11,  1,88,  4,58, 13,70,  7,99, 12,96,  9,90,  6,46,  8,69,  3,92, 19, 4, 11,45, 
     6,74, 11,78, 15,79, 17,44, 14, 2,  9,63,  7,68, 10,57, 16,33, 19,90,  8,69,  1,91,  4,35, 13,80,  5,26,  0,44,  3,91, 12,27, 18, 2,  2,61, 
    10,59, 11,46,  7,81, 14,42,  5,53,  2,44, 19,21, 15,45,  1,91, 12, 4,  8,76,  0,18, 16,72, 13,78,  9,20,  4,44, 17,52,  3,37, 18,68,  6,33, 
    19, 2,  9,63,  0,82,  3,37, 14, 3,  7,53, 13,89,  6,31, 11,63,  8, 6, 15,98, 10, 2,  4,23, 12,38,  1,87,  2,91, 18,60, 17,55,  5,93, 16,38, 
     6,74, 16,85, 10,55, 13,52,  9,87, 18,25, 14,85,  4,33,  1,42,  8,65, 17,59,  5,91,  2,91, 12,16,  3,30, 19,62,  0,70,  7,14, 15,35, 11,19, 
     4,16, 11,23,  5,70,  0,41, 15,12, 12,99, 13,26, 19,43, 14,14,  3,91, 16,50, 10,78,  9, 1,  1, 2, 18, 4,  6,80, 17,14,  8,63,  7,55,  2,14, 
     1,86, 12,32,  3,56, 15,81, 11,52,  2,14,  5, 7,  8,74, 16,33,  9,69, 17,23,  7,68,  4,45, 13,19,  6,38, 14,35,  0,21, 10,42, 19,86, 18,98, 
    19,33, 12,51,  7,96, 14, 5, 13,56, 10,90,  2,50, 11,41,  3,34,  0,93, 16,61, 17,67,  9,60,  1,31, 18, 5, 15,41,  8,85,  5,58,  6,57,  4,10, 
    16,25, 19,92, 12,17,  9,94,  3,67,  5,60,  6,71, 17,28, 13,70,  4,97,  8,56,  7,29,  1,56, 10,41, 14,57,  0,70, 11,26, 15,50,  2, 2, 18,44, 
     7,27,  6,48,  0,85, 19,17,  9, 1,  3,86, 16,88,  8,43, 11,58,  4,82, 12,51,  2,59, 13,38, 17,99, 18, 7, 15,49,  1,88, 14,56,  5,80, 10, 1, 
    12,61,  9,48, 19,90, 14,59,  7,80,  8,44,  0,26, 13,44, 16,86, 11,31,  6,72,  3,29,  1,68,  2,29,  4,49,  5,23, 10,59, 15,61, 18,70, 17,49, 
     1,37,  4,45, 18,24, 11,88,  0,18, 16,33, 12,42,  8, 4, 13, 7,  7,69,  2,68, 19,39,  3,87,  5,61,  9,42, 15,16, 17,43, 14,83, 10, 6,  6,36, 
    10,91, 16,35,  3, 9,  5,98,  4,49,  7,96, 11,68, 19,81, 15,10, 14,58,  9,21,  8,90,  6,26, 18,36, 12,91,  0,52,  1, 9,  2,49, 13,15, 17,80, 
     5,11,  4,78,  7,59,  8,47,  0,11, 11,24,  1,55,  3,87, 17,28, 14, 2, 16,23, 19,38, 15,71, 13,69, 10,97,  6,74, 12,43,  2,57,  9,44, 18,23, 
    13,13, 14,54, 16,19,  5, 3,  0, 4,  7,13, 18,77, 12,74,  1, 2,  3,66, 19,81, 17,60, 11,38,  4,90,  2,67,  9,34, 15,27,  8,57,  6,72, 10, 7, 
    14,29, 16,69, 19,13, 15,96,  7,90, 11,24,  4,90,  9, 3, 10,57,  3,83, 18,78,  6, 4, 13,24, 17,65,  2,44,  0,21,  5,56, 12,73,  1,93,  8,97, 
     8,53, 13,13,  6,18,  5,33,  1,79, 19,45, 14,17, 17,47,  9,45,  0,79, 12, 7, 11,89,  3,51,  4,32,  7,26, 10,32, 15,43, 18,62, 16,31,  2,14, 
     0,63, 19,56, 12,68, 16,49, 10,40,  5,51, 14, 3,  8,87,  7,63,  9,52, 18,95,  1,56, 11,97,  6,30, 17,99, 15,39, 13, 6,  2,76,  4,34,  3,73, 
    12,68, 14,31, 11,59,  6, 6,  0,30,  9,58,  3,73, 13,62,  4,71,  1,96, 18,23, 15,71,  8,20,  5,11,  2,50,  7,92, 10,62, 16,67, 17,10, 19,65, 
    18,98, 13,83,  7,89, 15,64,  1, 4, 11,27,  0,79, 14,25,  2,78,  3,36,  8,52, 10,41,  6,21,  4,62, 12,78, 17,92,  5,92, 19,88, 16,80,  9,64, 
    11,88,  5,78,  7,10, 10,14,  8, 8,  1,18,  2,10,  6,37, 14,49, 12,27, 17,94, 13,95,  0,37,  4,23, 16,15,  9,87,  3,54, 19, 1, 15,74, 18,40, 
     0,21,  1, 3, 19,32, 10,51, 16, 9, 14,76,  5,23,  6,73, 12,53,  3,81,  7,74,  4,92, 18,69, 17,56,  9,93, 15,52,  2,83, 13, 1, 11,17,  8,46
  };
  // Taillard 30x20 instance 5
  const int tai_30_20_5[] = {
    30, 20, // Number of jobs and machines
    19,49, 12,91, 14, 1, 10,37, 15,32,  7,69, 11,56, 17,65,  1,45,  5,66,  8,17,  0,72, 18,38,  2,64,  3,20,  4,68,  9,71, 13,51,  6,17, 16,26, 
    10,59,  5,43, 11,90,  8,51, 16,96,  6,62, 19,35, 15, 6,  0,54, 13,81,  1, 3, 17,80,  3,94,  7,39, 18,37,  4,21,  2,52, 14,51,  9,36, 12,89, 
    15,57, 19,90, 10,34,  5,37,  6,60,  7,51,  3,27,  9,29,  1,53, 16,20, 17,45, 11,16, 14, 2, 12,24,  4,34, 18,18,  2, 2,  8,75, 13,78,  0,46, 
    13,33,  9,15, 14,68, 18,19,  1,43, 11, 7, 19, 2,  2,19,  3,15,  4,58, 10,80,  5,48, 12,49, 17,82,  6,63,  8,26, 16, 4,  0,38, 15,62,  7,41, 
     3,82,  2,63, 10,72,  8,47,  1,56,  4,89,  5,71, 15,91, 12,75, 11,93, 13,59,  7,58,  0,20, 16,84, 17,63,  9,50,  6,48, 19,85, 14,39, 18,45, 
    19,56, 11,32,  6,34,  9,30,  7,40, 18,67, 13,30, 14,49, 15,17,  0,17, 16,15,  8,58, 12,47,  3,15, 10,21,  1,74,  5,85, 17, 7,  2,41,  4,79, 
    15, 5, 10,76,  5,48,  9,58, 19,23,  0,44,  8,63, 18,56, 12,59, 13,72,  6,34,  4,82, 17,86, 14,43,  7,70,  2,41,  1,97, 16,57,  3,38, 11,24, 
    19,59,  5,34, 14,66,  3,20,  4,13, 12,88, 15,95, 13, 6,  6,68, 18,41,  7,51,  9,20, 11,80,  1, 1, 16,43, 10, 6, 17,37,  0,34,  2,72,  8,62, 
    12,55,  5,59, 19,53, 10,74,  8,77, 11,72,  0,76, 17,95,  4,66,  6, 2, 13,26,  9,53, 16,41,  3,28,  2,26, 14,69,  7,74, 15,60,  1,79, 18, 5, 
     9,63, 15,94, 12,86,  5,97,  3,94, 13,82, 19,88,  8,25, 16,10,  4,72, 18,39, 17,49,  7, 5,  2,38,  6,85,  0,42, 10,89,  1,22, 14,34, 11,18, 
     1,62, 19,65, 18,25, 10,74,  6,48,  2,73, 15,92, 14,69,  7, 2, 17,17, 11,79, 13,81,  5,55, 12,37, 16,79,  4,95,  0,94,  9,31,  3,16,  8,44, 
    11,53, 19,14,  2,77, 17,92, 14,32,  7,47, 15,93,  4,41,  5, 6,  0,71,  1,69,  8,70,  3,96, 10,11, 13,39, 16,10, 12,15,  6,39,  9,98, 18,29, 
     6,25,  5,80,  3,38, 11,44, 13,78, 14,39,  9,29, 12,40,  8, 5,  0,47, 19,61,  1,47,  2,63, 18,80,  4,46,  7,76, 15,15, 17,54, 10,21, 16,25, 
     2,53,  6,99,  4,44, 12,54, 17,89, 16,75,  0,26, 10,58,  7,30,  3,17,  5, 3, 18,17,  1,14,  8, 1,  9,60, 19,49, 15,76, 11,83, 14, 9, 13,32, 
     9,22,  5,10,  1,94,  7,54, 11,91,  0,99,  8,91, 12, 8,  2,39, 19,59, 10, 3,  6,55,  4,29, 15,37, 13, 6, 18,64, 14,81, 16,84,  3,29, 17,95, 
    14,72, 11,67,  5,29, 10,57,  4, 9,  3,40,  1,78, 13,99,  7,53, 15,66,  2,85, 17,31,  0,42, 19,83, 18,46,  8,27,  9,47,  6,60, 16,67, 12,47, 
     2,45, 12,44,  5,83, 13, 8, 17,60,  4, 2, 14,10,  7, 3,  6,29, 11, 9, 19,37,  0,29, 16,22, 10,97, 15, 6,  3,41,  9,81, 18,74,  1,62,  8,95, 
     5,92,  4,37,  2,32, 15,28, 10,29,  3,62, 14,93,  7,30, 19,92,  0,45, 11,35, 17,77,  8,46, 12,47, 16,46,  9,81,  1,43, 13,43,  6,30, 18,18, 
     4,49,  9,86, 13,20,  5,90,  8, 4,  6,44,  7,72, 17,22,  0,90, 12, 1,  1,30,  3,71, 16,77, 15,62, 19,48, 10,25,  2,89, 11, 6, 18,95, 14,44, 
    14,62,  3,63, 16,92, 10,16,  6,20,  2,69,  5,65,  0,81, 13,21, 12,54,  9,97,  7,79,  8,37, 19,97, 18,93,  1,55, 17,70, 15,60, 11,36,  4,57, 
    17,76, 13,54, 16,76, 15,36,  5,93,  4,67,  3,35, 11,37,  0,44,  7,88, 10, 3,  2,22, 12,73, 14,65,  1,26,  8, 3, 19,99, 18,78,  9,38,  6,30, 
     2,77,  9,82, 17,66,  4,75,  7,95,  5,72,  0,76, 18,52, 19,72, 13, 4, 11,70,  1,76,  8,61, 15,88, 16,39, 12,36, 10,88,  6,69, 14,58,  3,14, 
    12,65,  4, 8, 18,90,  1,57, 19,73,  5, 2, 14,62,  8,48,  0,76, 10,87, 15,53,  6,23, 17,76,  2,74, 11,99, 16,34,  3,71, 13,27,  9,87,  7,46, 
    13,76,  6,54, 11,75,  5,69, 17,44, 16,26, 19,63, 10,75,  0,78,  4,78,  3,39,  1, 7,  9,73, 18,27,  2,55, 14,23,  7,55,  8,29, 15,56, 12,36, 
    14,17,  9,42, 13,56, 11,84,  2,44,  5,74,  6,62, 17,55,  8,55,  4,31, 12,71,  0,11, 15,16,  1,29, 18,50, 19,72,  3,64,  7,42, 16,28, 10,70, 
     9,76, 18,56,  6,78,  8, 2,  1, 6,  4,71, 16,19,  7,69, 12,30, 10,87, 13,57, 17,33,  5,87,  2,68,  0,24, 11,31, 19,56, 15, 5, 14,19,  3,82, 
     2,64, 19,36, 15,19,  8,90, 14,65,  3,80,  4,26,  1, 2,  6,52, 11,72, 17,17, 12,29, 13,60,  5,16,  9, 6, 16,91, 18,79,  7,43, 10,99,  0,26, 
    19,82,  1,80, 17,60,  6,93, 13,54,  2,24, 12,87,  8,63,  0,59,  9,85, 11,13,  3,32, 10,93,  4,33, 15,15,  7,48, 16,72, 18,23,  5,97, 14,76, 
    17,61, 14, 6,  6,87,  5,74, 18,67, 16,44, 13,63,  0,12, 19,81,  9,61,  2,26,  7,23, 10,76,  8,93,  4,97,  1,75, 12,76, 15,46,  3,66, 11,54, 
    19,77,  7, 6,  8,62,  0,22,  4,81,  3,44, 18,28,  9,97,  5,16,  2, 7, 12,34, 11, 3, 13,93, 17,12, 15,35,  6,88, 14, 9, 10,93,  1,87, 16,51
  };
  // Taillard 30x20 instance 6
  const int tai_30_20_6[] = {
    30, 20, // Number of jobs and machines
    11,66, 19,38, 16,15,  7, 7, 12,93,  1,57, 10,92,  8,56,  6,93, 18,60,  2,40, 13,74, 17,59, 14,72,  0,21,  3,24, 15,24,  4,57,  5,74,  9,69, 
     4,88, 19,70,  8,42, 17,14,  0,66,  5, 8,  1,32,  6,77, 11,11, 12,30, 10,48, 13,97,  2,89,  3,82, 15,81, 18,89, 16,76,  7,87, 14,44,  9,26, 
    19,53, 10,82, 14,37, 11,85,  3,31,  2,81, 13,24,  1,67,  0, 3, 12,12,  9, 8, 18,72,  4,87,  5,69, 16,19, 17,35,  7,97,  6,46,  8,73, 15,12, 
     2,31, 15,50,  6,74, 19,12, 17,52, 14,89, 12,67, 13,52,  1,21, 11,11,  5,31,  4,69,  9,35, 16,99, 18,24, 10,93,  8,87,  7,15,  0,20,  3,66, 
    17,14,  8,90, 10,14, 16,68, 19, 6,  4,79,  2,14,  6,15,  0,17,  3,68,  9,19, 12,46, 11,72,  7,33,  1,20, 15,12, 14,56, 13,97, 18,26,  5,36, 
     5,71,  6,95, 13, 2, 10,81, 19,93, 16,21,  9,98,  8,64, 15,30, 12,40,  4,67,  7,65, 14,16, 11,33, 17,51,  3,49, 18,68,  2,89,  0,92,  1,35, 
     2,28, 19,33,  4,81,  8,94,  9,11,  6,61,  7,36,  5,33, 11,92,  1,83, 13,14, 14,97, 17,36, 18,61, 10,72,  3,65, 12,26, 16,14,  0, 4, 15,80, 
    16,70,  7,96,  1,35,  6,11,  5,99, 12,83,  4,39, 10,43, 14,87, 15,19,  8,14, 18,46,  0,91,  3,17,  9,32, 11,32, 17,38, 13,46, 19,96,  2,22, 
    12,69,  0,36,  2, 8,  4,21,  1, 2,  6,32, 14,75, 16,81, 19,47,  3,64, 17,80,  5,75, 11,49,  8,41,  9,82, 10,25, 13,89, 15,33,  7,29, 18,47, 
     6, 9, 17,36, 13,22, 19,59, 16,32, 18,33, 11,72, 10,27,  3,45,  2,19, 14,49,  8,35,  5,57,  7,87, 15,59,  9,49, 12,83,  4,52,  0,66,  1,61, 
    12,24, 18,53, 11,61,  7,31, 17,14, 10,19,  6,26,  9,91,  3,53, 19,41, 14,77,  1,66,  4,81, 15,32,  0,29,  2,83, 13,13,  5,31,  8, 6, 16,21, 
     0,45, 18,89, 11,29,  3, 7, 15,47, 10,47,  5,25,  4,45,  7,60, 13,28,  8,83, 19,68, 17,12,  2,37, 14,69,  1,46,  6,47, 16,91,  9,20, 12,45, 
     3,23,  4,55,  7,60,  6,52,  5,17, 15,75,  8,54, 13,76, 17,35, 12,61,  9,51, 10,84, 18,38,  2,94, 14,30,  0,14, 11,78,  1,29, 19,99, 16,88, 
     5,14, 12,22, 14,99,  6,59,  3,89, 10,44, 13,98,  7,56,  8,22, 17,33, 15,41, 18,46,  4,65,  0,85,  1,38, 16, 3,  9,19, 11,39,  2, 5, 19,72, 
     7,12,  4,27, 10, 8, 13,13, 11,26, 16,35, 18,33, 12,49,  8,29, 15,39, 19,37,  1, 1,  0,74, 17,22,  2,38,  3,31,  9, 6, 14,98,  6,86,  5,69, 
     0,72, 17,74,  4, 6, 15,58, 11,27,  3,21, 14,19, 13,97, 10, 8,  7,70,  8,49,  1,25,  9,39, 16,95,  5,71, 18,81, 12,88,  6,11, 19,93,  2,71, 
    19,11,  1,82, 11,82, 17,80,  5,74, 18,60,  7,43, 10,75, 12, 4,  2,64,  0,52,  3,73,  4,77, 13,80,  6,89,  8,66, 16,28,  9,62, 15,82, 14,42, 
     3,11, 12,16,  1,12, 15,72,  7,35,  6,83,  8,73,  9,41, 13,23, 16,63, 19,16, 10,37,  4,28, 17,88,  5,75, 18,51, 14,23,  2,40,  0, 4, 11,78, 
    11,53, 10,30, 15,85, 12, 8,  7,67, 19,35,  9,58,  4,29,  8,54,  2,16,  1,58,  5,73,  6,15,  3,82, 17,76, 13,88,  0,71, 16,57, 18,63, 14,13, 
     0,34, 13,17,  9,36,  5,96, 15,84, 14,84, 17,29,  6,56, 11,83,  7,84, 12,52,  2,37, 10,41, 16,93,  1,79, 19,93,  4,37,  8, 1, 18,45,  3,33, 
    14,19,  2,79, 13,43, 16,43,  9,64, 19, 2,  7,14,  6,58, 18,47, 15,23,  4,93, 17,19,  0,57, 12,77,  5,32,  3,61, 11,27,  8,25, 10,52,  1,53, 
    16, 3,  9, 1, 17,73,  8,81,  6,88,  4,56, 19,58, 18,14,  7,88,  2,68, 14,16, 15,78, 12,48,  5,30, 10,68, 13, 5,  3,47,  0,28, 11,71,  1,19, 
     5,39, 19,72,  2,37, 16,33, 10,53,  6,95, 13, 8,  4,13, 11,23,  1,40,  3,15, 17, 6,  9,25,  8, 1, 15,22,  7,30, 18,10, 12, 7, 14,59,  0,14, 
     4,37,  0,98, 10,81,  2,73,  1,58,  8,27, 18,22,  5,39, 13,98,  7,35, 19,98, 14,73,  6,25, 15,73, 17,72, 16,79, 12,54, 11,94,  9,27,  3,30, 
     6,49,  5,63, 10,97, 11,87,  3,86,  7,81,  0,15,  8,92, 15,73,  9,76, 17,53,  4,75,  2,93, 19,70, 16,35, 12,13, 13,85, 18,95,  1,39, 14,57, 
    18,34,  9,42,  0,63, 11,73, 14, 6,  4,71,  1,76,  5,86, 19,97, 12,16, 17,54, 10,44,  6,49,  3,94, 13,92,  2,24, 15,31,  7,72,  8,35, 16,46, 
    17, 4,  0,72, 16,30,  4,47, 18,83,  1,23,  8,88,  5,72, 15,76,  2, 4,  3,10,  7,89, 19,75, 12,75, 11,24, 14,63, 13,76,  9,77, 10,36,  6,88, 
     4,80, 17,68, 13,65,  2,15, 15,36,  7,34, 10,94,  1, 7,  9,99, 16,44, 11,72,  6,12,  8,33, 18,77, 12,24,  0,57,  3,68, 14, 1,  5, 3, 19, 6, 
     6,90, 18, 3,  5,70,  2, 5,  3,72, 17,60, 19,32,  1,91, 14,42,  9,54, 12,18,  0,63, 15,54, 10,83,  4,92, 13,57, 16,96, 11,11,  8,98,  7,47, 
    10,77, 11,33, 17,66,  3,68,  6,99,  7,47,  5,52, 12,88, 14, 4,  1,71,  9,20, 18,29, 15,82,  0,11, 13,16, 19,57,  2, 4,  4,18, 16,29,  8,68
  };
  // Taillard 30x20 instance 7
  const int tai_30_20_7[] = {
    30, 20, // Number of jobs and machines
     6,59,  5,77, 15,73,  8,46, 13,99,  2, 7, 10,11, 17,75, 11,27,  0,22,  9,33,  1,70, 19, 9,  4,17, 16,33,  3,17, 12,40,  7,75, 18,15, 14,80, 
     2,16, 13,25, 11, 8,  4,41, 18,48, 12,88, 16,38, 17,61, 19,27, 14,78,  5,84,  3, 7,  1,16,  7,90,  9,66,  6,41, 15, 3,  0,47,  8,33, 10,57, 
    18,87, 16,14, 14,49, 13,90,  0,87,  7,29,  1,47, 19,68,  3,12, 12,68, 15,66,  6,58,  5,67,  2,89,  8,32,  9,89,  4, 2, 10,63, 17,70, 11,77, 
     9,10, 10,74,  5,56, 14,88, 18,77,  2,79,  8,69, 11,42, 12,12,  3,76,  4,78,  1,74,  6, 1, 19,16, 13,34,  7,60,  0,66, 16,71, 15,77, 17,84, 
     8,95,  3,24,  7,86,  9,61,  5,67,  1, 4, 17,27,  4, 8, 14,13, 15,12, 13,43, 12,64, 16, 6, 10,50, 11,36,  0,46, 19,71,  6,81,  2,42, 18, 4, 
     6,82,  8,99,  9,34,  2, 4, 13,89, 14,84,  1,77,  3,51, 15,12, 19,72,  4,37,  0, 4, 18,18, 10,91, 12,99, 17,16,  5, 6, 16, 4, 11,77,  7,97, 
    16,37,  7,44, 12,81,  2,72, 13,13,  6,66,  5,52,  8,68,  3, 4, 18,14,  9,31, 15,91, 17,71, 11,86, 14, 4,  0,55, 19, 7, 10, 8,  1,89,  4,80, 
     9,34,  5,32,  2,55,  8,66, 10,18, 13,76,  4,32,  1,28, 11, 7, 17,75, 19,77,  3,24, 14,91, 16, 4,  0,72,  7,84, 15,50, 18,45, 12,25,  6, 6, 
    16, 6,  4,97, 15,68,  1,22, 11,82,  8,74,  3,12,  6,80,  9,79, 17,15,  0,48, 18,91, 10,51,  2,19, 12,74, 13,48, 19,68,  5,43, 14,13,  7,31, 
     3,82, 14,19, 15,80, 17,13,  8,35, 10,98,  6,68, 19,12,  0, 1, 18,15,  9,58,  1,94, 16,54,  5,74,  4, 9, 13,50, 11,82, 12,68,  7,23,  2,76, 
     2,79, 15,20,  4,74, 16,43,  7,88,  8,99,  1,46,  6,75, 18,67, 19,81, 10,94, 14, 6,  5,60,  3,93, 17,88, 13,39,  0,32, 11,88, 12,80,  9,30, 
    13,74, 11,53,  5,31,  9, 1, 17,19,  6,18,  7,38, 16,79,  0,46,  8,74, 15,82, 10,84, 18,27,  1,46,  2,11, 19,37,  4,97,  3,88, 14,25, 12,51, 
    16,59,  7,59, 13,55, 12,20,  2,92, 19, 1, 10,31, 14,61, 15,87, 17,10,  5,40,  1,35,  4,15, 11,86,  0,20, 18,43,  3,39,  9, 9,  6,38,  8,28, 
     2,26,  0, 2, 19,81,  8,64, 18, 9, 10,47, 17,28,  3,78, 13,64,  6,77, 14,16, 16,69, 12,50,  7,81,  4,31,  1,87, 11,42, 15,23,  5,46,  9,45, 
     5,61,  1, 7, 14,75,  9,72,  8,83,  3, 8, 11,63, 10,27,  0,81,  7,76, 13,57, 12, 7,  2,88, 16,62, 19, 5, 17,32,  4,25, 15,53, 18,43,  6,75, 
    17,12, 16,48,  4,71,  0,54, 15,49,  5,47,  3,37, 14,72, 12,39,  2,77, 10,94, 18,82,  8,49,  6,42,  1,87, 19, 2, 11,10, 13,58,  9,81,  7,41, 
    10,84,  4,36,  3,98,  1,10, 17,22,  0,53,  5,51,  8,95,  7,62, 12,82, 19,48, 13,10, 15,29,  9,68, 14,60,  2, 5, 16,41, 11,15,  6,84, 18,45, 
     3, 9, 16,40, 15,20,  5,39, 13,83, 11,28, 19,94, 12,68,  8,19,  6,25, 18,13,  7,63, 17,69, 10,17,  0,74,  1,95, 14,91,  2,89,  4,16,  9,35, 
    14,69,  0,80, 11,20,  5,99,  4,23, 12, 8, 18,43,  8,34, 10,35, 13,83, 16,41,  7, 5,  6,86,  1,16,  3,29,  9,92, 15,44, 17,54,  2,21, 19,81, 
     7,82,  4,97, 11, 5,  2,36, 15,40, 14,58,  5, 8, 19,59, 16,78,  1,18,  9,32, 13,34,  3,66,  8,25, 10,10, 12,36,  0,88,  6,50, 17,82, 18,35, 
     8,74,  6,42, 10,86, 15,22,  5,39,  3,45, 18,26,  1,63, 17,65,  9,70, 19,33,  4,39, 16,74, 13,75,  0, 8,  7,26, 11,25, 14,13,  2,72, 12,98, 
    10,25, 17,46,  6,61,  8,74,  7,40, 19,25,  2,42,  0, 5,  3, 2,  5,65,  4, 1,  1,77, 14,13, 16,42,  9,31, 18,45, 12, 7, 11,20, 15,95, 13,75, 
     7,50,  0,78, 14,72,  9,53,  1,67,  6,46, 11,95, 10,29,  2, 3, 17,31, 15, 8,  5,26, 19,60,  8,52, 18,35, 12,57,  4,57, 13,91, 16,91,  3,35, 
    18,26, 15,80, 16,71, 10,64, 12,57,  6,43,  2,72,  1,99, 17,87,  5,81, 13,15,  4,23,  7,73,  8, 7,  9,70, 19,98, 11,66,  3,47,  0,10, 14,73, 
     0,20,  6,55, 12,87,  4,10,  8,16,  1,59,  5,91, 17,82, 15,53,  2,67,  7,60,  9,34, 11,78, 13,66, 16,98,  3,39, 10,14, 18,65, 14,52, 19,54, 
    18, 3,  9,26,  6, 8,  4,42, 11,70, 19,17,  8,56, 10,31,  3,29,  0,88, 13,60, 15,81, 12,23,  2,23, 16,43,  5,29, 17,74,  7,29,  1,30, 14,63, 
     5,67, 10,66,  1,88, 14,78, 11,79, 18,37, 16, 6,  7,35,  9,61, 13, 3, 17,67, 15,51,  2,64, 12,69,  6,65,  3,90,  8,95,  4,11,  0,28, 19,50, 
     5,54,  6,52,  8,16,  3,39, 15,56, 19,51,  4,49,  2,70, 16,59, 14,66, 13,57, 10,74,  1,86,  7,83, 18,82,  9,65,  0,40, 11,89, 12,53, 17, 3, 
    18,68,  8,44,  1,62, 14,25,  4,69,  6,48, 15,68, 10,70,  7,61, 13,51, 17,74,  5,24, 11,54, 16,69,  3,69, 12,33,  2,61,  0,18,  9,36, 19,78, 
     6, 7, 13,26,  5,79,  8,65,  2,16, 18, 3, 17,71, 19,62, 14,42, 10,44,  7,73, 15,79,  0, 9,  9,61, 11,63,  4,12,  1,47,  3,67, 16,34, 12, 5
  };
  // Taillard 30x20 instance 8
  const int tai_30_20_8[] = {
    30, 20, // Number of jobs and machines
     8,17, 10,42,  5, 3, 13,77, 11,53, 17,65, 19,14, 18,14,  9,77, 15,22,  3,26, 16,53,  0,36, 12,66,  7,26,  2,56,  1,14, 14,41,  6,69,  4,85, 
    14,57, 18,93,  1,85,  2,20, 13,94,  9, 3, 16,59,  6,80, 19,40, 17,83,  7,67,  8,55,  3,25, 10,24,  0,74, 11,47, 15,37,  5,98, 12, 9,  4,84, 
     9, 2,  6,62, 13,35, 10,87,  5,37, 15,79,  2, 4,  8,79, 19,61,  1,35,  4,39, 18,26, 11,24,  7,17, 16, 8, 12,88,  0,39,  3,25, 14,92, 17,48, 
    14,51,  3,30, 10,79, 16,27, 18,25,  1,13, 17, 3,  0,23,  6,17,  8,22,  2,45, 11,13, 19,72,  5,52,  7,56, 15,56, 12,84,  9,16,  4,50, 13,64, 
     5,72, 16,80, 11, 4,  9, 7, 17,85, 10,30, 18,75, 15,47,  0,94, 19,11,  6,75,  4,63, 13,58,  2,63,  1, 4, 14,33,  3,47, 12,78,  7, 8,  8,20, 
     7,32,  9,82,  1,45,  2,14,  4,10,  6,60, 10,98, 18,95, 17,61,  8,88,  5,66,  0,79, 19,98, 15,44, 14,48, 13,27, 11,47, 12,31,  3,13, 16,50, 
     0,32, 12,53,  1,33, 14,70, 18,59,  6,41,  8,95, 17,65, 10,91,  2, 7, 19,19,  5,82, 16,93, 11,56,  7,44, 13,47,  9,32,  3,62, 15,52,  4,15, 
    16, 5, 14,44, 18,94, 11,20,  1,35, 10,75,  5,92,  9,30,  7,69,  2, 4, 12,99, 15,71, 17,18,  8, 1,  0,75,  4,44, 19,35,  3,37,  6,53, 13,96, 
    10,60, 19,54, 18,41, 15,45,  8,79,  0,19,  2,53,  9,91, 13, 1,  3,74,  6,16,  7,56, 12,75, 11,95,  1,90,  4,86,  5,58, 16,42, 14,79, 17, 8, 
     9,78, 14,56, 10,24, 19,60,  8,88, 12,47,  7,33,  6,11, 18,92,  2,72, 11,42,  0,88, 13,30, 15,57, 16,97,  1,25, 17,26,  4, 5,  3,62,  5,45, 
     3,95,  4,62, 18,53, 15,69,  6,45,  2,48, 16,49,  7,59,  5,37, 12,23,  9,94, 17,19,  0,79,  8,81,  1, 9, 10,66, 14,32, 11,17, 19,38, 13,59, 
     2,61,  4,73,  1,79, 15,25, 16,75,  3, 5, 17,76,  6,26, 11,69, 12,18,  7,21, 18,21,  8,16, 13,39,  0,15, 14,64, 19,98, 10,70,  9,54,  5,32, 
    16,46, 19,94, 10,33,  9,24, 14,31, 12,57, 18,57,  2, 8, 17,88, 15,55,  6,69,  7,51,  5,94, 11,43,  1,35,  0,61,  4,14,  3,30,  8,84, 13,79, 
    12,97,  3, 7, 17,59,  4,87,  8,57,  9,37,  2, 4, 11, 2, 13,23,  5,45, 18,73, 10,72,  6,98,  7,79,  1,61, 15,15, 16,80, 14,77,  0,15, 19,76, 
     7,53,  0,66, 18,42, 19,59, 15, 6,  2,60,  9,30,  1,59, 10,63,  6,61,  3,83, 11,14, 13,78, 12,90,  8,38,  4,88, 16,20, 17,23,  5,81, 14,64, 
     1,75, 19,38, 14,15,  7,48, 17,37,  0,92, 13,99, 11,37, 16,79, 12,28, 10,68,  9,20,  6, 6,  3,57, 18,79,  8,97, 15,76,  5,11,  4, 6,  2,95, 
    10,74, 17,45,  9,93, 16, 9, 18,58,  4,16,  2,27, 11,19,  5,19, 15,69,  8,82,  3,25,  7,31,  0,51, 12,85,  6,42, 14,10, 19,85, 13,85,  1,27, 
    12,30, 11, 5,  7,54,  9, 3, 19,63, 16,47, 17,59,  6,45, 15,63, 13,40, 10,10,  4,16,  1,42,  8,46,  5,66,  0,34,  2, 1,  3,15, 14,81, 18,69, 
    15,98, 17,89, 10,45,  4,11, 14,12,  1,49,  3,44, 18,98,  8,15, 16,79,  6,98,  2,48,  0,19,  7,90, 11,20, 12,20,  5,13, 13,78, 19,32,  9,39, 
     0,20, 11, 4, 17,65, 10,99, 15,56,  5,61, 12,45,  9,93,  6,32,  3,44,  4,62,  1,94,  7,57, 14,58, 16,44,  2,88,  8, 1, 13,65, 18,73, 19,64, 
    13,15,  6,71,  4,39,  8,31, 18,32, 12,80,  0,54,  5,38,  3,51,  9,50,  1,58, 14,96, 11,96, 16, 9,  2,65, 15,32,  7,19, 19,54, 17, 7, 10,10, 
     8,53,  6,19, 14,68, 12,99,  0,77,  9,12, 10,81,  2,96, 18,46, 19,56, 13,41,  5, 8, 17,93,  1,10, 11,75,  7,75,  4,85, 15,32,  3,80, 16,84, 
    12,96, 16, 9,  8,42,  4,52, 19,66,  9,80, 13,45,  6,91,  3,31,  2,40, 10,12,  5,60, 14,99, 11,57, 15,68,  1,44, 17,16, 18,55,  0, 6,  7,84, 
    10,98, 18,29,  8,75, 15,40,  6,81,  7,73,  2,70,  0,29, 11,85,  5, 3, 17,89, 12,12, 14, 1,  9,46, 13,30,  3,28,  4,82,  1,10, 19,18, 16,97, 
     6,21, 16,47,  4, 2, 15,63,  0,57, 12,25,  7,25, 10,80,  5,70, 13,44, 17, 7,  3,30, 14,62, 18,55,  1,68, 19,56,  8, 1,  2,25,  9, 5, 11,13, 
    13,41, 19, 6,  0, 7,  5,80, 14,93,  8,12, 10,54, 17,12, 15,38, 12,30, 18,68,  3,36, 16,19, 11,46,  6,71,  1,71,  7,94,  2,66,  9,99,  4,57, 
    16,57, 18,55, 11,46, 15,15, 17,61,  4,64,  9,19,  1,14, 10,49,  5,58, 13,54,  3,54,  8,50,  0,32, 14,40,  2,47, 19,70, 12,97,  7,50,  6,65, 
     7,53,  0,32, 18, 2, 16,85,  6,17, 13,94, 17,46,  5,83, 14,63, 11,67,  9,46, 19,84,  3,34, 12,22,  4,24,  8,70, 10,63, 15,14,  1,76,  2,67, 
    17,25, 16,83,  9,87,  1,50,  0,60, 15,63, 11,86,  5, 5,  3,11, 18,27, 12, 8, 14,32, 13,16,  2,49,  7,20, 19,42, 10,59,  8,13,  4,86,  6,38, 
     7,64,  1,20, 16,31, 17,14,  3,50, 13,93,  0,72, 19,74,  6,13, 10,42,  9,18,  8,25, 15,83, 18,33,  5,21,  2,92, 11,48, 14,60, 12, 4,  4,80
  };
  // Taillard 30x20 instance 9
  const int tai_30_20_9[] = {
    30, 20, // Number of jobs and machines
     2,96,  8,26,  6,33,  0,19,  4,43, 15,17, 16,26, 13,66,  5,84, 12,56, 10,83,  7,66, 14,74, 19,24,  1,85,  3,47, 18,88, 17,97,  9,41, 11,77, 
     0,70, 10,46, 13,90,  3,61,  2,24,  7,63,  1,95, 16,34,  9,47, 17,50, 18,62, 15,10, 11,66,  8,52, 19,49,  5, 4,  4,94, 12,38, 14,93,  6,84, 
     0,48, 19,60, 11,15,  9,25, 15,21,  3,99, 13,56, 18,32,  1,31,  5,36,  7,74,  6,72, 10,91,  4,29, 14,34,  8,50, 12,21,  2,36, 16, 1, 17,30, 
     7,50, 19,84, 10,74, 11,35,  9,86, 14,42,  6,31, 12,62, 16,82, 13,66,  5,39, 15,48,  4,98,  3,99, 17,48,  8,77,  1,31, 18,51,  2,44,  0,41, 
     0,90,  6,27, 11,30,  1,68,  3,25, 16,94, 19,66, 12,48, 10,47,  7,16,  5,90,  2,23, 15, 5,  4, 3, 18,10, 14,37,  9,74,  8,28, 13,25, 17,86, 
    17,32,  8,76, 19,29, 16,60,  0,60, 12,21, 13, 2, 14,65,  3,22,  2,36,  1,80, 10,61,  7,55,  9,84, 15,99,  4,25,  6,68, 11,80, 18,67,  5,50, 
    18,90,  1, 9,  9,28, 11,38, 17,36,  0,19,  2, 4,  6,46, 16,84, 10,71,  4,60, 14,23, 19,63, 12,77, 15,72,  7, 2,  3,63,  5,24,  8,60, 13,99, 
    14,96,  9,78, 11,79,  8,90,  2,63, 16,80,  4,10,  1, 2, 19,67,  6,96,  0,69,  7,13, 15,42, 18,54, 12,76, 17,32, 13,75,  3,52,  5,98, 10,16, 
     3,31, 14,80,  9,77,  0,56, 11,85, 10,95,  4,59, 12,46,  5, 4, 16,85,  2,42,  1,14, 13, 4,  7,40, 17,40, 19,48,  6,90, 18,82,  8, 4, 15,87, 
    13, 3,  7,53,  9,33,  0,93,  3,62,  8,17, 11,65,  4,23,  5,10, 14,44, 15,49,  6, 2,  2,54, 17,25,  1,42, 10,57, 19,23, 18,16, 16,76, 12,12, 
     4,68, 17,54,  2,75,  8,29, 11,29, 19,98, 18,17, 13, 4, 12,10,  3,71,  1,26, 10, 3,  5,51, 15,79, 14,30,  9,58, 16,76,  6,81,  7,63,  0,60, 
     6,98,  0, 6, 12,66, 18,53,  1,60, 14,93, 19,52, 16,68, 10,81,  8,51, 17,85, 11,74,  2,12, 13,23,  7,43,  4,98,  5,26, 15,51,  3,22,  9,26, 
     2,90,  7,35, 13,76,  0, 7, 19,67,  4,10,  9,41, 11,41, 16,18,  3,41,  5,35,  6,13, 14,30, 18,28, 12,32, 15,95, 10,92,  1,71, 17,76,  8,78, 
    14,31,  3,64,  6,21,  7,72,  8,78, 12,88, 15, 4,  5,74, 16,26,  4,11,  0,41, 17,93,  1,32, 18,74,  2,18, 19,37, 10,28, 11,47,  9,98, 13,65, 
    18,10, 11,37,  2,99,  3,28,  5,84,  4,92, 15,12, 13,72, 19,84,  6,90,  1,35, 14,40,  0,63, 12,29,  8,89,  9,16, 16, 4,  7,38, 10,22, 17,84, 
    10,41, 13,38, 16,71,  8,65, 17,86, 11,30,  7,57,  5,71, 14,24, 15,10,  6,78,  3,74,  0,16, 19,25, 18, 6,  2,75,  4,68, 12,67,  1,69,  9,56, 
     2,46, 10,79, 19,36,  7,13, 18, 3,  4,57,  6,79,  5,53,  0,11, 11,45, 13,39,  1,87,  9,25, 12,62,  8,32, 16,13, 14,22,  3,93, 15,90, 17,90, 
     1,64, 13,70, 11, 9,  2,92,  3,15, 18,32, 17, 6,  9,96, 16,51,  6,87, 12,49, 15,75,  0,84,  4, 1,  7,10, 14,39,  8, 3,  5,89, 10,13, 19,21, 
    18,45, 10,40, 12,14, 16,69,  1,45, 11,98,  8,90, 19,19,  9,40, 17, 2,  5,47,  2,70, 13,46, 14,70,  7,93,  6,70, 15,93,  0,33,  3, 9,  4,85, 
     7,13,  8,85, 14,32, 11,30, 10,70, 13,61,  6,42, 16,41, 19,92,  2,87, 18,36,  3,58,  5,66,  4,70,  1,21, 12,22, 15,41,  0,88,  9,91, 17,94, 
    11,19,  6,51, 17, 8,  8,94,  4,72, 12,99,  1,18,  0,39, 19,30,  9,61, 16,19, 18,74,  2, 2,  3,77, 10,66, 15,28, 14,23,  7,14,  5,92, 13,90, 
    16,96, 19,92,  4,34,  5,10,  3,68, 18,94, 15,62,  8,83, 13,26,  0,87, 10,29, 14,95, 11,30,  1,49, 12,43,  2,85,  9, 1,  6,60, 17,80,  7,48, 
     3,42,  4,14,  2,55,  9,97,  7,65, 16,63,  0,74,  5,63, 13,67, 11,48,  8,63, 14,81, 18, 8, 10, 7, 17,22,  6,43, 12,53, 15,22,  1,93, 19,89, 
     7,14,  2, 2, 14, 8,  5,22,  6,93,  9,59, 10,15, 15, 9,  3,10, 16,81, 18,85, 12,62, 11,70, 17,64,  0,93,  8,26,  4,30, 13, 6, 19,86,  1,27, 
    17,10, 11,39, 18,56,  7,23, 12,44,  4,93, 10,90,  3,99,  8,80, 13,47,  2,38,  9,15, 15,41, 19,26,  6,48,  1,52, 16,75, 14,65,  5, 4,  0,57, 
     1,46,  4,78, 10,10,  7,13,  0,32,  9,63, 14,71,  5,66,  2,40, 18,13,  6,50,  3,97, 19,41, 16,95, 11,58, 12,57, 15,63, 17,42, 13,56,  8,31, 
    10,88, 14, 2, 12,34,  6,19, 18,86,  2,90, 13,84,  8,40,  9,52, 19,66, 11,76, 15,62, 17,27,  7,28,  0, 5,  5,72, 16,54,  3,46,  1,57,  4,66, 
    14,98, 13,44,  1,33,  5,20, 17,74,  2,30,  4, 4, 18,88,  3,19, 12,85, 19,81,  0,29, 10,72,  9,79, 11,54,  8,37,  7,95, 15,11, 16,11,  6, 2, 
     5,48, 11,34, 16,25, 12,26, 15,53, 10,97,  0,26,  2,23, 14,36,  4,17,  8,65,  1,97, 18, 5,  9,13, 17,71,  7,32, 19,26,  6, 6,  3,47, 13,57, 
    19,22, 14,87,  4,89,  8,41,  0,70,  1,35, 15,95,  9,62,  3,57, 12,52,  6,18, 13,94, 17,60,  7,34, 11,87, 16,22,  5,96,  2,59, 18,81, 10,90
  };
  // Taillard 50x15 instance 0
  const int tai_50_15_0[] = {
    50, 15, // Number of jobs and machines
     9,14, 10,79,  3, 6,  8,35,  5,42,  7,64,  4,51,  0,67, 12,13,  1, 9, 13,46,  6,84, 11,60,  2,10, 14,34, 
     1,45,  7,42,  8,95,  2,97,  9,43, 14,40, 12,25,  4,22,  3,57, 11,15, 13,59,  0,33,  5,83, 10,72,  6,27, 
     2,15, 13,92, 11,82,  4,76, 14,87,  8,93, 10,30,  3,96,  6,21,  5,76,  7,61,  1,62,  9, 7, 12,21,  0,38, 
    10,36, 12,96,  7,77,  1,97,  5,26, 14,13,  4,90,  9,60,  2,91,  3,86,  8,74,  0,64, 11,42, 13,93,  6, 1, 
     5,27,  3,61,  7,87,  0, 2,  9,30,  6,47, 11,58, 10, 5, 12,83,  4,72, 14,71,  2,52,  1,48,  8,54, 13,27, 
     3,44,  0,66,  6, 1,  2,12,  4,21,  1,24,  5,19, 14, 6, 10,31, 13,50, 11,84,  9,34,  7,59,  8,64, 12,53, 
    13,51, 10,48,  9,39,  8,75,  1,13,  6,94, 14, 5,  0,73, 12,38, 11,28,  7,77,  2,40,  3,45,  5,89,  4,89, 
    14, 5, 11,35, 10,87, 13,48, 12,25,  6, 4,  4,76,  0,22,  7,92,  5,77,  2,86,  1,35,  3,43,  8,75,  9,61, 
     7,49,  2,41,  8,80, 10,26,  1,18, 14,30,  5,43,  4,50,  0,26, 12,23,  3,22,  9,70,  6,44, 13,53, 11,41, 
    12,41,  5, 3,  4, 5,  7,30,  3,93,  1,76, 11,86,  8,20, 14,72,  6,66,  9,81,  2,37, 10,37,  0,48, 13,14, 
     9,27, 11, 8,  8,68,  5, 1,  4,76,  6,11,  2,45, 12,80, 14,24,  1,87, 10,48,  7,45,  0,84,  3,34, 13, 7, 
     3,80,  9, 4,  7,84,  5, 5,  4,52, 10,75,  2, 4,  8,93, 14,33, 11,34, 12,77,  6,55, 13,47,  0,83,  1,61, 
     8,63,  4,67, 12,28,  3,94,  2,58,  9,55, 10,24,  6,98,  0,91, 13,91, 14,17,  1,37,  5,40,  7,11, 11,18, 
    14,43,  3, 7,  8, 3, 11,67,  5,35,  7,39, 12,81, 10,99,  2,70,  0,28, 13,78,  6,88,  1,80,  9,41,  4,68, 
     1,47,  4,17, 14,90,  3,47, 10, 6,  6,86,  5,24,  2,57, 11,18,  0,74,  8,64,  7, 6,  9, 5, 13,96, 12,52, 
     4,54,  8,49, 11,67,  2,51, 13,19,  0,66,  7,51, 12,53,  6, 4,  3,95,  9,28,  1,45,  5,27, 10, 2, 14,68, 
    10,46, 12,50,  1,74,  4,65,  8,64,  7,15, 14,74,  9,90,  2,17,  0,98, 13,28,  6,18, 11,56,  5,80,  3,52, 
    13,51, 11,36,  9,98, 12, 8,  0,80,  1,77, 10,61,  4,95,  6,69, 14,13,  7,34,  8,44,  3,17,  2, 1,  5,37, 
    10,75,  2, 6, 14,16,  1,61,  7,45, 13,57,  4,25,  9,14,  5,31,  3,12,  8, 2,  0,44, 11,98,  6,47, 12, 7, 
     0,49, 11,71,  2,34, 10, 5,  6,90,  3,51,  4,18,  5,66, 12,56,  8,49,  7,38, 14,44,  9,21,  1,74, 13,47, 
     1,81,  4, 4,  8,29,  2,96,  3,78,  7,80, 12,65, 13,61,  9,84, 14,26,  0,36,  5,67, 11,60,  6,16, 10,67, 
     7,33, 12,53,  9,51,  5, 6,  1,95,  2,91, 14,11, 10,21,  6,76,  4,32,  8,56,  3,77,  0,41, 13,82, 11,18, 
     2,47,  0,18, 10,80,  7,82,  3,21, 14,24, 11,67,  1,68,  5,81, 12,49,  8,39,  6,29,  4,20,  9,79, 13,38, 
     9,82,  4,70, 14,56,  1,66,  3,16,  2,62,  8,27,  0, 6,  6, 1, 10,88,  7,45,  5,27, 11, 8, 12,87, 13,41, 
     0,78,  8,21,  2,87, 10,88, 13,33,  3,15, 12,68,  1,37,  7,33,  4,30,  9,48,  5,29,  6,16, 14,41, 11,30, 
    13,73, 14,86,  7,19,  6,99, 11,78, 12,76,  0, 8,  1,45, 10,96,  8,43,  4,47,  3, 8,  9,25,  5,57,  2,91, 
     7, 6, 11,89,  0,51,  6,53,  2,86,  8,64, 14,56,  5,81,  4, 6, 12,53, 13,62,  1,52,  9,51,  3,66, 10,22, 
     9,87,  7,96,  6,25,  5,66, 10,92,  1,44, 14,68, 11,50, 12,23,  0,45,  4,72,  3,93,  8, 9,  2,13, 13,87, 
     2,95,  3,16,  8,64,  1,72, 12,32, 13, 4,  7,51, 11,52,  5,35,  0,77,  9,39, 10,72,  4,65, 14,46,  6,67, 
     8,62,  4,30, 12,99, 11,67,  3,77,  7, 9,  0,56, 10,74,  6,86,  1,63, 14,81,  2,82,  5,71,  9,62, 13,56, 
     8, 6,  1,98,  4,48,  6, 3,  7,45, 14,85,  3,31, 10,43,  5,14, 12,70,  0,16, 13,87, 11,25,  9,62,  2,86, 
     7,92, 13,98,  0,91,  8,30,  9,35,  1,29, 10,80, 12,99,  5,25,  3,53,  4,49, 11,97,  2,34,  6,98, 14,13, 
    10,34,  8,66, 14,83,  2,78, 12,10,  0,95,  3,64,  5,43,  1,65,  9,37, 11,73,  7,33, 13,45,  4, 5,  6,18, 
     1,11,  2,21,  5,42,  9,70,  7,43,  4,48,  0,58,  3,54, 14, 9, 12,17, 13,73, 10,42,  6,18, 11,76,  8,29, 
    11,92,  2,89,  4,28, 14, 8,  6,13,  5,92,  7,13, 12,45,  9,47,  3,36,  8,44,  0,67, 10,25, 13,77,  1,43, 
    14,88,  8,36,  5,13,  7,65, 10, 9, 13,45,  1,89,  0,79,  9,99,  4,94,  3,37,  2,76,  6,99, 12,38, 11,79, 
    13,32,  7,65, 12,86, 11,44,  8, 2,  3,55,  9,66,  2,50,  1,31,  5,66, 14,88,  6,10,  0,46, 10,56,  4,36, 
     0,55, 14,78,  5,55,  2,62,  4,65, 10,97,  9, 9,  6,47,  3,56, 11,92, 12,21,  8,66, 13,40,  7,20,  1,97, 
     9,81,  7,43,  0,72,  6,71,  4, 5,  8,56,  1,84,  3,53, 12,98, 10,76,  5,15, 13,84, 11,48, 14,65,  2,19, 
    14,99, 10,23,  6,14,  5, 3,  0,25,  4,95,  8,53, 12,22,  9,31,  2, 3,  7,86,  1,40, 11,66,  3,80, 13,19, 
     3, 8, 11,37,  4,90, 14,98,  5,17,  8,88,  6,35,  2,52,  9, 1, 10,99,  7,36, 13,29,  1,77,  0,18, 12,55, 
     3,76,  0,33, 10,37,  2,81, 12,71,  7,57,  4,88,  8,29,  6,41, 14, 7, 11,43,  9, 7,  1,32, 13,15,  5,16, 
    14,22,  2,38,  6,21,  5,25,  1, 9,  3,50, 11,51, 13,83,  8,73, 12,85,  9,53,  4,21, 10,12,  0,10,  7,34, 
    10,89,  6,84, 14,94, 11,82,  7,42, 13,26,  1,16,  9,40,  8,55,  0,15,  4,31, 12,73,  2,95,  3,39,  5,11, 
     5,52,  2,71, 12,75,  7,11,  9,77, 11,75,  0,68,  1,55,  3,90, 10,21,  6,62,  8,23,  4,95, 14, 8, 13,66, 
     9,91, 11,45,  1,44,  7,23,  0,47, 14,60,  8,36,  5,81, 10,24,  6,60,  2,62, 13,13,  3,88,  4,38, 12,46, 
     7,90, 10,87,  9,49,  3,77, 13,32,  4,28, 12,80,  5,61,  8,75, 14,23,  1, 5,  6,43,  2,31,  0,80, 11,68, 
     4,92, 11,67, 13,78, 14,39,  5,74,  7,47,  2,56,  9,81, 12,51,  1,85,  0,12, 10,25,  3,64,  8,70,  6,49, 
    11,73, 10,73,  8,59,  3,47,  6,36,  9,81, 12,58,  7,38,  4,33,  1,19,  0,15,  2,97,  5,25, 13,67, 14,89, 
     9, 9,  6,58,  4,64,  2,46, 14, 2,  3,24,  8,68, 11, 4, 10,55, 13,95,  5,98,  7,57,  1,63,  0,31, 12,31
  };
  // Taillard 50x15 instance 1
  const int tai_50_15_1[] = {
    50, 15, // Number of jobs and machines
     1,17, 12,55,  2,62,  6,74, 10,38,  0,44, 11,29,  9,47,  4,94,  7,38,  5,64, 13,75, 14,60,  3,78,  8,10, 
     7,18,  1,59,  0,87, 11,40,  4,53,  9,38, 10,44,  2,38,  5, 7,  6, 9, 13,96, 12,67,  8,58,  3,28, 14,64, 
     9,42,  3,11,  5,93, 10,72,  8,58,  6,49, 12,46,  4,21,  7,93,  0,51, 13,13, 14,72, 11,78,  2,43,  1,55, 
    10,93,  7,47,  3,82, 12,64,  8,38,  0,24,  5,17,  6, 7, 11,49,  1, 4, 14,69,  9,39,  2,25,  4,85, 13,52, 
    11,48,  0,80,  9,48,  3, 3,  7, 7, 13,69,  8,53,  6,46,  5, 1,  1,52,  2,37, 12,25, 14,84, 10,85,  4,14, 
     2,49,  4,42, 14,62,  7,86,  1,14,  8,25,  0,62,  6,63,  5,86, 10, 7, 13,83,  3,84,  9,54, 12,23, 11,16, 
     4,56,  3,79,  7,97, 10,34,  9, 3,  8,83,  2,39, 13,44, 14,43, 11,98,  1,99,  6, 2,  5,47,  0,97, 12, 8, 
    13,61, 14,84,  2,96, 12,64,  5,58,  8,64, 10,14,  4,40,  0,94,  3,13,  7,24, 11,64,  1,63,  9,58,  6,74, 
    14,93,  6, 4,  1,28,  3,43,  0,92,  7,55,  4,87,  9,19, 10,23,  8,23, 12,99,  2,89, 11,42, 13,71,  5,96, 
     4,56,  3,31,  7,72,  6,88,  0, 6,  8,50, 10,66,  1,93, 11,26, 12,17, 13,62,  9, 4,  5,13, 14,46,  2,35, 
     1, 7,  3,81, 14,97,  8,52,  2,93,  5,28,  9,74,  6,17, 12,48,  0,45,  7,51, 10,65, 11,74,  4, 2, 13,10, 
    12,85,  2,85,  4,93,  6,35, 11,51,  9, 6,  3,91,  7,99,  8, 9,  5,38, 10, 3, 14,15,  1,39, 13,55,  0,35, 
     6,14, 14,28,  2,49, 11,53,  0,94,  3, 7, 12,14,  5,29,  1,30,  8,47, 13,50,  9,54,  7,25, 10,90,  4,84, 
     5,16, 11,43,  4,82,  0, 2,  9,86, 10,70, 14,49, 12,26,  3,63,  7,34,  1,86,  8, 1,  2,26, 13, 8,  6,11, 
     8,90,  3,19, 14,54, 12,27,  9,38, 10,57,  4,68,  5,70, 13,76,  7,30,  2,55, 11,98,  1, 9,  6,57,  0,81, 
    11,24,  4,95, 13,91,  0,57, 14,71,  5,71,  9,84, 10,49, 12,94,  6,74,  3,18,  2,22,  8,33,  1,73,  7,81, 
    14,86,  0, 5,  5,94,  4, 1, 10,68,  1,50,  8,53,  9,14, 12,82, 11,80,  3,42, 13, 1,  7,72,  6,48,  2,64, 
    14,19, 13,45,  1,50,  9,14,  8, 3, 12,82,  6, 4, 10,55,  0,94, 11,76,  7,64,  4,69,  3,32,  2,20,  5,48, 
    10,53,  0,33, 12,90,  4,13,  7,73,  1,48,  6,52,  5,57,  3,71,  2,13,  9,55,  8,95, 11,49, 13,32, 14, 8, 
     2,72,  8, 1, 10, 8,  0,63, 11, 7, 14, 5,  6,30,  1,71,  4,74,  7,79, 13,36, 12,71,  5,31,  3,79,  9,43, 
     3,93, 14,96, 11,93,  7,88,  0, 4,  9,12, 12,34, 13,11,  4,17,  8,20,  1,74, 10,70,  5,13,  6,52,  2,83, 
     9,87,  8,39,  7,84,  1,69,  0,65,  3,19,  6,82, 14,48,  2,87,  4,87,  5, 1, 10,58, 13,90, 11,22, 12,81, 
    10,57, 14,17,  6,58,  9,27, 13,48,  5,38,  7,77,  0,92, 12,11, 11,21,  8,70,  2,69,  3,47,  1,91,  4,70, 
    12,92,  2,17,  0, 6,  5,58, 13,47,  9,90,  6,33, 14,25,  1,22,  8,97,  3,40,  7,63,  4,95, 10,17, 11,20, 
     4,87,  1,80, 14, 3,  7,97,  6,53,  2,38, 10,28,  0,31, 11,47,  5, 4, 13,46,  3,11,  8,70, 12,54,  9,44, 
     0,82, 14,50, 11,60,  4,15,  6,66,  1,55, 10,25,  8,44,  3,94, 13,73,  9,78, 12,96,  7,22,  2,18,  5, 4, 
     4,91,  3,10,  0,87, 10,65,  6,12,  9,73,  5,17, 14, 6,  7,85,  1,29, 12,54,  2,72, 11,43, 13,48,  8,29, 
     7,48,  9,41,  2,44,  4,99, 14,14, 10, 9,  8,21,  1,70,  3,87, 13,66, 11,37,  0,82,  5,29, 12,56,  6,10, 
     6,28, 10,64, 11,87, 14,51,  9,52,  0,85,  1,85,  2,59,  7,44,  8,80,  5,51,  3,11, 12,63,  4,68, 13,85, 
     8,32, 12,38, 14,90,  7,18, 11, 9,  0,33, 10,43, 13,59,  2,52,  6,91,  3,57,  9,38,  5,15,  4,18,  1,79, 
     9,14,  5,31, 12,96,  7,95, 14,83,  1,68,  0, 7,  6,91,  3,49,  4,32,  2,93,  8,88, 13,11, 10, 2, 11, 2, 
     4,22,  5,39, 10,25,  1,25,  0,84,  2,49,  8,68,  9,18, 14,20, 12, 7,  6,93, 13,93, 11,64,  3,56,  7,61, 
     9,96,  7,13,  0,58, 10,20, 13, 5, 14,26,  8,26, 12, 6,  1,20,  2, 4,  4,60,  6,37, 11, 2,  3,45,  5,52, 
     1,49,  8,36,  0,25, 12,28,  5,46, 11,23,  2,35,  4, 8,  7,67, 14,45,  9,46,  6,13, 10, 4,  3,16, 13, 6, 
    14,12,  8,84,  5,62,  9,79,  6,98,  0,44, 13,25,  3,25,  2,29, 12,17,  7,12,  4,39,  1,58, 11,25, 10,57, 
     5, 3, 12,43,  8, 4, 14,87,  0,64,  1,36, 10,80,  3,22,  9,20,  4,59,  7,26,  2,45, 13,39,  6,99, 11,72, 
     4,39,  0,48, 11,55, 14,75,  5,64,  1,22,  6,43,  7,91,  9, 7, 12,66,  8,22,  3,43, 10,59,  2,38, 13,80, 
     7,50,  2,75, 10,50, 14,68,  6,33,  3,99,  8,32,  0,46,  1,10, 13,81,  4,93, 12,29,  5,13, 11,98,  9,13, 
     8,60,  6,14,  5,54,  2,11,  7,98,  3, 4,  0,56, 13,17, 14,18,  9,28,  1,85, 10,57, 12,82, 11,99,  4, 4, 
     2,79, 11,91, 14,23,  0,21,  6,91,  7,52, 13,48,  5, 1, 12,23,  8,88, 10, 6,  1,73,  4,12,  9, 1,  3, 3, 
     9,36, 12,40,  7,39,  8,14, 11,80, 14,24,  5,49, 10,27, 13,89,  1, 4,  2,68,  0,77,  4,98,  6,14,  3,74, 
    12,61, 10,48,  0,56, 13, 8,  4,76,  8,25,  3,43,  1,67,  7,10,  9,92,  6,67, 14,33, 11,51,  2,45,  5,98, 
    14,45,  7,38,  8,79,  4,35,  6,24, 13, 1,  5,51,  9,88,  3,94,  0,91,  1,48,  2, 2, 12,49, 10, 8, 11,86, 
    14,44,  6, 6,  3,31, 11,49,  1,11,  2,44, 10,98,  5,81,  8,42,  9,98, 12,77,  7, 9,  4,55, 13,29,  0, 9, 
    12,47,  3,87,  0,50, 11,40,  4,85,  2,86,  7,18,  1,48,  6,91, 10,94,  8,98,  5,86, 13,56, 14,75,  9,46, 
     2,44,  7,53,  0,99,  4,95,  5,32, 10,33, 13,68,  1,22, 11,49,  3,96,  9, 8, 12,87,  8,78, 14, 6,  6,62, 
     6,86,  3,97,  0,16,  8,33, 10,47,  9,93,  4,11,  7,82,  2, 7, 13,18, 14,29, 11,17,  1,56, 12,80,  5,82, 
     8,17,  2, 9,  5,17, 10,65,  6,88,  7,37,  0,53, 11,40,  1,35,  9,24,  4,71, 14,52,  3,30, 13,81, 12, 2, 
     5,65, 11,90,  3,38, 12,97,  7,96,  6,14,  2,85,  0,73, 13,95,  9,87, 14,10,  1,18,  4,17, 10, 4,  8,58, 
     3,69, 11,64,  5,77,  6, 3,  1,75,  2,99, 10,74,  8,56, 13,29, 14,96, 12,83,  9,64,  4,19,  0,18,  7,38
  };
  // Taillard 50x15 instance 2
  const int tai_50_15_2[] = {
    50, 15, // Number of jobs and machines
    14,68,  2,21,  5, 7,  1,19,  3,83,  7,74, 10,12,  4,69, 11,39,  8, 9,  0,63,  9,67,  6,58, 13,37, 12,15, 
     1,62,  0,83,  5,32,  2,56,  4,61,  7,67,  3, 9, 10,50, 13,88, 14,99, 12,50, 11,86,  6,42,  8,70,  9,30, 
    13,20, 14,40,  1,28,  4,51,  8,23,  7,34,  0,10,  6,71,  9,41,  2,14, 10,62, 12,41,  5,14,  3,72, 11,48, 
     9,47, 10,32,  3,99,  4,51, 12,85,  6,49,  1,32, 14,89,  0,75,  7,24, 13, 8, 11,49,  5,86,  2,97,  8,86, 
     1,12,  2,65,  0, 3,  3,89,  8,26, 13,67,  4,24,  7,24, 14, 4, 12,43, 11,33, 10,52,  9,40,  5,84,  6,99, 
     8,77,  9, 1,  2,81,  1,61,  7,51,  3,14, 14,78, 10,69, 11,95,  5,18, 12,15,  0,66,  4,74, 13,84,  6, 1, 
    10,23, 14,58,  7,33, 11,52,  4,26,  3,12,  2,97, 13,78,  5,51,  8,82,  9, 5,  0,74,  1,12,  6,25, 12,40, 
    12,68, 14,66, 10,14,  2,95,  1,19,  9,97,  6,58, 11,54, 13,74,  0, 5,  5,83,  7,92,  3, 8,  4,96,  8,80, 
     3,89, 12,76, 11,74,  8,77,  7,59,  0,39, 13,36,  2,39,  4,45,  9,34,  1,48, 10,72,  6,70, 14,75,  5,42, 
    10,38,  4,43,  6, 9, 13,29,  8,82,  0, 4, 14,42,  2,71,  9,92,  3,27,  1,44, 12,77,  7,55, 11,92,  5,90, 
    13, 7,  4,57,  7,20,  5,39,  8,61,  3,10,  6,93,  2,34,  9,85, 11,62,  0,29, 12, 4,  1,51, 10,62, 14,47, 
     4,65,  2,57,  6,76,  5,89,  1, 9, 13,61,  3,64,  0, 2, 14,85,  8,84, 10,27,  9,70, 12, 5,  7,59, 11,69, 
     0,36, 13,33,  4,79, 11, 8,  5,85, 12,76, 10,92,  3, 5,  9,23,  6,70,  2,24, 14, 1,  7,39,  8, 1,  1,71, 
     0,44, 13,48,  3,59,  1,56,  8,77,  6,12, 12,87,  2,41, 14,89,  5,24, 11,24,  4,58,  9,56, 10,17,  7,33, 
    11,52,  0,20,  3,57,  8,96,  9,12,  5,40,  1,60,  4, 7,  7,34, 13,91,  6,21,  2,44, 14,79, 12,54, 10,35, 
    11,12,  9,24,  3,15, 10,66,  5,62,  7, 4,  2,37, 12,33,  0,77,  8,67, 13,76,  4,41,  6,77,  1,51, 14,81, 
     5,63,  0,60,  7,49,  6,76,  2,75, 12,65, 13,26, 11,74,  9,11,  3,44,  1,67,  8,94, 10,90,  4,62, 14,86, 
     5,31,  7, 6, 12,62,  6,81,  8,72,  0,20,  1,18, 14,78,  2, 3,  4,98, 13,94,  9,22, 10, 4, 11,65,  3,72, 
     4,21,  8,44,  2,87, 11,31,  9,91,  3,31,  5,81,  7,10, 13,31, 10,24, 12,38,  1,90,  6,18, 14, 2,  0, 1, 
    10,31, 12,76,  3,62,  5, 1,  1,66, 11,36,  6,36,  8,55, 14,22,  4,86,  7,74,  2, 8,  9,59,  0,37, 13,37, 
    11, 4, 13,71,  1,39,  6,16,  4,33, 10,26,  9,45,  7,87,  5,42, 12,11,  3,18,  2,11,  0,17,  8,76, 14,51, 
     7,34,  9,38, 14,74,  0,54,  5,60,  3,91, 13,81, 12,92,  1,45,  6,51,  2,20,  4,71,  8,14, 10,13, 11,58, 
    12,66,  0,13, 14,21,  1,92,  7, 3,  9, 3,  8,78,  2,47,  4,46,  5,52,  6,87, 10,87, 13,62,  3,70, 11,90, 
     8,85,  9,17, 11,86,  5,96, 10,55, 12,74, 13,18,  7,81,  1,56,  0, 3,  2,31,  4,15,  3,92,  6,47, 14, 3, 
     6,93, 14,33, 12,74,  1,90,  0,52,  7,37, 11,42, 10,28,  8,83, 13,82,  2,72,  5,13,  3, 9,  9,52,  4,21, 
     9,11,  5,61, 13,47, 12,11,  8,41,  1, 7,  3,31,  2,51, 14, 2,  0,81, 11,45, 10,27,  6,52,  7,88,  4,61, 
     0,98,  6,52,  3,41, 10, 5,  5,20,  4,97,  9,48, 12,23,  1,34, 14,28,  7,75,  2,21, 11,60, 13,86,  8,34, 
     5,55,  9,99, 12,49, 14,44, 10,28,  2,37,  7,12,  0,69,  6,86,  4,74,  1,45, 13,95, 11,97,  3,78,  8, 1, 
     4,93,  0,42,  5,25, 11,34, 10,59, 12,10,  9,44,  7,38, 13,80,  3,29,  8,68,  2,71, 14,27,  1,24,  6,46, 
    13,29, 12,16,  3,78, 10,32,  2,30,  4,72,  7,91,  5,77, 14, 5,  0,90,  8,24, 11, 6,  1,70,  6,54,  9,52, 
    11,38, 14,68,  5,52,  7,79, 12, 5,  6,47, 10,42, 13, 4,  9,66,  2,47,  8,71,  1,79,  3,17,  0,64,  4,53, 
     1,51,  6,90,  5,62,  0,97, 11,77, 12,30,  2,17,  9,13,  4,65, 10,60,  7,90, 14,13, 13,78,  8,82,  3,77, 
     5,22,  9,38,  6,94, 14,10,  4,64,  0,22,  3,91, 11,70,  1,22, 13,54,  2,82,  7,49,  8,30, 12,62, 10,66, 
     6,88,  3,79, 13, 9,  2,18,  4,54, 14,20,  0,59, 12,24, 10,52,  5,70,  1,16,  9,32, 11,60,  7,79,  8,92, 
    13,44,  9, 7, 11,78,  4,85,  3,13,  0, 3,  7,58,  1,62,  2,59, 14,79,  5,31,  8,44,  6,12, 10,79, 12, 6, 
     8,72,  3,70,  5,97,  6,25,  2, 8, 12,99,  4,65, 10, 2,  7,92,  1, 3, 13,61, 14,95,  0,42, 11,82,  9,60, 
     2,15,  8,85,  7, 2,  4, 4,  3,69,  6,42, 12,73,  5,73, 13,28, 10,16, 14,37,  9,59,  1,46,  0,64, 11,41, 
    10,55, 14,88, 13,69,  6,56, 11,48,  0,17,  1, 2,  3,66, 12,70,  8,57,  7,67,  2,38,  4,45,  9,14,  5,94, 
     1,59,  8,99, 13,97, 12,14, 11, 3,  7,26,  0, 6,  2,47,  6,35,  3,71,  4,49,  9,91,  5,38, 10,74, 14,42, 
     7,41,  2,40,  0,98,  8,50,  5,54, 11,14,  6,64,  9,54, 12,84, 10,26,  1,56, 13,69, 14,96,  4, 6,  3,42, 
     4,88,  1,43, 14,40,  6,48, 13,46,  9,70,  8, 8, 12,36,  3,16,  5,85,  0,82, 10,50, 11,74,  2,28,  7,87, 
    13, 6,  5,48,  6,70,  9,98,  0,19, 10,24,  4,37,  8,38, 12,85, 11,99,  1,20, 14,76,  7,94,  3,90,  2,14, 
     4,56,  7,14,  3,61,  5,34,  1,25, 10,70,  2,50,  9,15,  8, 6, 13,77,  6,37, 12, 8, 11,63, 14,37,  0,25, 
     9,95,  1,34, 10,65, 12,88,  8,43, 13,42,  0,30,  3,62,  2,86, 11,52, 14,61,  4,16,  5,48,  6,62,  7,53, 
     0,44,  2,92, 12, 4, 10,59, 14, 3,  3,34,  8, 8,  9,78,  1,22,  4,98,  7, 9,  5,63, 13,84,  6,54, 11,53, 
    10,91,  7,14, 14,88, 13,54,  4,29,  1,66, 12,48,  0,58, 11,42,  6,26,  2, 8,  5, 7,  3,99,  8,23,  9,74, 
    14,67,  7,77,  0,32, 13,97, 10,71, 11,47,  5,67,  1,98,  8,43,  3,62,  2,84,  6,44,  4, 2, 12,60,  9,46, 
     9,27,  7,72,  8,62, 13, 7, 10,76,  2, 3,  3,30,  6,37,  4, 9, 12,13,  0,72,  5,99,  1,16, 11,17, 14,40, 
    12,52,  7,97,  8,25, 14,92, 10,54,  0,55, 13,51,  4,92,  6,40,  9,52, 11,62,  5,42,  1, 1,  2,56,  3, 9, 
    13,64, 14,83,  6,31,  9,47,  0,19,  3,62,  8,11,  5,44, 11,55, 12,60, 10,84,  2,64,  7,83,  1, 7,  4,10
  };
  // Taillard 50x15 instance 3
  const int tai_50_15_3[] = {
    50, 15, // Number of jobs and machines
    11,23, 14,58,  9,60,  1,43,  5,17, 13,68,  6,42,  8,53, 10,18,  7,42,  4,96, 12,19,  0,50,  2,62,  3,97, 
    12,29, 11,62, 10,56,  4,33,  6,72, 14,80,  2,64,  0,80,  5, 4,  3,40,  1,88, 13,78,  9,95,  8,30,  7,21, 
     4,34,  0,60, 13,78, 14,55,  7,41,  9, 3, 12,99, 11,32, 10,86,  2,26,  6,89,  1, 4,  5,49,  3,42,  8,78, 
     4,87, 12, 3, 14,27,  3,69,  8, 8,  9,28,  0,40,  6,73,  1, 2,  7,71, 11,50, 13,95,  2,14, 10,65,  5,63, 
    10,46,  4,40,  5,43, 14,48,  9,28, 12,15,  6,59, 13,58,  0,34,  3,57,  8,29,  2,45, 11,44,  1,27,  7,60, 
     8,67,  0,31, 12, 8,  1,21,  7,18,  4,46,  5,64,  3,27,  2,37, 14,95, 10,75,  9,19, 13,38,  6,91, 11,24, 
     9,33,  4,46, 13,59,  2,71, 14,19, 10,76, 12,61,  6,32,  1,29, 11,26,  5,31,  8,27,  7,71,  0,45,  3,42, 
     9,65,  4,27, 12,62, 11,74,  6, 2,  7,73,  0,40,  2,36, 10,98,  3, 6, 14,49, 13,69,  1,50,  8,58,  5,52, 
     8,94, 12,72,  3,48,  5,23,  7,97, 14,87,  1,73, 11,25, 10, 4,  4,40,  0,60,  9,11,  6,13, 13,66,  2,30, 
     8,69, 11,54,  9,14, 13,61,  3,12, 12,75,  5,25,  7,41, 10,96,  2,23,  1,26, 14,68,  6,92,  4,75,  0,13, 
     4,56,  9,43,  2,10, 10,67, 12,99,  6,50,  8,87,  0, 4, 13,28,  5,28, 11, 4, 14,56,  7,55,  1,83,  3,59, 
    10,46,  6,74,  5,12,  4,96, 12,75,  3,16,  9,90, 13,88, 14,12,  8,81,  2, 8,  7,28, 11,90,  1,20,  0,40, 
     5,36,  9,87,  0,96, 12,22,  8,91, 10,38, 13,92, 11,16,  3,28,  7,46, 14,74,  6,35,  1,15,  4,61,  2,50, 
     4,70,  5,52, 12, 5,  2,48,  6,58, 14,51,  0,32,  7,59,  1,89, 11,71, 13,59, 10,11,  8,79,  9,31,  3, 6, 
    14,94, 10,37,  8,50,  5,88,  4,87,  9,64,  7, 8, 13,17,  3,90,  1,14,  2,56,  6,25, 11,42, 12,18,  0, 5, 
     8,66, 12,61, 11,72,  7,25, 10,32,  2,46,  3,39,  6,92,  4,33,  1,54,  9,58,  5,28, 13,78, 14,61,  0,79, 
     1,97,  4,34,  5,47,  6,71,  9,84, 13,78,  3,62,  7,98,  2,64, 14,89, 11,55,  0,93, 10,86, 12,92,  8,94, 
    12,72,  8,40,  3,56, 10,31,  9, 9,  1,13,  7,62,  6,62,  2,55,  4, 2,  0,29, 13,67,  5,92, 14,31, 11,51, 
     5,35, 12,40,  1,32,  2,41,  3,64,  0,91,  7, 2, 13, 9,  6,48, 11,76,  4,45,  8,12, 10,78,  9, 8, 14,89, 
    12,53,  1,93,  8,79, 11,95,  2,19,  7,29,  6,86, 10,64,  4, 4, 13,65, 14,80,  9,41,  5,91,  3,38,  0,54, 
     3,75,  9,74,  8,38,  0,99, 11,44,  1,58, 14,88, 10,33,  4, 7, 13,43,  5,57,  2,43, 12, 6,  7,15,  6,22, 
    11,85,  1,45,  9, 3,  4,15,  3,50,  8,26, 12,92,  0,62,  6, 5,  7,77, 14,96, 10,59, 13,48,  2,12,  5,43, 
     1,25,  7,14,  6,34,  5,33,  9,18, 14,89,  2,49,  4,73,  3,89,  0,68, 13,72, 12,99, 11,49,  8,73, 10,62, 
     2, 9, 14,39,  9,62, 13,78,  7,10,  8,99,  6,54,  5,54,  4,28,  0,22, 12,90, 11, 8,  3,52,  1,50, 10,10, 
     9,88,  4,90, 12,66,  1,10,  6,76, 10,69,  8,94,  7,57,  2,31, 11, 2, 13,59, 14,18,  3, 1,  0,69,  5,98, 
     4,83,  1,25,  9,37,  0,24,  8,48,  2,55, 14,66,  3,34,  5,37,  6,80,  7,20, 13,77, 11,26, 12,72, 10,31, 
     7,22,  8,31,  6,45,  4,12, 13,87,  1,17,  5,62, 14,14,  2,91, 10, 7, 12,83,  0,58,  3,87,  9,30, 11,97, 
     5,36, 12,68, 10,10,  9,16,  1,69,  4,78, 13,46, 11,31, 14,70,  2,93,  7,96,  0,33,  3,45,  8,81,  6,78, 
     1,13, 13,21,  0,14,  5,75,  4,88,  6,14,  8,28, 11,81, 14,16, 12,82,  9,94, 10,55,  7,64,  3,78,  2,23, 
     3,92,  6,12, 10,46,  5, 2,  7, 5,  8,55,  9,76,  0, 4, 14, 5,  1,44, 12,40,  2,96, 11,62, 13,36,  4,25, 
     4,17,  8,86,  6,36,  9,10,  3,94,  2,65,  0, 4,  5,40, 13, 3,  7,12,  1,74, 12,99, 10, 5, 14,68, 11,38, 
     4,52,  0,44, 12,72,  6,24,  1,92,  3,88, 14, 7, 11,93,  7,12,  2,63,  5,71,  8,88, 10,75, 13,18,  9,38, 
     7,37, 10,64,  1,75, 14,40,  6,14, 11,50,  8,20,  4,23, 12,32,  5,18,  0,29, 13,63,  2,91,  9,64,  3,30, 
    12,69, 13,15,  1,39, 10,23,  7,51,  2,64,  9,54, 14,29,  4,91, 11,16,  5,95,  3,15,  6,20,  0,24,  8, 6, 
     0,10,  2,48, 10,63,  1,82, 12,47,  4,56,  5, 8, 11,56,  9,27,  3,82,  8,11,  7,10,  6,67, 13,89, 14,18, 
     0,49, 10,11, 13,50, 12,25,  6,35,  1,76,  8,76,  2, 1,  3,35, 14,69,  9,19,  4, 4,  7,26,  5,47, 11,11, 
    12,10,  2,15, 10,82,  4,50,  9,49,  6,56,  0,62,  1,57,  7,85, 13,26, 11,17,  8,36,  3,84, 14, 8,  5,68, 
     3,25,  1,19, 12,67,  5,28,  6,88,  2,68,  8,29, 14,63,  0, 8, 11, 5,  7, 5, 10,47, 13, 6,  9, 2,  4,97, 
    11,29,  6,62, 14,20,  5,56,  4,64,  3,82,  1,20,  0,11,  7,65, 12,66,  2,45, 13,58,  8,84, 10,73,  9,70, 
     3,52,  2,66, 10, 9,  9,83,  1,22,  5,77, 13,60,  8,28,  7,31,  4,12,  6,87,  0,85, 11, 8, 12,88, 14,74, 
     6,38,  2,35,  9,29,  7,67, 13,83,  8,57,  1,60, 11, 4, 12,13,  5,51, 10,18,  3,87, 14,18,  4,87,  0,37, 
     2,60,  4,21,  7,98, 14,77, 11,66,  8,81,  9, 8, 13,54,  0,62,  1,41,  5,36, 12,73, 10,50,  3, 1,  6, 3, 
    10,17, 12,23,  7,44, 11,62,  0,43, 14,50,  2,52,  1,18,  4,27,  6,16, 13,93,  5,97,  8,46,  3,80,  9,90, 
     0,94,  9,40, 11,46,  1,18, 14,39,  4,55,  3,52,  7,18,  2,46,  5, 5, 12,26, 10,39,  8,49, 13,94,  6,93, 
    10,56,  0,44, 12,91,  9,63,  5,52,  4,54,  7,31,  2,99,  8,42, 13, 6,  1, 1,  6,94, 11,32, 14,93,  3,97, 
     2,35,  8,27,  6,54,  5,67,  7,72, 11,97,  3,79, 14,13, 10,17,  1,56,  0,63,  4,98,  9,15, 13,18, 12, 3, 
     2,20,  5,47,  3,76, 13,58,  0,42, 10,76, 12,38, 14, 7,  6, 4,  7,25,  1,61,  8, 3,  9,62,  4, 8, 11,99, 
     2,71,  5,94, 14,24,  1,74,  7,77, 13,90, 12,69, 10,46,  3,63,  6,81,  9,33,  8,70, 11,40,  4,91,  0,22, 
    12,25,  1,60,  8,40,  3,82, 10, 1,  4,89,  5, 1, 11,13,  6,62, 14,96,  7,10,  9,83,  2,32, 13,55,  0,87, 
     5,31,  2,67, 14,58, 10,47, 11,37,  3,52,  6,89,  1,16,  9,36,  8,80, 12,80,  7,87,  0,24, 13,40,  4,94
  };
  // Taillard 50x15 instance 4
  const int tai_50_15_4[] = {
    50, 15, // Number of jobs and machines
     5,25,  0,10,  2,40, 14,50, 12,45,  4,91,  7, 6, 13,40,  1,19, 10,88, 11,87,  6,67,  9,34,  3, 1,  8,26, 
     7,53,  2,83,  1,52,  8,92, 12,92,  9, 8,  3,75, 11,56, 13,40, 14, 7,  0,20,  6,84,  4,64, 10, 7,  5,23, 
     4,99,  6,51,  5,86, 13,53,  0,86,  7,30, 14,87, 11, 8,  9,53, 10,39, 12,22,  3,55,  8,64,  1,32,  2,64, 
     9,68, 13,19,  3,13, 11,10, 14,64, 12,50,  6, 4,  1,28,  0,69,  4,68, 10,37,  5,78,  2,41,  7,51,  8,34, 
    11,23,  8,50,  1,25,  9,37,  2,93,  5,74,  6,47, 12,50,  7,40, 13,82,  3,29, 10,91,  0,79, 14,40,  4,19, 
     9,77,  3,19,  4,44,  7,79, 11,79,  2,33, 13,12,  5,12,  8,15, 12,32,  6,64, 10,26, 14,90,  0,49,  1,67, 
     1,28,  2,84,  3,95,  5,28, 13,93, 10,67,  6,75,  0, 8,  8,56,  4,79, 12,27, 11,18,  7,45,  9,20, 14,44, 
     1, 2, 14,60, 10,12, 12,35,  4,45,  8,40,  7,24, 13,90,  6, 1,  2,21,  9,75,  3,79,  5,91, 11,32,  0,41, 
     6,61, 13,30,  2,81,  4,70, 12,82, 14,25,  3, 9,  5,29, 10,27,  9,29, 11,68,  0,64,  1,23,  7,51,  8,51, 
    11,87, 13,94,  0,91, 10,14,  6,15,  9,38,  1,86,  2,16, 14,44,  3,63,  5,62,  8,87,  7,77,  4,31, 12,19, 
     9,64, 10,80,  5,92, 12,99,  7, 1,  0,30, 14,21,  6,76,  1,65,  2,13, 11,36,  4, 2,  8,77, 13,13,  3,68, 
    12,28,  5,53,  1,64, 13,24,  2,51,  8,82,  6,99,  4,21,  7,68, 10,41,  0,14,  3, 9, 14,91, 11,57,  9, 5, 
     4,51,  1,93,  8,77, 10,61,  5,22, 12,77,  6,55, 13,96,  7,76,  3,27,  9,12,  0,63, 14,84,  2,46, 11,14, 
    13,51, 12,35,  2,64,  5,79, 11,15,  7,82,  9,58, 14,72,  6,60,  0,99,  8,47, 10,44,  1,19,  3,99,  4,86, 
    11,49,  7,21, 14,37,  2,24, 12,96,  6,32,  4,94,  0,37,  9,28, 10,30,  1,41, 13,66,  5,12,  3,78,  8,82, 
    14,27,  5,63,  9,35, 11,52,  3,71,  0,62, 12,20,  1,16,  4,64,  6,80, 10,57,  8,34,  7,74, 13,13,  2,80, 
     0,72,  9,98,  2,50,  5,45, 13,73,  4,82,  1, 3, 14,53,  6, 4,  3,86, 10,54, 12,76,  8,55,  7,38, 11,53, 
    11,22,  6,30,  0,17,  5,53, 10,50, 12,86,  2,18, 14, 1, 13,35,  1,93,  7,90,  8, 5,  9,88,  3,11,  4,65, 
     7,28, 10, 4,  4,11,  0,87, 12,62, 14,59, 11,36,  9,57,  5,34,  3, 5,  2,76, 13,83,  1,91,  8,46,  6,64, 
     4,67,  9,20, 12,95, 10,52, 14,37,  3,88,  7,66, 11,87,  2,77,  0,38,  8,56,  1,78,  6,55, 13,28,  5,55, 
     3, 6, 10,19, 14,65,  5, 5,  6,68,  7,26,  1,93,  2,46,  4,49, 13,23, 12,83,  9,61,  8,88, 11,68,  0,62, 
    10,52, 12,25, 13,33,  2,14,  1,29,  0,61,  5,17,  6,82, 11,19,  8,37,  9,77,  3,41,  7,47, 14,45,  4,51, 
     5,68,  2,23,  0,18, 13,65,  1,89,  6,10,  4,98,  9,61,  8,38, 11,64, 10,91,  3,36, 14,66,  7,32, 12,24, 
    11,86,  4,90,  6,44, 10,60, 12,22,  8,83,  2,94,  7,14,  9,80, 14,46,  1,20,  0,13, 13,39,  3,67,  5,17, 
    11, 4, 14,21,  9,59, 13,50,  2,75,  0,41,  4,79,  1,36,  8,54, 10,72, 12,94,  6,46,  5,18,  7,81,  3,45, 
    14,77,  0,97,  8,61,  1,61, 11,93, 12,97, 13,86,  2,15,  9,73,  4,28,  7, 1,  6,80,  3,89, 10,52,  5,30, 
     3,15,  1,27, 12,35, 11,47,  4,79,  7,26,  6,72, 10,89,  9,35,  2,52, 13,17,  5,92, 14, 5,  8,20,  0,49, 
    12, 3, 14,93, 10,56,  0,82,  5,58, 13,65,  1,82, 11, 5,  4, 5,  3,92,  6,30,  8,35,  2,17,  9, 4,  7,78, 
     4,30, 10,55, 13,85,  6,50,  0,29, 11,77,  7,67,  9,55, 14,45, 12, 6,  8,48,  1,46,  3, 9,  2,31,  5,41, 
     3,23,  5,67,  2,33,  9,27, 13,78, 11,64, 12, 5,  6,16, 10, 6,  4,44,  8,43,  0,49, 14,12,  1,17,  7,85, 
     1,50,  6,56,  7,80,  5,54, 11, 8,  3,70, 12,83,  9,18,  2,31, 10, 4, 13,90,  4,87, 14, 1,  0, 5,  8,61, 
     5,22,  4,15, 10, 4,  9,40,  0,69,  6,98,  3,44,  1,77,  7,23, 13,14, 12,96, 11,90, 14,90,  8,78,  2,71, 
    11,94, 13,30,  6,51,  5,86,  1,69,  8,52, 14,11,  0,29,  3,37, 12,70,  4,34,  9,13,  7,10, 10,61,  2,48, 
    11, 7,  9,74,  2,14,  1,65, 12,19,  0,17,  8, 4, 13, 5, 10,27,  5,93, 14,91,  6, 9,  4,69,  7,35,  3, 5, 
    12,40,  8,89,  6,73,  4,92, 14,58, 13,68,  0,97,  9,94,  1,77,  3,43,  7,52,  5, 8, 10,41, 11,21,  2,56, 
     0,71,  4,43, 12,11,  6,65, 14,11, 10,15,  5,46,  7,78,  3, 3, 13,27,  1,33,  2,87,  9,97,  8,59, 11,37, 
     6,68, 11,96, 13,30,  5,84,  2, 2, 12,81,  0,57,  4, 7,  1,70, 10,39,  8,62,  3,94,  7,96,  9,38, 14,46, 
     5,96,  6,34, 11,71,  0,88,  2,10, 12,99,  4,48,  8,57,  7,31,  9,93, 13,33, 10,84, 14,28,  3,32,  1,72, 
     2,96,  6,15, 13,31,  7,93,  5,21,  0,40,  9,99,  4,60, 14,63, 10,95,  1,45, 12,33,  8,83, 11,11,  3,75, 
     7,47,  2,71,  8,37,  9,59,  0,72,  6,64,  4,61,  1,52, 11,20, 14,13, 10,11,  3,26,  5,28, 13,91, 12,27, 
     7,31, 11,76, 14, 4, 12,35,  4,18, 13,50,  5,16,  8,52,  2,85, 10,43,  3,44,  9,21,  1,72,  6,24,  0,12, 
     9,71,  7,30, 12,58,  4,71, 10,87,  1,74,  8,33, 13,26, 14, 3,  3,75,  0,87,  2,14, 11,33,  6,52,  5,38, 
     8,42, 14,39,  9, 8, 10,26,  7,26,  6, 9,  4, 1, 12,83,  1,85,  2,11, 11,81,  0,72,  3,87,  5,41, 13,44, 
    14,73,  7,59,  9, 4,  5,25, 10,68,  6, 3,  4,72, 12,69,  2,50, 13,62,  8,22,  0,77,  1, 1, 11, 4,  3,94, 
    14,59,  8,45, 13,39,  4,64,  9,35,  7,42, 10,16,  0,88, 12, 9,  1,88,  5,85,  2,54,  3,63,  6,21, 11,76, 
     8,66, 11,23,  1,28,  4,12,  9,22, 14,65,  0, 9,  3,27,  5,25, 12,75, 10,85,  7,19,  6,41,  2,20, 13,25, 
    10,28, 11,96, 13,37,  3,58,  9,94,  4,40,  2,42, 12,41,  6,20, 14,80,  5,29,  8,12,  1,81,  7, 8,  0,28, 
    14,83, 12,17,  6,26, 10,78,  4,95,  1,45, 13,16, 11,53,  2,12,  5,50,  7, 6,  8,87,  3,44,  0, 5,  9,63, 
     0,13, 14,85,  9,49,  1,73,  4,48, 11,44,  3,84, 13,85,  6, 1,  2,12, 10, 3,  8,39, 12,75,  7,73,  5,45, 
    11, 3,  3,15,  0,40, 13,77,  8,43, 14,91,  2,51,  7,17, 10,71, 12,33,  5,83,  6,61,  9,68,  4,14,  1,97
  };
  // Taillard 50x15 instance 5
  const int tai_50_15_5[] = {
    50, 15, // Number of jobs and machines
     7,34,  8,91,  0,93, 14,88, 11,59,  6,66,  5,50,  9,53, 13,45,  1, 1,  2,68, 12,77,  4,37, 10,61,  3,17, 
    14,19,  0,73,  6,15, 10,72,  1,38, 11,52,  2,14,  9,69,  8,19,  3,29,  4, 3,  5,48, 12,11,  7,54, 13,28, 
    10,59,  6,29,  3,84, 11, 4,  0,19,  5,35,  4,22,  1,50, 13,68,  2,28, 12,96, 14,23,  9,74,  7,90,  8,51, 
    11,14, 14,86,  3, 8,  0,85, 12,49,  9,56, 10,87,  7, 2,  5,85,  6,60,  2,97,  8,51,  4,39, 13,34,  1,89, 
     5,99,  7, 5,  0,39,  3,39, 10, 3,  1,73,  9,16, 11,62,  8,29, 12,68, 13,14, 14,38,  2,90,  6,56,  4,32, 
     6,23, 13,84,  1,34,  7,77, 10,25,  0,44, 11,45,  8,14,  9,79,  5,90, 12,78, 14,56,  4,37,  3,61,  2,96, 
    14,24,  8,15,  3,99,  9,49,  5,66,  0,99,  1,87,  6,11,  2,45,  7,84, 12,20, 10, 9,  4,71, 13,50, 11,54, 
     8,67, 13,11,  0,97,  3,63, 10,64,  1,33,  2,66, 14, 4, 11,89,  7,60,  5,51,  6,13, 12,33,  9,48,  4,70, 
     6,56, 10,66,  1, 8, 11,92,  5,81,  4,94, 12, 5,  9,21,  2,69,  8,61, 13,50, 14,99,  7,49,  0,26,  3,83, 
     4,14,  9,38,  6,82, 10,99,  8,77,  1,17,  7, 9, 14,21, 13,15,  5,43,  2,39, 11,39, 12,80,  0,19,  3,43, 
     4,76,  1,94,  3,34,  7,45, 13, 7,  5,83,  8,88, 10,47, 12,22,  0,90,  6,11, 14, 6, 11,22,  2,40,  9,51, 
     9,72,  7,65, 10, 2,  2,38,  6,96, 12,10,  8,58, 13,65, 14,17,  5,75,  1,65,  0,79,  3,83, 11,45,  4,52, 
     3,23,  2,35, 12,24,  4,67, 11,65,  7,18,  5, 7,  1,68, 14,19, 10,63,  8,18,  9,80, 13,19,  6,23,  0,39, 
    13,47, 14,70,  1,38,  3,14,  2,46, 11,48,  9,14,  8,45,  6,31,  0,35, 12,95, 10,75,  7,61,  5,44,  4,71, 
    10,26,  3,14,  2,46, 11, 1,  5,23,  9,50,  0,27, 12,82,  8,26, 13, 7,  4,55,  7,22, 14,21,  1,85,  6,66, 
     2,11, 13,84, 14,48, 11,49,  1,19,  5,98,  3,92,  4,42,  8,67, 12,57, 10,40,  0,78,  6,19,  9,52,  7,14, 
     6,12, 11,71,  3,17, 10,67,  4,20,  2,41,  0,74, 14,96,  7,87, 13,20,  1,84, 12,77,  9,72,  8,91,  5,37, 
     6,12,  8,12, 10,66, 14, 2,  1,17,  5,33,  3,30,  0,10,  9,72,  4, 6, 12, 6, 11,41, 13,39,  7,71,  2, 4, 
    10, 3,  1,89,  2, 9,  5,24,  7,80, 12, 7, 13,42, 11,85,  0,84,  9,89,  3,40,  8,42,  4,92, 14,91,  6,77, 
    12,98,  4,83, 11,65, 13,94,  9, 6,  8,96,  7,34, 10, 7,  0,49,  2,25,  3,47,  6, 8,  5,93, 14,67,  1,50, 
    11,52,  6,57,  0,73, 12,45, 14,55,  7,63, 10,99,  5,20,  2,59,  4,90,  8,31, 13,23,  3,99,  9,92,  1,53, 
    14,64,  7, 4,  9,25,  1,52,  4,72, 13,41,  5,11,  0,99,  2,35, 12,77, 10,89, 11,98,  6,63,  8,59,  3,99, 
     9,80,  5,93,  1,64, 14,13,  2,48,  3,47,  0,78, 11,59, 12,58, 10,73,  4, 3,  8,28,  7,13, 13,72,  6, 9, 
     9,24, 10,95,  3,93,  8,48,  0,78,  5,64,  6,26,  4,86, 12,83,  2,41, 14,62,  1,53,  7,35, 13,84, 11,45, 
    14,91,  8,86,  9,79,  5, 5, 11,85,  0,29, 12,73, 13, 9,  4,74,  7,69, 10,23,  1,80,  6,82,  3,34,  2,88, 
     6,76, 10,77,  7, 2, 13,28, 12,27,  3,27,  2,87, 11,33,  5,41,  4,99,  1, 2,  0,44,  8,16, 14,83,  9,91, 
    10,96, 11,27,  9,61, 13,42,  3,99,  7,76,  6,87, 12,36,  0,24, 14,21,  1,88,  4,43,  8,89,  5,78,  2,53, 
     5,86,  7,72,  3, 3, 14,91, 13,33, 10, 1,  8,37,  9,39,  1,30, 12,78,  2,52,  0,52,  4,64, 11,88,  6,75, 
     9,98, 10,52,  4,99, 14,11,  8,57,  6,40,  1,52,  2,75, 11,23, 12,49, 13,65,  0, 1,  3,57,  7,56,  5,92, 
    10,82,  1,33,  4,80,  2,23,  3, 7, 12,49,  5,24,  0,50,  9,29, 14,38,  8,47,  7, 3,  6,53, 13,89, 11,70, 
     3,32,  9,62, 11, 9,  5,88,  4,58,  2,70, 14, 9,  0,66, 12,18,  1,40, 10,33, 13,54,  8,60,  7,92,  6,88, 
     6, 2,  2, 5, 12,35, 13,65,  4,67,  0,58, 10,61,  1,72,  5,60,  9,84, 14,88,  7,23,  8,17, 11,71,  3,13, 
     0,65, 14,56, 11, 5,  5,94,  6,82,  7,76,  1,29,  3, 1,  4,93,  9,66, 13,47, 10,32,  8,42,  2,75, 12,13, 
    13,14,  6,86,  4,65, 11,41,  9, 3,  5,10,  2,49, 12,26, 14,10, 10,86,  0, 1,  3,32,  7,38,  8,48,  1,93, 
     0,98,  1,21, 13,61,  2,61,  4,54,  7,71, 12,98,  3,39, 10,14,  8,38, 11,74,  6, 2,  9,12,  5,93, 14,85, 
     7,93,  2, 1,  4,26,  3,57,  6,38,  9,80, 11,43, 13,64, 10,23,  8,88,  1,74, 14, 5,  5,16, 12,50,  0, 1, 
     1,23, 12,55,  2,72,  5,57,  0,46,  7,17, 13,80, 10,44, 14,80,  4,55, 11,75,  6,69,  9,34,  8,44,  3,30, 
    14,64,  4,93,  9,55, 12,78,  2, 9, 13,24,  7,59,  1,72,  5,30,  8,50, 11,81,  0, 7,  3,53,  6,69, 10, 3, 
     7,63, 10,40,  2,81, 13,33,  8,52,  0,86,  9, 2, 12,43,  3,57,  6,36, 11,53,  4,18, 14,22,  5,92,  1,40, 
     5,60, 14,79,  4,43,  2,83,  9,76,  0,79,  7,53,  6,72, 12,40,  3,37, 13,66,  1, 3, 10,52,  8,33, 11, 9, 
     1,28, 13,70,  0, 7, 14,51,  2,33, 12,57,  7,89,  9,60,  8,64,  4,36, 11,75, 10,49,  5,13,  6,36,  3,65, 
    11,74,  1,97,  0,88,  7,27,  6,95, 14,99, 12,17,  2,31,  5,87, 13,34,  8,28, 10,16,  4,16,  9,94,  3,14, 
    10,63,  8,47, 11, 6,  9,43,  4,48,  5,65,  0,83, 12,98,  1,58, 14,60, 13,12,  2,48,  7,93,  3,77,  6,32, 
    12,32, 13,94,  9,71,  5, 3,  1,20,  6,45,  3,10,  2,45,  0, 6, 11,57,  8,35,  4,76,  7,46, 10,87, 14,25, 
    14,45,  8,63,  9,82, 13,23, 11, 1, 10,13,  7,50,  3,64,  4,82,  2,55,  5,42,  0,14,  6,35, 12,15,  1,47, 
     9, 6,  6, 6, 12,28, 10,96,  2, 2,  7,85,  1,97,  4,90,  0,83,  5,76, 11,65,  3,46, 13,71,  8,42, 14,61, 
    12,95,  2,54,  1,46,  9,33,  6,13, 14,71,  5,37,  8,60,  3,50, 13,30, 11,56,  0,10,  7,62, 10,76,  4,57, 
    10,96,  2,64, 11, 6,  5,90,  9, 1,  0,99,  4,86, 13,27,  7,18,  8,56, 14,19,  3,73,  1,76,  6,82, 12,78, 
     6,98,  5,69, 10,68,  9,45,  4,17, 12,29,  3,15, 14,81,  0,31,  2,79, 13,54,  8,50,  7,73, 11, 2,  1,86, 
     8,44,  0,78, 14,31,  6, 8,  4, 8,  5,15, 10,95, 12,83,  9, 3,  7,30, 13,39,  3,92, 11,47,  2,49,  1,45
  };
  // Taillard 50x15 instance 6
  const int tai_50_15_6[] = {
    50, 15, // Number of jobs and machines
    11,30,  3,16, 12,71,  0,30, 14,78,  9,46,  1,18,  6,32,  2,34, 13,88,  7,94,  4,85,  5,83,  8,30, 10,73, 
     2,14,  8,51, 14, 6,  1,33,  0,17, 10,22,  6,63, 12,71,  7,14,  9,87,  3,58,  4,37, 11,55, 13,73,  5,78, 
    13,39,  0,26,  5,79,  6,10, 14,48, 11,48,  4,97,  3,22,  7,89,  9, 1, 12, 3,  8,44,  2, 9, 10,77,  1,40, 
    13,79,  4,66,  9,39, 11,61,  1,57,  6,96, 12,98,  2,54, 10,88,  3,21,  0,92,  7, 3, 14,39,  8,21,  5,73, 
     9, 5,  3,65,  4,93,  7,90, 11,64,  0,30,  8,93, 10,88,  6,91,  5,47, 14,26, 12,79,  1, 3,  2,77, 13,39, 
     2, 6,  0,61,  5,92,  3,22, 12,18,  1,52,  4,82,  6,48,  9,32,  7,73,  8,49, 10,16, 14,76, 13,55, 11,38, 
     6,16,  1,29,  9,93,  2,89,  7,61, 10,47,  4,25, 11,38,  0,28, 13,46, 14,93, 12,68,  3,99,  5,41,  8,59, 
     2,50, 13,77, 14,11,  4,79,  3,98, 10,66,  7,23,  9,15,  6,24,  5,41,  1, 8, 11,57, 12,68,  0,52,  8,30, 
     7,26, 14,36, 12,79,  2,92, 13,93,  8,11,  6,18,  0,71,  1,26,  5,95, 11,14,  9,86,  4,41,  3, 3, 10,47, 
    10,14,  5,16,  7,56,  3,74, 12,92,  6,33,  2,93,  0,68, 13,70,  8,38, 14,64,  9,79, 11, 8,  4,69,  1,74, 
    13,87,  4,74, 14,84,  6,78,  3,49,  2,45,  5,44,  8,53,  9,83, 12,28,  7, 3,  0,48, 11, 6, 10,52,  1,53, 
     9,71, 14, 9,  7,73,  6,90, 11,58, 12,16,  4,90,  5,54, 10,48, 13,64,  3,17,  1,63,  8,64,  0,98,  2,96, 
     8,58, 10,47,  7,95, 11,34, 13,14,  1,11,  2, 2,  3,52, 14,29, 12,65,  9,86,  4,60,  5,13,  6, 5,  0,16, 
     5,64, 10,29,  2,35,  4, 6,  7,90,  9,42, 12,36,  3,29, 13,57,  1,39, 14,52,  6,39,  8,93,  0,21, 11,77, 
    13,85,  1,98, 11,44,  7,85,  3,45,  8,64,  5,33,  4,49,  2,23,  6,84,  0,53, 14,17,  9,48, 12,31, 10,11, 
     3,33,  4,89,  7,48,  9,20,  8,95,  5,57, 11, 8, 14,19, 12,21,  2,36,  6,65, 10,37,  0,93, 13, 4,  1, 2, 
    10,51,  3,57,  9,69, 14,74,  0,95,  7,79, 13,37, 12,82,  6,75, 11,20,  4,49,  8,38,  1,78,  2,97,  5,73, 
     9,68, 12,91, 13, 5, 14,50,  6,32,  2,48,  4,39,  7,81,  0,32,  8,68, 11,92,  3,74, 10,27,  5,60,  1,59, 
     7,82,  3,63,  6,18, 12,32, 13,69, 14,82,  0,76,  5,39,  8,72,  1,90, 11, 4,  2,54, 10,79,  9,81,  4,72, 
     3,98, 11,97, 12,37,  4,95,  6,93,  0,56, 13,46,  8,85,  5, 7,  9,78,  7,71,  1,69, 10,47, 14,41,  2,64, 
     4,65,  2,64,  1,28,  3,46, 13,27, 10,53, 12, 6,  8,71, 14,12,  5,15,  7,61,  6,89, 11,56,  0,35,  9, 9, 
     7,86,  8,86,  5,72,  3,13, 13,69,  0,39, 10,98, 12,28, 11,32,  6,64,  2,21,  1,86,  4,50,  9, 8, 14,90, 
     8,96, 13,82, 14, 2,  5, 4, 12,27, 11,43, 10,35,  9,73,  2,97,  3,21,  7,78,  0,95,  6,76,  1,73,  4,89, 
     0,19,  9,17,  1,55,  8,53,  6,89, 12,40,  5, 3,  7, 3,  4,14,  3,40,  2,69, 13,50, 10,20, 14,41, 11,30, 
     8, 9,  5,75,  7,67,  2, 4,  6, 8, 11,70, 13,38,  4, 1,  9,99,  0,49,  1,43, 14,74, 12,31, 10, 9,  3, 1, 
    10,85, 11,90,  3,94,  8,15,  2,73,  7,50, 14,81,  4,26,  1,91,  5,10,  0,58,  6,32,  9,78, 13, 1, 12, 7, 
    11,58,  0,95,  5,31,  8,99,  9,89,  4,92,  1,18,  6,42,  2,95,  3, 7, 14,81, 10,47, 13,53, 12,94,  7,52, 
    11,13,  0,57,  8,45,  2,71,  1,16,  6,90, 13,25,  9,52, 14,72, 12,11, 10,21,  4,27,  5,34,  3,89,  7,27, 
     2,40, 12,44,  7,17,  4,94, 13,39,  8,49,  0,52,  9,17,  5,79, 11,44,  1,70,  3, 1, 10,67,  6, 1, 14, 8, 
     8,98,  3,38,  6,53,  1,98, 12,41,  7,63, 11, 2,  4,68,  9,28,  2,75, 10,14, 14,21, 13, 8,  5,65,  0, 1, 
     2, 5,  6,40,  7,27, 13,93,  8,65,  3,93,  1, 7, 10,39, 14,42, 11, 8,  9,29,  5,27, 12,57,  4,17,  0,77, 
    13,39,  5,18,  2,17, 12,93,  1, 8,  6,57,  4,66,  0,76,  3,28,  8,21,  7, 2,  9,82, 11,66, 10,77, 14,94, 
     0,75, 13,13,  4,48, 10,13, 14,39,  7,16,  8,69,  1,39,  3,35,  9,75,  6,85,  5,55, 11,45, 12,55,  2,85, 
     4,85, 13,82, 10,13,  5,89,  8,34,  2,10,  1,95,  3,38,  6, 4, 14,10, 12,84,  7,88, 11,65,  9,25,  0,50, 
     9,75, 13,74,  3,84,  5,71, 12,54, 14,53,  6,86,  1,11,  4,72, 10,27,  2,89,  7,60,  8,25, 11,17,  0,86, 
    12,16,  6,66, 11,24,  4, 6,  1,81,  9,26,  8,68,  5,41,  0, 2, 13,87,  2,86, 14,71, 10,32,  3,34,  7,69, 
     3,22, 10,16,  4,45, 13,56,  9, 8,  7,62, 14,20, 12,84, 11,87,  8,59,  2,53,  1,81,  0,43,  5, 3,  6,22, 
    13,28, 12,23,  1,66,  2,27,  8,49,  0,46,  7,49,  4,64, 10,43,  6,12,  9,95,  5,38,  3,31, 11,49, 14,18, 
    13,20,  8,22,  2,90,  7,84, 11,14, 12,36, 10, 5,  9,80,  6,99,  4,93,  1,67,  0,58,  3,37,  5,13, 14,55, 
     6,98, 11,88,  3,38,  4,95,  8,87,  2,69, 10,99, 13,85,  7,75,  5,39,  9,77,  1,57, 14,82,  0,96, 12,52, 
     8, 9,  2,89,  7,82, 13,81,  9,16,  3,40, 14,74, 12,27,  0,33,  1,33, 10,15,  6,78,  5,58, 11,79,  4,28, 
     6,64,  4,92,  8, 8,  9,48,  2,75,  5,29, 13,69,  1,33, 12,83, 11,19, 14,97,  0,74,  7,98, 10, 3,  3,39, 
    13,68,  9,87, 12,88,  3,72, 14,84,  5,26,  8,11,  7,95, 10,94,  4,90, 11, 8,  0,17,  1,53,  2,92,  6, 6, 
     7,97,  3,96, 10,29,  1,68,  8,96,  5,69,  0,95, 11,90,  9,67, 13,53,  2,23,  4,18, 14,54,  6,49, 12,18, 
    11,78, 13,44,  8,69,  1,25, 12,48,  4,77,  2, 1, 10,70,  6,14,  9,25,  3,95, 14,25,  0,53,  7,64,  5,39, 
     1, 3, 11,80, 10,70,  7,59,  5,43,  3,54, 14,54,  2,59, 13,29,  6,62,  4,89,  9, 3,  0,59, 12,79,  8, 8, 
    14,38, 11,15,  1,18,  8,76,  5,21, 13,82,  0,84,  9,53,  4,18, 12,74,  6,59, 10,61,  3,11,  2,58,  7,10, 
    10,22,  5, 7,  7,89,  2,39, 11,44,  4,41,  3,41, 14,83,  9,54,  0,87, 12,86,  6, 7,  1,70, 13,55,  8,73, 
    14, 5, 12,59, 10,22,  6,59,  4,68, 13,99,  8,41,  7,27,  2,62, 11,80,  5,30,  3,37,  1,60,  0,76,  9, 4, 
    12,24,  2,95, 10,67,  3,85,  4, 8,  0,15, 13,73, 14,52,  7,12,  5,39,  6,16,  8,62,  1,61, 11, 9,  9,53
  };
  // Taillard 50x15 instance 7
  const int tai_50_15_7[] = {
    50, 15, // Number of jobs and machines
    11,86,  1,13,  2,65,  8,20,  5,76, 13,82, 10,42, 14,10,  4,50, 12,29,  9,30,  6,53,  3,52,  7,19,  0,24, 
     1,19,  0,76,  3,77,  9,73,  5,76,  4,87, 10,61, 13,51,  2,61,  6,99,  7,38, 12,98,  8,34, 11,34, 14,44, 
     9,29,  7,80,  4,63,  0,15,  2,75,  1,27,  8,98, 10,38,  3,57,  5,80, 12,56, 14,54, 11,55,  6,35, 13,15, 
     0,72, 12,73,  3,67, 11,58, 10,14,  2,59, 13, 6,  7,70, 14,88,  6,66,  8,65,  4,96,  9,95,  5,56,  1,16, 
     3,21,  1,78,  9, 3,  5,76, 10,10,  8,46,  2,98,  0,12, 13,55,  7,92,  4,42,  6,68, 14,67, 11,75, 12,97, 
     5,30,  4,20, 10,80,  8,16,  3,76, 13,45,  1,97,  9,70,  6,54,  2,38, 11,76, 14,84,  0,55, 12,93,  7,67, 
    11,81,  3,53,  9,37,  2,84,  4,33, 13,69,  7,52,  6,11, 10, 2,  0, 7, 14,79, 12,36,  8,78,  1,92,  5,19, 
     7,28,  9,78,  0,62, 12,17, 13,38,  1,66,  3,68, 14, 6,  4,60,  5,93, 11,47,  6,63,  2,26, 10,58,  8,46, 
    14,19, 11, 4,  2,66, 12,51, 13,61,  4,16,  5,72,  9,20,  7,85,  6,95, 10,37,  0,21,  8,94,  1,64,  3, 8, 
    12,45,  1,84, 14,52, 10,50, 11,77,  2,38,  8,60,  3, 2, 13,50,  9,65,  7,71,  0,67,  5,95,  6,71,  4,24, 
     5,32, 14, 6,  2,42, 12,74,  7,56,  3,38,  6,55, 10,84,  1,96, 11,86,  9,47,  4,38,  8, 4, 13,72,  0,91, 
     7,81, 11,49,  1,40,  4,57,  6,16,  2, 7,  8,13,  9,18, 13,96, 10,84,  5,53, 14,70,  0,94, 12,74,  3,89, 
     8,71,  1,66,  3,45, 10,27, 11,79,  7,75,  6,71,  0,88, 13,14,  2,36, 14,88,  4,23, 12,52,  5,74,  9,78, 
    11,46,  6,69, 12,24, 13,20,  4,37,  1,53,  0,82,  9,34,  8,54,  5,47, 10,13,  2,28,  3,78,  7,42, 14,86, 
     8,46,  4,88,  3,37, 13,75,  1,56,  2,77, 12,21,  6, 8,  9,52,  0,53, 14,12,  5,81, 11,72,  7,79, 10,98, 
     8,12,  2,10,  5,98,  3,15,  4,55, 12,48, 14,91,  1,11,  9,28, 10,42,  6,13, 13,85,  0,15,  7,21, 11,24, 
     0,44, 10,46,  4,79,  7,13,  6,48, 14,78, 11,67,  9,72,  2,87,  5,64,  1,21, 13,58, 12,75,  8,86,  3,10, 
     1,32,  8,73,  6,70,  2,30,  9,91,  4,63, 13,33, 11,38, 10,42,  3,82, 14,71,  7,70,  5,78,  0,15, 12,80, 
    12, 6,  5,27, 11,79,  2,59,  7,77,  3,99, 14,27,  1,26,  8,61,  4,11, 13,20,  9,66,  6,96, 10,55,  0,48, 
    14,57,  8,47,  4,84,  5,84,  0,92,  2, 4, 11,62,  9,23,  1,56,  6,99, 13,68, 10, 5,  3,31,  7,83, 12,31, 
     3,24,  4,43, 13,48, 11,79,  7,40, 12,57,  6,90,  9,83, 14, 8,  5,99,  8,29,  1, 8,  0,40, 10,64,  2,57, 
     7,77,  8,53, 14, 1,  5,99,  2,39,  0,81,  1,58,  9,94, 10,41, 12,93,  6,61,  4,24, 13,32,  3,31, 11,48, 
     6,42, 11,39, 14,60,  7,41, 12,40,  9,45,  8,14,  0,27,  1, 8,  2,29,  4,89, 13,92,  3,74, 10,97,  5,16, 
     9,14,  2,28,  3,10, 14, 6,  8,27,  1,57, 10,54,  4,62, 13,57, 11,98,  6,32,  5,32,  7,21, 12,61,  0,66, 
     2, 6, 11,13,  1,33,  9,88,  0,92,  4,20, 12,79,  5,63,  8,29,  6,97,  7,66, 13,59,  3, 2, 10,83, 14,20, 
     1,36, 11,35,  8,70,  0,34, 12,60,  3,63, 10,90, 14,94,  7,56, 13,27,  2,49,  4,93,  6,27,  5,39,  9,44, 
     5,19,  3,13, 11,54, 12,69, 14,56,  2,32, 10,80,  8,30, 13,49,  6,74,  9,79,  0,25,  4,69,  1, 9,  7,51, 
     7,37,  8,92,  2,59,  5,11, 13,41,  4,68,  9, 3,  6, 6, 14, 3, 10,54, 12,98,  0,82,  3,21, 11,61,  1,95, 
     2,79,  0,15, 11,44,  5,91,  7,93, 12,38, 10,90, 13,21,  9,42,  6,40,  1,15,  8,24,  4,97,  3,34, 14,27, 
     7,74, 11,69,  9,81,  0, 7,  6,71,  8, 6,  4,32, 12,15,  2,28,  3, 6, 14,53,  5,73,  1,65, 13,29, 10,37, 
    14,45,  2,87,  8,27, 10,76,  7,64,  4,35,  3, 4,  1,57,  9,43,  0,98, 11,62,  5,49, 13,44, 12,75,  6,38, 
     6,93,  8,78,  7,92, 11,44,  4,20,  2,83,  3,51,  1,68, 10,91, 13, 7,  9,97,  5,69,  0,97, 14,94, 12,58, 
    13,80,  2,80,  1,22, 14,51,  6,71,  0,25, 12,13, 11, 7,  4,88, 10,26,  5,83,  7,73,  3,73,  9,39,  8,58, 
     0,77,  3,19, 14, 9,  7,60,  9,19,  2,87, 10,60,  4,48, 11,86,  6,50,  8, 7,  1,19,  5,14, 12,52, 13,97, 
     4, 4,  3,86, 11,56, 13,86,  7,49,  5,16,  0,46,  8,93,  6,87, 12,39,  2,22,  1, 1, 14,71, 10, 4,  9,84, 
     7,39, 13,84,  4,98,  8,95, 12,22, 14,48, 10,28, 11,27,  0,21,  5,55,  2,80,  9,10,  1,89,  3,87,  6,76, 
    12,69, 10,80,  4,59,  1,98,  8,76,  5,12,  9, 4,  2,58, 11,24,  0,86, 14,45,  6,89,  3,17,  7,30, 13,81, 
     0,22,  5, 5,  4,28, 14,18, 13,46,  1,88,  2,10, 10,90,  9,80,  8,53, 12,41,  6,98, 11,28,  7,12,  3,25, 
     2,93, 11,19,  9,83,  1,58,  7,61, 14, 7,  0,88,  5,17, 12,81,  3,69,  6,76, 13,12,  4,71,  8,61, 10,28, 
    12,21,  3,20, 10,89,  6,38,  9,91,  0,49,  2,42, 11,26,  7,89, 14,80,  8,10,  4,15,  1,49, 13,45,  5,59, 
     5,86, 10, 2, 12,20,  0,17,  8,48,  7,46,  6, 6,  9,43, 11,16,  1,51,  2,74,  4,81, 14,74,  3,64, 13,15, 
     6,47, 13,98,  7,32,  9,35,  3,81,  1,96, 14,42,  4,15, 11,35, 12,92,  2,55, 10,98,  5,61,  8,74,  0,30, 
     4,44,  8, 8,  9,53,  7,45,  6,71,  2,65,  5,87, 13, 4,  3,35, 11, 9, 10,30,  0,56, 14,67, 12,68,  1,90, 
     1,62,  7,31,  8,14,  2,43, 12,21,  9,58, 11,82,  4,85, 14,88, 10,33,  3,39,  6,70,  5,63,  0,82, 13,57, 
     7,71,  4,99,  8,78, 10,83,  3,88, 13, 9,  2,50, 12,38,  0,76,  1,85,  9,97, 14,19,  5,68,  6,51, 11,25, 
     2, 3, 14,57, 12,75,  7,95,  4, 6, 13,31,  5,79,  9,86,  0,95,  3,87,  1,66,  8,35, 10,68,  6,17, 11,18, 
     7,68,  2,71, 11,84, 13,64,  8,53, 10,67,  4,44, 12, 1,  9,63,  0,27,  3,10,  5,21, 14,50,  1,13,  6,76, 
     9,56, 14,77,  1,40,  2,82, 12,75, 11,92,  4,17,  7,72,  5,10, 10,12,  0,48,  3, 5,  6, 3, 13,13,  8,33, 
    12,25,  0,86, 10,32,  4,31,  8,14,  9,58,  7,31, 11,62,  3,41,  2,55,  5,44,  1,13, 14,53,  6,33, 13,63, 
    11, 8,  3,95,  5,44, 10,38,  1, 6,  6,95, 14,87, 13,47, 12,42,  0,72,  4,93,  2,92,  8,38,  9,98,  7,55
  };
  // Taillard 50x15 instance 8
  const int tai_50_15_8[] = {
    50, 15, // Number of jobs and machines
    10,46,  4,43,  0,25,  6,99,  2,90,  3,21, 12,27,  1,17,  5,16, 11,88,  8,64,  7, 9, 14,50, 13,55,  9,22, 
    14, 9,  5,39,  2,58,  6,16,  3,98,  7,58,  9,81,  8,51,  0,10,  1,31, 13,49, 10,65, 12,48, 11,62,  4,51, 
    11,54, 14,46,  2,96,  5,46,  7,16, 13,17, 12,72,  0,51,  3,33,  9,91, 10,18,  4,84,  6,87,  1,31,  8,51, 
     6, 3,  3,94,  4,91,  9, 2,  2,50,  0,89, 12,78,  7, 5,  5,30, 11,10, 10,22, 13,76,  1,50,  8,45, 14,28, 
     1, 5, 10,53,  0,57,  9,14,  5,90,  4, 8,  7,52, 12,76, 11,59,  3,15, 13,39, 14,40,  6,54,  8,57,  2,52, 
     2,31,  0, 2,  8,56,  4,64, 12,55,  9,96,  6, 6, 14,36, 13,29,  5,57,  3,86,  1,69,  7,54, 10,76, 11,89, 
     0,39, 11, 7,  2,32, 13,74,  8,90,  4,66, 10,76, 12,53, 14,46,  7,27,  1,83,  6,49,  3,72,  5,22,  9,53, 
     3,83,  2,18,  4,86, 10,89,  6,93, 12,63,  5,34, 13,97,  8,84,  9,61, 14,32, 11,48,  0,23,  1,81,  7,61, 
    12,32, 10,11,  1,18,  4,54,  7,96,  9,67,  6,73,  2,61,  3,15, 11,67, 14,34,  5,37, 13,65,  0,44,  8,32, 
    12,79,  5,23, 14,51,  7,60, 10, 9,  9,54,  6,85,  4,88,  3,83,  8,55, 11,87,  2,93, 13,80,  0,72,  1, 5, 
    10,54,  5,54, 14,54,  3,59,  2,49, 13,68,  7,56,  4, 9,  9,23, 12,58, 11,88,  6,82,  8,10,  0,87,  1,54, 
     5,72,  0,84, 14,29, 10,59,  9,60,  6,98, 11,41, 12,87,  8,27,  2,31,  1,79, 13,69,  7,64,  3,86,  4,77, 
     1,71, 12,61,  3,33,  5,55,  0,83,  8, 9, 11,87,  9,19,  2,49,  4,68, 13, 4, 14,24,  7,41,  6,49, 10,77, 
     8,36, 11,23, 10,36,  6,41,  5,56,  0,68,  4,81,  1,49, 14,45,  7,67, 12,89,  2,60,  3, 1, 13,58,  9,28, 
     6,43,  7,76,  1,42,  2,59, 10,50, 14, 3, 12,26,  8,41,  9,66,  0,52, 13, 8,  4,33,  3,40,  5,39, 11,50, 
     1,65,  8,30, 10,49,  4,14,  6,64,  7,34, 13,35, 14,66, 12,16,  5,45,  3,36,  0,80,  2, 5,  9, 2, 11,63, 
     1,53,  0, 7,  8,34,  3,53, 12,43,  2,85,  6, 9,  4,64, 14,92, 11,65, 10,15, 13, 1,  7, 6,  9,95,  5,82, 
     2,11,  7, 3, 14,33,  4,62,  9,70, 10, 6,  3, 3,  5,35,  6,57, 13,76, 12,38, 11,28,  8,78,  1, 7,  0,17, 
     6,40,  4,55,  1,21, 12,11,  0,58, 14, 8,  3,38,  8,19,  2,25,  7,55, 13,39, 10,92, 11,30,  5,94,  9,39, 
     8,34,  0,18, 14,99,  3,28,  7, 2,  6,41,  1,35, 13, 6,  5,70, 11, 4, 12,95,  2,59,  9, 7,  4,88, 10,71, 
    11,39, 13,48,  7,27, 14,27,  1,23, 12,80,  4,35,  5,67,  3,22,  9,85,  8,70, 10,36,  0,43,  2,80,  6,60, 
    12,15,  4,22,  1,93, 13,85,  6, 9,  0,38,  5,98,  8,25,  3,64, 10,45, 11,96, 14,36,  7,37,  2,46,  9,89, 
     2,23,  9,20, 10,46,  7,91, 11,45,  4,67,  8, 6,  0,69,  5,82,  3,76,  6, 5, 12,71, 13,82, 14,70,  1,72, 
     5,36,  9,93,  3,23,  1,13,  2,80, 12,44,  0,95,  7,81, 14,44,  4,44, 10,85, 11,58,  6,62, 13,18,  8,94, 
     5,60, 12,68,  4,84, 11,35,  9,92, 14,62,  8,94,  1,89,  3, 1,  2,48, 10,36,  0,35,  7,28,  6,37, 13,41, 
    14,62, 10,35,  2,62,  3,15,  0, 8,  9,18, 11,21, 12,28,  7,72,  6,65,  1,82,  5,16, 13,40,  8,93,  4,41, 
    14,12,  2,14,  9,53,  1,20,  7,99,  5,30,  8,48,  3, 9,  4,51, 12,12,  0,60,  6,51, 11,80, 13,81, 10, 9, 
     8,61, 14,62, 11,33,  7,61,  6,73,  9, 3, 12, 1, 10,19,  0,80,  4,40,  1,16,  5,23,  3, 7, 13,82,  2, 4, 
     2,12, 12,76,  4,95, 13,63, 14,52,  6,60,  9,86,  3,67,  5,26,  1,25,  0,85,  8,12,  7,86, 11,92, 10,73, 
    13,57,  3,56,  1,47,  6,98, 10,55, 11, 3,  9,29, 14,33,  0,24,  7,92,  5,51, 12,66,  4,38,  2,19,  8,59, 
     3,12,  1,54, 11,73,  6,33, 12,23,  7,75, 10,69, 14,93,  5,64, 13,46,  8,44,  0,81,  2, 1,  4,78,  9,98, 
     0,22,  2,95,  4,28, 13, 8,  5,59, 10,63,  3,87, 11,84,  7,22,  1,43, 14,85,  9,99, 12, 9,  6,11,  8,79, 
     9, 2, 10,20, 12,49,  5,32,  6,49, 14,17,  2,71,  0,79,  7,25,  3,78,  4,33,  1,20, 13,84, 11,60,  8,67, 
     4,12,  8,45,  0,81, 11,29,  2,41,  9,87,  7,57, 12,68,  1,79, 10,97,  3,16,  5,61, 13,63, 14,25,  6,51, 
     2,41, 14,33,  1,80, 11,76,  0,74,  6, 3,  4,55,  3,32, 10,20,  5,77, 12,60,  7,62,  9,70,  8,68, 13,91, 
     2,96,  6,13,  9,73,  4,16,  1, 6,  0,23, 14,88,  7,72,  3,37,  8,33, 13,98, 10,14,  5,81, 12,84, 11,95, 
     7,84,  3,47,  8,17, 13,38,  9,11, 10,33, 11,49,  4, 9,  5,51,  1,26, 14,99,  2,18,  6,41, 12,28,  0, 7, 
     9, 1,  7,14,  3,34,  6,49,  4,11,  8,78,  1, 8, 11, 8, 14,68, 13,38, 12,72,  5,70,  0,32,  2,81, 10,86, 
     2,93, 10,34,  0, 6,  5,99,  1, 4,  3,29, 14,24,  4,84, 12,53, 13,17,  8,50,  7,53,  6,24, 11,59,  9,62, 
     4,67, 14,75, 13,89,  0,82,  5,39,  2,82, 10,35,  7,58,  9,63,  1, 4, 11,64, 12, 8,  3,30,  8,53,  6,74, 
    14,44,  3,17,  1,26, 13,73,  5,34,  6,38,  8,45,  4,71,  0,16,  9,96,  2,86, 10,30, 12,46,  7,26, 11, 7, 
     1,45,  9,16,  5,96, 12,68,  7,48,  0,30, 11,79,  3,90, 14,84,  6,48, 10,79,  2,14,  4,42,  8,82, 13,26, 
     2, 1, 10,62,  1,45, 11, 3,  6, 7, 13,15, 12,22,  7,71,  3,19,  8,87,  5,55, 14,12,  9,50,  4,10,  0,36, 
     7,71, 12, 3, 14,61, 11,34,  9,60,  1,72,  5,34, 13,33,  3,69,  8,36,  4,88,  6, 1,  2, 3, 10,98,  0,90, 
     4,84,  0,50,  2,74,  5,16, 11,86, 13,32, 10, 2, 14,22,  6,22,  8,73,  9,16,  7, 8,  3,64, 12,70,  1,83, 
     9,57,  7,22,  6,43, 10, 2, 13,10,  1,37,  8,46,  5,89, 11,31, 14,27, 12,47,  0,85,  3,86,  4,81,  2,38, 
     7,39,  5,14,  9,64, 14,87,  4,34,  2,33,  1,37,  0,78, 12,84, 13,27,  6,46, 10,93,  8,75, 11,70,  3, 9, 
    11,25,  1,84,  0,15,  4,59,  9,85,  2,53,  3,29, 10,70,  5,50,  6,93, 13,23, 14,98, 12, 2,  8,17,  7,87, 
     7,11,  8, 7,  4,70, 14,19,  5,13,  2,23, 10,94,  0, 2, 12,55,  6,93, 13,77,  9,92, 11,39,  1,33,  3,75, 
    11,29,  6,60,  9,27, 13,57,  7,79,  2,67,  0,66,  3,22, 12,27,  4,20, 14, 5,  5,43, 10,79,  1, 9,  8,85
  };
  // Taillard 50x15 instance 9
  const int tai_50_15_9[] = {
    50, 15, // Number of jobs and machines
     2,76, 10,34, 12,28,  8, 8,  9,10,  0,74, 14,92, 13,30,  6,10,  7,82,  4,45, 11,74,  3,24,  5,87,  1,38, 
     3,90,  1,55, 10,16, 12,77,  0,86,  2,83,  9,26,  8,92, 13,23, 11,50,  6,74, 14,81,  5,15,  4,14,  7,31, 
     8,35,  0,56,  2,32,  1,64,  3,70,  7,78, 13,23,  6,46,  4, 8, 10,75, 12,45, 11, 5,  5, 7,  9,62, 14,73, 
    10,74,  4,13,  1,78,  6,88, 12,32,  3,26,  5, 8, 11, 2, 14,27,  2,29, 13,62,  9,48,  8,25,  0,78,  7, 8, 
     2,95,  1,68, 14,65,  8,93, 11,68, 13,75, 12,54,  6, 2,  5,60,  3,99,  7,13,  4, 3,  9,66,  0,57, 10,60, 
    11,43, 12,84,  9, 7,  7,70, 14,43,  6,37,  2,42,  1,92,  4,70,  0,91,  3,30,  8, 5, 10,57, 13,70,  5,80, 
     0,83,  8,97, 12,12,  1,19,  5,50,  4,18,  7,41, 14,66, 10,24,  2,98, 11,11,  9,21, 13,29,  3, 6,  6,86, 
     3,86,  7, 4, 13,94,  9,52, 12,22,  8,76, 10,81, 11,47,  4,92,  2,95,  0,15,  5,33,  1,85,  6,38, 14,54, 
    13,80, 12,56,  9,54,  6,87,  3,22, 14,93,  2,36,  8,83, 10,99, 11,27,  4,15,  1, 7,  5,69,  7,77,  0,50, 
     9,43, 10,49,  8,13, 12,27, 13,94, 11,91,  1,79,  5,80,  2,13,  7,81, 14,34,  6,19,  3,67,  0, 3,  4,67, 
     7,63,  3,56,  2,77,  8,17,  6,17, 10,51,  5, 3,  1, 9, 13, 7,  0,63, 12,87, 11,66, 14,92,  4,64,  9,60, 
     8,33, 12,86,  1,35,  2,69, 14,32,  4,86, 10,26,  5,98,  6,95,  9,31,  0,11, 13,82, 11,85,  7,71,  3,61, 
     9,81, 13,86,  3, 1, 14,18,  5,16,  2,94, 11,84,  0,11,  6,18,  8,41,  4,72, 12,15, 10,50,  7,79,  1,77, 
    12,67,  3,41,  5,99,  8,29,  7,62, 13,80,  6,39,  1, 1, 14,21,  0,38, 11,68,  2,88,  4,88, 10,95,  9, 2, 
     6,76,  4,80,  1,42,  5,65,  3, 4,  9,62,  0,50,  7,93, 14,74, 11,76, 10,10,  8,76, 13,55, 12,95,  2,94, 
     1,76,  8,40,  0,96,  9,89, 10,22, 13, 1,  7,22,  5,49,  4,12,  6,27, 14,17, 11,34, 12,48,  3,28,  2,32, 
     7,90, 13,50,  1,33,  9,53,  6,21, 10,35,  2,11,  8,53, 12,44,  4,58, 11,76, 14,32,  5,62,  3,60,  0,23, 
    13,96,  3,14,  2,67,  4,37,  0, 7,  8,23, 12,76, 11,82,  7,51,  6,68, 10,58,  1,66, 14,13,  9,40,  5,43, 
     4,32,  9,95,  1,70,  6,27,  0,79,  5,37, 13,98,  2,86, 12,85,  3,53, 10,25, 14,88,  7,32,  8,31, 11,26, 
     0,18,  3,31,  2,97, 13,94, 12,74, 10,73,  5,40, 14,34,  1,88,  6,52, 11,48,  8,72,  9,50,  4,20,  7, 6, 
     4,85,  1,90, 13,87,  9,57, 10,87,  0,11,  7,96,  8,69, 14,77,  6,95,  3,60, 11,37,  5,87, 12,83,  2,40, 
    12,76,  6, 1, 10,46,  0,31,  5,21,  3,57, 13,69,  1,96, 11,85,  8,40, 14,30,  9, 4,  2,61,  7,44,  4,29, 
     9,33,  6,48,  3,71,  0,27, 12,67,  2,11,  5,23,  7,97,  8,71, 13,23, 14,54,  4,91, 10,55, 11,12,  1,62, 
     0,65,  3,19,  8, 6, 13,45, 11,94, 12, 4, 14,46,  7,52,  9,93,  6,14, 10,49,  1,70,  2, 1,  4, 3,  5,23, 
     2,14,  5, 3,  0,34, 12,13, 13,46,  9,79,  3,82, 11,76, 10,14,  8, 6,  4,27,  7,34,  6,65,  1,51, 14,85, 
    13, 8,  2,41,  4,74,  6, 9, 12,42,  9,98,  7,65,  1,94, 11,24,  3,83,  8,21, 14,75,  5,26, 10,30,  0,67, 
     0,86, 11,96,  8,60,  4, 6,  7,78,  6,87, 13, 5,  1,26,  5,48,  3,81,  9,64, 10,20,  2,44, 14,91, 12,12, 
    10,75,  5,24,  0,27,  9,14,  1,33, 13,17,  3,93,  7, 5,  4, 5,  6,89, 12,53,  8,66, 14,85, 11,51,  2,96, 
     1,31,  8,63,  4,21, 10,37, 13,68,  3,78, 12,48,  0,66, 14,86,  9,18,  6,77, 11,22,  7,31,  2,87,  5,18, 
     8,23, 13,25,  6,22,  3,40,  1, 5,  7, 9,  9,29,  0,51, 12,61, 14,84,  2,50,  4, 4, 10,87,  5,36, 11,31, 
     4,16,  3,81, 12,18,  1,73, 13,26,  0,19,  6,54,  8,44, 14,92,  2, 8,  7, 6,  9,89, 11, 2, 10,46,  5,21, 
     0,57,  8,48,  1,31,  7,57, 14,11,  5,79,  4,68, 10,99,  9,44, 12,71,  6,59,  2,13, 13,10,  3,48, 11,32, 
     3,42,  6,34, 12,93, 11,63,  0,13,  2,38,  5,93,  1,34, 14,66,  8,62, 13,39,  4,68,  9,43, 10,72,  7,37, 
     5,86,  4,11, 14,33,  7,85,  0, 9,  8,33, 13,80,  2,92, 12,59,  9,21,  1,65,  6,19, 11,96, 10,17,  3,33, 
     6,44,  9,43, 12,78, 11,36,  2, 8,  1,12,  8, 4,  5, 8, 14, 2, 10,78,  4,43,  7,27, 13, 9,  3,16,  0,17, 
    10,81,  2,32,  4,38,  0,83, 14,32, 12,74, 13,76,  1, 6,  8,17,  5,28,  9,76,  6,66,  7,19, 11,27,  3,77, 
    11,49,  4,74,  8,35,  6,11,  2,81,  7,72, 14,76, 12,49,  5,67,  3,29,  0,52,  1,33,  9,72, 13,54, 10,19, 
    14,96,  7,73,  2,39, 11,69,  0,42,  4,77,  9,95,  8, 5,  5,37,  6,56, 10,21,  3,65, 13, 5, 12,40,  1, 8, 
     0,11, 14,44, 11,32,  4,43,  2,10,  6, 5,  5,62, 13,15, 12,92,  1,79, 10,30,  7,29,  8,21,  9,58,  3,29, 
     2,22,  5,83, 13,55,  8,95, 11,42, 12,42,  6,62,  4,12,  9,82, 10,52, 14,41,  7,40,  0,86,  1,28,  3,48, 
     1,64,  9,68,  4,14,  5,70,  2,63,  0,33, 13,82,  7,55,  6,17, 11,51,  3,96, 12,27,  8,79, 10,63, 14,28, 
     6,89,  4,80, 11,98,  8,54, 10,75,  7,97,  5,40, 13,62,  3,98,  1,38, 12,70,  2,39,  9,23, 14,12,  0,94, 
    12,21,  2, 8,  6,80,  5, 2, 14,66, 10,33,  8,22,  0,21,  9,70,  7,14,  1,32,  4,70, 11,78, 13,46,  3,38, 
    14,17,  9,22,  5,57,  1,60,  0,68,  3,86,  6,31,  7,16,  2,75, 13,65, 12,46, 10,56, 11,75,  4,99,  8, 6, 
     9, 4,  4,75,  1, 8, 14,35,  7,67,  6,88,  0,40, 10,90, 12, 9,  5,99, 13,93,  8,39, 11,59,  3,90,  2,69, 
     2,34,  5,61,  8,88, 12,54,  0,95, 10,22,  6,47,  9,91,  4,53,  3, 7,  1,94,  7,14, 14,70, 13,40, 11,31, 
    10,28,  3,90,  9, 5, 14,85, 13,83,  5, 6,  2,69,  0, 6,  4,57,  6,87, 12,93,  7,75, 11,70,  8,86,  1,68, 
     7,25, 10,44,  9,54, 14,94,  5,35, 12,62,  8,63, 11,51,  3,59,  2,68, 13,85,  0,48,  6,64,  4,40,  1,75, 
     7,29,  0,42,  2,56,  3,94, 11, 9, 14,31,  5,80, 12,52,  4,53,  9,82,  1, 8, 13,32,  8,94, 10,94,  6,32, 
     9,39,  2,12,  6,34,  8,24,  7,41, 12,85,  0,74,  3,46, 14,30, 13,89, 10, 2, 11,87,  1,57,  5,84,  4,34
  };
  // Taillard 50x20 instance 0
  const int tai_50_20_0[] = {
    50, 20, // Number of jobs and machines
     8,48, 19,40, 17,54, 15,71,  1,52,  2,70, 10,41,  7,76, 13,52, 18,24,  5, 5,  6,43,  3,68,  4,10, 14,49, 11, 9, 16,81,  0,30,  9,93, 12,17, 
    12,85, 11,18,  3,54, 16,42,  1,41,  2,71,  9,68,  8,82, 18,54,  4,49,  0,21, 19, 1, 17,58, 15, 1,  5,69, 10,58, 13,40, 14,59,  6,66,  7,29, 
     8,33,  6,34, 15,77, 14,42,  3,95,  7, 2, 19,71, 18,73,  1,19, 11,25,  4,45, 12,88, 17,19, 13,40,  0,42,  2,17,  9,81, 10,72, 16,70,  5,67, 
    12,51,  9,41,  5,74, 16,97,  0,26,  2, 4, 13,25, 18,12,  6,17, 11,76,  1, 6, 19,79,  8,49, 14,39,  7, 1,  4,27, 15,44, 10,75,  3, 1, 17,18, 
     1,22,  3,99, 13, 7,  4, 7, 18,72,  5,24,  6,19,  9,81,  7,23,  8,72, 15,50,  0,95, 14,31, 10,67, 12,67, 19,22, 16,12, 17,28,  2,68, 11,88, 
    16,52,  3,51,  4,44, 14,38,  1,64,  2,11, 10,62,  9,20,  5,54,  0,15, 18,83, 19,79,  8,55, 17,48, 12,38, 15,37, 13,42,  6,81,  7,89, 11,60, 
    19,82,  4,43, 15,57,  0, 1, 12,89, 13,11,  1,41,  7,50, 18,68,  5, 2,  9, 4,  3,65, 16,20,  8,56, 11,46,  2,36,  6,33, 14,56, 10,13, 17,50, 
     9,45, 10,11,  8,63,  4,59, 14,69,  5,39, 15,44,  7,61, 18,67,  6,72, 13,74, 19,59, 17,16,  2,26, 16,90, 11,66,  0,56, 12,47,  3,95,  1,39, 
     9,92, 13, 2, 15,88,  7,90, 18,45,  2,88, 11,90, 19,94,  1,34,  8, 1,  0,81,  4,64,  3,70, 12,55,  6, 7, 16,33, 14,21, 17,35, 10,62,  5,61, 
     6,89, 11,21, 17,61,  8,18, 16,77,  7,20,  0,42, 14,59,  4,79, 15,12,  2,56, 13,14, 18,21,  1,43,  9,89, 19,31, 10,71,  5,92,  3,47, 12,71, 
    13,61,  4,84,  7, 3,  6,73, 15,35, 11,36, 10,79,  9,88,  3,54, 17,96, 16,22,  5,70, 12,10,  1, 4,  8,76, 18,40, 14,85,  0,84, 19,93,  2,65, 
     3,68, 10,72, 14,74,  0,97,  7,63, 12,33,  9,96, 19, 4, 11,63,  5,31, 18, 1,  1,98, 13,39, 17,65,  2,72,  6,20,  4, 7, 16,63, 15,33,  8,26, 
     0,41, 10,65,  2,34, 18,71, 14,19, 12,49, 13,87,  1,61,  3,79,  9,61, 17,29, 15,22, 19,74,  4,68,  5,60,  8,23, 16,82,  6,33,  7,94, 11,42, 
     6,17, 11,40,  9,40,  8,28, 15, 6, 17,62,  0,83,  4,95, 14,44, 10,91, 12,79,  2,39, 13,68, 18,79,  5, 1,  1,20, 16,96, 19,62,  3,62,  7,70, 
     1,39,  9,89, 16,37, 17, 7, 15,84,  2,60,  5,61, 10,73,  8,64, 14,73,  7, 3, 11,75, 13, 3,  0,48, 19,74,  3,67,  4,39, 18,32, 12,69,  6,25, 
    11, 9, 17,83,  5,30,  1, 3, 12,31, 18,93, 10,86,  9,49,  2,34, 15,91, 16,56,  6,80,  7,33,  8,77, 14,35,  4,63,  0,72, 19,46, 13,22,  3,73, 
     3,21,  5,46, 13,33, 16,54, 14,22, 11,64, 17,20, 12,76,  7,77, 18,97, 15,28,  8,54,  2,81,  0,95,  9,81, 10,72,  4,80,  6,75, 19,18,  1,81, 
    16,52, 13,30,  8,38, 17,70,  5,22, 10,15, 18,66, 12,26,  1,55,  4,34, 14,13,  9,65,  6,87, 11,38,  3,85,  7,89, 15,77,  2,22,  0,67, 19,44, 
     4,63, 14,95, 15,18, 19,94, 17,73, 18,51,  1,35,  9,57,  8,38,  7,65,  6,69,  5,60,  0,90,  3,68, 16,32, 11,40, 10,11, 12,75, 13,97,  2,51, 
     9,68, 15,37, 13,39,  4,13, 14,76,  8,77,  3, 6,  2, 6,  5,53, 18,41,  7,72, 16,71,  6,46, 11,24, 10,46,  0,50, 17,12, 12,39, 19,92,  1,54, 
    18,93,  2,95, 17, 8, 16,27, 10,53,  8,75, 12, 3, 19,42, 14, 5,  1,24,  6,73,  5,88,  0,57,  7,20, 11,99,  3,39,  4,74, 15,75, 13,44,  9,24, 
     8,83, 17,14, 11,66, 15,96,  3,11,  6,36, 16,20, 13, 5, 12,72,  2,38, 19,79,  9,10, 14,27,  4,27, 18,90,  1, 8,  7,83, 10,10,  5,61,  0,69, 
    11,22, 17,56, 10,54, 19,50, 12,51, 18, 9, 16,15,  5,36, 15,20,  4,79,  1,51, 13,84,  2,40,  7,59,  0,48,  6,27, 14,65,  9,44,  3,40,  8,83, 
    18, 5,  7,75, 15,43,  9,17,  3,10, 19,92, 16,22, 11,36,  5, 7, 13,71, 17,77, 10,70, 12,10,  0,24,  1,78, 14,77,  2,56,  4,42,  6,16,  8,48, 
     4,37, 17,96,  6,81,  2,12, 18,92,  5,86, 12,63,  7,88,  0,28, 14,57, 11,58, 15,23,  1, 4, 19,95, 16,80,  8,12,  3,82,  9,53, 13, 5, 10,75, 
     6,58,  1,59, 15,65,  2,78, 18,68, 11,50,  7,38, 14,97, 10,72,  4,94,  0,59,  3,42,  9, 5, 13,19, 12,27, 19,54, 17,69,  8, 2,  5,56, 16,51, 
    15, 4,  6, 7, 18,36, 17,35,  0,80, 19,95,  1,51, 14,59, 16,93,  9, 5, 13,61,  4, 4,  5,43,  8,30,  2,93, 12,76, 11,42,  3,99,  7,30, 10,46, 
     9,88, 17,75, 10,81,  6,40,  3,61,  0,94, 12,78, 18,24, 16,19,  7,44, 14,96,  4,23, 13,90,  1,94,  8,80, 19,97, 11,24,  5,44,  2,54, 15,52, 
    13, 5,  4,99, 17,60, 14,87,  5,64, 11,36,  8,78,  0,32,  9, 4, 18,18, 12,26, 16,87,  2,74,  6,26, 15,90,  3,45, 10,35,  1,54, 19,27,  7,23, 
     2,93,  4,95, 10,11, 12,14, 17,99,  5,86,  7,41, 11,26, 16,50, 15,74,  3,21,  9, 6,  0,67, 18,87, 19,46,  6,84,  1,11, 13,89,  8,89, 14,66, 
     9,50,  2,71, 15,71,  7, 5, 11,60, 17,29, 14,17,  6,29, 12,98,  3,61, 19,87,  0,58, 10, 6,  1,60, 13,84,  8,92, 16,23,  5,25,  4,23, 18,57, 
    17,75,  4,60, 19,77, 15,48,  0,87,  3,52,  6,98,  2, 8, 12,55, 13,97,  1,55,  8,68, 16,59,  7,90,  5,50, 10,98, 14,57, 18,43,  9,72, 11,35, 
    13,46,  6,22,  4,11, 12,49,  5,34, 10,30, 11,79,  8,72,  2,77, 18,47,  9,55,  3,63, 14,58,  0,89, 17,71,  1,94, 15,95, 16,13, 19,97,  7,46, 
    15,25,  2,98,  3,71,  7,68, 13, 8, 16,72, 12,57, 14,39,  8,83,  4,17,  5,90,  0,31, 17,81, 10, 6,  6,97, 18,98,  9,82, 11,82, 19,52,  1,82, 
    15,42,  8,77, 10,71,  1,19, 18,80, 11,31,  0,66, 17,90,  4,18, 12,15,  3,76, 19,58,  5,92,  2,34, 14,66,  9, 8, 16,65,  7,67,  6,84, 13,42, 
     4,41, 12,42,  3,69, 18,81, 10,95, 19,16, 17,45,  6,52,  2,48,  1,35,  0,72,  5,80,  7,81,  8, 4, 11, 3, 14, 4,  9,96, 16,53, 13,14, 15,80, 
    16, 6, 17, 6,  9,12, 19,86,  7,26,  1,52,  6,70,  8,93,  3,81, 13,31, 15,89, 12,99, 11,99,  2,71, 14,74,  5, 7, 18,43,  4,86,  0, 1, 10,93, 
    12,44,  2,54,  0,36, 13,40,  4,68, 16,49,  7,45,  3,58, 18,44, 15,65,  5,72,  9,65,  6,53, 10,48, 11,90,  1,98, 19,60, 14,71, 17,27,  8,48, 
    15, 9,  1,16,  2,56, 16,27, 12,50, 19,57,  0,55,  8,87,  9,44,  5,47,  7,29, 11,82, 18,80,  4,43, 10,75,  3,10, 13,70, 14,38,  6,28, 17, 2, 
     7,29,  0,91, 12,85, 10,51,  1,86, 18,34,  8,73,  5,12, 19,14,  3,51, 11, 1, 15,38,  2,74, 16,92, 13,60,  6,43, 17,36,  4,23,  9,82, 14,30, 
    11,21,  2,97, 10, 4, 17,85,  7,21,  4,55,  6,34, 19,62, 16,78,  8,11,  9,34,  0,17,  1, 3,  5,43, 12,38, 15,44, 13,45, 18,17, 14, 3,  3,83, 
    15,29,  8, 6,  7,45,  5,15, 19,60, 17,29, 18,97, 14,91,  6,13, 16, 8,  3,50,  2,46, 10,72,  9,86, 13, 7, 12,30,  0,28, 11,13,  4,27,  1,42, 
     4,38,  3,10, 12,93, 19, 6,  7,72, 11,38, 15,73,  0,88, 14,44,  9,66, 10,79, 18,47, 13,61,  5, 6,  2,64,  6,18,  8, 2, 17, 6, 16,91,  1,37, 
     2,21,  7,20,  4,51, 18,96, 17,51,  1,42,  5,52,  8,37, 13,85,  9,18, 15,44,  3,60, 12,68,  6, 3, 11, 6, 16,20,  0,81, 19,96, 14,30, 10, 9, 
    13,16, 14,54,  1,53,  2,57, 16,46,  0,84, 11, 1, 19,76,  6,26,  7, 7,  5,69, 10,88, 15,29, 12,73, 17,32,  9,51,  4, 4, 18,74,  3,75,  8,75, 
     0,27, 16,54,  3,90,  7,25,  2,97, 17,68, 19,14,  4,54, 12,29,  8,14,  5, 8,  6, 1, 14,60,  9,13, 18,16, 10,41, 13,81,  1,35, 11,18, 15,79, 
     6,56, 13, 7, 12,31,  9,55,  2,85, 19,35, 14,82, 18,63,  4,35,  5,54, 10,52,  7,77, 15,82,  8,94,  3,81,  1,25,  0,24, 11,56, 16,23, 17,79, 
     8,33, 14,50,  7,22, 10,70,  9,59,  6,51,  0,80,  2,84, 16,47, 12,88, 13,27,  5,18, 19,34, 17,47,  4, 4, 15,41,  3,56, 11,42, 18,26,  1,66, 
     3,31, 10,83,  7, 9,  0,34,  1,62, 13,83, 18,61, 17,41,  8,58, 14,96, 16,87, 15,18, 11,56, 19, 2,  5,95, 12,21,  4,51,  9,13,  6,31,  2,96, 
    15,62,  5,95, 18, 8, 19, 3, 17,27,  9,19, 12,36, 14,97,  1,87, 11,62,  2,86,  7,21, 10,37,  0,11,  4,11,  6,67, 16,84,  8,34,  3,48, 13,97
  };
  // Taillard 50x20 instance 1
  const int tai_50_20_1[] = {
    50, 20, // Number of jobs and machines
    18,54, 12,96, 17,96,  4,62,  5,71, 11,42, 10,62,  8,93,  9,98, 13,10, 15,54,  1,13, 16,37,  7,61, 14,92, 19,90,  0,59,  6,64,  3,96,  2,64, 
    13,84,  7,93,  2,22,  6, 7,  5,83,  8,56, 12,94,  0,52,  9, 6, 15,85,  1,38, 19,82, 18, 3, 10,94,  4,32, 11,17, 14,66, 16,79, 17, 9,  3,34, 
    10,67,  7,69,  1, 5, 15,14, 18,53,  2,12, 14,17,  9,93, 11,28,  5,57, 19, 9,  3,78,  8,19, 13,62,  6,50, 16,85, 17, 3,  4,57, 12,95,  0,75, 
    10,99, 12,16, 18,84,  2,51,  9,84, 13,18,  8,21,  5,54,  4, 8, 19,38,  3,80,  6, 4,  0,81, 14,26, 16,48, 15,51,  1,93,  7,76, 11,32, 17,50, 
     2, 9,  7, 5, 14, 2, 19,71,  1,60,  5,98,  0, 3, 15,26,  6,60,  4,57, 16,55, 17,47,  8,45,  3,47, 11,79, 13,58, 18,99, 10,90, 12,21,  9,78, 
     6,90, 19,92, 17,96, 10,44,  3,88, 15,79,  2,78, 13, 3, 11,17,  1,11, 18,34,  0,15,  8,46, 16,47,  9,90,  5,58, 12,95,  7,33, 14,57,  4,91, 
     0,25,  2,99,  4,17, 10,64,  3,83,  7,62, 13,46, 15,21, 12,98, 18,50, 17,56, 14,57, 11, 4,  8,66, 19,55, 16, 3,  1,14,  6,10,  9,24,  5,93, 
    16,94,  0,18, 10,70,  6,92,  4,34,  7,29,  9,65, 13,19, 19,85,  5,55,  2,44, 17,87, 12,59, 18,34, 11,86,  3,72,  8,28, 14,62,  1,22, 15,35, 
    19,88,  8,10,  4,40,  9,96, 18,46,  7,48, 10,74,  6,82, 11,91,  1,45,  2,85, 15,53, 16,76,  5,33,  0,67,  3,63, 17,44, 12,82, 14,51, 13,47, 
     1,45, 17,16,  3,63,  5,16, 11,86,  4,74,  9,58, 18,39, 19,50, 16,28, 14,50,  6,68,  8,39,  0, 6, 10,35, 15, 6, 12,13,  7,26,  2,15, 13,56, 
    19,24,  3,83, 18,53,  7,60, 13,67, 16,78, 10,19, 11,51,  0,14,  1,48, 12,57, 15,52,  8,92,  6,49,  9,74,  4,82,  2,32, 14,63, 17,91,  5,68, 
    12, 7,  4,77,  8,97,  1,98, 11,20, 16,56,  5,66, 13,57,  0,54,  9,58, 19,20, 10,30, 15,77, 17,68,  2,63, 18, 6,  6,63, 14,67,  7,23,  3,26, 
    18,76,  6,81,  3,33,  4,35, 14,85,  0,29, 11,10,  5,52, 10,11,  1,21, 12, 1,  9,96, 13,45, 15,31, 17,43,  2,11, 19,47,  8,41, 16,69,  7,53, 
    10, 3,  7,52,  1, 3, 19,85,  9,34,  0,26,  6,75,  3,92, 16,83, 12, 8, 14,79,  4,69,  5,58, 15,67, 17, 1, 18,79, 13,64,  8,49,  2,57, 11, 4, 
     2,30, 15,19,  1,31, 19,44,  9, 2, 10,17, 16,82,  7,14,  3,82, 12,95,  8,25, 18,67, 11,75,  0,41, 14,92,  4, 3, 13, 6, 17,22,  6,34,  5,20, 
     1,16,  9,62,  4,49, 16, 9,  2,45, 11,41,  7,23,  3,43,  0,35,  5,42, 15,88,  8,19, 14, 3, 10, 8,  6,96, 17,27, 13,30, 18,37, 12,19, 19,28, 
    10,19,  2,64,  1,75, 17,20,  8,99,  0,85,  9,56, 16,98, 14,35,  4,70, 12,44,  7,16,  6,25, 11, 6, 15,10,  5,23, 18, 8, 13,98, 19,25,  3,99, 
     8, 1, 11,24, 19,43, 12, 5, 15,31,  9,54, 10, 1, 14,47,  2,67, 18,62, 17,24, 16,36,  7,94,  6,16, 13, 8,  4,56,  1,16,  3,18,  0,49,  5,69, 
    10,97, 19,89, 18,71,  5,87, 14,78,  0,85,  1,18, 11,19, 13,20, 15,96,  9,50, 16,66,  4,53, 12,70,  6,48,  7,20,  3,41,  8,93, 17,91,  2,51, 
     6,90, 10,29,  5,25,  9,68, 11,18,  7,47, 13,82,  1,35, 17,28,  4, 9, 15,45, 12,60, 14,64, 18,32,  2,16, 16,26, 19,47,  0,33,  8,89,  3,39, 
    10,79, 15,62, 12,25,  7,47,  2, 4, 11,14,  8,41,  5, 8, 14,91,  1,70,  9,27, 18,90, 16,73, 19,59,  3,21,  4,67,  6,77, 13,55,  0,54, 17,41, 
    19,39, 13,64,  9,55,  2,81,  6, 6,  7,73, 15,17, 10,44, 11,27,  8,57, 17,15,  4,33,  1,16, 18,38,  3, 1, 14,61,  0,11,  5,56, 12,65, 16,47, 
    12,48,  8,56, 18, 7,  2,34,  1,59, 11,65,  6, 1, 19,65,  7, 8, 16,19,  3,31, 14,57,  4,67,  9,57, 17,94, 13,32,  5,49, 15,31, 10,72,  0,68, 
    18,65,  5,11, 10,82, 14,50,  2,39,  4,56,  9,58, 17,59,  8, 5, 19,33, 12,81,  6,93,  1,46, 11,23,  3,80, 16,24,  0,13, 13,10,  7,36, 15,43, 
    11,60, 19,32, 15,37, 17,13,  3,56, 16, 6,  0,74,  7,83, 10,50,  1,60,  6,12, 13,90, 18,59,  2,32, 14,72,  5,76,  8,87, 12,25,  9,23,  4,64, 
     4,24,  1,78, 12,44,  7, 5, 10,87, 11,60,  9,53, 19,18,  6,91,  2,67, 18,59, 17,81, 14, 8,  0,16,  5,94, 16,94,  8,47, 13,26,  3,73, 15,69, 
    18, 8,  2,74,  8,93,  3, 9, 11,82,  0,87, 14,40, 12, 6,  4,70,  9,83, 17,86,  5,90,  6,77, 10,13, 19,65, 13,21,  7,83, 15,58, 16,95,  1,48, 
    16, 7, 17,93, 13,75,  1,82,  5,34,  7,28, 14, 8, 10,32,  0,63, 19, 5,  6,27,  2,90,  8,32,  9,15, 12,52, 15,63, 18,11,  3,96, 11,25,  4,22, 
     4,73,  1,73, 10,33,  6,73,  8,19,  9,48,  2, 7, 12,74,  7,77, 17,48, 11, 5,  0,47, 14,38, 13,73, 19,83, 16,24,  3,80,  5,80, 15,41, 18,26, 
     9,50, 17,34,  7,95,  8,44,  2,31, 11,80, 14,59, 13,53,  3,24, 15,18, 10,37, 19,72, 18,72,  1,73,  6, 6,  4,86, 16,17, 12,13,  0,89,  5,24, 
     2,95, 12,99, 11,35,  8,55, 18,67, 17,54,  7,15,  6,25,  5, 2,  1,60,  9,35, 13,14, 19,14, 14,37, 16,82, 10,37,  3,47,  0,28,  4,37, 15,21, 
    16,24,  4,23,  1,68, 17,13,  7,98, 11,67, 18,58,  0,81, 14,62,  9,11,  8,99,  6,75, 10,41, 13,23, 15,13, 12,35,  5,73, 19,35,  2,71,  3,28, 
    17,87,  7,39, 14,66,  9,67,  4,91, 13,71,  2,48, 10,17,  8,12, 15,95, 11,86,  6,71,  3,19,  5,49,  0,63, 12,66, 16,69, 18,73, 19,87,  1, 8, 
     6,69,  1, 3, 12,11, 18,76,  7,57, 19,55,  5,36, 10,95, 11,63, 16,78,  0,72,  9,28, 14,53, 15,10, 13,86,  3,13,  2,53,  8,14,  4, 4, 17,52, 
     5,67, 18,59, 19,84, 17,50, 14,91,  4,93,  0,53,  8,56, 10,30,  2,98, 12,53, 16,74,  9,60,  6,46,  7,57,  1,23, 13,27, 11,49, 15,54,  3,47, 
    15,99, 14,82,  9, 3,  8,61, 16,92,  2,67, 18,66, 17,81, 19,62, 12,41,  7,77,  3, 9,  1,56, 11,80,  4,90,  0,68, 10,81,  6,58, 13,86,  5,59, 
    16,76, 15,77, 19,23, 14,29, 18,25,  3,44,  2,15, 10,17,  5,96,  8,64,  7,71, 13,22,  6,87, 17,73,  0,72,  4, 8, 12,67,  1,92, 11,61,  9,97, 
     1,92,  2,47, 12,95, 11,55, 15,78, 19,58,  0,36, 10, 1, 17,23, 16,71, 18,47,  9,76,  6,34,  5,95,  7,25,  4, 4,  3,91,  8,22, 14,51, 13,53, 
     9,57, 19,61,  2,39,  6,22, 11,60,  3,90, 15,54, 16, 2,  4,83, 13,73,  7,36, 12,68,  8,99, 14,68,  0,61, 18,76,  1,88, 10,15, 17,96,  5,80, 
    12, 6, 16,59,  3,99,  5,30,  0,73,  9,56,  2,39,  1, 9, 11,80,  8,87, 10,74, 18, 6,  4,55,  6,39, 14,39,  7,42, 15,78, 13,37, 19,66, 17,37, 
    11,11, 19,83,  3,83, 17,87,  7,34, 18,45, 14,78,  5,13, 16,59,  4,93,  8,16,  6,85, 12,76,  9,54, 13,91, 15,93,  2,13,  1,56,  0,10, 10,99, 
     6,26,  1,45,  8,80, 11,44, 19, 9,  2,28,  5,70, 14,95,  3,40, 12,20,  7,63,  0,44,  9,63,  4,73, 17,59, 18,82, 16,41, 13,87, 15,89, 10,29, 
     7,64, 12,89, 17,62, 19,30,  6,13, 11,64, 15,42, 10,39,  9,89,  3,89,  5,57, 18,62,  8,99, 13,55,  2,40,  1,94,  0,25, 16,34, 14, 4,  4,65, 
    16,58, 11,99, 14,16,  8,46,  4,72,  5,40,  3,48, 15,15,  2,14,  0,15, 17,39,  9,17, 18,52,  6,43, 19, 9,  1,73,  7, 6, 13,65, 10,58, 12,13, 
    16,19,  3,72,  9,80, 17,48, 14,79,  0,71,  6,29,  5,14,  8,19,  4,27, 19,92, 12,97,  2,99,  1,74, 13,69,  7,20, 10,40, 18,41, 15,95, 11,39, 
    11,63,  3,25,  8,74,  9,40,  2,14, 14,98, 17,26,  0,55, 18, 9, 15,92, 10,67,  7,99, 19,48,  6,20,  4,87, 12,55, 16,65,  1,66, 13,10,  5, 4, 
     7, 2,  9,89,  2,99, 11,89, 12,73,  1,12,  6,78,  3,57, 16,38, 13,17, 14,64, 10,80, 17,47, 15,76,  0,99,  8,72, 19,31,  4,52, 18,49,  5,76, 
    13,45, 17,22,  1,11,  3,99,  2,65, 15,96, 12,95,  5,47, 10, 8, 11,57,  6,60, 14,38, 18,71,  0,70,  8,98,  4,57, 16, 4, 19,31,  9,73,  7,56, 
     5,28, 10,26, 16,47, 15,77,  9,89, 11,82, 12,94, 17,41,  7,89,  1,74,  8,95,  4,44,  0,60, 14,88,  3,61,  2,99,  6,36, 13,49, 19,41, 18, 1, 
     1,16,  7,71,  9,49, 13,25,  2,62, 12,98,  6,63,  0,79, 18,29, 19,82, 10,84,  4,76, 17,85, 11,18,  5,78,  3,18,  8,87, 15,65, 14,10, 16,78
  };
  // Taillard 50x20 instance 2
  const int tai_50_20_2[] = {
    50, 20, // Number of jobs and machines
     0,35,  1,73, 11,56, 17,28, 10,81,  7,82,  5, 5, 13,48,  3,36,  4,37, 19, 9,  9, 8, 12, 9, 14,20, 15,78,  2,77, 16,31, 18,44,  8, 9,  6,40, 
     3,41, 16,26,  5,27,  7,31, 10,62, 11,20, 14,17, 18,55, 13,57,  1,21,  8,61,  9,63, 19,17, 12,14,  0,59,  2,91, 15,54,  4,64, 17,21,  6,47, 
     6, 4, 11,92,  7,75,  2,21,  4,21, 12,98, 14,32,  5,41,  0,29, 13,42, 10,71, 18,90, 19,69,  1,87,  8,71, 15,18,  9,41,  3,78, 16,60, 17,85, 
    13,72,  6,21, 19, 8, 16,55,  2,86,  3,17, 10,98,  9,71,  7,18,  1,86,  0,84, 14,88, 12,97,  5,75, 18,70,  4,67,  8,36, 17,11, 15,97, 11,55, 
     8,88,  0,15, 13,29,  7,51, 10,82,  4,66, 16,26,  6,70,  1, 7, 17,87, 15,81,  3,76,  2,44,  9,27, 19,88, 18,34,  5,70, 11,62, 14,44, 12,67, 
     7,26, 12,24,  0,87, 17,48,  3,55, 15,68,  4,54, 13,83, 14,17,  6,50, 16,87, 19,18, 10,83,  1,32, 18,71,  9,72, 11,95,  5,39,  8,37,  2,21, 
    15,80,  7,31,  1,20,  0,51, 18,32,  5,21, 19,12, 10,79, 12, 1,  6,93, 17,80, 14,60,  3,17, 11,78, 13,87,  4,64, 16,47,  2,66,  9,64,  8,47, 
    19,59,  2,10, 16,80, 14,46,  5,62,  6,40, 15,68,  9,26, 17,54, 11,50, 10,61,  3,77, 12,40,  4,94,  0,36,  1,67,  7,59, 13,55,  8,15, 18,71, 
    13,36, 15,72, 12,53, 19,91,  4,65,  0,98,  3,60, 18,60, 10,62,  1,52, 17,39, 14,13,  5,44,  7,69,  6, 3, 11,97,  8,65,  2,16, 16,57,  9,65, 
     5, 8,  0,99,  6,34, 10,43,  1,38,  3,49,  2,86, 11,91, 18,14,  7,87,  4,45, 17,12,  9,45, 15,14, 16,58,  8,27, 13,68, 12, 5, 19,47, 14,61, 
     4,79, 11,51,  9,41,  3,51,  1,41,  2,44,  5,50, 16,17, 10,26,  8, 2,  7,16, 13,25, 18,38, 15,17, 17,89, 14,83,  6,59, 12,45,  0,37, 19,23, 
    11,97, 13,90,  1,92, 10, 3,  0, 2, 12, 9,  3,70, 15,15,  5,38,  9,48, 19,12, 17,10,  4, 9,  8,76, 14,27, 16, 8,  6,44,  7,81, 18, 7,  2,80, 
     7,45, 19,13, 14,50,  3,30, 16,85,  4,81, 11,32, 17,64,  5,11, 15,70, 10, 1,  9,93,  0,54, 18,13,  6,30, 12,17,  1,16,  2, 3, 13, 2,  8,19, 
     9,57,  8,81, 10,19,  2,46,  7,40, 12,39,  5,63,  4,92, 17,60, 14, 8,  3, 7, 16,39, 18, 9,  1,54, 15,68,  6,21, 19,66, 13,52, 11,54,  0,21, 
    14,66,  0,66,  1,48,  8,81,  2,30,  6,81,  3,46, 12,95,  9,22, 16,85,  4,57, 19,83,  5,48, 15,94, 13,11, 18,61, 11,92, 10,49,  7,83, 17,91, 
    16,28,  9,14, 19,65, 12,17,  6,97,  0,57, 17,33,  2,96, 13, 3,  4,32,  7,70, 18,78,  5,30, 11,68,  8,92,  1,48,  3, 9, 14,75, 15,31, 10,75, 
    15,25, 10,32, 11,11, 18,22, 14,72, 17,88,  5,50, 19,19, 12,34,  6,87, 16,80,  1,12,  7,26,  8,78,  3,63,  2, 3,  4,97, 13,27,  9,25,  0,21, 
     6, 1,  9,39, 18,20, 16,62, 12,71,  1,41,  2,70,  7, 8, 19,11, 11,66,  4,75,  0,67,  3,32,  5,74, 10, 8, 14,44, 17,94,  8,61, 15,18, 13,99, 
    18,56,  7,47, 12,11, 17,43, 19,57, 11,27,  5, 3, 15,54,  3,40,  8,86,  0,40,  6,30, 16,55, 13,48,  1,37,  2,14,  9,49,  4,59, 10,15, 14,99, 
    13,72, 15,11, 12,90, 14,18,  5,81, 18,44, 11,79,  6,77,  3,20,  8,86,  2,58, 17,44,  4,32, 16,33,  0,37,  7,92, 10,95,  1,77,  9,97, 19, 8, 
    17, 9,  4,37,  2, 7,  7,38,  1,10,  5, 4, 15,75, 18,54,  0, 2, 11,15, 10,95,  6, 4, 19,37,  8,19, 14,56, 13,44,  9,60,  3,90, 12,46, 16, 7, 
     8,33, 13,58,  1,88,  9,49,  2,50, 10, 3, 14,44,  0, 8,  3,82, 12,72, 16,99, 18,50,  5,57, 17,19,  7,12,  4,84, 19,69, 11,14,  6, 8, 15,10, 
    16,37,  5, 9, 11,91, 12,92, 17,17, 18,68,  8,34, 19,81,  3,26, 10,99, 13,72,  1,15,  4,93,  2,24,  9, 2, 15,73,  6,34,  7,42,  0,12, 14,99, 
    13,78,  1,98, 11, 9,  0,15, 16,97,  3,86,  7,88,  5,22, 14,31, 17,59, 19,70, 18,42, 10,42,  4,65,  6,18, 15,50,  9,28,  2,57, 12,87,  8,57, 
    19,51,  2,34, 14,97,  7,83,  3,15, 17,68,  6,87, 10,78, 15,57,  9,37,  0,61, 16,51,  1,93,  5,35, 18,57,  4,49, 13,42, 11,12, 12,76,  8,17, 
    11,33,  9,65,  5,62, 18,11, 16,36,  1, 4,  2,97, 17,22,  8,76, 12,17, 10,82, 19, 6, 15,96, 14,37,  3,26,  7,89,  0,41,  6,57, 13,23,  4, 9, 
    13, 6, 15,89, 11,61, 14,16, 12,42, 18,20,  1,30,  4,57,  9,66, 10,83, 16, 7, 17,21,  3,96, 19, 7,  7,31,  6,99,  0,14,  2,85,  5,57,  8,15, 
     7,45, 17,79, 16,88,  6,58, 14, 2, 19,13, 18,21,  1, 8,  2,37,  8,71,  5,99,  4,49, 10,57, 11,95,  9,19,  3,73, 15,64, 12,64,  0,55, 13,85, 
    18,51, 15,22, 16,11,  9,82,  6,90,  8,41, 17,88, 19,33,  1,91,  0,99, 11,69,  7, 6, 14,33,  3,25, 12,31, 10, 7,  4,38, 13,46,  5,41,  2, 8, 
     9,20, 12,96,  4,88,  8,49,  6,24,  3,89, 11,24, 14,66,  7,69, 15,42,  1,92,  2,62, 10,48, 13,95, 17,28, 19,43, 16,71,  5, 9, 18,53,  0,31, 
    15,44,  1,99, 17,49, 11,47,  7,60,  3,12, 18,22, 19,49,  4,40, 10,24, 13,51,  2, 2,  5,63,  8,99,  6,75,  0,52, 12,59, 16,16, 14,24,  9,55, 
    14,67, 19,73, 17, 3,  9,72, 18, 8, 11,70,  1,73,  4,59, 16,91,  8,69,  5,46,  7,31,  0, 6, 10,35,  2,37,  3,93, 13,42,  6,89, 15,34, 12,25, 
     9,31, 17,26, 12, 7,  3,67, 11,25,  2,43, 15,23, 10,31,  0,28, 16,57, 14,42,  1,16,  5,10,  8,26, 19,50,  7,69,  4,35,  6,18, 13,18, 18,77, 
     5,95, 16,10,  2, 7,  0,88,  8,78, 12,62, 11,93, 19,23, 15,94,  4,22,  6,85, 14,73,  7,59, 18,12, 17,58,  9,93,  1,47, 10,73, 13,90,  3,18, 
    14,82, 15,93,  3,10,  7,46, 11,13,  0,57, 12,30,  6,20,  1,71,  8,41, 13,35, 19,35,  2,52,  4,90,  5,18, 18,80, 17,29,  9,17, 10,74, 16,90, 
    13,39, 17, 7, 18,15, 14,49, 10,34, 15,50,  5,48, 16,77,  1,26,  3,27,  9,78,  7,38, 11,76, 12,40,  0, 2,  2,40,  4,92,  6,73, 19,86,  8, 6, 
    17,47,  2,28, 19,21, 15,80, 18,46,  5,63,  9,76, 16,20, 13, 5,  0,57,  1, 9,  7,71,  3,34, 14,27, 12,87, 10,24,  6,63,  4, 6,  8,66, 11,65, 
    11,52,  4,25,  5,67, 12,53,  3,97,  2, 8,  6,23, 16,84,  0,74, 10,75,  1,18, 15,53, 19,31, 18,66,  8,49, 13,51,  7,29, 17,52,  9,34, 14,44, 
    19,39,  6,17,  4,48, 17,93,  2,97, 18,79,  3,87, 13,40, 12, 2, 15,97, 14,47,  7,47, 10,45,  8,65,  9,29,  5,96, 11, 8, 16,42,  1,74,  0,18, 
     8,37, 16,83, 15,30, 12,92, 18,87, 17,51,  4,91,  0,39, 11,64, 19,65, 14,48,  6,68,  5,42,  1,10, 13,86,  3,96,  9,98, 10,35,  7,51,  2,48, 
    10,98, 17, 2, 13,60, 11,23,  6,52,  2,84,  9,38, 12, 3, 15, 1,  0,46,  7,44,  5,83, 14,68, 16, 9, 18,32, 19,19,  1,67,  3,10,  8,12,  4,99, 
     0,68,  3,38, 14,90,  1,38,  9,58, 18, 9,  5,88, 11,63,  8, 9,  7,35,  6, 4, 15,13, 16,54, 12,94, 17,89, 10,79, 19,74, 13,31,  2,70,  4,97, 
     8,90, 14,18, 16,91,  4,52, 11,51, 18,84, 12,53, 19, 8,  2,90,  5,87,  9,32, 15,21,  7,67,  0,13,  6,60, 13,75, 17,38,  1,36, 10,19,  3,68, 
     6,46, 15,67,  7,80, 16,62, 14,43,  9,65,  3, 9, 18,31, 11,66, 17,42,  0, 6,  2,43,  1,47, 19, 9, 10,30, 13, 6,  5,73,  4,20, 12, 1,  8,12, 
     9,76, 10,36, 11,91,  6,72,  4,61,  3, 8, 12,78, 13,56,  5,24,  0,20, 18,12, 19,51, 14,61, 17,91,  2,17,  7,13, 16,74, 15,63,  8, 9,  1,84, 
     5,71,  7,11, 17,72, 18,40,  8,73,  1, 9, 12, 7, 16,88, 19,19,  4,15, 10,45,  3,54,  0,86,  9,38, 15, 9, 14, 7,  2,74, 11,80, 13,75,  6,80, 
    13,38, 15,91, 10,72, 18,44, 14,31,  9,89,  7, 4, 19,83,  5,57,  3,63, 16,70,  4, 3, 17,12, 12,44,  8,83,  1, 7,  2,32,  0,36, 11,26,  6,49, 
     7,60, 16,88,  5, 3,  2, 4,  9,56,  8, 1, 15,95,  3,31, 19,54, 18,20, 13,51, 12,88,  4,82, 14,68, 17,69, 11,15,  0,72,  6, 3,  1,59, 10,43, 
    12,92,  9,88,  6,40, 16,47,  3,80,  8,23, 15, 7,  0,98, 11, 2,  5,90, 10,74, 18,23, 17,93, 19,94, 13,41,  7,49, 14, 2,  4, 9,  2,83,  1,84, 
    15,84,  8,68,  5,77,  9,54, 13,45,  1,70,  6,34, 14,64, 18,55, 12,66,  3,16, 19,11, 16, 9,  2,79, 17,54,  7,23,  0,63, 10, 1, 11,91,  4,50
  };
  // Taillard 50x20 instance 3
  const int tai_50_20_3[] = {
    50, 20, // Number of jobs and machines
     9,62,  6, 3,  3,77,  2,45,  7,42,  1,77, 15,42, 17,78,  0,20, 18,81,  4,39, 11,91,  5,13, 13,53, 19,30, 10,95,  8,82, 16,70, 12,37, 14,55, 
     2,56,  6,90, 13,21, 14,43,  8,12, 18,94,  7,81, 11,58,  9,20, 19,83,  1,41, 15,84, 17,16,  5, 6, 12,64, 16,63,  0,16,  4,12, 10,93,  3,39, 
    10,95, 11,15,  4,51, 19,53,  9,67, 12,53, 18,26, 17,40,  2,13, 16,39,  7,59,  3,90, 13,45,  6,36, 15,33,  5,76, 14,13,  0,72,  1,42,  8,56, 
     9,51,  0,82, 13,63,  2,66,  3,21,  7,66,  4,72,  6,35, 16,74, 10,60, 17,92, 15,28,  8,89, 12,58, 19,38, 18,14,  5,89,  1,17, 11,88, 14,14, 
    11,25,  9,38,  5,10,  6,71, 10,80, 19,41, 17,76,  1,92, 15,86, 16,33, 14,42,  4,90,  2,18,  0,17, 12,91, 18,24,  7,96, 13,82,  3,77,  8,83, 
     4,48,  8,21,  9,71,  3,94, 15,64, 19,67, 12,21, 17, 2,  0,58, 13,38,  6,12, 10,11,  5,63, 14,27, 11,92,  2,64, 18, 9,  7,50, 16,55,  1,15, 
     4,33,  2,99, 10,49, 19,66,  1,77, 16,88, 15,42,  8, 8,  0,64,  5, 2,  7,86, 18,72,  9,26, 17,86, 12,51, 14, 1,  3,40, 13,33, 11,74,  6, 6, 
    10,45,  9,83,  2,54,  7,19,  5,70, 11,16,  3,74, 18,27, 12,84, 13,13,  8, 6,  4,97, 19,47, 16,87, 15,31,  6,34, 14,37,  1,76, 17,31,  0,37, 
    16,62,  4,96, 14, 7, 18,84,  6,70, 19,31, 11,35, 15,37,  8,99, 17,64, 12,53,  0,39,  1,67,  9,20,  2,15, 10,53,  3,83,  7,25,  5,65, 13,78, 
    12,96,  6,24,  2,88, 11,61,  3,10,  5,77, 15,46,  7,49, 10,91,  1,41, 18,37,  8,69,  0,37, 19,85,  4,14, 17,34, 16,83,  9,30, 14,37, 13, 4, 
     5,29, 18,44, 12,55, 10,51, 17,49, 14,43,  6, 1,  7,32,  8,99, 16,49,  3,84,  1,53, 13,57,  4,40,  9,10, 11,58, 19,78,  0,27,  2,34, 15,32, 
    16, 5, 14,97,  3,37,  2,63,  7,66, 10,40,  1,95, 13,51, 11, 8, 19,35,  0,63,  9,17,  4,88,  5,15,  6,33, 18,11, 17,10,  8,84, 12,55, 15,28, 
     6,20,  0, 7,  9,23,  5,24, 14,11, 18,38,  3,56,  8,73, 11,22,  2,29,  4,12, 16,86, 12, 1, 19,23, 13, 6, 15,45, 17,70,  7,25,  1, 1, 10,79, 
     1,58, 10,62, 14,55,  7,79,  0,55,  6,27,  5,77, 18,13, 12,53,  2,31, 13,11, 15,81, 16, 7, 11,94,  9,11, 17,84,  4, 5, 19,67,  3,19,  8,24, 
     8,74,  3,26, 19,90, 13,73, 18,28,  5,16, 16,30,  6,69, 11,43,  7,48, 10,67,  2,91,  0, 3, 12, 1, 17,93, 14,52, 15,41,  4,31,  1,54,  9,57, 
     9,58,  6, 1,  4,92,  0,83, 18,99, 14,64,  2, 7, 19,22,  3,29,  5,48, 17,70,  7,69, 13,60,  1,51, 16,59, 11,19,  8,25, 12,67, 10,67, 15,71, 
    15, 5, 10,46,  4,19, 11,67,  0,41, 16, 8,  5,51,  7,11,  2,67, 18,68, 13,46, 19,16,  1,18,  3,12, 12,11, 14,11,  6,67,  9, 2,  8, 5, 17,99, 
    11,47,  6,35,  1,57,  9,69, 15,99,  0,16, 16,91, 10,36,  8,14,  3,58, 13,10,  5,91,  7,64, 17,44, 18,79, 12,66,  2,31,  4,10, 19,56, 14, 7, 
    17,45, 16, 3,  0,57, 14,44,  6,34, 13,27, 18,74,  7,88,  4,32, 19, 4,  1,98,  3,25,  8, 7, 15,73, 12,46, 11,14,  2,66,  9,87,  5,55, 10, 6, 
     5,66,  3,28, 18, 4,  0,69,  7,44,  2,58, 11,86,  6,64,  8,16,  4,19,  1, 9, 14,25, 16,26,  9,64, 12,45, 15,10, 10,95, 17,99, 19,46, 13,79, 
    19,30, 13, 9, 12,37,  2,85, 11,69, 18,22, 17,23, 15,97,  6,50, 16,36, 10,12,  9,10,  7,43,  1, 1,  3,33,  4, 7, 14,15,  8,45,  0,28,  5, 3, 
    14,44,  6,59,  7,92,  1,31, 13,69, 11,53,  2,63,  3,94, 12,74,  4,53,  0,67,  8,24, 16, 9, 17,31,  5,84, 10,50, 19,19, 18,78,  9, 3, 15,39, 
    15,46, 12,11,  5,59,  9,27,  1,79, 13,86, 14,51,  7,47,  4,22,  6,16, 11,24,  0,80,  3, 5, 16,57, 17,79, 19,42, 10,46,  2,99,  8,49, 18,54, 
    14,18,  3,37,  0,18, 18,76, 19,30,  8,88,  4,69,  9,19, 13,29, 11,41, 12,58, 10, 8,  6,37,  1,17,  7,23, 15,94, 17,92,  2, 1, 16,79,  5,34, 
    18,29, 16,85, 10,91,  1,43,  4,65,  2, 6, 19,69,  9, 4,  8,94, 12,72,  0,76,  5,83, 13,21,  6,45, 17,10,  7,84,  3,50, 11,74, 15,39, 14,55, 
     5,67, 17,76, 10,91,  4,69,  0,13,  9,72, 18,98,  8,16, 11,79,  3,38, 16,76, 15,70, 19,30,  7,85, 14,93, 12,39, 13, 3,  1,39,  6,88,  2,87, 
     9,90, 18,78, 19,61,  4,61,  0,34,  8, 4, 17,52, 12,59, 13,20, 10,29, 16, 6, 14,60,  1,93,  6,95,  2,55, 11,12,  3,92,  7,22, 15, 2,  5,91, 
    16,63,  0,85, 12,76,  8,72,  1,40, 13,42, 10, 4,  3,66, 19,63,  6,81,  9, 8, 18,61,  4,33,  2,98,  7,94, 15,98,  5,35, 17,90, 14,59, 11,27, 
     4,95, 12, 5, 14,91,  7, 1, 18,52, 17,87,  0,56,  6,83,  1,54,  9,15,  8,55, 11,10, 19,20,  3,73, 10,59, 13,30,  2,14, 16, 6,  5,48, 15,28, 
     4, 3,  3,93,  8,12,  1,66, 17,19,  9,85, 14,66, 11,96,  5,74, 15,73, 10,21, 16,62, 18,91, 13,97,  2,61,  0,62,  6,95, 19,13,  7,33, 12,62, 
     6,54,  7,68, 11,44, 12, 4,  5,62, 19,86,  9,85, 18,42, 10,55,  1,69,  8,67, 17,71,  3,79,  0,66, 13, 1, 14,12,  4,66,  2,94, 16,41, 15,43, 
    18,23, 15,25, 17,21, 12,15, 16,64, 19, 6, 10,96,  8,68, 14,58,  9,75,  3,66,  1,57,  6,85, 11,56,  2,24,  0,34,  7,64,  5, 1, 13,82,  4,71, 
     3,42, 16,97, 19,65, 13,51, 11, 1,  9,16, 12,74,  4,54,  5,87, 15,38,  2,48, 10,54, 14,44,  8,51,  0,48, 17,74,  6,82,  7,59,  1,46, 18,60, 
     5,60,  2,50, 13,86, 17, 7, 16,10, 12,12, 11,50,  8,67,  9,65,  1,41,  3,47, 10,59, 15,43, 19, 1, 14,97, 18,67,  7,81,  6,34,  4,27,  0,39, 
     2, 7, 14,52,  7,81,  0,58, 11, 4, 18,53,  6,86, 10,32, 16,54,  4,38,  9,71, 19,43, 12,58,  1,56, 15,63, 13,73,  8,54,  3,56, 17,34,  5,35, 
     9,99, 18,31, 17,90, 11,67, 15,73, 16,74,  6,69, 19,29,  1,92,  2,86, 10,90,  5,72, 14, 5, 12,21, 13,11,  4, 3,  3, 3,  8,29,  7,27,  0,39, 
     4,75, 14,71, 18,89,  7,31,  6,39, 13,70,  5, 5,  9,60,  0,13,  3,32, 11,22, 12, 1,  2,56, 17,53,  1,84, 10,47, 15,91, 19,85, 16,14,  8,10, 
     5,56,  8,94, 17,47,  9,81, 10,21, 16,84, 13,98,  7, 5,  2,76,  6, 6, 18,62, 19,40, 15,58,  0,18, 14,97,  4,89, 12,18, 11,48,  1,44,  3,48, 
    18,45,  8,55,  9,13,  3,15, 16,96, 10,19, 11,18,  4, 5, 17,62,  0,76, 12,61,  1,14,  6,22, 15,23,  7, 3, 19,80,  2,92, 14,86, 13,87,  5,23, 
     1,10, 11,70, 15, 5,  7,38, 16,42,  4,64,  3,99,  0,28, 13,30, 10,82,  6,92,  9,64, 14,36,  5,56,  2,11,  8,78, 17, 2, 12,18, 19,32, 18,54, 
    11,88,  6,82, 13,27,  9,53,  7,42,  0,53, 15, 6, 16,80,  8,55,  4,95, 12,83,  5,66, 14,11, 10,69,  1,89,  2,79, 18,50, 17, 7,  3,31, 19,46, 
    13,30, 15, 6,  3,64, 18,33, 14,41,  1,35, 11,92, 16,65,  8,54,  7,68,  9,52,  0,13,  4, 6,  6,36, 12,75, 17,59,  2,41, 10,97, 19,24,  5,77, 
    12,70,  3,46,  9,32,  5,34, 13,67, 19,10,  4,32, 17,32, 18, 5,  6, 4, 14,41, 11,13,  1,24,  8,13,  7,14, 16,85, 15,36,  2,18,  0, 1, 10,23, 
     5,43,  4,33, 17,16,  9,91,  3,87, 16, 5, 10,74,  2,43,  0,81, 13,23, 14,54, 11,83, 12, 5,  6,68,  1,45,  7,85, 18, 7, 15,44,  8,90, 19,97, 
     8,64, 18,98, 16,47, 19,16,  4,76, 10,50, 13,61,  3,62, 14,88, 15,37,  2,89,  5,18,  6,38, 17,10,  1, 1, 11,36,  9,10,  0,41,  7,52, 12,55, 
    14,78,  1,89,  5,76, 17,50, 15,89,  6,68, 12,48, 10, 1,  3,77,  7,99,  9,46, 18,50, 11,81,  4,18, 13,60,  8,65, 16,37,  0, 8, 19,47,  2,31, 
    15,56, 10,65,  9,15,  3,13,  2,40,  8,46, 12,74, 19,34, 17, 1,  0,56, 13, 2,  4,96, 16,12,  6,17,  1,20,  5,19, 14,46,  7,97, 18,75, 11,73, 
    10,11,  1,86,  3,80,  7, 8, 14,72,  0,15,  5, 8, 11,77, 15,24, 12,78,  4,53, 17,29, 16,13,  6,66, 13,64,  9,58, 19,19,  8,18, 18,45,  2, 5, 
    12,97, 17,73, 18,13,  2,34, 11, 5,  9,84, 10,60,  0,22, 15,40,  5, 3, 19, 8,  7,64,  8,23, 16,66,  6, 3, 13,35,  3,61,  1, 7, 14,32,  4,44, 
     0,33,  7,16,  9,52, 18,72, 19,54, 14,67,  6,71,  8,41,  1,55,  4,32, 17,41, 12,27, 10,54, 13,72, 16, 1,  2,14,  5, 5, 11,13,  3,85, 15,20
  };
  // Taillard 50x20 instance 4
  const int tai_50_20_4[] = {
    50, 20, // Number of jobs and machines
     7,32, 15,13, 11,32,  5,51,  9,74, 16,73,  0,48,  8,13,  1, 6,  4,59, 14,33, 18,18, 13,85, 10,13,  3,57, 19,82,  6,71,  2,32, 17,75, 12,50, 
    15, 6,  0,64,  2,34,  5,60, 12,49, 17, 3, 10,59,  4,47, 11,15,  1,77, 18,24,  3,78,  9,71, 14,19, 19,65,  8,88,  6,23, 16, 2,  7,32, 13, 5, 
    15,69, 11,26,  8,30, 17,90,  3,43,  7,17, 12,23, 13,62,  2,17, 16, 4,  4,18, 19,70,  9,19, 14,15,  0,17,  5,84,  1,54, 18,17, 10,55,  6,81, 
     6,23, 16, 7,  1,49,  7,94, 11,75,  5,56, 14,92,  4,58, 17,32,  2,88, 19,39,  3,59, 18,13, 10,14,  9,49,  0,53, 13,18, 15,13,  8,56, 12,35, 
    13,11,  8,59,  2,77,  0,20, 19,60, 11,33,  9,16, 10,13,  7,72,  1,35, 17,36, 12,92,  6,31, 14,92, 16,53,  4,89, 15,70, 18,21,  5,37,  3,12, 
     9,69,  5,55, 14,23, 17,81, 12,43, 11,62,  3,17,  0,95, 13,39, 10,29, 18,21,  1,95,  8,62,  7,41, 16,74, 15,75, 19, 7,  6,99,  2,58,  4, 8, 
    15,11, 12,84,  4,17,  2,87, 14,45,  5,68, 16,10, 18,10, 11,14,  8,86,  7,90, 10,94, 13, 9, 17,76,  0,75, 19,62,  1,61,  6,23,  3,96,  9,99, 
    10,88,  0,84, 18,41,  1,81, 16, 4,  7, 6,  8,72,  3,98, 13,70,  6,27, 15, 9,  9,45, 19,51, 11,84, 14,92, 12,41,  2,20,  4,19,  5,62, 17,29, 
     7,52, 14,25,  0, 6,  6,91, 12,12,  1,79, 11,26, 19,80,  8, 6,  5,16,  3,84, 18,33,  2,13, 10,64, 13,61, 16,41,  4,77,  9,31, 15,74, 17,67, 
     0,60,  7,95,  5,20,  8,99, 14,30,  1,48,  4,11, 15,55, 12, 7,  2,55,  3,17,  9,79, 10,18,  6,59, 18,22, 11,26, 16,35, 19, 1, 13,25, 17,40, 
     6,21, 10,14, 14,63,  7,63, 11,89,  0,19, 19,84, 15,85,  4,26,  9,78,  5,53, 16,98,  8,23,  2,84,  1,53,  3,47, 18, 2, 13,98, 17,58, 12,62, 
    12,17,  4,52,  1,12, 15,60, 19,21, 16,10, 17,18, 11,30,  6,57,  2,65, 13,31,  8,18, 18,95,  3,88,  0,41,  5,92, 14,75,  7,18, 10,87,  9,56, 
     1,36,  5, 6, 10,93,  4,57,  9,28,  2, 9, 16,93, 19,18,  7,38, 14,53,  3,88, 11,99, 17,11, 12,86, 13,75,  8,87, 15,16, 18,29,  0, 9,  6,53, 
    18,70,  2,59,  7,33, 17, 8, 14,80, 15, 8,  4,58,  3,97, 11,96, 19,47,  9,36,  5,29, 12, 1,  6,13,  8,17,  1,34, 16,34, 10,34,  0,65, 13, 7, 
     5,62, 15,33, 11,45, 14, 4, 12,20, 13,14, 18,24,  9,84, 19,58,  2,90, 16,95,  6,46,  4,60,  3,11,  1,29,  0,39,  7,24,  8,22, 17,93, 10,58, 
    16,30, 19,49,  0,93, 12,82, 18,67, 15,25,  1,63,  7,99, 14, 5,  9,93, 11,72, 13,13,  5,17,  2,73,  4, 5, 17,39,  6,20,  8,27, 10,50,  3,67, 
    10,99, 19,22, 16,94,  7,70,  6,69,  4,41, 15,46,  3,88, 13,87, 11,11,  8,55, 17,51, 18,56,  1,32, 14,29,  2, 6,  9,97,  5,54,  0,92, 12,84, 
     1,92,  3,29, 17,58, 18,91,  5,18, 11,15, 14,22,  9,79, 16,18, 12,95, 19,14, 13,66,  2,47,  4,70,  0,90,  6,60,  8,79,  7, 6, 10,60, 15,57, 
     8, 8, 11,64, 15,97,  2,20,  3,17, 10, 2, 18,65,  6,92,  5,29, 17,27, 12,62, 19,49,  7,95, 16, 5,  9,93,  1,38,  0,82,  4,41, 14,43, 13,16, 
    10,15, 18,11,  8,33, 13,83, 15,78, 19,32,  4,19,  9,52,  6,86, 16,20,  0, 8,  5,22,  2,42, 17,80,  1,61, 12,76, 11,15,  7,86, 14,15,  3,73, 
    13,48, 12,28,  7,37,  6,12, 15,61, 16,89, 11,31,  2,90,  9,92, 19,52, 18,99, 17,51, 14,48,  5,98,  0,99,  1,47,  4,98,  3,17,  8,32, 10,70, 
    12,78,  2,32,  1,66,  5,34,  3,58, 14, 6,  8,93, 17,19, 10,97, 13,42,  9,27, 11,22, 19,16, 18,92,  7,41,  4,87,  6,32,  0,49, 15, 1, 16,10, 
    18,99,  5,57, 17,77,  1,48, 12,33, 14,59,  7,51, 10,91,  6,75, 19,24,  4,15,  9,16,  0,56, 16,80, 11,42, 15,69,  3,73,  8,86, 13,85,  2,58, 
    19,72, 15,61,  4,26,  7,62,  8,15,  9,44,  6, 3,  0, 7, 10,78,  3,56, 18,90, 13,84,  1,34, 17,13, 16,95,  2,61, 11,52, 14,76, 12,22,  5,41, 
     5,24, 15,16, 16,13,  9,88, 17,92,  1,17,  7,27, 13,23,  6,10,  0,88, 18,88, 11,43, 12,70,  4,74, 10,84, 19, 5, 14,36,  3,71,  8,68,  2,69, 
    17,58,  8,70,  5,26,  0,59, 13,19, 16,87,  2,55,  1,12, 19,49, 18,41, 14,87,  4,52,  3,85, 12,39, 15,38,  7,21, 10,49,  6,16,  9, 8, 11,85, 
    12,48, 14,40, 19,65,  2,92,  5,11,  9,29, 17,68,  7,70,  3,21,  0,49, 18,42,  1,67, 13,40, 16, 6,  8,39, 10,29, 11,41,  4,82,  6,93, 15,19, 
     5,54,  3,66, 10,18,  1,54, 14,85, 11,42, 16,35, 12,55,  0,58,  4,33, 19,52, 17,28,  6,31,  7,97, 13,51,  8,14,  2,99, 18,50,  9,14, 15,31, 
     3,77,  6,11, 13,61, 18,44,  2,20, 19,39,  5,21, 15,80, 11, 3,  1,44, 14,13, 17,73, 10,96,  4,69, 16, 5,  8, 2,  9,73,  0,74,  7,30, 12,42, 
    14,93, 11,85,  0,73, 12,76, 17,33,  1,93, 19,98,  7,84, 15,83, 13,54, 18,15,  4,17,  3,33,  2,82,  8,52, 10,72,  9,37,  5,95,  6,45, 16,50, 
     7,82,  6,47, 19,93, 12,41,  4,23,  0,98,  8,12, 14,52,  9,78, 10,11,  1, 2, 15,25, 11, 2, 17,39,  5,79, 18,96,  2,33,  3,81, 13,74, 16,40, 
     6,66, 14,36,  5,12,  0,88,  3,83, 17, 4, 12,84, 16,68,  9,27,  2,65, 13,90,  1,73,  8, 1,  4,44,  7,27, 11,96, 19,73, 15,12, 18,27, 10,22, 
    16, 1,  8,41,  4,53,  1,29, 13,75,  0,17, 11,53, 17,83, 12,62,  3,93, 15, 5, 19,94, 18,25, 14,65,  6,68,  2,44,  9,16,  7,48,  5,58, 10, 7, 
     9, 9,  0,15, 16,42, 10,32, 19,51,  3,74,  4,58,  5,26, 12,51,  6,25, 17, 9,  1,52,  2,87,  7,23, 11,70, 18,58, 15,14, 14,49,  8,51, 13,54, 
     9,85,  5,14,  1, 8,  8, 8,  2,50,  0,94, 11,11, 14,87, 19,44,  4,35,  7,69, 18,62, 10,35, 15,55, 12,77, 17,92, 13,89, 16,20,  6,65,  3,13, 
    10,80, 11,79, 14,32,  7,35, 13,67, 15,44,  9,37,  5, 7,  1,93,  2,73,  4, 6, 16,77,  3,77, 19,84,  8,12, 18,48,  0,51, 17,73, 12,89,  6,27, 
    12,65, 16, 1,  9,29,  3,60, 13,55, 18,47,  1,69,  5,88, 10,62,  7,22, 17,44,  2, 3,  8,56,  4,75, 11,80,  0, 1, 15,65,  6,76, 19, 4, 14,66, 
     0, 3, 18, 6, 16,61,  9,62, 13,54,  1,85, 10,26,  7, 4,  3,27,  5,54, 12,84,  2, 3,  6, 2,  4,12, 19,44, 15,89, 11,81,  8,16, 17,79, 14,68, 
     3,15, 11,34, 18,70, 12,90, 15,47, 17,72,  0,25, 19,57,  4,20, 13,80, 16,88,  7,44,  9,78,  2,79,  6,53, 14,42, 10,64,  5,84,  8,15,  1,42, 
    13,29,  6,54, 16,11, 10,93,  1, 2,  9,86,  0,81,  7,23, 18,99,  8,49,  3,98, 11,78, 15,14,  5,25,  2,74, 14,94, 17,91, 12,32,  4, 5, 19,69, 
     3,23,  6,94, 12,61, 15,39, 14, 8,  7,71, 10,93, 13,72, 17,55, 16,95,  1,12,  2,60,  4,82,  0,46,  8,82, 18, 7,  5,55, 11,38,  9,86, 19,35, 
     6,45,  2,56, 19,10,  5,49, 12,77,  9,43, 10, 8, 11,66, 16,31, 17,74, 13,93,  0,49,  1,57, 18,23,  4,26, 14,97,  8,94,  3,71,  7,23, 15,97, 
     3,56, 19,90, 18,71, 10,50,  4,29,  5,63,  0, 1, 17,69,  9,97, 13,85, 16,42,  1,20, 12,20,  8, 3,  7,43,  6,86, 14,97, 15,22,  2,52, 11,21, 
     8,81, 16,12,  9,71,  7,39, 10,99, 15,70,  3,35,  4,84, 18,22, 17,47,  6,64, 19,98,  2, 7, 14,12,  1,75,  5,64, 12,86, 13,27,  0,38, 11,10, 
     9,35,  4, 6,  7,91, 12,16,  6,46, 13,79, 15,55, 17,96,  1,86,  5,45, 11,43,  2, 5, 14,95,  3,14, 19,30,  8, 4,  0,38, 10,91, 18, 9, 16,44, 
     4,20,  1,65,  7,18, 19,21, 17,13,  0,74,  9,44, 18,39, 16,97, 10,24,  3,33, 12,14,  2,43, 14,17,  6,80, 15,73, 13,39, 11, 6,  5,14,  8,43, 
    15,61,  2,83, 19,46, 14,71, 16,25,  9, 8,  1,81, 13,60, 10,14, 18,30, 17,10,  5, 1,  0,61,  4,53, 11,20,  7,19,  3,15,  8,12, 12,10,  6,35, 
    16,36, 14, 8, 10,78, 12,32, 13,93,  7,31,  8,11,  9,41, 18,12, 17,10,  5,49,  4,19, 19,99, 11,73,  2,95,  0,38,  6,83,  3,11,  1,25, 15,37, 
    11,37, 14,90,  0,80,  6,26,  2,42,  9,34, 15,67,  5,59, 12,81, 10,74, 16,17,  3,41,  7,27,  4,72, 19,37, 18,82, 13,78, 17,76,  8,59,  1,14, 
    13,47,  7, 1, 11,10,  0,88,  1,38, 19,83,  9,83, 17,99,  8, 1, 12,28, 18,61,  2,62, 14,76, 16,43,  3,29,  4,83, 15,97, 10,60,  6,29,  5,73
  };
  // Taillard 50x20 instance 5
  const int tai_50_20_5[] = {
    50, 20, // Number of jobs and machines
    10,75,  5,50,  2, 4, 17,34, 14,36, 19,69,  1,52,  4,44,  8,42, 16,12, 11,50,  3,12,  0,71,  6,89,  7,31, 12,14, 13,13, 18,53, 15,43,  9,43, 
     5,27, 13,18,  4,70,  3,84, 19, 8, 18,62, 16,91,  2,24,  0,94, 10,98, 11,78,  7,90,  1, 1, 15,24, 12,96, 14,61,  9,90,  8,67, 17, 7,  6,72, 
    10,56, 15,84, 19,95,  9,32, 16, 8, 12,43,  7,37, 17,25,  8,10,  1,85, 18,78, 13,76,  6,94,  5,60,  4,37,  0,68, 14,11, 11,66,  2,66,  3,83, 
    12,65, 10,95,  8,66,  4, 4, 15,34, 18,55, 11,35, 16,32,  6,78,  9,52,  0,10,  1,58, 19, 9,  5,73,  3,18, 17,52,  2,57, 14,59,  7,27, 13,89, 
     5,31, 15,81,  6,45,  7,21,  3,74, 17, 7,  9,43, 11,35,  1,23, 18,63, 12,12, 13,92,  4,78, 19, 9,  0,30, 10,22, 14,19,  8,70,  2, 4, 16,17, 
    17,53,  9,51,  8,35, 14,11, 10,55, 16,18, 13,89, 12,91, 11,16, 15,82,  2,84,  3, 4,  5, 2,  0,95,  1,17,  4,92,  6,81,  7,22, 18, 7, 19,58, 
    15,92, 19,47, 18,10, 12,47,  2, 4,  3,75,  5,41,  0,51, 14, 2,  4,73,  8,52,  1,38,  7,44, 17,23, 10,31,  9,90, 11,90, 13,81, 16,75,  6,84, 
     6,31,  5,20, 16,32,  4,54, 13,10,  1,96,  9, 6,  7,81, 14,73, 11,75,  0,95,  3,81,  2,31, 10,81, 18,90, 15,33, 19,64, 17,96,  8,21, 12, 1, 
    17,67,  1,61,  3,18,  6,81,  0,77,  9,35, 12,12, 19,28, 16,82,  7,10,  8,71,  4,94,  2,51, 13,54,  5,25, 18,37, 15,34, 11, 2, 14,71, 10,60, 
    12,29, 17,78,  5,33, 16,54,  7,20,  1,75,  8,88,  0,35, 19,70,  6,35,  3,53, 14,36,  2,82, 10,26, 15,24,  4,28, 18,10, 13,98,  9,86, 11, 2, 
     2,30, 18, 2,  0,96, 19,66, 10, 3, 17,81,  1, 4,  8,53,  7,12, 16,36, 15,61, 14,78,  5,25,  4,75,  6,44, 13,12,  3,81, 12,23, 11,41,  9,87, 
     7,98,  2,47, 12,85,  0,90, 16,74, 14,94,  3,24, 15,18, 17, 8,  5,90, 19,55,  4,67, 10,52,  8, 3, 18,28,  1,29, 13,73,  9,96, 11,57,  6,55, 
    17,18,  0,72, 12,88,  4,58,  5, 5,  1,81, 10,93,  8,95, 11,36, 15,51,  3,53, 16,61,  7,72,  6,63, 19,36, 13,48,  9,96, 14,92, 18,21,  2,61, 
     9,55, 15,45,  4,91,  3,24, 18,77,  1,71, 16,12, 14,91, 13,68,  5,55, 19,88,  0,76,  8,89,  7,14, 10,92, 12,64, 17,66, 11,13,  2,48,  6,57, 
     8,92,  5,18,  3,56, 19,69,  1,86,  2,23, 17,33, 13,86,  6,52, 10,71, 12,97, 15, 9, 11,16,  0,38,  9,69, 14,34,  4,43,  7, 1, 16,71, 18,84, 
     1,59, 15,50,  8,11, 13,21,  4,47, 12,52, 17,78, 19,72,  6,86,  9,28,  0,81,  2, 5, 18,29, 14,36, 11, 7,  5,68,  3,28, 16,97, 10,49,  7,93, 
    14,34,  9,20, 18,32,  6,90, 17,59, 10,53, 19,92, 16, 1,  4, 8,  5,10, 12,10, 15,25,  0, 8, 13,53,  1,25, 11,79,  3,85,  8,28,  7,10,  2,33, 
     8,37, 17,51,  5,79, 13, 6,  9,28, 14,40,  7,19, 18,60,  0,14, 10,12,  1,31,  2,25, 15,15, 16,84,  6,83, 11,85,  3,87, 12,34,  4,42, 19,92, 
    14,24,  6, 3,  8,12, 13,39, 16, 5,  0,28,  2,51,  3,68,  9,36, 18, 6, 15,12, 10,54, 11,61,  4,11,  5,99, 19,39,  1,53,  7, 3, 12,33, 17,48, 
    14,38,  8,81,  2,84,  7,12, 19,36,  5,96, 16,67,  6,46,  1,90, 13,78, 18,22, 11,83,  0,85,  4,97, 15,50, 12,78, 10,91,  9,83,  3,49, 17,31, 
     9,10,  5,42, 19,29,  3, 1, 11,88,  4, 7, 12,11,  2,41,  8,51,  1,40, 13,90,  6,20,  0,42, 18,40, 10,25, 17,31, 14, 8, 15,86, 16,84,  7,25, 
    18,12, 12,70,  0,93,  8,77,  2,18,  1,13, 16,70, 15,35, 13,97, 14,50, 11,32,  4,88,  7,98,  5,37,  9,82,  3,53, 10,21, 17,93,  6,60, 19,93, 
    19,14, 15,85,  0,21,  7,25, 12,72,  4,38,  3,43,  6,68, 10,38, 17,18, 14,35, 16,49,  8,99,  2,48,  9,87, 13,11, 18,16, 11,96,  1,36,  5,84, 
    17, 6,  9,23, 10,97,  8,72, 11,32, 16,77, 19,35, 12,62, 14,15,  6,72,  1,48, 15,34,  0,51,  7,24,  4,71,  2,18,  3, 5,  5,37, 13,56, 18,13, 
     1,10, 17,38,  5,81, 18,77,  2,83,  7,40, 16,43, 11,64, 15,91,  4, 1,  0,48, 10,80,  8,15,  3,21, 12,27, 19,16, 14,60, 13,45,  9,65,  6,88, 
     0,20, 17, 4,  9,79, 15,87, 16,27,  1,84,  6,20,  3,46, 12,91, 10, 9,  8,24,  5,98, 13,28,  2, 2, 19,76,  7,22,  4,88, 11,67, 14,32, 18,37, 
     4,23, 12, 7,  2,62,  1,49, 17,98, 14,61,  6,59, 15,38,  0,13,  9, 5,  5,75, 11,74,  7,65, 18,66,  3,60, 10,14, 16,79, 13,45,  8,47, 19,51, 
     1,23,  6,45, 13,96, 19,35, 18,37, 11,34, 17,29,  7,12, 10,89, 15,69,  4,74,  3,52,  9,98,  0,17, 16,86, 12,76,  8,96,  2,66, 14,86,  5,37, 
     2,74,  4,99, 16, 9, 12, 9, 14,74, 15,83,  9, 3,  8,96, 19,44, 10,23, 13, 3,  0,18,  1, 3, 11,45,  5,38,  6,24,  7,91, 17,36,  3,22, 18,15, 
    15,34,  4,23,  6,68, 13,11, 14,53, 19,77,  5,71,  1,65, 17,77, 16,48, 12,43, 11,45, 10,83, 18, 7,  8,93,  2,65,  0,83,  3, 8,  7, 5,  9,74, 
     8,40,  2,15, 15,30, 12,90, 10,62,  3,15, 18,13, 13, 3,  9,49,  0,90,  5,55,  7,65, 19,85, 16,87, 17,24,  1,61, 11,50,  4,48,  6,77, 14,21, 
    19,42,  8,91, 10,70, 16, 8, 18,45,  6,24, 14,63, 13,86, 11,16,  9,68,  2,57,  7,18,  3,60,  4,81,  0, 3,  1,17,  5,68, 15,97, 17,42, 12,54, 
    15,87,  9,95, 16,25, 10,61, 18, 9, 13,30, 11,84, 14,21,  6,88,  3,83,  1,72,  4,86,  8,20, 19,73, 17,39,  7,71,  0,52,  2,37,  5,72, 12,40, 
     1,46, 12,45, 17,87,  2,44,  9,27, 10,44, 15,19, 19,51,  4,83, 16, 2, 18,21, 11,13,  7,50,  8,17, 13,95,  0,83, 14,33,  5,29,  3,92,  6,62, 
     6,22, 19,73,  2,35, 17, 4, 15,22, 12,99, 14,99, 16,65,  4,54,  7,72,  1,59, 11,98,  5,44, 13,19,  9,23,  0,95, 18,87,  3,69,  8,68, 10,57, 
     3,20,  0,79,  6,95,  2, 6, 12,38, 16,36,  5,37,  8,21, 18,62,  1,48, 13,78,  9,18, 11,70, 14,78, 19, 1, 10,21, 17,85,  7,39, 15,34,  4,98, 
    14,56,  7,60,  3,33,  6,80,  9,68,  8,60, 16,41,  1,82, 15,17,  2,89, 17,59, 13,71,  5,49, 12,75, 19,45, 10,22, 18,60, 11,83,  4,71,  0,22, 
    18,72, 13,72, 19,11,  2,15, 10,72,  6,65,  4,56, 14,77, 15,68,  9,17,  7,59, 16,17,  1,68,  0,69, 12,71,  5,60, 11,39, 17,42,  8,78,  3,62, 
    12,12,  1,27, 10,16,  3,23,  0,12, 13,74,  2,72, 18,48, 15,35, 16,53,  9, 9,  8,80,  4,68,  7,15, 19,62,  5,23, 11,14, 14,89,  6,12, 17,67, 
     7,74, 12,53,  2,76, 13,97,  6,71,  4,81, 17,28,  0,70, 15,67, 19,26, 16,97,  3,11,  1,77,  5,56, 14,62, 11,41, 10,50, 18,40,  8,35,  9,79, 
     2,75, 18, 3,  5,32, 12,10,  0,93,  6, 2, 19,63,  9, 4,  1, 4, 15,18, 13,85, 14,28,  3,55, 16,80,  8,59, 17,74, 11,51, 10,74,  4,20,  7,46, 
     4,32,  2,77,  8,49, 11,76,  3, 6, 14,93, 19,57, 10,11,  9,69, 18,35,  6,59,  1,20,  5,22, 13,13, 17,35,  0,99, 15,95, 16,99,  7,74, 12,96, 
     3,55, 16,38, 14,80, 15,25, 12,73, 19,56, 18,87,  6,16,  7, 2,  5,80, 10,43,  2,50,  4,68,  0,12, 11,19,  1,25, 13,17, 17,52,  8,31,  9,31, 
    14,53,  3,69,  9,57, 16,98,  4,48, 17,26,  6,30, 10,20,  0,74, 18,47,  1,99, 15,78,  5,97, 12,94, 11,80,  2,74,  7,42, 19, 2, 13,46,  8,32, 
     9,54, 12,14,  1,96, 14,39, 15, 1,  7,98,  8,43,  2,57,  6,29, 10,76, 18,36,  3,58, 19,93, 11,17, 17,38, 13,81,  4,74, 16,20,  5,39,  0,98, 
     0,80, 19,94, 12,69, 15,69, 13,54,  7,30, 17,86,  6,80,  5, 6, 18,72,  9,65,  2,45, 14,89,  3,67, 16, 6, 11,28,  1,76,  8,28,  4,29, 10,28, 
    14, 1, 11,74,  6, 8,  8,96,  7,20, 12, 7,  5,26,  4,25, 19,18, 18,84, 15,29, 16,92,  2,18,  0,38, 10,93, 13, 8, 17,32,  9,67,  3,81,  1,17, 
    12,36, 11,41, 13,72, 19,31,  9,28, 18,52,  5,14,  6,59,  2,97, 15,71, 14,92,  0,50,  8, 4,  1,96,  3, 6,  7,99, 16, 1, 17,70, 10,58,  4,92, 
     6,24,  1,81,  9,84,  5,57, 15,59, 11,94, 10,31,  7,98,  0,37, 12,64,  2,69,  8,56, 14,71, 17,23, 13,30,  3,97, 16,86, 18,29, 19,16,  4,75, 
     2,33, 13,81,  8,58, 10,81, 18, 2,  4,25,  6,17,  7, 1,  5,72, 11,33, 16,18,  3,22, 17,44, 19,28,  0,69, 14,92, 15,90, 12,43,  1,53,  9,76
  };
  // Taillard 50x20 instance 6
  const int tai_50_20_6[] = {
    50, 20, // Number of jobs and machines
    15,37,  2,64, 17,15, 11,52,  3,71, 19,38,  5,53, 14,70,  7,76, 10,76, 18,61,  1,10, 12,51,  9,59, 13,12,  4,74,  6,61, 16,52,  0,68,  8,19, 
    10,79, 18,71, 14,32,  1,18,  9, 9,  6,99,  5,85, 16,94, 11,41, 19, 1,  0,18,  4,98,  3, 2,  2,47, 15,57, 17,44,  7,25, 13,48,  8,12, 12,24, 
    16,50, 12,55,  8,25,  3,24,  6,43,  9,64, 18,40, 19,37,  4,30, 10,71, 13,64, 11,13,  5,33, 14,26, 15,42,  1,64,  0,55, 17,76,  2,17,  7, 8, 
     8,31, 12,22,  6,38, 19, 9,  4,84,  5,68, 14,35,  2,94, 11,79, 16,79, 18,40, 10,39,  0, 9, 17,36,  9,82, 13,39,  1,33,  7,43, 15,86,  3,74, 
    18,17, 16,73, 17,55,  3,30,  8,28, 14,35, 15,72,  5,30,  1,50,  9, 3, 19,84, 13,72, 11, 4,  0,73,  2,54,  6,15,  4,64, 12,43, 10,10,  7,80, 
     7,55,  3,13, 12,64, 16,94, 10,89, 18,21,  6,31,  4,82,  9,54, 19,16, 15, 8, 11,99,  0,70, 17,22,  2,89,  5,65, 14,56, 13,92,  8,15,  1,77, 
    18,41, 19, 4,  7,42,  5,81, 10,82, 16,56, 15,79,  6,97, 13,47, 17,91,  1,48,  2,86, 14,28, 12,80,  3,89,  9,91,  4,44,  8,67, 11,67,  0,53, 
    15,27, 13,96,  8,43, 19,23,  6,50, 11,78,  3,46,  0,13, 18,54,  9,20, 16,37,  1,22,  2,13, 12,53,  7, 4,  4,53, 14, 7, 10,53,  5,56, 17, 4, 
     1,28,  0,83, 18,63,  6,76, 17, 2, 16,11,  5,16, 13,55, 10,78, 15,55,  9,77, 19,32,  4,27, 12,46, 14,17,  2,45,  3,40,  8,94,  7,10, 11,12, 
    18,59,  2,81, 17,85,  0,87,  5,37,  1,30, 13,32,  9,10, 10,72, 11,99,  7,23,  6,32, 12,27, 14,32,  3,77, 15, 6,  8,60, 16,85,  4,79, 19,66, 
    10,89,  1, 8,  5,85,  3,49, 19,54,  2,13,  4,32, 18,33, 15,53, 13,76,  6,83, 12,75,  9,29,  0,79,  7,65,  8,50, 11,37, 14,18, 17,36, 16,46, 
    11,51, 14, 5,  6, 6,  2,64,  9,33,  4,14,  0,42,  1,12,  8,92,  7,68, 18,94, 13,29, 19,89, 15,40, 16,10, 17,43,  3, 8,  5,82, 12,88, 10,86, 
    16, 7,  0,85, 13,12, 11,56,  2,61,  8, 6,  5,77,  9,64,  4,40,  7,13, 15,86, 14,38, 18,89, 10,98, 12,42, 19,93,  3,86,  6,97,  1,33, 17,22, 
     7,86,  0,45, 13,99,  5,80, 14, 8,  9,76,  3,42, 15,14,  4,81,  8,85, 11,88, 12,16, 19,48, 17,23, 16,27,  2,49,  1,42,  6, 5, 10,61, 18,26, 
     3,36, 11,60, 10,80,  6,34,  8,30,  5,53, 17,91,  9, 2, 14,89, 12,31,  2,61, 16,35, 18,69,  7,28,  1,16, 15,70, 19,88,  4, 1, 13,97,  0,33, 
    13,48, 14,21, 17,64, 11,50, 16,79,  5,26,  2,20,  3,11,  6,16,  1, 3, 18,61,  9,29, 15,97, 12,66,  0,98,  4,51, 19,54,  8,50, 10,96,  7,33, 
     8,61, 13,16, 10,30,  7,30,  9,96, 12,35,  2,20, 14,63,  6,61, 18,15, 11,45,  1,63,  4,66, 19,61, 17,70,  3,75,  0,89, 16,97,  5,17, 15,60, 
     7,78, 16,41,  2, 9,  6, 8,  3,26,  5,69,  1,55,  0,30,  9, 7, 13,27, 14,59, 12,33, 18,18,  4,77, 10,57, 11,90, 15,24, 19,41, 17, 6,  8,71, 
    17,61,  7,27,  1,46, 16,30, 18,46,  9,15, 10,24, 11,99,  6,44,  2, 1, 15,16,  5,11,  3,15, 19,38, 14,54, 12,13,  4,73, 13,68,  8,85,  0,54, 
    12,14,  6,33, 16,65, 11,97,  8,74, 14,55,  0,18, 19,76,  1,74,  2,70, 10,78,  3,15, 18,40, 17,22,  7,56,  9,68, 13,31,  5,53, 15,19,  4,89, 
    13,61,  3,97,  7,92,  1,64,  6,89, 19,51, 16,68, 11,83, 14,21, 17, 5,  2, 5, 18,85,  4,67, 10,94,  8,97, 15,71,  0,39,  5,58,  9,30, 12,82, 
     1,82, 12,20, 18,82,  5,14, 19,71,  2,91, 14,61, 17,19, 10,67,  3,78,  9,53,  4,56, 13,40, 11,51,  0,46,  6,95, 15,38,  7,12,  8, 9, 16,90, 
     6,97,  4,90,  8,34, 11,99, 17,28, 13,84,  5,65, 12,57,  1,29,  9,87, 15,13, 10,23,  2,51,  0,88, 19,36,  3, 7, 16,34, 18,18, 14,59,  7,96, 
     4,19,  7,85,  5,91, 19,30, 12,69,  9, 2, 15,96,  1,21, 17,81,  6,32, 16,79, 18,46, 14,92,  0,88, 13, 3,  3,20, 10,53, 11,21,  2,57,  8,20, 
    10, 9,  2, 5,  0,50, 13,26,  4,67, 16,64, 12,34,  1,73,  7,29, 15,56,  5, 7,  9,70,  3,54,  6,47, 18,82, 14,30, 17,17,  8,92, 11, 2, 19,23, 
    11,41, 14,79,  7,72, 16,88,  3, 1, 10,64,  6,99, 13,32,  4,55,  2,80,  0,28, 15,87,  5,65, 18,36,  1,72,  9, 5, 17,13,  8,56, 12,32, 19,72, 
     4,56, 16,85,  8,52,  3,64,  0,60, 17,81,  5,44, 12,43,  2, 9,  7, 4,  9,61, 18,24, 14,62, 10,56, 13,17, 19, 9, 11,29,  6,45,  1,63, 15, 7, 
    16,64,  5,68,  7,94, 10,46,  8,16, 17,10, 14,81,  0,19, 13,91, 18,75, 15,95, 12,21,  1,50,  6,82, 11, 2, 19,73,  2,55,  4,93,  9,39,  3,35, 
    12,69,  8,65,  7,25, 17,51,  2,28,  4,76,  9, 1, 18, 3,  1, 6,  5,74, 10,11, 13,59, 11,91, 19,10,  0,35, 15, 8, 14,27, 16,35,  6,28,  3,97, 
    15,41,  3,45, 16,53, 19,13,  6,92, 11,56,  8,42,  0,48, 12,67,  5,97,  9,80,  4,57, 18,80,  1,10,  7,90, 13,26,  2,68, 10,35, 17,84, 14,99, 
    13,43,  9,68,  1,28,  0,76,  8,32, 11,20,  5,89, 10,28, 17,46, 18,17, 16,41,  2,32, 14,37, 19,32,  4,48,  3,47,  7,32,  6, 2, 12,26, 15,37, 
    11,83, 16, 8,  4, 5, 17,24, 18,69,  6,69,  3,93, 13,54, 12,90, 19,15, 14,63, 15,71,  1,84,  7,78,  0,62,  5,84,  9,99, 10,82,  8,30,  2,51, 
     1,69,  3,64,  9,75,  0,23,  5,93, 17,48,  8,11,  7,18, 13,89, 11,96, 18,70, 10,99, 16,52,  4,70,  6,23,  2,85, 12,50, 19,81, 15,17, 14, 5, 
    15,87,  0,55, 10,48,  4,85,  2,17,  8,79,  6,73, 12,19, 13,22,  7,37, 18,12,  3,19, 16, 9, 11, 5, 14, 4,  5,85, 19,88, 17,46,  1,10,  9, 4, 
     7,86,  3,36, 13,90,  1,63, 12,48,  2, 4,  9,15, 15,15,  8,15, 14,39,  6,73, 10,89, 11,58,  4,80,  0,71, 18,54,  5,25, 16,41, 17,84, 19,73, 
    18,42,  6,48,  3,88,  9,71, 10,68, 13,25, 12,33,  5,88,  7,62, 19,51, 15,49,  1,76,  8,22, 17,47, 11,63,  4,61,  0,16, 14,10,  2,94, 16,47, 
    15,32, 13,41,  4,99,  0,48,  2,82, 10,52,  3,46,  8,67, 19,63,  9,16, 17,24, 11,32, 18,88,  6,74,  1,13,  7,24,  5,82, 16,28, 12,74, 14,14, 
     7,56, 19,60, 18,70, 16,93,  8,22, 12,62,  4,50,  6, 2,  0,15, 14,99,  9,20,  2,45, 15, 6,  5,81,  3,13,  1,51, 13,12, 10,12, 11,55, 17,35, 
    17,46,  7,12, 19,92, 16,23,  2,99, 13,11, 18,99, 11,88,  0,22,  1,18,  3,29, 10,53, 15,56,  6,56,  9,43,  4,53,  8,10, 12,41, 14,61,  5,12, 
     9,49, 18,48,  2,25,  4,40,  1,20, 10,10, 17,96,  5, 8,  0,48,  8,91,  3,88, 15,30, 12,90,  6,53, 16,25, 11,32, 19,43,  7,50, 14, 3, 13, 4, 
     7,90, 18,42,  1,20,  9,12,  5,61, 13,89, 16, 2, 11,57, 19,26, 14,77,  2,32, 15,41,  3,89, 17,45,  8,55, 12,37,  6,66,  4,11,  0, 1, 10,55, 
     5,22,  4,39, 10,26, 16,85,  7,61,  1,54,  2,27, 11,25,  0,31, 13,47,  8,54, 14,16,  6,77, 19,29,  3,71, 18,24,  9,86, 17,68, 15,21, 12,40, 
     2,63, 13,25, 10,19,  6,97,  1,61, 15,71, 18,76,  7,52, 12,18, 19,27, 11,97,  3,74,  5,16, 16,10,  0,72, 14,61,  8,48, 17,96,  4,83,  9,98, 
     2,14, 13,23, 17,81,  0,53, 19,83,  7,93, 12,86,  6,45, 10,71,  3, 9,  1,94, 18,91,  4,90, 11,34, 16,75,  8, 1, 14,73,  5,83, 15,75,  9,68, 
     7,18,  5,64, 14,22, 16,33,  1, 9, 17,57, 15,42,  6, 1,  8, 9, 12,45,  4,20, 18,24,  2,68, 11,86, 19,59, 10,90,  0,56,  9,41, 13,39,  3,43, 
    11,45, 16,67,  2,45,  7,18, 19,69,  3,26, 14,38,  4, 1,  6,71,  8,64,  5,29, 17,77, 10,50, 15,23, 12,24, 13,67,  9,80,  1,89,  0,96, 18,21, 
     2,39, 15,45,  4,60, 18,65,  3,67, 12,91,  6,99,  9,96, 19, 3,  8,11,  5, 1,  0,14, 13,94, 10, 9,  7,13, 11,82, 14, 8, 16,58, 17,13,  1,76, 
    18,57, 17,50, 19,55,  1,54,  3,46, 14,52, 13,44, 15, 3,  8,71,  0,80,  5, 7,  7,66, 10,25, 12, 5,  4,44, 11,44,  9,76,  6,83,  2,38, 16,94, 
    17,69,  4,32,  6,47, 16,61, 15,71, 11,39, 12,56,  8,69, 10,32,  1,60,  2,22,  3,68,  7,18, 13, 9, 14,76,  5,44,  0,39,  9,22, 18,16, 19,95, 
     7,48, 15,92,  6,62,  3,48, 12,93,  9,27,  8,80,  0,48,  2,85, 18,62, 17,24, 11,67, 10,88, 16,29,  5, 5,  4, 3, 13,77, 14,47,  1,13, 19,60
  };
  // Taillard 50x20 instance 7
  const int tai_50_20_7[] = {
    50, 20, // Number of jobs and machines
     4, 8,  3,41, 18, 4, 17,52, 12,47, 13,67, 14,65,  6,18,  7,73, 11,10,  1,70,  5,66, 16,64,  8,53,  9,34, 15,28, 10,74,  0,40, 19, 8,  2,30, 
    13,37,  5,91, 14,97, 18, 6,  1,48, 19,90, 11,32,  6,12,  3,93,  7,15,  2,33,  8, 5,  4, 2, 10,11, 15,96, 12,16, 17,82,  9,49,  0,48, 16,81, 
     5,11,  3,70, 18, 4, 15,92, 11,18, 19,41, 12,77,  1, 5,  6,49, 17,94,  4,32, 13,67, 14, 2,  0,16,  9,21,  8,69, 10,89,  7,32,  2, 6, 16,33, 
    13,87,  9, 3,  6,82,  0,44,  7,16,  8,54,  1, 9,  3,49, 15,28, 10,70,  5,85, 18,34,  2,53, 16,37,  4,59, 19,66, 11,41, 12,96, 17,84, 14,54, 
     1, 1, 11,31, 19,73, 18,35,  0,81,  8,84,  4,34,  7,53,  5,73,  2,30,  6,63, 13,76, 15,62, 16,14,  9,30, 17,31, 10,89, 14,28, 12,92,  3, 3, 
     5,64,  9,46,  6,81, 13,38,  8,46, 17,69, 10,10, 19,27, 18,36,  4,94, 14,53, 11,31,  3,15, 12,59,  2,31, 16, 6, 15, 1,  1,28,  0,43,  7,92, 
     8,27,  0,78,  4,10, 15,19,  2,89, 14,91, 18,42, 16,13, 11,75, 17, 2,  6,36,  7,14,  5,59,  3,14, 13, 9,  9,34, 10, 3,  1,85, 19,44, 12,94, 
     5, 8, 13,90,  1,38,  6,23, 17,69,  8,34,  0,13,  7,62, 10,38, 16,74, 15,67, 12,45,  2,62, 19,32, 14,86, 18,59, 11,84,  9,66,  4,37,  3,52, 
    16,45, 15,30, 18,23,  2, 7, 14,92,  0,72, 12,34,  6,63,  3,68, 11,36,  9,75, 17,10, 19,27, 13,89,  5,31,  8,88,  7,46,  1,36,  4,71, 10,23, 
    18,72, 12,36,  9, 5,  5,98, 11,38,  6,99, 19,92, 14,92,  1,63,  4,20, 17,42, 16,77,  3,71,  8, 3, 13,80, 15,95, 10,84,  2,32,  7,32,  0,66, 
     2,63,  3,15, 13,66,  0,16,  9,51,  7,26, 19,24, 17,78,  8,54,  1,66, 14,51, 18,30,  6,37, 16,72,  5,77, 15,11, 12,33, 10,30,  4,36, 11,24, 
     7,78, 14,75, 16,35,  5,10, 19,81,  8, 1,  1,28,  3,58, 11,80,  0,61, 17,52, 15,74,  2,17,  9,11, 18,64, 13,69, 10,27, 12,92,  6,79,  4,89, 
    10,25, 17,19, 11,53,  9,36, 19,53,  1,27, 15, 8,  4,23, 18,86, 14,31,  7, 2, 16,95, 12,53,  2,29,  5,35,  0,28, 13, 1,  8,71,  6,57,  3,56, 
     8,11,  2,56, 17,64, 19,45, 10,96, 14,94, 13,48, 16,79, 12,48, 11,58,  6,32, 18,58,  3,13,  9,95,  5,39,  0,21,  4,18,  7,83,  1,45, 15,69, 
    10,88, 11,25,  7, 9, 18,83, 17,12, 14,79, 16,41,  1,89,  9,93,  8,46,  2,24, 12,41,  3,59, 19,56, 15,19,  5,12,  4,34,  0,80, 13,81,  6,51, 
     5,22, 11,49,  9,73, 18,59, 12,48,  8,76,  2,78, 14,69,  1,66, 15,27,  7,73,  6,46, 16,42,  0,53,  4,62, 10,39, 19,15, 17, 4,  3,76, 13,53, 
    12,96,  7,91, 16,71,  5,60,  6,60, 15,60, 17,44,  0,18, 13,82,  8,90, 19,77, 10,93,  2,24,  4, 6, 18,77, 14,56,  9,82,  3,15,  1,85, 11,41, 
     9, 4,  4, 8, 17,10, 14, 4, 10,23,  0,17, 18,33,  2,50, 13,24, 16,12,  6,29, 19,65, 12,43,  8,68,  3,66,  7,82, 11,22,  1,89, 15,58,  5,25, 
     5, 4, 13,14,  1,62, 14,43,  0,49,  6,18, 19,91,  3,83,  7,53, 10,35,  9,79,  4,62, 11,42, 18,17, 17,62,  2,39, 15,76, 16,43,  8, 9, 12,39, 
     4, 1,  0,98, 13,77,  2,28,  1, 3,  3, 8, 15,43, 12,53, 11, 4,  6,80,  9,81, 18,98,  5,61,  8,91, 19,32, 16,65, 17,52, 14,29,  7,49, 10, 1, 
    10,40,  1,89, 15,65, 17,30,  9,23,  6,47, 18,97,  4,28, 19, 3,  7, 5, 13, 4,  5,85,  3,34, 11,24,  0,87, 14,63,  8,54, 12,54,  2,27, 16,95, 
    10,50, 11,24, 13,37,  7,99,  4,14, 17,99,  3,25,  9,64, 19,73, 14,64,  6,24,  2,86, 15,10, 16,76, 12,56, 18,81,  8,62,  5,49,  1,19,  0,78, 
     7,73,  4,76,  5,31, 12, 4,  9,10, 13,24,  3,67, 17,57, 11,25, 16,72,  8,30, 19,31, 10,79, 14,13, 18,41,  6,85, 15,79,  0,61,  2,45,  1, 3, 
    18,72,  6,49, 19,50,  9,95, 17,81, 10,80, 14,50, 15,41, 16,48,  0,68, 11,15,  5,29, 13,68,  7,83,  2,72,  3,25, 12,56,  1,19,  8,80,  4,60, 
     6,68, 15,30,  7,38, 14,23,  5, 5,  8,65,  2, 1,  9,18, 17,61, 11,51, 19,44, 12,64, 10,98,  1,99, 18,64, 16,87, 13,90,  3,66,  0,99,  4, 7, 
     9,94,  4,60,  7,19, 18,40,  5,46,  1, 7, 13, 7, 10,86, 15,32,  3, 3,  2,25, 19,89, 12,59, 11,69, 17,89, 14,65,  0, 9,  6,77, 16,35,  8,42, 
    16,93, 19,83, 13,79, 18,36,  1,15,  3,44,  9,45,  8,11, 12,47, 11, 2, 10,84,  0,51, 15,26,  7,62,  2,54,  5,71,  4,86,  6,64, 17,61, 14,38, 
     7,88,  4,37, 17,37, 12,36, 15,59,  1,14,  3,89,  5,93, 11, 6, 14,47,  8,44, 13, 1, 19,82, 10,84, 18,50,  6,21,  9, 4, 16,20,  2,98,  0,37, 
    16,65,  8, 4, 12,98, 19,29, 18,22,  7, 1,  4,70, 13,89,  6,73, 14, 5, 17,15,  3,33, 10,23,  1,63,  9,20, 11,28,  0,31,  2,62, 15,90,  5,53, 
    14, 5,  4,51, 12,90, 19,91, 17,24,  9,98, 18,31,  8,90,  5, 3, 13,56,  6, 5,  2,41, 10,75, 15,57,  1,49, 11,75,  3, 1,  0,66,  7,65, 16,58, 
    15,85,  8,43, 17,95,  9,42, 19,50, 18,32, 10,37,  0, 8,  5,68,  7,82,  6,78, 13,11, 12,45,  3,32,  4,66, 11,41,  1,53, 14,91, 16,65,  2,88, 
    16,87,  9,16, 12,43, 13,86,  6,67,  5,82, 11,70, 18,65, 17,56,  4,53, 10,22,  0,17,  3,94, 15,61,  1,69,  7,73,  8,33, 14,69, 19,36,  2,64, 
    16,85, 10, 6,  7,31, 15,23,  9,33,  3, 4, 14,69,  5,61,  1,50, 11,27, 12,25, 18,10,  2,26, 19,26,  4,42,  0, 9, 17,72,  6,30,  8,91, 13,83, 
    10,44,  6,77, 18,51, 14,49,  2,37,  7,20, 17,69,  8,95,  5,84,  1,43,  4,86, 15,86, 12,57,  3,29, 11,70, 13,94,  9,38,  0,39, 19,61, 16,60, 
    16,37,  3,22,  2,56, 11,88,  7,95, 14,19,  9,92, 15,64,  1,25,  6,69, 10,37, 18, 9,  0,82, 19,67,  5,90, 13,52,  8,64, 17,25, 12,62,  4,39, 
     4,68, 13,36,  3,85, 15,37, 19,66, 10,31, 18,35, 11,97, 14,76, 16,22,  1,45,  2,80,  9,90,  6,48,  5, 5,  7,56,  8,50, 12,66, 17,54,  0,53, 
     4,86,  3,21, 18,36,  2,42,  0,67, 17,38, 14,28,  8,58, 15,41, 13, 1,  9,67, 16,45,  1,25,  5,18, 19,12, 11,71, 10,49,  7,28, 12,21,  6,52, 
    12,93, 14,97,  5,35,  2,71,  3,62,  9,49,  7,81, 11,60, 13,64, 10,50,  4,33, 18,39,  0, 7, 15,44,  1,53, 16,20,  8,96, 17,49, 19,23,  6,70, 
    13,15,  0,22, 11,40,  8,12, 18,19, 17, 5, 10,32, 19,85, 16,48, 15,46,  6,97,  1, 4,  5,97,  4,95, 14,90,  2, 2, 12,69,  9,95,  3,68,  7,18, 
     3,52,  0,57, 13,67,  8,67,  5,91,  1,82,  7,86,  4,29, 17, 8, 11,35, 10,65, 12,13,  2,92, 19,28, 16,77,  9,99, 14,30,  6,16, 15,31, 18,35, 
     7,85, 19,24, 15,60,  5,60,  3,89,  1,99, 12,88, 11,15,  4,45,  0,60,  8,38, 16,44, 17,38,  2, 4, 10,95, 18,27,  9,27,  6,15, 14,76, 13,53, 
     9,15, 19,64,  8,29,  3,72, 17,98, 10,73, 16,28, 13,25, 11,35, 14,57,  0,26, 15,38,  1,42, 12,19,  7,75,  4, 2,  2,86,  6,46, 18,84,  5, 5, 
    19,57, 13, 4, 11,83,  6,13,  2,80, 12,45,  7, 6, 14,46,  0,63, 15,94,  8,54, 17,69, 16,69, 18,43,  1,62,  3, 6,  5,15,  9,35, 10,62,  4,44, 
     8,57, 10,67, 17,80, 16,80, 18,61,  9,84, 15,90,  1,63,  5,26,  6,98,  2,94,  3,92,  7,84,  0,16, 19,24, 12,67, 11,47, 13,35,  4,79, 14,99, 
    11,63,  8,45, 10,49,  7,38,  0,32, 15,87,  5,41, 17,33, 19,18, 18,40, 16,50,  2,84,  3,36, 14,99,  6,77,  9,16,  1,52, 13,20,  4,60, 12,66, 
    11, 1, 14,24, 13,37,  1,54,  9,39,  5,50,  7,38, 18,79,  4,88,  2,35, 19,58, 15,77,  0,43, 17,98, 12,52,  6,73, 16,45, 10,45,  3,84,  8,80, 
     6,26, 15, 9, 19,92, 16,70,  5,87, 12,33, 11,14,  4,83,  0,34,  2,98,  3, 5,  8,99, 10,93,  7,94,  1,43, 17,36, 14,26, 18,11,  9,28, 13,16, 
    12,19, 15,58, 14,30,  3,72,  1,39,  2,27,  9,18, 11,44, 16,20,  6,87, 10,82, 17,51,  8,78, 18,20,  4,19, 13,36,  0,42,  5,75, 19,85,  7,95, 
     6,41,  9,46, 10,81, 11,17, 12,25, 18,80,  0,41,  4,29,  1,99,  2,14, 13,28,  8,25, 16,58,  7,24,  3,59, 14,45,  5,17, 19,53, 15,16, 17,86, 
     3,30,  2, 3,  8,28, 19,92, 15,87, 13,28,  6,30,  4,69, 14,20, 16,94, 10,97,  5,91, 17, 5,  7,16, 11,88, 18,47,  1,66,  9,67, 12,15,  0,29
  };
  // Taillard 50x20 instance 8
  const int tai_50_20_8[] = {
    50, 20, // Number of jobs and machines
     2,84,  6,26,  0,16, 18,14,  9,43, 12,28,  5,86, 10,92, 19,32, 16,73, 15,61,  7,13,  3,48, 13,70,  8,68,  1,56, 17,84, 14,23,  4,94, 11,30, 
     3,77,  0,32, 13,55, 12,22,  2,83, 18,19,  4,49, 11,80,  1,27, 16,69, 17,46, 10,86, 19,51,  5, 1,  8,88, 15,77,  6,98,  9,48, 14,24,  7,63, 
    17,36,  7,68, 15,42,  2,45, 19,34,  0,73,  6,14,  1,82, 18,15, 13,35,  5,92, 11,10, 14,43,  3,18, 16,73,  4,69, 10,55, 12,63,  9,89,  8,98, 
     3,42,  5,23, 11,29, 15, 8,  7,93, 12,19,  8,64, 17,47, 18,19,  6, 4, 16,84, 14,72,  9, 2,  0,89,  1,77, 19,12,  4,92,  2,67, 13,38, 10,87, 
    19,57, 17,67, 11,99, 14, 9,  0,89,  6,58,  3,16,  1,35, 15,30, 10,58,  7,28, 13,70, 18,82,  2,30,  9,84,  8, 5,  5,13, 16, 3, 12,91,  4, 8, 
    11,34, 16,34,  1,32, 14,17,  9,49,  7,13,  0, 4,  6, 7, 17,38,  5,62,  4,46,  8,29, 19,87,  2,23, 12,44, 18,93, 13,15, 10, 9, 15,56,  3,42, 
    18, 4, 12,71,  3,63, 19,19,  2,18, 15,48,  1,47,  4,50,  8,82,  9,94, 16,22, 17, 7, 14,69, 10,68,  7,34,  5,75,  0,13, 11,55, 13,11,  6,60, 
     6,44, 19,20,  1,86,  0, 2, 16,90,  7,32, 17,54, 10,40, 12,40, 15, 3, 14,20, 11,68,  8,84,  4,40, 18,91, 13,53,  2,77,  9,83,  3,97,  5,63, 
    13,45, 12,72, 19, 8, 17,61, 14,51, 15,96, 11,94,  1,98,  7,83,  2,48, 16, 6, 10,22,  8,50,  4,24,  6,36,  3,21,  5,17,  9,48,  0,54, 18,21, 
     7,24, 18,20,  6,92, 13,47,  2,99, 16,85,  0,20,  3,90,  4, 1, 15,20,  5,36, 11,58,  9, 2,  1,86, 10,42, 19,38, 17,69, 12,79, 14,43,  8,55, 
     3,62,  8,21, 19,95,  4,53,  7,75,  1,90,  0,94, 14,43, 16,72, 12,38, 13,60, 11,30, 15, 6, 10,75,  5,31,  6,87,  2,63,  9,24, 18,16, 17,46, 
     4, 5, 18,48,  3,88, 12,78, 14,84, 13,85,  1, 1, 16,79, 10,95,  9,42, 11,21, 15,69,  2,89, 19,68,  8,56,  7,30,  5,96,  0,98, 17,75,  6,24, 
    17,58,  3,49,  7,53,  5,85, 16,20, 11,15,  6,44, 19,97,  9,24,  0,12, 12,98, 15,53, 10,67, 13,10,  8,91, 14,47,  1,78,  4, 5,  2,28, 18, 5, 
     0,55, 13,30,  8,68, 14,34,  2, 3, 11,58,  1,18, 16,20, 17,33,  7,62, 12,70,  9,87, 15,55,  5,91, 19, 2,  4,33, 18,22,  6,20,  3,26, 10,99, 
    15,64, 11,88,  3,94,  1,34,  9, 4, 13,78, 19,10,  8,67, 12,77,  7,53,  5,67, 14,91,  2,24, 17,30,  6,76, 18,70, 16,56, 10,89,  0,49,  4,74, 
     7,95, 14, 9,  3,29,  6,45, 18,64, 12,73, 19,62,  9,11,  0,31,  1,77, 11,61, 16,20, 15,54,  5,37,  2,17,  4,31, 17,32, 10,98,  8,20, 13,94, 
     9,14, 15,90,  8,73, 13,84,  7,75, 17,89,  4,41,  2,59, 18,58, 14,44, 11,21, 12,61,  0,91,  5,25,  6,48,  3,12, 16,37,  1,80, 19,77, 10,51, 
    14,47,  3,92, 18,11,  7,73, 17, 4,  5, 9, 15,95,  0,22,  9,32, 19,89, 10,64,  4,19,  8,69, 11,83, 13,91,  6,34, 12,80,  1,94, 16,93,  2,82, 
    12,92,  6,79, 11,74, 17, 2,  3,97, 18,68,  4,98, 14,78,  9,22,  1,41,  2,58, 16,99, 13,52,  0,67, 10,48,  5,33, 15,48,  7,58, 19,81,  8,39, 
    11,20,  5,96, 19,64,  0,95, 13, 6,  3,26,  7,39, 16,26, 10,92,  6,12,  9,65, 15,14,  2,71,  4,31, 17,97, 14,24,  1,85, 12,89,  8,11, 18, 5, 
     0,77,  9,82, 15,36,  3,52,  7,94,  1,87,  4, 7, 19,65, 13,40,  5,61, 17,79, 14, 2, 11, 7,  6,71,  2,49, 18,18, 10,61, 12,69,  8,50, 16,85, 
     2,63, 15,50,  4,88, 17,90,  5,35, 10,20, 16,69, 18,88, 14,43,  3,94,  1,51,  0,94, 19,12, 12,90,  6,36, 11, 3,  9,99, 13,37,  7, 8,  8,69, 
    19,43, 15,66, 16,92,  6,61, 11,11,  3,53, 12,54, 18,54,  4,42, 17,87, 14,86,  9,34,  8, 7,  0,60,  1,52,  7,96,  5,25,  2,55, 13,49, 10,56, 
     6,11, 18,71,  4,60,  5,73, 17,91, 16,99,  3,22,  7,74,  9,80,  1,79, 19,97, 12,93, 14,85,  8,72, 10,96, 13,52, 15,70, 11,11,  2,95,  0,85, 
     8,83,  5,39,  3,51, 12,49,  6,31,  7,18, 13,94, 19,88,  0,46, 10,98, 14,66, 11, 7, 16,20, 17,53, 15, 9,  2, 7,  9,14,  4,85, 18,32,  1,88, 
    17,98, 12,91, 11,78, 19,43,  9,55,  5,45,  2,59,  6,77,  3,31, 16,91, 14,60,  1,61, 18,62, 10,59, 13,89, 15,25,  4,18,  7,29,  0,49,  8,53, 
    10,26, 16,50,  1, 7, 12,78, 15,75,  6, 3,  9,28,  0,20, 11,58,  7,95, 18,38, 19,58,  8,70,  4,75,  5, 6, 17,26, 14,26, 13,54,  2,95,  3,12, 
    17,81, 13,71,  5,22, 11,56, 15,17, 16,14, 10,65, 12,90,  1,29, 14,47,  3, 8,  9,39, 19, 9,  8,58,  0,95,  6,25,  2, 1,  4,65, 18,44,  7,66, 
    16,62, 13,79, 18,70,  7,43,  3,95,  2,42,  8, 6, 17,88, 19, 5,  5,12,  6,54,  9,18,  0,83,  4,27, 12,38,  1,49, 15,94, 11,95, 10,78, 14,57, 
     7,18,  6,36, 12,70,  9,10, 16,25,  0,25,  5,72, 18,69, 13,15,  2,17,  3,31, 10,60, 11,69, 15,39,  1,54,  4,56,  8,85, 17,47, 14,40, 19,10, 
    13, 7, 10,14,  9,11, 16,33,  6,49,  0,82, 18,76, 19,11,  2,52,  3,20, 14,35,  5,73,  4,81, 15,42, 11, 4, 12,27,  8,14, 17,73,  7,10,  1,25, 
    11,54, 17,81, 15,52,  1,25,  4,78,  6,23, 13,38,  3,59, 12,22,  2,45,  7,66, 14,94,  5,57,  8,82, 16,51, 19,54, 10,25,  9,97,  0,55, 18,38, 
     8,51, 10,21,  0,36,  7,95, 17,68,  2,90, 16,76,  5,84,  3,37,  9,38, 18,30, 11,22, 13,28, 19,90, 15, 6, 12,91,  1,73,  4,93, 14,55,  6,38, 
    19,78, 17,17, 16,41,  3,69,  8,94,  6,83,  9,24, 10,55,  0,62,  5,27, 18,24,  4,70, 15,50,  7,21, 14,51, 12,19, 13,20, 11,59,  2,45,  1,22, 
    19, 2, 14,85, 10,58, 15,47, 12,66,  0,81,  8,44, 17,41,  6,18,  9,85, 16,56, 11, 2,  4,81,  7, 1,  5,36, 13,51,  3,25,  2,11, 18,21,  1, 1, 
    10,87,  6,65,  3,11,  4,85,  0,37, 11,26, 12,89,  1,27,  8,10,  7,13,  2,62, 17,30, 16,24,  5,79, 15,98, 18,61, 19,65, 14,29,  9,27, 13,14, 
    15,14, 11,47,  6,11, 12,29,  9,18, 18,39,  7,21,  5,80,  1,41,  0,62, 17,72, 19,49, 10,52,  8,83,  4,36,  3,31, 16,51,  2,57, 13,33, 14,47, 
     2, 9, 11,96, 13,42, 10,39, 12, 6,  3,67,  5,59,  1,91, 16,81, 14,90,  8, 7, 18,49, 15,96,  6,51,  9,38,  4,56, 17,47,  0,95, 19,23,  7,18, 
     9,78, 17,96,  8,79, 14,35, 11,98,  0,10,  7, 1,  6,82,  1,56, 19,31, 10,88, 12,52,  5,96, 15,75,  3,54,  4,12, 16,63, 18,44,  2,43, 13,19, 
    11,35, 17,73,  4,40, 18,52, 12,92, 19,77,  9,57, 16,66,  6,36,  8, 7,  1, 6,  0,69, 15,84,  5,54,  7,53,  3,97, 13,38, 14,93,  2,28, 10,66, 
    11,99,  6,88, 10,17,  5,33,  7, 3,  2,58, 19,22,  9,75,  0,57,  8, 5, 14,22,  3,11, 16,76, 13,93, 15,11, 12,63, 18,89,  1,96, 17,15,  4,60, 
     4,48,  5,96, 14,13, 12,45, 11,77,  8, 9, 15,60,  7, 8,  2,87, 16,29,  6,56, 19,39, 17,82, 18,34,  9,20,  0,58, 10,41,  1,48, 13,56,  3,41, 
    17,15,  6,77,  5,47,  8,60, 15,51, 19,51, 12,56, 14,30,  7,50, 11,39,  3,25, 18,90,  2,14, 10,67,  4,13, 16,42, 13,28,  9,36,  1, 7,  0, 2, 
     9,12,  6,68, 15,91,  5,12, 14,73, 10,74, 19,76,  2,59,  3,60, 16,20,  0,83, 17,88, 12,25, 13,77,  7,65, 18,44,  1,30,  8,84,  4,28, 11,63, 
     5,32,  2,11, 12,76,  8,27, 13,12, 11,67,  1,33,  3,17,  0,12, 15,58, 18,56, 16,31,  7,96,  9,89,  6,28, 17,33, 14,75, 19,99, 10,40,  4,74, 
    16,90, 14, 1, 12,68,  7, 4,  2,95,  5, 9,  3,61, 13,45,  4,63, 10,35, 11,81, 17,77, 15, 9,  6,53,  9,44,  0,59, 19,96,  8,76, 18,94,  1,20, 
     1,43, 14,79, 19,60, 12,98,  7, 3,  8, 4, 17,48, 10,72,  0,41,  3,90,  6,94,  2,11, 11,30, 13,10,  5,41, 15,79, 16,66, 18,11,  4,87,  9,68, 
    10,91,  9,65,  3,80, 12,50,  2,66,  8,35, 19,70,  0,98, 15,80, 17,47,  5, 9, 16, 7,  4,67,  6,22, 11,95, 13,11,  1,56, 14,25,  7,64, 18,57, 
    15,23,  3,75,  4,12, 10,51,  1,45, 16,23, 18,52,  5,77, 14,76,  8,48, 13,52,  6,53, 11,53, 17,18, 19, 3, 12,18,  7,56,  0,98,  9,71,  2,25, 
    12,64,  6,20,  8,89, 11,27, 17, 5,  1,47,  4,92, 15,23, 13,78,  5,64,  2,77, 14,12, 18,49, 16,40,  7, 3,  9,60, 10,54,  0,37, 19,15,  3,16
  };
  // Taillard 50x20 instance 9
  const int tai_50_20_9[] = {
    50, 20, // Number of jobs and machines
    10,20,  1,22,  2,38,  7,33, 17,24,  4,72,  3,75, 18,61,  0, 2, 14,17,  6,67, 19,50, 15,65, 16,77, 13,65, 12,35,  5,68, 11,76,  9,98,  8,81, 
    11,73,  3,86,  7,26, 18,42, 12,50,  5,13, 13,92,  6,15,  4,77, 14,74, 10,48,  2,54,  1,46, 16,60, 19,83,  0,78, 15, 1,  9,91,  8,33, 17,60, 
    14,67, 19,63,  5,54, 11,65, 16,29, 12,51,  1,74,  8,16,  6,11,  4,97,  2,87,  9,66, 18, 4, 10,10, 17,52,  0,85, 13,60,  3,64,  7,40, 15,49, 
    10,31, 16,73, 15, 2,  2, 4, 14,87,  1,81, 11,90,  7,92,  5,27, 18,51, 12, 4,  0,42,  4,21,  8,77, 17,71,  6, 5,  9,77,  3,81, 19,39, 13,32, 
    19,18,  8,13,  4,89, 14,45, 12,59, 13,90,  2,86,  9,71,  3, 5,  0,54,  1,42, 10,60, 17,32,  6,16, 18,86, 11,70, 16, 7,  7,16,  5,81, 15, 4, 
     0,59,  9,55, 11, 3, 18,43,  3,26, 15,19,  2,57, 17,75, 10,35,  8,40, 13,17, 12, 3, 14,96,  4,82,  5,26,  1,23,  6,97, 16,23,  7,65, 19,28, 
     0,83,  8,81, 12,19,  5,54, 11,86, 16,94,  7,49,  2,38, 15,38,  3,33, 10,93, 18,63,  6,51, 13,45, 14,68, 19, 4, 17,29,  1,53,  4,85,  9,21, 
    10,46, 15, 8, 19, 5, 17,29,  6,32,  1,34, 14, 3,  0,10, 16,45,  7,70,  4,77, 18, 1, 13,62,  3,79,  2,50, 12, 2, 11,18,  9,89,  8,86,  5,21, 
    11,17,  6,47,  7, 7,  1,20, 17, 9,  9,92, 12,23, 18,76,  0,71,  8,20,  4,21, 15,72, 14,71,  5,92,  3,92, 10,17, 19, 9, 16,38,  2,23, 13,34, 
    13, 6, 18,74,  8,29, 14,99, 11,80, 16,27, 10,43, 15,36,  0,31,  9,63,  1,34,  4,26, 19,98,  7,47,  3,69,  2, 5,  5,55, 12,22, 17,15,  6,19, 
    18,77,  8,12, 14,96,  2,84,  5,22,  1,16,  6,59, 13,94,  0,69, 17,68, 15,81,  3,13, 16,45, 11,61, 10,27, 19,26,  7,12,  4,86,  9,82, 12,85, 
     6,27, 18,81, 16, 2, 12,29,  0,61, 17, 2,  1,40, 11,78, 15,65,  7,68,  4,39,  2,68, 10,39,  8,63, 19,59, 13,79, 14,96,  5,11,  9,76,  3,48, 
    19,95, 13,62, 18,10, 11,68,  6,42,  3,61,  5,73, 14,51, 16,81,  4,75,  7,37, 15,96, 10,65, 17,45, 12,44,  8,43,  1,19,  9,44,  0,94,  2,56, 
     1,23, 15,70, 17,24, 14, 4, 10,86, 13,90,  4,86,  9,83,  0,75, 18,46, 11,92, 12,94, 16,23, 19,29,  8, 6,  2, 3,  7,33,  5,12,  6,51,  3,15, 
     5, 7, 15,55,  3,26, 14,30,  0,79,  4,58, 12,86,  1,70,  9,68,  6,98,  8,26, 11,32, 10,98, 17,35,  2,37, 19,13,  7,85, 13,85, 18,14, 16,34, 
    13,21, 19,68, 18, 1, 12,76, 11,44, 14, 4, 17,48,  1,39,  2,73,  6,81,  0,50, 15,97,  7,41, 10,79,  3,10,  9,73,  5,36,  8,28,  4,64, 16,31, 
    16,76,  5,55, 14,41,  7,46, 17,80,  0,99,  3,99,  1,59,  6,14,  2,86, 12,69,  8,24,  9,54, 11,34, 13,32, 19,98, 10,74,  4,67, 18,24, 15,56, 
    15,14,  1,32, 11,77, 17,81, 19,35,  0,38,  8,38, 18,38,  2,61,  6,87, 14,70, 16,79,  9,25,  5,46,  7,88,  3,18, 12,76,  4,98, 10,75, 13,23, 
     5,61,  1,92, 17, 6,  3,58, 18,26, 10,26, 12,36, 19,89,  7, 9, 15,35,  0,54,  2, 5,  6,72, 11,91,  9, 4, 14,61,  8,75, 13,42,  4,85, 16,45, 
    12,46, 19,89,  6,49, 13,86, 11,14,  5,62, 14,53,  8,59,  7,27,  2,86, 18,10,  0,87,  4,99, 17, 4,  1,84, 15,62, 10,74,  9,30, 16,68,  3,86, 
    12, 1, 13, 3, 11,84,  0,78, 10,42, 15,73,  8,98,  7,91,  3,70,  9,53,  5,57, 14,36, 18,76,  1, 4,  6,76, 19,22, 17,70,  4,67, 16, 6,  2,16, 
    12,80, 18,91, 13,24,  6,36,  2,44, 14,81,  7,78, 10,65,  3, 2, 17,91, 19,62, 15,41,  8,40, 11,96,  1,64,  0, 5,  9,81,  4,71, 16,31,  5,71, 
     9, 3, 14,68,  5,55, 17,13,  7,83, 10,13, 16,12,  1,98,  8,86,  6,10, 11,70, 12,59,  2,89,  0,74,  4,83, 19,30,  3,56, 15,99, 18,68, 13,67, 
     6,35, 19,74, 14,59, 13,17, 11,56,  8,64,  5,67,  4,77, 12,84,  2,10,  9,35,  0,79, 10,12, 16,91, 18,36, 17,84,  7, 1,  1,41,  3,98, 15,81, 
     4,32,  2,10, 11,39, 12,58, 14,24,  8,56, 17,83,  0,77,  9,78, 18,99, 19,95,  3,67,  5,81, 13,15, 16, 5,  6,35,  1,10, 10,24, 15,97,  7,44, 
     8,55,  9,41, 17,99,  2,27, 19,94, 14,12, 13,33,  4, 3,  0,71, 11, 5, 15,88,  6,25,  5,66,  3,80,  1,39, 16,90,  7,79, 12,56, 18,60, 10, 5, 
    16,69, 12,41,  2,25,  0,82,  1,32,  3,70, 13,38,  4,22, 14,95,  9,98,  5, 6, 11,64, 15,52,  8,46, 19,71, 17,41, 18,40,  6,35, 10,22,  7,33, 
    19,14, 14,38,  2,32, 16,38, 17,64,  5,67,  8,13, 12,74,  6,74,  3,67, 13,39, 10,53,  7,55, 11,15,  9,93,  1,37,  0, 7, 15,53,  4,54, 18,39, 
    17,84, 18,77,  7, 6,  1,45,  3,91,  9,77,  6,12, 16,96, 14,24, 12,90, 11,54,  0,52,  5,49,  2,97,  4,80,  8,84, 13,44, 15,37, 19,79, 10, 5, 
     4,69,  9,28, 13,32,  6,68,  5,59,  8,23,  2,86, 16,54,  7,41,  0,80, 12,57, 11,45, 18,19, 17,38, 19,89,  1,21,  3, 5, 14,64, 10,65, 15,16, 
     4,57,  6,47,  9, 1, 11,79,  8, 9, 10,66, 13,84,  7,21,  5,35,  2,97,  0,83,  3,45, 14,18, 12,21, 19,19,  1,37, 16,93, 17,83, 18,46, 15,54, 
     6,46,  0,12,  4,44, 12,75, 19,32, 11,15,  2,49, 16, 4,  1,29, 17,76,  8,65, 15,39, 18,73, 14,54,  9,63,  7,78,  5,18, 10,91,  3, 3, 13,93, 
     9,98, 14, 7,  2,47, 18,56, 15,37,  8, 3, 17,17,  0,94,  5, 9, 16,19, 11,91,  4,58, 10, 8, 13,32,  3,60,  6, 5,  7,64, 12, 3, 19,56,  1,98, 
     2,93,  9,63,  7,40,  6,88, 14,20, 17,11, 11,16, 12,55, 18,61,  8,81,  1,17, 10,16,  4,45, 16,46, 13,85, 15,89, 19, 1,  5,22,  0,59,  3,55, 
     7,76, 18,86,  0,83, 10,26, 17, 4,  2,61, 12,72,  4,81,  3,44, 19,48,  8,17, 13,72,  1,14, 11,46, 14,46,  6,80,  9,74, 16,11, 15,57,  5, 2, 
     1,13, 16,49, 17,76,  8,41,  9, 3, 11,13,  3,95,  2,98,  6,29,  7,48, 18,60,  5,67, 19,28, 15,64,  4,36, 12,47, 14,78, 10,83,  0,32, 13,88, 
     3,58, 10,74, 14,41, 19,75, 12,45, 11,19,  0,51,  4,39,  8,94, 13,41,  7,62,  9,75, 15,36,  1, 5, 16,80,  5,47, 17,36,  6,23,  2, 6, 18, 3, 
    15,92,  1,64,  8, 1,  5,65, 17, 2,  2,87, 11,45, 18,73, 16,30, 19,67, 12,68,  6,33,  3,35, 14,65, 10,50,  9,93,  4,36, 13,18,  7,39,  0,54, 
     5,40,  2,11, 13, 8, 14,29, 15,26, 10,65, 16,26,  8,17, 19,52, 17,24,  1,87,  0,67,  7,88,  4,83,  6,36, 12,30, 11,97,  9,19, 18,97,  3,68, 
    15,64,  0,61,  7,59, 16,65,  9,93, 19,80,  5,83, 12, 5,  2,50,  1,42, 17,49, 13,69, 18,86,  6,14,  3,84, 10,41,  8,31, 11,67,  4,88, 14,20, 
    15,45,  6,22,  3, 2,  2,83,  8,61, 18,55,  5,85, 19,72, 11,37,  9,56,  0,73, 13,65, 16,19,  1,42, 17,51, 10,88, 14, 2, 12, 1,  4, 1,  7,30, 
    10,74, 12,28, 19,87, 18,91,  8,68,  2,27,  1,42,  0,99,  6, 7,  4,23,  5,27, 16,16, 15,86, 14,61, 13,28,  9,62,  7,87, 17,72,  3,93, 11,67, 
    11,77, 16,20, 17,53,  1,52,  0,14,  9,57,  4, 2,  3,78,  6,63, 10,12, 18,10, 14, 1,  7,59,  5, 1,  8,67, 12,53, 15,76,  2,17, 13,80, 19,12, 
     4,74, 13,86, 17,78,  6,55,  8,75, 19,37, 12,36,  0,49,  5,50, 16,93,  1,52, 10,77,  7,83,  9,29, 18,84,  2,60,  3,92, 15,61, 14,31, 11, 3, 
     6,76, 18,26, 10,38, 19,61,  2,67,  0,60, 15,40, 11, 7,  4,20, 12, 7,  8,97,  1,76,  7,18, 14,83, 17,39,  9,64,  5,43,  3,39, 16,79, 13,79, 
     7,61, 19,73,  0,91,  1,42,  3,70, 13,73, 17,34,  2,81,  8,19, 14,64,  4,54, 16,96, 12,89,  5, 3,  9,55, 10,94, 11,93,  6,24, 18,10, 15,99, 
     2,73, 18,10, 19, 6,  0,67, 14,22, 11,50, 13, 4,  4,29,  7,34,  5,15, 15,46, 12,32, 17,66, 10,28,  9,67,  3,63,  1,60,  8,39, 16,65,  6,19, 
     2,94, 12,77, 17,36, 18,97, 16,38,  5, 8,  6,71,  8,83, 15,43,  9,86, 19,33, 13,94, 10,30,  7,33,  4,89,  3,37, 11, 1, 14,35,  0,94,  1,32, 
    10,24,  7,67, 16,61,  5,41, 12,18,  0, 4, 14,91,  3,73,  1,33, 11,96, 13,59, 18,63, 15,40, 17, 5,  9,10,  4,45, 19,27,  8,53,  2,24,  6,81, 
    14,31,  8,99, 12,14, 11,64, 19,89, 17,43,  3,93, 10,32,  2,32,  9,26,  0,22, 13,33, 16,89, 18,41,  5,40,  7,87,  6,26,  1,98, 15,53,  4,98
  };
  // Taillard 100x20 instance 0
  const int tai_100_20_0[] = {
    100, 20, // Number of jobs and machines
    11,83,  7,59, 18,49, 15,84,  0,35,  8,68, 19,58,  6,66, 13,44, 14, 2, 17,63,  2,25,  4,70,  1, 1, 16,84,  3,56,  9,35,  5,46, 12,81, 10,58, 
     0,11, 14,67,  7,45,  1,86, 11,91, 16,60,  6,61,  4, 6,  8,95,  5,18, 13, 7, 18,73,  9, 3, 17,92, 15,30, 12,93, 19, 7, 10,37,  2,14,  3, 9, 
    15,34,  4, 5, 10, 2, 14,72,  5,26, 13,55,  9,24, 19,61,  8,28, 16,98,  7, 3, 12, 1, 11,28,  1,92,  3,39,  6,26,  0,91,  2,41, 18, 6, 17,92, 
     0, 9,  5,77, 11,47,  7,45,  6,82, 10,20,  4,86, 18,88,  3,18,  8,83, 14,22,  9,73, 17,92,  2,84, 19,68, 13,55,  1,65, 16,10, 15,64, 12,60, 
     3,62, 10,75,  1,29, 13,81, 18,89, 19,34,  5,34, 11,24,  6,98, 17,95,  8,60, 14,23,  9,54, 16,72,  4, 3, 12,82,  2,41,  0,49, 15,97,  7,49, 
     3,80,  1,95, 17,51, 16,96,  2,24,  5,15, 14,90, 12,73, 11,99,  7,68, 13,96, 19,67, 15,96, 10,56,  0,23,  6,38, 18,97,  8,44,  9,41,  4,83, 
    17, 8, 13,92, 18,66,  1,59,  9,31,  8,63, 11, 2,  4,13, 16,32,  5,39,  3,53, 10,24,  2,85, 12,25, 14,53,  0,90,  6,49,  7,42, 19,54, 15, 1, 
    19,10, 14,68, 16,86, 17,26,  7,64,  9,30, 11,78,  2,71, 15,95,  3,17, 18,92,  5,30, 12,41,  6,54,  1,12,  4, 4,  8,17, 13,60, 10,54,  0,83, 
    10,24,  3,23,  5,46,  8,39, 14,32,  4,40, 16,52,  2,36, 17,83, 15,23, 18,18,  7,92, 12,95,  9,67, 19,31, 13,25,  0,69, 11,46,  1,51,  6,93, 
    15,54,  2,81,  7,37, 11,52, 12,66,  4, 2, 17,96,  8,95,  0,44, 10,91, 14,83,  5,67,  9,22,  6,44,  3, 1, 19,21, 13,45,  1,37, 18,71, 16,50, 
    16,52,  4,19,  1, 8, 17,85,  5,64, 12,32, 15,22,  0,37, 14,54, 13,96, 18,92,  2,58, 19,16, 11,86,  3,57,  7, 3,  8,68,  9,94, 10,17,  6,30, 
    15,29,  6, 8,  7,89,  3,33, 19,14, 13,99, 14,84,  0,78, 12, 5, 11,76, 17,61,  2,95,  4,21, 16, 9,  1,95, 18,56, 10,27,  8,75,  9,42,  5, 5, 
    10,57,  0,30,  7,52, 13,90,  4,58, 17,10,  2,43, 15, 7, 14,80, 12,16,  5,10,  9,92, 18,56, 16,21, 19,76,  3,15,  1,95,  8,10, 11,64,  6,68, 
     8,46,  2,77,  7,23, 16,33, 18, 4, 12,66, 11, 5, 19,52, 15, 2,  4,34, 14,24,  9,89,  3,46, 17,35,  6,10,  5,24, 10,53,  1,98, 13,19,  0,49, 
    18,91, 11,95, 13, 1, 16,90, 17,75,  6,59,  0,36, 14,57, 12,73,  3,39, 15,82,  2, 7,  8,27, 19, 1,  4,39,  7,21, 10,70,  5,14,  9,12,  1,69, 
    13,33, 12,13,  9,76,  0,33, 17,65, 15,24,  1,50, 19,39,  6,41,  7,70,  5,47, 18,89, 14,32,  4,49, 16,71, 11,31,  2,13, 10,68,  3,65,  8,93, 
    13,55, 18,81,  9,36,  4,28,  1,94,  8,85, 17,20, 16,94,  5,77, 14,20, 19,74, 15,32,  3,68, 10,24,  7, 1,  2,75,  6,16,  0,79, 12,82, 11,35, 
    13,78, 10,89, 12,63,  0,41, 19,19, 14,68, 18,87,  8, 6,  5,51,  3, 1, 11,48,  1,75,  9, 5,  2,47, 16,92, 17,45,  6,52,  7,21, 15,27,  4,84, 
    10,90,  4, 4,  3,70, 17,36, 13,46,  1,78,  2,60,  0,67, 12,39,  6,71,  7,61, 15,75,  8,13, 16,69,  9,76, 11, 4, 18,54,  5,97, 19,29, 14,59, 
    18,44,  6,89,  9,36, 13,25, 19,32, 14,77,  3,72,  5,39, 12,81,  1,58,  4,49, 17,56,  7,22,  8,46,  2,15, 10,58, 16,53, 11,27, 15,56,  0,11, 
    15,25, 13,66,  1, 4,  4,23, 19,78, 14,29, 18,16,  5,69,  8,26, 16,68, 10,95, 12,56,  6,31, 17,10,  7,83, 11,74,  3, 3,  2, 8,  0,24,  9,68, 
     9,19, 17,29,  5,94, 18,24,  8,86, 12,16, 14,64, 13,39,  0,20, 10,57,  6,15, 11,34,  1,68,  7,82,  4,57,  3,13, 19,57, 15,86,  2,72, 16,33, 
     1,46,  7,84, 17,16, 11,12,  9,22, 14,73, 18,31,  2,90, 15,57,  0,94,  4,23, 19,51, 10,17,  5,73, 12,23,  6,34, 16,48,  8,83,  3,32, 13,35, 
    19, 6, 10,32, 13,10, 18,62,  7,74, 12,82, 11,51, 15,51,  3,51, 16,71,  9,28,  4,27,  0,87,  6,22,  2,33, 14,45, 17,44,  5,96,  1,81,  8,53, 
     6,43,  9,42, 17,66,  2,75, 11,65, 16,83,  4,64,  1,21, 13, 6,  0,53, 10, 1, 12,67, 18,80, 15,25,  5,37, 14,71, 19,17,  8,48,  3,43,  7,12, 
    10,90,  1,67, 14,42,  7,71, 15,13,  4,98, 13,24,  2,77,  5,69, 17,90,  0,80,  8,73, 18,54,  6,13, 12,29, 11,25,  9,62, 16,11,  3,44, 19,13, 
     8,73, 12,67, 13,52,  7, 6,  5,30,  6,25, 19,73, 11,86,  3,53,  1,19,  9,84, 15,52, 17,80, 16,63, 10,21,  4, 2, 14,78,  2,29, 18,44,  0,90, 
    17,99, 18, 6,  1,72,  8,67, 16,69,  7,50,  5, 2,  0,53, 11,31,  9,61,  4,81, 15,94, 12,23, 19,89,  2,21,  3,75, 13,99, 14,51,  6,68, 10,97, 
     6,48,  8,70,  7,62,  0,60, 16,74,  5,98, 18,26, 13,96,  9,10,  1,68,  2,36,  3, 6, 14,24, 17, 4, 10,12, 19,29, 15,46, 12,79,  4,28, 11,79, 
    10,54, 15,41,  7,61, 13,56,  9,42,  2,59,  8,95,  6,46,  1,43, 17, 1, 11,49, 19,42,  5, 2, 12,32,  4,55, 14, 2,  3,57, 16,60,  0,76, 18,40, 
    17,79,  1,43,  4,79, 11,50, 13,51, 14,23,  9,12, 19,38,  6,80,  7,65, 15,42, 10,91,  0,76, 18,74,  8,36,  5,27,  3,47,  2,44, 12,10, 16,69, 
    10,33,  7,65, 13,93,  8,44,  9,80,  3,83, 12,54, 15,91,  6,87, 17,37, 19,71, 18, 3,  4,33, 14,38, 16,80,  2,91,  1,79,  0,55, 11,90,  5, 5, 
    14,76,  8,18,  7,19,  5,57, 13,71, 10,86,  6,59,  2,17, 18,21, 16,42, 15, 2,  9,51,  3,84,  4,22, 11, 7, 19,17,  0,52,  1,84, 17,16, 12,28, 
     9,23, 13,48, 16,68,  6,91, 10,76, 19,92, 12,39,  4,12, 17,15,  3,42,  5, 5, 18, 5, 11,85, 15,78,  7,75,  1,81,  0,51,  8,35,  2,70, 14,87, 
     9, 2, 12,37,  6,30,  8,14, 11,44, 10,68,  3,22,  4,24, 15,13,  7,80, 14,50, 17,89,  1,12, 18,45, 19,36,  2,30,  5,28, 16,56, 13,58,  0,60, 
    12,92, 13,34, 11, 2,  1,35, 10,82, 14,60, 19,76,  7, 3,  6,89,  9, 8,  8,97,  2,23,  5,26, 18,49,  0,66, 17,37, 15,55,  3,72,  4,70, 16,63, 
     1,72, 15,41, 12,53, 18,83, 10, 4,  9,68, 13,40,  3,80,  8,17,  7,90,  5,57,  2,81, 19,24,  0,26, 14,35,  6,13,  4,38, 17,59, 16,46, 11,38, 
    14,72, 11,65,  2,50,  9,92, 10,88, 13,11, 15,83, 18,87,  3,42,  7,57, 17,87, 16,83,  5,43,  4,98,  1,47, 12,74,  6,61,  0, 3,  8,81, 19,18, 
    14,13,  1,44, 13,84,  8,19,  6,69, 10,23,  7,32,  0,30,  9,26, 19,64, 16,55,  4,27, 15,52, 17,17,  2,97, 18,21,  3,73, 12,45, 11,31,  5,49, 
     2,38,  5,80, 16,48, 18,54,  0,54,  7,38, 13,39,  3,49,  4,29, 19,98,  8,86, 12,20, 17,23, 15,22, 14,81,  6,38,  1,98, 11,96, 10,90,  9,14, 
     9,70, 12,48,  6,93, 19,86,  2,21, 11,97,  1, 7, 17,68, 13, 5,  8,13,  7,26, 15,79,  4,94, 18,25, 10,50, 16,31,  0,37,  5,83,  3,41, 14, 7, 
    14,93, 11,75,  3,33,  9,46,  6,14, 17,17,  4,70, 13,63, 19,38, 16,63, 10,45,  0,99,  7,41, 15,12, 18,64, 12,34,  5,12,  1,54,  2,66,  8,31, 
    12,60, 18,97,  6,95,  7,41, 13,71,  1,88, 19,91,  0,92, 10,99,  9,52, 15,69, 17,79, 11,30,  8, 3,  5,55,  3,59, 16,88,  2,28, 14,71,  4,73, 
    18,53,  1,97, 10,53,  4,29,  7,64,  2,19, 14,15, 13,79,  3,99, 16,39, 12,72,  5,82,  6,77, 11,25, 19,96, 15,92, 17,98,  8,79,  9,31,  0,19, 
     7,56,  4, 6,  6,63, 16,50, 12,67,  1,80, 18,60,  0,36,  8,10, 17,14, 19,44,  5,81, 13,54,  3,20, 10,67,  2,25, 14,49, 11,78,  9,75, 15, 3, 
    16,83,  4,73,  1,22,  2, 4,  0,30, 17,27,  6,76,  8,54,  9,48,  3,82, 12,89, 10, 8,  7,80, 18,26, 13,98, 11,97,  5,10, 19,66, 14,76, 15,77, 
    17,75,  3,99,  2,70,  4,75,  8,16, 19,69, 16,52,  6,21, 15,23,  1,28, 10,56,  5, 5,  7,78, 13,72, 12,56, 18,23,  0,30, 14,51,  9,27, 11,38, 
    16,96, 17,25,  4,56, 13,67, 18,77, 15,61,  7,55,  5,87,  0,20, 14,60,  8,40,  6,23,  9,17, 10,24,  1,37,  3,52, 12,46, 19,96, 11,19,  2,46, 
     3,34,  8, 9,  7,13, 15,74,  9, 4, 12, 5, 17,41,  4, 3,  2,61,  5,28, 11,48, 14,63,  6,37,  1,58, 13, 6, 10,91,  0,35, 16,95, 18,98, 19,90, 
     3,50,  0,26, 17,10, 10,75,  4,93, 13, 9, 18,92,  7,55,  9,63, 15,51,  2,33, 14,60,  5,91,  6,51,  8,96, 12,18, 19,75, 11,73, 16,50,  1,62, 
    12,28,  8,41,  6,92, 11,57,  3,10, 16, 5, 10,85, 19,41,  1,98,  7,12, 15,62,  9,39,  2,53, 14,51, 13,81,  4,56,  0,39, 17, 5, 18,90,  5,65, 
    17,83,  3,57,  2,47,  7,37,  1,10, 10,13,  6, 1, 14,98, 19,35, 11,17, 13,59,  8,86,  4,16,  0,66,  5, 1, 18,19, 16,10,  9, 1, 15,59, 12,67, 
     1,24,  8,69,  7,87, 11,87, 15,95, 14,44, 10, 8,  6,42,  9,21, 13,39, 17,75,  5,79,  4,73, 16,21, 18, 8,  3,87,  2,66, 19,25, 12,58,  0,91, 
     4,45, 11,29,  8,95, 17,55,  3,62, 13,97,  9,77,  2, 2,  0,43,  1,68,  5,94, 16,20, 15,61, 10,78, 12,65,  6,70, 14,42,  7,69, 18,59, 19,38, 
    18,79,  6,55, 17,65, 11,61,  3,85, 13,36,  9,31, 15,80,  8, 1, 12,62,  5,73, 10,31,  2,62, 19,13, 16,27,  7,28, 14,52,  0,71,  4,76,  1,73, 
    13, 8, 19,75,  7,10,  9,65,  2,39, 11,50, 14,15,  0,46, 16, 6, 15,16,  1,87, 10,31,  4,78,  5,33,  3,81, 17, 1, 18,86,  6,16,  8,64, 12,55, 
    11,45,  8,51, 10,66, 16,28, 12,56,  7,42, 15,93,  4, 5, 14,99, 17,16, 19,76,  1,32,  5,29,  9,33,  2,23,  6,65, 18,52,  3,99, 13,91,  0,12, 
     4,57, 17,91,  6, 8, 10,79, 16,61,  7,54,  2,47, 19,93, 11,39, 13,62,  3, 6,  1,63,  8,28,  5, 4,  0,84, 12,60, 15,70, 18,22, 14,76,  9,61, 
    17,97,  2,40,  7,40,  4,86,  9,97, 15,45, 19,31, 16,56,  5,95, 12,30, 11,87,  3,23,  1,71,  0,61,  8,92,  6, 1, 10, 5, 14,92, 18,26, 13,15, 
    12,24,  4,58, 19,62,  5,41, 17,11,  6,17, 14,45, 11,73,  0, 2, 18,57,  8,65,  2,51, 13,75,  7,59,  1,97, 10,88,  3,52, 15, 8, 16,59,  9,71, 
     7,18, 19,68, 12,70,  3,85, 15, 8, 10,96,  2,82,  5,34,  0,76,  9,45, 18,99, 17,39,  1,26,  8,76,  6,45, 11,77, 16,97, 14, 3, 13, 5,  4,53, 
     7,12,  8,60,  4,53,  3,51, 18,17, 19,41, 11,25, 14,90,  6,21,  0,65, 16,34,  5,51, 17,60,  9,51, 10,67, 12,97, 15,40,  1,24,  2,22, 13,29, 
    17,29, 13,36, 19,91,  6,68,  3,64, 18, 2, 12, 6,  9,65, 16,44,  8,50,  5,21,  1,64,  7, 6, 14,99, 11,44,  4,13, 15,42,  2,12,  0,15, 10,42, 
    16,32,  2,59, 11,29,  7,75, 12,54, 14,93,  9,41, 17,10,  0,63, 10,70,  4,84,  1,23,  6, 9, 13,39, 19, 9,  8,23,  5,68,  3,65, 15, 9, 18,64, 
    16,83,  9,26,  7, 3, 18,37, 11,66,  8,63,  4,10,  3,83,  2,80, 15,45,  1,16,  0,82, 19,12,  5,85, 10,30, 12, 2, 13, 7, 14,13, 17,35,  6,46, 
     4,21,  1,29, 17,50,  6, 5,  8, 6, 13, 6, 19,88,  0,50, 14,50,  9,65,  3,68, 12,71, 10,45,  5,51, 18,52,  7,53, 16,37, 15, 1,  2, 9, 11,41, 
    12,88,  6,55, 16,65,  0,97,  2,94, 19,73,  4,47,  8, 8,  7,74, 14,43,  5,98,  3,13, 18, 8, 11,32, 15,53, 10,70, 13,70, 17,37,  1,52,  9,21, 
    15,41, 18,31, 11,53, 13,34, 19,88,  7,96,  1,69,  2,92, 16,35,  8, 3, 14,53, 17,92,  4,54,  9,21, 12,90, 10,68,  3,23,  6,16,  5,39,  0,25, 
     2,35, 13,49, 11, 2, 12,34,  4,89, 17,36,  7,81, 14,29,  6,99,  8,96,  5, 5,  3,28, 15,97, 16,11, 19,79, 18,91,  9,65,  1, 5, 10,94,  0,94, 
     9,84,  6,34,  7,56,  8,77, 14,31,  2,97,  3,80,  4,88, 18,33, 16,73, 15,71, 12,37,  0,78, 13,81, 10,71, 19,50, 17,46, 11,90,  5,22,  1,25, 
     9,78,  2,93, 12,11, 15,53,  8,57, 13, 3,  1,52, 19, 4,  5,99, 16,24, 14,10,  7,41,  4, 9, 10,27, 11,19, 18,90, 17,55,  6,43,  0, 5,  3, 9, 
     6,20, 18,46, 11,57,  8,63,  4,70,  7,64, 19,55,  9,30,  5,26,  2, 6, 17,70, 15,48,  0,91,  1,97, 16,77, 10,94, 13,88,  3,75, 12,52, 14,77, 
    12,35, 16,41,  8,48, 19,97, 17,73,  0,73, 14,67,  6,19, 18,72,  1,82,  5,79,  4,45,  9,28, 13,64,  7,22, 15,65,  3,79,  2,14, 11,29, 10,36, 
    10,55,  1,54,  2,86,  8,10,  7,73, 12,32,  4,61, 19,50, 16,33,  5,81,  6,57, 14,99,  9,12,  3,49, 18,37, 17,72, 11,90, 15,60,  0, 6, 13,66, 
     7,35, 14,34, 12,97,  5,23, 10,69, 13,56,  3,38, 16,15,  2,67,  1,75,  4,52, 19,91, 15,55,  0,56, 11,73, 18,80,  9,88, 17,25,  6,20,  8,65, 
     1,65, 13,76, 17, 6,  0,39, 19,91, 12,84, 15,14, 16,13, 10,32, 18,29,  4,23,  6,12,  5,33,  2,59, 14,40,  3,21,  8, 8, 11,82,  7,73,  9,77, 
    13,53,  4,21,  7, 5,  9,41,  8,93, 18,28,  2,37, 19,76, 12,43,  6,32, 17,17, 14,11,  0,72, 16,32,  5,47, 15,68, 11,90,  1,65, 10,63,  3,60, 
    14,91,  3,81,  1,76,  4,44, 17,30,  7,84, 16,54,  0,26,  8,19, 11,93, 10,83,  6, 7, 12,44, 18, 8,  5, 5, 13,66,  9,58,  2,98, 15,81, 19,36, 
    12,33, 13,56, 11,85, 18,15,  9,38,  8,64,  2,24,  4,73,  5,19,  0,26, 14,59,  6,12, 10,14, 17,22,  7,38,  1,65,  3,58, 15,92, 16,40, 19,52, 
    18,92,  5,12,  7,82,  3,41, 19,51, 11,46,  1, 7, 13,30,  9,26,  8,89,  6,62,  2,81, 14,82, 12,85,  4,70, 16,34,  0,15, 17,98, 10,97, 15,48, 
     6,11, 16,10, 10,73, 19,50, 13, 3,  0,16, 15,24,  9,88, 12,94,  8,11, 17,58,  7, 4,  4,66, 18,58, 14,42,  5,69,  2,98,  1,50, 11,25,  3,46, 
    17,87, 15,97, 10,84, 13,49, 11,82, 18, 3,  9,76,  1,77,  5,35,  0,41,  7,56,  4,31, 12,50, 16,56, 14,75,  3,42,  2,37,  8,94, 19, 3,  6,14, 
     2,88,  3,77,  8,64,  1,42, 13, 7, 15,84, 19,16,  7,20,  6,56, 10, 7,  5,47, 14, 2,  0,49,  4,53, 11,65, 17,76,  9,82, 12,12, 16,41, 18,94, 
    15,84,  5,99,  1,61, 11,28, 16,59,  0,64,  4,51,  7,77, 13,26, 18,43,  8,72, 17,63, 12,68,  9, 4,  6,86, 10,87,  3,77,  2, 8, 19,47, 14,38, 
     6, 7,  0,40,  3,96, 14, 7, 12,82,  9,61,  8,89, 16,76,  4,35, 18,97, 13,95,  1, 5, 10, 4,  7,11,  5,83, 19,65, 15,86,  2,14, 11,17, 17,34, 
     2,51, 10,72, 19,74, 13,35, 11,92,  0,53, 17,19,  4,90, 12,35,  9,47,  5,49, 18,34,  1,68,  6,46, 14,80,  8,30,  7,71, 16,31,  3,50, 15,70, 
     7,31, 10,71,  4,45,  6,75, 12,40, 15,87, 16,55, 17,21, 11,18,  9,21,  1, 1,  2, 5, 19,17,  8,83, 18,58, 13,68, 14,66,  0,89,  5,35,  3,67, 
     7,85, 12,13,  3,46, 14,96, 10,90,  2,31, 15,75, 18,88, 16,18,  9,80,  5,48,  1,74, 11,55,  8,68, 19,88,  4,85, 17,55,  6, 6, 13,82,  0,82, 
    17,89,  6,52,  8,29, 11,38,  7,90, 13,70,  0, 4,  5,86,  1,48, 10,59, 16,75,  4,38,  9,37,  3,46, 18,40, 15,23, 19,19, 14, 3, 12,35,  2,71, 
    15,64,  1,60,  7,86, 19,14, 10,61,  6,87,  5,47, 17,83,  2,66, 16,78,  8,90, 14,80, 13,80,  3,58,  4, 8,  9,14, 11,43,  0, 4, 18, 1, 12, 3, 
    17, 5,  5,61, 19,23,  4,57, 16,67,  7,49, 13,88, 10,23, 11,12,  2, 3,  8,29, 14,69,  9,52, 15,13, 18,16,  1, 2,  0,13,  6,50,  3,46, 12,73, 
    17,85, 18,86, 19,48,  7,71,  6,83, 13,17, 15,17,  1,18, 16,34,  9, 7,  8, 2,  5,41, 14,34,  3,37, 11, 7, 12,88,  0,59,  4,56,  2,63, 10,50, 
     8,59,  2,49, 13,81,  6,13, 18,29,  3,19, 19, 5,  7,77,  0,71,  4,11,  1,68,  9,90, 10,62,  5,96, 14,70, 17,31, 12,55, 11,80, 15,55, 16,85, 
    14,85,  7,88, 17,97, 18,58,  4,83,  9,47, 12,84, 13,95, 10,62,  5,72,  6,75, 11,40,  0,52,  8,79, 19,48, 15,53,  1, 5,  3,38, 16,44,  2,28, 
    18, 4,  4,16,  1,82, 13,45,  6,65,  3,35, 16,58,  0,31,  8,45, 14,12, 11,58, 19,51,  9,43,  7,12, 17,48,  2,38, 10,72,  5,93, 12,11, 15,94, 
    16,42, 19,49, 13,84, 18,90,  7,16, 10,78, 17,83,  3,82,  9, 7,  6,99,  0,22,  4,14,  8,63,  1,81, 15,78, 14, 7,  5,31, 12,14,  2,59, 11,33, 
    10,14,  0,67,  1,66,  9,24,  3,75, 18,85, 13,62,  7,59,  8,65,  5,74, 15,58,  2,18, 17,90,  6,81, 11,53, 16,24, 12, 8, 14,17, 19,99,  4,62, 
     4,68,  9,93,  3,19, 10,51,  8,36,  6,85, 12,66, 18, 7, 17,51, 16,89,  7,11, 11,70, 15, 2, 13,11, 19,49,  5,41,  0,68, 14,80,  1,51,  2, 2, 
     5,92, 19,16,  0, 6, 13,22, 16,93,  6,89, 17,29, 10,34, 15,62,  1,19,  3,61, 14,18,  2,91,  8,16,  9,27,  7, 4, 12,69, 18, 2,  4, 7, 11,83, 
     4,16,  7,19, 14,32,  8,39, 13,19, 12,51, 11,11,  1,31, 17,23,  9,72, 19,49,  0,71,  2,33, 16,73,  6, 3, 15,54,  3,37, 10,24,  5,17, 18,60
  };
  // Taillard 100x20 instance 1
  const int tai_100_20_1[] = {
    100, 20, // Number of jobs and machines
     8,53, 10,40,  5,82,  0,42,  7,98,  2,86, 14,50, 12,57,  3,28, 15, 7, 18,13,  9,24, 11,61, 16,54,  4,97, 13,92, 19,86, 17, 5,  6,32,  1,11, 
     3,44,  4,50,  6,39, 10,81, 16,51,  5,92, 13,65, 18,20,  1,75,  0,60, 14,95,  9,28, 15,14,  7,28, 17,54, 12,55,  8,91,  2, 3, 11,28, 19,93, 
     0,52,  8,67, 13,64,  5,83,  2,37,  9,21, 18,10,  1,62, 14,74,  6,86, 17,71, 15,43,  3,24, 19, 5, 12,78, 10,10,  7,40,  4,51, 16,89, 11,54, 
    12,74,  9,86, 14,71, 19,80, 16,21,  2,14,  0,49,  6,16,  1,26, 13,34, 17, 2,  8,17,  7,47,  5,68, 10,21, 15, 8,  4, 8, 11,51,  3,21, 18,71, 
     9,89, 17,50, 12,80, 18,88, 13,16,  8,58, 15,20, 10,32,  6,42, 14,89, 19,79,  4,87,  5,73,  2,74,  0,17, 11,53,  7,81,  3,85, 16,55,  1,57, 
    11,73,  5, 5, 18,26,  0,57,  9,45, 16,40, 19,46, 15,15,  4,79,  7,48,  8,20,  3,69, 17,16, 13,29,  2,48,  1,38, 14, 6, 10,18, 12,62,  6,86, 
     7,57,  3,89,  9,91, 10,96, 13,66, 19,49,  6,88, 14,62,  5,86, 16,80,  0,13,  1,58,  2,81, 18,70,  4,48, 17,61,  8,12, 12,69, 15,76, 11,23, 
     4,30,  5,65,  9,26, 18,60, 10,38,  8,66, 11,76,  0,85, 16,39, 13,96, 12,19, 14,75, 19, 5,  7,91, 15,14,  2,46,  1,27, 17,69,  6,95,  3, 3, 
    17, 1,  4,99,  8,97,  3,17,  7,51, 13,63, 12,82, 14,79,  9,60,  0,61,  5,92, 10,58, 15,13, 19, 2,  2, 3,  1,74,  6,43, 16,34, 18, 6, 11, 1, 
     0,18,  8,77, 19,12, 18,36, 13,64,  7,35, 17,66,  3,86, 15,98,  6,54,  2,11, 14,41,  9,15,  4,87, 16,76,  5,33, 11,85, 12,12,  1, 1, 10,62, 
    10,29, 12,17,  1,21,  6,93,  2,39,  5,48,  0,46, 13,60, 16, 9, 17,61,  3,43, 18,47,  8,47, 15,92, 11, 2,  7,77, 14,58,  4,71, 19,16,  9,50, 
     6,91,  7,42,  5,32, 10,96,  1,58,  3,24, 18,56,  2,63, 15,74, 16, 1,  4,42, 14,84,  0,56, 11,44, 13,58,  9, 1,  8, 5, 19,53, 17,44, 12,52, 
    18,48,  2,16,  4,37, 15,60,  7,54,  5,32, 11,69,  3,25, 13,48,  6,72,  0,77,  9,29, 17,64, 16,10,  1,53, 19,90,  8,71, 14,84, 12,82, 10,93, 
    17,23, 13,28, 14,99, 19,49,  0,79,  4, 7,  9,32,  6,89,  5,12,  1,22,  8,54, 12,88, 18,69,  7,65,  3,95, 11,84, 10,32, 16,64, 15,33,  2,55, 
    15,97,  1,49, 13,11,  8,79, 19,86, 17,67,  6,51,  2,80,  9,29,  0,75, 14,64,  7,59,  4,92,  5,85, 16,92, 18, 3, 10,94,  3,69, 11,34, 12,27, 
     8,75, 12,20,  5,87,  2,67, 19,70,  0,23, 15,93, 14,31,  4,72, 18,16, 11,36,  1, 5, 17,59,  3,75,  9,85, 10,24, 16,29,  7, 5, 13,47,  6, 1, 
    14,24,  6,13, 16,27,  9,21, 11,29, 12,19, 13,91,  4,37,  2,93, 15,76, 19,75, 17,15,  5,70, 18,70,  3,91, 10,57,  7,18,  1, 8,  0,29,  8,73, 
    12,16,  2,15, 13,76, 16, 1, 11,93, 18,87, 19,60, 17,67, 15,27,  7,91,  1,45,  8,28,  3, 7,  0,68,  4,97, 14, 6, 10,50,  6,71,  9,52,  5,99, 
    14,81,  6,53,  9,16, 18,88,  3,16, 13, 3,  2,49, 15,62, 19,26, 17,26, 12,43,  5,31,  1,75,  7,35,  0,72, 10,37,  4,79,  8,94, 11,94, 16, 7, 
    14,93, 19,21,  5,92,  3,60, 12,42, 15, 9, 16,93, 13,68,  9,28, 18,29, 10,45,  1,94,  7,97, 17,78,  0, 5,  4,66, 11,85,  2,39,  8,18,  6,80, 
    11,43,  7,33, 14,21, 13,63,  3,56, 17,90,  9,12,  8,12,  0,79,  5,51,  4,99, 19,98,  6,68, 15, 8, 10,61, 16,41,  2,58, 12, 8,  1,42, 18, 9, 
     7,66, 18,83, 17,38,  0,40,  1,57, 16,62, 12,31,  9,21, 14,88,  6,59,  8,82,  3,96,  4,69, 15,11,  2,49, 19, 1, 13,56,  5,97, 11,21, 10,26, 
     1,53,  5,12, 16,91,  3,86, 17,66,  2,85, 15, 2,  6,78,  4,23,  9,28, 11,16,  8,35, 13,55, 10,35, 14,51, 12, 3, 18,25,  0,54,  7, 6, 19,10, 
     1,10,  0,63, 16, 3, 10,67,  8,82, 14,12,  2,99, 17,57,  6,70,  5,67, 11,82, 13,37,  4,16,  9,47, 15,90, 12,71,  7,74, 19,75,  3,31, 18,69, 
    14,86,  0,17,  5, 1, 18,39, 10,42,  7,35, 17,85, 13,35,  2,93, 19,48,  6,22, 15,76,  3,25, 16,42, 11,65,  1, 3, 12,79,  9,95,  4,59,  8,24, 
    10,83,  7,72, 14,50,  6,93, 13, 8, 11, 7, 15,79,  9,57,  4,95,  8,68, 19,67, 12,15, 16,58,  1, 2,  5,34,  3,95,  0,72, 18,96, 17,30,  2,26, 
     0,11,  5,22, 10,81,  2,63,  1,95,  8, 2,  9,22,  4,81, 12, 2,  6,91, 11, 7, 18,36,  3,27,  7,87, 13,54, 16,24, 19, 4, 17,94, 14, 7, 15,26, 
    18,11, 17,21, 16,46,  8,91, 19,30,  6,18, 10,37, 14,35,  2, 4, 15,64,  7, 9,  9,57,  3,51,  4,41,  0,90, 12,24,  1,28, 11,94, 13,80,  5,32, 
    18,17,  7,40,  6,44, 14,72,  5,41,  8,31, 16,86, 19, 7,  3,60,  2,41, 17,11,  4,36,  1,20, 13,23,  0,81, 15,27, 10,53, 12, 8, 11,96,  9,77, 
    19,39,  1,40, 11,37, 17,70,  7,13,  3,43,  4,36,  6,81, 18,55, 16, 4, 15,60,  9,20, 12,31,  5,66,  0, 9,  8,22, 10, 6,  2,84, 13,94, 14,15, 
     2,52, 13,12,  5, 1, 16,32,  0,52, 11, 6,  7, 9, 17, 6,  9,18, 15,97, 18,82,  6,12, 14,62,  3,43, 19,88, 10, 4, 12, 8,  8,89,  1,49,  4,27, 
    10,69,  4,98,  5,43,  0,94,  8,71,  9,82,  1,42,  6,91, 17,20, 13,52,  7,45, 16,58, 11, 1,  3,19, 19, 7, 14, 9, 12,83, 18,93, 15,62,  2,68, 
    17, 6, 10,78, 12,55,  7,62, 19, 1,  2,33, 15,26,  9,20,  0,21, 11,99,  3, 8,  5,91,  8,76, 14,52,  4,33,  1,15, 16,72,  6,34, 18,34, 13,96, 
    15,49,  2,20, 16,54,  6,34,  4,43,  8,28, 14,59,  1,63, 11,95, 13,32, 17,23, 12,28, 18,47,  7,45,  9,28,  0,46, 19,25,  5,53, 10,18,  3,53, 
     6,41,  4,13,  5,43, 18, 8,  7,98,  9,75,  0,71, 15,60, 17,20,  1,15, 11,11, 12,65,  3,56,  2,89, 10,48, 13,83, 16,76, 19,82,  8,52, 14,26, 
     5,40, 14,20,  0,63,  8,10, 11,23,  6,79,  4,71, 15,12, 16,43,  3,15, 13,62,  2,19, 19, 4,  9,46,  7,70, 18,87, 17,45,  1,24, 10,30, 12,76, 
    15,37,  6,58,  2, 5, 12,29,  1,20,  9,24,  4,40, 17,34,  5,53,  3,72, 10,12,  0,26, 18,12, 14,40, 13,74,  8,29, 16,43, 11,42,  7,12, 19,43, 
     6,62,  9,35,  2,62,  0,20, 10,17, 11,44,  8,94, 18, 6, 13,83,  7,85,  3,14, 14,71, 15, 5, 16, 7,  4,53, 17,43, 19, 2, 12,11,  1,18,  5,89, 
     0,68,  1,94,  7,87,  9,34, 12,10,  2,76,  6,29, 16,41,  5,30,  3,58, 19,34, 10,83,  4,15, 13,30,  8,28, 15,24, 17,40, 11, 1, 18,72, 14,82, 
     2,55, 18, 9,  8,85,  0,74,  3, 6,  5,16,  9, 5, 15,24,  7,12, 11,33,  4,30, 16,34, 10,34, 14,17, 12,75,  6,38,  1,49, 17,78, 19,19, 13,35, 
     7,51,  5,40, 11,28,  2,84,  8,78, 15,93, 12,39, 18,14, 14,57, 17,52,  3,94, 16,10, 10,35,  1,97,  0,93, 19,87,  9,47,  6,27,  4,18, 13,24, 
    15,43, 16, 9,  7,63,  3,11,  9,26,  1,42, 17,53,  0,42, 14, 8,  8, 7,  2,77, 10,97, 19,43,  6,52, 11,32, 13,23, 18,32,  4,50, 12,99,  5,83, 
     3,51, 13,16, 16,83, 10,33,  2,90,  0,24, 15,21, 17,95,  4,11,  5,56,  8,21,  7,37,  9,72, 11, 5, 18,94, 19,28, 14,26, 12,67,  1,52,  6,95, 
     7,82,  8,72,  3,62, 19,61,  1,22, 12,18, 11,66,  2,28,  5,88,  0,48, 16,87, 18,41, 14,78, 17,70, 15, 2,  4,15,  6,13,  9,25, 13,44, 10,62, 
     1,28, 19,24, 12,56,  8,77, 10,21, 14,46, 11,30,  9,89,  2,56, 18,71,  0,23,  5,31, 13,26,  7,76,  6,70, 15,93,  4,86, 17,74, 16,79,  3,57, 
    16,40, 14,72, 10,96, 11,52,  9, 4, 17,17, 13,25, 18,92, 12,67,  2,77,  1,62,  4,11,  0, 3,  8,75, 15,21,  5,79,  3,90,  7,70, 19,40,  6,51, 
     7, 7, 15, 7, 11, 2,  1,61, 13,34,  3,46, 14, 7,  9,22,  4,36, 10,36,  6,11,  8,95,  2,11, 18,69, 12, 8, 19,56,  5,33,  0,69, 17,87, 16,58, 
     8,31,  1,11, 10, 5,  3,15, 12,51, 17,76, 15, 1,  7,51, 16,30,  9,36,  4,17,  5,52, 19,61, 11, 9,  2,80,  6,10, 13,75,  0,66, 18,32, 14,14, 
     9,84, 18,94,  4,61, 14,90, 12, 9,  5,72,  6,66,  8, 5, 17,76, 10,41,  2,87, 15,80,  3,50, 19,45,  1,95, 11,12,  7,32,  0,52, 16,28, 13,75, 
     5,60,  8,21, 14,60, 10,52, 12,42,  0,47,  4,77, 11,93,  2,27,  1,62, 13,24, 15,37, 17, 4, 19,75, 16,56,  9,16, 18,76,  6,64,  7,85,  3,11, 
     3,21, 15,10,  9,87,  5,47, 18,40,  7,45,  1,48, 17,27, 12,63,  0,29,  6,36, 19,25, 11,74, 14,19,  4,20,  8,78, 16,51, 13,32, 10,69,  2,23, 
    15,71, 13, 7,  1,64,  7,53,  0,89,  4,39, 17,86,  9, 2, 19,92,  5, 5, 18,52, 11,63, 16,27, 14,75,  8,46,  3, 3,  6,98,  2,82, 12, 6, 10,70, 
    12,51,  9,89,  8,36,  2,84, 18,95,  1,12, 19,97,  4,75, 10,57,  0,79, 13,91, 17,77, 15,17, 16,86,  6,58, 11,43,  3,15, 14,90,  7,28,  5,45, 
    17, 3, 18,31, 14,71,  8,51, 12, 6,  5,37, 11, 4,  2,81,  1,98, 15,97,  7,58, 16,19, 10,84,  6, 3, 13,96,  9,50,  0,21,  3,86,  4,44, 19,76, 
     5,66, 12, 9, 16,32, 11,78,  0,20,  8,69, 10,46,  4, 1,  7,93,  3,81,  2,18, 13,64, 17,40,  1,39, 14,86, 15,92, 18,69, 19,69,  9,17,  6,11, 
    19,98, 11,80,  2, 8,  3,98, 14,56, 10,47,  1,26, 12,30,  9,49,  6,55, 18,75, 13,66,  5,44,  8,12,  7,60, 17,81,  0,82, 15,38, 16,29,  4,72, 
     9,55,  8,18,  4,32,  1,78,  5,95, 12,40, 16,79, 19,35,  3,41, 10,40, 15,65, 14,89,  2,16,  6,20, 11,86, 18,59, 13,49,  0,86,  7,36, 17,25, 
    12, 6, 16,51,  2,26, 18,59,  3,20,  5,65,  4,50, 11,99, 13,96, 14,62, 10,47,  1,89,  9,39,  6,68, 17,28, 15,70,  7,29,  0,71,  8,94, 19,79, 
    17,51,  2,73,  5,20, 11,26, 16,11, 12,46, 14,35,  3,87, 15,84, 13,95,  4,84,  1,97,  6,50,  9,10, 18,99, 10,97,  7,86,  0,83, 19,27,  8,84, 
    18,58, 13,24,  1,99,  3,80,  0,78, 14,10, 12,53,  6,10, 11,99,  8,85, 15,80, 16, 7,  7,24, 19,66,  9,92, 10,74,  4,98,  2, 9, 17,30,  5,28, 
     7, 9,  6,88, 16,56,  0,23,  2,92,  5,38, 19,88, 15,64,  1,71, 10,59,  8,11, 14,32, 18,71,  4,62,  3,27, 13,20,  9,54, 11,43, 12, 2, 17,73, 
    17,69,  8,47, 19,60, 13,75,  9,13, 16,22,  7,16,  0,60,  3,87, 18,80, 10,33,  2,14,  1,59,  6,99, 11,97, 14,55,  4, 3, 15,40,  5,30, 12,36, 
     1,56, 10, 2,  7,31,  0,70, 15,92, 16,87, 17,49,  9,25, 13, 5,  5,42,  2,66,  3,18,  6, 1, 11,43, 12,32, 14,46,  8,48, 18,20, 19,11,  4,23, 
     9,29, 14,22, 17,17, 18,19, 13,35, 12,28,  4,19, 15,29,  7,80,  3,59,  5,86, 19,95,  0,36,  6,79,  1,82, 11,89,  8,72, 16,27,  2,86, 10, 4, 
     1,76,  9,31,  4,93,  5,64,  7,87,  8,84, 15,62, 13,41, 12, 6, 10,36,  3,12,  0,18, 14,68, 19,96,  2,90,  6,34, 16,67, 18,61, 11,73, 17,64, 
     1,41,  8,28, 16,39, 19,34,  9,36,  2,13, 12,95, 17,12,  0,10, 10,30,  5,34,  3,92, 11,14, 18,15, 15,10,  4,98,  6,75, 13,62, 14,12,  7,88, 
    16, 9,  5,83,  6,93,  9,98,  8,33, 15,81,  7,40, 17,18,  1,96, 10,52,  2,51, 14,99,  0,34, 18,46, 12,30, 13,55,  3,44,  4,32, 11,71, 19,10, 
    15,83, 18,66, 16,20, 19,22,  2,73, 14,76,  6,59,  7,74,  5,23, 12,90,  3,53,  0,11, 11,43, 17,88,  1,76,  8,65,  9,44, 10,52, 13,25,  4,54, 
    12,78, 18,91, 15,41, 11,54,  8,68,  7,60,  3,94,  1, 3,  2,38,  5,22, 17,33, 16,37, 10,76, 14,31, 13,24,  9,46,  4,20, 19,69,  0,53,  6,57, 
    12,80, 10,12,  4,46,  2, 5,  5,20,  7,42,  9,66, 11,32, 19,43, 18,58, 14,63, 17,89, 15,54,  3,79,  0,28, 13,42,  8,90,  6, 6,  1,66, 16,29, 
    15,63,  8,56,  4,43, 14,33, 19,10, 13,53,  5, 2, 17,24,  1, 6,  0,61,  9,14,  3,92, 11,27,  6,82, 10,63,  7, 2, 12,27, 18,25, 16,51,  2,91, 
    12,23,  3,62,  6,44, 10,32, 18,69, 14,86, 11,24, 15,42,  0,31, 17,35,  1,24, 13,34,  4,12,  9,35, 19,51,  5,73,  2, 5, 16,12,  7,52,  8,13, 
    14,53,  4,67,  2,91, 10,63, 17,97,  6,83,  0,51,  7,55, 19,14,  1,78, 11,17,  3,74, 16, 9, 15,63,  8,22, 13,71, 12,41,  9,81, 18,54,  5,46, 
    17,28, 18,16,  9,12,  4,30,  5,97,  2,86,  8, 9, 13,65,  6,51, 15,30, 12,15,  3,41, 14,91,  1,46,  0,18,  7,21, 16,89, 11, 2, 10,77, 19,78, 
    12,91, 11,33,  3,84,  1,79,  0, 4, 18, 7,  8,49, 16,45, 14, 9, 19,69, 13,86, 10,94,  9,90,  4,24, 17,21,  5,15,  2,38,  7,28,  6,26, 15,60, 
    17,94, 11,47,  2,96, 14,70, 19,51, 10,93,  3,64,  6,24,  8,45,  9,36, 13,62,  0,91, 15,18,  1,38, 16,47,  5,98,  7,51, 12,12,  4,26, 18,51, 
    18,50, 19,74,  3,34, 17,45,  1,33,  5,50,  4,69, 10,14,  2,89,  7,86, 12,57, 14,17,  9,80, 15,32,  8,72, 11,33, 13,51, 16,31,  6,43,  0,65, 
    11,14,  0, 4, 13,22, 14,27, 19,58,  3,12,  7,92,  4,27,  6,24, 10,13,  2, 8,  5,55,  8,66, 12,81,  9,12, 17,13, 15,57, 16,77, 18,25,  1,99, 
     7,99, 17,84,  3,72,  9,79,  4,72,  8,47, 19,47, 15,92, 14,49,  2,49,  0,43, 11,33, 13,30, 16,16, 10,35,  5, 2, 12,47,  6,59,  1,44, 18,98, 
    11, 1, 17,75,  9, 2,  1,26, 10,20, 12,54,  4,63,  0,31, 13,44,  6,98, 19,73,  5,41,  2,52,  7,81,  3,30, 14,57, 16,25,  8,70, 15,58, 18,83, 
    15,21,  0,15,  2,60,  9,30,  3,50,  7,31,  6,17, 12,34, 17,15,  4,98, 19,77, 14,57, 18,58,  5,40, 11,21,  1,25, 16,41,  8,78, 10,44, 13,26, 
    12,14, 16,31, 10,86, 13,68,  7,69, 19,71,  2,44,  6,10,  1,76, 15,82, 11,54,  0,40,  5,46,  9,47,  4,42, 17,86,  3,62, 18,41,  8,27, 14,12, 
    19,18,  5,46, 10,22, 15,66,  8, 5,  7,12,  6,27,  1,82, 14,24,  9,60, 16,10,  0,93, 11,54, 12,10, 17,60, 18,14,  3,16,  2,48, 13,18,  4,82, 
    10,84, 13,15,  9,59, 14,45, 18,63,  5,71, 19,86, 15, 2,  3,42,  8,46,  6,39,  4, 9,  7, 7,  1,32, 17,78, 11,12, 12,78,  0,68,  2,77, 16,90, 
     2,33, 15, 2, 11,35, 14,91,  8,49, 16,21, 19,29,  7,60,  4,88,  1,71,  0,14, 12, 6,  9,40, 13,53, 18, 2,  6,88,  5,38, 10, 7, 17,43,  3,34, 
     4,16,  2,95,  3, 2, 15,88,  8,45, 11,93, 18,77, 17,10, 16,71, 12,69,  1,33, 13,22, 14,54, 19,56,  5,31,  7,18,  0,63, 10,82,  6,98,  9,70, 
    12,15,  0,42,  4,44,  8,80, 19,15,  6,24,  5,76, 10,71,  9,38, 15,27,  7,48, 18,38,  3,73, 14,42,  2, 8, 13,52, 11,20, 17,40, 16,51,  1,99, 
    13,46, 15, 3, 19,82,  5,20,  4, 1,  7,45, 18,79, 16,70,  3,46, 11,48, 12,63,  2,87,  9,47,  0,43, 10, 1,  1,48, 14,65,  8,11, 17,25,  6, 9, 
    14,12, 13,17, 16,55,  8,75,  6,56, 10,89, 11, 6,  0,46,  9,22, 15,44, 17, 2, 18,14,  7,36,  1,96,  3, 6,  4,37, 19,62,  5,94, 12,77,  2,14, 
     6,37,  3,28, 12,16, 13,80,  2,10, 15,90,  0, 5, 10,17, 19,28, 18,40,  9,76,  1,30,  7,52, 17,77,  4,15,  5,50, 11,99, 16,99,  8,64, 14,63, 
     2,88, 14, 4,  9,26,  6, 9,  4,81, 16,47, 11,82,  8,52, 12,65, 13,63,  1,37, 18,59,  0,78,  7,51, 17,75,  3,24, 15,77,  5,28, 19,97, 10,98, 
    16,79,  8, 7,  9,25, 12,92, 18, 8, 11,90, 13,82,  5, 9,  4,53, 19,95,  6,51,  1,54, 17,29, 15,86,  7, 7,  2,58,  3,66,  0,84, 10,42, 14,10, 
    15, 5, 16,52, 11,70, 18,59, 14,10, 10,88,  5,56,  0,55,  7,73,  3,65,  6,66,  8,17,  9, 5, 12,22,  1,40, 17,42,  2, 1,  4,51, 19,95, 13,28, 
    14,30, 12,62,  6,46,  8,14,  4,62, 16,16,  5,24, 19,17, 15,70, 11,66, 17,57,  1,55,  2,79,  9,99, 13,27, 18,17,  3,75,  7, 6,  0,14, 10,61, 
    10,71, 11,22, 14,68, 12,74,  4,39,  9,68,  5,28, 18, 7, 17,67,  3, 7,  1,92,  7,45, 16,67, 13,35,  6,70,  0,52,  2,65,  8,99, 19,21, 15,51, 
    16, 5,  6,73,  3,10, 15,75,  7,16,  4,70, 12,39,  9,62,  8,65, 13,99,  2,57, 10, 7,  1,54, 19,52, 14,47,  0,43, 18, 1,  5,65, 11,88, 17,36, 
     5,30, 19,82,  8,82,  4,54,  0,94,  1,28, 14,75, 18, 1, 13,84,  7,61, 17,17, 11,45,  2,72,  6,58,  3,78, 10,63, 15,50,  9,77, 16, 8, 12,25, 
     9,17,  6,78, 16,20,  4,92, 19,57,  5,95, 18,25, 11,44, 13, 8, 15,93,  1,21,  2,21,  7,47,  8,38,  3,33, 14,48, 17,39, 12,69,  0,15, 10,70, 
     4, 2,  6, 4,  5,44,  7,38,  8,93,  0,94, 17,93,  9,18, 13,38, 19,66, 12,38, 14,60, 11,87, 16,49,  1,52,  3,72, 10,12,  2, 5, 18,21, 15, 6, 
    17,63, 14,21,  2,21, 12,53,  7,79,  9,10,  8,13,  5,45, 16,63, 18,39,  0,38,  3,50, 13,80, 15,45, 19,84,  4, 2, 11,29,  6,31,  1,27, 10,96
  };
  // Taillard 100x20 instance 2
  const int tai_100_20_2[] = {
    100, 20, // Number of jobs and machines
    14,57,  0,18, 15,44, 16,17,  6,66, 19,34,  4,83, 13,76,  1,66,  8,20, 17,94,  5,67, 10,76, 12, 1, 11,84, 18,31,  9,78,  3,16,  7,15,  2,91, 
     2,56, 13,91, 19,62, 11,32, 16,61,  4,49,  6,27, 17,94,  5,92,  8,27, 10, 7, 12,99, 15,26,  1,80,  0,90,  3,61,  7,40, 18,74, 14,18,  9,38, 
    10,25,  6,61,  2, 5, 16,73,  7, 9,  3,17, 13,25,  9,99, 14,22, 19,24,  4,92,  8,91, 17,19, 11,20,  0,34,  1,82, 18,91,  5,38, 15,87, 12,13, 
    12,30,  9,50, 18,13,  4,77, 19,10, 14,92,  3,85,  6,84, 13,98,  5,25,  1,59, 16,69, 17,68, 10,48,  2,79,  0,90,  7,41, 15,60,  8,58, 11,73, 
    16,70,  2,93,  8,72,  6,17, 14, 2, 12,87,  0,19, 19,44,  3,38, 11,29,  7,62, 10,44, 18,18, 13,52,  5,63, 15,41, 17,99,  4,68,  1,42,  9, 1, 
    19,64,  8,65,  1,34, 13,73, 17,32,  3,17, 10,31,  2,80,  9,63, 12,13,  7, 3,  5,34, 15, 2,  4,60, 14, 6,  6,93, 16,39, 11,31,  0,58, 18, 6, 
    17,91,  2,65, 14,22, 10, 4,  0,72, 19,50,  7,85,  3, 4, 18, 7,  8,58, 13,96, 15,21,  6,62, 11,15,  4,79,  9,36,  5,58, 16,43, 12,21,  1,45, 
     6, 6, 19, 3, 12,21, 11, 1, 13,60, 10,32,  8,31,  7,43,  9,85,  0,59,  4,48,  1,11,  3,70, 15,21, 16,88, 18,92,  5,90,  2, 4, 14,49, 17,42, 
    14,89, 16,99,  6,84,  0,69, 19,35,  5,22,  9,42, 18,13,  2, 5,  3,33, 17,15,  8,22, 10,16,  1,12, 12,22, 15,26, 11,52,  7,11, 13,39,  4,39, 
    13, 1,  5,67, 16,70,  4,74,  6,52, 19,35, 15,83,  1, 2,  9,57, 14,24,  7,58, 11,78,  8,65, 17,44,  2,95, 12, 5,  0,47, 10,69,  3,66, 18,38, 
    10,65,  5,71,  1,34,  2, 6,  6,88, 18,96,  9,80, 11,87,  8,56, 12,15,  3,62, 17,75, 13,61,  7, 3, 14,56, 15,67,  4, 6, 19,37,  0,34, 16, 4, 
     7,23,  5,43, 17,28, 10,78,  3,77,  8,55, 14,27,  4,80, 13,42,  6,51, 16,54,  0, 9, 12,85, 18,93,  2,77, 11,31, 19,76, 15,43,  1,29,  9, 8, 
    19,55,  5,49,  3,22,  0,21,  8,57,  7,25,  2, 2,  1,87,  9,74, 15,94, 17, 7, 16,85, 10,46,  4,75, 18,91, 12,39, 14,21,  6,17, 11,66, 13,83, 
    19,42,  4, 8, 16,25, 17,42,  7,39,  5,12,  2,11, 15,73,  9,25,  8,63,  1,65, 18,57,  6,77, 10,49,  0,18, 14, 1, 12,84,  3,58, 11,67, 13, 8, 
     3,30,  0,26,  8,35, 14,40,  5,34, 16, 1, 19,88,  4,92,  2,53, 11,58, 12,32, 17,70,  9,11, 15,24, 13,64,  7,50,  1,68,  6,99, 18,38, 10,11, 
     5,38, 17,93,  2,50, 10,69,  6,78,  9,62, 15,70,  0,45, 11, 7,  3,87,  4,77,  7,87,  8,86,  1,66, 19,35, 16,42, 13,82, 18,55, 14,28, 12,23, 
    10,82,  4,99, 15,98, 13,50,  9,49, 14,22,  1, 9,  2,52,  3,21, 16,38,  0,50, 19,74, 11,58,  7,44,  8, 5, 18,53, 17,61,  6,21,  5,89, 12,62, 
     2,12,  8,60,  7,27, 11,74,  0,60, 17, 8,  4,26, 13, 3, 19,30, 12, 6,  3,46, 15,63, 16,67,  5,17, 10,75, 18,57,  9,51,  6,52, 14,13,  1,87, 
    19,68,  9,23,  3, 6, 16,20, 10,74, 17,30, 12,14, 13,46,  2,49, 15,67,  6,34,  4,47,  8,83, 18,45,  5,43,  0,75,  7,41,  1,81, 11,80, 14,55, 
     2,43, 14,17, 15,26,  8, 5,  4,22, 10,51,  0, 9, 12,61,  1,16,  5,93, 17,76, 19,34, 13, 5, 11,30,  7,81, 16,36,  6,13,  9,23, 18,76,  3,31, 
     4,25,  9,57, 17,51, 11,51,  5,22,  8,91, 16,32,  1,22, 19, 7,  2,99, 18,61, 15, 7,  7,67, 12,87, 14,95, 13,79, 10,19,  6,37,  3,92,  0,23, 
     0,57,  3,23, 13,99, 11,45,  5,47,  6,59, 14,53,  1,68, 18,68,  2,55, 19,19, 16,76, 15,30,  8,87,  4,55,  7,77, 12,12, 10,40,  9,30, 17,76, 
    16,96,  8,25,  2,83,  4,41, 17,76, 18,45,  7,23, 15,65, 14,73, 12,35,  6, 4,  3,47, 19,60, 13,74, 10,94,  1,51,  0,50, 11,82,  9,84,  5,11, 
     2,89, 12,76, 14,15, 18,68, 11,81,  9,81, 19,55,  7,56,  6,78, 15,46,  5,92,  1,15,  4, 5, 10,12, 16,49,  8,60, 13,94, 17,12,  3,33,  0,47, 
    10,56,  9,14, 16,80,  0,11,  1,50,  7,51, 19,83, 13,65, 14,87,  2,50,  3,78,  6, 6, 12,18, 15,40,  4,43,  8,87, 17,74, 18,76,  5,73, 11,51, 
    15,63,  3,55,  5,49,  6,52, 18,66,  9,27, 10,66,  7, 1, 14,83, 13,23, 17,49,  1,95,  4,48, 11,26, 16,56, 19,20,  0, 7, 12, 6,  2,43,  8,52, 
    15,29, 19,90,  8,83,  4,33, 11,91,  6,45, 17,71,  3,49, 12,90,  7,72, 16,79, 10,56, 13,65,  1,11,  2,74, 18,36,  5,76, 14, 1,  0, 2,  9,74, 
     6,89, 11,18, 13,73,  5,45, 18,37,  3,44,  0,25, 16,86,  7,66,  2,19, 17,76,  1,31, 15,44,  8,12,  4,54, 19,73,  9,42, 14, 1, 10,91, 12,31, 
     1,73,  0,19, 16,97, 12,50,  7,74,  4,30, 18, 4,  9,69, 15,44, 17,98, 10,19,  6, 2, 19,90, 11,34,  3,46,  8,76, 14,81,  5,76, 13,40,  2,50, 
     2, 4, 19,60, 11, 5,  0,19, 18,49, 16,26, 13,18,  5,96,  7,37,  4,55,  8,83,  1,72, 14,76, 15,38,  6,95, 10,98,  3,65, 17,74,  9,80, 12,97, 
     0,16, 16,64,  1,20, 18,58, 10,99, 19, 4,  3, 9,  8,59,  9,29, 11,26,  2,23, 14,10, 15,15, 17,21,  5,19,  7,98,  6,77, 13,80,  4,76, 12,25, 
    13,51, 15,33,  1,14,  5,63, 14,46, 10, 6,  6,53, 12,27,  4,99,  0,87,  8,15,  7,77, 11,66,  2, 3, 19,51, 17, 3,  9, 9, 16,34,  3,77, 18,54, 
     9,67, 12,98, 10,57,  5,74, 16,47,  7,12,  6,92, 19,30,  8,45,  0,39, 13,36,  4,12,  1,86, 14,88, 17,15,  2,89, 11,96, 18,69,  3,43, 15,72, 
    17,84,  7,67, 16,30,  8,69, 18, 6,  9,87,  5,58,  1,37, 19,87, 13,12,  4,90, 14,38, 10,47, 15,99,  0, 8,  2,30,  3,48, 11,30,  6, 7, 12,98, 
     4,92,  3,74, 18, 8, 12,86,  9,36,  6,78, 11, 6, 14, 2, 10,42,  1,14,  7,72, 17,94,  8,83,  5, 4, 16,50,  0,90, 19,80, 15,27, 13,45,  2,71, 
    19,78, 13,41,  9,17,  0,86, 14,83,  8,71, 12,54, 10,80,  5,68,  7,16, 16,88, 17,40, 15,88,  3,11, 11, 9,  6,54,  4, 3, 18,24,  2,81,  1,18, 
    19,72,  7,64,  9,68, 17,46, 13,29,  5,92,  2,74, 15,68,  3,12,  1,68, 14,87, 16,80, 18,81,  0,44, 10,76,  8, 9,  6,26, 11,31,  4,37, 12,82, 
     7,44,  0,53, 17,86, 13,67, 12, 4, 14,14,  1,47,  8,19,  6,50, 18,68,  2,65,  3,22,  9,97,  5,11, 10,34, 15,75, 19,11, 11,14,  4,78, 16,80, 
     9,63,  7,48,  8,63, 19,43, 12, 9,  3,82,  1,13, 14,48, 18, 3, 11,69, 16,47, 13, 3,  4,56,  5,32, 17,41,  0,10,  2,48, 15,16,  6,98, 10,59, 
     6,42,  8,91, 18,11,  7,37, 16,97,  1,40,  5,35, 14,96,  3,95,  9,26, 19,80,  4,61, 11,69,  2,16, 17,59, 12,65, 15, 3,  0, 5, 10,85, 13,48, 
    19, 3,  3,22,  2,12,  8,63,  5,20,  7,83, 14,56, 17,76,  0,47,  1, 9, 13,42, 15,35, 12,23,  4,11,  9,20, 10,28, 18,51, 11,78, 16,35,  6,57, 
     3,13, 12,76,  4,20,  8,67, 10, 6,  1,19, 16,74,  0, 1,  9,13, 17,48, 14,44, 13,12, 19,46, 18,94, 15,26, 11,43,  6,37,  7,73,  2,48,  5,29, 
    13,25, 12,49,  0,51,  5,36,  8,50, 18,15,  7,66,  1,17,  3,43,  4,53, 16, 3, 11,47, 10,12, 15,74,  9, 8, 19,65, 17,29, 14, 8,  2,18,  6,36, 
     4,33, 16,14,  3,95, 19,48,  7,68,  1,24,  0,87, 11,43,  5,10,  6,40, 10,32,  8,43, 17,66,  9,42, 13,29, 12,20,  2,28, 18,39, 15,77, 14,41, 
     4,73, 15,66, 16,67,  8,67,  9,72, 17,63,  5,81, 11,84, 18, 7,  0,52, 14, 9,  6, 9, 10,35,  2,11,  3,37, 13,55, 12,96,  7,84, 19,97,  1,32, 
    15,67,  5,28, 16,85, 12,58,  1,51, 17,41,  0,64, 18,40, 10,54, 19,31,  8,47, 14,16, 11,43, 13,59,  2,67,  9,33,  4,15,  3,91,  7, 9,  6,52, 
     5,29,  4,62, 15,23,  9,60,  2,68,  7,20, 19,20, 14,32,  8,50, 16, 9, 11,22, 12,56,  0,94, 17,27,  6,41, 13,77,  3,19, 18,54,  1,87, 10,96, 
     3,69, 14,66,  5,86, 13,43, 15,52,  0,39,  4,27,  8,63, 19, 5, 16,70,  7, 2,  6,10, 17, 2, 12,13,  1,44, 10,92, 11,71,  2,14,  9, 4, 18,35, 
     2,65, 17,78,  7,26,  0,29, 14,76,  8,20,  1,64, 12,33, 18,91, 13,47, 10,89,  6,49, 15,50,  4,41,  3,61,  5,22,  9,99, 19,32, 16,12, 11,43, 
    18,78,  7,84, 14,81,  2,62,  1,42, 19,11,  6,65,  8,83, 11,75, 13,12, 10,46, 15,46,  3,61,  0,98, 12,49,  4,35, 17,39,  5,27,  9,23, 16,56, 
     2,49,  5,37,  8,37, 19,95,  7,91, 15,68,  4, 8, 16,18, 13,54,  6,23, 12,94, 18,62, 17,66, 14, 3,  0,21, 10,62, 11,72,  3,28,  9,91,  1,83, 
     5,21, 14,73, 18,45, 13,22, 15,89,  0, 4, 12, 4,  7, 1,  8,71,  6,44,  1,35,  3,99,  2,49, 10,19,  9,25, 19,77, 11,14,  4,90, 16,19, 17,85, 
    10,76, 15, 8,  9,68, 14,13, 11,98,  8,88,  7,50,  1,17,  3,73, 17,18, 18,22,  0,63,  5,73,  4,56, 16,61, 19,25,  2,71, 12,15, 13,50,  6,23, 
    13, 1, 14,99,  9,64, 11,51, 19,77,  6,12,  4,26,  7,27,  5,31, 12,23,  0,90, 18,65, 17,16,  8,91,  2,47,  1,35, 16,39,  3, 9, 10,64, 15,91, 
    11, 5, 18,42, 19, 3, 17,15, 14,74,  9,45,  4,34, 16,49,  8,45,  6,54,  0,86,  2,28,  7, 6, 15,77, 12,76, 13,59,  5,63,  1,21, 10,50,  3,93, 
    18,26,  5,94,  4,33,  0,20, 14,27,  1,83, 12,55, 19,93,  3,41, 16,62, 11, 6,  7,33, 17,47, 13,84,  8,25,  2, 5,  9, 8, 10,14, 15,26,  6,96, 
     5,90, 17,99, 14,19, 10,74,  8,15,  6,82, 18,55,  9,91,  0,53, 13, 8,  1,19,  2, 9, 19,60,  4,10,  7, 5, 11,37, 16,95,  3,19, 12,63, 15,57, 
     9,40,  7,19, 13, 3, 16,44,  3,30, 19,84, 14,49,  6,22,  5,15, 11,93,  1, 8, 18,90,  4,48, 12,87,  8,34,  0,42,  2,78, 10,76, 15,82, 17, 7, 
     1,47,  0, 4,  5,35, 19,15, 10,32,  2,64,  7,75,  9,63,  8,97,  4,51, 18,65, 14,41, 17,46, 11,64, 12,21,  6,50, 16, 3,  3,40, 15,35, 13,65, 
     3,89,  6,23,  1, 9,  7, 7,  9, 8, 14,77, 10,94,  0,82,  5,32, 13,93, 19, 4,  4,73, 12,94, 17,66,  8,82, 16,28,  2,38, 18,44, 11,26, 15, 5, 
    10,30,  1,68,  8,69, 18,57,  4,82,  6,75, 13,10, 12,37,  7,35,  0,95,  9,86, 14,25,  3,62, 11,81, 16,75, 19,59,  2,96,  5,84, 15,81, 17,99, 
    17,91,  4,61, 16,44,  7,11, 10,21, 14,14, 19,62, 12,16,  8,36,  1,51, 15,56,  3,77, 13,79, 11,53,  0,37,  6,48,  2,28, 18,61,  9,52,  5,72, 
    14,51, 16,18, 19, 9, 13, 6, 10,53,  8,72,  0,48, 11,72,  4,64,  6,49,  2,57,  7,42,  3,80, 18,92, 17, 9, 15, 6,  9,84, 12,58,  1,50,  5,79, 
     2,67,  8,16, 19, 5,  5,30, 12,12,  3,11, 11,26, 13,74,  9,36, 17,23,  7,50,  4,13,  6,59,  0,37, 15,79, 14,68, 16,20, 10,66,  1,87, 18,52, 
    14,52,  1, 4, 12, 6,  2,27,  0,35,  5,46,  9,82,  7, 2, 17,59, 16,71, 10,84,  4,15,  8,40, 13,77, 11,78, 18, 2, 15,62,  6,72, 19,79,  3,71, 
     1,76, 15,25, 18,40,  8,15, 16,83, 10,39, 14,66,  7,79, 12,78, 13,30,  2,18,  5,29,  9,56, 19,27,  0,44, 11,78,  6, 1,  4,69,  3,34, 17,18, 
     1,51, 19,89,  5,99,  4,54, 15,99,  6,76, 14,11,  8,92, 12, 4,  3,25, 18,66, 13,41, 11,92,  0,36,  2,55, 16,32, 10,47,  7, 1, 17,45,  9,91, 
     4,81, 15,87, 17,28,  6,15, 16,43,  5, 4, 10,67,  7,12, 18,95, 12, 4, 19,66,  9,59, 14,34, 13, 9,  0,25,  3, 2,  8, 6,  2,87, 11,85,  1,35, 
     2, 6, 14,11,  5,50, 11,84, 12,62,  7,94, 16, 2,  9,97, 13,52, 18, 4, 19,92,  3,44,  8,51,  6,23,  4,80, 10,35, 17,16, 15,46,  1, 5,  0,67, 
     6,62, 14,97,  8,52,  5,99, 11,83, 18,53,  2,57,  4, 8,  1,69, 15,37, 13,72,  9,95,  3,61, 16,76, 19,72, 12,54,  7,15,  0,91, 17,50, 10,36, 
    18,43,  5,33,  0,24, 15,84,  3,29,  4,32,  1,30, 17,42,  9,79,  7,58, 14,46, 16, 7,  8,89,  6,71, 11,91,  2, 9, 12,70, 19,40, 13,33, 10,22, 
     0,31,  1,23,  3,36, 18,37, 13,80,  8, 6, 17,57,  4,86,  2,52,  5, 1, 16,95,  6,68,  7,87, 19,18,  9,56, 15,28, 12,66, 10,88, 11,45, 14,20, 
    19,68,  1,14,  3,43, 15,90,  8,44, 13,21, 14,28, 18,97, 11,98,  2,48, 12,95, 17,84, 16,70,  9,38,  4,92, 10,56,  5,55,  6,53,  0,15,  7,21, 
     5,91,  1,56,  9,97,  0,46, 16,18, 10,64,  7,94, 11,99, 18,81,  4,62, 15,56,  3,53, 14,98, 12,32,  8,51, 19,24,  2,26,  6,81, 17,24, 13,26, 
     6,99,  1,46, 19,22,  3,25,  9,12, 17,28, 10,29,  2,87, 12,36,  0,38, 15,42,  8,35, 11,30, 18,57,  7,85, 14,97, 13, 2, 16,49,  5,83,  4,28, 
     0,13,  5,93,  4,59,  2, 3,  3,51, 15,74, 10,84, 19,83, 11,74, 17,89, 13,70, 14,45, 16,59,  6,54,  8, 1, 18,60,  9,78,  7,59, 12,46,  1,91, 
     1,47, 12,35,  7,25, 17,54, 10,50, 16,31,  4,72,  8,84,  6,69, 13,90, 14,55, 18,64,  5,89, 15,10,  0,51, 11,81,  3,58,  2,47,  9,36, 19,13, 
     8,82, 13,84,  3,25, 10,16,  6,91, 18,78, 19,78,  5,48,  4,72, 14,47, 15,60,  1,36, 11,27,  0,42,  7,34, 17,35,  2,85,  9,33, 16,70, 12,57, 
    12, 4, 13,97, 15,59,  8,96, 10,31,  4,72,  1,46,  3,47,  0,56,  9,64,  7,51,  2,77, 16,73, 19,67, 11,81, 14, 2,  5,78,  6,22, 18,48, 17, 9, 
     9,90, 16,21,  8,81,  1,42, 12,63,  5,98, 15,84, 18,29,  7,54,  4,91,  3,75, 14,90,  2, 1, 13,89, 10, 6,  0,42, 11, 2, 17,12, 19,33,  6,66, 
     4,80,  9,88, 16,57, 12,30, 19,94,  5,95, 15,58,  3,89,  0,47,  1,17, 17,24, 10,98, 18,62,  6,91,  2,49, 11,65,  7,34,  8,91, 13,94, 14,13, 
    10,41,  8,94, 14, 9,  9,95,  2,77,  3,87,  4,90,  0,86, 18,21,  1,13, 16,60, 17,36,  6,22,  7,71, 15,18, 12,15,  5,32, 19,14, 13,11, 11,54, 
     2,47, 14,18,  1,93,  0,57,  5,74, 19, 2, 15,58,  4,69,  7,10, 12,41,  3,80, 13, 3, 10,72,  6,60,  9,34,  8,60, 17,34, 16,32, 11,33, 18,89, 
     9,88,  4,31, 19,27, 12,88, 18,48,  6,27, 14, 5,  3,49,  8, 8, 11,67, 13,23,  5,48, 16,24,  7,89, 15,73, 17,76, 10,87,  0,55,  1,17,  2,63, 
    12,10,  0,83, 10,87, 19,46, 14,31, 17,79, 18,28,  3,15,  5,31, 16,83,  6,42,  7,40,  8,43,  9,60,  2, 5,  1,62,  4,48, 13,28, 15,13, 11,74, 
     9,73,  6,77,  0,94, 12,16,  3,89,  4,48, 19,93, 17,25,  5,76, 10, 4, 14,70,  7,66,  1,31, 13,63,  2,70, 18,91,  8,39, 11,67, 15, 3, 16, 5, 
    18,62,  5,93, 13,38,  0,65, 10,81,  8,45, 16,56,  7,69, 12,77, 14, 4, 17, 8, 15,66, 11,16,  3, 6,  6,76,  2,79,  1,24, 19,96,  4,80,  9,38, 
    17,95,  4,26, 18,84,  1,96,  7,91, 16,63, 13,48,  6,39, 19,28,  5,48, 14,83, 15,74, 10,84, 11, 2, 12,76,  0,45,  3,98,  9,70,  8,88,  2,68, 
     3,10, 17,96,  5,66,  9,79, 19,94, 18, 8,  0,19, 15, 9,  8, 8, 13,74, 11, 1,  1,30, 12,93,  4,91,  2,11, 10,57, 16,76,  6,75, 14,87,  7,40, 
     7,17, 13,98, 10,75, 17,57, 14, 1,  0,61,  2,72, 16,82, 18,60, 19,87,  3,94,  5,49, 11,64,  4,83,  9,42, 12,63, 15,29,  6,67,  8,99,  1,76, 
     1, 6,  5,92,  0, 9, 13,60,  4,24,  6,20,  3,78, 12,46,  7,16, 14,52, 16,78,  8,30, 11, 3, 15,63, 17,13,  9,68,  2,73, 10,55, 18,94, 19,65, 
    13,21, 12,93,  3,18, 14,34,  5,82, 19,51, 16,49, 10,70,  2,51,  0,58, 15,57,  8,80,  6,45, 17,69,  7,50,  1,94,  4,10, 11, 2, 18,71,  9,40, 
    16,17, 18,78, 11,56,  4,76,  8,36, 17,26,  3,87, 14,74,  7,35,  1,39, 13,32, 10,82,  0,84, 12,95,  2,41,  6,43,  9,50, 15,41, 19,46,  5,53, 
    12,67,  8,62,  4,71,  6,12,  0,23, 15,81, 19,65,  5,40,  9,59,  1,66,  3,78, 10,21,  2,48, 16,69, 18, 4, 13,95,  7,68, 17,36, 11,19, 14,51, 
    17,97, 14,84,  6,87,  8,95, 11,26, 13,14, 19,72, 12,24,  7,86,  4, 6, 16,45,  2,29, 15,19, 18,92,  3,80,  1,11,  0,52,  9,77, 10,89,  5,75, 
    14,20, 18,85, 13,48,  7,70,  4,31, 15,14,  3,81, 11,21, 19,57,  5,98, 10, 9,  2,23,  1,16,  0,46, 16,44,  6,16,  8,41, 17,17, 12,80,  9,13, 
     7,68, 16,30,  5,86, 13,54, 18,80, 19,78, 11,16,  1,89,  4,71,  8,97,  3,80,  9,69, 15,38,  2,77,  0, 3, 10,30, 12,61, 14,82,  6,88, 17,85, 
    17,97,  2,46,  8,49,  4,77, 19,27,  0,96, 14,82,  9,63, 18,21, 13,83,  5,51,  6,77, 12,80,  7,88,  3, 7, 16,97, 11,73,  1,58, 15,36, 10, 2, 
    10,44,  8,91, 19, 2,  2,94, 11,81,  5,51, 15,88,  9,43,  4,20,  0,79, 12,47,  1,62, 17,66, 18,41,  3,24,  7,60, 13,12,  6, 3, 14,77, 16,81, 
    13,43, 15,65,  4,68, 12,61,  1,69, 17,26, 18,47,  3,87,  2,53,  6,81,  7,59, 16,81, 11,80,  5,63, 14,31,  0,21,  8,46,  9, 3, 10,82, 19,64
  };
  // Taillard 100x20 instance 3
  const int tai_100_20_3[] = {
    100, 20, // Number of jobs and machines
    17,17, 16,25,  7,65,  0,31,  2,80,  5,64, 13,30, 15,36,  9,99, 10,16,  3,32,  1, 3, 19,28, 12,79, 14, 1,  6,50,  4,94, 11,31, 18,87,  8,94, 
    12,71,  3,86, 19,16, 13,79,  2,92,  7,79, 16,63, 17,10,  6,47, 14,64,  9,56,  5, 4,  1,87,  0,16, 18,43,  4,70, 10,15,  8,27, 15,99, 11,37, 
    17,81,  0,32, 18,76, 10,80,  2,19,  1, 6,  6,37,  4,59, 14, 8,  8, 3, 12,41, 15,79, 13, 8,  3,83, 16,68,  5,53, 11,29,  7,61, 19,24,  9,53, 
     8,78, 14,99,  7,97,  0,20, 13, 3, 12, 1,  2,97, 10,92,  3,43,  9,11,  1,25, 17,31, 11,98, 16,73,  4,38, 19,71, 15,69,  6,71,  5,73, 18, 2, 
    13,91,  0,90,  7,97, 16,24,  6, 8, 12,72,  2,54,  4,67,  5,63, 11,39, 18,76, 14,12, 10, 1,  1, 7, 19,46,  8,14,  3,54,  9,66, 17,55, 15,42, 
    11,67,  1,13, 18,74,  6,38, 17,10, 16,67,  0, 6, 19,50,  7,72, 10,93,  3,25,  2,72,  5,66, 13,71,  4,18,  9,77, 14,27, 12,20,  8,97, 15,17, 
    19,84,  2,19,  4, 7, 11,40, 18,73, 14,89, 16,85, 10,78,  5,29,  3,89, 13, 9,  0,88,  7,43,  6,52, 12,36, 15,44, 17,77,  1,44,  9,82,  8,90, 
     4,27, 17,88, 19,59, 11,79, 13,98, 16,66,  0, 9, 10, 1, 18,35,  7,69,  5,29,  9,67,  6,60, 12,43, 15,21, 14,19,  8,76,  2,55,  1,77,  3,15, 
    12,44,  3,43, 11,67,  7,41,  9,90, 17,71, 19,38, 15,39,  4,77, 18,18,  0,12, 14,17,  1,10,  8,77,  2,49,  5,25, 13,85, 10,30,  6,14, 16,92, 
    11,57,  0,41, 15,51, 14,41,  8,17, 17,91, 12,82, 16,68,  9,76, 18,13,  1,93,  7,12,  2,90, 19,58,  6,52, 10,83, 13,68,  4, 3,  5,11,  3,40, 
     6,63,  8,59, 12,18, 15,99, 16,78,  4, 5, 19,15,  2,71, 18,59, 11,21,  0,58, 10,29, 14,59,  3,98,  9, 2,  7,18, 17,87,  5,55, 13,83,  1,85, 
     9,54,  1,57,  3,51, 14,25, 13,57,  7,46,  8,93, 17,58, 19,20, 12, 4,  4,19, 18,91,  5,25,  2,88, 11,94,  6,52, 10,23, 16, 4,  0,27, 15,66, 
    18,73,  1,84,  9,85,  3,50,  4,61,  0,40, 14,93,  2,17, 15,86,  8,19, 16,49, 10,98, 13,74, 17, 6,  5,27,  7,91, 11,58, 19,59, 12,22,  6,15, 
     9,11, 12,56, 10,39, 14,20, 13,79, 17,55, 18,18,  4,40, 11,74,  3,91,  6,95,  1,65, 15,13,  7,26, 19,40,  0,89, 16,25,  5,33,  2, 3,  8,21, 
     3,51,  8,37,  9,49, 10,20,  5,52,  2,57, 19,17,  0,42, 16,50,  6,74, 17,33, 11,38, 15,16,  4,21,  1,35,  7,85, 18, 7, 12,66, 13,68, 14,66, 
     7,47, 16,27, 10,86,  8,19, 13, 9,  3,23,  0,64,  5,32, 11,24, 17,14,  9,47, 15,81,  4,45,  1,84,  6,10, 12,25,  2,74, 19,81, 18,98, 14,79, 
     0,94, 18,46,  9,63,  6,64,  7,72,  3,70,  2,86, 11,10, 16,11,  4,99, 14,93, 17,90, 19, 4,  5,75, 10,87,  8,35, 13,46, 15,27, 12,94,  1,83, 
    16,53,  9,33, 15,56,  5,19,  0,50,  7,82,  8,20, 18,49, 13,73,  2,21, 17,28, 19,40, 10,49,  4,83, 14,82, 12,16,  6,97, 11,86,  1,46,  3,45, 
     8,26, 10,29, 14,35,  7,31,  0,71, 19,78,  3,38,  9,11, 15,26,  5,12, 12,69, 13,64, 18, 1, 16,96, 17,36,  2,93,  6,52, 11,30,  4,22,  1,25, 
    12,12,  9,98,  5,14,  1,38, 10,94, 18,77,  2,52,  7,95,  8,20, 11,77, 14,42,  4,28, 15,69, 17,88,  3,29, 13,18, 16,95, 19,73,  0,16,  6,34, 
     7,22, 15, 5,  8,31, 17,25,  6, 8, 14,38, 19,17, 18,17,  5,13,  9,59, 12,36,  2,88,  0,57,  4,19,  1,57,  3,65, 13,88, 11,23, 10,52, 16,40, 
     4,19, 10,34,  2,49, 13,46, 11, 2, 18,45,  6,82, 19, 7, 16, 7,  8,50, 15,81, 14, 1, 12,99, 17,91,  9,54,  5, 4,  1,87,  3,29,  7,63,  0,45, 
    13,36,  9,38, 16,93, 17,27,  4,28, 10,81,  5, 2, 19,66,  3,28, 14,46,  0,96,  6,55,  8,99, 12,90,  7, 5,  2,37, 11,18, 18,55,  1,78, 15,53, 
     3,89, 16,93, 13,40,  1,76,  6,29,  7,61, 12,13,  9,95, 17,28, 11, 3, 18,89, 14,28, 19,13,  4,63,  8,32,  2,50,  0,24, 15,63,  5,76, 10,47, 
     2,44,  3,48, 13,32, 10,61, 15,85, 14,34, 12,21,  1,52, 17,66,  4,47,  7,30, 11,49,  0,88, 18,95,  6,94, 16,59,  9,77,  8, 5, 19,79,  5,27, 
    12,19, 19,64, 18,27,  1,30, 14,27, 16,57,  0,96, 10,36,  4, 8,  5,77,  3,59,  9,36, 13,13,  8,96,  7,88, 11,47,  2,80,  6,32, 17,43, 15,93, 
     9,73, 11,75, 14,37, 16,70, 13,90,  0,34, 12,87, 10, 6,  4,59,  2,79, 15,11,  5,50, 17,46, 19,12,  6,38,  1,55,  7,15,  3,18, 18,80,  8,98, 
    17,70,  6,90, 16,89,  7, 4, 19,77, 12,31,  0,43, 18,87, 13,52,  2,63,  1,75, 11,21,  8,30, 14,68,  5,81,  3,89,  9,68, 15,44, 10,63,  4,51, 
    12,70, 11, 9,  6,52,  5,55,  7,92, 19,60,  2,39, 17,13,  3,68, 15,72,  9,80, 13,45, 14,28,  8,69, 10,48, 16,29,  1,42,  0,55, 18,44,  4,88, 
     7,91, 12,28, 10,68, 13,49,  5,94, 11,86,  8,43,  0,43,  3,75,  1,98, 19,88,  9,67,  6,63,  2,34, 14,61, 15,85, 18,32,  4,96, 16,45, 17,51, 
     3,90, 12,97,  9,53, 10, 9,  2,63,  0,92,  1,18,  6,68, 17,55, 15,10,  4,78, 11,34, 13,57, 16,13, 18,64,  8,59, 19,56,  7,76, 14,41,  5,56, 
    16, 9, 18,44, 10,59, 17,61, 19, 6,  8,66, 15,56,  7,41, 11,97,  6,41, 14, 1,  4,21,  2,43, 12,76,  1,20,  9,73,  5,88, 13,98,  0,56,  3,26, 
    14,90, 18,60,  1,40, 17,78,  8,52, 12,10, 13,50, 15,61,  5,95,  3,28,  9,38,  7,24,  6,51, 10,96,  2,90, 19,18,  0,77, 16,11,  4,43, 11,93, 
     0,47,  8,30,  2,13, 12,63, 15,93, 16,42,  7,32,  1,12,  4,66,  9,94,  5,89, 11,25, 13,16,  6,96, 18,87, 14,39, 19,58, 17, 8, 10, 2,  3,17, 
    12,50, 11,62,  5,91,  8,25,  2,19, 16,73,  9,98,  4,63, 15,95, 10,57,  7,67,  1, 7, 13,76,  6,79, 14,90, 18,75,  0,42, 19,66, 17,90,  3,41, 
    12,18, 19,99, 10,89, 11,43, 17,49,  4, 8,  5,44, 18,33, 14,91,  0,39, 16,36,  9,72,  1,27,  7,70, 15,66,  6,82,  8,94,  3,46, 13,18,  2,98, 
    13,45,  3,13, 11,26, 16,14,  4,29,  8,90, 17,60,  0,84,  6,83,  2,41,  7, 7, 10,24,  5,73,  9,58, 15,91,  1,47, 12,82, 14,81, 19,67, 18,15, 
    10,52, 13,79, 19,18,  8, 3,  0,29,  9,88,  7,97, 15,55, 18,55, 17,15,  4,87,  1,17, 12,43,  3,58, 11,69, 14,66,  5,31,  6,43, 16, 8,  2, 2, 
    19,87,  8,51, 10,65,  3, 5,  7,34, 15,58, 14, 9,  5,10,  0,30, 11,87,  2,62,  9,89, 17,78,  1,37, 12,94, 16,40,  6,50, 18,83,  4,74, 13,83, 
     2,35, 15,34,  4,44,  0,65, 13,37, 10, 5, 19,51, 18,26, 12,65,  9,16,  7,90,  5,40, 14,76,  1,54,  8,72, 17,27, 11,37, 16,75,  6,77,  3,89, 
    14,15,  8, 1, 17,43, 15, 7, 11,64,  1,50,  5,26, 10,29,  6,13,  9,76, 19, 9,  7, 5,  2,85,  4,19, 18,23, 13,95, 16,76,  3,69,  0,48, 12,26, 
     4,84,  3,42,  5,99,  6,25,  9,75, 18,93, 19,76,  2,56, 11,23, 15,87, 17, 8,  0, 2,  8,60, 16,79, 12,30,  1,16, 13,61, 14,16, 10, 1,  7,31, 
    12,90,  8,41, 17,20, 19,50, 10,38, 13,76,  5,68, 14,66,  7,84,  6,77,  1,30,  2,55, 18,50,  3,40, 15,99, 11, 5,  9,27,  0,10,  4,32, 16,24, 
    18,33,  4,79,  0,55,  8,69, 13,53,  6,50,  3,81,  2,28,  9,91, 17,55,  1,50, 11,29, 15,27, 12, 9, 16,61, 10,27,  5,36,  7,43, 19, 5, 14,56, 
    10,72, 13,36,  7,27,  0,16, 15,50,  6,82, 11, 8,  8,79, 14,80,  2,22,  5,63,  3,61,  9,25, 16,61,  1,71,  4,93, 19,44, 18,65, 17,72, 12,88, 
    12, 7,  5,18, 14,59, 18,44,  8,64,  0,94,  7,55,  2,20, 10,11, 16,77,  9, 3,  6,26,  1, 8, 17,21, 11,73,  4,80, 15,89, 19,53,  3,19, 13,95, 
     5,35,  2, 7, 11,98, 18,66, 12,16, 15,89,  0, 6,  3,24, 17,18, 19,39, 13,52,  9,49,  7,52,  6,26, 16,11, 10,73, 14,91,  1,69,  4,99,  8,79, 
     3,65,  1,24, 10,87,  9,85,  2,66, 15,99, 13, 5, 12,29,  5,98,  0,17,  7, 4, 19,48, 11,45, 17,26,  4,80,  8, 5, 18,56, 14,53,  6,76, 16,41, 
     8,39,  3,34, 18,47,  9,60, 15,21,  0,11, 13,46, 10,14, 11,59, 19,51, 14, 7, 16,58,  7,49,  2,57,  1,71, 12,57,  4,47,  6, 1, 17,85,  5,72, 
    17,93,  2,88, 10,24,  9,32, 14,18, 16,44,  6,48,  1,18, 15,80,  8,67, 13,10,  7, 9,  5,16, 11,35,  0,16,  4,10, 12,29, 18,71, 19,89,  3,87, 
    10,36,  9,29, 12,35,  5,78,  7,17,  4, 7, 19,38,  0,50, 14, 6, 11,15,  2,14,  6,30,  1,36,  3,18, 15,59, 16,63, 13,92, 18,26,  8,46, 17,23, 
     5,90,  9,11, 14,87,  3,34, 12,20, 17, 1,  4,57, 11,30,  2,19, 19,25,  6,22, 15,32, 10,86,  0,31, 18, 4,  7,54,  1,32,  8,17, 16,24, 13,67, 
     2,91, 13,61, 15,53,  4, 1, 10,67,  5,53,  9,86,  0, 8, 17,12, 11,51,  7,90, 18,73,  3,44, 16,12,  8,92, 14,58, 19,57,  6,63, 12,82,  1,68, 
    18,77,  6,48,  5,72, 19,89,  7,36, 16,23, 13,39,  4,45, 15,39, 14,85, 10,54,  8,69, 12,44, 17,92,  0,86, 11,42,  2,83,  9,85,  3,23,  1,58, 
     7,15,  1, 5,  2,67, 17, 9, 16, 7, 10,81,  9, 4,  6,86,  5,74, 15,49, 12,26,  8, 9, 14,27,  3,33,  0,51, 19,95, 13,32, 11,24,  4,31, 18,58, 
    14,30,  8,90, 10,27, 15,17, 17,85,  4,27,  9,92,  7,11,  1,15,  3,61, 19,18,  0,34, 13,96,  2,56, 16,34,  5,44, 11,32,  6, 1, 12,16, 18,55, 
    18,21,  4,95,  5,81,  2, 1, 11,65, 17,69, 19,24, 14,47, 16,92, 15,25,  1,22,  7,60, 13,62,  9,97,  3,78, 10,63,  8,18,  0,35, 12,80,  6,55, 
    13,96, 14,27,  0,66,  8,52, 19,16, 15,20, 16,24,  6,59, 17,75,  4,64, 18,26, 10,88,  9,57,  5,68,  2,60, 11,12, 12,54,  3,99,  1,51,  7,71, 
    11,79, 15, 6, 19,41,  8, 8,  6,93, 12,46,  3,69, 18,81, 13,61,  4,44, 10, 2, 14,69,  5,72,  9,21,  7,55, 17,69,  2,13, 16,69,  1,80,  0,91, 
     1,82, 10,99,  4,92, 13,53, 14, 5,  9,33,  7,59, 19,32,  3,93, 17,13,  0,64,  8,19, 18,96, 11,12,  2,66,  5, 4, 15,83,  6,65, 16, 3, 12,79, 
    18,16,  2,62,  4, 1,  3,41,  5,84, 19,61, 16,31, 10,63,  6,10, 17,10,  8,14, 13,75, 12,14,  0,63,  1,53,  9,61, 15,41, 14, 7, 11,57,  7,15, 
    18, 5,  7,83, 10,25, 11,37,  6,81, 16,42,  5,20,  9,22, 13,50,  4,40,  1,77, 19,76,  3,58, 14,88, 15,77, 17,69,  0,63,  8,56,  2,49, 12,41, 
     9,43,  1,16, 10, 1,  0,56, 11,38,  8,41, 19,78,  7,81, 15, 5,  6,46, 12,10, 13, 6, 16,31,  4,44, 17,22, 18,78,  2,93, 14,42,  3,21,  5,43, 
     5,27,  9,26, 10,32,  7,20,  0,20,  4, 5, 15,43, 13,63, 16,23,  2,29, 19,45,  3,79, 12,68, 11,73, 18, 1, 14,10,  1,61,  8,56, 17,95,  6,36, 
    10,88,  3,21, 15,86, 17,92,  2,89,  6,30, 14,44, 19,25, 12,99,  8,91, 16,21, 13,45,  0,82, 11,41,  4,76,  1,33,  5,77,  9,49, 18,63,  7,37, 
     1,33,  0,54,  7,79, 19,56, 16,54, 17,75,  5,79,  8,99, 18,38, 15,13, 12, 2,  3,53, 11,89,  4,82, 10,53, 13,33,  6,43, 14,73,  2,25,  9,14, 
    16, 7,  2,43,  8,70,  5,22, 14,54,  9,81,  1,44,  4,94, 12,95, 10,29, 15,14, 13,50, 17,98,  7,18,  3,50, 19,96,  0,74,  6,66, 18,38, 11,49, 
    18,79, 12,40,  1,71,  3,39, 14,13,  2,19, 11, 1, 10,82,  0,58, 13,26, 15,44,  8,18,  7,94,  5, 8,  9,68, 19,90, 16,39,  6,47,  4,43, 17,80, 
    12,82, 10, 1, 19,76,  2,22,  9,17, 16,15,  6,64, 17,71,  7,22, 15,79,  3,89,  4,80,  5, 7,  1,78, 14,87, 11,88, 18,22,  0,59, 13,75,  8,24, 
    12,56,  9,93,  8,63,  6,23,  7,31,  0,90,  1,46, 11,83, 10,21, 18, 4,  3,53, 14,99, 13,65, 17,99, 16,58,  5, 5, 15,33,  2,17, 19,26,  4, 7, 
    13,35,  9,44,  8,10, 11, 5, 10,40, 15,66,  0,13, 18,20,  1,25,  5,95,  7,88,  4,32, 17,45,  2,11, 12,20, 16,61, 14,36,  6,45, 19, 5,  3,56, 
     2,41, 12, 6,  6,77,  4,69, 19,83,  7,17, 10,22,  1,32, 16,58, 17,63, 18,59,  3,31, 15,94, 11,90, 13,20,  8,83,  9,14,  0,67, 14,62,  5,62, 
    17,36, 11,81,  3,70, 15,20,  5,60, 13,22, 16, 8,  9,31, 14,53, 10,67,  6,66,  2,75,  0,22, 19,30, 12,47,  8,69,  4,68, 18,77,  7,10,  1, 4, 
    19,21,  6,97,  3,71,  5,13, 16,85, 14,95, 10,27,  4,10,  1,24, 11,43,  7,81,  9,88, 12,72, 17, 5, 15,51, 18,75,  8,44, 13,49,  0,43,  2,26, 
    14,69,  6,93,  4,66,  0,12,  2,81, 12,94, 10,61,  8,56, 15,56,  7,62,  3,25, 16, 2, 19, 5,  1,87, 17,75,  9,14, 13,14,  5,45, 11,30, 18, 8, 
     2,80,  1,21,  6,94,  7,86, 18,54, 16,70, 15,83, 14,39,  4,54, 11,76,  8,50,  9,18, 19,15,  0, 4, 10,34, 13,12,  3, 9,  5,47, 17,31, 12,65, 
     3,77, 11,87, 15,97, 19,90,  7,25,  2,77, 17,88, 18,76,  8,69,  4,15, 14,49,  1,27,  6,75, 10,90, 13,32,  9,26, 16,67,  0,82, 12,59,  5,72, 
     9,85, 19,40,  8,48, 10,64, 17, 8,  6,79,  4,96, 15,70, 13,25,  1,30, 14,75, 16,86, 11,72, 18,49,  5,74,  0,55,  7,87, 12,34,  3,98,  2,46, 
     7,87,  2,75,  1,55,  3,34, 11,62, 15,31,  0,53,  9,13, 14,63, 10,14, 13,20, 16,12, 17,14,  6,48, 19, 4, 12,64,  5,17, 18, 5,  4,85,  8,41, 
    17,55, 13,74,  9,15,  2,39,  3,64,  8,71,  5,18, 10,60, 15,28, 16,82,  6,93,  4,68,  1,75,  7,56, 19, 9, 11, 9,  0,20, 18,61, 14,78, 12, 5, 
     2,23, 13,66, 14,25, 10, 2, 15,13, 18,72,  1,51, 11,81, 16,47, 19, 6,  8,42, 12,44,  7,88,  0,36,  3,93,  5,21,  6, 6,  9,62, 17,18,  4,38, 
     0,84, 17, 1,  5,70,  9,63,  2,70,  8,22,  4,98, 15, 4,  7,85, 14,29,  3,81, 12,26,  6,49,  1,62, 10,49, 13,66, 16,41, 11,31, 18,51, 19,23, 
     8, 9, 11,20,  0,60, 15,22, 13,29, 14,89,  4,36,  7,51, 19,97, 12,39,  1,58,  3,50,  9,48, 10,94, 17,35,  2,95, 18,37, 16,79,  5,71,  6,12, 
     7,78,  8,95, 18,42, 12,60,  6,62, 14,99, 13,31, 16,94,  3,78, 10,27, 15,23,  5,60,  9,10,  1,80,  2,83, 11,54, 19,96, 17,15,  4,50,  0,64, 
    16,32, 15,59, 18,93, 11,95,  2,61, 14,19,  8,46,  4,75,  1,69,  9,76, 19,96,  3,50, 10,10,  7,69,  6,10, 13,98, 17,60,  0, 1,  5,81, 12,79, 
    16,23, 11,64, 19,70, 18,35, 12, 6,  0, 8,  7,97,  4, 7,  6,56, 13,92, 10,11, 15,50,  8,39,  1,72,  2,25,  5,85,  3, 4, 14,59,  9, 2, 17,27, 
     7,46, 15,90,  5,89,  2,66,  6,87, 13,35, 18,83,  0,90,  3,93, 12,54,  8,10,  9,22, 10,18, 17,97,  1,19, 14,94,  4,60, 16,90, 11,83, 19,41, 
     2,54, 17,53,  9, 4,  8,54,  5,73,  4,18, 12,26,  1,58, 18,44, 11, 1,  0,49, 15,95, 16,91, 14,38, 10,93,  3,29,  7,70, 19,14, 13, 6,  6,51, 
    16,57, 19, 4,  0,21,  2, 5, 14,64,  5,71,  9,67, 12,24, 18,39, 13,73,  3,98, 10,12, 17,34, 15,20,  7,31,  6,12, 11,40,  8,75,  1,22,  4,65, 
    14,63, 17,83, 16,29, 18,84,  0,25,  6,12, 13,81,  9,89,  3,44,  4,23, 19,66, 10,31,  1,29, 15,64,  2,67,  8,56, 11,34, 12,97,  7,27,  5,71, 
     6,99, 15,28, 17,73, 11,10,  1,85, 16,56,  5,56,  9,10, 13,30, 10,82,  4, 2,  2,45,  8,16, 12,36,  7,89, 14,27,  3,34, 19,85,  0,16, 18,75, 
     8,53, 19,33, 15,16, 18,81, 16,54, 14,15,  7,67, 17,46,  3,98,  5,72, 13,65,  2,42,  1,13,  9,40,  0,56,  6,43, 10,57, 11,56,  4,77, 12,94, 
    19,38,  8,83, 13,55,  0,80, 18,47, 16,28, 15,82,  5,82, 11,46,  6,89,  9,22, 12,69,  4,44,  2,84,  7,70,  3,32, 10,21, 17,55,  1,52, 14,13, 
    19,40, 13,55, 17,54,  0,73, 16,94, 14,80,  9,61, 12,74, 18,92, 10,23,  5,18,  4,41,  2,46, 15,94,  8,73,  7,31, 11,33,  1,35,  3,32,  6,15, 
    17,71,  4,20, 15,38,  3,72, 16,45, 18,75,  8,28,  9,30, 10,52, 11,57, 14,26,  1,77,  6,72, 13,43,  0,62,  5, 7, 19,41, 12,21,  2,58,  7,21, 
     5,18,  9,56, 15,73, 19,17, 16,31,  1,49,  3,47,  0,66, 12,17,  6,75, 11,25, 17,38, 13,43, 10,95, 18, 4,  7,71,  2,26,  8, 5, 14,26,  4,28, 
     6,33,  3,62, 16,48, 12,21, 15,79,  4,48, 11,93,  8,29,  5,99, 10,98, 18,93,  2,36, 14,49,  9,42, 17,22, 19,63,  7,43,  1,27,  0,10, 13, 3, 
    13,85, 18, 4, 15,41,  2, 8, 12,78, 19,88, 14,82, 10,53,  6,25,  0,80,  1,32,  9,28, 17,75, 11,31,  7,19,  5,11,  3, 4,  8,92,  4,31, 16, 1, 
    11,96,  0,24,  4,47, 10,35,  7,36, 17,83, 16,99, 14,22,  2,91,  3, 1,  5,43, 15,33, 12,97, 18,50,  8,41,  9,78,  6,92, 13,92, 19,40,  1,45, 
     9,97, 19,62,  1,35, 15,42, 18,51, 12,14,  2,64, 13,10, 11,95, 16, 7,  7,38, 14,11, 10,35, 17,25,  4,77,  6,82,  0,79,  8,81,  3,17,  5,33
  };
  // Taillard 100x20 instance 4
  const int tai_100_20_4[] = {
    100, 20, // Number of jobs and machines
     1,91,  0,20, 11,60,  9,13, 19,54,  8,38,  6,69, 14,66, 17,78, 13,32, 15,47,  7,81, 10,96,  2,69,  5,74, 18,91,  4,73, 12,67, 16,89,  3,81, 
    15,64,  1,49, 14,88,  9,17, 17,30, 11,77, 12,97,  3, 5, 10,87,  7,99, 13,22, 19,42,  8,50, 16,27,  6,19,  0,55,  2,77, 18,78,  4,20,  5,56, 
    14,62,  9,62,  2, 5,  0,72,  3,96,  6,25, 18,57,  8, 4, 16,36, 19, 2,  7,51, 12,13,  5,80,  4,82, 10,49, 15,96,  1,27, 13,25, 11,83, 17,42, 
     4,84,  9,31, 11, 2,  8,31,  1,15, 18,63, 13,19, 16,38,  7,75, 12,35, 15,52, 10,37, 17,91, 14,34,  2, 7,  5,63,  3, 7, 19,11,  6,45,  0,68, 
    17,15,  9,75, 18,88, 14,47,  2,61,  0,91, 10, 7,  7,28, 16, 8, 11,96,  6, 4,  4,91, 19,22,  8,60, 12,55, 13,22, 15,36,  1,37,  3,43,  5,13, 
     8, 2, 11,41,  4,28,  7,90,  9,14, 16,71, 18,71, 15,63, 17,90, 19,44,  5,87,  6,72,  3,31,  2,57, 13,42, 12, 1, 10, 6,  0,74, 14,27,  1,86, 
    11,65,  1,56, 16,77,  3,30, 10,24,  0,59,  5,13,  6,47, 17,56, 18,51, 15,55,  7,11, 19,74, 13,85,  4,46, 12,14,  2,57, 14, 8,  9,36,  8,17, 
    13,40, 14,27, 17,70,  2,49,  5,97, 16, 8,  3,20,  6,11,  9,78, 15,47, 18,69,  7,58,  4,35, 11,20, 10,52,  8, 5,  0,56,  1,68, 19,44, 12,74, 
    11,13,  9,47,  7,69, 16,24, 10,89, 18,97, 13,51, 12,41, 17,29,  6,69,  0,60,  4,50, 15,83,  5,64,  2,86,  1,96, 19,58,  8,57,  3,60, 14,47, 
     0,23, 10,40,  7,84, 13,77, 19,60,  9,53,  2,48, 11,58, 15,14, 14,16,  8,20, 12,79, 17,88,  5,25, 18,29,  3,68,  4, 3,  1,85, 16,49,  6, 6, 
    17,14,  2,41, 15,11,  8,36, 12,62,  9,82, 18,55, 16,78, 13,39,  7,99,  6,93, 19,73,  5,93,  1,91,  3,36, 10,11,  4,78,  0,70, 11,70, 14,62, 
     8,84,  2,14,  1,38, 18,36,  4,68, 11,16, 10,92, 17,41, 14, 4,  9,47, 19,50, 13,88,  6,92,  5,29,  0,23, 16,19, 12,10,  3, 9, 15,24,  7,74, 
     2,93, 12, 8, 16,35,  5,11,  9,82, 14, 2, 18,41, 10,34,  1,22,  7,98, 17,72,  4,67, 13,98,  8,19,  6,16,  0, 7,  3,82, 15,67, 19,24, 11,53, 
    10,84,  8, 6, 19,73,  2,99,  1,69, 17,77, 12,25, 18,39, 13,27, 14, 8, 15,67,  4,38,  3,62, 11, 8,  9,35,  6,44,  5,45,  0,56, 16,46,  7,35, 
    18,26, 15,83, 19,13,  9,37,  7,71,  1,24, 12,20,  3,45,  2,33, 11,48, 17,75, 16,15, 10,29,  0,15,  5,44,  4,55, 13, 1,  6,21,  8,67, 14,27, 
     4,98,  8,22,  3,97,  7, 9, 18,55, 16,90, 11,51, 13,30, 10,48,  5,47, 15,48, 17,78, 19,20,  9,14,  2,21,  1,97,  0,69, 14,25,  6,55, 12,52, 
     7,44,  1,51, 17, 7,  0,59,  3,48, 14,86, 15,30, 18,99,  8,51, 13,47, 19, 1,  2,16,  6,75, 16,29,  9,64, 12,65, 11,94,  4,83,  5,61, 10,36, 
     8,23, 15,24,  7,60, 19,59, 18,77, 17,43, 16,20,  6,99, 11,28, 10,65,  3,85, 12,22,  9,94,  0,62,  5,24, 14,56,  1,75,  4,11,  2,48, 13,26, 
    19,88,  7,62, 17,75,  6,83,  1,71,  4,32,  0, 6,  8,89,  3,44, 11,30,  9,10, 12,35, 10,78,  2,40, 15,48, 14,82, 16,77,  5,73, 13,19, 18,86, 
    11,92,  8,76, 16,53, 19,85,  2,46, 10,42,  0,34,  9,85, 14,53, 17,20,  1,24,  3,15, 18,26, 13,11, 12,28,  5,96,  6,49,  4, 7,  7,33, 15,82, 
     5,31, 11,10, 18,19,  2,83, 10,10, 15,77, 16,95,  8,57, 12,49,  1,67,  0,34, 14,21, 19,27,  4,43,  7,56,  6,93, 13,47, 17,80,  3,28,  9,78, 
     1,55, 12,46, 13,33, 14,46,  9,50,  8,24,  7,66,  5,30, 17,88,  0,26, 15,79, 10,32, 18,67,  6,24, 11,20,  4,67, 16, 4,  3, 8,  2, 4, 19,31, 
    13,15,  2,30, 14,96, 17,50,  6,73, 10,42, 11,56,  0,67,  4,16,  7,37,  5,28, 18,14, 16,12, 19,17,  1,12, 15,89,  8,90,  9,92,  3,92, 12,42, 
    14,98, 15, 2,  1,61, 11, 9,  7,48, 13,69, 17,65, 12, 8,  9,82,  3,64, 10,61,  6, 5, 16,21, 18,97,  5,63,  8,85,  0,18,  2,39, 19,91,  4,64, 
    15,70, 12,51,  3,89, 13,76, 18,77,  5, 6, 14,22, 19,29, 17, 5,  1,36,  7,54, 16,31,  9, 6,  2,87, 11,84,  0, 3, 10, 5,  4,88,  8,35,  6,89, 
     3,56,  9,80, 19,14, 17,28, 15,14,  2, 6, 10, 6, 16,79, 12,84, 11,23,  7,76, 14,38, 18,28,  8,98,  1,98, 13,38,  5,57,  4,27,  6,62,  0,94, 
    12,92, 18,26,  8,24, 10,18,  9,30, 13,11, 19,62, 16,38,  4,52,  1, 3,  3,86, 15,17, 11, 2,  5,50, 14,87,  6,10,  0,37,  7,42,  2,30, 17, 5, 
     0,48, 10,89, 11,99,  5,49, 16,54, 15,57,  7,32,  3,86, 18,75,  4,64, 13,99,  1,60,  6,71,  2,52, 12,74,  9,18, 17,13,  8,59, 19,26, 14,59, 
     9,26,  7,38,  0,98,  3,29,  4,29, 15,53, 13,50, 17, 7,  2,81,  6,78, 10,85, 19,26, 11,27,  5,52, 12, 7,  1,94, 14,86,  8,54, 18,51, 16,17, 
     8,95,  3,25,  4,71,  7,16, 11,64,  6,23, 18,77, 14,29, 16,90, 19,55,  2,83,  1,18, 12,42, 15,78, 10,33, 13,12,  5,28, 17,97,  0,61,  9, 7, 
    11, 1, 14,54,  1,42, 15,78,  2,15, 10,71,  4,69, 16,10,  3,67,  9,27,  5,82, 12,59, 18,93,  7,87, 17,55,  8,61, 19,70,  0,58,  6,86, 13, 7, 
    13,42,  8,71, 18,94, 19,73,  6,58,  9,22,  5,42,  1,13, 16,56, 11,74,  0,12,  2,26,  4,70, 14,91, 15,68, 12, 1, 10, 2,  3,12,  7,91, 17,53, 
     0,67, 14, 2,  6,68, 19,89,  1, 4,  3,83, 16,63, 18,58, 10,46,  2,62,  9,99,  8,90, 11,91, 15, 9,  7,83,  4,15,  5,83, 13,47, 17,95, 12,42, 
     2,87,  3,27, 17, 7, 14,33,  5,92, 18,52,  1,82,  0,29,  8,88, 16,21,  7,45, 12,61, 19,34, 11,53,  9,26, 15,23,  4,69, 10,73,  6,87, 13,53, 
    18,46,  9,51, 16,57,  3,88,  2,71, 11,10,  4, 9,  5,45,  0,90, 12,16, 10,55, 15,91,  6,65,  8,36, 14,19,  7,70, 17,92, 19,16,  1,76, 13, 1, 
    18,43,  5,46,  7,63, 10,62, 12,72, 19,12, 16,98, 17,53, 11,57,  3,67,  8,45,  4,90,  0,83, 14,36, 15,54,  9,85,  6,29,  2, 6, 13,36,  1,32, 
    17,28,  7,65, 12, 7,  4,50, 15, 1,  6,61, 18,56,  5,21,  2,39, 16,57, 11,43, 19,34,  8,57, 10,39,  3,22,  1,80, 13,88,  9,66,  0,90, 14,20, 
    10,10,  6, 2, 18,11,  4,82,  9,56, 16,78, 14,75,  2,40,  7,34, 13,42, 11,51, 19,17,  1,62,  8,32,  5,88,  3,70, 12,50, 17,78,  0,39, 15,98, 
     3, 8,  1,26, 18,11,  5,29, 17,99,  4, 9,  2,58, 10,60,  7,52, 14,36, 19,14,  0,47, 13,70, 12,10, 16, 1,  9,66, 15,51, 11,96,  6,41,  8,73, 
    14,75,  4,89,  1,92, 12,16, 10,52,  3,98, 19,21,  7,50,  9,61,  8,22, 11,78, 18,51,  5,70,  2,41, 13,39,  6,19,  0,91, 17,62, 15,46, 16,39, 
    11,52,  3,91,  1,24,  0, 2,  7,43, 12,37, 16,11, 19,55, 14,51,  4,27, 15,28,  5,91, 10,45,  2,78, 17,29,  9,66, 13,87,  8,86, 18,16,  6,20, 
     8, 2, 10,27, 17,80, 13,11,  4,19, 19,60,  6,58,  2,16,  3,34,  5,68,  1,89, 14,83, 11, 5,  7,43, 18,70, 15,10, 16,65,  0,70,  9,12, 12,24, 
    10,96, 15,86,  9,48,  7,74, 16,10, 13, 4,  8,55,  5,53,  2,56, 17, 1,  0,52, 12,52, 14,86, 19, 6,  6,65, 18,58,  1,80,  4,96, 11,38,  3,94, 
    18,68,  3,23, 11,50,  9,80, 10,83,  0,71, 12,33,  7,48,  6,28,  4,60,  2,76,  1,18, 14,32, 17,65, 13,33,  8,98, 19,28,  5,92, 16,95, 15,11, 
    10,74, 12,79, 17,40,  1,53, 11,24,  9,30, 15,52,  6,26, 19,90,  2,53, 16,50, 18,91, 14,70, 13,90,  5,31,  7,35,  3,66,  8,90,  4, 1,  0,21, 
     5,45, 16,17, 11,81,  8,97, 17,81,  4,67, 10,82,  6,24,  3,27, 14,52,  2,59,  7,55, 12,74,  9,25, 15,58,  1,62, 13,29,  0,88, 19,97, 18,80, 
    10, 1, 11,24,  1,11, 16,88,  0,77,  3,64,  7,27, 19,13, 14,52, 17,49,  9,19,  4,62, 18,10,  8,42, 15,97, 12,49, 13,76,  5,59,  6,36,  2,43, 
    15,92, 10,82, 16,21, 11,70,  8,89,  0,26, 19,24,  5, 1, 17,22,  7,12, 14, 6, 12,34,  3,88,  1,28, 13,54,  9, 8, 18,74,  2,41,  4, 1,  6,74, 
    19,29,  2, 7,  9,78, 12,90,  6,78, 11,94,  5, 7, 16,53, 10, 8, 15,92,  8,18,  7,59,  4,59, 13,96,  3,52,  1,69, 18,64, 17,76,  0,82, 14,36, 
    14,18,  4,64, 12,86, 16,82,  0, 6,  9, 2, 17,82,  2,46,  7,22,  6,13, 19,92,  1,38,  5,72,  3,45, 11,66, 13,48, 10,10,  8,94, 18,83, 15, 5, 
     7,27,  0,30,  9,99, 13,72,  4,17, 11,69,  2,59, 10, 3, 19,93,  1, 4, 14,30,  3,18,  8,36,  5,65,  6,93, 12,37, 15,10, 18,45, 16,14, 17,98, 
    16, 9,  7,66,  5,33, 10,35,  9,25, 13,44,  1,13,  3, 8, 14,47, 15,80,  8,99, 12,23, 18,59,  0, 2, 19,89, 11,16,  2,30, 17,45,  4, 3,  6,55, 
    11,74,  3, 7, 14, 3,  2,96,  6, 9,  9,44,  7,15, 19,16,  8,98, 15,19, 10, 8, 12, 1, 13,87, 18, 9,  5,48, 16,76,  1,26,  0,63, 17,52,  4,12, 
     8,90, 10,39,  3,58, 16,30,  0,35,  6,80,  9,69, 11, 7,  7,70,  1,52, 19,88, 17,56, 14, 8,  4,12,  2, 4, 13,48, 18,57,  5,34, 15,70, 12,13, 
    19,91, 11,39,  7,86, 12,47,  3,61, 16,19, 14,72,  6,63,  8,44, 10,95, 15,52,  0,61,  9, 7, 13,94, 18,77, 17,58,  5,11,  4,21,  1,54,  2,80, 
    10,14, 14,62, 15,13, 12,84,  1,15,  8,19,  9,70, 18,32, 11,57,  2,38, 17,90,  5,37,  6,31, 19,64,  3,20,  4,12, 13, 9,  7,75, 16,90,  0,15, 
     3,48,  8,13, 13, 5, 12,13, 18,19, 15,87,  4,80,  7, 6,  6,54, 16,41,  5,56,  9,10, 19,64,  2,26, 10,33, 11,11, 14,20,  0,32,  1,24, 17,27, 
     1,65,  3,89,  2,39,  7,77, 15,18,  0,42,  6,45,  5,83, 12,54,  4,63, 16,82,  9,50, 11, 9, 17,72,  8,46, 18,51, 14,36, 19,27, 13, 9, 10,48, 
    15,34, 18,85,  8, 2,  5,29,  3,17,  4,60, 10,69, 14,55, 16,34,  7,12,  2,17, 19,60,  9, 1, 13,63, 12,62,  0,14, 11,76,  6, 2,  1,85, 17, 9, 
     1,13,  3,44,  9,98,  6,16,  0,63, 16,31, 18,73,  4,75, 10,31, 15,46, 14,85, 19,79,  8,67, 12, 6, 11,25,  2,40,  5,39, 17,16,  7,60, 13,64, 
    17,23,  3,94, 19,55, 13,25, 12,93,  1,30,  6,95, 16,56,  9, 1, 10, 3,  7,77, 18, 1, 11,52,  8,52,  5,21,  2, 5, 15, 1,  0, 9,  4,23, 14,78, 
     2,10,  4,68,  5,56,  0,72,  7,63,  3,80, 16,91, 11,80,  6,22,  8,70, 18,39, 10,38, 17,57, 12,79,  1,60, 14,19, 15,95, 13,35, 19,41,  9, 3, 
    14,55,  5,50,  8, 2,  3,69, 11,44, 15,19,  4,70, 13,71, 16,53,  1,98,  7,65, 12, 4, 19,46,  9,94,  6,48,  2,86, 10, 6,  0,34, 17,97, 18,45, 
    18,85,  0,38, 11,27,  2,93,  1,81,  4,17, 15,14, 16,78, 17,89,  3,90, 10,97,  5,97, 19,12,  9,28,  8,72, 14,93,  6,18, 12,90,  7,53, 13,23, 
     9,83,  0,11, 15,94, 13,44, 12,80, 18,94, 14,24,  6,65, 10,22,  7,74, 16,62,  1,58, 11,86,  5,49,  8,94,  2,29, 19,75,  4,13, 17,26,  3,82, 
    13,31,  7,76, 11, 9,  2,16,  4,28, 10,88,  6,97, 19,43,  8,14,  0,89, 17,38,  1,66, 18,62, 14, 8,  3,41,  9,81, 15,38, 12,16, 16,26,  5,16, 
     7,51,  9, 1, 12,80, 11,63,  3, 1,  2,61, 18,98, 13,58, 10,47,  1,15, 14,83, 16,16,  8,50,  6,36,  0,13,  4,46, 17,97,  5,87, 19, 3, 15,50, 
    13,98, 16,65,  1,22, 19,31, 14,38, 11,45, 15,72,  4,72, 18,37,  5,63, 10,37,  0,50,  9, 6,  7,30,  2,11, 12,34,  6,46,  8,34,  3,37, 17,82, 
    15,31, 10,19, 12,30, 13,80,  5,24, 17,74,  2,67,  6,39, 11,35,  0,79,  3,42,  1,24,  7,45, 19, 5,  8,13, 14,44, 16,93,  4,33,  9,34, 18,28, 
    11,19,  4, 4, 15,52,  8,82, 13,42, 14,71,  2,19,  3,47,  7,17, 10,67,  6,38,  0,23,  5,79, 12,95,  9,21, 16,56, 17,19, 19,52,  1,25, 18,68, 
    10,58,  9,37, 11,46, 19, 4, 12,73,  7,86, 18, 8,  6,51,  8,46, 14,38, 16,33,  0,34,  2,89,  3,90,  1,73,  5,81, 15,37, 17,50, 13,80,  4,63, 
    10,74, 18,26, 15,75, 19,40,  3,29,  1,79, 16,83,  2,40,  5,35, 13,53, 14,57, 12,99,  0,57,  7,94,  8,99, 11,53, 17,93,  9,75,  6,29,  4,79, 
    18,39, 10,21,  2,51,  6,92,  7,92,  1,36,  4,73,  5,53,  0,62, 17,97, 15,62,  3,17, 11,80,  9,29, 14,86, 16,89, 12,26, 13,29,  8, 9, 19,83, 
    14,89, 18,88, 19, 9, 11,63,  6,64, 16,85, 10,95, 15,87,  5,44,  1,15, 13,23,  8,10,  3,93, 12,67, 17,69,  7,13,  9,35,  2,71,  0,51,  4,90, 
     1,53, 14,72,  2,49, 18,11,  6,52,  3,61,  0,36,  4,99, 11,49, 12,83, 19,10, 13,32, 17,39, 16,39,  8,86,  5,13,  9,54,  7,91, 15, 6, 10,51, 
    16,77,  1,59, 11,83, 17,50, 12,88,  9, 8, 14, 9,  0,58,  6,45, 18,25,  7,11,  3,34, 13,68, 19,35,  5,26,  4,48, 15, 1,  8,48,  2,12, 10,56, 
     4,24,  3,60,  0,20,  8,16, 12,65,  2,66, 16,47, 18,43, 14,63, 17, 9, 15,14,  6,70,  5,46, 11,32, 10,54, 13,17, 19,85,  7, 6,  9,12,  1,11, 
    13,89,  9,19,  1,25, 12,62, 11,72, 18,21,  6,32,  8,56,  7,61,  2,23, 16,33,  5,66, 19,65,  3,10, 10,27,  0,39, 15,46,  4,31, 14,23, 17,24, 
    11,15, 19,99,  0,14, 12,33,  5,81, 18,39, 10,49, 17,96,  9,31,  2,62,  3,40,  6,54,  7,53, 16,91,  1,87, 13,36, 14,79,  4,27, 15, 2,  8,34, 
     6,41, 13,15,  7,27, 19,90,  1,91, 14, 2, 15,69,  3,89, 17,30, 11,67,  2,37, 12,22,  9,79,  8,78,  0,73, 10,71, 18,67,  4,48,  5,10, 16,54, 
     4,61,  7, 2, 13,27,  9,20,  5,75,  6, 2,  0,58, 15,36, 17,54, 14,46, 10,85, 19,27, 11,63,  2,42, 16,88,  8,75, 18,30,  1,46,  3,19, 12,12, 
    12,19, 18,59,  3,73, 13,90,  7,20, 10,27,  4,36,  6,16,  9,53,  8,68,  2,10,  5,59, 11,83, 19,22, 16, 6, 17,91,  1,22, 14,74, 15,99,  0,61, 
    18,97, 15,85, 11,29,  7,20, 10,99, 14,29,  6,49, 16,86,  3,96,  2,83,  0,14,  4,95, 13,17,  9,45,  1,65,  8,48, 19,64,  5, 9, 12,26, 17,36, 
     7,59, 18,54,  6,77,  4,99,  1,90,  9,61, 19,65, 17,58, 16,42,  2,86, 15,64,  5, 3, 13,55,  8,79,  3,88, 14,13,  0,63, 12,66, 11,83, 10,85, 
     7,92,  0,29, 11,62, 14,81,  9,81,  2,97, 10,96,  3,66,  4,81,  1,75, 13,93,  8,51, 17,73, 16,76, 15,33, 19,34,  5,95, 18, 4,  6,59, 12,69, 
    13,18, 14,59, 11,15,  7,56, 19,85,  1,32,  6,68, 15,70, 18,24,  4,53,  8,12,  5,69, 10,35,  3,89, 16, 6, 12,41, 17,87,  9,41,  0,85,  2,51, 
     2, 4,  6,70, 10,84,  8,80, 11,17, 15,43, 13,50,  5,17, 19,50, 18,66,  7,24, 12,15,  9,90,  1,83, 14,45,  3,15,  0,72, 17,26, 16,89,  4,83, 
    15,13,  7,52, 14,88,  0,54, 12,50,  3,59,  6,51, 19,36, 13,87,  9,98, 11,26, 10,56,  5,89,  2,38, 18,48, 17,58,  1, 3,  8,72, 16,10,  4,81, 
    19,63,  4,29, 15,64, 13,50, 11,11,  7,38,  0,19,  3, 7, 14,59,  6,40, 12,56, 17, 5,  2,59, 16,83, 18,46,  1,41,  5,90,  9, 4, 10,24,  8,34, 
     4, 9,  0,82, 11,69, 10,14, 15,77,  8,39,  9,27, 16,98,  7, 8,  6,53,  2,15, 19, 1, 14,68, 12,56,  5,72,  1,61,  3,49, 18,35, 13,67, 17,21, 
     1,84,  2,41, 10,78,  4,53,  7,82, 14,32,  9,51, 11,27, 19,56, 18, 4,  8,19,  5,41, 12,80,  0,33, 16,71,  3,18, 15,13, 17, 3, 13,43,  6,95, 
     7,68, 18,59,  2,67, 15,24, 10, 2, 11,69, 13,45, 17,85,  9,51, 19,46, 14,86,  1,11,  4, 2,  6,97, 16,53,  5,44,  8,64, 12,33,  0,32,  3,53, 
     7,24,  8,97, 14,11, 15,69,  1, 6, 10,12, 16,84,  3,32, 13, 1,  0,90,  6,32,  2,98,  5,79, 17,36,  9,56,  4, 1, 19,17, 18,84, 11,80, 12,64, 
    12,47, 17,64,  5,16, 18,64, 16,69,  1,16,  7,45,  2,62,  0,16, 10, 5, 11,26,  3,70, 14,48,  9,58, 19,75, 13,13, 15,11,  4,47,  6,50,  8,29, 
    17,72, 10,51, 19,16,  4,32,  3,43,  8,61,  1,11,  5, 1, 14,74, 12,17,  9,15,  2,72, 13,87, 18,81, 11,26, 16,26,  6,74,  0,30,  7,79, 15,40, 
     5,59, 11,69,  1,98,  6,60, 16,73, 14,37,  9,35, 15,47, 13,17,  3,93, 18,83,  7,27,  0,95,  2,77, 19,67, 17,30, 10,78,  4,26, 12,45,  8,55, 
     1,42,  3,93, 13,88, 17,89, 14,31, 19,89, 11,59,  6,86,  8,28, 10,23, 16, 4, 12,72, 15,43,  7,28,  0,52,  9,18, 18,60,  5,91,  2,86,  4, 6, 
     2,84,  3, 8, 13,84,  4,67, 17,84, 14,11, 12,89, 19,38, 18,12, 15,74,  9,14, 11,50,  5,83, 10,42,  8,77,  0,88,  1,58,  7,58, 16, 8,  6,17, 
    12,28, 14,15, 11,97,  5,31,  1,15,  6,35, 16, 9,  9,59, 18,50, 13,98,  0,86, 10, 6, 17,49,  3,24, 15,89,  4,56,  7,85, 19,14,  2,78,  8,20, 
     1,51, 11,25, 14,56,  0,67, 18,51, 13,44,  4,80, 19,66,  5,23, 12,74,  7,12, 16,52, 17,13, 15, 4,  8,36,  2,69,  3, 5,  6,21,  9,57, 10,79
  };
  // Taillard 100x20 instance 5
  const int tai_100_20_5[] = {
    100, 20, // Number of jobs and machines
     9,53, 19,48,  7,17,  6,42,  2,70, 13,99, 11,23, 18,79, 16,42,  8,88, 14,23,  3,77,  1,59,  4,45,  5,84,  0,58, 15,47, 10,91, 12,29, 17,48, 
     3,52,  2,74,  6,35, 15, 3, 16,45, 18,36,  8,92, 14,20, 10,58, 12,14, 13, 5,  1,93, 17,46,  0,36,  5,94, 11,76,  7,63,  4,95,  9,66, 19,49, 
    12,42, 11,62, 13,83, 16,59,  7,60,  9, 8,  3,15, 19,10, 10, 1, 14,78,  2,27,  0,98, 18,35,  6,22,  1,80, 17,77, 15,86,  5,63,  4,44,  8,39, 
    12,10, 19, 6,  5,62, 11,67, 14,66,  4,38, 13,20,  7,76,  6,94, 15,67, 18,34,  1,59,  0,27,  3,61, 17,10,  2,67, 10,21, 16,90,  8,73,  9,14, 
    12,17,  2,37, 10,21, 11,91,  8, 7, 18,44, 17,22,  6,50,  5,31, 13,82,  9,53, 14,21,  4,89, 16,35, 19,48, 15,16,  3,70,  1,93,  0,41,  7, 5, 
    12,63, 14,68, 18,45,  0,41,  9,78,  3,74, 15,97, 16,14,  2,48, 13,54, 17,91,  7,92, 11,91, 10,64,  5,61, 19, 6,  8, 6,  4,60,  1,65,  6,73, 
     2,91, 18,58,  8,75, 10,11, 17, 7, 12,86, 19,70,  9,26,  7,53,  4, 5, 11,81, 14,66,  1,57,  3,77,  0,89, 16,49, 13, 3, 15,59,  6,93,  5,37, 
     5,33, 11,97,  0,81, 18,35, 16,83,  6,28,  3,47,  7,34, 13,23,  9,96, 10,31, 14,94, 19,63, 15,21, 12, 1,  1,56, 17,36,  8,50,  4,91,  2,88, 
     8,37,  1,70,  7,38, 15,84, 17,25, 12,32, 14,72, 18,37, 16,23,  0,14,  3,44, 13, 3,  9,25,  4,91, 19,97, 11,88, 10,64,  6,81,  5,65,  2,59, 
    11,40, 13,23,  0,52,  4,39,  8,64, 19, 9, 14,65,  2,22, 17,71, 12,51, 18, 7,  6, 7,  3, 7,  5,34, 10,59, 15, 2, 16,45,  1,41,  7,96,  9,20, 
     0,48,  9,63,  2,47,  7,12,  4,56,  3,86, 15,86, 17, 1,  6,20,  1,80, 10,82,  8, 4, 19, 8,  5,55, 16,85, 18,75, 11,22, 14,11, 12,46, 13,48, 
    16,76, 11,56,  6,19, 13, 7, 17,44, 14,90,  2,23,  5,12,  1,57,  4,99,  8,75,  9,22, 15,59, 19,96,  3, 5,  0,67, 18,94,  7,64, 12,88, 10,77, 
    11,87, 14,22, 18,14,  8,19,  1,98,  9,68,  5,47, 10,97,  2,10,  3,71, 16,66,  0,42,  4,21, 17,49, 13,87, 19,49,  6,54, 12,74, 15,92,  7,25, 
     3,22, 19,64, 14,21, 10,44, 16,34,  7,43,  0,14,  1,73, 12,38,  5,58, 11,52,  6,26, 18, 6,  8,56,  4,98,  2, 2, 15,98,  9,49, 13,91, 17,65, 
     6,58,  9,56, 11,73,  4,22, 14,65,  8,94, 10,98, 19,13,  5,25, 12,66, 18,35, 17,50,  3, 8, 15,47,  0,33,  1,24, 16,53, 13,39,  7,43,  2,96, 
     6,99,  1,72,  5,40, 18,88, 13,39, 15,26,  9,57,  3,81,  7,63,  8,71, 16,93, 14,59, 10,12,  4,71, 17,87, 11,52, 12,67,  0,27, 19,70,  2,91, 
    12,63, 16,87,  2,72,  3,44,  0,20, 18,77,  9,48, 10,34, 19,62,  4,64, 13,26, 15,68,  6,92, 17,18,  7,51,  5,43, 11,15,  8,70,  1,23, 14,56, 
     6, 6, 15,39,  0,65,  5,47,  2,69, 12,17, 19,35, 10,94,  3,52,  7,58, 13,63,  4,63, 11,89, 16,56,  8,30,  1,24, 18,37, 14, 3,  9,61, 17,12, 
     4,80,  1,50, 15,35, 19,43,  0,96, 18, 3, 16,81, 14,10, 12,14, 11,32,  3,54,  9,21, 10,57,  8,78,  2,56, 17, 8,  6,60,  5,37,  7,47, 13, 2, 
    12,93, 19,46, 13,47,  8,13, 10,87, 14,60, 17,83,  1,69,  7,32,  3,12,  4, 2, 18,89,  5, 3,  6,12,  2,14, 15,99,  0,69, 11,12, 16,79,  9,72, 
    15,99, 12,55, 11,44, 19,16,  3,49,  0,86, 14,40,  4,40,  1,41, 13,36, 18,93,  8,34, 17,88,  2,90,  7,99,  5,37, 16,54,  6,82, 10, 4,  9,55, 
     4,31,  6,26,  5,12,  7,48,  1,41,  2, 2, 13,25,  3,17,  9,32, 18,77, 16,98, 15, 2, 12,82,  0,92, 11,33, 17,53, 10,53,  8,28, 14,27, 19,86, 
    19,86, 17,59, 18,40, 12,42,  5,46, 15,78,  0,14,  8, 7,  4,49,  1, 6,  6,28, 10,12, 13,73, 16,35, 14,64,  2,57,  7,77, 11,64,  9,63,  3,15, 
    16,17, 15,59, 18,60,  8,37,  1,95,  7,78, 17,61, 10,45,  0,51,  9,67, 12,34, 14,45, 11,47,  5,86, 13,74,  6, 5,  3,67,  4,79,  2,27, 19, 9, 
    12,70, 11,23,  1,29,  0,78,  8,54,  6, 3, 15,27,  3,82, 17,30,  9,90,  7,27, 19,95, 13, 8, 18, 9, 14,58,  4,41, 16,25,  5,87, 10,35,  2,24, 
     9,39,  1,62, 19,80, 16,74,  7,25,  5,43, 17,79, 11,52,  8,12,  0, 6,  2,67,  3,46, 18,37,  4,14, 12,37, 15, 2, 10,82, 14,12, 13,38,  6, 9, 
    10,84,  5,87, 15,40, 16,12,  2, 9,  9,62,  4,27,  0,68,  8,24, 19,79, 14,52, 13,18,  1,57, 12,93, 17,61, 18,15, 11,61,  3,21,  7,93,  6,57, 
     8,54,  5,89,  9,30, 19, 4,  2,37, 11,72, 18, 1,  4,10,  3,65,  7,81, 12,22,  1,17,  6,72, 10,41, 14,51, 15,42, 16,21,  0,39, 17, 1, 13,59, 
     4,65,  6,82, 12,11,  5,85,  9, 8, 15,53,  0,89, 16,29, 11,60,  3,22,  7,46,  8,29, 17,22, 18,71, 13,35, 19, 6,  2,22, 10,77, 14,69,  1,30, 
    16,43, 15,35, 10,84,  3,68,  5,85, 14, 3, 12,11, 18,13, 13,66,  0,74,  1,30,  8,54, 19,12, 17,94,  7,85,  4,86,  2,74, 11,17,  6,88,  9, 6, 
    11,81,  5, 3,  3,69,  4,81,  0,52,  9,59, 16,88, 17,99, 13,63, 12,35, 18,48, 10,26,  2,44,  1,38, 14,32, 19, 4,  7,80, 15,44,  8,86,  6,90, 
    15,35,  2,14, 11,63, 12,25,  5, 7,  1,22, 10,72,  3,38,  7,63, 14,80, 18,65, 19,84, 13, 8,  4,18, 17,79,  8,42,  6,94,  9,89,  0,29, 16,40, 
    16,98, 17,76, 13,87,  2,69,  1,65,  6,64, 14,58,  4, 7, 18,18, 15, 7, 19, 9,  0,36, 12,48,  8,48,  9,83, 10,43,  5,35,  3,84, 11,32,  7,84, 
    14,22,  0,95, 17,92, 19,95, 18,70, 11,84,  4, 3,  3,56, 12,31, 15,10, 10,35, 13,47, 16,94,  9,61,  7,45,  2,47,  1, 4,  5, 1,  6,35,  8,53, 
    16, 1, 15,22, 14,46,  6,91,  4,78,  5,48,  1,32, 19,93,  0,28, 10,91,  2,16, 12,52, 17,48, 13,43,  8,11,  9,68,  3,44,  7,51, 11,11, 18,33, 
    14,26,  3,65,  5,93, 18, 8,  6,44, 16,98,  1,50, 19,36, 10, 5,  2,68, 12, 6,  9,49,  0,46,  4,55, 17, 1,  7,94, 13,13, 11,39, 15,90,  8,40, 
    10,63, 16,30,  7,24, 11,41, 14,97,  5,71, 13,92, 12,91, 17,22,  6,97,  9,49,  2,40,  3,37,  1,36,  8,23, 15,83, 18,53,  0,71, 19,64,  4,76, 
     5,37, 15,60, 13,79, 16,25, 18,83, 19,21,  8,10, 11,78,  4,75,  6, 9,  3,70,  2,36,  0,41, 17, 8,  1, 1, 10,19, 12, 1, 14, 6,  9,67,  7,46, 
    13, 5,  0,29, 11,76,  7,16, 18,81,  5, 9,  6,94,  8,82, 14,71,  4, 5,  9, 3, 10,38, 19,25, 15,85,  1,45,  2,62,  3,30, 12,58, 17,10, 16,20, 
     5,85, 11,99, 13,30,  9,23, 17,91,  3,32, 14,61, 12,42, 19,11,  4,56, 16,33,  1,30, 15,27,  2,19, 18,14,  0,81, 10,21,  8,56,  7,57,  6,68, 
     1, 4,  7,20, 15,37,  9,62, 19,79,  5,62, 17,71,  8,18,  2,82, 11, 7,  3,42, 12,97,  6,10,  0, 6, 14,25, 10,70, 16,27, 13,87, 18,69,  4,53, 
    12,14, 13,19,  6,36, 18,52,  8,88,  2,67, 14, 2,  1,30, 16,83,  4,86,  7,97,  9,33,  0,23, 19, 5,  5,42, 11,77, 10,47,  3,58, 17,23, 15,44, 
     5,82, 13,73,  9,91, 11,78, 10,54, 16, 1,  4,30, 18, 9,  0,37, 17, 3,  8,71,  3,26, 14,98,  2,70,  7,35, 19,55, 12,72,  6,47, 15,35,  1,85, 
    10,36,  1,14, 17,96,  2,61, 19, 3, 14,30,  0,98,  8,75, 15,41, 16,79,  3,96,  9,73, 18,54,  4,72, 12,29,  7, 6,  6,31,  5,32, 11,34, 13,50, 
    19,93, 13,16,  3,96,  7,73, 10,94,  6,36,  9,29,  2,38, 14,63, 11,14, 17, 7, 12, 7,  4,56, 16,54,  5, 7, 15,83, 18,33,  8,60,  0,88,  1,58, 
    16,64, 12,11, 14,13, 19, 4,  9,74,  8,23, 15,84, 18,62,  0, 8,  6,42,  2,96, 11,26, 13,41,  7, 2,  4,28,  3,14,  1,99,  5,14, 17,65, 10,80, 
     4,10,  3,99, 10,20, 15,80, 11,60,  5, 2,  0,98, 12,97, 14, 1,  1, 1,  2,59, 19, 1, 16,26, 18,99,  8,89,  6,38,  7,15, 17,47,  9,74, 13,14, 
     1,52, 19,50,  5,13, 14,73, 12,88, 13,67, 18,78,  9,82,  7,18, 16,86,  0,27,  6,34, 10,26, 15,30, 17,17,  4, 2, 11,87,  2,85,  8,40,  3,77, 
     4,70,  6,98,  3,23, 13,29,  1,88, 10,31,  2, 7,  5,92, 18, 3, 16,80, 17,18,  7,39, 15,57,  8,56,  9, 8, 19, 1,  0,79, 14,90, 11,74, 12,55, 
    19,25,  6,29,  0,51,  3,83,  5,91,  4,93, 12,44,  8,16,  2,23,  7, 7, 17,57, 18,56,  9,72,  1,41, 10,68, 14,25, 13,63, 16,43, 15,87, 11,53, 
     9,35, 18,12, 12,89, 10,41,  1,70,  2,18, 13,70,  6,69,  8, 5, 17,59,  7,30,  4,34, 11,23,  3,42,  0,45, 15,92, 19,92, 16,60, 14,98,  5,41, 
    11,61,  1, 4,  2,30, 15,50, 13,50,  8,39,  4,41, 10,15, 16,22, 14,55, 12,37, 17, 7, 18,88,  6,58,  5,90,  0,64,  3, 7, 19,23,  7,19,  9,37, 
    16, 4, 12,57, 10, 1,  2,80,  6,21, 18,35,  9,32, 17,98,  0,78,  1,50, 15,79,  4,53, 19,40, 11,19,  7,25, 13,84,  8,91,  5,51, 14,95,  3, 4, 
    13,73,  5,92,  8,25, 19,41,  9,58, 15,36,  1,50, 10,90,  4,98, 17,39, 18,94, 14,64,  2,15,  0,29,  7,27,  3,89, 16,50, 12,26, 11,38,  6,11, 
    14,82,  2,37,  3,99,  6,34, 12,94,  5,46, 16,74, 19,45,  4,78,  1,41,  9,58,  7,43,  0,71, 15,59, 11,25, 17,55, 18,84, 13,48, 10,84,  8,57, 
     2,11, 11,53,  0,39, 15,81, 19,66,  6,99,  4,73,  3,76, 17, 7, 14,91,  9,33, 16,47, 18,14,  8,51,  1,25, 13, 4,  5,15,  7,26, 10,79, 12,70, 
     6,79, 15,97,  9, 9,  0,56, 14,50,  8,43, 13,55,  1,32,  4,91,  5,75, 17,83, 18,17, 12,55, 10,43, 19,80, 11,62,  2,60,  7, 3, 16,56,  3,33, 
    13,97,  2,40, 12,32,  3, 7,  5,85,  6,60, 17,19, 19,13,  8,54, 10,62, 16,47,  0,11, 15,32, 11,49, 18,65,  9,69,  7,67,  1,96,  4, 7, 14,71, 
    19, 7,  7,54, 15,19, 14,14, 10,68,  9,74,  4, 1, 17,62, 13,53,  2,51,  8,35,  6,23, 11,88,  1,68, 18,53, 16,39,  3,65,  0,64, 12,93,  5,93, 
    12, 9,  0,28, 17, 6, 18,73,  6,73,  5,71, 11, 7,  3,57,  2,95, 10,93, 15,56,  7,67,  9,23, 13,30,  8,98,  1,37,  4,34, 14,27, 19,45, 16,62, 
     3,32,  0,71, 14,47,  5,16,  4,50, 15,96,  2,31, 17,10, 12,63, 10,10, 16,74,  7,88,  6,32, 11,50,  8,93,  9,41, 19,55, 13, 5, 18,69,  1,83, 
     7, 4,  4, 8, 15,91, 13,54, 11,59,  9,51,  1,58,  8,55,  6,63,  0, 3, 12,64, 18,38, 19,21, 14, 8,  3,69, 16,93, 10,95,  2,61, 17,37,  5,74, 
     7,86,  4,46,  1,71, 19,94,  6,84, 14,72, 13,51, 12,75,  0,46, 17,17, 11,35, 18,22,  5,43, 15,64, 10, 4,  8,32,  9,22,  3,71, 16,26,  2,62, 
     1,44,  9,55,  5,12, 14,38, 17,35,  0, 6,  3,61,  2,93, 18,18, 13,62,  4,42, 11,23, 12, 9,  6,71,  8,79, 19,84,  7,90, 10,67, 16,92, 15,48, 
     1,78, 18,22, 12, 8,  0,85,  5,92, 11, 8,  9,45, 14,38,  2,20, 15,38,  8,73,  6,53,  3,44, 17,19, 10,75, 13,41,  7,71,  4, 6, 19,84, 16,74, 
     8,30, 19,41, 18,20,  7,49,  4,60, 16,59,  6,40, 15,72,  9,31, 12,26,  1,55, 14, 3, 17,61,  2,57, 10,58,  3,87,  0,88, 13,40, 11,94,  5,78, 
     7, 8,  2,15,  1,66,  6,46, 11,22, 12,66, 14,13, 18, 7,  9,30, 15,64,  3,55,  8,31, 17,96, 16,82, 19,60,  0,14, 13,63,  4,56,  5,71, 10,63, 
    16,29,  3,10,  5,78, 10,46, 15,94,  8,97,  6,85, 14,86, 13,63, 19,50, 18,49,  7,27, 17,63, 11,81,  2, 4,  0,80,  4, 9, 12,95,  1, 6,  9,83, 
     5,63, 11,28,  1,37,  0,62, 16,56, 10,73,  6,54,  2,70,  7,53,  8, 7, 12,35, 14,64, 18,22,  4,98, 15,41, 19, 6, 17,88,  9,20, 13, 5,  3,86, 
     8,84,  3,84,  2,21, 14,35,  0,54,  1,98, 16,95,  6,54, 11,63,  9,59,  4,74, 18,90, 19,68, 10,98,  5,39, 17,83, 13, 5,  7,19, 12,74, 15,73, 
     2, 1, 16,41,  9,31,  6,10, 19,99,  3,94,  0, 9,  7,69, 12,55, 17,92, 18,17,  5,89, 14,73, 10,10,  4,98, 13,38, 15,13, 11,52,  1,48,  8,87, 
    11,70,  7,40, 15,18, 14,41,  2,26,  0,65,  1, 7,  4,56, 12,32,  9,37,  3,32, 19,48, 16,77,  5,26, 17,91, 10, 5, 13,33, 18,74,  8,90,  6,97, 
    15, 1, 11,55,  8,80, 14,27,  3,58, 13,92, 18,18,  0,91,  4,23,  6,96,  1,86, 17,40, 19,17, 12,39,  9,49,  5,31,  2,60,  7,53, 16,74, 10,23, 
     6,73, 17,19,  5,89,  0,90,  7,27, 15,84, 18,73, 19,30,  1,51, 12,33,  2,84, 10,21,  4,38,  8,29,  3,34,  9,12, 14,57, 16,46, 13, 9, 11, 8, 
    16,65,  6,39, 17,15,  5,72, 11,65,  9, 6, 13,99, 19,83,  1,65, 14,84,  2,85,  7,90,  8,23, 18,45,  0,74, 15,88,  4,26, 12,41, 10,61,  3,28, 
     3,86,  5,60, 17,48, 15, 8,  6,18, 13,14, 12,71, 16,41, 14,76, 10,57,  0,72, 18,23,  1,81,  7,50,  8,60, 11,15,  4,33, 19,54,  2,67,  9,80, 
    12,25, 15,19, 19,88,  3,68, 10,40, 16,42,  2,60, 18,29, 11,87,  7,57,  9,77,  8,23, 17,91, 13,34,  0,96,  1,22,  4,84,  5,26, 14,66,  6,76, 
     5,41,  6,23, 12,34,  3,31,  4,65, 10,43, 15,51,  0,35, 17,42, 14,56, 19,29,  8,79,  7,22, 16,66,  9,68, 13,32,  1,21, 11,82,  2,42, 18,66, 
     1,29, 17,24, 18,48, 15,13,  0,93, 14,68,  5,17, 11,51,  8,49,  3,76, 16,40,  6,88,  2,85,  9,43, 13,56,  4,31, 12,77,  7,88, 10,53, 19,53, 
     8,17,  6,18,  1,36, 10, 1,  5,54, 12,20,  0,95, 11,20,  2,57,  3,42,  4, 3, 15,55, 17,18, 13,39, 19,85, 18,28,  7,21, 14,49,  9, 7, 16,98, 
     0,89, 14,97, 18,66, 15,17,  5, 2,  7,32, 12,94, 11,71, 10,87, 16,45,  6,45,  9,21, 19,91,  2,30,  3,33,  8,20, 17,97,  4,34, 13,15,  1,24, 
    10,57,  2,89, 18,78, 13,21, 11,58,  9,78, 14,25,  4,51, 15, 7, 12, 1, 19,61, 17,92,  1,13,  3,73,  6,35, 16,46,  0,61,  8, 4,  7,68,  5, 3, 
    10,77,  4,79,  7,61, 18,52,  8,74,  9,31, 17,71, 19,98,  1, 4, 14,62, 16,44, 15,38, 11,25, 13,29,  5,40,  0,30,  2,65,  3,79, 12,57,  6,10, 
     0,50,  3,95,  1,33, 19,34, 15,49,  8,25, 16,22, 11,87, 13,46, 10,13,  5,96,  9,98,  6,43,  7,80, 17,70, 12, 7, 14,97,  4,19, 18,74,  2,47, 
    10,83,  6,93,  8,90,  2,96,  7,75, 15,41, 13,68,  9,43, 18,77, 12,32, 16,75,  1, 1, 17,28, 11,11, 14,90,  5, 5,  4, 4,  3,63, 19,79,  0,92, 
    18,61, 12,65,  6,48, 19, 2, 15,98, 16,44,  1, 4,  2,50,  7,66,  3,97, 17,26,  4,85,  0,71,  9,52, 11,32, 13,98, 14,60, 10,88,  8,75,  5,43, 
     9,97, 15,42,  8,77, 14, 4,  0,80,  7,95,  3,17, 18,50,  4,32, 13,44,  2,93, 12, 2,  6, 5, 19,68,  5,78, 16,21, 11,70, 17,18, 10,49,  1, 5, 
    16,86, 18,18, 15,71,  0,19,  6,13,  2,72, 12,12, 14,64,  3,78, 11,37,  9,69, 13,13,  7,78,  8,60,  5,71, 19,63, 10,77, 17,41,  4,84,  1,71, 
     9,17,  6,80, 12,96, 18,44, 15, 2, 11,92, 17,32,  3,57,  7,74, 14,32, 10,48,  0,61,  4,47, 16,78, 19,81,  8,86,  1,29,  2,64, 13,48,  5,34, 
    16, 8, 13,79, 17,71, 14,87, 12, 1, 15,61, 18,82,  8, 7, 19,53,  6,70,  2,54, 11,62,  5,75, 10,13,  0,53,  3,53,  4,78,  1,97,  9,18,  7,39, 
    16,70,  2,55, 14,92,  3,59, 19,62,  9,16,  6, 4, 17,29, 15,18,  0,20,  7,56, 11,82,  5,33, 12,77, 18,64,  8,25, 10,75,  1,46, 13,54,  4,55, 
    13,71,  9,69,  1, 9, 18,92,  7,69, 12,91, 19,63,  2, 3, 11,96,  6, 9, 17,48,  3,29, 16,14, 14,15, 15,11,  4, 2,  8,43,  0,52,  5,54, 10,96, 
     3,72, 19,65,  9,18, 16,80, 14,55,  5,45,  4,34,  6,67, 15,59, 11, 5,  1,38,  7,99,  8,91, 10,15,  2,42, 13,74,  0,24, 17,23, 18,10, 12,53, 
    11, 3,  7,17, 15,78, 16,72,  3,63,  2,92, 12,35,  5,62, 17,86, 19, 5, 14,55,  6,77,  9,95, 10,49,  8, 6, 18,58,  0,16, 13,78,  1,62,  4,73, 
     8,38, 15,51, 13,28, 14,76, 18,38, 16,96, 19,91,  0,32,  7,90, 17,73, 12,56, 11,67,  4, 4,  1,55,  3,77,  9,20,  6,43,  2,62,  5,79, 10,49, 
    17,37,  8,71,  2,80, 11,33,  9,95,  4,41, 10,15, 15,67,  5,10, 19,94,  0,63, 14,97,  6,56, 16,42, 18,35,  3,63,  7,16, 12,92,  1,13, 13,88, 
     0,14, 18,41, 12,82,  4,47,  8,49, 14,52,  5,64,  6,25, 16,12, 13,56,  2,47,  7,42,  9,77, 15,89, 17,92,  1,41,  3,75, 19,23, 11,40, 10,91, 
     9,99, 15,52, 14,30, 12,27,  7,86,  3,29,  5,80, 19,78,  0,76,  2,13,  1,12,  8,23,  4,66, 16,30,  6,66, 18,52, 10,67, 11,42, 13,91, 17,84, 
    19,94,  4, 7, 10,40, 18,29,  1,87, 17,18,  0,82,  8,75,  5,32,  9,45, 14,19,  6,23, 12,69,  7, 1,  2,92, 15, 9,  3,29, 13,37, 16, 8, 11,78, 
     4,79, 11,39,  9,46, 17,95,  0, 4, 12,55, 14,80,  1,79, 19,35, 13,11,  2, 4,  3,24, 15,63, 16,14,  6,34,  8,14, 10,57,  7,58,  5,14, 18,60
  };
  // Taillard 100x20 instance 6
  const int tai_100_20_6[] = {
    100, 20, // Number of jobs and machines
     3, 1, 10,32, 16,15, 19,15,  7,72,  1,41, 12,49,  2,17, 15,12,  5,39,  0,66, 17,64,  9,10, 13,24, 14,60, 18,76,  6,72,  8,47, 11,99,  4,92, 
     2,28, 18,62,  9,64, 16,95, 14,94,  0,86,  3,87,  6,11,  5,46,  4,64, 15,86, 12,76,  8,11,  1,32, 10,44, 17,31, 11,77, 19,99,  7,16, 13,97, 
     7,78,  2,28, 11, 2, 14, 3,  0,88, 12,69, 13,34,  3,99,  8,33,  1,70, 18,18, 16,22,  5,41, 17,20, 15,21,  9,44,  4,66, 10,18, 19,73,  6,80, 
     1,20, 18,22, 12,76,  6,43,  8,60,  5,91, 19,88, 11,26,  9,12,  3, 8,  2,65, 13,39, 17,49,  4,83, 10,30,  0,78, 15,21, 14,96,  7, 6, 16,56, 
     6,56, 12,52,  8,73, 10,20, 11,76, 19,73,  2,21,  3,21,  4,40, 18,69, 16,19,  5,93,  0,92, 15,74, 13,88, 17,74,  7,95,  9,43,  1,50, 14,77, 
     4,79,  7,57, 17,70,  5,34, 16,28, 15,28, 12,84, 11,40,  0, 6,  6,53, 19,42,  2,99,  3, 7, 18,18,  1,23,  9,12, 14,87, 10,86, 13,13,  8,66, 
     3,67, 16, 2, 19,69,  6,87, 10,56, 17,22,  4,24, 15,67,  1,60,  0, 7,  9, 2,  8,71,  2,63, 12,63,  5,99, 18,79,  7,84, 14, 7, 11,97, 13,84, 
    12,19, 19,98,  5,65,  4,56,  8,21, 15,81,  9,91,  2,52, 16,86, 10,64,  1, 2, 13, 6,  7,81, 11,81, 17, 5, 14,84, 18,88,  3,70,  6,76,  0,72, 
     3,73,  4,73, 13,24,  8,26, 19,22, 18,53, 14,94,  5,81, 12,25, 15,57, 10,47,  1,86,  9,79,  6,92, 11,45,  0,17, 17, 3, 16,12,  7,60,  2,60, 
    13,66, 18,71, 19,69, 17,77, 16,95,  9,30,  6,45, 14,46,  8,35,  0,33, 12,95,  1,20, 11,30, 10,50, 15,92,  3,97,  4,99,  2, 2,  5, 2,  7,24, 
    19,91,  4,85, 17,16,  3,91, 15,40, 18,59, 11,64,  0,30, 10,62,  7,94,  6,67, 14,75, 12,22,  5,46,  8,97, 16,40,  2, 9, 13,38,  9, 7,  1,94, 
     2,99, 10,82, 13,94,  8,20, 11,41, 12,85,  9,21,  4,80, 16,53,  6,50, 15,73,  1,37, 14,89, 19,60,  0,30,  5, 5,  3,10,  7,12, 17,40, 18,29, 
     7,34, 17,27, 11,75,  4,74,  2,82, 12, 9, 15,75,  9,87, 18,81, 16,12,  8,12, 14,26, 10,18,  5,89, 19,84,  0,87,  6,14,  1,93, 13,20,  3,81, 
    11,93,  1,48,  6,61,  2,57,  7,89, 17,62, 10,51,  5,89, 16,43,  4, 9, 13,73, 19,24,  3,17,  8,73,  0,12, 15,76, 18,72, 12,11, 14,34,  9,75, 
    18, 7,  7,62, 11, 2,  3,28,  9,73,  1,57, 12,51,  4,75, 13,86,  5,25, 14,98, 17,63,  8,41,  2,64, 15,24,  6,62,  0,40, 16,21, 10,34, 19,44, 
     4,71,  1,44,  6,66, 18,93,  0,61, 13,66,  3,42,  7,51, 15,32,  5,52,  8,31, 11,30, 12,63, 10,29,  2,90, 19,75, 17,94,  9, 9, 16,40, 14,22, 
    11,14,  4,46, 10,10,  5,52, 14, 7,  8, 1,  6,13,  0,77,  9,37,  2,63, 15,37, 19,59, 13,33,  3,30,  1,78, 17,38,  7,77, 18,12, 16,46, 12,33, 
    17,72,  3,71,  1,84, 15,10, 13,95,  6,24, 12,18, 19,56,  7, 3, 16,49, 11,77, 10,19,  0,70,  4,24,  2,50, 18,19,  5,60, 14,45,  8,77,  9, 6, 
    14,11, 15,20, 12,12, 19,61, 17,99, 18,29, 10,38,  1,76,  6,48, 13,56,  3,98,  9,80,  4,52,  0,30, 11,23, 16,59,  7,66,  2, 4,  8,43,  5,26, 
     5,80,  2,71,  4,50, 10,69, 19,27,  0,54,  3, 4, 18,60,  1,93,  7, 1, 16,76, 12,11, 11,88, 14,87,  9,42, 15,73, 17,87, 13,40,  8,15,  6,72, 
    18,79,  1, 4, 14, 8, 11,65, 19,72, 16,70,  9,69, 17,99,  3, 1, 15,73,  0,57, 13,52, 10,80,  7,85, 12,58,  5, 8,  6,31,  4,91,  2,89,  8,60, 
    10,88,  2,39, 19,70,  6,65,  0,78, 15, 9, 18,65,  5,54, 13,96, 14,14, 12,46, 17,51,  7,64, 11,55,  1,56, 16, 6,  4, 2,  9,15,  8,53,  3,20, 
     4,57, 14,39, 17,78, 16,71, 15,91,  3,57,  5,73,  7,36, 11,81, 19,92, 18,59,  9, 8, 13,24, 12,48,  2,88,  0,69,  6,49,  8,55, 10, 7,  1,82, 
    12,31,  8,56,  3,97, 14,98, 18,18, 19,55, 15,69,  6,33, 11,73, 13, 4, 16,44,  0,97,  1,76, 10,21,  5,39,  2,73, 17,85,  7,30,  9,65,  4,15, 
     8, 2, 16,76, 17,94, 12,62,  1,28, 14,91,  2,37,  6,62,  7,49,  3,34, 10,71, 18,65, 13,56,  0,78,  5,10, 19,52, 15,43, 11,92,  9,34,  4,58, 
     8,28, 13,79,  9,40,  0,44, 10,33, 19, 2,  4,85, 18,53, 17,19,  7,71,  2,16, 15, 8, 12,38, 11,52,  3,98,  1,30,  5,15, 14, 5,  6,16, 16,88, 
     2,89,  1,36, 19, 2, 15,42,  8,74, 11,89,  5,69, 16,13,  7,14, 12,83,  0,46, 18,78, 17,69, 13,48, 10,64,  9,64, 14, 2,  3,34,  6,38,  4,40, 
    13,30,  4,92, 18,48, 11,93, 12,74, 15,71,  3,85, 14,25,  5,98,  8,71, 10, 1,  9,83,  0, 8, 16,73,  6,44,  1,26, 17,38, 19,68,  2, 9,  7,63, 
    19,45, 16,77, 17,11, 18,49, 12,54,  3,93,  6,42,  8,43, 15,57,  9,17,  7,61,  4, 9,  1,65, 13,32,  5,28,  2,53, 11,63, 14,17,  0,66, 10,66, 
     7,57,  5, 4, 13,30, 12,33,  9,99, 19, 4,  1,51,  0,42, 14,76, 10,44, 17,16,  8,90,  6,89, 15,91,  2,61,  3,81, 11,86, 16,92,  4, 7, 18, 7, 
    18,33,  1,58,  8,22,  5,22, 13,63, 12, 7,  7,22, 11,78, 19,84,  4,95, 10,83,  2, 7, 16,66,  3,35, 14, 9,  0,29,  6,43, 17,88,  9,95, 15, 7, 
     8,88, 12,52, 17,18,  7,46, 11, 8, 14, 5, 10,76,  1,20,  5,19, 18,71,  0,30,  9,22, 15,57, 13,53,  3,91,  6,68, 19, 8, 16,47,  2,81,  4,33, 
    15,48, 13,53, 14,34,  2,61, 10,23,  9,11, 12, 5, 19,81,  0,30,  5,25,  8,13, 17,68,  1, 9, 16,38, 18, 6,  3,99, 11,18,  4,19,  7,53,  6,81, 
    18,50,  7, 3,  5,61, 15,88, 12,98,  6,56,  4,92,  0,87, 13,34, 11,29, 17,89,  9,80,  8,96, 16,26,  3,41, 10,97, 19,66,  1,49,  2,19, 14,33, 
    19,59,  4,66,  7,26, 17, 8, 14,41,  3,63, 12,69,  8,64,  6,58, 18, 2,  9,92, 10,26,  2,13, 11,40,  1,88, 16,34,  0, 8, 13,45,  5,94, 15,72, 
    11,46,  4,95,  7,36,  2,26, 15,79,  3,76,  0,37,  8,73, 12,41, 10,94,  5,82, 16,36, 19,48,  6,96, 13,73, 18,79, 17,30,  1,15, 14,87,  9,34, 
    17,30,  5,17, 11,94, 10,82,  3,44, 18,78,  6,97,  0,99, 15,82, 13,87, 16,56, 14,87, 19,42,  9,52, 12,87,  7,15,  4, 1,  8, 2,  2,90,  1,71, 
     4,40,  8,13, 18, 6, 13,43,  3,46,  7,84, 15,66,  6,14,  0,66, 16,16, 19,94, 12,63,  9,16, 11,36, 17,40, 10,49,  5,53,  2,34,  1,60, 14, 8, 
     3, 1,  8,40,  6,59, 11,83, 12,69, 15,76,  4,38, 17,46, 13,65,  0,65, 16,61,  1,73, 10, 2, 14,63, 19,20,  7,74, 18,45,  9,40,  5,76,  2,70, 
    10,37,  8, 4, 12,33,  3,80,  1,41,  7,74, 15,67, 13,27, 16,83, 18, 2,  2,52, 11,53, 17,59,  0,27,  6,15,  4,44, 14,45, 19,17,  5,55,  9,59, 
     0,10, 17, 9, 18,19,  7,60, 10,88,  2,88,  1,29, 16,60, 13,48,  6,85,  5,89, 11,47, 14,98,  8,16,  4, 9, 19,98,  3,52,  9,95, 15,49, 12,96, 
     3,77,  2,96,  4,70, 11,19, 16,40,  8,97, 12,12,  9,63, 15,82, 13, 7, 14,76, 19,93,  5,86,  7,82,  6,46,  1,19, 17,47, 10,90,  0,45, 18,11, 
     6,67,  8,42,  7,17,  5,55, 10,59, 17,50, 12,58,  1,89,  9,90, 14,49, 13,86, 16,20, 15,41, 19,19,  4,53, 11,94,  3,87,  0,63, 18,53,  2,21, 
     2,20, 12,70, 16,20, 11,71,  0,39, 14,35,  4,19, 18,90,  8,60,  7,24,  5,77, 13,59, 10,55,  3,17,  9,35,  6,77, 19,93,  1,33, 17,60, 15,16, 
    17,53, 18,46,  6,12,  7,21,  0,20,  3,22, 15,27,  8,38,  1,43, 10,66,  9,38, 11,98, 14,55,  2,41,  5,99, 19,17, 16,91, 12,73,  4,24, 13,59, 
    12, 5,  9,82,  8,32, 17,28,  2,43,  5,50, 14,85, 13,58, 10,13, 19,50, 18,20,  1,51,  3,17,  4,54, 11,16,  0,20,  6,98, 15,45, 16,87,  7,56, 
     6,74,  2, 8, 13,20,  7,63, 16,93,  9,22,  4,87, 11,66, 12, 9, 14, 4, 15, 7,  8,30, 19,92, 10,36, 18,68, 17,78,  5, 6,  1,48,  0,85,  3,26, 
     0,44, 10, 4, 12,16, 13,72,  6,70,  3,52, 15,85,  4,79, 16,79, 14,76,  9,64, 11,71,  2,83, 18,37,  1,84,  5,11,  7,22,  8,28, 17, 2, 19,57, 
     3,86, 17,45, 15,13, 13, 3, 19,24,  7,47, 12,30, 11,19,  6,17, 18,25,  8,92,  4,75,  0,96,  5,24, 16,16,  9,74, 14,67,  1,70,  2,42, 10,90, 
    16,47, 18, 6, 10,19,  5,38,  4,12,  8,93, 15,55, 14,63,  3,41, 19,14,  0, 3,  6,11,  9,76, 17,32, 12,83,  7,33,  2, 2, 11,76,  1,73, 13,27, 
    10,58,  7, 2, 13,77,  8,72,  6,98,  5,60, 16,38,  2,44, 18,67,  9, 1,  3,57,  0,48,  1,37, 15,89, 12,81, 11,79, 19,81, 17,33,  4,22, 14,33, 
     1,26, 10,15,  7,17,  4, 9, 19,91,  3,40,  5,18,  9, 6, 13,49,  6,90, 15,11, 11,68, 17,26, 14,67,  2,14, 18,94, 16,85,  0,96,  8,11, 12,97, 
    14,15,  8, 3,  5,51, 16,15,  0,69, 19,12,  9,23, 15,33, 18,66, 10,24,  4,44,  1,78, 17,48,  3,50, 11,89,  7, 6, 12,58,  2,20,  6, 5, 13,69, 
    15,62,  2,52,  8,19, 18,92, 13,40,  0, 1,  6,97,  9,56, 17,74,  4,46,  3,33, 12,33, 10,28, 19,84, 14,51, 16,25, 11,59,  5,90,  1,38,  7,59, 
     0,64, 12,37, 15, 2,  1, 9, 10, 6,  3,34,  4, 1,  7,42,  9,92,  6,61, 19,48, 14,61, 17,62, 13,11,  2,78, 18,29,  5,40,  8,15, 16,61, 11,39, 
     9,81, 13, 9, 15, 4,  6,67, 17,35, 10,89,  8, 7, 16, 1,  1,55,  2, 3,  4,11,  0,35, 19,66,  3,19,  5,28,  7,69, 11,22, 14, 3, 12,42, 18,56, 
     4,91,  1,26,  2, 5,  5,98, 18,10,  0,72,  7,78,  8,81, 14,11, 16,51, 13,63,  6,80, 11,29, 12,63,  3,66,  9,80, 15, 3, 17,68, 10,50, 19,16, 
     4,95, 13,43, 17,70,  5,12, 10,73,  3,10,  2,88,  6,85, 14,43,  0,57, 16,70, 12,15,  9,13,  8,54, 15,96, 19,99,  7,18, 11,64,  1,85, 18,53, 
     4,48,  1,16,  3,56, 15, 9,  6,66, 18,83,  9,26,  7,40, 19,28,  5,73, 10,79, 14,47,  2,82, 16,59, 13,64,  0,37,  8,79, 11,27, 17, 6, 12,99, 
    13,99,  0,33, 19,85,  2,27,  5,93,  3, 3,  6,56, 16,11, 12,81, 11,42, 17,10, 10,73,  9,27, 15,59,  8,18,  7, 1, 14,62, 18,55,  1,59,  4,60, 
     0, 6,  5,67, 14,42, 11,39,  1,14,  4,39, 12,49, 10,12, 15,91, 19,67,  8,91, 16,69,  2,38,  3,13, 18,40, 17,39, 13, 4,  9,55,  6, 2,  7,85, 
     8,83, 11,15,  6,18, 12, 9, 18,87,  4,66, 16,24, 13, 2,  0,90,  1,54, 19,28, 17,87,  3,50,  5,76,  2,59,  7,28,  9,53, 10,79, 15, 9, 14,31, 
     6,71,  2,92,  0,38,  4, 6, 14,86,  1,17, 18,22,  3,88,  9,80,  5, 4, 11,13, 13,82, 12,44, 19,71,  7, 7, 16,36, 15,43,  8,10, 10,65, 17,21, 
     1,49, 13,50, 19,60,  2,63, 18,51,  5,62,  7, 4,  3,86,  8,27, 12,32, 15,32,  6,66,  4, 4, 14, 4, 16,23,  9,99, 11,99,  0,78, 10, 2, 17,38, 
     2,47,  0,76,  1, 7, 12,62,  7,32,  3,64,  8,28, 16,45, 14,65, 11,91, 19,36,  9,44,  4,80,  5, 5,  6, 7, 10,20, 15,54, 17,87, 18,85, 13, 5, 
     0, 2, 15,78,  3,78, 11,39, 17,94,  9,94, 16,83, 10,46, 12,97,  4,19,  7,64,  8,96, 18,22,  1,47, 14,32,  2,71, 13,93,  5,92, 19,13,  6, 9, 
    10,51, 13,48,  6,60, 14,99,  2,53, 17,35, 15,19,  5,78, 18,65,  8,72,  9,72,  3,60,  4,92, 16,15, 11,61,  7,29,  1,99, 19,74, 12,53,  0,11, 
    14,41, 10,54, 17,17, 12,91,  5,48,  4,80,  7,95, 11,84, 13,13,  8,91,  0,44,  2,81, 16,74, 15,44, 18,32,  9, 9, 19, 4,  3,13,  6,60,  1,83, 
    17,13,  6, 4, 12,23,  0,84, 16,41,  5,73,  9,77,  4,23,  7,16, 13,56, 11,19, 15,63, 18,72,  8,35,  1,62,  2,45,  3,50, 10,89, 19,38, 14,76, 
     9,17,  0,98,  6,22, 14,86,  2,32,  3,92, 12, 2,  7,27,  5,52, 16,41, 19,11,  1,46, 10,97,  8,45, 18,92, 11,55, 13,58, 17,95,  4,58, 15,20, 
    19,52, 15,90, 12,37, 17,63, 18,25, 14, 2, 13,50,  4,53,  1,67,  0,21,  6,86,  2,90,  9,31,  8,59, 11,45,  7, 5, 16,75,  3,93,  5,28, 10, 8, 
    11,17, 13,90, 10,80,  9,90,  2,72,  8,31,  5, 6,  4,85,  6,30, 18,60, 17,69, 14,91,  7,27, 12,29, 16,21, 19,36,  3,13, 15,74,  0,60,  1,11, 
     8,74,  0,80, 12,93, 10,91,  9,60, 13, 8,  1,96,  3,61,  4,73, 17,27, 19,62, 18,78, 15,45, 11,77,  6,62,  5,83, 14,25,  7,71,  2,11, 16,58, 
    19,98,  8,32,  5,97, 18,26,  2,16, 14,65,  3,16, 15,97, 12, 5,  4, 4, 16,95,  0,10, 11,88, 13,35,  7,77,  1,36,  9,44,  6,76, 10,75, 17,14, 
    11, 4, 16,47,  7,71, 14,32, 15,20, 19,54,  1,87, 13,59,  2, 5, 17,10,  9,43,  3,13,  6,24,  4, 8, 10,27,  0,33,  5,56,  8,13, 18,33, 12,35, 
    11,17,  3,19, 14,71,  0,91, 17,23,  4,25, 15,45, 13,69,  2,86,  8,91, 18,95,  1,36, 12, 9, 19,25,  5,48, 10,99, 16,87,  9,52,  6,19,  7,67, 
    13,72,  4,18, 18,91, 11,69, 15,13,  0,43, 16,80,  3,63, 19,41, 17,82,  1,20,  8,31, 14,83,  5,55, 12,89,  6, 5,  9,68,  2,38, 10,81,  7,41, 
     7,53, 13,63,  4,35, 19,69,  0,31,  8,39, 15,21,  5,97,  1,31, 18,11, 16,76,  3,29, 11,88,  9,68, 12,96, 10,79, 17,37, 14, 9,  2,80,  6,88, 
     8,99, 11, 8,  7,84, 18,70,  4,45,  0,91,  6,10,  9,97,  5,94, 13,73, 17,33, 10,20, 15,78, 12,43, 16,34,  2,92, 14,39, 19,36,  3,49,  1,73, 
    15,19,  3,16,  8,69, 11,96, 17,43, 16, 8,  9,70, 14, 9,  6,18, 10,41,  1,89,  4,40, 18,88, 12,89, 13,67,  0,58,  7,39, 19,44,  2,33,  5,17, 
     5,35,  7,76, 11,19, 18,57, 10,82,  1,99, 16,11, 13,39,  4,31,  9,25,  6,65, 14, 2,  8,77, 19,70,  0,68, 12,25,  2,74,  3,46, 15,31, 17,31, 
    11,45,  9,61,  6,54,  8,72, 18,35,  7,68,  1,89,  4,19, 17,69, 10,10,  3,74, 15,56, 12,64, 19,11, 16,12,  2,42, 13,92,  5,82, 14,60,  0,19, 
    10,23,  3,73,  4,26, 15, 4, 16,25,  1, 6,  2,33, 14,29,  7,11, 11,82, 19,29, 13,53,  0,83, 12,61,  6,26, 17,79, 18,97,  8,93,  5,32,  9,61, 
    17,69,  5,86, 11,43,  2,11, 13,25,  0,76,  7,57, 12,34,  6,64,  9,33,  4,68,  8,90, 14,63, 16,38,  1,44, 18,52, 19, 9, 15,53, 10, 7,  3,77, 
     4,54,  5,62,  0,42, 18,63,  9,87, 16,98, 19,94,  3,70,  1,26,  2,14, 17, 4,  6,85,  8,44, 13,71, 10,21, 11,37, 12,30, 14,46,  7, 2, 15,76, 
    10,13, 13, 7,  5,19,  0, 4,  7,39, 16,57,  4,52, 19,98,  9,27,  2,54, 11,77, 17,82, 14,66, 15,96, 18,43,  3,78,  8, 3,  1,12,  6,61, 12,67, 
     0,51,  4,49,  6,76,  3,30, 15,21,  5,98,  2,22, 10,64, 11,45, 14,14, 13, 1, 17,79, 16,69,  8,14,  9,71, 18,77, 19,74,  1,65,  7,57, 12,63, 
    15,16, 12, 9, 19,30, 13,62, 11,23, 18,47,  4,31, 16,22,  5,55, 17,99,  2,50,  8,42,  7,18, 14,49,  0,72,  9,56, 10,54,  6,95,  1,69,  3,93, 
     6,34,  5,90, 15,86,  2,45,  9, 5,  4,99,  1,79, 19,95, 17,95, 10,82,  0,95, 11,43, 12,34, 14,95,  7,52,  8,46, 16,66,  3,13, 18,43, 13,47, 
    18,72,  7, 3,  4,75, 17,32, 16,51,  9,57,  8,62,  3,51, 12, 2, 14,98,  6,63, 19,60,  2,93, 15,56, 10,53,  0,90,  1,93,  5,47, 13,78, 11,31, 
    10,13,  9,87,  5,26,  1,73, 14,37, 19,65,  0,34, 18,77,  3,46,  7,33, 16,78,  6,51,  4, 5,  2,73, 12,41, 13,71, 15,74,  8,44, 11,48, 17,52, 
    17,20, 15,40, 16,28,  4,58, 13,32,  1,73,  9,76, 10,84, 11,31, 19,38,  0,92,  5,13, 14,42,  8,76, 12,54,  7, 4,  2,76,  3, 7, 18,40,  6,28, 
    19,37, 11,79, 13,70,  6,49,  4,40, 18,25,  1,51, 12,66, 17,27,  9,94,  3,37, 14,35,  0,28,  8,85, 10,84,  7,37,  5,47, 16,53, 15,82,  2,86, 
     7,88,  8,97,  1,99, 15,81, 14,48,  5, 3,  3,67, 11,62,  2,41, 19,53,  4,68,  6,70, 18,48, 12,98, 16,36, 17,29,  9,38, 10, 1, 13,13,  0,83, 
     5,26,  7,82,  4,58, 16,13,  2,92, 19,35, 18,55, 10,88,  0,32,  1,75, 11,85,  8,98, 17,11, 13,39, 14,74, 12,61,  9,57,  3,59,  6,63, 15,53, 
     9,70,  5,28, 18,97, 16,81,  7,54, 15,60,  8,71,  0,15,  3,87,  6,25, 12,94,  1,39, 10,20, 14,74, 13,49, 19,82, 17,47,  4,62, 11, 5,  2,24, 
    15, 6,  8, 8,  9,52, 17,41,  1,33, 14,52,  3,28, 12,61,  5,86, 16,88,  6,66, 11,13, 10,64,  4,94, 13,10, 18, 9,  7,79,  2,56,  0,12, 19,74, 
    12,70,  0,99, 10,99, 11,25,  2, 7,  9,43,  7,88,  5,75, 16,74, 13,44,  6,44, 15,80,  3,41,  8,74, 14,57, 19,84, 18,19,  1,27, 17,25,  4,78, 
    14,64, 15,88,  8,45, 16,54,  5,46, 11,53, 12, 7,  0,96,  9,37,  3,92, 18,11, 17,38, 13,69,  6,93,  4,90, 19,28,  1,49, 10,78,  7,96,  2,45, 
     7,43, 18,86,  0,15, 14, 1, 12,51, 17, 7,  8,95, 10,80, 16,97,  4, 3, 19,74, 13,97,  2,70, 15,89,  6,64,  3,97,  1,40,  9,32, 11,52,  5,64
  };
  // Taillard 100x20 instance 7
  const int tai_100_20_7[] = {
    100, 20, // Number of jobs and machines
    10,63, 17,84, 18, 3,  2,53,  0, 1, 19,35, 15,20,  5,43,  4,43,  9,85, 14,23,  1,21, 13,10,  3,77,  7,83, 16,27,  8,35, 11,18, 12,44,  6,39, 
     6,97, 10,48,  0,63,  3,76, 17,18,  9, 9, 19,75,  5,36,  4,72, 12,79, 14,99, 18,94,  7,46,  8,16, 15,33, 11,42, 13,65,  2,98, 16,30,  1,50, 
     5,64,  1,48, 15,93,  4,47, 13,81, 11,31,  9,54,  6,89, 19,22,  8,63,  0,58, 12,25, 10,64, 17,82, 16,61,  2,64, 18,38,  3,17, 14,27,  7,70, 
    10,32, 19,72, 14,42,  0,83,  2, 2, 12,33, 13,29,  4,90, 11,35,  5,91,  8,26, 15,85,  7,72, 16,19,  1,95,  6, 5,  9,39,  3,75, 18,83, 17,70, 
    12,90, 14,76, 16,27,  1,88, 11,59,  4,95,  7,51, 13,22, 15,79,  8,30, 18,10, 17,91,  9,74, 10, 4,  2,36,  6,98, 19,54,  5,33,  0, 2,  3,22, 
    13,71, 14,99, 10,22, 11,64,  1,30,  2,11, 15,94,  4,14,  3,83,  6,47, 17,48, 19,45, 18,39, 12, 7, 16,32,  0, 2,  8,93,  5,40,  9,14,  7,48, 
     1,80, 10,46, 16, 4,  7,45,  3,64,  2,19, 12,85, 17,39,  5,78,  6,62, 18,69,  9,65, 15,82,  0,75, 11,40, 13,15,  8,68,  4,11, 14,44, 19, 3, 
     9,95, 13,89, 10,98, 14,75,  4,27, 12,74,  2,20, 18,57,  0,53, 11,94,  7,17,  3,78, 19,21,  1,99, 17,31,  8,99, 16,86,  5,20,  6,80, 15,86, 
     2,21,  3,52,  6,56, 15,22, 13,19,  5,84, 11,96, 18,57,  4,31,  7,39,  0,13, 10, 6,  9,58, 17,32, 12,41,  1,47, 14,13, 19,30,  8,44, 16,80, 
     0,71,  5,31, 15,17,  3,53, 18,85,  7,48, 14,52,  9,34, 16,39, 19,84,  2, 2,  6,31, 11,57, 12,63, 13, 1,  4,57,  1,30,  8,61, 10,72, 17,17, 
    14,14, 18,98, 13,44,  2,10,  8,53, 10,72, 19,65, 15,49,  9,21,  3,84,  1,94, 17,99,  5,61,  4,64, 16,29, 12,98,  6,55, 11,71,  7,43,  0,64, 
    11,14, 16,74, 15,57,  4,81,  9,40, 10, 7,  7,85,  5,32,  2,35,  8,34, 13,41, 14,79, 12,80,  6, 1, 19,95,  1,25,  0,43, 18,19,  3,98, 17,48, 
     8,42, 19,71,  5, 9, 12,26, 10, 3, 15, 3,  3,31,  4, 3, 13,40,  9,21, 17,69,  1,89, 16,94,  2,94, 14,59,  0, 9,  6,71, 11,53, 18,45,  7,75, 
     5,72, 10,86, 14,22,  7,71,  3,22, 13, 8, 16,73, 11,15, 15,90,  6,40, 17, 1, 12, 4,  4,81,  8,10,  1,83,  0,32,  9,89, 18, 4, 19,53,  2,38, 
     2,52,  3,45, 15,43, 14, 9, 19,90,  7,40, 11,26,  4,52, 16,22, 10, 9, 17,34,  9,94, 12,69,  1,28,  5,29,  8,62,  0,63,  6,99, 18,97, 13,14, 
    18,81, 12,50,  9, 9, 17,27, 14,91,  2,53,  3,93, 16,64, 15,23,  7,42, 13,77,  4,50,  0,84,  8,13, 11,90, 19,84,  1,81,  6,60,  5,92, 10,72, 
    16,87, 12, 4,  4,56, 18,40,  3,55, 17,29,  7,31,  2,29, 13,39,  5,38,  9,94, 19,40, 14,88, 10, 1,  0, 3,  8,46, 11,36,  1,44,  6,92, 15,34, 
     6,62,  7,95,  8,69, 15,29,  2,45,  3,77, 11,34, 18,59,  0,90,  4,83,  9,83, 17,72, 19,98, 13,53, 12,84, 10,94, 16,71,  1,49, 14,17,  5,28, 
     8,66,  6,24, 17,74, 12,73, 15,77,  5,75,  0,66,  3,14, 16,21,  7, 8,  9, 1, 13,39, 10,76,  4,14, 14,70, 11,20,  1,20, 18,66, 19,23,  2,74, 
     9,54,  5,35,  0,58,  6,72,  3,98, 11,62, 13,68, 15,87,  1,21,  7,67, 14, 9,  2,12, 17,45,  8,36, 16,43, 18, 3, 12,28, 19,24,  4,70, 10,45, 
    15,78,  7,28,  6,60,  9,69, 18,87,  3,34,  0,70, 19, 2, 16,83, 10,94, 12,22, 11,62,  4,37,  2,48,  8,20, 14, 5, 17,13,  1,75, 13,40,  5,27, 
     9,92, 11,78, 10,84,  3,97, 13,77, 17,84,  2,58,  0,85, 12,40,  8,56,  1,55,  6,63,  4,43,  7,36, 19,52, 15,43, 16, 2, 14, 9, 18,74,  5,80, 
     9,53, 18,85, 14,79, 19,79,  0,26,  8, 4, 11,19, 10,11, 12,11,  3,71,  7, 6, 16,67, 17,26,  5, 3, 15,10,  1,74,  6,60,  4,77,  2,34, 13,40, 
     0,23, 11,34,  2, 1,  1,83, 12,95,  5,19, 14,44,  4,40, 16,22, 10,58, 17,72,  9,18,  3,23,  8,16, 13,91,  7,37, 15,25,  6, 4, 19,92, 18,90, 
     9,93,  7,28,  0,48,  2,30,  1,90, 11,23,  3,73, 14,21, 19,29, 12,23, 13,44,  4,94, 17,28, 18,81, 10,53,  6,87,  5,68, 15, 1,  8,39, 16,46, 
    16,90, 15,58, 18, 7,  9,86,  8,88,  4,64, 13, 9,  6,78, 12,74, 10,42, 11,93,  3,15,  5,32,  0,77,  1,42, 17,27, 19, 9,  2, 3, 14,69,  7,16, 
    19,44, 10,60,  5,21,  3,87,  2,65,  0, 8, 13,65, 18,42,  8,31,  4,43,  6,50,  7,54, 15,41,  9,79, 17,80, 14,89, 16,18,  1,45, 11,57, 12,17, 
     3,53, 19,89, 16,59,  8,68,  4,58, 17,58, 11,74,  2,46, 10,54, 13,76,  7,66, 15, 3,  1,79,  6,75,  0,13, 18,85,  5,52, 14,27,  9,61, 12, 2, 
     4,74,  7,28, 16,29, 11,82,  9, 3,  6,31, 14,73,  3,47, 18,16, 10,54, 19,63,  0,27, 13,11,  1,79,  2,41,  8,65, 15,76, 12,61,  5,43, 17,28, 
    10, 3, 15,18, 14,48,  4,20,  8,65, 18,42,  3,55, 13,40, 17,17,  5,12, 12, 9,  7,32,  6,70,  2,26,  9,44, 11,33, 19,82, 16, 6,  1,37,  0, 3, 
    15,78,  7,33,  4,61, 14,81,  1,18, 17,75,  0,16,  9,31, 18,24,  2, 1, 10,36, 16,88,  8,57, 19,37, 11, 7,  5,76, 12,40,  6,79, 13,67,  3,19, 
    17,20, 13,36,  8,67,  9,64, 10,51,  5,56, 19,29,  0,15, 14,49,  6,72,  2,87, 12,77, 11, 5, 18,78, 16,32,  3,18, 15,83,  7,53,  1,84,  4,15, 
     9,55,  8,94,  7,78, 15,85,  1,21,  2, 5, 18,17,  5,96,  4,19,  3,72, 19, 2,  0,62,  6,18, 17,23, 14,64, 13,63, 11,90, 10,52, 12, 9, 16,16, 
    10,82, 19,20,  1,50,  6,21,  4,57, 13, 9, 12,49,  0, 5, 11,14, 14,71,  5,47,  9,35,  2, 8,  7,92,  3,73,  8,67, 18,66, 15,93, 17,48, 16,65, 
     8,82, 19,40,  4,14, 18,51, 12,86, 11, 8, 14,25,  2,76, 10,41,  0,10, 16,60,  9,96,  1,68,  6, 8,  5,79, 17,18,  7,75, 13,35, 15,31,  3,14, 
    19,45,  4, 2, 11,94,  0, 4, 16,27, 12,72, 18,76,  3,62, 15, 9,  1,14, 14, 7,  2,69,  6,48,  9,79, 10,52,  5, 4,  7,55, 17,75, 13,62,  8,93, 
    19,13, 13,63,  4,75,  0,67,  9,26, 10,59,  3,81,  8,55, 14,66, 18,28,  1,49,  2,16, 15,69, 17, 3, 12,13,  6,72,  5,76, 16,96,  7,63, 11,38, 
     5,36,  1,83, 12, 3,  3,10,  0,47, 14,65,  7,26,  9,62, 18,45,  2,55, 19,73,  4,30,  6,86, 13,53, 17,60, 15,41,  8,54, 16,33, 11,40, 10,20, 
    10,61, 12,28,  0,53, 17,79, 16,62,  5,87,  9,44, 14,21, 19,22,  8,67,  6, 8, 13,52,  4,43,  1, 8, 15,84,  2,26,  7,86,  3,62, 11,11, 18,74, 
     8,14,  2, 1, 18,30,  9,74,  6, 2,  4,87, 11,31,  0,84,  5,42, 10,11, 17,90, 16,42,  3,34, 19,14,  7,13, 14,75, 13,50, 12,61,  1,10, 15,76, 
     5,22, 12,58, 19,38,  7,14,  9,96, 14,76, 15,27, 10,16,  0,69, 16,49, 13,35, 18,93,  3,48,  8,43, 17,62,  4,34,  6,44,  1,51, 11,86,  2,50, 
     4,37, 18,48,  7,87, 12,78, 17, 6, 13,77, 19,98,  0,66,  9,33,  8,73,  3,67,  5,29,  2,77, 10,95,  6,40, 14,23, 15,69, 11, 1,  1,27, 16, 8, 
    19,64, 13,87,  6,50,  7,60,  9,50, 18,12, 15,33,  5,45, 16,16, 11,55,  0,88, 17,88,  1,25, 12,33,  3,15,  8, 2,  2,44,  4,75, 10,61, 14,67, 
     0,20, 19,95, 14,27,  2,69, 13,11, 12,23,  3,34, 15, 8, 17,64,  7,63,  4,73, 11,58,  9,33, 10,26,  1,84,  8, 4,  6,82, 18,35, 16,30,  5,51, 
     2,42,  7,63, 10,68,  0,93, 12,46, 16,60,  1,40, 13, 3,  9,73,  8,87, 19,52, 15,70, 14,35, 17,92,  4,19,  3,40,  6,48, 18,41, 11,70,  5,13, 
     7,53, 15,55, 11,32,  6,60, 18,40, 13,16,  2, 5, 12,53, 10,68, 19,43, 17,67,  5,60, 16,38, 14,23,  4, 2,  3,67,  9,42,  1,25,  8,80,  0, 1, 
    11,84,  7,99, 19,77,  5,51,  1,23, 15,77, 13,73, 17, 1,  2,65,  0,36,  9,88,  8,11,  6,80, 12,85,  3,51, 10,26, 16, 5,  4,83, 18,20, 14,60, 
    12,21, 13,12,  2,94,  3, 8, 15,14,  7,34, 11, 4,  8,58,  0,60, 19,51,  5,45, 18,95,  1,21,  6,92, 16,90, 17,77,  4,13, 14,51,  9,94, 10,33, 
    15,75,  0,85,  2,81, 19,81, 13,92,  9,96, 14,86, 11,91,  4,97, 10,12,  7,15, 16,11,  8,84, 17,27,  5,33,  3, 1, 18,87, 12,55,  1,32,  6, 8, 
     4,20, 10,71,  9,64,  0,85,  5,49, 11, 6, 14,52,  3,85, 13,15,  8,47, 17, 5,  1,23, 18,40, 15,46, 12,48,  7,87,  6,92,  2,54, 19,83, 16, 2, 
     3,74,  6,32,  5,15,  0,93, 15,14,  2,41, 13,97,  1,83, 11,29,  4,93, 19,30,  9,98, 18,15, 17,48, 16,31,  8,40, 10,36,  7,53, 12,26, 14,99, 
     7,59, 14,66, 16,28, 19,26, 11,65, 12,95, 15,54,  6,23,  8,25, 18, 3,  5,84,  0,72,  4,56, 13,94, 17,92,  9,15, 10,41,  2,27,  1,46,  3,52, 
     7,99, 15, 7,  8,33, 18,90, 13,82, 17,16, 14,81, 10,80, 19,75,  9,92,  1,93,  5,49,  2,14,  0,16,  4,49, 12,88,  6,51, 11,50, 16,11,  3,34, 
     7,43, 13,35,  4,91,  3,90,  0,34, 10,70, 12,22,  1,26, 16,55, 19,34,  6,55, 14,34,  2,68, 15,99,  8,24, 17,40,  5,25, 18,60, 11,13,  9,82, 
    18,68,  2,48, 14,64, 11,76,  6,15,  9,23,  5,62, 12,26,  1,39, 10,52, 13, 6,  0,47,  4,39, 17,81,  3,47,  8,98,  7,25, 16,58, 19,67, 15,46, 
    10,17,  4,35,  9,85,  1,15, 19,85, 17,46, 18, 5,  7,73, 12,63,  6,97, 15,21,  3,34,  2,59, 13, 2, 16,64,  5,88,  0,22, 11,17, 14,47,  8,77, 
     9,81,  2,60,  5,17, 15,25, 12,46, 17,80, 18,15,  1,31,  8,60, 10,28, 11,41,  0,35,  4,20, 14,81, 19,40,  3,51,  6,14, 13,36, 16,30,  7,75, 
     3,21, 15,93,  9,89, 19,15, 11, 8, 17,92, 10,19,  6,19,  8,45, 16,46,  4,97, 13, 6,  2,42, 12,60, 14,10,  1,37,  5,97,  0, 2,  7,51, 18,56, 
    19,60, 15,82, 18,95,  5,92, 17, 2, 14,78, 16,86,  1,66,  9,92,  0,93,  6,39, 12,58,  3,57, 10, 6, 13,18, 11,98,  7,52,  4,83,  8,36,  2,60, 
    11,55,  5,94, 12,93,  7,65,  4,33,  1,22, 17,38, 18,45,  8,18, 15, 7, 10,49,  2,81,  9,16,  3, 2, 14,67,  6, 8,  0,34, 19,49, 16,69, 13,35, 
    15,28, 17, 2, 18,78,  1,54,  3,14,  6,47, 19,71, 14,72, 16,51, 13,83,  9,24, 11,13,  0,11,  5,21,  4,71,  2,78, 12,68, 10,84,  7,47,  8, 7, 
     5,17, 17,85,  2,94, 13,95,  4,11, 19,58, 18,75,  1,70, 12,64, 16,16, 10,73, 11,22,  8,17,  6,45,  7,74,  3,83,  0,72, 14,13,  9,47, 15,79, 
    12,42,  7,37, 16,72,  9,16,  6,53,  8, 5, 18,62, 14,88,  5,35, 17,85,  4,58, 13,58, 19, 7, 11,81, 10,88,  0,74,  2,75,  1,67, 15,71,  3,58, 
    19,39,  2, 5,  7,92,  3,68,  8,34, 16,92,  9,17, 18,33,  1, 9, 10,55, 11,49,  5,49, 12,12,  0,24,  6,92, 14,70, 13,41, 17,37,  4,15, 15,22, 
     5,75,  7,34,  3,69, 12,48, 14, 7,  8,16,  1,27, 19,71, 17,11, 11,48,  9, 3, 18,39,  2,98,  0, 2, 10,39, 16, 2,  4,11,  6,68, 13,75, 15,49, 
    12,88,  3,16,  0,66,  9,34, 15, 2,  6, 5,  8,73, 16,25, 17,98, 19,57,  7,46, 14,91, 10,82, 18,51, 11, 1, 13,41,  5,57,  2,67,  1,28,  4,67, 
    16,37,  8,46, 18, 1,  3,87, 13,55,  9,25, 10,43,  6,10,  4,27, 11, 6,  5,91, 12, 2, 17,50, 14,94,  7, 1, 19,47,  0,79,  2,24, 15,64,  1,11, 
    13,32,  0,24,  3,84, 14,13,  6,11,  5,98,  8,74, 11,84,  4,82,  9,99, 12, 4,  7,98, 18,65, 16, 7, 17,88,  1,25, 10,68, 19,93,  2,73, 15,32, 
    14,42, 13,76, 18,43,  1,24, 11,92,  8,40,  2,36,  6,52, 15,59, 12,63,  5,15, 19,54, 17,70,  3,86, 16,48,  4,62,  9,20, 10,49,  7,15,  0,15, 
    19,42, 15,31,  5,80, 10,43, 12,80, 13,23,  0,89,  3,59,  6,19, 11,54, 16,15,  9,18, 17,89,  8,47,  1,38, 14,21,  2,87,  7,66,  4,76, 18,62, 
    15,54, 11,53, 12,19, 19,55,  1,83,  7,72, 13,13,  6,83, 17,54, 18,28, 10,22, 14,89,  0,13,  9,73, 16,24,  5,12,  8,82,  2,15,  3,78,  4,94, 
     4,18, 16, 3,  9,84,  6,89, 13,89,  0,73,  7, 5, 18,57, 10,38, 17, 9,  8,77, 15,17, 19,14,  3,32,  2,28, 14,15,  5,33, 12,48, 11,49,  1,40, 
     3,48, 12,33,  4,79,  0,42, 10,25,  2, 9,  7,25,  1, 1, 17,50, 15,55,  6,97, 11,96, 19,18,  8,25,  5,24,  9,77, 13,24, 16,52, 14,37, 18,88, 
     8,57,  0,50, 16,64,  2,89, 19,10,  4,80,  5,63,  6, 6, 15,32,  9,34,  7,34,  1,68, 12, 5, 13,76, 17,77, 11,86,  3,67, 14,76, 10,81, 18,51, 
    10,75, 17,91, 18,97,  7,14,  3,62, 14,40,  5,43,  2,16,  0,93,  1,55,  6, 4,  4,25, 19,29,  9,45,  8,83, 16,46, 15,31, 12,13, 11,57, 13,68, 
     5,64,  3,75,  2,35,  9,71,  1,38,  8,54, 11,36, 13,71,  0,66, 14,72, 10,58, 12,20, 16,67,  7,69,  4,46,  6,68, 18,93, 17,39, 15,48, 19,77, 
    13,59,  0,34, 14, 6,  6,76, 11,13, 12,59,  5,95,  7,78, 18,66, 10,92,  9,89, 19,77, 17,66,  4,16, 16, 7,  3,75,  1,97,  2, 7, 15,81,  8,13, 
     8,10,  2,38,  6,29,  7,38, 19,27, 11, 8,  9,59, 13,60, 10,36,  4,86, 14,39, 18,19,  3,84,  5,50, 15,65, 12,58,  1,62,  0,71, 17,22, 16, 6, 
     5, 6, 18,32, 14,38,  3,53,  0,59, 15,33, 11, 5,  9,37,  6,85,  2,33,  1,47,  4,70,  8, 6, 12,32, 17,86, 13,26, 10,36, 19,64,  7,78, 16,16, 
     6,60, 18, 4, 15,44, 16,14,  9,45, 13,83,  1, 7, 14,75,  3,86, 17,88, 19,25,  0,85, 12,99,  5,80,  8,50,  2,91,  4,14, 11,87, 10,73,  7,72, 
     0,92, 17,25,  6,93, 12,30, 11, 9, 16,86, 18,79,  5,71,  9, 5,  8,47, 13,61,  2,78,  3,76,  7,71, 15,98,  4,34, 14,23, 10,82,  1,53, 19,21, 
    16,91,  9,81,  2,70, 14,36,  1,96, 15,23, 17,55,  7,93,  3,86,  5,15,  4,19, 11,18,  6,66, 18,44,  8,58, 19,62, 10,54, 13,79, 12,11,  0,49, 
     4,62, 10,77,  0, 9, 14, 4,  2,80, 11,25,  1,16,  3,95, 12,30,  9,57, 18,40,  6,81, 15, 3, 13,28,  7,45,  8,59, 19,94,  5,31, 17,91, 16,55, 
    17,85, 14,23, 10,78,  4,68,  9,59,  2, 9,  3,71, 15,92, 11,50,  0,38, 16,35,  1,31,  5,91,  7,57, 13,70,  8,60,  6,73, 18,39, 19,76, 12,60, 
    17,61, 11,77,  0,22,  7,43, 13,66, 10,47,  4,53, 16,97, 15,91,  9,64,  1,93,  3,46,  5,20,  6,90, 12,58, 19,36, 18,98,  2,70,  8,49, 14,92, 
     3,35, 11,56, 10,45, 19,15,  7,79, 15,66,  6,55,  9, 1, 17,89, 13,29, 16,98,  8,31,  2,45, 14,51,  4,83, 18, 7,  1,76, 12, 5,  5,78,  0,77, 
    15, 8, 12,58, 18,24,  7,62, 13,77, 11,38,  2, 9, 14, 6,  5,88, 16,23,  3, 9, 17,12,  9, 9,  1,65,  6,68,  4,57,  0,38, 19,15, 10,98,  8,55, 
    13,96, 17,28, 14,52, 18,91,  5, 9,  8,15, 16,44, 11,69,  3,99,  7,15, 15,65,  2, 9,  9,27,  1,56, 12,70,  4,39,  0,21, 10,59, 19, 2,  6,15, 
     7,98,  4,84,  3,44,  1,10, 18,84, 12,78,  2,19, 13,80,  9,28, 14,19,  8,58,  5,76,  6,49, 16,88, 11,42, 19,78,  0,78, 15,91, 17,83, 10,29, 
     2,18, 19,40, 11,21,  6, 8, 17,31, 15,56,  0,56, 13,78,  7,45,  8,31, 10,79,  3,47, 12,73, 16,66,  4,27,  9,98,  5,73, 14,79,  1,74, 18, 8, 
     8,95,  3, 2, 17,41, 16,46,  1,19, 14,35,  2,41,  0,82,  4,77,  9, 3,  7,52, 18,15, 13,12, 12, 3, 10,30,  5,93,  6,96, 15,68, 19,73, 11,67, 
    17,92, 13,60,  7,45, 12,65, 18, 5,  8,81, 14,21,  6, 2,  3,71,  2,57, 16,18,  9, 2, 11,60,  5,41, 10,43,  0,36, 19,75, 15,55,  1,79,  4,38, 
    15,35, 12, 6,  5, 5, 19, 4, 11,39,  8,43,  6,76,  0,83, 16,38,  9,47, 14, 3,  2,68, 10,58,  1,49,  3,99,  4,20, 17, 9, 18,39,  7,11, 13, 9, 
    11,38,  2,56,  6,65,  5,54, 19,95,  3, 6, 18, 8,  8,53,  0, 9,  7,44, 10,39, 12,18, 15,77, 14, 6,  1,43, 16,26,  4,54, 17,64, 13,43,  9,71, 
     1,75,  4,31, 16,51, 14,95, 13,50,  7,98,  9,80,  2,38,  8,99,  5,78,  3,83,  6,38, 15, 4, 10,68, 11,44,  0,95, 12,77, 17,16, 18,89, 19,52, 
     5,10,  0,41,  3,52,  9,97,  2,55, 17,50, 19,48,  6,87, 14,97, 18, 1, 10,27, 15,69,  4, 4, 13,75,  8,59,  1,50,  7,22, 11,73, 12,73, 16,57, 
    12, 8,  6,62,  3,24,  8,29, 16,87, 13,72,  0,34,  1,23, 18,35,  9, 1, 11, 4, 15,42,  5,90,  2,23,  4,25, 17,62, 10,46, 19,14, 14,79,  7, 1, 
     9,13,  4,96, 10,43,  8,92, 14,11,  0,94, 12, 5, 17,34, 18,38, 19,68, 11,70,  7,65, 13,62,  1, 9, 16,35,  2,94,  5,12,  6,30, 15,91,  3,63, 
     2,33,  7,14, 15,79,  5,24,  9,11,  6, 5,  8,22,  4,61, 12,27,  0,88,  3, 4, 16,78, 11,62, 14,72, 19,56, 13,18, 18,62,  1,47, 10,27, 17,94, 
     1,34,  3, 6, 12,61, 15,60,  8,48,  6, 2, 13,64,  7,27,  5,16, 14,71, 18,71,  4,47,  9,52, 19,73, 11,42, 10,99,  2, 5, 17,72, 16,43,  0,27
  };
  // Taillard 100x20 instance 8
  const int tai_100_20_8[] = {
    100, 20, // Number of jobs and machines
    16,83, 18,46,  5,63, 13,19, 12,28,  9,87, 10,91,  0,61, 17,14,  4,84,  2, 9, 11,91,  8,24,  6,28,  7,60, 15,26,  3,11, 14, 6, 19, 4,  1,96, 
     6,70,  9,72, 17,45, 10,50,  1, 1,  7,46, 11,67, 13,43,  0,75,  5,44, 14,62,  4,91,  8,99,  3,96, 12,34, 18,60, 19,96,  2,19, 15,10, 16,52, 
     3,94, 13,42,  8,82, 18,39,  9,33,  2,48, 15,42, 16,32,  4,69,  1,93, 14,65, 10,74, 12,47,  6, 3, 17,31, 19,55,  0,81,  7,91,  5,71, 11,66, 
    10,49,  4,20, 13,37, 18,95, 19,24, 12, 6,  9,65, 17,34,  3,63,  7,50,  8,12,  6,64, 14,56,  2,31, 11,37,  1,93, 15,89, 16,32,  0,96,  5,47, 
     3,35, 19,15,  9,66,  4,89, 18,72, 15,47,  2,58, 10,34, 16,61, 17,50,  0,17,  6,31,  1,80,  5,84, 11,13,  7,43,  8,96, 12,73, 14,37, 13,97, 
     1,56,  7,64, 13,15, 16,78, 17,35,  5,71,  8,85,  4,63, 12,36, 19,91,  6,48,  3,26, 18,14,  0,55, 11, 5, 14,62,  2,57, 10,94, 15,53,  9, 6, 
    10,97,  6,18, 14,12, 19,65, 16,77,  4,69, 12,27, 17,78,  3,34,  8,59,  1, 2, 15,22, 13,77, 18,51,  9, 6,  2,25,  7, 6,  5,49, 11, 7,  0,97, 
     1,83, 14,27, 13,34, 12,98, 19,41,  7,43, 15,22, 16, 4, 17,45,  0,19, 10,44, 11,62,  6,79,  2,12,  8,31,  4,60, 18,34,  3,93,  9,95,  5, 1, 
     9,39,  5,74, 11,94, 16,68,  3,77, 18,10,  8,29,  7,60, 17,13,  4,38, 10,44,  2,64, 19,61,  6,95,  1,73,  0,37, 13,37, 12, 2, 15,13, 14,32, 
    10,12,  5,60,  6,43, 12,12,  1,40,  4,78,  7,66, 11,25, 13,76, 19,28,  2,68,  3,59, 17,10, 15,77, 14,87,  8,93,  9,45, 18,44, 16,95,  0,23, 
    18,55,  4,61, 16,62,  2,49, 19,62,  8,94, 12, 2,  0,93,  3,35, 10,82,  1,63,  9,25, 13,84, 17,61, 15,60,  6,95, 11,42,  7,19,  5, 1, 14,99, 
    15,13, 11,55,  8,31, 17,34, 12,70,  2,95,  0,86,  5,33,  3,64, 19,10, 16,97, 14,11,  9,79, 10,53,  6,87,  1, 5,  7,97,  4,78, 13,82, 18,58, 
    19,68,  8,25, 13, 8, 12,48,  1,86,  5,26, 14,66, 15,96, 16,27,  3,77,  0,20,  4, 5, 10,10,  2,64, 18,66, 11,57, 17,74,  9,96,  6,11,  7,83, 
     4,33,  6,44,  9,21, 15,31,  1,12, 12,19,  0,85, 14,16, 10,79, 18,98, 17,20, 13,18,  2,21,  8,77, 16,18,  3,45, 11,58,  7,49,  5,94, 19,88, 
    18,71, 19,36, 13,83,  8,48, 14,23, 17,84,  0,18,  9,95, 10,26, 16,94,  2,60,  5,28,  4,81, 15,71,  7,75,  3,63, 11,40, 12,65,  1,34,  6,97, 
     5,13, 16,50,  1,15, 18,85,  0,23, 13,57,  6,93, 12,84,  4,74, 15,55,  9,94, 14,13,  3,99, 17,10,  8,59, 10,60, 19,95, 11,76,  7,69,  2,14, 
    14,44, 17,64, 19,29,  5,69, 10,96,  1,23,  4,27,  2, 4, 11,13,  6, 8,  7,59,  8,29, 15, 3, 12,79,  0,14, 13,45,  3,59,  9,85, 16, 2, 18,75, 
     1,40,  5,33,  6,11, 17,61, 12,16,  8,85, 19,95, 14,83,  9,82,  4,94, 11,37, 13,50, 15,61, 16,82, 10,36,  7, 6, 18,14,  3, 2,  0,99,  2,78, 
     6,73,  9,48,  4, 8, 16,34,  3,80, 15,67, 11,47,  5,13, 10,99,  7,10,  2,31,  0,65, 17,65,  8,26, 18, 3, 13,47,  1,68, 19,11, 12,12, 14,43, 
    10,64, 12,45,  3,18,  6,24, 14, 9,  2,39, 17,98, 11,51,  5,28,  9,18,  8,34,  7, 2, 15,18,  4,24, 18,82,  0,36, 19,48,  1,51, 16, 2, 13,86, 
     9,68, 11,72, 15,54,  2,29, 17,85, 10,11,  4,36, 14,35, 18, 8,  6,47,  8,70,  3,56, 16,32,  7,97, 13,27,  1,32, 19,77,  0,50, 12,15,  5,64, 
     3,57, 17, 4, 19,87,  8,45, 12,72, 11,84,  1,64,  6,13, 14,33,  7,93,  0,20, 18,31,  2,69, 15,96, 10,59,  4,87, 16,82,  9,91, 13,91,  5,15, 
     8,40, 15,98, 18,95, 10,96, 13,28,  7,91,  3,65, 16,31,  0,12, 14,25,  1,11,  2,11,  4,69, 17,77, 11,52,  5,17, 12,44,  9,42, 19,76,  6,89, 
    11,86,  0,26,  6,77,  1,59,  7,54,  9,17, 10, 9, 14,37, 12,29, 15,37, 13,42,  8,26, 19,84,  5,98,  2,53,  4,50, 18,75, 17,37,  3,93, 16,60, 
    10,49,  2,67, 13,35, 11,64,  9,10,  4,17,  3, 1,  5, 6, 18,82, 19,75,  6,32,  1,41, 15,26,  8,93, 17,74,  0,90, 12,33, 16,35, 14,99,  7,50, 
    18,35,  1,71,  9,57,  3,46, 12,11,  6,44, 10,86, 13,53, 17, 4,  5, 1,  0,22,  4, 2, 11,40,  2,82, 15,60, 16,91,  7, 3, 19,45,  8,79, 14,72, 
    12,55, 16,56,  2,96,  8,29,  9,28, 19, 8,  6,29,  0,26,  3,78,  7,27, 14,29, 10,82,  1,31, 15,58,  5, 1, 11,83, 17,55, 18,81, 13,28,  4,99, 
     1,71,  4,42,  8,23, 15,55,  2,59, 14,97,  6,86, 18,20, 13,86, 12,32, 10,57, 16,63,  0,18,  9,95,  7,39,  3,90, 17, 3, 19,37,  5,87, 11,85, 
    12,60, 16,44, 17,77,  1,22,  4,75, 11, 4,  0, 6, 15,68,  9, 6,  3,19,  8,82,  5,95, 13,36, 10,48,  2, 7, 19,88,  6,54, 18, 2, 14,75,  7,13, 
     6, 7, 10,77,  2,77, 18,13,  5,97,  3,22, 13,21, 11,88,  4,90,  7,95, 17,11, 12,30, 19,61,  8,11,  9,39,  0,48,  1,88, 16,58, 14,23, 15,89, 
    17,56,  8,65, 16, 5,  4,89, 14,99, 11,60, 13,73,  7,56, 12,68,  3,63, 10,48,  1,70,  9,51, 15, 7,  2, 8,  5, 7,  6,43,  0,18, 19,18, 18,81, 
     3,52,  5,15,  4,75,  1,66, 15,92, 16,66, 17,16, 12,96,  7,72, 14,35, 11,26, 13,21, 18,69, 10,86, 19,15,  6,60,  0, 6,  9,61,  8, 2,  2, 8, 
    11,92,  3,84, 17,16, 16,27, 10,77,  1,82, 14,67, 18,74,  0,81,  4,92, 13,75,  5,92, 12,50,  8,83,  7,43,  6,42,  2,49, 15,51,  9,36, 19,89, 
    11,42,  3,97,  8, 2,  9,70,  2,59, 15,57,  0,30, 12,23,  7,49, 14,20, 13,68,  4,10, 10,61, 17,84,  6,15, 16,58, 18,28,  1,61,  5,67, 19,10, 
     6,29, 15,57, 14,26, 19,79, 13,71,  1,15, 17,46,  3,74, 16, 6, 12,97,  2,51, 10, 3,  8,63, 18,79,  0,85,  4,86,  9,43,  7,22, 11, 1,  5,39, 
     1,82, 14,35, 16,20,  8,49,  2,97,  3,15,  6,34, 18,92, 12,23,  5,60, 10,13,  7,52, 19,38,  4,34, 13,68,  0,97, 17, 1, 11,18, 15,73,  9,72, 
    11, 4,  6,86, 15, 2,  4,24,  3,42,  2,70, 16,55, 19,45, 12,49,  5,96, 18,78, 14,73,  7, 1,  1,48,  8,17, 13,53, 10,15, 17,48,  0,86,  9, 4, 
    16,87, 14,40,  9,55, 12, 7,  6,33,  2,44,  5,78,  1,21, 11,48,  8,92,  3,60, 15,69, 13,32, 18,75, 19,89, 10,72,  4,62, 17, 7,  7,50,  0,22, 
     0,74,  7,52,  6,82, 13,25, 16,24,  5,93, 15,74, 12,58, 14,83,  4, 2,  8,12,  3,76,  9,97, 19, 1,  2,11, 18,29,  1,57, 10,41, 11,29, 17,78, 
     5,32,  3,83,  2,80,  0,89, 13,99,  4,25, 16, 5,  6,29,  9,77, 10,41,  7, 6, 15,77,  8,71, 19,21, 18,14, 12,73, 14,61, 11,26, 17,54,  1,99, 
    14,19, 13,40,  3,68, 10, 4,  4,99,  5,72, 19,18, 15,45, 16,33,  9,41,  7,38,  0,75,  1,61, 12, 8, 18,73,  8,80,  2,73, 17,64,  6,33, 11,14, 
    14,77, 19,61,  5,21, 11,82,  7,77,  3,24,  9,96,  1,35,  4,11,  2,23, 15,77,  6,86, 12, 8, 17, 5,  0,43, 13,12, 18,34,  8,31, 10,44, 16,15, 
    11,51,  7,63, 19,19,  4,14,  3, 5, 13,50, 17,18, 16, 4, 14,84,  2,50,  1,41, 12,39,  8,98,  5,67,  6,31,  0,59,  9,69, 15,91, 10,67, 18,34, 
     4,92, 15,57,  8,73, 19,80,  1,57, 13,77, 18,83,  2,97, 17,47,  0,95,  5,84, 11, 9,  6,56,  3,51,  7,32, 14,11, 16,67,  9,46, 12,50, 10,69, 
    12,40,  8, 1, 13,51, 11,33,  1,75, 17,63, 19,62, 14,54,  0,36,  4,17,  7,59,  6,96, 10,68,  9,44,  3,57, 15,89,  5,50, 18, 9, 16,11,  2,10, 
    18,45,  1,99, 19,92,  2, 8,  5,77,  8,15, 17,80,  4,13, 11,50,  9,30, 14,66, 12, 9,  7,77,  0,72, 16,26,  6,96, 13,85,  3,58, 15,43, 10,42, 
    12,56, 10,28,  1,81,  3,20, 15,12, 13,14,  2,47,  8,83,  9,64, 11,44, 18,42, 19,68,  5,95,  7,17,  4,55, 16,62, 17,69,  0,71,  6,93, 14,12, 
     7,17,  6,26, 12,98, 10,49,  8,78, 14,36,  9,87,  1,96, 16,32,  5,23,  0,56, 11,84,  3,21,  4,69, 19,98, 17, 9, 13,75, 18,78, 15,32,  2,57, 
     8,33,  5,82, 18,69, 17,32,  7,61, 15,49,  1,61, 10,98, 14,81, 13,61,  3,19,  6,63, 12,46, 19,81, 16,22,  0,80,  4,43, 11,20,  9,57,  2,31, 
     5,97, 13,65,  4,83, 19,61,  7,35, 12,85, 10,98,  2,56,  9,21, 14,96, 17,20,  1,79, 11,70, 15,38,  6,31, 16,57, 18,33,  3,83,  0,58,  8,29, 
     7,37, 11,23, 13,32, 19,94,  9,86,  6,44,  2, 6,  4,61, 14,22, 12,15, 18,42, 17,67,  3,36,  0,83, 16,52, 10,49, 15, 5,  8,85,  5,63,  1,60, 
     5,99,  2,95,  4,66, 18,61,  7, 5, 14, 6, 13,57, 16, 3,  8,37, 19,87, 17,27, 15,11, 11,28,  9,61, 12,93,  3,74,  1, 5, 10,42,  0,45,  6,51, 
     7,18, 10,44, 19, 4, 13,65,  9,82, 12,97, 16,28,  1,62,  5,36, 14,96,  6,76, 18,59, 17,51,  8,16,  2,89,  0,39,  3,55, 11,10, 15,24,  4,62, 
     0,78,  1, 8,  3, 9, 18, 4, 14,48, 10,48,  5,77,  9,65,  4,71,  6, 4, 12,18, 11,93, 15,70,  8,41,  7,27, 13,85, 17,62, 19,20, 16,27,  2,90, 
     7,57,  4, 1, 12,14, 17,11, 19,38, 13,74,  1, 6,  6,72,  3,16, 10,11,  0,25,  5, 5, 15,86,  2,83,  8, 5, 14,41, 16,52, 11,39,  9,80, 18,74, 
     5,76, 16,47, 13,33,  8,31, 18,61,  3,78, 15,27, 12,68, 17,34, 14,99,  1,44,  7,64,  9,13, 10,45,  4,55,  6,19, 19,99, 11,34,  0,84,  2,74, 
    17,38,  7,79, 14, 6, 12, 8,  3,12,  9,72,  5, 4,  8, 4,  0, 8, 10,55, 11,23,  1,77,  6,52, 16,55,  4,54, 15,30, 13,84, 19,79,  2,66, 18,21, 
     5,59, 14,31,  7,83,  9,57, 10,67, 12,17, 15,70, 19,23, 18,14, 13,54,  8,10,  1,24,  0,79, 17,67,  6,77,  4,90,  2, 7,  3,94, 16,34, 11,69, 
     1,36,  9, 7,  8,27, 11,16, 13,16,  5,10,  3,60,  7,78,  6,80, 14, 7, 17,37, 18,76, 10,31, 16,36,  0,26, 12,63, 15,78,  2,78, 19,92,  4,46, 
    19,71, 16,60, 10,74, 17,29, 14,27, 13,57,  4,83,  3, 5,  5,12,  6,68,  2,96,  8,25, 11,50, 15,85,  9,23,  1, 1,  7,77,  0,74, 18,21, 12,33, 
    16,24, 14,21,  6,94, 17, 4,  9, 3,  4, 4,  3,56,  5,23,  2,70, 15,42, 11,68,  7,98, 18,48, 12,48,  1, 9,  8,98,  0,64, 13,69, 10,56, 19,84, 
     7,99, 15,52,  3,52, 12,72,  9,95, 17,10, 13,94, 10,51, 11,77,  1,13, 14,82, 16,61,  6,68,  0,39,  2,60, 19,43,  5,94,  8,28,  4,36, 18,77, 
    19, 4, 10,45,  4,11,  9,16, 13,98,  5,94, 16,20, 12,99,  2,89,  1,63, 17,35,  7,72,  3,88, 15,22, 18,72, 11,71, 14,69,  8,50,  0,77,  6,40, 
     1,67, 10,90,  3,46, 17,10, 11,19,  0,24,  5,36,  6,38, 15,73,  9,74, 16,33,  2,17, 14, 9, 18,15,  4,95,  7,33, 12,46,  8,85, 19,34, 13,51, 
    12, 9, 11, 8, 13,97,  7,64,  4,64, 17,96, 15,33,  6,26,  9,88, 18,67, 14,20,  1,18, 16,67, 19,51,  5,77,  0,43,  2,77,  3,83,  8, 9, 10,20, 
    19,43,  0,21, 16,51, 10,35, 13,11, 18,30,  4,89, 14,48, 17,70, 11,62, 15,56,  7,94,  2,12,  8,25,  9,83,  1,21, 12,91,  6,80,  5,43,  3,75, 
     9,13, 10,35,  8,43,  6,80,  3,53, 19,11,  7,97,  1,29, 18,83, 17, 6, 13,73,  0,10, 14,55, 11,32,  2,49, 16,50, 12,22,  4,86, 15,20,  5,81, 
    13,55,  4,22,  2,91, 19,63,  3,15,  9, 5,  8,96, 12,53, 14,68, 18, 5, 10,59,  0,18,  5,87, 11,76, 17,19, 16,92,  1,18,  7,17,  6,20, 15,29, 
     0, 8, 16, 2,  7,32,  3,12, 13,55, 18,27, 17, 9, 10,60, 15,44, 14,86, 12,66,  6,52,  9,35,  4,21, 19,76,  1,50,  5,58,  8,25,  2,26, 11,22, 
    19,18,  0,94,  5,83, 16,51,  6,91, 18,74, 13, 9, 11,29,  2,93, 15,75, 12,62,  4,46,  3,68, 10,77,  1,34,  7,65, 14,73,  9,73,  8,40, 17,44, 
     6,53,  1,17,  3,56, 19,38, 12,63, 16,65, 10,38,  4,65,  7,48, 15,59,  8,40, 18,33,  5,15, 13,86,  9,66, 17,38, 11,95, 14,80,  2,47,  0,10, 
     4,40,  9,68,  8,53, 14,69,  5,22,  0,97, 10,49,  1,71,  2,82,  6,78,  7, 3, 19,17, 18,20, 12,52, 17, 8, 15, 5, 11,99,  3,64, 13,85, 16,61, 
     4,94,  1,12,  6,38, 10,68, 13,48, 12,33, 19, 8,  0,42,  3,81,  5,54, 14,67, 15,63,  8,45, 11,44,  7,11, 18,48,  2,25,  9,26, 17,56, 16,48, 
     6,59, 10,38,  5,83,  0,43, 18, 8,  9,29, 19,56, 16,54,  8,34, 15,41, 11,51,  3,89, 14,83,  1,13,  2,61,  4,13, 13,33, 12, 5, 17,36,  7,10, 
     9,94,  0,42, 13,54, 16,41,  8,78, 15,80,  4,53,  1,98,  6,10, 14,86, 19,19, 18,72, 17,24,  2,96, 12, 6, 10,14,  3,27,  5,19,  7,19, 11,88, 
    18,63, 14,34,  3,66,  8,80,  6,61,  0,12, 12,30,  9,31, 16,40, 11,94, 19,92, 10,89,  7,58,  4,49, 13,29, 15,93, 17, 1,  2,56,  1,50,  5,99, 
    15,97, 18,72, 14,27,  4,38,  6,72, 13,90,  8,91, 17,51,  9, 9,  1,14, 16,67,  7,65,  0, 4, 12,13,  3,84, 10, 1,  5,20, 11,73,  2,12, 19,62, 
     1,82, 10,84,  0,45, 12,46,  9, 5,  6,25, 13,31,  8,60,  2,12, 11,32, 19,21, 14,50,  4,77,  3,36, 15,86,  7,34, 18,11, 17,12, 16,60,  5, 1, 
     0,41,  4,22, 15,20, 10,76,  1,45,  5,60,  8,97, 16,82, 19,33,  2,59, 14,48, 11,58,  9,95, 18,45, 13,21,  3,85, 12,82, 17,50,  7,18,  6,21, 
    10,21,  5,66, 17,31,  3,15,  4,92,  7,11, 13,34, 16,46,  0,28, 19,96, 14,72,  2,79,  6,85,  1,78, 11,31,  9,76, 15,25, 12, 2, 18,20,  8,67, 
     5,77,  6,39, 18,71, 13,83, 11,70,  9,47, 10,32, 14,11,  2,26,  7, 1,  1,75,  0,17, 17,77,  4,98,  8, 5, 15,10, 19,11, 16,33, 12,73,  3,67, 
    18,16,  9,39,  6,29, 12,27, 10,70,  3,52, 13,11,  1,26,  7,62, 14,19, 17,99,  0,38, 11,95,  4,13, 19, 3, 16,44,  8,30, 15,46,  2,72,  5,89, 
     0,65, 10,97, 17,23, 12,38, 14,57,  3,73,  1,33, 16,11,  7, 9,  4,96,  9, 4, 11,78, 18,38,  6,54, 13,63, 19,98,  2,86,  5,58, 15,49,  8,96, 
    14,95, 12,86,  3,58, 11,46, 10,51, 16,99,  1,59,  2, 4,  0,33,  5, 6, 17,50,  8,49,  6,62, 19,69,  7,37, 15,65,  9,12,  4,56, 13,88, 18,82, 
     3,61, 14,53, 11,33, 13, 5,  8,38, 18,63,  7,35,  0,74, 19,56,  4,54, 16,73,  9,17, 15,61, 10,49,  2, 4, 12,76, 17,11,  5,64,  6,70,  1,50, 
     8,17,  4,35, 17,73, 15,28,  0,81, 11,18, 13,95,  6,26, 12,93,  3,32,  2, 3,  7,39,  5,13,  1,97,  9,94, 16,53, 18,11, 19,80, 14,30, 10,28, 
     3, 1, 18,98, 19,32,  7,56, 15,39,  2,29, 14,33, 12,82, 10,57,  5,17,  0,39, 17,77, 13,73,  8,12,  4,91,  9,67, 11,72, 16,94,  6,14,  1,73, 
     1,36,  3,48, 18,66,  7,23, 16,52,  0,92,  9,90, 13, 1, 12, 1,  6,39,  4,49, 11,25,  8, 4, 14,16,  2,74, 19,36,  5,51, 17,61, 10,16, 15,56, 
     0,90, 11,21, 16,99, 14,29,  4,49,  1,13,  7,94,  5,26,  2,20, 17,73, 10,21, 15,28, 18,54,  8, 3,  3,97, 19,43, 12,48,  9,55,  6,20, 13,70, 
    16,64, 18, 6, 11,72,  3,74, 17,25,  5,43,  4, 8, 19,20, 15,80, 13,89,  7, 6,  0,44, 14, 2,  1, 6,  8,54,  6,51, 12,41,  2,98,  9,90, 10,94, 
    16,91,  8,59, 15,27,  6,65, 11,45, 13,22,  5,97,  9,53, 12,72, 10,47,  3,98,  0,49, 18,76,  1,86, 17,82, 19,52, 14,62,  2,48,  7,37,  4,25, 
    19,97,  0,28,  3,87, 14,56, 11,96, 10,14,  9,78,  4, 5, 16,29,  8,14, 12,73,  7,52, 13, 2,  2,34,  6,19, 18,16, 17,21,  5,79, 15,71,  1, 6, 
    12,93, 16,14,  8,82,  0,81,  5,55,  4,54, 10,51, 13,61,  2,22, 19,28, 14, 3, 18,35, 11,22, 15,46,  6,80,  3,66,  1,92, 17, 2,  9,17,  7,61, 
     4,53,  1, 2, 16,51, 13,65, 10,75, 14,14, 12,13,  7,51,  8,18, 11,79, 15,25, 17,52,  9,49,  3,43,  0,49,  6,83, 19,36,  5,54,  2,43, 18,72, 
     3, 1,  0,32, 17,66,  6,52,  5,90, 12, 8, 13,33, 18,72, 19,80, 14,40,  4,38,  9,98,  2,71,  7,98, 16,14, 11,35,  1,23, 10,22, 15,23,  8,93, 
     2,49, 19,16,  9,85, 14, 9,  3,61,  1,21,  0,86, 16,14,  6,81,  7,59, 15,34, 17,59, 11,19, 13,85, 18,29, 12,67,  4,46,  5,90,  8,42, 10,23, 
    16,25, 18,84,  7, 4, 17,53,  1,91, 10, 1,  8,26, 12,19,  2,98, 11,97,  0, 5, 14,24,  9,76,  4,28,  5,18,  6,81, 15,70, 13,84,  3,11, 19,66, 
    13, 5, 15, 6,  9,81, 12,49,  7,30,  0,77, 16,44, 19, 2,  2,73,  1,78,  8,15, 10,38, 11,39,  6,88, 18,72,  4,69, 14,30,  3, 6, 17,46,  5, 7, 
     9,41,  5,14, 14,85,  7, 1,  3,69,  4, 9, 19,43, 16,88, 17,44, 12,85,  6,80,  1,50, 11, 3, 13,31,  8,74, 10,88, 15,85,  2, 3,  0,72, 18,85, 
    11,78,  4,50,  8,76, 12,48,  3,38,  2,84, 10,21, 13,57, 16,72, 18, 8, 17,55,  0,56,  1,23,  9, 4,  5,57, 15,56, 19,58,  6,27, 14,66,  7,11
  };
  // Taillard 100x20 instance 9
  const int tai_100_20_9[] = {
    100, 20, // Number of jobs and machines
     9,54, 12,87,  4,61,  7,35, 16, 5,  0,48,  1,33, 18,21, 17,65,  5,83, 14,78, 11,14,  8,70, 10,25,  3,36, 15,56, 13,87,  2,32, 19,96,  6,62, 
    17,68, 13,15, 19,22,  6,53, 10,33, 11,61,  1,73,  7,63, 16,96, 18,61, 14,88, 15,86,  5,53,  3, 2,  9,29,  2,14,  0,49,  4,14,  8,22, 12,66, 
    17,68, 10,70,  3,84,  4,19, 19,32,  7,58, 15,87,  0,82, 16, 7, 12,47, 14,68, 18,71,  8, 3, 11,93,  6,24,  2,31,  5,19,  1,56,  9,88, 13,71, 
     9,58, 19,72,  3,51, 10,42,  1,31,  5,63, 16,49, 14,83,  7,28, 17,92, 18,83, 13, 7,  4, 7,  0,30, 11,14, 15,27,  8,58, 12,32,  2,16,  6,67, 
    13,36, 16,51,  4,65, 15,40,  9,79, 10,39,  0,33,  2,77,  3,84, 11,68, 17,71,  5,54, 19,40,  7,94, 18,56,  6,94,  1,55, 12,10, 14,13,  8,13, 
    16,51,  7,27,  9,31,  6,65, 13,26,  5,62, 18,88, 11,69, 17,47, 19,36, 14, 3, 15,90, 10,90,  0,31,  4,24, 12,35,  2,24,  1,68,  3,18,  8,70, 
    14, 8,  5, 6,  1,93, 10,60,  9,68, 13,32, 16,95, 17,17,  2,12, 18,79, 19,78, 12,26, 15,15,  6,83,  0, 8,  3,31,  7, 5,  4, 6, 11,36,  8,75, 
     1,41, 11,95, 10, 4,  3,98, 17,22,  9,64, 14,41, 18,56,  0,23,  4,72, 15,10, 16,35,  6,55, 12,17,  7,10,  5,32, 13,76,  8,49, 19,95,  2,14, 
     2,96, 17,23,  8,44, 19,19, 10,90, 13, 6, 15,67, 12,37, 16,64,  1,80, 18,98,  6,66, 11,74,  0,65, 14, 3,  3,15,  9,50,  7,98,  5,46,  4,80, 
     3, 1, 17,65, 11,36,  0,35,  5,95, 15,99, 13,82, 12,46, 16,26, 10,35,  2,96,  6, 6,  4,28,  9,92, 19,12,  8,42,  1,47,  7,78, 18,10, 14,76, 
     2,78,  8,99, 14,90,  5,46, 15,71, 13,76, 10,45, 19,96, 12,58,  9, 3,  4,70,  0,80,  6,30,  3,85, 18,93,  7,16, 16,26, 11,79,  1,44, 17,21, 
     1,82, 13,16,  8,56,  3,35,  7,81, 14,97,  9, 5,  6,68, 10,60, 15,33, 19,57, 11,42,  4,72, 16,77,  2,53,  0,26, 18,66, 17,81,  5,84, 12,84, 
     5,29,  0, 6,  7,38,  1,96,  4,85,  8,36, 10,71,  3, 5, 13,53,  9,11, 11,87, 17,99, 12,42, 14,69, 15,74,  6,34,  2,25, 19,10, 16,25, 18,32, 
     7,40, 19,73,  5,80,  6,70,  9, 7, 14,35,  8,35,  4,20,  3,68, 15,29,  1, 7, 10,12, 11,70, 12,47,  2,46, 16,78,  0,28, 13,48, 17,50, 18,22, 
     2,94, 18,79,  0, 1, 13,39,  6,33, 16,94,  5,69, 19,19, 14,29,  9,33,  7,72,  3,48,  1,88, 10, 9, 17, 1,  8,99,  4,20, 11,29, 12,83, 15,44, 
    12, 4, 11,18,  1, 9, 17,82,  4,59, 13,64, 16,56, 18,78,  6,23,  7,61,  8,80, 10,91,  2,38, 14,89,  5,85,  3,23,  9,48, 19,90,  0,29, 15,97, 
    11,10,  8, 9, 10,82, 17,28,  3,48, 14,66,  1,22,  9,32, 13,34,  5,43, 19,64,  4,43, 18,24,  6,88, 16,44, 12,15,  7,28,  0,54,  2,94, 15,80, 
     2,50,  7,54, 15,27,  0,69, 17,34, 18, 5, 10,22,  3,29, 16,47, 11,24, 19,52, 13,26,  9,51, 14,58, 12,50,  5,84,  8,15,  1,93,  4, 2,  6,16, 
    16,71, 12,25, 15, 2, 11,75,  5,62,  3,79,  2,35,  8,87,  0,19,  6,50, 10,33,  7,79,  9,63, 18, 9, 17,24, 19,39,  4, 2, 14,20, 13,84,  1,53, 
     1,78, 14,11, 19,42,  0,60, 10,76, 16,57, 15,29,  7,50, 17,79, 18,81,  3,19,  4,24, 13,91,  8,57,  5,80,  9,74,  6,99, 11, 4,  2,62, 12, 2, 
    12, 4,  9,65,  2,42, 15,97, 14,56,  8,92, 17,49,  6,83,  1,18, 11,86, 10,48,  5,24, 16,42, 13,99,  4,87,  3,59,  0,22,  7,34, 18,63, 19,45, 
    11,54, 17,64, 18,80, 12,51, 19,75, 14,42,  8,60, 10,33, 15, 6,  7,90,  2,32,  1,77,  4,11, 16,63, 13,40,  9,30,  0,37,  6,62,  3,94,  5, 8, 
    18, 6, 12,77, 17,76,  8,23,  5,61, 10,90,  6, 6,  9,85, 13,45,  0,86,  2,10, 15,71,  7,13,  4,68, 16,16,  1,33, 14,95, 11,52,  3,88, 19,39, 
     9,47, 12,30,  8,67, 18,99,  5,52,  2,29, 15,23, 11, 8,  0,77,  6,80,  1,46,  4,54, 17,64,  3,45, 13,17,  7,12, 14,35, 19, 3, 10,81, 16,15, 
    18,24, 16,50, 17,47, 11,88,  6,68,  8,42,  1, 2,  7,24, 12, 2, 10,63,  3,88,  9,72, 14,28,  0,47, 19,31,  2,62,  5,59,  4, 6, 15, 9, 13,60, 
    17,68, 15,82,  6,22,  4,44,  7,55,  1,25,  0,45, 13,75,  9,13, 19,84, 18,17,  5,67,  3,21, 10, 6,  2,49, 16,67, 11, 7,  8,74, 14,29, 12,20, 
     3,73, 15, 2,  7,26,  8,15, 10,29,  9,44,  2,26, 17,99, 19,86, 14,59, 16,13,  5,90,  1,13, 18,17, 12,68, 11,88,  0,42,  4,61, 13,65,  6,72, 
     3, 3, 17, 1, 12,40,  9,33,  4,29, 10,42, 15,13, 13,15,  0,78, 11,37,  8,30,  2,92,  1,49,  5,91, 19,42,  7,74, 16,53, 18,59,  6,56, 14,32, 
    19,31, 16,84, 18,29, 13,75,  5,68, 11,12,  8,29,  9,51,  0,76,  2,98, 14,33, 17,99, 10,28,  4,17,  6,89,  1, 4,  3,21, 12,84,  7,90, 15, 4, 
    18, 5,  8,69,  2,19,  6,45, 12,35, 17,89, 19,40, 10,38,  9,30, 15,52,  5,73,  1, 7,  3,45,  7,60, 14,43, 13,55, 11,10,  4,97,  0,93, 16,63, 
    12,76, 13,61,  1,21,  3,41, 18,74, 17,51, 15,36,  6,49,  5,26,  9,31, 11,67, 16,69, 14, 6, 10,54,  0,65,  8, 7,  4,70,  7,45,  2,32, 19,25, 
     7, 1,  8,69, 13,28,  0,25, 16,25, 12,24, 15,74, 17,48,  6, 4,  4,12, 11,80, 18,28, 10,75,  9,82,  2,34, 14,99,  5,19, 19,59,  1,10,  3,11, 
    11,46, 17,81, 19,64,  7,28,  6,89, 12,71, 16,29,  9,27, 13,81,  0,34,  2,78, 15, 7, 10,51,  5,25,  4,47,  1,14, 18,32, 14,47,  3,90,  8,31, 
     6, 4,  8,40,  9,32, 17,52, 16,37,  4,35, 10,85, 14,26, 18,45,  2,58,  7,42, 19,75,  5,13,  0,15,  3,86, 11,74, 13,98,  1,19, 15,45, 12,60, 
    16,44,  8,65,  3, 5,  4,30, 13,51,  1,42,  2,88,  9,79, 10,47,  7,46, 12,62,  6,50, 17,58,  5,82,  0,51, 18,89, 11,89, 19,63, 15,59, 14,84, 
     7,50, 11,61,  2,36, 15,14, 16,65,  4,86, 18,87,  0,28, 17,50, 13, 8, 19,99, 10,73,  9,96, 12,94,  1,87,  6,68, 14,96,  8,69,  3,19,  5,33, 
    12,81,  6,81,  0,29, 17,54,  5,24, 10,55, 13, 5,  8,36,  1,48, 16,41,  4,34,  7,33, 11,48, 19,37, 18,32,  2,34,  9,52, 14,27,  3,80, 15,14, 
     2,36,  8,31,  7, 7, 10,88, 17,99,  6,55, 13,11,  3,65, 19,47, 18,18,  9,46, 12,39, 11,78, 14,46,  1,81, 16,31,  5,80,  4,35, 15,92,  0,51, 
    10,65,  3,46,  1,31, 15, 3, 14,21, 11,55, 13,11,  2, 5, 16,40,  4,52,  8,99, 18,11, 12,36,  9,83,  5,92,  7, 5, 19,19,  0,58, 17,47,  6,34, 
     8,50, 14,56, 19,57,  2,27, 18,20, 17,26, 16,71,  5,59,  6,14, 13,46, 10,22,  3,66,  7,20,  0,33,  4,39, 12,33,  9,86,  1,30, 15,37, 11,77, 
     9, 1,  1,51, 19,77,  6,72,  4,87, 17,50, 13,94,  7,84, 15, 8,  8,73,  0,13,  2,12, 10,97, 16,86, 14,63, 18,44,  5,14,  3,67, 12,19, 11,52, 
     8,22,  7, 5,  4,51, 14,18, 18,23, 16,97, 19,91, 15,86,  2,13, 10,27,  6,68, 12,44,  9,93,  5,82,  0,17,  1,48, 13,98,  3,30, 17,86, 11,92, 
     2, 5,  9,92, 18,10,  7,63, 16,41, 13,26, 14,14, 15,30, 10, 3, 17, 9, 11,99,  3,54,  5,14,  0,98,  8,23, 12,11,  4,66, 19,48,  1,25,  6,79, 
     6,82,  2,50, 16,86, 10,19, 18,70,  0,76, 17,99, 19,34,  3,53,  5,23,  7,89, 14,82,  9,19, 12,55, 15,15,  8,23,  4,99, 13,63,  1, 1, 11,75, 
    10,43,  0,25, 12,40,  1,18,  6,43, 14,86, 18,75, 11,24,  7,82, 16,24,  9,71,  8,89,  4,67, 15,28, 13,96, 19,20,  5,14,  3,85,  2,59, 17,38, 
    14,28,  5, 8,  4,99, 19, 8,  9,53,  8,51,  3,92, 12,61,  1,79,  0,73, 15,80, 18,24, 17,55, 13,73, 10,55,  6, 9, 11,66,  7,22, 16,95,  2,60, 
     2,14, 13,89, 17,24, 11,37,  4,30,  9,88, 15, 1,  1,15,  7,14,  0,36,  5,14, 12,62, 16, 3, 14,27,  8,44, 10,50,  3,63,  6,26, 19,91, 18,85, 
     2,24,  5,74, 14,42, 10,43,  1,58,  7,29,  3,58, 18,54, 19,76,  6,22,  4,42,  8,80, 17,45, 12,28, 15,32,  0,59, 11, 1, 13,81, 16,48,  9,15, 
     4,90, 13,20, 10,88, 15,84, 14,39, 12, 6, 18,12, 11,14,  1,19,  5,44, 17,10,  9,26,  2, 6, 16,75,  0,24,  8,38, 19,53,  6,37,  3,69,  7,46, 
     3,54, 17,78,  4,20, 13,66,  8, 2, 15,52, 19,47,  7,84,  6,28, 11,11, 12,66, 14,45,  1, 8, 10,22,  9,14,  0,31,  5,88, 18,73, 16,21,  2,70, 
    11,36,  8,29,  0,65, 13,32,  5,79,  7, 2, 10,42,  9,95,  2,33,  4, 5,  1,62, 12,47, 17,20,  6, 2, 18, 2, 15,36, 14,22, 19,96,  3,61, 16,85, 
     3,27, 11,22, 15,60,  8,87, 10,30,  6,64,  5,75, 18,26,  2,25,  9,76, 19,28, 16,53, 14,65, 17,59, 12,92,  4,54,  7,92,  1,28, 13,20,  0,48, 
    10,57, 18,13,  3,93,  8,47,  2,40, 12,81, 11,89, 13,52,  6,58, 14,20, 19,19,  1,37, 16,69, 17, 1,  0,13,  7,60,  4,83, 15,35,  9,64,  5,73, 
     3,63, 14,86,  0,36,  4,85,  5,91,  7,12, 13,46,  6,52, 15,70, 17,25, 12,50, 10,41,  1, 4,  9, 9, 11,36,  2,99, 18,57,  8,91, 16,96, 19,43, 
     2,15,  8,84,  7,73,  9,93, 19,51, 16,14, 17,25, 15,75,  6,39,  3,14,  4,43, 13,73, 10,82, 12, 2,  0,72,  5,66, 14,30, 18,64,  1,19, 11,63, 
    18,38, 12,74, 11,59,  7,69, 17,60,  8,99, 13,14, 15,48,  9, 9, 14,50, 16,83,  5,85,  2,74, 10,10, 19,95,  3,10,  1,80,  0,92,  6,92,  4,39, 
     0,48,  7,75,  2,18, 16,65, 17,11,  4,32, 13,61,  1,46,  9,22, 10, 1, 11,38, 14,33, 15,78,  3,39,  5,67,  6,79, 12,66,  8,99, 19,22, 18,66, 
     3,86, 13,95, 18,30,  9,19, 17,90, 12,65, 14,79, 19,23,  7,69, 10,81,  0,30, 11, 9,  5,99,  8,86,  2,66,  4,62, 16,32,  1,97,  6,25, 15,37, 
     7,42, 18,67, 10,84,  5,55,  8,76, 13,48,  2,95, 16,59,  3,69, 14,53,  1,65,  4,30, 19,94,  9,85, 15,15,  0,96, 11,68, 17, 1, 12,30,  6,94, 
     6,28, 12,76,  8, 2,  0,86,  4,58, 16,40,  5,14, 19,32, 11,11,  2,13, 14, 1, 13,56,  7,92, 18,13,  3,85, 10,15,  9,74,  1,51, 17, 4, 15,66, 
     8,52, 17,73, 15,57, 10,13,  6,52,  3,49,  5,38,  1,14,  9,87,  2,63, 13,78, 11,40, 14,13,  7,23, 12,42, 18,99,  0,67,  4,28, 19,65, 16,92, 
     2,90,  7,59, 17,86, 10,30,  5,34, 12,79, 13,77, 18, 9,  0,84, 15,73,  4, 9,  1,44,  9,43,  8,59,  6, 6, 11,90, 19, 3,  3,19, 14, 4, 16,33, 
    15,34,  5,18,  4,25, 19,51,  8,21, 13,93,  3,22,  0,56, 14,80,  1,77, 12,67, 11,89, 10,15, 17,60,  9,15,  2,80, 18,23,  7,90,  6,86, 16,17, 
    10,21, 16,61, 19,25,  6,91,  2,67,  5,84,  0,15,  1,59,  9,35,  4, 5,  8,38, 12, 9,  3,54, 17,58, 14,36,  7,36, 13,69, 15, 7, 11,22, 18, 5, 
    17, 3,  4,73,  3,36, 11,75,  6,95, 12,71, 18,10, 14, 7,  9,13, 19,33,  7,39, 15,30, 10,82,  0,76,  5,68, 13,37,  1,62,  2,44, 16, 6,  8,51, 
    19,87,  1,31,  4,28,  6,68, 14, 7,  2,85,  3,57,  8,82, 16,98, 11,16, 18,12, 13,28, 10, 5, 15,56,  7,78,  9, 7,  5,18, 12, 7,  0, 9, 17,15, 
    16, 8, 17, 2, 11,50,  1,51,  8,97,  6,86,  7,72, 13,92, 18,29,  4,58,  3,10,  0,13, 10,25,  9,42,  2,11, 14,46, 19,74,  5,86, 12,20, 15,31, 
     8,43,  3, 4,  1,31, 14,98, 13,38,  7,13, 19, 9, 10,87,  0,68, 17,54, 12,75,  2,37,  4,11, 16,58, 18,46,  6,49,  5, 6,  9,15, 15,28, 11,86, 
    13,80, 16,90,  4,47, 17,26,  7, 9, 19,10,  6,18,  3,95,  2,19, 12,52, 11,17, 10,55,  1,74, 14,24,  9,85, 15,40,  0,62,  8,60,  5,96, 18,96, 
     6,30, 17,75,  7,59, 14,22,  5, 4,  4,52,  3,97, 10,39,  1,49, 12,36,  8,91, 15,79,  2,17, 18,38, 11, 4,  9,57, 16,44, 13,46, 19,17,  0,66, 
    11, 3,  6,70, 10,95,  4,22,  8,72, 18,39,  0,96, 16,94,  2,47, 17,35,  5,37, 14,59,  1,45, 15,87, 13,99,  7,33,  3,48,  9, 4, 19,92, 12,68, 
     7,18, 17,74, 15,88,  1,60, 12, 8,  9,45,  5, 1, 14,83,  0,71,  2,78,  4,71, 11,52, 13,36, 18,18, 16,12,  3,93, 19,72,  6,37, 10,32,  8,28, 
    11,46, 16,80, 10,12, 17,43, 18,76,  5,83, 13,74, 19,44,  7,97, 14,46, 12,30, 15,56,  4,79,  9,34,  2,56,  3,49,  1, 2,  6,63,  8,41,  0,87, 
     2,53, 13,35, 19,98,  4,19,  6,22,  7,31, 10,89, 18,94, 14,27, 17,92, 12,80, 16,60, 11,22,  8,31, 15,23,  5, 5,  9,40,  0, 5,  1,90,  3,13, 
    16,32, 11,43,  3,75, 10,15, 18,37, 14, 5,  1,64,  7, 4,  6,30, 17,29, 19,14, 15,35,  2,85,  0,75,  8,45, 13,45,  4, 2, 12, 7,  9,29,  5,68, 
     5,95, 13,11,  0, 5,  1,20,  3,49,  6,16, 17,50, 19,10,  4,15,  2,63,  8,97,  9, 9, 16,88,  7,64, 10,12, 11,95, 18,19, 12,16, 14,54, 15, 9, 
    18,52, 17,33,  5,16, 10,10,  8,87,  9,62,  0,76, 11,88, 12,22,  2,63, 19,50, 14,87, 16,16,  1,56,  6,38, 15,57,  4,52,  3,71,  7,15, 13,18, 
     8,89, 15,32,  1,77,  6,66,  2,88, 19, 6, 16,37, 17,35,  0,49, 10,50, 12,89,  5,62,  9,28, 13,31,  4,61, 14,41,  3,71, 11, 2, 18,22,  7, 9, 
     7,32,  8,20,  3,77, 17,70, 14,23,  5,82,  1,51, 11,75,  9,34, 15,55,  4,91, 12,49, 16,46,  6,80, 10, 6,  2,69, 13,70, 19,99, 18,85,  0,45, 
    18,32, 15,27,  8,43, 16,53, 13,70,  6,17, 11,57,  4,77, 10,91,  2,49,  3, 6,  1, 6, 19,47,  7,93, 12, 4,  9,54,  0,93, 14,68, 17,48,  5,93, 
    11,16,  3, 5, 10,39, 17,79, 19,51, 16,39,  5,39,  6,32, 13,38,  9,28,  8,96, 14,35, 12,56, 18,42,  1,45, 15,83,  2,42,  4,40,  7,25,  0, 3, 
    11,77,  8, 1,  5,11, 13,87, 18,21, 16,44,  1,92,  3,77,  7, 9,  6,92,  9,10, 14,31, 10,62,  2,65,  4,44, 19,78, 15,22, 17,13,  0,26, 12,72, 
    17,11, 19,46,  8, 8,  0,77,  4,31,  1,48, 11,37,  3,99, 13,27, 16,99,  9,31, 10,73,  5,60, 14,40,  2,81, 18, 9, 12,20,  7,81, 15,71,  6, 2, 
     6,60,  5,20,  1,62, 18,18,  7,18,  0,33, 19,38, 16,48,  2,37, 12,78, 14,76, 17,28,  3,65, 10,61,  8,40,  4,28, 15,67, 13, 4,  9,17, 11,26, 
     2,54, 17,89,  7,69, 12,93,  0,69, 14,93,  4,54, 19,30, 15,14, 10,14,  8,71,  1,49,  6,10,  3,86, 18,88,  5,67,  9,17, 11, 2, 16, 2, 13,20, 
    16,54, 12,83,  0,59,  2,70, 14,18,  5,34,  6, 7,  3, 2, 17,45, 15, 6, 18,80, 10,21, 11,72,  7,80,  9,89,  1,52,  8,33, 19,41,  4,76, 13, 5, 
     9,53,  3,15, 11,75, 18,10, 15,48,  1,54, 19, 1, 16,43, 13,22, 17,31,  0,63,  5, 1, 14,68, 10,74, 12,23,  4,32,  2,53,  8,71,  6,93,  7,22, 
    16,21, 14,51,  2,56, 11,39,  5,32, 13,17, 17,37, 19,21,  8,42, 15,81,  9,56, 10,29,  1,11, 12,78,  7,40,  4,66,  6,57,  3,92, 18, 5,  0,38, 
     7,30,  9,31, 16,92,  2,26, 14,56,  3,67,  5,88,  6,11, 18,25, 19,24, 13,98,  8, 8, 15,34, 17,18,  0,80, 12,82, 11,28, 10,87,  4,77,  1,14, 
     5,40, 18,43, 13,85, 10,84,  8,76, 14,64,  9, 7, 12,51,  4,51,  2,15,  7,96, 11,31, 19,85,  6,30,  1,85, 17,45,  0,73, 15,73,  3,17, 16,57, 
     6, 3, 11,32,  8,92,  0,97,  1,78,  2,83, 15,37,  7,41, 12, 4, 14,62, 18,16,  5,36, 16, 4, 10,26, 19,28, 17, 2,  4,16,  3,37,  9,93, 13,26, 
     7,64, 10,90, 16,88,  9,32,  5,13, 15,70, 12, 5,  6,30,  3,44,  2,82,  0,98, 19,44, 18,67,  8,24, 11,49,  4,99, 17, 9,  1,28, 13,96, 14,70, 
    15,83, 18,29, 19,27,  0,78, 10,85, 12,10, 14,77,  9,91,  1,24,  3,97, 17,19,  2,54,  8,40,  6,39, 13,46, 16,89,  7,83,  5,93, 11,53,  4,90, 
    13,76, 19,56,  7,40,  4,73,  6,14, 18,74, 12,75,  1,55, 17,90, 16, 8,  2, 9, 14,59,  3,60,  5,27, 15, 1,  0,17, 11, 6,  9,41, 10,41,  8,10, 
     0,23,  3,77,  8, 7, 12,66,  9, 5, 16,85,  2,87,  6, 1, 19,40, 14,69,  5,62, 17,90, 11,20, 15,18,  4,32,  1,36, 18, 4,  7, 6, 10,47, 13,28, 
    11,45,  9,87, 19, 4,  3,17,  1, 6,  7,13,  2,72,  4,66, 18,68,  6,87, 15, 1, 17,79, 16,44, 14, 5, 10,33,  0,32, 12,20,  8,63, 13,73,  5,62, 
    10,81,  5, 3,  9,13,  8,42,  3,22,  1,32,  0,83,  4,56,  7,28, 15,96, 11,34, 19,42, 18,22, 13,86, 14,45, 17,79,  2, 7, 12,43, 16,27,  6,41, 
     6, 4, 10,55,  4,31, 19,22,  2,28, 13,44, 12,15, 11,88,  8,27,  9,66, 17,50,  0,25,  1,27,  7,91, 14,53, 15,71,  3,10, 16,12,  5, 2, 18,61, 
    11,70,  9,69,  8,26, 16,78, 13,42,  7,97, 19,68,  5,88, 10,37,  3,23, 18,46,  2,55, 14,29, 17,81, 12,54,  4,70,  0,71,  6, 2,  1,53, 15,28, 
    11,39, 12,56,  9,87,  7,21, 15, 9, 18,82, 16,44,  8,63, 19, 9,  4,50,  1,34, 17,58, 13,18,  5,87,  0,42, 14,78,  2,98,  6,30, 10,46,  3,21
  };
  
  const int* js[] = {
    &test[0],
    &abz5[0], &abz6[0], &abz7[0], &abz8[0], &abz9[0],
    &ft06[0], &ft10[0], &ft20[0],
    &la01[0], &la02[0], &la03[0], &la04[0], &la05[0], &la06[0], &la07[0], &la08[0],
    &la09[0], &la10[0], &la11[0], &la12[0], &la13[0], &la14[0], &la15[0], &la16[0],
    &la17[0], &la18[0], &la19[0], &la20[0], &la21[0], &la22[0], &la23[0], &la24[0],
    &la25[0], &la26[0], &la27[0], &la28[0], &la29[0], &la30[0], &la31[0], &la32[0],
    &la33[0], &la34[0], &la35[0], &la36[0], &la37[0], &la38[0], &la39[0], &la40[0],
    &orb01[0], &orb02[0], &orb03[0], &orb04[0], &orb05[0],
    &orb06[0], &orb07[0], &orb08[0], &orb09[0], &orb10[0],
    &swv01[0], &swv02[0], &swv03[0], &swv04[0], &swv05[0],
    &swv06[0], &swv07[0], &swv08[0], &swv09[0], &swv10[0],
    &swv11[0], &swv12[0], &swv13[0], &swv14[0], &swv15[0],
    &swv16[0], &swv17[0], &swv18[0], &swv19[0], &swv20[0],
    &yn1[0], &yn2[0], &yn3[0], &yn4[0],
    &tai_15_15_0[0], &tai_15_15_1[0], &tai_15_15_2[0], &tai_15_15_3[0],
    &tai_15_15_4[0], &tai_15_15_5[0], &tai_15_15_6[0], &tai_15_15_7[0],
    &tai_15_15_8[0], &tai_15_15_9[0],
    &tai_20_15_0[0], &tai_20_15_1[0], &tai_20_15_2[0], &tai_20_15_3[0],
    &tai_20_15_4[0], &tai_20_15_5[0], &tai_20_15_6[0], &tai_20_15_7[0],
    &tai_20_15_8[0], &tai_20_15_9[0],
    &tai_20_20_0[0], &tai_20_20_1[0], &tai_20_20_2[0], &tai_20_20_3[0],
    &tai_20_20_4[0], &tai_20_20_5[0], &tai_20_20_6[0], &tai_20_20_7[0],
    &tai_20_20_8[0], &tai_20_20_9[0],
    &tai_30_15_0[0], &tai_30_15_1[0], &tai_30_15_2[0], &tai_30_15_3[0],
    &tai_30_15_4[0], &tai_30_15_5[0], &tai_30_15_6[0], &tai_30_15_7[0],
    &tai_30_15_8[0], &tai_30_15_9[0],
    &tai_30_20_0[0], &tai_30_20_1[0], &tai_30_20_2[0], &tai_30_20_3[0],
    &tai_30_20_4[0], &tai_30_20_5[0], &tai_30_20_6[0], &tai_30_20_7[0],
    &tai_30_20_8[0], &tai_30_20_9[0],
    &tai_50_15_0[0], &tai_50_15_1[0], &tai_50_15_2[0], &tai_50_15_3[0],
    &tai_50_15_4[0], &tai_50_15_5[0], &tai_50_15_6[0], &tai_50_15_7[0],
    &tai_50_15_8[0], &tai_50_15_9[0],
    &tai_50_20_0[0], &tai_50_20_1[0], &tai_50_20_2[0], &tai_50_20_3[0],
    &tai_50_20_4[0], &tai_50_20_5[0], &tai_50_20_6[0], &tai_50_20_7[0],
    &tai_50_20_8[0], &tai_50_20_9[0],
    &tai_100_20_0[0], &tai_100_20_1[0], &tai_100_20_2[0], &tai_100_20_3[0],
    &tai_100_20_4[0], &tai_100_20_5[0], &tai_100_20_6[0], &tai_100_20_7[0],
    &tai_100_20_8[0], &tai_100_20_9[0]
  };

  const char* name[] = {
    "test",
    "abz5", "abz6", "abz7", "abz8", "abz9",
    "ft06", "ft10", "ft20",
    "la01", "la02", "la03", "la04", "la05", "la06", "la07", "la08",
    "la09", "la10", "la11", "la12", "la13", "la14", "la15", "la16",
    "la17", "la18", "la19", "la20", "la21", "la22", "la23", "la24",
    "la25", "la26", "la27", "la28", "la29", "la30", "la31", "la32",
    "la33", "la34", "la35", "la36", "la37", "la38", "la39", "la40",
    "orb01", "orb02", "orb03", "orb04", "orb05",
    "orb06", "orb07", "orb08", "orb09", "orb10",
    "swv01", "swv02", "swv03", "swv04", "swv05",
    "swv06", "swv07", "swv08", "swv09", "swv10",
    "swv11", "swv12", "swv13", "swv14", "swv15",
    "swv16", "swv17", "swv18", "swv19", "swv20",
    "yn1", "yn2", "yn3", "yn4", 
    "tai-15-15-0", "tai-15-15-1", "tai-15-15-2", "tai-15-15-3", "tai-15-15-4",
    "tai-15-15-5", "tai-15-15-6", "tai-15-15-7", "tai-15-15-8", "tai-15-15-9",
    "tai-20-15-0", "tai-20-15-1", "tai-20-15-2", "tai-20-15-3", "tai-20-15-4",
    "tai-20-15-5", "tai-20-15-6", "tai-20-15-7", "tai-20-15-8", "tai-20-15-9",
    "tai-20-20-0", "tai-20-20-1", "tai-20-20-2", "tai-20-20-3", "tai-20-20-4",
    "tai-20-20-5", "tai-20-20-6", "tai-20-20-7", "tai-20-20-8", "tai-20-20-9",
    "tai-30-15-0", "tai-30-15-1", "tai-30-15-2", "tai-30-15-3", "tai-30-15-4",
    "tai-30-15-5", "tai-30-15-6", "tai-30-15-7", "tai-30-15-8", "tai-30-15-9",
    "tai-30-20-0", "tai-30-20-1", "tai-30-20-2", "tai-30-20-3", "tai-30-20-4",
    "tai-30-20-5", "tai-30-20-6", "tai-30-20-7", "tai-30-20-8", "tai-30-20-9",
    "tai-50-15-0", "tai-50-15-1", "tai-50-15-2", "tai-50-15-3", "tai-50-15-4",
    "tai-50-15-5", "tai-50-15-6", "tai-50-15-7", "tai-50-15-8", "tai-50-15-9",
    "tai-50-20-0", "tai-50-20-1", "tai-50-20-2", "tai-50-20-3", "tai-50-20-4",
    "tai-50-20-5", "tai-50-20-6", "tai-50-20-7", "tai-50-20-8", "tai-50-20-9", 
    "tai-100-20-0", "tai-100-20-1", "tai-100-20-2", "tai-100-20-3",
    "tai-100-20-4", "tai-100-20-5", "tai-100-20-6", "tai-100-20-7",
    "tai-100-20-8", "tai-100-20-9",
    nullptr
  };

}

// STATISTICS: example-any

