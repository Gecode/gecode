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
#include <iomanip>

namespace Test { namespace Float {

  /*
   * Complete assignments
   *
   */
  void
  CpltAssignment::operator++(void) {
    using namespace Gecode;
    int i = n-1;
    while (true) {
      FloatNum ns = dsv[i].min() + step;
      dsv[i] = FloatVal(ns,nextafter(ns,ns+1));
      if ((dsv[i].max() < d.max()) || (i == 0))
        return;
      dsv[i--] = FloatVal(d.min(),nextafter(d.min(),d.max()));
    }
  }

  /*
   * Extended assignments
   *
   */
  void
  ExtAssignment::operator++(void) {
    using namespace Gecode;
    assert(n > 1);
    int i = n-2;
    while (true) {
      FloatNum ns = dsv[i].min() + step;
      dsv[i] = FloatVal(ns,nextafter(ns,ns+1));
      if ((dsv[i].max() < d.max()) || (i == 0)) {
        if (curPb->extendAssignement(*this)) return;
        if ((dsv[i].max() >= d.max()) && (i == 0)) return;
        continue;
      }
      dsv[i--] = FloatVal(d.min(),nextafter(d.min(),d.max()));
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
    os << "[" << a[i].min() << "," << a[i].max() << "]" << ((i!=n-1) ? "," : "}");
  return os;
}

namespace Test { namespace Float {

  Gecode::FloatNum randFValDown(Gecode::FloatNum l, Gecode::FloatNum u) {
    using namespace Gecode;
    using namespace Gecode::Float;
    Rounding r;
    return
      r.add_down(
        l,
        r.mul_down(
                   r.div_down(
                              Base::rand(static_cast<unsigned int>(Int::Limits::max)),
                              static_cast<FloatNum>(Int::Limits::max)
                              ),
                   r.sub_down(u,l)
                   )
        );
  }

  Gecode::FloatNum randFValUp(Gecode::FloatNum l, Gecode::FloatNum u) {
    using namespace Gecode;
    using namespace Gecode::Float;
    Rounding r;
    return
      r.sub_up(
        u,
        r.mul_down(
          r.div_down(
            Base::rand(static_cast<unsigned int>(Int::Limits::max)),
            static_cast<FloatNum>(Int::Limits::max)
          ),
          r.sub_down(u,l)
        )
      );
  }


  TestSpace::TestSpace(int n, Gecode::FloatVal& d0, Gecode::FloatNum s,
                       Test* t)
    : d(d0), step(s),
      x(*this,n,Gecode::Float::Limits::min,Gecode::Float::Limits::max),
      test(t), reified(false) {
    Gecode::FloatVarArgs _x(*this,n,d.min(),d.max());
    if (x.size() == 1)
      Gecode::dom(*this,x[0],_x[0]);
    else
      Gecode::dom(*this,x,_x);
    Gecode::BoolVar b(*this,0,1);
    r = Gecode::Reify(b,Gecode::RM_EQV);
    if (opt.log)
      olog << ind(2) << "Initial: x[]=" << x
           << std::endl;
  }

  TestSpace::TestSpace(int n, Gecode::FloatVal& d0, Gecode::FloatNum s,
                       Test* t, Gecode::ReifyMode rm)
    : d(d0), step(s), x(*this,n,d.min(),d.max()), test(t), reified(true) {
    Gecode::BoolVar b(*this,0,1);
    r = Gecode::Reify(b,rm);
    if (opt.log)
      olog << ind(2) << "Initial: x[]=" << x
           << " b=" << r.var()
           << std::endl;
  }

  TestSpace::TestSpace(TestSpace& s)
    : Gecode::Space(s), d(s.d), step(s.step), test(s.test), reified(s.reified) {
    x.update(*this, s.x);
    Gecode::BoolVar b;
    Gecode::BoolVar sr(s.r.var());
    b.update(*this, sr);
    r.var(b); r.mode(s.r.mode());
  }

  Gecode::Space*
  TestSpace::copy(void) {
    return new TestSpace(*this);
  }

  void
  TestSpace::enable(void) {
    Gecode::PropagatorGroup::all.enable(*this);
  }

  void
  TestSpace::disable(void) {
    Gecode::PropagatorGroup::all.disable(*this);
    (void) status();
  }

  void
  TestSpace::dropUntil(const Assignment& a) {
    for (int i = x.size(); i--; )
      Gecode::rel(*this, x[i], Gecode::FRT_GQ, a[i].min());
  }

  bool
  TestSpace::assigned(void) const {
    for (int i=x.size(); i--; )
      if (!x[i].assigned())
        return false;
    return true;
  }

  bool
  TestSpace::matchAssignment(const Assignment& a) const {
    for (int i=x.size(); i--; )
      if ((x[i].min() < a[i].min()) && (x[i].max() > a[i].max()))
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
  TestSpace::rel(int i, Gecode::FloatRelType frt, Gecode::FloatVal n) {
    if (opt.log) {
      olog << ind(4) << "x[" << i << "] ";
      switch (frt) {
      case Gecode::FRT_EQ: olog << "="; break;
      case Gecode::FRT_NQ: olog << "!="; break;
      case Gecode::FRT_LQ: olog << "<="; break;
      case Gecode::FRT_LE: olog << "<"; break;
      case Gecode::FRT_GQ: olog << ">="; break;
      case Gecode::FRT_GR: olog << ">"; break;
      }
      olog << " [" << n.min() << "," << n.max() << "]" << std::endl;
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
  TestSpace::assign(const Assignment& a, MaybeType& sol, bool skip) {
    using namespace Gecode;
    int i = skip ? static_cast<int>(Base::rand(a.size())) : -1;

    for (int j=a.size(); j--; )
      if (i != j) {
        if ((x[j].min() == a[j].max()) || (x[j].max() == a[j].min()))
        {
          sol = MT_MAYBE;
          return;
        }
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
    if (min)
      rel(i, FRT_LQ, nextafter(x[i].min(), x[i].max()));
    else
      rel(i, FRT_GQ, nextafter(x[i].max(), x[i].min()));
  }

  Gecode::FloatNum
  TestSpace::cut(int* cutDirections) {
    using namespace Gecode;
    using namespace Gecode::Float;
    // Select variable to be cut
    int i = 0;
    while (x[i].assigned()) i++;
    for (int j=x.size(); j--; ) {
      if (!x[j].assigned() && (x[j].size() > x[i].size())) i = j;
    }
    Rounding r;
    if (cutDirections[i]) {
      FloatNum m = r.div_up(r.add_up(x[i].min(),x[i].max()),2);
      FloatNum n = nextafter(x[i].min(), x[i].max());
      if (m > n)
        rel(i, FRT_LQ, m);
      else
        rel(i, FRT_LQ, n);
    } else {
      FloatNum m = r.div_down(r.add_down(x[i].min(),x[i].max()),2);
      FloatNum n = nextafter(x[i].max(), x[i].min());
      if (m < n)
        rel(i, FRT_GQ, m);
      else
        rel(i, FRT_GQ, n);
    }
    return x[i].size();
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
      if (a[i].max() < x[i].max()) {
        Gecode::FloatNum v=randFValDown(a[i].max(),x[i].max());
        if (v==x[i].max())
          v = a[i].max();
        assert((v >= a[i].max()) && (v <= x[i].max()));
        rel(i, Gecode::FRT_LQ, v);
      }
      break;
    case 1:
      if (a[i].min() > x[i].min()) {
        Gecode::FloatNum v=randFValUp(x[i].min(),a[i].min());
        if (v==x[i].min())
          v = a[i].min();
        assert((v <= a[i].min()) && (v >= x[i].min()));
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

  unsigned int
  TestSpace::propagators(void) {
    return Gecode::PropagatorGroup::all.size(*this);
  }


  const Gecode::FloatRelType FloatRelTypes::frts[] =
    {Gecode::FRT_EQ,Gecode::FRT_NQ,Gecode::FRT_LQ,Gecode::FRT_LE,
     Gecode::FRT_GQ,Gecode::FRT_GR};

  Assignment*
  Test::assignment(void) const {
    switch (assigmentType) {
    case CPLT_ASSIGNMENT:
      return new CpltAssignment(arity,dom,step);
    case RANDOM_ASSIGNMENT:
      return new RandomAssignment(arity,dom,step);
    case EXTEND_ASSIGNMENT:
      return new ExtAssignment(arity,dom,step,this);
    default :
      GECODE_NEVER;
    }
    return NULL; // Avoid compiler warnings
  }

  bool
  Test::extendAssignement(Assignment&) const {
    GECODE_NEVER;
    return false;
  }

  bool
  Test::subsumed(const TestSpace& ts) const {
    if (!testsubsumed) return true;
    if (const_cast<TestSpace&>(ts).propagators() == 0) return true;
    if (assigmentType == EXTEND_ASSIGNMENT) return true;
    return false;
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
    TestSpace* search_s = new TestSpace(arity,dom,step,this);
    post(*search_s,search_s->x);
    branch(*search_s,search_s->x,FLOAT_VAR_NONE(),FLOAT_VAL_SPLIT_MIN());
    Search::Options search_o;
    search_o.threads = 1;
    DFS<TestSpace> * e_s = new DFS<TestSpace>(search_s,search_o);
    while (a()) {
      MaybeType sol = solution(a);
      if (opt.log) {
        olog << ind(1) << "Assignment: " << a;
        switch (sol) {
        case MT_TRUE: olog << " (solution)"; break;
        case MT_FALSE: olog << " (no solution)"; break;
        case MT_MAYBE: olog << " (maybe)"; break;
        }
        olog << std::endl;
      }
      START_TEST("Assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        TestSpace* sc = NULL;
        s->post();
        switch (Base::rand(2)) {
          case 0:
            if (opt.log)
              olog << ind(3) << "No copy" << std::endl;
            sc = s;
            s = NULL;
            break;
          case 1:
            if (opt.log)
              olog << ind(3) << "Copy" << std::endl;
            if (s->status() != SS_FAILED) {
              sc = static_cast<TestSpace*>(s->clone());
            } else {
              sc = s; s = NULL;
            }
            break;
          default: assert(false);
        }
        sc->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!sc->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*sc), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(sc->failed(), "Solved on non-solution");
        }
        delete s; delete sc;
      }
      START_TEST("Partial assignment (after posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->post();
        s->assign(a,sol,true);
        (void) s->failed();
        s->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Assignment (after posting, disable)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->post();
        s->disable();
        s->enable();
        s->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Partial assignment (after posting, disable)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->post();
        s->disable();
        s->enable();
        s->assign(a,sol,true);
        (void) s->failed();
        s->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Assignment (before posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->assign(a,sol);
        s->post();
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Partial assignment (before posting)");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->assign(a,sol,true);
        s->post();
        (void) s->failed();
        s->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      START_TEST("Prune");
      {
        TestSpace* s = new TestSpace(arity,dom,step,this);
        s->post();
        while (!s->failed() && !s->assigned() && !s->matchAssignment(a))
          if (!s->prune(a,testfix)) {
            problem = "No fixpoint";
            delete s;
            goto failed;
          }
        s->assign(a,sol);
        if (sol == MT_TRUE) {
          CHECK_TEST(!s->failed(), "Failed on solution");
          CHECK_TEST(subsumed(*s), "No subsumption");
        } else if (sol == MT_FALSE) {
          CHECK_TEST(s->failed(), "Solved on non-solution");
        }
        delete s;
      }
      if (reified && !ignore(a) && (sol != MT_MAYBE)) {
        if (eqv()) {
          START_TEST("Assignment reified (rewrite after post, <=>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_EQV);
          s->post();
          s->rel(sol == MT_TRUE);
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (imp()) {
          START_TEST("Assignment reified (rewrite after post, =>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_IMP);
          s->post();
          s->rel(sol == MT_TRUE);
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (pmi()) {
          START_TEST("Assignment reified (rewrite after post, <=)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_PMI);
          s->post();
          s->rel(sol == MT_TRUE);
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (eqv()) {
          START_TEST("Assignment reified (immediate rewrite, <=>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_EQV);
          s->rel(sol == MT_TRUE);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (imp()) {
          START_TEST("Assignment reified (immediate rewrite, =>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_IMP);
          s->rel(sol == MT_TRUE);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (pmi()) {
          START_TEST("Assignment reified (immediate rewrite, <=)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_PMI);
          s->rel(sol == MT_TRUE);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          delete s;
        }
        if (eqv()) {
          START_TEST("Assignment reified (before posting, <=>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_EQV);
          s->assign(a,sol);
          s->post();
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (s->r.var().assigned()) {
            if (sol == MT_TRUE) {
              CHECK_TEST(s->r.var().val()==1, "Zero on solution");
            } else if (sol == MT_FALSE) {
              CHECK_TEST(s->r.var().val()==0, "One on non-solution");
            }
          }
          delete s;
        }
        if (imp()) {
          START_TEST("Assignment reified (before posting, =>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_IMP);
          s->assign(a,sol);
          s->post();
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (sol == MT_TRUE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          } else if ((sol = MT_FALSE) && s->r.var().assigned()) {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
        if (pmi()) {
          START_TEST("Assignment reified (before posting, <=)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_PMI);
          s->assign(a,sol);
          s->post();
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (sol == MT_TRUE) {
            if (s->r.var().assigned()) {
              CHECK_TEST(s->r.var().val()==1, "Zero on solution");
            }
          } else if (sol == MT_FALSE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          }
          delete s;
        }
        if (eqv()) {
          START_TEST("Assignment reified (after posting, <=>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_EQV);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (s->r.var().assigned()) {
            if (sol == MT_TRUE) {
              CHECK_TEST(s->r.var().val()==1, "Zero on solution");
            } else if (sol == MT_FALSE) {
              CHECK_TEST(s->r.var().val()==0, "One on non-solution");
            }
          }
          delete s;
        }
        if (imp()) {
          START_TEST("Assignment reified (after posting, =>)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_IMP);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (sol == MT_TRUE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          } else if (s->r.var().assigned()) {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
        if (pmi()) {
          START_TEST("Assignment reified (after posting, <=)");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_PMI);
          s->post();
          s->assign(a,sol);
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (sol == MT_TRUE) {
            if (s->r.var().assigned()) {
              CHECK_TEST(s->r.var().val()==1, "Zero on solution");
            }
          } else if (sol == MT_FALSE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          }
          delete s;
        }
        if (eqv()) {
          START_TEST("Prune reified, <=>");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_EQV);
          s->post();
          while (!s->failed() && !s->matchAssignment(a) &&
                 (!s->assigned() || !s->r.var().assigned()))
            if (!s->prune(a,testfix)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (s->r.var().assigned()) {
            if (sol == MT_TRUE) {
              CHECK_TEST(s->r.var().val()==1, "Zero on solution");
            } else if (sol == MT_FALSE) {
              CHECK_TEST(s->r.var().val()==0, "One on non-solution");
            }
          }
          delete s;
        }
        if (imp()) {
          START_TEST("Prune reified, =>");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_IMP);
          s->post();
          while (!s->failed() && !s->matchAssignment(a) &&
                 (!s->assigned() || ((sol == MT_FALSE) &&
                                     !s->r.var().assigned())))
            if (!s->prune(a,testfix)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if (sol == MT_TRUE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          } else if ((sol == MT_FALSE) && s->r.var().assigned()) {
            CHECK_TEST(s->r.var().val()==0, "One on non-solution");
          }
          delete s;
        }
        if (pmi()) {
          START_TEST("Prune reified, <=");
          TestSpace* s = new TestSpace(arity,dom,step,this,RM_PMI);
          s->post();
          while (!s->failed() && !s->matchAssignment(a) &&
                 (!s->assigned() || ((sol == MT_TRUE) &&
                                     !s->r.var().assigned())))
            if (!s->prune(a,testfix)) {
              problem = "No fixpoint";
              delete s;
              goto failed;
            }
          CHECK_TEST(!s->failed(), "Failed");
          CHECK_TEST(subsumed(*s), "No subsumption");
          if ((sol == MT_TRUE) && s->r.var().assigned()) {
            CHECK_TEST(s->r.var().val()==1, "Zero on solution");
          } else if (sol == MT_FALSE) {
            CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
          }
          delete s;
        }
      }

      if (testsearch) {
        if (sol == MT_TRUE) {
          START_TEST("Search");
          if (!search_s->failed()) {
            TestSpace* ss = static_cast<TestSpace*>(search_s->clone());
            ss->dropUntil(a);
            delete e_s;
            e_s = new DFS<TestSpace>(ss,search_o);
            delete ss;
          }
          TestSpace* s = e_s->next();
          CHECK_TEST(s != NULL, "Solutions exhausted");
          CHECK_TEST(subsumed(*s), "No subsumption");
          for (int i=a.size(); i--; ) {
            CHECK_TEST(s->x[i].assigned(), "Unassigned variable");
            CHECK_TEST(Gecode::Float::overlap(a[i], s->x[i].val()), "Wrong value in solution");
          }
          delete s;
        }
      }

      ++a;
    }

    if (testsearch) {
      test = "Search";
      if (!search_s->failed()) {
        TestSpace* ss = static_cast<TestSpace*>(search_s->clone());
        ss->dropUntil(a);
        delete e_s;
        e_s = new DFS<TestSpace>(ss,search_o);
        delete ss;
      }
      if (e_s->next() != NULL) {
        problem = "Excess solutions";
        goto failed;
      }
    }

    delete ap;
    delete search_s;
    delete e_s;
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
    delete e_s;

    return false;
  }

}}

#undef START_TEST
#undef CHECK_TEST

// STATISTICS: test-float
