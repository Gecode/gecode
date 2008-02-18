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

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/set.hh"

#include "gecode/search.hh"

namespace Test { namespace Branch {

  /// Space for executing integer tests
  class IntTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::IntVarArray x;
    /// Initialize test space
    IntTestSpace(int n, Gecode::IntSet& d)
      : x(this, n, d) {}
    /// Constructor for cloning \a s
    IntTestSpace(bool share, IntTestSpace& s) 
      : Gecode::Space(share,s) {
      x.update(this, share, s.x);
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
      : x(this, n, 0, 1) {}
    /// Constructor for cloning \a s
    BoolTestSpace(bool share, BoolTestSpace& s) 
      : Gecode::Space(share,s) {
      x.update(this, share, s.x);
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
      : x(this, n, Gecode::IntSet::empty, d) {}
    /// Constructor for cloning \a s
    SetTestSpace(bool share, SetTestSpace& s) 
      : Gecode::Space(share,s) {
      x.update(this, share, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(bool share) {
      return new SetTestSpace(share,*this);
    }
  };
#endif

#ifdef GECODE_HAS_CPLTSET_VARS
  /// Space for executing Boolean tests
  class CpltSetTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::CpltSetVarArray x;
    /// Initialize test space
    CpltSetTestSpace(int n, Gecode::IntSet& d)
      : x(this, n, Gecode::IntSet::empty, d) {}
    /// Constructor for cloning \a s
    CpltSetTestSpace(bool share, CpltSetTestSpace& s) 
      : Gecode::Space(share,s) {
      x.update(this, share, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(bool share) {
      return new CpltSetTestSpace(share,*this);
    }
  };
#endif

  /** \name Collection of possible arguments for integer branchings
   *
   * \relates IntTestSpace BoolTestSpace
   */
  //@{
  /// Integer variable selections
  const Gecode::IntVarBranch int_var_branch[] = {
    Gecode::INT_VAR_NONE,
    Gecode::INT_VAR_MIN_MIN,       
    Gecode::INT_VAR_MIN_MAX,
    Gecode::INT_VAR_MAX_MIN,
    Gecode::INT_VAR_MAX_MAX,
    Gecode::INT_VAR_SIZE_MIN,
    Gecode::INT_VAR_SIZE_MAX,
    Gecode::INT_VAR_DEGREE_MIN,
    Gecode::INT_VAR_DEGREE_MAX,
    Gecode::INT_VAR_SIZE_DEGREE_MIN,
    Gecode::INT_VAR_SIZE_DEGREE_MAX,
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
    "INT_VAR_NONE",
    "INT_VAR_MIN_MIN",       
    "INT_VAR_MIN_MAX",
    "INT_VAR_MAX_MIN",
    "INT_VAR_MAX_MAX",
    "INT_VAR_SIZE_MIN",
    "INT_VAR_SIZE_MAX",
    "INT_VAR_DEGREE_MIN",
    "INT_VAR_DEGREE_MAX",
    "INT_VAR_SIZE_DEGREE_MIN",
    "INT_VAR_SIZE_DEGREE_MAX",
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
    Gecode::INT_VAL_SPLIT_MIN,
    Gecode::INT_VAL_SPLIT_MAX
  };
  /// Number of integer value selections
  const int n_int_val_branch =
    sizeof(int_val_branch)/sizeof(Gecode::IntValBranch);
  /// Names for integer value selections
  const char* int_val_branch_name[] = {
    "INT_VAL_MIN",
    "INT_VAL_MED",
    "INT_VAL_MAX",
    "INT_VAL_SPLIT_MIN",
    "INT_VAL_SPLIT_MAX"
  };
  //@}

#ifdef GECODE_HAS_SET_VARS
  /** \name Collection of possible arguments for set branchings
   *
   * \relates SetTestSpace
   */
  //@{
  /// Set variable selections
  const Gecode::SetVarBranch set_var_branch[] = {
    Gecode::SET_VAR_NONE,
    Gecode::SET_VAR_MIN_CARD,
    Gecode::SET_VAR_MAX_CARD,
    Gecode::SET_VAR_MIN_UNKNOWN_ELEM,
    Gecode::SET_VAR_MAX_UNKNOWN_ELEM
  };
  /// Number of set variable selections
  const int n_set_var_branch = 
    sizeof(set_var_branch)/sizeof(Gecode::SetVarBranch);
  /// Names for set variable selections
  const char* set_var_branch_name[] = {
    "SET_VAR_NONE",
    "SET_VAR_MIN_CARD",
    "SET_VAR_MAX_CARD",
    "SET_VAR_MIN_UNKNOWN_ELEM",
    "SET_VAR_MAX_UNKNOWN_ELEM"
  };
  /// Set value selections
  const Gecode::SetValBranch set_val_branch[] = {
    Gecode::SET_VAL_MIN,
    Gecode::SET_VAL_MAX
  };
  /// Number of set value selections
  const int n_set_val_branch =
    sizeof(set_val_branch)/sizeof(Gecode::SetValBranch);
  /// Names for set value selections
  const char* set_val_branch_name[] = {
    "SET_VAL_MIN",
    "SET_VAL_MAX"
  };
  //@}
#endif

#ifdef GECODE_HAS_CPLTSET_VARS
  /** \name Collection of possible arguments for CpltSet branchings
   *
   * \relates CpltSetTestSpace
   */
  //@{
  /// CpltSet variable selections
  const Gecode::CpltSetVarBranch cpltset_var_branch[] = {
    Gecode::CPLTSET_VAR_NONE,
    Gecode::CPLTSET_VAR_MIN_CARD,
    Gecode::CPLTSET_VAR_MAX_CARD,
    Gecode::CPLTSET_VAR_MIN_UNKNOWN_ELEM,
    Gecode::CPLTSET_VAR_MAX_UNKNOWN_ELEM
  };
  /// Number of CpltSet variable selections
  const int n_cpltset_var_branch = 
    sizeof(cpltset_var_branch)/sizeof(Gecode::CpltSetVarBranch);
  /// Names for CpltSet variable selections
  const char* cpltset_var_branch_name[] = {
    "CPLTSET_VAR_NONE",
    "CPLTSET_VAR_MIN_CARD",
    "CPLTSET_VAR_MAX_CARD",
    "CPLTSET_VAR_MIN_UNKNOWN_ELEM",
    "CPLTSET_VAR_MAX_UNKNOWN_ELEM"
  };
  /// Set value selections
  const Gecode::CpltSetValBranch cpltset_val_branch[] = {
    Gecode::CPLTSET_VAL_MIN_UNKNOWN,
    Gecode::CPLTSET_VAL_MAX_UNKNOWN,
    Gecode::CPLTSET_VAL_MIN_UNKNOWN_EX_FIRST,
    Gecode::CPLTSET_VAL_MAX_UNKNOWN_EX_FIRST
  };
  /// Number of set value selections
  const int n_cpltset_val_branch =
    sizeof(cpltset_val_branch)/sizeof(Gecode::CpltSetValBranch);
  /// Names for set value selections
  const char* cpltset_val_branch_name[] = {
    "CPLTSET_VAL_MIN_UNKNOWN",
    "CPLTSET_VAL_MAX_UNKNOWN",
    "CPLTSET_VAL_MIN_UNKNOWN_EX_FIRST",
    "CPLTSET_VAL_MAX_UNKNOWN_EX_FIRST"
  };
  //@}
#endif

  /// Information about one test-run
  class RunInfo {
  public:
    std::string var, val;
    int a_c, c_d;
    RunInfo(const char* varname, const char* valname,
            int a_c_val, int c_d_val)
      : var(varname), val(valname), a_c(a_c_val), c_d(c_d_val) {}
    void print(std::ostream& o) const {
      o << "(" << var << ", " << val << ", " << a_c << ", " << c_d << ")";
    }
  };

}}

std::ostream&
operator<<(std::ostream& os, const Test::Branch::RunInfo& ri) {
  ri.print(os);
  return os;
}


namespace Test { namespace Branch {


  IntTest::IntTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Branch::Int::"+s), arity(a), dom(d) {
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
    post(root, root->x);
    results.clear();

    for (int var = n_int_var_branch; var--; ) {
      for (int val = n_int_val_branch; val--; ) {
        IntTestSpace* clone = static_cast<IntTestSpace*>(root->clone(false));
        int a_c = Base::rand(10);
        int c_d = Base::rand(10);
        branch(clone, clone->x, int_var_branch[var], int_val_branch[val]);
        Gecode::DFS<IntTestSpace> e_s(clone, a_c, c_d);
        delete clone;
        
        // Find number of solutions
        int solutions = 0;
        do {
          Space *ex = e_s.next();
          if (ex == NULL) break;
          delete ex;
          ++solutions;
        } while(true);
        results[solutions].push_back(RunInfo(int_var_branch_name[var], 
                                             int_val_branch_name[val],
                                             a_c, c_d));
      }
    }
    if (results.size() > 1) 
      goto failed;
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
    
    return results.size() == 1;
  }

  BoolTest::BoolTest(const std::string& s, int a)
    : Base("Branch::Bool::"+s), arity(a) {
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
    post(root, root->x);
    results.clear();

    for (int var = n_int_var_branch; var--; ) {
      for (int val = n_int_val_branch; val--; ) {
        BoolTestSpace* clone = 
          static_cast<BoolTestSpace*>(root->clone(false));
        int a_c = Base::rand(10);
        int c_d = Base::rand(10);
        branch(clone, clone->x, int_var_branch[var], int_val_branch[val]);
        Gecode::DFS<BoolTestSpace> e_s(clone, a_c, c_d);
        delete clone;
        
        // Find number of solutions
        int solutions = 0;
        do {
          Space *ex = e_s.next();
          if (ex == NULL) break;
          delete ex;
          ++solutions;
        } while(true);
        results[solutions].push_back(RunInfo(int_var_branch_name[var], 
                                             int_val_branch_name[val],
                                             a_c, c_d));
      }
    }
    if (results.size() > 1) 
      goto failed;
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
    
    return results.size() == 1;
  }

#ifdef GECODE_HAS_SET_VARS
  SetTest::SetTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Branch::Set::"+s), arity(a), dom(d) {
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
    post(root, root->x);
    root->status();
    results.clear();

    for (int var = n_set_var_branch; var--; ) {
      for (int val = n_set_val_branch; val--; ) {
        SetTestSpace* clone = static_cast<SetTestSpace*>(root->clone(false));
        int a_c = Base::rand(10);
        int c_d = Base::rand(10);
        branch(clone, clone->x, set_var_branch[var], set_val_branch[val]);
        Gecode::DFS<SetTestSpace> e_s(clone, a_c, c_d);
        delete clone;
        
        // Find number of solutions
        int solutions = 0;
        do {
          Space *ex = e_s.next();
          if (ex == NULL) break;
          delete ex;
          ++solutions;
        } while(true);
        results[solutions].push_back(RunInfo(set_var_branch_name[var], 
                                             set_val_branch_name[val],
                                             a_c, c_d));
      }
    }
    if (results.size() > 1) 
      goto failed;
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
    
    return results.size() == 1;
  }
#endif

#ifdef GECODE_HAS_CPLTSET_VARS
  CpltSetTest::CpltSetTest(const std::string& s, int a,
                           const Gecode::IntSet& d)
    : Base("Branch::CpltSet::"+s), arity(a), dom(d) {
  }

  bool
  CpltSetTest::run(void) {
    using std::map;
    using std::vector;
    using std::string;
    using std::ostream;
    using namespace Gecode;
    
    // Results of tests run
    map<int, vector<RunInfo> > results;
    // Set up root space
    CpltSetTestSpace* root = new CpltSetTestSpace(arity,dom);
    post(root, root->x);
    root->status();
    results.clear();

    for (int var = n_cpltset_var_branch; var--; ) {
      for (int val = n_cpltset_val_branch; val--; ) {
        CpltSetTestSpace* clone = 
          static_cast<CpltSetTestSpace*>(root->clone(false));
        int a_c = Base::rand(10);
        int c_d = Base::rand(10);
        branch(clone, clone->x, cpltset_var_branch[var], 
               cpltset_val_branch[val]);
        Gecode::DFS<CpltSetTestSpace> e_s(clone, a_c, c_d);
        delete clone;
        
        // Find number of solutions
        int solutions = 0;
        do {
          Space *ex = e_s.next();
          if (ex == NULL) break;
          delete ex;
          ++solutions;
        } while(true);
        results[solutions].push_back(RunInfo(cpltset_var_branch_name[var], 
                                             cpltset_val_branch_name[val],
                                             a_c, c_d));
      }
    }
    if (results.size() > 1) 
      goto failed;
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
    
    return results.size() == 1;
  }
#endif

}}

// STATISTICS: test-branch
