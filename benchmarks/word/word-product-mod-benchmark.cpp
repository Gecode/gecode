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
#include "driver-output.hpp"

#include <climits>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace Gecode;

namespace {

  struct Instance {
    unsigned int width;
    WordDomainType domain;
    WordValue x_min, x_max, y_min, y_max, result_min, result_max;
    int modulus;
  };

  struct Options {
    Instance instance;
    std::string id;
    bool uses_direct_posting;
  };

  struct SearchResult {
    unsigned long long solutions=0;
    unsigned long long checksum=0;
    std::vector<std::tuple<WordValue,WordValue,WordValue> > projections;
    Search::Statistics statistics;
  };

  /** Compare direct product-modulo posting with an equal cube bridge.
   * Signed input domains still multiply their unsigned bit encodings.
   */
  class Model : public Space {
  public:
    WordVar x, y, result;
    IntVar modulus;
    Model(const Instance& p, bool uses_direct_posting)
      : x(*this,p.width,p.domain,p.x_min,p.x_max),
        y(*this,p.width,p.domain,p.y_min,p.y_max),
        result(*this,p.width,WDT_UNSIGNED,p.result_min,p.result_max),
        modulus(*this,p.modulus,p.modulus) {
      if (uses_direct_posting) {
        product_mod(*this,x,y,modulus,result);
      } else {
        WordVar cube_x(*this,p.width), cube_y(*this,p.width), cube_result(*this,p.width);
        rel(*this,cube_x,WRT_EQ,x); rel(*this,cube_y,WRT_EQ,y);
        rel(*this,cube_result,WRT_EQ,result);
        product_mod(*this,cube_x,cube_y,modulus,cube_result);
      }
      branch(*this,result,WORD_VAL_SPLIT_MIN());
      WordVarArgs operands={x,y};
      branch(*this,operands,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    }
    Model(Model& s) : Space(s) {
      x.update(*this,s.x); y.update(*this,s.y);
      result.update(*this,s.result); modulus.update(*this,s.modulus);
    }
    Space* copy(void) override { return new Model(*this); }
  };


  WordValue
  parse_endpoint(const char* option, const std::string& value,
                 unsigned int width, WordDomainType domain) {
    const WordValue mask=Word::width_mask(width);
    if (domain == WDT_UNSIGNED) {
      const unsigned long long parsed=
        WordDriver::parse_unsigned(option,value.c_str());
      if (parsed > mask)
        throw std::invalid_argument(std::string(option)+" exceeds Word width");
      return static_cast<WordValue>(parsed);
    }
    const long long parsed=WordDriver::parse_signed(option,value.c_str());
    const long long minimum=(width == 64) ? LLONG_MIN : -(1LL << (width-1));
    const long long maximum=(width == 64) ? LLONG_MAX : (1LL << (width-1))-1;
    const bool fits_width=(parsed >= minimum) && (parsed <= maximum);
    if (!fits_width)
      throw std::invalid_argument(std::string(option)+" exceeds signed Word width");
    return static_cast<WordValue>(parsed) & mask;
  }

