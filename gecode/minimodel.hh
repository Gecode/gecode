/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
 *     Mikael Lagerkvist, 2005
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

#ifndef __GECODE_MINIMODEL_HH__
#define __GECODE_MINIMODEL_HH__

#include <gecode/kernel.hh>
#include <gecode/int.hh>
#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif
#include <gecode/int/linear.hh>

#include <gecode/minimodel/exception.hpp>

#include <iostream>

/*
 * Support for DLLs under Windows
 *
 */

#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_MINIMODEL
#define GECODE_MINIMODEL_EXPORT __declspec( dllexport )
#else
#define GECODE_MINIMODEL_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY

#define GECODE_MINIMODEL_EXPORT __attribute__ ((visibility("default")))

#else

#define GECODE_MINIMODEL_EXPORT

#endif
#endif

// Configure auto-linking
#ifndef GECODE_BUILD_MINIMODEL
#define GECODE_LIBRARY_NAME "MiniModel"
#include <gecode/support/auto-link.hpp>
#endif

namespace Gecode {

  /// Minimalistic modeling support
  namespace MiniModel {}

  class LinRel;
#ifdef GECODE_HAS_SET_VARS
  class SetExpr;
#endif

  /// Base class for non-linear expressions
  class NonLinExpr {
  public:
    /// Return variable constrained to be equal to the expression
    virtual IntVar post(Home home, IntVar* ret, IntConLevel icl) const = 0;
    /// Post expression to be in relation \a irt with \a c
    virtual void post(Home home, IntRelType irt, int c,
                      IntConLevel icl) const = 0;
    /// Post reified expression to be in relation \a irt with \a c
    virtual void post(Home home, IntRelType irt, int c,
                      BoolVar b, IntConLevel icl) const = 0;
    /// Destructor
    virtual ~NonLinExpr(void) {}
    /// Return fresh variable if \a x is NULL, \a x otherwise
    static IntVar result(Home home, IntVar* x) {
      if (x==NULL)
        return IntVar(home,Int::Limits::min,Int::Limits::max);
      return *x;
    }
    /// Constrain \a x to be equal to \a y if \a x is not NULL
    static IntVar result(Home home, IntVar* x, IntVar y) {
      if (x!=NULL)
        rel(home,*x,IRT_EQ,y);
      return y;
    }
    /// Memory management
    void* operator new(size_t size) { return heap.ralloc(size); }
    /// Memory management
    void operator delete(void* p, size_t) { heap.rfree(p); }
  };

