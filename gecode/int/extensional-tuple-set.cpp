/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Zayenz Lagerkvist, 2007
 *     Christian Schulte, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/int/extensional.hh>

namespace Gecode { namespace Int { namespace Extensional {

  /**
   * \brief Iterate table values in a delta interval
   *
   * The tuple-set ranges are searched before iteration, so gaps between
   * supported values do not contribute to the running time.
   */
  class SparseDeltaValues {
  protected:
    const TupleSet::Range* r;
    const TupleSet::Range* e;
    int upper;
    int value;
    int last;

    void
    set_range(int lower) {
      if ((r == e) || (r->min > upper)) {
        r = e;
        return;
      }
      value = (r->min < lower) ? lower : r->min;
      last = (r->max < upper) ? r->max : upper;
    }

  public:
    SparseDeltaValues(const TupleSet& ts, int i, int lower, int upper0)
      : r(ts.fst(i)), e(ts.lst(i)+1), upper(upper0), value(0), last(0) {
      const TupleSet::Range* l = r;
      const TupleSet::Range* u = e;
      while (l < u) {
        const TupleSet::Range* m = l + (u-l) / 2;
        if (m->max < lower)
          l = m + 1;
        else
          u = m;
      }
      r = l;
      set_range(lower);
    }

    bool
    operator ()(void) const {
      return r != e;
    }

    void
    operator ++(void) {
      if (value != last) {
        value++;
        return;
      }
      r++;
      set_range(r == e ? 0 : r->min);
    }

    int
    val(void) const {
      assert(r != e);
      return value;
    }
  };

  /// Advisor shared by the sparse positive, negative, and reified actors
  template<class View>
  class SparseAdvisor : public ViewAdvisor<View> {
  public:
    using ViewAdvisor<View>::view;
  protected:
    int variable;
  public:
    SparseAdvisor(Space& home, Propagator& p, Council<SparseAdvisor>& c,
                  View x, int variable0)
      : ViewAdvisor<View>(home,p,c,x), variable(variable0) {}
    SparseAdvisor(Space& home, SparseAdvisor& advisor)
      : ViewAdvisor<View>(home,advisor), variable(advisor.variable) {}
    int index(void) const {
      return variable;
    }
    void dispose(Space& home, Council<SparseAdvisor>& c) {
      (void) ViewAdvisor<View>::dispose(home,c);
    }
  };

  /// Return the domain-size product, saturated just above \a limit
  template<class View>
  unsigned long long
  domain_product(const ViewArray<View>& x, unsigned long long limit,
                 int excluded=-1) {
    unsigned long long product = 1ULL;
    for (int i=0; i<x.size(); i++) {
      if (i == excluded)
        continue;
      const unsigned long long size =
        static_cast<unsigned long long>(x[i].size());
      if ((product > limit) ||
          ((size > 0ULL) && (product > limit / size)))
        return limit + 1ULL;
      product *= size;
    }
    return product;
  }

  /**
   * \brief Active tuple state shared by sparse table actors
   *
   * The derived actor supplies deactivate_tuple(), which adds any
   * actor-specific bookkeeping to removal from the active tuple set.
   */
  template<class Derived, class View>
  class SparseTupleState {
  protected:
    TupleSet ts;
    int arity;
    unsigned int n_tuples;
    unsigned int n_vals;
    unsigned int* active_ids;
    unsigned int* pos_in_active;
    unsigned int active_limit;
    int* gid_val;
    const unsigned int* tv;

    forceinline Derived&
    derived(void) {
      return static_cast<Derived&>(*this);
    }

    forceinline const unsigned int*
    tuple_gids(unsigned int tid) const {
      const unsigned long long index =
        static_cast<unsigned long long>(tid) *
        static_cast<unsigned long long>(arity);
      return tv + index;
    }

    forceinline unsigned int
    tuple_gid(unsigned int tid, int variable) const {
      const unsigned int gid = tuple_gids(tid)[variable];
      assert(gid < n_vals);
      return gid;
    }