  Options
  parse_options(int argc, char* argv[]) {
    bool uses_direct_posting=true;
    std::string id="reduce-small", domain="unsigned";
    unsigned long long width=9;
    std::string x_min="10", x_max="30", y_min="10", y_max="30";
    unsigned long long result_min=0, result_max=16, modulus=17;
    std::set<std::string> seen;
    for (int i=1; i<argc; i++) {
      const std::string option=argv[i];
      if (!seen.insert(option).second)
        throw std::invalid_argument("duplicate option: "+option);
      const char* value=WordDriver::consume_value(i,argc,argv);
      if (option == "--variant") {
        const std::string variant=value;
        if (variant == "bounded") uses_direct_posting=true;
        else if (variant == "compact") uses_direct_posting=false;
        else throw std::invalid_argument("--variant must be compact or bounded");
      } else if (option == "--case-id") id=value;
      else if (option == "--width")
        width=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--domain") domain=value;
      else if (option == "--x-min") x_min=value;
      else if (option == "--x-max") x_max=value;
      else if (option == "--y-min") y_min=value;
      else if (option == "--y-max") y_max=value;
      else if (option == "--result-min")
        result_min=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--result-max")
        result_max=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--modulus")
        modulus=WordDriver::parse_unsigned(option.c_str(),value);
      else throw std::invalid_argument("unknown option: "+option);
    }
    const bool has_valid_width=(width >= 1) && (width <= 64);
    if (!has_valid_width)
      throw std::invalid_argument("--width must be between 1 and 64");
    const bool has_valid_domain=(domain == "unsigned") || (domain == "signed");
    if (!has_valid_domain)
      throw std::invalid_argument("--domain must be unsigned or signed");
    const unsigned int bits=static_cast<unsigned int>(width);
    const bool has_valid_result=(result_min <= result_max) &&
      (result_max <= Word::width_mask(bits));
    const bool has_valid_modulus=(modulus > 0) &&
      (modulus <= static_cast<unsigned long long>(Int::Limits::max));
    if (!has_valid_result || !has_valid_modulus)
      throw std::invalid_argument("invalid result interval or modulus");
    const WordDomainType domain_type=
      domain == "signed" ? WDT_SIGNED : WDT_UNSIGNED;
    const Instance instance={bits,domain_type,
      parse_endpoint("--x-min",x_min,bits,domain_type),
      parse_endpoint("--x-max",x_max,bits,domain_type),
      parse_endpoint("--y-min",y_min,bits,domain_type),
      parse_endpoint("--y-max",y_max,bits,domain_type),
      static_cast<WordValue>(result_min),static_cast<WordValue>(result_max),
      static_cast<int>(modulus)};
    const WordValue sign=domain_type == WDT_SIGNED ? WordValue(1) << (bits-1) : 0;
    const bool has_ordered_inputs=
      ((instance.x_min ^ sign) <= (instance.x_max ^ sign)) &&
      ((instance.y_min ^ sign) <= (instance.y_max ^ sign));
    if (!has_ordered_inputs)
      throw std::invalid_argument("input minimum exceeds maximum");
    return {instance,id,uses_direct_posting};
  }

  /** Enumerate all public projections and retain the comparison checksum. */
  SearchResult
  run_search(const Instance& instance, bool uses_direct_posting) {
    std::unique_ptr<Model> root(new Model(instance,uses_direct_posting));
    DFS<Model> search(root.get());
    root.reset();
    SearchResult result;
    const unsigned long long x_weight=1315423911ULL;
    const unsigned long long y_weight=2654435761ULL;
    while (std::unique_ptr<Model> solution{search.next()}) {
      const WordValue x=solution->x.val(), y=solution->y.val();
      const WordValue value=solution->result.val();
      result.solutions++;
      result.projections.emplace_back(x,y,value);
      result.checksum += (x*x_weight) ^ (y*y_weight) ^ value;
    }
    result.statistics=search.statistics();
    return result;
  }

  void
  print_result(const Options& options, const SearchResult& result) {
    std::cout << "{\"schema_version\":1,\"status\":\"ok\",\"case_id\":";
    WordDriver::write_string(std::cout,options.id);
    std::cout << ",\"variant\":\""
              << (options.uses_direct_posting ? "bounded" : "compact") << "\""
              << ",\"solutions\":" << result.solutions
              << ",\"checksum\":" << result.checksum
              << ",\"semantic_status\":\""
              << (result.solutions ? "sat" : "unsat") << "\""
              << ",\"decision_variables\":[\"x\",\"y\",\"result\"]"
              << ",\"projections\":[";
    for (std::size_t i=0; i<result.projections.size(); i++) {
      if (i != 0) std::cout << ',';
      std::cout << '[' << std::get<0>(result.projections[i]) << ','
                << std::get<1>(result.projections[i]) << ','
                << std::get<2>(result.projections[i]) << ']';
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
    const Options options=parse_options(argc,argv);
    const SearchResult result=
      run_search(options.instance,options.uses_direct_posting);
    print_result(options,result);
    return 0;
  } catch (const std::invalid_argument& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
