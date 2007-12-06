/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/kernel.hh"

namespace Gecode {

  GECODE_KERNEL_EXPORT unsigned long int Space::unused_uli;

  /*
   * Spaces
   *
   */

  GECODE_KERNEL_EXPORT VarDisposerBase* Space::vd[AllVarConf::idx_d];

  void VarDisposerBase::dispose(Space*,VarImpBase*) {}
  VarDisposerBase::~VarDisposerBase(void) {}



  Space::Space(void) {
    // Initialize array for forced deletion to be empty
    d_fst = NULL;
    d_cur = NULL;
    d_lst = NULL;
    // Initialize variable entry points
    for (int i=0; i<AllVarConf::idx_pu; i++)
      _vars_pu[i] = NULL;
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
    // Initialize propagator pool
    pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      pool[i].init();
    // Initialize actor and branching links
    a_actors.init();
    b_status = static_cast<Branching*>(&a_actors);
    b_commit = static_cast<Branching*>(&a_actors);
    branch_id = 0;
    pu.n_sub = 0;
    shared = NULL;
  }



  /*
   * Space deletion
   *
   */

#ifdef __GNUC__
  /// To avoid warnings from GCC
  Actor::~Actor(void) {}
#endif

  size_t
  Actor::allocated(void) const {
    return 0;
  }

  size_t
  Space::allocated(void) const {
    size_t s = mm.allocated();
    Actor** a = d_fst;
    Actor** e = d_cur;
    while (a < e) {
      s += (*a)->allocated(); a++;
    }
    return s;
  }

  void
  Space::d_resize(void) {
    if (d_fst == NULL) {
      // Create new array
      d_fst = static_cast<Actor**>(alloc(4*sizeof(Actor*)));
      d_cur = d_fst;
      d_lst = d_fst+4;
    } else {
      // Resize existing array
      ptrdiff_t n = d_lst - d_fst;
      assert(n != 0);
      Actor** a = static_cast<Actor**>(alloc(2*n*sizeof(Actor*)));
      memcpy(a, d_fst, n*sizeof(Actor*));
      reuse(d_fst,n*sizeof(Actor*));
      d_fst = a;
      d_cur = a+n;
      d_lst = a+2*n;
    }
  }
  
  Space::~Space(void) {
    // Mark space as failed
    fail();
    // Delete actors that must be deleted
    {
      Actor** a = d_fst;
      Actor** e = d_cur;
      // So that d_unforce knows that deletion is in progress
      d_fst = NULL;
      while (a < e) {
        (void) (*a)->dispose(this);
        a++;
      }
    }
    // Delete variables that were registered for deletion
    for (int i=AllVarConf::idx_d; i--;)
      if (_vars_d[i] != NULL)
        vd[i]->dispose(this, _vars_d[i]);
  }


  /*
   * Performing propagation
   *
   */

  forceinline bool
  Space::pool_get(Propagator*& p) {
    while (true) {
      // Head of the queue
      ActorLink* lnk = &pool[pool_next];
      // First propagator or link back to queue
      ActorLink* fst = lnk->next();
      if (lnk != fst) {
        p = static_cast<Propagator*>(fst);
        return true;
      }
      if (pool_next == 0)
        return false;
      pool_next--;
    }
    GECODE_NEVER;
    return false;
  }

