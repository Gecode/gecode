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

#include "gecode/minimodel/exception.icc"

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

#ifdef GCC_HASCLASSVISIBILITY

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
    private:
      typedef typename VarViewTraits<Var>::View View;
      /// Nodes for linear expressions
      class Node {
      private:
        /// Nodes are reference counted
        unsigned int use;
        /// Left and right subtrees
        Node *left, *right;
        /// Left and right signs for entire subtress
        int signLeft, signRight;
        /// Coefficient
        int a;
        /// Variable
        Var x;
      public:
        /// Construct node for \a a multiplied with \a x
        Node(int a, const Var& x);
        /// Construct node from nodes \a n0 and \a n1 with signs \a s0 and \a s1
        Node(Node* n0, int s0, Node* n1, int s1);

        /// Increment reference count
        void increment(void);
        /// Decrement reference count and possibly free memory
        bool decrement(void);
        
        /// Fill in array of linear terms based on this node
        int fill(Int::Linear::Term<View> t[], int i, int s) const;
        
        /// Memory management
        static void* operator new(size_t size);
        /// Memory management
        static void  operator delete(void* p,size_t size);
      };
      Node* ax;       ///< Node for expression
    public:
      unsigned int n; ///< Number of variables in sub terms
      int c;          ///< Constant for expression
      int sign;       ///< Sign for expression
      /// Default constructor
      LinExpr(void);
      /// Copy constructor
      LinExpr(const LinExpr& e);
      /// Create expression \f$a\cdot x+c\f$
      LinExpr(const Var& x, int a=1, int c=0);
      /// Create expression \f$e_0+s\cdot e_1\f$ (where \a s is the sign)
      LinExpr(const LinExpr& e0, const LinExpr& e1, int s);
      /// Create expression \f$s\cdot e+c\f$ (where \a s is the sign)
      LinExpr(const LinExpr& e, int c, int s);
      /// Create expression \f$a\cdot e\f$
      LinExpr(int a, const LinExpr& e);
      /// Assignment operator
      const LinExpr& operator=(const LinExpr& e);
      /// Post propagator
      void post(Space* home, IntRelType irt, IntConLevel icl) const;
      /// Post reified propagator
      void post(Space* home, IntRelType irt, const BoolVar& b) const;
      /// Post propagator and return variable for value
      IntVar post(Space* home, IntConLevel icl) const;
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
      void post(Space* home, bool t, IntConLevel icl) const;
      /// Post refied propagator for relation
      void post(Space* home, const BoolVar& b) const;
    };

  }

}

