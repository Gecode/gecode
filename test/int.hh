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

#include "test/test.hh"

#include <gecode/int.hh>

namespace Test {

  /// Testing finite domain integers
  namespace Int {

    /**
     * \defgroup TaskTestInt Testing finite domain integers
     * \ingroup TaskTest
     */

    /**
     * \defgroup TaskTestIntInt General test support
     * \ingroup TaskTestInt
     */
    //@{
    /// %Base class for assignments
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

    /// Generate random selection of assignments
    class RandomMixAssignment : public Assignment {
    protected:
      int* vals; ///< The current values for the variables
      int  a;    ///< How many assigments still to be generated
      int _n1;   ///< How many variables in the second set
      Gecode::IntSet _d1; ///< Domain for second set of variables
      /// Generate new value according to domain \a d
      int randval(const Gecode::IntSet& d);
    public:
      /// Initialize for \a a assignments for \a n0 variables and values \a d0
      RandomMixAssignment(int n0, const Gecode::IntSet& d0,
                          int n1, const Gecode::IntSet& d1, int a0);
      /// Test whether all assignments have been iterated
      virtual bool operator()(void) const;
      /// Move to next assignment
      virtual void operator++(void);
      /// Return value for variable \a i
      virtual int operator[](int i) const;
      /// Destructor
      virtual ~RandomMixAssignment(void);
    };

    /// Level of consistency to test for
    enum ConTestLevel {
      CTL_NONE,     ///< No consistency-test
      CTL_DOMAIN,   ///< Test for domain-consistency
      CTL_BOUNDS_D, ///< Test for bounds(d)-consistency
      CTL_BOUNDS_Z, ///< Test for bounds(z)-consistency
    };

    class Test;

    /// Space for executing tests
    class TestSpace : public Gecode::Space {
    public:
      /// Initial domain
      Gecode::IntSet d;
      /// Variables to be tested
      Gecode::IntVarArray x;
      /// Reification information
      Gecode::Reify r;
      /// The test currently run
      Test* test;
      /// Whether the test is for a reified propagator
      bool reified;

      /**
       * \brief Create test space without reification
       *
       * Creates \a n variables with domain \a d for test \a t.
       *
       */
      TestSpace(int n, Gecode::IntSet& d, Test* t);
      /**
       * \brief Create test space with reification
       *
       * Creates \a n variables with domain \a d for test \a t and stores
       * the reification mode \a rm.
       *
       */
      TestSpace(int n, Gecode::IntSet& d, Test* t, Gecode::ReifyMode rm);
      /// Constructor for cloning \a s
      TestSpace(TestSpace& s);
      /// Copy space during cloning
      virtual Gecode::Space* copy(void);
      /// Test whether all variables are assigned
      bool assigned(void) const;
      /// Post propagator
      void post(void);
      /// Compute a fixpoint and check for failure
      bool failed(void);
      /// Randomly select an unassigned variable
      int rndvar(void);
      /// Randomly select a pruning rel for variable \a i
      void rndrel(const Assignment& a, int i, Gecode::IntRelType& irt, int& v);
      /// Perform integer tell operation on \a x[i]
      void rel(int i, Gecode::IntRelType irt, int n);
      /// Perform Boolean tell on \a b
      void rel(bool sol);
      /// Assign all (or all but one, if \a skip is true) variables to values in \a a
      void assign(const Assignment& a, bool skip=false);
      /// Assing a random variable to a random bound
      void bound(void);
      /** \brief Prune some random values from variable \a i
       *
       * If \a bounds_only is true, then the pruning is only done on the
       * bounds of the variable.
       */
      void prune(int i, bool bounds_only);
      /// Prune some random values for some random variable
      void prune(void);
      /// Prune values but not those in assignment \a a
      bool prune(const Assignment& a, bool testfix);
      /// Disable propagators in space and compute fixpoint (make all idle)
      void disable(void);
      /// Enable propagators in space
      void enable(void);
      /// Prune values also in a space \a c with disabled propagators, but not those in assignment \a a
      bool disabled(const Assignment& a, TestSpace& c, bool testfix);
      /// Return the number of propagators
      unsigned int propagators(void);
    };

