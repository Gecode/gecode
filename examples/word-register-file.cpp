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

#include <gecode/driver.hh>
#include <gecode/word.hh>

#include "../benchmarks/word/driver-output.hpp"

#include <cstdint>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

using namespace Gecode;

namespace {
  struct InvalidNumber {};

  template<class Unsigned>
  Unsigned parse_unsigned(const char* text, int base) {
    errno = 0;
    char* end = nullptr;
    const unsigned long long value = std::strtoull(text,&end,base);
    const bool has_prefix = (text[0] >= '0' && text[0] <= '9') ||
      (text[0] == '+');
    if (!has_prefix || errno != 0 || end == text || *end != '\0' ||
        value > std::numeric_limits<Unsigned>::max())
      throw InvalidNumber();
    return static_cast<Unsigned>(value);
  }


  /// Decimal unsigned options with complete-token and range validation.
  class UnsignedOption : public Driver::UnsignedIntOption {
  public:
    UnsignedOption(const char* name, const char* help, unsigned int value)
      : Driver::UnsignedIntOption(name,help,value) {}
    int parse(int argc, char* argv[]) override {
      if (const char* text = argument(argc,argv)) {
        cur = parse_unsigned<unsigned int>(text,10);
        return 2;
      }
      return 0;
    }
  };

  class RegisterOptions : public Options {
  private:
    Driver::StringOption _formulation;
    UnsignedOption _size;
    UnsignedOption _allowed_mask;
    Driver::StringOption _projection;
    Driver::StringOption _search_control;
    UnsignedOption _batch;
  public:
    enum Formulation { COMPACT_WORD, BOUNDED_WORD };
    enum Projection { PROJECTION_NONE, PROJECTION_ALL };
    enum SearchControl { SEARCH_LSB, SEARCH_SPLIT_MIN };

    explicit RegisterOptions(const char* name)
      : Options(name),
        _formulation("formulation","model formulation",COMPACT_WORD),
        _size("size","register count",4),
        _allowed_mask("allowed-mask","bit mask of selectable indices",0xffffffffU),
        _projection("projection","none or all public projections",PROJECTION_NONE),
        _search_control("search-control","Word value selector",SEARCH_LSB),
        _batch("batch","complete in-process repetitions",1U) {
      solutions(0);
      add(_formulation);
      _formulation.add(COMPACT_WORD,"compact-word","compact Word variables");
      _formulation.add(BOUNDED_WORD,"bounded-word",
                       "unsigned-bounded Word variables");
      add(_size); add(_allowed_mask); add(_projection); add(_search_control); add(_batch);
      _projection.add(PROJECTION_NONE,"none","do not emit projections");
      _projection.add(PROJECTION_ALL,"all","emit all public projections");
      _search_control.add(SEARCH_LSB,"lsb","least-significant-bit first");
      _search_control.add(SEARCH_SPLIT_MIN,"split-min","lower ranked split");
    }
    Formulation formulation(void) const {
      return static_cast<Formulation>(_formulation.value());
    }
    const char* formulation_name(void) const {
      return formulation() == COMPACT_WORD ? "compact-word" : "bounded-word";
    }
    unsigned int size(void) const { return _size.value(); }
    unsigned int allowed_mask(void) const { return _allowed_mask.value(); }
    Projection projection(void) const {
      return static_cast<Projection>(_projection.value());
    }
    SearchControl search_control(void) const {
      return static_cast<SearchControl>(_search_control.value());
    }
    unsigned int batch(void) const { return _batch.value(); }
  };

}

/**
 * \brief %Example: Select and accumulate a small Word register file
 *
 * Repeating numeric register windows feed one selected value. The outer windows
 * are disjoint from the selected range but have overlapping cube hulls, so
 * bounded Element can reject them before branching. The compact formulation
 * posts the same numeric ranges through ordinary Word relations.
 *
 * \ingroup Example
 */
