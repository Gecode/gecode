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

    /// Type for comparisons and solutions
    enum MaybeType {
      MT_FALSE = 0, //< Does hold
      MT_TRUE,      //< Does not hold
      MT_MAYBE      //< Might or might not hold
    };

    /// Three-valued conjunction of MaybeType
    MaybeType operator &(MaybeType a, MaybeType b);

    /// Assignment possible types
    enum AssignmentType {
      CPLT_ASSIGNMENT = 0,
      RANDOM_ASSIGNMENT,
      EXTEND_ASSIGNMENT
    };

    class Test;

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
      virtual Gecode::FloatVal operator[](int i) const = 0;
      /// Set assignment to value \a val for variable \a i
      virtual void set(int i, const Gecode::FloatVal& val) = 0;
      /// Return number of variables
      int size(void) const;
      /// Destructor
      virtual ~Assignment(void);
    };

    /// Generate all assignments
    class CpltAssignment : public Assignment {
    protected:
      Gecode::FloatVal* dsv; ///< Iterator for each variable
      Gecode::FloatNum step; ///< Step for next assignment
    public:
      /// Initialize assignments for \a n variables and values \a d with step \a s
      CpltAssignment(int n, const Gecode::FloatVal& d, Gecode::FloatNum s);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const;
      /// Move to next assignment
      virtual void operator++(void);
      /// Return value for variable \a i
      virtual Gecode::FloatVal operator[](int i) const;
      /// Set assignment to value \a val for variable \a i
      virtual void set(int i, const Gecode::FloatVal& val);
      /// Destructor
      virtual ~CpltAssignment(void);
    };

    /// Generate all assignments except the last variable and complete it to get a solution
    class ExtAssignment : public Assignment {
    protected:
      const Test* curPb;     ///< Current problem used to complete assignment
      Gecode::FloatVal* dsv; ///< Iterator for each variable
      Gecode::FloatNum step; ///< Step for next assignment
    public:
      /// Initialize assignments for \a n variables and values \a d with step \a s
      ExtAssignment(int n, const Gecode::FloatVal& d, Gecode::FloatNum s, const Test * pb);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const;
      /// Move to next assignment
      virtual void operator++(void);
      /// Return value for variable \a i
      virtual Gecode::FloatVal operator[](int i) const;
      /// Set assignment to value \a val for variable \a i
      virtual void set(int i, const Gecode::FloatVal& val);
      /// Destructor
      virtual ~ExtAssignment(void);
    };


    /// Generate random selection of assignments
    class RandomAssignment : public Assignment {
    protected:
      Gecode::FloatVal* vals; ///< The current values for the variables
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
      virtual Gecode::FloatVal operator[](int i) const;
      /// Set assignment to value \a val for variable \a i
      virtual void set(int i, const Gecode::FloatVal& val);
      /// Destructor
      virtual ~RandomAssignment(void);
    };

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
       * Creates \a n variables with domain \a d and step \a s for
       * test \a t.
       *
       */
      TestSpace(int n, Gecode::FloatVal& d, Gecode::FloatNum s, Test* t);
      /**
       * \brief Create test space
       *
       * Creates \a n variables with domain \a d and step \a s for
       * test \a t annd reification mode \a rm.
       *
       */
      TestSpace(int n, Gecode::FloatVal& d, Gecode::FloatNum s, Test* t,
                Gecode::ReifyMode rm);
      /// Constructor for cloning \a s
      TestSpace(TestSpace& s);
      /// Copy space during cloning
      virtual Gecode::Space* copy(void);
      /// Add constraints to skip solutions to the \a a assignment
      virtual void dropUntil(const Assignment& a);
      /// Test whether all variables are assigned
      bool assigned(void) const;
      /// Test whether all variables match assignment \a a
      bool matchAssignment(const Assignment& a) const;
      /// Post propagator
      void post(void);
      /// Compute a fixpoint and check for failure
      bool failed(void);
      /// Perform integer tell operation on \a x[i]
      void rel(int i, Gecode::FloatRelType frt, Gecode::FloatVal n);
      /// Perform Boolean tell on \a b
      void rel(bool sol);
      /// Assign all (or all but one, if \a skip is true) variables to values in \a a
      /// If assignment of a variable is MT_MAYBE (if the two intervals are contiguous),
      /// \a sol is set to MT_MAYBE
      void assign(const Assignment& a, MaybeType& sol, bool skip=false);
      /// Assing a random variable to a random bound
      void bound(void);
      /// Cut the bigger variable to an half sized interval. It returns
      /// the new size of the cut interval. \a cutDirections gives the direction
      /// to follow (upper part or lower part of the interval).
      Gecode::FloatNum cut(int* cutDirections);
      /// Prune some random values from variable \a i
      void prune(int i);
      /// Prune some random values for some random variable
      void prune(void);
      /// Prune values but not those in assignment \a a
      bool prune(const Assignment& a, bool testfix);
      /// Disable propagators in space and compute fixpoint (make all idle)
      void disable(void);
      /// Enable propagators in space
      void enable(void);
      /// Return the number of propagators
      unsigned int propagators(void);
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
      /// Gives the type of assignment to use
      AssignmentType assigmentType;
      /// Does the constraint also exist as reified constraint
      bool reified;
      /// Which reification modes are supported
      int rms;
      /// Whether to perform search test
      bool testsearch;
      /// Whether to perform fixpoint test
      bool testfix;
      /// Whether to test for subsumption
      bool testsubsumed;
      /// \name Test for reification modes
      //@{
      /// Test whether equivalence as reification mode is supported
      bool eqv(void) const;
      /// Test whether implication as reification mode is supported
      bool imp(void) const;
      /// Test whether reverse implication as reification mode is supported
      bool pmi(void) const;
      //@}
    public:
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s and arity \a a and variable
       * domain \a d and step \a st and assignment type \a at. Also
       * tests for a reified constraint, if \a r is true.
       */
      Test(const std::string& s, int a, const Gecode::FloatVal& d,
           Gecode::FloatNum st, AssignmentType at,
           bool r);
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s and arity \a a and variable
       * domain \a min ... \a max and step \a st and assignment type \a at. Also
       * tests for a reified constraint, if \a r is true.
       */
      Test(const std::string& s, int a,
           Gecode::FloatNum min, Gecode::FloatNum max,
           Gecode::FloatNum st, AssignmentType at,
           bool r);
      /// Create assignment
      virtual Assignment* assignment(void) const;
      /// Complete the current assignment to get a feasible one (which satisfies all constraint).
      /// If such an assignment is computed, it returns true, false otherwise
      virtual bool extendAssignement(Assignment& a) const;
      /// Check for solution
      virtual MaybeType solution(const Assignment&) const = 0;
      /// Test if \a ts is subsumed or not (i.e. if there is no more propagator unless
      /// the assignment is an extended assigment.
      bool subsumed(const TestSpace& ts) const;
      /// Whether to ignore assignment for reification
      virtual bool ignore(const Assignment& a) const;
      /// Post constraint
      virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) = 0;
      /// Post reified constraint
      virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                        Gecode::Reify r);
      /// Perform test
      virtual bool run(void);
      /// \name Mapping scalar values to strings
      //@{
      /// Map float relation to string
      static std::string str(Gecode::FloatRelType frt);
      /// Map floatNum to string
      static std::string str(Gecode::FloatNum f);
      /// Map floatVal to string
      static std::string str(Gecode::FloatVal f);
      /// Map float array to string
      static std::string str(const Gecode::FloatValArgs& f);
      //@}
      /// \name General support
      //@{
      /// Compare \a x and \a y with respect to \a r
      static MaybeType cmp(Gecode::FloatVal x, Gecode::FloatRelType r,
                           Gecode::FloatVal y);
      /// Whether \a x and \a y are equal
      static MaybeType eq(Gecode::FloatVal x, Gecode::FloatVal y);
      /// Flip a coin and return true or false randomly
      bool flip(void);
      //@}
    };
    //@}

    /// Iterator for float relation types
    class FloatRelTypes {
    private:
      /// Array of relation types
      static const Gecode::FloatRelType frts[6];
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

