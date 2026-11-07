/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#ifndef GECODE_FLATZINC_AST_HH
#define GECODE_FLATZINC_AST_HH

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <exception>
#include <utility>

/**
 * \namespace Gecode::FlatZinc::AST
 * \brief Abstract syntax trees for the %FlatZinc interpreter
 */

namespace Gecode::FlatZinc::AST {

    class Call;
    class Array;
    class Atom;
    class SetLit;

    /// %Exception signaling type error
    class GECODE_VTABLE_EXPORT TypeError : public std::exception {
    private:
        std::string _what;

    public:
        TypeError() = default;
        TypeError(std::string what) : _what(std::move(what)) {}
        const char*
        what() const noexcept override {
            return _what.c_str();
        }
    };

    /**
     * \brief A node in a %FlatZinc abstract syntax tree
     */
    class GECODE_VTABLE_EXPORT Node {
    public:
        /// Destructor
        virtual ~Node();

        /// Append \a n to an array node
        void append(Node* n);

        /// Test if node has atom with \a id
        bool has_atom(const std::string& id);
        /// Test if node is int, if yes set \a i to the value
        bool is_int(int& i);
        /// Test if node is float, if yes set \a d to the value
        bool is_float(double& d);
        /// Test if node is function call with \a id
        bool is_call(const std::string& id);
        /// Return function call
        Call* get_call();
        /// Test if node is function call or array containing function call \a id
        bool has_call(const std::string& id);
        /// Return function call \a id
        Call* get_call(const std::string& id);
        /// Cast this node to an array node
        Array* get_array();
        /// Cast this node to an Atom node
        Atom* get_atom();
        /// Return name of variable represented by this node
        std::string get_var_name();
        /// Cast this node to an integer variable node
        int get_int_var();
        /// Cast this node to a Boolean variable node
        int get_bool_var();
        /// Cast this node to a Float variable node
        int get_float_var();
        /// Cast this node to a set variable node
        int get_set_var();

        /// Cast this node to an integer node
        int get_int();
        /// Cast this node to a Boolean node
        bool get_bool();
        /// Cast this node to a Float node
        double get_float();
        /// Cast this node to a set literal node
        SetLit* get_set();

        /// Cast this node to a string node
        std::string get_string();

        /// Test if node is an integer variable node
        bool is_int_var();
        /// Test if node is a Boolean variable node
        bool is_bool_var();
        /// Test if node is a set variable node
        bool is_set_var();
        /// Test if node is a float variable node
        bool is_float_var();
        /// Test if node is an integer node
        bool is_int();
        /// Test if node is a float node
        bool is_float();
        /// Test if node is a Boolean node
        bool is_bool();
        /// Test if node is a string node
        bool is_string();
        /// Test if node is an array node
        bool is_array();
        /// Test if node is a set literal node
        bool is_set();
        /// Test if node is an atom node
        bool is_atom();

        /// Output string representation
        virtual void print(std::ostream&) = 0;
    };

    /// Boolean literal node
    class GECODE_VTABLE_EXPORT BoolLit : public Node {
    public:
        bool b;
        BoolLit(bool b0) : b(b0) {}
        void
        print(std::ostream& os) override {
            os << "b(" << (b ? "true" : "false") << ")";
        }
    };
    /// Integer literal node
    class GECODE_VTABLE_EXPORT IntLit : public Node {
    public:
        int i;
        IntLit(int i0) : i(i0) {}
        void
        print(std::ostream& os) override {
            os << "i(" << i << ")";
        }
    };
    /// Float literal node
    class GECODE_VTABLE_EXPORT FloatLit : public Node {
    public:
        double d;
        FloatLit(double d0) : d(d0) {}
        void
        print(std::ostream& os) override {
            os << "f(" << d << ")";
        }
    };
    /// %Set literal node
    class GECODE_VTABLE_EXPORT SetLit : public Node {
    public:
        bool interval;
        int min;
        int max;
        std::vector<int> s;
        // NOLINTNEXTLINE(modernize-use-equals-default)
        SetLit() {}
        SetLit(int min0, int max0) : interval(true), min(min0), max(max0) {}
        SetLit(const std::vector<int>& s0) : interval(false), s(s0) {}
        explicit SetLit(SetLit* s0) : interval(s0->interval), min(s0->min), max(s0->max), s(s0->s) {}
        bool
        empty() const {
            return ((interval && min > max) || (!interval && s.empty()));
        }
        void
        print(std::ostream& os) override {
            os << "s()";
        }
    };

