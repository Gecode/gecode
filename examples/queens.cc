/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

#include "examples/support.hh"
#include "gecode/minimodel.hh"

#if defined(BINLIN_NQ_ADVISOR_BASE) || defined(BINLIN_NQ_ADVISOR_AVOID)

using namespace Int;

  class NQC : public Propagator {
  protected:
    IntView x0;
    IntView x1;
    int c;
    Council<ViewAdvisor<IntView> > co;
    /// Constructor for cloning \a p
    NQC(Space* home, bool share, NQC& p);
    /// Constructor for creation
    NQC(Space* home, IntView x0, IntView x1, int c);
  public:
    virtual ExecStatus 
    advise(Space* home, Advisor& _a, const Delta& d) {
      ViewAdvisor<IntView>& a = static_cast<ViewAdvisor<IntView>&>(_a);
#if defined(BINLIN_NQ_ADVISOR_BASE)
      if (IntView::modevent(d) == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home);
      return ES_NOFIX;
#endif
#if defined(BINLIN_NQ_ADVISOR_AVOID)
      if (IntView::modevent(d) == ME_INT_VAL)
        return ES_SUBSUMED_NOFIX(a,home);
      return ES_FIX;
#endif
    }
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Post propagator for \f$x_0+x_1 \neq c\f$
    static ExecStatus post(Space* home, IntView x0, IntView x1, int c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  forceinline
  NQC::NQC(Space* home, IntView _x0, IntView _x1, int _c)
    : Propagator(home), x0(_x0), x1(_x1), c(_c), co(home,2) {
    new (home) ViewAdvisor<IntView>(home,this,co,x0);
    new (home) ViewAdvisor<IntView>(home,this,co,x1);
    if (x0.assigned() || x1.assigned())
      IntView::schedule(home,this,ME_INT_VAL);
  }

  ExecStatus
  NQC::post(Space* home, IntView x0, IntView x1, int c) {
    /*
    if (x0.assigned()) {
      GECODE_ME_CHECK(x1.nq(home,c-x0.val()));
    } else if (x1.assigned()) {
      GECODE_ME_CHECK(x0.nq(home,c-x1.val()));
    } else {
      (void) new (home) NQC(home,x0,x1,c);
    }
    */
    (void) new (home) NQC(home,x0,x1,c);
    return ES_OK;
  }


  forceinline size_t
  NQC::dispose(Space* home) {
    assert(!home->failed());
    co.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  forceinline
  NQC::NQC(Space* home, bool share, NQC& p)
    : Propagator(home,share,p), c(p.c), co(home,share,p.co) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
  }
  
  Actor*
  NQC::copy(Space* home, bool share) {
    return new (home) NQC(home,share,*this);
  }

  PropCost
  NQC::cost(void) const {
    return PC_UNARY_LO;
  }

  ExecStatus
  NQC::propagate(Space* home) {
    if (x0.assigned()) {
      GECODE_ME_CHECK(x1.nq(home,x0.val()-c));
      return ES_SUBSUMED(this,home);
    } 
    if (x1.assigned()) {
      GECODE_ME_CHECK(x0.nq(home,c+x1.val()));
      return ES_SUBSUMED(this,home);
    }
    return ES_FIX;
  }


#endif


/**
 * \brief %Example: n-%Queens puzzle
 *
 * Place n queens on an n times n chessboard such that they do not
 * attack each other.
 *
 * \ingroup ExProblem
 *
 */
class Queens : public Example {
protected:
  /// Position of queens on boards
  IntVarArray q;
public:
  /// The actual problem
  Queens(const Options& opt)
    : q(this,opt.size,0,opt.size-1) {
    const int n = q.size();
    if (opt.naive) {
      for (int i = 0; i<n; i++)
        for (int j = i+1; j<n; j++) {
          post(this, q[i] != q[j]);
#if defined(BINLIN_NQ_ADVISOR_BASE) || defined(BINLIN_NQ_ADVISOR_AVOID)
          NQC::post(this, q[i], q[j], j-i); 
          NQC::post(this, q[i], q[j], i-j); 
#else
          post(this, q[i]+i != q[j]+j);
          post(this, q[i]-i != q[j]-j);
#endif
        }
    } else {
      IntArgs c(n);
      for (int i = n; i--; ) c[i] = i;
      distinct(this, c, q, opt.icl);
      for (int i = n; i--; ) c[i] = -i;
      distinct(this, c, q, opt.icl);
      distinct(this, q, opt.icl);
    }
    branch(this, q, BVAR_SIZE_MIN, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  Queens(bool share, Queens& s) : Example(share,s) {
    q.update(this, share, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Queens(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < q.size(); i++) {
      std::cout << q[i] << ", ";
      if ((i+1) % 10 == 0)
        std::cout << std::endl << "\t";
    }
    std::cout << std::endl;
    }
};

/** \brief Main-function
 *  \relates Queens
 */
int
main(int argc, char** argv) {
  Options opt("Queens");
  opt.iterations = 500;
  opt.size       = 100;
  opt.parse(argc,argv);
  Example::run<Queens,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

