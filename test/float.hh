/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
 *     Vincent Barichard, 2012
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

#ifndef __GECODE_TEST_FLOAT_HH__
#define __GECODE_TEST_FLOAT_HH__

#include "test/test.hh"

#include <gecode/float.hh>

namespace Test {

  /// Testing domain floats
  namespace Float {

    /**
     * \defgroup TaskTestFloat Testing domain floats
     * \ingroup TaskTest
     */

    /**
     * \defgroup TaskTestFloatFloat General test support
     * \ingroup TaskTestFloat
     */
    //@{
    /// %Base class for assignments
    class Assignment {
    protected:
      int n;              ///< Number of variables
      Gecode::FloatVal d; ///< Domain for each variable
    public:
      /// Initialize assignments for \a n0 variables and values \a d0
      Assignment(int n0, const Gecode::FloatVal& d0);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const = 0;
      /// Move to next assignment
      virtual void operator++(void) = 0;
      /// Return value for variable \a i
      virtual Gecode::FloatNum operator[](int i) const = 0;
      /// Return number of variables
      int size(void) const;
      /// Destructor
      virtual ~Assignment(void);
    };

    /// Generate all assignments
    class CpltAssignment : public Assignment {
    protected:
      Gecode::FloatNum* dsv; ///< Iterator for each variable
      Gecode::FloatNum step; ///< Step for next assignment
    public:
      /// Initialize assignments for \a n variables and values \a d with step \a s
      CpltAssignment(int n, const Gecode::FloatVal& d, Gecode::FloatNum s);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const;
      /// Move to next assignment
      virtual void operator++(void);
      /// Return value for variable \a i
      virtual Gecode::FloatNum operator[](int i) const;
      /// Destructor
      virtual ~CpltAssignment(void);
    };


    /// Generate random selection of assignments
    class RandomAssignment : public Assignment {
    protected:
      Gecode::FloatNum* vals; ///< The current values for the variables
      int  a;                 ///< How many assigments still to be generated
      /// Generate new value according to domain
      Gecode::FloatNum randval(void);
    public:
      /// Initialize for \a a assignments for \a n variables and values \a d
      RandomAssignment(int n, const Gecode::FloatVal& d, int a);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const;
      /// Move to next assignment
      virtual void operator++(void);
      /// Return value for variable \a i
      virtual Gecode::FloatNum operator[](int i) const;
      /// Destructor
      virtual ~RandomAssignment(void);
    };

    class Test;

    /// Space for executing tests
    class TestSpace : public Gecode::Space {
    public:
      /// Initial domain
      Gecode::FloatVal d;
      /// Step for going to next solution
      Gecode::FloatNum step;
      /// Variables to be tested
      Gecode::FloatVarArray x;
      /// Reification information
      Gecode::Reify r;
      /// The test currently run
      Test* test;
      /// Whether the test is for a reified propagator
      bool reified;

      /**
       * \brief Create test space
       *
       * Creates \a n variables with domain \a d and step \a s for test \a t and stores
       * whether the test is for a reified propagator (\a re) with
       * reification mode \a rm.
       *
       */
      TestSpace(int n, Gecode::FloatVal& d, Gecode::FloatNum s, Test* t, 
                bool re, Gecode::ReifyMode rm = Gecode::RM_EQV);
      /// Constructor for cloning \a s
      TestSpace(bool share, TestSpace& s);
      /// Copy space during cloning
      virtual Gecode::Space* copy(bool share);
      /// Add constraints to skip solutions to the next assignment
      virtual void next_as(const Space& s);
      /// Test whether all variables are assigned
      bool assigned(void) const;
      /// Post propagator
      void post(void);
      /// Compute a fixpoint and check for failure
      bool failed(void);
      /// Perform integer tell operation on \a x[i]
      void rel(int i, Gecode::FloatRelType frt, Gecode::FloatNum n);
      /// Perform Boolean tell on \a b
      void rel(bool sol);
      /// Assign all (or all but one, if \a skip is true) variables to values in \a a
      void assign(const Assignment& a, bool skip=false);
      /// Assing a random variable to a random bound
      void bound(void);
      /// Prune some random values from variable \a i
      void prune(int i);
      /// Prune some random values for some random variable
      void prune(void);
      /// Prune values but not those in assignment \a a
      bool prune(const Assignment& a, bool testfix);
      /// \name Mapping scalar values to strings
      //@{
      /// Map reification mode to string
      static std::string str(Gecode::ReifyMode rm);
      //@}
    };

