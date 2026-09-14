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
#include <gecode/int.hh>
#include <gecode/search.hh>
#include <gecode/word.hh>

#include <cstdint>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <new>
#include <vector>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#include <unistd.h>
#endif

using namespace Gecode;

namespace {

  const unsigned int address_width = 12;
  const unsigned int window_start = 0x100;
  const unsigned int lengths[] = {0x20,0x30,0x20,0x40,0x30,0x20};

  unsigned int length(unsigned int i) {
    return lengths[i % (sizeof(lengths)/sizeof(lengths[0]))];
  }

  unsigned int window_end(unsigned int n, unsigned int slack) {
    unsigned int end = window_start+slack;
    for (unsigned int i=0; i<n; i++)
      end += length(i);
    return end;
  }

  class DMAOptions : public Options {
  private:
    Driver::UnsignedIntOption _size;
    Driver::StringOption _formulation;
    Driver::UnsignedIntOption _retain_clones;
    Driver::UnsignedIntOption _batch;
    Driver::StringOption _measurement;
    Driver::StringOption _search_control;
    Driver::StringOption _projection;
    Driver::UnsignedIntOption _window_slack;
    Driver::UnsignedIntOption _selected_cap;
  public:
    enum Formulation { COMPACT_WORD, BOUNDED_WORD, WORD_INT_CHANNEL, INT_BOOL };
    enum Measurement { SOLVE, LAYOUT, RETAIN_CLONES, BATCH };
    enum SearchControl { NATIVE, PUBLIC_MIN };
    enum Projection { PROJECTION_NONE, PROJECTION_FIRST, PROJECTION_ALL };

    DMAOptions(const char* name)
      : Options(name), _size("size","descriptor count",6),
        _formulation("formulation","model formulation",COMPACT_WORD),
        _retain_clones("retain-clones","number of stabilized root clones",0),
        _batch("batch","in-process searches for tiny cases",1),
        _measurement("measurement","solve, layout, retained clones, or batch",SOLVE),
        _search_control("search-control","native or aligned public decisions",NATIVE),
        _projection("projection","none, first, or all public projections",
                    PROJECTION_NONE),
        _window_slack("window-slack","bytes of descriptor-window slack",0x20),
        _selected_cap("selected-cap","maximum selected limit",0x300) {
      solutions(0);
      add(_size);
      add(_formulation);
      _formulation.add(COMPACT_WORD,"compact-word","compact Word variables");
      _formulation.add(BOUNDED_WORD,"bounded-word","unsigned-bounded Word variables");
      _formulation.add(WORD_INT_CHANNEL,"word-int-channel","compact Word variables with numeric Int channels");
      _formulation.add(INT_BOOL,"int-bool","integer and Boolean variables");
      add(_retain_clones);
      add(_batch);
      add(_search_control);
      _search_control.add(NATIVE,"native","formulation-native value choice");
      _search_control.add(PUBLIC_MIN,"public-min","aligned public decision order and minimum values");
      add(_projection);
      _projection.add(PROJECTION_NONE,"none","do not emit public projections");
      _projection.add(PROJECTION_FIRST,"first","emit the first public projection");
      _projection.add(PROJECTION_ALL,"all","emit every public projection");
      add(_window_slack);
      add(_selected_cap);
      add(_measurement);
      _measurement.add(SOLVE,"solve","exhaustively solve the model");
      _measurement.add(LAYOUT,"layout","report object layout sizes");
      _measurement.add(RETAIN_CLONES,"retain-clones","retain stabilized root clones and report RSS");
      _measurement.add(BATCH,"batch","repeat search in-process without startup timing");
    }
    Formulation formulation(void) const {
      return static_cast<Formulation>(_formulation.value());
    }
    Measurement measurement(void) const {
      return static_cast<Measurement>(_measurement.value());
    }
    unsigned int retain_clones(void) const { return _retain_clones.value(); }
    unsigned int batch(void) const { return _batch.value(); }
    SearchControl search_control(void) const {
      return static_cast<SearchControl>(_search_control.value());
    }
    const char* search_control_name(void) const {
      return search_control() == NATIVE ? "native" : "public-min";
    }
    Projection projection(void) const {
      return static_cast<Projection>(_projection.value());
    }
    unsigned int size(void) const { return _size.value(); }
    unsigned int window_slack(void) const { return _window_slack.value(); }
    unsigned int selected_cap(void) const { return _selected_cap.value(); }
    const char* formulation_name(void) const {
      switch (formulation()) {
      case COMPACT_WORD: return "compact-word";
      case BOUNDED_WORD: return "bounded-word";
      case WORD_INT_CHANNEL: return "word-int-channel";
      default: return "int-bool";
      }
    }
  };

