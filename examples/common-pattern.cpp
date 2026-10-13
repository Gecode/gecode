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
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace Gecode;

/// Options for the input sequences and the pattern language
class CommonPatternOptions : public Options {
protected:
  /// Input sequences as a comma-separated command-line value
  Driver::StringValueOption _sequences;
  /// Whether the pattern must start with A and end with C
  Driver::BoolOption _motif;
  /// Validated input sequences
  std::vector<std::string> input;
public:
  /// Initialize the example options
  CommonPatternOptions(void)
    : Options("CommonPattern"),
      _sequences("sequences","comma-separated nonempty sequences over A..Z",
                 "ABCBC,BABC,ABDC"),
      _motif("motif","require the pattern to start with A and end with C",true) {
    add(_sequences);
    add(_motif);
  }

  /// Read and validate the input after parsing command-line options
  bool read(void) {
    const std::string text(_sequences.value());
    if (text.empty() || (text.back() == ','))
      return false;
    std::istringstream stream(text);
    std::string sequence;
    while (std::getline(stream,sequence,',')) {
      if (sequence.empty() ||
          (sequence.size() >= static_cast<size_t>(Int::Limits::max)) ||
          (sequence.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
           std::string::npos))
        return false;
      input.push_back(sequence);
    }
    return true;
  }

  /// Return the input sequences
  const std::vector<std::string>& sequences(void) const { return input; }
  /// Whether to impose the pattern language
  bool motif(void) const { return _motif.value(); }
};

namespace {

  /**
   * \brief Automaton accepting all subsequences of \a text
   *
   * State i means that the next match must occur at or after position i.
   * Matching the earliest occurrence of a symbol leaves every later match
   * available. All states accept, including the state after the final item.
   */
  DFA
  subsequences(const std::string& text) {
    const int n = static_cast<int>(text.size());
    int next[26];
    std::fill(next,next+26,-1);
    std::vector<DFA::Transition> transitions;
    for (int i=n; i--; ) {
      next[text[i]-'A'] = i+1;
      for (int a=0; a<26; a++)
        if (next[a] >= 0)
          transitions.emplace_back(i,'A'+a,next[a]);
    }
    transitions.emplace_back(-1,0,0);
    std::vector<int> finals(n+2);
    for (int i=0; i<=n; i++)
      finals[i] = i;
    finals[n+1] = -1;
    return DFA(0,transitions.data(),finals.data());
  }

}

/**
 * \brief %Example: constrained common subsequences of unknown length
 *
 * Find a longest nonempty pattern occurring as a subsequence of every input.
 * Matching preserves order but may skip input items. By default the pattern
 * must also start with A and end with C. The inputs ABCBC, BABC, and ABDC have
 * two such patterns, AC and ABC, with ABC the unique longest one.
 *
 * This is the all-input-support case of constrained sequential pattern mining.
 * Kemmar et al., "PREFIX-PROJECTION Global Constraint for Sequential Pattern
 * Mining", CP 2015, doi:10.1007/978-3-319-23219-5_17, model varying pattern
 * lengths with a fixed array and a special termination item. An OpenIntVarList
 * instead contains only actual pattern positions: no padding symbol, suffix
 * implications, or padding transitions in the pattern-language automaton.
 * The shortest input bounds the length, but does not allocate that many
 * decision variables. Constraints posted once follow the growing list.
 *
 * Each input is represented by a subsequence DFA. This keeps the example in
 * the modelling API; it does not implement the paper's shared prefix-projection
 * propagator or arbitrary minimum-support thresholds. Automata still require
 * storage for the inputs, and lazy variables alone do not guarantee faster
 * search. OpenRoute provides a smaller introduction to open lists.
 *
 * Run with -search dfs to enumerate all feasible patterns, -motif false to
 * remove the pattern-language restriction, or -sequences ABCBC,BABC,ABDC to
 * supply input sequences. Default branch-and-bound search maximizes length.
 * For example, -search dfs -motif false enumerates seven patterns on the
 * default input: A, B, C, AB, AC, BC, and ABC.
 *
 * \ingroup Example
 */
class CommonPattern : public IntMaximizeScript {
protected:
  /// Items of the pattern, materialized as required by propagation and search
  OpenIntVarList pattern;
public:
  /// Search engines
  enum { SEARCH_BAB, SEARCH_DFS };

  /// Construct the model
  CommonPattern(const CommonPatternOptions& opt)
    : IntMaximizeScript(opt) {
    int bound = Int::Limits::max;
    for (const std::string& sequence : opt.sequences())
      bound = std::min(bound,static_cast<int>(sequence.size()));
    pattern = OpenIntVarList(*this,IntSet('A','Z'),bound);
    rel(*this,pattern.length(),IRT_GQ,1);

    for (const std::string& sequence : opt.sequences())
      extensional(*this,pattern,subsequences(sequence));
    if (opt.motif()) {
      REG alphabet(IntArgs::create(26,'A',1));
      extensional(*this,pattern,REG('A') + *alphabet + REG('C'));
    }

    // Assign the current prefix, then close it or add another item.
    branch(*this,pattern,INT_VAR_NONE(),INT_VAL_MIN());
  }

  /// Maximize the eventual pattern length
  virtual IntVar cost(void) const {
    return pattern.length();
  }

  /// Constructor for cloning \a s
  CommonPattern(CommonPattern& s) : IntMaximizeScript(s) {
    pattern.update(*this,s.pattern);
  }

  /// Copy during cloning
  virtual Space* copy(void) {
    return new CommonPattern(*this);
  }

  /// Print the current prefix (complete in a solution)
  virtual void print(std::ostream& os) const {
    os << "\t";
    for (int i=0; i<pattern.prefix_size(); i++)
      if (pattern[i].assigned())
        os << static_cast<char>(pattern[i].val());
      else
        os << pattern[i];
    os << " (length " << pattern.length() << ")" << std::endl;
  }
};

/** \brief Main-function
 *  \relates CommonPattern
 */
int
main(int argc, char* argv[]) {
  CommonPatternOptions opt;
  opt.solutions(0);
  opt.search(CommonPattern::SEARCH_BAB);
  opt.search(CommonPattern::SEARCH_BAB,"bab","find a longest pattern");
  opt.search(CommonPattern::SEARCH_DFS,"dfs","enumerate feasible patterns");
  opt.parse(argc,argv);
  if (!opt.read()) {
    std::cerr << "Expected comma-separated nonempty sequences over A..Z"
              << std::endl;
    return 1;
  }
  if (opt.search() == CommonPattern::SEARCH_DFS)
    IntMaximizeScript::run<CommonPattern,DFS,CommonPatternOptions>(opt);
  else
    IntMaximizeScript::run<CommonPattern,BAB,CommonPatternOptions>(opt);
  return 0;
}

// STATISTICS: example-any
