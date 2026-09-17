/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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
 */

#ifndef GECODE_WORD_ARITHMETIC_GLOBAL_PROPAGATOR_HPP
#define GECODE_WORD_ARITHMETIC_GLOBAL_PROPAGATOR_HPP

#include <gecode/word/arithmetic/global-filter.hpp>

namespace Gecode { namespace Word { namespace Arithmetic { namespace Global {
  class Data : public SharedHandle {
    class Object : public SharedHandle::Object {
    public:
      const Specification spec;
      explicit Object(const Specification& s) : spec(s) {}
    };
  public:
    explicit Data(const Specification& s) : SharedHandle(new Object(s)) {}
    const Specification& get(void) const {
      return static_cast<const Object*>(object())->spec;
    }
  };

  template<Family family>
  class Global : public Propagator {
    ViewArray<WordView> x;
    Data data;
    bool advanced_stage;
    bool basic_pending;
    Global(Home home,ViewArray<WordView>& views,const Data& d)
      : Propagator(home),x(views),data(d),advanced_stage(false),basic_pending(false) {
      x.subscribe(home,*this,PC_WORD_DOM);
      home.notice(*this,AP_DISPOSE);
    }
    Global(Space& home,Global& p)
      : Propagator(home,p),data(p.data),advanced_stage(p.advanced_stage),
        basic_pending(p.basic_pending) {
      x.update(home,p.x);
    }
  public:
    static void post(Home home,ViewArray<WordView>& x,const Data& d) {
      (void) new (home) Global(home,x,d);
    }
    virtual Actor* copy(Space& home) { return new (home) Global(home,*this); }
    virtual PropCost cost(const Space&,const ModEventDelta&) const {
      // Account for coefficient/factor occurrences, not just unique views.
      // The nontrivial word arithmetic has a fixed minimum workload of four.
      return advanced_stage ? PropCost::quadratic(PropCost::HI,data.get().work) :
        PropCost::linear(PropCost::LO,data.get().work);
    }
    virtual void reschedule(Space& home) { x.reschedule(home,*this,PC_WORD_DOM); }
    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE); x.cancel(home,*this,PC_WORD_DOM);
      data.~Data(); (void) Propagator::dispose(home); return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home,const ModEventDelta&) {
      const auto& s=data.get();
      Domains domains; bool assigned=true;
      std::vector<Integer> values;
      domains.reserve(x.size()); values.reserve(x.size());
      for (int i=0;i<x.size();i++) {
        domains.emplace_back(x[i],s.signed_values[i]); assigned&=x[i].assigned();
        values.push_back(domains.back().lower);
      }
      // Check exact satisfaction and subsume every fully assigned actor.
      if (assigned) return satisfies(s,values) ? home.ES_SUBSUMED(*this) : ES_FAILED;
      bool ok=family==PRODUCT ? product(s,domains) :
        family==DIVMOD ? divmod(s,domains,advanced_stage) : linear(s,domains,advanced_stage);
      if (!ok) return ES_FAILED;
      bool changed=false, assigned_after=true;
      for (int i=0;i<x.size();i++) {
        const Domain& d=domains[i];
        ModEvent me=d.publish(home,x[i]);
        if (me_failed(me)) return ES_FAILED;
        changed|=me_modified(me);
        assigned_after&=x[i].assigned();
      }
      // Publication can complete the tuple.  Check the exact relation now,
      // rather than retaining an assigned propagator for another scheduling
      // round (or relying on the filtering rules to imply satisfaction).
      if (assigned_after) {
        values.clear();
        for (int i=0;i<x.size();i++)
          values.push_back(domains[i].number(x[i].val()));
        return satisfies(s,values) ? home.ES_SUBSUMED(*this) : ES_FAILED;
      }
      bool has_advanced=s.advanced &&
        (family==LINEAR || family==RADIX || family==IMAGE || (family==DIVMOD && s.weighted));
      if (has_advanced && !advanced_stage) {
        // Yield after one basic pass even when it prunes: arithmetic bounds
        // can otherwise take exponentially many passes before advanced runs.
        basic_pending=changed;
        advanced_stage=true;
        return changed ? home.ES_NOFIX_PARTIAL(*this,WordView::med(ME_WORD_DOM)) :
          home.ES_FIX_PARTIAL(*this,WordView::med(ME_WORD_DOM));
      }
      if (advanced_stage) {
        advanced_stage=false;
        // ES_NOFIX alone retains the current queue. Explicitly enqueue at the
        // basic cost when advanced deductions require another arithmetic pass.
        bool again=changed || basic_pending;
        basic_pending=false;
        return again ? home.ES_NOFIX_PARTIAL(*this,WordView::med(ME_WORD_DOM)) : ES_FIX;
      }
      return changed ? ES_NOFIX : ES_FIX;
    }
  };

}}}}

#endif

// STATISTICS: word-prop
