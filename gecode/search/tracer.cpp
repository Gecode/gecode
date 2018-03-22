/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

  StdSearchTracer::StdSearchTracer(std::ostream& os0)
    : os(os0) {}

  void
  StdSearchTracer::init(void) {
    os << "trace<Search>::init()" << std::endl;
      for (unsigned int e=0U; e<engines(); e++) {
        os << "\t" << e << ": " 
           << t2s[engine(e).type()];
        if (engine(e).meta()) {
          os << ", engines: {";
          for (unsigned int i=engine(e).efst(); i<engine(e).elst(); i++) {
            os << i; if (i+1 < engine(e).elst()) os << ",";
          }
        } else {
          os << ", workers: {";
          for (unsigned int i=engine(e).wfst(); i<engine(e).wlst(); i++) {
            os << i; if (i+1 < engine(e).wlst()) os << ",";
          }
        }
        os << "}" << std::endl;
      }
    }

  void
  StdSearchTracer::round(unsigned int eid) {
    os << "trace<Search>::round(e:" << eid << ")" << std::endl;
  }

  void
  StdSearchTracer::skip(const EdgeInfo& ei) {
    os << "trace<Search>Search::skip(w:" << ei.wid()
       << ",n:" << ei.nid()
       << ",a:" << ei.alternative() << ")" << std::endl;
  }

  void 
  StdSearchTracer::node(const EdgeInfo& ei, const NodeInfo& ni) {
    os << "trace<Search>::node(";
    switch (ni.type()) {
    case NodeType::FAILED:
      os << "FAILED";
      break;
    case NodeType::SOLVED:
      os << "SOLVED";
      break;
    case NodeType::BRANCH:
      os << "BRANCH(" << ni.choice().alternatives() << ")";
      break;
    }
    if (!ei)
      os << ",root";
    os << ",w:" << ni.wid() << ',';
    if (ei)
      os << "p:" << ei.nid() << ',';
    os << "n:" << ni.nid() << ')';
    if (ei) {
      if (ei.wid() != ni.wid())
        os << " [stolen from w:" << ei.wid() << "]";
      os << std::endl
         << '\t' << ei.string()
         << std::endl;
    } else {
      os << std::endl;
    }
  }
  
  void
  StdSearchTracer::done(void) {
    os << "trace<Search>::done()" << std::endl;
  }

  StdSearchTracer::~StdSearchTracer(void) {}

  StdSearchTracer StdSearchTracer::def;

}

// STATISTICS: search-trace
