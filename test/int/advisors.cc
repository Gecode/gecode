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
    class BndAdvisor : public ViewAdvisor<IntView> {
      using ViewAdvisor<IntView>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, CouncilBase& c, IntView v) 
        : ViewAdvisor<IntView>(home,p,c,v) {
        if (x.assigned())
          schedule(home, Int::ME_INT_VAL);
      }
      BndAdvisor(Space* home, bool share, BndAdvisor& a) 
        : ViewAdvisor<IntView>(home, share, a) {}
      virtual ExecStatus advise(Space* home, const Delta& d) {
        ModEvent me = IntView::modevent(d);
        if (me == ME_INT_VAL || me == ME_INT_BND)
          schedule(home, me);
        if (me == ME_INT_VAL) return ES_SUBSUMED(this,sizeof(*this));
        return ES_OK;
      }
    };
    
  public:
    typedef Council<BndAdvisor> DC;
  private:
    DC dc;
  protected:
    IntView x0, x1;

    /// Constructor for cloning \a p
    Eq(Space* home, bool share, Eq& p) 
      : Propagator(home, share, p),
        dc(home, share, p.dc) {
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }
    /// Constructor for posting
    Eq(Space* home, IntView _x0, IntView _x1) 
      : Propagator(home),
        dc(home, 2),
        x0(_x0), x1(_x1) {
      (void) new (home) BndAdvisor(home, this, dc, x0);
      BndAdvisor* bd1 = new (home) BndAdvisor(home, this, dc, x1);

      BndAdvisor* bd2 = new (home) BndAdvisor(home, this, dc, x0);
      BndAdvisor* bd3 = new (home) BndAdvisor(home, this, dc, x1);

      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);

      bd2->dispose(home);

      BndAdvisor* bd7 = new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);

      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
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
    class BndAdvisor : public ViewAdvisor<BoolView> {
      using ViewAdvisor<BoolView>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, CouncilBase& c, BoolView v) 
        : ViewAdvisor<BoolView>(home,p,c,v) {
        if (x.assigned())
          schedule(home, Int::ME_BOOL_VAL);
      }
      BndAdvisor(Space* home, bool share, BndAdvisor& a) 
        : ViewAdvisor<BoolView>(home, share, a) {}
      virtual ExecStatus advise(Space* home, const Delta& d) {
        schedule(home, BoolView::modevent(d));
        return ES_SUBSUMED(this,sizeof(*this));
      }
    };
    
  public:
    typedef Council<BndAdvisor> DC;
  private:
    DC dc;
  protected:
    BoolView x0, x1;

    /// Constructor for cloning \a p
    BoolEq(Space* home, bool share, BoolEq& p) 
      : Propagator(home, share, p),
        dc(home, share, p.dc) {
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }
    /// Constructor for posting
    BoolEq(Space* home, BoolView _x0, BoolView _x1) 
      : Propagator(home),
        dc(home, 2),
        x0(_x0), x1(_x1) {
      (void) new (home) BndAdvisor(home, this, dc, x0);
      BndAdvisor* bd1 = new (home) BndAdvisor(home, this, dc, x1);

      BndAdvisor* bd2 = new (home) BndAdvisor(home, this, dc, x0);
      BndAdvisor* bd3 = new (home) BndAdvisor(home, this, dc, x1);

      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);

      bd2->dispose(home);

      BndAdvisor* bd7 = new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);

      bd1->dispose(home);
      bd3->dispose(home);
      bd7->dispose(home);

      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
      (void) new (home) BndAdvisor(home, this, dc, x0);
      (void) new (home) BndAdvisor(home, this, dc, x1);
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
