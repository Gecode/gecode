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

#include <gecode/kernel.hh>

namespace Gecode {

  /*
   * Variable type disposer
   *
   */
  void
  VarDisposerBase::dispose(Space&,VarImpBase*) {}

  VarDisposerBase::~VarDisposerBase(void) {}



  /*
   * Actor
   *
   */
  size_t
  Actor::allocated(void) const {
    return 0;
  }

#ifdef __GNUC__
  /// To avoid warnings from GCC
  Actor::~Actor(void) {}
#endif



  /*
   * Propagator
   *
   */
  ExecStatus
  Propagator::advise(Space&, Advisor&, const Delta&) {
    assert(false);
    return ES_FAILED;
  }



  /*
   * Space: Misc
   *
   */
  unsigned long int Space::unused_uli;
  bool Space::unused_b;
  StatusStatistics Space::unused_status;
  CloneStatistics Space::unused_clone;
  CommitStatistics Space::unused_commit;

#ifdef GECODE_HAS_VAR_DISPOSE
  VarDisposerBase* Space::vd[AllVarConf::idx_d];
#endif

  Space::Space(void)
    : sra(new SharedRegionArea), n_wmp(0) {
#ifdef GECODE_HAS_VAR_DISPOSE
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
#endif
    // Initialize propagator and branching links
    pl.init();
    bl.init();
    b_status = b_commit = Branching::cast(&bl);
    // Initialize array for forced deletion to be empty
    d_fst = d_cur = d_lst = NULL;
    // Initialize propagator queues
    pc.p.active = &pc.p.queue[0]-1;
    for (int i=0; i<=PropCost::AC_MAX; i++)
      pc.p.queue[i].init();
    pc.p.branch_id = 0;
    pc.p.n_sub = 0;
  }

  void
  Space::d_resize(void) {
    if (d_fst == NULL) {
      // Create new array
      d_fst = alloc<Actor*>(4);
      d_cur = d_fst;
      d_lst = d_fst+4;
    } else {
      // Resize existing array
      unsigned int n = static_cast<unsigned int>(d_lst - d_fst);
      assert(n != 0);
      d_fst = realloc<Actor*>(d_fst,n,2*n);
      d_cur = d_fst+n;
      d_lst = d_fst+2*n;
    }
  }

  unsigned int
  Space::propagators(void) const {
    unsigned int n = 0;
    for (Propagators p(*this); p(); ++p)
      n++;
    return n;
  }

  unsigned int
  Space::branchings(void) const {
    unsigned int n = 0;
    for (Branchings b(*this); b(); ++b)
      n++;
    return n;
  }

  Space::~Space(void) {
    // Release region area
    if (sra->release())
      delete sra;
    // Mark space as failed
    fail();
    // Delete actors that must be deleted
    {
      Actor** a = d_fst;
      Actor** e = d_cur;
      // So that d_unforce knows that deletion is in progress
      d_fst = NULL;
      while (a < e) {
        (void) (*a)->dispose(*this);
        a++;
      }
    }
#ifdef GECODE_HAS_VAR_DISPOSE
    // Delete variables that were registered for disposal
    for (int i=AllVarConf::idx_d; i--;)
      if (_vars_d[i] != NULL)
        vd[i]->dispose(*this, _vars_d[i]);
#endif
  }



  /*
   * Space: propagation
   *
   */

