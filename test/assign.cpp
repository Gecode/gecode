/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include "test/assign.hh"

#include <gecode/search.hh>

namespace Test { namespace Assign {

  /// Space for executing integer tests
  class IntTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::IntVarArray x;
    /// Initialize test space
    IntTestSpace(int n, Gecode::IntSet& d)
      : x(*this, n, d) {}
    /// Constructor for cloning \a s
    IntTestSpace(IntTestSpace& s)
      : Gecode::Space(s) {
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

  /// Space for executing Boolean tests
  class SetTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::SetVarArray x;
    /// Initialize test space
    SetTestSpace(int n, const Gecode::IntSet& d)
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

  /// Space for executing Boolean tests
  class FloatTestSpace : public Gecode::Space {
  public:
    /// Variables to be tested
    Gecode::FloatVarArray x;
    /// Initialize test space
    FloatTestSpace(int n, const Gecode::FloatVal& d)
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

  /** \name Collection of possible arguments for integer assignments
   *
   * \relates IntTestSpace
   */
  //@{
  /// Names for integer assignments
  const char* int_assign_name[] = {
    "INT_ASSIGN_MIN",
    "INT_ASSIGN_MED",
    "INT_ASSIGN_MAX",
    "INT_ASSIGN_RND",
    "INT_ASSIGN"
  };
  /// Number of integer value selections
  const int n_int_assign =
    sizeof(int_assign_name)/sizeof(const char*);
  /// Test function for branch value function
  int int_val(const Gecode::Space&, Gecode::IntVar x, int) {
    return x.min();
  }
  //@}

  /** \name Collection of possible arguments for Boolean assignments
   *
   * \relates BoolTestSpace
   */
  //@{
  /// Names for integer assignments
  const char* bool_assign_name[] = {
    "BOOL_ASSIGN_MIN",
    "BOOL_ASSIGN_MAX",
    "BOOL_ASSIGN_RND",
    "BOOL_ASSIGN"
  };
  /// Number of integer value selections
  const int n_bool_assign =
    sizeof(bool_assign_name)/sizeof(const char*);
  /// Test function for branch value function
  int bool_val(const Gecode::Space&, Gecode::BoolVar x, int) {
    return x.min();
  }
  //@}

  IntTest::IntTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Int::Assign::"+s), arity(a), dom(d) {
  }

  bool
  IntTest::run(void) {
    using namespace Gecode;
    IntTestSpace* root = new IntTestSpace(arity,dom);
    post(*root, root->x);
    (void) root->status();

    for (int val = 0; val<n_int_assign; val++) {
      IntTestSpace* clone = static_cast<IntTestSpace*>(root->clone());
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);

      Rnd r(1);
      IntAssign ia;
      switch (val) {
      case 0: ia = INT_ASSIGN_MIN(); break;
      case 1: ia = INT_ASSIGN_MED(); break;
      case 2: ia = INT_ASSIGN_MAX(); break;
      case 3: ia = INT_ASSIGN_RND(r); break;
      case 4: ia = INT_ASSIGN(&int_val); break;
      default: GECODE_NEVER;
      }

      assign(*clone, clone->x, ia);
      Gecode::DFS<IntTestSpace> e_s(clone, o);
      delete clone;

      // Find number of solutions
      int solutions = 0;
      while (Space* s = e_s.next()) {
        delete s; solutions++;
      }
      if (solutions != 1) {
        std::cout << "FAILURE" << std::endl
                  << "\tc_d=" << o.c_d << ", a_d=" << o.a_d << std::endl
                  << "\t" << int_assign_name[val] << std::endl;
        delete root;
        return false;
      }
    }
    delete root;
    return true;
  }

  BoolTest::BoolTest(const std::string& s, int a)
    : Base("Bool::Assign::"+s), arity(a) {
  }

  bool
  BoolTest::run(void) {
    using namespace Gecode;
    BoolTestSpace* root = new BoolTestSpace(arity);
    post(*root, root->x);
    (void) root->status();

    for (int val = n_bool_assign; val--; ) {
      BoolTestSpace* clone = static_cast<BoolTestSpace*>(root->clone());
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);
      Rnd r(1);
      BoolAssign ia;
      switch (val) {
      case 0: ia = BOOL_ASSIGN_MIN(); break;
      case 1: ia = BOOL_ASSIGN_MAX(); break;
      case 2: ia = BOOL_ASSIGN_RND(r); break;
      case 3: ia = BOOL_ASSIGN(&bool_val); break;
      default: GECODE_NEVER;
      }

      assign(*clone, clone->x, ia);
      Gecode::DFS<BoolTestSpace> e_s(clone, o);
      delete clone;

      // Find number of solutions
      int solutions = 0;
      while (Space* s = e_s.next()) {
        delete s; solutions++;
      }
      if (solutions != 1) {
        std::cout << "FAILURE" << std::endl
                  << "\tc_d=" << o.c_d << ", a_d=" << o.a_d << std::endl
                  << "\t" << int_assign_name[val] << std::endl;
        delete root;
        return false;
      }
    }
    delete root;
    return true;
  }

#ifdef GECODE_HAS_SET_VARS

