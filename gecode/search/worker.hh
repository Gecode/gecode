/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#ifndef __GECODE_SEARCH_WORKER_HH__
#define __GECODE_SEARCH_WORKER_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search {

  /**
   * \brief %Search worker control including memory information
   */
  class Worker : public Statistics {
  protected:
    /// Whether engine has been stopped
    bool _stopped;
    /// Memory required for a single space
    size_t mem_space;
    /// Memory for the current space (including memory for caching)
    size_t mem_cur;
    /// Current total memory
    size_t mem_total;
    /// Depth of root node (for work stealing)
    unsigned long int root_depth;
  public:
    /// Initialize with space size \a sz
    Worker(size_t sz);
    /// Reset stop information
    void start(void);
    /// Check whether engine must be stopped (with additional stackspace \a sz)
    bool stop(const Options& o, size_t sz);
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// New space \a s and choice \a c get pushed on stack
    void push(const Space* s, const Choice* c);
    /// Space \a s1 is replaced by space \a s2 due to constraining
    void constrained(const Space* s1, const Space* s2);
    /// New space \a s is added for adaptive recomputation
    void adapt(const Space* s);
    /// Space \a s and choice \a c get popped from stack
    void pop(const Space* s, const Choice* c);
    /// Space \a s gets used for LAO (removed from stack)
    void lao(const Space* s);
    /// Space \a s becomes current space (\a s = NULL: current space deleted)
    void current(const Space* s);
    /// Reset statistics for space \a s with root depth \a d
    void reset(const Space* s, unsigned long int d=0);
    /// Reset statistics for failed space
    void reset(void);
    /// Record stack depth \a d
    void stack_depth(unsigned long int d);
    /// Return steal depth
    unsigned long int steal_depth(unsigned long int d) const;
  };



  forceinline
  Worker::Worker(size_t sz)
    : _stopped(false), mem_space(sz), mem_cur(0), mem_total(0), 
      root_depth(0) {
    memory = 0;
  }

  forceinline void
  Worker::start(void) {
    _stopped = false;
  }

  forceinline bool
  Worker::stop(const Options& o, size_t sz) {
    if (o.stop == NULL)
      return false;
    memory += sz;
    _stopped |= o.stop->stop(*this,o);
    memory -= sz;
    return _stopped;
  }

  forceinline bool
  Worker::stopped(void) const {
    return _stopped;
  }

  forceinline void
  Worker::push(const Space* s, const Choice* c) {
    if (s != NULL)
      mem_total += mem_space + s->allocated();
    mem_total += c->size();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Worker::adapt(const Space* s) {
    mem_total += mem_space + s->allocated();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Worker::constrained(const Space* s1, const Space* s2) {
    mem_total -= s1->allocated();
    mem_total += s2->allocated();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Worker::lao(const Space* s) {
    mem_total -= mem_space + s->allocated();
  }

  forceinline void
  Worker::pop(const Space* s, const Choice* c) {
    if (s != NULL)
      mem_total -= mem_space + s->allocated();
    mem_total -= c->size();
  }

  forceinline void
  Worker::current(const Space* s) {
    if (s == NULL) {
      mem_total -= mem_cur;
      mem_cur = 0;
    } else {
      mem_cur = mem_space + s->allocated();
      mem_total += mem_cur;
      if (mem_total > memory)
        memory = mem_total;
    }
  }

  forceinline void
  Worker::reset(const Space* s, unsigned long int d) {
    mem_cur   = mem_space + s->allocated();
    mem_total = mem_cur;
    if (mem_total > memory)
      memory = mem_total;
    root_depth = d;
    if (depth < d)
      depth = d;
  }

  forceinline void
  Worker::reset(void) {
    mem_cur    = 0;
    mem_total  = 0;
    root_depth = 0;
  }

  forceinline void
  Worker::stack_depth(unsigned long int d) {
    if (depth < root_depth + d)
      depth = root_depth + d;
  }

  forceinline unsigned long int
  Worker::steal_depth(unsigned long int d) const {
    return root_depth + d;
  }

}}

#endif

// STATISTICS: search-other
