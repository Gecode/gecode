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

#if GECODE_USE_DEMONS

namespace {
  IntSet s(-5,5);

  class Eq : public Propagator {
    class BndDemon : public IntUnaryViewDemon<IntView> {
      PropagatorPointer pp;
      using IntUnaryViewDemon<IntView>::_v;
    public:
      BndDemon(Space* home, Propagator* p, IntView v) 
        : IntUnaryViewDemon<IntView>(home,p,v), pp(p) {
        if (_v.assigned()) {
          pp.schedule<VTI_INT,IntMeDiff>(home, Int::ME_INT_VAL);
          derr << "Posting on assigned val, scheduled p with v=" << _v.val() << std::endl;
        }
      }
      BndDemon(Space* home, Propagator* p, bool share, BndDemon& d) 
        : IntUnaryViewDemon<IntView>(home, p, share, d), pp(p) {}
      Demon *copy(Space *home, Propagator* p, bool share) {
        return new (home) BndDemon(home, p, share, *this); 
      }
      size_t dispose(Space* home) {
        derr << "Disposing demon" << std::endl;
        (void) IntUnaryViewDemon<IntView>::dispose(home);
        return sizeof(*this);
      }
    private:
      ExecStatus _propagate(Space *home, ModEvent me, int lo, int hi) {
        derr << "Demon invoked _v=" << _v << " lo=" << lo << " hi=" << hi << " me=" << me << std::endl;
        if (me == ME_INT_VAL || me == ME_INT_BND) {
          std::cerr << "Scheduling propagator" << std::endl;
          pp.schedule<VTI_INT,IntMeDiff>(home, me);
        }
        if (me == ME_INT_VAL) return ES_SUBSUMED(this,sizeof(*this));
        return ES_OK;
      }
    };
    
  public:
    typedef DynamicDemonCollection<BndDemon> DC;
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
      BndDemon *bd0 = new (home) BndDemon(home, this, x0);
      BndDemon *bd1 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd0);
      dc.add(home, this, bd1);

      BndDemon *bd2 = new (home) BndDemon(home, this, x0);
      BndDemon *bd3 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd2);
      dc.add(home, this, bd3);


      BndDemon *bd4 = new (home) BndDemon(home, this, x0);
      BndDemon *bd5 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd4);
      dc.add(home, this, bd5);

      bd2->dispose(home);

      BndDemon *bd6 = new (home) BndDemon(home, this, x0);
      BndDemon *bd7 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd6);
      dc.add(home, this, bd7);


      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      BndDemon *bd8 = new (home) BndDemon(home, this, x0);
      BndDemon *bd9 = new (home) BndDemon(home, this, x1);
      BndDemon *bda = new (home) BndDemon(home, this, x0);
      BndDemon *bdb = new (home) BndDemon(home, this, x1);
      BndDemon *bdc = new (home) BndDemon(home, this, x0);
      BndDemon *bdd = new (home) BndDemon(home, this, x1);
      BndDemon *bde = new (home) BndDemon(home, this, x0);
      BndDemon *bdf = new (home) BndDemon(home, this, x1);
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
      derr << "Copying actor..." << std::endl;
      Actor* res =  new (home) Eq(home, share, *this);
      derr << "...copying finished" << std::endl;
      return res;
    }
    virtual PropCost cost(void) const { return PC_BINARY_LO; }
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home) {
      std::cerr << "Eq: Propagating x0=" << x0 << " x1=" << x1 << std::endl;

#define GECODE_ME_MOD(me) {\
  ModEvent m = me; \
  if (me_failed(m))  { \
      derr << "Eq: Failure with x0=" << x0 << " x1=" << x1 << std::endl; \
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
          derr << "Eq: Propagation gave x0=" << x0 
               << " x1=" << x1 << ", subsuming" << std::endl;
          return ES_SUBSUMED(this, dispose(home));
        }
      } while(mod);

#undef GECODE_ME_MOD
      derr << "Eq: Propagation gave x0=" << x0 << " x1=" << x1 << std::endl;
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

class BasicIntDemon : public IntTest {
public:
  BasicIntDemon(void)
    : IntTest("Int::Demon",2,s) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Int::IntView x0(x[0]), x1(x[1]);
    derr << std::endl;
    Eq::post(home, x0, x1);
    derr << "Posting finished" << std::endl;
  }
};

