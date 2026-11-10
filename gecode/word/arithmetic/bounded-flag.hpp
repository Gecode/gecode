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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_FLAG_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_FLAG_HPP

#include <gecode/word/arithmetic/bounded-domain.hpp>
#include <gecode/word/arithmetic/bounded-terminal.hpp>
#include <gecode/word/arithmetic/bounded-binary.hpp>

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Staged carry or borrow filtering with terminal-actor rewriting.
  template<class View, BoundArithmeticOperation op>
  class BoundFlagArithmetic : public Propagator {
  public:
    static bool numeric_regime(View x, View y, Int::BoolView flag) {
      if (flag.assigned()) return true;
      const WordValue mask=width_mask(x.width());
      return (op == BoundArithmeticOperation::ADD) ?
        ((x.rank_minimum() > mask-y.rank_minimum()) ||
         (x.rank_maximum() <= mask-y.rank_maximum())) :
        ((x.rank_maximum() < y.rank_minimum()) ||
         (x.rank_minimum() >= y.rank_maximum()));
    }
    virtual Actor* copy(Space& home) {
      return new (home) BoundFlagArithmetic(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta& med) const {
      return (View::me(med) == ME_WORD_BND) ?
        PropCost::ternary(PropCost::LO) :
        PropCost::linear(PropCost::HI,x0.width());
    }
    virtual void reschedule(Space& home) {
      x0.reschedule(home,*this,PC_WORD_DOM);
      x1.reschedule(home,*this,PC_WORD_DOM);
      x2.reschedule(home,*this,PC_WORD_DOM);
      flag.reschedule(home,*this,Int::PC_BOOL_VAL);
    }
    virtual size_t dispose(Space& home) {
      x0.cancel(home,*this,PC_WORD_DOM);
      x1.cancel(home,*this,PC_WORD_DOM);
      x2.cancel(home,*this,PC_WORD_DOM);
      flag.cancel(home,*this,Int::PC_BOOL_VAL);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
    virtual ExecStatus propagate(Space& home, const ModEventDelta& med) {
      if (View::me(med) == ME_WORD_BND) {
        const BoundFilterResult bounds=narrow_bounds(home,x0,x1,x2,flag);
        GECODE_ES_CHECK(bounds.status);
        const bool is_assigned=x0.assigned() && x1.assigned() && x2.assigned() &&
          flag.assigned();
        if (is_assigned)
          return home.ES_SUBSUMED(*this);
        if (flag.zero())
          GECODE_REWRITE(*this,(BoundArithmetic<View,op,terminal_clear>::post(
            home(*this),x0,x1,x2)));
        if (flag.one())
          GECODE_REWRITE(*this,(BoundArithmetic<View,op,terminal_set>::post(
            home(*this),x0,x1,x2)));
        if (bounds.needs_cube)
          return home.ES_NOFIX_PARTIAL(*this,View::med(ME_WORD_BITS));
        return ES_FIX;
      }
      const ExecStatus status=narrow(home,x0,x1,x2,flag);
      GECODE_ES_CHECK(status);
      const bool is_assigned=
        x0.assigned() && x1.assigned() && x2.assigned() && flag.assigned();
      if (is_assigned)
        return home.ES_SUBSUMED(*this);
      if (flag.zero())
        GECODE_REWRITE(*this,(BoundArithmetic<View,op,terminal_clear>::post(
          home(*this),x0,x1,x2)));
      if (flag.one())
        GECODE_REWRITE(*this,(BoundArithmetic<View,op,terminal_set>::post(
          home(*this),x0,x1,x2)));
      return status;
    }
    static ExecStatus post(Home home, View x, View y, View z,
                           Int::BoolView flag) {
      const ExecStatus status=narrow(home,x,y,z,flag);
      GECODE_ES_CHECK(status);
      const bool is_assigned=
        x.assigned() && y.assigned() && z.assigned() && flag.assigned();
      if (!is_assigned)
        schedule_unless_fixpoint(
          home,*new (home) BoundFlagArithmetic(home,x,y,z,flag),status);
      return ES_OK;
    }
  protected:
    View x0;
    View x1;
    View x2;
    Int::BoolView flag;
    BoundFlagArithmetic(Home home, View x, View y, View z, Int::BoolView b)
      : Propagator(home), x0(x), x1(y), x2(z), flag(b) {
      x0.subscribe(home,*this,PC_WORD_DOM);
      x1.subscribe(home,*this,PC_WORD_DOM);
      x2.subscribe(home,*this,PC_WORD_DOM);
      flag.subscribe(home,*this,Int::PC_BOOL_VAL);
    }
    BoundFlagArithmetic(Space& home, BoundFlagArithmetic& p)
      : Propagator(home,p) {
      x0.update(home,p.x0); x1.update(home,p.x1); x2.update(home,p.x2);
      flag.update(home,p.flag);
    }
    /// Intersect permitted terminal values, then narrow their numeric relation.
    static TerminalNarrowResult narrow_ranges(BoundLocalDomain& x,
                                             BoundLocalDomain& y,
                                             BoundLocalDomain& z,
                                             unsigned int terminal) {
      const WordValue mask=width_mask(x.width);
      const bool is_wrapping=(op == BoundArithmeticOperation::ADD) ?
        x.minimum > mask-y.minimum : x.maximum < y.minimum;
      const bool is_nonwrapping=(op == BoundArithmeticOperation::ADD) ?
        x.maximum <= mask-y.maximum : x.minimum >= y.maximum;
      if (is_wrapping) terminal &= terminal_set;
      if (is_nonwrapping) terminal &= terminal_clear;
      if (terminal == 0U) return TerminalNarrowResult{ES_FAILED,terminal};
      const bool is_consistent=narrow_bound_terminal_ranges<op>(
        x,y,z,terminal);
      return TerminalNarrowResult{is_consistent ? ES_OK : ES_FAILED,terminal};
    }
    /**
     * \brief Count a narrowed terminal as progress of a local pass
     *
     * A terminal narrows at most twice, so repeating after it terminates.
     */
    static PassProgress note_terminal(PassProgress progress,
                                      unsigned int old_terminal,
                                      unsigned int terminal) {
      const bool is_narrowed=(progress != PassProgress::FAILED) &&
        (terminal != old_terminal);
      return is_narrowed ? PassProgress::CHANGED : progress;
    }
    /// Publish Word representatives before narrowing the carry or borrow flag.
    static ExecStatus publish(Home home, const BoundLocalPass<View,3>& pass,
                              Int::BoolView flag, unsigned int terminal) {
      const ExecStatus status=pass.publish(home);
      GECODE_ES_CHECK(status);
      if (terminal == terminal_clear) GECODE_ME_CHECK(flag.zero(home));
      else if (terminal == terminal_set) GECODE_ME_CHECK(flag.one(home));
      return status;
    }
    static BoundFilterResult narrow_bounds(Home home, View x, View y, View z,
                                    Int::BoolView flag) {
      const View input[3]={x,y,z};
      BoundLocalPass<View,3> pass(input);
      const BoundCubeSnapshot<3> initial=pass.snapshot_bits();
      unsigned int terminal=flag.one() ? terminal_set : flag.zero() ? terminal_clear : terminal_any;
      const unsigned int initial_terminal=terminal;
      PassProgress progress;
      // Bounds-only stage: a closure jump stops it and hands over to the cube stage.
      do {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        const unsigned int old_terminal=terminal;
        pass.defer_synchronization();
        const TerminalNarrowResult ranges=narrow_ranges(
          pass.domain(0),pass.domain(1),pass.domain(2),terminal);
        if (ranges.status == ES_FAILED)
          return BoundFilterResult{ES_FAILED,false};
        terminal=ranges.terminal;
        progress=note_terminal(pass.close_changed(previous),old_terminal,
                               terminal);
        if (progress == PassProgress::FAILED)
          return BoundFilterResult{ES_FAILED,false};
      } while (progress == PassProgress::CHANGED);
      const bool has_new_bits=(terminal != initial_terminal) ||
        pass.has_new_bits(initial);
      return finish_bound_stage(
        pass_result(publish(home,pass,flag,terminal),progress),has_new_bits,
        [&](void) { return narrow(home,x,y,z,flag); });
    }
    /// Filter cube and interval until publication closes no value set further
    static ExecStatus narrow(Home home, View x, View y, View z,
                             Int::BoolView flag) {
      return repeat_until_closed([&](void) {
        return narrow_once(home,x,y,z,flag);
      });
    }
    /// Filter cube, interval and flag locally to a fixpoint and publish them
    static PassResult narrow_once(Home home, View x, View y, View z,
                                  Int::BoolView flag) {
      const View input[3]={x,y,z};
      BoundLocalPass<View,3> pass(input);
      unsigned int terminal=flag.one() ? terminal_set : flag.zero() ? terminal_clear : terminal_any;
      bool needs_cube=true;
      PassProgress progress;
      // Expensive pass: a closure jump stops it, so cheaper propagators run first.
      do {
        const BoundDomainSnapshot<3> previous=pass.snapshot();
        const unsigned int old_terminal=terminal;
        pass.defer_synchronization();
        if (needs_cube) {
          const TerminalNarrowResult result=(op == BoundArithmeticOperation::ADD) ?
            narrow_add(home,pass.view(0),pass.view(1),pass.view(2),terminal) :
            narrow_subtract(home,pass.view(0),pass.view(1),pass.view(2),terminal);
          if (result.status < ES_OK) return PassResult::FAILED;
          terminal=result.terminal;
        }
        const BoundCubeSnapshot<3> before_ranges=pass.snapshot_bits();
        const unsigned int before_terminal=terminal;
        const TerminalNarrowResult ranges=narrow_ranges(
          pass.domain(0),pass.domain(1),pass.domain(2),terminal);
        if (ranges.status < ES_OK) return PassResult::FAILED;
        terminal=ranges.terminal;
        progress=note_terminal(pass.close_changed(previous),old_terminal,
                               terminal);
        if (progress == PassProgress::FAILED) return PassResult::FAILED;
        needs_cube=(terminal != before_terminal) ||
          pass.has_new_bits(before_ranges);
      } while (progress == PassProgress::CHANGED);
      return pass_result(publish(home,pass,flag,terminal),progress);
    }
  };

}}}

#endif
