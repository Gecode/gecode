/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *     Christian Schulte, 2007
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

#ifndef __GECODE_GENERATOR_PROJECOTRS_HH
#define __GECODE_GENERATOR_PROJECTORS_HH

#include "gecode/set.hh"

namespace Gecode {

  /**
   * \defgroup TaskModelSetProjector Projector constraints
   * \ingroup TaskModelSet
   *
   */

  //@{

  /**
   * \brief Code representing set-valued expressions for finite set projectors
   *
   * Set-valued expressions are represented using a simple stack-based
   * language.
   */
  class SetExprCode {
  private:
    /// The actual instructions
    SharedArray<int> c;
  public:
    /// Instructions for set-valued expression code
    enum Instruction {
      COMPLEMENT, ///< Complement
      INTER,      ///< Intersection
      UNION,      ///< Union
      GLB,        ///< Greatest lower bound
      LUB,        ///< Least upper bound
      EMPTY,      ///< Constant empty set
      UNIVERSE,   ///< Constant universal set
      LAST        ///< First integer to use for immediate arguments
    };

    /// To incrementally build up the instructions
    class Stream {
    private:
      /// Instruction stream
      Support::DynamicArray<int> is;
      /// Current instruction
      int n;
    public:
      /// Initialize stream to be empty
      Stream(void);
      /// Add instruction or variable reference to stream
      void add(int i);
      /// Return size of code stream
      int size(void) const;
      /// Return instruction at position \a i
      int operator[](int i) const;
    };
 
    ///\name Construction and initialization
    //@{
    /// Default constructor
    SetExprCode(void);
    /// Construct from code Stream
    SetExprCode(const Stream& s);
    /// Copy constructor
    SetExprCode(const SetExprCode& sc);
    //@}

    /// Copying
    void update(Space& home, bool share, SetExprCode& sc);
    
    /// Return number of instructions
    int size(void) const;

    /// Return instruction at position \a i
    int operator[](int i) const;

  };
  
  /**
   * \brief Set-valued expressions for finite set projectors
   */
  class SetExpr {
  public:
    /// Type of variable indices
    typedef int var_idx;

    /// Relation used to connect two set expressions
    enum RelType {
      REL_INTER, ///< Intersection relation
      REL_UNION  ///< Union relation
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
    /// Returns the arity of the set expression
    GECODE_SET_EXPORT int arity(void) const;
    /// Returns code for this set expression
    GECODE_SET_EXPORT SetExprCode encode(void) const;
    /// Destructor
    GECODE_SET_EXPORT ~SetExpr(void);
  };

}

///\name Specifying finite set projectors
//@{

/// Return set expression for the complement of \a s
Gecode::SetExpr operator-(const Gecode::SetExpr& s);
/// Return set expression for the union of \a s and \a t
Gecode::SetExpr operator||(const Gecode::SetExpr& s,
                           const Gecode::SetExpr& t);
/// Return set expression for the intersection of \a s and \a t
Gecode::SetExpr operator&&(const Gecode::SetExpr& s,
                           const Gecode::SetExpr& t);
/// Return set expression for the difference of \a s and \a t
Gecode::SetExpr operator-(const Gecode::SetExpr& s,
                          const Gecode::SetExpr& t);

///@}

namespace Gecode {

  /**
   * \brief Finite set projector specification
   */
  class Projector {
  private:
    SetExpr::var_idx i; ///< The variable for this projector
    SetExprCode glb; ///< The greatest lower bound set expression code
    SetExprCode lub; ///< The least upper bound set expression code
    int _arity; ///< The arity of this projector
  public:
    /// Default constructor
    Projector(void);
    /// Construct a projector \f$(\mathrm{sglb}\subseteq x\subseteq\mathrm{slub})\f$
    Projector(SetExpr::var_idx x, const SetExpr& sglb, const SetExpr& slub);

    /// Returns the scope of the projector
    GECODE_SET_EXPORT void scope(Support::DynamicArray<int>& scope) const;

    /// Returns the arity of the projector
    GECODE_SET_EXPORT int arity(void) const;
              
    /// Propagate the projector
    template <bool negated>
    ExecStatus propagate(Space& home, ViewArray<Set::SetView>& x);

    /// Check for subsumption or failure
    GECODE_SET_EXPORT ExecStatus check(Space& home,
                                       ViewArray<Set::SetView>& x);

    /// Compute size of greatest lower bound
    unsigned int glbSize(ViewArray<Set::SetView>& x);
    
    /// Compute size of least upper bound
    unsigned int lubSize(ViewArray<Set::SetView>& x);

    /// Return the set expression code for the glb
    const SetExprCode& getGlb(void) const;

    /// Return the set expression code for the lub
    const SetExprCode& getLub(void) const;

    /// Return the variable index for this projector
    SetExpr::var_idx getIdx(void) const;

  };
  
  /**
   * \brief Group of finite set projector specifications
   */
  class ProjectorSet {
  private:
    SharedArray<Projector> _ps; ///< The projectors
    int _count; ///< The number of projectors in the set
    int _arity; ///< The maximum arity of the projectors in the set
    
  public:
    
    /// Construct empty projector set
    ProjectorSet(void);
    
    /// Construct projector set for \a n projectors
    ProjectorSet(int n);

