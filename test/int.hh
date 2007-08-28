/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

#ifndef __GECODE_TEST_INT_HH__
#define __GECODE_TEST_INT_HH__

#include "gecode/int.hh"
#include "test.hh"

/**
 * \defgroup TaskTestInt Testing finite domain integers
 * \ingroup TaskTest
 */

/**
 * \defgroup TaskTestIntInt General test support
 * \ingroup TaskTestInt
 */
//@{
/// Base class for assignments
class Assignment {
protected:
  int n;            ///< Number of variables
  Gecode::IntSet d; ///< Domain for each variable
public:
  /// Initialize assignments for \a n0 variables and values \a d0
  Assignment(int n0, const Gecode::IntSet& d0);
  /// Test whether all assignments have been iterated
  virtual bool operator()(void) const = 0;
  /// Move to next assignment
  virtual void operator++(void) = 0;
  /// Return value for variable \a i
  virtual int operator[](int i) const = 0;
  /// Return number of variables
  int size(void) const;
  /// Destructor
  virtual ~Assignment(void);
};

/// Generate all assignments
class CpltAssignment : public Assignment {
protected:
  Gecode::IntSetValues* dsv; ///< Iterator for each variable
public:
  /// Initialize assignments for \a n0 variables and values \a d0
  CpltAssignment(int n, const Gecode::IntSet& d);
  /// Test whether all assignments have been iterated
  virtual bool operator()(void) const;
  /// Move to next assignment
  virtual void operator++(void);
  /// Return value for variable \a i
  virtual int operator[](int i) const;
  /// Destructor
  virtual ~CpltAssignment(void);
};

/// Generate random selection of assignments
class RandomAssignment : public Assignment {
protected:
  int* vals; ///< The current values for the variables
  int  a;    ///< How many assigments still to be generated
  /// Generate new value according to domain
  int randval(void);
public:
  /// Initialize for \a a assignments for \a n0 variables and values \a d0
  RandomAssignment(int n, const Gecode::IntSet& d, int a);
  /// Test whether all assignments have been iterated
  virtual bool operator()(void) const;
  /// Move to next assignment
  virtual void operator++(void);
  /// Return value for variable \a i
  virtual int operator[](int i) const;
  /// Destructor
  virtual ~RandomAssignment(void);
};

/**
 * \brief Print assignment \a
 * \relates Assignment
 */
std::ostream& operator<<(std::ostream& os, const Assignment& a);

/**
 * \brief Base class for tests with integer constraints
 *
 */
class IntTest : public TestBase {
protected:
  /// Number of variables
  int arity;
  /// Domain of variables
  Gecode::IntSet dom;
  /// Does the constraint also exist as reified constraint
  bool reified;
  /// Consistency level of the propagator
  Gecode::IntConLevel icl;
  /// Whether to test for domain-consistency
  bool testdomcon;
  /// Whether to perform search test
  bool testsearch;

public:
  /**
   * \brief Constructor
   *
   * Constructs a test with name \a t and arity \a a and variable
   * domain \a d. Also tests for a reified constraint, 
   * if \a r is true. The consistency level \a is maintained for
   * convenience.
   */
  IntTest(const std::string& s, int a, const Gecode::IntSet& d, 
          bool r=false, Gecode::IntConLevel i=Gecode::ICL_DEF);
  /**
   * \brief Constructor
   *
   * Constructs a test with name \a t and arity \a a and variable
   * domain \a min ... \a max. Also tests for a reified constraint, 
   * if \a r is true. The consistency level \a is maintained for
   * convenience.
   */
  IntTest(const std::string& s, int a, int min, int max, 
          bool r=false, Gecode::IntConLevel i=Gecode::ICL_DEF);
  /// Create assignment
  virtual Assignment* assignment(void) const;
  /// Check for solution
  virtual bool solution(const Assignment&) const = 0;
  /// Post propagator
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) = 0;
  /// Post reified propagator
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                    Gecode::BoolVar b);
  /// Perform test
  virtual bool run(const Options& opt);
  /// Map integer consistency level to string representation
  static std::string icl2str(Gecode::IntConLevel icl, bool verbose=false);
  /// Map integer to string
  static std::string str(int i);
};
//@}

#include "test/int.icc"

#endif

// STATISTICS: test-int