    bool
    remove_tuple(unsigned int tid) {
      if (tid >= n_tuples)
        return false;
      const unsigned int position = pos_in_active[tid];
      if ((position >= active_limit) ||
          (active_ids[position] != tid))
        return false;

      const unsigned int last = active_limit - 1U;
      const unsigned int last_tid = active_ids[last];
      active_ids[position] = last_tid;
      pos_in_active[last_tid] = position;
      active_limit = last;
      return true;
    }

    void
    deactivate_value_support(int variable, int value) {
      const unsigned int* begin = nullptr;
      const unsigned int* end = nullptr;
      unsigned int gid = 0U;
      if (TupleSetAccess::sparse_support(ts,variable,value,begin,end,gid) &&
          derived().support_active(gid))
        for (const unsigned int* tuple=begin; tuple<end; tuple++)
          derived().deactivate_tuple(*tuple);
    }

    void
    deactivate_for_domain(int variable, const View& view) {
      unsigned int position = 0U;
      while (position < active_limit) {
        const unsigned int tid = active_ids[position];
        const unsigned int gid = tuple_gid(tid,variable);
        if (!view.in(gid_val[gid]))
          derived().deactivate_tuple(tid);
        else
          position++;
      }
    }

    void
    deactivate_removed_values(int variable, const View& view,
                              const Delta& delta) {
      if (view.assigned() || view.any(delta)) {
        deactivate_for_domain(variable,view);
        return;
      }
      const unsigned int scan_limit = active_limit;
      unsigned int scanned = 0U;
      for (SparseDeltaValues values(ts,variable,view.min(delta),
                                    view.max(delta)); values(); ++values) {
        if (scanned == scan_limit) {
          deactivate_for_domain(variable,view);
          return;
        }
        scanned++;
        const int value = values.val();
        if (!view.in(value)) {
          deactivate_value_support(variable,value);
          if (active_limit == 0U)
            return;
        }
      }
    }

    void
    deactivate_for_all_domains(const ViewArray<View>& x) {
      unsigned int position = 0U;
      while (position < active_limit) {
        const unsigned int tid = active_ids[position];
        const unsigned int* row = tuple_gids(tid);
        bool keep = true;
        for (int variable=0; variable<arity; variable++)
          if (!x[variable].in(gid_val[row[variable]])) {
            keep = false;
            break;
          }
        if (keep)
          position++;
        else
          derived().deactivate_tuple(tid);
      }
    }

    SparseTupleState(Home home, ViewArray<View>& x, const TupleSet& ts0)
      : ts(ts0), arity(x.size()),
        n_tuples(static_cast<unsigned int>(ts0.tuples())),
        n_vals(TupleSetAccess::sparse_values(ts0)),
        active_ids(static_cast<Space&>(home).alloc<unsigned int>(n_tuples)),
        pos_in_active(static_cast<Space&>(home).alloc<unsigned int>(n_tuples)),
        active_limit(n_tuples),
        gid_val(static_cast<Space&>(home).alloc<int>(n_vals)),
        tv(TupleSetAccess::sparse_tuple_value_ids(ts0)) {
      assert(tv != nullptr);
      for (unsigned int i=0U; i<n_tuples; i++) {
        active_ids[i] = i;
        pos_in_active[i] = i;
      }
      for (unsigned int i=0U; i<n_vals; i++)
        gid_val[i] = 0;
      for (int variable=0; variable<x.size(); variable++)
        for (const TupleSet::Range* range=ts.fst(variable);
             range<=ts.lst(variable); range++) {
          unsigned int gid = 0U;
          if (!TupleSetAccess::support_id(ts,variable,range->min,gid))
            GECODE_NEVER;
          int value = range->min;
          while (true) {
            assert(gid < n_vals);
            gid_val[gid] = value;
            if (value == range->max)
              break;
            value++;
            gid++;
          }
        }
    }

    SparseTupleState(Space& home, const SparseTupleState& state)
      : ts(state.ts), arity(state.arity), n_tuples(state.n_tuples),
        n_vals(state.n_vals),
        active_ids(home.alloc<unsigned int>(state.n_tuples)),
        pos_in_active(home.alloc<unsigned int>(state.n_tuples)),
        active_limit(state.active_limit), gid_val(home.alloc<int>(state.n_vals)),
        tv(TupleSetAccess::sparse_tuple_value_ids(ts)) {
      for (unsigned int i=0U; i<n_tuples; i++) {
        active_ids[i] = state.active_ids[i];
        pos_in_active[i] = state.pos_in_active[i];
      }
      for (unsigned int i=0U; i<n_vals; i++)
        gid_val[i] = state.gid_val[i];
    }
  };

