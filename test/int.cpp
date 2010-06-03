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

#include "test/int.hh"

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

  void
  RandomMixAssignment::operator++(void) {
    for (int i=n-_n1; i--; )
      vals[i] = randval(d);
    for (int i=_n1; i--; )
      vals[n-_n1+i] = randval(_d1);
    a--;
  }

}}

std::ostream&
operator<<(std::ostream& os, const Test::Int::Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}

namespace Test { namespace Int {

  TestSpace::TestSpace(int n, Gecode::IntSet& d0, bool r, Test* t, bool log)
    : d(d0), x(*this,n,d), b(*this,0,1), reified(r), test(t) {
    if (opt.log && log) {
      olog << ind(2) << "Initial: x[]=" << x;
      if (reified)
        olog << " b=" << b;
      olog << std::endl;
    }
  }

  TestSpace::TestSpace(bool share, TestSpace& s)
    : Gecode::Space(share,s), d(s.d), reified(s.reified), test(s.test) {
    x.update(*this, share, s.x);
    b.update(*this, share, s.b);
  }

  Gecode::Space* 
  TestSpace::copy(bool share) {
    return new TestSpace(share,*this);
  }

  bool 
  TestSpace::assigned(void) const {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        return false;
    return true;
  }

  void 
  TestSpace::post(void) {
    if (reified){
      test->post(*this,x,b);
      if (opt.log)
        olog << ind(3) << "Posting reified propagator" << std::endl;
    } else {
      test->post(*this,x);
      if (opt.log)
        olog << ind(3) << "Posting propagator" << std::endl;
    }
  }

  bool 
  TestSpace::failed(void) {
    if (opt.log) {
      olog << ind(3) << "Fixpoint: " << x;
      bool f=(status() == Gecode::SS_FAILED);
      olog << std::endl << ind(3) << "     -->  " << x << std::endl;
      return f;
    } else {
      return status() == Gecode::SS_FAILED;
    }
  }

  void 
  TestSpace::rel(int i, Gecode::IntRelType irt, int n) {
    if (opt.log) {
      olog << ind(4) << "x[" << i << "] ";
      switch (irt) {
      case Gecode::IRT_EQ: olog << "="; break;
      case Gecode::IRT_NQ: olog << "!="; break;
      case Gecode::IRT_LQ: olog << "<="; break;
      case Gecode::IRT_LE: olog << "<"; break;
      case Gecode::IRT_GQ: olog << ">="; break;
      case Gecode::IRT_GR: olog << ">"; break;
      }
      olog << " " << n << std::endl;
    }
    Gecode::rel(*this, x[i], irt, n);
  }

  void 
  TestSpace::rel(bool sol) {
    int n = sol ? 1 : 0;
    assert(reified);
    if (opt.log)
      olog << ind(4) << "b = " << n << std::endl;
    Gecode::rel(*this, b, Gecode::IRT_EQ, n);
  }

  void 
  TestSpace::assign(const Assignment& a, bool skip) {
    using namespace Gecode;
    int i = skip ? static_cast<int>(Base::rand(a.size())) : -1;
    for (int j=a.size(); j--; )
      if (i != j) {
        rel(j, IRT_EQ, a[j]);
        if (Base::fixpoint() && failed())
          return;
      }
  }

