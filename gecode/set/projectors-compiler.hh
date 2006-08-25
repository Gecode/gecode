/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-04-11 15:58:37 +0200 (Tue, 11 Apr 2006) $ by $Author: tack $
 *     $Revision: 3188 $
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

#ifndef __GECODE_GENERATOR_PROJECOTRS_COMPILER_HH
#define __GECODE_GENERATOR_PROJECTORS_COMPILER_HH

#include "gecode/set/projectors.hh"

namespace Gecode {

  /**
   * \brief Specification for a C++ finite set propagator
   *
   */
  class ProjectorPropagatorSpec {
  public:
    /// The name of the propagator
    std::string _name;
    /// The namespace to place the propagator in
    std::string _namespace;

    /// Arity of the generated propagator, 0 meaning n-ary
    int _arity;
    /// Whether this propagator is reified
    bool _reified;
    /// Whether this propagator is negated
    bool _negated;

    ProjectorSet _ps;
  };

  /**
   * \brief Compiler from projector specifications to C++
   *
   */
  class ProjectorCompiler {
  public:
    /// Indentation helper class
    class Indent {
    public:
      /// current indentation
      int i;
      /// Default constructor
      Indent(void);
      /// Increment indentation level
      void operator++(void);
      /// Decrement indentation level
      void operator--(void);
    };
  private:
    /// Indentation to use
    Indent indent;
    /// The outstream for the header file (.hh)
    std::ostream& hhos;
    /// The outstream for the implementation header file (.icc)
    std::ostream& iccos;

    /// The projector specification that gets compiled
    const ProjectorPropagatorSpec& spec;    

    ///\name Internal helper functions
    //@{
    /// Test if this is an nary propagator
    bool nary(void);
    /// Return string representation of propagation cost
    std::string propcost(void);
    /// Emit test that checks if all variables are assigned
    void allAssigned(std::ostream&);
    /// Emit argument list for the views
    void viewsarglist(std::ostream&);
    /// Emit constructor initializer list for the views
    void initarglist(std::ostream&);
    /// Return string representation of propagation condition
    std::string propcond(PropCond);
    /// Emit template parameters for member functions
    void templateparams(void);
    /// Emit template head for member functions and class
    void templatehead(std::ostream&);
    /// Emit file header, including namespace
    void header(std::ostream&);
    /// Emit file footer
    void footer(std::ostream&);
    /// Emit class delcaration
    void classdef(void);
    /// Emit implementation
    void standardMemberFunctions(void);
    /// Emit iterator for set expression
    int iterator(const SetExprCode& instrs,
                 bool countSize=false,
                 bool invert=false);
    /// Emit propagation function
    void propagation(void);
    //@}

  public:
    /// Description of the fixpoint to compute
    enum FixpointType {
      NO_FIX,    ///< Do not compute fixpoint, return ES_NOFIX
      ITER_FIX,  ///< Compute fixpoint by iteration
    };
    /// Description of how generic the propagator should be
    enum ViewsType {
      SINGLE_VIEW, ///< Make propagator parametric in a single view
      MULTI_VIEW   ///< Make propagator parametric in one view per variable
    };

    ///\name Configuration options
    //@{
    /// Whether to generate a test case, default=false
    bool compiletest;
    /// What fixpoint to compute, default=ITER_FIX
    FixpointType fixpoint;
    /// How generic the propagator is, default=SINGLE_VIEW
    ViewsType views; 
    //@}

    /// Construct compiler
    GECODE_SET_EXPORT ProjectorCompiler(std::ostream& hhos,
					std::ostream& iccos,
					const ProjectorPropagatorSpec& spec);
    /// Do the compilation
    GECODE_SET_EXPORT void compile(void);
  };


}

#endif

// STATISTICS: set-prop
