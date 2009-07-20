/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_GRAPH_HH__
#define __GECODE_GRAPH_HH__

#include <gecode/int.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_GRAPH
#define GECODE_GRAPH_EXPORT __declspec( dllexport )
#else
#define GECODE_GRAPH_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_GRAPH_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_GRAPH_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_GRAPH
#define GECODE_LIBRARY_NAME "Graph"
#include <gecode/support/auto-link.hpp>
#endif

/**
 * \namespace Gecode::Graph
 * \brief Graph constraints and branchings 
 *
 */

namespace Gecode {

  /**
   * \defgroup TaskModelGraph Graph constraints
   * \ingroup TaskModel
   */
  //@{
  /** \brief Post propagator such that \a x forms a circuit
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes.
   *
   * Supports domain (\a icl = ICL_DOM) and value propagation (all
   * other values for \a icl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable 
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   */
  GECODE_GRAPH_EXPORT void
  circuit(Space& home, const IntVarArgs& x,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with costs \a y and \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes. The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is 
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of 
   * the entire circuit. The variables \a y define the cost
   * of the edge in \a x: that is, if \f$x_i=j\f$ then \f$y_i=c_{i*n+j}\f$.
   *
   * Supports domain (\a icl = ICL_DOM) and value propagation (all
   * other values for \a icl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable 
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \a x and \a y do not have the same
   *    size or if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_GRAPH_EXPORT void
  circuit(Space& home, 
          const IntArgs& c,
          const IntVarArgs& x, const IntVarArgs& y, IntVar z,
          IntConLevel icl=ICL_DEF);
  /** \brief Post propagator such that \a x forms a circuit with cost \a z
   *
   * \a x forms a circuit if the graph with edges \f$i\to j\f$ where
   * \f$x_i=j\f$ has a single cycle covering all nodes. The integer array
   * \a c gives the costs of all possible edges where \f$c_{i*|x|+j}\f$ is 
   * the cost of the edge \f$i\to j\f$. The variable \a z is the cost of 
   * the entire circuit.
   *
   * Supports domain (\a icl = ICL_DOM) and value propagation (all
   * other values for \a icl), where this refers to whether value or
   * domain consistent distinct in enforced on \a x for circuit.
   *
   * Throws the following exceptions:
   *  - Int::ArgumentSame, if \a x contains the same unassigned variable 
   *    multiply.
   *  - Int::TooFewArguments, if \a x has no elements.
   *  - Int::ArgumentSizeMismacth, if \f$|x|\times|x|\neq|c|\f$.
   */
  GECODE_GRAPH_EXPORT void
  circuit(Space& home, 
          const IntArgs& c,
          const IntVarArgs& x, IntVar z,
          IntConLevel icl=ICL_DEF);
  //@}

}

#endif

// STATISTICS: graph-post

