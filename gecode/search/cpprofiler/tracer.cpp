/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 *
 *  Contributing authors:
 *     Kevin Leo <kevin.leo@monash.edu>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Kevin Leo, 2017
 *     Christian Schulte, 2017
 *     Maxim Shishmarev, 2017
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

#ifdef GECODE_HAS_CPPROFILER

#include <gecode/search/cpprofiler/message.hpp>
#include <gecode/search/cpprofiler/connector.hpp>

namespace Gecode {

  CPProfilerSearchTracer::GetInfo::GetInfo(void) {}

  CPProfilerSearchTracer::GetInfo::~GetInfo(void) {}


  CPProfilerSearchTracer::CPProfilerSearchTracer(int eid, std::string name0,
                                                 unsigned int port,
                                                 const GetInfo* pgetinfo) :
    connector(new CPProfiler::Connector(port)), execution_id(eid), name(name0), restart(0), 
    pgi(pgetinfo) {
  }

  void
  CPProfilerSearchTracer::init(void) {
    // Try to find out whether engine is a restart engine
    bool restarts = ((engines() == 2U) &&
                     (engine(0U).type() == EngineType::RBS));
    connector->connect();
    connector->start(name, execution_id, restarts);
  }

  void
  CPProfilerSearchTracer::round(unsigned int) {
    restart++;
    connector->restart(restart);
  }

  void
  CPProfilerSearchTracer::skip(const EdgeInfo& ei) {
    CPProfiler::NodeUID dummy_node{-1, restart, -1};
    CPProfiler::NodeUID parent{-1,restart,-1};

    int alt = 0;
    std::string label;
    if (ei) {
      parent.nid = static_cast<int>(ei.nid());
      parent.tid = static_cast<int>(ei.wid());
      dummy_node.tid = static_cast<int>(ei.wid());
      alt = static_cast<int>(ei.alternative());
      label = ei.string();
    }

    auto node = connector->createNode(dummy_node, parent,
                                     alt, 0, CPProfiler::NodeStatus::SKIPPED)
      .set_label(label)
      .set_info("");
    connector->sendNode(node);
  }

  void
  CPProfilerSearchTracer::node(const EdgeInfo& ei, const NodeInfo& ni) {
    CPProfiler::NodeUID this_node{static_cast<int>(ni.nid()),
        restart,
        static_cast<int>(ni.wid())};
    CPProfiler::NodeUID parent {-1,restart,-1};

    int alt = 0;
    int alts = 0;

    CPProfiler::NodeStatus ns = CPProfiler::NodeStatus::FAILED;
    switch(ni.type()) {
    case NodeType::SOLVED:
      ns = CPProfiler::NodeStatus::SOLVED;
      break;
    case NodeType::BRANCH:
      ns = CPProfiler::NodeStatus::BRANCH;
      alts = static_cast<int>(ni.choice().alternatives());
      break;
    case NodeType::FAILED:
      ns = CPProfiler::NodeStatus::FAILED;
      break;
    default:
      GECODE_NEVER;
    }

    std::string label;
    if(ei) {
      parent = {static_cast<int>(ei.nid()),
                restart,
                static_cast<int>(ei.wid())};

      alt = static_cast<int>(ei.alternative());
      label = ei.string();
    } else {
      alt = restart;
    }

    std::string info;
    if(pgi) {
      info = pgi->getInfo(ni.space());
    }

    auto node = connector->createNode(this_node, parent, alt, alts, ns)
      .set_label(label)
      .set_info(info);
    connector->sendNode(node);
  }

  void
  CPProfilerSearchTracer::done(void) {
    connector->disconnect();
  }

  CPProfilerSearchTracer::~CPProfilerSearchTracer(void) {
    delete connector;
    delete pgi;
  }

}

#endif

// STATISTICS: search-trace