  class DMADescriptor : public Script {
  private:
    DMAOptions::Formulation formulation;
    unsigned int descriptor_count;
    unsigned int descriptor_window_end;
    unsigned int selected_cap;
    WordVarArray base_word, end_word, flag_word;
    WordVar selected_base_word, selected_end_word, selected_plus_word,
      selected_limit_word;
    IntVarArray base_int, end_int, flag_int;
    IntVar selected_base_int, selected_end_int, selected_limit_int;
    IntVar index;
    BoolVarArray write, execute;
    BoolVar selected_write;

    static WordVar numeric_word(Space& home, DMAOptions::Formulation f,
                                unsigned int lo, unsigned int hi) {
      return (f == DMAOptions::BOUNDED_WORD) ?
        WordVar(home,address_width,WDT_UNSIGNED,lo,hi) :
        WordVar(home,address_width);
    }

    static WordVarArray numeric_words(Space& home, unsigned int n,
                                      DMAOptions::Formulation f,
                                      unsigned int lo, unsigned int hi) {
      if (n == 0)
        return WordVarArray(home,0);
      return (f == DMAOptions::BOUNDED_WORD) ?
        WordVarArray(home,static_cast<int>(n),address_width,WDT_UNSIGNED,lo,hi) :
        WordVarArray(home,static_cast<int>(n),address_width,WDT_CUBE);
    }

    static WordVar flag(Space& home) {
      return WordVar(home,4,1,7);
    }

    void post_word_model(bool numeric_channels) {
      const unsigned int limit = descriptor_window_end;
      for (unsigned int i=0; i<descriptor_count; i++) {
        rel(*this,base_word[i],WRT_UGQ,address_width,window_start);
        rel(*this,base_word[i],WRT_ULQ,address_width,limit-0x20);
        rel(*this,end_word[i],WRT_UGQ,address_width,window_start+0x20);
        rel(*this,end_word[i],WRT_ULQ,address_width,limit);
        for (unsigned int b=0; b<4; b++)
          channel(*this,base_word[i],b,0);
        channel(*this,flag_word[i],0,1);
        channel(*this,flag_word[i],1,write[i]);
        channel(*this,flag_word[i],2,execute[i]);
        channel(*this,flag_word[i],3,0);
        rel(*this,write[i]+execute[i] <= 1);
        if (numeric_channels) {
          channel(*this,base_word[i],base_int[i],WDT_UNSIGNED);
          channel(*this,end_word[i],end_int[i],WDT_UNSIGNED);
          rel(*this,end_int[i] == base_int[i]+static_cast<int>(length(i)));
          rel(*this,end_int[i],IRT_LQ,static_cast<int>(limit));
          if (i+1 < descriptor_count)
            rel(*this,end_int[i],IRT_LQ,base_int[i+1]);
        } else {
          add(*this,base_word[i],address_width,length(i),end_word[i]);
          rel(*this,end_word[i],WRT_ULQ,address_width,limit);
          if (i+1 < descriptor_count)
            rel(*this,end_word[i],WRT_ULQ,base_word[i+1]);
        }
        if (i+1 < descriptor_count)
          rel(*this,flag_word[i],WRT_ULQ,flag_word[i+1]);
      }
      linear(*this,write,IRT_EQ,static_cast<int>(descriptor_count/3));
      linear(*this,execute,IRT_EQ,static_cast<int>(descriptor_count/3));
      element(*this,base_word,index,selected_base_word);
      element(*this,end_word,index,selected_end_word);
      element(*this,flag_word,index,flag_word_selected());
      channel(*this,flag_word_selected(),1,selected_write);
      add(*this,selected_end_word,address_width,0x10,selected_plus_word);
      ite(*this,selected_write,selected_plus_word,selected_end_word,
          selected_limit_word);
      if (numeric_channels) {
        channel(*this,selected_base_word,selected_base_int,WDT_UNSIGNED);
        channel(*this,selected_end_word,selected_end_int,WDT_UNSIGNED);
        channel(*this,selected_plus_word,_selected_plus_int,WDT_UNSIGNED);
        channel(*this,selected_limit_word,selected_limit_int,WDT_UNSIGNED);
        rel(*this,selected_limit_int,IRT_LQ,static_cast<int>(selected_cap));
        rel(*this,selected_limit_int,IRT_LQ,static_cast<int>(limit));
      } else {
        rel(*this,selected_limit_word,WRT_ULQ,address_width,selected_cap);
        rel(*this,selected_limit_word,WRT_ULQ,address_width,limit);
      }
    }