  bool
  Space::propagate(unsigned long int& pn) {
    assert(!failed());
    // To initialize pointer
    PropModEvent  pme_dummy = 0;
    // Point to propagator modevent of last executed propagators
    PropModEvent* pme_prop = &pme_dummy;
    // Xor mask for last executed propagator
    PropModEvent  pme_mask = 0;
    while (true) {
      // Process modified variables (in first iteration: from commit or setup)
      process();
      // Reset propagator modevent
      *pme_prop ^= pme_mask;
      Propagator* p;
      if (!pool_get(p))
        return true;
      pn++; pme_prop = &p->u.pme;
      ExecStatus es = p->propagate(this);
      p->unlink(); 
      switch (es) {
      case ES_FAILED:
        fail(); 
        return false;
      case ES_FIX:
        // Propagator is currently in no queue, put into idle
        a_actors.head(p);
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        // The mask resets all events after processing (idle)
        pme_mask = AllVarConf::pme_assigned;
        break;
      case ES_NOFIX:
        // Propagator is currently in no queue, put into idle
        a_actors.head(p);
        // Mark propagator as idle
        p->u.pme = 0;
        // The mask leaves the pme unchanged
        pme_mask = 0;
        break;
      case __ES_SUBSUMED:
        /*
         * Even though the propagator is already subsumed, it might
         * still be contained in the dependency arrays of assigned
         * variables. This is due to the fact that propagators
         * are not unsubscribed from assigned variable implementations.
         *
         * This is quite dirty: the propagator is already reused,
         * even though its pme might be used in processing. However,
         * this is safe: only the pme is used, and the pme will not be
         * overwritten by reusing the memory (unless memory is requested,
         * which does not happen during processing).
         */
        reuse(p,p->u.size);
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        // The mask does not matter
        break;
      case __ES_FIX_PARTIAL:
        // Schedule propagator with specified propagator events
        pool_put(p);
        // Remember the event set to be kept after processing
        pme_mask = AllVarConf::pme_assigned ^ p->u.pme;
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        break;
      case __ES_NOFIX_PARTIAL:
        // Start from the specified propagator events
        pool_put(p);
        // The mask leaves the pme unchanged
        pme_mask = 0;
        break;
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return true;
  }

  bool
  Space::stable(void) const {
    const_cast<Space*>(this)->process();
    int pn = pool_next;
    while (true) {
      // Head of the queue
      const ActorLink* lnk = &pool[pn];
      // First propagator or link back to queue
      const ActorLink* fst = lnk->next();
      if (lnk != fst)
        return false;
      if (pn == 0)
        return true;
      pn--;
    }
    GECODE_NEVER;
    return true;
  }

  /*
   * Step-by-step propagation
   * 
   */
  ExecStatus
  Space::step(void) {
    if (failed())
      return ES_FAILED;

    // Point to propagator after one process iteration
    Propagator* p = NULL;
    // Xor mask for last executed propagator
    PropModEvent pme_mask = 0;
    // ExecStatus of last executed propagator
    ExecStatus es = ES_STABLE;
    while (true) {
      // Process modified variables (in first iteration: from commit or setup)
      process();
      // Reset propagator modevent
      if (p != NULL) {
        p->u.pme ^= pme_mask;
        return es;
      }
      if (!pool_get(p))
        return ES_STABLE;
      es = p->propagate(this);
      p->unlink(); 
      switch (es) {
      case ES_FAILED:
        fail(); 
        return ES_FAILED;
      case ES_FIX:
        // Put propagator in idle queue
        a_actors.head(p);
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        // The mask resets all events after processing (idle)
        pme_mask = AllVarConf::pme_assigned;
        break;
      case ES_NOFIX:
        // Propagator is currently in no queue, put into idle
        a_actors.head(p);
        // Mark propagator as idle
        p->u.pme = 0;
        // The mask leaves the pme unchanged
        pme_mask = 0;
        break;
      case __ES_SUBSUMED:
        /*
         * Even though the propagator is already subsumed, it might
         * still be contained in the dependency arrays of assigned
         * variables. This is due to the fact that propagators
         * are not unsubscribed from assigned variable implementations.
         *
         * This is quite dirty: the propagator is already reused,
         * even though its pme might be used in processing. However,
         * this is safe: only the pme is used, and the pme will not be
         * overwritten by reusing the memory (unless memory is requested,
         * which does not happen during processing).
         */
        reuse(p,p->u.size);
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        // The mask does not matter
        break;
      case __ES_FIX_PARTIAL:
        // Schedule propagator with specified propagator events
        pool_put(p);
        // Remember the event set to be kept after processing
        pme_mask = AllVarConf::pme_assigned ^ p->u.pme;
        // Prevent that propagator gets rescheduled (turn on all events)
        p->u.pme = AllVarConf::pme_assigned;
        break;
      case __ES_NOFIX_PARTIAL:
        // Start from the specified propagator events
        pool_put(p);
        // The mask leaves the pme unchanged
        pme_mask = 0;
        break;
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return es;
  }

  void
  Space::commit(const BranchingDesc* d, unsigned int a) {
    if (failed())
      return;
    /*
     * This relies on the fact that branching descriptions must be
     * used in the order of creation. If a branching description
     * with an id different from the id of the current branching
     * is used, it is clear that the current branching can be discarded
     * as all of its descriptions must have been used already.
     *
     */
    while ((b_commit != &a_actors) && (d->id != b_commit->id)) {
      Branching* b = b_commit;
      b_commit = static_cast<Branching*>(b_commit->next());
      if (b == b_status)
        b_status = b_commit;
      b->unlink(); 
      reuse(b,b->dispose(this));
    }
    if (b_commit == &a_actors)
      throw SpaceNoBranching();
    if (a >= d->alternatives())
      throw SpaceIllegalAlternative();
    if (b_commit->commit(this,d,a) == ES_FAILED)
      fail();
  }

  /*
   * Space cloning
   *
   * Cloning is performed in two steps:
   *  - The space itself is copied by the copy constructor. This
   *    also copies all propagators, branchings, and variables.
   *    The copied variables are recorded by the variable processor.
   *  - In the second step the dependency information of the recorded
   *    variables is updated and their forwarding information is reset.
   *
   */

  Space::Space(bool share, Space& s) 
    : mm(s.mm,s.pu.n_sub*sizeof(Propagator**)), branch_id(s.branch_id) {
    // Initialize variable entry points
    pu.vars_noidx = NULL;
    for (int i=0; i<AllVarConf::idx_pu; i++)
      _vars_pu[i] = NULL;
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
    shared = NULL;
    // Copy all actors
    {
      ActorLink* p  = &a_actors;
      ActorLink* e  = &s.a_actors;
      for (ActorLink* a = e->next(); a != e; a = a->next()) {
        ActorLink* c = static_cast<Actor*>(a)->copy(this,share);
        // Link copied actor
        p->next(c); c->prev(p);
        // Note that forwarding is done in the constructors
        p = c;
      }
      // Link last actor
      p->next(&a_actors); a_actors.prev(p);
    }
    // Setup array for actor disposal
    {
      ptrdiff_t n = s.d_cur - s.d_fst;
      if (n == 0) {
        // No actors
        d_fst = NULL;
        d_cur = NULL;
        d_lst = NULL;
      } else {
        // Leave one entry free
        Actor** a = static_cast<Actor**>(alloc((n+1)*sizeof(Actor*)));
        d_fst = a;
        d_cur = a+n;
        d_lst = a+n+1;
        Actor** f = s.d_fst;
        do {
          n--;
          a[n] = static_cast<Actor*>(f[n]->prev());
        } while (n != 0);
      }
    }
    // Setup branching pointers
    if (s.b_status == &s.a_actors) {
      b_status = static_cast<Branching*>(&a_actors);
    } else {
      b_status = static_cast<Branching*>(s.b_status->prev());
    }
    if (s.b_commit == &s.a_actors) {
      b_commit = static_cast<Branching*>(&a_actors);
    } else {
      b_commit = static_cast<Branching*>(s.b_commit->prev());
    }
  }


  /*
   * Stage 2: updating variables
   *
   */

  Space*
  Space::clone(bool share, unsigned long int& pn) {
    if (failed() || !propagate(pn))
      throw SpaceFailed("Space::clone");

    /*
     * Stage one
     *
     * Copy all data structures (which in turn will invoke the
     * constructor Space::Space.
     */
    Space* c = copy(share);

    /*
     * Stage two
     *
     * Update subscriptions and reset forwarding pointers
     *
     */
    // Update variables without indexing structure
    for (VarImpBase* x = c->pu.vars_noidx; x != NULL; x = x->next()) {
      x->u.free_me = 0;
      x->u.fwd     = NULL;
    }
    // Update variables with indexing structure
    {
      ActorLink** s = static_cast<ActorLink**>(c->mm.subscriptions());
      c->update(s);
    }
    // Re-establish prev links (reset forwarding information)
    ActorLink* p_a = &a_actors;
    ActorLink* c_a = p_a->next();
    // First update propagators and check for advisors that also must be reset
    while (c_a != b_commit) {
      Propagator* p = static_cast<Propagator*>(c_a);
      if (p->u.advisors != NULL) {
        ActorLink* a = p->u.advisors;
        p->u.advisors = NULL;
        do {
          a->prev(p); a = a->next();
        } while (a != NULL);
      }
      c_a->prev(p_a); p_a = c_a; c_a = c_a->next();
    }
    // Now the branchings
    while (c_a != &a_actors) {
      c_a->prev(p_a); p_a = c_a; c_a = c_a->next();
    }
    assert(c_a->prev() == p_a);
    // Reset links for shared objects
    for (SharedHandle::Object* s = c->shared; s != NULL; s = s->next)
      s->fwd = NULL;
    c->shared = NULL;
    // Initialize propagator pool
    c->pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      c->pool[i].init();
    // Copy number of subscriptions
    c->pu.n_sub = pu.n_sub;
    return c;
  }

  unsigned int
  Space::propagators(void) const {
    if (failed())
      throw SpaceFailed("Space::propagators");
    unsigned int n = 0;
    for (const ActorLink* a = a_actors.next(); a != b_commit; a = a->next())
      n++;
    return n;
  }

  unsigned int
  Space::branchings(void) const {
    if (failed())
      throw SpaceFailed("Space::branchings");
    unsigned int n = 0;
    for (const ActorLink* a = b_status; a != &a_actors; a = a->next())
      n++;
    return n;
  }

  Reflection::SpecIter
  Space::actorSpecs(Reflection::VarMap& m) {
    Reflection::SpecIter i(this, m);
    return i;
  }

  void
  Space::getVars(Reflection::VarMap&) {}

  /*
   * Propagator
   *
   */
  ExecStatus 
  Propagator::advise(Space*, Advisor*, const Delta*) {
    assert(false);
    return ES_FAILED;
  }

  /*
   * Actor
   *
   */
  Reflection::ActorSpec&
  Actor::spec(Space*, Reflection::VarMap&) {
    throw Reflection::NoReflectionDefinedException();
  }

  Reflection::ActorSpec&
  Actor::spec(Space*, Reflection::VarMap&, const Support::Symbol& name) {
    return *new Reflection::ActorSpec(name);
  }

}

// STATISTICS: kernel-core
