/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

namespace Test::Fault {

    using namespace Gecode;
    using Gecode::Support::FailPoint::Phase;

    Support::Mutex fault_mutex;

    class FaultScope {
    private:
        Support::Lock lock;

    public:
        FaultScope() : lock(fault_mutex) {
            Support::FailPoint::reset();
        }
        ~FaultScope() {
            Support::FailPoint::reset();
        }
    };

    class ThrowingPropagator : public Propagator {
    protected:
        ThrowingPropagator(Home home) : Propagator(home) {}
        ThrowingPropagator(Space& home, ThrowingPropagator& p) : Propagator(home, p) {
            Support::FailPoint::check(Phase::PROPAGATOR_COPY);
        }

    public:
        static void
        post(Home home) {
            (void)new (home) ThrowingPropagator(home);
        }
        Actor*
        copy(Space& home) override {
            return new (home) ThrowingPropagator(home, *this);
        }
        PropCost
        cost(const Space&, const ModEventDelta&) const override {
            return PropCost::unary(PropCost::LO);
        }
        void
        reschedule(Space&) override {}
        ExecStatus
        propagate(Space& home, const ModEventDelta&) override {
            return home.ES_SUBSUMED(*this);
        }
        size_t
        dispose(Space& home) override {
            (void)Propagator::dispose(home);
            return sizeof(*this);
        }
    };

    class AdvisorCopyPropagator;

    class ThrowingAdvisor : public Advisor {
    protected:
        int id;

    public:
        ThrowingAdvisor(Space& home, Propagator& p, Council<ThrowingAdvisor>& c, int id0)
            : Advisor(home, p, c), id(id0) {}
        ThrowingAdvisor(Space& home, ThrowingAdvisor& a) : Advisor(home, a), id(a.id) {
            Support::FailPoint::check(Phase::ADVISOR_COPY);
        }
        Propagator*
        owner() const {
            return &propagator();
        }
        void
        dispose(Space& home, Council<ThrowingAdvisor>& c) {
            Advisor::dispose(home, c);
        }
    };

    class AdvisorCopyPropagator : public Propagator {
    protected:
        Council<ThrowingAdvisor> c;
        AdvisorCopyPropagator(Home home) : Propagator(home), c(home) {
            (void)new (home) ThrowingAdvisor(home, *this, c, 0);
            (void)new (home) ThrowingAdvisor(home, *this, c, 1);
        }
        AdvisorCopyPropagator(Space& home, AdvisorCopyPropagator& p) : Propagator(home, p) {
            c.update(home, p.c);
        }

    public:
        static AdvisorCopyPropagator*
        post(Home home) {
            return new (home) AdvisorCopyPropagator(home);
        }
        bool
        advisors_point_to_self() const {
            Advisors<ThrowingAdvisor> as(c);
            int n = 0;
            while (as()) {
                if (as.advisor().owner() != this)
                    return false;
                ++n;
                ++as;
            }
            return n == 2;
        }
        Actor*
        copy(Space& home) override {
            return new (home) AdvisorCopyPropagator(home, *this);
        }
        PropCost
        cost(const Space&, const ModEventDelta&) const override {
            return PropCost::unary(PropCost::LO);
        }
        void
        reschedule(Space&) override {}
        ExecStatus
        propagate(Space&, const ModEventDelta&) override {
            return ES_FIX;
        }
        size_t
        dispose(Space& home) override {
            c.dispose(home);
            (void)Propagator::dispose(home);
            return sizeof(*this);
        }
    };

    class ThrowingChoice : public Choice {
    public:
        ThrowingChoice(const Brancher& b) : Choice(b, 1) {}
        void
        archive(Archive& e) const override {
            Choice::archive(e);
        }
    };

    class ThrowingBrancher : public Brancher {
    protected:
        bool done;
        ThrowingBrancher(Home home) : Brancher(home), done(false) {}
        ThrowingBrancher(Space& home, ThrowingBrancher& b) : Brancher(home, b), done(b.done) {
            Support::FailPoint::check(Phase::BRANCHER_COPY);
        }

