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

#include "gecode/int/distinct.hh"

#include <cmath>
#include <algorithm>

namespace {

  IntSet ds(0,5);

  class AD : public NaryPropagator<IntView,PC_INT_NONE> {
  protected:
    using NaryPropagator<IntView,PC_INT_NONE>::x;
    Council<ViewAdvisor<IntView> > c;
    /// Constructor for posting
    AD(Space* home, ViewArray<IntView>& x)
      : NaryPropagator<IntView,PC_INT_NONE>(home,x), c(home) {
      for (int i=x.size(); i--; ) {
        (void) new (home) ViewAdvisor<IntView>(home,this,c,x[i]);
        if (x[i].assigned())
          IntView::schedule(home,this,ME_INT_VAL);
      }
    }
    /// Constructor for cloning \a p
    AD(Space* home, bool share, AD& p) 
      : NaryPropagator<IntView,PC_INT_NONE>(home,share,p) {
      c.update(home,share,p.c);
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share) {
      return new (home) AD(home,share,*this);
    }
    virtual ExecStatus advise(Space* home, Advisor* _a, const Delta* d) {
      ViewAdvisor<IntView>* a = static_cast<ViewAdvisor<IntView>*>(_a);
      ModEvent me = IntView::modevent(d);
      if (me == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home,c);
      return ES_FIX;
    }
    /// Perform propagation
    virtual ExecStatus propagate(Space* home) {
      GECODE_ES_CHECK((Gecode::Int::Distinct::prop_val<IntView,true>(home,x)));
      return (x.size() < 2) ? ES_SUBSUMED(this,home) : ES_FIX;
    }
    /// Post propagator for view array \a x
    static ExecStatus post(Space* home, ViewArray<IntView>& x) {
      if (x.size() > 1)
        (void) new (home) AD(home,x);
      return ES_OK;
    }
    size_t dispose(Space* home) {
      c.dispose(home);
      (void) NaryPropagator<IntView,PC_INT_NONE>::dispose(home);
      return sizeof(*this);
    }
  };