/**
 * \defgroup TaskMiniModelLin Linear expressions and relations
 *
 * Linear expressions can be freely composed of sums and differences of
 * integer variables (Gecode::IntVar) possibly with integer coefficients
 * and integer constants.
 *
 * Note that integer variables are automatically available as linear
 * expressions.
 *
 * Linear relations are obtained from linear expressions with the normal
 * relation operators.
 *
 * \ingroup TaskMiniModel
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
        BT_VAR, ///< Variable
        BT_NOT, ///< Negation
        BT_AND, ///< Conjunction
        BT_OR,  ///< Disjunction
        BT_IMP, ///< Implication
        BT_XOR, ///< Exclusive or
        BT_EQV, ///< Equivalence
        BT_RLIN ///< Reified linear relation
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
        LinRel<IntVar> rl;

        /// Default constructor
        Node(void);
        /// Decrement reference count and possibly free memory
        GECODE_MINIMODEL_EXPORT 
        bool decrement(void);
        /// Post propagators for nested conjunctive and disjunctive expression
        GECODE_MINIMODEL_EXPORT         
        int post(Space* home, NodeType t, BoolVarArgs& b, int i) const;
        /// Post propagators for expression
        GECODE_MINIMODEL_EXPORT
        void post(Space* home, BoolVar b) const;
        /// Post propagators for expression
        GECODE_MINIMODEL_EXPORT
        BoolVar post(Space* home) const;
        /// Post propagators for relation
        GECODE_MINIMODEL_EXPORT
        void post(Space* home, bool t) const;
        
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
      BoolExpr(const BoolExpr& l, NodeType t, const BoolExpr& r);
      /// Construct expression for variable
      BoolExpr(const BoolVar& x);
      /// Construct expression for negation
      BoolExpr(const BoolExpr& e, NodeType t);
      /// Construct expression for reified linear relation
      BoolExpr(const LinRel<IntVar>& rl);
      /// Post propagators for expression
      BoolVar post(Space* home) const;
      /// Post propagators for relation
      void post(Space* home, bool t) const;

      /// Assignment operator
      const BoolExpr& operator=(const BoolExpr& e);
      /// Destructor
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
      void post(Space* home) const;
    };
  }

}

/**
 * \defgroup TaskMiniModelBool Boolean expressions and relations
 *
 * Boolean expressions can be freely composed of variables with
 * the usual connectives and reified linear expressions.
 *
 * Boolean relations are obtained from Boolean expressions with
 * functions \a tt (stating that the expression must be true)
 * and \a ff (stating that the expression must be false).
 *
 * \ingroup TaskMiniModel
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

/// Reification of linear expression
Gecode::MiniModel::BoolExpr
operator~(const Gecode::MiniModel::LinExpr<Gecode::IntVar>&);

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
   * \defgroup TaskMiniModelPost Posting of expressions and relations
   *
   * \ingroup TaskMiniModel
   */
  //@{
  /// Post linear expression and return its value
  template <class Var>
  IntVar post(Space* home, const MiniModel::LinExpr<Var>& e,
              IntConLevel icl=ICL_DEF);
  /// Post linear expression (special case for integer variable) and return its value
  IntVar post(Space* home, const IntVar& x,
              IntConLevel icl=ICL_DEF);
  /// Post linear expression (special case for constant) and return its value
  IntVar post(Space* home, int n,
              IntConLevel icl=ICL_DEF);

  /// Post linear relation
  template <class Var>
  void post(Space* home, const MiniModel::LinRel<Var>& r,
            IntConLevel icl=ICL_DEF);
  /// Make it work for special integer only-case
  void post(Space* home, bool r,
            IntConLevel icl=ICL_DEF);

  /// Post Boolean expression and return its value
  BoolVar post(Space* home, const MiniModel::BoolExpr& e,
               IntConLevel icl=ICL_DEF);
  /// Post Boolean expression (special case for variable) and return its value
  BoolVar post(Space* home, const BoolVar& b,
               IntConLevel icl=ICL_DEF);
  /// Post Boolean relation
  void post(Space* home, const MiniModel::BoolRel& r,
            IntConLevel icl=ICL_DEF);
  //@}

}

#include "gecode/minimodel/lin-expr.icc"
#include "gecode/minimodel/lin-rel.icc"
#include "gecode/minimodel/bool-expr.icc"
#include "gecode/minimodel/bool-rel.icc"

namespace Gecode {

