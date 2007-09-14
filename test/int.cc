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

namespace Test { namespace Int {

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

  class ind {
  public:
    int i;
    ind(int j) : i(j) {}
  };

}}

std::ostream&
operator<<(std::ostream& os, const Test::Int::Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}

std::ostream&
operator<<(std::ostream& os, const Test::Int::ind& i) {
  for (int j=i.i; j--; )
    os << "  ";
  return os;
}

namespace Test { namespace Int {



  class TestSpace : public Gecode::Space {
  public:
    Gecode::IntVarArray x;
  private:
    const Options opt;

  public:
    TestSpace(int n, Gecode::IntSet& d, const Options& o, bool log=true)
      : x(this, n, d), opt(o) {
      if (opt.log && log) {
        std::cerr << ind(2) << "Initial: ";
        state();
        std::cerr << std::endl;
      }
    }
    TestSpace(bool share, TestSpace& s) 
      : Gecode::Space(share,s), opt(s.opt) {
      x.update(this, share, s.x);
    }
    virtual Gecode::Space* copy(bool share) {
      return new TestSpace(share,*this);
    }
  
    void state(void) {
      if (opt.log) {
        std::cerr << "x={";
        for (int i=0; i<x.size(); i++)
          std::cerr << x[i] << ((i<x.size()-1) ? "," : "");
        std::cerr << "};";
      }
    }
    bool failed(void) {
      if (opt.log) {
        std::cerr << ind(3) << "Fixpoint: ";
        state();
        bool f=(status() == Gecode::SS_FAILED);
        std::cerr << std::endl << ind(3) << "     -->  ";
        state();
        std::cerr << std::endl;
        return f;
      } else {
        return status() == Gecode::SS_FAILED;
      }
    }

    /// Perform tell operation on \a x[i]
    void rel(int i, Gecode::IntRelType irt, int n) {
      if (opt.log) {
        std::cerr << ind(4) << "x[" << i << "] ";
        switch (irt) {
        case Gecode::IRT_EQ: std::cerr << "="; break;
        case Gecode::IRT_NQ: std::cerr << "!="; break;
        case Gecode::IRT_LQ: std::cerr << "<="; break;
        case Gecode::IRT_LE: std::cerr << "<"; break;
        case Gecode::IRT_GQ: std::cerr << ">="; break;
        case Gecode::IRT_GR: std::cerr << ">"; break;
        }
        std::cerr << " " << n << std::endl;
      }
      Gecode::rel(this, x[i], irt, n);
    }
    /// Assign all variables to values in \a a
    void assign(const Assignment& a) {
      using namespace Gecode;
      for (int i=a.size(); i--; ) {
        rel(i, IRT_EQ, a[i]);
        if ((Base::rand(opt.fixprob) == 0) && failed())
          return;
      }
    }
    /// Test whether all variables are assigned
    bool assigned(void) const {
      for (int i=x.size(); i--; )
        if (!x[i].assigned())
          return false;
      return true;
    }
    /// Prune some random values for some random variable
    void prune(void) {
      using namespace Gecode;
      // Select variable to be pruned
      int i = Base::rand(x.size());
      while (x[i].assigned()) {
        i = (i+1) % x.size();
      }
      // Prune values
      for (int vals = Base::rand(x[i].size()-1)+1; vals--; ) {
        int v;
        Gecode::Int::ViewRanges<Gecode::Int::IntView> it(x[i]);
        unsigned int skip = Base::rand(x[i].size()-1);
        while (true) {
          if (it.width() > skip) {
            v = it.min() + skip; break;
          }
          skip -= it.width(); ++it;
        }
        rel(i, IRT_NQ, v);
      }
    }

    static Gecode::BoolVar unused;