  /// Linear expressions
  class LinExpr {
    friend class LinRel;
#ifdef GECODE_HAS_SET_VARS
    friend class SetExpr;
#endif
  public:
    /// Type of linear expression
    enum NodeType {
      NT_CONST,    ///< Integer constant
      NT_VAR_INT,  ///< Linear term with integer variable
      NT_VAR_BOOL, ///< Linear term with Boolean variable
      NT_NONLIN,   ///< Non-linear expression
      NT_SUM_INT,  ///< Sum of integer variables
      NT_SUM_BOOL, ///< Sum of Boolean variables
      NT_ADD,      ///< Addition of linear terms
      NT_SUB,      ///< Subtraction of linear terms
      NT_MUL       ///< Multiplication by coefficient
    };
  private:
    /// Nodes for linear expressions
    class Node {
    public:
      /// Nodes are reference counted
      unsigned int use;
      /// Integer variables in tree
      int n_int;
      /// Boolean variables in tree
      int n_bool;
      /// Type of expression
      NodeType t;
      /// Subexpressions
      Node *l, *r;
      /// Sum of integer or Boolean variables, or non-linear expression
      union {
        /// Integer views and coefficients
        Int::Linear::Term<Int::IntView>* ti;
        /// Bool views and coefficients
        Int::Linear::Term<Int::BoolView>* tb;
        /// Non-linear expression
        NonLinExpr* ne;
      } sum;
      /// Coefficient and offset
      int a, c;
      /// Integer variable (potentially)
      IntVar x_int;
      /// Boolean variable (potentially)
      BoolVar x_bool;
      /// Default constructor
      Node(void);
      /// Generate linear terms from expression
      GECODE_MINIMODEL_EXPORT
      void fill(Home home, IntConLevel icl,
                Int::Linear::Term<Int::IntView>*& ti,
                Int::Linear::Term<Int::BoolView>*& tb,
                double m, double& d) const;
      /// Generate linear terms for expressions
      int fill(Home home, IntConLevel icl,
               Int::Linear::Term<Int::IntView>* ti,
               Int::Linear::Term<Int::BoolView>* tb) const;
      /// Decrement reference count and possibly free memory
      bool decrement(void);
      /// Destructor
      ~Node(void);
      /// Memory management
      static void* operator new(size_t size);
      /// Memory management
      static void  operator delete(void* p,size_t size);
    };
    Node* n;
  public:
    /// Default constructor
    GECODE_MINIMODEL_EXPORT
    LinExpr(void);
    /// Create expression for constant \a c
    GECODE_MINIMODEL_EXPORT
    LinExpr(int c);
    /// Create expression
    GECODE_MINIMODEL_EXPORT
    LinExpr(const IntVar& x, int a=1);
    /// Create expression
    GECODE_MINIMODEL_EXPORT
    LinExpr(const BoolVar& x, int a=1);
    /// Create sum expression
    GECODE_MINIMODEL_EXPORT
    explicit LinExpr(const IntVarArgs& x);
    /// Create sum expression
    GECODE_MINIMODEL_EXPORT
    LinExpr(const IntArgs& a, const IntVarArgs& x);
    /// Create sum expression
    GECODE_MINIMODEL_EXPORT
    explicit LinExpr(const BoolVarArgs& x);
    /// Create sum expression
    GECODE_MINIMODEL_EXPORT
    LinExpr(const IntArgs& a, const BoolVarArgs& x);
    /// Copy constructor
    LinExpr(const LinExpr& e);
    /// Create expression for type and subexpressions
    GECODE_MINIMODEL_EXPORT
    LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1);
    /// Create expression for type and subexpression
    GECODE_MINIMODEL_EXPORT
    LinExpr(const LinExpr& e0, NodeType t, int c);
    /// Create expression for multiplication
    GECODE_MINIMODEL_EXPORT
    LinExpr(int a, const LinExpr& e);
    /// Create non-linear expression
    GECODE_MINIMODEL_EXPORT
    explicit LinExpr(NonLinExpr* e);
    /// Assignment operator
    GECODE_MINIMODEL_EXPORT
    const LinExpr& operator =(const LinExpr& e);
    /// Post propagator
    void post(Home home, IntRelType irt, IntConLevel icl) const;
    /// Post reified propagator
    void post(Home home, IntRelType irt, const BoolVar& b,
              IntConLevel icl) const;
    /// Post propagator and return variable for value
    IntVar post(Home home, IntConLevel icl) const;
    /// Return non-linear expression inside, or NULL if not non-linear
    NonLinExpr* nle(void) const;
    /// Destructor
    GECODE_MINIMODEL_EXPORT
    ~LinExpr(void);
  };

  class BoolExpr;

  /// Linear relations
  class LinRel {
    friend class BoolExpr;
  private:
    /// Linear expression describing the entire relation
    LinExpr e;
    /// Which relation
    IntRelType irt;
    /// Negate relation type
    static IntRelType neg(IntRelType irt);
    /// Default constructor
    LinRel(void);
  public:
    /// Create linear relation for expressions \a l and \a r
    LinRel(const LinExpr& l, IntRelType irt, const LinExpr& r);
    /// Create linear relation for expression \a l and integer \a r
    LinRel(const LinExpr& l, IntRelType irt, int r);
    /// Create linear relation for integer \a l and expression \a r
    LinRel(int l, IntRelType irt, const LinExpr& r);
    /// Post propagator for relation (if \a t is false for negated relation)
    void post(Home home, bool t,  IntConLevel icl) const;
    /// Post reified propagator for relation (if \a t is false for negated relation)
    void post(Home home, const BoolVar& b, bool t, IntConLevel icl) const;
  };

  /**
   * \defgroup TaskModelMiniModelLin Linear expressions and relations
   *
   * Linear expressions can be freely composed of sums and differences of
   * integer variables (Gecode::IntVar) or Boolean variables
   * (Gecode::BoolVar) possibly with integer coefficients and integer
   * constants.
   *
   * Note that both integer and Boolean variables are automatically
   * available as linear expressions.
   *
   * Linear relations are obtained from linear expressions with the normal
   * relation operators.
   *
   * \ingroup TaskModelMiniModel
   */

  //@{
  /// Construct linear expression as sum of integer variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(int, const IntVar&);
  /// Construct linear expression as sum of Boolean variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(int, const BoolVar&);
  /// Construct linear expression as sum of linear expression and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(int, const LinExpr&);
  /// Construct linear expression as sum of integer variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const IntVar&, int);
  /// Construct linear expression as sum of Boolean variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const BoolVar&, int);
  /// Construct linear expression as sum of linear expression and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const LinExpr&, int);
  /// Construct linear expression as sum of integer variables
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const IntVar&, const IntVar&);
  /// Construct linear expression as sum of integer and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const IntVar&, const BoolVar&);
  /// Construct linear expression as sum of Boolean and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const BoolVar&, const IntVar&);
  /// Construct linear expression as sum of Boolean variables
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const BoolVar&, const BoolVar&);
  /// Construct linear expression as sum of integer variable and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const IntVar&, const LinExpr&);
  /// Construct linear expression as sum of Boolean variable and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const BoolVar&, const LinExpr&);
  /// Construct linear expression as sum of linear expression and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const LinExpr&, const IntVar&);
  /// Construct linear expression as sum of linear expression and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const LinExpr&, const BoolVar&);
  /// Construct linear expression as sum of linear expressions
  GECODE_MINIMODEL_EXPORT LinExpr
  operator +(const LinExpr&, const LinExpr&);

  /// Construct linear expression as sum of integer variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(int, const IntVar&);
  /// Construct linear expression as sum of Boolean variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(int, const BoolVar&);
  /// Construct linear expression as sum of integer and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(int, const LinExpr&);
  /// Construct linear expression as sum of integer variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const IntVar&, int);
  /// Construct linear expression as sum of Boolean variable and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const BoolVar&, int);
  /// Construct linear expression as sum of linear expression and integer
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const LinExpr&, int);
  /// Construct linear expression as sum of integer variables
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const IntVar&, const IntVar&);
  /// Construct linear expression as sum of integer and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const IntVar&, const BoolVar&);
  /// Construct linear expression as sum of Boolean and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const BoolVar&, const IntVar&);
  /// Construct linear expression as sum of Boolean variables
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const BoolVar&, const BoolVar&);
  /// Construct linear expression as sum of integer variable and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const IntVar&, const LinExpr&);
  /// Construct linear expression as sum of Boolean variable and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const BoolVar&, const LinExpr&);
  /// Construct linear expression as sum of linear expression and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const LinExpr&, const IntVar&);
  /// Construct linear expression as sum of linear expression and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const LinExpr&, const BoolVar&);
  /// Construct linear expression as sum of linear expressions
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const LinExpr&, const LinExpr&);

  /// Construct linear expression as negative of integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const IntVar&);
  /// Construct linear expression as negative of Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const BoolVar&);
  /// Construct linear expression as negative of linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator -(const LinExpr&);

  /// Construct linear expression as product of integer coefficient and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(int, const IntVar&);
  /// Construct linear expression as product of integer coefficient and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(int, const BoolVar&);
  /// Construct linear expression as product of integer coefficient and integer variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(const IntVar&, int);
  /// Construct linear expression as product of integer coefficient and Boolean variable
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(const BoolVar&, int);
  /// Construct linear expression as product of integer coefficient and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(const LinExpr&, int);
  /// Construct linear expression as product of integer coefficient and linear expression
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(int, const LinExpr&);

  /// Construct linear expression as sum of integer variables
  GECODE_MINIMODEL_EXPORT LinExpr
  sum(const IntVarArgs& x);
  /// Construct linear expression as sum of integer variables with coefficients
  GECODE_MINIMODEL_EXPORT LinExpr
  sum(const IntArgs& a, const IntVarArgs& x);
  /// Construct linear expression as sum of Boolean variables
  GECODE_MINIMODEL_EXPORT LinExpr
  sum(const BoolVarArgs& x);
  /// Construct linear expression as sum of Boolean variables with coefficients
  GECODE_MINIMODEL_EXPORT LinExpr
  sum(const IntArgs& a, const BoolVarArgs& x);

  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(int l, const IntVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(int l, const BoolVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(int l, const LinExpr& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const IntVar& l, int r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const BoolVar& l, int r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const LinExpr& l, int r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const IntVar& l, const IntVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const IntVar& l, const BoolVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const BoolVar& l, const IntVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const BoolVar& l, const BoolVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const IntVar& l, const LinExpr& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const BoolVar& l, const LinExpr& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const LinExpr& l, const IntVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const LinExpr& l, const BoolVar& r);
  /// Construct linear equality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator ==(const LinExpr& l, const LinExpr& r);

  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(int l, const IntVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(int l, const BoolVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(int l, const LinExpr& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const IntVar& l, int r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const BoolVar& l, int r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const LinExpr& l, int r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const IntVar& l, const IntVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const IntVar& l, const BoolVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const BoolVar& l, const IntVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const BoolVar& l, const BoolVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const IntVar& l, const LinExpr& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const BoolVar& l, const LinExpr& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const LinExpr& l, const IntVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const LinExpr& l, const BoolVar& r);
  /// Construct linear disequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator !=(const LinExpr& l, const LinExpr& r);

  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(int l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(int l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(int l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const IntVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const BoolVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const LinExpr& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const IntVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const IntVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const BoolVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const BoolVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const IntVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const BoolVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const LinExpr& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const LinExpr& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <(const LinExpr& l, const LinExpr& r);

  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(int l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(int l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(int l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const IntVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const BoolVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const LinExpr& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const IntVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const IntVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const BoolVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const BoolVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const IntVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const BoolVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const LinExpr& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const LinExpr& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator <=(const LinExpr& l, const LinExpr& r);

  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(int l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(int l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(int l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const IntVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const BoolVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const LinExpr& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const IntVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const IntVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const BoolVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const BoolVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const IntVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const BoolVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const LinExpr& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const LinExpr& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >(const LinExpr& l, const LinExpr& r);

  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(int l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(int l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(int l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const IntVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const BoolVar& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const LinExpr& l, int r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const IntVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const IntVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const BoolVar& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const BoolVar& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const IntVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const BoolVar& l, const LinExpr& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const LinExpr& l, const IntVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const LinExpr& l, const BoolVar& r);
  /// Construct linear inequality relation
  GECODE_MINIMODEL_EXPORT LinRel
  operator >=(const LinExpr& l, const LinExpr& r);
  //@}

#ifdef GECODE_HAS_SET_VARS
  /// %Set expressions
  class SetExpr {
  public:
    /// Type of set expression
    enum NodeType {
      NT_VAR,    ///< Variable
      NT_CONST,  ///< Constant
      NT_LEXP,   ///< Linear expression
      NT_CMPL,   ///< Complement
      NT_INTER,  ///< Intersection
      NT_UNION,  ///< Union
      NT_DUNION  ///< Disjoint union
    };
    /// Check if types agree
    static bool same(NodeType t0, NodeType t1);
    /// %Node for set expression
    class Node {
    public:
      /// Nodes are reference counted
      unsigned int use;
      /// Number of variables in subtree with same type (for INTER and UNION)
      int same;
      /// Type of expression
      NodeType t;
      /// Subexpressions
      Node *l, *r;
      /// Possibly a variable
      SetVar x;
      /// Possibly a constant
      IntSet s;
      /// Possibly a linear expression
      LinExpr e;

      /// Default constructor
      Node(void);
      /// Decrement reference count and possibly free memory
      GECODE_MINIMODEL_EXPORT
      bool decrement(void);
      /// Memory management
      static void* operator new(size_t size);
      /// Memory management
      static void  operator delete(void* p, size_t size);
    };
    /// %Node for negation normalform (%NNF)
    class NNF {
    public:
      /// Type of node
      NodeType t;
      /// Number of positive literals for node type
      int p;
      /// Number of negative literals for node type
      int n;
      /// Union depending on nodetype \a t
      union {
        /// For binary nodes (and, or, eqv)
        struct {
          /// Left subtree
          NNF* l;
          /// Right subtree
          NNF* r;
        } b;
        /// For atomic nodes
        struct {
          /// Pointer to corresponding Boolean expression node
          Node* x;
        } a;
      } u;
      /// Is formula negative
      bool neg;
      /// Create negation normalform
      GECODE_MINIMODEL_EXPORT
      static NNF* nnf(Region& r, Node* n, bool neg);
      /// Post propagators for nested conjunctive and disjunctive expression
      GECODE_MINIMODEL_EXPORT
      void post(Home home, NodeType t, SetVarArgs& b, int& i) const;
      /// Post propagators for expression
      GECODE_MINIMODEL_EXPORT
      void post(Home home, SetRelType srt, SetVar s) const;
      /// Post propagators for reified expression
      GECODE_MINIMODEL_EXPORT
      void post(Home home, SetRelType srt, SetVar s, BoolVar b) const;
      /// Post propagators for relation
      GECODE_MINIMODEL_EXPORT
      void post(Home home, SetRelType srt, const NNF* n) const;
      /// Post reified propagators for relation (or negated relation if \a t is false)
      GECODE_MINIMODEL_EXPORT
      void post(Home home, BoolVar b, bool t, SetRelType srt,
                const NNF* n) const;
      /// Allocate memory from region
      static void* operator new(size_t s, Region& r);
      /// No-op (for exceptions)
      static void operator delete(void*);
      /// No-op
      static void operator delete(void*, Region&);
    };
  private:
    /// Pointer to node for expression
    Node* n;
  public:
    /// Default constructor
    GECODE_MINIMODEL_EXPORT
    SetExpr(void);
    /// Copy constructor
    SetExpr(const SetExpr& e);
    /// Construct expression for type and subexpresssions
    GECODE_MINIMODEL_EXPORT
    SetExpr(const SetExpr& l, NodeType t, const SetExpr& r);
    /// Construct expression for variable
    GECODE_MINIMODEL_EXPORT
    SetExpr(const SetVar& x);
    /// Construct expression for integer variable
    GECODE_MINIMODEL_EXPORT
    explicit SetExpr(const LinExpr& x);
    /// Construct expression for constant
    GECODE_MINIMODEL_EXPORT
    SetExpr(const IntSet& s);
    /// Construct expression for negation
    GECODE_MINIMODEL_EXPORT
    SetExpr(const SetExpr& e, NodeType t);
    /// Post propagators for expression
    SetVar post(Home home) const;
    /// Post propagators for relation
    void post(Home home, SetRelType srt, const SetExpr& e) const;
    /// Post propagators for reified relation
    void post(Home home, BoolVar b, bool t,
              SetRelType srt, const SetExpr& e) const;
    /// Assignment operator
    GECODE_MINIMODEL_EXPORT
    const SetExpr& operator =(const SetExpr& e);
    /// Destructor
    GECODE_MINIMODEL_EXPORT
    ~SetExpr(void);
  };

  /// Comparison relation (for two-sided comparisons)
  class SetCmpRel {
  public:
    /// Left side of relation
    SetExpr l;
    /// Right side of relation
    SetExpr r;
    /// Which relation
    SetRelType srt;
    /// Constructor
    SetCmpRel(const SetExpr& l, SetRelType srt, const SetExpr& r);
  };

  /// %Set relations
  class SetRel {
  private:
    /// Expression
    SetExpr _e0;
    /// Relation
    SetRelType _srt;
    /// Expression
    SetExpr _e1;
  public:
    /// Default constructor
    SetRel(void);
    /// Constructor
    SetRel(const SetExpr& e0, SetRelType srt, const SetExpr& e1);
    /// Constructor
    SetRel(const SetCmpRel& r);
    /// Post propagators for relation (or negated relation if \a t is false)
    void post(Home home, bool t) const;
    /// Post propagators for reified relation (or negated relation if \a t is false)
    void post(Home home, BoolVar b, bool t) const;
  };

  /**
   * \defgroup TaskModelMiniModelSet Set expressions and relations
   *
   * Set expressions and relations can be freely composed of variables
   * with the usual connectives.
   *
   * \ingroup TaskModelMiniModel
   */

  //@{
  /// Singleton expression
  GECODE_MINIMODEL_EXPORT SetExpr
  singleton(const LinExpr&);
  /// Complement expression
  GECODE_MINIMODEL_EXPORT SetExpr
  operator -(const SetExpr&);
  /// Intersection of set expressions
  GECODE_MINIMODEL_EXPORT SetExpr
  operator &(const SetExpr&, const SetExpr&);
  /// Union of set expressions
  GECODE_MINIMODEL_EXPORT SetExpr
  operator |(const SetExpr&, const SetExpr&);
  /// Disjoint union of set expressions
  GECODE_MINIMODEL_EXPORT SetExpr
  operator +(const SetExpr&, const SetExpr&);
  /// Difference of set expressions
  GECODE_MINIMODEL_EXPORT SetExpr
  operator -(const SetExpr&, const SetExpr&);

  /// Intersection of set variables
  GECODE_MINIMODEL_EXPORT SetExpr
  inter(const SetVarArgs&);
  /// Union of set variables
  GECODE_MINIMODEL_EXPORT SetExpr
  setunion(const SetVarArgs&);
  /// Disjoint union of set variables
  GECODE_MINIMODEL_EXPORT SetExpr
  setdunion(const SetVarArgs&);

  /// Cardinality of set expression
  GECODE_MINIMODEL_EXPORT LinExpr
  cardinality(const SetExpr&);
  /// Minimum element of set expression
  GECODE_MINIMODEL_EXPORT LinExpr
  min(const SetExpr&);
  /// Minimum element of set expression
  GECODE_MINIMODEL_EXPORT LinExpr
  max(const SetExpr&);

  /// Equality of set expressions
  GECODE_MINIMODEL_EXPORT SetRel
  operator ==(const SetExpr&, const SetExpr&);
  /// Disequality of set expressions
  GECODE_MINIMODEL_EXPORT SetRel
  operator !=(const SetExpr&, const SetExpr&);
  /// Subset of set expressions
  GECODE_MINIMODEL_EXPORT SetCmpRel
  operator <=(const SetExpr&, const SetExpr&);
  /// Subset of set expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator <=(const SetCmpRel&, const SetExpr&);
  /// Superset of set expressions
  GECODE_MINIMODEL_EXPORT SetCmpRel
  operator >=(const SetExpr&, const SetExpr&);
  /// Superset of set expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator >=(const SetCmpRel&, const SetExpr&);
  /// Disjointness of set expressions
  GECODE_MINIMODEL_EXPORT SetRel
  operator ||(const SetExpr&, const SetExpr&);
  //@}
#endif

  /// Boolean expressions
  class BoolExpr {
  public:
    /// Type of Boolean expression
    enum NodeType {
      NT_VAR,  ///< Variable
      NT_NOT,  ///< Negation
      NT_AND,  ///< Conjunction
      NT_OR,   ///< Disjunction
      NT_EQV,  ///< Equivalence
      NT_RLIN, ///< Reified linear relation
      NT_RSET, ///< Reified set relation
      NT_MISC  ///< Other Boolean expression
    };
    /// Miscealloneous Boolean expressions
    class MiscExpr {
    public:
      /** Constrain \a b to be equivalent to the expression
       *  (negated if \a neg)
       */
      virtual void post(Space& home, BoolVar b, bool neg,
                        IntConLevel icl) = 0;
      /// Destructor
      virtual GECODE_MINIMODEL_EXPORT ~MiscExpr(void);
      /// Memory management
      static void* operator new(size_t size);
      /// Memory management
      static void  operator delete(void* p, size_t size);
    };
    /// %Node for Boolean expression
    class Node {
    public:
      /// Nodes are reference counted
      unsigned int use;
      /// Number of variables in subtree with same type (for AND and OR)
      int same;
      /// Type of expression
      NodeType t;
      /// Subexpressions
      Node *l, *r;
      /// Possibly a variable
      BoolVar x;
      /// Possibly a reified linear relation
      LinRel rl;
#ifdef GECODE_HAS_SET_VARS
      /// Possibly a reified set relation
      SetRel rs;
#endif
      /// Possibly a misc Boolean expression
      MiscExpr* m;

      /// Default constructor
      Node(void);
      /// Destructor
      ~Node(void);
      /// Decrement reference count and possibly free memory
      GECODE_MINIMODEL_EXPORT
      bool decrement(void);
      /// Memory management
      static void* operator new(size_t size);
      /// Memory management
      static void  operator delete(void* p, size_t size);
    };
    /// %Node for negation normalform (%NNF)
    class NNF {
    public:
      /// Type of node
      NodeType t;
      /// Number of positive literals for node type
      int p;
      /// Number of negative literals for node type
      int n;
      /// Union depending on nodetype \a t
      union {
        /// For binary nodes (and, or, eqv)
        struct {
          /// Left subtree
          NNF* l;
          /// Right subtree
          NNF* r;
        } b;
        /// For atomic nodes
        struct {
          /// Is atomic formula negative
          bool neg;
          /// Pointer to corresponding Boolean expression node
          Node* x;
        } a;
      } u;
      /// Create negation normalform
      GECODE_MINIMODEL_EXPORT
      static NNF* nnf(Region& r, Node* n, bool neg);
      /// Post propagators for nested conjunctive and disjunctive expression
      GECODE_MINIMODEL_EXPORT
      void post(Home home, NodeType t,
                BoolVarArgs& bp, BoolVarArgs& bn,
                int& ip, int& in,
                IntConLevel icl) const;
      /// Post propagators for expression
      GECODE_MINIMODEL_EXPORT
      BoolVar expr(Home home, IntConLevel icl) const;
      /// Post propagators for relation
      GECODE_MINIMODEL_EXPORT
      void rel(Home home, IntConLevel icl) const;
      /// Allocate memory from region
      static void* operator new(size_t s, Region& r);
      /// No-op (for exceptions)
      static void operator delete(void*);
      /// No-op
      static void operator delete(void*, Region&);
    };
  private:
    /// Pointer to node for expression
    Node* n;
  public:
    /// Default constructor
    GECODE_MINIMODEL_EXPORT
    BoolExpr(void);
    /// Copy constructor
    BoolExpr(const BoolExpr& e);
    /// Construct expression for type and subexpresssions
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const BoolExpr& l, NodeType t, const BoolExpr& r);
    /// Construct expression for variable
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const BoolVar& x);
    /// Construct expression for negation
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const BoolExpr& e, NodeType t);
    /// Construct expression for reified linear relation
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const LinRel& rl);
#ifdef GECODE_HAS_SET_VARS
    /// Construct expression for reified set relation
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const SetRel& rs);
    /// Construct expression for reified set relation
    GECODE_MINIMODEL_EXPORT
    BoolExpr(const SetCmpRel& rs);
#endif
    /// Construct expression for miscellaneous Boolean expression
    GECODE_MINIMODEL_EXPORT
    explicit BoolExpr(MiscExpr* m);
    /// Post propagators for expression
    BoolVar expr(Home home, IntConLevel icl) const;
    /// Post propagators for relation
    void rel(Home home, IntConLevel icl) const;
    /// Assignment operator
    GECODE_MINIMODEL_EXPORT
    const BoolExpr& operator =(const BoolExpr& e);
    /// Destructor
    GECODE_MINIMODEL_EXPORT
    ~BoolExpr(void);
  };

  /**
   * \defgroup TaskModelMiniModelBool Boolean expressions
   *
   * Boolean expressions can be freely composed of variables with
   * the usual connectives and reified linear expressions.
   *
   * \ingroup TaskModelMiniModel
   */

  //@{
  /// Negated Boolean expression
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator !(const BoolExpr&);
  /// Conjunction of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator &&(const BoolExpr&, const BoolExpr&);
  /// Disjunction of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator ||(const BoolExpr&, const BoolExpr&);
  /// Exclusive-or of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator ^(const BoolExpr&, const BoolExpr&);

  /// Non-equivalence of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator !=(const BoolExpr&, const BoolExpr&);
  /// Equivalence of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator ==(const BoolExpr&, const BoolExpr&);
  /// Implication of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator >>(const BoolExpr&, const BoolExpr&);
  /// Reverse implication of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator <<(const BoolExpr&, const BoolExpr&);

  //@}

  /**
   * \defgroup TaskModelMiniModelPost Posting of expressions and relations
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// Post linear expression and return its value
  GECODE_MINIMODEL_EXPORT IntVar 
  expr(Home home, const LinExpr& e, IntConLevel icl=ICL_DEF);
#ifdef GECODE_HAS_SET_VARS
  /// Post set expression and return its value
  GECODE_MINIMODEL_EXPORT SetVar
  expr(Home home, const SetExpr& e);
#endif
  /// Post Boolean expression and return its value
  GECODE_MINIMODEL_EXPORT BoolVar
  expr(Home home, const BoolExpr& e, IntConLevel icl=ICL_DEF);
  /// Post Boolean relation
  GECODE_MINIMODEL_EXPORT void 
  rel(Home home, const BoolExpr& e, IntConLevel icl=ICL_DEF);
  //@}

}

#include <gecode/minimodel/lin-expr.hpp>
#include <gecode/minimodel/lin-rel.hpp>
#include <gecode/minimodel/bool-expr.hpp>
#include <gecode/minimodel/set-expr.hpp>
#include <gecode/minimodel/set-rel.hpp>

namespace Gecode {

  namespace MiniModel {
    class ExpInfo;
  }

  /**
   * \brief Regular expressions over integer values
   *
   * \ingroup TaskModelMiniModel
   */
  class GECODE_MINIMODEL_EXPORT REG {
    friend class MiniModel::ExpInfo;
  private:
    /// Implementation of the actual expression tree
    class Exp;
    /// The expression tree
    Exp* e;
    /// Initialize with given expression tree \a
    REG(Exp* e);
  public:
    /// Initialize as empty sequence (epsilon)
    REG(void);
    /// Initialize as single integer \a s
    REG(int s);
    /**
     * \brief Initialize as alternative of integers
     *
     * Throws an exception of type MiniModel::TooFewArguments if \a x
     * is empty.
     */
    REG(const IntArgs& x);

    /// Initialize from regular expression \a r
    REG(const REG& r);
    /// Assign to regular expression \a r
    const REG& operator =(const REG& r);

    /// Return expression for: this expression followed by \a r
    REG operator +(const REG& r);
    /// This expression is followed by \a r
    REG& operator +=(const REG& r);
    /// Return expression for: this expression or \a r
    REG operator |(const REG& r);
    /// This expression or \a r
    REG& operator |=(const REG& r);
    /// Return expression for: this expression arbitrarily often (Kleene star)
    REG operator *(void);
    /// Return expression for: this expression at least once
    REG operator +(void);
    /// Return expression for: this expression at least \a n and at most \a m times
    REG operator ()(unsigned int n, unsigned int m);
    /// Return expression for: this expression at least \a n times
    REG operator ()(unsigned int n);
    /// Print expression
    template<class Char, class Traits>
    std::basic_ostream<Char,Traits>&
    print(std::basic_ostream<Char,Traits>& os) const;
    /// Return DFA for regular expression
    operator DFA(void);
    /// Destructor
    ~REG(void);
  };

  /** \relates Gecode::REG
   * Print regular expression \a r
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const REG& r);


  /**
   * \defgroup TaskModelMiniModelArith Arithmetic functions
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// \brief Return expression for \f$|e|\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  abs(const LinExpr& e);
  /// \brief Return expression for \f$\min(x,y)\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  min(const LinExpr& x, const LinExpr& y);
  /// \brief Return expression for \f$\min(x)\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  min(const IntVarArgs& x);
  /// \brief Return expression for \f$\max(x,y)\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  max(const LinExpr& x, const LinExpr& y);
  /// \brief Return expression for \f$\max(x)\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  max(const IntVarArgs& x);
  /// \brief Return expression for \f$x\cdot y\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  operator *(const LinExpr& x, const LinExpr& y);
  /// \brief Return expression for \f$x\ \mathrm{div}\ y\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  operator /(const LinExpr& x, const LinExpr& y);
  /// \brief Return expression for \f$x\ \mathrm{mod}\ y\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  operator %(const LinExpr& x, const LinExpr& y);
  /// \brief Return expression for \f$x^2\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  sqr(const LinExpr& x);
  /// \brief Return expression for \f$\lfloor\sqrt{x}\rfloor\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  sqrt(const LinExpr& x);
  /// \brief Return expression for \f$x[y]\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  element(const IntVarArgs& x, const LinExpr& y);
  /// \brief Return expression for \f$x[y]\f$
  GECODE_MINIMODEL_EXPORT BoolExpr
  element(const BoolVarArgs& x, const LinExpr& y);
  /// \brief Return expression for \f$x[y]\f$
  GECODE_MINIMODEL_EXPORT LinExpr
  element(const IntArgs& x, const LinExpr& y);
  //@}

  /**
   * \defgroup TaskModelMiniModelChannel Channel functions
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// Return Boolean variable equal to \f$x\f$
  inline BoolVar
  channel(Home home, IntVar x,
          IntConLevel icl=ICL_DEF) {
    (void) icl;
    BoolVar b(home,0,1); channel(home,b,x);
    return b;
  }
  /// Return integer variable equal to \f$b\f$
  inline IntVar
  channel(Home home, BoolVar b,
          IntConLevel icl=ICL_DEF) {
    (void) icl;
    IntVar x(home,0,1); channel(home,b,x);
    return x;
  }
#ifdef GECODE_HAS_SET_VARS 
  /// Return set variable equal to \f$\{x_0,\dots,x_{n-1}\}\f$
  inline SetVar
  channel(Home home, const IntVarArgs& x, IntConLevel icl=ICL_DEF) {
    (void) icl;
    SetVar s(home,IntSet::empty,Set::Limits::min,Set::Limits::max);
    rel(home,SOT_UNION,x,s);
    nvalues(home,x,IRT_EQ,expr(home,cardinality(s)));
    return s;
  }
#endif
  //@}

}

namespace Gecode {

  /**
   * \defgroup TaskModelMiniModelIntAlias Aliases for integer constraints
   *
   * Contains definitions of common constraints which have different
   * names in Gecode.
   *
   * \ingroup TaskModelMiniModel
   */

  //@{
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, int n, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, const IntArgs& y, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, int n, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,z,icl);
  }

  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,z,icl);
  }

  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,m,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,z,icl);
  }
  /** \brief Post constraint \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,z,icl);
  }
  /** \brief Post lexical order between \a x and \a y.
   */
  inline void
  lex(Home home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl=ICL_DEF) {
    rel(home,x,r,y,icl);
  }
  /** \brief Post lexical order between \a x and \a y.
   */
  inline void
  lex(Home home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel icl=ICL_DEF) {
    rel(home,x,r,y,icl);
  }
  /** \brief Post constraint \f$\{x_0,\dots,x_{n-1}\}=y\f$
   */
  inline void
  values(Home home, const IntVarArgs& x, IntSet y,
         IntConLevel icl=ICL_DEF) {
    dom(home,x,y,icl);
    nvalues(home,x,IRT_EQ,y.size(),icl);
  }

  //@}

#ifdef GECODE_HAS_SET_VARS
  /**
   * \defgroup TaskModelMiniModelSetAlias Aliases for set constraints
   *
   * Contains definitions of common constraints which have different
   * names in Gecode.
   *
   * \ingroup TaskModelMiniModel
   */

  //@{
  /** \brief Post constraint \f$\{x_0,\dots,x_{n-1}\}=y\f$
   * 
   * In addition to constraining \a y to the union of the \a x, this
   * also posts an nvalue constraint for additional cardinality propagation.
   */
  inline void
  channel(Home home, const IntVarArgs& x, SetVar y) {
    rel(home,SOT_UNION,x,y);
    nvalues(home,x,IRT_EQ,expr(home,cardinality(y)));
  }
  
  /** \brief Post constraint \f$\bigcup_{i\in y}\{x_i\}=z\f$
   */
  inline void
  range(Home home, const IntVarArgs& x, SetVar y, SetVar z) {
    element(home,SOT_UNION,x,y,z);
  }

  /** \brief Post constraint \f$\bigcup_{i\in z}\{j\ |\ x_j=i\}=z\f$
   *
   * Note that this creates one temporary set variable for each element
   * in the upper bound of \a z, so make sure that the bound is tight.
   */
  inline void
  roots(Home home, const IntVarArgs& x, SetVar y, SetVar z) {
    SetVarArgs xiv(home,z.lubMax()+1,IntSet::empty,0,x.size()-1);
    channel(home,x,xiv);
    element(home,SOT_UNION,xiv,z,y);
  }
  
  //@}
#endif

}

namespace Gecode {

  template<class> class Matrix;

  /** \brief A slice of a matrix.
   *
   * This class represents a slice of the matrix. It is used to get
   * context-dependent behaviour. The slice will be automatically
   * converted to an ArgsType Args-array or to a Matrix<ArgsType>
   * depending on the context where it is used.
   */
  template<class A>
  class Slice {
  public:
    /// The type of the Args-array type for ValueType values
    typedef typename ArrayTraits<A>::ArgsType ArgsType;
  private:
    ArgsType _r;     ///< The elements of the slice
    unsigned int _fc, ///< From column
      _tc,            ///< To column
      _fr,            ///< From row
      _tr;            ///< To row
  public:
    /// Construct slice
    Slice(const Matrix<A>& a, int fc, int tc, int fr, int tr);
    /** \brief Reverses the contents of the slice, and returns a
     *  reference to it.
     */
    Slice& reverse(void);
    /// Cast to array type
    operator ArgsType(void);
    /// Cast to matrix type
    operator Matrix<ArgsType>(void);

    /// Cast to array type
    operator const ArgsType(void) const;
    /// Cast to matrix type
    operator const Matrix<ArgsType>(void) const;
  };
  
  /// Concatenate \a x and \a y
  template<class A>
  typename Slice<A>::ArgsType
  operator+(const Slice<A>& x, const Slice<A>& y);

  /// Concatenate \a x and \a y
  template<class A>
  typename Slice<A>::ArgsType
  operator+(const Slice<A>& x, const typename ArrayTraits<A>::ArgsType& y);

  /// Concatenate \a x and \a y
  template<class A>
  typename Slice<A>::ArgsType
  operator+(const typename ArrayTraits<A>::ArgsType& x, const Slice<A>& y);

  /// Concatenate \a x and \a y
  template<class A>
  typename Slice<A>::ArgsType
  operator+(const Slice<A>& x, const typename ArrayTraits<A>::ValueType& y);

  /// Concatenate \a x and \a y
  template<class A>
  typename Slice<A>::ArgsType
  operator+(const typename ArrayTraits<A>::ValueType& x, const Slice<A>& y);

  /** \brief Matrix-interface for arrays
   *
   * This class allows for wrapping some array and accessing it as a
   * matrix.
   *
   * \note This is a light-weight wrapper, and is not intended for
   * storing variables directly instead of in an array.
   *
   * \ingroup TaskModelMiniModel
   */
  template<class A>
  class Matrix {
  public:
    /// The type of elements of this array
    typedef typename ArrayTraits<A>::ValueType ValueType;
    /// The type of the Args-array type for ValueType values
    typedef typename ArrayTraits<A>::ArgsType ArgsType;

  private:
    /// The type of storage for this array
    typedef typename ArrayTraits<A>::StorageType StorageType;
    StorageType _a; ///< The array wrapped
    int _w; ///< The width of the matrix
    int _h; ///< The height of the matrix

  public:
    /** \brief Basic constructor
     *
     * Constructs a Matrix from the array \a a, using \a w and \a h as
     * the width and height of the matrix.
     *
     * The elements in the wrapped array \a a are accessed in
     * row-major order.
     *
     * \exception MiniModel::ArgumentSizeMismatch Raised if the
     *            parameters \a w and \a h doesn't match the size
     *            of the array \a a.
     */
    Matrix(A a, int w, int h);

    /** \brief Basic constructor
     *
     * Constructs a square Matrix from the array \a a, using \a n as
     * the length of the sides.
     *
     * The elements in the wrapped array \a a are accessed in
     * row-major order.
     *
     * \exception MiniModel::ArgumentSizeMismatch Raised if the
     *            parameter \a n doesn't match the size
     *            of the array \a a.
     */
    Matrix(A a, int n);

    /// Return the width of the matrix
    int width(void) const;
    /// Return the height of the matrix
    int height(void) const;
    /// Return an Args-array of the contents of the matrix
    ArgsType const get_array(void) const;

    /** \brief Access element (\a c, \a r) of the matrix
     *
     * \exception MiniModel::ArgumentOutOfRange Raised if \a c or \a r
     *            are out of range.
     */
    ValueType& operator ()(int c, int r);

    /** \brief Access element (\a c, \a r) of the matrix
     *
     * \exception MiniModel::ArgumentOutOfRange Raised if \a c or \a r
     *            are out of range.
     */
    const ValueType& operator ()(int c, int r) const;

    /** \brief Access slice of the matrix
     *
     * This function allows accessing a slice of the matrix, located at
     * columns \f$[fc,tc)\f$ and rows \f$[fr,tr)\f$. The result of this
     * function is an object that can be converted into either a
     * Matrix<ArgsType> or into ArgsType.
     *
     * For further information, see Slice.
     */
    Slice<A> slice(int fc, int tc, int fr, int tr) const;

    /// Access row \a r.
    Slice<A> row(int r) const;

    /// Access column \a c.
    Slice<A> col(int c) const;
  };

  /** \relates Gecode::Matrix
   * Print matrix \a m
   */
  template<class Char, class Traits, class A>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Matrix<A>& m);

  /** \relates Gecode::Matrix
   * Print slice \a s
   */
  template<class Char, class Traits, class A>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const Slice<A>& s);

  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<IntArgs>& m, IntVar x, IntVar y,  
               IntVar z, IntConLevel icl=ICL_DEF);
  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<IntArgs>& m, IntVar x, IntVar y,  
               BoolVar z, IntConLevel icl=ICL_DEF);
  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<IntVarArgs>& m, IntVar x, IntVar y,  
               IntVar z, IntConLevel icl=ICL_DEF);
  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<BoolVarArgs>& m, IntVar x, IntVar y,  
               BoolVar z, IntConLevel icl=ICL_DEF);
