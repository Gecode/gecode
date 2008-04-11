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
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/connected_components.hpp>

namespace Gecode { namespace Decomposition {

  unsigned int
  SingletonDescBase::domainSize(void) const { return _size; }

  SingletonDescBase::SingletonDescBase(const Branching* b, 
                                       unsigned int alt,
                                       unsigned int size)
   : BranchingDesc(b, alt), _size(size) {}

  void
  findVars(boost::adjacency_list<boost::setS, boost::vecS, 
                                 boost::undirectedS>& g,
           int p,
           Reflection::VarMap& vars, Reflection::Arg* s) {
    if (s->isVar()) {
      if (!vars.spec(s->toVar()).assigned())
        boost::add_edge(s->toVar(), p, g);
    } else if (s->isArray()) {
      Reflection::ArrayArg* a = s->toArray();
      for (int j=a->size(); j--;)
        findVars(g, p, vars, (*a)[j]);
    } else if (s->isPair()) {
      findVars(g, p, vars, s->first());
      findVars(g, p, vars, s->second());
    }
  }

  int
  connectedComponents(const Space* home, Reflection::VarMap& vars,
                      std::vector<int>& label) {
    using namespace boost;
    adjacency_list<setS, vecS, undirectedS> g(vars.size());
    int propIndex = vars.size();
    for (Reflection::ActorSpecIter si(home,vars); si(); ++si) {      
      Reflection::ActorSpec as = si.actor();
      if (!as.isBranching()) {
        for (int i=as.noOfArgs(); i--;)
          findVars(g, propIndex, vars, as[i]);
        propIndex++;
      }
    }
    
    label = std::vector<int>(num_vertices(g));
    int labelNumber = connected_components(g, &label[0]);

    return labelNumber;
  }

  void
  DecompDesc::significantVars(int alt, std::vector<int>& sv) const {
    assert(alt >= 0 && (unsigned int)alt < alternatives());
      int count = 0;
      for (std::vector<int>::const_iterator it=label.begin(); 
           it!=label.end(); it++) {
        if (*it==alt) count++;
      }
      sv.resize(count);
      for (int i=label.size()-1; i>=0; i--)
        if (label[i]==alt) sv[--count]=i;
  }

}}

// STATISTICS: dds-any
