/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_TEST_WORD_HH
#define GECODE_TEST_WORD_HH

#include "test/test.hh"

#include <gecode/word.hh>
#include <gecode/float-word.hh>

#include <set>
#include <utility>
#include <vector>

namespace Test {

  /// Testing fixed-width words
  namespace Word {

    /**
     * \defgroup TaskTestWord Testing fixed-width words
     * \ingroup TaskTest
     */

    /**
     * \defgroup TaskTestWordSupport General word test support
     * \ingroup TaskTestWord
     */
    //@{

    /// A partial word domain
    class Domain {
    private:
      unsigned int _width;
      Gecode::WordValue _lo;
      Gecode::WordValue _hi;
    public:
      /// Construct a domain with width \a width and masks \a lo and \a hi
      Domain(unsigned int width, Gecode::WordValue lo,
             Gecode::WordValue hi);
      /// Return the word width
      unsigned int width(void) const;
      /// Return the width mask
      Gecode::WordValue mask(void) const;
      /// Return the known-one mask
      Gecode::WordValue lo(void) const;
      /// Return the may-be-one mask
      Gecode::WordValue hi(void) const;
      /// Return the unknown-bit mask
      Gecode::WordValue unknown(void) const;
      /// Return the number of unknown bits
      unsigned int unknown_size(void) const;
      /// Test whether \a value is represented by this domain
      bool in(Gecode::WordValue value) const;
    };

    /// Iterate the concrete values represented by a domain
    class Values {
    private:
      Domain d;
      Gecode::WordValue subset;
      bool more;
    public:
      /// Initialize for domain \a d0
      explicit Values(const Domain& d0);
      /// Test whether values remain
      bool operator()(void) const;
      /// Move to the next value
      void operator++(void);
      /// Return the current value
      Gecode::WordValue val(void) const;
    };

    /// Test whether exhaustive enumeration stays within \a limit assignments
    bool exhaustive(const Domain& d, int arity,
                    Gecode::WordValue limit=4096U);
    /// Return all concrete values represented by \a d
    std::vector<Gecode::WordValue> values(const Domain& d);
    /** \brief Return the known bits of the cube with known-one mask \a lo
     * and may-be-one mask \a hi in a word of width \a width
     *
     * Throws Word::OutOfLimits for an invalid width or masks outside the
     * width, and Word::VariableEmptyDomain if \a lo has a bit outside
     * \a hi.
     */
    Gecode::WordBits cube_bits(unsigned int width, Gecode::WordValue lo,
                               Gecode::WordValue hi);

    /** \brief Return the encodings of width \a width from \a minimum to
     * \a maximum in IEEE total order
     *
     * The walk follows the sign bit alone, independently of the library's
     * rank map: negative encodings descend to negative zero, then positive
     * encodings ascend.
     */
    std::vector<Gecode::WordValue>
    float_order_range(unsigned int width, Gecode::WordValue minimum,
                      Gecode::WordValue maximum);

    /** \brief Return the least and the greatest of the nonempty encodings
     * \a values of width \a width in IEEE total order
     *
     * Like float_order_range(), the comparison follows the sign bit alone.
     */
    std::pair<Gecode::WordValue,Gecode::WordValue>
    float_order_extremes(unsigned int width,
                         const std::set<Gecode::WordValue>& values);

    /** \brief Return the position of encoding \a code of width \a width in
     * IEEE total order
     *
     * Like float_order_range(), the position follows the sign bit alone: an
     * encoding with the sign bit set maps to its complement, any other
     * encoding to itself with the sign bit set.
     */
    Gecode::WordValue float_order_position(unsigned int width,
                                           Gecode::WordValue code);

    /** \brief Test whether encoding \a code of width \a width lies in the
     * total-order window from \a minimum to \a maximum
     */
    bool float_order_contains(unsigned int width, Gecode::WordValue minimum,
                              Gecode::WordValue maximum,
                              Gecode::WordValue code);