namespace {
  BasicIntDemon _basicintdemon;
}




namespace {
  IntSet bs(0,1);


  class BoolEq : public Propagator {
    class BndDemon : public IntUnaryViewDemon<BoolView, Int::PC_BOOL_DEMON> {
      PropagatorPointer pp;
      using IntUnaryViewDemon<BoolView, Int::PC_BOOL_DEMON>::_v;
    public:
      BndDemon(Space* home, Propagator* p, BoolView v) 
        : IntUnaryViewDemon<BoolView, Int::PC_BOOL_DEMON>(home,p,v), pp(p) {
        if (_v.assigned()) {
          pp.schedule<VTI_BOOL,BoolMeDiff>(home, Int::ME_BOOL_VAL);
          derr << "Posting on assigned val, scheduled p with v=" << _v.val() << std::endl;
        }
      }
      BndDemon(Space* home, Propagator* p, bool share, BndDemon& d) 
        : IntUnaryViewDemon<BoolView, Int::PC_BOOL_DEMON>(home, p, share, d), pp(p) {}
      Demon *copy(Space *home, Propagator* p, bool share) {
        return new (home) BndDemon(home, p, share, *this); 
      }
      size_t dispose(Space* home) {
        derr << "Disposing demon" << std::endl;
        (void) IntUnaryViewDemon<BoolView, Int::PC_BOOL_DEMON>::dispose(home);
        return sizeof(*this);
      }
    private:
      ExecStatus _propagate(Space *home, ModEvent me, int lo, int hi) {
        derr << "Demon invoked _v=" << _v << " lo=" << lo << " hi=" << hi << " me=" << me << std::endl;
        std::cerr << "Scheduling propagator" << std::endl;
        pp.schedule<VTI_BOOL,BoolMeDiff>(home, me);
        return ES_SUBSUMED(this,sizeof(*this));
      }
    };
    
  public:
    typedef DynamicDemonCollection<BndDemon> DC;
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
      BndDemon *bd0 = new (home) BndDemon(home, this, x0);
      BndDemon *bd1 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd0);
      dc.add(home, this, bd1);

      BndDemon *bd2 = new (home) BndDemon(home, this, x0);
      BndDemon *bd3 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd2);
      dc.add(home, this, bd3);


      BndDemon *bd4 = new (home) BndDemon(home, this, x0);
      BndDemon *bd5 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd4);
      dc.add(home, this, bd5);

      bd2->dispose(home);

      BndDemon *bd6 = new (home) BndDemon(home, this, x0);
      BndDemon *bd7 = new (home) BndDemon(home, this, x1);
      dc.add(home, this, bd6);
      dc.add(home, this, bd7);


      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      BndDemon *bd8 = new (home) BndDemon(home, this, x0);
      BndDemon *bd9 = new (home) BndDemon(home, this, x1);
      BndDemon *bda = new (home) BndDemon(home, this, x0);
      BndDemon *bdb = new (home) BndDemon(home, this, x1);
      BndDemon *bdc = new (home) BndDemon(home, this, x0);
      BndDemon *bdd = new (home) BndDemon(home, this, x1);
      BndDemon *bde = new (home) BndDemon(home, this, x0);
      BndDemon *bdf = new (home) BndDemon(home, this, x1);
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
      derr << "Copying actor..." << std::endl;
      Actor* res =  new (home) BoolEq(home, share, *this);
      derr << "...copying finished" << std::endl;
      return res;
    }
    virtual PropCost cost(void) const { return PC_BINARY_LO; }
    /// Perform propagation
    virtual ExecStatus  propagate(Space* home) {
      std::cerr << "BoolEq: Propagating x0=" << x0 << " x1=" << x1 << std::endl;

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

class BasicBoolDemon : public IntTest {
public:
  BasicBoolDemon(void)
    : IntTest("Bool::Demon",2,bs) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Int::BoolView x0(link(home,x[0])), x1(link(home,x[1]));
    derr << std::endl;
    BoolEq::post(home, x0, x1);
    derr << "Posting finished" << std::endl;
  }
};

namespace {
  BasicBoolDemon _basicbooldemon;
}

#endif

// STATISTICS: test-int
