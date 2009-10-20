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

#include <algorithm>
#include <map>
#include <vector>
#include <iostream>

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

#include <gecode/search.hh>

namespace Test { namespace Branch {

  /// Space for executing integer tests
  class IntTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::IntVarArray x;
    /// Variable selection criteria
    Gecode::IntVarBranch vara, varb;
    /// Varlue selection criterion
    Gecode::IntValBranch val;
    /// Initialize test space
    IntTestSpace(int n, Gecode::IntSet& d)
      : x(*this, n, d), 
        vara(Gecode::INT_VAR_NONE), varb(Gecode::INT_VAR_NONE),
        val(Gecode::INT_VAL_MIN) {}
    /// Constructor for cloning \a s
    IntTestSpace(bool share, IntTestSpace& s)
      : Gecode::Space(share,s), vara(s.vara), varb(s.varb), val(s.val) {
      x.update(*this, share, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(bool share) {
      return new IntTestSpace(share,*this);
    }
  };

  /// Space for executing Boolean tests
  class BoolTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::BoolVarArray x;
    /// Initialize test space
    BoolTestSpace(int n)
      : x(*this, n, 0, 1) {}
    /// Constructor for cloning \a s
    BoolTestSpace(bool share, BoolTestSpace& s)
      : Gecode::Space(share,s) {
      x.update(*this, share, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(bool share) {
      return new BoolTestSpace(share,*this);
    }
  };

#ifdef GECODE_HAS_SET_VARS
  /// Space for executing Boolean tests
  class SetTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::SetVarArray x;
    /// Initialize test space
    SetTestSpace(int n, Gecode::IntSet& d)
      : x(*this, n, Gecode::IntSet::empty, d) {}
    /// Constructor for cloning \a s
    SetTestSpace(bool share, SetTestSpace& s)
      : Gecode::Space(share,s) {
      x.update(*this, share, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(bool share) {
      return new SetTestSpace(share,*this);
    }
  };
#endif

  /** \name Collection of possible arguments for integer branchers
   *
   * \relates IntTestSpace BoolTestSpace
   */
  //@{
  /// Integer variable selections
  const Gecode::IntVarBranch int_var_branch[] = {
    Gecode::INT_VAR_NONE, // Use several single variable branchers
    Gecode::INT_VAR_NONE,
    Gecode::INT_VAR_RND,
    Gecode::INT_VAR_DEGREE_MIN,
    Gecode::INT_VAR_DEGREE_MAX,
    Gecode::INT_VAR_AFC_MIN,
    Gecode::INT_VAR_AFC_MAX,
    Gecode::INT_VAR_MIN_MIN,
    Gecode::INT_VAR_MIN_MAX,
    Gecode::INT_VAR_MAX_MIN,
    Gecode::INT_VAR_MAX_MAX,
    Gecode::INT_VAR_SIZE_MIN,
    Gecode::INT_VAR_SIZE_MAX,
    Gecode::INT_VAR_SIZE_DEGREE_MIN,
    Gecode::INT_VAR_SIZE_DEGREE_MAX,
    Gecode::INT_VAR_SIZE_AFC_MIN,
    Gecode::INT_VAR_SIZE_AFC_MAX,
    Gecode::INT_VAR_REGRET_MIN_MIN,
    Gecode::INT_VAR_REGRET_MIN_MAX,
    Gecode::INT_VAR_REGRET_MAX_MIN,
    Gecode::INT_VAR_REGRET_MAX_MAX
  };
  /// Number of integer variable selections
  const int n_int_var_branch =
    sizeof(int_var_branch)/sizeof(Gecode::IntVarBranch);
  /// Names for integer variable selections
  const char* int_var_branch_name[] = {
    "SINGLE VARIABLE",
    "INT_VAR_NONE",
    "INT_VAR_RND",
    "INT_VAR_DEGREE_MIN",
    "INT_VAR_DEGREE_MAX",
    "INT_VAR_AFC_MIN",
    "INT_VAR_AFC_MAX",
    "INT_VAR_MIN_MIN",
    "INT_VAR_MIN_MAX",
    "INT_VAR_MAX_MIN",
    "INT_VAR_MAX_MAX",
    "INT_VAR_SIZE_MIN",
    "INT_VAR_SIZE_MAX",
    "INT_VAR_SIZE_DEGREE_MIN",
    "INT_VAR_SIZE_DEGREE_MAX",
    "INT_VAR_SIZE_AFC_MIN",
    "INT_VAR_SIZE_AFC_MAX",
    "INT_VAR_REGRET_MIN_MIN",
    "INT_VAR_REGRET_MIN_MAX",
    "INT_VAR_REGRET_MAX_MIN",
    "INT_VAR_REGRET_MAX_MAX"
  };
  /// Integer value selections
  const Gecode::IntValBranch int_val_branch[] = {
    Gecode::INT_VAL_MIN,
    Gecode::INT_VAL_MED,
    Gecode::INT_VAL_MAX,
    Gecode::INT_VAL_RND,
    Gecode::INT_VAL_SPLIT_MIN,
    Gecode::INT_VAL_SPLIT_MAX,
    Gecode::INT_VAL_RANGE_MIN,
    Gecode::INT_VAL_RANGE_MAX,
    Gecode::INT_VALUES_MIN,
    Gecode::INT_VALUES_MAX
  };
  /// Number of integer value selections
  const int n_int_val_branch =
    sizeof(int_val_branch)/sizeof(Gecode::IntValBranch);
  /// Names for integer value selections
  const char* int_val_branch_name[] = {
    "INT_VAL_MIN",
    "INT_VAL_MED",
    "INT_VAL_MAX",
    "INT_VAL_RND",
    "INT_VAL_SPLIT_MIN",
    "INT_VAL_SPLIT_MAX",
    "INT_VAL_RANGE_MIN",
    "INT_VAL_RANGE_MAX",
    "INT_VALUES_MIN",
    "INT_VALUES_MAX"
  };
  //@}

#ifdef GECODE_HAS_SET_VARS
  /** \name Collection of possible arguments for set branchers
   *
   * \relates SetTestSpace
   */
  //@{
  /// Set variable selections
  const Gecode::SetVarBranch set_var_branch[] = {
    Gecode::SET_VAR_NONE, // Use several single variable branchers
    Gecode::SET_VAR_NONE,
    Gecode::SET_VAR_RND,
    Gecode::SET_VAR_DEGREE_MIN,
    Gecode::SET_VAR_DEGREE_MAX,
    Gecode::SET_VAR_AFC_MIN,
    Gecode::SET_VAR_AFC_MAX,
    Gecode::SET_VAR_MIN_MIN,
    Gecode::SET_VAR_MIN_MAX,
    Gecode::SET_VAR_MAX_MIN,
    Gecode::SET_VAR_MAX_MAX,
    Gecode::SET_VAR_SIZE_MIN,
    Gecode::SET_VAR_SIZE_MAX,
    Gecode::SET_VAR_SIZE_DEGREE_MIN,
    Gecode::SET_VAR_SIZE_DEGREE_MAX,
    Gecode::SET_VAR_SIZE_AFC_MIN,
    Gecode::SET_VAR_SIZE_AFC_MAX
  };
  /// Number of set variable selections
  const int n_set_var_branch =
    sizeof(set_var_branch)/sizeof(Gecode::SetVarBranch);
  /// Names for set variable selections
  const char* set_var_branch_name[] = {
    "SINGLE VARIABLE",
    "SET_VAR_NONE",
    "SET_VAR_RND",
    "SET_VAR_DEGREE_MIN",
    "SET_VAR_DEGREE_MAX",
    "SET_VAR_AFC_MIN",
    "SET_VAR_AFC_MAX",
    "SET_VAR_MIN_MIN",
    "SET_VAR_MIN_MAX",
    "SET_VAR_MAX_MIN",
    "SET_VAR_MAX_MAX",
    "SET_VAR_SIZE_MIN",
    "SET_VAR_SIZE_MAX",
    "SET_VAR_SIZE_DEGREE_MIN",
    "SET_VAR_SIZE_DEGREE_MAX",
    "SET_VAR_SIZE_AFC_MIN",
    "SET_VAR_SIZE_AFC_MAX"
  };
  /// Set value selections
  const Gecode::SetValBranch set_val_branch[] = {
    Gecode::SET_VAL_MIN_INC,
    Gecode::SET_VAL_MIN_EXC,
    Gecode::SET_VAL_MED_INC,
    Gecode::SET_VAL_MED_EXC,
    Gecode::SET_VAL_MAX_INC,
    Gecode::SET_VAL_MAX_EXC,
    Gecode::SET_VAL_RND_INC,
    Gecode::SET_VAL_RND_EXC
  };
  /// Number of set value selections
  const int n_set_val_branch =
    sizeof(set_val_branch)/sizeof(Gecode::SetValBranch);
  /// Names for set value selections
  const char* set_val_branch_name[] = {
    "SET_VAL_MIN_INC",
    "SET_VAL_MIN_EXC",
    "SET_VAL_MED_INC",
    "SET_VAL_MED_EXC",
    "SET_VAL_MAX_INC",
    "SET_VAL_MAX_EXC",
    "SET_VAL_RND_INC",
    "SET_VAL_RND_EXC"
  };
  //@}
#endif

  /// Information about one test-run
  class RunInfo {
  public:
    std::string var, val;
    unsigned int a_d, c_d;
    RunInfo(const std::string& vara, const std::string& varb,
            const std::string& valname,
            const Gecode::Search::Options& o)
      : var(vara + "::" + varb), val(valname), a_d(o.a_d), c_d(o.c_d) {}
    void print(std::ostream& o) const {
      o << "(" << var << ", " << val << ", " << a_d << ", " << c_d << ")";
    }
  };

}}

std::ostream&
operator<<(std::ostream& os, const Test::Branch::RunInfo& ri) {
  ri.print(os);
  return os;
}


namespace Test { namespace Branch {

  /// Find number of solutions
  template<class TestSpace>
  int solutions(TestSpace* c, Gecode::Search::Options& o) {
    o.a_d = Base::rand(10);
    o.c_d = Base::rand(10);
    Gecode::DFS<TestSpace> e_s(c, o);
    delete c;

    // Find number of solutions
    int s = 0;
    do {
      Gecode::Space* ex = e_s.next();
      if (ex == NULL) break;
      delete ex;
      ++s;
    } while (true);
    return s;
  }

  IntTest::IntTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Int::Branch::"+s), arity(a), dom(d) {
  }

  bool
  IntTest::run(void) {
    using std::map;
    using std::vector;
    using std::string;
    using std::ostream;
    using namespace Gecode;

    // Results of tests run
    map<int, vector<RunInfo> > results;
    // Set up root space
    IntTestSpace* root = new IntTestSpace(arity,dom);
    post(*root, root->x);
    results.clear();

    for (int vara = 0; vara<n_int_var_branch; vara++) {
      for (int varb = 1; varb<n_int_var_branch; varb++) {
        for (int val = 0; val<n_int_val_branch; val++) {
          IntTestSpace* c = static_cast<IntTestSpace*>(root->clone(false));
          if (vara == 0)
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], int_val_branch[val]);
          else
            branch(*c, c->x,
                   tiebreak(int_var_branch[vara], int_var_branch[varb]),
                   int_val_branch[val]);
          Gecode::Search::Options o;
          results[solutions(c,o)].push_back
            (RunInfo(int_var_branch_name[vara],
                     int_var_branch_name[varb],
                     int_val_branch_name[val],
                     o));
        }
      }
    }
    if (results.size() > 1)
      goto failed;
    delete root;
    return true;
  failed:
    std::cout   << "FAILURE" << std::endl;
    for (map<int, vector<RunInfo> >::iterator it = results.begin();
         it != results.end(); ++it) {
      std::cout << "Number of solutions: " << it->first << std::endl;
      for (unsigned int i = 0; i < it->second.size(); ++i)
        std::cout << it->second[i] << " ";
      std::cout << std::endl;
    }

    delete root;
    return results.size() == 1;
  }

