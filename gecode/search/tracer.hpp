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

#include <climits>

namespace Gecode {

  /*
   * Engine information
   *
   */
  forceinline
  SearchTracer::EngineInfo::EngineInfo(void) {}

  forceinline
  SearchTracer::EngineInfo::EngineInfo(EngineType et,
                                       unsigned int fst, unsigned int lst)
    : _type(et), _fst(fst), _lst(lst) {}

  forceinline SearchTracer::EngineType
  SearchTracer::EngineInfo::type(void) const {
    return _type;
  }

  forceinline bool
  SearchTracer::EngineInfo::meta(void) const {
    return (type() == EngineType::RBS) || (type() == EngineType::PBS);
  }

  forceinline unsigned int
  SearchTracer::EngineInfo::wfst(void) const {
    assert((type() == EngineType::DFS) || (type() == EngineType::BAB) ||
           (type() == EngineType::LDS) || (type() == EngineType::AOE));
    return _fst;
  }

  forceinline unsigned int
  SearchTracer::EngineInfo::wlst(void) const {
    assert((type() == EngineType::DFS) || (type() == EngineType::BAB) ||
           (type() == EngineType::LDS) || (type() == EngineType::AOE));
    return _lst;
  }

  forceinline unsigned int
  SearchTracer::EngineInfo::workers(void) const {
    return wlst() - wfst();
  }

  forceinline unsigned int
  SearchTracer::EngineInfo::efst(void) const {
    assert((type() == EngineType::RBS) || (type() == EngineType::PBS));
    return _fst;
  }
  
  forceinline unsigned int
  SearchTracer::EngineInfo::elst(void) const {
    assert((type() == EngineType::RBS) || (type() == EngineType::PBS));
    return _lst;
  }

  forceinline
  unsigned int SearchTracer::EngineInfo::engines(void) const {
    return elst() - efst();
  }      


  /*
   * Edge information
   *
   */
  forceinline void
  SearchTracer::EdgeInfo::invalidate(void) {
    _wid=UINT_MAX; _s.clear();
  }

  forceinline void
  SearchTracer::EdgeInfo::init(unsigned int wid, unsigned int nid,
                               unsigned int a) {
    _wid=wid; _nid=nid; _a=a; _s="";
  }

  forceinline void
  SearchTracer::EdgeInfo::init(unsigned int wid, unsigned int nid,
                               unsigned int a,
                               const Space& s, const Choice& c) {
    _wid=wid; _nid=nid; _a=a;
    std::ostringstream os;
    s.print(c, a, os); _s = os.str();
  }

  forceinline
  SearchTracer::EdgeInfo::EdgeInfo(unsigned int wid, unsigned int nid,
                                   unsigned int a)
    : _wid(wid), _nid(nid), _a(a) {}

  forceinline
  SearchTracer::EdgeInfo::EdgeInfo(void)
    : _wid(UINT_MAX) {}

  forceinline
  SearchTracer::EdgeInfo::operator bool(void) const {
    return _wid != UINT_MAX;
  }

  forceinline unsigned int
  SearchTracer::EdgeInfo::wid(void) const {
    assert(*this);
    return _wid;
  }

  forceinline unsigned int
  SearchTracer::EdgeInfo::nid(void) const {
    assert(*this);
    return _nid;
  }

  forceinline unsigned int
  SearchTracer::EdgeInfo::alternative(void) const {
    assert(*this);
    return _a;
  }

  forceinline std::string
  SearchTracer::EdgeInfo::string(void) const {
    assert(*this);
    return _s;
  }


  /*
   * Node information
   *
   */
  forceinline
  SearchTracer::NodeInfo::NodeInfo(NodeType nt,
                                   unsigned int wid, unsigned int nid,
                                   const Space& s, const Choice* c)
    : _nt(nt), _wid(wid), _nid(nid), _s(s), _c(c) {}

  forceinline SearchTracer::NodeType
  SearchTracer::NodeInfo::type(void) const {
    return _nt;
  }

  forceinline unsigned int
  SearchTracer::NodeInfo::wid(void) const {
    return _wid;
  }

  forceinline unsigned int
  SearchTracer::NodeInfo::nid(void) const {
    return _nid;
  }

  forceinline const Space&
  SearchTracer::NodeInfo::space(void) const {
    return _s;
  }

  forceinline const Choice&
  SearchTracer::NodeInfo::choice(void) const {
    assert(_nt == NodeType::BRANCH);
    return *_c;
  }


  /*
   * The actual tracer
   *
   */
  forceinline void
  SearchTracer::_round(unsigned int eid) {
    Support::Lock l(m);
    round(eid);
  }

  forceinline void
  SearchTracer::_skip(const EdgeInfo& ei) {
    Support::Lock l(m);
    skip(ei);
  }

  forceinline void
  SearchTracer::_node(const EdgeInfo& ei, const NodeInfo& ni) {
    Support::Lock l(m);
    node(ei,ni);
  }

  forceinline
  SearchTracer::SearchTracer(void) 
    : pending(1U), n_e(0U), n_w(0U), es(heap), w2e(heap) {}

  forceinline void
  SearchTracer::engine(EngineType t, unsigned int n) {
    assert(pending > 0);
    pending += n-1;
    switch (t) {
    case EngineType::PBS: case EngineType::RBS:
      es[n_e]=EngineInfo(t,n_e+1,n_e+1+n);
      break;
    case EngineType::DFS: case EngineType::BAB:
    case EngineType::LDS: case EngineType::AOE:
      es[n_e]=EngineInfo(t,n_w,n_w+n);
      break;
    default: GECODE_NEVER;
    }
    n_e++;
    assert(pending > 0);
  }

  forceinline void
  SearchTracer::worker(unsigned int& wid, unsigned int& eid) {
    assert(pending > 0);
    pending--;
    w2e[n_w]=eid=n_e-1;
    wid=n_w++;
    if (pending == 0) {
      n_active = n_w;
      init();
    }
  }

  forceinline void
  SearchTracer::worker(void) {
    Support::Lock l(m);
    if (--n_active == 0U)
      done();
  }

  forceinline unsigned int
  SearchTracer::workers(void) const {
    return n_w;
  }

  forceinline unsigned int
  SearchTracer::engines(void) const {
    return n_e;
  }

  forceinline const SearchTracer::EngineInfo&
  SearchTracer::engine(unsigned int eid) const {
    assert(eid < n_e);
    return es[eid];
  }


  forceinline unsigned int
  SearchTracer::eid(unsigned int wid) const {
    assert(wid < n_w);
    return w2e[wid];
  }

  forceinline
  SearchTracer::~SearchTracer(void) {}

}

// STATISTICS: search-trace
