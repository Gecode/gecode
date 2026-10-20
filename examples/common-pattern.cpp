/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
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

#include <gecode/driver.hh>

#include "common-pattern.hh"

#include <fstream>
#include <memory>

using namespace Gecode;

/// Sequence database and restrictions on the patterns to mine
class CommonPatternOptions : public Options {
protected:
  Driver::StringValueOption _sequences, _file, _support, _required;
  Driver::StringOption _format, _language;
  Driver::UnsignedIntOption _min_length;
  Driver::BoolOption _motif;
  std::unique_ptr<PatternMining::Database> input;
  std::vector<int> required_items;
  int threshold;
public:
  CommonPatternOptions(void)
    : Options("CommonPattern"),
      _sequences("sequences","comma-separated sequences over A..Z","ABCBC,BABC,ABDC"),
      _file("file","sequence database; one sequence per line",""),
      _support("support","minimum supporting sequences, as a percentage","100"),
      _required("required","items that must occur, comma-separated letters or IDs",""),
      _format("format","database format",PatternMining::LETTERS),
      _language("language","pattern language",PatternMining::DEFAULT),
      _min_length("min-length","minimum pattern length",1),
      _motif("motif","default language starts with A and ends with C",true),
      threshold(0) {
    _format.add(PatternMining::LETTERS,"letters");
    _format.add(PatternMining::INTEGERS,"integers");
    _format.add(PatternMining::SPMF,"spmf");
    _language.add(PatternMining::DEFAULT,"default","use -motif");
    _language.add(PatternMining::NONE,"none");
    _language.add(PatternMining::MOTIF,"motif");
    _language.add(PatternMining::RE10,"re10");
    _language.add(PatternMining::RE14,"re14");
    _language.add(PatternMining::RE2,"re2");
    add(_sequences); add(_file); add(_format); add(_support);
    add(_min_length); add(_required); add(_language); add(_motif);
  }

  /// Validate external input before constructing a space
  void read(void) {
    if (_min_length.value() < 1 || _min_length.value() > Int::Limits::max)
      throw std::invalid_argument("Minimum length must be a positive Gecode integer");
    PatternMining::Sequences sequences;
    const std::string filename(_file.value());
    if (filename.empty()) {
      if (_format.value() != PatternMining::LETTERS)
        throw std::invalid_argument("Integer and SPMF formats require -file");
      const std::string text(_sequences.value());
      if (text.empty() || text.back() == ',')
        throw std::invalid_argument("Expected comma-separated nonempty A..Z sequences");
      std::istringstream stream(text);
      std::string sequence;
      while (std::getline(stream,sequence,','))
        sequences.push_back(PatternMining::read_sequence(sequence,PatternMining::LETTERS));
    } else {
      std::ifstream stream(filename);
      sequences = PatternMining::read_database(stream,
        static_cast<PatternMining::Format>(_format.value()));
    }
    input.reset(new PatternMining::Database(sequences));
    threshold = PatternMining::minimum_support(_support.value(),input->ends.size());
    const std::string items(_required.value());
    if (!items.empty()) {
      if (items.back() == ',')
        throw std::invalid_argument("Empty required item");
      std::istringstream stream(items);
      std::string item;
      while (std::getline(stream,item,',')) {
        const std::vector<int> parsed = PatternMining::read_sequence(item,
          letters() ? PatternMining::LETTERS : PatternMining::INTEGERS);
        if (parsed.size() != 1)
          throw std::invalid_argument("Required items must be single letters or IDs");
        required_items.push_back(parsed[0]);
      }
    }
  }

  const PatternMining::Database& database(void) const { return *input; }
  const std::vector<int>& required(void) const { return required_items; }
  int support(void) const { return threshold; }
  int min_length(void) const { return static_cast<int>(_min_length.value()); }
  bool letters(void) const { return _format.value() == PatternMining::LETTERS; }
  PatternMining::Language language(void) const {
    if (_language.value() == PatternMining::DEFAULT)
      return _motif.value() ? PatternMining::MOTIF : PatternMining::NONE;
    return static_cast<PatternMining::Language>(_language.value());
  }
};