    public:
        static void
        post(Home home) {
            (void)new (home) ThrowingBrancher(home);
        }
        bool
        status(const Space&) const override {
            return !done;
        }
        const Choice*
        choice(Space&) override {
            return new ThrowingChoice(*this);
        }
        const Choice*
        choice(const Space&, Archive&) override {
            return new ThrowingChoice(*this);
        }
        ExecStatus
        commit(Space&, const Choice&, unsigned int) override {
            done = true;
            return ES_OK;
        }
        void
        print(const Space&, const Choice&, unsigned int, std::ostream&) const override {}
        Actor*
        copy(Space& home) override {
            return new (home) ThrowingBrancher(home, *this);
        }
        size_t
        dispose(Space&) override {
            return sizeof(*this);
        }
    };

    class CloneCopySpace : public Space {
    public:
        CloneCopySpace() {
            ThrowingPropagator::post(*this);
            ThrowingPropagator::post(*this);
            ThrowingBrancher::post(*this);
            ThrowingBrancher::post(*this);
        }
        // NOLINTNEXTLINE(modernize-use-equals-default)
        CloneCopySpace(CloneCopySpace& s) : Space(s) {}
        Space*
        copy() override {
            return new CloneCopySpace(*this);
        }
    };

    class DisposeSpace : public Space {
    public:
        IntVarArray x;
        DisposeSpace() : x(*this, 3, 0, 2) {
            branch(*this, x, INT_VAR_ACTION_SIZE_MAX(), INT_VAL_MIN());
        }
        DisposeSpace(DisposeSpace& s) : Space(s) {
            x.update(*this, s.x);
        }
        Space*
        copy() override {
            return new DisposeSpace(*this);
        }
    };

    class NoticeDisposeActor : public Actor {
    public:
        static int disposed;

        NoticeDisposeActor(Home home) : Actor() {
            home.notice(*this, AP_DISPOSE);
        }
        static void
        post(Home home) {
            (void)new (home) NoticeDisposeActor(home);
        }
        Actor*
        copy(Space& home) override {
            return new (home) NoticeDisposeActor(home);
        }
        size_t
        dispose(Space& home) override {
            home.ignore(*this, AP_DISPOSE);
            disposed++;
            return sizeof(*this);
        }
    };

    int NoticeDisposeActor::disposed = 0;

    class NoticeDisposeSpace : public Space {
    public:
        NoticeDisposeSpace(int registered = 0, bool fail_next = false) {
            for (int i = 0; i < registered; i++)
                NoticeDisposeActor::post(*this);
            if (fail_next) {
                Support::FailPoint::fail_after(Phase::SPACE_DISPOSE_NOTICE_ARRAY, 0);
                NoticeDisposeActor::post(*this);
            } else if (registered == 0) {
                NoticeDisposeActor::post(*this);
            }
        }
        // NOLINTNEXTLINE(modernize-use-equals-default)
        NoticeDisposeSpace(NoticeDisposeSpace& s) : Space(s) {}
        Space*
        copy() override {
            return new NoticeDisposeSpace(*this);
        }
    };

    class MiniModelSpace : public Space {
    public:
        MiniModelSpace() = default;
        // NOLINTNEXTLINE(modernize-use-equals-default)
        MiniModelSpace(MiniModelSpace& s) : Space(s) {}
        Space*
        copy() override {
            return new MiniModelSpace(*this);
        }
    };

    class DerivedCopySpace : public Space {
    public:
        IntVarArray x;
        DerivedCopySpace() : x(*this, 2, 0, 1) {
            ThrowingPropagator::post(*this);
            ThrowingBrancher::post(*this);
        }
        DerivedCopySpace(DerivedCopySpace& s) : Space(s) {
            Support::FailPoint::check(Phase::DERIVED_SPACE_COPY);
            x.update(*this, s.x);
        }
        Space*
        copy() override {
            return new DerivedCopySpace(*this);
        }
    };

