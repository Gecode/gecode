/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/branch.hh"
#include "test/log.hh"

#include <algorithm>
#include <map>
#include <vector>
#include <iostream>

#include "gecode/kernel.hh"
//#include "gecode/set.hh"
#include "gecode/search.hh"


using std::map;
using std::vector;
using std::string;
using std::ostream;



class BranchTestSpace : public Space {
public:
  IntVarArray x;
  //SetVarArray y;
private:
  const Options opt;

public:
  BranchTestSpace(int n, IntSet& d, const Options& o)
    : x(this, n, d), opt(o) {
    Log::initial(x, "x");
  }
  BranchTestSpace(bool share, BranchTestSpace& s) : Space(share,s), opt(s.opt) {
    x.update(this, share, s.x);
  }
  virtual Space* copy(bool share) {
    return new BranchTestSpace(share,*this);
  }
};


namespace {
  /** \name Collection of possible arguments for branchings
   *
   * \relates IntTest
   */
  //@{
  /// Integer variable selections
  const BvarSel bvarsel[] = {
    BVAR_NONE,
    BVAR_MIN_MIN,       
    BVAR_MIN_MAX,
    BVAR_MAX_MIN,
    BVAR_MAX_MAX,
    BVAR_SIZE_MIN,
    BVAR_SIZE_MAX,
    BVAR_DEGREE_MIN,
    BVAR_DEGREE_MAX,
    BVAR_REGRET_MIN_MIN,
    BVAR_REGRET_MIN_MAX/*,
                         // These two crash the system
    BVAR_REGRET_MAX_MIN,
    BVAR_REGRET_MAX_MAX*/};
  /// Number of integer variable selections
  const int nbvarsel = sizeof(bvarsel)/sizeof(BvarSel);
  /// Names for integer variable selections
  const char* bvarsel_name[] = {
    "BVAR_NONE",
    "BVAR_MIN_MIN",       
    "BVAR_MIN_MAX",
    "BVAR_MAX_MIN",
    "BVAR_MAX_MAX",
    "BVAR_SIZE_MIN",
    "BVAR_SIZE_MAX",
    "BVAR_DEGREE_MIN",
    "BVAR_DEGREE_MAX",
    "BVAR_REGRET_MIN_MIN",
    "BVAR_REGRET_MIN_MAX",
    "BVAR_REGRET_MAX_MIN",
    "BVAR_REGRET_MAX_MAX"};
  /// Integer value selections
  const BvalSel bvalsel[] = {
    BVAL_MIN,
    BVAL_MED,
    BVAL_MAX,
    BVAL_SPLIT_MIN,
    BVAL_SPLIT_MAX};
  /// Number of integer value selections
  const int nbvalsel = sizeof(bvalsel)/sizeof(BvalSel);
  /// Names for integer value selections
  const char* bvalsel_name[] = {
    "BVAL_MIN",
    "BVAL_MED",
    "BVAL_MAX",
    "BVAL_SPLIT_MIN",
    "BVAL_SPLIT_MAX"};
  //@}
  /// Information about one test-run
  struct RunInfo {
    string var, val;
    int a_c, c_d;
    RunInfo(const char* varname, const char* valname,
            int a_c_val, int c_d_val)
      : var(varname), val(valname), a_c(a_c_val), c_d(c_d_val) {}

    void print(ostream& o) const {
      o << "(" << var << ", " << val << ", " << a_c << ", " << c_d << ")";
    }
  };
  ostream&
  operator<<(ostream& os, const RunInfo& ri) {
    ri.print(os);
    return os;
  }

}

bool
BranchCompleteTest::run(const Options& opt) {
  // Search used
  const char* search    = "NONE";
  // Results of tests run
  map<int, vector<RunInfo> > results;
  // Set up root space
  BranchTestSpace* root = new BranchTestSpace(arity,dom,opt);
  post(root, root->x);
  // Test with DFS-search
  search = "DFS";
  results.clear();
  for (int var = nbvarsel; var--; ) {
    for (int val = nbvalsel; val--; ) {
      BranchTestSpace* clone = static_cast<BranchTestSpace*>(root->clone(false));
      int a_c = Test::randgen(10),
        c_d = Test::randgen(10);
      branch(clone, clone->x, bvarsel[var], bvalsel[val]);
      Gecode::DFS<BranchTestSpace> e_s(clone, a_c, c_d);
      delete clone;

      // Find number of solutions
      int solutions = 0;
      do {
        Space *ex = e_s.next();
        if (ex == NULL) break;
        delete ex;
        ++solutions;
      } while(true);
      results[solutions].push_back(RunInfo(bvarsel_name[var], bvalsel_name[val],
                                           a_c, c_d));
    }
  }
  if (results.size() > 1) goto failed;
  return true;
 failed:
  std::cout   << "FAILURE" << std::endl;
  std::cout   << "Search method:       " << search << std::endl;
  for (map<int, vector<RunInfo> >::iterator it = results.begin();
       it != results.end(); ++it) {
    std::cout << "Number of solutions: " << it->first << std::endl;
    for (unsigned int i = 0; i < it->second.size(); ++i)
      std::cout << it->second[i] << " ";
    std::cout << std::endl;
  }

  return results.size() == 1;
}

// STATISTICS: test-branch
