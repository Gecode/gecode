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

#include "gecode/kernel.hh"
#include "gecode/int.hh"
#include "gecode/int/linear.hh"

#include "gecode/minimodel/exception.hpp"

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

namespace Gecode {

  /// Minimal modelling support
  namespace MiniModel {

    /// Linear expressions
    template <class Var>
    class LinExpr {
    public:
      /// Type of linear expression
      enum NodeType {
        NT_VAR, ///< Linear term with variable
        NT_ADD, ///< Addition of linear terms
        NT_SUB, ///< Subtraction of linear terms
        NT_MUL  ///< Multiplication by coefficient
      };
    private:
      typedef typename VarViewTraits<Var>::View View;
      /// Nodes for linear expressions
      class Node {
      public:
        /// Nodes are reference counted
        unsigned int use;
        /// Variables in tree
        unsigned int n;
        /// Type of expression
        NodeType t;
        /// Subexpressions
        Node *l, *r;
        /// Coefficient and offset
        int a, c;
        /// Variable (potentially)
        Var x;
        /// Default constructor
        Node(void);
        /// Generate linear terms from expression
        int fill(Int::Linear::Term<View> t[], int i, int m, 
                 int c_i, int& c_o) const;
        /// Decrement reference count and possibly free memory
        bool decrement(void);
        /// Memory management
        static void* operator new(size_t size);
        /// Memory management
        static void  operator delete(void* p,size_t size);
      };
      Node* n;       
    public:
      /// Default constructor
      LinExpr(void);
      /// Create expression
      LinExpr(const Var& x, int a=1);
      /// Copy constructor
      LinExpr(const LinExpr& e);
      /// Create expression for type and subexpressions
      LinExpr(const LinExpr& e0, NodeType t, const LinExpr& e1);
      /// Create expression for type and subexpression
      LinExpr(const LinExpr& e0, NodeType t, int c);
      /// Create expression for multiplication
      LinExpr(int a, const LinExpr& e);
      /// Assignment operator
      const LinExpr& operator=(const LinExpr& e);
      /// Post propagator
      void post(Space& home, IntRelType irt, 
                IntConLevel icl, PropKind pk) const;
      /// Post reified propagator
      void post(Space& home, IntRelType irt, const BoolVar& b,
                IntConLevel icl, PropKind pk) const;
      /// Post propagator and return variable for value
      IntVar post(Space& home, 
                  IntConLevel icl, PropKind pk) const;
      /// Destructor
      ~LinExpr(void);
    };

    /// Linear relations
    template<class Var>
    class LinRel {
    private:
      /// Linear expression describing the entire relation
      LinExpr<Var> e;
      /// Which relation
      IntRelType   irt;
      /// Negate relation type
      static IntRelType neg(IntRelType irt);
    public:
      /// Default constructor
      LinRel(void);
      /// Create linear relation for expressions \a l and \a r
      LinRel(const LinExpr<Var>& l, IntRelType irt, const LinExpr<Var>& r);
      /// Create linear relation for expression \a l and integer \a r
      LinRel(const LinExpr<Var>& l, IntRelType irt, int r);
      /// Create linear relation for integer \a l and expression \a r
      LinRel(int l, IntRelType irt, const LinExpr<Var>& r);
      /// Post propagator for relation (if \a t is false for negated relation)
      void post(Space& home, bool t, 
                IntConLevel icl, PropKind pk) const;
      /// Post reified propagator for relation
      void post(Space& home, const BoolVar& b,
                IntConLevel icl, PropKind pk) const;
    };

  }

}

/**
 * \defgroup TaskModelMiniModelLin Linear expressions and relations
 *
 * Linear expressions can be freely composed of sums and differences of
 * integer variables (Gecode::IntVar) or Boolean variables (Gecode::BoolVar)
 * possibly with integer coefficients and integer constants.
 *
 * Note that both integer and Boolean variables are automatically available 
 * as linear expressions.
 *
 * Linear relations are obtained from linear expressions with the normal
 * relation operators.
 *
 * \ingroup TaskModelMiniModel
 */

//@{

/// Construct linear expression as sum of linear expression and integer
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator+(int,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);
/// Construct linear expression as sum of integer and linear expression
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator+(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&,
          int);
/// Construct linear expression as sum of linear expressions
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator+(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);
/// Construct linear expression as difference of linear expression and integer
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator-(int,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);
/// Construct linear expression as difference of integer and linear expression
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&,
          int);
/// Construct linear expression as difference of linear expressions
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);
/// Construct linear expression as negative of linear expression
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);