    /** \brief Narrow the bounds of the float-typed word \a x to the
     * total-order window from encoding \a minimum to encoding \a maximum
     *
     * The word keeps its variable, so aliases of it stay intact. Fails
     * \a home if no value remains.
     */
    void float_bounds(Gecode::Space& home, Gecode::WordVar x,
                      Gecode::WordValue minimum, Gecode::WordValue maximum);

    /** \brief Return a word of float domain type \a format restricted to
     * the total-order window from \a minimum to \a maximum
     *
     * The endpoints are encodings. The word has a value set per
     * \a value_set.
     */
    Gecode::WordVar
    float_window(Gecode::Space& home, Gecode::WordDomainType format,
                 Gecode::WordValue minimum, Gecode::WordValue maximum,
                 Gecode::WordValueSetOption value_set=Gecode::WVS_DEFAULT);

    /// Return a word of float domain type \a type assigned to encoding \a code
    Gecode::WordVar
    float_code(Gecode::Space& home, Gecode::WordDomainType type,
                   Gecode::WordValue code);

    /// Base class for concrete assignments
    class Assignment {
    protected:
      int n;
      Domain d;
    public:
      /// Initialize assignments for \a n0 variables over \a d0
      Assignment(int n0, const Domain& d0);
      /// Test whether assignments remain
      virtual bool has_more(void) const = 0;
      /// Move to the next assignment
      virtual void next(Gecode::Support::RandomGenerator& rand) = 0;
      /// Return the value for variable \a i
      virtual Gecode::WordValue operator[](int i) const = 0;
      /// Return the number of variables
      int size(void) const;
      /// Return the common domain
      const Domain& domain(void) const;
      /// Destructor
      virtual ~Assignment(void);
    };

    /// Generate all concrete assignments
    class CpltAssignment : public Assignment {
    private:
      Gecode::WordValue* vals;
      bool more;
    public:
      /// Initialize for \a n variables over \a d
      CpltAssignment(int n, const Domain& d);
      /// Test whether assignments remain
      virtual bool has_more(void) const;
      /// Move to the next assignment
      virtual void next(Gecode::Support::RandomGenerator& rand);
      /// Return the value for variable \a i
      virtual Gecode::WordValue operator[](int i) const;
      /// Destructor
      virtual ~CpltAssignment(void);
    };

    /// Generate a deterministic random sample of concrete assignments
    class RandomAssignment : public Assignment {
    private:
      Gecode::WordValue* vals;
      int remaining;
      Gecode::WordValue randval(Gecode::Support::RandomGenerator& rand) const;
    public:
      /// Initialize a sample of \a samples assignments
      RandomAssignment(int n, const Domain& d, int samples,
                       Gecode::Support::RandomGenerator& rand);
      /// Test whether assignments remain
      virtual bool has_more(void) const;
      /// Move to the next assignment
      virtual void next(Gecode::Support::RandomGenerator& rand);
      /// Return the value for variable \a i
      virtual Gecode::WordValue operator[](int i) const;
      /// Destructor
      virtual ~RandomAssignment(void);
    };

    /// Iterate partial domains for one word
    class PartialDomains {
    private:
      unsigned int _width;
      Gecode::WordValue state;
      Gecode::WordValue end;
    public:
      /// Largest width accepted for exhaustive partial-domain iteration
      static const unsigned int max_width = 8;
      /// Initialize partial domains of width \a width
      explicit PartialDomains(unsigned int width);
      /// Test whether domains remain
      bool operator()(void) const;
      /// Move to the next domain
      void operator++(void);
      /// Return the current domain
      Domain domain(void) const;
    };

    /// Generate all partial-domain assignments within the exhaustive limit
    class PartialAssignment {
    private:
      int n;
      unsigned int _width;
      Gecode::WordValue state;
      Gecode::WordValue end;
    public:
      /// Initialize for \a n variables of width \a width
      PartialAssignment(int n, unsigned int width);
      /// Test whether assignments remain
      bool has_more(void) const;
      /// Move to the next assignment
      void next(void);
      /// Return the domain for variable \a i
      Domain operator[](int i) const;
      /// Return the number of variables
      int size(void) const;
    };

    class Test;

