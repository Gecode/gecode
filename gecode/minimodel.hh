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

  /// Linear expressions
  class LinExpr {
    friend class LinRel;
  public:
    /// Type of linear expression
    enum NodeType {
      NT_VAR_INT,  ///< Linear term with integer variable
      NT_VAR_BOOL, ///< Linear term with Boolean variable
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
      unsigned int n_int;
      /// Boolean variables in tree
      unsigned int n_bool;
      /// Type of expression
      NodeType t;
      /// Subexpressions
      Node *l, *r;
      /// Sum of integer or Boolean variables
      union {
        /// Integer views and coefficients
        Int::Linear::Term<Int::IntView>* ti;
        /// Bool views and coefficients
        Int::Linear::Term<Int::BoolView>* tb;
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
      void fill(Int::Linear::Term<Int::IntView>*& ti,
                Int::Linear::Term<Int::BoolView>*& tb,
                double m, double& d) const;
      /// Generate linear terms for expressions
      int fill(Int::Linear::Term<Int::IntView>* ti,
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
    /// Default constructor
    LinExpr(void);
  public:
    /// Create expression
    LinExpr(const IntVar& x, int a=1);
    /// Create expression
    LinExpr(const BoolVar& x, int a=1);
    /// Create sum expression
    explicit LinExpr(const IntVarArgs& x);
    /// Create sum expression
    LinExpr(const IntArgs& a, const IntVarArgs& x);
    /// Create sum expression
    explicit LinExpr(const BoolVarArgs& x);
    /// Create sum expression
    LinExpr(const IntArgs& a, const BoolVarArgs& x);
    /// Copy constructor
    LinExpr(const LinExpr& e);
    /// Create expression for type and subexpressions
    LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1);
    /// Create expression for type and subexpression
    LinExpr(const LinExpr& e0, NodeType t, int c);
    /// Create expression for multiplication
    LinExpr(int a, const LinExpr& e);
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
   * relation operator s.
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


  /// Boolean expressions
  class BoolExpr {
  public:
    /// Type of Boolean expression
    enum NodeType {
      NT_VAR, ///< Variable
      NT_NOT, ///< Negation
      NT_AND, ///< Conjunction
      NT_OR,  ///< Disjunction
      NT_EQV, ///< Equivalence
      NT_RLIN ///< Reified linear relation
    };
    /// %Node for Boolean expression
    class Node {
    public:
      /// Nodes are reference counted
      unsigned int use;
      /// Number of variables in subtree with same type (for AND and OR)
      unsigned int same;
      /// Type of expression
      NodeType t;
      /// Subexpressions
      Node *l, *r;
      /// Possibly a variable
      BoolVar x;
      /// Possibly a reified linear relation
      LinRel rl;

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
      unsigned int p;
      /// Number of negative literals for node type
      unsigned int n;
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
      BoolVar post(Home home, IntConLevel icl) const;
      /// Post propagators for relation
      GECODE_MINIMODEL_EXPORT
      void post(Home home, bool t, IntConLevel icl) const;
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
    /// Copy constructor
    BoolExpr(const BoolExpr& e);
    /// Construct expression for type and subexpresssions
    BoolExpr(const BoolExpr& l, NodeType t, const BoolExpr& r);
    /// Construct expression for variable
    BoolExpr(const BoolVar& x);
    /// Construct expression for negation
    BoolExpr(const BoolExpr& e, NodeType t);
    /// Construct expression for reified linear relation
    BoolExpr(const LinRel& rl);
    /// Post propagators for expression
    BoolVar post(Home home, IntConLevel icl) const;
    /// Post propagators for relation
    void post(Home home, bool t, IntConLevel icl) const;
    /// Assignment operator
    GECODE_MINIMODEL_EXPORT
    const BoolExpr& operator =(const BoolExpr& e);
    /// Destructor
    GECODE_MINIMODEL_EXPORT
    ~BoolExpr(void);
  };

  /// Boolean relations
  class BoolRel {
  private:
    /// Expression
    BoolExpr e;
    /// Whether expression is true or false
    bool t;
  public:
    /// Constructor
    BoolRel(const BoolExpr& e, bool t);
    /// Post propagators for relation
    void post(Home home, IntConLevel icl) const;
  };

  /**
   * \defgroup TaskModelMiniModelBool Boolean expressions and relations
   *
   * Boolean expressions can be freely composed of variables with
   * the usual connectives and reified linear expressions.
   *
   * Boolean relations are obtained from Boolean expressions with
   * functions \a tt (stating that the expression must be true)
   * and \a ff (stating that the expression must be false).
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
  /// Reification of linear relations
  GECODE_MINIMODEL_EXPORT BoolExpr
  operator ~(const LinRel&);

  /// Equivalence of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  eqv(const BoolExpr&, const BoolExpr&);
  /// Implication of Boolean expressions
  GECODE_MINIMODEL_EXPORT BoolExpr
  imp(const BoolExpr&, const BoolExpr&);

  /// State that Boolean expression must be true
  GECODE_MINIMODEL_EXPORT BoolRel
  tt(const BoolExpr&);

  /// State that Boolean expression must be false
  GECODE_MINIMODEL_EXPORT BoolRel
  ff(const BoolExpr&);
  //@}

  /**
   * \defgroup TaskModelMiniModelPost Posting of expressions and relations
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// Post linear expression and return its value
  GECODE_MINIMODEL_EXPORT IntVar 
  post(Home home, const LinExpr& e, IntConLevel icl=ICL_DEF);
  /// Post linear relation
  GECODE_MINIMODEL_EXPORT void 
  post(Home home, const LinRel& r, IntConLevel icl=ICL_DEF);
  /// Post Boolean expression and return its value
  GECODE_MINIMODEL_EXPORT BoolVar
  post(Home home, const BoolExpr& e, IntConLevel icl=ICL_DEF);
  /// Post Boolean relation
  GECODE_MINIMODEL_EXPORT void 
  post(Home home, const BoolRel& r, IntConLevel icl=ICL_DEF);
  //@}

}

#include <gecode/minimodel/lin-expr.hpp>
#include <gecode/minimodel/lin-rel.hpp>
#include <gecode/minimodel/bool-expr.hpp>
#include <gecode/minimodel/bool-rel.hpp>

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
  /** \brief Return variable constrained to \f$|x|\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  abs(Home home, IntVar x,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$\min(x,y)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  min(Home home, IntVar x, IntVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$\min(x)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  min(Home home, const IntVarArgs& x,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$\max(x,y)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  max(Home home, IntVar x, IntVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$\max(x)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  max(Home home, const IntVarArgs& x,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$x\cdot y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  mult(Home home, IntVar x, IntVar y,
       IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$x\ \mathrm{div}\ y\f$
   *
   * Supports bounds consistency (\a icl = ICL_BND, default).
   */
  IntVar
  div(Home home, IntVar x, IntVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$x\ \mathrm{mod}\ y\f$
   *
   * Supports bounds consistency (\a icl = ICL_BND, default).
   */
  IntVar
  mod(Home home, IntVar x, IntVar y,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$x^2\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  sqr(Home home, IntVar x,
      IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$\lfloor\sqrt{x}\rfloor\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  sqrt(Home home, IntVar x,
       IntConLevel icl=ICL_DEF);
  /** \brief Return variable constrained to \f$x+y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  plus(Home home, IntVar x, IntVar y,
       IntConLevel icl=ICL_DEF);
  /** Return variable constrained to \f$x-y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  minus(Home home, IntVar x, IntVar y,
        IntConLevel icl=ICL_DEF);
  //@}
}

#include <gecode/minimodel/arithmetic.hpp>

namespace Gecode {

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
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, int n, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq m\f$
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
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, int n, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq z\f$
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

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq m\f$
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
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq z\f$
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

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=m\f$
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
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=z\f$
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
  /** \brief Post propagator for lexical order between \a x and \a y.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  inline void
  lex(Home home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl=ICL_DEF) {
    rel(home,x,r,y,icl);
  }
  /** \brief Post propagator for lexical order between \a x and \a y.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  inline void
  lex(Home home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel icl=ICL_DEF) {
    rel(home,x,r,y,icl);
  }

  //@}

}

namespace Gecode {

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
    typedef typename ArrayTraits<A>::value_type value_type;
    /// The type of the Args-array type for value_type values
    typedef typename ArrayTraits<A>::args_type args_type;

    /** \brief A slice of a matrix.
     *
     * This class represents a slice of the matrix. It is used to get
     * context-dependent behaviour. The slice will be automatically
     * converted to an args_type Args-array or to a Matrix<args_type>
     * depending on the context where it is used.
     */
    class Slice {
      args_type _r;     ///< The elements of the slice
      unsigned int _fc, ///< From column
        _tc,            ///< To column
        _fr,            ///< From row
        _tr;            ///< To row
    public:
      /// Construct slice
      Slice(Matrix<A>& a, int fc, int tc, int fr, int tr);
      /** \brief Reverses the contents of the slice, and returns a
       *  reference to it.
       */
      Slice& reverse(void);
      /// Cast to array type
      operator typename Matrix<A>::args_type(void);
      /// Cast to matrix type
      operator Matrix<typename Matrix<A>::args_type>(void);
    };

  private:
    /// The type of storage for this array
    typedef typename ArrayTraits<A>::storage_type storage_type;
    storage_type _a; ///< The array wrapped
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
    args_type const get_array(void) const;

    /** \brief Access element (\a c, \a r) of the matrix
     *
     * \exception MiniModel::ArgumentOutOfRange Raised if \a c or \a r
     *            are out of range.
     */
    value_type& operator ()(int c, int r);

    /** \brief Access slice of the matrix
     *
     * This function allows accessing a slice of the matrix, located at
     * columns \f$[fc,tc)\f$ and rows \f$[fr,tr)\f$. The result of this
     * function is an object that can be converted into either a
     * Matrix<args_type> or into args_type.
     *
     * For further information, see Slice.
     */
    Slice slice(int fc, int tc, int fr, int tr);

    /// Access row \a r.
    Slice row(int r);

    /// Access column \a c.
    Slice col(int c);
  };

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

