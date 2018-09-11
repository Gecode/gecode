/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christopher Mears <chris.mears@monash.edu>
 *
 *  Copyright:
 *     Christopher Mears, 2012
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

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#include <gecode/int/branch.hh>

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#include <gecode/set/branch.hh>
#include <stdarg.h>
#endif

#include <gecode/minimodel.hh>

#include "test/test.hh"

#include <vector>

/**
 * \namespace Test::LDSB
 * \brief Testing for LDSB
 */
namespace Test { namespace LDSB {

  using namespace Gecode;

  /// Returns true iff a and b are equal (they have the same size and
  /// the same elements in the same positions).
  bool
  equal(const IntArgs& a, const IntArgs& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0 ; i < a.size() ; ++i)
      if (a[i] != b[i])
        return false;
    return true;
  }

#ifdef GECODE_HAS_SET_VARS
  /// Returns true iff a and b are equal (they have the same size and
  /// the same elements in the same positions).
  bool
  equal(const IntSetArgs& a, const IntSetArgs& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0 ; i < a.size() ; ++i) {
      // Compare the two sets a[i] and b[i].
      // Perhaps TODO: use Iter::Ranges::equal instead.
      if (a[i].size() != b[i].size()) return false;
      IntSetValues x(a[i]);
      IntSetValues y(b[i]);
      while (x() && y()) {
        if (x.val() != y.val()) return false;
        ++x;
        ++y;
      }
    }
    return true;
  }
#endif

  /**
   * \brief Checks found solutions against expected solutions
   *
   * Iterates through the solutions returned by the search engine and
   * checks each one against the array of expected solutions.  The
   * order of solutions must be the same.  Returns true if the
   * expected solutions match the found solutions exactly, otherwise
   * prints an explanation to standard error and returns false.
   */
  template <class T, class VarArgsType>
  bool
  check(DFS<T>& e, std::vector<VarArgsType> expected) {
    int nexpected = expected.size();
    for (int i = 0 ; i < nexpected ; ++i) {
      T* s = e.next();
      if (s == NULL) {
        if (opt.log) {
          olog << "Expected a solution but there are no more solutions." << std::endl;
          olog << "(Expected " << nexpected << " but only found " << i << ")" << std::endl;
          olog << "Expected: " << expected[i] << std::endl;
        }
        return false;
      }
      if (!equal(s->solution(), expected[i])) {
        if (opt.log) {
          olog << "Solution does not match expected." << std::endl;
          olog << "Solution: " << s->solution() << std::endl;
          olog << "Expected: " << expected[i] << std::endl;
        }
        return false;
      }
      delete s;
    }
    T* s = e.next();
    if (s != NULL) {
      if (opt.log) {
        olog << "More solutions than expected:" << std::endl;
        olog << "(Expected only " << nexpected << ")" << std::endl;
        olog << s->solution() << std::endl;
      }
      return false;
    }

    // Nothing went wrong.
    return true;
  }


  /// %Test space
  class OneArray : public Space {
  public:
    /// Variables
    IntVarArray xs;
    /// Constructor for creation
    OneArray(int n, int l, int u) : xs(*this,n,l,u) {
    }
    /// Constructor for cloning \a s
    OneArray(OneArray& s) : Space(s) {
      xs.update(*this,s.xs);
    }
    /// Copy during cloning
    virtual Space* copy(void) {
      return new OneArray(*this);
    }
    /// Return the solution as IntArgs
    IntArgs solution(void) {
      IntArgs a(xs.size());
      for (int i = 0 ; i < a.size() ; ++i)
        a[i] = xs[i].val();
      return a;
    }
    /// Expected solutions
    virtual IntArgs* expectedSolutions(void) { return NULL; }
  };

#ifdef GECODE_HAS_SET_VARS
  /// %Test space (set version)
  class OneArraySet : public Space {
  public:
    /// Variables
    SetVarArray xs;
    /// Constructor for creation
    OneArraySet(int n, int l, int u) : xs(*this,n, IntSet::empty, l,u) {
    }
    /// Constructor for cloning \a s
    OneArraySet(OneArraySet& s) : Space(s) {
      xs.update(*this,s.xs);
    }
    /// Copy during cloning
    virtual Space* copy(void) {
      return new OneArraySet(*this);
    }
    /// Return the solution as IntSetArgs
    IntSetArgs solution(void) {
      IntSetArgs a(xs.size());
      for (int i = 0 ; i < a.size() ; ++i) {
        SetVarGlbRanges glbranges(xs[i]);
        a[i] = IntSet(glbranges);
      }
      return a;
    }
    /// Expected solutions
    virtual IntSetArgs* expectedSolutions(void) { return NULL; }
  };
#endif

  /// %Test for %LDSB infrastructure
  template <class T>
  class LDSB : public Base {
  public:
    /// Recomputation distance
    unsigned int c_d;
    /// Adaptation distance
    unsigned int a_d;
    /// Initialize test
    LDSB(std::string label, unsigned int c=0, unsigned int a=0)
      : Test::Base("LDSB::" + label),
        c_d(c), a_d(a) {}
    /// Perform actual tests
    bool run(void) {
      OneArray *s = new OneArray(T::n, T::l, T::u);
      T::setup(*s, s->xs);
      Search::Options o = Search::Options::def;
      if (c_d != 0) o.c_d = c_d;
      if (a_d != 0) o.a_d = a_d;
      DFS<OneArray> e(s,o);
      bool r = check(e, T::expectedSolutions());
      delete s;
      return r;
    }
  };

