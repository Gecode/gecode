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

#include <climits>
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace Gecode;

namespace {

  struct Instance {
    unsigned int width;
    WordValue input_min, input_max, output_min, output_max;
  };

  struct Options {
    Instance instance;
    unsigned int batch;
    bool uses_lsb;
  };

  struct ComparisonResult {
    std::vector<std::pair<WordValue,WordValue> > projections;
    std::uint64_t batch_solutions=0;
  };

  /** A variable-shift ALU with carry- and sign-controlled output selection. */
  class Model : public Space {
  public:
    WordVar input, output;
    Model(const Instance& instance, bool uses_lsb)
      : input(*this,instance.width,WDT_UNSIGNED,
              instance.input_min,instance.input_max),
        output(*this,instance.width,WDT_UNSIGNED,
               instance.output_min,instance.output_max) {
      const unsigned int width=instance.width;
      const WordValue mask=Word::width_mask(width);
      WordVar amount(*this,width,WDT_UNSIGNED,0U,3U);
      WordVar shifted_input(*this,width,WDT_UNSIGNED), addend(*this,width,
        WDT_UNSIGNED,WordValue(0x1d)&mask,WordValue(0x1d)&mask);
      WordVar sum(*this,width,WDT_UNSIGNED), shifted(*this,width,WDT_UNSIGNED),
        xored(*this,width,WDT_UNSIGNED), selected(*this,width,WDT_UNSIGNED),
        incremented(*this,width,WDT_UNSIGNED);
      BoolVar has_carry(*this,0,1), is_negative(*this,0,1);
      rel(*this,input,WOT_AND,width,3U,amount);
      shift_left(*this,input,amount,shifted_input);
      add(*this,shifted_input,addend,sum,has_carry);
      channel(*this,sum,width-1,is_negative);
      arithmetic_shift_right(*this,sum,amount,shifted);
      rel(*this,sum,WOT_XOR,width,WordValue(0x15)&mask,xored);
      ite(*this,has_carry,xored,shifted,selected);
      add(*this,selected,width,1U,incremented);
      ite(*this,is_negative,incremented,selected,output);
      if (uses_lsb) {
        branch(*this,input,WORD_VAL_LSB()); branch(*this,output,WORD_VAL_LSB());
      } else {
        branch(*this,input,WORD_VAL_SPLIT_MIN()); branch(*this,output,WORD_VAL_SPLIT_MIN());
      }
    }
    Model(Model& s) : Space(s) {
      input.update(*this,s.input); output.update(*this,s.output);
    }
    Space* copy(void) override { return new Model(*this); }
  };


  Options
  parse_options(int argc, char* argv[]) {
    unsigned long long width=0, batch=1;
    WordValue input_min=0, input_max=0, output_min=0, output_max=0;
    bool uses_lsb=false;
    std::set<std::string> seen;
    for (int i=1; i<argc; i++) {
      const std::string option=argv[i];
      if (!seen.insert(option).second)
        throw std::invalid_argument("duplicate option: "+option);
      const char* value=WordDriver::consume_value(i,argc,argv);
      if (option == "--search") {
        const std::string search=value;
        if (search == "lsb") uses_lsb=true;
        else if (search == "split-min") uses_lsb=false;
        else throw std::invalid_argument("--search must be lsb or split-min");
      } else if (option == "--comparison-width")
        width=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--batch")
        batch=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--input-min")
        input_min=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--input-max")
        input_max=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--output-min")
        output_min=WordDriver::parse_unsigned(option.c_str(),value);
      else if (option == "--output-max")
        output_max=WordDriver::parse_unsigned(option.c_str(),value);
      else throw std::invalid_argument("unknown option: "+option);
    }
    for (const char* option : {"--comparison-width", "--input-min",
                              "--input-max", "--output-min", "--output-max"}) {
      if (seen.count(option) == 0)
        throw std::invalid_argument(std::string("missing option: ")+option);
    }
    const bool has_valid_width=(width >= 2) && (width <= 64);
    if (!has_valid_width)
      throw std::invalid_argument("--comparison-width must be between 2 and 64");
    const bool has_valid_batch=(batch > 0) && (batch <= UINT_MAX);
    if (!has_valid_batch)
      throw std::invalid_argument("--batch must be between 1 and UINT_MAX");
    const unsigned int bits=static_cast<unsigned int>(width);
    const WordValue mask=Word::width_mask(bits);
    const bool has_valid_intervals=(input_min <= input_max) &&
      (output_min <= output_max) && (input_max <= mask) && (output_max <= mask);
    if (!has_valid_intervals)
      throw std::invalid_argument("intervals must be ordered and fit the Word width");
    return {{bits,input_min,input_max,output_min,output_max},
            static_cast<unsigned int>(batch),uses_lsb};
  }

  /** Retain first-trial projections and count solutions from every trial. */
  ComparisonResult
  run_comparison(const Options& options) {
    ComparisonResult result;
    for (unsigned int trial=0; trial<options.batch; trial++) {
      std::unique_ptr<Model> root(new Model(options.instance,options.uses_lsb));
      DFS<Model> search(root.get());
      root.reset();
      while (std::unique_ptr<Model> solution{search.next()}) {
        result.batch_solutions++;
        if (trial == 0)
          result.projections.emplace_back(solution->input.val(),
                                          solution->output.val());
      }
    }
    return result;
  }

  void
  print_result(unsigned int batch, const ComparisonResult& result) {
    std::cout << "{\"schema_version\":1,\"semantic_status\":\""
              << (result.projections.empty() ? "unsat" : "sat")
              << "\",\"solutions\":" << result.projections.size()
              << ",\"batch\":" << batch
              << ",\"batch_solutions\":" << result.batch_solutions
              << ",\"decision_variables\":[\"input\",\"output\"],\"projections\":[";
    for (std::size_t i=0; i<result.projections.size(); i++) {
      if (i != 0) std::cout << ',';
      std::cout << '[' << result.projections[i].first << ','
                << result.projections[i].second << ']';
    }
    std::cout << "]}\n";
  }
}

int
main(int argc, char* argv[]) {
  try {
    const Options options=parse_options(argc,argv);
    const ComparisonResult result=run_comparison(options);
    print_result(options.batch,result);
    return 0;
  } catch (const std::invalid_argument& error) {
    std::cerr << error.what() << '\n';
    return 2;
  }
}
