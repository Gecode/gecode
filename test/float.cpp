/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
 *     Vincent Barichard, 2012
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

#include "test/float.hh"

#include <algorithm>

namespace Test { namespace Float {

  /*
   * Complete assignments
   *
   */
  void
  CpltAssignment::operator++(void) {
    int i = n-1;
    while (true) {
      dsv[i] += step;
      if ((dsv[i] < d.max()) || (i == 0))
        return;
      dsv[i--] = d.min();
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

}}

std::ostream&
operator<<(std::ostream& os, const Test::Float::Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}

namespace Test { namespace Float {

  Gecode::FloatNum randFValDown(Gecode::FloatNum l, Gecode::FloatNum u) {
    using namespace Gecode;
    using namespace Gecode::Float;
    return 
      Round.add_down(
        l,
        Round.mul_down(
          Round.div_down(
            Base::rand(static_cast<unsigned int>(Int::Limits::max)),
            static_cast<FloatNum>(Int::Limits::max)
          ),
          Round.sub_down(u,l)
        )
      );
  }

  Gecode::FloatNum randFValUp(Gecode::FloatNum l, Gecode::FloatNum u) {
    using namespace Gecode;
    using namespace Gecode::Float;
    return 
      Round.sub_up(
        u,
        Round.mul_down(
          Round.div_down(
            Base::rand(static_cast<unsigned int>(Int::Limits::max)),
            static_cast<FloatNum>(Int::Limits::max)
          ),
          Round.sub_down(u,l)
        )
      );
  }


  std::string
  TestSpace::str(Gecode::ReifyMode rm) {
    using namespace Gecode;
    switch (rm) {
    case RM_EQV: return "<=>";
    case RM_IMP: return "=>";
    case RM_PMI: return "<=";
    default: return "???";
    }
  }

  TestSpace::TestSpace(int n, Gecode::FloatVal& d0, Gecode::FloatNum s, Test* t,
                       bool re, Gecode::ReifyMode rm)
    : d(d0), step(s), x(*this,n,d.min(),d.max()), test(t), reified(re) {
    Gecode::BoolVar b(*this,0,1);
    r = Gecode::Reify(b,rm);
    if (opt.log) {
      olog << ind(2) << "Initial: x[]=" << x;
      if (reified)
        olog << " b=" << r.var() << " [mode=" << str(r.mode()) << "]";
      olog << std::endl;
    }
  }

  TestSpace::TestSpace(bool share, TestSpace& s)
    : Gecode::Space(share,s), d(s.d), step(s.step), test(s.test), reified(s.reified) {
    x.update(*this, share, s.x);
    Gecode::BoolVar b;
    Gecode::BoolVar sr(s.r.var());
    b.update(*this, share, sr);
    r.var(b); r.mode(s.r.mode());
  }

  Gecode::Space* 
  TestSpace::copy(bool share) {
    return new TestSpace(share,*this);
  }

