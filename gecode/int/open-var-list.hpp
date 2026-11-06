/*
 *  Main author:
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

#include <utility>

namespace Gecode {

    template <class Var>
    class OpenVarList<Var>::Object : public LocalObject {
    public:
        class Functions {
        public:
            Factory create;
            Transition transition;

            Functions(Factory create0, Transition transition0)
                : create(std::move(create0)), transition(std::move(transition0)) {}
        };

        IntVar length;
        // A separate event source reports appended positions even when length.min()
        // already requires them and therefore does not change during construction.
        IntVar materialized;
        Var* x;
        int n;
        int capacity;
        using VarImpSet = std::unordered_set<const void*>;
        VarImpSet identities;
        // Only committed positions are indexed. Assigned variables need no alias
        // check, so their entries need not be removed.
        int indexed;
        SharedData<Functions> functions;

        static int
        checked_maximum(int max) {
            Int::Limits::check(max, "OpenVarList");
            if (max < 0)
                throw Int::VariableEmptyDomain("OpenVarList");
            return max;
        }

        static Factory
        checked(Factory create) {
            if (!create)
                throw InvalidFunction("OpenVarList");
            return create;
        }

        static Transition
        checked(Transition t) {
            if (!t)
                throw InvalidFunction("OpenVarList");
            return t;
        }

        Object(Home home, Factory create, int max)
            : LocalObject(home), length(home, 0, checked_maximum(max)), materialized(home, 0, max), x(nullptr), n(0),
              capacity(0), indexed(0), functions(Functions(checked(std::move(create)), Transition())) {
            home.notice(*this, AP_DISPOSE);
        }

        Object(Home home, Factory create, Transition t, int max)
            : LocalObject(home), length(home, 0, checked_maximum(max)), materialized(home, 0, max), x(nullptr), n(0),
              capacity(0), indexed(0), functions(Functions(checked(std::move(create)), checked(std::move(t)))) {
            home.notice(*this, AP_DISPOSE);
        }

        Object(Space& home, Object& s)
            : LocalObject(home, s), x(nullptr), n(s.n), capacity(s.n), indexed(0), functions(s.functions) {
            length.update(home, s.length);
            materialized.update(home, s.materialized);
            if (capacity > 0) {
                x = heap.alloc<Var>(capacity);
                try {
                    for (int i = 0; i < n; i++)
                        x[i].update(home, s.x[i]);
                } catch (...) {
                    heap.free<Var>(x, capacity);
                    throw;
                }
            }
        }

        Actor*
        copy(Space& home) override {
            return new (home) Object(home, *this);
        }

        size_t
        dispose(Space& home) override {
            home.ignore(*this, AP_DISPOSE);
            if (capacity > 0)
                heap.free<Var>(x, capacity);
            identities.~VarImpSet();
            functions.~SharedData<Functions>();
            return sizeof(*this);
        }

        void
        check_alias(Var y) {
            if (y.assigned())
                return;
            // Each clone builds its own index on the first alias check. Indexing a
            // position on the next append puts allocation failures before changes
            // to length or materialized.
            for (; indexed < n; indexed++)
                if (!x[indexed].assigned())
                    identities.insert(x[indexed].varimp());
            if (identities.find(y.varimp()) != identities.end())
                throw Int::ArgumentSame("OpenVarList::append");
        }

        void
        reserve() {
            if (n == capacity) {
                int next = (capacity < 4) ? 4 : capacity + std::min(capacity / 2, Int::Limits::max - capacity);
                x = heap.realloc<Var>(x, capacity, next);
                capacity = next;
            }
        }

        void
        append(Space& home, Var y) {
            if (home.failed())
                return;
            if (n == Int::Limits::max) {
                home.fail();
                return;
            }
            check_alias(y);
            reserve();
            const int next = n + 1;
            Int::IntView l(length);
            Int::IntView m(materialized);
            if (me_failed(l.gq(home, next)) || me_failed(m.gq(home, next))) {
                home.fail();
                return;
            }
            x[n++] = y;
            const Transition& transition = functions().transition;
            if (transition) {
                Space::NoGPI no_gpi(home);
                transition(home, OpenVarList<Var>(this), next - 1);
            }
        }

        void
        materialize(Space& home, int size) {
            Space::NoGPI no_gpi(home);
            Int::Limits::nonnegative(size, "OpenVarList::materialize");
            Int::IntView l(length);
            if (me_failed(l.gq(home, size))) {
                home.fail();
                return;
            }
            while (n < l.min()) {
                const int i = n;
                GECODE_VALID_FUNCTION(functions().create);
                Var y = functions().create(home, i);
                append(home, y);
                if (home.failed())
                    return;
            }
        }

        void
        subscribe(Space& home, Propagator& p) {
            Int::IntView(materialized).subscribe(home, p, Int::PC_INT_BND);
        }

        void
        cancel(Space& home, Propagator& p) {
            Int::IntView(materialized).cancel(home, p, Int::PC_INT_BND);
        }

        void
        reschedule(Space& home, Propagator& p) {
            Int::IntView(materialized).reschedule(home, p, Int::PC_INT_BND);
        }
    };

} // namespace Gecode

namespace Gecode { namespace Int {

    /// Materialize positions required by the minimum eventual length
    template <class Var>
    class OpenMaterialize : public Propagator, private LocalHandle {
    protected:
        using Object = OpenVarList<Var>::Object;
        IntView length;

        OpenMaterialize(Home home, Object* source) : Propagator(home), LocalHandle(source), length(source->length) {
            length.subscribe(home, *this, PC_INT_BND);
        }

        OpenMaterialize(Space& home, OpenMaterialize& p) : Propagator(home, p) {
            LocalHandle::update(home, p);
            length.update(home, p.length);
        }

    public:
        Actor*
        copy(Space& home) override {
            return new (home) OpenMaterialize(home, *this);
        }

        PropCost
        cost(const Space&, const ModEventDelta&) const override {
            const int n = length.min() - static_cast<Object*>(object())->n;
            return (n <= 1) ? PropCost::unary(PropCost::LO) : PropCost::linear(PropCost::LO, n);
        }

        void
        reschedule(Space& home) override {
            length.reschedule(home, *this, PC_INT_BND);
        }

        ExecStatus
        propagate(Space& home, const ModEventDelta&) override {
            static_cast<Object*>(object())->materialize(home, length.min());
            if (home.failed())
                return ES_FAILED;
            if (length.assigned())
                return home.ES_SUBSUMED(*this);
            return ES_FIX;
        }

        size_t
        dispose(Space& home) override {
            length.cancel(home, *this, PC_INT_BND);
            LocalHandle::~LocalHandle();
            (void)Propagator::dispose(home);
            return sizeof(*this);
        }

        static void
        post(Home home, Object* source) {
            if (!source->length.assigned())
                (void)new (home) OpenMaterialize(home, source);
        }
    };

}} // namespace Gecode::Int

namespace Gecode {

    template <class Var>
    OpenVarList<Var>::OpenVarList(Object* source) : LocalHandle(source) {}

    template <class Var>
    OpenVarList<Var>::OpenVarList(Space& home, Factory create, int max)
        : LocalHandle(new (home) Object(home, std::move(create), max)) {
        Int::OpenMaterialize<Var>::post(home, static_cast<Object*>(object()));
    }

    template <class Var>
    OpenVarList<Var>::OpenVarList(Space& home, Factory create, Transition t, int max)
        : LocalHandle(new (home) Object(home, std::move(create), std::move(t), max)) {
        Int::OpenMaterialize<Var>::post(home, static_cast<Object*>(object()));
    }

    forceinline
    OpenIntVarList::OpenIntVarList(const OpenVarList<IntVar>& x)
        : OpenVarList<IntVar>(x) {}

    forceinline
    OpenBoolVarList::OpenBoolVarList(const OpenVarList<BoolVar>& x)
        : OpenVarList<BoolVar>(x) {}

    template <class Var>
    void
    OpenVarList<Var>::update(Space& home, OpenVarList<Var>& s) {
        LocalHandle::update(home, s);
    }

    template <class Var>
    int
    OpenVarList<Var>::known_length() const {
        return static_cast<Object*>(object())->n;
    }

    template <class Var>
    Var
    OpenVarList<Var>::operator[](int i) const {
        Object* s = static_cast<Object*>(object());
        assert((i >= 0) && (i < s->n));
        return s->x[i];
    }

    template <class Var>
    IntVar
    OpenVarList<Var>::length() const {
        return static_cast<Object*>(object())->length;
    }

    template <class Var>
    void
    OpenVarList<Var>::append(Space& home, Var y) {
        if (!home.failed())
            static_cast<Object*>(object())->append(home, y);
    }

    template <class Var>
    void
    OpenVarList<Var>::materialize(Space& home, int n) {
        if (!home.failed())
            static_cast<Object*>(object())->materialize(home, n);
    }

    template <class Var>
    Var
    OpenVarList<Var>::get(Space& home, int i) {
        Int::Limits::nonnegative(i, "OpenVarList::get");
        if (home.failed())
            return Var();
        Object* s = static_cast<Object*>(object());
        if (i >= s->length.max())
            throw Int::OutOfLimits("OpenVarList::get");
        materialize(home, i + 1);
        if (home.failed())
            return Var();
        return static_cast<Object*>(object())->x[i];
    }

    template <class Var>
    void
    OpenVarList<Var>::close(Space& home) {
        if (home.failed())
            return;
        materialize(home, length().min());
        if (home.failed())
            return;
        Object* s = static_cast<Object*>(object());
        if (me_failed(Int::IntView(s->length).eq(home, s->n)))
            home.fail();
    }

    template <class Var>
    void
    OpenVarList<Var>::subscribe(Space& home, Propagator& p) {
        static_cast<Object*>(object())->subscribe(home, p);
    }

    template <class Var>
    void
    OpenVarList<Var>::cancel(Space& home, Propagator& p) {
        static_cast<Object*>(object())->cancel(home, p);
    }

    template <class Var>
    void
    OpenVarList<Var>::reschedule(Space& home, Propagator& p) {
        static_cast<Object*>(object())->reschedule(home, p);
    }

} // namespace Gecode

namespace Gecode { namespace Int {

    /// Construct an element view without changing the shared source list
    template <class View>
    forceinline View
    open_list_view(typename View::VarType x) {
        return View(x);
    }

    template <>
    forceinline MinusView
    open_list_view<MinusView>(IntVar x) {
        return MinusView(IntView(x));
    }

}} // namespace Gecode::Int

namespace Gecode {

    template <class View>
    OpenViewList<View>::OpenViewList(const OpenVarList<Var>& list) : LocalHandle(list) {}

    template <class View>
    void
    OpenViewList<View>::update(Space& home, OpenViewList<View>& s) {
        LocalHandle::update(home, s);
    }

    template <class View>
    int
    OpenViewList<View>::known_length() const {
        return static_cast<Object*>(object())->n;
    }

    template <class View>
    View
    OpenViewList<View>::operator[](int i) const {
        Object* s = static_cast<Object*>(object());
        assert((i >= 0) && (i < s->n));
        return Int::open_list_view<View>(s->x[i]);
    }

    template <class View>
    Int::IntView
    OpenViewList<View>::length() const {
        return Int::IntView(static_cast<Object*>(object())->length);
    }

    template <class View>
    void
    OpenViewList<View>::materialize(Space& home, int n) {
        if (!home.failed())
            static_cast<Object*>(object())->materialize(home, n);
    }

    template <class View>
    void
    OpenViewList<View>::subscribe(Space& home, Propagator& p) {
        static_cast<Object*>(object())->subscribe(home, p);
    }

    template <class View>
    void
    OpenViewList<View>::cancel(Space& home, Propagator& p) {
        static_cast<Object*>(object())->cancel(home, p);
    }

    template <class View>
    void
    OpenViewList<View>::reschedule(Space& home, Propagator& p) {
        static_cast<Object*>(object())->reschedule(home, p);
    }

} // namespace Gecode

// STATISTICS: int-var