    /**
     * \brief %Base class for tests with float constraints
     *
     */
    class Test : public Base {
    protected:
      /// Number of variables
      int arity;
      /// Domain of variables
      Gecode::FloatVal dom;
      /// Step for going to next solution
      Gecode::FloatNum step;
      /// Does the constraint also exist as reified constraint
      bool reified;
      /// Whether to perform search test
      bool testsearch;
      /// Whether to perform fixpoint test
      bool testfix;

    public:
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s and arity \a a and variable
       * domain \a d and step \a st. Also tests for a reified constraint,
       * if \a r is true.
       */
      Test(const std::string& s, int a, const Gecode::FloatVal& d, Gecode::FloatNum st,
           bool r);
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s and arity \a a and variable
       * domain \a min ... \a max and step \a st. Also tests for a reified constraint,
       * if \a r is true.
       */
      Test(const std::string& s, int a, Gecode::FloatNum min, Gecode::FloatNum max, Gecode::FloatNum st,
           bool r);
      /// Create assignment
      virtual Assignment* assignment(void) const;
      /// Check for solution
      virtual bool solution(const Assignment&) const = 0;
      /// Whether to ignore assignment for reification
      virtual bool ignore(const Assignment&) const;
      /// Post constraint
      virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) = 0;
      /// Post reified constraint
      virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                        Gecode::Reify r);
      /// Perform test
      virtual bool run(void);
      /// \name Mapping scalar values to strings
      //@{
      /// Map extensional propagation kind to string
      static std::string str(Gecode::ExtensionalPropKind epk);
      /// Map float relation to string
      static std::string str(Gecode::FloatRelType frt);
      /// Map float to string
      static std::string str(Gecode::FloatNum f);
      /// Map float array to string
      static std::string str(const Gecode::FloatArgs& f);
      //@}
      /// \name General support
      //@{
      /// Compare \a x and \a y with respect to \a r
      template<class T> static bool cmp(T x, Gecode::FloatRelType r, T y);
      //@}
    };
    //@}

  /// Iterator for reification modes
  class ReifyModes {
  protected:
    /// Array of reification modes
    Gecode::ReifyMode rms[3];
    /// Current position in mode array
    int i;
  public:
    /// Initialize iterator with reification mode \a rm
    ReifyModes(Gecode::ReifyMode rm);
    /// Initialize iterator with reification modes \a rm0 and \a rm1
    ReifyModes(Gecode::ReifyMode rm0, Gecode::ReifyMode rm1);
    /// Initialize iterator with all three reification modes
    ReifyModes(void);
    /// Test whether iterator is done
    bool operator()(void) const;
    /// Increment to next reification mode
    void operator++(void);
    /// Return current reification mode
    Gecode::ReifyMode rm(void) const;
    };

    /// Iterator for float relation types
    class FloatRelTypes {
    private:
      /// Array of relation types
      static const Gecode::FloatRelType frts[3];
      /// Current position in relation type array
      int i;
    public:
      /// Initialize iterator
      FloatRelTypes(void);
      /// Reset iterator
      void reset(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next relation type
      void operator++(void);
      /// Return current relation type
      Gecode::FloatRelType frt(void) const;
    };

  }
}

/**
 * \brief Print assignment \a
 * \relates Assignment
 */
std::ostream& operator<<(std::ostream& os, const Test::Float::Assignment& a);

#include "test/float.hpp"

#endif

// STATISTICS: test-float

