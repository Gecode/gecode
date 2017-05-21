/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#include <gecode/search.hh>

namespace Gecode {

  const char*
  StdSearchTracer::t2s[SearchTracer::EngineType::AOE + 1] = {
    "DFS", "BAB", "LDS",
    "RBS", "PBS",
    "AOE"
  };

  StdSearchTracer::StdSearchTracer(void) {}

  void
  StdSearchTracer::init(void) {
    std::cout << "trace<Search>::init()" << std::endl;
      for (unsigned int e=0U; e<engines(); e++) {
        std::cout << "\t" << e << ": " 
                  << t2s[engine(e).type()];
        switch (engine(e).type()) {
        case EngineType::DFS: 
        case EngineType::BAB:
        case EngineType::LDS:
        case EngineType::AOE:
          std::cout << ", workers: {";
          for (unsigned int i=engine(e).wfst(); i<engine(e).wlst(); i++) {
            std::cout << i; if (i+1 < engine(e).wlst()) std::cout << ",";
          }
          std::cout << "}" << std::endl;
          break;
        case EngineType::RBS:
        case EngineType::PBS:
          std::cout << ", engines: {";
          for (unsigned int i=engine(e).efst(); i<engine(e).elst(); i++) {
            std::cout << i; if (i+1 < engine(e).elst()) std::cout << ",";
          }
          std::cout << "}" << std::endl;
          break;
        default: GECODE_NEVER;
        }
      }
    }

  void
  StdSearchTracer::round(unsigned int eid) {
    std::cout << "trace<Search>::round(e:" << eid << ")" << std::endl;
  }

  void
  StdSearchTracer::skip(const EdgeInfo& ei) {
    std::cout << "trace<Search>Search::skip(w:" << ei.wid()
              << ",n:" << ei.nid()
              << ",a:" << ei.alternative() << ")" << std::endl;
  }

  void
  StdSearchTracer::skip(const EdgeInfo& ei, const NodeInfo& ni) {
    std::cout << "trace<Search>Search::skip(w:" << ei.wid()
              << ",n:" << ei.nid()
              << ",a:" << ei.alternative() << ")" << std::endl;
  }

  void 
  StdSearchTracer::node(const EdgeInfo& ei, const NodeInfo& ni) {
    std::cout << "trace<Search>::node(";
    switch (ni.type()) {
    case NodeType::FAILED:
      std::cout << "FAILED";
      break;
    case NodeType::SOLVED:
      std::cout << "SOLVED";
      break;
    case NodeType::BRANCH:
      std::cout << "BRANCH(" << ni.choice().alternatives() << ")";
      break;
    }
    if (!ei)
      std::cout << ",root";
    std::cout << ','
              << "w:" << ni.wid() << ',';
    if (ei)
      std::cout << "p:" << ei.nid() << ',';
    std::cout << "n:" << ni.nid() << ')';
    if (ei) {
      if (ei.wid() != ni.wid())
        std::cout << " [stolen from w:" << ei.wid() << "]";
      std::cout << std::endl
                << '\t' << ei.string()
                << std::endl;
    } else {
      std::cout << std::endl;
    }
  }
  
  void StdSearchTracer::done(void) {
    std::cout << "trace<Search>::done()" << std::endl;
  }

  StdSearchTracer::~StdSearchTracer(void) {}

}

// STATISTICS: search-trace
