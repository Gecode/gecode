/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
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

#ifndef __GECODE_TEST_SET_HH__
#define __GECODE_TEST_SET_HH__

#include <gecode/set.hh>
#include "test/test.hh"
#include "test/int.hh"

namespace Test {

  /// Testing finite sets
  namespace Set {

    /**
     * \defgroup TaskTestSet Testing finite sets
     * \ingroup TaskTest
     */

    /// Fake space for creation of regions
    class FakeSpace : public Gecode::Space {
    public:
      /// Faked constructor
      FakeSpace(void) {}
      /// Faked copy function
      virtual Gecode::Space* copy(bool share) {
        (void) share;
        return NULL;
      }
    };

    /**
     * \defgroup TaskTestSetSupport General set test support
     * \ingroup TaskTestSet
     */
    //@{

    /// Value iterator producing subsets of an IntSet
    class CountableSetValues {
    private:
      Gecode::IntSetValues dv;
      int cur;
      int i;
    public:
      /// Default constructor
      CountableSetValues(void) {}
      /// Initialize with set \a d0 and bit-pattern \a cur0
      CountableSetValues(const Gecode::IntSet& d0, int cur0)
      : dv(d0), cur(cur0), i(1) {
        if (! (i & cur))
          operator++();
      }
      /// Initialize with set \a d0 and bit-pattern \a cur0
      void init(const Gecode::IntSet& d0, int cur0) {
        dv = d0;
        cur = cur0;
        i = 1;
        if (! (i & cur))
          operator++();
      }
      /// Test if finished
      bool operator()(void) const {
        return i<=cur;
      }
      /// Move to next value
      void operator++(void) {
        do {
          ++dv;
          i = i<<1;
        } while (! (i & cur) && i<cur);
      }
      /// Return current value
      int val(void) const { return dv.val(); }
    };

    /// Range iterator producing subsets of an IntSet
    class CountableSetRanges
    : public Gecode::Iter::Values::ToRanges<CountableSetValues> {
    private:
      /// The corresponding value iterator
      CountableSetValues v;
    public:
      /// Default constructor
      CountableSetRanges(void) {}
      /// Initialize with set \a d0 and bit-pattern \a cur0
      CountableSetRanges(const Gecode::IntSet& d, int cur) : v(d, cur) {
        Gecode::Iter::Values::ToRanges<CountableSetValues>::init(v);
      }
      /// Initialize with set \a d0 and bit-pattern \a cur0
      void init(const Gecode::IntSet& d, int cur) {
        v.init(d, cur);
        Gecode::Iter::Values::ToRanges<CountableSetValues>::init(v);
      }
    };

    /// Iterate all subsets of a given set
    class CountableSet {
    private:
      /// The superset
      Gecode::IntSet d;
      /// Integer representing the current subset to iterate
      unsigned int cur;
      /// Endpoint of iteration
      unsigned int lubmax;
    public:
      /// Initialize with set \a s
      CountableSet(const Gecode::IntSet& s);
      /// Default constructor
      CountableSet(void) {}
      /// Initialize with set \a s
      void init(const Gecode::IntSet& s);
      /// Check if still subsets left
      bool operator()(void) const { return cur<lubmax; }
      /// Move to next subset
      void operator++(void);
      /// Return current subset
      int val(void) const;
    };

    /// Generate all set assignments
    class SetAssignment {
    private:
      /// Arity
      int n;
      /// Iterator for each set variable
      CountableSet* dsv;
      /// Iterator for int variable
      Test::Int::CpltAssignment ir;
      /// Flag whether all assignments have been generated
      bool done;
    public:
      /// The common superset for all domains
      Gecode::IntSet lub;
      /// How many integer variables to iterate
      int withInt;
      /// Initialize with \a n set variables, initial bound \a d and \a i int variables
      SetAssignment(int n, const Gecode::IntSet& d, int i = 0);
      /// Test whether all assignments have been iterated
      bool operator()(void) const { return !done; }
      /// Move to next assignment
      void operator++(void);
      /// Return value for variable \a i
      int operator[](int i) const {
        assert((i>=0) && (i<n));
        return dsv[i].val();
      }
      /// Return value for first integer variable
      int intval(void) const { return ir[0]; }
      /// Return assignment for integer variables
      const Test::Int::Assignment& ints(void) const { return ir; }
      /// Return arity
      int size(void) const { return n; }
      /// Destructor
      ~SetAssignment(void) { delete [] dsv; }
    };


    class SetTest;
    
    /// Space for executing set tests
    class SetTestSpace : public Gecode::Space {
    public:
      /// Initial domain
      Gecode::IntSet d;
      /// Set variables to be tested
      Gecode::SetVarArray x;
      /// Int variables to be tested
      Gecode::IntVarArray y;
      /// How many integer variables are used by the test
      int withInt;
      /// Control variable for reified propagators
      Gecode::BoolVar b;
      /// Whether the test is for a reified propagator
      bool reified;
      /// The test currently run
      SetTest* test;
      