  void 
  TestSpace::bound(void) {
    using namespace Gecode;
    // Select variable to be assigned
    int i = Base::rand(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    bool min = Base::rand(2);
    rel(i, IRT_EQ, min ? x[i].min() : x[i].max());
  }

  void 
  TestSpace::prune(int i, bool bounds_only) {
    using namespace Gecode;
    // Prune values
    if (bounds_only) {
      if (Base::rand(2) && !x[i].assigned()) {
        int v=x[i].min()+1+Base::rand(static_cast
                                      <unsigned int>(x[i].max()-x[i].min()));
        assert((v > x[i].min()) && (v <= x[i].max()));
        rel(i, Gecode::IRT_LE, v);
      }
      if (Base::rand(2) && !x[i].assigned()) {
        int v=x[i].min()+Base::rand(static_cast
                                    <unsigned int>(x[i].max()-x[i].min()));
        assert((v < x[i].max()) && (v >= x[i].min()));
        rel(i, Gecode::IRT_GR, v);
      }
    } else {
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
  }

  void 
  TestSpace::prune(void) {
    using namespace Gecode;
    // Select variable to be pruned
    int i = Base::rand(x.size());
    while (x[i].assigned()) {
      i = (i+1) % x.size();
    }
    prune(i, false);
  }

  bool 
  TestSpace::prune(const Assignment& a, bool testfix) {
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
        rel(i, Gecode::IRT_LE, v);
      }
      break;
    case 1:
      if (a[i] > x[i].min()) {
        int v=x[i].min()+Base::rand(static_cast
                                    <unsigned int>(a[i]-x[i].min()));
        assert((v < a[i]) && (v >= x[i].min()));
        rel(i, Gecode::IRT_GR, v);
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
        rel(i, Gecode::IRT_NQ, v);
        break;
      }
    }
    if (Base::fixpoint()) {
      if (failed() || !testfix)
        return true;
      TestSpace* c = static_cast<TestSpace*>(clone());
      if (opt.log)
        olog << ind(3) << "Testing fixpoint on copy" << std::endl;
      c->post();
      if (c->failed()) {
        delete c; return false;
      }
      for (int i=x.size(); i--; )
        if (x[i].size() != c->x[i].size()) {
          delete c; return false;
        }
      if (reified && (b.size() != c->b.size())) {
        delete c; return false;
      }
      if (opt.log)
        olog << ind(3) << "Finished testing fixpoint on copy" << std::endl;
      delete c;
    }
    return true;
  }


  const Gecode::IntConLevel IntConLevels::icls[] =
    {Gecode::ICL_DOM,Gecode::ICL_BND,Gecode::ICL_VAL};

  const Gecode::IntRelType IntRelTypes::irts[] =
    {Gecode::IRT_EQ,Gecode::IRT_NQ,Gecode::IRT_LQ,
     Gecode::IRT_LE,Gecode::IRT_GQ,Gecode::IRT_GR};

  const Gecode::BoolOpType BoolOpTypes::bots[] =
    {Gecode::BOT_AND,Gecode::BOT_OR,Gecode::BOT_IMP,
     Gecode::BOT_EQV,Gecode::BOT_XOR};

  Assignment*
  Test::assignment(void) const {
    return new CpltAssignment(arity,dom);
  }


  /// Check the test result and handle failed test
#define CHECK_TEST(T,M)                                         \
if (opt.log)                                                    \
  olog << ind(3) << "Check: " << (M) << std::endl;              \
if (!(T)) {                                                     \
  problem = (M); delete s; goto failed;                         \
}

  /// Start new test
#define START_TEST(T)                                           \
  if (opt.log) {                                                \
     olog.str("");                                              \
     olog << ind(2) << "Testing: " << (T) << std::endl;         \
  }                                                             \
  test = (T);

  bool
  Test::ignore(const Assignment&) const {
    return false;
  }

  void
  Test::post(Gecode::Space&, Gecode::IntVarArray&,
             Gecode::BoolVar) {}