    bool prune(const Assignment& a, Test& it,
               bool r, Gecode::BoolVar& b = unused) {
      using namespace Gecode;
      // Select variable to be pruned
      int i = Base::rand(x.size());
      while (x[i].assigned())
        i = (i+1) % x.size();
      // Select mode for pruning
      switch (Base::rand(3)) {
      case 0:
        if (a[i] < x[i].max()) {
          int v=a[i]+1+Base::rand(static_cast
                                  <unsigned int>(x[i].max()-a[i]));
          assert((v > a[i]) && (v <= x[i].max()));
          rel(i, IRT_LE, v);
        }
        break;
      case 1:
        if (a[i] > x[i].min()) {
          int v=x[i].min()+Base::rand(static_cast
                                      <unsigned int>(a[i]-x[i].min()));
          assert((v < a[i]) && (v >= x[i].min()));
          rel(i, IRT_GR, v);
        }
        break;
      default:
        {
          int v;
          Gecode::Int::ViewRanges<Gecode::Int::IntView> it(x[i]);
          unsigned int skip = Base::rand(x[i].size()-1);
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
            skip -= it.width(); ++it;
          }
          rel(i, IRT_NQ, v);
          break;
        }
      }
      if (Base::rand(opt.fixprob) == 0) {
        if (failed())
          return true;
        TestSpace* c = static_cast<TestSpace*>(clone());
        if (!r) {
          it.post(c,c->x);
          Log::log("Post again");
          if (c->failed()) {
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

  Gecode::BoolVar TestSpace::unused;


  const Gecode::IntConLevel IntConLevels::icls[] =
    {Gecode::ICL_DOM,Gecode::ICL_BND,Gecode::ICL_VAL};

  const Gecode::IntRelType IntRelTypes::irts[] = 
    {Gecode::IRT_EQ,Gecode::IRT_NQ,Gecode::IRT_LQ,
     Gecode::IRT_LE,Gecode::IRT_GQ,Gecode::IRT_GR};

  Assignment*
  Test::assignment(void) const {
    return new CpltAssignment(arity,dom);
  }

  /// Check the test result and handle failed test
#define CHECK_TEST(T,M)                         \
if (opt.log)                                    \
  std::cerr << ind(3) << "Check: " << (M) << std::endl;           \
if (!(T)) {                                     \
  problem = (M); delete s; goto failed;         \
}

  /// Start new test
#define START_TEST(T)                            \
  if (opt.log)                                   \
     std::cerr << ind(2) << "Testing: " << (T) << std::endl; \
  test = (T);


  void 
  Test::post(Gecode::Space* home, Gecode::IntVarArray& x, 
             Gecode::BoolVar b) {}

  bool
  Test::run(const Options& opt) {
    using namespace Gecode;
    const char* test    = "NONE";
    const char* problem = "NONE";

    // Set up assignments
    Assignment* ap = assignment();
    Assignment& a = *ap;

    // Set up space for all solution search
    TestSpace* search_s = new TestSpace(arity,dom,opt,false);
    post(search_s,search_s->x);
    branch(search_s,search_s->x,INT_VAR_NONE,INT_VAL_MIN);
    DFS<TestSpace> e_s(search_s);
    delete search_s;

    while (a()) {
      bool sol = solution(a);
      if (opt.log)
        std::cerr << ind(1)
                  << "Assignment: " << a 
                  << (sol ? " (solution)" : " (no solution)")
                  << std::endl;

      START_TEST("Assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,opt);
        post(s,s->x);
        s->assign(a);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Assignment (before posting)");
      {      
        TestSpace* s = new TestSpace(arity,dom,opt);
        s->assign(a); 
        post(s,s->x);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Prune");
      {      
        TestSpace* s = new TestSpace(arity,dom,opt);
        post(s,s->x);
        while (!s->failed() && !s->assigned())
          if (!s->prune(a,*this,false)) {
            problem = "No fixpoint";
            delete s;
            goto failed;
          }
        s->assign(a);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }

      if (reified) {
        START_TEST("Assignment reified (rewrite after post)");
        {      
          TestSpace* s = new TestSpace(arity,dom,opt);
          BoolVar b(s,0,1);
          Log::initial(b, "b");
          post(s,s->x,b);
          if (sol) {
            rel(s, b, IRT_EQ, 1);
          } else {
            rel(s, b, IRT_EQ, 0);
          }
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          delete s;
        }
        START_TEST("Assignment reified (immediate rewrite)");
        {        
          TestSpace* s = new TestSpace(arity,dom,opt);
          BoolVar b(s,0,1);
          Log::initial(b, "b");
          if (sol) {
            rel(s, b, IRT_EQ, 1);
          } else {
            rel(s, b, IRT_EQ, 0);
          }
          post(s,s->x,b);
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          delete s;
        }
        START_TEST("Assignment reified (before posting)");
        {        
          TestSpace* s = new TestSpace(arity,dom,opt);
          BoolVar b(s,0,1);
          Log::initial(b, "b");
          s->assign(a); 
          post(s,s->x,b);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          CHECK_TEST(b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(b.val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Assignment reified (after posting)");
        {        
          TestSpace* s = new TestSpace(arity,dom,opt);
          BoolVar b(s,0,1);
          Log::initial(b, "b");
          post(s,s->x,b);
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          CHECK_TEST(b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(b.val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Prune reified");
        {        
          TestSpace* s = new TestSpace(arity,dom,opt);
          BoolVar b(s,0,1);
          Log::initial(b, "b");
          post(s,s->x,b);
          while (!s->failed() && !s->assigned() && !b.assigned())
            if (!s->prune(a,*this,true,b)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          CHECK_TEST(b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(b.val()==0, "One on non-solution");
          }
          delete s;
        }
      }

      if (testsearch) {
        if (sol) {
          START_TEST("Search");
          TestSpace* s = e_s.next();
          CHECK_TEST(s != NULL, "Solutions exhausted");
          CHECK_TEST(s->propagators()==0, "No subsumtion");
          for (int i=a.size(); i--; ) {
            CHECK_TEST(s->x[i].assigned(), "Unassigned variable");
            CHECK_TEST(a[i] == s->x[i].val(), "Wrong value in solution");
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
      START_TEST("Full domain consistency");
      TestSpace* s = new TestSpace(arity,dom,opt);
      post(s,s->x);
      while (!s->failed() && !s->assigned())
        s->prune();
      CHECK_TEST(!s->failed(), "Failed");
      CHECK_TEST(s->propagators()==0, "No subsumtion");
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

}}

#undef START_TEST
#undef CHECK_TEST

// STATISTICS: test-int
