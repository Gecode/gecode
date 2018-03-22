/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
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

#include <algorithm>
#include <type_traits>

namespace Gecode { namespace Int { namespace Extensional {
      
  /*
   * Advisor
   *
   */
  template<class View>
  forceinline void
  Compact<View>::CTAdvisor::adjust(void) {
    {
      int n = view().min();
      assert((_fst->min <= n) && (n <= _lst->max));
      while (n > _fst->max)
        _fst++;
      assert((_fst->min <= n) && (n <= _lst->max));
    }
    {
      int n = view().max();
      assert((_fst->min <= n) && (n <= _lst->max));
      while (n < _lst->min)
        _lst--;
      assert((_fst->min <= n) && (n <= _lst->max));
    }
  }

  template<class View>
  forceinline
  Compact<View>::CTAdvisor::CTAdvisor
  (Space& home, Propagator& p, 
   Council<CTAdvisor>& c, const TupleSet& ts, View x0, int i)
    : ViewAdvisor<View>(home,p,c,x0), _fst(ts.fst(i)), _lst(ts.lst(i)) {
    adjust();
  }

  template<class View>
  forceinline
  Compact<View>::CTAdvisor::CTAdvisor(Space& home, CTAdvisor& a)
    : ViewAdvisor<View>(home,a), _fst(a._fst), _lst(a._lst) {}

  template<class View>
  forceinline const typename Compact<View>::Range*
  Compact<View>::CTAdvisor::fst(void) const {
    return _fst;
  }

  template<class View>
  forceinline const typename Compact<View>::Range*
  Compact<View>::CTAdvisor::lst(void) const {
    return _lst;
  }

  template<class View>
  forceinline void
  Compact<View>::CTAdvisor::dispose(Space& home, Council<CTAdvisor>& c) {
    (void) ViewAdvisor<View>::dispose(home,c);
  }


  /*
   * Status
   *
   */
  template<class View>
  forceinline
  Compact<View>::Status::Status(StatusType t)
    : s(t) {}
  template<class View>
  forceinline
  Compact<View>::Status::Status(const Status& s)
    : s(s.s) {}
  template<class View>
  forceinline typename Compact<View>::StatusType
  Compact<View>::Status::type(void) const {
    return static_cast<StatusType>(s & 3);
  }
  template<class View>
  forceinline bool
  Compact<View>::Status::single(CTAdvisor& a) const {
    if (type() != SINGLE)
      return false;
    assert(type() == 0);
    return reinterpret_cast<CTAdvisor*>(s) == &a;
  }
  template<class View>
  forceinline void
  Compact<View>::Status::touched(CTAdvisor& a) {
    if (!single(a))
      s = MULTIPLE;
  }
  template<class View>
  forceinline void
  Compact<View>::Status::none(void) {
    s = NONE;
  }
  template<class View>
  forceinline void
  Compact<View>::Status::propagating(void) {
    s = PROPAGATING;
  }
  


  /*
   * The propagator base class
   *
   */
  template<class View>
  const typename Compact<View>::Range*
  Compact<View>::range(CTAdvisor& a, int n) {
    assert((n > a.fst()->max) && (n < a.lst()->min));
    const Range* f=a.fst()+1;
    const Range* l=a.lst()-1;
    assert(f<=l);
    while (f < l) {
      const Range* m = f + ((l-f) >> 1);
      if (n < m->min) {
        l=m-1;
      } else if (n > m->max) {
        f=m+1;
      } else {
        f=m; break;
      }
    }
    assert((f->min <= n) && (n <= f->max));
    return f;
  }

  template<class View>
  forceinline const BitSetData*
  Compact<View>::supports(CTAdvisor& a, int n) {
    const Range* fnd;
    const Range* fst=a.fst();
    const Range* lst=a.lst();
    if (n <= fst->max) {
      fnd=fst; goto found;
    } else if (n >= lst->min) {
      fnd=lst; goto found;
    } else {
      fnd=range(a,n);
    }
  found:
    assert((fnd->min <= n) && (n <= fnd->max));
    return fnd->supports(n_words,n);
  }