  BoolTest::BoolTest(const std::string& s, int a)
    : Base("Bool::Branch::"+s), arity(a) {
  }

  bool
  BoolTest::run(void) {
    using std::map;
    using std::vector;
    using std::string;
    using std::ostream;
    using namespace Gecode;

    // Results of tests run
    map<int, vector<RunInfo> > results;
    // Set up root space
    BoolTestSpace* root = new BoolTestSpace(arity);
    post(*root, root->x);
    results.clear();

    for (int vara = 0; vara<n_int_var_branch; vara++) {
      for (int varb = 1; varb<n_int_var_branch; varb++) {
        for (int val = 0; val<n_int_val_branch; val++) {
          BoolTestSpace* c = static_cast<BoolTestSpace*>(root->clone(false));
          if (vara == 0)
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], int_val_branch[val]);
          else
            branch(*c, c->x,
                   tiebreak(int_var_branch[vara], int_var_branch[varb]),
                   int_val_branch[val]);
          Gecode::Search::Options o;
          results[solutions(c,o)].push_back
            (RunInfo(int_var_branch_name[vara],
                     int_var_branch_name[varb],
                     int_val_branch_name[val],
                     o));
        }
      }
    }
    if (results.size() > 1)
      goto failed;
    delete root;
    return true;
  failed:
    std::cout   << "FAILURE" << std::endl;
    for (map<int, vector<RunInfo> >::iterator it = results.begin();
         it != results.end(); ++it) {
      std::cout << "Number of solutions: " << it->first << std::endl;
      for (unsigned int i = 0; i < it->second.size(); ++i)
        std::cout << it->second[i] << " ";
      std::cout << std::endl;
    }