    /**
     * \brief %Base class for tests with integer constraints
     *
     */
    class Test : public Base {
    protected:
      /// Number of variables
      int arity;
      /// Domain of variables
      Gecode::IntSet dom;
      /// Does the constraint also exist as reified constraint
      bool reified;
      /// Which reification modes are supported
      int rms;
      /// Propagation level
      Gecode::IntPropLevel ipl;
      /// Whether to test for certain consistency
      ConTestLevel contest;
      /// Whether to perform search test
      bool testsearch;
      /// Whether to perform fixpoint test
      bool testfix;
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
       * Constructs a test with prefix \a p, name \a s, arity \a a,
       * and variable domain \a d. Also tests for a reified
       * constraint, if \a r is true. The propagation level is
       * maintained for convenience.
       */
      Test(const std::string& p, const std::string& s,
           int a, const Gecode::IntSet& d, bool r=false,
           Gecode::IntPropLevel i=Gecode::IPL_DEF);
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s, arity \a a, and variable
       * domain \a d. Also tests for a reified constraint,
       * if \a r is true. The propagation level is
       * maintained for convenience.
       */
      Test(const std::string& s,
           int a, const Gecode::IntSet& d, bool r=false,
           Gecode::IntPropLevel i=Gecode::IPL_DEF);
      /**
       * \brief Constructor
       *
       * Constructs a test with prefix \a p, name \a s, arity \a a,
       * and variable domain \a min ... \a max. Also tests for
       * a reified constraint, if \a r is true. The propagation
       * level is maintained for convenience.
       */
      Test(const std::string& p, const std::string& s,
           int a, int min, int max, bool r=false,
           Gecode::IntPropLevel i=Gecode::IPL_DEF);
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s, arity \a a, variable
       * domain \a min ... \a max. Also tests for a reified constraint,
       * if \a r is true. The propagation level is
       * maintained for convenience.
       */
      Test(const std::string& s,
           int a, int min, int max, bool r=false,
           Gecode::IntPropLevel i=Gecode::IPL_DEF);
      /// Create assignment
      virtual Assignment* assignment(void) const;
      /// Check for solution
      virtual bool solution(const Assignment&) const = 0;
      /// Whether to ignore assignment for reification
      virtual bool ignore(const Assignment&) const;
      /// Post constraint
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) = 0;
      /// Post reified constraint
      virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                        Gecode::Reify r);
      /// Perform test
      virtual bool run(void);
      /// \name Mapping scalar values to strings
      //@{
      /// Map integer propagation level to string
      static std::string str(Gecode::IntPropLevel ipl);
      /// Map integer relation to string
      static std::string str(Gecode::IntRelType irl);
      /// Map Boolean operation to string
      static std::string str(Gecode::BoolOpType bot);
      /// Map bool to string
      static std::string str(bool b);
      /// Map integer to string
      static std::string str(int i);
      /// Map integer array to string
      static std::string str(const Gecode::IntArgs& i);
      //@}
      /// \name General support
      //@{
      /// Compare \a x and \a y with respect to \a r
      template<class T> static bool cmp(T x, Gecode::IntRelType r, T y);
      //@}
    };
    //@}

    /// Iterator for simple integer propagation levels
    class IntPropLevels {
    private:
      /// Array of propagation levels
      static const Gecode::IntPropLevel ipls[3];
      /// Current position in level array
      int i;
    public:
      /// Initialize iterator
      IntPropLevels(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next level
      void operator++(void);
      /// Return current level
      Gecode::IntPropLevel ipl(void) const;
    };

    /// Iterator for basic and advanced integer propagation levels
    class IntPropBasicAdvanced {
    private:
      /// Array of propagation levels
      static const Gecode::IntPropLevel ipls[3];
      /// Current position in level array
      int i;
    public:
      /// Initialize iterator
      IntPropBasicAdvanced(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next level
      void operator++(void);
      /// Return current level
      Gecode::IntPropLevel ipl(void) const;
    };

    /// Iterator for integer relation types
    class IntRelTypes {
    private:
      /// Array of relation types
      static const Gecode::IntRelType irts[6];
      /// Current position in relation type array
      int i;
    public:
      /// Initialize iterator
      IntRelTypes(void);
      /// Reset iterator
      void reset(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next relation type
      void operator++(void);
      /// Return current relation type
      Gecode::IntRelType irt(void) const;
    };

    /// Iterator for Boolean operation types
    class BoolOpTypes {
    private:
      /// Array of operation types
      static const Gecode::BoolOpType bots[5];
      /// Current position in operation type array
      int i;
    public:
      /// Initialize iterator
      BoolOpTypes(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next operation type
      void operator++(void);
      /// Return current operation type
      Gecode::BoolOpType bot(void) const;
    };

  }
}

/**
 * \brief Print assignment \a
 * \relates Assignment
 */
std::ostream& operator<<(std::ostream& os, const Test::Int::Assignment& a);

#include "test/int.hpp"

#endif

// STATISTICS: test-int

