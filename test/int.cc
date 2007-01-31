/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
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
#include "test/log.hh"
#include <algorithm>

Assignment::Assignment(int n0, const IntSet& d0)
  : n(n0), dsv(new IntSetValues[n]), d(d0) {
  reset();
}

void
Assignment::reset(void) {
  done = false;
  for (int i=n; i--; )
    dsv[i].init(d);
}

void
Assignment::operator++(void) {
  int i = n-1;
  while (true) {
    ++dsv[i];
    if (dsv[i]())
      return;
    dsv[i].init(d);
    --i;
    if (i<0) {
      done = true;
      return;
    }
  }
}

RandomAssignment::RandomAssignment(int n0, const IntSet& d0, int count0)
  : Assignment(n0, d0), vals(new int[n0]),
    count(count0), left(count0) {
  reset();
}

void
RandomAssignment::reset(void) {
  left = count;
  for (int i=n; i--; )
    vals[i] = randval();
}

void
RandomAssignment::operator++(void) {
  for (int i = n; i--; )
    vals[i] = randval();
  --left;
}

int
RandomAssignment::randval(void) {
  int v;
  IntSetRanges it(d);
  unsigned int skip = Test::randgen(d.size());
  while (true) {
    if (it.width() > skip) {
      v = it.min() + skip;
      break;
    }
    skip -= it.width();
    ++it;
  }
  return v;
}

std::ostream&
operator<<(std::ostream& os, const Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}

#define FORCE_FIXFLUSH                            \
do {                                              \
  if (Test::randgen(opt.fixprob) == 0) {          \
    Log::fixpoint();                              \
    if (status() == SS_FAILED) return;            \
  }                                               \
  if (Test::randgen(opt.flushprob) == 0) {        \
    flush();                                      \
    Log::flush();                                 \
  }                                               \
} while(0)