  template<class View>
  forceinline
  Compact<View>::ValidSupports::ValidSupports(const Compact<View>& p,
                                              CTAdvisor& a)
    : n_words(p.n_words), max(a.view().max()),
      xr(a.view()), sr(a.fst()), n(xr.min()) {
    while (n > sr->max)
      sr++;
    s=sr->supports(n_words,n);
  }
  template<class View>
  forceinline
  Compact<View>::ValidSupports::ValidSupports(const TupleSet& ts,
                                              int i, View x)
    : n_words(ts.words()), max(x.max()), xr(x), sr(ts.fst(i)), n(xr.min()) {
    while (n > sr->max)
      sr++;
    s=sr->supports(n_words,n);
  }
  template<class View>
  forceinline void
  Compact<View>::ValidSupports::operator ++(void) {
    n++;
    if (n <= xr.max()) {
      assert(n <= sr->max);
      s += n_words;
    } else if (n <= max) {
      while (n > xr.max())
        ++xr;
      n = xr.min();
      while (n > sr->max)
        sr++;
      s = sr->supports(n_words,n);
      assert((xr.min() <= n) && (n <= xr.max()));
      assert((sr->min <= n) && (n <= sr->max));
      assert(sr->min <= xr.min());
    }
  }
  template<class View>
  forceinline bool
  Compact<View>::ValidSupports::operator ()(void) const {
    return n <= max;
  }
  template<class View>
  forceinline const BitSetData*
  Compact<View>::ValidSupports::supports(void) const {
    assert(s == sr->supports(n_words,n));
    return s;
  }
  template<class View>
  forceinline int
  Compact<View>::ValidSupports::val(void) const {
    return n;
  }

  /*
   * Lost supports iterator
   *
   */
  template<class View>
  forceinline
  Compact<View>::LostSupports::LostSupports
  (const Compact<View>& p, CTAdvisor& a, int l0, int h0)
    : n_words(p.n_words), r(a.fst()), l(l0), h(h0) {
    // Move to first value for which there is support
    while (l > r->max)
      r++;
    l=std::max(l,r->min);
    s=r->supports(n_words,l);
  }      
  template<class View>
  forceinline void
  Compact<View>::LostSupports::operator ++(void) {
    l++; s += n_words;
    while ((l <= h) && (l > r->max)) {
      r++; l=r->min; s=r->s;
    }
  }
  template<class View>
  forceinline bool
  Compact<View>::LostSupports::operator ()(void) const {
    return l<=h;
  }
  template<class View>
  forceinline const TupleSet::BitSetData*
  Compact<View>::LostSupports::supports(void) const {
    assert((l >= r->min) && (l <= r->max));
    assert(s == r->supports(n_words,l));
    return s;
  }

  template<class View>
  forceinline
  Compact<View>::Compact(Space& home, Compact& p)
    : Propagator(home,p), unassigned(p.unassigned), n_words(p.n_words),
      status(NONE), ts(p.ts) {
    c.update(home,p.c);
  }
  
  template<class View>
  forceinline
  Compact<View>::Compact(Home home, ViewArray<View>& x,
                         const TupleSet& ts0)
    : Propagator(home), unassigned(x.size()), n_words(ts0.words()),
      status(MULTIPLE), ts(ts0), c(home) {
    home.notice(*this, AP_DISPOSE);
  }
      
  template<class View>
  forceinline size_t
  Compact<View>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    c.dispose(home);
    ts.~TupleSet();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  /*
   * The propagator proper
   *
   */
  template<class View, class Table>
  template<class TableProp>
  forceinline
  CompactTable<View,Table>::CompactTable(Space& home, TableProp& p)
    : Compact<View>(home,p), table(home,p.table) {
    assert(!table.empty());
  }

