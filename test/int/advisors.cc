/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
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

#include "test/int.hh"
#include "gecode/minimodel.hh"
#include "test/log.hh"

#include <cmath>
#include <algorithm>

#if GECODE_USE_ADVISORS

namespace {
  IntSet s(-5,5);

  class Eq : public Propagator {
    class BndAdvisor : public IntUnaryViewAdvisor<IntView> {
      PropagatorPointer pp;
      using IntUnaryViewAdvisor<IntView>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, IntView v) 
        : IntUnaryViewAdvisor<IntView>(home,p,v), pp(p) {
        if (x.assigned()) {
          pp.schedule<VTI_INT,IntMeDiff>(home, Int::ME_INT_VAL);
        }
      }
      BndAdvisor(Space* home, Propagator* p, bool share, BndAdvisor& d) 
        : IntUnaryViewAdvisor<IntView>(home, p, share, d), pp(p) {}
      Advisor *copy(Space *home, Propagator* p, bool share) {
        return new (home) BndAdvisor(home, p, share, *this); 
      }
      size_t dispose(Space* home) {
        (void) IntUnaryViewAdvisor<IntView>::dispose(home);
        return sizeof(*this);
      }
      ExecStatus advise(Space *home, ModEvent me, int lo, int hi) {
        if (me == ME_INT_VAL || me == ME_INT_BND) {
          pp.schedule<VTI_INT,IntMeDiff>(home, me);
        }
        if (me == ME_INT_VAL) return ES_SUBSUMED(this,sizeof(*this));
        return ES_OK;
      }
    };
    
  public:
    typedef DynamicAdvisorCollection<BndAdvisor> DC;
  private:
    DC dc;
  protected:
    IntView x0, x1;

    /// Constructor for cloning \a p
    Eq(Space* home, bool share, Eq& p) 
      : Propagator(home, share, p),
        dc(home, this, share, p.dc) {
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }
    /// Constructor for posting
    Eq(Space* home, IntView _x0, IntView _x1) 
      : Propagator(home),
        dc(home, this, 2),
        x0(_x0), x1(_x1) {
      BndAdvisor *bd0 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd1 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd0);
      dc.add(home, this, bd1);

      BndAdvisor *bd2 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd3 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd2);
      dc.add(home, this, bd3);


      BndAdvisor *bd4 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd5 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd4);
      dc.add(home, this, bd5);

      bd2->dispose(home);

      BndAdvisor *bd6 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd7 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd6);
      dc.add(home, this, bd7);


      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      BndAdvisor *bd8 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd9 = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bda = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdb = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bdc = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdd = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bde = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdf = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd8);
      dc.add(home, this, bd9);
      dc.add(home, this, bda);
      dc.add(home, this, bdb);
      dc.add(home, this, bdc);
      dc.add(home, this, bdd);
      dc.add(home, this, bde);
      dc.add(home, this, bdf);
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share) {
      Actor* res =  new (home) Eq(home, share, *this);
      return res;
    }
    virtual PropCost cost(void) const { return PC_BINARY_LO; }
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home) {
#define GECODE_ME_MOD(me) {\
  ModEvent m = me; \
  if (me_failed(m))  { \
      return ES_FAILED; \
  } \
  if (me_modified(m)) mod |= true; \
}
      bool mod = false;
      do {
        mod = false;

        GECODE_ME_MOD(x0.lq(home, x1.max()));
        GECODE_ME_MOD(x0.gq(home, x1.min()));
        GECODE_ME_MOD(x1.lq(home, x0.max()));
        GECODE_ME_MOD(x1.gq(home, x0.min()));
        if (x0.assigned() && x1.assigned()) {
          return ES_SUBSUMED(this, dispose(home));
        }
      } while(mod);

#undef GECODE_ME_MOD
      return ES_NOFIX;
    }
    /// Post bounds-consistent propagator \f$ x_0=x_1\f$
    static  ExecStatus  post(Space* home, IntView x0, IntView x1) {
      (void) new (home) Eq(home, x0, x1);
      return ES_OK;
    }

    size_t
    dispose(Space* home) {
      (void) dc.dispose(home);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };
}

class BasicIntAdvisor : public IntTest {
public:
  BasicIntAdvisor(void)
    : IntTest("Int::Advisor",2,s) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Int::IntView x0(x[0]), x1(x[1]);
    Eq::post(home, x0, x1);
  }
};

