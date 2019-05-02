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
 * INFORMS Journal of Computing, Volume 27, Issue 2, 2015.
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

    l = master->cost().min();
    u = master->cost().max();

    FailTimeStop fts(opt.fail_probe(),opt.time_probe());
    CommonOptions so(opt);
    so.stop = &fts;
    bool stopped = false;

    std::cout << "\tProbing..." << std::endl;
    
    std::cout << "\t\tBounds: [" << l << "," << u << "]"
              << std::endl;
    
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

  // Dichotomic search
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
      if (opt.time_adjust() > 0U)
        so.stop = Search::Stop::time(opt.time_adjust());
      bool stopped = false;
      while (l < u) {
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

    if (l == u) {
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
      if (opt.time_solve() > 0U)
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

#include "examples/job-shop-instances.hpp"

// STATISTICS: example-any