  template<class View, class Table>
  Actor*
  CompactTable<View,Table>::copy(Space& home) {
    assert((table.words() > 0U) && (table.width() >= table.words()));
    if (table.words() <= 4U) {
      switch (table.width()) {
      case 0U:
        GECODE_NEVER; break;
      case 1U:
        return new (home) CompactTable<View,TinyBitSet<1U>>(home,*this);
      case 2U:
        return new (home) CompactTable<View,TinyBitSet<2U>>(home,*this);
      case 3U:
        return new (home) CompactTable<View,TinyBitSet<3U>>(home,*this);
      case 4U:
        return new (home) CompactTable<View,TinyBitSet<4U>>(home,*this);
      default:
        break;
      }
    }
    if (std::is_same<Table,BitSet<unsigned char>>::value) {
      goto copy_char;
    } else if (std::is_same<Table,BitSet<unsigned short int>>::value) {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR:
        goto copy_char;
      case Gecode::Support::IT_SHRT:
        goto copy_short;
      case Gecode::Support::IT_INT:
      default:
        GECODE_NEVER;
      }
    } else {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR:
        goto copy_char;
      case Gecode::Support::IT_SHRT:
        goto copy_short;
      case Gecode::Support::IT_INT:
        return new (home) CompactTable<View,BitSet<unsigned int>>(home,*this);
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
      return nullptr;
    }
  copy_char:
    return new (home) CompactTable<View,BitSet<unsigned char>>(home,*this);
  copy_short:
    return new (home) CompactTable<View,BitSet<unsigned short int>>(home,*this);
  }

  template<class View,class Table>
  forceinline
  CompactTable<View,Table>::CompactTable(Home home, ViewArray<View>& x,
                                         const TupleSet& ts)
    : Compact<View>(home,x,ts), table(home,ts.words()) {
    Region r;
    BitSetData* mask = r.alloc<BitSetData>(table.size());
    // Invalidate tuples
    for (int i = x.size(); i--; ) {
      table.clear_mask(mask);
      for (ValidSupports vs(ts,i,x[i]); vs(); ++vs)
        table.add_to_mask(vs.supports(),mask);
      table.template intersect_with_mask<false>(mask);
      if (table.empty())
        return;
    }
    // Post advisors
    for (int i = x.size(); i--; ) {
      if (!x[i].assigned())
        (void) new (home) CTAdvisor(home,*this,c,ts,x[i],i);
      else
        unassigned--;
    }
    // Schedule propagator
    if (unassigned < x.size())
      View::schedule(home,*this,ME_INT_VAL);
    else
      View::schedule(home,*this,ME_INT_BND);
  }
      
  template<class View, class Table>
  forceinline size_t
  CompactTable<View,Table>::dispose(Space& home) {
    (void) Compact<View>::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Table>
  PropCost
  CompactTable<View,Table>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,unassigned);
  }

  template<class View, class Table>
  void
  CompactTable<View,Table>::reschedule(Space& home) {
    // Modified variable, subsumption, or failure
    if ((status.type() != StatusType::NONE) || (unassigned == 0) || table.empty())
      View::schedule(home,*this,ME_INT_DOM);
  }

