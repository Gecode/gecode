/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#ifndef __GECODE_GENERATOR_PROJECOTRS_COMPILER_HH
#define __GECODE_GENERATOR_PROJECTORS_COMPILER_HH

#include <gecode/set/projectors.hh>

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
      void operator ++(void);
      /// Decrement indentation level
      void operator --(void);
    };
  private:
    /// Indentation to use
    Indent indent;
    /// The outstream for the header file (.hh)
    std::ostream& hhos;
    /// The outstream for the implementation header file (.hpp)
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