  SpaceStatus
  Space::status(StatusStatistics& stat) {
    SpaceStatus s = SS_FAILED;
    if (failed()) {
      s = SS_FAILED; goto exit;
    }
    if (!stable()) {
      assert(pc.p.active >= &pc.p.queue[0]);
      Propagator* p;
      ModEventDelta med_o;
      goto unstable;
    execute:
      stat.propagate++;
      // Keep old modification event delta
      med_o = p->u.med;
      // Clear med but leave propagator in queue
      p->u.med = 0;
      switch (p->propagate(*this,med_o)) {
      case ES_FAILED:
        fail(); s = SS_FAILED; goto exit;
      case ES_NOFIX:
        // Find next, if possible
        if (p->u.med != 0) {
        unstable:
          // There is at least one propagator in a queue
          do {
            assert(pc.p.active >= &pc.p.queue[0]);
            // First propagator or link back to queue
            ActorLink* fst = pc.p.active->next();
            if (pc.p.active != fst) {
              p = Propagator::cast(fst);
              goto execute;
            }
            pc.p.active--;
          } while (true);
          GECODE_NEVER;
        }
        // Fall through
      case ES_FIX:
        // Clear med and put into idle queue
        p->u.med = 0; p->unlink(); pl.head(p);
      stable_or_unstable:
        // There might be a propagator in the queue
        do {
          assert(pc.p.active >= &pc.p.queue[0]);
          // First propagator or link back to queue
          ActorLink* fst = pc.p.active->next();
          if (pc.p.active != fst) {
            p = Propagator::cast(fst);
            goto execute;
          }
        } while (--pc.p.active >= &pc.p.queue[0]);
        assert(pc.p.active < &pc.p.queue[0]);
        goto stable;
      case __ES_SUBSUMED:
        p->unlink(); rfree(p,p->u.size);
        goto stable_or_unstable;
      case __ES_PARTIAL:
        // Schedule propagator with specified propagator events
        assert(p->u.med != 0);
        enqueue(p);
        goto unstable;
      default:
        GECODE_NEVER;
      }
    }
  stable:
    /*
     * Find the next branching that has still alternatives left
     *
     * It is important to note that branchings reporting to have no more
     * alternatives left can not be deleted. They can not be deleted
     * as there might be branching descriptions to be used in commit
     * that refer to one of these branchings. This e.g. happens when
     * we combine branch-and-bound search with adaptive recomputation: during
     * recomputation, a copy is constrained to be better than the currently
     * best solution, then the first half of the BranchingDescs is posted,
     * and a fixpoint computed (for storing in the middle of the path). Then
     * the remaining BranchingDescs are posted, and because of the additional
     * constraints that the space must be better than the previous solution,
     * the corresponding Branchings may already have no alternatives left.
     *
     * A branching reporting that no more alternatives exist will eventually
     * be deleted in commit. It will be deleted if the first branching
     * description is used in commit that does not refer to this branching.
     * As we insist that branching descriptions are used in order of
     * creation, all further branching descriptions cannot refer to this
     * branching.
     *
     */
    while (b_status != Branching::cast(&bl)) {
      if (b_status->status(*this)) {
        s = SS_BRANCH; goto exit;
      }
      b_status = Branching::cast(b_status->next());
    }
    // No branching with alternatives left, space is solved
    s = SS_SOLVED;
  exit:
    stat.wmp = (n_wmp > 0);
    if (n_wmp == 1) n_wmp = 0;
    return s;
  }


