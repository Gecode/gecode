/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Martin Mann <mmann@informatik.uni-freiburg.de>
 *     Sebastian Will <will@informatik.uni-freiburg.de>
 *
 *  Copyright:
 *     Guido Tack, 2008
 *     Martin Mann, 2008
 *     Sebastian Will, 2008
 *
 *  Last modified:
 *     $Date: 2007-04-05 15:25:13 +0200 (Thu, 05 Apr 2007) $ by $Author: tack $
 *     $Revision: 4322 $
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

#include "gecode/dds.hh"
#include <boost/graph/graphviz.hpp>
#include <boost/graph/connected_components.hpp>

namespace Gecode { namespace Decomposition {

  void
  ConstraintGraph::printDot(std::ostream & out) {
    boost::write_graphviz(out, g);
  }

  int
  ConstraintGraph::getComponentLabel(ConstraintGraph::ComponentLabel &label) {
    using namespace boost;
    label = ConstraintGraph::ComponentLabel(num_vertices(g));

    int labelNumber = boost::connected_components(g, &label[0]);

    return labelNumber;
  }

}}

// STATISTICS: dds-any
