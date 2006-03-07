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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_MINIMODEL_HH__
#define __GECODE_MINIMODEL_HH__

#include "./kernel.hh"
#include "./int.hh"
#include "./int/linear.hh"

#include "./minimodel/exception.icc"

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

#define GECODE_MINIMODEL_EXPORT

#endif

namespace Gecode {

  /// Minimal modelling support
  namespace MiniModel {

    /// Linear expressions
    class LinExpr {
    private:
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
	IntVar x;
      public:
	/// Construct node for \a a multiplied with \a x
	Node(int a, const IntVar& x);
	/// Construct node from nodes \a n0 and \a n1 with signs \a s0 and \a s1
	Node(Node* n0, int s0, Node* n1, int s1);

	/// Increment reference count
	void increment(void);
	/// Decrement reference count and possibly free memory
	GECODE_MINIMODEL_EXPORT bool decrement(void);
	
	/// Fill in array of linear terms based on this node
	GECODE_MINIMODEL_EXPORT int
	fill(Int::Linear::Term t[], int i, int s) const;
	
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
      LinExpr(const IntVar& x, int a=1, int c=0);
      /// Create expression \f$e_0+s\cdot e_1\f$ (where \a s is the sign)
      LinExpr(const LinExpr& e0, const LinExpr& e1, int s);
      /// Create expression \f$s(e+c)\f$ (where \a s is the sign)
      LinExpr(const LinExpr& e, int c, int s);
      /// Create expression \f$a\cdot e\f$
      LinExpr(int a, const LinExpr& e);
      /// Assignment operator
      const LinExpr& operator=(const LinExpr& e);
      /// Post propagator
      GECODE_MINIMODEL_EXPORT
      void post(Space* home, IntRelType irt, IntConLevel icl) const;
      /// Post reified propagator
      GECODE_MINIMODEL_EXPORT
      void post(Space* home, IntRelType irt, const BoolVar& b) const;
      /// Post propagator and return variable for value
      GECODE_MINIMODEL_EXPORT
      IntVar post(Space* home, IntConLevel icl) const;
      /// Destructor
      ~LinExpr(void);
    };

