/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Search { namespace Par {


  /*
   * Basic access routines
   */
  template<class Tracer>
  forceinline Engine<Tracer>&
  Engine<Tracer>::Worker::engine(void) const {
    return _engine;
  }
  template<class Tracer>
  forceinline const Options&
  Engine<Tracer>::opt(void) const {
    return _opt;
  }
  template<class Tracer>
  forceinline unsigned int
  Engine<Tracer>::workers(void) const {
    return static_cast<unsigned int>(opt().threads);
  }
  template<class Tracer>
  forceinline bool
  Engine<Tracer>::stopped(void) const {
    return has_stopped;
  }



  /*
   * Engine: command and wait handling
   */
  template<class Tracer>
  forceinline typename Engine<Tracer>::Cmd
  Engine<Tracer>::cmd(void) const {
    return _cmd;
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::block(void) {
    _cmd = C_WAIT;
    _m_wait.acquire();
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::release(Cmd c) {
    _cmd = c;
    _m_wait.release();
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait(void) {
    _m_wait.acquire(); _m_wait.release();
  }


  /*
   * Engine: initialization
   */
  template<class Tracer>
  forceinline
  Engine<Tracer>::Worker::Worker(Space* s, Engine& e)
    : tracer(e.opt().tracer), _engine(e),
      path(s == NULL ? 0 : e.opt().nogoods_limit), d(0),
      idle(false) {
    tracer.worker();
    if (s != NULL) {
      if (s->status(*this) == SS_FAILED) {
        fail++;
        cur = NULL;
        if (!engine().opt().clone)
          delete s;
      } else {
        cur = snapshot(s,engine().opt());
      }
    } else {
      cur = NULL;
    }
  }

  template<class Tracer>
  forceinline
  Engine<Tracer>::Engine(const Options& o)
    : _opt(o), solutions(heap) {
    // Initialize termination information
    _n_term_not_ack = workers();
    _n_not_terminated = workers();
    // Initialize search information
    n_busy = workers();
    has_stopped = false;
    // Initialize reset information
    _n_reset_not_ack = workers();
  }


  /*
   * Statistics
   */
  template<class Tracer>
  forceinline Statistics
  Engine<Tracer>::Worker::statistics(void) {
    m.acquire();
    Statistics s = *this;
    m.release();
    return s;
  }


  /*
   * Engine: search control
   */
  template<class Tracer>
  forceinline bool
  Engine<Tracer>::signal(void) const {
    return solutions.empty() && (n_busy > 0) && !has_stopped;
  }
  template<class Tracer>
  forceinline void
  Engine<Tracer>::idle(void) {
    m_search.acquire();
    bool bs = signal();
    n_busy--;
    if (bs && (n_busy == 0))
      e_search.signal();
    m_search.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::busy(void) {
    m_search.acquire();
    assert(n_busy > 0);
    n_busy++;
    m_search.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::stop(void) {
    m_search.acquire();
    bool bs = signal();
    has_stopped = true;
    if (bs)
      e_search.signal();
    m_search.release();
  }


  /*
   * Engine: termination control
   */
  template<class Tracer>
  forceinline void
  Engine<Tracer>::terminated(void) {
    unsigned int n;
    _m_term.acquire();
    n = --_n_not_terminated;
    _m_term.release();
    // The signal must be outside of the look, otherwise a thread might be
    // terminated that still holds a mutex.
    if (n == 0)
      _e_terminate.signal();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_terminate(void) {
    _m_term.acquire();
    if (--_n_term_not_ack == 0)
      _e_term_ack.signal();
    _m_term.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait_terminate(void) {
    _m_wait_terminate.acquire();
    _m_wait_terminate.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::terminate(void) {
    // Grab the wait mutex for termination
    _m_wait_terminate.acquire();
    // Release all threads
    release(C_TERMINATE);
    // Wait until all threads have acknowledged termination request
    _e_term_ack.wait();
    // Release waiting threads
    _m_wait_terminate.release();
    // Wait until all threads have in fact terminated
    _e_terminate.wait();
    // Now all threads are terminated!
  }

  /*
   * Engine: reset control
   */
  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_reset_start(void) {
    _m_reset.acquire();
    if (--_n_reset_not_ack == 0)
      e_reset_ack_start.signal();
    _m_reset.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::ack_reset_stop(void) {
    _m_reset.acquire();
    if (++_n_reset_not_ack == workers())
      e_reset_ack_stop.signal();
    _m_reset.release();
  }

  template<class Tracer>
  forceinline void
  Engine<Tracer>::wait_reset(void) {
    m_wait_reset.acquire();
    m_wait_reset.release();
  }



  /*
   * Worker: finding and stealing working
   */
  template<class Tracer>
  forceinline Space*
  Engine<Tracer>::Worker::steal(unsigned long int& d, 
                                Tracer& myt, Tracer& ot) {
    /*
     * Make a quick check whether the worker might have work
     *
     * If that is not true any longer, the worker will be asked
     * again eventually.
     */
    if (!path.steal())
      return NULL;
    m.acquire();
    Space* s = path.steal(*this,d,myt,ot);
    m.release();
    // Tell that there will be one more busy worker
    if (s != NULL)
      engine().busy();
    return s;
  }

  /*
   * Return No-Goods
   */
  template<class Tracer>
  forceinline NoGoods&
  Engine<Tracer>::Worker::nogoods(void) {
    return path;
  }

  /*
   * Engine: search control
   */
  template<class Tracer>
  Space*
  Engine<Tracer>::next(void) {
    // Invariant: the worker holds the wait mutex
    m_search.acquire();
    if (!solutions.empty()) {
      // No search needs to be done, take leftover solution
      Space* s = solutions.pop();
      m_search.release();
      return s;
    }
    // We ignore stopped (it will be reported again if needed)
    has_stopped = false;
    // No more solutions?
    if (n_busy == 0) {
      m_search.release();
      return NULL;
    }
    m_search.release();
    // Okay, now search has to continue, make the guys work
    release(C_WORK);

    /*
     * Wait until a search related event has happened. It might be that
     * the event has already been signalled in the last run, but the
     * solution has been removed. So we have to try until there has
     * something new happened.
     */
    while (true) {
      e_search.wait();
      m_search.acquire();
      if (!solutions.empty()) {
        // Report solution
        Space* s = solutions.pop();
        m_search.release();
        // Make workers wait again
        block();
        return s;
      }
      // No more solutions or stopped?
      if ((n_busy == 0) || has_stopped) {
        m_search.release();
        // Make workers wait again
        block();
        return NULL;
      }
      m_search.release();
    }
    GECODE_NEVER;
    return NULL;
  }

  template<class Tracer>
  Support::Terminator* 
  Engine<Tracer>::Worker::terminator(void) const {
    return &_engine;
  }

  /*
   * Termination and deletion
   */
  template<class Tracer>
  Engine<Tracer>::Worker::~Worker(void) {
    delete cur;
    path.reset(0);
    tracer.done();
  }

}}}

// STATISTICS: search-par
