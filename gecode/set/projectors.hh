/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#ifndef __GECODE_GENERATOR_PROJECOTRS_HH
#define __GECODE_GENERATOR_PROJECTORS_HH

#include "gecode/set.hh"
#include <map>
#include <vector>

namespace Gecode {

  /**
   * \defgroup TaskIntSetProjector Projector constraints
   * \ingroup TaskIntSet
   *
   */

  //@{

  class SetExprCode {
  public:
    enum Instruction {
	COMPLEMENT, INTER, UNION, GLB, LUB, EMPTY, UNIVERSE, LAST
    };
    typedef std::vector<int> code;
  };
  
  /**
   * \brief Set-valued expressions for finite set projectors
   */
  class SetExpr {
  public:
    /// Type of variable indices
    typedef int var_idx;
    /// Scope of a projector
    typedef std::map<int, PropCond> proj_scope;

    /// Combine two projector scopes into one
    GECODE_SET_EXPORT
    static proj_scope combineScopes(const proj_scope& s1, 
				    const proj_scope& s2);

    /// Relation used to connect two set expressions
    enum RelType {
      REL_INTER,
      REL_UNION
    };
    
  private:
    /// Nodes for set expressions
    class Node;

    Node* ax; ///< Node for expression
    int sign; ///< Sign for expression
  public:

    /// Construct constant set expression for the empty set
    SetExpr(void);
    /// Copy constructor
    GECODE_SET_EXPORT SetExpr(const SetExpr& s);
    /// Construct set expression for variable \a v
    GECODE_SET_EXPORT SetExpr(var_idx v);
    /// Construct set expression from \a s with sign \a sign
    GECODE_SET_EXPORT SetExpr(const SetExpr& s, int sign);
    /// Construct set expression from \a s with sign \a ssign and \a t with sign \a tsign under relation \a r
    GECODE_SET_EXPORT SetExpr(const SetExpr& s, int ssign,
			      RelType r,
			      const SetExpr& t, int tsign);
    /// Assignment operator
    GECODE_SET_EXPORT const SetExpr& operator=(const SetExpr& e);
    /// Returns the scope of the set expression
    GECODE_SET_EXPORT proj_scope scope(int sign) const;
    /// Returns the arity of the set expression
    GECODE_SET_EXPORT int arity(void) const;
    /// Returns code for this set expression
    GECODE_SET_EXPORT SetExprCode::code encode(void) const;
    /// Destructor
    GECODE_SET_EXPORT ~SetExpr(void);
  };

  ///\name Specifying finite set projectors
  //@{

  /// Return set expression for the complement of \a s
  SetExpr operator-(const SetExpr& s);
  /// Return set expression for the union of \a s and \a t
  SetExpr operator||(const SetExpr& s, const SetExpr& t);
  /// Return set expression for the intersection of \a s and \a t
  SetExpr operator&&(const SetExpr&, const SetExpr&);
  /// Return set expression for the difference of \a s and \a t
  SetExpr operator-(const SetExpr&, const SetExpr&);

  ///@}

  /**
   * \brief Finite set projector specification
   */
  class Projector {
  private:
    SetExpr::var_idx i; ///< The variable for this projector
    SetExprCode::code glb; ///< The greatest lower bound set expression code
    SetExprCode::code lub; ///< The least upper bound set expression code
    SetExpr::proj_scope _scope; ///< The scope of this projector
    int _arity; ///< The arity of this projector
  public:
    /// Default constructor
    Projector(void);
    /// Construct a projector \f$(\mathrm{sglb}\subseteq x\subseteq\mathrm{slub})\f$
    Projector(SetExpr::var_idx x, const SetExpr& sglb, const SetExpr& slub);

    /// Returns the scope of the projector
    GECODE_SET_EXPORT SetExpr::proj_scope scope(void) const;

    /// Returns the arity of the projector
    GECODE_SET_EXPORT int arity(void) const;
	      
    /// Propagate the projector
    template <bool negated>
    ExecStatus propagate(Space* home, ViewArray<Set::SetView>& x);

    /// Check for subsumption or failure
    GECODE_SET_EXPORT ExecStatus check(Space* home,
				       ViewArray<Set::SetView>& x);