    /// Space for executing word tests
    class TestSpace : public Gecode::Space {
    public:
      /// Initial domain
      Domain d;
      /// Variables under test
      Gecode::WordVarArray x;
      /// Reification information
      Gecode::Reify r;
      /// Test currently run
      Test* test;
      /// Whether the test is reified
      bool reified;

      /// Create an ordinary test space, with value sets per \a value_set
      TestSpace(int n, const Domain& d, Test* t=nullptr,
                Gecode::WordValueSetOption value_set=Gecode::WVS_NONE);
      /// Create a reified test space, with value sets per \a value_set
      TestSpace(int n, const Domain& d, Test* t, Gecode::ReifyMode rm,
                Gecode::WordValueSetOption value_set=Gecode::WVS_NONE);
      /// Constructor for cloning \a s
      TestSpace(TestSpace& s);
      /// Copy during cloning
      virtual Gecode::Space* copy(void);
      /// Test whether all variables are assigned
      bool assigned(void) const;
      /// Test whether all variables match assignment \a a
      bool match(const Assignment& a) const;
      /// Assign variables, optionally skipping one variable
      void assign(const Assignment& a, bool skip,
                  Gecode::Support::RandomGenerator& rand);
      /// Restrict variables to domains in \a domains
      void narrow(const std::vector<Domain>& domains);
      /** \brief Remove random values from variables with a value set
       *
       * The values of assignment \a a stay, so whether \a a is a solution
       * does not change. Stops when the space fails.
       */
      void remove_values(const Assignment& a,
                         Gecode::Support::RandomGenerator& rand);
      /// Assign the reification control variable
      void rel(bool value);
      /// Post the test constraint
      void post(void);
      /// Compute a fixpoint and test for failure
      bool failed(void);
      /// Return the number of propagators
      unsigned int propagators(void);
    };

    /**
     * \brief Base class for word constraint tests
     *
     * Later word-propagator tests provide concrete and reified posting
     * methods and a concrete-value oracle through this interface.
     *
     * Every assignment is checked twice when the width is at most eight:
     * once with plain cube variables, and once with variables that carry a
     * value set (WVS_EXACT) and lose random values outside the assignment,
     * either before or after posting. The holes make a value-set tell close
     * a domain beyond the requested cube, which propagators must handle.
     */
    class Test : public Base {
    protected:
      int arity;
      Domain dom;
      bool reified;
      int rms;
      bool eqv(void) const;
      bool imp(void) const;
      bool pmi(void) const;
      /// Check assignment \a a, whose solution status is \a sol
      bool check(const Assignment& a, bool sol,
                 Gecode::WordValueSetOption value_set);
    public:
      /// Construct a test
      Test(const std::string& name, int arity, const Domain& domain,
           bool reified=false);
      /// Create the concrete assignments used by the test
      virtual Assignment* assignment(void) const;
      /// Check whether an assignment is a solution
      virtual bool solution(const Assignment& a) const = 0;
      /// Post an ordinary constraint
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x) = 0;
      /// Post a reified constraint
      virtual void post(Gecode::Space& home, Gecode::WordVarArray& x,
                        Gecode::Reify r);
      /// Perform the assigned-value and lifecycle tests
      virtual bool run(void);
    };
    //@}

  }
}

/// Print word domain \a d
std::ostream& operator<<(std::ostream& os, const Test::Word::Domain& d);
/// Print word assignment \a a
std::ostream& operator<<(std::ostream& os, const Test::Word::Assignment& a);
/// Print the name of domain representation \a t
std::ostream& operator<<(std::ostream& os, Gecode::WordDomainType t);
/// Print the name of rounding direction \a r
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Rounding r);
/// Print the name of subnormal treatment \a s
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Subnormals s);
/// Print the name of overflow result \a o
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Overflow o);
/// Print the name of zero sign treatment \a z
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Zeros z);
/// Print the name of NaN selection \a n
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Nan n);
/// Print the name of tininess detection \a t
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Tininess t);
/// Print the name of comparison signaling \a c
std::ostream& operator<<(std::ostream& os, Gecode::FloatWordPolicy::Comparison c);

#include "test/word.hpp"

#endif

// STATISTICS: test-word
