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

#ifndef GECODE_WORD_ARITHMETIC_BOUNDED_DOMAIN_HPP
#define GECODE_WORD_ARITHMETIC_BOUNDED_DOMAIN_HPP

// STATISTICS: word-prop

namespace Gecode { namespace Word { namespace Arithmetic {

  /// Bounds-filter status and whether cube propagation must run afterward.
  struct BoundFilterResult {
    ExecStatus status;
    bool needs_cube;
  };

  /** \brief Finish a bounds-only stage from its loop and publication result
   *
   * A stage that fixed bits (\a has_new_bits) or stopped after a closure
   * jump needs the cube stage next. When a value set closed a view beyond
   * its record, which may fix bits, \a narrow runs both stages instead.
   */
  template<class Narrow>
  forceinline BoundFilterResult
  finish_bound_stage(PassResult result, bool has_new_bits, Narrow narrow) {
    switch (result) {
    case PassResult::FAILED:
      return {ES_FAILED,false};
    case PassResult::FIXPOINT:
      return {ES_FIX,has_new_bits};
    case PassResult::STOPPED:
      return {ES_NOFIX,true};
    case PassResult::CLOSED_BEYOND: {
      const ExecStatus status=narrow();
      return {status,status == ES_NOFIX};
    }
    }
    GECODE_NEVER;
    return {ES_FAILED,false};
  }

  /** \brief Local cube and ranked interval for one distinct arithmetic role
   *
   * The endpoints are ranks in kind's ordering; lo and hi are encoded bits.
   * Synchronized records describe their intersection. A deferred pass can
   * temporarily separate them and must synchronize before publication.
   * Publishing a synchronized record reproduces it, unless a value set
   * closes the view further.
   */
  struct BoundLocalDomain {
    unsigned int width;
    WordDomainType kind;
    WordValue lo;
    WordValue hi;
    WordValue minimum;
    WordValue maximum;
    /// Whether cube/interval synchronization is batched by the current phase
    bool deferred;

    bool synchronize(void) {
      return synchronize_domain(width,kind,lo,hi,minimum,maximum);
    }
    bool intersect_range(WordValue min, WordValue max) {
      minimum=std::max(minimum,min);
      maximum=std::min(maximum,max);
      return (minimum <= maximum) && (deferred || synchronize());
    }
  };

  forceinline WordRankInterval
  snapshot_bound_interval(const BoundLocalDomain& domain) {
    return {domain.minimum,domain.maximum};
  }

  /// Compare deductions only; width, kind, and batching mode stay fixed.
  forceinline bool
  operator ==(const BoundLocalDomain& x, const BoundLocalDomain& y) {
    return (x.lo == y.lo) && (x.hi == y.hi) &&
      (x.minimum == y.minimum) && (x.maximum == y.maximum);
  }

  /**
   * \brief Classify a pass over \a n records (see PassProgress)
   *
   * \a previous holds the records at the start of the pass, \a deduced
   * after its deductions, and \a closed after closing them.
   */
  forceinline PassProgress
  classify_records(const BoundLocalDomain* previous,
                   const BoundLocalDomain* deduced,
                   const BoundLocalDomain* closed, unsigned int n) {
    bool has_changed=false, has_new_bits=false, has_jump=false;
    for (unsigned int i=0; i<n; i++) {
      has_changed |= !(closed[i] == previous[i]);
      has_new_bits |= (closed[i].lo != previous[i].lo) ||
        (closed[i].hi != previous[i].hi);
      has_jump |= (closed[i].minimum != deduced[i].minimum) ||
        (closed[i].maximum != deduced[i].maximum);
    }
    return classify_pass(has_changed,has_new_bits,has_jump);
  }

  /// Adapt a local domain to the existing cube algorithms without variable tells.
  class BoundLocalView {
  public:
    BoundLocalView(void) : domain(nullptr) {}
    explicit BoundLocalView(BoundLocalDomain& source) : domain(&source) {}
    unsigned int width(void) const { return domain->width; }
    WordValue mask(void) const { return width_mask(domain->width); }
    WordValue lo(void) const { return domain->lo; }
    WordValue hi(void) const { return domain->hi; }
    WordValue unknown(void) const { return domain->hi & ~domain->lo; }
    bool assigned(void) const {
      return (domain->lo == domain->hi) && (domain->minimum == domain->maximum);
    }
    WordValue val(void) const { assert(assigned()); return domain->lo; }
    ModEvent narrow(Space&, WordValue lo, WordValue hi) {
      domain->lo |= lo;
      domain->hi &= hi;
      const bool is_inconsistent=(domain->lo & ~domain->hi) != 0 ||
        (!domain->deferred && !domain->synchronize());
      if (is_inconsistent)
        return ME_WORD_FAILED;
      return assigned() ? ME_WORD_VAL : ME_WORD_DOM;
    }
    bool operator ==(const BoundLocalView& y) const { return domain == y.domain; }
    bool operator !=(const BoundLocalView& y) const { return domain != y.domain; }
  private:
    BoundLocalDomain* domain;
  };

  template<class View>
  forceinline BoundLocalDomain
  snapshot_bound_domain(View x) {
    static_assert(View::supports_bounds,
                  "bounded arithmetic requires a bounded Word view");
    return BoundLocalDomain{x.width(),x.domain_type(),x.lo(),x.hi(),
                            x.rank_minimum(),x.rank_maximum(),false};
  }