  /**
   * \brief Regular expressions over integer values
   *
   * \ingroup TaskMiniModel
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
   * \defgroup TaskMiniModelArith Arithmetic functions
   *
   * \ingroup TaskMiniModel
   */
  //@{
  /// Return variable constrained to \f$|x|\f$
  GECODE_MINIMODEL_EXPORT IntVar
  abs(Space* home, IntVar x, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$\min(x,y)\f$
  GECODE_MINIMODEL_EXPORT IntVar
  min(Space* home, IntVar x, IntVar y, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$\min(x)\f$
  GECODE_MINIMODEL_EXPORT IntVar
  min(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$\max(x,y)\f$
  GECODE_MINIMODEL_EXPORT IntVar
  max(Space* home, IntVar x, IntVar y, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$\max(x)\f$
  GECODE_MINIMODEL_EXPORT IntVar
  max(Space* home, const IntVarArgs& x, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$x\cdot y\f$
  GECODE_MINIMODEL_EXPORT IntVar
  mult(Space* home, IntVar x, IntVar y, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$x^2\f$
  GECODE_MINIMODEL_EXPORT IntVar
  sqr(Space* home, IntVar x, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$x+y\f$
  GECODE_MINIMODEL_EXPORT IntVar
  plus(Space* home, IntVar x, IntVar y, IntConLevel icl=ICL_DEF);
  /// Return variable constrained to \f$x-y\f$
  GECODE_MINIMODEL_EXPORT IntVar
  minus(Space* home, IntVar x, IntVar y, IntConLevel icl=ICL_DEF);
  //@}

}

namespace Gecode {

  /**
   * \defgroup TaskMiniModelChannel Channel functions
   *
   * \ingroup TaskMiniModel
   */
  //@{
  /// Return Boolean variable equal to \f$x\f$
  inline BoolVar
  channel(Space* home, IntVar x, IntConLevel icl=ICL_DEF) {
    BoolVar b(home,0,1); channel(home,b,x);
    return b;
  }
  /// Return integer variable equal to \f$x\f$
  inline IntVar
  channel(Space* home, BoolVar b, IntConLevel icl=ICL_DEF) {
    IntVar x(home,0,1); channel(home,b,x);
    return x;
  }
  //@}

}

namespace Gecode {

  /**
   * \defgroup TaskMiniModelIntAlias Aliases for integer constraints
   *
   * Contains definitions of common constraints which have different
   * names in Gecode.
   *
   * \ingroup TaskMiniModel
   */

  //@{
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, int n, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, IntVar y, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq m\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, const IntArgs& y, int m,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\leq z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, int n, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_LQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\leq z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, IntVar y, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\leq z\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atmost(Space* home, const IntVarArgs& x, const IntArgs& y, IntVar z,
         IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_LQ,z,icl);
  }

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq m\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}\geq z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_GQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}\geq z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}\geq z\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  atleast(Space* home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_GQ,z,icl);
  }

  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, int n, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=m\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, IntVar y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=m\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, const IntArgs& y, int m,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,m,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=n\}=z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, int n, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,n,IRT_EQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y\}=z\f$
   *
   * Supports domain-consistent propagation only.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, IntVar y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,z,icl);
  }
  /** \brief Post propagator for \f$\#\{i\in\{0,\ldots,|x|-1\}\;|\;x_i=y_i\}=z\f$
   *
   * Supports domain-consistent propagation only.
   *
   * Throws an exception of type Int::ArgumentSizeMismatch, if
   *  \a x and \a y are of different size.
   */
  inline void
  exactly(Space* home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF) {
    count(home,x,y,IRT_EQ,z,icl);
  }
  /** \brief Post propagator for lexical order between \a x and \a y.
   *
   * - Throws an exception of type Int::ArgumentSizeMismatch, if
   *   \a x and \a y are of different size.
   */
  inline void
  lex(Space* home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntConLevel icl=ICL_DEF) {
    rel(home,x,r,y,icl);
  }

  //@}

}

namespace Gecode {

  /**
   * \defgroup TaskMiniModelScheduling Alternative interfaces to scheduling constraints
   *
   * \ingroup TaskMiniModel
   */

  //@{
  /** \brief Creates propagator for a producer-consumer constraint.
   *
   * This function will create a propagator that models a
   * producer-consumer constraint, using the translation of such
   * constraints into cumulatives due to Helmut Simoni and Trijntje
   * Cornelissens ("Modelling Producer/Consumer Constraints",
   * Principles and Practice of Constraint Progamming 1995, Cassis,
   * France).
   *
   * \param produce_date \f$ produce\_date_i \f$ is the date of event
   *                      \f$ i \f$.
   * \param produce_amount \f$ produce\_amount_i \f$ is the amount
   *                       produced by event \f$ i \f$.
   * \param consume_date \f$ consume\_date_j \f$ is the date of event
   *                     \f$ j \f$.
   * \param consume_amount \f$ consume\_amount_j \f$ is the amount
   *                       produced by event \f$ j \f$.
   * \param initial is the amount available at the start
   * \param icl Supports value consistency only (\c cl = ICL_VAL, default)
   *
   * \todo Make amounts variable, requires changes to cumulatives.
   *
   * \exception Int::ArgumentSizeMismatch Raised if the sizes of the arguments
   *            representing producer events or the sizes of the arguments
   *            representing consumer events does not match.
   * \exception Int::NumericalOverflow Raised if any numerical argument is
   *            larger than Limits::Int::int_max or less than
   *            Limits::Int::int_min.
   */
  GECODE_MINIMODEL_EXPORT void
  producer_consumer(Space *home,
                    const IntVarArgs& produce_date, const IntArgs& produce_amount,
                    const IntVarArgs& consume_date, const IntArgs& consume_amount,
                    int initial, IntConLevel icl=ICL_DEF);


