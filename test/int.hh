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

#include "test/test.hh"

#include "gecode/int.hh"

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
    class Test : public Base {
    protected:
      /// Number of variables
      int arity;
      /// Domain of variables
      Gecode::IntSet dom;
      /// Does the constraint also exist as reified constraint
      bool reified;
      /// Consistency level
      Gecode::IntConLevel icl;
      /// Propagation kind
      Gecode::PropKind pk;
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
       * if \a r is true. Consistency level and propagation kind are 
       * maintained for convenience.
       */
      Test(const std::string& s, int a, const Gecode::IntSet& d, bool r=false, 
           Gecode::IntConLevel i=Gecode::ICL_DEF,
           Gecode::PropKind p=Gecode::PK_DEF);
      /**
       * \brief Constructor
       *
       * Constructs a test with name \a t and arity \a a and variable
       * domain \a min ... \a max. Also tests for a reified constraint, 
       * if \a r is true. Consistency level and propagation kind are
       * maintained for convenience.
       */
      Test(const std::string& s, int a, int min, int max, bool r=false,
           Gecode::IntConLevel i=Gecode::ICL_DEF,
           Gecode::PropKind p=Gecode::PK_DEF);
      /// Create assignment
      virtual Assignment* assignment(void) const;
      /// Check for solution
      virtual bool solution(const Assignment&) const = 0;
      /// Post constraint
      virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) = 0;
      /// Post reified constraint
      virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                        Gecode::BoolVar b);
      /// Perform test
      virtual bool run(const Options& opt);
      /// \name Mapping scalar values to strings
      //@{
      /// Map propagation kind to string
      static std::string str(Gecode::PropKind pk);
      /// Map integer consistency level to string
      static std::string str(Gecode::IntConLevel icl);
      /// Map integer relation to string
      static std::string str(Gecode::IntRelType pk);
      /// Map integer to string
      static std::string str(int i);
      //@}
      /// \name General support
      //@{
      /// Compare \a x and \a y with respect to \a r
      template <class T> static bool cmp(T x, Gecode::IntRelType r, T y);
      //@}
    };
    //@}
    
    /// Iterator for integer consistency levels
    class IntConLevels {
    private:
      /// Array of consistency levels
      static const Gecode::IntConLevel icls[3];
      /// Current position in level array
      int i; 
    public:
      /// Initialize iterator
      IntConLevels(void);
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next level
      void operator++(void);
      /// Return current level
      Gecode::IntConLevel icl(void) const;
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
      /// Test whether iterator is done
      bool operator()(void) const;
      /// Increment to next relation type
      void operator++(void);
      /// Return current relation type
      Gecode::IntRelType irt(void) const;
    };
    
  }
}

#include "test/int.icc"

#endif

// STATISTICS: test-int