  bool
  Test::run(void) {
    using namespace Gecode;
    const char* test    = "NONE";
    const char* problem = "NONE";

    // Set up assignments
    Assignment* ap = assignment();
    Assignment& a = *ap;

    // Set up space for all solution search
    TestSpace* search_s = new TestSpace(arity,dom,false,this,false);
    post(*search_s,search_s->x);
    branch(*search_s,search_s->x,INT_VAR_NONE,INT_VAL_MIN);
    Search::Options search_o;
    search_o.threads = 1;
    DFS<TestSpace> e_s(search_s,search_o);
    delete search_s;

    while (a()) {
      bool sol = solution(a);
      if (opt.log) {
        olog << ind(1) << "Assignment: " << a
             << (sol ? " (solution)" : " (no solution)")
             << std::endl;
      }

      START_TEST("Assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,false,this);
        TestSpace* sc = NULL;
        s->post();
        switch (Base::rand(3)) {
          case 0:
            if (opt.log)
              olog << ind(3) << "No copy" << std::endl;
            sc = s;
            s = NULL;
            break;
          case 1:
            if (opt.log)
              olog << ind(3) << "Unshared copy" << std::endl;
            if (s->status() != SS_FAILED) {
              sc = static_cast<TestSpace*>(s->clone(false));
            } else {
              sc = s; s = NULL;
            }
            break;
          case 2:
            if (opt.log)
              olog << ind(3) << "Shared copy" << std::endl;
            if (s->status() != SS_FAILED) {
              sc = static_cast<TestSpace*>(s->clone(true));
            } else {
              sc = s; s = NULL;
            }
            break;
          default: assert(false);
        }
        sc->assign(a);
        if (sol) {
          CHECK_TEST(!sc->failed(), "Failed on solution");
          CHECK_TEST(sc->propagators()==0, "No subsumption");
        } else {
          CHECK_TEST(sc->failed(), "Solved on non-solution");
        }
        delete s; delete sc;
      }
      START_TEST("Partial assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,false,this);
        s->post();
        s->assign(a,true);
        (void) s->failed();
        s->assign(a);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumption");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Assignment (before posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,false,this);
        s->assign(a);
        s->post();
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumption");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Partial assignment (before posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,false,this);
        s->assign(a,true);
        s->post();
        (void) s->failed();
        s->assign(a);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumption");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Prune");
      {
        TestSpace* s = new TestSpace(arity,dom,false,this);
        s->post();
        while (!s->failed() && !s->assigned())
          if (!s->prune(a,testfix)) {
            problem = "No fixpoint";
            delete s;
            goto failed;
          }
        s->assign(a);
        if (sol) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(s->propagators()==0, "No subsumption");
        } else {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }

      if (reified && !ignore(a)) {
        START_TEST("Assignment reified (rewrite after post)");
        {
          TestSpace* s = new TestSpace(arity,dom,true,this);
          s->post();
          s->rel(sol);
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          delete s;
        }
        START_TEST("Assignment reified (immediate rewrite)");
        {
          TestSpace* s = new TestSpace(arity,dom,true,this);
          s->rel(sol);
          s->post();
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          delete s;
        }
        START_TEST("Assignment reified (before posting)");
        {
          TestSpace* s = new TestSpace(arity,dom,true,this);
          s->assign(a);
          s->post();
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->b.val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Assignment reified (after posting)");
        {
          TestSpace* s = new TestSpace(arity,dom,true,this);
          s->post();
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->b.val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Prune reified");
        {
          TestSpace* s = new TestSpace(arity,dom,true,this);
          s->post();
          while (!s->failed() && (!s->assigned() || !s->b.assigned()))
            if (!s->prune(a,testfix)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->b.assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->b.val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->b.val()==0, "One on non-solution");
          }
          delete s;
        }
      }

      if (testsearch) {
        if (sol) {
          START_TEST("Search");
          TestSpace* s = e_s.next();
          CHECK_TEST(s != NULL, "Solutions exhausted");
          CHECK_TEST(s->propagators()==0, "No subsumption");
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

    switch (contest) {
    case CTL_NONE: break;
    case CTL_DOMAIN: {
      START_TEST("Full domain consistency");
      TestSpace* s = new TestSpace(arity,dom,false,this);
      s->post();
      if (!s->failed()) {
        while (!s->failed() && !s->assigned())
          s->prune();
        CHECK_TEST(!s->failed(), "Failed");
        CHECK_TEST(s->propagators()==0, "No subsumption");
      }
      delete s;
      // Fall-through -- domain implies bounds(d) and bounds(z)
    }
    case CTL_BOUNDS_D: {
      START_TEST("Bounds(D)-consistency");
      TestSpace* s = new TestSpace(arity,dom,false,this);
      s->post();
      for (int i = s->x.size(); i--; )
        s->prune(i, false);
      if (!s->failed()) {
        while (!s->failed() && !s->assigned())
          s->bound();
        CHECK_TEST(!s->failed(), "Failed");
        CHECK_TEST(s->propagators()==0, "No subsumption");
      }
      delete s;
      // Fall-through -- bounds(d) implies bounds(z)
    }
    case CTL_BOUNDS_Z: {
      START_TEST("Bounds(Z)-consistency");
      TestSpace* s = new TestSpace(arity,dom,false,this);
      s->post();
      for (int i = s->x.size(); i--; )
        s->prune(i, true);
      if (!s->failed()) {
        while (!s->failed() && !s->assigned())
          s->bound();
        CHECK_TEST(!s->failed(), "Failed");
        CHECK_TEST(s->propagators()==0, "No subsumption");
      }
      delete s;
      break;
    }
    }

    delete ap;
    return true;

  failed:
    if (opt.log)
      olog << "FAILURE" << std::endl
           << ind(1) << "Test:       " << test << std::endl
           << ind(1) << "Problem:    " << problem << std::endl;
    if (a() && opt.log)
      olog << ind(1) << "Assignment: " << a << std::endl;
    delete ap;

    return false;
  }

}}

#undef START_TEST
#undef CHECK_TEST

// STATISTICS: test-int