    /// Variable node base class
    class GECODE_VTABLE_EXPORT Var : public Node {
    public:
        int i;         //< Index
        std::string n; //< Name
        /// Constructor
        Var(int i0, const std::string& n0) : i(i0), n(n0) {}
    };
    /// Boolean variable node
    class GECODE_VTABLE_EXPORT BoolVar : public Var {
    public:
        /// Constructor
        BoolVar(int i0, const std::string& n0 = "") : Var(i0, n0) {}
        void
        print(std::ostream& os) override {
            os << "xb(" << i << ")";
        }
    };
    /// Integer variable node
    class GECODE_VTABLE_EXPORT IntVar : public Var {
    public:
        IntVar(int i0, const std::string& n0 = "") : Var(i0, n0) {}
        void
        print(std::ostream& os) override {
            os << "xi(" << i << ")";
        }
    };
    /// Float variable node
    class GECODE_VTABLE_EXPORT FloatVar : public Var {
    public:
        FloatVar(int i0, const std::string& n0 = "") : Var(i0, n0) {}
        void
        print(std::ostream& os) override {
            os << "xf(" << i << ")";
        }
    };
    /// %Set variable node
    class GECODE_VTABLE_EXPORT SetVar : public Var {
    public:
        SetVar(int i0, const std::string& n0 = "") : Var(i0, n0) {}
        void
        print(std::ostream& os) override {
            os << "xs(" << i << ")";
        }
    };

    /// %Array node
    class GECODE_VTABLE_EXPORT Array : public Node {
    public:
        std::vector<Node*> a;
        Array(const std::vector<Node*>& a0) : a(a0) {}
        Array(Node* n) : a(1) {
            a[0] = n;
        }
        Array(int n = 0) : a(n) {}
        void
        print(std::ostream& os) override {
            os << "[";
            for (unsigned int i = 0; i < a.size(); i++) {
                a[i]->print(os);
                if (i < a.size() - 1)
                    os << ", ";
            }
            os << "]";
        }
        ~Array() override {
            for (int i = a.size(); i--;)
                delete a[i];
        }
    };

    /// %Node representing a function call
    class GECODE_VTABLE_EXPORT Call : public Node {
    public:
        std::string id;
        Node* args;
        Call(const std::string& id0, Node* args0) : id(id0), args(args0) {}
        ~Call() override {
            delete args;
        }
        void
        print(std::ostream& os) override {
            os << id << "(";
            args->print(os);
            os << ")";
        }
        Array*
        get_args(unsigned int n) {
            Array* a = args->get_array();
            if (a->a.size() != n)
                throw TypeError("arity mismatch");
            return a;
        }
    };

    /// %Node representing an array access
    class GECODE_VTABLE_EXPORT ArrayAccess : public Node {
    public:
        Node* a;
        Node* idx;
        ArrayAccess(Node* a0, Node* idx0) : a(a0), idx(idx0) {}
        ~ArrayAccess() override {
            delete a;
            delete idx;
        }
        void
        print(std::ostream& os) override {
            a->print(os);
            os << "[";
            idx->print(os);
            os << "]";
        }
    };

    /// %Node representing an atom
    class GECODE_VTABLE_EXPORT Atom : public Node {
    public:
        std::string id;
        Atom(const std::string& id0) : id(id0) {}
        void
        print(std::ostream& os) override {
            os << id;
        }
    };

    /// %String node
    class GECODE_VTABLE_EXPORT String : public Node {
    public:
        std::string s;
        String(const std::string& s0) : s(s0) {}
        void
        print(std::ostream& os) override {
            os << "s(\"" << s << "\")";
        }
    };

    inline Node::~Node() = default;

    inline void
    Node::append(Node* new_node) {
        Array* a = dynamic_cast<Array*>(this);
        if (!a)
            throw TypeError("array expected");
        a->a.push_back(new_node);
    }

    inline bool
    Node::has_atom(const std::string& id) {
        if (Array* a = dynamic_cast<Array*>(this)) {
            for (int i = a->a.size(); i--;)
                if (Atom* at = dynamic_cast<Atom*>(a->a[i]))
                    if (at->id == id)
                        return true;
        } else if (Atom* a = dynamic_cast<Atom*>(this)) {
            return a->id == id;
        }
        return false;
    }

    inline bool
    Node::is_call(const std::string& id) {
        if (Call* a = dynamic_cast<Call*>(this)) {
            if (a->id == id)
                return true;
        }
        return false;
    }

    inline Call*
    Node::get_call() {
        if (Call* a = dynamic_cast<Call*>(this))
            return a;
        throw TypeError("call expected");
    }

    inline bool
    Node::has_call(const std::string& id) {
        if (Array* a = dynamic_cast<Array*>(this)) {
            for (int i = a->a.size(); i--;)
                if (Call* at = dynamic_cast<Call*>(a->a[i]))
                    if (at->id == id) {
                        return true;
                    }
        } else if (Call* a = dynamic_cast<Call*>(this)) {
            return a->id == id;
        }
        return false;
    }

