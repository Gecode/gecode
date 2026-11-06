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

#include "test/test.hh"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

    int passing_runs = 0;
    int failing_runs = 0;

    class PassingSmokeTest : public Test::Base {
    public:
        PassingSmokeTest() : Test::Base("Smoke::A-Pass") {}

        bool
        run() override {
            passing_runs++;
            return true;
        }
    } passing_smoke_test;

    class FailingSmokeTest : public Test::Base {
    public:
        FailingSmokeTest() : Test::Base("Smoke::B-Fail", Test::TestTags::sweep()) {}

        bool
        run() override {
            failing_runs++;
            return false;
        }
    } failing_smoke_test;

    unsigned int observed_seed = 0;
    int replay_runs = 0;
    class ReplaySmokeTest : public Test::Base {
    public:
        ReplaySmokeTest() : Test::Base("Smoke::Replay", Test::TestTags()) {}
        bool
        run() override {
            if (++replay_runs == 1) {
                // Leave a state above the seed setter's modulus for the next iteration.
                _rand.seed(44488);
                (void)_rand.next();
                return true;
            }
            observed_seed = _rand.seed();
            (void)_rand(10);
            return false;
        }
    } replay_smoke_test;

    bool
    require(bool condition, const std::string& message) {
        if (!condition) {
            std::cerr << "public-runner-smoke: " << message << '\n' << std::flush;
            return false;
        }
        return true;
    }

    int
    run_and_capture(std::initializer_list<const char*> args, std::string& output) {
        std::vector<char*> argv;
        argv.reserve(args.size());
        for (const char* arg : args) {
            argv.push_back(const_cast<char*>(arg));
        }

        std::ostringstream captured;
        std::streambuf* old = std::cout.rdbuf(captured.rdbuf());
        const int rc = Test::run_registered_tests(static_cast<int>(argv.size()), argv.data());
        std::cout.rdbuf(old);
        output = captured.str();
        return rc;
    }

} // namespace

int
main() {
    std::string list_output;
    if (!require(run_and_capture({"public-runner-smoke", "-list"}, list_output) == EXIT_SUCCESS,
                 "-list should succeed")) {
        return EXIT_FAILURE;
    }
    const std::string pass_name = "Smoke::A-Pass";
    const std::string fail_name = "Smoke::B-Fail";
    const std::size_t pass_pos = list_output.find(pass_name);
    const std::size_t fail_pos = list_output.find(fail_name);
    if (!require(pass_pos != std::string::npos, "-list output missing pass test")) {
        return EXIT_FAILURE;
    }
    if (!require(fail_pos != std::string::npos, "-list output missing fail test")) {
        return EXIT_FAILURE;
    }
    if (!require(pass_pos < fail_pos, "-list output is not alphabetically ordered")) {
        return EXIT_FAILURE;
    }
    if (!require(passing_runs == 0 && failing_runs == 0, "-list should not execute any registered tests")) {
        return EXIT_FAILURE;
    }

    std::string tagged_list_output;
    if (!require(run_and_capture({"public-runner-smoke", "-list-with-tags"}, tagged_list_output) == EXIT_SUCCESS,
                 "-list-with-tags should succeed")) {
        return EXIT_FAILURE;
    }
    if (!require(tagged_list_output.contains(pass_name + " [standard]"), "default test should have the standard tag")) {
        return EXIT_FAILURE;
    }
    if (!require(tagged_list_output.contains("Smoke::B-Fail [sweep]"), "explicit test tag should be listed")) {
        return EXIT_FAILURE;
    }

    std::string normal_output;
    if (!require(run_and_capture({"public-runner-smoke", "-tag", "standard", "-iter", "1", "-stop", "true"},
                                 normal_output) == EXIT_SUCCESS,
                 "standard tag selection should succeed")) {
        return EXIT_FAILURE;
    }
    if (!require(normal_output.contains(pass_name) && !normal_output.contains(fail_name),
                 "standard tag selection chose the wrong tests")) {
        return EXIT_FAILURE;
    }
    if (!require(passing_runs == 1 && failing_runs == 0, "standard tag selection run counts are wrong")) {
        return EXIT_FAILURE;
    }

    std::string pass_output;
    if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::A-Pass", "-iter", "1", "-stop", "true"},
                                 pass_output) == EXIT_SUCCESS,
                 "filtered passing run should succeed")) {
        return EXIT_FAILURE;
    }
    if (!require(pass_output.contains(pass_name), "filtered passing run did not print the selected test")) {
        return EXIT_FAILURE;
    }
    if (!require(!pass_output.contains(fail_name), "filtered passing run executed an unselected test")) {
        return EXIT_FAILURE;
    }
    if (!require(passing_runs == 2 && failing_runs == 0, "filtered passing run counts are wrong")) {
        return EXIT_FAILURE;
    }

    std::string fail_output;
    if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::B-Fail", "-iter", "1", "-stop", "true"},
                                 fail_output) == EXIT_FAILURE,
                 "filtered failing run should fail")) {
        return EXIT_FAILURE;
    }
    if (!require(fail_output.contains(fail_name), "filtered failing run did not print the selected test")) {
        return EXIT_FAILURE;
    }
    if (!require(fail_output.contains("Options: -replay "), "filtered failing run did not preserve seed diagnostics")) {
        return EXIT_FAILURE;
    }
    if (!require(fail_output.contains("-test Smoke::B-Fail"),
                 "filtered failing run did not preserve test diagnostics")) {
        return EXIT_FAILURE;
    }
    if (!require(passing_runs == 2 && failing_runs == 1, "filtered failing run counts are wrong")) {
        return EXIT_FAILURE;
    }

    std::string replay_output;
    if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::Replay", "-seed", "12345", "-iter", "2"},
                                 replay_output) == EXIT_FAILURE &&
                     replay_runs == 2,
                 "seed-sensitive test should fail on its second iteration"))
        return EXIT_FAILURE;
    const std::string seed = std::to_string(observed_seed);
    if (!require(replay_output.contains("Options: -replay " + seed),
                 "failure should report the iteration's initial random state"))
        return EXIT_FAILURE;
    if (!require(run_and_capture({"public-runner-smoke", "-test", "Smoke::Replay", "-replay", seed.c_str()},
                                 replay_output) == EXIT_FAILURE &&
                     std::to_string(observed_seed) == seed,
                 "replay should restore the reported random state"))
        return EXIT_FAILURE;

    std::string combined_output;
    if (!require(
            run_and_capture({"public-runner-smoke", "-tag", "standard", "-tag", "sweep", "-iter", "1", "-stop", "true"},
                            combined_output) == EXIT_FAILURE,
            "multiple tags should select their union")) {
        return EXIT_FAILURE;
    }
    if (!require(combined_output.contains(pass_name) && combined_output.contains(fail_name),
                 "multiple tag selection did not run both tags")) {
        return EXIT_FAILURE;
    }
    if (!require(passing_runs == 3 && failing_runs == 2, "multiple tag selection run counts are wrong")) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// STATISTICS: test-core
