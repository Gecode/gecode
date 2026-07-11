/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
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

#include <algorithm>
#include <type_traits>

namespace Gecode { namespace Int { namespace Extensional {

  /*
   * Shared compact-table advisor
   *
   */
  forceinline
  CompactAdvisorIndex<false>::CompactAdvisorIndex(int) {}

  forceinline int
  CompactAdvisorIndex<false>::index(void) const {
    return 0;
  }

  forceinline
  CompactAdvisorIndex<true>::CompactAdvisorIndex(int i)
    : _index(i) {}

  forceinline int
  CompactAdvisorIndex<true>::index(void) const {
    return _index;
  }

  template<class View, bool pos, bool indexed>
  forceinline void
  CompactAdvisor<View,pos,indexed>::adjust(void) {
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
      if (_fst > _lst)
        return;
      {
        int n = view().max();
        while ((_fst < _lst) && (n < _lst->min))
          _lst--;
        if (n < _lst->min)
          _fst = _lst + 1;
      }
    }
  }

  template<class View, bool pos, bool indexed>
  forceinline
  CompactAdvisor<View,pos,indexed>::CompactAdvisor
  (Space& home, Propagator& p, 
   Council<CompactAdvisor>& c, const TupleSet& ts, View x0, int i)
    : ViewAdvisor<View>(home,p,c,x0), CompactAdvisorIndex<indexed>(i),
      _fst(ts.fst(i)), _lst(ts.lst(i)) {
    adjust();
  }

  template<class View, bool pos, bool indexed>
  forceinline
  CompactAdvisor<View,pos,indexed>::CompactAdvisor
  (Space& home, CompactAdvisor& a)
    : ViewAdvisor<View>(home,a), CompactAdvisorIndex<indexed>(a.index()),
      _fst(a._fst), _lst(a._lst) {}

  template<class View, bool pos, bool indexed>
  forceinline int
  CompactAdvisor<View,pos,indexed>::index(void) const {
    return CompactAdvisorIndex<indexed>::index();
  }

  template<class View, bool pos, bool indexed>
  forceinline const typename CompactAdvisor<View,pos,indexed>::Range*
  CompactAdvisor<View,pos,indexed>::fst(void) const {
    return _fst;
  }

  template<class View, bool pos, bool indexed>
  forceinline const typename CompactAdvisor<View,pos,indexed>::Range*
  CompactAdvisor<View,pos,indexed>::lst(void) const {
    return _lst;
  }

  template<class View, bool pos, bool indexed>
  forceinline void
  CompactAdvisor<View,pos,indexed>::dispose
  (Space& home, Council<CompactAdvisor>& c) {
    (void) ViewAdvisor<View>::dispose(home,c);
  }


  /*
   * Shared positive propagator status
   *
   */
  template<class Advisor>
  forceinline
  CompactStatus<Advisor>::CompactStatus(StatusType t)
    : s(t) {}

  template<class Advisor>
  forceinline
  CompactStatus<Advisor>::CompactStatus(const CompactStatus& status)
    : s(status.s) {}

  template<class Advisor>
  forceinline typename CompactStatus<Advisor>::StatusType
  CompactStatus<Advisor>::type(void) const {
    return static_cast<StatusType>(s & 3);
  }

  template<class Advisor>
  forceinline bool
  CompactStatus<Advisor>::single(Advisor& a) const {
    return (type() == SINGLE) &&
      (reinterpret_cast<Advisor*>(s) == &a);
  }

  template<class Advisor>
  forceinline void
  CompactStatus<Advisor>::touched(Advisor& a) {
    if (type() == NONE) {
      s = reinterpret_cast<ptrdiff_t>(&a);
      assert((s & 3) == SINGLE);
    } else if ((type() == SINGLE) && !single(a)) {
      s = MULTIPLE;
    }
  }

  template<class Advisor>
  forceinline void
  CompactStatus<Advisor>::none(void) {
    s = NONE;
  }

  template<class Advisor>
  forceinline void
  CompactStatus<Advisor>::propagating(void) {
    s = PROPAGATING;
  }


  /*
   * Shared clone-time table selection
   *
   */
  template<template<class, class> class TablePropagator, class View>
  class CompactActorFactory {
  public:
    template<class Table, class Source>
    static Actor* copy(Space& home, Source& source) {
      return new (home) TablePropagator<View,Table>(home,source);
    }
  };

  template<template<class, class, class, ReifyMode> class TablePropagator,
           class View, class CtrlView, ReifyMode rm>
  class ReCompactActorFactory {
  public:
    template<class Table, class Source>
    static Actor* copy(Space& home, Source& source) {
      return new (home) TablePropagator<View,Table,CtrlView,rm>(home,source);
    }
  };

  template<class Factory, class Table, class Source>
  Actor*
  compact_copy(Space& home, Source& source) {
    // A disabled actor can become empty before it is cloned. Keep the
    // concrete table type: width-based selection requires an active word.
    if (source.table.empty())
      return Factory::template copy<Table>(home,source);

    const unsigned int words = source.table.words();
    const unsigned int width = source.table.width();
    assert((words > 0U) && (width >= words));

    if (words <= 4U) {
      switch (width) {
      case 0U: GECODE_NEVER; break;
      case 1U: return Factory::template copy<TinyBitSet<1U>>(home,source);
      case 2U: return Factory::template copy<TinyBitSet<2U>>(home,source);
      case 3U: return Factory::template copy<TinyBitSet<3U>>(home,source);
      case 4U: return Factory::template copy<TinyBitSet<4U>>(home,source);
      default: break;
      }
    }

    if (std::is_same<Table,BitSet<unsigned char>>::value)
      return Factory::template copy<BitSet<unsigned char>>(home,source);

    switch (Gecode::Support::u_type(width)) {
    case Gecode::Support::IT_CHAR:
      return Factory::template copy<BitSet<unsigned char>>(home,source);
    case Gecode::Support::IT_SHRT:
      return Factory::template copy<BitSet<unsigned short int>>(home,source);
    case Gecode::Support::IT_INT:
      if (std::is_same<Table,BitSet<unsigned int>>::value)
        return Factory::template copy<BitSet<unsigned int>>(home,source);
      GECODE_NEVER;
    default:
      GECODE_NEVER;
    }
    GECODE_NEVER;
    return nullptr;
  }