/**
 * \brief %Example: constrained frequent subsequences of unknown length
 *
 * A pattern must occur in at least -support percent of the input sequences.
 * Matching preserves order and may skip items; repeated items match distinct
 * positions. Support counts records, including duplicate records, once each.
 * By default the inputs ABCBC, BABC, ABDC must all support the pattern, which
 * must start with A and end with C. AC and ABC are feasible; ABC is longest.
 *
 * The pattern is an ordered list of items of unknown length. The frequency
 * constraint requires sufficiently many records to contain that list as a
 * subsequence. Ordinary length and regular constraints restrict the pattern:
 * they do not restrict the matching positions in the input records. Default
 * DFS enumerates every qualifying pattern; -search bab finds a longest one.
 * These are different queries from closed, maximal, or top-k frequent mining.
 *
 * An OpenIntVarList constructs pattern positions as needed, without an end
 * marker or unused suffix variables. Pattern languages therefore describe
 * just the pattern, without padding transitions. The private frequency
 * constraint maintains supporting suffixes after the assigned prefix and
 * removes items with insufficient support. This prefix projection follows
 * Kemmar et al., "PREFIX-PROJECTION Global Constraint for Sequential Pattern
 * Mining", CP 2015, doi:10.1007/978-3-319-23219-5_17, with an additional
 * remaining-suffix length bound. Projection is a filtering method, independent
 * of the pattern's length, item and language restrictions. OpenRoute is the
 * simpler introduction to open lists.
 *
 * The paper's Table 1 can be mined with:
 * -sequences ABCBC,BABC,AB,BCD -support 50 -language none.
 * -min-length and -required constrain length and item membership. -language
 * also accepts motif (A, any items, C), re10, re14, re2 (expressions
 * from the paper). Their letters mean item IDs A=1 through Z=26; integer data
 * must use the corresponding mapping for these named expressions.
 *
 * -file reads one sequence per line in -format letters, integers, or spmf.
 * Integer IDs are positive and preserved in output. SPMF records must have
 * singleton itemsets: "1 -1 2 -1 3 -1 -2" is the sequence ABC. Multi-item
 * itemsets are rejected because they define a different mining problem.
 * No gap or time restrictions are supported.
 *
 * \ingroup Example
 */
class CommonPattern : public IntMaximizeScript {
protected:
  OpenIntVarList pattern;
  IntVar frequency;
  PatternMining::Database database;
  bool letters;
public:
  enum { SEARCH_BAB, SEARCH_DFS };

  CommonPattern(const CommonPatternOptions& opt)
    : IntMaximizeScript(opt), database(opt.database()), letters(opt.letters()) {
    pattern = OpenIntVarList(*this,database.domain(),database.max_length());
    rel(*this,pattern.length(),IRT_GQ,opt.min_length());
    frequency = PatternMining::frequent(*this,pattern,database,opt.support());
    if (opt.language() != PatternMining::NONE)
      extensional(*this,pattern,PatternMining::language(opt.language(),database));
    if (!opt.required().empty()) {
      REG alphabet(IntArgs(database.alphabet));
      for (int item : opt.required())
        extensional(*this,pattern,*alphabet + REG(item) + *alphabet);
    }
    branch(*this,pattern,INT_VAR_NONE(),INT_VAL_MIN());
  }

  virtual IntVar cost(void) const { return pattern.length(); }

  CommonPattern(CommonPattern& s)
    : IntMaximizeScript(s), database(s.database), letters(s.letters) {
    pattern.update(*this,s.pattern);
    frequency.update(*this,s.frequency);
  }

  virtual Space* copy(void) { return new CommonPattern(*this); }

  virtual void print(std::ostream& os) const {
    os << "\t";
    for (int i=0; i<pattern.known_length(); i++) {
      if (i && !letters)
        os << ' ';
      if (pattern[i].assigned()) {
        const int item = pattern[i].val();
        if (letters)
          os << static_cast<char>('A'+item-1);
        else
          os << item;
      } else {
        os << pattern[i];
      }
    }
    os << " (length " << pattern.length()
       << ", support " << frequency << '/' << database.ends.size()
       << ")" << std::endl;
  }
};

/** \brief Main-function
 *  \relates CommonPattern
 */
int
main(int argc, char* argv[]) {
  CommonPatternOptions opt;
  opt.solutions(0);
  opt.search(CommonPattern::SEARCH_DFS);
  opt.search(CommonPattern::SEARCH_BAB,"bab","find a longest pattern");
  opt.search(CommonPattern::SEARCH_DFS,"dfs","enumerate frequent patterns");
  opt.parse(argc,argv);
  try {
    opt.read();
  } catch (const std::invalid_argument& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
  if (opt.search() == CommonPattern::SEARCH_DFS)
    IntMaximizeScript::run<CommonPattern,DFS,CommonPatternOptions>(opt);
  else
    IntMaximizeScript::run<CommonPattern,BAB,CommonPatternOptions>(opt);
  return 0;
}

// STATISTICS: example-any