class IntTestSpace : public Space {
public:
  IntVarArray x;
private:
  const Options opt;

public:
  IntTestSpace(int n, IntSet& d, const Options& o)
    : x(this, n, d), opt(o) {
    Log::initial(x, "x");
  }
  IntTestSpace(bool share, IntTestSpace& s) : Space(share,s), opt(s.opt) {
    x.update(this, share, s.x);
  }
  virtual Space* copy(bool share) {
    return new IntTestSpace(share,*this);
  }
  bool is_failed(void) {
    Log::fixpoint();
    return status() == SS_FAILED;
  }
  void assign(const Assignment& a) {
    for (int i=a.size(); i--; ) {
      Log::assign(Log::mk_name("x", i), a[i]);
      rel(this, x[i], IRT_EQ, a[i]);
      FORCE_FIXFLUSH;
    }
  }
  bool assigned(void) const {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        return false;
    return true;
  }
  void prune(void) {
    // Select variable to be pruned
    int i = Test::randgen(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    // Prune values
    for (int vals = Test::randgen(x[i].size()-1)+1; vals--; ) {
      int v;
      Int::ViewRanges<Int::IntView> it(x[i]);
      unsigned int skip = Test::randgen(x[i].size()-1);
      while (true) {
        if (it.width() > skip) {
          v = it.min() + skip;
          break;
        }
        skip -= it.width();
        ++it;
      }
      Log::prune(x[i], Log::mk_name("x", i), IRT_NQ, v);
      rel(this, x[i], IRT_NQ, v);
      Log::prune_result(x[i]);
    }
  }

  static BoolVar unused;

  bool prune(const Assignment& a, IntTest& it,
             bool r, BoolVar& b = unused) {
    // Select variable to be pruned
    int i = Test::randgen(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    // Select mode for pruning
    int m=Test::randgen(3);
    if ((m == 0) && (a[i] < x[i].max())) {
      int v=a[i]+1+Test::randgen(static_cast<unsigned int>(x[i].max()-a[i]));
      assert((v > a[i]) && (v <= x[i].max()));
      Log::prune(x[i], Log::mk_name("x", i), IRT_LE, v);
      rel(this, x[i], IRT_LE, v);
      Log::prune_result(x[i]);
    } else if ((m == 1) && (a[i] > x[i].min())) {
      int v=x[i].min()+Test::randgen(static_cast<unsigned int>(a[i]-x[i].min()));
      assert((v < a[i]) && (v >= x[i].min()));
      Log::prune(x[i], Log::mk_name("x", i), IRT_GR, v);
      rel(this, x[i], IRT_GR, v);
      Log::prune_result(x[i]);
    } else  {
      int v;
      Int::ViewRanges<Int::IntView> it(x[i]);
      unsigned int skip = Test::randgen(x[i].size()-1);
      while (true) {
        if (it.width() > skip) {
          v = it.min() + skip;
          if (v == a[i]) {
            if (it.width() == 1) {
              ++it; v = it.min();
            } else if (v < it.max()) {
              ++v;
            } else {
              --v;
            }
          }
          break;
        }
        skip -= it.width();
        ++it;
      }
      Log::prune(x[i], Log::mk_name("x", i), IRT_NQ, v);
      rel(this, x[i], IRT_NQ, v);
      Log::prune_result(x[i]);
    }
    if (Test::randgen(opt.fixprob) == 0) {                
      Log::fixpoint();
      if (status() == SS_FAILED)
        return true;
      IntTestSpace* c = static_cast<IntTestSpace*>(clone());
      Log::print(c->x, "x");
      if (!r) {
        it.post(c,c->x);
        Log::fixpoint();
        if (c->status() == SS_FAILED) {
          Log::print(c->x, "x");
          delete c;
          return false;
        }
        for (int i=x.size(); i--; )
          if (x[i].size() != c->x[i].size()) {
            Log::print(c->x, "x");
            delete c;
            return false;
          }
      } else {
        Log::print(b, "b");
        BoolVar cb(c,0,1);
        Log::initial(cb, "cb");
        it.post(c,c->x,cb);
        Log::fixpoint();
        if (c->status() == SS_FAILED) {
          Log::print(c->x, "x");
          Log::print(cb, "cb");
          delete c;
          return false;
        }
        if (cb.size() != b.size()) {
          Log::print(c->x, "x");
          Log::print(cb, "cb");
          delete c;
          return false;
        }
        for (int i=x.size(); i--; )
          if (x[i].size() != c->x[i].size()) {
            Log::print(c->x, "x");
            Log::print(cb, "cb");
            delete c;
            return false;
          }
      }
      delete c;
    }                                                
    if (Test::randgen(opt.flushprob) == 0) {                
      flush();                                        
      Log::flush();                                
    }                
    return true;
  }

};

BoolVar IntTestSpace::unused;



Assignment*
IntTest::make_assignment() {
  return new Assignment(arity, dom);
}

#define CHECK(T,M)                                \
if (!(T)) {                                       \
  problem = (M);                                  \
  delete s;                                       \
  goto failed;                                    \
}

bool
IntTest::run(const Options& opt) {
  const char* test    = "NONE";
  const char* problem = "NONE";
  bool has_assignment = true;
  // Set up assignments
  Assignment* ap = make_assignment();
  Assignment& a = *ap;
  // Set up space for all solution search
  IntTestSpace* search_s = new IntTestSpace(arity,dom,opt);
  post(search_s,search_s->x);
  log_posting();
  branch(search_s,search_s->x,BVAR_NONE,BVAL_MIN);
  Gecode::DFS<IntTestSpace> e_s(search_s);
  delete search_s;
  while (a()) {
    bool is_sol = solution(a);
   {
      test = "Assignment (after posting)";
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      post(s,s->x);
      log_posting();
      s->assign(a);
      if (is_sol) {
        CHECK(!s->is_failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->is_failed(), "Solved on non-solution");
      }
      delete s;
    }
    {
      test = "Assignment (before posting)";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      s->assign(a); 
      post(s,s->x);
      log_posting();
      if (is_sol) {
        CHECK(!s->is_failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->is_failed(), "Solved on non-solution");
      }
      delete s;
    }
    if (reified) {
      test = "Assignment reified (rewrite after post)";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      BoolVar b(s,0,1);
      Log::initial(b, "b");
      post(s,s->x,b);
      log_posting();
      if (is_sol) {
        rel(s, b, IRT_EQ, 1);
      } else {
        rel(s, b, IRT_EQ, 0);
      }
      s->assign(a);
      CHECK(!s->is_failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      delete s;
    }
    if (reified) {
      test = "Assignment reified (immediate rewrite)";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      BoolVar b(s,0,1);
      Log::initial(b, "b");
      if (is_sol) {
        rel(s, b, IRT_EQ, 1);
      } else {
        rel(s, b, IRT_EQ, 0);
      }
      post(s,s->x,b);
      log_posting();
      s->assign(a);
      CHECK(!s->is_failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      delete s;
    }
    if (reified) {
      test = "Assignment reified (before posting)";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      BoolVar b(s,0,1);
      Log::initial(b, "b");
      s->assign(a); 
      post(s,s->x,b);
      log_posting();
      CHECK(!s->is_failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
        CHECK(b.val()==1, "Zero on solution");
      } else {
        CHECK(b.val()==0, "One on non-solution");
      }
      delete s;
    }
    if (reified) {
      test = "Assignment reified (after posting)";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      BoolVar b(s,0,1);
      Log::initial(b, "b");
      post(s,s->x,b);
      log_posting();
      s->assign(a);
      CHECK(!s->is_failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
        CHECK(b.val()==1, "Zero on solution");
      } else {
        CHECK(b.val()==0, "One on non-solution");
      }
      delete s;
    }
    {
      test = "Prune";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      post(s,s->x);
      log_posting();
      while (!s->failed() && !s->assigned())
        if (!s->prune(a,*this,false)) {
          problem = "No fixpoint";
          delete s;
          goto failed;
        }
      s->assign(a);
      if (is_sol) {
        CHECK(!s->is_failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->is_failed(), "Solved on non-solution");
      }
      delete s;
    }
    if (reified) {
      test = "Prune reified";
      Log::reset();
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      BoolVar b(s,0,1);
      Log::initial(b, "b");
      post(s,s->x,b);
      log_posting();
      while (!s->failed() && !s->assigned() && !b.assigned())
        if (!s->prune(a,*this,true,b)) {
          problem = "No fixpoint";
          delete s;
          goto failed;
        }
      CHECK(!s->is_failed(), "Failed");
      CHECK(s->propagators()==0, "No subsumtion");
      CHECK(b.assigned(), "Control variable unassigned");
      if (is_sol) {
        CHECK(b.val()==1, "Zero on solution");
      } else {
        CHECK(b.val()==0, "One on non-solution");
      }
      delete s;
    }
    if (do_search_test()) {
      Log::reset();
      test = "Search";
      if (is_sol) {
        IntTestSpace* s = e_s.next();
        CHECK(s != NULL,    "Solutions exhausted");
        CHECK(s->propagators()==0, "No subsumtion");
        for (int i=a.size(); i--; ) {
          CHECK(s->x[i].assigned(), "Unassigned variable");
          CHECK(a[i] == s->x[i].val(), "Wrong value in solution");
        }
        delete s;
      }
    }
    ++a;
  }
  if (do_search_test()) {
    test = "Search";
    if (e_s.next() != NULL) {
      problem = "Excess solutions";
      goto failed;
    }
  }
  if (icl == ICL_DOM) {
    has_assignment = false;
    test = "Full domain consistency";
    Log::reset();
    IntTestSpace* s = new IntTestSpace(arity,dom,opt);
    post(s,s->x);
    log_posting();
    while (!s->is_failed() && !s->assigned()) {
      s->prune();
    }
    CHECK(!s->is_failed(), "Failed");
    CHECK(s->propagators()==0, "No subsumtion");
    delete s;
    has_assignment = true;
  }
  delete ap;
  return true;
 failed:
  std::cout   << "FAILURE" << std::endl
              << "\t" << "Test:       " << test << std::endl
              << "\t" << "Problem:    " << problem << std::endl;
  if (has_assignment)
    std::cout << "\t" << "Assignment: " << a << std::endl;
  delete ap;
  return false;
}

#undef CHECK

// STATISTICS: test-int