/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator*(int, const Gecode::IntVar&);
/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator*(const Gecode::IntVar&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator*(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr<Gecode::IntVar>
operator*(int, const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);


/// Construct linear expression as sum of linear expression and integer
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator+(int,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);
/// Construct linear expression as sum of integer and linear expression
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator+(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&,
          int);
/// Construct linear expression as sum of linear expressions
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator+(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);
/// Construct linear expression as difference of linear expression and integer
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator-(int,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);
/// Construct linear expression as difference of integer and linear expression
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&,
          int);
/// Construct linear expression as difference of linear expressions
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);
/// Construct linear expression as negative of linear expression
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator-(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);

/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator*(int, const Gecode::BoolVar&);
/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator*(const Gecode::BoolVar&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator*(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr<Gecode::BoolVar>
operator*(int, const Gecode::MiniModel::LinExpr<Gecode::BoolVar>&);


/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator==(int l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator==(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           int r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator==(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);

/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator!=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator!=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           int r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator!=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<(int l,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
          int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator<=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>(int l,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
          int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
          const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::IntVar>
operator>=(const Gecode::MiniModel::LinExpr<Gecode::IntVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::IntVar>& r);


/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator==(int l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator==(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           int r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator==(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator!=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator!=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           int r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator!=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<(int l,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
          int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator<=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>(int l,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
          int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
          const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>=(int l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel<Gecode::BoolVar>
operator>=(const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& l,
           const Gecode::MiniModel::LinExpr<Gecode::BoolVar>& r);

//@}

namespace Gecode {

  namespace MiniModel {

    /// Boolean expressions
    class BoolExpr {
    public:
      /// Type of Boolean expression
      enum NodeType {
        NT_VAR,       ///< Variable
        NT_NOT,       ///< Negation
        NT_AND,       ///< Conjunction
        NT_OR,        ///< Disjunction
        NT_IMP,       ///< Implication
        NT_XOR,       ///< Exclusive or
        NT_EQV,       ///< Equivalence
        NT_RLIN_INT,  ///< Reified linear relation (integer variables)
        NT_RLIN_BOOL  ///< Reified linear relation (Boolean variables)
      };
      /// Node for Boolean expression
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
        /// Possibly a reified linear relation over integer variables
        LinRel<IntVar> rl_int;
        /// Possibly a reified linear relation over Boolean variables
        LinRel<BoolVar> rl_bool;

        /// Default constructor
        Node(void);
        /// Decrement reference count and possibly free memory
        GECODE_MINIMODEL_EXPORT 
        bool decrement(void);
        /// Post propagators for nested conjunctive and disjunctive expression
        GECODE_MINIMODEL_EXPORT         
        int post(Space& home, NodeType t, BoolVarArgs& b, int i,
                 IntConLevel icl, PropKind pk) const;
        /// Post propagators for expression
        GECODE_MINIMODEL_EXPORT
        void post(Space& home, BoolVar b,
                  IntConLevel icl, PropKind pk) const;
        /// Post propagators for expression
        GECODE_MINIMODEL_EXPORT
        BoolVar post(Space& home,
                     IntConLevel icl, PropKind pk) const;
        /// Post propagators for relation
        GECODE_MINIMODEL_EXPORT
        void post(Space& home, bool t,
                  IntConLevel icl, PropKind pk) const;
        
        /// Memory management
        static void* operator new(size_t size);
        /// Memory management
        static void  operator delete(void* p,size_t size);
      };
    private:
      /// Pointer to node for expression
      Node* n;
    public:
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
      BoolExpr(const LinRel<IntVar>& rl);
      /// Construct expression for reified linear relation
      GECODE_MINIMODEL_EXPORT 
      BoolExpr(const LinRel<BoolVar>& rl);
      /// Post propagators for expression
      BoolVar post(Space& home, IntConLevel icl, PropKind pk) const;
      /// Post propagators for relation
      void post(Space& home, bool t, IntConLevel icl, PropKind pk) const;

      /// Assignment operator
      GECODE_MINIMODEL_EXPORT 
      const BoolExpr& operator=(const BoolExpr& e);
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
      void post(Space& home, IntConLevel icl, PropKind pk) const;
    };
  }

}

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
Gecode::MiniModel::BoolExpr
operator!(const Gecode::MiniModel::BoolExpr&);

/// Conjunction of Boolean expressions
Gecode::MiniModel::BoolExpr
operator&&(const Gecode::MiniModel::BoolExpr&,
           const Gecode::MiniModel::BoolExpr&);

/// Disjunction of Boolean expressions
Gecode::MiniModel::BoolExpr
operator||(const Gecode::MiniModel::BoolExpr&,
           const Gecode::MiniModel::BoolExpr&);

/// Exclusive-or of Boolean expressions
Gecode::MiniModel::BoolExpr
operator^(const Gecode::MiniModel::BoolExpr&,
          const Gecode::MiniModel::BoolExpr&);

/// Reification of linear relations
template <class Var>
Gecode::MiniModel::BoolExpr
operator~(const Gecode::MiniModel::LinRel<Var>&);

namespace Gecode {

  /// Equivalence of Boolean expressions
  MiniModel::BoolExpr
  eqv(const MiniModel::BoolExpr&,
      const MiniModel::BoolExpr&);
  /// Implication of Boolean expressions
  MiniModel::BoolExpr
  imp(const MiniModel::BoolExpr&,
      const MiniModel::BoolExpr&);

  /// State that Boolean expression must be true
  MiniModel::BoolRel
  tt(const MiniModel::BoolExpr&);

  /// State that Boolean expression must be false
  MiniModel::BoolRel
  ff(const MiniModel::BoolExpr&);

}

//@}

namespace Gecode {

  /**
   * \defgroup TaskModelMiniModelPost Posting of expressions and relations
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// Post linear expression and return its value
  template <class Var>
  IntVar post(Space& home, const MiniModel::LinExpr<Var>& e,
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /// Post linear expression (special case for integer variable) and return its value
  IntVar post(Space& home, const IntVar& x,
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /// Post linear expression (special case for constant) and return its value
  IntVar post(Space& home, int n,
              IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

  /// Post linear relation
  template <class Var>
  void post(Space& home, const MiniModel::LinRel<Var>& r,
            IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /// Make it work for special integer only-case
  void post(Space& home, bool r,
            IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

  /// Post Boolean expression and return its value
  BoolVar post(Space& home, const MiniModel::BoolExpr& e,
               IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /// Post Boolean expression (special case for variable) and return its value
  BoolVar post(Space& home, const BoolVar& b,
               IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /// Post Boolean relation
  void post(Space& home, const MiniModel::BoolRel& r,
            IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  //@}

}

#include "gecode/minimodel/lin-expr.hpp"
#include "gecode/minimodel/lin-rel.hpp"
#include "gecode/minimodel/bool-expr.hpp"
#include "gecode/minimodel/bool-rel.hpp"

namespace Gecode {

  /**
   * \brief Regular expressions over integer values
   *
   * \ingroup TaskModelMiniModel
   */
  class GECODE_MINIMODEL_EXPORT REG {
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
    const REG& operator=(const REG& r);

    /// Return expression for: this expression followed by \a r
    REG operator+(const REG& r);
    /// This expression is followed by \a r
    REG& operator+=(const REG& r);
    /// Return expression for: this expression or \a r
    REG operator|(const REG& r);
    /// This expression or \a r
    REG& operator|=(const REG& r);
    /// Return expression for: this expression arbitrarily often (Kleene star)
    REG operator*(void);
    /// Return expression for: this expression at least once
    REG operator+(void);
    /// Return expression for: this expression at least \a n and at most \a m times
    REG operator()(unsigned int n, unsigned int m);
    /// Return expression for: this expression at least \a n times
    REG operator()(unsigned int n);
    /// Print expression
    std::ostream& print(std::ostream&) const;
    /// Return DFA for regular expression
    operator DFA(void);
    /// Destructor
    ~REG(void);
  };

}

/** \relates Gecode::REG
 * Print regular expression \a r
 */
GECODE_MINIMODEL_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::REG& r);


namespace Gecode {

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
  abs(Space& home, IntVar x, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$\min(x,y)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  min(Space& home, IntVar x, IntVar y, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$\min(x)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  min(Space& home, const IntVarArgs& x, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$\max(x,y)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  max(Space& home, IntVar x, IntVar y, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$\max(x)\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  max(Space& home, const IntVarArgs& x, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$x\cdot y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  mult(Space& home, IntVar x, IntVar y, 
       IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$x^2\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  sqr(Space& home, IntVar x, 
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$\lfloor\sqrt{x}\rfloor\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  sqrt(Space& home, IntVar x, 
       IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** \brief Return variable constrained to \f$x+y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  plus(Space& home, IntVar x, IntVar y, 
       IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  /** Return variable constrained to \f$x-y\f$
   *
   * Supports both bounds consistency (\a icl = ICL_BND, default)
   * and domain consistency (\a icl = ICL_DOM).
   */
  IntVar
  minus(Space& home, IntVar x, IntVar y, 
        IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
  //@}
}

#include "gecode/minimodel/arithmetic.hpp"

namespace Gecode {

  /**
   * \defgroup TaskModelMiniModelChannel Channel functions
   *
   * \ingroup TaskModelMiniModel
   */
  //@{
  /// Return Boolean variable equal to \f$x\f$
  inline BoolVar
  channel(Space& home, IntVar x, 
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    (void) icl; (void) pk;
    BoolVar b(home,0,1); channel(home,b,x);
    return b;
  }
  /// Return integer variable equal to \f$b\f$
  inline IntVar
  channel(Space& home, BoolVar b, 
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    (void) icl; (void) pk;
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
  atmost(Space& home, const IntVarArgs& x, int n, int m,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_LQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Space& home, const IntVarArgs& x, IntVar y, int m,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_LQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Space& home, const IntVarArgs& x, const IntArgs& y, int m,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_LQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Space& home, const IntVarArgs& x, int n, IntVar z,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_LQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atmost(Space& home, const IntVarArgs& x, IntVar y, IntVar z,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_LQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Space& home, const IntVarArgs& x, const IntArgs& y, IntVar z,
         IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_LQ,z,icl,pk);
  }

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_GQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_GQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_GQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_GQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_GQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Space& home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_GQ,z,icl,pk);
  }

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_EQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=m\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_EQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=m\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_EQ,m,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,n,IRT_EQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=z\f$
   *
   * Supports domain consistent propagation only.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_EQ,z,icl,pk);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=z\f$
   *
   * Supports domain consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Space& home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    count(home,x,y,IRT_EQ,z,icl,pk);
  }
  /** \brief Post propagator for lexical order between \a x and \a y.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  inline void
  lex(Space& home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    rel(home,x,r,y,icl,pk);
  }
  /** \brief Post propagator for lexical order between \a x and \a y.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   * \a x and \a y are of different size.
   */
  inline void
  lex(Space& home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF) {
    rel(home,x,r,y,icl,pk);
  }

  //@}

}

/**
 * \addtogroup TaskModelMiniModelMatrix Matrix interface for arrays
 *
 * Wrapper for arrays to act as a two-dimensional matrix.
 *
 * \ingroup TaskModelMiniModel
 */
//@{
namespace Gecode {

  /** \brief Matrix-interface for arrays
   *
   * This class allows for wrapping some array and accessing it as a
   * matrix.
   *
   * \note This is a light-weight wrapper, and is not intended for
   * storing variables directly instead of in an array.
   *
   * \ingroup TaskModelMiniModelMatrix
   */
  template <class A>
  class Matrix {
  public:
    /// The type of elements of this array
    typedef typename ArrayTraits<A>::value_type value_type;
    /// The type of the Args-array type for value_type values.
    typedef typename ArrayTraits<A>::args_type  args_type;

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
      Slice(Matrix<A>& a,
            unsigned int fc, unsigned int tc,
            unsigned int fr, unsigned int tr);

      /** \brief Reverses the contents of the slice, and returns a
       *  reference to it.
       */
      Slice& reverse(void);

      operator args_type(void);
      operator Matrix<args_type>(void);
    };

  private:
    /// The type of storage for this array
    typedef typename ArrayTraits<A>::storage_type storage_type;
    storage_type _a; ///< The array wrapped
    unsigned int _w, ///< The width of the matrix
      _h;            ///< The height of the matrix

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
    Matrix(A a, unsigned int w, unsigned int h);

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
    Matrix(A a, unsigned int n);

    /// Return the width of the matrix
    unsigned int width(void) const;
    /// Return the height of the matrix
    unsigned int height(void) const;
    /// Return an Args-array of the contents of the matrix
    args_type const get_array(void);

    /** \brief Access element (\a c, \a r) of the matrix
     *
     * \exception MiniModel::ArgumentOutOfRange Raised if \a c or \a r
     *            are out of range.
     */
    value_type& operator()(unsigned int c, unsigned int r);

    /** \brief Access slice of the matrix
     *
     * This function allows accessing a slice of the matrix, located at
     * columns \f$[fc,tc)\f$ and rows \f$[fr,tr)\f$. The result of this
     * function is an object that can be converted into either a
     * Matrix<args_type> or into args_type.
     *
     * For further information, see Slice.
     */
    Slice slice(unsigned int fc, unsigned int tc,
                unsigned int fr, unsigned int tr);

    /// Access row \a r.
    Slice row(int r);

    /// Access column \a c.
    Slice col(int c);
  };
}
//@}

#include "gecode/minimodel/matrix.hpp"

namespace Gecode {

/**
 * \defgroup TaskModelMiniModelOptimize Support for optimization
 *
 * Provides for minimizing or maximizing the cost value as defined by
 * a cost-member function of a space.
 *
 * \ingroup TaskModelMiniModel
 */
//@{
  namespace MiniModel {

    /// Baseclass for cost-based optimization
    template <IntRelType irt>
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

#include "gecode/minimodel/optimize.hpp"

#endif

// IFDEF: GECODE_HAS_INT_VARS
// STATISTICS: minimodel-any

