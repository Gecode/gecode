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

#include <cstdint>
#include <iostream>
#include <vector>

using namespace Gecode;

namespace {

  class RegisterOptions : public Options {
  private:
    Driver::StringOption _formulation;
    Driver::UnsignedIntOption _size;
    Driver::UnsignedIntOption _allowed_mask;
    Driver::StringOption _projection;
    Driver::StringOption _search_control;
    Driver::UnsignedIntOption _batch;
  public:
    enum Formulation { COMPACT_WORD, BOUNDED_WORD };
    enum Projection { PROJECTION_NONE, PROJECTION_ALL };
    enum SearchControl { SEARCH_LSB, SEARCH_SPLIT_MIN };

    RegisterOptions(const char* name)
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
 * Four numeric register windows feed one selected value. The outer windows
 * are disjoint from the selected range but have overlapping cube hulls, so
 * bounded Element can reject them before branching. The compact formulation
 * posts the same numeric ranges through ordinary Word relations.
 *
 * \ingroup Example
 */
class WordRegisterFile : public Script {
private:
  RegisterOptions::Formulation formulation;
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
  /// Actual model
  WordRegisterFile(const RegisterOptions& opt)
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

    branch(*this,index,INT_VAL_MIN());
    if (opt.search_control() == RegisterOptions::SEARCH_SPLIT_MIN)
      branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    else
      branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_LSB());
    WordVarArgs derived={selected,total};
    if (opt.search_control() == RegisterOptions::SEARCH_SPLIT_MIN)
      branch(*this,derived,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
    else
      branch(*this,derived,WORD_VAR_NONE(),WORD_VAL_LSB());
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
  virtual Space* copy(void) {
    return new WordRegisterFile(*this);
  }
  /// Whether only the two numerically supported windows remain selectable
  bool index_pruned(void) const {
    return !index.in(0) && index.in(1) && index.in(2) && !index.in(3);
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
};

/** \brief Main-function
 *  \relates WordRegisterFile
 */
int
main(int argc, char* argv[]) {
  RegisterOptions opt("WordRegisterFile");
  opt.parse(argc,argv);
  if ((opt.size() == 0U) || (opt.size() > 8U)) {
    std::cerr << "register count must be between 1 and 8\n";
    return 2;
  }
  if (opt.batch() == 0U) return 2;
  bool index_pruned=false; unsigned int root_propagators=0, root_branchers=0;
  std::uint64_t solutions=0, checksum=0;
  std::uint64_t first_solutions=0, nodes=0, failures=0, propagations=0;
  std::vector<std::vector<unsigned int> > projections;
  for (unsigned int trial=0; trial<opt.batch(); trial++) {
    WordRegisterFile* root=new WordRegisterFile(opt); StatusStatistics root_statistics;
    const SpaceStatus root_status=root->status(root_statistics);
    if (trial == 0U) { index_pruned=root->index_pruned(); root_propagators=PropagatorGroup::all.size(*root); root_branchers=BrancherGroup::all.size(*root); }
    DFS<WordRegisterFile> search(root_status == SS_FAILED ? nullptr : root); delete root;
    while (WordRegisterFile* solution=search.next()) {
      ++solutions; if (trial == 0U) ++first_solutions;
      checksum += solution->solution_value();
      if (trial == 0U && opt.projection() == RegisterOptions::PROJECTION_ALL) projections.push_back(solution->public_projection());
      delete solution;
    }
    const Search::Statistics statistics=search.statistics();
    nodes+=statistics.node; failures+=statistics.fail;
    propagations+=root_statistics.propagate+statistics.propagate;
  }
  std::cout << "{\"schema_version\":1,\"status\":\"ok\""
            << ",\"formulation\":\"" << opt.formulation_name() << "\""
            << ",\"solutions\":" << first_solutions
            << ",\"batch\":" << opt.batch() << ",\"batch_solutions\":" << solutions
            << ",\"checksum\":" << checksum
            << ",\"semantic_status\":\"" << (solutions ? "sat" : "unsat") << "\""
            << ",\"decision_variables\":[\"index\"";
  for (unsigned int i=0; i<opt.size(); i++)
    std::cout << ",\"register[" << i << "]\"";
  std::cout << "],\"projections\":[";
  for (std::size_t i=0; i<projections.size(); i++) {
    if (i) std::cout << ',';
    std::cout << '[';
    for (std::size_t j=0; j<projections[i].size(); j++) {
      if (j) std::cout << ',';
      std::cout << projections[i][j];
    }
    std::cout << ']';
  }
  std::cout << ']'
            << ",\"index_pruned\":" << (index_pruned ? "true" : "false")
            << ",\"nodes\":" << nodes
            << ",\"failures\":" << failures
            << ",\"propagations\":" << propagations
            << ",\"root_propagators\":" << root_propagators
            << ",\"root_branchers\":" << root_branchers << "}\n";
  return 0;
}

// STATISTICS: example-any