    WordVar& flag_word_selected(void) { return _selected_flag_word; }
    const WordVar& flag_word_selected(void) const { return _selected_flag_word; }
    WordVar _selected_flag_word;

    void post_int_model(void) {
      const int limit = static_cast<int>(descriptor_window_end);
      for (unsigned int i=0; i<descriptor_count; i++) {
        rel(*this,base_int[i] % 16 == 0);
        rel(*this,end_int[i] == base_int[i]+static_cast<int>(length(i)));
        rel(*this,end_int[i],IRT_LQ,limit);
        rel(*this,write[i]+execute[i] <= 1);
        rel(*this,flag_int[i] == 1+2*write[i]+4*execute[i]);
        if (i+1 < descriptor_count) {
          rel(*this,end_int[i],IRT_LQ,base_int[i+1]);
          rel(*this,flag_int[i],IRT_LQ,flag_int[i+1]);
        }
      }
      linear(*this,write,IRT_EQ,static_cast<int>(descriptor_count/3));
      linear(*this,execute,IRT_EQ,static_cast<int>(descriptor_count/3));
      element(*this,base_int,index,selected_base_int);
      element(*this,end_int,index,selected_end_int);
      element(*this,flag_int,index,_selected_flag_int);
      rel(*this,_selected_flag_int == 1+2*selected_write+4*_selected_execute);
      ite(*this,selected_write,_selected_plus_int,selected_end_int,
          selected_limit_int);
      rel(*this,_selected_plus_int == selected_end_int+0x10);
      rel(*this,selected_limit_int,IRT_LQ,static_cast<int>(selected_cap));
      rel(*this,selected_limit_int,IRT_LQ,limit);
    }

    IntVar _selected_flag_int, _selected_plus_int;
    BoolVar _selected_execute;

