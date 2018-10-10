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
    /*    std::cout << "CTAdvisor(" << i << "," << x << ")" << std::endl;    
    for (const Range* r = _fst; r <= _lst; r++)
      std::cout << "\t" << r->min << "..." << r->max << std::endl;
    std::cout << "adjust()" << std::endl;
    */
    adjust();
    /*
    for (const Range* r = _fst; r <= _lst; r++)
      std::cout << "\t" << r->min << "..." << r->max << std::endl;
    */
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
   * Status
   *
   */
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::Status::Status(StatusType t)
    : s(t) {}
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::Status::Status(const Status& s)
    : s(s.s) {}
  template<class View, bool pos>
  forceinline typename Compact<View,pos>::StatusType
  Compact<View,pos>::Status::type(void) const {
    return static_cast<StatusType>(s & 3);
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::Status::single(CTAdvisor& a) const {
    if (type() != SINGLE)
      return false;
    assert(type() == 0);
    return reinterpret_cast<CTAdvisor*>(s) == &a;
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::Status::touched(CTAdvisor& a) {
    if (!single(a))
      s = MULTIPLE;
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::Status::none(void) {
    s = NONE;
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::Status::propagating(void) {
    s = PROPAGATING;
  }
  


  /*
   * The propagator base class
   *
   */
  template<class View, bool pos>
  const typename Compact<View,pos>::Range*
  Compact<View,pos>::range(CTAdvisor& a, int n) {
    assert((n > a.fst()->max) && (n < a.lst()->min));
    if (pos) {
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
    } else {
      const Range* f=a.fst()+1;
      const Range* l=a.lst()-1;
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
        fnd=fst; goto foundp;
      } else if (n >= lst->min) {
        fnd=lst; goto foundp;
      } else {
        fnd=range(a,n);
      }
    foundp:
      assert((fnd->min <= n) && (n <= fnd->max));
      return fnd->supports(n_words,n);
    } else {
      if ((n < fst->min) || (n > lst->max))
        return nullptr;
      if (n <= fst->max) {
        fnd=fst; goto foundn;
      } else if (n >= lst->min) {
        fnd=lst; goto foundn;
      } else {
        fnd=range(a,n);
        if (!fnd)
          return nullptr;
      }
    foundn:
      assert((fnd->min <= n) && (n <= fnd->max));
      return fnd->supports(n_words,n);
    }
  }

  template<class View, bool pos>
  forceinline unsigned long long int
  Compact<View,pos>::size(void) const {
    unsigned long long int s = 1U;
    for (Advisors<CTAdvisor> as(c); as(); ++as)
      s *= static_cast<unsigned long long int>(as.advisor().view().size());
    return s;
  }

  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::ValidSupports::find(void) {
    //    std::cout << "ValidSupports:find()" << std::endl;
    assert(!pos);
    assert(n <= max);
    while (true) {
      /*
      std::cout << "find..." << std::endl;
      if (xr()) {
        std::cout << "\t(1) xr(), min=" << xr.min() << ", max="
                  << xr.max() << std::endl;
      }
      */
      while (xr() && (n > xr.max()))
        ++xr;
      if (!xr()) {
        n = max+1; return;
      }
      /*
      if (xr()) {
        std::cout << "\t(2) xr(), min=" << xr.min() << ", max="
                  << xr.max() << ", n=" << n << std::endl;
      }
      */
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
      //      std::cout << "ValidSupports(" << i << "," << x << ")" << std::endl;
      find();
      /*
      std::cout << "\t" << "n=" << n << ", max=" << max << std::endl;
      for (const Range* r = ts.fst(i); r <= ts.lst(i); r++)
        std::cout << "\t\t" << r->min << "..." << r->max << std::endl;
      */
    }
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::ValidSupports::operator ++(void) {
    //    std::cout << "ValidSupports:++()" << std::endl;
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
    // Move to first value for which there is support
    if (pos) {
      while (l > r->max)
        r++;
      l = std::max(l,r->min);
      s = r->supports(n_words,l);
    } else {
      while ((r <= lst) && (l > r->max))
        r++;
      if (r > lst) {
        l=h+1;
      } else {
        l = std::max(l,r->min);
        s = r->supports(n_words,l);
      }
    }
  }      
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::LostSupports::operator ++(void) {
    l++; s += n_words;
    if (pos) {
      while ((l <= h) && (l > r->max)) {
        r++; l=r->min; s=r->s;
      }
    } else {
      while ((l <= h) && (r <= lst) && (l > r->max)) {
        r++; l=r->min; s=r->s;
      }
      if (r > lst)
        l=h+1;
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
  forceinline
  Compact<View,pos>::Compact(Space& home, Compact& p)
    : Propagator(home,p), unassigned(p.unassigned),
      n_words(p.n_words), status(NONE), ts(p.ts) {
    c.update(home,p.c);
  }
  
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::Compact(Home home, ViewArray<View>& x,
                             const TupleSet& ts0)
    : Propagator(home), unassigned(x.size()),
      n_words(ts0.words()), status(MULTIPLE), ts(ts0), c(home) {
    home.notice(*this, AP_DISPOSE);
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
   * The propagator proper
   *
   */
  template<class View, class Table, bool pos>
  template<class TableProp>
  forceinline
  CompactTable<View,Table,pos>::CompactTable(Space& home, TableProp& p)
    : Compact<View,pos>(home,p), table(home,p.table) {
    assert(!table.empty());
  }

  template<class View, class Table, bool pos>
  Actor*
  CompactTable<View,Table,pos>::copy(Space& home) {
    assert((table.words() > 0U) && (table.width() >= table.words()));
    if (table.words() <= 4U) {
      switch (table.width()) {
      case 0U:
        GECODE_NEVER; break;
      case 1U:
        return new (home) CompactTable<View,TinyBitSet<1U>,pos>(home,*this);
      case 2U:
        return new (home) CompactTable<View,TinyBitSet<2U>,pos>(home,*this);
      case 3U:
        return new (home) CompactTable<View,TinyBitSet<3U>,pos>(home,*this);
      case 4U:
        return new (home) CompactTable<View,TinyBitSet<4U>,pos>(home,*this);
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
        return new (home) CompactTable<View,BitSet<unsigned int>,pos>(home,*this);
      default: GECODE_NEVER;
      }
      GECODE_NEVER;
      return nullptr;
    }
  copy_char:
    return new (home) CompactTable<View,BitSet<unsigned char>,pos>(home,*this);
  copy_short:
    return new (home) CompactTable<View,BitSet<unsigned short int>,pos>(home,*this);
  }

  template<class View, class Table, bool pos>
  forceinline
  CompactTable<View,Table,pos>::CompactTable(Home home, ViewArray<View>& x,
                                             const TupleSet& ts)
    : Compact<View,pos>(home,x,ts), table(home,ts.words()) {
    Region r;
    BitSetData* mask = r.alloc<BitSetData>(table.size());
    // Invalidate tuples
    //    std::cout << "Building table..." << std::endl;
    for (int i=0; i<x.size(); i++) {
      table.clear_mask(mask);
      for (ValidSupports vs(ts,i,x[i]); vs(); ++vs)
        table.add_to_mask(vs.supports(),mask);
      table.template intersect_with_mask<false>(mask);
      if (table.empty())
        return;
    }
    //    table.print();
    // Post advisors
    //    std::cout << "Post advisors..." << std::endl;
    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned())
        (void) new (home) CTAdvisor(home,*this,c,ts,x[i],i);
      else
        unassigned--;
    }
    // Schedule propagator
    //    std::cout << "Schedule propagator..., unassigned=" 
    //              << unassigned << std::endl;
    if (unassigned < x.size())
      View::schedule(home,*this,ME_INT_VAL);
    else
      View::schedule(home,*this,ME_INT_BND);
  }
      
  template<class View, class Table, bool pos>
  forceinline size_t
  CompactTable<View,Table,pos>::dispose(Space& home) {
    (void) Compact<View,pos>::dispose(home);
    return sizeof(*this);
  }

  template<class View, class Table, bool pos>
  PropCost
  CompactTable<View,Table,pos>::cost(const Space&, 
                                     const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::HI,unassigned);
  }

  template<class View, class Table, bool pos>
  void
  CompactTable<View,Table,pos>::reschedule(Space& home) {
    if (pos) {
      // Modified variable, subsumption, or failure
      if ((status.type() != StatusType::NONE) || 
          (unassigned == 0) || table.empty())
        View::schedule(home,*this,ME_INT_DOM);
    } else {
      View::schedule(home,*this,ME_INT_DOM);
    }
  }

  template<class View, class Table, bool pos>
  ExecStatus
  CompactTable<View,Table,pos>::propagate(Space& home, const ModEventDelta&) {
    if (pos) {
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
              goto noadjustp;
            } else {
              Iter::Values::Array rnq(nq,n_nq);
              GECODE_ASSUME(n_nq >= 2U);
              ModEvent me = x.minus_v(home,rnq,false);
              if (me_failed(me)) return ES_FAILED;
              assert(me != ME_INT_VAL);
            }
            a.adjust();
          noadjustp: ;
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

    } else {
      //      std::cout << "propagate()" << std::endl;
      //      table.print();
      if (table.empty() || (unassigned == 0))
        return home.ES_SUBSUMED(*this);

      Region r;
      // Size of the Cartesian product
      unsigned long long int xs = size();
      //      std::cout << "\t(2)xs=" << xs << std::endl;
      // Scan all values of all unassigned variables to see if they
      // are still supported.
      for (Advisors<CTAdvisor> as(c); as(); ++as) {
        CTAdvisor& a = as.advisor();
        View x = a.view();

        // How many values to remove
        int* nq = r.alloc<int>(x.size());
        unsigned int n_nq = 0U;

        // Adjust for the current variable domain
        xs /= x.size();

        ValidSupports vs(*this,a);
        if (!vs()) {
          // No valid supports left, subsumed
          return home.ES_SUBSUMED(*this);
        }

        for (; vs(); ++vs)
          if (table.ones(vs.supports()) == xs) {
            nq[n_nq++] = vs.val();
            table.nand_with_mask(vs.supports());
          }

        // Remove collected values
        if (n_nq > 0U) {
          if (n_nq == 1U) {
            ModEvent me = x.nq(home,nq[0]);
            if (me_failed(me))
              return ES_FAILED;
            //            if (me == ME_INT_VAL)
            //              unassigned--;
            //            else
            // Unassigned is not decremented here but in advise()
              xs *= x.size();
          } else {
            Iter::Values::Array rnq(nq,n_nq);
            GECODE_ASSUME(n_nq >= 2U);
            ModEvent me = x.minus_v(home,rnq,false);
            if (me_failed(me))
              return ES_FAILED;
            //            if (me == ME_INT_VAL)
            //              unassigned--;
            else
            // Unassigned is not decremented here but in advise()
              xs *= x.size();
          }
          a.adjust();
        }
        r.free();
      }

      if (table.empty() || (unassigned <= 1))
        return home.ES_SUBSUMED(*this);
      return ES_NOFIX;
    }
  }

  template<class View, class Table, bool pos>
  forceinline ExecStatus
  CompactTable<View,Table,pos>::post(Home home, ViewArray<View>& x,
                                     const TupleSet& ts) {
    auto ct = new (home) CompactTable(home,x,ts);
    if (pos) {
      assert((x.size() > 1) && (ts.tuples() > 1));
      return ct->table.empty() ? ES_FAILED : ES_OK;
    } else {
      return ES_OK;
    }
  }

  template<class View, class Table, bool pos>
  ExecStatus
  CompactTable<View,Table,pos>::advise(Space& home,
                                       Advisor& a0, const Delta& d) {
    CTAdvisor& a = static_cast<CTAdvisor&>(a0);

    if (pos) {
      // Do not fail a disabled propagator
      if (table.empty())
        return Compact<View,pos>::disabled() ?
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
            return Compact<View,pos>::disabled() ?
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
        return Compact<View,pos>::disabled() ?
          home.ES_NOFIX_DISPOSE(c,a) : ES_FAILED;
      
      // Schedule propagator
      return ES_NOFIX;
    } else {
      //      std::cout << "advise(" << a.view() << ")" << std::endl;
      //      table.print();

      // We are subsumed
      if (table.empty())
        return home.ES_NOFIX_DISPOSE(c,a);
      
      View x = a.view();
      
      if (x.assigned()) {
        //        std::cout << "\t(2) x.assigned()=" << x.val() << std::endl;
        unassigned--;
        // Variable is assigned -- intersect with its value
        const BitSetData* s = supports(a,x.val());
        if (!s) {
          table.flush();
          //          subsumed = true;
          //          std::cout << "\t(3) subsumed..." << std::endl;
        } else {
          table.template intersect_with_mask<true>(s);
        }
        //        std::cout << "\t(4) leave table..." << std::endl;
        //        table.print();
        return home.ES_NOFIX_DISPOSE(c,a);
      }
      
      if (!x.any(d) && (x.min(d) == x.max(d))) {
        const BitSetData* s = supports(a,x.min(d));
        if (s)
          table.nand_with_mask(s);
        a.adjust();
      } else {
        a.adjust();
        Region r;
        BitSetData* mask = r.alloc<BitSetData>(table.size());
        // Collect all tuples to be kept in a temporary mask
        table.clear_mask(mask);
        for (ValidSupports vs(*this,a); vs(); ++vs)
          table.add_to_mask(vs.supports(),mask);
        table.template intersect_with_mask<false>(mask);
      }

      if (table.empty())
        return home.ES_NOFIX_DISPOSE(c,a);
      
      // Schedule propagator
      return ES_NOFIX;
    }
  }


  /*
   * Post function
   */
  template<class View, bool pos>
  inline ExecStatus
  postcompact(Home home, ViewArray<View>& x, const TupleSet& ts) {

    if (pos) {
      // All variables pruned to correct domain
      for (int i=0; i<x.size(); i++) {
        TupleSet::Ranges r(ts,i);
        GECODE_ME_CHECK(x[i].inter_r(home, r, false));
      }

      if ((x.size() <= 1) || (ts.tuples() <= 1))
        return ES_OK;
    } else {
      if (ts.tuples() == 0)
        return ES_OK;
      // Check whether a variable does not overlap
      for (int i=0; i<x.size(); i++) {
        TupleSet::Ranges rs(ts,i);
        ViewRanges<View> rx(x[i]);
        if (Iter::Ranges::disjoint(rs,rx))
          return ES_OK;
      }
    }

    // Choose the right bit set implementation
    switch (ts.words()) {
    case 0U:
      GECODE_NEVER; return ES_OK;
    case 1U:
      return CompactTable<View,TinyBitSet<1U>,pos>::post(home,x,ts);
    case 2U:
      return CompactTable<View,TinyBitSet<2U>,pos>::post(home,x,ts);
    case 3U:
      return CompactTable<View,TinyBitSet<3U>,pos>::post(home,x,ts);
    case 4U:
      return CompactTable<View,TinyBitSet<4U>,pos>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return CompactTable<View,BitSet<unsigned char>,pos>
          ::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return CompactTable<View,BitSet<unsigned short int>,pos>
          ::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return CompactTable<View,BitSet<unsigned int>,pos>
          ::post(home,x,ts);
      default: GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}


// STATISTICS: int-prop
