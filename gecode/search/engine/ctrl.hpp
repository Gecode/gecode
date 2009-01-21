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

namespace Gecode { namespace Search {

  forceinline
  Engine::Engine(size_t sz)
    : _stopped(false), mem_space(sz), mem_cur(0), mem_total(0) {
    memory = 0;
  }

  forceinline void
  Engine::start(void) {
    _stopped = false;
  }

  forceinline bool
  Engine::stop(Stop* st, size_t sz) {
    if (st == NULL)
      return false;
    memory += sz;
    _stopped |= st->stop(*this);
    memory -= sz;
    return _stopped;
  }

  forceinline bool
  Engine::stopped(void) const {
    return _stopped;
  }

  forceinline void
  Engine::push(const Space* s, const BranchingDesc* d) {
    if (s != NULL)
      mem_total += mem_space + s->allocated();
    mem_total += d->size();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Engine::adapt(const Space* s) {
    mem_total += mem_space + s->allocated();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Engine::constrained(const Space* s1, const Space* s2) {
    mem_total -= s1->allocated();
    mem_total += s2->allocated();
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Engine::lao(const Space* s) {
    mem_total -= mem_space + s->allocated();
  }

  forceinline void
  Engine::pop(const Space* s, const BranchingDesc* d) {
    if (s != NULL)
      mem_total -= mem_space + s->allocated();
    mem_total -= d->size();
  }

  forceinline void
  Engine::current(const Space* s) {
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
  Engine::reset(const Space* s) {
    mem_cur   = mem_space + s->allocated();
    mem_total = mem_cur;
    if (mem_total > memory)
      memory = mem_total;
  }

  forceinline void
  Engine::reset(void) {
    mem_cur   = 0;
    mem_total = 0;
  }

}}

// STATISTICS: search-any