  public:
    DMADescriptor(const DMAOptions& opt)
      : Script(opt), formulation(opt.formulation()), descriptor_count(opt.size()),
        descriptor_window_end(window_end(opt.size(),opt.window_slack())),
        selected_cap(opt.selected_cap()),
        base_word(numeric_words(*this,formulation == DMAOptions::INT_BOOL ? 0 : opt.size(),
                                formulation,window_start,descriptor_window_end-0x20)),
        end_word(numeric_words(*this,formulation == DMAOptions::INT_BOOL ? 0 : opt.size(),
                               formulation,window_start+0x20,descriptor_window_end)),
        flag_word(*this,formulation == DMAOptions::INT_BOOL ? 0 : opt.size(),4,1,7),
        selected_base_word(), selected_end_word(), selected_plus_word(),
        selected_limit_word(),
        base_int(*this,(formulation == DMAOptions::INT_BOOL ||
                       formulation == DMAOptions::WORD_INT_CHANNEL) ? opt.size() : 0,
                 window_start,static_cast<int>(descriptor_window_end-0x20)),
        end_int(*this,(formulation == DMAOptions::INT_BOOL ||
                      formulation == DMAOptions::WORD_INT_CHANNEL) ? opt.size() : 0,
                window_start+0x20,static_cast<int>(descriptor_window_end)),
        flag_int(*this,formulation == DMAOptions::INT_BOOL ? opt.size() : 0,1,5),
        selected_base_int(), selected_end_int(), selected_limit_int(),
        index(*this,0,static_cast<int>(opt.size()-1)),
        write(*this,opt.size(),0,1), execute(*this,opt.size(),0,1),
        selected_write(*this,0,1), _selected_flag_word(),
        _selected_flag_int(), _selected_plus_int(), _selected_execute() {
      if ((descriptor_count < 3) || (descriptor_count > 9)) {
        std::cerr << "descriptor count must be between 3 and 9\n";
        std::exit(EXIT_FAILURE);
      }
      const unsigned int limit = descriptor_window_end;
      if (formulation != DMAOptions::INT_BOOL) {
        selected_base_word = numeric_word(*this,formulation,window_start,limit-0x20);
        selected_end_word = numeric_word(*this,formulation,window_start+0x20,limit);
        selected_plus_word = numeric_word(*this,formulation,window_start+0x30,
                                          limit+0x10);
        selected_limit_word = numeric_word(*this,formulation,window_start+0x20,
                                           limit);
        _selected_flag_word = flag(*this);
        if (formulation == DMAOptions::WORD_INT_CHANNEL) {
          selected_base_int = IntVar(*this,window_start,static_cast<int>(limit-0x20));
          selected_end_int = IntVar(*this,window_start+0x20,static_cast<int>(limit));
          _selected_plus_int = IntVar(*this,window_start+0x30,
                                      static_cast<int>(limit+0x10));
          selected_limit_int = IntVar(*this,window_start+0x20,static_cast<int>(limit));
        }
        post_word_model(formulation == DMAOptions::WORD_INT_CHANNEL);
      } else {
        selected_base_int = IntVar(*this,window_start,static_cast<int>(limit-0x20));
        selected_end_int = IntVar(*this,window_start+0x20,static_cast<int>(limit));
        selected_limit_int = IntVar(*this,window_start+0x20,static_cast<int>(limit));
        _selected_flag_int = IntVar(*this,1,5);
        _selected_plus_int = IntVar(*this,window_start+0x30,static_cast<int>(limit+0x10));
        _selected_execute = BoolVar(*this,0,1);
        post_int_model();
      }

      branch(*this,index,INT_VAL_MIN());
      branch(*this,write,BOOL_VAR_NONE(),BOOL_VAL_MIN());
      branch(*this,execute,BOOL_VAR_NONE(),BOOL_VAL_MIN());
      if (formulation == DMAOptions::INT_BOOL) {
        branch(*this,base_int,INT_VAR_NONE(),INT_VAL_MIN());
        if (opt.search_control() == DMAOptions::PUBLIC_MIN)
          return;
        IntVarArgs rest(end_int);
        rest << flag_int << selected_base_int << selected_end_int
             << selected_limit_int << _selected_flag_int << _selected_plus_int;
        branch(*this,rest,INT_VAR_NONE(),INT_VAL_MIN());
      } else {
        const WordValBranch values = (formulation == DMAOptions::BOUNDED_WORD) ?
          WORD_VAL_SPLIT_MIN() :
          (opt.search_control() == DMAOptions::PUBLIC_MIN ?
           WORD_VAL_MSB() : WORD_VAL_LSB());
        branch(*this,base_word,WORD_VAR_NONE(),values);
        if (opt.search_control() == DMAOptions::PUBLIC_MIN)
          return;
        WordVarArgs rest(end_word);
        rest << selected_base_word << selected_end_word
             << selected_plus_word << selected_limit_word;
        branch(*this,rest,WORD_VAR_NONE(),values);
        branch(*this,flag_word,WORD_VAR_NONE(),WORD_VAL_LSB());
        branch(*this,_selected_flag_word,WORD_VAL_LSB());
        if (formulation == DMAOptions::WORD_INT_CHANNEL) {
          IntVarArgs numeric_rest(base_int);
          numeric_rest << end_int << selected_base_int << selected_end_int
                       << _selected_plus_int << selected_limit_int;
          branch(*this,numeric_rest,INT_VAR_NONE(),INT_VAL_MIN());
        }
      }
    }

    DMADescriptor(DMADescriptor& s)
      : Script(s), formulation(s.formulation), descriptor_count(s.descriptor_count),
        descriptor_window_end(s.descriptor_window_end), selected_cap(s.selected_cap) {
      index.update(*this,s.index); write.update(*this,s.write);
      execute.update(*this,s.execute); selected_write.update(*this,s.selected_write);
      if (formulation != DMAOptions::INT_BOOL) {
        base_word.update(*this,s.base_word); end_word.update(*this,s.end_word);
        flag_word.update(*this,s.flag_word);
        selected_base_word.update(*this,s.selected_base_word);
        selected_end_word.update(*this,s.selected_end_word);
        selected_plus_word.update(*this,s.selected_plus_word);
        selected_limit_word.update(*this,s.selected_limit_word);
        _selected_flag_word.update(*this,s._selected_flag_word);
        if (formulation == DMAOptions::WORD_INT_CHANNEL) {
          base_int.update(*this,s.base_int); end_int.update(*this,s.end_int);
          selected_base_int.update(*this,s.selected_base_int);
          selected_end_int.update(*this,s.selected_end_int);
          _selected_plus_int.update(*this,s._selected_plus_int);
          selected_limit_int.update(*this,s.selected_limit_int);
        }
      } else {
        base_int.update(*this,s.base_int); end_int.update(*this,s.end_int);
        flag_int.update(*this,s.flag_int);
        selected_base_int.update(*this,s.selected_base_int);
        selected_end_int.update(*this,s.selected_end_int);
        selected_limit_int.update(*this,s.selected_limit_int);
        _selected_flag_int.update(*this,s._selected_flag_int);
        _selected_plus_int.update(*this,s._selected_plus_int);
        _selected_execute.update(*this,s._selected_execute);
      }
    }

