/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Samuel Gagnon, 2018
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

#ifdef GECODE_HAS_CBS

namespace Gecode { namespace Int { namespace Branch {

  template<class View>
  forceinline void
  CBSBrancher<View>::VarIdToPos::init() {
    assert(object() == nullptr);
    object(new VarIdToPosO());
  }

  template<class View>
  forceinline bool
  CBSBrancher<View>::VarIdToPos::isIn(unsigned int var_id) const {
    auto *hm = &static_cast<VarIdToPosO*>(object())->_varIdToPos;
    return hm->find(var_id) != hm->end();
  }

  template<class View>
  forceinline int
  CBSBrancher<View>::VarIdToPos::operator[](unsigned int i) const {
    return static_cast<VarIdToPosO*>(object())->_varIdToPos.at(i);
  }

  template<class View>
  forceinline void
  CBSBrancher<View>::VarIdToPos::insert(unsigned int var_id,
                                             unsigned int pos) {
    static_cast<VarIdToPosO*>(object())
      ->_varIdToPos.insert(std::make_pair(var_id, pos));
  }

  template<class View>
  CBSBrancher<View>::CBSBrancher(Home home, ViewArray<View>& x0)
    : Brancher(home), x(x0),
      logProp(typename decltype(logProp)::size_type(),
              typename decltype(logProp)::hasher(),
              typename decltype(logProp)::key_equal(),
              typename decltype(logProp)::allocator_type(home)) {
    home.notice(*this, AP_DISPOSE);
    varIdToPos.init();
    for (int i=0; i<x.size(); i++)
      varIdToPos.insert(x[i].id(), i);
  }

  template<class View>
  forceinline void
  CBSBrancher<View>::post(Home home, ViewArray<View>& x) {
    (void) new (home) CBSBrancher(home,x);
  }

  template<class View>
  Actor* CBSBrancher<View>::copy(Space& home) {
    return new (home) CBSBrancher(home,*this);
  }

  template<class View>
  forceinline size_t
  CBSBrancher<View>::dispose(Space& home) {
    home.ignore(*this, AP_DISPOSE);
    varIdToPos.~VarIdToPos();
    (void) Brancher::dispose(home);
    return sizeof(*this);
  }

  template<class View>
  CBSBrancher<View>::CBSBrancher(Space& home, CBSBrancher& b)
    : Brancher(home,b),
      varIdToPos(b.varIdToPos),
      logProp(b.logProp.begin(), b.logProp.end(),
              typename decltype(logProp)::size_type(),
              typename decltype(logProp)::hasher(),
              typename decltype(logProp)::key_equal(),
              typename decltype(logProp)::allocator_type(home)) {
    x.update(home,b.x);
  }

  template<class View>
  bool CBSBrancher<View>::status(const Space& home) const {
    for (Propagators p(home, PropagatorGroup::all); p(); ++p) {
      // Sum of domains of all variable in propagator
      unsigned int domsum;
      // Same, but for variables that are also in this brancher.
      unsigned int domsum_b;

      // If the propagator doesn't support counting-based search, domsum and
      // domsum_b are going to be equal to 0.
      p.propagator().domainsizesum([this](unsigned int var_id)
                         { return inbrancher(var_id); },
                         domsum, domsum_b);

      if (domsum_b > 0)
        return true;
    }

    return false;
  }

  template<class View>
  forceinline bool
  CBSBrancher<View>::inbrancher(unsigned int varId) const {
    return varIdToPos.isIn(varId);
  }

  template<class View>
  const Choice* CBSBrancher<View>::choice(Space& home) {
    // Structure for keeping the maximum solution density assignment
    struct {
      unsigned int var_id;
      int val;
      double dens;
    } maxSD{0, 0, -1};

    // Lambda we pass to propagators via solndistrib to query solution densities
    auto SendMarginal = [this](unsigned int prop_id, unsigned int var_id,
                               int val, double dens) {
        if (logProp[prop_id].dens < dens) {
          logProp[prop_id].var_id = var_id;
          logProp[prop_id].val = val;
          logProp[prop_id].dens = dens;
        }
    };

    for (auto& kv : logProp)
      kv.second.visited = false;

    for (Propagators p(home, PropagatorGroup::all); p(); ++p) {
      unsigned int prop_id = p.propagator().id();
      unsigned int domsum;
      unsigned int domsum_b;

      p.propagator().domainsizesum([this](unsigned int var_id)
                                   { return inbrancher(var_id); },
                                   domsum, domsum_b);

      // If the propagator doesn't share any unasigned variables with this
      // brancher, we continue and it will be deleted from the log afterwards.
      if (domsum_b == 0)
        continue;

      // New propagators can be created as we solve the problem. If this is the
      // case we create a new entry in the log.
      if (logProp.find(prop_id) == logProp.end())
        logProp.insert(std::make_pair(prop_id, PropInfo{0, 0, 0, -1, true}));
      else
        logProp[prop_id].visited = true;

      // If the domain size sum of all variables in the propagator has changed
      // since the last time we called this function, we need to recompute
      // solution densities. Otherwise, we can reuse them.
      if (logProp[prop_id].domsum != domsum) {
        logProp[prop_id].dens = -1;
        // Solution density computation
        p.propagator().solndistrib(home, SendMarginal);
        logProp[prop_id].domsum = domsum;
      }
    }

    // We delete unvisited propagators from the log and look for the highest
    // solution density across all propagators.
    for (const auto& kv : logProp) {
      unsigned int prop_id = kv.first;
      const PropInfo& info = kv.second;
      if (!info.visited)
        logProp.erase(prop_id);
      else if (info.dens > maxSD.dens)
        maxSD = {info.var_id, info.val, info.dens};
    }

    assert(maxSD.dens != -1);
    assert(!x[varIdToPos[maxSD.var_id]].assigned());
    return new PosValChoice<int>(*this, 2, varIdToPos[maxSD.var_id], maxSD.val);
  }

  template<class View>
  forceinline const Choice*
  CBSBrancher<View>::choice(const Space&, Archive& e) {
    int pos, val;
    e >> pos >> val;
    return new PosValChoice<int>(*this, 2, pos, val);
  }

  template<class View>
  forceinline ExecStatus
  CBSBrancher<View>::commit(Space& home, const Choice& c, unsigned int a) {
    const auto& pvc = static_cast<const PosValChoice<int>&>(c);
    int pos = pvc.pos().pos;
    int val = pvc.val();
    if (a == 0)
      return me_failed(x[pos].eq(home, val)) ? ES_FAILED : ES_OK;
    else
      return me_failed(x[pos].nq(home, val)) ? ES_FAILED : ES_OK;
  }

  template<class View>
  forceinline void
  CBSBrancher<View>::print(const Space&, const Choice& c, unsigned int a,
                           std::ostream& o) const {
    const auto& pvc = static_cast<const PosValChoice<int>&>(c);
    int pos=pvc.pos().pos, val=pvc.val();
    if (a == 0)
      o << "x[" << pos << "] = " << val;
    else
      o << "x[" << pos << "] != " << val;
  }

}}}

#endif

// STATISTICS: int-branch