  template<class View, bool pos>
  class SparseInc
    : public Propagator,
      protected SparseTupleState<SparseInc<View,pos>,View> {
  protected:
    typedef Extensional::SparseAdvisor<View> SparseAdvisor;
    typedef SparseTupleState<SparseInc<View,pos>,View> State;
    friend class SparseTupleState<SparseInc<View,pos>,View>;
    using State::active_limit;
    using State::arity;
    using State::deactivate_for_all_domains;
    using State::deactivate_for_domain;
    using State::deactivate_removed_values;
    using State::gid_val;
    using State::n_tuples;
    using State::n_vals;
    using State::remove_tuple;
    using State::ts;
    using State::tuple_gids;
    using State::tv;

    ViewArray<View> x;
    Council<SparseAdvisor> c;
    unsigned int* support_count;
    int* gid_var;
    unsigned int* zero_queue;
    unsigned int zero_queue_size;
    unsigned char* queued;
    bool in_propagate;

    void
    enqueue_zero(unsigned int gid) {
      if ((gid < n_vals) && (queued[gid] == 0U)) {
        queued[gid] = 1U;
        zero_queue[zero_queue_size++] = gid;
      }
    }

    void
    init_gid_maps(void) {
      for (unsigned int i=0U; i<n_vals; i++)
        gid_var[i] = -1;
      for (int a=0; a<x.size(); a++) {
        for (const TupleSet::Range* r=ts.fst(a); r<=ts.lst(a); r++) {
          unsigned int gid = 0U;
          if (!TupleSetAccess::support_id(ts,a,r->min,gid))
            GECODE_NEVER;
          int n = r->min;
          while (true) {
            assert(gid < n_vals);
            gid_var[gid] = a;
            if (n == r->max)
              break;
            n++;
            gid++;
          }
        }
      }
    }

    void
    init_support_counts(void) {
      assert(tv != nullptr);
      const unsigned int* offsets = TupleSetAccess::sparse_support_offsets(ts);
      if (offsets != nullptr) {
        for (unsigned int i=0U; i<n_vals; i++)
          support_count[i] = offsets[i+1U] - offsets[i];
      } else {
        for (unsigned int i=0U; i<n_vals; i++)
          support_count[i] = 0U;
        const unsigned long long n_tv =
          static_cast<unsigned long long>(n_tuples) *
          static_cast<unsigned long long>(arity);
        for (unsigned long long i=0ULL; i<n_tv; i++) {
          const unsigned int gid = tv[i];
          assert(gid < n_vals);
          support_count[gid]++;
        }
      }
    }

    void
    deactivate_tuple(unsigned int tid) {
      if (!remove_tuple(tid))
        return;

      const unsigned int* row = tuple_gids(tid);
      for (int a=0; a<arity; a++) {
        const unsigned int gid = row[a];
        assert(gid < n_vals);
        assert(support_count[gid] > 0U);
        support_count[gid]--;
        if (pos && (support_count[gid] == 0U))
          enqueue_zero(gid);
      }
    }

    forceinline bool
    support_active(unsigned int gid) const {
      assert(gid < n_vals);
      return support_count[gid] != 0U;
    }

    ExecStatus
    process_zero_queue(Space& home) {
      if (zero_queue_size == 0U)
        return ES_OK;

      Region r;
      const int arity = x.size();
      unsigned int* n_rm = r.alloc<unsigned int>(arity);
      unsigned int* p_rm = r.alloc<unsigned int>(arity);
      int** rm = r.alloc<int*>(arity);
      for (int i=0; i<arity; i++) {
        n_rm[i] = 0U;
        p_rm[i] = 0U;
        rm[i] = nullptr;
      }

      for (unsigned int i=0U; i<zero_queue_size; i++) {
        const unsigned int gid = zero_queue[i];
        assert(gid < n_vals);
        queued[gid] = 0U;
        const int a = gid_var[gid];
        if ((a < 0) || (a >= arity))
          continue;
        if (x[a].in(gid_val[gid]))
          n_rm[a]++;
      }

      for (int i=0; i<arity; i++)
        if (n_rm[i] > 0U)
          rm[i] = r.alloc<int>(n_rm[i]);

      for (unsigned int i=0U; i<zero_queue_size; i++) {
        const unsigned int gid = zero_queue[i];
        assert(gid < n_vals);
        const int a = gid_var[gid];
        if ((a < 0) || (a >= arity))
          continue;
        if (x[a].in(gid_val[gid]))
          rm[a][p_rm[a]++] = gid_val[gid];
      }

      zero_queue_size = 0U;

      for (int i=0; i<arity; i++) {
        if (p_rm[i] == 0U)
          continue;
        if (x[i].assigned())
          continue;
        if (p_rm[i] == 1U) {
          GECODE_ME_CHECK(x[i].nq(home,rm[i][0]));
          continue;
        }
        Support::quicksort(rm[i], static_cast<int>(p_rm[i]));
        unsigned int j = 1U;
        for (unsigned int k=1U; k<p_rm[i]; k++)
          if (rm[i][k] != rm[i][j-1U])
            rm[i][j++] = rm[i][k];
        if (j == 1U) {
          GECODE_ME_CHECK(x[i].nq(home,rm[i][0]));
        } else {
          Iter::Values::Array iv(rm[i],j);
          GECODE_ASSUME(j >= 2U);
          GECODE_ME_CHECK(x[i].minus_v(home,iv,false));
        }
      }
      return ES_OK;
    }

    bool
    atmostone(void) const {
      Advisors<SparseAdvisor> as(c);
      if (!as())
        return true;
      ++as;
      return !as();
    }

  public:
    SparseInc(Home home, ViewArray<View>& x0, const TupleSet& ts0)
      : Propagator(home), State(home,x0,ts0), x(home,x0), c(home),
        support_count(static_cast<Space&>(home).alloc<unsigned int>(n_vals)),
        gid_var(static_cast<Space&>(home).alloc<int>(n_vals)),
        zero_queue(static_cast<Space&>(home).alloc<unsigned int>(n_vals)),
        zero_queue_size(0U),
        queued(static_cast<Space&>(home).alloc<unsigned char>(n_vals)),
        in_propagate(false) {
      home.notice(*this, AP_DISPOSE);
      for (unsigned int i=0U; i<n_vals; i++)
        queued[i] = 0U;

      init_gid_maps();
      init_support_counts();

      for (int i=0; i<arity; i++)
        if (!x[i].assigned())
          (void) new (home) SparseAdvisor(home,*this,c,x[i],i);

      deactivate_for_all_domains(x);
    }

    SparseInc(Space& home, SparseInc<View,pos>& p)
      : Propagator(home,p), State(home,p), x(), c(home),
        support_count(home.alloc<unsigned int>(p.n_vals)),
        gid_var(home.alloc<int>(p.n_vals)),
        zero_queue(home.alloc<unsigned int>(p.n_vals)),
        zero_queue_size(p.zero_queue_size),
        queued(home.alloc<unsigned char>(p.n_vals)),
        in_propagate(false) {
      x.update(home,p.x);
      c.update(home,p.c);
      for (unsigned int i=0U; i<n_vals; i++) {
        support_count[i] = p.support_count[i];
        gid_var[i] = p.gid_var[i];
        queued[i] = p.queued[i];
      }
      for (unsigned int i=0U; i<zero_queue_size; i++)
        zero_queue[i] = p.zero_queue[i];
    }

    static ExecStatus
    post(Home home, ViewArray<View>& x, const TupleSet& ts) {
      bool assigned = true;
      for (int i=0; i<x.size(); i++)
        if (!x[i].assigned()) {
          assigned = false;
          break;
        }
      if (assigned) {
        bool in_table = false;
        for (int t=0; t<ts.tuples() && !in_table; t++) {
          TupleSet::Tuple tuple = ts[t];
          bool same = true;
          for (int i=0; i<x.size(); i++)
            if (tuple[i] != x[i].val()) {
              same = false;
              break;
            }
          in_table = same;
        }
        if (pos)
          return in_table ? ES_OK : ES_FAILED;
        return in_table ? ES_FAILED : ES_OK;
      }

      if (pos) {
        if (x.size() == 0)
          return (ts.tuples() == 0) ? ES_FAILED : ES_OK;
        if (ts.tuples() == 0)
          return ES_FAILED;

        for (int i=0; i<x.size(); i++) {
          TupleSet::Ranges r(ts,i);
          GECODE_ME_CHECK(x[i].inter_r(home, r, false));
        }

        if ((x.size() <= 1) || (ts.tuples() <= 1))
          return ES_OK;

        SparseInc<View,pos>* p = new (home) SparseInc<View,pos>(home,x,ts);
        if (p->active_limit == 0U)
          return ES_FAILED;
        View::schedule(home,*p,ME_INT_DOM);
        return ES_OK;
      }

      if (x.size() == 0)
        return (ts.tuples() == 0) ? ES_OK : ES_FAILED;
      if (ts.tuples() == 0)
        return ES_OK;

      SparseInc<View,pos>* p = new (home) SparseInc<View,pos>(home,x,ts);
      View::schedule(home,*p,ME_INT_DOM);
      return ES_OK;
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) SparseInc<View,pos>(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::quadratic(PropCost::HI,x.size());
    }

    virtual void
    reschedule(Space& home) {
      View::schedule(home,*this,ME_INT_DOM);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this, AP_DISPOSE);
      c.dispose(home);
      ts.~TupleSet();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      if (pos) {
        if (active_limit == 0U)
          return ES_FAILED;

        in_propagate = true;
        ExecStatus es = process_zero_queue(home);
        in_propagate = false;
        if (es != ES_OK)
          return es;

        if (active_limit == 0U)
          return ES_FAILED;
        return atmostone() ? home.ES_SUBSUMED(*this) : ES_FIX;
      }

      if (active_limit == 0U)
        return home.ES_SUBSUMED(*this);

      const unsigned long long cap_all =
        static_cast<unsigned long long>(active_limit);
      const unsigned long long all = domain_product(x,cap_all);
      if (all == cap_all)
        return ES_FAILED;

      Region r;
      in_propagate = true;
      for (int i=0; i<x.size(); i++) {
        if (x[i].assigned())
          continue;
        const unsigned long long cap =
          static_cast<unsigned long long>(active_limit);
        const unsigned long long other = domain_product(x,cap,i);
        if (other > cap)
          continue;

        int* rm = r.alloc<int>(x[i].size());
        unsigned int n_rm = 0U;
        for (const TupleSet::Range* rg=ts.fst(i); rg<=ts.lst(i); rg++) {
          unsigned int gid = 0U;
          if (!TupleSetAccess::support_id(ts,i,rg->min,gid))
            GECODE_NEVER;
          int v = rg->min;
          while (true) {
            assert(gid < n_vals);
            if (x[i].in(v) &&
                (support_count[gid] ==
                 static_cast<unsigned int>(other)))
              rm[n_rm++] = v;
            if (v == rg->max)
              break;
            v++;
            gid++;
          }
        }
        if (n_rm == 0U)
          continue;
        if (n_rm == 1U) {
          GECODE_ME_CHECK(x[i].nq(home,rm[0]));
        } else {
          Iter::Values::Array iv(rm,n_rm);
          GECODE_ASSUME(n_rm >= 2U);
          GECODE_ME_CHECK(x[i].minus_v(home,iv,false));
        }
        deactivate_for_domain(i,x[i]);
        if (active_limit == 0U) {
          in_propagate = false;
          return home.ES_SUBSUMED(*this);
        }
        r.free();
      }
      in_propagate = false;
      return ES_FIX;
    }