    delete root;
    return results.size() == 1;
  }

#ifdef GECODE_HAS_SET_VARS
  SetTest::SetTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Set::Branch::"+s), arity(a), dom(d) {
  }

  bool
  SetTest::run(void) {
    using std::map;
    using std::vector;
    using std::string;
    using std::ostream;
    using namespace Gecode;

    // Results of tests run
    map<int, vector<RunInfo> > results;
    // Set up root space
    SetTestSpace* root = new SetTestSpace(arity,dom);
    post(*root, root->x);
    root->status();
    results.clear();

    for (int vara = 0; vara<n_set_var_branch; vara++) {
      for (int varb = 1; varb<n_set_var_branch; varb++) {
        for (int val = 0; val<n_set_val_branch; val++) {
          SetTestSpace* c = static_cast<SetTestSpace*>(root->clone(false));
          if (vara == 0)
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], set_val_branch[val]);
          else
            branch(*c, c->x,
                   tiebreak(set_var_branch[vara], set_var_branch[varb]),
                   set_val_branch[val]);
          Gecode::Search::Options o;
          results[solutions(c,o)].push_back
            (RunInfo(set_var_branch_name[vara],
                     set_var_branch_name[varb],
                     set_val_branch_name[val],
                     o));
        }
      }
    }
    if (results.size() > 1)
      goto failed;
    delete root;
    return true;
  failed:
    std::cout   << "FAILURE" << std::endl;
    for (map<int, vector<RunInfo> >::iterator it = results.begin();
         it != results.end(); ++it) {
      std::cout << "Number of solutions: " << it->first << std::endl;
      for (unsigned int i = 0; i < it->second.size(); ++i)
        std::cout << it->second[i] << " ";
      std::cout << std::endl;
    }

    delete root;
    return results.size() == 1;
  }
#endif

}}

// STATISTICS: test-branch