  void
  Space::_commit(const BranchingDesc& d, unsigned int a) {
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
    while ((b_commit != Branching::cast(&bl)) &&
           (d._id != b_commit->id)) {
      Branching* b = b_commit;
      b_commit = Branching::cast(b_commit->next());
      if (b == b_status)
        b_status = b_commit;
      b->unlink();
      rfree(b,b->dispose(*this));
    }
    if (b_commit == Branching::cast(&bl))
      throw SpaceNoBranching();
    if (a >= d.alternatives())
      throw SpaceIllegalAlternative();
    if (b_commit->commit(*this,d,a) == ES_FAILED)
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
    : mm(s.mm,s.pc.p.n_sub*sizeof(Propagator**)),
      sra(s.sra->copy(share)), n_wmp(s.n_wmp) {
#ifdef GECODE_HAS_VAR_DISPOSE
    for (int i=0; i<AllVarConf::idx_d; i++)
      _vars_d[i] = NULL;
#endif
    for (int i=0; i<AllVarConf::idx_c; i++)
      pc.c.vars_u[i] = NULL;
    pc.c.vars_noidx = NULL;
    pc.c.copied = NULL;
    // Copy all propagators
    {
      ActorLink* p = &pl;
      ActorLink* e = &s.pl;
      for (ActorLink* a = e->next(); a != e; a = a->next()) {
        Actor* c = Actor::cast(a)->copy(*this,share);
        // Link copied actor
        p->next(ActorLink::cast(c)); ActorLink::cast(c)->prev(p);
        // Note that forwarding is done in the constructors
        p = c;
      }
      // Link last actor
      p->next(&pl); pl.prev(p);
    }
    // Copy all branchings
    {
      ActorLink* p = &bl;
      ActorLink* e = &s.bl;
      for (ActorLink* a = e->next(); a != e; a = a->next()) {
        Actor* c = Actor::cast(a)->copy(*this,share);
        // Link copied actor
        p->next(ActorLink::cast(c)); ActorLink::cast(c)->prev(p);
        // Note that forwarding is done in the constructors
        p = c;
      }
      // Link last actor
      p->next(&bl); bl.prev(p);
    }
    // Setup array for actor disposal
    {
      unsigned int n = static_cast<unsigned int>(s.d_cur - s.d_fst);
      if (n == 0) {
        // No actors
        d_fst = d_cur = d_lst = NULL;
      } else {
        // Leave one entry free
        d_fst = alloc<Actor*>(n+1);
        d_cur = d_fst+n;
        d_lst = d_cur+1;
        do {
          n--;
          d_fst[n] = Actor::cast(s.d_fst[n]->prev());
        } while (n != 0);
      }
    }
    // Setup branching pointers
    if (s.b_status == &s.bl) {
      b_status = Branching::cast(&bl);
    } else {
      b_status = Branching::cast(s.b_status->prev());
    }
    if (s.b_commit == &s.bl) {
      b_commit = Branching::cast(&bl);
    } else {
      b_commit = Branching::cast(s.b_commit->prev());
    }
  }

  Space*
  Space::_clone(bool share) {
    if (failed())
      throw SpaceFailed("Space::clone");
    if (!stable())
      throw SpaceNotStable("Space::clone");

    // Copy all data structures (which in turn will invoke the constructor)
    Space* c = copy(share);

    // Update variables without indexing structure
    VarImp<NoIdxVarImpConf>* x =
      static_cast<VarImp<NoIdxVarImpConf>*>(c->pc.c.vars_noidx);
    while (x != NULL) {
      VarImp<NoIdxVarImpConf>* n = x->next();
      x->base = NULL; x->u.idx[0] = 0;
      x = n;
    }
    // Update variables with indexing structure
    c->update(static_cast<ActorLink**>(c->mm.subscriptions()));

    // Re-establish prev links (reset forwarding information)
    {
      ActorLink* p_a = &pl;
      ActorLink* c_a = p_a->next();
      // First update propagators and advisors
      while (c_a != &pl) {
        Propagator* p = Propagator::cast(c_a);
        if (p->u.advisors != NULL) {
          ActorLink* a = p->u.advisors;
          p->u.advisors = NULL;
          do {
            a->prev(p); a = a->next();
          } while (a != NULL);
        }
        c_a->prev(p_a); p_a = c_a; c_a = c_a->next();
      }
    }
    {
      ActorLink* p_a = &bl;
      ActorLink* c_a = p_a->next();
      // Update branchings
      while (c_a != &bl) {
        c_a->prev(p_a); p_a = c_a; c_a = c_a->next();
      }
    }

    // Reset links for copied objects
    for (CopiedHandle::Object* s = c->pc.c.copied; s != NULL; s = s->next)
      s->fwd = NULL;

    // Initialize propagator queue
    c->pc.p.active = &c->pc.p.queue[0]-1;
    for (int i=0; i<=PropCost::AC_MAX; i++)
      c->pc.p.queue[i].init();
    // Copy propagation only data
    c->pc.p.n_sub = pc.p.n_sub;
    c->pc.p.branch_id = pc.p.branch_id;
    return c;
  }

  void
  Space::constrain(const Space&) {
    throw SpaceConstrainUndefined();
  }

}

// STATISTICS: kernel-core
