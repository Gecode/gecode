/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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
 */

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/word.hh>

#include "../benchmarks/word/driver-output.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace Gecode;

namespace {

  struct Instance {
    std::string id;
    unsigned int width;
    unsigned int rounds;
    WordValue key;
    unsigned int shift;
    WordValue target_lo;
    WordValue target_hi;
    unsigned long long int expected_solutions;
    WordValue expected_input;
    bool has_excluded_input;
    WordValue excluded_input;
  };

  /// XOR/rotate fixture using compact Word domains.
  class NativeWordModel : public Space {
  public:
    WordVarArray state;
    WordVarArray mixed;

    explicit NativeWordModel(const Instance& i)
      : state(*this,static_cast<int>(i.rounds+1),i.width,0,
              Word::width_mask(i.width)),
        mixed(*this,static_cast<int>(i.rounds),i.width,0,
              Word::width_mask(i.width)) {
      for (unsigned int r=0; r<i.rounds; r++) {
        rel(*this,state[r],WOT_XOR,i.width,i.key,mixed[r]);
        rotate_left(*this,mixed[r],i.shift,state[r+1]);
      }
      dom(*this,state[i.rounds],i.target_lo,i.target_hi);
      if (i.has_excluded_input)
        rel(*this,state[0],WRT_NQ,i.width,i.excluded_input);
      branch(*this,state[0],WORD_VAL_LSB());
    }

    NativeWordModel(NativeWordModel& s) : Space(s) {
      state.update(*this,s.state);
      mixed.update(*this,s.mixed);
    }

    Space* copy(void) override {
      return new NativeWordModel(*this);
    }
  };

  /// Bit decomposition with the same input projection and decision order.
  class BooleanModel : public Space {
  private:
    const unsigned int width;
    const unsigned int rounds;

    unsigned int state_index(unsigned int round, unsigned int bit) const {
      return round*width+bit;
    }

    unsigned int mixed_index(unsigned int round, unsigned int bit) const {
      return (rounds+1)*width+round*width+bit;
    }

  public:
    BoolVarArray bit;

    explicit BooleanModel(const Instance& i)
      : width(i.width), rounds(i.rounds),
        bit(*this,static_cast<int>((2*i.rounds+1)*i.width),0,1) {
      for (unsigned int r=0; r<rounds; r++)
        for (unsigned int b=0; b<width; b++) {
          BoolVar x = bit[state_index(r,b)];
          BoolVar y = bit[mixed_index(r,b)];
          rel(*this,x,((i.key >> b) & 1U) ? IRT_NQ : IRT_EQ,y);
          rel(*this,y,IRT_EQ,
              bit[state_index(r+1,(b+i.shift) % width)]);
        }
      for (unsigned int b=0; b<width; b++)
        if (((i.target_lo >> b) & 1U) == ((i.target_hi >> b) & 1U))
          rel(*this,bit[state_index(rounds,b)],IRT_EQ,
              static_cast<int>((i.target_lo >> b) & 1U));
      if (i.has_excluded_input) {
        BoolVarArgs differs(static_cast<int>(width));
        for (unsigned int b=0; b<width; b++) {
          differs[b] = BoolVar(*this,0,1);
          rel(*this,bit[state_index(0,b)],
              ((i.excluded_input >> b) & 1U) ? IRT_NQ : IRT_EQ,
              differs[b]);
        }
        linear(*this,differs,IRT_GQ,1);
      }

      BoolVarArgs decision(static_cast<int>(width));
      for (unsigned int b=0; b<width; b++)
        decision[b] = bit[state_index(0,b)];
      branch(*this,decision,BOOL_VAR_NONE(),BOOL_VAL_MIN());
    }

    BooleanModel(BooleanModel& s)
      : Space(s), width(s.width), rounds(s.rounds) {
      bit.update(*this,s.bit);
    }

    Space* copy(void) override {
      return new BooleanModel(*this);
    }

    WordValue input(void) const {
      WordValue value = 0;
      for (unsigned int b=0; b<width; b++)
        value |= static_cast<WordValue>(bit[state_index(0,b)].val()) << b;
      return value;
    }
  };

  struct SearchResult {
    unsigned long long int solutions;
    std::vector<WordValue> inputs;
    unsigned long long int propagation_calls;
    unsigned long long int nodes;
    unsigned long long int failures;
    unsigned int root_propagators;
    unsigned int root_branchers;
  };

  WordValue input_value(const NativeWordModel& model) {
    return model.state[0].val();
  }

  WordValue input_value(const BooleanModel& model) {
    return model.input();
  }

  template<class Model>
  SearchResult run(const Instance& instance) {
    std::unique_ptr<Model> root(new Model(instance));
    StatusStatistics root_statistics;
    if (root->status(root_statistics) == SS_FAILED) {
      root.reset();
      return SearchResult {0, {}, root_statistics.propagate, 0, 0, 0, 0};
    }
    const unsigned int root_propagators =
      PropagatorGroup::all.size(*root);
    const unsigned int root_branchers = BrancherGroup::all.size(*root);
    Search::Options options;
    options.c_d = 64;
    options.a_d = 64;
    DFS<Model> dfs(root.get(),options);
    root.reset();

    unsigned long long int solutions = 0;
    std::vector<WordValue> inputs;
    while (std::unique_ptr<Model> solution{dfs.next()}) {
      const WordValue input = input_value(*solution);
      inputs.push_back(input);
      solutions++;
    }
    const Search::Statistics statistics = dfs.statistics();
    SearchResult metrics = {
      solutions, inputs,
      root_statistics.propagate+statistics.propagate,
      statistics.node, statistics.fail, root_propagators, root_branchers
    };
    return metrics;
  }