    virtual Space* copy(void) { return new DMADescriptor(*this); }

    std::uint64_t solution_value(void) const {
      std::uint64_t value = static_cast<std::uint64_t>(index.val()+1);
      if (formulation == DMAOptions::INT_BOOL) {
        value += static_cast<std::uint64_t>(selected_limit_int.val());
        for (unsigned int i=0; i<descriptor_count; i++)
          value += static_cast<std::uint64_t>(i+1)*base_int[i].val()+flag_int[i].val();
      } else {
        value += selected_limit_word.val();
        for (unsigned int i=0; i<descriptor_count; i++)
          value += static_cast<std::uint64_t>(i+1)*base_word[i].val()+flag_word[i].val();
      }
      return value;
    }

    std::vector<unsigned int> public_projection(void) const {
      std::vector<unsigned int> values;
      values.reserve(1+2*descriptor_count);
      values.push_back(static_cast<unsigned int>(index.val()));
      if (formulation == DMAOptions::INT_BOOL) {
        for (unsigned int i=0; i<descriptor_count; i++)
          values.push_back(static_cast<unsigned int>(base_int[i].val()));
        for (unsigned int i=0; i<descriptor_count; i++)
          values.push_back(static_cast<unsigned int>(flag_int[i].val()));
      } else {
        for (unsigned int i=0; i<descriptor_count; i++)
          values.push_back(static_cast<unsigned int>(base_word[i].val()));
        for (unsigned int i=0; i<descriptor_count; i++)
          values.push_back(static_cast<unsigned int>(flag_word[i].val()));
      }
      return values;
    }
  };

  unsigned long long rss_bytes(void) {
#if defined(__linux__)
    std::ifstream status("/proc/self/statm");
    unsigned long long pages=0, resident=0;
    if (status >> pages >> resident)
      return resident*static_cast<unsigned long long>(sysconf(_SC_PAGESIZE));
#endif
#if defined(__APPLE__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF,&usage) == 0)
      return static_cast<unsigned long long>(usage.ru_maxrss);
#endif
    return 0;
  }

}

/**
 * \brief Compare four equivalent DMA descriptor-window formulations
 *
 * The model deliberately exercises address arithmetic, ordering, Word flag
 * bits, Element, and a Boolean-controlled limit. It is one mixed workload,
 * not a prediction for all models that use Word variables.
 *
 * \ingroup Example
 */
