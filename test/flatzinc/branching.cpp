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

#include "test/flatzinc.hh"

#include <gecode/flatzinc/branch.hh>

#include <memory>
#include <optional>
#include <sstream>

namespace Test { namespace FlatZinc { namespace {
    typedef std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> FlatZincSpacePtr;

    /// Parse \a source and post its branchers with options \a args
    FlatZincSpacePtr
    post_branchers(const std::string& source, std::vector<std::string> args = {}) {
        Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
        std::string cmd("fzn-gecode");
        int argc = static_cast<int>(args.size()) + 1;
        std::vector<char*> argv(argc);
        argv[0] = cmd.data();
        for (int i = 1; i < argc; i++)
            argv[i] = args[i - 1].data();
        fznopt.parse(argc, argv.data());

        Gecode::FlatZinc::Printer p;
        std::stringstream ss(source);
        std::ostringstream err;
        FlatZincSpacePtr fg(Gecode::FlatZinc::parse(ss, p, err));
        if (fg)
            fg->create_branchers(p, fg->solve_annotations(), fznopt, false, err);
        return fg;
    }

    /// Return how the first choice of \a source prints for alternative \a a
    std::string
    first_choice(const std::string& source, unsigned int a) {
        FlatZincSpacePtr fg = post_branchers(source);
        if (!fg || (fg->status() != Gecode::SS_BRANCH))
            return "";
        std::unique_ptr<const Gecode::Choice> c(fg->choice());
        std::ostringstream os;
        fg->Gecode::Space::print(*c, a, os);
        return os.str();
    }

    /// Verify that the -decay option reaches the default branchers.
    class DefaultSearchDecay : public Base {
    private:
        static bool
        uses_option(const std::string& source) {
            FlatZincSpacePtr fg = post_branchers(source, {"-decay", "0.5"});
            return fg && (fg->afc_decay() == 0.5);
        }

    public:
        DefaultSearchDecay(void) : Base("FlatZinc::Branching::DefaultSearchDecay") {}

        bool
        run(void) override {
            return uses_option("var 0..1: x :: output_var;\n"
                               "var bool: b :: output_var;\n"
                               "solve satisfy;\n") &&
                   uses_option("var 0..1: x :: output_var;\nsolve satisfy;\n") &&
                   uses_option("var bool: b :: output_var;\nsolve satisfy;\n")
#ifdef GECODE_HAS_SET_VARS
                   && uses_option("var set of 1..2: s :: output_var;\nsolve satisfy;\n")
#endif
                ;
        }
    };

    DefaultSearchDecay default_search_decay;

    /// Verify that the combined variable selections keep their decay.
    class IntBoolDecay : public Base {
    private:
        static bool
        keeps_decay(const Gecode::FlatZinc::IntBoolVarBranch& b) {
            return (b.decay() == 0.5) && !b.rnd();
        }

    public:
        IntBoolDecay(void) : Base("FlatZinc::Branching::IntBoolDecay") {}

        bool
        run(void) override {
            using namespace Gecode::FlatZinc;
            return keeps_decay(int_bool_var_afc_max(0.5)) && keeps_decay(int_bool_var_afc_size_max(0.5)) &&
                   keeps_decay(int_bool_var_action_max(0.5)) && keeps_decay(int_bool_var_action_size_max(0.5));
        }
    };

    IntBoolDecay int_bool_decay;

#ifdef GECODE_HAS_SET_VARS
    /// Verify that set choices print the relations of their value selection.
    class SetRelationNames : public Base {
    private:
        static bool
        prints(const std::string& annotation, const std::string& left, const std::string& right) {
            const std::string source = "var set of 1..3: s :: output_var;\nsolve :: " + annotation + " satisfy;\n";
            return (first_choice(source, 0) == left) && (first_choice(source, 1) == right);
        }

    public:
        SetRelationNames(void) : Base("FlatZinc::Branching::SetRelationNames") {}

        bool
        run(void) override {
            return prints("set_search([s], input_order, indomain_min, complete)", "s in 1", "s not in 1") &&
                   prints("set_search([s], input_order, outdomain_min, complete)", "s not in 1", "s in 1") &&
                   prints("set_default_search(input_order, outdomain_min)", "s not in 1", "s in 1");
        }
    };

    SetRelationNames set_relation_names;
#endif

    /// Tracer remembering the first value a variable is assigned to
    class FirstAssignment : public Gecode::IntTracer {
    public:
        std::optional<int> value;

        void
        init(const Gecode::Space&, const Gecode::IntTraceRecorder&) override {}
        void
        prune(const Gecode::Space&, const Gecode::IntTraceRecorder& t, const Gecode::ViewTraceInfo&, int i,
              Gecode::IntTraceDelta&) override {
            if (!value && t[i].assigned())
                value = t[i].val();
        }
        void
        fail(const Gecode::Space&, const Gecode::IntTraceRecorder&) override {}
        void
        fix(const Gecode::Space&, const Gecode::IntTraceRecorder&) override {}
        void
        done(const Gecode::Space&, const Gecode::IntTraceRecorder&) override {}
    };

    /// Verify that a cloned auxiliary brancher keeps the default search.
    class AuxVarClone : public Base {
    public:
        AuxVarClone(void) : Base("FlatZinc::Branching::AuxVarClone") {}

        bool
        run(void) override {
            FlatZincSpacePtr fg = post_branchers("var 0..3: a :: var_is_introduced;\n"
                                                 "solve :: int_default_search(input_order, indomain_max) satisfy;\n");
            if (!fg)
                return false;
            FirstAssignment tracer;
            Gecode::trace(*fg, fg->iv_aux, Gecode::TE_PRUNE, tracer);
            if (fg->status() != Gecode::SS_BRANCH)
                return false;
            std::unique_ptr<Gecode::Space> clone(fg->clone());
            if (clone->status() != Gecode::SS_BRANCH)
                return false;
            std::unique_ptr<const Gecode::Choice> c(clone->choice());
            return tracer.value == 3;
        }
    };

    AuxVarClone aux_var_clone;
}}} // namespace Test::FlatZinc

// STATISTICS: test-flatzinc
