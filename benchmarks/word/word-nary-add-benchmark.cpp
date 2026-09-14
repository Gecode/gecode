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

#include <gecode/search.hh>
#include <gecode/word.hh>

#include "driver-options.hpp"

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace Gecode;

namespace {

  struct Instance {
    int segments;
    WordValue total;
    WordDomainType domain;
  };

  struct SearchResult {
    unsigned long long solutions=0;
    unsigned long long checksum=0;
    std::vector<WordValue> first;
    Search::Statistics statistics;
  };

  /** Aligned, nondecreasing segment lengths with a fixed modular total. */
  class ScatterGather : public Space {
  public:
    WordVarArray lengths;
    WordVar total;
    explicit ScatterGather(const Instance& instance)
      : lengths(make_lengths(*this,instance.segments,instance.domain)),
        total(make_total(*this,instance.total,instance.domain)) {
      for (int i=0; i<instance.segments; i++) {
        dom(*this,lengths[i],0U,0xff0U);
        if (instance.domain == WDT_CUBE) {
          rel(*this,lengths[i],WRT_UGQ,12,64U);
          rel(*this,lengths[i],WRT_ULQ,12,256U);
        }
      }
      for (int i=1; i<instance.segments; i++)
        rel(*this,lengths[i-1],WRT_ULQ,lengths[i]);
      add(*this,WordVarArgs(lengths),total);
      branch(*this,lengths,WORD_VAR_NONE(),instance.domain == WDT_CUBE ?
             WORD_VAL_LSB() : WORD_VAL_SPLIT_MIN());
    }
    ScatterGather(ScatterGather& s) : Space(s) {
      lengths.update(*this,s.lengths);
      total.update(*this,s.total);
    }
    Space* copy(void) override { return new ScatterGather(*this); }
  private:
    static WordVarArray
    make_lengths(Space& home, int n, WordDomainType domain) {
      return domain == WDT_CUBE ? WordVarArray(home,n,12,0U,0xfffU) :
        WordVarArray(home,n,12,domain,64U,256U);
    }
    static WordVar
    make_total(Space& home, WordValue value, WordDomainType domain) {
      return domain == WDT_CUBE ? WordVar(home,12,value,value) :
        WordVar(home,12,domain,value,value);
    }
  };

  Instance
  parse_options(int argc, char* argv[]) {
    unsigned long long segments=4, total=0;
    bool has_total=false;
    WordDomainType domain=WDT_UNSIGNED;
    std::set<std::string> seen;
    for (int i=1; i<argc; i++) {
      const std::string option=argv[i];
      if (!seen.insert(option).second)
        throw std::invalid_argument("duplicate option: "+option);
      const char* value=WordDriver::consume_value(i,argc,argv);
      if (option == "--segments") {
        segments=WordDriver::parse_unsigned(option.c_str(),value);
      } else if (option == "--variant") {
        const std::string variant=value;
        if (variant == "compact") domain=WDT_CUBE;
        else if (variant == "bounded") domain=WDT_UNSIGNED;
        else throw std::invalid_argument("--variant must be compact or bounded");
      } else if (option == "--total") {
        total=WordDriver::parse_unsigned(option.c_str(),value);
        has_total=true;
      } else {
        throw std::invalid_argument("unknown option: "+option);
      }
    }
    const bool is_supported_count=(segments == 4) || (segments == 6) ||
                                  (segments == 8);
    if (!is_supported_count)
      throw std::invalid_argument("--segments must be 4, 6, or 8");
    if (total > 0xfffU)
      throw std::invalid_argument("--total must fit a 12-bit Word");
    return {static_cast<int>(segments),has_total ? total : 160*segments,domain};
  }

  /** Collect the count, wrapping checksum, and first DFS witness. */
  SearchResult
  run_search(const Instance& instance) {
    std::unique_ptr<ScatterGather> root(new ScatterGather(instance));
    DFS<ScatterGather> search(root.get());
    root.reset();
    SearchResult result;
    while (std::unique_ptr<ScatterGather> solution{search.next()}) {
      result.solutions++;
      for (int i=0; i<instance.segments; i++) {
        const WordValue value=solution->lengths[i].val();
        result.checksum += static_cast<unsigned long long>(i+1)*value;
        if (result.solutions == 1) result.first.push_back(value);
      }
    }
    result.statistics=search.statistics();
    return result;
  }

  void
  print_result(const Instance& instance, const SearchResult& result) {
    std::cout << "{\"schema_version\":1,\"status\":\"ok\",\"segments\":"
              << instance.segments << ",\"total\":" << instance.total
              << ",\"variant\":\""
              << (instance.domain == WDT_CUBE ? "compact" : "bounded")
              << "\",\"solutions\":" << result.solutions
              << ",\"checksum\":" << result.checksum
              << ",\"semantic_status\":\""
              << (result.solutions ? "sat" : "unsat") << "\""
              << ",\"decision_variables\":[";
    for (int i=0; i<instance.segments; i++) {
      if (i != 0) std::cout << ',';
      std::cout << "\"length[" << i << "]\"";
    }
    std::cout << "],\"projections\":[";
    if (!result.first.empty()) {
      std::cout << '[';
      for (std::size_t i=0; i<result.first.size(); i++) {
        if (i != 0) std::cout << ',';
        std::cout << result.first[i];
      }
      std::cout << ']';
    }
    std::cout << "]"
              << ",\"nodes\":" << result.statistics.node
              << ",\"failures\":" << result.statistics.fail
              << ",\"propagations\":" << result.statistics.propagate << "}\n";
  }
}

int
main(int argc, char* argv[]) {
  try {
    const Instance instance=parse_options(argc,argv);
    const SearchResult result=run_search(instance);
    print_result(instance,result);
    return 0;
  } catch (const std::invalid_argument& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