namespace {
  BasicIntAdvisor _basicintadvisor;
}




namespace {
  IntSet bs(0,1);


  class BoolEq : public Propagator {
    class BndAdvisor : public IntUnaryViewAdvisor<BoolView, Int::PC_BOOL_ADVISOR> {
      PropagatorPointer pp;
      using IntUnaryViewAdvisor<BoolView, Int::PC_BOOL_ADVISOR>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, BoolView v) 
        : IntUnaryViewAdvisor<BoolView, Int::PC_BOOL_ADVISOR>(home,p,v), pp(p) {
        if (x.assigned()) {
          pp.schedule<VTI_BOOL,BoolMeDiff>(home, Int::ME_BOOL_VAL);
        }
      }
      BndAdvisor(Space* home, Propagator* p, bool share, BndAdvisor& d) 
        : IntUnaryViewAdvisor<BoolView, Int::PC_BOOL_ADVISOR>(home, p, share, d), 
          pp(p) {}
      Advisor *copy(Space *home, Propagator* p, bool share) {
        return new (home) BndAdvisor(home, p, share, *this); 
      }
      size_t dispose(Space* home) {
        (void) IntUnaryViewAdvisor<BoolView, Int::PC_BOOL_ADVISOR>::dispose(home);
        return sizeof(*this);
      }
      ExecStatus advise(Space *home, ModEvent me, int lo, int hi) {
        pp.schedule<VTI_BOOL,BoolMeDiff>(home, me);
        return ES_SUBSUMED(this,sizeof(*this));
      }
    };
    
  public:
    typedef DynamicAdvisorCollection<BndAdvisor> DC;
  private:
    DC dc;
  protected:
    BoolView x0, x1;

    /// Constructor for cloning \a p
    BoolEq(Space* home, bool share, BoolEq& p) 
      : Propagator(home, share, p),
        dc(home, this, share, p.dc) {
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }
    /// Constructor for posting
    BoolEq(Space* home, BoolView _x0, BoolView _x1) 
      : Propagator(home),
        dc(home, this, 2),
        x0(_x0), x1(_x1) {
      BndAdvisor *bd0 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd1 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd0);
      dc.add(home, this, bd1);

      BndAdvisor *bd2 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd3 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd2);
      dc.add(home, this, bd3);


      BndAdvisor *bd4 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd5 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd4);
      dc.add(home, this, bd5);

      bd2->dispose(home);

      BndAdvisor *bd6 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd7 = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd6);
      dc.add(home, this, bd7);


      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      BndAdvisor *bd8 = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bd9 = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bda = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdb = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bdc = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdd = new (home) BndAdvisor(home, this, x1);
      BndAdvisor *bde = new (home) BndAdvisor(home, this, x0);
      BndAdvisor *bdf = new (home) BndAdvisor(home, this, x1);
      dc.add(home, this, bd8);
      dc.add(home, this, bd9);
      dc.add(home, this, bda);
      dc.add(home, this, bdb);
      dc.add(home, this, bdc);
      dc.add(home, this, bdd);
      dc.add(home, this, bde);
      dc.add(home, this, bdf);
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share) {
      Actor* res =  new (home) BoolEq(home, share, *this);
      return res;
    }
    virtual PropCost cost(void) const { return PC_BINARY_LO; }
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home) {
      if (x1.assigned()) {
        if (me_failed(x0.eq(home, x1.val())))
          return ES_FAILED;
      } else {
        if (me_failed(x1.eq(home, x0.val())))
          return ES_FAILED;
      }

      return ES_SUBSUMED(this, dispose(home));
    }
    /// Post bounds-consistent propagator \f$ x_0=x_1\f$
    static  ExecStatus  post(Space* home, BoolView x0, BoolView x1) {
      (void) new (home) BoolEq(home, x0, x1);
      return ES_OK;
    }

    size_t
    dispose(Space* home) {
      (void) dc.dispose(home);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };
}

class BasicBoolAdvisor : public IntTest {
public:
  BasicBoolAdvisor(void)
    : IntTest("Bool::Advisor",2,bs) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Int::BoolView x0(channel(home,x[0])), x1(channel(home,x[1]));
    BoolEq::post(home, x0, x1);
  }
};

namespace {
  BasicBoolAdvisor _basicbooladvisor;
}

#endif

// STATISTICS: test-int