  /** \brief Tell a local record to \a x and report whether it is a fixpoint
   *
   * Publishing a synchronized record reproduces it, unless a value set
   * closes the domain further. The pass has not filtered with that closure,
   * so the result is then ES_NOFIX, and otherwise ES_FIX.
   */
  template<class View>
  forceinline ExecStatus
  publish_bound_domain(Home home, View x, const BoundLocalDomain& domain) {
    if (me_failed(narrow_word_domain(
          home,x,{domain.lo,domain.hi},{domain.minimum,domain.maximum})))
      return ES_FAILED;
    const bool is_beyond=x.has_value_set() &&
      !(snapshot_bound_domain(x) == domain);
    return is_beyond ? ES_NOFIX : ES_FIX;
  }

  /// Completed domain checkpoint for a fixed-arity arithmetic pass.
  template<unsigned int arity>
  struct BoundDomainSnapshot {
    BoundLocalDomain domains[arity];
  };

  /// Cube checkpoint taken at entry or after the current cube filter.
  template<unsigned int arity>
  struct BoundCubeSnapshot {
    WordCube cubes[arity];
  };

  /** \brief Stack-owned aliases, checkpoints, and closure for small arithmetic
   *
   * Only first representatives are exposed as roles or views and published.
   * Borrowed local views remain valid for this noncopyable owner's lifetime.
   */
  template<class View, unsigned int arity>
  class BoundLocalPass {
  public:
    explicit BoundLocalPass(const View (&input)[arity]) {
      static_assert((arity >= 2) && (arity <= 4),
                    "local arithmetic passes have two to four roles");
      for (unsigned int i=0; i<arity; i++) {
        views[i]=input[i];
        domains[i]=snapshot_bound_domain(input[i]);
        representatives[i]=i;
        for (unsigned int j=0; j<i; j++)
          if (input[i].varimp() == input[j].varimp()) {
            representatives[i]=representatives[j];
            break;
          }
      }
    }
    BoundLocalPass(const BoundLocalPass&)=delete;
    BoundLocalPass& operator =(const BoundLocalPass&)=delete;

    BoundLocalDomain& domain(unsigned int role) {
      return domains[representatives[role]];
    }
    BoundLocalView view(unsigned int role) {
      return BoundLocalView(domain(role));
    }
    BoundDomainSnapshot<arity> snapshot(void) const {
      BoundDomainSnapshot<arity> result;
      for (unsigned int i=0; i<arity; i++) result.domains[i]=domains[i];
      return result;
    }
    BoundCubeSnapshot<arity> snapshot_bits(void) const {
      BoundCubeSnapshot<arity> result;
      for (unsigned int i=0; i<arity; i++)
        result.cubes[i]=WordCube{domains[i].lo,domains[i].hi};
      return result;
    }
    void defer_synchronization(void) {
      for (unsigned int i=0; i<arity; i++) domains[i].deferred=true;
    }
    /** \brief Close the records changed since \a previous and classify the
     * pass (see PassProgress)
     *
     * Unchanged slots already describe closed domains, including aliases.
     */
    PassProgress close_changed(const BoundDomainSnapshot<arity>& previous) {
      const BoundDomainSnapshot<arity> deduced=snapshot();
      for (unsigned int i=0; i<arity; i++) {
        domains[i].deferred=false;
        const bool has_failed=!(domains[i] == previous.domains[i]) &&
          !domains[i].synchronize();
        if (has_failed) return PassProgress::FAILED;
      }
      return classify(previous,deduced);
    }
    /** \brief Close every representative and classify the pass since
     * \a previous
     *
     * Division closes each representative even when its record is unchanged.
     */
    PassProgress close_distinct(const BoundDomainSnapshot<arity>& previous) {
      const BoundDomainSnapshot<arity> deduced=snapshot();
      for (unsigned int i=0; i<arity; i++) domains[i].deferred=false;
      for (unsigned int i=0; i<arity; i++) {
        const bool has_failed=(representatives[i] == i) &&
          !domains[i].synchronize();
        if (has_failed) return PassProgress::FAILED;
      }
      return classify(previous,deduced);
    }
    bool is_unchanged(const BoundDomainSnapshot<arity>& previous) const {
      for (unsigned int i=0; i<arity; i++)
        if (!(domains[i] == previous.domains[i])) return false;
      return true;
    }
    bool has_new_bits(const BoundCubeSnapshot<arity>& previous) const {
      for (unsigned int i=0; i<arity; i++) {
        const bool has_changed=(domains[i].lo != previous.cubes[i].lo) ||
          (domains[i].hi != previous.cubes[i].hi);
        if (has_changed) return true;
      }
      return false;
    }
    /// Publish representatives; ES_NOFIX if a value set closed beyond one.
    ExecStatus publish(Home home) const {
      ExecStatus status=ES_FIX;
      for (unsigned int i=0; i<arity; i++)
        if (representatives[i] == i) {
          status=combine_publication(
            status,publish_bound_domain(home,views[i],domains[i]));
          GECODE_ES_CHECK(status);
        }
      return status;
    }
    /// Publish changed representatives; ES_NOFIX as for publish().
    ExecStatus publish_changed(Home home,
                              const BoundDomainSnapshot<arity>& initial) const {
      ExecStatus status=ES_FIX;
      for (unsigned int i=0; i<arity; i++) {
        const bool needs_publication=(representatives[i] == i) &&
          !(domains[i] == initial.domains[i]);
        if (needs_publication) {
          status=combine_publication(
            status,publish_bound_domain(home,views[i],domains[i]));
          GECODE_ES_CHECK(status);
        }
      }
      return status;
    }
  private:
    /// Classify the pass from its start \a previous and its deductions
    PassProgress classify(const BoundDomainSnapshot<arity>& previous,
                          const BoundDomainSnapshot<arity>& deduced) const {
      return classify_records(previous.domains,deduced.domains,domains,arity);
    }
    View views[arity];
    BoundLocalDomain domains[arity];
    unsigned int representatives[arity];
  };

}}}

#endif
