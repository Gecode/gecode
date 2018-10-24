/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
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
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::CTAdvisor::adjust(void) {
    if (pos) {
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
    } else {
      {
        int n = view().min();
        while ((_fst <= _lst) && (n > _fst->max))
          _fst++;
      }
      {
        int n = view().max();
        while ((_fst <= _lst) && (n < _lst->min))
          _lst--;
      }
    }
  }

  template<class View, bool pos>
  forceinline
  Compact<View,pos>::CTAdvisor::CTAdvisor
  (Space& home, Propagator& p, 
   Council<CTAdvisor>& c, const TupleSet& ts, View x0, int i)
    : ViewAdvisor<View>(home,p,c,x0), _fst(ts.fst(i)), _lst(ts.lst(i)) {
    adjust();
  }

  template<class View, bool pos>
  forceinline
  Compact<View,pos>::CTAdvisor::CTAdvisor(Space& home, CTAdvisor& a)
    : ViewAdvisor<View>(home,a), _fst(a._fst), _lst(a._lst) {}

  template<class View, bool pos>
  forceinline const typename Compact<View,pos>::Range*
  Compact<View,pos>::CTAdvisor::fst(void) const {
    return _fst;
  }

  template<class View, bool pos>
  forceinline const typename Compact<View,pos>::Range*
  Compact<View,pos>::CTAdvisor::lst(void) const {
    return _lst;
  }

  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::CTAdvisor::dispose(Space& home, Council<CTAdvisor>& c) {
    (void) ViewAdvisor<View>::dispose(home,c);
  }


  /*
   * The propagator base class
   *
   */
  template<class View, bool pos>
  const typename Compact<View,pos>::Range*
  Compact<View,pos>::range(CTAdvisor& a, int n) {
    assert((n > a.fst()->max) && (n < a.lst()->min));

    const Range* f=a.fst()+1;
    const Range* l=a.lst()-1;

    assert(!pos || (f<=l));

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

    if (pos) {
      assert((f->min <= n) && (n <= f->max));
      return f;
    } else {
      if ((f <= l) && (f->min <= n) && (n <= f->max))
        return f;
      else
        return nullptr;
    }
  }

  template<class View, bool pos>
  forceinline const BitSetData*
  Compact<View,pos>::supports(CTAdvisor& a, int n) {
    const Range* fnd;
    const Range* fst=a.fst();
    const Range* lst=a.lst();
    if (pos) {
      if (n <= fst->max) {
        fnd=fst;
      } else if (n >= lst->min) {
        fnd=lst;
      } else {
        fnd=range(a,n);
      }
    } else {
      if ((n < fst->min) || (n > lst->max))
        return nullptr;
      if (n <= fst->max) {
        fnd=fst;
      } else if (n >= lst->min) {
        fnd=lst;
      } else {
        fnd=range(a,n);
        if (!fnd)
          return nullptr;
      }
    }
    assert((fnd->min <= n) && (n <= fnd->max));
    return fnd->supports(n_words,n);
  }

  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::ValidSupports::find(void) {
    assert(!pos);
    assert(n <= max);
    while (true) {
      while (xr() && (n > xr.max()))
        ++xr;
      if (!xr()) {
        n = max+1; return;
      }
      assert(n <= xr.max());
      n = std::max(n,xr.min());
      
      while ((sr <= lst) && (n > sr->max))
        sr++;
      if (sr > lst) {
        n = max+1; return;
      }
      assert(n <= sr->max);
      n = std::max(n,sr->min);

      if ((xr.min() <= n) && (n <= xr.max())) {
        s = sr->supports(n_words,n);
        return;
      }
    }
    GECODE_NEVER;
  }

  template<class View, bool pos>
  forceinline
  Compact<View,pos>::ValidSupports::ValidSupports(const Compact<View,pos>& p,
                                                  CTAdvisor& a)
    : n_words(p.n_words), max(a.view().max()),
      xr(a.view()), sr(a.fst()), lst(a.lst()), n(xr.min()) {
    if (pos) {
      while (n > sr->max)
        sr++;
      s = sr->supports(n_words,n);
    } else {
      s = nullptr; // To avoid warnings
      find();
    }
  }
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::ValidSupports::ValidSupports(const TupleSet& ts,
                                                  int i, View x)
    : n_words(ts.words()), max(x.max()),
      xr(x), sr(ts.fst(i)), lst(ts.lst(i)), n(xr.min()) {
    if (pos) {
      while (n > sr->max)
        sr++;
      s = sr->supports(n_words,n);
    } else {
      s = nullptr; // To avoid warnings
      find();
    }
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::ValidSupports::operator ++(void) {
    n++;
    if (pos) {
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
    } else {
      if ((n <= sr->max) && (n <= xr.max())) {
        s += n_words;
      } else if (n <= max) {
        find();
      }
    }
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::ValidSupports::operator ()(void) const {
    return n <= max;
  }
  template<class View, bool pos>
  forceinline const BitSetData*
  Compact<View,pos>::ValidSupports::supports(void) const {
    assert(s == sr->supports(n_words,n));
    return s;
  }
  template<class View, bool pos>
  forceinline int
  Compact<View,pos>::ValidSupports::val(void) const {
    return n;
  }

  /*
   * Lost supports iterator
   *
   */
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::LostSupports::LostSupports
  (const Compact<View,pos>& p, CTAdvisor& a, int l0, int h0)
    : n_words(p.n_words), r(a.fst()), l(l0), h(h0) {
    assert(pos);
    // Move to first value for which there is support
    while (l > r->max)
      r++;
    l = std::max(l,r->min);
    s = r->supports(n_words,l);
  }      
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::LostSupports::operator ++(void) {
    l++; s += n_words;
    while ((l <= h) && (l > r->max)) {
      r++; l=r->min; s=r->s;
    }
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::LostSupports::operator ()(void) const {
    return l<=h;
  }
  template<class View, bool pos>
  forceinline const TupleSet::BitSetData*
  Compact<View,pos>::LostSupports::supports(void) const {
    assert((l >= r->min) && (l <= r->max));
    assert(s == r->supports(n_words,l));
    return s;
  }

  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::all(void) const {
    Advisors<CTAdvisor> as(c);
    return !as();
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::atmostone(void) const {
    Advisors<CTAdvisor> as(c);
    if (!as()) return true;
    ++as;
    return !as();
  }

  template<class View, bool pos>
  forceinline
  Compact<View,pos>::Compact(Space& home, Compact& p)
    : Propagator(home,p), n_words(p.n_words), ts(p.ts) {
    c.update(home,p.c);
  }
  
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::Compact(Home home, const TupleSet& ts0)
    : Propagator(home), n_words(ts0.words()), ts(ts0), c(home) {
    home.notice(*this, AP_DISPOSE);
  }

  template<class View, bool pos>
  template<class Table>
  void
  Compact<View,pos>::setup(Space& home, Table& table, ViewArray<View>& x) {
    // For scheduling the propagator
    ModEvent me = ME_INT_BND;
    Region r;
    BitSetData* mask = r.alloc<BitSetData>(table.size());
    // Invalidate tuples
    for (int i=0; i<x.size(); i++) {
      table.clear_mask(mask);
      for (ValidSupports vs(ts,i,x[i]); vs(); ++vs)
        table.add_to_mask(vs.supports(),mask);
      table.template intersect_with_mask<false>(mask);
      // The propagator must be scheduled for subsumption
      if (table.empty())
        goto schedule;
    }
    // Post advisors
    for (int i=0; i<x.size(); i++)
      if (!x[i].assigned())
        (void) new (home) CTAdvisor(home,*this,c,ts,x[i],i);
      else
        me = ME_INT_VAL;
    // Schedule propagator
  schedule:
    View::schedule(home,*this,me);
  }

  template<class View, bool pos>
  template<class Table>
  forceinline bool
  Compact<View,pos>::full(const Table& table) const {
    unsigned long long int s = 1U;
    for (Advisors<CTAdvisor> as(c); as(); ++as) {
      s *= static_cast<unsigned long long int>(as.advisor().view().size());
      if (s > table.bits())
        return false;
    }
    return s == table.ones();
  }

  template<class View, bool pos>
  PropCost
  Compact<View,pos>::cost(const Space&, const ModEventDelta&) const {
    // Computing this is crucial in case there are many propagators!
    int n = 0;
    // The value of 3 is cheating from the Gecode kernel...
    for (Advisors<CTAdvisor> as(c); as() && (n <= 3); ++as)
      n++;
    return PropCost::quadratic(PropCost::HI,n);
  }

  template<class View, bool pos>
  forceinline size_t
  Compact<View,pos>::dispose(Space& home) {
    home.ignore(*this,AP_DISPOSE);
    c.dispose(home);
    ts.~TupleSet();
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  /*
   * Status for the positive propagator
   *
   */
  template<class View, class Table>
  forceinline
  PosCompact<View,Table>::Status::Status(StatusType t)
    : s(t) {}
  template<class View, class Table>
  forceinline
  PosCompact<View,Table>::Status::Status(const Status& s)
    : s(s.s) {}
  template<class View, class Table>
  forceinline typename PosCompact<View,Table>::StatusType
  PosCompact<View,Table>::Status::type(void) const {
    return static_cast<StatusType>(s & 3);
  }
  template<class View, class Table>
  forceinline bool
  PosCompact<View,Table>::Status::single(CTAdvisor& a) const {
    if (type() != SINGLE)
      return false;
    assert(type() == 0);
    return reinterpret_cast<CTAdvisor*>(s) == &a;
  }
  template<class View, class Table>
  forceinline void
  PosCompact<View,Table>::Status::touched(CTAdvisor& a) {
    if (!single(a))
      s = MULTIPLE;
  }
  template<class View, class Table>
  forceinline void
  PosCompact<View,Table>::Status::none(void) {
    s = NONE;
  }
  template<class View, class Table>
  forceinline void
  PosCompact<View,Table>::Status::propagating(void) {
    s = PROPAGATING;
  }
  
  /*
   * The propagator proper
   *
   */
  template<class View, class Table>
  template<class TableProp>
  forceinline
  PosCompact<View,Table>::PosCompact(Space& home, TableProp& p)
    : Compact<View,true>(home,p), status(NONE), table(home,p.table) {
    assert(!table.empty());
  }

  template<class View, class Table>
  Actor*
  PosCompact<View,Table>::copy(Space& home) {
    assert((table.words() > 0U) && (table.width() >= table.words()));
    if (table.words() <= 4U) {
      switch (table.width()) {
      case 0U:
        GECODE_NEVER; break;
      case 1U:
        return new (home) PosCompact<View,TinyBitSet<1U>>(home,*this);
      case 2U:
        return new (home) PosCompact<View,TinyBitSet<2U>>(home,*this);
      case 3U:
        return new (home) PosCompact<View,TinyBitSet<3U>>(home,*this);
      case 4U:
        return new (home) PosCompact<View,TinyBitSet<4U>>(home,*this);
      default:
        break;
      }
    }
    if (std::is_same<Table,BitSet<unsigned char>>::value) {
      goto copy_char;
    } else if (std::is_same<Table,BitSet<unsigned short int>>::value) {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  GECODE_NEVER;
      default:                       GECODE_NEVER;
      }
    } else {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  goto copy_int;
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
      return nullptr;
    }
  copy_char:
    return new (home) PosCompact<View,BitSet<unsigned char>>(home,*this);
  copy_short:
    return new (home) PosCompact<View,BitSet<unsigned short int>>(home,*this);
  copy_int:
    return new (home) PosCompact<View,BitSet<unsigned int>>(home,*this);
  }

  template<class View, class Table>
  forceinline
  PosCompact<View,Table>::PosCompact(Home home, ViewArray<View>& x,
                                     const TupleSet& ts)
    : Compact<View,true>(home,ts), status(MULTIPLE), table(home,ts.words()) {
    setup(home,table,x);
  }
      
  template<class View, class Table>
  forceinline ExecStatus
  PosCompact<View,Table>::post(Home home, ViewArray<View>& x,
                               const TupleSet& ts) {
    auto ct = new (home) PosCompact(home,x,ts);
    assert((x.size() > 1) && (ts.tuples() > 1));
    return ct->table.empty() ? ES_FAILED : ES_OK;
  }

  template<class View, class Table>
  forceinline size_t
  PosCompact<View,Table>::dispose(Space& home) {
    (void) Compact<View,true>::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Table>
  void
  PosCompact<View,Table>::reschedule(Space& home) {
    // Modified variable, subsumption, or failure
    if ((status.type() != StatusType::NONE) || 
        all() || table.empty())
      View::schedule(home,*this,ME_INT_DOM);
  }

  template<class View, class Table>
  ExecStatus
  PosCompact<View,Table>::propagate(Space& home, const ModEventDelta&) {
    if (table.empty())
      return ES_FAILED;
    if (all())
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
        if (!x.assigned())
          a.adjust();
      } else { // x.size() > 2
        // How many values to remove
        int* nq = r.alloc<int>(x.size());
        unsigned int n_nq = 0;
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
            GECODE_ME_CHECK(x.nq(home,nq[0]));
          } else if (n_nq == x.size() - 1U) {
            GECODE_ME_CHECK(x.eq(home,last_support));
            goto noadjust;
          } else {
            Iter::Values::Array rnq(nq,n_nq);
            GECODE_ASSUME(n_nq >= 2U);
            GECODE_ME_CHECK(x.minus_v(home,rnq,false));
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
    return atmostone() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  template<class View, class Table>
  ExecStatus
  PosCompact<View,Table>::advise(Space& home, Advisor& a0, const Delta& d) {
    CTAdvisor& a = static_cast<CTAdvisor&>(a0);

    // Do not fail a disabled propagator
    if (table.empty())
      return Compact<View,true>::disabled() ?
        home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
      
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
          return Compact<View,true>::disabled() ?
            home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
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
      return Compact<View,true>::disabled() ?
        home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
      
    // Schedule propagator
    return ES_NOFIX;
  }


  /*
   * Post function
   */
  template<class View>
  ExecStatus
  postposcompact(Home home, ViewArray<View>& x, const TupleSet& ts) {
    if (ts.tuples() == 0)
      return (x.size() == 0) ? ES_OK : ES_FAILED;
    
    // All variables pruned to correct domain
    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges r(ts,i);
      GECODE_ME_CHECK(x[i].inter_r(home, r, false));
    }

    if ((x.size() <= 1) || (ts.tuples() <= 1))
      return ES_OK;

    // Choose the right bit set implementation
    switch (ts.words()) {
    case 0U:
      GECODE_NEVER; return ES_OK;
    case 1U:
      return PosCompact<View,TinyBitSet<1U>>::post(home,x,ts);
    case 2U:
      return PosCompact<View,TinyBitSet<2U>>::post(home,x,ts);
    case 3U:
      return PosCompact<View,TinyBitSet<3U>>::post(home,x,ts);
    case 4U:
      return PosCompact<View,TinyBitSet<4U>>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return PosCompact<View,BitSet<unsigned char>>
          ::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return PosCompact<View,BitSet<unsigned short int>>
          ::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return PosCompact<View,BitSet<unsigned int>>
          ::post(home,x,ts);
      default: GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

  
  /*
   * The negative propagator
   *
   */
  template<class View, class Table>
  template<class TableProp>
  forceinline
  NegCompact<View,Table>::NegCompact(Space& home, TableProp& p)
    : Compact<View,false>(home,p), table(home,p.table) {
    assert(!table.empty());
  }

  template<class View, class Table>
  Actor*
  NegCompact<View,Table>::copy(Space& home) {
    assert((table.words() > 0U) && (table.width() >= table.words()));
    if (table.words() <= 4U) {
      switch (table.width()) {
      case 0U:
        GECODE_NEVER; break;
      case 1U:
        return new (home) NegCompact<View,TinyBitSet<1U>>(home,*this);
      case 2U:
        return new (home) NegCompact<View,TinyBitSet<2U>>(home,*this);
      case 3U:
        return new (home) NegCompact<View,TinyBitSet<3U>>(home,*this);
      case 4U:
        return new (home) NegCompact<View,TinyBitSet<4U>>(home,*this);
      default:
        break;
      }
    }
    if (std::is_same<Table,BitSet<unsigned char>>::value) {
      goto copy_char;
    } else if (std::is_same<Table,BitSet<unsigned short int>>::value) {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  GECODE_NEVER;
      default:                       GECODE_NEVER;
      }
    } else {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  goto copy_int;
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
      return nullptr;
    }
  copy_char:
    return new (home) NegCompact<View,BitSet<unsigned char>>(home,*this);
  copy_short:
    return new (home) NegCompact<View,BitSet<unsigned short int>>(home,*this);
  copy_int:
    return new (home) NegCompact<View,BitSet<unsigned int>>(home,*this);
  }

  template<class View, class Table>
  forceinline
  NegCompact<View,Table>::NegCompact(Home home, ViewArray<View>& x,
                                     const TupleSet& ts)
    : Compact<View,false>(home,ts), table(home,ts.words()) {
    setup(home,table,x);
  }
      
  template<class View, class Table>
  forceinline ExecStatus
  NegCompact<View,Table>::post(Home home, ViewArray<View>& x,
                               const TupleSet& ts) {
    auto ct = new (home) NegCompact(home,x,ts);
    return ct->full(ct->table) ? ES_FAILED : ES_OK;
  }

  template<class View, class Table>
  forceinline size_t
  NegCompact<View,Table>::dispose(Space& home) {
    (void) Compact<View,false>::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Table>
  void
  NegCompact<View,Table>::reschedule(Space& home) {
    View::schedule(home,*this,ME_INT_DOM);
  }

  template<class View, class Table>
  ExecStatus
  NegCompact<View,Table>::propagate(Space& home, const ModEventDelta&) {
#ifndef NDEBUG
    if (!table.empty()) {
      // Check that all views have at least one value with support
      for (Advisors<CTAdvisor> as(c); as(); ++as) {
        ValidSupports vs(*this,as.advisor());
        assert(vs());
      }
    }
#endif

    if (table.empty())
      return home.ES_SUBSUMED(*this);

    // Estimate whether any pruning will be possible
    unsigned long long int x_size = 1U;
    unsigned long long int x_max = 1U;
    /* The size of the Cartesian product will be x_size times x_max,
     * where x_max is the size of the largest variable domain.
     */
    for (Advisors<CTAdvisor> as(c); as(); ++as) {
      unsigned long long int n = as.advisor().view().size();
      if (n > x_max) {
        x_size *= x_max; x_max = n;
      } else {
        x_size *= n;
      }
      if (x_size > table.bits())
        return ES_FIX;
    }
    if (x_size > table.ones())
      return ES_FIX;

    {
      // Adjust to size of the Cartesian product
      x_size *= x_max;
      
      Region r;

      for (Advisors<CTAdvisor> as(c); as(); ++as) {
        assert(!table.empty());
        CTAdvisor& a = as.advisor();
        View x = a.view();
        
        // Adjust for the current variable domain
        x_size /= static_cast<unsigned long long int>(x.size());
        
        if ((x_size <= table.bits()) && (x_size <= table.ones())) {
          // How many values to remove
          int* nq = r.alloc<int>(x.size());
          unsigned int n_nq = 0U;
        
          for (ValidSupports vs(*this,a); vs(); ++vs)
            if (x_size == table.ones(vs.supports()))
              nq[n_nq++] = vs.val();
        
          // Remove collected values
          if (n_nq > 0U) {
            if (n_nq == 1U) {
              GECODE_ME_CHECK(x.nq(home,nq[0]));
            } else {
              Iter::Values::Array rnq(nq,n_nq);
              GECODE_ASSUME(n_nq >= 2U);
              GECODE_ME_CHECK(x.minus_v(home,rnq,false));
            }
            if (table.empty())
              return home.ES_SUBSUMED(*this);
            a.adjust();
          }
          r.free();
        }
        // Re-adjust size
        x_size *= static_cast<unsigned long long int>(x.size());
      }
    }

    if (table.ones() == x_size)
      return ES_FAILED;
    if (table.empty() || atmostone())
      return home.ES_SUBSUMED(*this);
    return ES_FIX;
  }

  template<class View, class Table>
  ExecStatus
  NegCompact<View,Table>::advise(Space& home, Advisor& a0, const Delta&) {
    CTAdvisor& a = static_cast<CTAdvisor&>(a0);

    // We are subsumed
    if (table.empty())
      return home.ES_NOFIX_DISPOSE(c,a);
      
    View x = a.view();
      
    a.adjust();

    if (x.assigned()) {
      // Variable is assigned -- intersect with its value
      if (const BitSetData* s = supports(a,x.val()))
        table.template intersect_with_mask<true>(s);
      else
        table.flush(); // Mark as subsumed
      return home.ES_NOFIX_DISPOSE(c,a);
    }
      
    {
      ValidSupports vs(*this,a);
      if (!vs()) {
        table.flush(); // Mark as subsumed
        return home.ES_NOFIX_DISPOSE(c,a);
      }

      Region r;
      BitSetData* mask = r.alloc<BitSetData>(table.size());
      // Collect all tuples to be kept in a temporary mask
      table.clear_mask(mask);
      do {
        table.add_to_mask(vs.supports(),mask);
        ++vs;
      } while (vs());
      table.template intersect_with_mask<false>(mask);
    }
    
    if (table.empty())
      return home.ES_NOFIX_DISPOSE(c,a);
    
    // Schedule propagator
    return ES_NOFIX;
  }


  /*
   * Post function
   */
  template<class View>
  ExecStatus
  postnegcompact(Home home, ViewArray<View>& x, const TupleSet& ts) {
    if (ts.tuples() == 0)
      return ES_OK;

    // Check whether a variable does not overlap with supported values
    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges rs(ts,i);
      ViewRanges<View> rx(x[i]);
      if (Iter::Ranges::disjoint(rs,rx))
        return ES_OK;
    }

    // Choose the right bit set implementation
    switch (ts.words()) {
    case 0U:
      GECODE_NEVER; return ES_OK;
    case 1U:
      return NegCompact<View,TinyBitSet<1U>>::post(home,x,ts);
    case 2U:
      return NegCompact<View,TinyBitSet<2U>>::post(home,x,ts);
    case 3U:
      return NegCompact<View,TinyBitSet<3U>>::post(home,x,ts);
    case 4U:
      return NegCompact<View,TinyBitSet<4U>>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return NegCompact<View,BitSet<unsigned char>>
          ::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return NegCompact<View,BitSet<unsigned short int>>
          ::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return NegCompact<View,BitSet<unsigned int>>
          ::post(home,x,ts);
      default: GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }


  /*
   * The reified propagator
   *
   */
  template<class View, class Table, class CtrlView, ReifyMode rm>
  template<class TableProp>
  forceinline
  ReCompact<View,Table,CtrlView,rm>::ReCompact(Space& home, TableProp& p)
    : Compact<View,false>(home,p), table(home,p.table) {
    b.update(home,p.b);
    y.update(home,p.y);
    assert(!table.empty());
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  Actor*
  ReCompact<View,Table,CtrlView,rm>::copy(Space& home) {
    assert((table.words() > 0U) && (table.width() >= table.words()));
    if (table.words() <= 4U) {
      switch (table.width()) {
      case 0U:
        GECODE_NEVER; break;
      case 1U:
        return new (home) ReCompact<View,TinyBitSet<1U>,CtrlView,rm>
          (home,*this);
      case 2U:
        return new (home) ReCompact<View,TinyBitSet<2U>,CtrlView,rm>
          (home,*this);
      case 3U:
        return new (home) ReCompact<View,TinyBitSet<3U>,CtrlView,rm>
          (home,*this);
      case 4U:
        return new (home) ReCompact<View,TinyBitSet<4U>,CtrlView,rm>
          (home,*this);
      default:
        break;
      }
    }
    if (std::is_same<Table,BitSet<unsigned char>>::value) {
      goto copy_char;
    } else if (std::is_same<Table,BitSet<unsigned short int>>::value) {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  GECODE_NEVER;
      default:                       GECODE_NEVER;
      }
    } else {
      switch (Gecode::Support::u_type(table.width())) {
      case Gecode::Support::IT_CHAR: goto copy_char;
      case Gecode::Support::IT_SHRT: goto copy_short;
      case Gecode::Support::IT_INT:  goto copy_int;
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
      return nullptr;
    }
  copy_char:
    return new (home) ReCompact<View,BitSet<unsigned char>,CtrlView,rm>
      (home,*this);
  copy_short:
    return new (home) ReCompact<View,BitSet<unsigned short int>,CtrlView,rm>
      (home,*this);
  copy_int:
    return new (home) ReCompact<View,BitSet<unsigned int>,CtrlView,rm>
      (home,*this);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  forceinline
  ReCompact<View,Table,CtrlView,rm>::ReCompact(Home home, ViewArray<View>& x,
                                               const TupleSet& ts, CtrlView b0)
    : Compact<View,false>(home,ts), table(home,ts.words()), b(b0), y(x) {
    b.subscribe(home,*this,PC_BOOL_VAL);
    setup(home,table,x);
  }
      
  template<class View, class Table, class CtrlView, ReifyMode rm>
  forceinline ExecStatus
  ReCompact<View,Table,CtrlView,rm>::post(Home home, ViewArray<View>& x,
                                          const TupleSet& ts, CtrlView b) {
    if (b.one()) {
      if (rm == RM_PMI)
        return ES_OK;
      return postposcompact(home,x,ts);
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return ES_OK;
      return postnegcompact(home,x,ts);
    }
    (void) new (home) ReCompact(home,x,ts,b);
    return ES_OK;
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  forceinline size_t
  ReCompact<View,Table,CtrlView,rm>::dispose(Space& home) {
    b.cancel(home,*this,PC_BOOL_VAL);
    (void) Compact<View,false>::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  void
  ReCompact<View,Table,CtrlView,rm>::reschedule(Space& home) {
    View::schedule(home,*this,ME_INT_DOM);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::propagate(Space& home,
                                               const ModEventDelta&) {
    if (b.one()) {
      if (rm == RM_PMI)
        return home.ES_SUBSUMED(*this);
      TupleSet keep(ts);
      GECODE_REWRITE(*this,postposcompact(home(*this),y,keep));
    }
    if (b.zero()) {
      if (rm == RM_IMP)
        return home.ES_SUBSUMED(*this);
      TupleSet keep(ts);
      GECODE_REWRITE(*this,postnegcompact(home(*this),y,keep));
    }

    if (table.empty()) {
      if (rm != RM_PMI)
        GECODE_ME_CHECK(b.zero_none(home));
      return home.ES_SUBSUMED(*this);
    }
    if (full(table)) {
      if (rm != RM_IMP)
        GECODE_ME_CHECK(b.one_none(home));
      return home.ES_SUBSUMED(*this);
    }      

    return ES_FIX;
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::advise(Space& home,
                                            Advisor& a0, const Delta&) {
    CTAdvisor& a = static_cast<CTAdvisor&>(a0);

    // We are subsumed
    if (table.empty() || b.assigned())
      return home.ES_NOFIX_DISPOSE(c,a);
      
    View x = a.view();
      
    a.adjust();
    
    if (x.assigned()) {
      // Variable is assigned -- intersect with its value
      if (const BitSetData* s = supports(a,x.val()))
        table.template intersect_with_mask<true>(s);
      else
        table.flush(); // Mark as failed
      return home.ES_NOFIX_DISPOSE(c,a);
    }
      
    {
      ValidSupports vs(*this,a);
      if (!vs()) {
        table.flush(); // Mark as failed
        return home.ES_NOFIX_DISPOSE(c,a);
      }

      Region r;
      BitSetData* mask = r.alloc<BitSetData>(table.size());
      // Collect all tuples to be kept in a temporary mask
      table.clear_mask(mask);
      do {
        table.add_to_mask(vs.supports(),mask);
        ++vs;
      } while (vs());
      table.template intersect_with_mask<false>(mask);
    }
    
    if (table.empty())
      return home.ES_NOFIX_DISPOSE(c,a);
    
    // Schedule propagator
    return ES_NOFIX;
  }


  /*
   * Post function
   */
  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  postrecompact(Home home, ViewArray<View>& x, const TupleSet& ts,
                CtrlView b) {
    // Enforce invariant that there is at least one tuple...
    if (ts.tuples() == 0) {
      if (x.size() != 0) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
      } else {
        if (rm != RM_IMP)
          GECODE_ME_CHECK(b.one(home));
      }
      return ES_OK;
    }
    // Check whether a variable does not overlap with supported values
    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges rs(ts,i);
      ViewRanges<View> rx(x[i]);
      if (Iter::Ranges::disjoint(rs,rx)) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
    }
    // Choose the right bit set implementation
    switch (ts.words()) {
    case 0U:
      GECODE_NEVER; return ES_OK;
    case 1U:
      return ReCompact<View,TinyBitSet<1U>,CtrlView,rm>::post(home,x,ts,b);
    case 2U:
      return ReCompact<View,TinyBitSet<2U>,CtrlView,rm>::post(home,x,ts,b);
    case 3U:
      return ReCompact<View,TinyBitSet<3U>,CtrlView,rm>::post(home,x,ts,b);
    case 4U:
      return ReCompact<View,TinyBitSet<4U>,CtrlView,rm>::post(home,x,ts,b);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return ReCompact<View,BitSet<unsigned char>,CtrlView,rm>
          ::post(home,x,ts,b);
      case Gecode::Support::IT_SHRT:
        return ReCompact<View,BitSet<unsigned short int>,CtrlView,rm>
          ::post(home,x,ts,b);
      case Gecode::Support::IT_INT:
        return ReCompact<View,BitSet<unsigned int>,CtrlView,rm>
          ::post(home,x,ts,b);
      default: GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}

// STATISTICS: int-prop
