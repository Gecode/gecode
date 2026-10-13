/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 * Benchmark companion to examples/common-pattern.cpp.
 * Copyright Mikael Zayenz Lagerkvist, 2026.
 * Distributed under the MIT license; see LICENSE.
 */

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace Gecode;

namespace {

  /// The representations compared on identical enumeration problems
  enum class Model { OpenDFA, PaddedDFA, PaperProjection };

  /// Earliest-match subsequence DFA, optionally accepting trailing zeroes
  DFA
  subsequences(const std::string& text, bool padded) {
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
    if (padded) {
      for (int i=0; i<=n+1; i++)
        transitions.emplace_back(i,0,n+1);
    }
    transitions.emplace_back(-1,0,0);
    const int states = n+1+static_cast<int>(padded);
    std::vector<int> finals(states+1);
    for (int i=0; i<states; i++)
      finals[i] = i;
    finals[states] = -1;
    return DFA(0,transitions.data(),finals.data());
  }

  /// Pattern restrictions from the example and CP2015 Section 5(d)
  REG
  language(const std::string& name) {
    if (name == "motif")
      return REG('A') + *REG(IntArgs::create(26,'A',1)) + REG('C');
    if (name == "re10")
      return *REG('A') + REG('B') + (REG('B') | REG('C')) +
        *REG('D') + REG('E') + *REG('F') + (REG('G') | REG('H')) +
        *REG('I');
    if (name == "re14")
      return *REG('A') + (REG('Q') | (REG('B') + *REG('S') +
             (REG('B') | REG('C')))) + *REG('D') + REG('E') +
        *(REG('I') | REG('S')) + (REG('F') | REG('H')) +
        *REG('G') + REG('R');
    // RE2: the middle position is any item in the alphabet.
    return (REG('S') | REG('T')) + REG(IntArgs::create(26,'A',1)) +
      (REG('R') | REG('K'));
  }

  /**
   * \brief CP2015 Algorithm 2 specialized to all-input support
   *
   * This is a reimplementation, not the authors' unavailable executable.
   * Each input has one earliest-match cursor, advanced after each item.
   * All sequences must support the prefix; the intersection of suffix items
   * is therefore exactly GETFREQITEMS at 100% support. Only the current
   * projection is retained in a space; Gecode cloning restores parent states.
   * Fixed positions and zero termination retain the paper's representation.
   */
  class PaperProjection : public NaryPropagator<Int::IntView,Int::PC_INT_VAL> {
  protected:
    /// Immutable concatenated database and exclusive sequence ends
    IntSharedArray items, ends;
    /// Next unmatched input position for each sequence
    int* cursor;
    /// Number of consumed contiguous pattern positions
    int consumed;
  public:
    /// Post the initial projection
    PaperProjection(Space& home, ViewArray<Int::IntView>& x,
                    const IntSharedArray& data, const IntSharedArray& limits)
      : NaryPropagator<Int::IntView,Int::PC_INT_VAL>(home,x),
        items(data), ends(limits), cursor(home.alloc<int>(ends.size())),
        consumed(0) {
      for (int i=0; i<ends.size(); i++)
        cursor[i] = (i == 0) ? 0 : ends[i-1];
      home.notice(*this,AP_DISPOSE);
    }

    /// Clone the current projection independently of its parent
    PaperProjection(Space& home, PaperProjection& p)
      : NaryPropagator<Int::IntView,Int::PC_INT_VAL>(home,p),
        items(p.items), ends(p.ends), cursor(home.alloc<int>(ends.size())),
        consumed(p.consumed) {
      std::copy(p.cursor,p.cursor+ends.size(),cursor);
    }

    virtual Actor* copy(Space& home) {
      return new (home) PaperProjection(home,*this);
    }

    virtual PropCost cost(const Space&, const ModEventDelta&) const {
      return PropCost::linear(PropCost::HI,items.size()+x.size());
    }

