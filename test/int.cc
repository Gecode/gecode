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

#include "gecode/serialization.hh"
#include "test/int.hh"
#include "test/log.hh"
#include <algorithm>

/*
 * Complete assignments
 *
 */
void
CpltAssignment::operator++(void) {
  int i = n-1;
  while (true) {
    ++dsv[i];
    if (dsv[i]() || (i == 0))
      return;
    dsv[i--].init(d);
  }
}

/*
 * Random assignments
 *
 */
void
RandomAssignment::operator++(void) {
  for (int i = n; i--; )
    vals[i]=randval();
  a--;
}

std::ostream&
operator<<(std::ostream& os, const Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}





class IntTestSpace : public Gecode::Space {
public:
  Gecode::IntVarArray x;
private:
  const Options opt;

public:
  IntTestSpace(int n, Gecode::IntSet& d, const Options& o)
    : x(this, n, d), opt(o) {
    Log::initial(x, "x");
  }
  IntTestSpace(bool share, IntTestSpace& s) 
    : Gecode::Space(share,s), opt(s.opt) {
    x.update(this, share, s.x);
  }
  virtual Gecode::Space* copy(bool share) {
    return new IntTestSpace(share,*this);
  }
  
  bool failed(void) {
    using namespace Gecode;
    Log::fixpoint();
    return status() == SS_FAILED;
  }

  void assign(const Assignment& a) {
    using namespace Gecode;
    for (int i=a.size(); i--; ) {
      Log::assign(Log::mk_name("x", i), a[i]);
      rel(this, x[i], IRT_EQ, a[i]);
      if ((TestBase::randgen(opt.fixprob) == 0) && failed())
        return;
    }
  }

  bool assigned(void) const {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        return false;
    return true;
  }