  template<class Advisor>
  forceinline bool
  compact_all(const Council<Advisor>& council) {
    Advisors<Advisor> advisors(council);
    return !advisors();
  }

  template<class Advisor>
  forceinline bool
  compact_atmostone(const Council<Advisor>& council) {
    Advisors<Advisor> advisors(council);
    if (!advisors())
      return true;
    ++advisors;
    return !advisors();
  }

  template<class View, class Advisor, class ValidSupports, class Table>
  void
  compact_setup(Space& home, Propagator& propagator,
                Council<Advisor>& council, const TupleSet& ts,
                Table& table, ViewArray<View>& x) {
    ModEvent me = ME_INT_BND;
    Region region;
    BitSetData* mask = region.alloc<BitSetData>(table.size());

    for (int i=0; i<x.size(); i++) {
      table.clear_mask(mask);
      for (ValidSupports supports(ts,i,x[i]); supports(); ++supports)
        table.add_to_mask(supports.support(),mask);
      table.template intersect_with_mask<false>(mask);
      if (table.empty())
        goto schedule;
    }

    for (int i=0; i<x.size(); i++) {
      if (!x[i].assigned())
        (void) new (home) Advisor(home,propagator,council,ts,x[i],i);
      else
        me = ME_INT_VAL;
    }

  schedule:
    View::schedule(home,propagator,me);
  }

  template<class Advisor, class Table>
  forceinline bool
  compact_full(const Council<Advisor>& council, const Table& table) {
    unsigned long long int size = 1U;
    for (Advisors<Advisor> advisors(council); advisors(); ++advisors) {
      size *= static_cast<unsigned long long int>
        (advisors.advisor().view().size());
      if (size > table.bits())
        return false;
    }
    return size == table.ones();
  }

  template<class Advisor>
  forceinline PropCost
  compact_cost(const Council<Advisor>& council) {
    int n = 0;
    // The value of 3 is cheating from the Gecode kernel...
    for (Advisors<Advisor> advisors(council);
         advisors() && (n <= 3); ++advisors)
      n++;
    return PropCost::quadratic(PropCost::HI,n);
  }

  template<class Support>
  class CompactSupportPolicy;

  template<>
  class CompactSupportPolicy<const BitSetData*> {
  public:
    static bool empty(const BitSetData* support) {
      return support == nullptr;
    }
    template<class Table>
    static void intersect(Table& table, const BitSetData* support) {
      table.template intersect_with_mask<true>(support);
    }
  };

  template<>
  class CompactSupportPolicy<CompressedSupport> {
  public:
    static bool empty(const CompressedSupport& support) {
      return support.empty();
    }
    template<class Table>
    static void intersect(Table& table, const CompressedSupport& support) {
      table.intersect_with_mask(support);
    }
  };

  template<class Support>
  forceinline bool
  compact_support_empty(const Support& support) {
    return CompactSupportPolicy<Support>::empty(support);
  }

  template<class Table, class Support>
  forceinline void
  compact_intersect(Table& table, const Support& support) {
    CompactSupportPolicy<Support>::intersect(table,support);
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
    if (!pos && (fst > lst))
      return nullptr;
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
    assert(value <= max_value);
    while (true) {
      while (view_ranges() && (value > view_ranges.max()))
        ++view_ranges;
      if (!view_ranges()) {
        value = max_value+1; return;
      }
      assert(value <= view_ranges.max());
      value = std::max(value,view_ranges.min());
      
      while ((support_range <= last_support_range) &&
             (value > support_range->max))
        support_range++;
      if (support_range > last_support_range) {
        value = max_value+1; return;
      }
      assert(value <= support_range->max);
      value = std::max(value,support_range->min);

      if ((view_ranges.min() <= value) && (value <= view_ranges.max())) {
        support_words = support_range->supports(n_words,value);
        return;
      }
    }
    GECODE_NEVER;
  }