    class DerivedUpdateSpace : public Space {
    public:
        IntVarArray x;
        DerivedUpdateSpace() : x(*this, 2, 0, 1) {
            rel(*this, x[0] != x[1]);
            branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
        }
        DerivedUpdateSpace(DerivedUpdateSpace& s) : Space(s) {
            x.update(*this, s.x);
            Support::FailPoint::check(Phase::DERIVED_SPACE_COPY);
        }
        Space*
        copy() override {
            return new DerivedUpdateSpace(*this);
        }
    };

    class FaultLocalObject : public LocalObject {
    public:
        int value;
        FaultLocalObject(Home home, int v) : LocalObject(home), value(v) {}
        FaultLocalObject(Space& home, FaultLocalObject& o) : LocalObject(home, o), value(o.value) {
            Support::FailPoint::check(Phase::LOCAL_OBJECT_COPY);
        }
        Actor*
        copy(Space& home) override {
            return new (home) FaultLocalObject(home, *this);
        }
        size_t
        dispose(Space&) override {
            return sizeof(*this);
        }
    };

    class FaultLocalHandle : public LocalHandle {
    public:
        FaultLocalHandle() = default;
        FaultLocalHandle(FaultLocalObject* o) : LocalHandle(o) {}
        void
        update(Space& home, FaultLocalHandle& h) {
            LocalHandle::update(home, h);
        }
        int
        value() const {
            return static_cast<FaultLocalObject*>(object())->value;
        }
    };

    class LocalCopySpace : public Space {
    public:
        FaultLocalHandle h;
        LocalCopySpace() : h(new (*this) FaultLocalObject(*this, 42)) {}
        LocalCopySpace(LocalCopySpace& s) : Space(s) {
            h.update(*this, s.h);
        }
        Space*
        copy() override {
            return new LocalCopySpace(*this);
        }
    };

    class AdvisorCopySpace : public Space {
    public:
        AdvisorCopyPropagator* p;
        AdvisorCopySpace() : p(AdvisorCopyPropagator::post(*this)) {}
        AdvisorCopySpace(AdvisorCopySpace& s) : Space(s), p(nullptr) {}
        Space*
        copy() override {
            return new AdvisorCopySpace(*this);
        }
        bool
        advisors_point_to_source() const {
            return (p != nullptr) && p->advisors_point_to_self();
        }
    };

    class BranchPostSpace : public Space {
    public:
        IntVarArray x;
        BranchPostSpace() : x(*this, 4, 0, 3) {
            rel(*this, x[0] != x[1]);
        }
        BranchPostSpace(BranchPostSpace& s) : Space(s) {
            x.update(*this, s.x);
        }
        Space*
        copy() override {
            return new BranchPostSpace(*this);
        }
        void
        post_action_branch() {
            branch(*this, x, INT_VAR_ACTION_SIZE_MAX(), INT_VAL_MIN());
        }
        void
        post_chb_branch() {
            branch(*this, x, INT_VAR_CHB_SIZE_MAX(), INT_VAL_MIN());
        }
        void
        post_plain_branch() {
            branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
        }
    };

