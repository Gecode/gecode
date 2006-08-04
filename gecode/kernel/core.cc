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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/kernel.hh"

namespace Gecode {

  unsigned long int Space::unused_uli;

  /*
   * Spaces
   *
   */

  VarTypeProcessorBase* Space::vtp[VTI_LAST];

  VarTypeProcessorBase::~VarTypeProcessorBase(void) {}

  Space::Space(void) {
    // Initialize variable entry points
    for (int i=0; i<VTI_LAST; i++)
      vars[i]=NULL;
    vars_noidx = NULL;
    // Initialize deleters
    deleters = NULL;
    // Initialize propagator pool
    pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      pool[i].init();
    // Initialize actor and branching links
    a_actors.init();
    a_actors.init_delete();
    b_status = static_cast<Branching*>(&a_actors);
    b_commit = static_cast<Branching*>(&a_actors);
    n_sub = 0;
    sub = NULL;
  }



  /*
   * Space deletion
   *
   */

  void
  Actor::flush(void) {}

  size_t
  Actor::cached(void) const {
    return 0;
  }

  void
  Space::flush(void) {
    // Flush actors registered for deletion
    ActorDeleteLink* e = &a_actors;
    ActorDeleteLink* a = e->next_delete();
    while (a != e) {
      static_cast<Actor*>(a)->flush(); a = a->next_delete();
    }
  }

  size_t
  Space::cached(void) const {
    size_t s = 0;
    const ActorDeleteLink* e = &a_actors;
    const ActorDeleteLink* a = e->next_delete();
    while (a != e) {
      s += static_cast<const Actor*>(a)->cached();
      a = a->next_delete();
    }
    return s;
  }

  Space::~Space(void) {
    // Mark space as failed
    fail();
    // Delete actors that must be deleted
    ActorDeleteLink* e = &a_actors;
    ActorDeleteLink* a = e->next_delete();
    while (a != e) {
      Actor* d = static_cast<Actor*>(a);
      a = a->next_delete();
      d->dispose(this);
    }
    // Invoke deleters
    for (Deleter* d=deleters; d != NULL; d=d->next()) {
      d->do_delete(this);
    }
    if (sub != NULL)
      Memory::free(sub);
  }


  /*
   * Performing propagation
   *
   */

  forceinline void
  Space::process(void) {
    for (int vti=VTI_LAST; vti--; ) {
      VarBase* vs = vars[vti];
      if (vs != NULL) {
	vars[vti] = NULL; vtp[vti]->process(this,vs);
      }
    }
  }

  forceinline bool
  Space::pool_get(Propagator*& p) {
    for (int c = pool_next+1; c--; ) {
      // Head of the queue
      ActorLink* lnk = &pool[c];
      // First propagator or link back to queue
      ActorLink* fst = lnk->next();
      if (lnk != fst) {
	pool_next = c;
	// Unlink first propagator from queue
	ActorLink* snd = fst->next();
	lnk->next(snd); snd->prev(lnk);
	p = static_cast<Propagator*>(fst);
	return true;
      }
    }
    pool_next = 0;
    return false;
  }

