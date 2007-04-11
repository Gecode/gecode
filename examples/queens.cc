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

  class NQC : public Propagator {
  protected:
    IntView x0;
    IntView x1;
    int c;
    Council<ViewAdvisor<IntView> > co;
    /// Constructor for cloning \a p
    NQC(Space* home, bool share, NQC& p);
    /// Constructor for creation
    NQC(Space* home, A x0, B x1, Val c);
  public:
    virtual ExecStatus 
    advise(Space* home, Advisor& a, const Delta& d) {
#if defined(BINLIN_NQ_ADVISOR_BASE)
      return ES_NOFIX;
#endif
#if defined(BINLIN_NQ_ADVISOR_AVOID)
      if (A::modevent(d) == ME_INT_VAL)
        return ES_NOFIX;
      return ES_FIX;
#endif
    }
    /// Constructor for rewriting \a p during cloning
    NQC(Space* home, bool share, Propagator& p, A x0, B x1, Val c);
    /// Create copy during cloning
    virtual Actor* copy(Space* home, bool share);
    /// Perform propagation
    virtual ExecStatus propagate(Space* home);
    /// Cost function (defined as PC_UNARY_LO)
    virtual PropCost cost(void) const;
    /// Post propagator for \f$x_0+x_1 \neq c\f$
    static ExecStatus post(Space* home, A x0, B x1, Val c);
    /// Delete propagator and return its size
    virtual size_t dispose(Space* home);
  };

  forceinline
  NQC<Val,A,B>::NQC(Space* home, A _x0, B _x1, Val _c)
    : Propagator(home), x0(_x0), x1(_x1), c(_c), co(home,2) {
    x0.subscribe(home, new (home) Advisor(home,this,co));
    x1.subscribe(home, new (home) Advisor(home,this,co));
    if (x0.assigned())
      A::schedule(home,this,ME_INT_VAL);
    if (x1.assigned())
      B::schedule(home,this,ME_INT_VAL);
  }

  ExecStatus
  NQC<Val,A,B>::post(Space* home, A x0, B x1, Val c) {
    /*
    if (x0.assigned()) {
      GECODE_ME_CHECK(x1.nq(home,c-x0.val()));
    } else if (x1.assigned()) {
      GECODE_ME_CHECK(x0.nq(home,c-x1.val()));
    } else {
      (void) new (home) NQC<Val,A,B>(home,x0,x1,c);
    }
    */
    (void) new (home) NQC<Val,A,B>(home,x0,x1,c);
    return ES_OK;
  }


  template <class Val, class A, class B>
  forceinline
  NQC<Val,A,B>::NQC(Space* home, bool share, Propagator& p,
                        A y0, B y1, Val c0)
    : Propagator(home,share,p), c(c0), co(home,share,co) {
    std::cout << "AAARGH" << std::endl;
    assert(false);
    x0.update(home,share,y0);
    x1.update(home,share,y1);
  }

  template <class Val, class A, class B>
  forceinline size_t
  NQC<Val,A,B>::dispose(Space* home) {
    assert(!home->failed());
    co.dispose(home);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  template <class Val, class A, class B>
  forceinline
  NQC<Val,A,B>::NQC(Space* home, bool share, NQC<Val,A,B>& p)
    : Propagator(home,share,p), c(p.c), co(home,share,p.co) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
  }
  
  template <class Val, class A, class B>
  Actor*
  NQC<Val,A,B>::copy(Space* home, bool share) {
    return new (home) NQC<Val,A,B>(home,share,*this);
  }

  template <class Val, class A, class B>
  PropCost
  NQC<Val,A,B>::cost(void) const {
    return PC_UNARY_LO;
  }

  template <class Val, class A, class B>
  ExecStatus
  NQC<Val,A,B>::propagate(Space* home) {
    if (x0.assigned()) {
      GECODE_ME_CHECK(x1.nq(home,c-x0.val()));
      return ES_SUBSUMED(this,home);
    } 
    if (x1.assigned()) {
      GECODE_ME_CHECK(x0.nq(home,c-x1.val()));
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
          post(this, q[i]+i != q[j]+j);
          post(this, q[i]-i != q[j]-j);
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