    inline bool
    Node::is_int(int& i) {
        if (IntLit* il = dynamic_cast<IntLit*>(this)) {
            i = il->i;
            return true;
        }
        return false;
    }

    inline bool
    Node::is_float(double& d) {
        if (FloatLit* fl = dynamic_cast<FloatLit*>(this)) {
            d = fl->d;
            return true;
        }
        return false;
    }

    inline Call*
    Node::get_call(const std::string& id) {
        if (Array* a = dynamic_cast<Array*>(this)) {
            for (int i = a->a.size(); i--;)
                if (Call* at = dynamic_cast<Call*>(a->a[i]))
                    if (at->id == id)
                        return at;
        } else if (Call* a = dynamic_cast<Call*>(this)) {
            if (a->id == id)
                return a;
        }
        throw TypeError("call expected");
    }

    inline Array*
    Node::get_array() {
        if (Array* a = dynamic_cast<Array*>(this))
            return a;
        throw TypeError("array expected");
    }

    inline Atom*
    Node::get_atom() {
        if (Atom* a = dynamic_cast<Atom*>(this))
            return a;
        throw TypeError("atom expected");
    }

    inline std::string
    Node::get_var_name() {
        if (Var* a = dynamic_cast<Var*>(this))
            return a->n;
        throw TypeError("variable expected");
    }
    inline int
    Node::get_int_var() {
        if (IntVar* a = dynamic_cast<IntVar*>(this))
            return a->i;
        throw TypeError("integer variable expected");
    }
    inline int
    Node::get_bool_var() {
        if (BoolVar* a = dynamic_cast<BoolVar*>(this))
            return a->i;
        throw TypeError("bool variable expected");
    }
    inline int
    Node::get_float_var() {
        if (FloatVar* a = dynamic_cast<FloatVar*>(this))
            return a->i;
        throw TypeError("integer variable expected");
    }
    inline int
    Node::get_set_var() {
        if (SetVar* a = dynamic_cast<SetVar*>(this))
            return a->i;
        throw TypeError("set variable expected");
    }
    inline int
    Node::get_int() {
        if (IntLit* a = dynamic_cast<IntLit*>(this))
            return a->i;
        throw TypeError("integer literal expected");
    }
    inline bool
    Node::get_bool() {
        if (BoolLit* a = dynamic_cast<BoolLit*>(this))
            return a->b;
        throw TypeError("bool literal expected");
    }
    inline double
    Node::get_float() {
        if (FloatLit* a = dynamic_cast<FloatLit*>(this))
            return a->d;
        throw TypeError("float literal expected");
    }
    inline SetLit*
    Node::get_set() {
        if (SetLit* a = dynamic_cast<SetLit*>(this))
            return a;
        throw TypeError("set literal expected");
    }
    inline std::string
    Node::get_string() {
        if (String* a = dynamic_cast<String*>(this))
            return a->s;
        throw TypeError("string literal expected");
    }
    inline bool
    Node::is_int_var() {
        return (dynamic_cast<IntVar*>(this) != nullptr);
    }
    inline bool
    Node::is_bool_var() {
        return (dynamic_cast<BoolVar*>(this) != nullptr);
    }
    inline bool
    Node::is_set_var() {
        return (dynamic_cast<SetVar*>(this) != nullptr);
    }
    inline bool
    Node::is_float_var() {
        return (dynamic_cast<FloatVar*>(this) != nullptr);
    }
    inline bool
    Node::is_int() {
        return (dynamic_cast<IntLit*>(this) != nullptr);
    }
    inline bool
    Node::is_bool() {
        return (dynamic_cast<BoolLit*>(this) != nullptr);
    }
    inline bool
    Node::is_float() {
        return (dynamic_cast<FloatLit*>(this) != nullptr);
    }
    inline bool
    Node::is_set() {
        return (dynamic_cast<SetLit*>(this) != nullptr);
    }
    inline bool
    Node::is_string() {
        return (dynamic_cast<String*>(this) != nullptr);
    }
    inline bool
    Node::is_array() {
        return (dynamic_cast<Array*>(this) != nullptr);
    }
    inline bool
    Node::is_atom() {
        return (dynamic_cast<Atom*>(this) != nullptr);
    }

    inline Node*
    extract_singleton(Node* n) {
        if (Array* a = dynamic_cast<Array*>(n)) {
            if (a->a.size() == 1) {
                Node* ret = a->a[0];
                a->a[0] = nullptr;
                delete a;
                return ret;
            }
        }
        return n;
    }

} // namespace Gecode::FlatZinc::AST

#endif

// STATISTICS: flatzinc-any
