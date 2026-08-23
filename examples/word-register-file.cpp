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

using namespace Gecode;

namespace {

  class RegisterOptions : public Options {
  private:
    Driver::StringOption _formulation;
  public:
    enum Formulation { COMPACT_WORD, BOUNDED_WORD };

    RegisterOptions(const char* name)
      : Options(name),
        _formulation("formulation","model formulation",COMPACT_WORD) {
      solutions(0);
      add(_formulation);
      _formulation.add(COMPACT_WORD,"compact-word","compact Word variables");
      _formulation.add(BOUNDED_WORD,"bounded-word",
                       "unsigned-bounded Word variables");
    }
    Formulation formulation(void) const {
      return static_cast<Formulation>(_formulation.value());
    }
    const char* formulation_name(void) const {
      return formulation() == COMPACT_WORD ? "compact-word" : "bounded-word";
    }
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
    : Script(opt), formulation(opt.formulation()), registers(*this,4),
      index(*this,0,3),
      selected(numeric_word(*this,formulation,5U,6U)),
      increment0(numeric_word(*this,formulation,1U,1U)),
      increment1(numeric_word(*this,formulation,2U,2U)),
      total(numeric_word(*this,formulation,8U,9U)) {
    const WordValue minimum[] = {3U,4U,6U,7U};
    const WordValue maximum[] = {4U,5U,7U,8U};
    for (int i=0; i<registers.size(); i++) {
      registers[i]=numeric_word(*this,formulation,minimum[i],maximum[i]);
      range(registers[i],minimum[i],maximum[i]);
    }
    range(selected,5U,6U);
    range(increment0,1U,1U);
    range(increment1,2U,2U);
    range(total,8U,9U);
    element(*this,registers,index,selected);
    WordVarArgs addend={selected,increment0,increment1};
    add(*this,addend,total);

    branch(*this,index,INT_VAL_MIN());
    branch(*this,registers,WORD_VAR_NONE(),WORD_VAL_LSB());
    WordVarArgs derived={selected,total};
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
      registers[0].val()+2U*registers[1].val()+
      3U*registers[2].val()+4U*registers[3].val();
  }
};

/** \brief Main-function
 *  \relates WordRegisterFile
 */
int
main(int argc, char* argv[]) {
  RegisterOptions opt("WordRegisterFile");
  opt.parse(argc,argv);
  WordRegisterFile* root=new WordRegisterFile(opt);
  StatusStatistics root_statistics;
  if (root->status(root_statistics) == SS_FAILED) {
    delete root;
    std::cerr << "register-file model failed during initial propagation\n";
    return 1;
  }
  const bool index_pruned=root->index_pruned();
  const unsigned int root_propagators=PropagatorGroup::all.size(*root);
  const unsigned int root_branchers=BrancherGroup::all.size(*root);
  DFS<WordRegisterFile> search(root);
  delete root;
  std::uint64_t solutions=0, checksum=0;
  while (WordRegisterFile* solution=search.next()) {
    ++solutions;
    checksum += solution->solution_value();
    delete solution;
  }
  const Search::Statistics statistics=search.statistics();
  std::cout << "{\"schema_version\":1,\"status\":\"ok\""
            << ",\"formulation\":\"" << opt.formulation_name() << "\""
            << ",\"solutions\":" << solutions
            << ",\"checksum\":" << checksum
            << ",\"index_pruned\":" << (index_pruned ? "true" : "false")
            << ",\"nodes\":" << statistics.node
            << ",\"failures\":" << statistics.fail
            << ",\"propagations\":"
            << root_statistics.propagate+statistics.propagate
            << ",\"root_propagators\":" << root_propagators
            << ",\"root_branchers\":" << root_branchers << "}\n";
  return 0;
}

// STATISTICS: example-any