    virtual ExecStatus
    advise(Space& home, Advisor& a0, const Delta& d) {
      SparseAdvisor& sa = static_cast<SparseAdvisor&>(a0);
      if (active_limit == 0U) {
        if (pos)
          return disabled() ? home.ES_NOFIX_DISPOSE(c,sa) : ES_FAILED;
        return ES_NOFIX;
      }

      View xv = sa.view();
      if (in_propagate)
        return xv.assigned() ? home.ES_FIX_DISPOSE(c,sa) : ES_FIX;

      const int i = sa.index();

      if (xv.assigned()) {
        deactivate_for_domain(i,xv);
        if ((active_limit == 0U) && pos)
          return disabled() ? home.ES_NOFIX_DISPOSE(c,sa) : ES_FAILED;
        return home.ES_NOFIX_DISPOSE(c,sa);
      }
      deactivate_removed_values(i,xv,d);

      if ((active_limit == 0U) && pos)
        return disabled() ? home.ES_NOFIX_DISPOSE(c,sa) : ES_FAILED;
      return ES_NOFIX;
    }
  };

  template<class View, class CtrlView, ReifyMode rm>
  class SparseReifInc
    : public Propagator,
      protected SparseTupleState<SparseReifInc<View,CtrlView,rm>,View> {
  protected:
    typedef Extensional::SparseAdvisor<View> SparseAdvisor;
    typedef SparseTupleState<SparseReifInc<View,CtrlView,rm>,View> State;
    friend class SparseTupleState<SparseReifInc<View,CtrlView,rm>,View>;
    using State::active_limit;
    using State::arity;
    using State::deactivate_for_all_domains;
    using State::deactivate_for_domain;
    using State::deactivate_removed_values;
    using State::n_tuples;
    using State::remove_tuple;
    using State::ts;

    ViewArray<View> x;
    CtrlView b;
    Council<SparseAdvisor> c;

    void
    deactivate_tuple(unsigned int tid) {
      (void) remove_tuple(tid);
    }

    forceinline bool
    support_active(unsigned int) const {
      return true;
    }

  public:
    static ExecStatus
    post_pos(Home home, ViewArray<View>& x, const TupleSet& ts) {
      return SparseInc<View,true>::post(home,x,ts);
    }

    static ExecStatus
    post_neg(Home home, ViewArray<View>& x, const TupleSet& ts) {
      return SparseInc<View,false>::post(home,x,ts);
    }

    SparseReifInc(Home home, ViewArray<View>& x0, const TupleSet& ts0, CtrlView b0)
      : Propagator(home), State(home,x0,ts0), x(home,x0), b(b0), c(home) {
      home.notice(*this, AP_DISPOSE);

      b.subscribe(home,*this,PC_BOOL_VAL);
      for (int i=0; i<arity; i++)
        if (!x[i].assigned())
          (void) new (home) SparseAdvisor(home,*this,c,x[i],i);

      deactivate_for_all_domains(x);
    }

    SparseReifInc(Space& home, SparseReifInc<View,CtrlView,rm>& p)
      : Propagator(home,p), State(home,p), x(), b(), c(home) {
      x.update(home,p.x);
      b.update(home,p.b);
      c.update(home,p.c);
    }

    static ExecStatus
    post(Home home, ViewArray<View>& x, const TupleSet& ts, CtrlView b) {
      if (b.one()) {
        if (rm == RM_PMI)
          return ES_OK;
        return SparseInc<View,true>::post(home,x,ts);
      }
      if (b.zero()) {
        if (rm == RM_IMP)
          return ES_OK;
        return SparseInc<View,false>::post(home,x,ts);
      }
      SparseReifInc<View,CtrlView,rm>* p =
        new (home) SparseReifInc<View,CtrlView,rm>(home,x,ts,b);
      View::schedule(home,*p,ME_INT_DOM);
      return ES_OK;
    }

    virtual Actor*
    copy(Space& home) {
      return new (home) SparseReifInc<View,CtrlView,rm>(home,*this);
    }

    virtual PropCost
    cost(const Space&, const ModEventDelta&) const {
      return PropCost::quadratic(PropCost::HI,x.size());
    }

    virtual void
    reschedule(Space& home) {
      View::schedule(home,*this,ME_INT_DOM);
    }

    virtual size_t
    dispose(Space& home) {
      home.ignore(*this, AP_DISPOSE);
      c.dispose(home);
      b.cancel(home,*this,PC_BOOL_VAL);
      ts.~TupleSet();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }

    virtual ExecStatus
    propagate(Space& home, const ModEventDelta&) {
      if (b.one()) {
        if (rm == RM_PMI)
          return home.ES_SUBSUMED(*this);
        TupleSet keep(ts);
        GECODE_REWRITE(*this,post_pos(home(*this),x,keep));
      }
      if (b.zero()) {
        if (rm == RM_IMP)
          return home.ES_SUBSUMED(*this);
        TupleSet keep(ts);
        GECODE_REWRITE(*this,post_neg(home(*this),x,keep));
      }

      if (active_limit == 0U) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero_none(home));
        return home.ES_SUBSUMED(*this);
      }

