/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *     Christian Schulte, 2009
 *     Vincent Barichard, 2012
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
#ifdef GECODE_HAS_FLOAT_VARS
#include <gecode/float.hh>
#endif

#include <gecode/search.hh>

namespace Test { namespace Branch {

  /// Test function for tie-break limit function
  double tbl(const Gecode::Space&, double w, double b) {
    return (w + (b-w)/2.0);
  }

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
        vara(Gecode::INT_VAR_NONE()), varb(Gecode::INT_VAR_NONE()),
        val(Gecode::INT_VAL_MIN()) {}
    /// Constructor for cloning \a s
    IntTestSpace(IntTestSpace& s)
      : Gecode::Space(s), vara(s.vara), varb(s.varb), val(s.val) {
      x.update(*this, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(void) {
      return new IntTestSpace(*this);
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
    BoolTestSpace(BoolTestSpace& s)
      : Gecode::Space(s) {
      x.update(*this, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(void) {
      return new BoolTestSpace(*this);
    }
  };

#ifdef GECODE_HAS_SET_VARS
  /// Space for executing Set tests
  class SetTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::SetVarArray x;
    /// Initialize test space
    SetTestSpace(int n, Gecode::IntSet& d)
      : x(*this, n, Gecode::IntSet::empty, d) {}
    /// Constructor for cloning \a s
    SetTestSpace(SetTestSpace& s)
      : Gecode::Space(s) {
      x.update(*this, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(void) {
      return new SetTestSpace(*this);
    }
  };
#endif

#ifdef GECODE_HAS_FLOAT_VARS
  /// Space for executing Float tests
  class FloatTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::FloatVarArray x;
    /// Initialize test space
    FloatTestSpace(int n, Gecode::FloatVal& d)
      : x(*this, n, d.min(), d.max()) {}
    /// Constructor for cloning \a s
    FloatTestSpace(FloatTestSpace& s)
      : Gecode::Space(s) {
      x.update(*this, s.x);
    }
    /// Copy space during cloning
    virtual Gecode::Space* copy(void) {
      return new FloatTestSpace(*this);
    }
  };
#endif

  /** \name Collection of possible arguments for integer branchers
   *
   * \relates IntTestSpace BoolTestSpace
   */
  //@{
  /// Names for integer variable selections
  const char* int_var_branch_name[] = {
    "SINGLE VARIABLE",
    "INT_VAR_NONE",
    "INT_VAR_RND",
    "INT_VAR_MERIT_MIN",
    "INT_VAR_MERIT_MAX",
    "INT_VAR_DEGREE_MIN",
    "INT_VAR_DEGREE_MAX",
    "INT_VAR_AFC_MIN",
    "INT_VAR_AFC_MAX",
    "INT_VAR_ACTION_MIN",
    "INT_VAR_ACTION_MAX",
    "INT_VAR_CHB_MIN",
    "INT_VAR_CHB_MAX",
    "INT_VAR_MIN_MIN",
    "INT_VAR_MIN_MAX",
    "INT_VAR_MAX_MIN",
    "INT_VAR_MAX_MAX",
    "INT_VAR_SIZE_MIN",
    "INT_VAR_SIZE_MAX",
    "INT_VAR_DEGREE_SIZE_MIN",
    "INT_VAR_DEGREE_SIZE_MAX",
    "INT_VAR_AFC_SIZE_MIN",
    "INT_VAR_AFC_SIZE_MAX",
    "INT_VAR_ACTION_SIZE_MIN",
    "INT_VAR_ACTION_SIZE_MAX",
    "INT_VAR_CHB_SIZE_MIN",
    "INT_VAR_CHB_SIZE_MAX",
    "INT_VAR_REGRET_MIN_MIN",
    "INT_VAR_REGRET_MIN_MAX",
    "INT_VAR_REGRET_MAX_MIN",
    "INT_VAR_REGRET_MAX_MAX"
  };
  /// Number of integer variable selections
  const int n_int_var_branch =
    sizeof(int_var_branch_name)/sizeof(const char*);
  /// Names for Boolean variable selections
  const char* bool_var_branch_name[] = {
    "SINGLE VARIABLE",
    "BOOL_VAR_NONE",
    "BOOL_VAR_RND",
    "BOOL_VAR_MERIT_MIN",
    "BOOL_VAR_MERIT_MAX",
    "BOOL_VAR_DEGREE_MIN",
    "BOOL_VAR_DEGREE_MAX",
    "BOOL_VAR_AFC_MIN",
    "BOOL_VAR_AFC_MAX",
    "BOOL_VAR_ACTION_MIN",
    "BOOL_VAR_ACTION_MAX",
    "BOOL_VAR_CHB_MIN",
    "BOOL_VAR_CHB_MAX"
  };
  /// Number of integer variable selections
  const int n_bool_var_branch =
    sizeof(bool_var_branch_name)/sizeof(const char*);
  /// Test function for branch merit function
  double int_merit(const Gecode::Space&, Gecode::IntVar x, int) {
    return x.min();
  }
  /// Test function for branch merit function
  double bool_merit(const Gecode::Space&, Gecode::BoolVar x, int) {
    return x.min();
  }
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
    "INT_VAL",
    "INT_VALUES_MIN",
    "INT_VALUES_MAX"
  };
  /// Number of integer value selections
  const int n_int_val_branch =
    sizeof(int_val_branch_name)/sizeof(const char*);
  /// Names for Boolean value selections
  const char* bool_val_branch_name[] = {
    "BOOL_VAL_MIN",
    "BOOL_VAL_MAX",
    "BOOL_VAL_RND",
    "BOOL_VAL"
  };
  /// Number of Boolean value selections
  const int n_bool_val_branch =
    sizeof(bool_val_branch_name)/sizeof(const char*);
  /// Test function for branch value function
  int int_val(const Gecode::Space&, Gecode::IntVar x, int) {
    return x.min();
  }
  /// Test function for branch value function
  int bool_val(const Gecode::Space&, Gecode::BoolVar x, int) {
    return x.min();
  }
  //@}

#ifdef GECODE_HAS_SET_VARS
  /** \name Collection of possible arguments for set branchers
   *
   * \relates SetTestSpace
   */
  //@{
  /// Names for set variable selections
  const char* set_var_branch_name[] = {
    "SINGLE VARIABLE",
    "SET_VAR_NONE",
    "SET_VAR_RND",
    "SET_VAR_MERIT_MIN",
    "SET_VAR_MERIT_MAX",
    "SET_VAR_DEGREE_MIN",
    "SET_VAR_DEGREE_MAX",
    "SET_VAR_AFC_MIN",
    "SET_VAR_AFC_MAX",
    "SET_VAR_ACTION_MIN",
    "SET_VAR_ACTION_MAX",
    "SET_VAR_CHB_MIN",
    "SET_VAR_CHB_MAX",
    "SET_VAR_MIN_MIN",
    "SET_VAR_MIN_MAX",
    "SET_VAR_MAX_MIN",
    "SET_VAR_MAX_MAX",
    "SET_VAR_SIZE_MIN",
    "SET_VAR_SIZE_MAX",
    "SET_VAR_DEGREE_SIZE_MIN",
    "SET_VAR_DEGREE_SIZE_MAX",
    "SET_VAR_AFC_SIZE_MIN",
    "SET_VAR_AFC_SIZE_MAX",
    "SET_VAR_ACTION_SIZE_MIN",
    "SET_VAR_ACTION_SIZE_MAX",
    "SET_VAR_CHB_SIZE_MIN",
    "SET_VAR_CHB_SIZE_MAX"
  };
  /// Number of set variable selections
  const int n_set_var_branch =
    sizeof(set_var_branch_name)/sizeof(const char*);
  /// Test function for branch merit function
  double set_merit(const Gecode::Space&, Gecode::SetVar, int) {
    return 2.0;
  }
  /// Names for set value selections
  const char* set_val_branch_name[] = {
    "SET_VAL_MIN_INC",
    "SET_VAL_MIN_EXC",
    "SET_VAL_MED_INC",
    "SET_VAL_MED_EXC",
    "SET_VAL_MAX_INC",
    "SET_VAL_MAX_EXC",
    "SET_VAL_RND_INC",
    "SET_VAL_RND_EXC",
    "SET_VAL"
  };
  /// Number of set value selections
  const int n_set_val_branch =
    sizeof(set_val_branch_name)/sizeof(const char*);
  /// Test function for branch value function
  int set_val(const Gecode::Space&, Gecode::SetVar x, int) {
    Gecode::SetVarUnknownRanges r(x);
    return r.min();
  }
  //@}
#endif

#ifdef GECODE_HAS_FLOAT_VARS
  /** \name Collection of possible arguments for float branchers
   *
   * \relates FloatTestSpace
   */
  //@{
  /// Names for float variable selections
  const char* float_var_branch_name[] = {
    "SINGLE VARIABLE",
    "FLOAT_VAR_NONE",
    "FLOAT_VAR_RND",
    "FLOAT_VAR_MERIT_MIN",
    "FLOAT_VAR_MERIT_MAX",
    "FLOAT_VAR_DEGREE_MIN",
    "FLOAT_VAR_DEGREE_MAX",
    "FLOAT_VAR_AFC_MIN",
    "FLOAT_VAR_AFC_MAX",
    "FLOAT_VAR_ACTION_MIN",
    "FLOAT_VAR_ACTION_MAX",
    "FLOAT_VAR_CHB_MIN",
    "FLOAT_VAR_CHB_MAX",
    "FLOAT_VAR_MIN_MIN",
    "FLOAT_VAR_MIN_MAX",
    "FLOAT_VAR_MAX_MIN",
    "FLOAT_VAR_MAX_MAX",
    "FLOAT_VAR_SIZE_MIN",
    "FLOAT_VAR_SIZE_MAX",
    "FLOAT_VAR_DEGREE_SIZE_MIN",
    "FLOAT_VAR_DEGREE_SIZE_MAX",
    "FLOAT_VAR_AFC_SIZE_MIN",
    "FLOAT_VAR_AFC_SIZE_MAX",
    "FLOAT_VAR_ACTION_SIZE_MIN",
    "FLOAT_VAR_ACTION_SIZE_MAX",
    "FLOAT_VAR_CHB_SIZE_MIN",
    "FLOAT_VAR_CHB_SIZE_MAX"
  };
  /// Number of float variable selections
  const int n_float_var_branch =
    sizeof(float_var_branch_name)/sizeof(const char*);
  /// Test function for branch merit function
  double float_merit(const Gecode::Space&, Gecode::FloatVar x, int) {
    return static_cast<double>(x.degree());
  }
  /// Names for float value selections
  const char* float_val_branch_name[] = {
    "FLOAT_VAL_SPLIT_MIN",
    "FLOAT_VAL_SPLIT_MAX",
    "FLOAT_VAL_SPLIT_RND",
    "FLOAT_VAL"
  };
  /// Number of float value selections
  const int n_float_val_branch =
    sizeof(float_val_branch_name)/sizeof(const char*);
  /// Test function for branch value function
  Gecode::FloatNumBranch float_val(const Gecode::Space&,
                                   Gecode::FloatVar x, int) {
    Gecode::FloatNumBranch nl; nl.n=x.med(); nl.l=true;
    return nl;
  }
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
  int solutions(TestSpace* c, Gecode::Search::Options& o, int maxNbSol = -1) {
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
      if ((maxNbSol >= 0) && (maxNbSol == s)) break;
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

    IntArgs d(arity);
    for (int i=arity; i--; )
      d[i]=i;

    for (int vara = 0; vara<n_int_var_branch; vara++) {
      for (int varb = 1; varb<n_int_var_branch; varb++) {
        for (int val = 0; val<n_int_val_branch; val++) {
          Rnd r(1);

          IntValBranch ivb;
          switch (val) {
          case  0: ivb = INT_VAL_MIN(); break;
          case  1: ivb = INT_VAL_MED(); break;
          case  2: ivb = INT_VAL_MAX(); break;
          case  3: ivb = INT_VAL_RND(r); break;
          case  4: ivb = INT_VAL_SPLIT_MIN(); break;
          case  5: ivb = INT_VAL_SPLIT_MAX(); break;
          case  6: ivb = INT_VAL_RANGE_MIN(); break;
          case  7: ivb = INT_VAL_RANGE_MAX(); break;
          case  8: ivb = INT_VAL(&int_val); break;
          case  9: ivb = INT_VALUES_MIN(); break;
          case 10: ivb = INT_VALUES_MAX(); break;
          }

          IntTestSpace* c = static_cast<IntTestSpace*>(root->clone());

          if ((vara == 0) && (val < 11)) {
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], ivb);
          } else {
            Rnd ra(1);
            IntVarBranch ivba;
            IntAction iaa(*c, c->x, 0.9);
            IntCHB ica(*c, c->x);
            switch (vara) {
            case  0: ivba = INT_VAR_NONE(); break;
            case  1: ivba = INT_VAR_NONE(); break;
            case  2: ivba = INT_VAR_RND(ra); break;
            case  3: ivba = INT_VAR_MERIT_MIN(&int_merit); break;
            case  4: ivba = INT_VAR_MERIT_MAX(&int_merit); break;
            case  5: ivba = INT_VAR_DEGREE_MIN(); break;
            case  6: ivba = INT_VAR_DEGREE_MAX(); break;
            case  7: ivba = INT_VAR_AFC_MIN(0.5); break;
            case  8: ivba = INT_VAR_AFC_MAX(0.5); break;
            case  9: ivba = INT_VAR_ACTION_MIN(iaa); break;
            case 10: ivba = INT_VAR_ACTION_MAX(iaa); break;
            case 11: ivba = INT_VAR_CHB_MIN(ica); break;
            case 12: ivba = INT_VAR_CHB_MAX(ica); break;
            case 13: ivba = INT_VAR_MIN_MIN(); break;
            case 14: ivba = INT_VAR_MIN_MAX(); break;
            case 15: ivba = INT_VAR_MAX_MIN(); break;
            case 16: ivba = INT_VAR_MAX_MAX(); break;
            case 17: ivba = INT_VAR_SIZE_MIN(); break;
            case 18: ivba = INT_VAR_SIZE_MAX(); break;
            case 19: ivba = INT_VAR_DEGREE_SIZE_MIN(); break;
            case 20: ivba = INT_VAR_DEGREE_SIZE_MAX(); break;
            case 21: ivba = INT_VAR_AFC_SIZE_MIN(); break;
            case 22: ivba = INT_VAR_AFC_SIZE_MAX(); break;
            case 23: ivba = INT_VAR_ACTION_SIZE_MIN(iaa); break;
            case 24: ivba = INT_VAR_ACTION_SIZE_MAX(iaa); break;
            case 25: ivba = INT_VAR_CHB_SIZE_MIN(ica); break;
            case 26: ivba = INT_VAR_CHB_SIZE_MAX(ica); break;
            case 27: ivba = INT_VAR_REGRET_MIN_MIN(); break;
            case 28: ivba = INT_VAR_REGRET_MIN_MAX(); break;
            case 29: ivba = INT_VAR_REGRET_MAX_MIN(); break;
            case 30: ivba = INT_VAR_REGRET_MAX_MAX(); break;
            }

            Rnd rb(2);
            IntVarBranch ivbb;
            IntAction iab(*c, c->x, 0.9, &int_merit);
            IntCHB icb(*c, c->x, &int_merit);
            switch (varb) {
            case  0: ivbb = INT_VAR_NONE(); break;
            case  1: ivbb = INT_VAR_NONE(); break;
            case  2: ivbb = INT_VAR_RND(rb); break;
            case  3: ivbb = INT_VAR_MERIT_MIN(&int_merit,&tbl); break;
            case  4: ivbb = INT_VAR_MERIT_MAX(&int_merit,&tbl); break;
            case  5: ivbb = INT_VAR_DEGREE_MIN(&tbl); break;
            case  6: ivbb = INT_VAR_DEGREE_MAX(&tbl); break;
            case  7: ivbb = INT_VAR_AFC_MIN(0.5,&tbl); break;
            case  8: ivbb = INT_VAR_AFC_MAX(0.5,&tbl); break;
            case  9: ivbb = INT_VAR_ACTION_MIN(iab,&tbl); break;
            case 10: ivbb = INT_VAR_ACTION_MAX(iab,&tbl); break;
            case 11: ivbb = INT_VAR_CHB_MIN(icb,&tbl); break;
            case 12: ivbb = INT_VAR_CHB_MAX(icb,&tbl); break;
            case 13: ivbb = INT_VAR_MIN_MIN(&tbl); break;
            case 14: ivbb = INT_VAR_MIN_MAX(&tbl); break;
            case 15: ivbb = INT_VAR_MAX_MIN(&tbl); break;
            case 16: ivbb = INT_VAR_MAX_MAX(&tbl); break;
            case 17: ivbb = INT_VAR_SIZE_MIN(&tbl); break;
            case 18: ivbb = INT_VAR_SIZE_MAX(&tbl); break;
            case 19: ivbb = INT_VAR_DEGREE_SIZE_MIN(&tbl); break;
            case 20: ivbb = INT_VAR_DEGREE_SIZE_MAX(&tbl); break;
            case 21: ivbb = INT_VAR_AFC_SIZE_MIN(1.0,&tbl); break;
            case 22: ivbb = INT_VAR_AFC_SIZE_MAX(1.0,&tbl); break;
            case 23: ivbb = INT_VAR_ACTION_SIZE_MIN(iab,&tbl); break;
            case 24: ivbb = INT_VAR_ACTION_SIZE_MAX(iab,&tbl); break;
            case 25: ivbb = INT_VAR_CHB_SIZE_MIN(icb,&tbl); break;
            case 26: ivbb = INT_VAR_CHB_SIZE_MAX(icb,&tbl); break;
            case 27: ivbb = INT_VAR_REGRET_MIN_MIN(&tbl); break;
            case 28: ivbb = INT_VAR_REGRET_MIN_MAX(&tbl); break;
            case 29: ivbb = INT_VAR_REGRET_MAX_MIN(&tbl); break;
            case 30: ivbb = INT_VAR_REGRET_MAX_MAX(&tbl); break;
            }

            switch (Base::rand(9U)) {
            case 0U:
              branch(*c, c->x, ivba, ivb); break;
            case 1U:
              branch(*c, c->x, ivbb, ivb); break;
            case 2U:
              branch(*c, c->x, tiebreak(ivba,ivbb), ivb); break;
            case 3U:
              branch(*c, c->x, tiebreak(ivbb,ivba), ivb); break;
            case 4U:
              branch(*c, c->x, tiebreak(ivba,ivba,ivbb), ivb); break;
            case 5U:
              branch(*c, c->x, tiebreak(ivba,ivbb,ivbb), ivb); break;
            case 6U:
              branch(*c, c->x, tiebreak(ivbb,ivba,ivba), ivb); break;
            case 7U:
              branch(*c, c->x, tiebreak(ivba,ivba,ivbb,ivba), ivb); break;
            case 8U:
              branch(*c, c->x, tiebreak(ivbb,ivba,ivbb,ivba), ivb); break;
            }

          }
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

    for (int vara = 0; vara<n_bool_var_branch; vara++) {
      for (int varb = 1; varb<n_bool_var_branch; varb++) {
        for (int val = 0; val<n_bool_val_branch; val++) {

          Rnd r(1);

          BoolValBranch bvb;
          switch (val) {
          case  0: bvb = BOOL_VAL_MIN(); break;
          case  1: bvb = BOOL_VAL_MAX(); break;
          case  2: bvb = BOOL_VAL_RND(r); break;
          case  3: bvb = BOOL_VAL(&bool_val); break;
          }

          BoolTestSpace* c = static_cast<BoolTestSpace*>(root->clone());

          if (vara == 0) {
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], bvb);
          } else {


            Rnd ra(1);
            BoolVarBranch bvba;
            BoolAction baa(*c, c->x, 0.9);
            BoolCHB bca(*c, c->x);
            switch (vara) {
            case  0: bvba = BOOL_VAR_NONE(); break;
            case  1: bvba = BOOL_VAR_NONE(); break;
            case  2: bvba = BOOL_VAR_RND(ra); break;
            case  3: bvba = BOOL_VAR_MERIT_MIN(&bool_merit); break;
            case  4: bvba = BOOL_VAR_MERIT_MAX(&bool_merit); break;
            case  5: bvba = BOOL_VAR_DEGREE_MIN(); break;
            case  6: bvba = BOOL_VAR_DEGREE_MAX(); break;
            case  7: bvba = BOOL_VAR_AFC_MIN(0.5); break;
            case  8: bvba = BOOL_VAR_AFC_MAX(0.5); break;
            case  9: bvba = BOOL_VAR_ACTION_MIN(baa); break;
            case 10: bvba = BOOL_VAR_ACTION_MAX(baa); break;
            case 11: bvba = BOOL_VAR_CHB_MIN(bca); break;
            case 12: bvba = BOOL_VAR_CHB_MAX(bca); break;
            }

            Rnd rb(2);
            BoolVarBranch bvbb;
            BoolAction bab(*c, c->x, 0.9, &bool_merit);
            BoolCHB bcb(*c, c->x, &bool_merit);
            switch (varb) {
            case  0: bvbb = BOOL_VAR_NONE(); break;
            case  1: bvbb = BOOL_VAR_NONE(); break;
            case  2: bvbb = BOOL_VAR_RND(rb); break;
            case  3: bvbb = BOOL_VAR_MERIT_MIN(&bool_merit,&tbl); break;
            case  4: bvbb = BOOL_VAR_MERIT_MAX(&bool_merit,&tbl); break;
            case  5: bvbb = BOOL_VAR_DEGREE_MIN(&tbl); break;
            case  6: bvbb = BOOL_VAR_DEGREE_MAX(&tbl); break;
            case  7: bvbb = BOOL_VAR_AFC_MIN(0.5,&tbl); break;
            case  8: bvbb = BOOL_VAR_AFC_MAX(0.5,&tbl); break;
            case  9: bvbb = BOOL_VAR_ACTION_MIN(bab,&tbl); break;
            case 10: bvbb = BOOL_VAR_ACTION_MAX(bab,&tbl); break;
            case 11: bvbb = BOOL_VAR_CHB_MIN(bcb,&tbl); break;
            case 12: bvbb = BOOL_VAR_CHB_MAX(bcb,&tbl); break;
            }

            switch (Base::rand(9U)) {
            case 0U:
              branch(*c, c->x, bvba, bvb); break;
            case 1U:
              branch(*c, c->x, bvbb, bvb); break;
            case 2U:
              branch(*c, c->x, tiebreak(bvba,bvbb), bvb); break;
            case 3U:
              branch(*c, c->x, tiebreak(bvbb,bvba), bvb); break;
            case 4U:
              branch(*c, c->x, tiebreak(bvba,bvba,bvbb), bvb); break;
            case 5U:
              branch(*c, c->x, tiebreak(bvba,bvbb,bvbb), bvb); break;
            case 6U:
              branch(*c, c->x, tiebreak(bvbb,bvba,bvba), bvb); break;
            case 7U:
              branch(*c, c->x, tiebreak(bvba,bvba,bvbb,bvba), bvb); break;
            case 8U:
              branch(*c, c->x, tiebreak(bvbb,bvba,bvbb,bvba), bvb); break;
            }

          }
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
          Rnd r(1);

          SetValBranch svb;
          switch (val) {
          case 0: svb = SET_VAL_MIN_INC(); break;
          case 1: svb = SET_VAL_MIN_EXC(); break;
          case 2: svb = SET_VAL_MED_INC(); break;
          case 3: svb = SET_VAL_MED_EXC(); break;
          case 4: svb = SET_VAL_MAX_INC(); break;
          case 5: svb = SET_VAL_MAX_EXC(); break;
          case 6: svb = SET_VAL_RND_INC(r); break;
          case 7: svb = SET_VAL_RND_EXC(r); break;
          case 8: svb = SET_VAL(&set_val); break;
          }

          SetTestSpace* c = static_cast<SetTestSpace*>(root->clone());

          if (vara == 0) {
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], svb);
          } else {
            Rnd ra(1);
            SetVarBranch svba;
            SetAction saa(*c, c->x, 0.9);
            SetCHB sca(*c, c->x);
            switch (vara) {
            case  0: break;
            case  1: svba = SET_VAR_NONE(); break;
            case  2: svba = SET_VAR_RND(ra); break;
            case  3: svba = SET_VAR_MERIT_MIN(&set_merit); break;
            case  4: svba = SET_VAR_MERIT_MAX(&set_merit); break;
            case  5: svba = SET_VAR_DEGREE_MIN(); break;
            case  6: svba = SET_VAR_DEGREE_MAX(); break;
            case  7: svba = SET_VAR_AFC_MIN(0.5); break;
            case  8: svba = SET_VAR_AFC_MAX(0.5); break;
            case  9: svba = SET_VAR_ACTION_MIN(saa); break;
            case 10: svba = SET_VAR_ACTION_MAX(saa); break;
            case 11: svba = SET_VAR_CHB_MIN(sca); break;
            case 12: svba = SET_VAR_CHB_MAX(sca); break;
            case 13: svba = SET_VAR_MIN_MIN(); break;
            case 14: svba = SET_VAR_MIN_MAX(); break;
            case 15: svba = SET_VAR_MAX_MIN(); break;
            case 16: svba = SET_VAR_MAX_MAX(); break;
            case 17: svba = SET_VAR_SIZE_MIN(); break;
            case 18: svba = SET_VAR_SIZE_MAX(); break;
            case 19: svba = SET_VAR_DEGREE_SIZE_MIN(); break;
            case 20: svba = SET_VAR_DEGREE_SIZE_MAX(); break;
            case 21: svba = SET_VAR_AFC_SIZE_MIN(); break;
            case 22: svba = SET_VAR_AFC_SIZE_MAX(); break;
            case 23: svba = SET_VAR_ACTION_SIZE_MIN(saa); break;
            case 24: svba = SET_VAR_ACTION_SIZE_MAX(saa); break;
            case 25: svba = SET_VAR_CHB_SIZE_MIN(sca); break;
            case 26: svba = SET_VAR_CHB_SIZE_MAX(sca); break;
            }

            Rnd rb(2);
            SetVarBranch svbb;
            SetAction sab(*c, c->x, 0.9, &set_merit);
            SetCHB scb(*c, c->x, &set_merit);
            switch (varb) {
            case  0: break;
            case  1: svbb = SET_VAR_NONE(); break;
            case  2: svbb = SET_VAR_RND(rb); break;
            case  3: svbb = SET_VAR_MERIT_MIN(&set_merit,&tbl); break;
            case  4: svbb = SET_VAR_MERIT_MAX(&set_merit,&tbl); break;
            case  5: svbb = SET_VAR_DEGREE_MIN(&tbl); break;
            case  6: svbb = SET_VAR_DEGREE_MAX(&tbl); break;
            case  7: svbb = SET_VAR_AFC_MIN(0.5,&tbl); break;
            case  8: svbb = SET_VAR_AFC_MAX(0.5,&tbl); break;
            case  9: svbb = SET_VAR_ACTION_MIN(sab,&tbl); break;
            case 10: svbb = SET_VAR_ACTION_MAX(sab,&tbl); break;
            case 11: svbb = SET_VAR_CHB_MIN(scb,&tbl); break;
            case 12: svbb = SET_VAR_CHB_MAX(scb,&tbl); break;
            case 13: svbb = SET_VAR_MIN_MIN(&tbl); break;
            case 14: svbb = SET_VAR_MIN_MAX(&tbl); break;
            case 15: svbb = SET_VAR_MAX_MIN(&tbl); break;
            case 16: svbb = SET_VAR_MAX_MAX(&tbl); break;
            case 17: svbb = SET_VAR_SIZE_MIN(&tbl); break;
            case 18: svbb = SET_VAR_SIZE_MAX(&tbl); break;
            case 19: svbb = SET_VAR_DEGREE_SIZE_MIN(&tbl); break;
            case 20: svbb = SET_VAR_DEGREE_SIZE_MAX(&tbl); break;
            case 21: svbb = SET_VAR_AFC_SIZE_MIN(1.0,&tbl); break;
            case 22: svbb = SET_VAR_AFC_SIZE_MAX(1.0,&tbl); break;
            case 23: svbb = SET_VAR_ACTION_SIZE_MIN(sab,&tbl); break;
            case 24: svbb = SET_VAR_ACTION_SIZE_MAX(sab,&tbl); break;
            case 25: svbb = SET_VAR_CHB_SIZE_MIN(scb,&tbl); break;
            case 26: svbb = SET_VAR_CHB_SIZE_MAX(scb,&tbl); break;
            }

            switch (Base::rand(9U)) {
            case 0U:
              branch(*c, c->x, svba, svb); break;
            case 1U:
              branch(*c, c->x, svbb, svb); break;
            case 2U:
              branch(*c, c->x, tiebreak(svba,svbb), svb); break;
            case 3U:
              branch(*c, c->x, tiebreak(svbb,svba), svb); break;
            case 4U:
              branch(*c, c->x, tiebreak(svba,svba,svbb), svb); break;
            case 5U:
              branch(*c, c->x, tiebreak(svba,svbb,svbb), svb); break;
            case 6U:
              branch(*c, c->x, tiebreak(svbb,svba,svba), svb); break;
            case 7U:
              branch(*c, c->x, tiebreak(svba,svba,svbb,svba), svb); break;
            case 8U:
              branch(*c, c->x, tiebreak(svbb,svba,svbb,svba), svb); break;
            }

          }
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

#ifdef GECODE_HAS_FLOAT_VARS
  FloatTest::FloatTest(const std::string& s, int a, const Gecode::FloatVal& d, int nbs)
    : Base("Float::Branch::"+s), arity(a), dom(d), nbSols(nbs) {
  }

  bool
  FloatTest::run(void) {
    using std::map;
    using std::vector;
    using std::string;
    using std::ostream;
    using namespace Gecode;

    // Results of tests run
    map<int, vector<RunInfo> > results;
    // Set up root space
    FloatTestSpace* root = new FloatTestSpace(arity,dom);
    post(*root, root->x);
    root->status();
    results.clear();

    for (int vara = 0; vara<n_float_var_branch; vara++) {
      for (int varb = 1; varb<n_float_var_branch; varb++) {
        for (int val = 0; val<n_float_val_branch; val++) {
          Rnd r(1);

          FloatValBranch fvb;
          switch (val) {
          case 0: fvb = FLOAT_VAL_SPLIT_MIN(); break;
          case 1: fvb = FLOAT_VAL_SPLIT_MAX(); break;
          case 2: fvb = FLOAT_VAL_SPLIT_RND(r); break;
          case 3: fvb = FLOAT_VAL(&float_val); break;
          }

          FloatTestSpace* c = static_cast<FloatTestSpace*>(root->clone());
          if (vara == 0) {
            for (int i=0; i<c->x.size(); i++)
              branch(*c, c->x[i], fvb);
          } else {
            Rnd ra(1);
            FloatVarBranch fvba;
            FloatAction faa(*c, c->x, 0.9);
            FloatCHB fca(*c, c->x);
            switch (vara) {
            case  0: break;
            case  1: fvba = FLOAT_VAR_NONE(); break;
            case  2: fvba = FLOAT_VAR_RND(ra); break;
            case  3: fvba = FLOAT_VAR_MERIT_MIN(&float_merit); break;
            case  4: fvba = FLOAT_VAR_MERIT_MAX(&float_merit); break;
            case  5: fvba = FLOAT_VAR_DEGREE_MIN(); break;
            case  6: fvba = FLOAT_VAR_DEGREE_MAX(); break;
            case  7: fvba = FLOAT_VAR_AFC_MIN(0.5); break;
            case  8: fvba = FLOAT_VAR_AFC_MAX(0.5); break;
            case  9: fvba = FLOAT_VAR_ACTION_MIN(faa); break;
            case 10: fvba = FLOAT_VAR_ACTION_MAX(faa); break;
            case 11: fvba = FLOAT_VAR_CHB_MIN(fca); break;
            case 12: fvba = FLOAT_VAR_CHB_MAX(fca); break;
            case 13: fvba = FLOAT_VAR_MIN_MIN(); break;
            case 14: fvba = FLOAT_VAR_MIN_MAX(); break;
            case 15: fvba = FLOAT_VAR_MAX_MIN(); break;
            case 16: fvba = FLOAT_VAR_MAX_MAX(); break;
            case 17: fvba = FLOAT_VAR_SIZE_MIN(); break;
            case 18: fvba = FLOAT_VAR_SIZE_MAX(); break;
            case 19: fvba = FLOAT_VAR_DEGREE_SIZE_MIN(); break;
            case 20: fvba = FLOAT_VAR_DEGREE_SIZE_MAX(); break;
            case 21: fvba = FLOAT_VAR_AFC_SIZE_MIN(); break;
            case 22: fvba = FLOAT_VAR_AFC_SIZE_MAX(); break;
            case 23: fvba = FLOAT_VAR_ACTION_SIZE_MIN(faa); break;
            case 24: fvba = FLOAT_VAR_ACTION_SIZE_MAX(faa); break;
            case 25: fvba = FLOAT_VAR_CHB_SIZE_MIN(fca); break;
            case 26: fvba = FLOAT_VAR_CHB_SIZE_MAX(fca); break;
            }

            Rnd rb(2);
            FloatVarBranch fvbb;
            FloatAction fab(*c, c->x, 0.9, &float_merit);
            FloatCHB fcb(*c, c->x, &float_merit);
            switch (varb) {
            case  0: break;
            case  1: fvbb = FLOAT_VAR_NONE(); break;
            case  2: fvbb = FLOAT_VAR_RND(rb); break;
            case  3: fvbb = FLOAT_VAR_MERIT_MIN(&float_merit,&tbl); break;
            case  4: fvbb = FLOAT_VAR_MERIT_MAX(&float_merit,&tbl); break;
            case  5: fvbb = FLOAT_VAR_DEGREE_MIN(&tbl); break;
            case  6: fvbb = FLOAT_VAR_DEGREE_MAX(&tbl); break;
            case  7: fvbb = FLOAT_VAR_AFC_MIN(0.5,&tbl); break;
            case  8: fvbb = FLOAT_VAR_AFC_MAX(0.5,&tbl); break;
            case  9: fvbb = FLOAT_VAR_ACTION_MIN(fab,&tbl); break;
            case 10: fvbb = FLOAT_VAR_ACTION_MAX(fab,&tbl); break;
            case 11: fvbb = FLOAT_VAR_CHB_MIN(fcb,&tbl); break;
            case 12: fvbb = FLOAT_VAR_CHB_MAX(fcb,&tbl); break;
            case 13: fvbb = FLOAT_VAR_MIN_MIN(&tbl); break;
            case 14: fvbb = FLOAT_VAR_MIN_MAX(&tbl); break;
            case 15: fvbb = FLOAT_VAR_MAX_MIN(&tbl); break;
            case 16: fvbb = FLOAT_VAR_MAX_MAX(&tbl); break;
            case 17: fvbb = FLOAT_VAR_SIZE_MIN(&tbl); break;
            case 18: fvbb = FLOAT_VAR_SIZE_MAX(&tbl); break;
            case 19: fvbb = FLOAT_VAR_DEGREE_SIZE_MIN(&tbl); break;
            case 20: fvbb = FLOAT_VAR_DEGREE_SIZE_MAX(&tbl); break;
            case 21: fvbb = FLOAT_VAR_AFC_SIZE_MIN(1.0,&tbl); break;
            case 22: fvbb = FLOAT_VAR_AFC_SIZE_MAX(1.0,&tbl); break;
            case 23: fvbb = FLOAT_VAR_ACTION_SIZE_MIN(fab,&tbl); break;
            case 24: fvbb = FLOAT_VAR_ACTION_SIZE_MAX(fab,&tbl); break;
            case 25: fvbb = FLOAT_VAR_CHB_SIZE_MIN(fcb,&tbl); break;
            case 26: fvbb = FLOAT_VAR_CHB_SIZE_MAX(fcb,&tbl); break;
            }

            switch (Base::rand(9U)) {
            case 0U:
              branch(*c, c->x, fvba, fvb); break;
            case 1U:
              branch(*c, c->x, fvbb, fvb); break;
            case 2U:
              branch(*c, c->x, tiebreak(fvba,fvbb), fvb); break;
            case 3U:
              branch(*c, c->x, tiebreak(fvbb,fvba), fvb); break;
            case 4U:
              branch(*c, c->x, tiebreak(fvba,fvba,fvbb), fvb); break;
            case 5U:
              branch(*c, c->x, tiebreak(fvba,fvbb,fvbb), fvb); break;
            case 6U:
              branch(*c, c->x, tiebreak(fvbb,fvba,fvba), fvb); break;
            case 7U:
              branch(*c, c->x, tiebreak(fvba,fvba,fvbb,fvba), fvb); break;
            case 8U:
              branch(*c, c->x, tiebreak(fvbb,fvba,fvbb,fvba), fvb); break;
            }

          }
          Gecode::Search::Options o;
          results[solutions(c,o,nbSols)].push_back
            (RunInfo(float_var_branch_name[vara],
                     float_var_branch_name[varb],
                     float_val_branch_name[val],
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