      /**
       * \brief Create test space
       *
       * Creates \a n set variables with domain \a d0,
       * \a i integer variables with domain \a d0, and stores whether
       * the test is for a reified propagator (\a r), and the test itself
       * (\a t).
       *
       */
      SetTestSpace(int n, Gecode::IntSet& d0, int i, bool r, SetTest* t,
                   bool log=true);
      /// Constructor for cloning \a s
      SetTestSpace(bool share, SetTestSpace& s);
      /// Copy space during cloning
      virtual Gecode::Space* copy(bool share);
      /// Post propagator
      void post(void);
      /// Compute a fixpoint and check for failure
      bool failed(void);
      /// Perform set tell operation on \a x[i]
      void rel(int i, Gecode::SetRelType srt, const Gecode::IntSet& is);
      /// Perform cardinality tell operation on \a x[i]
      void cardinality(int i, int cmin, int cmax);
      /// Perform integer tell operation on \a y[i]
      void rel(int i, Gecode::IntRelType irt, int n);
      /// Perform Boolean tell on \a b
      void rel(bool sol);
      /// Assign all variables to values in \a a
      void assign(const SetAssignment& a);
      /// Test whether all variables are assigned
      bool assigned(void) const;
      /// Remove value \a v from the upper bound of \a x[i]
      void removeFromLub(int v, int i, const SetAssignment& a);
      /// Remove value \a v from the lower bound of \a x[i]
      void addToGlb(int v, int i, const SetAssignment& a);
      /// Perform fixpoint computation
      bool fixprob(void);
      /// Perform random pruning
      bool prune(const SetAssignment& a);
    };

    /**
     * \brief %Base class for tests with set constraints
     *
     */
    class SetTest : public Base {
    private:
      /// Number of variables
      int     arity;
      /// Domain of variables (least upper bound)
      Gecode::IntSet  lub;
      /// Does the constraint also exist as reified constraint
      bool    reified;
      /// Number of additional integer variables
      int    withInt;

      /// Remove \a v values from the least upper bound of \a x, but not those in \f$\mathrm{a}_i\f$
      void removeFromLub(int v, Gecode::SetVar& x, int i,
                         const Gecode::IntSet& a);
      /// Add \a v values to the greatest lower bound of \a x, but not those in \f$\mathrm{a}_i\f$
      void addToGlb(int v, Gecode::SetVar& x, int i, const Gecode::IntSet& a);
      SetAssignment* make_assignment(void);
    public:
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a s and arity \a a and variable
       * domain \a d. Also tests for a reified constraint,
       * if \a r is true. In addition, \a w integer variables are provided.
       */
      SetTest(const std::string& s,
              int a, const Gecode::IntSet& d, bool r=false, int w=0)
        : Base("Set::"+s), arity(a), lub(d), reified(r), withInt(w)  {}
      /// Check for solution
      virtual bool solution(const SetAssignment&) const = 0;
      /// Post propagator
      virtual void post(Gecode::Space& home, Gecode::SetVarArray& x,
                        Gecode::IntVarArray& y) = 0;
      /// Post reified propagator
      virtual void post(Gecode::Space&, Gecode::SetVarArray&,
                        Gecode::IntVarArray&, Gecode::BoolVar) {}
      /// Perform test
      virtual bool run(void);

      /// \name Mapping scalar values to strings
      //@{
      /// Map set relation to string
      static std::string str(Gecode::SetRelType srt);
      /// Map set operation to string
      static std::string str(Gecode::SetOpType srt);
      /// Map integer to string
      static std::string str(int i);
      /// Map integer array to string
      static std::string str(const Gecode::IntArgs& i);
      //@}
    };
    //@}

    /// Iterator for set relation types
    class SetRelTypes {
    private:
      /// Array of relation types
      static const Gecode::SetRelType srts[6];
      /// Current position in relation type array
      int i;
    public:
      /// Initialize iterator
      SetRelTypes(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next relation type
      void operator++(void);
      /// Return current relation type
      Gecode::SetRelType srt(void) const;
    };

    /// Iterator for Boolean operation types
    class SetOpTypes {
    private:
      /// Array of operation types
      static const Gecode::SetOpType sots[4];
      /// Current position in operation type array
      int i;
    public:
      /// Initialize iterator
      SetOpTypes(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next operation type
      void operator++(void);
      /// Return current operation type
      Gecode::SetOpType sot(void) const;
    };

}}

/**
 * \brief Print assignment \a
 * \relates SetAssignment
 */
std::ostream&
operator<<(std::ostream&, const Test::Set::SetAssignment& a);

#include "test/set.hpp"

#endif

// STATISTICS: test-set