    /// Compute size of greatest lower bound
    unsigned int glbSize(ViewArray<Set::SetView>& x);
    
    /// Compute size of least upper bound
    unsigned int lubSize(ViewArray<Set::SetView>& x);

  };
  
  /**
   * \brief Group of finite set projector specifications
   */
  class ProjectorSet {
  private:
    Support::SharedArray<Projector,true> _ps; ///< The projectors
    int _count; ///< The number of projectors in the set
    int _arity; ///< The maximum arity of the projectors in the set
    void makeLast(int i); ///< Swap projector \a i with the last projector
  public:
    
    /// Construct empty projector set
    ProjectorSet(void);

    /// Used for copying
    void update(Space* home, bool share, ProjectorSet& p);

    /// Add projector \a p to the set
    GECODE_SET_EXPORT void add(const Projector& p);

    /// Returns the arity of the projector set
    int arity(void) const;

    /// Returns the scope of the projector set
    GECODE_SET_EXPORT SetExpr::proj_scope scope(void) const;
    
    /// Propagate the set
    template <bool negated>
    ExecStatus propagate(Space* home, ViewArray<Set::SetView>& x);

    /// Check for subsumption or failure
    GECODE_SET_EXPORT ExecStatus check(Space* home,
				       ViewArray<Set::SetView>& x);

  };

  ///\name Posting projection propagators
  //@{

  /**
   * \brief Post projection propagators for binary constraint
   *
   * Post projection propagators for binary constraint on variables
   * \a xa and \a ya, specified by \a ps. If \a negated is true, the
   * negation of the specified constraint is propagated.
   */
  GECODE_SET_EXPORT void
  projector(Space* home, const SetVar& xa, const SetVar& ya,
	    ProjectorSet& ps, bool negated=false);

  /**
   * \brief Post reified projection propagators for binary constraint
   *
   * Post projection propagators for binary constraint on variables
   * \a xa and \a ya, specified by \a ps, reified with \a bv.
   */
  GECODE_SET_EXPORT void
  projector(Space* home, const SetVar& xa, const SetVar& ya,
	    const BoolVar& bv,
	    ProjectorSet& ps);

  /**
   * \brief Post projection propagators for ternary constraint
   *
   * Post projection propagators for ternary constraint on variables
   * \a xa, \a ya and \a za, specified by \a ps. If \a negated is true, the
   * negation of the specified constraint is propagated.
   */
  GECODE_SET_EXPORT void
  projector(Space* home,
	    const SetVar& xa, const SetVar& ya, const SetVar& za,
	    ProjectorSet& ps, bool negated=false);

  /**
   * \brief Post reified projection propagators for ternary constraint
   *
   * Post projection propagators for ternary constraint on variables
   * \a xa, \a ya and \a za, specified by \a ps, reified with \a bv.
   */
  GECODE_SET_EXPORT void
  projector(Space* home,
	    const SetVar& xa, const SetVar& ya, const SetVar& za,
	    const BoolVar& bv,
	    ProjectorSet& ps);
  
  /**
   * \brief Post projection propagator for cardinality constraint
   *
   * Post propagator for the constraint that \a i is the
   * cardinality of the set represented by \a p, interpreted over the
   * variables \a xa and \a ya.
   */
  GECODE_SET_EXPORT void
  projector(Space* home, const SetVar& xa, const SetVar& ya,
	    const IntVar& i,
	    Projector& p);

  /**
   * \brief Post projection propagator for cardinality constraint
   *
   * Post propagator for the constraint that \a i is the
   * cardinality of the set represented by \a p, interpreted over the
   * variables \a xa, \a ya, and \a za.
   */
  GECODE_SET_EXPORT void
  projector(Space* home, const SetVar& xa, const SetVar& ya,
	    const SetVar& za, const IntVar& i,
	    Projector& p);

  //@}

  //@}

}

#include "gecode/set/projectors/set-expr.icc"
#include "gecode/set/projectors/projector.icc"
#include "gecode/set/projectors/projector-set.icc"
  
#endif

// STATISTICS: set-prop