class WordRegisterFile : public Script {
private:
  const RegisterOptions::Formulation formulation;
  WordVarArray registers;
  IntVar index;
  WordVar selected;
  WordVar increment0;
  WordVar increment1;
  WordVar total;

  static WordVar numeric_word(Space& home,
                              RegisterOptions::Formulation formulation,
                              WordValue minimum, WordValue maximum) {
    return formulation == RegisterOptions::BOUNDED_WORD ?
      WordVar(home,4,WDT_UNSIGNED,minimum,maximum) : WordVar(home,4);
  }

  void range(WordVar x, WordValue minimum, WordValue maximum) {
    if (formulation == RegisterOptions::COMPACT_WORD) {
      rel(*this,x,WRT_UGQ,4,minimum);
      rel(*this,x,WRT_ULQ,4,maximum);
    }
  }
public:
  /// Select a register and constrain its incremented value.
  explicit WordRegisterFile(const RegisterOptions& opt)
    : Script(opt), formulation(opt.formulation()), registers(*this,opt.size()),
      index(*this,0,static_cast<int>(opt.size()-1)),
      selected(numeric_word(*this,formulation,5U,6U)),
      increment0(numeric_word(*this,formulation,1U,1U)),
      increment1(numeric_word(*this,formulation,2U,2U)),
      total(numeric_word(*this,formulation,8U,9U)) {
    const WordValue minimum[] = {3U,4U,6U,7U};
    for (int i=0; i<registers.size(); i++) {
      const WordValue lo=minimum[i % 4], hi=lo+1U;
      registers[i]=numeric_word(*this,formulation,lo,hi);
      range(registers[i],lo,hi);
      if ((opt.allowed_mask() & (1U << i)) == 0U)
        rel(*this,index,IRT_NQ,i);
    }
    range(selected,5U,6U);
    range(increment0,1U,1U);
    range(increment1,2U,2U);
    range(total,8U,9U);
    element(*this,registers,index,selected);
    WordVarArgs addend={selected,increment0,increment1};
    add(*this,addend,total);

    post_branchers(opt.search_control());
  }

  /// Constructor for cloning \a s
  WordRegisterFile(WordRegisterFile& s)
    : Script(s), formulation(s.formulation) {
    registers.update(*this,s.registers);
    index.update(*this,s.index);
    selected.update(*this,s.selected);
    increment0.update(*this,s.increment0);
    increment1.update(*this,s.increment1);
    total.update(*this,s.total);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordRegisterFile(*this);
  }
  /// Whether the stable nonfailed root retains exactly the supported indices.
  bool index_pruned(unsigned int allowed_mask) const {
    for (int i=0; i<registers.size(); i++) {
      const bool supported = (allowed_mask & (1U << i)) != 0U &&
        (i % 4 == 1 || i % 4 == 2);
      if (index.in(i) != supported) return false;
    }
    return true;
  }
  /// Stable contribution for semantic comparison
  std::uint64_t solution_value(void) const {
    return static_cast<std::uint64_t>(index.val()+1) +
      3U*selected.val()+5U*total.val()+
      register_value();
  }
  std::uint64_t register_value(void) const {
    std::uint64_t value=0;
    for (int i=0; i<registers.size(); i++)
      value += static_cast<std::uint64_t>(i+1)*registers[i].val();
    return value;
  }
  std::vector<unsigned int> public_projection(void) const {
    std::vector<unsigned int> values;
    values.push_back(static_cast<unsigned int>(index.val()));
    for (int i=0; i<registers.size(); i++)
      values.push_back(static_cast<unsigned int>(registers[i].val()));
    return values;
  }
private:
  void post_branchers(RegisterOptions::SearchControl search) {
    branch(*this,index,INT_VAL_MIN());
    if (search == RegisterOptions::SEARCH_SPLIT_MIN)
      branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    else
      branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_LSB());
    WordVarArgs derived={selected,total};
    if (search == RegisterOptions::SEARCH_SPLIT_MIN)
      branch(*this,derived,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    else
      branch(*this,derived,WORD_VAR_NONE(),WORD_VAL_LSB());
  }

};