  /** \brief Creates propagator for the cumulative constraint.
   *
   * This function will create a propagator for the cumulative constraint, by
   * translating it into a cumulatives constraint.
   *
   * \param start \f$ start_i \f$ is the start date assigned to task \f$ i \f$
   * \param duration \f$ duration_i \f$ is the duration of task \f$ i \f$
   * \param height \f$ height_i \f$ is the height is the amount of resources
   *         consumed by task \f$ i \f$
   * \param limit \c limit_r  is the amount of resource available
   * \param at_most tells if the amount of resources used for a machine
   *         should be less than the limit (\c at_most = true, default) or
   *         greater than the limit (\c at_most = false)
   * \param icl Supports value-consistency only (\c cl = ICL_VAL, default).
   *
   * \exception Int::ArgumentSizeMismatch Raised if the sizes of the arguments
   *            representing tasks does not match.
   * \exception Int::NumericalOverflow Raised if any numerical argument is
   *            larger than Limits::Int::int_max or less than
   *            Limits::Int::int_min.
   *
   */
  GECODE_MINIMODEL_EXPORT void
  cumulative(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             const IntVarArgs& height, int limit, bool at_most = true,
             IntConLevel cl=ICL_DEF);

  /** \brief Creates propagator for the cumulative constraint.
   *
   * \copydoc cumulative()
   */
  GECODE_MINIMODEL_EXPORT void
  cumulative(Space *home, const IntVarArgs& start, const IntArgs& duration,
             const IntVarArgs& height, int limit, bool at_most = true,
             IntConLevel cl=ICL_DEF);

  /** \brief Creates propagator for the cumulative constraint.
   *
   * \copydoc cumulative()
   */
  GECODE_MINIMODEL_EXPORT void
  cumulative(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             const IntArgs& height, int limit, bool at_most = true,
             IntConLevel cl=ICL_DEF);

  /** \brief Creates propagator for the cumulative constraint.
   *
   * \copydoc cumulative()
   */
  GECODE_MINIMODEL_EXPORT void
  cumulative(Space *home, const IntVarArgs& start, const IntArgs& duration,
             const IntArgs& height, int limit, bool at_most = true,
             IntConLevel cl=ICL_DEF);

  /** \brief Creates propagator for the serialized constraint.
   *
   * This function will create a propagator for the serialized constraint, by
   * translating it into a cumulative constraint.
   *
   * \param start \f$ start_i \f$ is the start date assigned to task \f$ i \f$
   * \param duration \f$ duration_i \f$ is the duration of task \f$ i \f$
   * \param icl Supports value-consistency only (\c cl = ICL_VAL, default).
   *
   * \exception Int::ArgumentSizeMismatch Raised if the sizes of the arguments
   *            representing tasks does not match.
   * \exception Int::NumericalOverflow Raised if any numerical argument is
   *            larger than Limits::Int::int_max or less than
   *            Limits::Int::int_min.
   *
   */
  GECODE_MINIMODEL_EXPORT void
  serialized(Space *home, const IntVarArgs& start, const IntVarArgs& duration,
             IntConLevel cl=ICL_DEF);


  /** \brief Creates propagator for the serialized constraint.
   *
   * \copydoc serialized()
   */
  GECODE_MINIMODEL_EXPORT void
  serialized(Space *home, const IntVarArgs& start, const IntArgs& duration,
             IntConLevel cl=ICL_DEF);


  //@}

}

/**
 * \addtogroup TaskMiniModelMatrix Matrix interface for arrays
 *
 * Wrapper for arrays to act as a two-dimensional matrix.
 *
 * \ingroup TaskMiniModel
 */
//@{
namespace Gecode { namespace MiniModel {

  /** \brief Matrix-interface for arrays
   *
   * This class allows for wrapping some array and accessing it as a
   * matrix.
   *
   * \note This is a light-weight wrapper, and is not intended for
   * storing variables directly instead of in an array.
   *
   * \ingroup TaskMiniModelMatrix
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
}}

#include "gecode/minimodel/matrix.icc"
//@}

#endif

// STATISTICS: minimodel-any