  /** \name Collection of possible arguments for set assignments
   *
   * \relates SetTestSpace
   */
  //@{
  /// Names for integer assignments
  const char* set_assign_name[] = {
    "SET_ASSIGN_MIN_INC",
    "SET_ASSIGN_MIN_EXC",
    "SET_ASSIGN_MED_INC",
    "SET_ASSIGN_MED_EXC",
    "SET_ASSIGN_MAX_INC",
    "SET_ASSIGN_MAX_EXC",
    "SET_ASSIGN_RND_INC",
    "SET_ASSIGN_RND_EXC",
    "SET_ASSIGN"
  };
  /// Number of set value selections
  const int n_set_assign =
    sizeof(set_assign_name)/sizeof(const char*);
  /// Test function for branch value function
  int set_val(const Gecode::Space&, Gecode::SetVar x, int) {
    Gecode::SetVarUnknownRanges r(x);
    return r.min();
  }
  //@}

  SetTest::SetTest(const std::string& s, int a, const Gecode::IntSet& d)
    : Base("Set::Assign::"+s), arity(a), dom(d) {
  }

  bool
  SetTest::run(void) {
    using namespace Gecode;
    SetTestSpace* root = new SetTestSpace(arity,dom);
    post(*root, root->x);
    (void) root->status();

    for (int val = n_set_assign; val--; ) {
      SetTestSpace* clone = static_cast<SetTestSpace*>(root->clone());
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);

      Rnd r(1);

      SetAssign sa;
      switch (val) {
      case 0: sa = SET_ASSIGN_MIN_INC(); break;
      case 1: sa = SET_ASSIGN_MIN_EXC(); break;
      case 2: sa = SET_ASSIGN_MED_INC(); break;
      case 3: sa = SET_ASSIGN_MED_EXC(); break;
      case 4: sa = SET_ASSIGN_MAX_INC(); break;
      case 5: sa = SET_ASSIGN_MAX_EXC(); break;
      case 6: sa = SET_ASSIGN_RND_INC(r); break;
      case 7: sa = SET_ASSIGN_RND_EXC(r); break;
      case 8: sa = SET_ASSIGN(&set_val); break;
      default: GECODE_NEVER;
      }

      assign(*clone, clone->x, sa);
      Gecode::DFS<SetTestSpace> e_s(clone, o);
      delete clone;

      // Find number of solutions
      int solutions = 0;
      while (Space* s = e_s.next()) {
        delete s; solutions++;
      }
      if (solutions != 1) {
        std::cout << "FAILURE" << std::endl
                  << "\tc_d=" << o.c_d << ", a_d=" << o.a_d << std::endl
                  << "\t" << set_assign_name[val] << std::endl;
        delete root;
        return false;
      }
    }
    delete root;
    return true;
  }

#endif

#ifdef GECODE_HAS_FLOAT_VARS

  /** \name Collection of possible arguments for float assignments
   *
   * \relates FloatTestSpace
   */
  //@{
  /// Names for float assignments
  const char* float_assign_name[] = {
    "FLOAT_ASSIGN_MIN",
    "FLOAT_ASSIGN_MAX",
    "FLOAT_ASSIGN_RND",
    "FLOAT_ASSIGN"
  };
  /// Number of float value selections
  const int n_float_assign =
    sizeof(float_assign_name)/sizeof(const char*);
  /// Test function for branch value function
  Gecode::FloatNumBranch float_val(const Gecode::Space&,
                                   Gecode::FloatVar x, int) {
    Gecode::FloatNumBranch nl; nl.n=x.med(); nl.l=true;
    return nl;
  }
  //@}

  FloatTest::FloatTest(const std::string& s, int a, const Gecode::FloatVal& d)
    : Base("Float::Assign::"+s), arity(a), dom(d) {
  }

  bool
  FloatTest::run(void) {
    using namespace Gecode;
    FloatTestSpace* root = new FloatTestSpace(arity,dom);
    post(*root, root->x);
    (void) root->status();

    for (int val = n_float_assign; val--; ) {
      FloatTestSpace* clone = static_cast<FloatTestSpace*>(root->clone());
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);

      Rnd r(1);

      FloatAssign fa;
      switch (val) {
      case 0: fa = FLOAT_ASSIGN_MIN(); break;
      case 1: fa = FLOAT_ASSIGN_MAX(); break;
      case 2: fa = FLOAT_ASSIGN_RND(r); break;
      case 3: fa = FLOAT_ASSIGN(&float_val); break;
      default: GECODE_NEVER;
      }

      assign(*clone, clone->x, fa);
      Gecode::DFS<FloatTestSpace> e_s(clone, o);
      delete clone;

      // Find number of solutions
      int solutions = 0;
      while (Space* s = e_s.next()) {
        delete s; solutions++;
      }
      if (solutions != 1) {
        std::cout << "FAILURE" << std::endl
                  << "\tc_d=" << o.c_d << ", a_d=" << o.a_d << std::endl
                  << "\t" << float_assign_name[val] << std::endl;
        delete root;
        return false;
      }
    }
    delete root;
    return true;
  }

#endif

}}

// STATISTICS: test-branch
