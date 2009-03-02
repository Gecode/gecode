/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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
    IntTestSpace(bool share, IntTestSpace& s)
      : Gecode::Space(share,s) {
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
    SetTestSpace(int n, const Gecode::IntSet& d)
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

  /** \name Collection of possible arguments for integer assignments
   *
   * \relates IntTestSpace BoolTestSpace
   */
  //@{
  /// Integer value assignments
  const Gecode::IntAssign int_assign[] = {
    Gecode::INT_ASSIGN_MIN,
    Gecode::INT_ASSIGN_MED,
    Gecode::INT_ASSIGN_MAX,
    Gecode::INT_ASSIGN_RND
  };
  /// Number of integer value selections
  const int n_int_assign =
    sizeof(int_assign)/sizeof(Gecode::IntAssign);
  /// Names for integer assignments
  const char* int_assign_name[] = {
    "INT_ASSIGN_MIN",
    "INT_ASSIGN_MED",
    "INT_ASSIGN_MAX",
    "INT_ASSIGN_RND"
  };
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

    for (int val = n_int_assign; val--; ) {
      IntTestSpace* clone = static_cast<IntTestSpace*>(root->clone(false));
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);
      assign(*clone, clone->x, int_assign[val]);
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

    for (int val = n_int_assign; val--; ) {
      BoolTestSpace* clone = static_cast<BoolTestSpace*>(root->clone(false));
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);
      assign(*clone, clone->x, int_assign[val]);
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
  /// Set value assignments
  const Gecode::SetAssign set_assign[] = {
    Gecode::SET_ASSIGN_MIN_INC,
    Gecode::SET_ASSIGN_MIN_EXC,
    Gecode::SET_ASSIGN_MED_INC,
    Gecode::SET_ASSIGN_MED_EXC,
    Gecode::SET_ASSIGN_MAX_INC,
    Gecode::SET_ASSIGN_MAX_EXC,
    Gecode::SET_ASSIGN_RND_INC,
    Gecode::SET_ASSIGN_RND_EXC
  };
  /// Number of set value selections
  const int n_set_assign =
    sizeof(set_assign)/sizeof(Gecode::SetAssign);
  /// Names for integer assignments
  const char* set_assign_name[] = {
    "SET_ASSIGN_MIN_INC",
    "SET_ASSIGN_MIN_EXC",
    "SET_ASSIGN_MED_INC",
    "SET_ASSIGN_MED_EXC",
    "SET_ASSIGN_MAX_INC",
    "SET_ASSIGN_MAX_EXC",
    "SET_ASSIGN_RND_INC",
    "SET_ASSIGN_RND_EXC"
  };
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

    for (int val = n_int_assign; val--; ) {
      SetTestSpace* clone = static_cast<SetTestSpace*>(root->clone(false));
      Gecode::Search::Options o;
      o.a_d = Base::rand(10);
      o.c_d = Base::rand(10);
      assign(*clone, clone->x, set_assign[val]);
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

}}

// STATISTICS: test-branch
