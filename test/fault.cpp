/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include "test/test.hh"

namespace Test { namespace Fault {

  using namespace Gecode;
  using Gecode::Support::FailPoint::Phase;

  class ThrowingPropagator : public Propagator {
  protected:
    ThrowingPropagator(Home home) : Propagator(home) {}
    ThrowingPropagator(Space& home, ThrowingPropagator& p)
      : Propagator(home,p) {
      Support::FailPoint::check(Phase::PropagatorCopy);
    }
  public:
    static void post(Home home) {
      (void) new (home) ThrowingPropagator(home);
    }
    virtual Actor* copy(Space& home) {
      return new (home) ThrowingPropagator(home,*this);
    }
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::unary(PropCost::LO);
    }
    virtual void reschedule(Space&) {}
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      return home.ES_SUBSUMED(*this);
    }
    virtual size_t dispose(Space& home) {
      (void) Propagator::dispose(home);
      return sizeof(*this);
    }
  };

  class ThrowingChoice : public Choice {
  public:
    ThrowingChoice(const Brancher& b) : Choice(b,1) {}
    virtual void archive(Archive& e) const {
      Choice::archive(e);
    }
  };

  class ThrowingBrancher : public Brancher {
  protected:
    bool done;
    ThrowingBrancher(Home home) : Brancher(home), done(false) {}
    ThrowingBrancher(Space& home, ThrowingBrancher& b)
      : Brancher(home,b), done(b.done) {
      Support::FailPoint::check(Phase::BrancherCopy);
    }
  public:
    static void post(Home home) {
      (void) new (home) ThrowingBrancher(home);
    }
    virtual bool status(const Space&) const {
      return !done;
    }
    virtual const Choice* choice(Space&) {
      return new ThrowingChoice(*this);
    }
    virtual const Choice* choice(const Space&, Archive&) {
      return new ThrowingChoice(*this);
    }
    virtual ExecStatus commit(Space&, const Choice&, unsigned int) {
      done = true;
      return ES_OK;
    }
    virtual void print(const Space&, const Choice&,
                       unsigned int, std::ostream&) const {}
    virtual Actor* copy(Space& home) {
      return new (home) ThrowingBrancher(home,*this);
    }
    virtual size_t dispose(Space&) {
      return sizeof(*this);
    }
  };

  class CloneCopySpace : public Space {
  public:
    CloneCopySpace(void) {
      ThrowingPropagator::post(*this);
      ThrowingBrancher::post(*this);
    }
    CloneCopySpace(CloneCopySpace& s) : Space(s) {}
    virtual Space* copy(void) {
      return new CloneCopySpace(*this);
    }
  };

  class DisposeSpace : public Space {
  public:
    IntVarArray x;
    DisposeSpace(void) : x(*this,3,0,2) {
      branch(*this,x,INT_VAR_ACTION_SIZE_MAX(),INT_VAL_MIN());
    }
    DisposeSpace(DisposeSpace& s) : Space(s) {
      x.update(*this,s.x);
    }
    virtual Space* copy(void) {
      return new DisposeSpace(*this);
    }
  };

  class DerivedCopySpace : public Space {
  public:
    IntVarArray x;
    DerivedCopySpace(void) : x(*this,2,0,1) {
      ThrowingPropagator::post(*this);
      ThrowingBrancher::post(*this);
    }
    DerivedCopySpace(DerivedCopySpace& s) : Space(s) {
      Support::FailPoint::check(Phase::DerivedSpaceCopy);
      x.update(*this,s.x);
    }
    virtual Space* copy(void) {
      return new DerivedCopySpace(*this);
    }
  };

  class FaultLocalObject : public LocalObject {
  public:
    int value;
    FaultLocalObject(Home home, int v) : LocalObject(home), value(v) {}
    FaultLocalObject(Space& home, FaultLocalObject& o)
      : LocalObject(home,o), value(o.value) {
      Support::FailPoint::check(Phase::LocalObjectCopy);
    }
    virtual Actor* copy(Space& home) {
      return new (home) FaultLocalObject(home,*this);
    }
    virtual size_t dispose(Space&) {
      return sizeof(*this);
    }
  };

  class FaultLocalHandle : public LocalHandle {
  public:
    FaultLocalHandle(void) {}
    FaultLocalHandle(FaultLocalObject* o) : LocalHandle(o) {}
    void update(Space& home, FaultLocalHandle& h) {
      LocalHandle::update(home,h);
    }
    int value(void) const {
      return static_cast<FaultLocalObject*>(object())->value;
    }
  };

  class LocalCopySpace : public Space {
  public:
    FaultLocalHandle h;
    LocalCopySpace(void)
      : h(new (*this) FaultLocalObject(*this,42)) {}
    LocalCopySpace(LocalCopySpace& s) : Space(s) {
      h.update(*this,s.h);
    }
    virtual Space* copy(void) {
      return new LocalCopySpace(*this);
    }
  };

  bool clone_after_failed_copy(Phase p) {
    CloneCopySpace s;
    if (s.status() == SS_FAILED)
      return false;
    Support::FailPoint::reset();
    Support::FailPoint::fail_after(p,0);
    try {
      Space* c = s.clone();
      delete c;
      Support::FailPoint::reset();
      return false;
    } catch (const MemoryExhausted&) {
      Support::FailPoint::reset();
    } catch (...) {
      Support::FailPoint::reset();
      return false;
    }
    Space* c = s.clone();
    delete c;
    const Choice* ch = s.choice();
    bool ok = ch != nullptr;
    if (ch != nullptr) {
      s.commit(*ch,0);
      delete ch;
    }
    return ok && (s.status() != SS_FAILED);
  }

  bool expect_memory_exhausted(Phase p, unsigned long long n) {
    DerivedCopySpace s;
    if (s.status() == SS_FAILED)
      return false;
    Support::FailPoint::reset();
    Support::FailPoint::fail_after(p,n);
    try {
      Space* c = s.clone();
      delete c;
    } catch (const MemoryExhausted&) {
      Support::FailPoint::reset();
      Space* c = s.clone();
      delete c;
      const Choice* ch = s.choice();
      bool ok = ch != nullptr;
      if (ch != nullptr) {
        s.commit(*ch,0);
        delete ch;
      }
      return ok && (s.status() != SS_FAILED);
    } catch (...) {
      Support::FailPoint::reset();
      return false;
    }
    Support::FailPoint::reset();
    return false;
  }

  bool clone_after_failed_local_object_copy(void) {
    LocalCopySpace s;
    Support::FailPoint::reset();
    Support::FailPoint::fail_after(Phase::LocalObjectCopy,0);
    try {
      Space* c = s.clone();
      delete c;
    } catch (const MemoryExhausted&) {
      Support::FailPoint::reset();
      if (s.h.value() != 42)
        return false;
      LocalCopySpace* c = static_cast<LocalCopySpace*>(s.clone());
      bool ok = (c != nullptr) && (c->h.value() == 42);
      delete c;
      return ok;
    } catch (...) {
      Support::FailPoint::reset();
      return false;
    }
    Support::FailPoint::reset();
    return false;
  }

  bool clone_after_failed_disposal_array(void) {
    DisposeSpace s;
    if (s.status() == SS_FAILED)
      return false;
    Support::FailPoint::reset();
    Support::FailPoint::fail_after(Phase::SpaceDisposalArray,0);
    try {
      Space* c = s.clone();
      delete c;
      Support::FailPoint::reset();
      return false;
    } catch (const MemoryExhausted&) {
      Support::FailPoint::reset();
    } catch (...) {
      Support::FailPoint::reset();
      return false;
    }
    Space* c = s.clone();
    delete c;
    DisposeSpace* root = static_cast<DisposeSpace*>(s.clone());
    DFS<DisposeSpace> e(root);
    Space* sol = e.next();
    bool ok = (sol != nullptr);
    delete sol;
    return ok;
  }

  class CloneDisposalArray : public Base {
  public:
    CloneDisposalArray(void)
      : Base("Fault::Clone::DisposalArray") {}
    virtual bool run(void) {
      return clone_after_failed_disposal_array();
    }
  };

  class ClonePropagatorCopy : public Base {
  public:
    ClonePropagatorCopy(void)
      : Base("Fault::Clone::PropagatorCopy") {}
    virtual bool run(void) {
      return clone_after_failed_copy(Phase::PropagatorCopy);
    }
  };

  class CloneBrancherCopy : public Base {
  public:
    CloneBrancherCopy(void)
      : Base("Fault::Clone::BrancherCopy") {}
    virtual bool run(void) {
      return clone_after_failed_copy(Phase::BrancherCopy);
    }
  };

  class CloneDerivedSpaceCopy : public Base {
  public:
    CloneDerivedSpaceCopy(void)
      : Base("Fault::Clone::DerivedSpaceCopy") {}
    virtual bool run(void) {
      return expect_memory_exhausted(Phase::DerivedSpaceCopy,0);
    }
  };

  class CloneLocalObjectCopy : public Base {
  public:
    CloneLocalObjectCopy(void)
      : Base("Fault::Clone::LocalObjectCopy") {}
    virtual bool run(void) {
      return clone_after_failed_local_object_copy();
    }
  };

  CloneDisposalArray clone_disposal_array;
  ClonePropagatorCopy clone_propagator_copy;
  CloneBrancherCopy clone_brancher_copy;
  CloneDerivedSpaceCopy clone_derived_space_copy;
  CloneLocalObjectCopy clone_local_object_copy;

}}

// STATISTICS: test-fault