    /// Linear relations
    class LinRel {
    private:
      /// Linear expression describing the entire relation
      LinExpr    e;
      /// Which relation
      IntRelType irt;
      /// Negate relation type
      static IntRelType neg(IntRelType irt);
    public:
      /// Default constructor
      LinRel(void);
      /// Create linear relation for expressions \a l and \a r
      LinRel(const LinExpr& l, IntRelType irt, const LinExpr& r);
      /// Create linear relation for expression \a l and integer \a r
      LinRel(const LinExpr& l, IntRelType irt, int r);
      /// Create linear relation for integer \a l and expression \a r
      LinRel(int l, IntRelType irt, const LinExpr& r);
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
Gecode::MiniModel::LinExpr
operator+(int,
	  const Gecode::MiniModel::LinExpr&);
/// Construct linear expression as sum of integer and linear expression
Gecode::MiniModel::LinExpr
operator+(const Gecode::MiniModel::LinExpr&,
	  int);
/// Construct linear expression as sum of linear expressions
Gecode::MiniModel::LinExpr
operator+(const Gecode::MiniModel::LinExpr&,
	  const Gecode::MiniModel::LinExpr&);

/// Construct linear expression as difference of linear expression and integer
Gecode::MiniModel::LinExpr
operator-(int,
	  const Gecode::MiniModel::LinExpr&);
/// Construct linear expression as difference of integer and linear expression
Gecode::MiniModel::LinExpr
operator-(const Gecode::MiniModel::LinExpr&,
	  int);
/// Construct linear expression as difference of linear expressions
Gecode::MiniModel::LinExpr
operator-(const Gecode::MiniModel::LinExpr&,
	  const Gecode::MiniModel::LinExpr&);

/// Construct linear expression as negative of linear expression
Gecode::MiniModel::LinExpr
operator-(const Gecode::MiniModel::LinExpr&);


/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr
operator*(int, const Gecode::IntVar&);
/// Construct linear expression as product of integer coefficient and integer variable
Gecode::MiniModel::LinExpr
operator*(const Gecode::IntVar&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr
operator*(const Gecode::MiniModel::LinExpr&, int);
/// Construct linear expression as product of integer coefficient and linear expression
Gecode::MiniModel::LinExpr
operator*(int, const Gecode::MiniModel::LinExpr&);


/// Construct linear equality relation
Gecode::MiniModel::LinRel
operator==(int l,
	   const Gecode::MiniModel::LinExpr& r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel
operator==(const Gecode::MiniModel::LinExpr& l,
	   int r);
/// Construct linear equality relation
Gecode::MiniModel::LinRel
operator==(const Gecode::MiniModel::LinExpr& l,
	   const Gecode::MiniModel::LinExpr& r);

/// Construct linear disequality relation
Gecode::MiniModel::LinRel
operator!=(int l,
	   const Gecode::MiniModel::LinExpr& r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel
operator!=(const Gecode::MiniModel::LinExpr& l,
	   int r);
/// Construct linear disequality relation
Gecode::MiniModel::LinRel
operator!=(const Gecode::MiniModel::LinExpr& l,
	   const Gecode::MiniModel::LinExpr& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<(int l,
	  const Gecode::MiniModel::LinExpr& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<(const Gecode::MiniModel::LinExpr& l,
	  int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<(const Gecode::MiniModel::LinExpr& l,
	  const Gecode::MiniModel::LinExpr& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<=(int l,
	   const Gecode::MiniModel::LinExpr& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<=(const Gecode::MiniModel::LinExpr& l,
	   int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator<=(const Gecode::MiniModel::LinExpr& l,
	   const Gecode::MiniModel::LinExpr& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>(int l,
	  const Gecode::MiniModel::LinExpr& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>(const Gecode::MiniModel::LinExpr& l,
	  int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>(const Gecode::MiniModel::LinExpr& l,
	  const Gecode::MiniModel::LinExpr& r);

/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>=(int l,
	   const Gecode::MiniModel::LinExpr& r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>=(const Gecode::MiniModel::LinExpr& l,
	   int r);
/// Construct linear inequality relation
Gecode::MiniModel::LinRel
operator>=(const Gecode::MiniModel::LinExpr& l,
	   const Gecode::MiniModel::LinExpr& r);

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
	/// Possibly a reified linear relation
	LinRel rl;

	/// Default constructor
	Node(void);
	/// Decrement reference count and possibly free memory
	GECODE_MINIMODEL_EXPORT bool decrement(void);
	/// Post propagators for nested conjunctive and disjunctive expression
	GECODE_MINIMODEL_EXPORT int post(Space* home, NodeType t, 
					 BoolVarArgs& b, int i) const;
	/// Post propagators for expression
	GECODE_MINIMODEL_EXPORT void post(Space* home, BoolVar b) const;
	/// Post propagators for expression
	GECODE_MINIMODEL_EXPORT BoolVar post(Space* home) const;
	/// Post propagators for relation
	GECODE_MINIMODEL_EXPORT void post(Space* home, bool t) const;
	
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
      BoolExpr(const LinRel& rl);
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
operator~(const Gecode::MiniModel::LinExpr&);

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
  IntVar post(Space* home, const MiniModel::LinExpr& e, 
	      IntConLevel icl=ICL_DEF);
  /// Post linear expression (special case for variable) and return its value
  IntVar post(Space* home, const IntVar& x,
	      IntConLevel icl=ICL_DEF);
  /// Post linear expression (special case for constant) and return its value
  IntVar post(Space* home, int n,
	      IntConLevel icl=ICL_DEF);

  /// Post linear relation
  void post(Space* home, const MiniModel::LinRel& r,
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

#include "minimodel/lin-expr.icc"
#include "minimodel/lin-rel.icc"
#include "minimodel/bool-expr.icc"
#include "minimodel/bool-rel.icc"

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
    unsigned int const width(void);
    /// Return the height of the matrix
    unsigned int const height(void);
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
    args_type row(int r);
    
    /// Access column \a c.
    args_type col(int c);
  };
}}

#include "minimodel/matrix.icc"
//@}

#endif

// STATISTICS: minimodel-any

