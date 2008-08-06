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

#include "gecode/dds.hh"

#include <queue>
#include <vector>

namespace Gecode { namespace Decomposition {

  void
  Partition::init(int noOfElements, int noOfComponents) {
    elements = 
      static_cast<int*>(heap.ralloc(sizeof(int)*noOfElements));
    separators =
      static_cast<int*>(heap.ralloc(sizeof(int)*noOfComponents));
    _size = noOfComponents;
  }

  Partition::~Partition(void) {
    heap.rfree(elements);
    heap.rfree(separators);
  }

  size_t
  SingletonDescBase::size(void) const {
    return sizeof(SingletonDescBase);
  }

  size_t
  DecompDesc::size(void) const {
    return sizeof(DecompDesc);
  }

  std::vector<int>
  DecompDesc::significantVars(int alt) const {
    assert(alt >= 0 && (unsigned int)alt < alternatives());
    std::vector<int> sv(component[alt+1]-component[alt]);
    for (int i=component[alt+1]-component[alt]; i--;)
      sv[i] = element[(i+component[alt])*2+1];
    return sv;
  }

  DecompDesc::~DecompDesc(void) {
    heap.rfree(component);
    heap.rfree(element);
  }

  class Node {
  public:
    int component;
    std::vector<int> out;
    Node(void) : component(-1) {}
  };
  
  void
  findVars(Node* g, int p, Reflection::VarMap& vars, Reflection::Arg* s) {
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
                           Partition& p) {
    int noOfVars = vars.size();
    std::vector<Reflection::ActorSpec> as;
    for (Reflection::ActorSpecIter si(home,vars); si(); ++si) {      
      Reflection::ActorSpec asi = si.actor();
      if (!asi.isBranching())
        as.push_back(asi);
    }
    std::vector<int> label;
    int graphSize = vars.size()+as.size();
    Scratch s(home);
    Node* g = s.alloc<Node>(graphSize);
    for (int i=noOfVars; i--;) {
      if (vars.spec(i).assigned()) {
        g[i].component = 0;
        label.push_back(i);
      }
    }
    for (unsigned int i=0; i<as.size(); i++) {
      Reflection::ActorSpec asi = as[i];
      for (int j=asi.noOfArgs(); j--;)
        findVars(g,noOfVars+i,vars,asi[j]);
    }

    std::vector<int> separators;
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
    
    p.init(label.size(), separators.size());
    for (int i=label.size(); i--;)
      p[i] = label[i];
    for (int i=separators.size(); i--;)
      p.component(i) = separators[i];
  }

}}

// STATISTICS: dds-any
