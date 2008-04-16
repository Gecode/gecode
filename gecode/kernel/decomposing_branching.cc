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

#include "gecode/kernel.hh"

#include <queue>

namespace Gecode { namespace Decomposition {

  size_t
  SingletonDescBase::size(void) const {
    return sizeof(SingletonDescBase);
  }

  unsigned int
  SingletonDescBase::domainSize(void) const { return _size; }

  unsigned int
  SingletonDescBase::idx(void) const { return _idx; }

  SingletonDescBase::SingletonDescBase(const Branching* b, 
                                       unsigned int alt,
                                       unsigned int idx,
                                       unsigned int size)
   : BranchingDesc(b, alt), _idx(idx), _size(size) {}

  size_t
  DecompDesc::size(void) const {
    return sizeof(DecompDesc);
  }

  class Node {
  public:
    int component;
    std::vector<int> out;
    Node(void) : component(-1) {}
  };

  typedef std::vector<Node> graph;
  
  void
  findVars(graph& g, int p, Reflection::VarMap& vars, Reflection::Arg* s) {
    if (s->isVar()) {
      if (!vars.spec(s->toVar()).assigned()) {
        g[s->toVar()].out.push_back(p);
        g[p].out.push_back(s->toVar());
      }
    } else if (s->isArray()) {
      Reflection::ArrayArg* a = s->toArray();
      for (int j=a->size(); j--;)
        findVars(g, p, vars, (*a)[j]);
    } else if (s->isPair()) {
      findVars(g, p, vars, s->first());
      findVars(g, p, vars, s->second());
    }    
  }

  void connectedComponents(const Space* home, Reflection::VarMap& vars,
                           std::vector<int>& label,
                           std::vector<int>& separators) {
    int noOfVars = vars.size();
    std::vector<Reflection::ActorSpec> as;
    for (Reflection::ActorSpecIter si(home,vars); si(); ++si) {      
      Reflection::ActorSpec asi = si.actor();
      if (!asi.isBranching())
        as.push_back(asi);
    }
    graph g(vars.size());
    for (int i=noOfVars; i--;) {
      if (vars.spec(i).assigned()) {
        g[i].component = 0;
        label.push_back(i);
      }
    }
    for (unsigned int i=0; i<as.size(); i++) {
      Reflection::ActorSpec asi = as[i];
      Node newNode;
      g.push_back(newNode);
      for (int j=asi.noOfArgs(); j--;)
        findVars(g,noOfVars+i,vars,asi[j]);
    }

    label = std::vector<int>(0);
    separators = std::vector<int>(0);
    separators.push_back(0);

    int curComponent = 0;
    for (int i=0; i<vars.size(); i++) {
      if (g[i].component == -1) {
        std::queue<int> q;
        q.push(i);
        while (!q.empty()) {
          int cur = q.front();
          q.pop();
          g[cur].component = curComponent;
          if (cur<noOfVars)
            label.push_back(cur);
          for (int j=g[cur].out.size(); j--;) {
            assert(g[g[cur].out[j]].component == -1 ||
                   g[g[cur].out[j]].component == curComponent);
            if (g[g[cur].out[j]].component == -1)
              q.push(g[cur].out[j]);
          }
        }
        curComponent++;
        if (separators[separators.size()-1] != static_cast<int>(label.size()))
          separators.push_back(label.size());
      }
    }
    if (separators.size() == 1)
      separators.push_back(label.size());
  }

  void
  DecompDesc::significantVars(int alt, std::vector<int>& sv) const {
    assert(alt >= 0 && (unsigned int)alt < alternatives());
    sv.resize(select[alt+1]-select[alt]);
    for (int i=select[alt+1]-select[alt]; i--;)
      sv[i] = label[i+select[alt]].second;
  }

}}

// STATISTICS: dds-any