  void TestSpace::next_as(const Gecode::Space& _s) {
    const TestSpace& s = static_cast<const TestSpace&>(_s);
    int i = x.size()-1;
    while (true) {
      Gecode::FloatNum v = Gecode::Float::Round.add_up(s.x[i].max(),step);
      if ((v < d.max()) || (i == 0)) {
        Gecode::rel(*this, x[i], Gecode::FRT_GQ, v);
        while (i--) 
          Gecode::rel(*this, x[i], Gecode::FRT_GQ, s.x[i].max());
        return;
      }
      i--;
    }
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
      test->post(*this,x,r);
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
  TestSpace::rel(int i, Gecode::FloatRelType frt, Gecode::FloatNum n) {
    if (opt.log) {
      olog << ind(4) << "x[" << i << "] ";
      switch (frt) {
      case Gecode::FRT_EQ: olog << "="; break;
      case Gecode::FRT_LQ: olog << "<="; break;
      case Gecode::FRT_GQ: olog << ">="; break;
      }
      olog << " " << n << std::endl;
    }
    Gecode::rel(*this, x[i], frt, n);
  }

  void 
  TestSpace::rel(bool sol) {
    int n = sol ? 1 : 0;
    assert(reified);
    if (opt.log)
      olog << ind(4) << "b = " << n << std::endl;
    Gecode::rel(*this, r.var(), Gecode::IRT_EQ, n);
  }

  void 
  TestSpace::assign(const Assignment& a, bool skip) {
    using namespace Gecode;
    int i = skip ? static_cast<int>(Base::rand(a.size())) : -1;
    for (int j=a.size(); j--; )
      if (i != j) {
        rel(j, FRT_EQ, a[j]);
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
    rel(i, FRT_EQ, min ? x[i].min() : x[i].max());
  }

  void 
  TestSpace::prune(int i) {
    using namespace Gecode;
    // Prune values
    if (Base::rand(2) && !x[i].assigned()) {
      Gecode::FloatNum v=randFValUp(x[i].min(),x[i].max());
      assert((v >= x[i].min()) && (v <= x[i].max()));
      rel(i, Gecode::FRT_LQ, v);
    }
    if (Base::rand(2) && !x[i].assigned()) {
      Gecode::FloatNum v=randFValDown(x[i].min(),x[i].max());
      assert((v <= x[i].max()) && (v >= x[i].min()));
      rel(i, Gecode::FRT_GQ, v);
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
    prune(i);
  }

  bool 
  TestSpace::prune(const Assignment& a, bool testfix) {
    // Select variable to be pruned
    int i = Base::rand(x.size());
    while (x[i].assigned())
      i = (i+1) % x.size();
    // Select mode for pruning
    switch (Base::rand(2)) {
    case 0:
      if (a[i] < x[i].max()) {
        Gecode::FloatNum v=randFValDown(a[i],x[i].max());
        assert((v >= a[i]) && (v <= x[i].max()));
        rel(i, Gecode::FRT_LQ, v);
      }
      break;
    case 1:
      if (a[i] > x[i].min()) {
        Gecode::FloatNum v=randFValUp(x[i].min(),a[i]);
        assert((v <= a[i]) && (v >= x[i].min()));
        rel(i, Gecode::FRT_GQ, v);
      }
      break;
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
      if (reified && (r.var().size() != c->r.var().size())) {
        delete c; return false;
      }
      if (opt.log)
        olog << ind(3) << "Finished testing fixpoint on copy" << std::endl;
      delete c;
    }
    return true;
  }


  const Gecode::FloatRelType FloatRelTypes::frts[] =
    {Gecode::FRT_EQ,Gecode::FRT_LQ,
     Gecode::FRT_GQ};

  Assignment*
  Test::assignment(void) const {
    return new CpltAssignment(arity,dom,step);
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
  Test::post(Gecode::Space&, Gecode::FloatVarArray&,
             Gecode::Reify) {}

  bool
  Test::run(void) {
    using namespace Gecode;
    const char* test    = "NONE";
    const char* problem = "NONE";

    // Set up assignments
    Assignment* ap = assignment();
    Assignment& a = *ap;

    // Set up space for all solution search
    TestSpace* search_s_ini = new TestSpace(arity,dom,step,this,false);
    post(*search_s_ini,search_s_ini->x);
    branch(*search_s_ini,search_s_ini->x,FLOAT_VAR_NONE,FLOAT_VAL_SPLIT_MIN);
    Search::Options search_o;
    search_o.threads = 1;
    TestSpace* search_s = static_cast<TestSpace*>(search_s_ini->clone(false));

    while (a()) {
      bool sol = solution(a);
      if (opt.log) {
        olog << ind(1) << "Assignment: " << a
             << (sol ? " (solution)" : " (no solution)")
             << std::endl;
      }

      START_TEST("Assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this,false);
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
        TestSpace* s = new TestSpace(arity,dom,step,this,false);
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
        TestSpace* s = new TestSpace(arity,dom,step,this,false);
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
        TestSpace* s = new TestSpace(arity,dom,step,this,false);
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
        TestSpace* s = new TestSpace(arity,dom,step,this,false);
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
        for (ReifyModes rms; rms(); ++rms) {
          START_TEST("Assignment reified (rewrite after post)");
          TestSpace* s = new TestSpace(arity,dom,step,this,true,rms.rm());
          s->post();
          s->rel(sol);
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          delete s;
        }
        START_TEST("Assignment reified (rewrite failure)");
        {
          TestSpace* s = new TestSpace(arity,dom,step,this,true);
          s->post();
          s->rel(!sol);
          s->assign(a);
          CHECK_TEST(s->failed(), "Not failed");
          delete s;
        }
        for (ReifyModes rms; rms(); ++rms) {
          START_TEST("Assignment reified (immediate rewrite)");
          TestSpace* s = new TestSpace(arity,dom,step,this,true,rms.rm());
          s->rel(sol);
          s->post();
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          delete s;
        }
        START_TEST("Assignment reified (immediate failure)");
        {
          TestSpace* s = new TestSpace(arity,dom,step,this,true);
          s->rel(!sol);
          s->post();
          s->assign(a);
          CHECK_TEST(s->failed(), "Not failed");
          delete s;
        }
        START_TEST("Assignment reified (before posting)");
        {
          TestSpace* s = new TestSpace(arity,dom,step,this,true);
          s->assign(a);
          s->post();
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->r.var().val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Assignment reified (after posting)");
        {
          TestSpace* s = new TestSpace(arity,dom,step,this,true);
          s->post();
          s->assign(a);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->r.var().val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
        START_TEST("Prune reified");
        {
          TestSpace* s = new TestSpace(arity,dom,step,this,true);
          s->post();
          while (!s->failed() && 
                 (!s->assigned() || !s->r.var().assigned()))
            if (!s->prune(a,testfix)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(s->propagators()==0, "No subsumption");
          CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
          if (sol) {
            CHECK_TEST(s->r.var().val()==1, "Zero on solution");
          } else {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
      }

      if (testsearch) {
        if (sol) {
          START_TEST("Search");
          DFS<TestSpace> e_s(search_s,search_o);
          TestSpace* s = e_s.next();
          delete search_s;
          search_s= static_cast<TestSpace*>(search_s_ini->clone(false));
          search_s->next_as(*s);
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
      DFS<TestSpace> e_s(search_s,search_o);
      TestSpace* s = e_s.next();
      delete s;
      if (e_s.next() != NULL) {
        problem = "Excess solutions";
        goto failed;
      }
    }

    START_TEST("Bounds-consistency");
    {
      TestSpace* s = new TestSpace(arity,dom,step,this,false);
      s->post();
      for (int i = s->x.size(); i--; )
        s->prune(i);
      if (!s->failed()) {
        while (!s->failed() && !s->assigned())
          s->bound();
        CHECK_TEST(!s->failed(), "Failed");
        CHECK_TEST(s->propagators()==0, "No subsumption");
      }
      delete s;
    }

    delete ap;
    delete search_s;
    delete search_s_ini;
    return true;

  failed:
    if (opt.log)
      olog << "FAILURE" << std::endl
           << ind(1) << "Test:       " << test << std::endl
           << ind(1) << "Problem:    " << problem << std::endl;
    if (a() && opt.log)
      olog << ind(1) << "Assignment: " << a << std::endl;
    delete ap;
    delete search_s;
    delete search_s_ini;

    return false;
  }

}}

#undef START_TEST
#undef CHECK_TEST

// STATISTICS: test-float