      const unsigned long long cap_all =
        static_cast<unsigned long long>(active_limit);
      const unsigned long long all = domain_product(x,cap_all);
      if (all == cap_all) {
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one_none(home));
        return home.ES_SUBSUMED(*this);
      }
      return ES_FIX;
    }

    virtual ExecStatus
    advise(Space& home, Advisor& a0, const Delta& d) {
      SparseAdvisor& sa = static_cast<SparseAdvisor&>(a0);
      if (b.assigned())
        return home.ES_NOFIX_DISPOSE(c,sa);

      if (active_limit == 0U)
        return ES_NOFIX;

      View xv = sa.view();
      const int i = sa.index();
      if (xv.assigned()) {
        deactivate_for_domain(i,xv);
        return home.ES_NOFIX_DISPOSE(c,sa);
      }
      deactivate_removed_values(i,xv,d);
      return ES_NOFIX;
    }
  };

  template<class View>
  ExecStatus
  post_tuple_set(Home home, ViewArray<View>& x, const TupleSet& t, bool pos) {
    switch (t.representation()) {
    case EPK_DENSE:
      return pos ? postposcompact<View>(home,x,t) :
        postnegcompact<View>(home,x,t);
    case EPK_SPARSE:
      return pos ? SparseInc<View,true>::post(home,x,t) :
        SparseInc<View,false>::post(home,x,t);
    case EPK_DENSE_COMPRESSED:
      return pos ? postposcompact_compressed<View>(home,x,t) :
        postnegcompact_compressed<View>(home,x,t);
    case EPK_AUTO:
    default:
      GECODE_NEVER;
      return ES_FAILED;
    }
  }

  forceinline ReifyMode
  negated_reify_mode(ReifyMode mode) {
    switch (mode) {
    case RM_EQV: return RM_EQV;
    case RM_IMP: return RM_PMI;
    case RM_PMI: return RM_IMP;
    default:
      GECODE_NEVER;
      return RM_EQV;
    }
  }

  template<class View, class CtrlView>
  ExecStatus
  post_sparse_reified(Home home, ViewArray<View>& x, const TupleSet& t,
                      CtrlView b, ReifyMode mode) {
    switch (mode) {
    case RM_EQV:
      return SparseReifInc<View,CtrlView,RM_EQV>::post(home,x,t,b);
    case RM_IMP:
      return SparseReifInc<View,CtrlView,RM_IMP>::post(home,x,t,b);
    case RM_PMI:
      return SparseReifInc<View,CtrlView,RM_PMI>::post(home,x,t,b);
    default:
      throw UnknownReifyMode("Int::extensional");
    }
  }

  template<class View, class CtrlView>
  ExecStatus
  post_dense_reified(Home home, ViewArray<View>& x, const TupleSet& t,
                     CtrlView b, ReifyMode mode) {
    switch (mode) {
    case RM_EQV: return postrecompact<View,CtrlView,RM_EQV>(home,x,t,b);
    case RM_IMP: return postrecompact<View,CtrlView,RM_IMP>(home,x,t,b);
    case RM_PMI: return postrecompact<View,CtrlView,RM_PMI>(home,x,t,b);
    default:
      throw UnknownReifyMode("Int::extensional");
    }
  }

  template<class View, class CtrlView>
  ExecStatus
  post_compressed_reified(Home home, ViewArray<View>& x, const TupleSet& t,
                          CtrlView b, ReifyMode mode) {
    switch (mode) {
    case RM_EQV:
      return postrecompact_compressed<View,CtrlView,RM_EQV>(home,x,t,b);
    case RM_IMP:
      return postrecompact_compressed<View,CtrlView,RM_IMP>(home,x,t,b);
    case RM_PMI:
      return postrecompact_compressed<View,CtrlView,RM_PMI>(home,x,t,b);
    default:
      throw UnknownReifyMode("Int::extensional");
    }
  }

  template<class View, class CtrlView>
  ExecStatus
  post_reified_tuple_set(Home home, ViewArray<View>& x, const TupleSet& t,
                         CtrlView b, ReifyMode mode) {
    switch (t.representation()) {
    case EPK_DENSE:
      return post_dense_reified(home,x,t,b,mode);
    case EPK_SPARSE:
      return post_sparse_reified(home,x,t,b,mode);
    case EPK_DENSE_COMPRESSED:
      return post_compressed_reified(home,x,t,b,mode);
    case EPK_AUTO:
    default:
      GECODE_NEVER;
      return ES_FAILED;
    }
  }

  forceinline ExecStatus
  post_reified_constant(Home home, BoolView b, ReifyMode mode, bool value) {
    switch (mode) {
    case RM_EQV:
      if (value)
        GECODE_ME_CHECK(b.one(home));
      else
        GECODE_ME_CHECK(b.zero(home));
      break;
    case RM_IMP:
      if (!value)
        GECODE_ME_CHECK(b.zero(home));
      break;
    case RM_PMI:
      if (value)
        GECODE_ME_CHECK(b.one(home));
      break;
    default:
      GECODE_NEVER;
      return ES_FAILED;
    }
    return ES_OK;
  }

}}}

