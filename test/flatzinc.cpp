/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2014
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

#include <memory>

namespace Test { namespace FlatZinc {

    namespace {

        /// Verify that FlatZinc table conversion uses automatic representation.
        class TupleSetAutoRepresentation : public Base {
        public:
            TupleSetAutoRepresentation() : Base("FlatZinc::TupleSet::AutoRepresentation") {}

            bool
            run() override {
                using namespace Gecode;

                const int n = 4096;
                IntArgs tuples(2 * n);
                for (int i = 0; i < n; i++) {
                    tuples[2 * i] = i;
                    tuples[2 * i + 1] = n + i;
                }

                Gecode::FlatZinc::FlatZincSpace space;
                TupleSet ts = space.arg_to_tuple_set(tuples, 2);
                return ts.finalized() && (ts.representation() == EPK_DENSE_COMPRESSED) && (ts.tuples() == n);
            }
        };

        TupleSetAutoRepresentation tuple_set_auto_representation;

        /// Verify that statistics do not override an explicit Gist mode.
        class GistStatisticsMode : public Base {
        private:
            static Gecode::ScriptMode
            mode(std::vector<std::string> args) {
                Gecode::FlatZinc::FlatZincOptions opt("Gecode/FlatZinc");
                std::string cmd("fzn-gecode");
                int argc = static_cast<int>(args.size()) + 1;
                std::vector<char*> argv(argc);
                argv[0] = const_cast<char*>(cmd.data());
                for (int i = 1; i < argc; i++)
                    argv[i] = const_cast<char*>(args[i - 1].data());
                opt.parse(argc, argv.data());
                return opt.mode();
            }

        public:
            GistStatisticsMode() : Base("FlatZinc::Options::GistStatisticsMode", TestTags::check()) {}

            bool
            run() override {
                return (mode({"-s"}) == Gecode::SM_STATISTICS) && (mode({"-mode", "gist", "-s"}) == Gecode::SM_GIST) &&
                       (mode({"-s", "-mode", "gist"}) == Gecode::SM_GIST);
            }
        };

        GistStatisticsMode gist_statistics_mode;

#ifndef GECODE_HAS_GIST
        /// Verify that unavailable Gist mode is rejected instead of running search.
        FlatZincErrorTest gist_unavailable(TestTags::check(), "Options::GistUnavailable",
                                           "var 1..1: x :: output_var;\nsolve satisfy;\n", {"-mode", "gist", "-s"},
                                           "Gist mode is unavailable in this build");
#endif

    } // namespace

    FlatZincTest::FlatZincTest(const std::string& name, const std::string& source, const std::string& expected,
                               bool all_solutions, std::vector<std::string> cmdline_opt, OutputCheck check,
                               BeforeRun before)
        : FlatZincTest(TestTags::standard(), name, source, expected, all_solutions, std::move(cmdline_opt),
                       std::move(check), std::move(before)) {}

    FlatZincTest::FlatZincTest(TestTags tags, const std::string& name, const std::string& source,
                               const std::string& expected, bool all_solutions, std::vector<std::string> cmdline_opt,
                               OutputCheck check, BeforeRun before)
        : Base("FlatZinc::" + name, tags), _name(name), _source(source), _expected(expected),
          _all_solutions(all_solutions), _cmdline_opt(std::move(cmdline_opt)), _check(std::move(check)),
          _before(std::move(before)) {}

    FlatZincErrorTest::FlatZincErrorTest(const std::string& name, const std::string& source,
                                         std::vector<std::string> cmdline_opt, std::string expected_message)
        : FlatZincErrorTest(TestTags::standard(), name, source, std::move(cmdline_opt), std::move(expected_message)) {}

    FlatZincErrorTest::FlatZincErrorTest(TestTags tags, const std::string& name, const std::string& source,
                                         std::vector<std::string> cmdline_opt, std::string expected_message)
        : FlatZincTest(tags, name, source, "", false, std::move(cmdline_opt)),
          _expected_message(std::move(expected_message)) {}

    bool
    FlatZincTest::run() {
        using namespace Gecode;
        Support::Timer t_total;
        t_total.start();
        Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
        if (!_cmdline_opt.empty()) {
            std::string cmd("fzn-gecode");
            int argc = static_cast<int>(_cmdline_opt.size()) + 1;
            std::vector<char*> argv(argc);
            argv[0] = const_cast<char*>(cmd.data());
            for (int i = 1; i < argc; ++i) {
                argv[i] = const_cast<char*>(_cmdline_opt[i - 1].data());
            }
            fznopt.parse(argc, argv.data());
        }
        fznopt.all_solutions(_all_solutions);
        Gecode::FlatZinc::Printer p;
        try {
            if (_before) {
                _before();
            }
            std::stringstream ss(_source);
            std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> fg(Gecode::FlatZinc::parse(ss, p, olog));

            if (fg) {
                fg->create_branchers(p, fg->solve_annotations(), fznopt, false, olog);
                fg->shrink_arrays(p);
                std::ostringstream os;
                fg->run(os, p, fznopt, t_total);

                const std::string output = os.str();
                fg.reset();
                if (_check ? _check(output) : (_expected == output)) {
                    return true;
                }
                if (opt.log)
                    olog << "FlatZinc produced the following output:\n" << output << "\n";
                return false;
            }
            if (opt.log)
                olog << "Could not parse input\n";
            return false;

        } catch (Gecode::FlatZinc::Error& e) {
            if (opt.log)
                olog << ind(2) << "FlatZinc error : " << e.to_string() << '\n';
            return false;
        }
        return true;
    }

    bool
    FlatZincErrorTest::run() {
        using namespace Gecode;
        Support::Timer t_total;
        t_total.start();
        Gecode::FlatZinc::FlatZincOptions fznopt("Gecode/FlatZinc");
        if (!_cmdline_opt.empty()) {
            std::string cmd("fzn-gecode");
            int argc = static_cast<int>(_cmdline_opt.size()) + 1;
            std::vector<char*> argv(argc);
            argv[0] = const_cast<char*>(cmd.data());
            for (int i = 1; i < argc; ++i) {
                argv[i] = const_cast<char*>(_cmdline_opt[i - 1].data());
            }
            fznopt.parse(argc, argv.data());
        }
        Gecode::FlatZinc::Printer p;
        std::ostringstream os;
        try {
            std::stringstream ss(_source);
            std::unique_ptr<Gecode::FlatZinc::FlatZincSpace> fg(Gecode::FlatZinc::parse(ss, p, olog));
            if (fg) {
                fg->create_branchers(p, fg->solve_annotations(), fznopt, false, olog);
                fg->shrink_arrays(p);
                fg->run(os, p, fznopt, t_total);
            }
            return false;
        } catch (Gecode::FlatZinc::Error& e) {
            const std::string& message = e.to_string();
            if (opt.log)
                olog << ind(2) << "Expected FlatZinc error : " << message << '\n';
            return (!os.str().contains("----------")) &&
                   (_expected_message.empty() || (message.contains(_expected_message)));
        } catch (Gecode::Exception& e) {
            const std::string message = e.what();
            if (opt.log)
                olog << ind(2) << "Expected Gecode exception : " << message << '\n';
            return (!os.str().contains("----------")) &&
                   (_expected_message.empty() || (message.contains(_expected_message)));
        }
        return false;
    }

}} // namespace Test::FlatZinc

// STATISTICS: test-flatzinc