    bool
    clone_after_failed_copy(Phase p, unsigned long long n) {
        FaultScope scope;
        CloneCopySpace s;
        if (s.status() == SS_FAILED)
            return false;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(p, n);
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
            s.commit(*ch, 0);
            delete ch;
        }
        return ok && (s.status() != SS_FAILED);
    }

    bool
    expect_memory_exhausted(Phase p, unsigned long long n) {
        FaultScope scope;
        DerivedCopySpace s;
        if (s.status() == SS_FAILED)
            return false;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(p, n);
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
                s.commit(*ch, 0);
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

    bool
    clone_after_failed_local_object_copy() {
        FaultScope scope;
        LocalCopySpace s;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::LOCAL_OBJECT_COPY, 0);
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

    bool
    clone_after_failed_advisor_copy() {
        FaultScope scope;
        AdvisorCopySpace s;
        if (!s.advisors_point_to_source())
            return false;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::ADVISOR_COPY, 1);
        try {
            Space* c = s.clone();
            delete c;
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            if (!s.advisors_point_to_source())
                return false;
            AdvisorCopySpace* c = static_cast<AdvisorCopySpace*>(s.clone());
            bool ok = (c != nullptr);
            delete c;
            return ok && s.advisors_point_to_source();
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    bool
    clone_after_failed_disposal_array() {
        FaultScope scope;
        DisposeSpace s;
        if (s.status() == SS_FAILED)
            return false;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::SPACE_DISPOSAL_ARRAY, 0);
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
        DFS<DisposeSpace> e(&s);
        Space* sol = e.next();
        bool ok = (sol != nullptr);
        delete sol;
        return ok;
    }

    bool
    derived_update_space_is_usable(DerivedUpdateSpace& s) {
        Space* c = s.clone();
        delete c;
        DFS<DerivedUpdateSpace> e(&s);
        Space* sol = e.next();
        bool ok = (sol != nullptr);
        delete sol;
        return ok;
    }

    bool
    clone_after_failed_derived_update() {
        FaultScope scope;
        DerivedUpdateSpace s;
        if (s.status() == SS_FAILED)
            return false;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::DERIVED_SPACE_COPY, 0);
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
        return derived_update_space_is_usable(s);
    }

    bool
    heap_allocation_failpoint_throws() {
        FaultScope scope;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::HEAP, 0);
        try {
            int* x = heap.alloc<int>(1);
            heap.free<int>(x, 1);
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            return true;
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    bool
    heap_reallocation_failpoint_throws() {
        FaultScope scope;
        int* x = heap.alloc<int>(1);
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::HEAP, 0);
        try {
            int* y = heap.realloc<int>(x, 1, 2);
            Support::FailPoint::reset();
            heap.free<int>(y, 2);
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            heap.free<int>(x, 1);
            return true;
        } catch (...) {
            Support::FailPoint::reset();
            heap.free<int>(x, 1);
            return false;
        }
    }

    bool
    clone_heap_failures_recover_source() {
        FaultScope scope;
        DerivedUpdateSpace s;
        if (s.status() == SS_FAILED)
            return false;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 32; budget++) {
            Support::FailPoint::reset();
            Support::FailPoint::fail_after(Phase::HEAP, budget);
            try {
                Space* c = s.clone();
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                delete c;
                return saw_failure && (checks > 0) && derived_update_space_is_usable(s);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
                if (!derived_update_space_is_usable(s))
                    return false;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }

    bool
    branch_post_heap_failures_are_recoverable(bool chb) {
        FaultScope scope;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 32; budget++) {
            BranchPostSpace s;
            if (s.status() == SS_FAILED)
                return false;
            Support::FailPoint::reset();
            Support::FailPoint::fail_after(Phase::HEAP, budget);
            try {
                if (chb)
                    s.post_chb_branch();
                else
                    s.post_action_branch();
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                DFS<BranchPostSpace> e(&s);
                Space* sol = e.next();
                bool ok = (sol != nullptr);
                delete sol;
                return saw_failure && (checks > 0) && ok;
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
                s.post_plain_branch();
                DFS<BranchPostSpace> e(&s);
                Space* sol = e.next();
                bool ok = (sol != nullptr);
                delete sol;
                if (!ok)
                    return false;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }

    bool
    int_set_heap_failures_release_object() {
        FaultScope scope;
        int ranges[3][2] = {{0, 0}, {2, 2}, {4, 4}};
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 8; budget++) {
            IntSet::fault_reset_allocations();
            Support::FailPoint::reset();
            Support::FailPoint::fail_after(Phase::HEAP, budget);
            try {
                unsigned long long checks;
                {
                    IntSet s(ranges, 3);
                    checks = Support::FailPoint::count();
                }
                Support::FailPoint::reset();
                return saw_failure && (checks > 0) && (IntSet::fault_live_allocations() == 0);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
                if (IntSet::fault_live_allocations() != 0)
                    return false;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }

    bool
    minimodel_heap_failures_release_nodes() {
        FaultScope scope;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 32; budget++) {
            LinIntExpr::fault_reset_allocations();
            Support::FailPoint::reset();
            try {
                MiniModelSpace home;
                IntVarArgs x(home, 4, 0, 3);
                Support::FailPoint::fail_after(Phase::HEAP, budget);
                {
                    LinIntExpr e = min(x);
                    (void)e;
                }
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                return saw_failure && (checks > 0) && (LinIntExpr::fault_live_allocations() == 0);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
                if (LinIntExpr::fault_live_allocations() != 0)
                    return false;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }

    bool
    linear_sum_constructor_heap_failures_release_nodes(int kind) {
        FaultScope scope;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 8; budget++) {
            LinIntExpr::fault_reset_allocations();
            Support::FailPoint::reset();
            try {
                MiniModelSpace home;
                IntVarArgs x(home, 4, 0, 3);
                BoolVarArgs b(home, 4, 0, 1);
                IntArgs a({1, 2, 3, 4});
                Support::FailPoint::fail_after(Phase::HEAP, budget);
                switch (kind) {
                case 0: {
                    LinIntExpr e(x);
                    (void)e;
                } break;
                case 1: {
                    LinIntExpr e(a, x);
                    (void)e;
                } break;
                case 2: {
                    LinIntExpr e(b);
                    (void)e;
                } break;
                case 3: {
                    LinIntExpr e(a, b);
                    (void)e;
                } break;
                default:
                    return false;
                }
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                return saw_failure && (checks > 0) && (LinIntExpr::fault_live_allocations() == 0);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
                if (LinIntExpr::fault_live_allocations() != 0)
                    return false;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }

    bool
    minimodel_linear_sum_heap_failures_release_nodes() {
        for (int kind = 0; kind < 4; kind++)
            if (!linear_sum_constructor_heap_failures_release_nodes(kind))
                return false;
        return true;
    }

#ifdef GECODE_HAS_FLOAT_VARS
    bool
    minimodel_float_heap_failures_are_recoverable() {
        FaultScope scope;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 32; budget++) {
            Support::FailPoint::reset();
            try {
                MiniModelSpace home;
                FloatVarArgs x(home, 4, 0.0, 3.0);
                Support::FailPoint::fail_after(Phase::HEAP, budget);
                {
                    LinFloatExpr e = min(x);
                    (void)e;
                }
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                return saw_failure && (checks > 0);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }
#endif

#ifdef GECODE_HAS_SET_VARS
    bool
    minimodel_set_heap_failures_are_recoverable() {
        FaultScope scope;
        bool saw_failure = false;
        for (unsigned long long budget = 0; budget < 32; budget++) {
            Support::FailPoint::reset();
            try {
                MiniModelSpace home;
                SetVarArgs x(home, 4, IntSet::empty, 1, 1);
                Support::FailPoint::fail_after(Phase::HEAP, budget);
                {
                    SetExpr e = set_union(x);
                    (void)e;
                }
                unsigned long long checks = Support::FailPoint::count();
                Support::FailPoint::reset();
                return saw_failure && (checks > 0);
            } catch (const MemoryExhausted&) {
                Support::FailPoint::reset();
                saw_failure = true;
            } catch (...) {
                Support::FailPoint::reset();
                return false;
            }
        }
        return false;
    }
#endif

    bool
    dispose_notice_initial_failure_does_not_dispose_actor() {
        FaultScope scope;
        NoticeDisposeActor::disposed = 0;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::SPACE_DISPOSE_NOTICE_ARRAY, 0);
        try {
            NoticeDisposeSpace s;
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            return NoticeDisposeActor::disposed == 0;
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    bool
    dispose_notice_resize_failure_does_not_dispose_unregistered_actor() {
        FaultScope scope;
        NoticeDisposeActor::disposed = 0;
        Support::FailPoint::reset();
        try {
            NoticeDisposeSpace s(4, true);
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            return NoticeDisposeActor::disposed == 4;
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    bool
    minimodel_failure_releases_default_nodes() {
        FaultScope scope;
        LinIntExpr::fault_reset_allocations();
        Support::FailPoint::reset();
        try {
            MiniModelSpace home;
            IntVarArgs x(home, 2, 0, 1);
            Support::FailPoint::fail_after(Phase::MINI_MODEL, 1);
            LinIntExpr e = min(x);
            (void)e;
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            return LinIntExpr::fault_live_allocations() == 0;
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    class FaultBoolMisc : public BoolExpr::Misc {
    public:
        static int disposed;

        FaultBoolMisc() = default;
        ~FaultBoolMisc() override {
            disposed++;
        }
        void
        post(Home, BoolVar, bool, const IntPropLevels&) override {}
    };

    int FaultBoolMisc::disposed = 0;

    bool
    minimodel_heap_failure_releases_bool_misc() {
        FaultScope scope;
        FaultBoolMisc::disposed = 0;
        FaultBoolMisc* m = new FaultBoolMisc;
        Support::FailPoint::reset();
        Support::FailPoint::fail_after(Phase::HEAP, 0);
        try {
            BoolExpr b(m);
            (void)b;
            Support::FailPoint::reset();
            return false;
        } catch (const MemoryExhausted&) {
            Support::FailPoint::reset();
            return FaultBoolMisc::disposed == 1;
        } catch (...) {
            Support::FailPoint::reset();
            return false;
        }
    }

    class MiniModelDefaultNodes : public Base {
    public:
        MiniModelDefaultNodes() : Base("Fault::MiniModel::DefaultNodes") {}
        bool
        run() override {
            return minimodel_failure_releases_default_nodes();
        }
    };

    class DisposeNoticeArray : public Base {
    public:
        DisposeNoticeArray() : Base("Fault::Dispose::NoticeArray") {}
        bool
        run() override {
            return dispose_notice_initial_failure_does_not_dispose_actor();
        }
    };

    class DisposeNoticeArrayResize : public Base {
    public:
        DisposeNoticeArrayResize() : Base("Fault::Dispose::NoticeArrayResize") {}
        bool
        run() override {
            return dispose_notice_resize_failure_does_not_dispose_unregistered_actor();
        }
    };

    class CloneDisposalArray : public Base {
    public:
        CloneDisposalArray() : Base("Fault::Clone::DisposalArray") {}
        bool
        run() override {
            return clone_after_failed_disposal_array();
        }
    };

    class CloneDerivedSpaceUpdate : public Base {
    public:
        CloneDerivedSpaceUpdate() : Base("Fault::Clone::DerivedSpaceUpdate") {}
        bool
        run() override {
            return clone_after_failed_derived_update();
        }
    };

    class HeapAllocation : public Base {
    public:
        HeapAllocation() : Base("Fault::Heap::Allocation") {}
        bool
        run() override {
            return heap_allocation_failpoint_throws();
        }
    };

    class HeapReallocation : public Base {
    public:
        HeapReallocation() : Base("Fault::Heap::Reallocation") {}
        bool
        run() override {
            return heap_reallocation_failpoint_throws();
        }
    };

    class CloneHeapFailures : public Base {
    public:
        CloneHeapFailures() : Base("Fault::Clone::HeapFailures") {}
        bool
        run() override {
            return clone_heap_failures_recover_source();
        }
    };

    class BranchActionHeapFailures : public Base {
    public:
        BranchActionHeapFailures() : Base("Fault::Branch::ActionHeapFailures") {}
        bool
        run() override {
            return branch_post_heap_failures_are_recoverable(false);
        }
    };

    class BranchChbHeapFailures : public Base {
    public:
        BranchChbHeapFailures() : Base("Fault::Branch::ChbHeapFailures") {}
        bool
        run() override {
            return branch_post_heap_failures_are_recoverable(true);
        }
    };

    class IntSetHeapFailures : public Base {
    public:
        IntSetHeapFailures() : Base("Fault::IntSet::HeapFailures") {}
        bool
        run() override {
            return int_set_heap_failures_release_object();
        }
    };

    class MiniModelHeapFailures : public Base {
    public:
        MiniModelHeapFailures() : Base("Fault::MiniModel::HeapFailures") {}
        bool
        run() override {
            return minimodel_heap_failures_release_nodes();
        }
    };

    class MiniModelLinearSumHeapFailures : public Base {
    public:
        MiniModelLinearSumHeapFailures() : Base("Fault::MiniModel::LinearSumHeapFailures") {}
        bool
        run() override {
            return minimodel_linear_sum_heap_failures_release_nodes();
        }
    };

    class MiniModelBoolMiscHeapFailure : public Base {
    public:
        MiniModelBoolMiscHeapFailure() : Base("Fault::MiniModel::BoolMiscHeapFailure") {}
        bool
        run() override {
            return minimodel_heap_failure_releases_bool_misc();
        }
    };

#ifdef GECODE_HAS_FLOAT_VARS
    class MiniModelFloatHeapFailures : public Base {
    public:
        MiniModelFloatHeapFailures() : Base("Fault::MiniModel::FloatHeapFailures") {}
        bool
        run() override {
            return minimodel_float_heap_failures_are_recoverable();
        }
    };
#endif

#ifdef GECODE_HAS_SET_VARS
    class MiniModelSetHeapFailures : public Base {
    public:
        MiniModelSetHeapFailures() : Base("Fault::MiniModel::SetHeapFailures") {}
        bool
        run() override {
            return minimodel_set_heap_failures_are_recoverable();
        }
    };
#endif

    class ClonePropagatorCopy : public Base {
    public:
        ClonePropagatorCopy() : Base("Fault::Clone::PropagatorCopy") {}
        bool
        run() override {
            return clone_after_failed_copy(Phase::PROPAGATOR_COPY, 0) &&
                   clone_after_failed_copy(Phase::PROPAGATOR_COPY, 1);
        }
    };

    class CloneBrancherCopy : public Base {
    public:
        CloneBrancherCopy() : Base("Fault::Clone::BrancherCopy") {}
        bool
        run() override {
            return clone_after_failed_copy(Phase::BRANCHER_COPY, 0) && clone_after_failed_copy(Phase::BRANCHER_COPY, 1);
        }
    };

    class CloneAdvisorCopy : public Base {
    public:
        CloneAdvisorCopy() : Base("Fault::Clone::AdvisorCopy") {}
        bool
        run() override {
            return clone_after_failed_advisor_copy();
        }
    };

    class CloneDerivedSpaceCopy : public Base {
    public:
        CloneDerivedSpaceCopy() : Base("Fault::Clone::DerivedSpaceCopy") {}
        bool
        run() override {
            return expect_memory_exhausted(Phase::DERIVED_SPACE_COPY, 0);
        }
    };

    class CloneLocalObjectCopy : public Base {
    public:
        CloneLocalObjectCopy() : Base("Fault::Clone::LocalObjectCopy") {}
        bool
        run() override {
            return clone_after_failed_local_object_copy();
        }
    };

    BranchActionHeapFailures branch_action_heap_failures;
    BranchChbHeapFailures branch_chb_heap_failures;
    CloneDisposalArray clone_disposal_array;
    CloneDerivedSpaceUpdate clone_derived_space_update;
    CloneHeapFailures clone_heap_failures;
    DisposeNoticeArray dispose_notice_array;
    DisposeNoticeArrayResize dispose_notice_array_resize;
    HeapAllocation heap_allocation;
    HeapReallocation heap_reallocation;
    IntSetHeapFailures int_set_heap_failures;
    MiniModelDefaultNodes minimodel_default_nodes;
    MiniModelBoolMiscHeapFailure minimodel_bool_misc_heap_failure;
    MiniModelHeapFailures minimodel_heap_failures;
#ifdef GECODE_HAS_FLOAT_VARS
    MiniModelFloatHeapFailures minimodel_float_heap_failures;
#endif
#ifdef GECODE_HAS_SET_VARS
    MiniModelSetHeapFailures minimodel_set_heap_failures;
#endif
    ClonePropagatorCopy clone_propagator_copy;
    CloneBrancherCopy clone_brancher_copy;
    CloneAdvisorCopy clone_advisor_copy;
    CloneDerivedSpaceCopy clone_derived_space_copy;
    CloneLocalObjectCopy clone_local_object_copy;
    MiniModelLinearSumHeapFailures minimodel_linear_sum_heap_failures;

} // namespace Test::Fault

// STATISTICS: test-fault
