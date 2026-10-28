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
#include <gecode/driver.hh>
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/branch.hh>
#include <gecode/search.hh>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace Test { namespace FlatZinc { namespace {
    /// Integer and Boolean variables share one randomized value brancher.
    class IntBoolSpace : public Gecode::Space {
    public:
        Gecode::IntVarArray x;
        Gecode::BoolVarArray b;
        IntBoolSpace(void) : x(*this, 5, 0, 3), b(*this, 3, 0, 1) {
            using namespace Gecode;
            Gecode::FlatZinc::branch(*this, x, b, Gecode::FlatZinc::int_bool_var_afc_size_max(0.99),
                                     INT_VAL_RND(Rnd(3)));
        }
        IntBoolSpace(IntBoolSpace& s) : Space(s) {
            x.update(*this, s.x);
            b.update(*this, s.b);
        }
        Space*
        copy(void) override {
            return new IntBoolSpace(*this);
        }
    };

    std::string
    solutions(unsigned int distance) {
        Gecode::Search::Options options;
        options.c_d = distance;
        options.a_d = distance;
        IntBoolSpace root;
        Gecode::DFS<IntBoolSpace> search(&root, options);
        std::ostringstream result;
        for (int i = 0; i < 40; i++) {
            std::unique_ptr<IntBoolSpace> s(search.next());
            if (!s)
                return "";
            result << s->x << s->b << ';';
        }
        return result.str();
    }

    /// Check that random value selection does not depend on recomputation
    class RandomIntBoolBranch : public Base {
    public:
        RandomIntBoolBranch(void) : Base("FlatZinc::random_intbool_branch") {}
        bool
        run(void) override {
            // solutions() is empty unless it finds all 40 solutions
            const std::string cloned = solutions(1);
            return !cloned.empty() && (cloned == solutions(4)) && (cloned == solutions(100));
        }
    } random_intbool_branch;

    /// Return the initial random state after parsing \a args with options \a Opt
    template <class Opt>
    std::string
    parsed_state(std::vector<std::string> args) {
        Opt opt("random-options");
        args.insert(args.begin(), "random-options");
        std::vector<char*> argv;
        argv.reserve(args.size());
        for (std::string& a : args)
            argv.push_back(a.data());
        int argc = static_cast<int>(argv.size());
        opt.parse(argc, argv.data());
        return opt.rnd().state();
    }

    /// Check that seeds keep all 64 bits and that -state restores any state
    template <class Opt>
    bool
    random_options(const std::string& seed) {
        const std::string state = parsed_state<Opt>({seed, "18446744073709551615"});
        const std::string arbitrary = "splitmix-v1:fedcba9876543210:0123456789abcdef";
        return (state.rfind("splitmix-v1:ffffffffffffffff:", 0) == 0) &&
               (parsed_state<Opt>({seed, "0xffffffffffffffff"}) == state) &&
               (parsed_state<Opt>({"-state", state}) == state) &&
               (parsed_state<Opt>({"-state", arbitrary}) == arbitrary);
    }

    /// Script options with a second random option and its own state option
    class ModelRandomOptions : public Gecode::Options {
    public:
        Gecode::Driver::RandomOption model;
        explicit ModelRandomOptions(const char* name)
            : Gecode::Options(name), model("model-seed", "model generator seed", 5U, "model-state") {
            add(model);
        }
    };

    /// Check that each random option takes only its own state option
    bool
    own_state_options(void) {
        const std::string a = "splitmix-v1:fedcba9876543210:0123456789abcdef";
        const std::string b = "splitmix-v1:0123456789abcdef:fedcba9876543211";
        ModelRandomOptions opt("random-options");
        std::vector<std::string> args = {"random-options", "-model-state", b, "-state", a};
        std::vector<char*> argv;
        argv.reserve(args.size());
        for (std::string& s : args)
            argv.push_back(s.data());
        int argc = static_cast<int>(argv.size());
        opt.parse(argc, argv.data());
        return (argc == 1) && (opt.rnd().state() == a) && (opt.model.rnd().state() == b);
    }

    /// Check the random options of the driver and of FlatZinc
    class RandomOptions : public Base {
    public:
        RandomOptions(void) : Base("FlatZinc::random_options") {}
        bool
        run(void) override {
            return random_options<Gecode::Options>("-seed") &&
                   random_options<Gecode::FlatZinc::FlatZincOptions>("-r") &&
                   (parsed_state<Gecode::FlatZinc::FlatZincOptions>({"-r", "-1"}) ==
                    parsed_state<Gecode::FlatZinc::FlatZincOptions>({"-r", "18446744073709551615"})) &&
                   own_state_options();
        }
    } random_options_test;
}}} // namespace Test::FlatZinc

// STATISTICS: test-flatzinc