namespace Gecode {

  void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
              IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;

    if (x.size() == 0) {
      if (pos ? (t.tuples() > 0) : (t.tuples() == 0))
        return;
      home.fail();
      return;
    }
    if (t.tuples() == 0) {
      if (!pos)
        return;
      home.fail();
      return;
    }

    ViewArray<IntView> views(home,x);
    GECODE_ES_FAIL((Extensional::post_tuple_set(home,views,t,pos)));
  }

  void
  extensional(Home home, const IntVarArgs& x, const TupleSet& t, bool pos,
              Reify r, IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;

    if ((x.size() == 0) || (t.tuples() == 0)) {
      const bool value = (x.size() == 0) ?
        (pos ? (t.tuples() > 0) : (t.tuples() == 0)) : !pos;
      BoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_constant
                      (home,control,r.mode(),value)));
      return;
    }

    ViewArray<IntView> views(home,x);
    if (pos) {
      BoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_tuple_set
                      (home,views,t,control,r.mode())));
    } else {
      NegBoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_tuple_set
                      (home,views,t,control,
                       Extensional::negated_reify_mode(r.mode()))));
    }
  }

  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if ((t.min() < 0) || (t.max() > 1))
      throw NotZeroOne("Int::extensional");
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;

    if (x.size() == 0) {
      if (pos ? (t.tuples() > 0) : (t.tuples() == 0))
        return;
      home.fail();
      return;
    }
    if (t.tuples() == 0) {
      if (!pos)
        return;
      home.fail();
      return;
    }

    ViewArray<BoolView> views(home,x);
    GECODE_ES_FAIL((Extensional::post_tuple_set(home,views,t,pos)));
  }

  void
  extensional(Home home, const BoolVarArgs& x, const TupleSet& t, bool pos,
              Reify r, IntPropLevel) {
    using namespace Int;
    if (!t.finalized())
      throw NotYetFinalized("Int::extensional");
    if (t.arity() != x.size())
      throw ArgumentSizeMismatch("Int::extensional");
    if ((t.min() < 0) || (t.max() > 1))
      throw NotZeroOne("Int::extensional");
    if (same(x))
      throw ArgumentSame("Int::extensional");
    GECODE_POST;

    if ((x.size() == 0) || (t.tuples() == 0)) {
      const bool value = (x.size() == 0) ?
        (pos ? (t.tuples() > 0) : (t.tuples() == 0)) : !pos;
      BoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_constant
                      (home,control,r.mode(),value)));
      return;
    }

    ViewArray<BoolView> views(home,x);
    if (pos) {
      BoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_tuple_set
                      (home,views,t,control,r.mode())));
    } else {
      NegBoolView control(r.var());
      GECODE_ES_FAIL((Extensional::post_reified_tuple_set
                      (home,views,t,control,
                       Extensional::negated_reify_mode(r.mode()))));
    }
  }

}

// STATISTICS: int-post