  template<class View, class Table>
  ExecStatus
  CompactTable<View,Table>::propagate(Space& home, const ModEventDelta&) {
    if (table.empty())
      return ES_FAILED;
    if (unassigned == 0)
      return home.ES_SUBSUMED(*this);

    Status touched(status);
    // Mark as performing propagation
    status.propagating();

    Region r;
    // Scan all values of all unassigned variables to see if they
    // are still supported.
    for (Advisors<CTAdvisor> as(c); as(); ++as) {
      CTAdvisor& a = as.advisor();
      View x = a.view();

      // No point filtering variable if it was the only modified variable
      if (touched.single(a) || x.assigned())
        continue;
      
      if (x.size() == 2) { // Consider min and max values only
        if (!table.intersects(supports(a,x.min())))
          GECODE_ME_CHECK(x.eq(home,x.max()));
        else if (!table.intersects(supports(a,x.max())))
          GECODE_ME_CHECK(x.eq(home,x.min()));
        if (x.assigned())
          unassigned--;
        else
          a.adjust();
      } else { // x.size() > 2
        // How many values to remove
        int* nq = r.alloc<int>(x.size());
        unsigned int n_nq = 0U;
        // The initialization is here just to avoid warnings...
        int last_support = 0;
        for (ValidSupports vs(*this,a); vs(); ++vs)
          if (!table.intersects(vs.supports()))
            nq[n_nq++] = vs.val();
          else
            last_support = vs.val();
        // Remove collected values
        if (n_nq > 0U) {
          if (n_nq == 1U) {
            ModEvent me = x.nq(home,nq[0]);
            if (me_failed(me)) return ES_FAILED;
            assert(me != ME_INT_VAL);
          } else if (n_nq == x.size() - 1U) {
            GECODE_ME_CHECK(x.eq(home,last_support));
            unassigned--;
            goto noadjust;
          } else {
            Iter::Values::Array rnq(nq,n_nq);
            GECODE_ASSUME(n_nq >= 2U);
            ModEvent me = x.minus_v(home,rnq,false);
            if (me_failed(me)) return ES_FAILED;
            assert(me != ME_INT_VAL);
          }
          a.adjust();
        noadjust: ;
        }
        r.free();
      }
    }

    // Mark as no touched variable
    status.none();
    // Should not be in a failed state
    assert(!table.empty());
    // Subsume if there is at most one non-assigned variable
    return (unassigned <= 1) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, class Table>
  forceinline ExecStatus
  CompactTable<View,Table>::post(Home home, ViewArray<View>& x,
                                 const TupleSet& ts) {
    // All variables pruned to correct domain
    for (int i=x.size(); i--; ) {
      TupleSet::Ranges r(ts,i);
      GECODE_ME_CHECK(x[i].inter_r(home, r, false));
    }
    if ((x.size() > 1) && (ts.tuples() > 1)) {
      CompactTable<View,Table>* ct = new (home) CompactTable(home,x,ts);
      if (ct->table.empty())
        return ES_FAILED;
    }
    return ES_OK;
  }

  template<class View, class Table>
  ExecStatus
  CompactTable<View,Table>::advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor& a = static_cast<CTAdvisor&>(a0);

    // Do not fail a disabled propagator
    if (table.empty())
      return Compact<View>::disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
    
    View x = a.view();

    /* 
     * Do not schedule if propagator is performing propagation,
     * and dispose if assigned.
     */ 
    if (status.type() == StatusType::PROPAGATING)
      return x.assigned() ? home.ES_FIX_DISPOSE(c,a) : ES_FIX;

    // Update status
    status.touched(a);

    if (x.assigned()) {
      // Variable is assigned -- intersect with its value
      table.template intersect_with_mask<true>(supports(a,x.val()));
      unassigned--;
      return home.ES_NOFIX_DISPOSE(c,a);
    }

    if (!x.any(d) && (x.min(d) == x.max(d))) {
      table.nand_with_mask(supports(a,x.min(d)));
      a.adjust();
    } else if (!x.any(d) && (x.width(d) <= x.size())) {
      // Incremental update, using the removed values
      for (LostSupports ls(*this,a,x.min(d),x.max(d)); ls(); ++ls) {
        table.nand_with_mask(ls.supports());
        if (table.empty())
          return Compact<View>::disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
      }
      a.adjust();
    } else {
      a.adjust();
      // Reset-based update, using the values that are left
      if (x.size() == 2) {
        table.intersect_with_masks(supports(a,x.min()),
                                   supports(a,x.max()));
      } else {
        Region r;
        BitSetData* mask = r.alloc<BitSetData>(table.size());
        // Collect all tuples to be kept in a temporary mask
        table.clear_mask(mask);
        for (ValidSupports vs(*this,a); vs(); ++vs)
          table.add_to_mask(vs.supports(),mask);
        table.template intersect_with_mask<false>(mask);
      }
    }

    // Do not fail a disabled propagator
    if (table.empty())
      return Compact<View>::disabled() ? home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
    
    // Schedule propagator
    return ES_NOFIX;
  }


  /*
   * Post function
   */
  template<class View>
  inline ExecStatus
  postcompact(Home home, ViewArray<View>& x, const TupleSet& ts) {
    assert(ts.words() > 0U);
    // All variables pruned to correct domain
    for (int i=x.size(); i--; ) {
      TupleSet::Ranges r(ts,i);
      GECODE_ME_CHECK(x[i].inter_r(home, r, false));
    }
    // Choose the right bit set implementation
    switch (ts.words()) {
    case 0U:
      GECODE_NEVER; return ES_OK;
    case 1U:
      return CompactTable<View,TinyBitSet<1U>>::post(home,x,ts);
    case 2U:
      return CompactTable<View,TinyBitSet<2U>>::post(home,x,ts);
    case 3U:
      return CompactTable<View,TinyBitSet<3U>>::post(home,x,ts);
    case 4U:
      return CompactTable<View,TinyBitSet<4U>>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return CompactTable<View,BitSet<unsigned char>>::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return CompactTable<View,BitSet<unsigned short int>>::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return CompactTable<View,BitSet<unsigned int>>::post(home,x,ts);
      default: GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}


// STATISTICS: int-prop