    virtual size_t dispose(Space& home) {
      home.ignore(*this,AP_DISPOSE);
      items.~IntSharedArray();
      ends.~IntSharedArray();
      (void) NaryPropagator<Int::IntView,Int::PC_INT_VAL>::dispose(home);
      return sizeof(*this);
    }

    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
      for (;;) {
        while ((consumed < x.size()) && x[consumed].assigned()) {
          const int a = x[consumed].val();
          if (a == 0) {
            for (int j=consumed+1; j<x.size(); j++)
              GECODE_ME_CHECK(x[j].eq(home,0));
            return home.ES_SUBSUMED(*this);
          }
          for (int i=0; i<ends.size(); i++) {
            while ((cursor[i] < ends[i]) && (items[cursor[i]] != a))
              cursor[i]++;
            if (cursor[i] == ends[i])
              return ES_FAILED;
            cursor[i]++;
          }
          consumed++;
        }
        if (consumed == x.size())
          return home.ES_SUBSUMED(*this);

        bool frequent[26];
        std::fill(frequent,frequent+26,true);
        for (int i=0; i<ends.size(); i++) {
          bool present[26] = {};
          for (int j=cursor[i]; j<ends[i]; j++)
            present[items[j]-'A'] = true;
          for (int a=0; a<26; a++)
            frequent[a] = frequent[a] && present[a];
        }
        // Zero always remains available after a supported nonempty prefix.
        int values[27];
        int n = 1;
        values[0] = 0;
        for (int a=0; a<26; a++)
          if (frequent[a])
            values[n++] = 'A'+a;
        const IntSet allowed(values,n);
        for (int j=consumed; j<x.size(); j++) {
          IntSetValues v(allowed);
          GECODE_ME_CHECK(x[j].inter_v(home,v,false));
        }
        // Filtering can assign the next position: consume it before fixing.
        if (!x[consumed].assigned())
          return ES_FIX;
      }
    }
  };

  /// One enumeration problem using a selected pattern representation
  class Pattern : public Space {
  protected:
    OpenIntVarList open;
    IntVarArray padded;
    Model model;
  public:
    Pattern(const std::vector<std::string>& input, Model m,
            const std::string& restriction, int bound)
      : model(m) {
      bool present[26] = {};
      for (const std::string& sequence : input)
        for (char a : sequence)
          present[a-'A'] = true;
      int values[27];
      int n = 1;
      values[0] = 0;
      for (int a=0; a<26; a++)
        if (present[a])
          values[n++] = 'A'+a;
      if (model == Model::OpenDFA) {
        open = OpenIntVarList(*this,IntSet(values+1,n-1),bound);
        rel(*this,open.length(),IRT_GQ,1);
        for (const std::string& sequence : input)
          extensional(*this,open,subsequences(sequence,false));
        if (restriction != "none")
          extensional(*this,open,language(restriction));
        branch(*this,open,INT_VAR_NONE(),INT_VAL_MIN());
        return;
      }
      padded = IntVarArray(*this,bound,IntSet(values,n));
      rel(*this,padded[0],IRT_NQ,0);
      if (model == Model::PaddedDFA) {
        for (const std::string& sequence : input)
          extensional(*this,padded,subsequences(sequence,true));
      } else {
        int size = 0;
        for (const std::string& sequence : input)
          size += static_cast<int>(sequence.size());
        IntSharedArray items(size), ends(static_cast<int>(input.size()));
        int k = 0, i = 0;
        for (const std::string& sequence : input) {
          for (char a : sequence)
            items[k++] = a;
          ends[i++] = k;
        }
        ViewArray<Int::IntView> x(*this,IntVarArgs(padded));
        (void) new (*this) PaperProjection(*this,x,items,ends);
      }
      if (restriction != "none")
        extensional(*this,padded,language(restriction) + *REG(0));
      branch(*this,padded,INT_VAR_NONE(),INT_VAL_MIN());
    }

    Pattern(Pattern& s) : Space(s), model(s.model) {
      if (model == Model::OpenDFA)
        open.update(*this,s.open);
      else
        padded.update(*this,s.padded);
    }

    virtual Space* copy(void) { return new Pattern(*this); }

    /// Emit actual items only, in the same format for all representations
    void print(void) const {
      if (model == Model::OpenDFA) {
        for (int i=0; i<open.prefix_size(); i++)
          std::cout << static_cast<char>(open[i].val());
      } else {
        for (int i=0; (i<padded.size()) && (padded[i].val() != 0); i++)
          std::cout << static_cast<char>(padded[i].val());
      }
      std::cout << '\n';
    }
  };

}

int
main(int argc, char* argv[]) {
  if (argc != 6) {
    std::cerr << "Usage: compare open-dfa|padded-dfa|paper-projection "
              << "none|motif|re10|re14|re2 FILE print|count max|shortest\n";
    return 1;
  }
  const std::string variant(argv[1]), restriction(argv[2]),
    output(argv[4]), length_bound(argv[5]);
  if (((variant != "open-dfa") && (variant != "padded-dfa") &&
       (variant != "paper-projection")) ||
      ((restriction != "none") && (restriction != "motif") &&
       (restriction != "re10") && (restriction != "re14") &&
       (restriction != "re2")) ||
      ((output != "print") && (output != "count")) ||
      ((length_bound != "max") && (length_bound != "shortest")))
    return 1;
  std::ifstream file(argv[3]);
  std::vector<std::string> input;
  std::string sequence;
  size_t total = 0;
  while (std::getline(file,sequence)) {
    if (sequence.empty() || (sequence.find_first_not_of(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos)) {
      std::cerr << "Expected one nonempty A..Z sequence per line\n";
      return 1;
    }
    total += sequence.size();
    if (total >= static_cast<size_t>(Int::Limits::max))
      return 1;
    input.push_back(sequence);
  }
  if (!file.eof() || input.empty())
    return 1;
  int bound = static_cast<int>(input.front().size());
  for (const std::string& text : input)
    bound = (length_bound == "max") ?
      std::max(bound,static_cast<int>(text.size())) :
      std::min(bound,static_cast<int>(text.size()));
  const Model model = (variant == "open-dfa") ? Model::OpenDFA :
    (variant == "padded-dfa") ? Model::PaddedDFA : Model::PaperProjection;
  const auto start = std::chrono::steady_clock::now();
  Pattern* root = new Pattern(input,model,restriction,bound);
  Search::Options options;
  options.threads = 1;
  options.clone = false;
  DFS<Pattern> engine(root,options);
  unsigned long long solutions = 0;
  while (Pattern* solution = engine.next()) {
    solutions++;
    if (output == "print")
      solution->print();
    delete solution;
  }
  const auto end = std::chrono::steady_clock::now();
  const Search::Statistics stats = engine.statistics();
  std::cerr << "{\"solutions\":" << solutions
            << ",\"nodes\":" << stats.node
            << ",\"failures\":" << stats.fail
            << ",\"propagations\":" << stats.propagate
            << ",\"model_search_seconds\":"
            << std::chrono::duration<double>(end-start).count() << "}\n";
  return 0;
}