  template<class View, bool pos>
  forceinline
  Compact<View,pos>::ValidSupports::ValidSupports(const Compact<View,pos>& p,
                                                  CTAdvisor& a)
    : n_words(p.n_words), max_value(a.view().max()),
      view_ranges(a.view()), support_range(a.fst()),
      last_support_range(a.lst()), value(view_ranges.min()) {
    if (pos) {
      while (value > support_range->max)
        support_range++;
      support_words = support_range->supports(n_words,value);
    } else {
      support_words = nullptr; // To avoid warnings
      find();
    }
  }
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::ValidSupports::ValidSupports(const TupleSet& ts,
                                                  int i, View x)
    : n_words(ts.words()), max_value(x.max()),
      view_ranges(x), support_range(ts.fst(i)),
      last_support_range(ts.lst(i)), value(view_ranges.min()) {
    if (pos) {
      while (value > support_range->max)
        support_range++;
      support_words = support_range->supports(n_words,value);
    } else {
      support_words = nullptr; // To avoid warnings
      find();
    }
  }
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::ValidSupports::operator ++(void) {
    value++;
    if (pos) {
      if (value <= view_ranges.max()) {
        assert(value <= support_range->max);
        support_words += n_words;
      } else if (value <= max_value) {
        while (value > view_ranges.max())
          ++view_ranges;
        value = view_ranges.min();
        while (value > support_range->max)
          support_range++;
        support_words = support_range->supports(n_words,value);
        assert((view_ranges.min() <= value) &&
               (value <= view_ranges.max()));
        assert((support_range->min <= value) &&
               (value <= support_range->max));
        assert(support_range->min <= view_ranges.min());
      }
    } else {
      if ((value <= support_range->max) &&
          (value <= view_ranges.max())) {
        support_words += n_words;
      } else if (value <= max_value) {
        find();
      }
    }
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::ValidSupports::operator ()(void) const {
    return value <= max_value;
  }
  template<class View, bool pos>
  forceinline const BitSetData*
  Compact<View,pos>::ValidSupports::support(void) const {
    assert(support_words == support_range->supports(n_words,value));
    return support_words;
  }
  template<class View, bool pos>
  forceinline int
  Compact<View,pos>::ValidSupports::val(void) const {
    return value;
  }

  /*
   * Lost supports iterator
   *
   */
  template<class View, bool pos>
  forceinline
  Compact<View,pos>::LostSupports::LostSupports
  (const Compact<View,pos>& p, CTAdvisor& a,
   int first_value, int last_value0)
    : n_words(p.n_words), support_range(a.fst()),
      last_support_range(a.lst()), value(first_value),
      last_value(last_value0) {
    assert(pos);
    // Move to first value for which there is support
    while (value > support_range->max)
      support_range++;
    value = std::max(value,support_range->min);
    support_words = support_range->supports(n_words,value);
  }      
  template<class View, bool pos>
  forceinline void
  Compact<View,pos>::LostSupports::operator ++(void) {
    value++;
    support_words += n_words;
    while ((value <= last_value) && (value > support_range->max)) {
      support_range++;
      value = support_range->min;
      support_words = support_range->s;
    }
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::LostSupports::operator ()(void) const {
    return value<=last_value;
  }
  template<class View, bool pos>
  forceinline const TupleSet::BitSetData*
  Compact<View,pos>::LostSupports::support(void) const {
    assert((value >= support_range->min) && (value <= support_range->max));
    assert(support_words == support_range->supports(n_words,value));
    return support_words;
  }

  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::all(void) const {
    return compact_all(c);
  }
  template<class View, bool pos>
  forceinline bool
  Compact<View,pos>::atmostone(void) const {
    return compact_atmostone(c);
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
    compact_setup<View,CTAdvisor,ValidSupports>
      (home,*this,c,ts,table,x);
  }

  template<class View, bool pos>
  template<class Table>
  forceinline bool
  Compact<View,pos>::full(const Table& table) const {
    return compact_full(c,table);
  }

  template<class View, bool pos>
  PropCost
  Compact<View,pos>::cost(const Space&, const ModEventDelta&) const {
    return compact_cost(c);
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

  template<class Actor>
  class PosCompactAlgorithm {
  public:
    static void reschedule(Actor& actor, Space& home) {
      if ((actor.status.type() != Actor::StatusType::NONE) ||
          actor.all() || actor.table.empty())
        Actor::ViewType::schedule(home,actor,ME_INT_DOM);
    }

    static ExecStatus propagate(Actor& actor, Space& home) {
      if (actor.table.empty())
        return ES_FAILED;
      if (actor.all())
        return home.ES_SUBSUMED(actor);

      typename Actor::Status touched(actor.status);
      actor.status.propagating();
      Region region;
      for (Advisors<typename Actor::CTAdvisor> advisors(actor.c);
           advisors(); ++advisors) {
        typename Actor::CTAdvisor& advisor = advisors.advisor();
        typename Actor::ViewType view = advisor.view();
        if (touched.single(advisor) || view.assigned())
          continue;

        if (view.size() == 2) {
          if (!actor.table.intersects(actor.supports(advisor,view.min())))
            GECODE_ME_CHECK(view.eq(home,view.max()));
          else if (!actor.table.intersects(actor.supports(advisor,view.max())))
            GECODE_ME_CHECK(view.eq(home,view.min()));
          if (!view.assigned())
            advisor.adjust();
        } else {
          int* remove = region.alloc<int>(view.size());
          unsigned int n_remove = 0U;
          int last_support = 0;
          for (typename Actor::ValidSupports supports(actor,advisor);
               supports(); ++supports)
            if (!actor.table.intersects(supports.support()))
              remove[n_remove++] = supports.val();
            else
              last_support = supports.val();
          if (n_remove > 0U) {
            if (n_remove == 1U) {
              GECODE_ME_CHECK(view.nq(home,remove[0]));
            } else if (n_remove == view.size() - 1U) {
              GECODE_ME_CHECK(view.eq(home,last_support));
              goto noadjust;
            } else {
              Iter::Values::Array values(remove,n_remove);
              GECODE_ASSUME(n_remove >= 2U);
              GECODE_ME_CHECK(view.minus_v(home,values,false));
            }
            advisor.adjust();
          noadjust: ;
          }
          region.free();
        }
      }
      actor.status.none();
      assert(!actor.table.empty());
      return actor.atmostone() ? home.ES_SUBSUMED(actor) : ES_FIX;
    }

    static ExecStatus advise(Actor& actor, Space& home, Advisor& advisor0,
                             const Delta& delta) {
      typename Actor::CTAdvisor& advisor =
        static_cast<typename Actor::CTAdvisor&>(advisor0);
      if (actor.table.empty())
        return actor.disabled() ?
          home.ES_NOFIX_DISPOSE(actor.c,advisor) : ES_FAILED;

      typename Actor::ViewType view = advisor.view();
      if (actor.status.type() == Actor::StatusType::PROPAGATING)
        return view.assigned() ?
          home.ES_FIX_DISPOSE(actor.c,advisor) : ES_FIX;
      actor.status.touched(advisor);

      if (view.assigned()) {
        const auto support = actor.supports(advisor,view.val());
        if (compact_support_empty(support))
          actor.table.flush();
        else
          compact_intersect(actor.table,support);
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      }

      if (!view.any(delta) && (view.min(delta) == view.max(delta))) {
        const auto support = actor.supports(advisor,view.min(delta));
        if (!compact_support_empty(support))
          actor.table.nand_with_mask(support);
        advisor.adjust();
      } else if (!view.any(delta) &&
                 (view.width(delta) <= view.size())) {
        for (typename Actor::LostSupports supports
               (actor,advisor,view.min(delta),view.max(delta));
             supports(); ++supports) {
          actor.table.nand_with_mask(supports.support());
          if (actor.table.empty())
            return actor.disabled() ?
              home.ES_NOFIX_DISPOSE(actor.c,advisor) : ES_FAILED;
        }
        advisor.adjust();
      } else {
        advisor.adjust();
        if (view.size() == 2) {
          const auto min_support = actor.supports(advisor,view.min());
          const auto max_support = actor.supports(advisor,view.max());
          const bool has_min = !compact_support_empty(min_support);
          const bool has_max = !compact_support_empty(max_support);
          if (has_min && has_max)
            actor.table.intersect_with_masks(min_support,max_support);
          else if (has_min)
            compact_intersect(actor.table,min_support);
          else if (has_max)
            compact_intersect(actor.table,max_support);
          else
            actor.table.flush();
        } else {
          Region region;
          BitSetData* mask = region.alloc<BitSetData>(actor.table.size());
          actor.table.clear_mask(mask);
          for (typename Actor::ValidSupports supports(actor,advisor);
               supports(); ++supports)
            actor.table.add_to_mask(supports.support(),mask);
          actor.table.template intersect_with_mask<false>(mask);
        }
      }

      if (actor.table.empty())
        return actor.disabled() ?
          home.ES_NOFIX_DISPOSE(actor.c,advisor) : ES_FAILED;
      return ES_NOFIX;
    }
  };

  template<class Actor>
  class NegCompactAlgorithm {
  public:
    static void reschedule(Actor& actor, Space& home) {
      Actor::ViewType::schedule(home,actor,ME_INT_DOM);
    }

    static ExecStatus propagate(Actor& actor, Space& home) {
#ifndef NDEBUG
      if (!actor.table.empty())
        for (Advisors<typename Actor::CTAdvisor> advisors(actor.c);
             advisors(); ++advisors) {
          typename Actor::ValidSupports supports(actor,advisors.advisor());
          assert(supports());
        }
#endif
      if (actor.table.empty())
        return home.ES_SUBSUMED(actor);

      unsigned long long product_without_largest = 1U;
      unsigned long long largest_domain = 1U;
      for (Advisors<typename Actor::CTAdvisor> advisors(actor.c);
           advisors(); ++advisors) {
        const unsigned long long size = advisors.advisor().view().size();
        if (size > largest_domain) {
          product_without_largest *= largest_domain;
          largest_domain = size;
        } else {
          product_without_largest *= size;
        }
        if (product_without_largest > actor.table.bits())
          return ES_FIX;
      }
      if (product_without_largest > actor.table.ones())
        return ES_FIX;

      unsigned long long product = product_without_largest * largest_domain;
      Region region;
      for (Advisors<typename Actor::CTAdvisor> advisors(actor.c);
           advisors(); ++advisors) {
        assert(!actor.table.empty());
        typename Actor::CTAdvisor& advisor = advisors.advisor();
        typename Actor::ViewType view = advisor.view();
        product /= static_cast<unsigned long long>(view.size());
        if ((product <= actor.table.bits()) &&
            (product <= actor.table.ones())) {
          int* remove = region.alloc<int>(view.size());
          unsigned int n_remove = 0U;
          for (typename Actor::ValidSupports supports(actor,advisor);
               supports(); ++supports)
            if (product == actor.table.ones(supports.support()))
              remove[n_remove++] = supports.val();
          if (n_remove > 0U) {
            if (n_remove == 1U) {
              GECODE_ME_CHECK(view.nq(home,remove[0]));
            } else {
              Iter::Values::Array values(remove,n_remove);
              GECODE_ASSUME(n_remove >= 2U);
              GECODE_ME_CHECK(view.minus_v(home,values,false));
            }
            if (actor.table.empty())
              return home.ES_SUBSUMED(actor);
            advisor.adjust();
          }
          region.free();
        }
        product *= static_cast<unsigned long long>(view.size());
      }

      if (actor.table.ones() == product)
        return ES_FAILED;
      if (actor.table.empty() || actor.atmostone())
        return home.ES_SUBSUMED(actor);
      return ES_FIX;
    }

    static ExecStatus advise(Actor& actor, Space& home, Advisor& advisor0) {
      typename Actor::CTAdvisor& advisor =
        static_cast<typename Actor::CTAdvisor&>(advisor0);
      if (actor.table.empty())
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);

      typename Actor::ViewType view = advisor.view();
      advisor.adjust();
      if (view.assigned()) {
        const auto support = actor.supports(advisor,view.val());
        if (compact_support_empty(support))
          actor.table.flush();
        else
          compact_intersect(actor.table,support);
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      }

      typename Actor::ValidSupports supports(actor,advisor);
      if (!supports()) {
        actor.table.flush();
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      }
      Region region;
      BitSetData* mask = region.alloc<BitSetData>(actor.table.size());
      actor.table.clear_mask(mask);
      do {
        actor.table.add_to_mask(supports.support(),mask);
        ++supports;
      } while (supports());
      actor.table.template intersect_with_mask<false>(mask);

      if (actor.table.empty())
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      return ES_NOFIX;
    }
  };

  template<class Actor>
  class ReCompactAlgorithm {
  public:
    static void reschedule(Actor& actor, Space& home) {
      Actor::ViewType::schedule(home,actor,ME_INT_DOM);
    }

    static ExecStatus propagate(Actor& actor, Space& home) {
      if (actor.b.one()) {
        if (Actor::mode == RM_PMI)
          return home.ES_SUBSUMED(actor);
        TupleSet keep(actor.ts);
        GECODE_REWRITE(actor,Actor::post_pos(home(actor),actor.y,keep));
      }
      if (actor.b.zero()) {
        if (Actor::mode == RM_IMP)
          return home.ES_SUBSUMED(actor);
        TupleSet keep(actor.ts);
        GECODE_REWRITE(actor,Actor::post_neg(home(actor),actor.y,keep));
      }

      if (actor.table.empty()) {
        if (Actor::mode != RM_PMI)
          GECODE_ME_CHECK(actor.b.zero_none(home));
        return home.ES_SUBSUMED(actor);
      }
      if (actor.full(actor.table)) {
        if (Actor::mode != RM_IMP)
          GECODE_ME_CHECK(actor.b.one_none(home));
        return home.ES_SUBSUMED(actor);
      }
      return ES_FIX;
    }

    static ExecStatus advise(Actor& actor, Space& home, Advisor& advisor0) {
      typename Actor::CTAdvisor& advisor =
        static_cast<typename Actor::CTAdvisor&>(advisor0);
      if (actor.table.empty() || actor.b.assigned())
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);

      typename Actor::ViewType view = advisor.view();
      advisor.adjust();
      if (view.assigned()) {
        const auto support = actor.supports(advisor,view.val());
        if (compact_support_empty(support))
          actor.table.flush();
        else
          compact_intersect(actor.table,support);
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      }

      typename Actor::ValidSupports supports(actor,advisor);
      if (!supports()) {
        actor.table.flush();
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      }
      Region region;
      BitSetData* mask = region.alloc<BitSetData>(actor.table.size());
      actor.table.clear_mask(mask);
      do {
        actor.table.add_to_mask(supports.support(),mask);
        ++supports;
      } while (supports());
      actor.table.template intersect_with_mask<false>(mask);

      if (actor.table.empty())
        return home.ES_NOFIX_DISPOSE(actor.c,advisor);
      return ES_NOFIX;
    }
  };


  /*
   * The propagator proper
   *
   */
  template<class View, class Table>
  template<class TableProp>
  forceinline
  PosCompact<View,Table>::PosCompact(Space& home, TableProp& p)
    : Compact<View,true>(home,p),
      status(p.status.type() == StatusType::NONE ?
             Status::NONE : Status::MULTIPLE),
      table(home,p.table) {}

  template<class View, class Table>
  Actor*
  PosCompact<View,Table>::copy(Space& home) {
    typedef CompactActorFactory<PosCompact,View> Factory;
    return compact_copy<Factory,Table>(home,*this);
  }

  template<class View, class Table>
  forceinline
  PosCompact<View,Table>::PosCompact(Home home, ViewArray<View>& x,
                                     const TupleSet& ts)
    : Compact<View,true>(home,ts), status(Status::MULTIPLE),
      table(home,ts.words()) {
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
    PosCompactAlgorithm<PosCompact>::reschedule(*this,home);
  }

  template<class View, class Table>
  ExecStatus
  PosCompact<View,Table>::propagate(Space& home, const ModEventDelta&) {
    return PosCompactAlgorithm<PosCompact>::propagate(*this,home);
  }

  template<class View, class Table>
  ExecStatus
  PosCompact<View,Table>::advise(Space& home, Advisor& advisor,
                                 const Delta& delta) {
    return PosCompactAlgorithm<PosCompact>::advise
      (*this,home,advisor,delta);
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
    : Compact<View,false>(home,p), table(home,p.table) {}

  template<class View, class Table>
  Actor*
  NegCompact<View,Table>::copy(Space& home) {
    typedef CompactActorFactory<NegCompact,View> Factory;
    return compact_copy<Factory,Table>(home,*this);
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
    NegCompactAlgorithm<NegCompact>::reschedule(*this,home);
  }

  template<class View, class Table>
  ExecStatus
  NegCompact<View,Table>::propagate(Space& home, const ModEventDelta&) {
    return NegCompactAlgorithm<NegCompact>::propagate(*this,home);
  }

  template<class View, class Table>
  ExecStatus
  NegCompact<View,Table>::advise(Space& home, Advisor& advisor,
                                 const Delta&) {
    return NegCompactAlgorithm<NegCompact>::advise(*this,home,advisor);
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
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  Actor*
  ReCompact<View,Table,CtrlView,rm>::copy(Space& home) {
    typedef ReCompactActorFactory<ReCompact,View,CtrlView,rm> Factory;
    return compact_copy<Factory,Table>(home,*this);
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
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::post_pos
  (Home home, ViewArray<View>& x, const TupleSet& ts) {
    return postposcompact(home,x,ts);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::post_neg
  (Home home, ViewArray<View>& x, const TupleSet& ts) {
    return postnegcompact(home,x,ts);
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
    ReCompactAlgorithm<ReCompact>::reschedule(*this,home);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::propagate
  (Space& home, const ModEventDelta&) {
    return ReCompactAlgorithm<ReCompact>::propagate(*this,home);
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  ExecStatus
  ReCompact<View,Table,CtrlView,rm>::advise
  (Space& home, Advisor& advisor, const Delta&) {
    return ReCompactAlgorithm<ReCompact>::advise(*this,home,advisor);
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

  /*
   * Compact table with compressed supports
   *
   */
  template<class View, bool pos>
  class CompactCompressed : public Propagator {
  protected:
    typedef TupleSet::Range Range;
    typedef TupleSet::CSupportWord CSupportWord;
    typedef CompactAdvisor<View,pos,true> CTAdvisor;

    class ValidSupports {
    protected:
      const TupleSet& tuple_set;
      ViewRanges<View> view_ranges;
      int variable;
      const Range* support_range;
      const Range* last_support_range;
      int value;
      const CSupportWord* support_begin;
      const CSupportWord* support_end;
      bool valid;
      void find(void) {
        while (view_ranges() && (support_range <= last_support_range)) {
          if (value < view_ranges.min())
            value = view_ranges.min();
          if (value > view_ranges.max()) {
            ++view_ranges;
            if (view_ranges())
              value = view_ranges.min();
            continue;
          }
          while ((support_range <= last_support_range) &&
                 (value > support_range->max))
            support_range++;
          if (support_range > last_support_range)
            break;
          if (value < support_range->min) {
            value = support_range->min;
            continue;
          }
          if (value > view_ranges.max()) {
            ++view_ranges;
            if (view_ranges())
              value = view_ranges.min();
            continue;
          }
          if (TupleSetAccess::dense_compressed_support(tuple_set,variable,
                                                       value,support_begin,
                                                       support_end)) {
            valid = true;
            return;
          }
          value++;
        }
        valid = false;
      }
    public:
      ValidSupports(const CompactCompressed<View,pos>& p, CTAdvisor& a)
        : tuple_set(p.ts), view_ranges(a.view()), variable(a.index()),
          support_range(a.fst()), last_support_range(a.lst()), value(0),
          support_begin(nullptr), support_end(nullptr), valid(false) {
        if (view_ranges()) {
          value = view_ranges.min();
          find();
        }
      }
      ValidSupports(const TupleSet& ts0, int i0, View x)
        : tuple_set(ts0), view_ranges(x), variable(i0),
          support_range(ts0.fst(i0)), last_support_range(ts0.lst(i0)),
          value(0), support_begin(nullptr), support_end(nullptr),
          valid(false) {
        if (view_ranges()) {
          value = view_ranges.min();
          find();
        }
      }
      void operator ++(void) {
        value++;
        find();
      }
      bool operator ()(void) const {
        return valid;
      }
      int val(void) const {
        return value;
      }
      CompressedSupport support(void) const {
        return CompressedSupport(support_begin,support_end);
      }
    };

    class LostSupports {
    protected:
      const CompactCompressed<View,pos>& propagator;
      CTAdvisor& advisor;
      const Range* support_range;
      const Range* last_support_range;
      int value;
      int last_value;
      const CSupportWord* support_begin;
      const CSupportWord* support_end;
      bool valid;
      void find(void) {
        while ((value <= last_value) &&
               (support_range <= last_support_range)) {
          if (value < support_range->min)
            value = support_range->min;
          if (value > last_value)
            break;
          while ((support_range <= last_support_range) &&
                 (value > support_range->max))
            support_range++;
          if (support_range > last_support_range)
            break;
          if (value < support_range->min)
            continue;
          if (TupleSetAccess::dense_compressed_support(propagator.ts,
                                                       advisor.index(),value,
                                                       support_begin,
                                                       support_end)) {
            valid = true;
            return;
          }
          value++;
        }
        valid = false;
      }
    public:
      LostSupports(const CompactCompressed<View,pos>& p0, CTAdvisor& a0,
                   int first_value, int last_value0)
        : propagator(p0), advisor(a0), support_range(a0.fst()),
          last_support_range(a0.lst()), value(first_value),
          last_value(last_value0),
          support_begin(nullptr), support_end(nullptr), valid(false) {
        assert(pos);
        find();
      }
      void operator ++(void) {
        value++;
        find();
      }
      bool operator ()(void) const {
        return valid;
      }
      CompressedSupport support(void) const {
        return CompressedSupport(support_begin,support_end);
      }
    };

    bool all(void) const {
      return compact_all(c);
    }
    bool atmostone(void) const {
      return compact_atmostone(c);
    }

  protected:
    const unsigned int n_words;
    TupleSet ts;
    Council<CTAdvisor> c;

    CompactCompressed(Space& home, CompactCompressed& p)
      : Propagator(home,p), n_words(p.n_words), ts(p.ts), c(home) {
      c.update(home,p.c);
    }
    CompactCompressed(Home home, const TupleSet& ts0)
      : Propagator(home), n_words(ts0.words()), ts(ts0), c(home) {
      home.notice(*this, AP_DISPOSE);
    }

    CompressedSupport supports(CTAdvisor& a, int n) const {
      const CSupportWord* begin = nullptr;
      const CSupportWord* end = nullptr;
      if (!TupleSetAccess::dense_compressed_support
          (ts,a.index(),n,begin,end))
        return CompressedSupport();
      return CompressedSupport(begin,end);
    }

    template<class Table>
    void setup(Space& home, Table& table, ViewArray<View>& x) {
      compact_setup<View,CTAdvisor,ValidSupports>
        (home,*this,c,ts,table,x);
    }

    template<class Table>
    bool full(const Table& table) const {
      return compact_full(c,table);
    }

  public:
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return compact_cost(c);
    }
    size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      c.dispose(home);
      ts.~TupleSet();
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  template<class View, class Table>
  class PosCompactCompressed : public CompactCompressed<View,true> {
    template<class Actor> friend class PosCompactAlgorithm;
  public:
    typedef View ViewType;
    typedef CompactCompressed<View,true> Base;
    typedef typename Base::ValidSupports ValidSupports;
    typedef typename Base::CTAdvisor CTAdvisor;
    typedef typename Base::LostSupports LostSupports;

    using Base::setup;
    using Base::supports;
    using Base::all;
    using Base::atmostone;
    using Base::c;
    using Base::ts;

    typedef CompactStatus<CTAdvisor> Status;
    typedef typename Status::StatusType StatusType;

    Status status;
    Table table;

    template<class TableProp>
    PosCompactCompressed(Space& home, TableProp& p)
      : Base(home,p),
        status(p.status.type() == StatusType::NONE ?
               Status::NONE : Status::MULTIPLE),
        table(home,p.table) {}
    PosCompactCompressed(Home home, ViewArray<View>& x, const TupleSet& ts)
      : Base(home,ts), status(Status::MULTIPLE),
        table(home,ts.words(),true) {
      setup(home,table,x);
    }

    virtual Actor* copy(Space& home) {
      typedef CompactActorFactory<PosCompactCompressed,View> Factory;
      return compact_copy<Factory,Table>(home,*this);
    }

    static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& ts) {
      auto ct = new (home) PosCompactCompressed(home,x,ts);
      assert((x.size() > 1) && (ts.tuples() > 1));
      return ct->table.empty() ? ES_FAILED : ES_OK;
    }

    virtual size_t dispose(Space& home) {
      (void) Base::dispose(home);
      return sizeof(*this);
    }

    virtual void reschedule(Space& home) {
      PosCompactAlgorithm<PosCompactCompressed>::reschedule(*this,home);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      return PosCompactAlgorithm<PosCompactCompressed>::propagate(*this,home);
    }

    virtual ExecStatus advise(Space& home, Advisor& advisor,
                              const Delta& delta) {
      return PosCompactAlgorithm<PosCompactCompressed>::advise
        (*this,home,advisor,delta);
    }
  };

  template<class View>
  ExecStatus
  postposcompact_compressed(Home home, ViewArray<View>& x, const TupleSet& ts) {
    if (ts.tuples() == 0)
      return (x.size() == 0) ? ES_OK : ES_FAILED;

    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges r(ts,i);
      GECODE_ME_CHECK(x[i].inter_r(home, r, false));
    }
    if ((x.size() <= 1) || (ts.tuples() <= 1))
      return ES_OK;

    switch (ts.words()) {
    case 0U: GECODE_NEVER; return ES_OK;
    case 1U: return PosCompactCompressed<View,TinyBitSet<1U>>::post(home,x,ts);
    case 2U: return PosCompactCompressed<View,TinyBitSet<2U>>::post(home,x,ts);
    case 3U: return PosCompactCompressed<View,TinyBitSet<3U>>::post(home,x,ts);
    case 4U: return PosCompactCompressed<View,TinyBitSet<4U>>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return PosCompactCompressed<View,BitSet<unsigned char>>::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return PosCompactCompressed<View,BitSet<unsigned short int>>::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return PosCompactCompressed<View,BitSet<unsigned int>>::post(home,x,ts);
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

  template<class View, class Table>
  class NegCompactCompressed : public CompactCompressed<View,false> {
    template<class Actor> friend class NegCompactAlgorithm;
  public:
    typedef View ViewType;
    typedef CompactCompressed<View,false> Base;
    typedef typename Base::ValidSupports ValidSupports;
    typedef typename Base::CTAdvisor CTAdvisor;

    using Base::setup;
    using Base::full;
    using Base::supports;
    using Base::atmostone;
    using Base::c;
    using Base::ts;

    Table table;

    template<class TableProp>
    NegCompactCompressed(Space& home, TableProp& p)
      : Base(home,p), table(home,p.table) {}
    NegCompactCompressed(Home home, ViewArray<View>& x, const TupleSet& ts)
      : Base(home,ts), table(home,ts.words(),true) {
      setup(home,table,x);
    }

    virtual Actor* copy(Space& home) {
      typedef CompactActorFactory<NegCompactCompressed,View> Factory;
      return compact_copy<Factory,Table>(home,*this);
    }

    static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& ts) {
      auto ct = new (home) NegCompactCompressed(home,x,ts);
      return ct->full(ct->table) ? ES_FAILED : ES_OK;
    }

    virtual size_t dispose(Space& home) {
      (void) Base::dispose(home);
      return sizeof(*this);
    }

    virtual void reschedule(Space& home) {
      NegCompactAlgorithm<NegCompactCompressed>::reschedule(*this,home);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      return NegCompactAlgorithm<NegCompactCompressed>::propagate(*this,home);
    }

    virtual ExecStatus advise(Space& home, Advisor& advisor, const Delta&) {
      return NegCompactAlgorithm<NegCompactCompressed>::advise
        (*this,home,advisor);
    }
  };

  template<class View>
  ExecStatus
  postnegcompact_compressed(Home home, ViewArray<View>& x, const TupleSet& ts) {
    if (ts.tuples() == 0)
      return ES_OK;

    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges rs(ts,i);
      ViewRanges<View> rx(x[i]);
      if (Iter::Ranges::disjoint(rs,rx))
        return ES_OK;
    }

    switch (ts.words()) {
    case 0U: GECODE_NEVER; return ES_OK;
    case 1U: return NegCompactCompressed<View,TinyBitSet<1U>>::post(home,x,ts);
    case 2U: return NegCompactCompressed<View,TinyBitSet<2U>>::post(home,x,ts);
    case 3U: return NegCompactCompressed<View,TinyBitSet<3U>>::post(home,x,ts);
    case 4U: return NegCompactCompressed<View,TinyBitSet<4U>>::post(home,x,ts);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return NegCompactCompressed<View,BitSet<unsigned char>>::post(home,x,ts);
      case Gecode::Support::IT_SHRT:
        return NegCompactCompressed<View,BitSet<unsigned short int>>::post(home,x,ts);
      case Gecode::Support::IT_INT:
        return NegCompactCompressed<View,BitSet<unsigned int>>::post(home,x,ts);
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

  template<class View, class Table, class CtrlView, ReifyMode rm>
  class ReCompactCompressed : public CompactCompressed<View,false> {
    template<class Actor> friend class ReCompactAlgorithm;
  public:
    typedef View ViewType;
    static constexpr ReifyMode mode = rm;
    typedef CompactCompressed<View,false> Base;
    typedef typename Base::ValidSupports ValidSupports;
    typedef typename Base::CTAdvisor CTAdvisor;

    using Base::setup;
    using Base::full;
    using Base::supports;
    using Base::c;
    using Base::ts;

    Table table;
    CtrlView b;
    ViewArray<View> y;

    template<class TableProp>
    ReCompactCompressed(Space& home, TableProp& p)
      : Base(home,p), table(home,p.table) {
      b.update(home,p.b);
      y.update(home,p.y);
    }
    ReCompactCompressed(Home home, ViewArray<View>& x, const TupleSet& ts,
                        CtrlView b0)
      : Base(home,ts), table(home,ts.words(),true), b(b0), y(x) {
      b.subscribe(home,*this,PC_BOOL_VAL);
      setup(home,table,x);
    }

    virtual Actor* copy(Space& home) {
      typedef ReCompactActorFactory
        <ReCompactCompressed,View,CtrlView,rm> Factory;
      return compact_copy<Factory,Table>(home,*this);
    }

    static ExecStatus post(Home home, ViewArray<View>& x, const TupleSet& ts,
                           CtrlView b) {
      if (b.one()) {
        if (rm == RM_PMI)
          return ES_OK;
        return postposcompact_compressed(home,x,ts);
      }
      if (b.zero()) {
        if (rm == RM_IMP)
          return ES_OK;
        return postnegcompact_compressed(home,x,ts);
      }
      (void) new (home) ReCompactCompressed(home,x,ts,b);
      return ES_OK;
    }

    static ExecStatus post_pos(Home home, ViewArray<View>& x,
                               const TupleSet& ts) {
      return postposcompact_compressed(home,x,ts);
    }

    static ExecStatus post_neg(Home home, ViewArray<View>& x,
                               const TupleSet& ts) {
      return postnegcompact_compressed(home,x,ts);
    }

    virtual size_t dispose(Space& home) {
      b.cancel(home,*this,PC_BOOL_VAL);
      (void) Base::dispose(home);
      return sizeof(*this);
    }

    virtual void reschedule(Space& home) {
      ReCompactAlgorithm<ReCompactCompressed>::reschedule(*this,home);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      return ReCompactAlgorithm<ReCompactCompressed>::propagate(*this,home);
    }

    virtual ExecStatus advise(Space& home, Advisor& advisor, const Delta&) {
      return ReCompactAlgorithm<ReCompactCompressed>::advise
        (*this,home,advisor);
    }
  };

  template<class View, class CtrlView, ReifyMode rm>
  ExecStatus
  postrecompact_compressed(Home home, ViewArray<View>& x, const TupleSet& ts,
                           CtrlView b) {
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
    for (int i=0; i<x.size(); i++) {
      TupleSet::Ranges rs(ts,i);
      ViewRanges<View> rx(x[i]);
      if (Iter::Ranges::disjoint(rs,rx)) {
        if (rm != RM_PMI)
          GECODE_ME_CHECK(b.zero(home));
        return ES_OK;
      }
    }

    switch (ts.words()) {
    case 0U: GECODE_NEVER; return ES_OK;
    case 1U:
      return ReCompactCompressed<View,TinyBitSet<1U>,CtrlView,rm>::post(home,x,ts,b);
    case 2U:
      return ReCompactCompressed<View,TinyBitSet<2U>,CtrlView,rm>::post(home,x,ts,b);
    case 3U:
      return ReCompactCompressed<View,TinyBitSet<3U>,CtrlView,rm>::post(home,x,ts,b);
    case 4U:
      return ReCompactCompressed<View,TinyBitSet<4U>,CtrlView,rm>::post(home,x,ts,b);
    default:
      switch (Gecode::Support::u_type(ts.words())) {
      case Gecode::Support::IT_CHAR:
        return ReCompactCompressed<View,BitSet<unsigned char>,CtrlView,rm>
          ::post(home,x,ts,b);
      case Gecode::Support::IT_SHRT:
        return ReCompactCompressed<View,BitSet<unsigned short int>,CtrlView,rm>
          ::post(home,x,ts,b);
      case Gecode::Support::IT_INT:
        return ReCompactCompressed<View,BitSet<unsigned int>,CtrlView,rm>
          ::post(home,x,ts,b);
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}

// STATISTICS: int-prop
