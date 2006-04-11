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
  unsigned int Space::unused_ui;

  /*
   * Spaces
   *
   */

  Space::VarTypeData Space::vtd[VTI_LAST];

  VarTypeProcessorBase::~VarTypeProcessorBase(void) {}

  Space::Space(void) {
    // Initialize variable entry points
    for (int i=0; i<VTI_LAST; i++)
      vars[i]=NULL;
    // Initialize propagator pool
    pool_next = 0;
    for (int i=0; i<=PC_MAX; i++)
      pool[i].init();
    // Initialize actor and branching links
    a_actors.init();
    a_actors.init_delete();
    b_fst = static_cast<Branching*>(&a_actors);
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
    // Delete actors that must be deleted
    ActorDeleteLink* e = &a_actors;
    ActorDeleteLink* a = e->next_delete();
    while (a != e) {
      Actor* d = static_cast<Actor*>(a);
      a = a->next_delete();
      d->~Actor();
    }
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
	vars[vti] = NULL; vtd[vti].proc->process(this,vs);
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
  Space::propagators(void) {
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
	  p->unlink_delete();
	  delete p;
	  mm.reuse(reinterpret_cast<MemoryManager::ReuseChunk*>(p));
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
      }
    }
    return pn;
  }


  /*
   * Performing branching after propagation
   *
   */

  unsigned int
  Space::branchings(void) {
    while (b_fst != &a_actors) {
      unsigned int alt = b_fst->branch();
      if (alt > 0)
	return alt;
      Branching* b = b_fst;
      b_fst = static_cast<Branching*>(b->next());
      b->unlink();
      b->unlink_delete();
      delete b;
      mm.reuse(reinterpret_cast<MemoryManager::ReuseChunk*>(b));
    }
    return 0;
  }




  /*
   * Main control for propagation and branching
   *  - a space only propagates and branches if requested by
   *    either a status, commit, ot clone operation
   *  - for all of the operations the number of propagation
   *    steps performed is returned in the last (optional)
   *    reference argument
   *
   */

  SpaceStatus
  Space::status(unsigned int& a, unsigned long int& pn) {
    // Perform propagation and do not continue when failed
    pn += propagators();
    if (failed())
      return SS_FAILED;
    // Find out how many alternatives the next branching provides
    // No alternatives means that the space is solved
    a = branchings();
    return (a > 0) ? SS_BRANCH : SS_SOLVED;
  }

  void
  Space::commit(unsigned int a, BranchingDesc* d,
		unsigned long int& pn) {
    if (d == NULL) {
      // If no branching description is provided, the first step
      // is to perform propagation and also run the branchings
      // in order to find out whether committing is actually possible
      pn += propagators();
      if (failed())
	throw SpaceFailed("Space::commit");
      unsigned int alt = branchings();
      if (alt == 0)
	throw SpaceNoBranching();
      if (a >= alt)
	throw SpaceIllegalAlternative();
      assert(b_fst != NULL);
      // Perform branching proper
      if (b_fst->commit(this,a,NULL) == ES_FAILED)
	fail();
    } else {
      if (failed())
	throw SpaceFailed("Space::commit");
      // Invariant for committing with BranchingDescs:
      // * completeness: if there is more than one distributor,
      //                 before committing to a description for the
      //                 second distributor, you have to commit to as
      //                 many descriptions of the first to make it disappear
      // This might still be incorrect if propagators create new distributors.
      while (d->id != b_fst->id) {
	Branching* b = b_fst;
	b_fst = static_cast<Branching*>(b_fst->next());
	b->unlink();
	b->unlink_delete();
	delete b;
      }
      if (b_fst->commit(this,a,d) == ES_FAILED)
	fail();
    }
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
    // Setup branching pointer
    if (s.b_fst == &s.a_actors) {
      b_fst = static_cast<Branching*>(&a_actors);
    } else {
      b_fst = static_cast<Branching*>(s.b_fst->prev());
    }
  }


  /*
   * Stage 2: updating variables
   *
   */

  Space*
  Space::clone(bool share, unsigned long int& pn) {
    pn += propagators();
    if (failed())
      throw SpaceFailed("Space::clone");
    // Start stage one
    Space* c = copy(share);
    // Stage 2: update variables
    for (int vti=VTI_LAST; vti--; ) {
      VarBase* vs = c->vars[vti];
      if (vs != NULL) {
	c->vars[vti] = NULL; vtd[vti].proc->update(c,vs);
      }
    }
    // Re-establish prev links (reset forwarding information)
    ActorLink* p = &a_actors;
    ActorLink* a = p->next();
    while (a != &a_actors) {
      a->prev(p); p = a; a = a->next();
    }
    assert(a->prev() == p);
    return c;
  }

}

// STATISTICS: kernel-core

