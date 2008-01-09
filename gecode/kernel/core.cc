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

#ifdef GECODE_HAVE_VAR_DISPOSE
  GECODE_KERNEL_EXPORT VarDisposerBase* Space::vd[AllVarConf::idx_d];
#endif

  void VarDisposerBase::dispose(Space*,VarImpBase*) {}
  VarDisposerBase::~VarDisposerBase(void) {}



  Space::Space(void) {
#ifdef GECODE_HAVE_VAR_DISPOSE
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
#endif
    // Initialize actor and branching links
    a_actors.init();
    b_status = static_cast<Branching*>(&a_actors);
    b_commit = static_cast<Branching*>(&a_actors);
    // Initialize array for forced deletion to be empty
    d_fst = NULL;
    d_cur = NULL;
    d_lst = NULL;
    // Initialize propagator pool
    pu.p.pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      pu.p.pool[i].init();
    pu.p.branch_id = 0;
    pu.p.n_sub = 0;
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
#ifdef GECODE_HAVE_VAR_DISPOSE
    // Delete variables that were registered for disposal
    for (int i=AllVarConf::idx_d; i--;)
      if (_vars_d[i] != NULL)
        vd[i]->dispose(this, _vars_d[i]);
#endif
  }


  /*
   * Performing propagation
   *
   */

  forceinline bool
  Space::pool_get(Propagator*& p) {
    while (true) {
      // Head of the queue
      ActorLink* lnk = &pu.p.pool[pu.p.pool_next];
      // First propagator or link back to queue
      ActorLink* fst = lnk->next();
      if (lnk != fst) {
        p = static_cast<Propagator*>(fst);
        return true;
      }
      if (pu.p.pool_next == 0)
        return false;
      pu.p.pool_next--;
    }
    GECODE_NEVER;
    return false;
  }

  bool
  Space::propagate(unsigned long int& pn) {
    assert(!failed());
    Propagator* p;
    while (pool_get(p)) {
      pn++;
      // Keep old propagator modification events
      PropModEvent pme_o = p->u.pme;
      // Clear pme but leave propagator in queue
      p->u.pme = 0;
      switch (p->propagate(this,pme_o)) {
      case ES_FAILED:
        fail(); 
        return false;
      case ES_FIX:
        // Clear pme and put into idle queue
        p->u.pme = 0; p->unlink(); a_actors.head(p);
        break;
      case ES_NOFIX:
        // If no need to be run, clear pme and put into idle queue
        if (p->u.pme == 0) {
          p->unlink(); a_actors.head(p);
        }
        break;
      case __ES_SUBSUMED:
        p->unlink(); reuse(p,p->u.size);
        break;
      case __ES_PARTIAL:
        // Schedule propagator with specified propagator events
        p->unlink(); pool_put(p);
        break;
      default:
        GECODE_NEVER;
      }
    }
    return true;
  }

  bool
  Space::stable(void) const {
    int pn = pu.p.pool_next;
    while (true) {
      // Head of the queue
      const ActorLink* lnk = &pu.p.pool[pn];
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
    Propagator* p;
    if (!pool_get(p))
      return ES_STABLE;
    // Keep old propagator modification events
    PropModEvent pme_o = p->u.pme;
    // Clear pme but leave propagator in queue
    p->u.pme = 0;
    ExecStatus es = p->propagate(this,pme_o);
    switch (es) {
    case ES_FAILED:
      fail(); 
      break;
    case ES_FIX:
      // Clear pme and put into idle queue
      p->u.pme = 0; p->unlink(); a_actors.head(p);
      break;
    case ES_NOFIX:
      // If no need to be run, clear pme and put into idle queue
      if (p->u.pme == 0) {
        p->unlink(); a_actors.head(p);
      }
      break;
    case __ES_SUBSUMED:
      p->unlink(); reuse(p,p->u.size);
      break;
    case __ES_PARTIAL:
      // Schedule propagator with specified propagator events
      p->unlink(); pool_put(p);
      break;
    default:
      GECODE_NEVER;
    }
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
    while ((b_commit != &a_actors) && (d->_id != b_commit->id)) {
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
   *    The copied variables are recorded.
   *  - In the second step the dependency information of the recorded
   *    variables is updated and their forwarding information is reset.
   *
   */

  Space::Space(bool share, Space& s) 
    : mm(s.mm,s.pu.p.n_sub*sizeof(Propagator**)) {
#ifdef GECODE_HAVE_VAR_DISPOSE
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
#endif
    for (int i=0; i<AllVarConf::idx_u; i++)
      pu.u.vars_u[i] = NULL;
    pu.u.vars_noidx = NULL;
    pu.u.shared = NULL;
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
    for (VarImpBase* x = c->pu.u.vars_noidx; x != NULL; x = x->next()) {
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
    for (SharedHandle::Object* s = c->pu.u.shared; s != NULL; s = s->next)
      s->fwd = NULL;
    // Initialize propagator pool
    c->pu.p.pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      c->pu.p.pool[i].init();
    // Copy processing only data
    c->pu.p.n_sub = pu.p.n_sub;
    c->pu.p.branch_id = pu.p.branch_id;
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

  void
  Space::getVars(Reflection::VarMap&, bool) {}

  Reflection::BranchSpec
  Space::branchSpec(Reflection::VarMap& m, const BranchingDesc* d) {
    Branching* bra = b_commit;
    while ((bra != &a_actors) && (d->_id != bra->id)) {
      bra = static_cast<Branching*>(bra->next());
    }
    if (bra == &a_actors)
      throw SpaceNoBranching();
    return bra->branchSpec(this, m, d);
  }

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
   * Branching
   *
   */
  Reflection::BranchSpec
  Branching::branchSpec(Space*, Reflection::VarMap&, const BranchingDesc*) {
    throw Reflection::NoReflectionDefinedException();
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