  void prune(void) {
    using namespace Gecode;
    // Select variable to be pruned
    int i = TestBase::randgen(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    // Prune values
    for (int vals = TestBase::randgen(x[i].size()-1)+1; vals--; ) {
      int v;
      Int::ViewRanges<Int::IntView> it(x[i]);
      unsigned int skip = TestBase::randgen(x[i].size()-1);
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

  static Gecode::BoolVar unused;

  bool prune(const Assignment& a, IntTest& it,
             bool r, Gecode::BoolVar& b = unused) {
    using namespace Gecode;
    // Select variable to be pruned
    int i = TestBase::randgen(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    // Select mode for pruning
    int m=TestBase::randgen(3);
    if ((m == 0) && (a[i] < x[i].max())) {
      int v=a[i]+1+TestBase::randgen(static_cast<unsigned int>(x[i].max()-a[i]));
      assert((v > a[i]) && (v <= x[i].max()));
      Log::prune(x[i], Log::mk_name("x", i), IRT_LE, v);
      rel(this, x[i], IRT_LE, v);
      Log::prune_result(x[i]);
    } else if ((m == 1) && (a[i] > x[i].min())) {
      int v=x[i].min()+TestBase::randgen(static_cast<unsigned int>(a[i]-x[i].min()));
      assert((v < a[i]) && (v >= x[i].min()));
      Log::prune(x[i], Log::mk_name("x", i), IRT_GR, v);
      rel(this, x[i], IRT_GR, v);
      Log::prune_result(x[i]);
    } else  {
      int v;
      Int::ViewRanges<Int::IntView> it(x[i]);
      unsigned int skip = TestBase::randgen(x[i].size()-1);
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
    if (TestBase::randgen(opt.fixprob) == 0) {                
      Log::fixpoint();
      Log::print(x, "x");
      if (status() == SS_FAILED)
        return true;
      IntTestSpace* c = static_cast<IntTestSpace*>(clone());
      Log::print(c->x, "x");
      if (!r) {
        it.post(c,c->x);
        Log::log("Post again","//Post again");
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
    return true;
  }

};

Gecode::BoolVar IntTestSpace::unused;


const Gecode::IntConLevel IntConLevels::icls[] =
  {Gecode::ICL_DOM,Gecode::ICL_BND,Gecode::ICL_VAL};

const Gecode::IntRelType IntRelTypes::irts[] = 
  {Gecode::IRT_EQ,Gecode::IRT_NQ,Gecode::IRT_LQ,
   Gecode::IRT_LE,Gecode::IRT_GQ,Gecode::IRT_GR};

Assignment*
IntTest::assignment(void) const {
  return new CpltAssignment(arity,dom);
}

#define CHECK(T,M)                                \
if (!(T)) {                                       \
  problem = (M);                                  \
  delete s;                                       \
  goto failed;                                    \
}

void 
IntTest::post(Gecode::Space* home, Gecode::IntVarArray& x, 
              Gecode::BoolVar b) {}

bool
IntTest::run(const Options& opt) {
  using namespace Gecode;
  const char* test    = "NONE";
  const char* problem = "NONE";

  // Set up assignments
  Assignment* ap = assignment();
  Assignment& a = *ap;

  // Set up space for all solution search
  IntTestSpace* search_s = new IntTestSpace(arity,dom,opt);
  post(search_s,search_s->x);
  log_posting();
  branch(search_s,search_s->x,INT_VAR_NONE,INT_VAL_MIN);
  Gecode::DFS<IntTestSpace> e_s(search_s);
  delete search_s;

  while (a()) {

    bool sol = solution(a);

    {
      test = "Assignment (after posting)";
      
      IntTestSpace* s = new IntTestSpace(arity,dom,opt);
      post(s,s->x);
      log_posting();
      s->assign(a);
      if (sol) {
        CHECK(!s->failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->failed(), "Solved on non-solution");
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
      if (sol) {
        CHECK(!s->failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->failed(), "Solved on non-solution");
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
      if (sol) {
        CHECK(!s->failed(), "Failed on solution");
        CHECK(s->propagators()==0, "No subsumtion");
      } else {
        CHECK(s->failed(), "Solved on non-solution");
      }
      delete s;
    }

    if (reified) {
      {
        test = "Assignment reified (rewrite after post)";
        Log::reset();
      
        IntTestSpace* s = new IntTestSpace(arity,dom,opt);
        BoolVar b(s,0,1);
        Log::initial(b, "b");
        post(s,s->x,b);
        log_posting();
        if (sol) {
          rel(s, b, IRT_EQ, 1);
        } else {
          rel(s, b, IRT_EQ, 0);
        }
        s->assign(a);
        CHECK(!s->failed(), "Failed");
        CHECK(s->propagators()==0, "No subsumtion");
        delete s;
      }
      {
        test = "Assignment reified (immediate rewrite)";
        Log::reset();
        
        IntTestSpace* s = new IntTestSpace(arity,dom,opt);
        BoolVar b(s,0,1);
        Log::initial(b, "b");
        if (sol) {
          rel(s, b, IRT_EQ, 1);
        } else {
          rel(s, b, IRT_EQ, 0);
        }
        post(s,s->x,b);
        log_posting();
        s->assign(a);
        CHECK(!s->failed(), "Failed");
        CHECK(s->propagators()==0, "No subsumtion");
        delete s;
      }
      {
        test = "Assignment reified (before posting)";
        Log::reset();
        
        IntTestSpace* s = new IntTestSpace(arity,dom,opt);
        BoolVar b(s,0,1);
        Log::initial(b, "b");
        s->assign(a); 
        post(s,s->x,b);
        log_posting();
        CHECK(!s->failed(), "Failed");
        CHECK(s->propagators()==0, "No subsumtion");
        CHECK(b.assigned(), "Control variable unassigned");
        if (sol) {
          CHECK(b.val()==1, "Zero on solution");
        } else {
          CHECK(b.val()==0, "One on non-solution");
        }
        delete s;
      }
      {
        test = "Assignment reified (after posting)";
        Log::reset();
        
        IntTestSpace* s = new IntTestSpace(arity,dom,opt);
        BoolVar b(s,0,1);
        Log::initial(b, "b");
        post(s,s->x,b);
        log_posting();
        s->assign(a);
        CHECK(!s->failed(), "Failed");
        CHECK(s->propagators()==0, "No subsumtion");
        CHECK(b.assigned(), "Control variable unassigned");
        if (sol) {
          CHECK(b.val()==1, "Zero on solution");
        } else {
          CHECK(b.val()==0, "One on non-solution");
        }
        delete s;
      }
      {
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
        CHECK(!s->failed(), "Failed");
        CHECK(s->propagators()==0, "No subsumtion");
        CHECK(b.assigned(), "Control variable unassigned");
        if (sol) {
          CHECK(b.val()==1, "Zero on solution");
        } else {
          CHECK(b.val()==0, "One on non-solution");
        }
        delete s;
      }
    }

    if (testsearch) {
      Log::reset();
      test = "Search";
      if (sol) {
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

  if (testsearch) {
    test = "Search";
    if (e_s.next() != NULL) {
      problem = "Excess solutions";
      goto failed;
    }
  }

  if ((icl == ICL_DOM) && testdomcon) {
    test = "Full domain consistency";
    Log::reset();
    IntTestSpace* s = new IntTestSpace(arity,dom,opt);
    post(s,s->x);
    log_posting();
    while (!s->failed() && !s->assigned()) {
      s->prune();
    }
    CHECK(!s->failed(), "Failed");
    CHECK(s->propagators()==0, "No subsumtion");
    delete s;
  }

  delete ap;
  return true;

 failed:
  std::cout   << "FAILURE" << std::endl
              << "\t" << "Test:       " << test << std::endl
              << "\t" << "Problem:    " << problem << std::endl;
  if (a())
    std::cout << "\t" << "Assignment: " << a << std::endl;

  delete ap;

  return false;
}

#undef CHECK

// STATISTICS: test-int
