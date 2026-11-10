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
 *
 */

#include <gecode/word.hh>

namespace Gecode { namespace Word {
  void WordVarImp::subscribe(Space& home, Propagator& p, PropCond pc,
                             bool is_scheduled) {
    WordVarImpBase::subscribe(home,p,pc,assigned(),is_scheduled);
  }
  void WordVarImp::reschedule(Space& home, Propagator& p, PropCond pc) {
    WordVarImpBase::reschedule(home,p,pc,assigned());
  }
  void WordVarImp::subscribe(Space& home, Advisor& a, bool is_run_on_failure) {
    WordVarImpBase::subscribe(home,a,assigned(),is_run_on_failure);
  }

  ModEvent
  WordVarImp::narrow_values(Space& home, const WordValueSet& set) {
    assert(_width <= max_value_set_width);
    if (_has_value_set)
      return static_cast<ValueSetWordVarImp*>(this)->
        intersect_values(home,set);
    const WordValueSet next = intersect_value_sets(values(),set);
    if (is_empty_value_set(next))
      return fail(home);
    const WordCube hull = value_set_hull(next,_width);
    if (!bounded())
      return narrow(home,hull.lo,hull.hi);
    const WordRankInterval interval = value_set_rank_interval(
      order_value_set(_domain_type,_width,next),_width);
    return static_cast<BoundedWordVarImp*>(this)->
      narrow_domain(home,hull.lo,hull.hi,interval.minimum,interval.maximum);
  }

  ModEvent
  BoundedWordVarImp::close_intersection(Space& home,
                                        const WordDomain& intersection) {
    const bool is_intersection_empty =
      ((intersection.cube.lo & ~intersection.cube.hi) != 0) ||
      (intersection.interval.minimum > intersection.interval.maximum);
    if (is_intersection_empty)
      return fail(home);
    const WordDomainResult result =
      compute_domain_closure(_width,_domain_type,intersection);
    if (!result.is_consistent)
      return fail(home);
    const WordDomain current = {{_lo,_hi},{_minimum,_maximum}};
    const WordDomain next = result.domain;
    const ModEvent event = classify_domain_change(current,next);
    if (event == ME_WORD_NONE)
      return ME_WORD_NONE;
    WordDelta delta(_hi & ~next.cube.hi,next.cube.lo & ~_lo,_domain_type,
                    _minimum,_maximum,next.interval.minimum,
                    next.interval.maximum);
    _lo = next.cube.lo;
    _hi = next.cube.hi;
    _minimum = next.interval.minimum;
    _maximum = next.interval.maximum;
    return notify(home,event,delta);
  }

  ModEvent
  ValueSetWordVarImp::narrow_cube(Space& home, WordCube cube) {
    return replace_values(home,
                          restrict_value_set_to_cube(_values,_width,cube));
  }

  ModEvent
  ValueSetWordVarImp::narrow_domain_values(Space& home, WordCube cube,
                                           WordRankInterval interval) {
    const WordValueSet in_cube =
      restrict_value_set_to_cube(_values,_width,cube);
    return replace_values(home,restrict_value_set_to_ranks(
      _domain_type,_width,in_cube,interval));
  }

  ModEvent
  ValueSetWordVarImp::narrow_ranks(Space& home, WordRankInterval interval) {
    return replace_values(home,restrict_value_set_to_ranks(
      _domain_type,_width,_values,interval));
  }

  ModEvent
  ValueSetWordVarImp::assign_value(Space& home, WordValue value) {
    return replace_values(home,singleton_value_set(value));
  }

  ModEvent
  ValueSetWordVarImp::remove_value(Space& home, WordValue value) {
    return replace_values(home,without_value(_values,value));
  }

  ModEvent
  ValueSetWordVarImp::intersect_values(Space& home, const WordValueSet& set) {
    return replace_values(home,intersect_value_sets(_values,set));
  }
}}

// STATISTICS: word-var