  unsigned long long int parse_unsigned_or_exit(const char* option, const char* value,
      unsigned long long maximum=std::numeric_limits<unsigned long long>::max()) {
    errno = 0;
    char* end = nullptr;
    const unsigned long long int parsed = std::strtoull(value,&end,0);
    const bool has_prefix = (value[0] >= '0' && value[0] <= '9') ||
      (value[0] == '+');
    if (!has_prefix || errno != 0 || end == value || *end != '\0' ||
        parsed > maximum) {
      std::cerr << "invalid value for " << option << ": " << value << "\n";
      std::exit(EXIT_FAILURE);
    }
    return parsed;
  }

  const char* consume_value_or_exit(int& i, int argc, char* argv[]) {
    if (++i >= argc) {
      std::cerr << "missing value for " << argv[i-1] << "\n";
      std::exit(EXIT_FAILURE);
    }
    return argv[i];
  }

}

/**
 * \brief Differential native-word and Boolean decomposition benchmark fixture
 *
 * This intentionally small driver is orchestrated by
 * benchmarks/word/benchmark.py. Its allocation and clone-footprint metrics are
 * structural model counters, not kernel allocation or byte measurements.
 *
 * \ingroup Example
 */
namespace {
  /// Validated inputs for the equivalent Word and Boolean fixtures.
  struct Request {
    std::string variant = "native-word";
    Instance instance = {
      "word-xor-rotate-smoke",8,3,165,3,128,143,16,66,false,0
    };
  };

  bool parse_request(int argc, char* argv[], Request& request) {
    std::string& variant = request.variant;
    Instance& instance = request.instance;
    for (int i=1; i<argc; i++) {
      const char* option = argv[i];
      if (std::strcmp(option,"--variant") == 0)
        variant = consume_value_or_exit(i,argc,argv);
      else if (std::strcmp(option,"--instance-id") == 0)
        instance.id = consume_value_or_exit(i,argc,argv);
      else if (std::strcmp(option,"--width") == 0)
        instance.width = static_cast<unsigned int>(
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv),
                                 std::numeric_limits<unsigned int>::max()));
      else if (std::strcmp(option,"--rounds") == 0)
        instance.rounds = static_cast<unsigned int>(
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv),
                                 std::numeric_limits<unsigned int>::max()));
      else if (std::strcmp(option,"--key") == 0)
        instance.key = parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      else if (std::strcmp(option,"--shift") == 0)
        instance.shift = static_cast<unsigned int>(
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv),
                                 std::numeric_limits<unsigned int>::max()));
      else if (std::strcmp(option,"--target-lo") == 0)
        instance.target_lo = parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      else if (std::strcmp(option,"--target-hi") == 0)
        instance.target_hi = parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      else if (std::strcmp(option,"--expected-solutions") == 0)
        instance.expected_solutions =
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      else if (std::strcmp(option,"--expected-input") == 0)
        instance.expected_input =
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      else if (std::strcmp(option,"--excluded-input") == 0) {
        instance.has_excluded_input = true;
        instance.excluded_input =
          parse_unsigned_or_exit(option,consume_value_or_exit(i,argc,argv));
      }
      else {
        std::cerr << "unknown option: " << option << "\n";
        return false;
      }
    }

    if ((variant != "native-word") && (variant != "bool-decomposition")) {
      std::cerr << "--variant must be native-word or bool-decomposition\n";
      return false;
    }
    if (instance.id.empty() || (instance.width == 0) ||
        (instance.width > 8*sizeof(WordValue)) || (instance.rounds == 0) ||
        (instance.shift >= instance.width) ||
        ((instance.target_lo & ~instance.target_hi) != 0) ||
        (((instance.key | instance.target_hi | instance.expected_input |
           instance.excluded_input) &
          ~Word::width_mask(instance.width)) != 0)) {
      std::cerr << "invalid benchmark instance\n";
      return false;
    }

    // Validate each formulation's signed array size before narrowing.
    const unsigned long long cells = variant == "native-word" ?
      instance.rounds+1ULL : (2ULL*instance.rounds+1ULL)*instance.width;
    if (cells > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
      std::cerr << "benchmark dimensions exceed array limits\n";
      return false;
    }
    return true;
  }

  void write_result(const Request& request, const SearchResult& metrics) {
    const Instance& instance = request.instance;
    const std::string& variant = request.variant;
    const bool ok = (metrics.solutions == instance.expected_solutions);
    std::cout << "{\"schema_version\":1,\"status\":\""
              << (ok ? "ok" : "mismatch") << "\",\"solver_variant\":\""
              << variant << "\",\"instance_id\":";
    WordDriver::write_string(std::cout,instance.id);
    std::cout << ",\"solutions\":" << metrics.solutions
              << ",\"expected_solutions\":" << instance.expected_solutions
              << ",\"semantic_status\":\""
              << (metrics.solutions == 0 ? "unsat" : "sat") << "\""
              << ",\"decision_variables\":[\"input\"]"
              << ",\"projections\":";
    WordDriver::write_scalar_rows(std::cout,metrics.inputs);
    std::cout << ",\"propagation_calls\":" << metrics.propagation_calls
              << ",\"root_propagators\":" << metrics.root_propagators
              << ",\"root_branchers\":" << metrics.root_branchers
              << ",\"nodes\":" << metrics.nodes
              << ",\"failures\":" << metrics.failures << "}\n";

  }
}

int main(int argc, char* argv[]) {
  Request request;
  if (!parse_request(argc,argv,request)) return EXIT_FAILURE;
  const SearchResult result = request.variant == "native-word" ?
    run<NativeWordModel>(request.instance) : run<BooleanModel>(request.instance);
  write_result(request,result);
  return result.solutions == request.instance.expected_solutions ?
    EXIT_SUCCESS : EXIT_FAILURE;
}

// STATISTICS: example-any