  unsigned long int
  Space::propagate(void) {
    if (failed())
      return 0;
    const PropModEvent PME_NONE = 0;
    const PropModEvent PME_ASSIGNED  =
      ((ME_GEN_ASSIGNED <<  0) | (ME_GEN_ASSIGNED <<  4) |
       (ME_GEN_ASSIGNED <<  8) | (ME_GEN_ASSIGNED << 12) |
       (ME_GEN_ASSIGNED << 16) | (ME_GEN_ASSIGNED << 20) |
       (ME_GEN_ASSIGNED << 24) | (ME_GEN_ASSIGNED << 28));
    /*
     * Count the number of propagation steps performed
     *
     */
    unsigned long int pn = 0;
    /*
     * Process modified variables, there might be modified variables
     * either from initializing the space or from a commit operation
     *
     */
    process();
    Propagator* p;
    while (pool_get(p)) {
      pn++;
      switch (p->propagate(this)) {
      case ES_FAILED:
	fail();
	return pn;
      case ES_FIX:
	{
	  // Put propagator in idle queue
	  propagator(p);
	  // Prevent that propagator gets rescheduled (turn on all events)
	  p->pme = PME_ASSIGNED;
	  process();
	  p->pme = PME_NONE;
	}
	break;
      case ES_NOFIX:
	{
	  // Propagator is currently in no queue, put in into idle
	  propagator(p);
	  p->pme = PME_NONE;
	  process();
	}
	break;
      case ES_SUBSUMED:
	{
	  // Prevent that propagator gets rescheduled (turn on all events)
	  p->pme = PME_ASSIGNED;
	  process();
	  p->destruct(this);
	}
	break;
      case __ES_FIX_PARTIAL:
	{
	  // Remember the event set to be kept after processing
	  PropModEvent keep = p->pme;
	  // Prevent that propagator gets rescheduled (turn on all events)
	  p->pme = PME_ASSIGNED;
	  process();
	  p->pme = keep;
	  assert(p->pme);
	  pool_put(p);
	}
	break;
      case __ES_NOFIX_PARTIAL:
	{
	  // Start from the specified propagator events
	  pool_put(p);
	  process();
	}
	break;
      default:
	GECODE_NEVER;
      }
    }
    return pn;
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
      b->unlink(); b->destruct(this);
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

  Space::Space(bool share, Space& s) : mm(s.mm) {
    // Initialize variable entry points
    for (int i=0; i<VTI_LAST; i++)
      vars[i]=NULL;
    vars_noidx = NULL;
    // Copy deleters
    {
      deleters = NULL;
      for (Deleter* d = s.deleters; d != NULL; d=d->next()) {
	Deleter* od = deleters;
	deleters = d->copy(this, share);
	if (deleters == NULL)
	  // Deleter can be discarded
	  deleters = od;
	else {
	  // Link deleter and set forwarding pointer
	  deleters->next(od);
	  d->forward(deleters);
	}
      }
    }
    // Initialize propagator pool
    pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      pool[i].init();
    // Copy all actors
    {
      ActorLink* p  = &a_actors;
      ActorLink* e  = &s.a_actors;
      for (ActorLink* a = e->next(); a != e; a = a->next()) {
	ActorLink* c = static_cast<Actor*>(a)->copy(this,share);
	// Link copied actor
	p->next(c); c->prev(p);
	// Forward
	a->prev(c);
	//
	static_cast<ActorDeleteLink*>(c)->init_delete();
	p = c;
      }
      // Link last actor
      p->next(&a_actors); a_actors.prev(p);
    }
    // Setup delete links
    {
      ActorDeleteLink* p  = &a_actors;
      ActorDeleteLink* e  = &s.a_actors;
      for (ActorDeleteLink* a = e->next_delete(); a != e;
	   a = a->next_delete()) {
	ActorDeleteLink* c = static_cast<ActorDeleteLink*>(a->prev());
	// Link copied actor
	p->next_delete(c); c->prev_delete(p);
	// Forward
	p = c;
      }
      // Link last actor
      p->next_delete(&a_actors); a_actors.prev_delete(p);
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

  /// A fake combination class
  class __Combine {
  public:
    ModEvent operator()(ModEvent,ModEvent) {
      GECODE_NEVER;
      return ME_GEN_ASSIGNED;
    }
  };

  Space*
  Space::clone(bool share, unsigned long int& pn) {
    pn += propagate();
    if (failed())
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
    for (Variable<VTI_NOIDX,0,__Combine>* x
	   = static_cast<Variable<VTI_NOIDX,0,__Combine>*>(c->vars_noidx);
	 x != NULL; x = x->next())
      x->u.free_me = 0;
    c->vars_noidx = NULL;
    // Update variables with indexing structure
    Propagator** s;
    if (n_sub > 0)
      s = reinterpret_cast<Propagator**>
	(Memory::malloc(n_sub*sizeof(Propagator*)));
    else
      s = NULL;
    c->sub = s;
    for (int vti=VTI_LAST; vti--; ) {
      VarBase* vs = c->vars[vti];
      if (vs != NULL) {
	c->vars[vti] = NULL; vtp[vti]->update(vs,s);
      }
    }
    // Update the number of subscriptions (both in copy and original)
    // Remember: this is a conservative estimate
    unsigned int n = s - c->sub;
    assert(n <= n_sub);
    c->n_sub = n; n_sub = n;
    // Re-establish prev links (reset forwarding information)
    ActorLink* p = &a_actors;
    ActorLink* a = p->next();
    while (a != &a_actors) {
      a->prev(p); p = a; a = a->next();
    }
    assert(a->prev() == p);
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

}

// STATISTICS: kernel-core