namespace {
  /// Counts, first-root diagnostic, and first-search public projections.
  struct Result {
    bool index_pruned=false;
    unsigned int root_propagators=0, root_branchers=0;
    std::uint64_t solutions=0, checksum=0, first_solutions=0;
    std::uint64_t nodes=0, failures=0, propagations=0;
    std::vector<std::vector<unsigned int> > projections;
  };
  Result enumerate(const RegisterOptions& opt) {
    Result result;
    for (unsigned int trial=0; trial<opt.batch(); trial++) {
      std::unique_ptr<WordRegisterFile> root(new WordRegisterFile(opt));
      StatusStatistics root_statistics;
      const SpaceStatus root_status=root->status(root_statistics);
      if (trial == 0U) {
        result.index_pruned=root_status != SS_FAILED &&
          root->index_pruned(opt.allowed_mask());
        result.root_propagators=PropagatorGroup::all.size(*root);
        result.root_branchers=BrancherGroup::all.size(*root);
      }
      DFS<WordRegisterFile> search(root_status == SS_FAILED ? nullptr : root.get());
      root.reset();
      while (std::unique_ptr<WordRegisterFile> solution{search.next()}) {
        ++result.solutions; if (trial == 0U) ++result.first_solutions;
        result.checksum += solution->solution_value();
        if (trial == 0U && opt.projection() == RegisterOptions::PROJECTION_ALL)
          result.projections.push_back(solution->public_projection());
      }
      const Search::Statistics statistics=search.statistics();
      result.nodes+=statistics.node; result.failures+=statistics.fail;
      result.propagations+=root_statistics.propagate+statistics.propagate;
    }
    return result;
  }

  void write_result(const RegisterOptions& opt, const Result& result) {
    const auto& index_pruned=result.index_pruned;
    const auto& root_propagators=result.root_propagators;
    const auto& root_branchers=result.root_branchers;
    const auto& solutions=result.solutions;
    const auto& checksum=result.checksum;
    const auto& first_solutions=result.first_solutions;
    const auto& nodes=result.nodes;
    const auto& failures=result.failures;
    const auto& propagations=result.propagations;
    const auto& projections=result.projections;
    std::cout << "{\"schema_version\":1,\"status\":\"ok\""
              << ",\"formulation\":\"" << opt.formulation_name() << "\""
              << ",\"solutions\":" << first_solutions
              << ",\"batch\":" << opt.batch() << ",\"batch_solutions\":" << solutions
              << ",\"checksum\":" << checksum
              << ",\"semantic_status\":\"" << (solutions ? "sat" : "unsat") << "\""
              << ",\"decision_variables\":[\"index\"";
    for (unsigned int i=0; i<opt.size(); i++)
      std::cout << ",\"register[" << i << "]\"";
    std::cout << "],\"projections\":";
    WordDriver::write_rows(std::cout,projections);
    std::cout << ",\"index_pruned\":" << (index_pruned ? "true" : "false")
              << ",\"nodes\":" << nodes
              << ",\"failures\":" << failures
              << ",\"propagations\":" << propagations
              << ",\"root_propagators\":" << root_propagators
              << ",\"root_branchers\":" << root_branchers << "}\n";
  }
}

int
main(int argc, char* argv[]) {
  RegisterOptions opt("WordRegisterFile");
  try {
    opt.parse(argc,argv);
  } catch (const InvalidNumber&) {
    std::cerr << "invalid unsigned option value\n";
    return 2;
  }
  if ((opt.size() == 0U) || (opt.size() > 8U)) {
    std::cerr << "register count must be between 1 and 8\n";
    return 2;
  }
  if (opt.batch() == 0U) return 2;
  write_result(opt,enumerate(opt));
  return 0;
}

// STATISTICS: example-any