  class AdviseDistinct : public IntTest {
  public:
    AdviseDistinct(void)
      : IntTest("Advisor::Distinct",5,ds,false,ICL_DEF) {}
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<x.size(); i++)
        for (int j=i+1; j<x.size(); j++)
          if (x[i]==x[j])
            return false;
      return true;
    }
    virtual void post(Space* home, IntVarArray& x) {
      if (home->failed())
        return;
      ViewArray<IntView> xv(home,x.size());
      for (int i=x.size(); i--; )
        xv[i]=x[i];
      AD::post(home,xv);
    }
  };

  AdviseDistinct _ad;

  IntSet s(-5,5);

  class Eq : public Propagator {
  protected:

    class BndAdvisor : public ViewAdvisor<IntView> {
      using ViewAdvisor<IntView>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, 
                 Council<BndAdvisor>& c, IntView v) 
        : ViewAdvisor<IntView>(home,p,c,v) {
        if (x.assigned())
          schedule(home, Int::ME_INT_VAL);
        else
          schedule(home, Int::ME_INT_BND);
      }
      BndAdvisor(Space* home, bool share, BndAdvisor& a) 
        : ViewAdvisor<IntView>(home,share,a) {}
    };
    
    Council<BndAdvisor> c;
    IntView x0, x1;

    virtual ExecStatus advise(Space* home, Advisor* _a, const Delta* d) {
      BndAdvisor* a = static_cast<BndAdvisor*>(_a);
      ModEvent me = IntView::modevent(d);
      if (me == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home,c);
      return ES_NOFIX;
    }

    /// Constructor for cloning \a p
    Eq(Space* home, bool share, Eq& p) 
      : Propagator(home, share, p) {
      c.update(home,share,p.c);
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }

    /// Constructor for posting
    Eq(Space* home, IntView _x0, IntView _x1) 
      : Propagator(home), c(home), x0(_x0), x1(_x1) {
      (void) new (home) BndAdvisor(home, this, c, x0);
      (void) new (home) BndAdvisor(home, this, c, x1);
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share) {
      return new (home) Eq(home, share, *this);
    }
    virtual PropCost cost(void) const { 
      return PC_BINARY_LO; 
    }
    /// Perform propagation
    virtual ExecStatus propagate(Space* home) {
      while ((x0.min() != x1.min()) &&
             (x0.max() != x1.max())) {
        GECODE_ME_CHECK(x0.lq(home, x1.max()));
        GECODE_ME_CHECK(x0.gq(home, x1.min()));
        GECODE_ME_CHECK(x1.lq(home, x0.max()));
        GECODE_ME_CHECK(x1.gq(home, x0.min()));
      }
      if (x0.assigned() && x1.assigned())
        return ES_SUBSUMED(this, dispose(home));
      return ES_NOFIX;
    }
    /// Post bounds-consistent propagator \f$ x_0=x_1\f$
    static  ExecStatus  post(Space* home, IntView x0, IntView x1) {
      (void) new (home) Eq(home, x0, x1);
      return ES_OK;
    }
    size_t
    dispose(Space* home) {
      c.dispose(home);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  class BasicIntAdvisor : public IntTest {
  public:
    BasicIntAdvisor(void)
      : IntTest("Advisor::Eq::Int",2,s) {}
    virtual bool solution(const Assignment& x) const {
      return x[0] == x[1];
    }
    virtual void post(Space* home, IntVarArray& x) {
      Int::IntView x0(x[0]), x1(x[1]);
      Eq::post(home, x0, x1);
    }
  };

  BasicIntAdvisor _basicintadvisor;

  IntSet bs(0,1);

  class BoolEq : public Propagator {
  protected:
    class BndAdvisor : public ViewAdvisor<BoolView> {
      using ViewAdvisor<BoolView>::x;
    public:
      BndAdvisor(Space* home, Propagator* p, Council<BndAdvisor>& c, 
                 BoolView v) 
        : ViewAdvisor<BoolView>(home,p,c,v) {
        if (x.assigned())
          schedule(home, Int::ME_BOOL_VAL);
      }
      BndAdvisor(Space* home, bool share, BndAdvisor& a) 
        : ViewAdvisor<BoolView>(home,share,a) {}
    };
    
    Council<BndAdvisor> c;

    BoolView x0, x1;
    virtual ExecStatus advise(Space* home, Advisor* _a, const Delta* d) {
      BndAdvisor* a = static_cast<BndAdvisor*>(_a);
      return ES_SUBSUMED_NOFIX(a,home,c);
    }

    /// Constructor for cloning \a p
    BoolEq(Space* home, bool share, BoolEq& p) 
      : Propagator(home, share, p) {
      c.update(home,share,p.c);
      x0.update(home,share,p.x0);
      x1.update(home,share,p.x1);
    }
    /// Constructor for posting
    BoolEq(Space* home, BoolView _x0, BoolView _x1) 
      : Propagator(home), c(home), x0(_x0), x1(_x1) {
      (void) new (home) BndAdvisor(home, this, c, x0);
      (void) new (home) BndAdvisor(home, this, c, x1);
    }
  public:
    /// Copy propagator during cloning
    virtual Actor* copy(Space* home, bool share) {
      return new (home) BoolEq(home, share, *this);
    }
    virtual PropCost cost(void) const { 
      return PC_BINARY_LO; 
    }
    /// Perform propagation
    virtual ExecStatus propagate(Space* home) {
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
      c.dispose(home);
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  class BasicBoolAdvisor : public IntTest {
  public:
    BasicBoolAdvisor(void)
      : IntTest("Advisor::Eq::Bool",2,bs) {}
    virtual bool solution(const Assignment& x) const {
      return x[0] == x[1];
    }
    virtual void post(Space* home, IntVarArray& x) {
      Int::BoolView x0(channel(home,x[0])), x1(channel(home,x[1]));
      BoolEq::post(home, x0, x1);
    }
  };

  BasicBoolAdvisor _basicbooladvisor;
}


// STATISTICS: test-int