int
main(int argc, char* argv[]) {
  DMAOptions opt("WordDMADescriptor");
  opt.parse(argc,argv);
  if ((opt.size() < 3U) || (opt.size() > 9U) ||
      (window_end(opt.size(),opt.window_slack()) >
       Word::width_mask(address_width)) ||
      (opt.selected_cap() > Word::width_mask(address_width))) {
    std::cerr << "invalid descriptor count, window slack, or selected cap\n";
    return 2;
  }
  if (opt.measurement() == DMAOptions::LAYOUT) {
    std::cout << "{\"schema_version\":1,\"measurement\":\"layout\""
              << ",\"word_var_bytes\":" << sizeof(WordVar)
              << ",\"int_var_bytes\":" << sizeof(IntVar)
              << ",\"bool_var_bytes\":" << sizeof(BoolVar)
              << ",\"word_var_imp_bytes\":" << sizeof(Word::WordVarImp)
              << ",\"bounded_word_var_imp_bytes\":"
              << sizeof(Word::BoundedWordVarImp)
              << ",\"int_var_imp_bytes\":" << sizeof(Int::IntVarImp)
              << ",\"bool_var_imp_bytes\":" << sizeof(Int::BoolVarImp)
              << ",\"model_bytes\":" << sizeof(DMADescriptor) << "}\n";
    return 0;
  }
  const auto construction_start = std::chrono::steady_clock::now();
  DMADescriptor* root = new DMADescriptor(opt);
  const auto construction_end = std::chrono::steady_clock::now();
  StatusStatistics root_statistics;
  const auto root_start = std::chrono::steady_clock::now();
  const bool root_failed = root->status(root_statistics) == SS_FAILED;
  const auto root_end = std::chrono::steady_clock::now();
  if (opt.measurement() == DMAOptions::RETAIN_CLONES) {
    std::vector<Space*> clones;
    try {
      clones.reserve(opt.retain_clones());
      for (unsigned int i=0; i<opt.retain_clones(); i++)
        clones.push_back(root->clone());
    } catch (const std::bad_alloc&) {
      for (Space* clone : clones) delete clone;
      delete root;
      std::cerr << "could not retain requested clones\n";
      return 3;
    }
    std::cout << "{\"schema_version\":1,\"measurement\":\"retained-clones\""
              << ",\"formulation\":\"" << opt.formulation_name() << "\""
              << ",\"size\":" << opt.size()
              << ",\"retained_clones\":" << clones.size()
              << ",\"rss_bytes\":" << rss_bytes() << "}\n";
    for (Space* clone : clones) delete clone;
    delete root;
    return 0;
  }
  const unsigned int root_propagators = PropagatorGroup::all.size(*root);
  const unsigned int root_branchers = BrancherGroup::all.size(*root);
  const unsigned int batch = opt.measurement() == DMAOptions::BATCH ?
    opt.batch() : 1U;
  std::uint64_t solutions = 0, checksum = 0;
  std::vector<std::vector<unsigned int> > projections;
  Search::Statistics statistics;
  const auto search_start = std::chrono::steady_clock::now();
  for (unsigned int iteration=0; iteration<batch; iteration++) {
    if (root_failed)
      break;
    DFS<DMADescriptor> search(static_cast<DMADescriptor*>(root->clone()));
    std::uint64_t iteration_solutions = 0, iteration_checksum = 0;
    while (DMADescriptor* solution = search.next()) {
      ++iteration_solutions;
      iteration_checksum += solution->solution_value();
      if ((iteration == 0) &&
          ((opt.projection() == DMAOptions::PROJECTION_ALL) ||
           ((opt.projection() == DMAOptions::PROJECTION_FIRST) &&
            projections.empty())))
        projections.push_back(solution->public_projection());
      delete solution;
    }
    if (iteration == 0) {
      solutions = iteration_solutions;
      checksum = iteration_checksum;
    } else if ((solutions != iteration_solutions) ||
               (checksum != iteration_checksum)) {
      delete root;
      std::cerr << "inconsistent in-process batch result\n";
      return 4;
    }
    const Search::Statistics current = search.statistics();
    statistics.propagate += current.propagate;
    statistics.node += current.node;
    statistics.fail += current.fail;
  }
  const auto search_end = std::chrono::steady_clock::now();
  delete root;
  const std::chrono::duration<double> construction_seconds =
    construction_end-construction_start;
  const std::chrono::duration<double> root_seconds = root_end-root_start;
  const std::chrono::duration<double> search_seconds = search_end-search_start;
  std::cout << "{\"schema_version\":1,\"status\":\"ok\""
            << ",\"formulation\":\"" << opt.formulation_name() << "\""
            << ",\"search_control\":\"" << opt.search_control_name() << "\""
            << ",\"size\":" << opt.size()
            << ",\"window_slack\":" << opt.window_slack()
            << ",\"selected_cap\":" << opt.selected_cap()
            << ",\"batch_iterations\":" << batch
            << ",\"construction_seconds\":" << construction_seconds.count()
            << ",\"root_seconds\":" << root_seconds.count()
            << ",\"search_seconds\":" << search_seconds.count()
            << ",\"semantic_status\":\""
            << (solutions == 0 ? "unsat" : "sat") << "\""
            << ",\"decision_variables\":[\"index\"";
  for (unsigned int i=0; i<opt.size(); i++)
    std::cout << ",\"base[" << i << "]\"";
  for (unsigned int i=0; i<opt.size(); i++)
    std::cout << ",\"flag[" << i << "]\"";
  std::cout << "]" << ",\"projections\":[";
  for (std::size_t i=0; i<projections.size(); i++) {
    if (i != 0) std::cout << ',';
    std::cout << '[';
    for (std::size_t j=0; j<projections[i].size(); j++) {
      if (j != 0) std::cout << ',';
      std::cout << projections[i][j];
    }
    std::cout << ']';
  }
  std::cout << "]"
            << ",\"solutions\":" << solutions
            << ",\"checksum\":" << checksum
            << ",\"nodes\":" << statistics.node
            << ",\"failures\":" << statistics.fail
            << ",\"propagations\":"
            << root_statistics.propagate+statistics.propagate
            << ",\"root_propagators\":" << root_propagators
            << ",\"root_branchers\":" << root_branchers << "}\n";
  return 0;
}

// STATISTICS: example-any
