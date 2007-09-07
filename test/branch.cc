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

#include "test/branch.hh"
#include "test/log.hh"

#include <algorithm>
#include <map>
#include <vector>
#include <iostream>

#include "gecode/kernel.hh"
//#include "gecode/set.hh"
#include "gecode/search.hh"

namespace Test {

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
  const IntVarBranch bvarsel[] = {
    INT_VAR_NONE,
    INT_VAR_MIN_MIN,       
    INT_VAR_MIN_MAX,
    INT_VAR_MAX_MIN,
    INT_VAR_MAX_MAX,
    INT_VAR_SIZE_MIN,
    INT_VAR_SIZE_MAX,
    INT_VAR_DEGREE_MIN,
    INT_VAR_DEGREE_MAX,
    INT_VAR_REGRET_MIN_MIN,
    INT_VAR_REGRET_MIN_MAX,
    INT_VAR_REGRET_MAX_MIN,
    INT_VAR_REGRET_MAX_MAX
  };
  /// Number of integer variable selections
  const int nbvarsel = sizeof(bvarsel)/sizeof(IntVarBranch);
  /// Names for integer variable selections
  const char* bvarsel_name[] = {
    "INT_VAR_NONE",
    "INT_VAR_MIN_MIN",       
    "INT_VAR_MIN_MAX",
    "INT_VAR_MAX_MIN",
    "INT_VAR_MAX_MAX",
    "INT_VAR_SIZE_MIN",
    "INT_VAR_SIZE_MAX",
    "INT_VAR_DEGREE_MIN",
    "INT_VAR_DEGREE_MAX",
    "INT_VAR_REGRET_MIN_MIN",
    "INT_VAR_REGRET_MIN_MAX",
    "INT_VAR_REGRET_MAX_MIN",
    "INT_VAR_REGRET_MAX_MAX"
  };
  /// Integer value selections
  const IntValBranch bvalsel[] = {
    INT_VAL_MIN,
    INT_VAL_MED,
    INT_VAL_MAX,
    INT_VAL_SPLIT_MIN,
    INT_VAL_SPLIT_MAX};
  /// Number of integer value selections
  const int nbvalsel = sizeof(bvalsel)/sizeof(IntValBranch);
  /// Names for integer value selections
  const char* bvalsel_name[] = {
    "INT_VAL_MIN",
    "INT_VAL_MED",
    "INT_VAL_MAX",
    "INT_VAL_SPLIT_MIN",
    "INT_VAL_SPLIT_MAX"};
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
      int 
        a_c = TestBase::rand(10),
        c_d = TestBase::rand(10);
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

}

// STATISTICS: test-branch