#ifdef GECODE_HAS_SET_VARS
  /// %Test for %LDSB infrastructure
  template <class T>
  class LDSBSet : public Base {
  public:
    /// Recomputation distance
    unsigned int c_d;
    /// Adaptation distance
    unsigned int a_d;
    /// Initialize test
    LDSBSet(std::string label, unsigned int c=0, unsigned int a=0)
      : Test::Base("LDSB::" + label),
        c_d(c), a_d(a) {}
    /// Perform actual tests
    bool run(void) {
      OneArraySet *s = new OneArraySet(T::n, T::l, T::u);
      T::setup(*s, s->xs);
      Search::Options o = Search::Options::def;
      if (c_d != 0) o.c_d = c_d;
      if (a_d != 0) o.a_d = a_d;
      DFS<OneArraySet> e(s,o);
      bool r = check(e, T::expectedSolutions());
      delete s;
      return r;
    }
  };
#endif

  // Test cases

  /// %Test for variable symmetry
  class VarSym1 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries syms;
      IntArgs indices({0,1,2,3});
      syms << VariableSymmetry(xs, indices);
      distinct(home, xs);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for variable symmetry
  class VarSym1b {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries syms;
      syms << VariableSymmetry(xs);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for variable symmetry
  class VarSym2 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries syms;
      IntArgs indices({0,1,2,3});
      syms << VariableSymmetry(xs);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,0,0}));
      expected.push_back(IntArgs({0,0,0,1}));
      expected.push_back(IntArgs({0,0,0,2}));
      expected.push_back(IntArgs({0,0,0,3}));
      expected.push_back(IntArgs({0,0,1,1}));
      expected.push_back(IntArgs({0,0,1,2}));
      expected.push_back(IntArgs({0,0,1,3}));
      expected.push_back(IntArgs({0,0,2,2}));
      expected.push_back(IntArgs({0,0,2,3}));
      expected.push_back(IntArgs({0,0,3,3}));
      expected.push_back(IntArgs({0,1,1,1}));
      expected.push_back(IntArgs({0,1,1,2}));
      expected.push_back(IntArgs({0,1,1,3}));
      expected.push_back(IntArgs({0,1,2,2}));
      expected.push_back(IntArgs({0,1,2,3}));
      expected.push_back(IntArgs({0,1,3,3}));
      expected.push_back(IntArgs({0,2,2,2}));
      expected.push_back(IntArgs({0,2,2,3}));
      expected.push_back(IntArgs({0,2,3,3}));
      expected.push_back(IntArgs({0,3,3,3}));
      expected.push_back(IntArgs({1,1,1,1}));
      expected.push_back(IntArgs({1,1,1,2}));
      expected.push_back(IntArgs({1,1,1,3}));
      expected.push_back(IntArgs({1,1,2,2}));
      expected.push_back(IntArgs({1,1,2,3}));
      expected.push_back(IntArgs({1,1,3,3}));
      expected.push_back(IntArgs({1,2,2,2}));
      expected.push_back(IntArgs({1,2,2,3}));
      expected.push_back(IntArgs({1,2,3,3}));
      expected.push_back(IntArgs({1,3,3,3}));
      expected.push_back(IntArgs({2,2,2,2}));
      expected.push_back(IntArgs({2,2,2,3}));
      expected.push_back(IntArgs({2,2,3,3}));
      expected.push_back(IntArgs({2,3,3,3}));
      expected.push_back(IntArgs({3,3,3,3}));
      return expected;
    }
  };

  /// %Test for variable symmetry
  class VarSym3 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries syms;
      distinct(home, xs);
      syms << VariableSymmetry(IntVarArgs() << xs[0] << xs[1]);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      expected.push_back(IntArgs({0,1,3,2}));
      expected.push_back(IntArgs({0,2,1,3}));
      expected.push_back(IntArgs({0,2,3,1}));
      expected.push_back(IntArgs({0,3,1,2}));
      expected.push_back(IntArgs({0,3,2,1}));
      expected.push_back(IntArgs({1,2,0,3}));
      expected.push_back(IntArgs({1,2,3,0}));
      expected.push_back(IntArgs({1,3,0,2}));
      expected.push_back(IntArgs({1,3,2,0}));
      expected.push_back(IntArgs({2,3,0,1}));
      expected.push_back(IntArgs({2,3,1,0}));
      return expected;
    }
  };

  /// %Test for variable symmetry
  class VarSym4 {
  public:
    /// Number of variables
    static const int n = 3;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 2;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      IntVarArgs symvars;
      symvars << xs[0];
      s << VariableSymmetry(symvars);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2}));
      expected.push_back(IntArgs({0,2,1}));
      expected.push_back(IntArgs({1,0,2}));
      expected.push_back(IntArgs({1,2,0}));
      expected.push_back(IntArgs({2,0,1}));
      expected.push_back(IntArgs({2,1,0}));
      return expected;
    }
  };

  /// %Test for variable symmetry
  class VarSym5 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Matrix<IntVarArray> m(xs, 4, 1);
      Symmetries s;
      s << VariableSymmetry(m.slice(0,2, 0,1));
      s << VariableSymmetry(m.slice(2,4, 0,1));
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      expected.push_back(IntArgs({0,2,1,3}));
      expected.push_back(IntArgs({0,3,1,2}));
      expected.push_back(IntArgs({1,2,0,3}));
      expected.push_back(IntArgs({1,3,0,2}));
      expected.push_back(IntArgs({2,3,0,1}));
      return expected;
    }
  };

  /// %Test for matrix symmetry
  class MatSym1 {
  public:
    /// Number of variables
    static const int n = 6;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 2, 3);
      Symmetries s;
      s << rows_interchange(m);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0, 0,0, 0,0}));
      expected.push_back(IntArgs({0,0, 0,0, 0,1}));
      expected.push_back(IntArgs({0,0, 0,0, 1,0}));
      expected.push_back(IntArgs({0,0, 0,0, 1,1}));
      expected.push_back(IntArgs({0,0, 0,1, 0,0}));
      expected.push_back(IntArgs({0,0, 0,1, 0,1}));
      expected.push_back(IntArgs({0,0, 0,1, 1,0}));
      expected.push_back(IntArgs({0,0, 0,1, 1,1}));
      expected.push_back(IntArgs({0,0, 1,0, 1,0}));
      expected.push_back(IntArgs({0,0, 1,0, 1,1}));
      expected.push_back(IntArgs({0,0, 1,1, 1,1}));
      expected.push_back(IntArgs({0,1, 0,0, 0,0}));
      expected.push_back(IntArgs({0,1, 0,0, 0,1}));
      expected.push_back(IntArgs({0,1, 0,0, 1,0}));
      expected.push_back(IntArgs({0,1, 0,0, 1,1}));
      expected.push_back(IntArgs({0,1, 0,1, 0,0}));
      expected.push_back(IntArgs({0,1, 0,1, 0,1}));
      expected.push_back(IntArgs({0,1, 0,1, 1,0}));
      expected.push_back(IntArgs({0,1, 0,1, 1,1}));
      expected.push_back(IntArgs({0,1, 1,0, 1,0}));
      expected.push_back(IntArgs({0,1, 1,0, 1,1}));
      expected.push_back(IntArgs({0,1, 1,1, 1,1}));
      expected.push_back(IntArgs({1,0, 1,0, 1,0}));
      expected.push_back(IntArgs({1,0, 1,0, 1,1}));
      expected.push_back(IntArgs({1,0, 1,1, 1,1}));
      expected.push_back(IntArgs({1,1, 1,1, 1,1}));
      return expected;
    }
  };

  /// %Test for matrix symmetry
  class MatSym2 {
  public:
    /// Number of variables
    static const int n = 6;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 2, 3);
      Symmetries s;
      s << columns_interchange(m);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0, 0,0, 0,0}));
      expected.push_back(IntArgs({0,0, 0,0, 0,1}));
      expected.push_back(IntArgs({0,0, 0,0, 1,1}));
      expected.push_back(IntArgs({0,0, 0,1, 0,0}));
      expected.push_back(IntArgs({0,0, 0,1, 0,1}));
      expected.push_back(IntArgs({0,0, 0,1, 1,0}));
      expected.push_back(IntArgs({0,0, 0,1, 1,1}));
      expected.push_back(IntArgs({0,0, 1,1, 0,0}));
      expected.push_back(IntArgs({0,0, 1,1, 0,1}));
      expected.push_back(IntArgs({0,0, 1,1, 1,1}));
      expected.push_back(IntArgs({0,1, 0,0, 0,0}));
      expected.push_back(IntArgs({0,1, 0,0, 0,1}));
      expected.push_back(IntArgs({0,1, 0,0, 1,0}));
      expected.push_back(IntArgs({0,1, 0,0, 1,1}));
      expected.push_back(IntArgs({0,1, 0,1, 0,0}));
      expected.push_back(IntArgs({0,1, 0,1, 0,1}));
      expected.push_back(IntArgs({0,1, 0,1, 1,0}));
      expected.push_back(IntArgs({0,1, 0,1, 1,1}));
      expected.push_back(IntArgs({0,1, 1,0, 0,0}));
      expected.push_back(IntArgs({0,1, 1,0, 0,1}));
      expected.push_back(IntArgs({0,1, 1,0, 1,0}));
      expected.push_back(IntArgs({0,1, 1,0, 1,1}));
      expected.push_back(IntArgs({0,1, 1,1, 0,0}));
      expected.push_back(IntArgs({0,1, 1,1, 0,1}));
      expected.push_back(IntArgs({0,1, 1,1, 1,0}));
      expected.push_back(IntArgs({0,1, 1,1, 1,1}));
      expected.push_back(IntArgs({1,1, 0,0, 0,0}));
      expected.push_back(IntArgs({1,1, 0,0, 0,1}));
      expected.push_back(IntArgs({1,1, 0,0, 1,1}));
      expected.push_back(IntArgs({1,1, 0,1, 0,0}));
      expected.push_back(IntArgs({1,1, 0,1, 0,1}));
      expected.push_back(IntArgs({1,1, 0,1, 1,0}));
      expected.push_back(IntArgs({1,1, 0,1, 1,1}));
      expected.push_back(IntArgs({1,1, 1,1, 0,0}));
      expected.push_back(IntArgs({1,1, 1,1, 0,1}));
      expected.push_back(IntArgs({1,1, 1,1, 1,1}));
      return expected;
    }
  };

  /// %Test for matrix symmetry
  class MatSym3 {
  public:
    /// Number of variables
    static const int n = 6;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 2, 3);
      Symmetries s;
      s << rows_interchange(m);
      s << columns_interchange(m);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0, 0,0, 0,0}));
      expected.push_back(IntArgs({0,0, 0,0, 0,1}));
      expected.push_back(IntArgs({0,0, 0,0, 1,1}));
      expected.push_back(IntArgs({0,0, 0,1, 0,0}));
      expected.push_back(IntArgs({0,0, 0,1, 0,1}));
      expected.push_back(IntArgs({0,0, 0,1, 1,0}));
      expected.push_back(IntArgs({0,0, 0,1, 1,1}));
      expected.push_back(IntArgs({0,0, 1,1, 1,1}));
      expected.push_back(IntArgs({0,1, 0,0, 0,0}));
      expected.push_back(IntArgs({0,1, 0,0, 0,1}));
      expected.push_back(IntArgs({0,1, 0,0, 1,0}));
      expected.push_back(IntArgs({0,1, 0,0, 1,1}));
      expected.push_back(IntArgs({0,1, 0,1, 0,0}));
      expected.push_back(IntArgs({0,1, 0,1, 0,1}));
      expected.push_back(IntArgs({0,1, 0,1, 1,0}));
      expected.push_back(IntArgs({0,1, 0,1, 1,1}));
      expected.push_back(IntArgs({0,1, 1,0, 1,0}));
      expected.push_back(IntArgs({0,1, 1,0, 1,1}));
      expected.push_back(IntArgs({0,1, 1,1, 1,1}));
      expected.push_back(IntArgs({1,1, 1,1, 1,1}));
      return expected;
    }
  };

  /// %Test for matrix symmetry
  class MatSym4 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 1, 4);
      Symmetries s;
      s << rows_reflect(m);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0, 0, 0, 0}));
      expected.push_back(IntArgs({0, 0, 0, 1}));
      expected.push_back(IntArgs({0, 0, 1, 0}));
      expected.push_back(IntArgs({0, 0, 1, 1}));
      expected.push_back(IntArgs({0, 1, 0, 0}));
      expected.push_back(IntArgs({0, 1, 0, 1}));
      expected.push_back(IntArgs({0, 1, 1, 0}));
      expected.push_back(IntArgs({0, 1, 1, 1}));
      expected.push_back(IntArgs({1, 0, 0, 1}));
      expected.push_back(IntArgs({1, 0, 1, 1}));
      expected.push_back(IntArgs({1, 1, 1, 1}));
      return expected;
    }
  };

  /// %Test for variable sequence symmetry
  class SimIntVarSym1 {
  public:
    /// Number of variables
    static const int n = 12;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 3, 4);
      // The values in the first column are distinct.
      distinct(home, m.col(0));
      // Each row sums to 3.
      for (int i = 0 ; i < 4 ; ++i)
        linear(home, m.row(i), IRT_EQ, 3);

      // Rows are interchangeable.
      Symmetries s;
      s << VariableSequenceSymmetry(xs, 3);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,3, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,0,2, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,1,1, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,1,1, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,2,0, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,2,0, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,0,2, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,1,1, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,1,1, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,2,0, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,2,0, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,0,2, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,1,1, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,1,1, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,2,0, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,2,1, 1,2,0, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,0,2, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,1,1, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,1,1, 2,1,0, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,2,0, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,3,0, 1,2,0, 2,1,0, 3,0,0}));
      return expected;
    }
  };

  /// %Test for variable sequence symmetry
  class SimIntVarSym2 {
    /// Number of rows
    static const int nrows = 4;
    /// Number of columns
    static const int ncols = 3;
  public:
    /// Number of variables
    static const int n = nrows*ncols;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 3, 4);
      // The values in the first column are distinct.
      distinct(home, m.col(0));
      // Each row sums to 3.
      for (int i = 0 ; i < nrows ; ++i)
        linear(home, m.row(i), IRT_EQ, 3);

      Symmetries s;

      IntArgs a = IntArgs::create(n, 0);
      // Rows are interchangeable.
      s << VariableSequenceSymmetry(xs, 3);
      // Elements (i,1) and (i,2) in row i are interchangeable,
      // separately for each row.
      for (int i = 0 ; i < nrows ; i++) {
        IntVarArgs symvars;
        symvars << m(1,i) << m(2,i);
        s << VariableSymmetry(symvars);
      }
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,3, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,0,3, 1,1,1, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,0,2, 2,0,1, 3,0,0}));
      expected.push_back(IntArgs({0,1,2, 1,1,1, 2,0,1, 3,0,0}));
      return expected;
    }
  };

  /// %Test for value sequence symmetry
  class SimIntValSym1 {
  public:
    /// Number of variables
    static const int n = 2;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 6;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      rel(home, xs[0] + xs[1] == 6);
      // Values 0,1,2 are symmetric with 6,5,4.
      IntArgs values({0,1,2, 6,5,4});
      Symmetries s;
      s << ValueSequenceSymmetry(values, 3);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,6}));
      expected.push_back(IntArgs({1,5}));
      expected.push_back(IntArgs({2,4}));
      expected.push_back(IntArgs({3,3}));
      return expected;
    }
  };

  /// %Test for value sequence symmetry
  class SimIntValSym2 {
  public:
    /// Number of variables
    static const int n = 3;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 8;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      TupleSet tuples(3);
      tuples.add({1,1,1}).add({4,4,4}).add({7,7,7})
        .add({0,1,5}).add({0,1,8}).add({3,4,2})
        .add({3,4,8}).add({6,7,2}).add({6,7,5})
        .finalize();
      extensional(home, xs, tuples);

      // Values 0,1,2 are symmetric with 3,4,5, and with 6,7,8.
      IntArgs values({0,1,2, 3,4,5, 6,7,8});
      Symmetries s;
      s << ValueSequenceSymmetry(values, 3);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,5}));
      expected.push_back(IntArgs({1,1,1}));
      return expected;
    }
  };

  /// %Test for value sequence symmetry
  class SimIntValSym3 {
  public:
    /// Number of variables
    static const int n = 2;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 6;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      rel(home, xs[0] + xs[1] == 6);
      Symmetries s;
      // Values 0,1,2 are symmetric with 6,5,4.
      s << values_reflect(0,6);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MED(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({3,3}));
      expected.push_back(IntArgs({2,4}));
      expected.push_back(IntArgs({1,5}));
      expected.push_back(IntArgs({0,6}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym1 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      IntArgs indices({0,1,2,3});
      s << ValueSymmetry(indices);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym1b {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      s << ValueSymmetry(xs[0]);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym1c {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      s << ValueSymmetry(xs[0]);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MAX(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({3,2,1,0}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym2 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries s;
      IntArgs indices({0,1,2,3});
      s << ValueSymmetry(indices);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,0,0}));
      expected.push_back(IntArgs({0,0,0,1}));
      expected.push_back(IntArgs({0,0,1,0}));
      expected.push_back(IntArgs({0,0,1,1}));
      expected.push_back(IntArgs({0,0,1,2}));
      expected.push_back(IntArgs({0,1,0,0}));
      expected.push_back(IntArgs({0,1,0,1}));
      expected.push_back(IntArgs({0,1,0,2}));
      expected.push_back(IntArgs({0,1,1,0}));
      expected.push_back(IntArgs({0,1,1,1}));
      expected.push_back(IntArgs({0,1,1,2}));
      expected.push_back(IntArgs({0,1,2,0}));
      expected.push_back(IntArgs({0,1,2,1}));
      expected.push_back(IntArgs({0,1,2,2}));
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym2b {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries s;
      s << ValueSymmetry(xs[0]);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,0,0}));
      expected.push_back(IntArgs({0,0,0,1}));
      expected.push_back(IntArgs({0,0,1,0}));
      expected.push_back(IntArgs({0,0,1,1}));
      expected.push_back(IntArgs({0,0,1,2}));
      expected.push_back(IntArgs({0,1,0,0}));
      expected.push_back(IntArgs({0,1,0,1}));
      expected.push_back(IntArgs({0,1,0,2}));
      expected.push_back(IntArgs({0,1,1,0}));
      expected.push_back(IntArgs({0,1,1,1}));
      expected.push_back(IntArgs({0,1,1,2}));
      expected.push_back(IntArgs({0,1,2,0}));
      expected.push_back(IntArgs({0,1,2,1}));
      expected.push_back(IntArgs({0,1,2,2}));
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym3 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      IntArgs indices({0,1});
      s << ValueSymmetry(indices);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      expected.push_back(IntArgs({0,1,3,2}));
      expected.push_back(IntArgs({0,2,1,3}));
      expected.push_back(IntArgs({0,2,3,1}));
      expected.push_back(IntArgs({0,3,1,2}));
      expected.push_back(IntArgs({0,3,2,1}));
      expected.push_back(IntArgs({2,0,1,3}));
      expected.push_back(IntArgs({2,0,3,1}));
      expected.push_back(IntArgs({2,3,0,1}));
      expected.push_back(IntArgs({3,0,1,2}));
      expected.push_back(IntArgs({3,0,2,1}));
      expected.push_back(IntArgs({3,2,0,1}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym4 {
  public:
    /// Number of variables
    static const int n = 3;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 2;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      IntArgs indices({0});
      s << ValueSymmetry(indices);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2}));
      expected.push_back(IntArgs({0,2,1}));
      expected.push_back(IntArgs({1,0,2}));
      expected.push_back(IntArgs({1,2,0}));
      expected.push_back(IntArgs({2,0,1}));
      expected.push_back(IntArgs({2,1,0}));
      return expected;
    }
  };

  /// %Test for value symmetry
  class ValSym5 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      IntArgs indices0({0,1});
      IntArgs indices1({2,3});
      s << ValueSymmetry(indices0);
      s << ValueSymmetry(indices1);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      expected.push_back(IntArgs({0,2,1,3}));
      expected.push_back(IntArgs({0,2,3,1}));
      expected.push_back(IntArgs({2,0,1,3}));
      expected.push_back(IntArgs({2,0,3,1}));
      expected.push_back(IntArgs({2,3,0,1}));
      return expected;
    }
  };

  /// %Test for variable and value symmetry
  class VarValSym1 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries s;
      s << VariableSymmetry(xs);
      s << ValueSymmetry(IntArgs::create(4,0));
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,0,0}));
      expected.push_back(IntArgs({0,0,0,1}));
      expected.push_back(IntArgs({0,0,1,1}));
      expected.push_back(IntArgs({0,0,1,2}));
      expected.push_back(IntArgs({0,1,1,1}));
      expected.push_back(IntArgs({0,1,1,2}));
      expected.push_back(IntArgs({0,1,2,2}));  // This solution is symmetric to the previous one.
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

  /// %Test for %LDSB infrastructure with %Latin square problem
  class LDSBLatin : public Base {
  public:
    /// %Latin square space
    class Latin : public Space {
    public:
      IntVarArray xs;
      Latin(int n = 4) : xs(*this, n*n, 1, n)
      {
        Matrix<IntVarArray> m(xs, n, n);
        for (int i = 0 ; i < n ; i++) {
          distinct(*this, m.col(i));
          distinct(*this, m.row(i));
        }
        Symmetries s;
        s << rows_interchange(m);
        s << columns_interchange(m);
        s << ValueSymmetry(IntSet(1,n));
        branch(*this, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
      }
      // Search support.
      Latin(Latin& s) : Space(s)
      { xs.update(*this, s.xs); }
      virtual Space* copy(void)
      { return new Latin(*this); }
      IntArgs solution(void) {
        IntArgs a(xs.size());
        for (int i = 0 ; i < a.size() ; ++i)
          a[i] = xs[i].val();
        return a;
      }

    /// Compute list of expected solutions
      static std::vector<IntArgs> expectedSolutions(void) {
        static std::vector<IntArgs> expected;
        expected.clear();
        expected.push_back(IntArgs({1,2,3,4, 2,1,4,3, 3,4,1,2, 4,3,2,1}));
        expected.push_back(IntArgs({1,2,3,4, 2,1,4,3, 3,4,2,1, 4,3,1,2}));
        expected.push_back(IntArgs({1,2,3,4, 2,3,4,1, 3,4,1,2, 4,1,2,3}));
        expected.push_back(IntArgs({1,2,3,4, 2,4,1,3, 3,1,4,2, 4,3,2,1}));
        return expected;
      }
    };
    /// Initialize test
    LDSBLatin(std::string label) : Test::Base("LDSB::" + label) {}
    /// Perform actual tests
    bool run(void) {
      Latin *s = new Latin();
      DFS<Latin> e(s);
      bool r = check(e, Latin::expectedSolutions());
      delete s;
      return r;
    }
  };

  /* This test should fail if the recomputation-handling does not work
   * properly.
   *
   * Why recomputation can be a problem
   * ==================================
   *
   * Every branch point in LDSB is binary, with a left and a right
   * branch.  Whenever backtracking happens -- when a right branch is
   * explored -- LDSB computes a set of symmetric literals to
   * exclude.
   *
   * !!! This calculation may depend on the current domains of the
   * !!! variables.
   *
   * During recomputation, parts of the search tree are replayed.  To
   * be specific, the branching constraints are posted, but no
   * propagation happens.  This means that at a given branch point,
   * the domains during recomputation may be different (weaker) than
   * they were the first time during search.
   *
   * !!! This *cannot* cause solutions to be missed --- LDSB will not
   * !!! be incorrect --- but it *does* change what will be pruned.
   *
   * If recomputation is not handled properly, the difference in
   * domains will cause extra solutions to be found.  This is a result
   * of symmetries failing to be broken.
   *
   */

  /// %Test for handling of recomputation
  class Recomputation {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      TupleSet t(2);
      t.add({0,0}).add({1,1}).finalize();
      IntVarArgs va;
      va << xs[0] << xs[2];
      extensional(home, va, t);
      Symmetries syms;
      syms << VariableSequenceSymmetry(xs, 2);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0,0,0}));
      expected.push_back(IntArgs({0,0,0,1}));

      // This is the solution that will be found if recomputation is
      // not handled.  After branching on x[0]=0, we try x[1]=0.  When
      // x[1]=0 backtracks, the symmetry [x[0],x[1]] <-> [x[2],x[3]]
      // is active --- but only after propagation!  (Without
      // propagation, we do not have x[2]=0.)  If propagation happens,
      // we know that symmetry is active and we can post x[3]!=0.  If
      // it doesn't, we don't use the symmetry and we find a solution
      // where x[3]=0.

      // expected.push_back(IntArgs({0,1,0,0}));

      expected.push_back(IntArgs({0,1,0,1}));

      expected.push_back(IntArgs({1,0,1,0}));
      expected.push_back(IntArgs({1,0,1,1}));
      expected.push_back(IntArgs({1,1,1,1}));
      return expected;
    }
  };

  double position(const Space& home, IntVar x, int i) {
    (void) home;
    (void) x;
    return i;
  }

  /// %Test tiebreaking variable heuristic.
  class TieBreak {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries syms;
      IntArgs indices({0,1,2,3});
      syms << VariableSymmetry(xs, indices);
      distinct(home, xs);
      // This redundant constraint is to trick the variable
      // heuristic.
      rel(home, xs[1] != xs[2]);
      // xs[1] and xs[2] have higher degree than the others, so they
      // are considered first.  xs[2] is higher than x[1] by the merit
      // function, so it is assigned first.  Now all remaining
      // variables have the same degree, so they are searched in
      // reverse order (according to the merit function).  So, the
      // solution found is {3, 2, 0, 1}.
      branch(home, xs, tiebreak(INT_VAR_DEGREE_MAX(), INT_VAR_MERIT_MAX(position)), INT_VAL_MIN(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({3,2,0,1}));
      return expected;
    }
  };