    /// Used for copying
    void update(Space& home, bool share, ProjectorSet& p);

    /// Add projector \a p to the set
    GECODE_SET_EXPORT void add(const Projector& p);

    /// Returns the arity of the projector set
    int arity(void) const;

    /// Returns the scope of the projector set
    GECODE_SET_EXPORT void scope(Support::DynamicArray<int>& scope,
                                 unsigned int size) const;
    
    /// Propagate the set
    template <bool negated>
    ExecStatus propagate(Space& home, ViewArray<Set::SetView>& x);

    /// Check for subsumption or failure
    GECODE_SET_EXPORT ExecStatus check(Space& home,
                                       ViewArray<Set::SetView>& x);

    /// Return number of projectors int the set
    int size(void) const;

    /// Return projector \a i from the set
    const Projector& operator[](int i) const;

  };

  ///\name Formulas for specifying set constraints
  //@{

  /// Formulas for specifying set constraints
  class Formula {
  public:
    enum Operator { AND, OR, IMPL, EQUIV };
  private:
    /// Nodes for formulas
    class Node;
    Node* ax; ///< Node for formula
    int sign; ///< Sign for formula
  public:
    /// Copy constructor
    GECODE_SET_EXPORT Formula(const Formula& f);
    /// Assignment operator
    GECODE_SET_EXPORT const Formula& operator=(const Formula& f);
    /// Destructor
    GECODE_SET_EXPORT ~Formula(void);
    /// Construct formula for variable \a var
    GECODE_SET_EXPORT Formula(int var);
    /// Construct constant formula
    GECODE_SET_EXPORT Formula(bool b);
    /// Construct formula from \a f with sign \a s
    GECODE_SET_EXPORT Formula(const Formula& f, int sign);
    /// Construct formula for \f$\mathit{fs}\times f\mathit{op} \mathit{gs}\times g\f$
    GECODE_SET_EXPORT Formula(const Formula& f, int fs,
                              Operator r,
                              const Formula& g, int gs);
    /// Extract projectors from formula
    GECODE_SET_EXPORT ProjectorSet projectors(void);
    
  };

}

///\name Operations on formulas for specifying set constraints
//@{

/// Return formula \f$f\land g\f$ \relates Formula
Gecode::Formula operator&(const Gecode::Formula& f, const Gecode::Formula& g);
/// Return formula \f$f\lor g\f$ \relates Formula
Gecode::Formula operator|(const Gecode::Formula& f, const Gecode::Formula& g);
/// Return formula \f$\lnot f\f$ \relates Formula
Gecode::Formula operator-(const Gecode::Formula& f);
/// Return formula \f$f\rightarrow f\f$ \relates Formula
Gecode::Formula operator>>(const Gecode::Formula& f,
                           const Gecode::Formula& g);
/// Return formula \f$f\leftrightarrow g\f$ \relates Formula
Gecode::Formula operator==(const Gecode::Formula& f,
                           const Gecode::Formula& g);

//@}

namespace Gecode {
  
  ///\name Posting projection propagators
  //@{

  /**
   * \brief Post projection propagators for nary constraint
   *
   * Post projection propagators for nary constraint on variables
   * \a x, specified by \a ps. If \a negated is true, the
   * negation of the specified constraint is propagated.
   */
  GECODE_SET_EXPORT void
  projector(Space& home, const SetVarArgs& xa,
            ProjectorSet& ps, bool negated=false);

  /**
   * \brief Post projection propagators for binary constraint
   *
   * Post projection propagators for binary constraint on variables
   * \a xa and \a ya, specified by \a ps. If \a negated is true, the
   * negation of the specified constraint is propagated.
   */
  GECODE_SET_EXPORT void
  projector(Space& home, const SetVar& xa, const SetVar& ya,
            ProjectorSet& ps, bool negated=false);

  /**
   * \brief Post reified projection propagators for binary constraint
   *
   * Post projection propagators for binary constraint on variables
   * \a xa and \a ya, specified by \a ps, reified with \a bv.
   */
  GECODE_SET_EXPORT void
  projector(Space& home, const SetVar& xa, const SetVar& ya,
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
  projector(Space& home,
            const SetVar& xa, const SetVar& ya, const SetVar& za,
            ProjectorSet& ps, bool negated=false);

  /**
   * \brief Post reified projection propagators for ternary constraint
   *
   * Post projection propagators for ternary constraint on variables
   * \a xa, \a ya and \a za, specified by \a ps, reified with \a bv.
   */
  GECODE_SET_EXPORT void
  projector(Space& home,
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
  projector(Space& home, const SetVar& xa, const SetVar& ya,
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
  projector(Space& home, const SetVar& xa, const SetVar& ya,
            const SetVar& za, const IntVar& i,
            Projector& p);

  //@}

  //@}

}

GECODE_SET_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::SetExprCode& sec);

GECODE_SET_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::Projector& p);

GECODE_SET_EXPORT std::ostream&
operator<<(std::ostream&, const Gecode::ProjectorSet& ps);

#include "gecode/set/projectors/set-expr.icc"
#include "gecode/set/projectors/projector.icc"
#include "gecode/set/projectors/projector-set.icc"
#include "gecode/set/projectors/formula.icc"

#endif

// STATISTICS: set-prop