#ifdef GECODE_HAS_SET_VARS
  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<IntSetArgs>& m, IntVar x, IntVar y,  
               SetVar z);
  /** \brief Element constraint for matrix
   *
   * Here, \a x and \a y are the coordinates and \a z is the value
   * at position \a m(x,y).
   * \relates Gecode::Matrix
   */
  void element(Home home, const Matrix<SetVarArgs>& m, IntVar x, IntVar y,  
               SetVar z);
#endif

}

#include <gecode/minimodel/matrix.hpp>

namespace Gecode {

  /**
   * \defgroup TaskModelMiniModelOptimize Support for cost-based optimization
   *
   * Provides for minimizing or maximizing the cost value as defined by
   * a cost-member function of a space.
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  namespace MiniModel {

    /// Baseclass for cost-based optimization
    template<IntRelType irt>
    class OptimizeSpace : public Space {
    public:
      /// Default constructor
      OptimizeSpace(void);
      /// Constructor for cloning
      OptimizeSpace(bool share, OptimizeSpace& s);
      /// Member function constraining according to cost
      virtual void constrain(const Space& best);
      /// Return variable with current cost
      virtual IntVar cost(void) const = 0;
    };

  }

  /// Class for minimizing cost
  typedef MiniModel::OptimizeSpace<IRT_LE> MinimizeSpace;

  /// Class for maximizing cost
  typedef MiniModel::OptimizeSpace<IRT_GR> MaximizeSpace;
  //@}

}

#include <gecode/minimodel/optimize.hpp>

#endif

// IFDEF: GECODE_HAS_INT_VARS
// STATISTICS: minimodel-any