#ifdef GECODE_HAS_SET_VARS
  /// Convenient way to make IntSetArgs
  IntSetArgs ISA(int n, ...) {
    IntSetArgs sets;
    va_list args;
    va_start(args, n);
    int i = 0;
    IntArgs a;
    while (i < n) {
      int x = va_arg(args,int);
      if (x == -1) {
        i++;
        sets << IntSet(a);
        a = IntArgs();
      } else {
        a << x;
      }
    }
    va_end(args);
    return sets;
  }

  /// %Test for set variable symmetry
  class SetVarSym1 {
  public:
    /// Number of variables
    static const int n = 2;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, SetVarArray& xs) {
      Symmetries syms;
      syms << VariableSymmetry(xs);
      branch(home, xs, SET_VAR_NONE(), SET_VAL_MIN_INC(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntSetArgs> expectedSolutions(void) {
      static std::vector<IntSetArgs> expected;
      expected.clear();
      expected.push_back(ISA(2, 0,1,-1, 0,1,-1));
      expected.push_back(ISA(2, 0,1,-1, 0,  -1));
      expected.push_back(ISA(2, 0,1,-1,   1,-1));
      expected.push_back(ISA(2, 0,1,-1,     -1));
      expected.push_back(ISA(2, 0,  -1, 0,1,-1));
      expected.push_back(ISA(2, 0,  -1, 0,  -1));
      expected.push_back(ISA(2, 0,  -1,   1,-1));
      expected.push_back(ISA(2, 0,  -1,     -1));
      // expected.push_back(ISA(2,   1,-1, 0,1,-1));
      // expected.push_back(ISA(2,   1,-1, 0,  -1));
      expected.push_back(ISA(2,   1,-1,   1,-1));
      expected.push_back(ISA(2,   1,-1,     -1));
      // expected.push_back(ISA(2,     -1, 0,1,-1));
      // expected.push_back(ISA(2,     -1, 0,  -1));
      // expected.push_back(ISA(2,     -1,   1,-1));
      expected.push_back(ISA(2,     -1,     -1));
      return expected;
    }
  };

  /*
   * This tests the special handling of value symmetries on set
   * values.  Look at the third solution (commented out) below.  The
   * first variable has been assigned to {0,1}.  If the value symmetry
   * is not handled specially, then we will consider the value
   * symmetry broken because the search has touched each value.
   * However, because both values have been assigned to the same
   * variable, 0 and 1 are still symmetric.  Therefore, the third
   * solution is symmetric to the second one and should be excluded.
   */

  /// %Test for set value symmetry
  class SetValSym1 {
  public:
    /// Number of variables
    static const int n = 2;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, SetVarArray& xs) {
      Symmetries syms;
      syms << ValueSymmetry(IntArgs({0,1}));
      branch(home, xs, SET_VAR_NONE(), SET_VAL_MIN_INC(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntSetArgs> expectedSolutions(void) {
      static std::vector<IntSetArgs> expected;
      expected.clear();
      expected.push_back(ISA(2, 0,1,-1, 0,1,-1));
      expected.push_back(ISA(2, 0,1,-1, 0,  -1));
      // expected.push_back(ISA(2, 0,1,-1,   1,-1)); // XXXXX bad solution
      expected.push_back(ISA(2, 0,1,-1,     -1));
      expected.push_back(ISA(2, 0,  -1, 0,1,-1));
      expected.push_back(ISA(2, 0,  -1, 0,  -1));
      expected.push_back(ISA(2, 0,  -1,   1,-1));
      expected.push_back(ISA(2, 0,  -1,     -1));
      // expected.push_back(ISA(2,   1,-1, 0,1,-1));
      // expected.push_back(ISA(2,   1,-1, 0,  -1));
      // expected.push_back(ISA(2,   1,-1,   1,-1));
      // expected.push_back(ISA(2,   1,-1,     -1));
      expected.push_back(ISA(2,     -1, 0,1,-1));
      expected.push_back(ISA(2,     -1, 0,  -1));
      // expected.push_back(ISA(2,     -1,   1,-1));
      expected.push_back(ISA(2,     -1,     -1));
      return expected;
    }
  };

  /// %Test for set value symmetry
  class SetValSym2 {
  public:
    /// Number of variables
    static const int n = 3;
    /// Lower bound of values
    static const int l = 1;
    /// Upper bound of values
    static const int u = 4;
    /// Setup problem constraints and symmetries
    static void setup(Home home, SetVarArray& xs) {
      Symmetries syms;
      syms << ValueSymmetry(IntArgs({1,2,3,4}));
      for (int i = 0 ; i < 3 ; i++)
        cardinality(home, xs[i], 1, 1);
      branch(home, xs, SET_VAR_NONE(), SET_VAL_MIN_INC(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntSetArgs> expectedSolutions(void) {
      static std::vector<IntSetArgs> expected;
      expected.clear();
      expected.push_back(ISA(3, 1,-1, 1,-1, 1,-1));
      expected.push_back(ISA(3, 1,-1, 1,-1, 2,-1));
      expected.push_back(ISA(3, 1,-1, 2,-1, 1,-1));
      expected.push_back(ISA(3, 1,-1, 2,-1, 2,-1));
      expected.push_back(ISA(3, 1,-1, 2,-1, 3,-1));
      return expected;
    }
  };

  /// %Test for set variable sequence symmetry
  class SetVarSeqSym1 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 1;
    /// Setup problem constraints and symmetries
    static void setup(Home home, SetVarArray& xs) {
      Symmetries syms;
      syms << VariableSequenceSymmetry(xs,2);
      rel(home, xs[0], SOT_INTER, xs[1], SRT_EQ, IntSet::empty);
      rel(home, xs[2], SOT_INTER, xs[3], SRT_EQ, IntSet::empty);
      for (int i = 0 ; i < 4 ; i++)
        cardinality(home, xs[i], 1, 1);
      branch(home, xs, SET_VAR_NONE(), SET_VAL_MIN_INC(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntSetArgs> expectedSolutions(void) {
      static std::vector<IntSetArgs> expected;
      expected.clear();
      expected.push_back(ISA(4, 0,-1, 1,-1, 0,-1, 1,-1));
      expected.push_back(ISA(4, 0,-1, 1,-1, 1,-1, 0,-1));
      //      expected.push_back(ISA(4, 1,-1, 0,-1, 0,-1, 1,-1));
      expected.push_back(ISA(4, 1,-1, 0,-1, 1,-1, 0,-1));
      return expected;
    }
  };

  /// %Test for set variable sequence symmetry
  class SetVarSeqSym2 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 0;
    /// Setup problem constraints and symmetries
    static void setup(Home home, SetVarArray& xs) {
      Symmetries syms;
      syms << VariableSequenceSymmetry(xs,2);
      rel(home, xs[0], SRT_EQ, xs[2]);
      branch(home, xs, SET_VAR_NONE(), SET_VAL_MIN_INC(), syms);
    }
    /// Compute list of expected solutions
    static std::vector<IntSetArgs> expectedSolutions(void) {
      static std::vector<IntSetArgs> expected;
      expected.clear();

      // Symmetric solutions are commented out.
      expected.push_back(ISA(4, 0, -1,0,-1,0,-1,0,-1));
      expected.push_back(ISA(4, 0, -1,0,-1,0,-1,  -1));
      // expected.push_back(ISA(4, 0, -1,0,-1,  -1,0,-1));
      // expected.push_back(ISA(4, 0, -1,0,-1,  -1,  -1));
      // expected.push_back(ISA(4, 0, -1,  -1,0,-1,0,-1));
      expected.push_back(ISA(4, 0, -1,  -1,0,-1,  -1));
      // expected.push_back(ISA(4, 0, -1,  -1,  -1,0,-1));
      // expected.push_back(ISA(4, 0, -1,  -1,  -1,  -1));
      // expected.push_back(ISA(4,    -1,0,-1,0,-1,0,-1));
      // expected.push_back(ISA(4,    -1,0,-1,0,-1,  -1));
      expected.push_back(ISA(4,    -1,0,-1,  -1,0,-1));
      expected.push_back(ISA(4,    -1,0,-1,  -1,  -1));
      // expected.push_back(ISA(4,    -1,  -1,0,-1,0,-1));
      // expected.push_back(ISA(4,    -1,  -1,0,-1,  -1));
      // expected.push_back(ISA(4,    -1,  -1,  -1,0,-1));
      expected.push_back(ISA(4,    -1,  -1,  -1,  -1));

      return expected;
    }
  };

  /// %Test for reflection symmetry
  class ReflectSym1 {
  public:
    /// Number of variables
    static const int n = 6;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 6;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Matrix<IntVarArray> m(xs, 3, 2);

      distinct(home, xs);
      rel(home, abs(m(0,0)-m(1,0))==1);
      rel(home, abs(m(0,1)-m(1,1))==1);
      rel(home, abs(m(1,0)-m(2,0))==1);
      rel(home, abs(m(1,1)-m(2,1))==1);

      Symmetries s;
      s << values_reflect(l, u);
      s << rows_interchange(m);
      s << columns_reflect(m);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3,4,5}));
      expected.push_back(IntArgs({0,1,2,4,5,6}));
      expected.push_back(IntArgs({0,1,2,5,4,3}));
      expected.push_back(IntArgs({0,1,2,6,5,4}));
      return expected;
    }
  };

    /// %Test for reflection symmetry
  class ReflectSym2 {
  public:
    /// Number of variables
    static const int n = 2;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      Symmetries s;
      s << values_reflect(l, u);
      branch(home, xs, INT_VAR_NONE(), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,0}));
      expected.push_back(IntArgs({0,1}));
      expected.push_back(IntArgs({0,2}));
      expected.push_back(IntArgs({0,3}));
      expected.push_back(IntArgs({1,0}));
      expected.push_back(IntArgs({1,1}));
      expected.push_back(IntArgs({1,2}));
      expected.push_back(IntArgs({1,3}));
      return expected;
    }
  };

  /// %Test with action
  class Action1 {
  public:
    /// Number of variables
    static const int n = 4;
    /// Lower bound of values
    static const int l = 0;
    /// Upper bound of values
    static const int u = 3;
    /// Setup problem constraints and symmetries
    static void setup(Home home, IntVarArray& xs) {
      distinct(home, xs);
      Symmetries s;
      s << VariableSymmetry(xs);
      s << ValueSymmetry(IntArgs::create(4,0));
      branch(home, xs, INT_VAR_ACTION_MIN(0.8), INT_VAL_MIN(), s);
    }
    /// Compute list of expected solutions
    static std::vector<IntArgs> expectedSolutions(void) {
      static std::vector<IntArgs> expected;
      expected.clear();
      expected.push_back(IntArgs({0,1,2,3}));
      return expected;
    }
  };

#endif

  LDSB<VarSym1> varsym1("VarSym1");
  LDSB<VarSym1b> varsym1b("VarSym1b");
  LDSB<VarSym2> varsym2("VarSym2");
  LDSB<VarSym3> varsym3("VarSym3");
  LDSB<VarSym4> varsym4("VarSym4");
  LDSB<VarSym5> varsym5("VarSym5");
  LDSB<MatSym1> matsym1("MatSym1");
  LDSB<MatSym2> matsym2("MatSym2");
  LDSB<MatSym3> matsym3("MatSym3");
  LDSB<MatSym4> matsym4("MatSym4");
  LDSB<SimIntVarSym1> simintvarsym1("SimIntVarSym1");
  LDSB<SimIntVarSym2> simintvarsym2("SimIntVarSym2");
  LDSB<SimIntValSym1> simintvalsym1("SimIntValSym1");
  LDSB<SimIntValSym2> simintvalsym2("SimIntValSym2");
  LDSB<SimIntValSym3> simintvalsym3("SimIntValSym3");
  LDSB<ValSym1> valsym1("ValSym1");
  LDSB<ValSym1b> valsym1b("ValSym1b");
  LDSB<ValSym1c> valsym1c("ValSym1c");
  LDSB<ValSym2> valsym2("ValSym2");
  LDSB<ValSym2b> valsym2b("ValSym2b");
  LDSB<ValSym3> valsym3("ValSym3");
  LDSB<ValSym4> valsym4("ValSym4");
  LDSB<ValSym5> valsym5("ValSym5");
  LDSB<VarValSym1> varvalsym1("VarValSym1");
  LDSBLatin latin("Latin");
  LDSB<Recomputation> recomp("Recomputation", 999,999);
  LDSB<TieBreak> tiebreak("TieBreak");

#ifdef GECODE_HAS_SET_VARS
  LDSB<ReflectSym1> reflectsym1("ReflectSym1");
  LDSB<ReflectSym2> reflectsym2("ReflectSym2");
  LDSB<Action1> action1("Action1");

  LDSBSet<SetVarSym1> setvarsym1("SetVarSym1");
  LDSBSet<SetValSym1> setvalsym1("SetValSym1");
  LDSBSet<SetValSym2> setvalsym2("SetValSym2", 0, 1);
  LDSBSet<SetVarSeqSym1> setvarseqsym1("SetVarSeqSym1");
  LDSBSet<SetVarSeqSym2> setvarseqsym2("SetVarSeqSym2");
#endif
}}

// STATISTICS: test-core
